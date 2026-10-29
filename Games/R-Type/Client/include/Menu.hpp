/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Menu
*/

#ifndef MENU_HPP_
#define MENU_HPP_

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
    
    struct MenuItem {};

    struct ConnectingText {};


    struct Button {
        Rectangle rect;
        std::string label;
        std::function<void(GameManager &)> onClick;
        bool hovered = false;
    };

    inline bool quitRequested = false;

    void createMenu(GameManager &gm);
    void connectToServer(GameManager &gm);
    void systemButtons(GameManager &gm);
    void systemDrawMenu(GameManager &gm);
    void systemConnecting(GameManager &gm);
    void systemServerLost(GameManager &gm);

} // namespace RType::Client

#endif /* !MENU_HPP_ */
