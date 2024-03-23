#pragma once

#include "GameHistory.h"
#include "GameEngine.h"

#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>

#include <json.hpp>

#include <thread>
#include <memory>

class GameServer {
public:
    using Server = websocketpp::server<websocketpp::config::asio>;

    GameServer() {
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
    void playNewGame(uint32_t seed, const std::vector<IBot*>& bots) {
        ge_.reset(new GameEngine(bots, true, true));
        StaticGameState sgs;
        GameState gs { .staticGs = sgs };
        std::default_random_engine rng{seed};
        ge_->initializeRandomly(gs, rng);

        gameLoop(gs);
    }

    void gameLoop(GameState& gs) {
        while (!ge_->gameEnded(gs)) {
            const auto field = gs.field();
            gs.cache->reset();
            gs.fieldStateIdx = 0;
            gs.cache->fieldByState_.push_back(field);

            std::vector<std::string> logs;
            ge_->setLogger([&logs] (const std::string& str) {
                logs.emplace_back(str);
            });

            ge_->advanceGs(gs);

            states_.emplace_back(GameInfo{
                .gs = gs,
                .field = gs.field(),
                .logs = std::move(logs)
            });
        }
    }

    void on_message(websocketpp::connection_hdl hdl, Server::message_ptr msg) {
        std::cout << "GameServer Received message: " << msg->get_payload() << std::endl;

        lastResponse_ = msg->get_payload();
    }

    void on_open(websocketpp::connection_hdl hdl) {
        hClient_ = hdl;
        std::cout << "GameServer Received connection!" << std::endl;

        if (lastRequest_) {
            std::cout << "Seems like reconnections, resend message!" << std::endl;
            server_.send(*hClient_, *lastRequest_, websocketpp::frame::opcode::text);
        }
    }

    nlohmann::json rpc(const std::string& msg, const nlohmann::json& data) {
        nlohmann::json j;
        j["data"] = data;
        j["msg"] = msg;

        lastResponse_ = std::nullopt;
        lastRequest_ = j.dump();
        server_.send(*hClient_, *lastRequest_, websocketpp::frame::opcode::text);
        while (!lastResponse_.has_value()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
        lastRequest_ = std::nullopt;
        return nlohmann::json::parse(*lastResponse_);
    }

    Server server_;
    std::optional<websocketpp::connection_hdl> hClient_;
    websocketpp::lib::shared_ptr<websocketpp::lib::thread> thread_;
    std::unique_ptr<std::thread> gameThread_;

    std::optional<std::string> lastResponse_;
    std::optional<std::string> lastRequest_;

    GameHistory states_;
    std::unique_ptr<GameEngine> ge_;
};