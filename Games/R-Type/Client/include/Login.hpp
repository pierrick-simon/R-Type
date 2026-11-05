/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Login
*/

#ifndef CLIENT_LOGIN_HPP_
#define CLIENT_LOGIN_HPP_

#include <chrono>
#include <string>

#include "AccountProtocol.hpp"
#include "GameManager.hpp"

namespace RType::Client {

    inline constexpr std::chrono::milliseconds LOGIN_RESEND_RATE {500};

    inline constexpr float LOGIN_POPUP_WIDTH = 1000.f;
    inline constexpr float LOGIN_POPUP_HEIGHT = 660.f;
    inline constexpr float LOGIN_POPUP_PADDING = 40.f;

    /** @brief Main menu buttons whose state depends on the login. */
    struct StartGameButton {};

    struct LoginButton {};

    struct DisconnectButton {};

    /** @brief Every entity of the login window. */
    struct LoginPopupItem {};

    /** @brief "Create account" / "Login" toggle buttons of the window. */
    struct LoginModeButton {
        AccountMode mode;
    };

    struct LoginConfirmButton {};

    struct LoginStatusText {};

    /**
     * @brief Account the player is logged in with.
     *
     * Kept outside of the master connection: when the player comes back from
     * a lobby, the client reconnects to the master and logs in again with it
     * without asking anything.
     */
    struct Account {
        bool loggedIn = false;
        std::string username;
        std::string password;
    };

    inline Account _account;

    /**
     * @brief Client file keeping the account between two launches:
     *        3 lines, server IP, username, password.
     *
     * Written when the player logs in, removed by "Disconnect". The account
     * is only used again on the same server.
     */
    inline constexpr const char *CREDENTIALS_PATH = "credentials.txt";

    /** @brief At launch: fills the IP and username with the saved ones. */
    void loadSavedServer();
    /** @brief When connecting to _clInfos._ip: logs in automatically with
     *         the saved account if it belongs to this server. */
    void restoreSavedAccount();

    /**
     * @brief Component of the master connection entity: state of the main
     *        menu and of the login window.
     */
    struct LoginClient {
        bool menuShown = false;
        /** @brief The master accepted our login on this connection. */
        bool sessionOk = false;
        bool popupOpen = false;
        AccountMode mode = AccountMode::Login;

        /** @brief A request is sent until its answer is received. */
        bool pending = false;
        /** @brief Automatic login after a reconnection (no window). */
        bool silent = false;
        AccountMode pendingMode = AccountMode::Login;
        std::string username;
        std::string password;
        std::chrono::steady_clock::time_point lastSent {};

        std::string message;
        bool messageError = false;
    };

    void openLoginPopup(GameManager &gm);
    void closeLoginPopup(GameManager &gm);
    void logout(GameManager &gm);

    /** @brief Shows the menu once connected, sends the requests, handles
     *         the answers and updates the buttons. */
    void systemLogin(GameManager &gm);
    /** @brief Draws the password fields as '*' (raylib default font: the
     *         font of the game has no '*'). Must run after the sprites. */
    void systemDrawPasswordFields(GameManager &gm);
    /** @brief Draws the panel of the login window (raylib). */
    void systemDrawLoginPopup(GameManager &gm);

} // namespace RType::Client

#endif /* !CLIENT_LOGIN_HPP_ */
