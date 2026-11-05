/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Login
*/

#include "Login.hpp"

#include <cstdio>
#include <fstream>
#include <optional>
#include <vector>

#include "Client.hpp"
#include "ClientManager.hpp"
#include "Clock.hpp"
#include "KeyboardManager.hpp"
#include "Menu.hpp"

namespace RType::Client {

    namespace {

        const raylib::Color COLOR_INFO(200, 205, 225, 255);
        const raylib::Color COLOR_ERROR(255, 110, 110, 255);

        struct SavedAccount {
            std::string ip;
            std::string username;
            std::string password;
        };

        std::optional<SavedAccount> readCredentials()
        {
            std::ifstream file(CREDENTIALS_PATH);
            SavedAccount saved;

            if (!std::getline(file, saved.ip) ||
                !std::getline(file, saved.username) ||
                !std::getline(file, saved.password))
                return std::nullopt;
            if (!isValidIp(saved.ip) ||
                !AccountProtocol::isValidUsername(saved.username) ||
                saved.password.empty())
                return std::nullopt;
            return saved;
        }

        void saveCredentials()
        {
            std::ofstream file(CREDENTIALS_PATH, std::ios::trunc);

            file << _clInfos._ip << '\n'
                 << _account.username << '\n'
                 << _account.password << '\n';
        }

        void deleteCredentials()
        {
            std::remove(CREDENTIALS_PATH);
        }

        struct MasterLogin {
            ClientManager *client = nullptr;
            LoginClient *login = nullptr;
        };

        MasterLogin findMaster(GameManager &gm)
        {
            MasterLogin master;

            gm.view<ClientManager, LoginClient>().each(
                [&master](Entity, ClientManager &c, LoginClient &l) {
                    master = {&c, &l};
                });
            return master;
        }

        template<typename Tag>
        void destroyTagged(GameManager &gm)
        {
            std::vector<Entity> items;

            gm.view<Tag>().each(
                [&items](Entity e, Tag &) { items.push_back(e); });
            for (const Entity e : items)
                gm.destroyEntity(e);
        }

        bool keyPressed(GameManager &gm, const Key::Value key)
        {
            bool pressed = false;

            gm.view<KeyboardManager>().each([&](Entity, KeyboardManager &k) {
                for (const auto value : k.keyPressed)
                    if (value == key)
                        pressed = true;
            });
            return pressed;
        }

        template<typename Tag>
        void setDisabled(GameManager &gm, const bool disabled)
        {
            gm.view<Tag, Button>().each(
                [disabled](Entity, Tag &, Button &button) {
                    button.disabled = disabled;
                });
        }

        float popupX()
        {
            return (static_cast<float>(WINDOW_WIDTH) - LOGIN_POPUP_WIDTH) / 2.f;
        }

        float popupY()
        {
            return (static_cast<float>(WINDOW_HEIGTH) - LOGIN_POPUP_HEIGHT) /
                2.f;
        }

        void setMessage(
            LoginClient &l, const std::string &message, const bool error = true)
        {
            l.message = message;
            l.messageError = error;
        }

        std::string statusMessage(const AccountStatus status)
        {
            switch (status) {
                case AccountStatus::WrongCredentials:
                    return "Wrong username or password";
                case AccountStatus::UsernameTaken:
                    return "This username is already taken";
                case AccountStatus::InvalidUsername:
                    return "Invalid username";
                case AccountStatus::InvalidPassword:
                    return "Invalid password";
                default:
                    return "";
            }
        }

        Entity addPopupButton(GameManager &gm, float x, float y,
            const std::string &label,
            std::function<void(GameManager &)> onClick)
        {
            const Entity e = addButton(gm, x, y, label, std::move(onClick));

            gm.addComponent<LoginPopupItem>(e);
            return e;
        }

        Entity addPopupText(GameManager &gm, float y, const std::string &str,
            float size, raylib::Color color = COLOR_INFO)
        {
            const Entity e = addText(gm,
                static_cast<float>(WINDOW_WIDTH) / 2.f,
                y,
                str,
                size,
                {},
                color);

            gm.addComponent<LoginPopupItem>(e);
            return e;
        }

        Entity addPopupField(GameManager &gm, float y, const Field field,
            const std::string &value)
        {
            const Entity e = addPopupButton(gm,
                (static_cast<float>(WINDOW_WIDTH) - BUTTON_WIDTH) / 2.f,
                y,
                value,
                [field](GameManager &) { activeField = field; });

            gm.addComponent<TextField>(e, TextField {field});
            gm.addComponent<Clock>(e);
            return e;
        }

        void selectMode(GameManager &gm, const AccountMode mode)
        {
            LoginClient *l = findMaster(gm).login;

            if (l == nullptr || l->pending)
                return;
            l->mode = mode;
            setMessage(*l, "");
        }

        void confirmLogin(GameManager &gm)
        {
            LoginClient *l = findMaster(gm).login;

            if (l == nullptr || !l->popupOpen || l->pending)
                return;
            const std::string &username = _clInfos._username;
            const std::string &password = _clInfos._password;

            if (!AccountProtocol::isValidUsername(username)) {
                setMessage(*l, "Username must be 3 to 16 letters, digits or _");
                return;
            }
            if (l->mode == AccountMode::Create &&
                !AccountProtocol::isValidPassword(password)) {
                setMessage(*l, "Password must be 4 to 32 characters, no space");
                return;
            }
            if (password.empty()) {
                setMessage(*l, "Enter your password");
                return;
            }
            l->pending = true;
            l->silent = false;
            l->pendingMode = l->mode;
            l->username = username;
            l->password = password;
            l->lastSent = {};
            activeField = Field::None;
            setMessage(*l, "Please wait...", false);
        }

        bool handleResult(LoginClient &l, const AccountResult &result)
        {
            bool closePopup = false;

            l.pending = false;
            if (result.status == AccountStatus::Ok) {
                _account = {true, l.username, l.password};
                _clInfos._username = l.username;
                l.sessionOk = true;
                closePopup = !l.silent;
                saveCredentials();
            } else if (l.silent) {
                _account = {};
                deleteCredentials();
            } else
                setMessage(l, statusMessage(result.status));
            l.silent = false;
            return closePopup;
        }

        void updateButtons(GameManager &gm, const LoginClient *l)
        {
            const bool session = l != nullptr && l->sessionOk;

            setDisabled<StartGameButton>(gm, !session);
            setDisabled<LoginButton>(gm, _account.loggedIn);
            setDisabled<DisconnectButton>(gm, !_account.loggedIn);
            if (l == nullptr || !l->popupOpen)
                return;
            gm.view<LoginModeButton, Button>().each(
                [l](Entity, LoginModeButton &mode, Button &button) {
                    button.selected = mode.mode == l->mode;
                    button.disabled = l->pending;
                });
            setDisabled<LoginConfirmButton>(gm, l->pending);
            gm.view<LoginStatusText, Text2D>().each(
                [l](Entity, LoginStatusText &, Text2D &text) {
                    text.str = l->message;
                    text.color = l->messageError ? COLOR_ERROR : COLOR_INFO;
                });
        }

    } // namespace

    void openLoginPopup(GameManager &gm)
    {
        LoginClient *l = findMaster(gm).login;

        if (l == nullptr || l->popupOpen || _account.loggedIn)
            return;
        const float x = popupX();
        const float y = popupY();
        const float toggleGap = LOGIN_POPUP_WIDTH - 2.f * BUTTON_WIDTH -
            2.f * (LOGIN_POPUP_PADDING + 30.f);
        destroyTagged<MenuItem>(gm);
        const Entity create = addPopupButton(gm,
            x + LOGIN_POPUP_PADDING + 30.f,
            y + LOGIN_POPUP_PADDING,
            "Create account",
            [](GameManager &g) { selectMode(g, AccountMode::Create); });
        const Entity login = addPopupButton(gm,
            x + LOGIN_POPUP_PADDING + 30.f + BUTTON_WIDTH + toggleGap,
            y + LOGIN_POPUP_PADDING,
            "Login",
            [](GameManager &g) { selectMode(g, AccountMode::Login); });

        gm.addComponent<LoginModeButton>(
            create, LoginModeButton {AccountMode::Create});
        gm.addComponent<LoginModeButton>(
            login, LoginModeButton {AccountMode::Login});
        addPopupText(gm, y + 175.f, "Username", 28.f);
        addPopupField(gm, y + 195.f, Field::Username, _clInfos._username);
        addPopupText(gm, y + 320.f, "Password", 28.f);
        _clInfos._password.clear();
        addPopupField(gm, y + 340.f, Field::Password, "");
        const float buttonsY =
            y + LOGIN_POPUP_HEIGHT - LOGIN_POPUP_PADDING - BUTTON_HEIGHT;
        const Entity status = addPopupText(gm, y + 478.f, "", 28.f);
        const Entity confirm = addPopupButton(gm,
            x + LOGIN_POPUP_PADDING + 30.f,
            buttonsY,
            "Confirm",
            confirmLogin);

        gm.addComponent<LoginStatusText>(status);
        gm.addComponent<LoginConfirmButton>(confirm);
        addPopupButton(gm,
            x + LOGIN_POPUP_WIDTH - LOGIN_POPUP_PADDING - 30.f - BUTTON_WIDTH,
            buttonsY,
            "Back",
            closeLoginPopup);
        l->mode = AccountMode::Login;
        setMessage(*l, "");
        l->popupOpen = true;
        activeField = Field::Username;
    }

    void closeLoginPopup(GameManager &gm)
    {
        LoginClient *l = findMaster(gm).login;

        if (l == nullptr || !l->popupOpen)
            return;
        destroyTagged<LoginPopupItem>(gm);
        l->popupOpen = false;
        if (!l->silent)
            l->pending = false;
        if (activeField == Field::Username || activeField == Field::Password)
            activeField = Field::None;
        _clInfos._password.clear();
        createMenu(gm);
    }

    void logout(GameManager &gm)
    {
        MasterLogin master = findMaster(gm);

        if (master.client != nullptr && master.client->isConnected())
            AccountProtocol::sendRequest(
                *master.client, AccountMode::Logout, "", "");
        if (master.login != nullptr) {
            master.login->sessionOk = false;
            master.login->pending = false;
        }
        _account = {};
        deleteCredentials();
    }

    void loadSavedServer()
    {
        const auto saved = readCredentials();

        if (!saved)
            return;
        _clInfos._ip = saved->ip;
        _clInfos._username = saved->username;
    }

    void restoreSavedAccount()
    {
        const auto saved = readCredentials();

        _account = {};
        if (saved && saved->ip == _clInfos._ip)
            _account = {true, saved->username, saved->password};
    }

    void systemLogin(GameManager &gm)
    {
        bool showMenu = false;
        bool closePopup = false;
        LoginClient *popup = nullptr;

        gm.view<ClientManager, LoginClient, AccountResult>().each(
            [&](Entity,
                ClientManager &c,
                LoginClient &l,
                AccountResult &result) {
                const auto now = std::chrono::steady_clock::now();

                if (!c.isConnected())
                    return;
                if (!l.menuShown) {
                    l.menuShown = true;
                    showMenu = true;
                }
                if (_account.loggedIn && !l.sessionOk && !l.pending) {
                    l.pending = true;
                    l.silent = true;
                    l.pendingMode = AccountMode::Login;
                    l.username = _account.username;
                    l.password = _account.password;
                    l.lastSent = {};
                }
                if (l.pending && now - l.lastSent >= LOGIN_RESEND_RATE) {
                    AccountProtocol::sendRequest(
                        c, l.pendingMode, l.username, l.password);
                    l.lastSent = now;
                }
                if (result.received) {
                    result.received = false;
                    if (l.pending && result.mode == l.pendingMode)
                        closePopup = handleResult(l, result);
                }
                if (l.popupOpen)
                    popup = &l;
            });
        if (showMenu) {
            destroyTagged<ConnectingText>(gm);
            createMenu(gm);
        }
        if (popup != nullptr) {
            if (keyPressed(gm, Key::Value::Enter))
                confirmLogin(gm);
            if (keyPressed(gm, Key::Value::Tab))
                activeField = activeField == Field::Username ? Field::Password
                                                             : Field::Username;
            if (keyPressed(gm, Key::Value::Escape))
                closePopup = true;
        }
        if (closePopup)
            closeLoginPopup(gm);
        updateButtons(gm, findMaster(gm).login);
    }

    void systemDrawPasswordFields(GameManager &gm)
    {
        gm.view<TextField, Button>().each(
            [](Entity, TextField &f, Button &button) {
                if (f.id != Field::Password)
                    return;
                const bool blink =
                    activeField == f.id && (f.elapsed.count() / 400) % 2 == 0;
                const std::string str =
                    std::string(_clInfos._password.size(), '*') +
                    (blink ? "_" : "");
                const ::Font font = ::GetFontDefault();
                const float size = 40.f;
                const float spacing = 6.f;
                const Vector2 measure =
                    ::MeasureTextEx(font, str.c_str(), size, spacing);

                ::DrawTextEx(font,
                    str.c_str(),
                    {button.rect.x + (button.rect.width - measure.x) / 2.f,
                        button.rect.y + (button.rect.height - measure.y) / 2.f},
                    size,
                    spacing,
                    WHITE);
            });
    }

    void systemDrawLoginPopup(GameManager &gm)
    {
        const LoginClient *l = findMaster(gm).login;

        if (l == nullptr || !l->popupOpen)
            return;
        const Rectangle panel {
            popupX(), popupY(), LOGIN_POPUP_WIDTH, LOGIN_POPUP_HEIGHT};

        ::DrawRectangle(
            0, 0, WINDOW_WIDTH, WINDOW_HEIGTH, Color {0, 0, 0, 160});
        ::DrawRectangleRec(panel, Color {30, 36, 70, 255});
        ::DrawRectangleLinesEx(panel, 4.f, Color {200, 200, 220, 255});
        ::DrawLineEx({panel.x + LOGIN_POPUP_PADDING, panel.y + 150.f},
            {panel.x + panel.width - LOGIN_POPUP_PADDING, panel.y + 150.f},
            2.f,
            Color {200, 200, 220, 120});
    }

} // namespace RType::Client
