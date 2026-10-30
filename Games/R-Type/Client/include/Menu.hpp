/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Menu
*/

#ifndef MENU_HPP_
#define MENU_HPP_

#include <chrono>
#include <functional>
#include <raylib-cpp.hpp>
#include <string>

#include "GameManager.hpp"

namespace RType::Client {

    constexpr float BUTTON_WIDTH = 400.f;
    constexpr float BUTTON_HEIGHT = 90.f;
    constexpr float BUTTON_GAP = 40.f;
    constexpr int TITLE_SIZE = 120;
    constexpr int LABEL_SIZE = 48;
    constexpr std::size_t USERNAME_MAX_LENGTH = 16;
    constexpr std::size_t IP_MAX_LENGTH = 15;
    constexpr float IP_BUTTON_WIDTH = 480.f;

    struct MenuItem {};

    struct ConnectingText {
        std::chrono::milliseconds elapsed {0};
    };

    enum class Field { None, Username, Ip };

    struct TextField {
        Field id;
        std::chrono::milliseconds elapsed {0};
    };

    struct Button {
        Rectangle rect;
        std::string label;
        std::function<void(GameManager &)> onClick;
        bool hovered = false;
        bool error = false;
    };

    inline bool quitRequested = false;
    inline Field activeField = Field::None;
    inline bool ipInvalid = false;

    struct ClientInfos {
        std::string _username = "Player";
        std::string _ip = "127.0.0.1";
    };

    inline ClientInfos _clInfos;

    void createMenu(GameManager &gm);
    void connectToServer(
        GameManager &gm, const std::string &host, const int port);
    bool isValidIp(const std::string &ip);
    
    void systemButtons(GameManager &gm);
    void systemDrawMenu(GameManager &gm);
    void systemConnecting(GameManager &gm);
    void systemServerLost(GameManager &gm);
    void systemTextFields(GameManager &gm);

} // namespace RType::Client

#endif /* !MENU_HPP_ */
