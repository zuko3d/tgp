#pragma once
#include "Bot.h"
#include "serialize.h"
#include "StaticData.h"

#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>

#include <json.hpp>

#include <functional>
#include <optional>
#include <random>

class WebUiBot: public IBot {
public:
    using Server = websocketpp::server<websocketpp::config::asio>;

    WebUiBot(std::default_random_engine g)
        : rng(g)
    {
        using namespace std::placeholders;
        server_.set_access_channels(websocketpp::log::alevel::all);
        server_.clear_access_channels(websocketpp::log::alevel::frame_payload);
        server_.set_message_handler(std::bind(&WebUiBot::on_message, this, _1, _2));
        server_.set_open_handler(std::bind(&WebUiBot::on_open, this, _1));

        server_.init_asio();
        server_.listen(9002);
        server_.start_accept();
        // server_.run();
        thread_.reset(new websocketpp::lib::thread(&Server::run, &server_));
        std::cout << "WebUiBot listening on 9002" << std::endl;
        while (!hClient_.has_value()) {
            std::cout << "Waiting for web-client..." << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }

    ~WebUiBot() {
        std::cout << "WebUiBot closing" << std::endl;
        server_.stop();
        thread_->join();
    }

    void triggerFinal(const GameState& gs) {
        std::cout << "triggerFinal..." << std::endl;

        nlohmann::json j;
        j["action"] = "triggerFinal";

        const auto ret = rpc(gs, j.dump());
    };

    Race chooseRace(const GameState& gs, const std::vector<Race>& races) {
        std::cout << "chooseRace..." << std::endl;

        nlohmann::json j;
        j["action"] = "chooseRace";
        j["choices"] = toJson(races);

        const auto ret = rpc(gs, j.dump());

        return (Race) ret["choice"].get<int>();
    }

    TerrainType chooseTerrainType(const GameState& gs, const std::vector<TerrainType>& colors) {
        std::cout << "chooseTerrainType..." << std::endl;

        nlohmann::json j;
        j["action"] = "chooseTerrainType";
        j["choices"] = toJson(colors);

        const auto ret = rpc(gs, j.dump());

        return (TerrainType) ret["choice"].get<int>();
    }
    int chooseRoundBooster(const GameState& gs) {
        std::cout << "chooseRoundBooster..." << std::endl;

        nlohmann::json j;
        j["action"] = "chooseRoundBooster";
        j["boosters"] = toJson(gs.boosters);

        const auto ret = rpc(gs, j.dump());

        return ret["choice"].get<int>();
    }

    FullAction chooseAction(const GameState& gs, const std::vector<Action>& actions) {

        std::cout << "chooseAction..." << std::endl;

        nlohmann::json j;
        j["action"] = "chooseAction";
        j["variants"] = toJson(actions);

        const auto ret = rpc(gs, j.dump());

        if (ret.contains("freeAction")) {
            return FullAction{
                .preAction = { (FreeActionMarketType) ret["freeAction"].get<int>()},
                .action = Action{
                    .type = ActionType::None
                },
                .postAction = {}
            };
        } else {
            std::vector<FreeActionMarketType> postAction;
            if (ret.contains("freeActionPost")) {
                postAction.push_back((FreeActionMarketType) ret["freeActionPost"].get<int>());
            }
            return FullAction{
                .preAction = {},
                .action = Action{
                    .type = (ActionType) ret["type"].get<int>(),
                    .param1 = ret["param1"].get<int>(),
                    .param2 = ret["param2"].get<int>(),
                },
                .postAction = postAction,
            };
        }
    }

    GodColor chooseGodToMove(const GameState& gs, int amount) {
        std::cout << "chooseGodToMove..." << std::endl;

        nlohmann::json j;
        j["action"] = "chooseGodToMove";

        const auto ret = rpc(gs, j.dump());

        return (GodColor) ret["choice"].get<int>();
    }

    FlatMap<BookColor, int8_t, 4> chooseBookColorToGet(const GameState& gs, int amount) {
        std::cout << "chooseBookColorToGet..." << std::endl;

        FlatMap<BookColor, int8_t, 4> ret;

        for (int i = 0; i < amount; i++) {
            nlohmann::json j;
            j["action"] = "chooseBookColorToGet";
            j["amount"] = amount - i;
            const auto response = rpc(gs, j.dump())["choice"].get<int>();
            ret[(BookColor) response]++;
        }
        return ret;
    }
    
    FlatMap<BookColor, int8_t, 4> chooseBooksToSpend(const GameState& gs, int amount) {
        std::cout << "chooseBooksToSpend..." << std::endl;

        FlatMap<BookColor, int8_t, 4> ret;

        auto books = gs.players[gs.activePlayer].resources.books;
        for (int i = 0; i < amount; i++) {
            nlohmann::json j;
            j["action"] = "chooseBooksToSpend";
            j["amount"] = amount - i;
            std::vector<BookColor> colors;
            for (const auto [color, val]: books) {
                if (val > 0) {
                    colors.push_back(color);
                }
            }
            j["colors"] = toJson(colors);
            const BookColor response = (BookColor) rpc(gs, j.dump())["choice"].get<int>();
            ret[response]++;
            books[response]--;
        }
        return ret;
    }

    // How many bricks you want to spend on terraforming POS hex
    int8_t chooseBricks(const GameState& gs, int8_t pos) { return 0; }
    
    bool wannaBuildMine(const GameState& gs, int8_t coord) { 
        std::cout << "wannaBuildMine..." << std::endl;

        nlohmann::json j;
        j["action"] = "wannaBuildMine";
        j["coord"] = coord;

        const auto ret = rpc(gs, j.dump());

        return (bool) ret["choice"].get<int>();
    }

    bool wannaCharge(const GameState& gs, int amount) {
        std::cout << "wannaCharge..." << std::endl;

        nlohmann::json j;
        j["action"] = "wannaCharge";
        j["amount"] = amount;

        const auto ret = rpc(gs, j.dump());

        return (bool) ret["choice"].get<int>();
    }
    
    FedTileOrigin chooseFedTile(const GameState& gs) {
        std::vector<FedTileOrigin> variants;
        for (const auto [tile, amnt]: gs.fedTilesAvailable) {
            if (amnt > 0) {
                variants.push_back(tile);
            }
        }
        std::cout << "chooseFedTile..." << std::endl;

        nlohmann::json j;
        j["action"] = "chooseFedTile";
        j["variants"] = toJson(variants);

        const auto ret = rpc(gs, j.dump());

        return (FedTileOrigin) ret["choice"].get<int>();
    }

    TechTile chooseTechTile(const GameState& gs) {
        const auto& ps = gs.players[gs.activePlayer];
        std::vector<TechTile> variants;
        for (int i = 0; i < 12; i++) {
            TechTile tile = (TechTile) i;
            if (!ps.techTiles[tile]) variants.push_back(tile);
        }
        std::cout << "chooseTechTile..." << std::endl;

        nlohmann::json j;
        j["action"] = "chooseTechTile";
        j["variants"] = toJson(variants);

        const auto ret = rpc(gs, j.dump());

        return (TechTile) ret["choice"].get<int>();
    }

    int8_t choosePlaceToSpade(const GameState& gs, int amount, const std::vector<int8_t>& possiblePos) {
        if (possiblePos.empty()) return -1;
        std::cout << "choosePlaceToSpade..." << std::endl;

        nlohmann::json j;
        j["action"] = "choosePlaceToSpade";
        j["possiblePos"] = toJson(possiblePos);
        j["amount"] = amount;

        const auto ret = rpc(gs, j.dump());

        return (int8_t) ret["choice"].get<int>();
    }

    int8_t choosePlaceForBridge(const GameState& gs, const std::vector<int8_t>& possiblePos) {
        if (possiblePos.empty()) return -1;
        std::cout << "choosePlaceForBridge..." << std::endl;

        std::vector<std::array<int8_t, 2>> possiblePairs;
        for (const auto& p: possiblePos) {
            const auto& pair = StaticData::fieldOrigin().bridgeConnections[p];
            possiblePairs.push_back({pair.first, pair.second});
            possiblePairs.push_back({pair.second, pair.first});
        }

        nlohmann::json j;
        j["action"] = "choosePlaceForBridge";
        j["possiblePairs"] = toJson(possiblePairs);

        const auto ret = rpc(gs, j.dump());

        for (const auto& p: possiblePos) {
            const auto& pair = StaticData::fieldOrigin().bridgeConnections[p];
            if ((ret["from"].get<int>() == pair.first && ret["to"].get<int>() == pair.second) || (ret["from"].get<int>() == pair.second && ret["to"].get<int>() == pair.first)) {
                return p;
            }
        }

        return -1;
    }

    int8_t choosePlaceToBuildForFree(const GameState& gs,  Building building, const std::vector<int8_t>& possiblePos) {
        if (possiblePos.empty()) return -1;
        std::cout << "choosePlaceToBuildForFree..." << std::endl;

        nlohmann::json j;
        j["action"] = "choosePlaceToBuildForFree";
        j["choices"] = toJson(possiblePos);
        j["building"] = toJson(building);

        const auto ret = rpc(gs, j.dump());

        return ret["choice"].get<int>();
    }

private:
    // void on_message(Server* s, websocketpp::connection_hdl hdl, Server::message_ptr msg) {
    void on_message(websocketpp::connection_hdl hdl, Server::message_ptr msg) {
        std::cout << "Received message: " << msg->get_payload() << std::endl;

        lastResponse_ = msg->get_payload();
    }

    void on_open(websocketpp::connection_hdl hdl) {
        hClient_ = hdl;
        std::cout << "Received connection!" << std::endl;

        if (lastRequest_) {
            std::cout << "Seems like reconnections, resend message!" << std::endl;
            server_.send(*hClient_, *lastRequest_, websocketpp::frame::opcode::text);
        }
    }

    nlohmann::json rpc(const GameState& gs, const std::string& msg) {
        nlohmann::json j;
        j["gs"] = toJson(gs);
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

    std::default_random_engine rng;
    Server server_;
    std::optional<websocketpp::connection_hdl> hClient_;
    websocketpp::lib::shared_ptr<websocketpp::lib::thread> thread_;
    std::optional<std::string> lastResponse_;
    std::optional<std::string> lastRequest_;
};
