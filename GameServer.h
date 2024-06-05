#pragma once

#include "Bot.h"
#include "GameHistory.h"
#include "GameEngine.h"
#include "serialize.h"

#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>

#include <json.hpp>

#include <functional>
#include <mutex>
#include <thread>
#include <memory>

class GameServer {
public:
    using Server = websocketpp::server<websocketpp::config::asio>;

    GameServer(IBot* humanBot, IBot* aiBot)
        : humanBot_(humanBot)
        , aiBot_(aiBot)
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

    void playNewGame(NewGameParams params) {
        stopOngoingGame();
        gameThread_.reset(new std::thread(&GameServer::playNewGameJob, this, params));
    }

    void setupGeCallbacks(GameState& gs) {
        ge_->setEventLogger([this] (LogEvent event) {
            std::lock_guard lock(sendLogsMutex_);
            logEvents_.emplace_back(event);
        });

        ge_->setLogger([this] (const std::string& str) {
            std::lock_guard lock(sendLogsMutex_);
            curLogs_.emplace_back(str);
        });
        ge_->setWpStatser([this, &gs] (WpSource source, int amount) {
            curScores_[gs.activePlayer].at(gs.round)[source] += amount;
        });
        ge_->setLogCheckpointer([this, &gs] () {
            {
                std::lock_guard lock(sendLogsMutex_);
                states_.emplace_back(GameInfo{
                    .gs = gs.clone(),
                    .logs = std::move(curLogs_),
                    .scores = curScores_[gs.activePlayer % 2],
                    .logEvents = std::move(logEvents_)
                });
            
                curLogs_.clear();
                logEvents_.clear();
            }
            sendLogs();
        });
    }

    void playNewGameJob(NewGameParams params) {
        bots_.clear();
        for (const auto [idx, isHuman]: enumerate(params.isHuman)) {
            if (isHuman) {
                bots_.push_back(humanBot_);
            } else {
                bots_.push_back(aiBot_);
            }
            bots_.back()->reset();
        }
        {
            std::lock_guard lock(sendLogsMutex_);
            states_.clear();
        }

        ge_.reset(new GameEngine(bots_, true, true));
        
        GameState gs;
        std::default_random_engine rng{params.seed};

        setupGeCallbacks(gs);

        try {
            ge_->initializeRandomly(gs, rng, params.races, params.colors);
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
        {
            std::lock_guard lock(sendLogsMutex_);
            states_.resize(state + 1);
        }

        ge_.reset(new GameEngine(bots_, true, true));
        
        GameState gs = states_.back().gs.clone();

        setupGeCallbacks(gs);

        try {
            gameLoop(gs);

            // Just wait while user is watching on the final results
            while (true) {
                std::this_thread::sleep_for(std::chrono::seconds(10));
            }
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
        std::lock_guard lock(sendLogsMutex_);

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
                NewGameParams params;
                std::cerr << "New Game! " << *lastResponse_ << std::endl;
                fromJson(resp, params);
                playNewGame(params);
            } else if (resp["action"] == "rewind") {
                rewindState(resp["state"].get<int>());
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
    std::vector<LogEvent> logEvents_;
    std::unique_ptr<GameEngine> ge_;

    IBot* humanBot_;
    IBot* aiBot_;
    std::vector<IBot*> bots_;

    bool resetGame_ = false;

    std::vector<std::string> curLogs_;
    std::array<WpByRound, 2> curScores_;

    std::mutex sendLogsMutex_;
};
