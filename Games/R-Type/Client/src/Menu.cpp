/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Menu
*/

#include <vector>

#include "ClientManager.hpp"
#include "Lobby.hpp"
#include "LobbyProtocol.hpp"
#include "Menu.hpp"
#include "Text2D.hpp"
#include "Assets.hpp"
#include "Clock.hpp"
#include "SpriteSheet2D.hpp"
#include "Texture2D.hpp"

namespace RType::Client {

    bool isValidIp(const std::string &ip)
    {
        std::error_code ec;
        
        asio::ip::make_address_v4(ip, ec);
        return !ec;
    }

    void showConnecting(GameManager &gm)
    {
        const Entity e = gm.createEntity();
        Text2D text;

        text.fontSize = 40;
        text.str = "Connexion";
        gm.addComponent<ConnectingText>(e);
        gm.addComponent<Clock>(e);
        gm.addComponent<Text2D>(e, text);
        gm.addComponent<Font>(e, gm.getAssetsMap().at(Asset::FONT));
        gm.addComponent<Position2D>(e, 1920.f / 2.f - 230.f, 1080.f / 2.f - 30.f);
    }

    Entity addText(GameManager &gm, float x, float y, const std::string &str,
        float size, TextAnchor anchor, raylib::Color color)
    {
        const Entity e = gm.createEntity();
        Text2D text;

        text.fontSize = size;
        text.str = str;
        text.color = color;
        text.spacing = 1.f;
        gm.addComponent<Text2D>(e, text);
        gm.addComponent<Font>(e, gm.getAssetsMap().at(Asset::FONT));
        gm.addComponent<Position2D>(e, x, y);
        gm.addComponent<TextAnchor>(e, anchor);
        return e;
    }

    Entity addButton(GameManager &gm, float x, float y,
        const std::string &label, std::function<void(GameManager &)> onClick,
        float width)
    {
        const Entity e = gm.createEntity();
        const float scale = width / BUTTON_SPRITE_WIDTH;
        const float height = BUTTON_HEIGHT * scale;

        gm.addComponent<Button>(e, Button {{x, y, width, height},
                label, std::move(onClick), false, false, false});
        gm.addComponent<Position2D>(e, x, y);
        gm.addComponent<SpriteSheet2D>(
            e, Size2D {1, 3}, BUTTON_TILE_NORMAL, 0, scale, false);
        gm.addComponent<Texture2D>(e, gm.getAssetsMap().at(Asset::MENU_BUTTON));
        const Entity text =
            addText(gm, x + width / 2.f, y + height / 2.f, label, LABEL_SIZE);
        gm.addComponent<ButtonLabel>(text, ButtonLabel {e});
        return e;
    }

    namespace {
        Entity addMenuButton(GameManager &gm, float x, float y,
            const std::string &label,
            std::function<void(GameManager &)> onClick,
            float width = BUTTON_WIDTH)
        {
            const Entity e = addButton(gm, x, y, label, std::move(onClick), width);

            gm.addComponent<MenuItem>(e);
            return e;
        }
    } // namespace

    void startGame(GameManager &gm)
    {
        if (!gm.view<ClientManager>().getVector().empty())
            return;
        if (!isValidIp(_clInfos._ip)) {
            ipInvalid = true;
            return;
        }
        activeField = Field::None;
        std::vector<Entity> items;

        gm.view<MenuItem>().each([&items](Entity e, MenuItem &) { items.push_back(e); });
        for (const Entity e : items)
            gm.destroyEntity(e);
        connectToServer(gm, _clInfos._ip, LOGIN_SERVER_PORT);
        showConnecting(gm);
    }

    void createMenu(GameManager &gm)
    {
        const float firstY = 480.f;
        const float step = BUTTON_HEIGHT + BUTTON_GAP;
        const float firstX = (1920.f - BUTTON_WIDTH) / 2.f;
        const float ipX = (1920.f - IP_BUTTON_WIDTH) / 2.f;

        addMenuButton(gm, firstX, firstY, "Start Game", startGame);
        const Entity ipButton = addMenuButton(gm, ipX, firstY + step, _clInfos._ip,
            [](GameManager &) { activeField = Field::Ip; }, IP_BUTTON_WIDTH);
        gm.addComponent<TextField>(ipButton, TextField {Field::Ip});
        gm.addComponent<Clock>(ipButton);
        addMenuButton(gm, firstX, firstY + 2 * step, "Leave Game",
            [](GameManager &) { quitRequested = true; });
        const Entity usernameButton = addMenuButton(gm, 20, 1080 - (BUTTON_HEIGHT * 2), _clInfos._username,
            [](GameManager &) { activeField = Field::Username; });
        gm.addComponent<TextField>(usernameButton, TextField {Field::Username});
        gm.addComponent<Clock>(usernameButton);
    }

    void connectToServer(
        GameManager &gm, const std::string &host, const int port)
    {
        const Entity c = gm.createEntity();

        gm.addComponent<ClientManager>(c, host, port);
        gm.addComponent<LobbyState>(c);
        gm.addComponent<LobbyClient>(c);
    }

} // namespace RType::Client
