/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Menu
*/

#include <vector>

#include "Assets.hpp"
#include "ClientManager.hpp"
#include "Clock.hpp"
#include "Lobby.hpp"
#include "LobbyListPage.hpp"
#include "LobbyProtocol.hpp"
#include "Login.hpp"
#include "Menu.hpp"
#include "SpriteSheet2D.hpp"
#include "Text2D.hpp"
#include "Texture2D.hpp"
#include "Settings.hpp"

namespace RType::Client {

    constexpr float MENU_FIRST_Y = 400.f;
    constexpr float MENU_BUTTON_GAP = 15.f;

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
        gm.addComponent<Position2D>(
            e, 1920.f / 2.f - 230.f, 1080.f / 2.f - 30.f);
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

        gm.addComponent<Button>(e,
            Button {{x, y, width, height},
                label,
                std::move(onClick),
                false,
                false,
                false});
        gm.addComponent<Position2D>(e, x, y);
        gm.addComponent<SpriteSheet2D>(
            e, Size2D {1, 3}, BUTTON_TILE_NORMAL, 0, scale, false);
        gm.addComponent<Texture2D>(e, gm.getAssetsMap().at(Asset::MENU_BUTTON));
        const Entity text =
            addText(gm, x + width / 2.f, y + height / 2.f, label, LABEL_SIZE);
        gm.addComponent<ButtonLabel>(text, ButtonLabel {e});
        return e;
    }

    Entity addMenuButton(GameManager &gm, float x, float y,
        const std::string &label, std::function<void(GameManager &)> onClick,
        float width = BUTTON_WIDTH)
    {
        const Entity e = addButton(gm, x, y, label, std::move(onClick), width);

        gm.addComponent<MenuItem>(e);
        return e;
    }

    template<typename Tag>
    void destroyAllTagged(GameManager &gm)
    {
        std::vector<Entity> items;

        gm.view<Tag>().each([&items](Entity e, Tag &) { items.push_back(e); });
        for (const Entity e : items)
            gm.destroyEntity(e);
    }

    Entity addServerButton(GameManager &gm, float x, float y,
        const std::string &label, std::function<void(GameManager &)> onClick)
    {
        const Entity e = addButton(gm, x, y, label, std::move(onClick));

        gm.addComponent<ServerItem>(e);
        return e;
    }

    void connectToGame(GameManager &gm)
    {
        if (!gm.view<ClientManager>().getVector().empty())
            return;
        if (!isValidIp(_clInfos._ip)) {
            ipInvalid = true;
            return;
        }
        activeField = Field::None;
        destroyAllTagged<ServerItem>(gm);
        restoreSavedAccount();
        connectToMaster(gm, _clInfos._ip, LOGIN_SERVER_PORT);
        showConnecting(gm);
    }

    void createServerPage(GameManager &gm)
    {
        const float centerX = 1920.f / 2.f;
        const float fieldY = (1080.f - BUTTON_HEIGHT) / 2.f;
        const float step = BUTTON_HEIGHT + BUTTON_GAP;
        const Entity title = addText(
            gm, centerX, fieldY - 40.f, "Server IP Address", SERVER_LABEL_SIZE);
        const Entity ipButton = addServerButton(gm,
            (1920.f - IP_BUTTON_WIDTH) / 2.f,
            fieldY,
            _clInfos._ip,
            [](GameManager &) { activeField = Field::Ip; });

        gm.addComponent<ServerItem>(title);
        gm.addComponent<TextField>(ipButton, TextField {Field::Ip});
        gm.addComponent<Clock>(ipButton);
        addServerButton(gm,
            (1920.f - BUTTON_WIDTH) / 2.f,
            fieldY + step * (float) 1.5,
            "Connect to Server",
            connectToGame);
        addServerButton(gm,
            1920.f - BUTTON_WIDTH - BUTTON_GAP * 2,
            1080.f - (BUTTON_HEIGHT) * 2 - BUTTON_GAP,
            "Leave Game",
            [](GameManager &) { quitRequested = true; });
    }

    void createMenu(GameManager &gm)
    {
        const float step = BUTTON_HEIGHT + MENU_BUTTON_GAP;
        const float firstX = (1920.f - BUTTON_WIDTH) / 2.f;
        const Entity start = addMenuButton(
            gm, firstX, MENU_FIRST_Y, "Start Game", openLobbyList);
        const Entity login = addMenuButton(
            gm, firstX, MENU_FIRST_Y + step, "Login", openLoginPopup);
        const Entity disconnect = addMenuButton(
            gm, firstX, MENU_FIRST_Y + step * 2.f, "Disconnect", logout);

        gm.addComponent<StartGameButton>(start);
        gm.addComponent<LoginButton>(login);
        gm.addComponent<DisconnectButton>(disconnect);
        addMenuButton(
            gm, firstX, MENU_FIRST_Y + step * 3.f, "Settings", openSettings);
        addMenuButton(gm,
            firstX,
            MENU_FIRST_Y + step * 4.f,
            "Change Server",
            returnToServerPage);
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
