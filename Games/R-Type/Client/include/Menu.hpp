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

#include "Font.hpp"
#include "GameManager.hpp"
#include "Text2D.hpp"

namespace RType::Client {

    constexpr float BUTTON_WIDTH = 400.f;
    constexpr float BUTTON_HEIGHT = 90.f;
    constexpr float BUTTON_GAP = 40.f;
    constexpr int TITLE_SIZE = 120;
    constexpr int LABEL_SIZE = 48;
    constexpr std::size_t USERNAME_MAX_LENGTH = 16;
    constexpr std::size_t IP_MAX_LENGTH = 15;
    constexpr float IP_BUTTON_WIDTH = 400.f;

    /** @brief Button.png: 3 tiles of 400x90 (normal, hovered, disabled). */
    constexpr float BUTTON_SPRITE_WIDTH = 400.f;
    constexpr std::size_t BUTTON_TILE_NORMAL = 0;
    constexpr std::size_t BUTTON_TILE_HOVERED = 1;
    constexpr std::size_t BUTTON_TILE_DISABLED = 2;
    /** @brief Labels shrink below LABEL_SIZE to keep this margin. */
    constexpr float BUTTON_TEXT_PADDING = 30.f;

    struct MenuItem {};

    /** @brief Tag of the background sprite shown outside of the game. */
    struct MenuBackground {};

    /**
     * @brief Alignment of a Text2D around its Position2D, in fraction of its
     * size: {0, 0} = top-left, {0.5, 0.5} = centered, {1, 0.5} = right.
     */
    struct TextAnchor {
        float x = 0.5f;
        float y = 0.5f;
    };

    /**
     * @brief Put on the Text2D entity displaying the label of a Button.
     * Destroyed automatically when its button is destroyed.
     */
    struct ButtonLabel {
        Entity owner;
    };

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
        bool disabled = false;
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
    Entity addButton(GameManager &gm, float x, float y,
        const std::string &label, std::function<void(GameManager &)> onClick,
        float width = BUTTON_WIDTH);
    Entity addText(GameManager &gm, float x, float y, const std::string &str,
        float size, TextAnchor anchor = {},
        raylib::Color color = raylib::Color::White());
    void connectToServer(
        GameManager &gm, const std::string &host, const int port);
    bool isValidIp(const std::string &ip);
    
    void systemButtons(GameManager &gm);
    void systemButtonVisuals(GameManager &gm);
    void systemTextAnchor(Text2D &text, Font &font, const TextAnchor &anchor);
    void systemMenuBackground(GameManager &gm);
    void systemConnecting(GameManager &gm);
    void systemServerLost(GameManager &gm);
    void systemTextFields(GameManager &gm);

} // namespace RType::Client

#endif /* !MENU_HPP_ */
