#pragma once

#include "Bot.h"
#include "GameHistory.h"
#include "GameEngine.h"
#include "serialize.h"

#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>

#include <json.hpp>

#include <functional>
#include <thread>
#include <memory>

class GameServer {
public:
    using Server = websocketpp::server<websocketpp::config::asio>;

    GameServer(const std::vector<IBot*>& bots)
        : bots_(bots)
    {
        using namespace std::placeholders;

        server_.set_access_channels(websocketpp::log::alevel::all);
        server_.clear_access_channels(websocketpp::log::alevel::frame_payload);
        server_.set_message_handler(std::bind(&GameServer::on_message, this, _1, _2));
        server_.set_open_handler(std::bind(&GameServer::on_open, this, _1));

        server_.init_asio();
        server_.listen(9102);
        server_.start_accept();
        // server_.run();
        thread_.reset(new websocketpp::lib::thread(&Server::run, &server_));
        std::cout << "GameServer listening on 9102" << std::endl;
        while (!hClient_.has_value()) {
            std::cout << "Waiting for web-client..." << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }

    ~GameServer() {
        std::cout << "GameServer closing" << std::endl;
        server_.stop();
        thread_->join();
    }

private:
    void stopOngoingGame() {
        if (gameThread_ && gameThread_->joinable()) {
            for (auto& bot: bots_) {
                bot->stop();
            }
            std::cerr << "Waiting current gameThread_ to end" << std::endl;
            gameThread_->join();
        }
    }

    void playNewGame(uint32_t seed, const std::vector<IBot*>& bots) {
        stopOngoingGame();
        gameThread_.reset(new std::thread(&GameServer::playNewGameJob, this, seed, bots));
    }
    void playNewGameJob(uint32_t seed, const std::vector<IBot*>& bots) {
        for (auto bot: bots) {
            bot->reset();
        }
        states_.clear();

        ge_.reset(new GameEngine(bots, true, true));
        ge_->setLogger([this] (const std::string& str) {
            curLogs_.emplace_back(str);
        });
        GameState gs;
        std::default_random_engine rng{seed};

        ge_->setLogCheckpointer([this, &gs] () {
            states_.emplace_back(GameInfo{
                .gs = gs.clone(),
                .logs = std::move(curLogs_)
            });
            curLogs_.clear();
            sendLogs();
        });

        try {
            ge_->initializeRandomly(gs, rng);
            gameLoop(gs);
        } catch (const std::exception& e) {
            if (std::string{e.what()} == "halt") {
                std::cerr << "Halt!" << std::endl;
            }
        }
    }

    void rewindState(uint32_t state) {
        stopOngoingGame();
        gameThread_.reset(new std::thread(&GameServer::rewindStateJob, this, state));
    }
    void rewindStateJob(uint32_t state) {
        for (auto bot: bots_) {
            bot->reset();
        }
        assert(state < states_.size());
        states_.resize(state + 1);

        ge_.reset(new GameEngine(bots_, true, true));
        
        GameState gs = states_.back().gs.clone();

        ge_->setLogCheckpointer([this, &gs] () {
            states_.emplace_back(GameInfo{
                .gs = gs.clone(),
                .logs = std::move(curLogs_)
            });
            curLogs_.clear();
            sendLogs();
        });

        try {
            gameLoop(gs);
        } catch (const std::exception& e) {
            if (std::string{e.what()} == "halt") {
                std::cerr << "Halt!" << std::endl;
            }
        }
    }

    void gameLoop(GameState& gs) {
        while (!ge_->gameEnded(gs) && !resetGame_) {
            const auto field = gs.field();
            gs.cache->reset();
            gs.fieldStateIdx = 0;
            gs.cache->fieldByState_.push_back(field);

            ge_->advanceGs(gs);
        }
    }

    void sendLogs() {
        nlohmann::json j;
        j["action"] = "logs";
        j["data"] = toJson(states_);
        server_.send(*hClient_, j.dump(), websocketpp::frame::opcode::text);
    }

    void on_message(websocketpp::connection_hdl hdl, Server::message_ptr msg) {
        std::cout << "GameServer Received message: " << msg->get_payload() << std::endl;

        lastResponse_ = msg->get_payload();

        const auto resp = nlohmann::json::parse(*lastResponse_);
        if (resp.contains("action")) {
            if (resp["action"] == "new-game") {
                playNewGame(resp["seed"].get<int>(), bots_);
            } else {
                std::cerr << "Unknown action! Msg: " << *lastResponse_ << std::endl;
            }
        }
    }

    void on_open(websocketpp::connection_hdl hdl) {
        hClient_ = hdl;
        std::cout << "GameServer Received connection!" << std::endl;

        // if (lastRequest_) {
        //     std::cout << "Seems like reconnections, resend message!" << std::endl;
        //     server_.send(*hClient_, *lastRequest_, websocketpp::frame::opcode::text);
        // }
        sendLogs();
    }

    // nlohmann::json rpc(const std::string& msg, const nlohmann::json& data) {
    //     nlohmann::json j;
    //     j["data"] = data;
    //     j["msg"] = msg;

    //     lastResponse_ = std::nullopt;
    //     lastRequest_ = j.dump();
    //     server_.send(*hClient_, *lastRequest_, websocketpp::frame::opcode::text);
    //     while (!lastResponse_.has_value()) {
    //         std::this_thread::sleep_for(std::chrono::milliseconds(50));
    //     }
    //     lastRequest_ = std::nullopt;
    //     return nlohmann::json::parse(*lastResponse_);
    // }

    Server server_;
    std::optional<websocketpp::connection_hdl> hClient_;
    websocketpp::lib::shared_ptr<websocketpp::lib::thread> thread_;
    std::unique_ptr<std::thread> gameThread_;

    std::optional<std::string> lastResponse_;
    // std::optional<std::string> lastRequest_;

    GameHistory states_;
    std::unique_ptr<GameEngine> ge_;

    std::vector<IBot*> bots_;
    bool resetGame_ = false;

    std::vector<std::string> curLogs_;
};
