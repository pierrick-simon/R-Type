/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Menu
*/

#include <vector>

#include "ClientManager.hpp"
#include "Menu.hpp"
#include "Text2D.hpp"
#include "Assets.hpp"
#include "Clock.hpp"

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

    Entity addButton(GameManager &gm, float x, float y, const std::string &label,
        std::function<void(GameManager &)> onClick, float width = BUTTON_WIDTH)
    {
        const Entity e = gm.createEntity();

        gm.addComponent<MenuItem>(e);
        gm.addComponent<Button>(e, Button {{x, y, width, BUTTON_HEIGHT},
                label, std::move(onClick), false, false});
        return e;
    }

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

        addButton(gm, firstX, firstY, "Jouer", startGame);
        const Entity ipButton = addButton(gm, ipX, firstY + step, _clInfos._ip,
            [](GameManager &) { activeField = Field::Ip; }, IP_BUTTON_WIDTH);
        gm.addComponent<TextField>(ipButton, TextField {Field::Ip});
        gm.addComponent<Clock>(ipButton);
        addButton(gm, firstX, firstY + 2 * step, "Quitter",
            [](GameManager &) { quitRequested = true; });
        const Entity usernameButton = addButton(gm, 20, 1080 - (BUTTON_HEIGHT * 2), _clInfos._username,
            [](GameManager &) { activeField = Field::Username; });
        gm.addComponent<TextField>(usernameButton, TextField {Field::Username});
        gm.addComponent<Clock>(usernameButton);
    }

    void connectToServer(
        GameManager &gm, const std::string &host, const int port)
    {
        const Entity c = gm.createEntity();

        gm.addComponent<ClientManager>(c, host, port);
    }

} // namespace RType::Client
