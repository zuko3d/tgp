#pragma once
#include "Bot.h"

#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>

#include <functional>
#include <random>

class WebUiBot: public IBot {
public:
    using Server = websocketpp::server<websocketpp::config::asio>;

    WebUiBot(std::default_random_engine g)
        : rng(g)
    {
        using namespace std::placeholders;
        server_.set_message_handler(std::bind(&WebUiBot::on_message, this, _1, _2));

        server_.init_asio();
        server_.listen(9002);
        server_.start_accept();
        // server_.run();
        thread_.reset(new websocketpp::lib::thread(&Server::run, &server_));
        std::cout << "WebUiBot listening on 9002" << std::endl;
    }

    ~WebUiBot() {
        std::cout << "WebUiBot closing" << std::endl;
        server_.stop();
        thread_->join();
    }

    Race chooseRace(const GameState& gs, const std::vector<Race>& races) {
        return races[rng() % races.size()];
    }

    TerrainType chooseTerrainType(const GameState& gs, const std::vector<TerrainType>& colors) {
        return colors[rng() % colors.size()];
    }
    int chooseRoundBooster(const GameState& gs) {
        return rng() % gs.boosters.size();
    }

    FullAction chooseAction(const GameState& gs, const std::vector<Action>& actions) {
        return FullAction{
            .preAction = {},
            .action = actions[rng() % actions.size()],
            .postAction = {}
        };
    }

    GodColor chooseGodToMove(const GameState& gs, int amount) {
        return (GodColor) (rng() % 4);
    }

    FlatMap<BookColor, int8_t, 4> chooseBookColorToGet(const GameState& gs, int amount) { 
        FlatMap<BookColor, int8_t, 4> ret = { 0, 0, 0, 0 };
        ret[(BookColor) (rng() % 4)] = amount;
        return ret;
    }
    FlatMap<BookColor, int8_t, 4> chooseBooksToSpend(const GameState& gs, int amount) {
        FlatMap<BookColor, int8_t, 4> ret = { 0, 0, 0, 0 };
        const auto& ps = gs.players[gs.activePlayer];
        assert(sum(ps.resources.books.values()) >= amount);

        for (const auto [color, val]: ps.resources.books) {
            const auto amnt = std::min((int) val, amount);
            amount -= amnt;
            ret[color] = amnt;
            if (amount == 0) break;
        }

        return ret;
    }

    // How many bricks you want to spend on terraforming POS hex
    int8_t chooseBricks(const GameState& gs, int8_t pos) { return 0; }
    
    bool wannaBuildMine(const GameState& gs, int8_t coord) { return true; }
    bool wannaCharge(const GameState& gs, int amount) { return true; }
    
    FedTileOrigin chooseFedTile(const GameState& gs) {
        for (const auto [tile, amnt]: gs.fedTilesAvailable) {
            if (amnt > 0) {
                return tile;
            }
        }
        assert(false);
        return -1;
    }

    TechTile chooseTechTile(const GameState& gs) {
        const auto& ps = gs.players[gs.activePlayer];
        for (int i = 0; i < 12; i++) {
            TechTile tile = (TechTile) i;
            if (!ps.techTiles[tile]) return tile;
        }
        assert(false);
        return (TechTile) -1;
    }

    int8_t choosePlaceToSpade(const GameState& gs, int amount, const std::vector<int8_t>& possiblePos) {
        if (possiblePos.empty()) return -1;
        
        const auto& ps = gs.players[gs.activePlayer];
        const auto r = gs.field->reachable(gs.activePlayer, ps.navLevel);
        return r[rng() % r.size()];
    }

    int8_t choosePlaceForBridge(const GameState& gs, const std::vector<int8_t>& possiblePos) {
        if (possiblePos.empty()) return -1;
        return possiblePos[rng() % possiblePos.size()];
    }

    int8_t choosePlaceToBuildForFree(const GameState& gs,  Building building, const std::vector<int8_t>& possiblePos) {
        if (possiblePos.empty()) return -1;
        return possiblePos[rng() % possiblePos.size()];
    }

private:
    // void on_message(Server* s, websocketpp::connection_hdl hdl, Server::message_ptr msg) {
    void on_message(websocketpp::connection_hdl hdl, Server::message_ptr msg) {
        std::cout << "Received message: " << msg->get_payload() << std::endl;
    }

    std::default_random_engine rng;
    Server server_;
    websocketpp::lib::shared_ptr<websocketpp::lib::thread> thread_;
};
