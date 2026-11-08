/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** MenuSystems
*/

#include <optional>
#include <vector>

#include "Assets.hpp"
#include "ClientManager.hpp"
#include "Clock.hpp"
#include "Listener.hpp"
#include "Leaderboard.hpp"
#include "Lobby.hpp"
#include "LobbyListPage.hpp"
#include "LobbyProtocol.hpp"
#include "Menu.hpp"
#include "MouseManager.hpp"
#include "Settings.hpp"
#include "Sound.hpp"
#include "Sprite2D.hpp"
#include "SpriteSheet2D.hpp"
#include "Text2D.hpp"
#include "Texture2D.hpp"

namespace RType::Client {

    namespace {

        std::string &fieldValue(const Field field)
        {
            if (field == Field::Ip)
                return _clInfos._ip;
            if (field == Field::LobbyName)
                return _clInfos._lobbyName;
            if (field == Field::Password)
                return _clInfos._password;
            return _clInfos._username;
        }

        std::string fieldFallback(const Field field)
        {
            if (field == Field::Ip)
                return "127.0.0.1";
            if (field == Field::LobbyName)
                return _clInfos._lobbyNameDefault;
            if (field == Field::Password)
                return "";
            return "Player1";
        }

        std::size_t fieldMaxLength(const Field field)
        {
            if (field == Field::Ip)
                return IP_MAX_LENGTH;
            if (field == Field::LobbyName)
                return LOBBY_NAME_MAX_LENGTH;
            if (field == Field::Password)
                return PASSWORD_MAX_LENGTH;
            return USERNAME_MAX_LENGTH;
        }

        bool fieldAccepts(const Field field, const int key)
        {
            if (field == Field::Ip)
                return (key >= '0' && key <= '9') || key == '.';
            if (field == Field::LobbyName) {
                const std::string extra = " !,./;?_";

                return (key >= 'a' && key <= 'z') ||
                    (key >= 'A' && key <= 'Z') || (key >= '0' && key <= '9') ||
                    extra.find(static_cast<char>(key)) != std::string::npos;
            }
            if (field == Field::Password)
                return key > ' ' && key <= '~';
            return (key >= 'a' && key <= 'z') || (key >= 'A' && key <= 'Z') ||
                (key >= '0' && key <= '9') || key == '_';
        }

        bool contains(const Rectangle &rect, const Vector2 &point)
        {
            return point.x >= rect.x && point.x <= rect.x + rect.width &&
                point.y >= rect.y && point.y <= rect.y + rect.height;
        }

        void stopEditing()
        {
            if (activeField == Field::None)
                return;
            if (fieldValue(activeField).empty())
                fieldValue(activeField) = fieldFallback(activeField);
            activeField = Field::None;
        }

    } // namespace

    void systemConnecting(GameManager &gm)
    {
        bool connected = false;

        gm.view<ClientManager, LobbyState>().each(
            [&](Entity, ClientManager &, LobbyState &state) {
                connected = state.received;
            });
        gm.view<ConnectingText, Text2D, Clock>().each(
            [](Entity, ConnectingText &c, Text2D &text, Clock &clock) {
                c.elapsed += clock.getElapsed();
                const auto dots = (c.elapsed.count() / 400) % 4;
                text.str = "Connexion" +
                    std::string(static_cast<std::size_t>(dots), '.');
            });
        if (!connected)
            return;
        gm.view<ConnectingText>().each(
            [&gm](Entity e, ConnectingText &) { gm.destroyEntity(e); });
    }

    void systemTextFields(GameManager &gm)
    {
        if (activeField != Field::None) {
            std::string &value = fieldValue(activeField);

            if (activeField == Field::Ip)
                ipInvalid = false;
            for (int key = ::GetCharPressed(); key > 0;
                key = ::GetCharPressed()) {
                if (fieldAccepts(activeField, key) &&
                    value.size() < fieldMaxLength(activeField))
                    value += static_cast<char>(key);
            }
            if (::IsKeyPressed(KEY_BACKSPACE) && !value.empty())
                value.pop_back();
            if (::IsKeyPressed(KEY_ENTER) || ::IsKeyPressed(KEY_ESCAPE))
                stopEditing();
        }
        gm.view<TextField, Button, Clock>().each(
            [](Entity, TextField &f, Button &button, Clock &clock) {
                f.elapsed += clock.getElapsed();
                const bool blink =
                    activeField == f.id && (f.elapsed.count() / 400) % 2 == 0;
                const std::string &value = fieldValue(f.id);
                button.label =
                    (f.id == Field::Password ? std::string(value.size(), '*')
                                             : value) +
                    (blink ? "_" : "");
                button.error = f.id == Field::Ip && ipInvalid;
            });
    }

    void systemServerLost(GameManager &gm)
    {
        bool lost = false;
        bool masterLost = false;

        gm.view<ClientManager>().each([&](Entity e, ClientManager &c) {
            if (!c.isTimedOut())
                return;
            lost = true;
            if (gm.hasComponent<LobbyListClient>(e))
                masterLost = true;
        });
        if (masterLost)
            returnToServerPage(gm);
        else if (lost)
            returnToMenu(gm);
    }

    void systemButtons(GameManager &gm)
    {
        const Vector2 mouse = ::GetMousePosition();
        bool pressed = false;
        std::vector<std::function<void(GameManager &)>> clicked;

        gm.view<MouseManager>().each([&pressed](Entity, MouseManager &m) {
            for (const auto button : m.buttonPressed)
                if (button == MouseButton::Value::Left)
                    pressed = true;
        });

        if (activeField != Field::None && pressed) {
            bool onActiveField = false;
            gm.view<TextField, Button>().each(
                [&](Entity, TextField &f, Button &button) {
                    if (f.id == activeField && contains(button.rect, mouse))
                        onActiveField = true;
                });
            if (!onActiveField)
                stopEditing();
        }
        gm.view<Button>().each([&](Entity, Button &button) {
            button.hovered = contains(button.rect, mouse);
            if (button.hovered && pressed && !button.disabled)
                clicked.push_back(button.onClick);
        });
        for (auto &callback : clicked)
            callback(gm);
    }

    void systemButtonVisuals(GameManager &gm, Assets &assets)
    {
        std::vector<Entity> orphans;
        gm.view<Button, SpriteSheet2D>().each(
            [&gm](Entity e, Button &button, SpriteSheet2D &sheet) {
                const bool editing = gm.hasComponent<TextField>(e) &&
                    gm.getComponent<TextField>(e).id == activeField;

                if (button.disabled)
                    sheet.currentTile = BUTTON_TILE_DISABLED;
                else if (button.hovered || editing || button.selected)
                    sheet.currentTile = BUTTON_TILE_HOVERED;
                else
                    sheet.currentTile = BUTTON_TILE_NORMAL;
                sheet.color = button.error ? raylib::Color(255, 110, 110, 255)
                                           : raylib::Color::White();
            });
        for (auto [e, label, text] :
            gm.view<ButtonLabel, Text2D>().getVector()) {
            if (!gm.hasComponent<Button>(label.owner)) {
                orphans.push_back(e);
                continue;
            }
            if (!text.assetId)
                continue;
            const Button &button = gm.getComponent<Button>(label.owner);
            const float maxWidth =
                button.rect.width - 2.f * BUTTON_TEXT_PADDING;
            float width = 0.f;
            auto assetFont = assets.get<Font>(text.assetId.value());
            if (!assetFont)
                continue;
            auto &font = assetFont.value().get();
            text.str = button.label;
            text.fontSize = LABEL_SIZE;
            width = font.MeasureText(text.str, text.fontSize, text.spacing).x;
            if (width > maxWidth && width > 0.f)
                text.fontSize = LABEL_SIZE * maxWidth / width;
        }
        for (const Entity e : orphans)
            gm.destroyEntity(e);
    }

    void systemTextAnchor(Text2D &text, Font &font, const TextAnchor &anchor)
    {
        const auto size =
            font.MeasureText(text.str, text.fontSize, text.spacing);

        text.origin.x = size.x * anchor.x;
        text.origin.y = size.y * anchor.y;
    }

    namespace {

        std::optional<AssetId> menuTrack(GameManager &gm)
        {
            std::optional<AssetId> track = Asset::MENU_MUSIC;

            gm.view<LobbyState, LobbyClient>().each(
                [&track](Entity, LobbyState &state, LobbyClient &l) {
                    if (l.inGame && !l.gameOverShown)
                        track = std::nullopt;
                    else if (l.inGame)
                        track = state.victory ? Asset::VICTORY_MUSIC
                                              : Asset::GAME_OVER_MUSIC;
                });
            return track;
        }

    } // namespace

    void systemMenuMusic(GameManager &gm, Assets &assets)
    {
        const std::optional<AssetId> track = menuTrack(gm);
        std::vector<Entity> stale;
        bool playing = false;

        gm.view<MenuMusic>().each([&](Entity e, MenuMusic &music) {
            if (track && music.track == *track)
                playing = true;
            else
                stale.push_back(e);
        });
        for (const Entity e : stale)
            gm.destroyEntity(e);
        if (!track || playing)
            return;
        auto decoder = assets.get<Decoder>(*track);

        if (!decoder)
            return;
        for (auto [_, listener] : gm.view<Listener>().getVector()) {
            const Entity music = gm.createEntity();
            SoundInfo info;

            info.state = SoundInfo::PLAY;
            info.loop = *track == Asset::MENU_MUSIC;
            gm.addComponent<MenuMusic>(music, MenuMusic {*track});
            gm.addComponent<Sound>(music, listener, decoder->get(), info);
        }
    }

    void systemMenuBackground(GameManager &gm)
    {
        bool inGame = false;
        bool lobbyPage = false;
        std::vector<Entity> backgrounds;
        std::optional<AssetId> wanted = Asset::MENU_BACKGROUND;

        gm.view<LobbyClient>().each([&](Entity, LobbyClient &l) {
            inGame = inGame || l.inGame;
            lobbyPage = lobbyPage || l.shown;
        });
        gm.view<LobbyListClient>().each(
            [&lobbyPage](Entity, LobbyListClient &l) {
                lobbyPage = lobbyPage || l.shown;
            });
        if (inGame)
            wanted = std::nullopt;
        else if (lobbyPage)
            wanted = Asset::MENU_BACK_SIDE;
        else if (!gm.view<ConnectingText>().getVector().empty() ||
            !gm.view<SettingsItem>().getVector().empty() ||
            !gm.view<LeaderboardItem>().getVector().empty())
            wanted = Asset::MENU_BACK;
        bool upToDate = false;

        gm.view<MenuBackground>().each(
            [&](Entity e, MenuBackground &background) {
                if (wanted && background.asset == *wanted && !upToDate)
                    upToDate = true;
                else
                    backgrounds.push_back(e);
            });
        for (const Entity e : backgrounds)
            gm.destroyEntity(e);
        if (!wanted || upToDate)
            return;
        const Entity e = gm.createEntity();

        gm.addComponent<MenuBackground>(e, MenuBackground {*wanted});
        gm.addComponent<Sprite2D>(e, *wanted);
        gm.addComponent<Position2D>(e, 0.f, 0.f);
    }

} // namespace RType::Client
