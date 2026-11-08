/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** AccountProtocol
*/

#ifndef ACCOUNTPROTOCOL_HPP_
#define ACCOUNTPROTOCOL_HPP_

#include <cstdint>
#include <string>
#include <vector>
#include "ClientManager.hpp"
#include "Context.hpp"
#include "Network.hpp"

namespace RType {

    class ServerManager;

    inline constexpr std::size_t ACCOUNT_USERNAME_MIN = 3;
    inline constexpr std::size_t ACCOUNT_USERNAME_MAX = 16;
    inline constexpr std::size_t ACCOUNT_PASSWORD_MIN = 4;
    inline constexpr std::size_t ACCOUNT_PASSWORD_MAX = 32;

    enum class AccountMode : uint8_t { Login = 0, Create = 1, Logout = 2 };

    enum class AccountStatus : uint8_t {
        Ok = 0,
        WrongCredentials = 1,
        UsernameTaken = 2,
        InvalidUsername = 3,
        InvalidPassword = 4
    };

    struct AccountRequest {
        bool pending = false;
        AccountMode mode = AccountMode::Login;
        std::string username;
        std::string password;

        std::string loggedUser;

        bool answered = false;
        AccountMode lastMode = AccountMode::Login;
        std::string lastUsername;
        std::string lastPassword;
        AccountStatus lastStatus = AccountStatus::Ok;
    };

    struct AccountResult {
        bool received = false;
        AccountMode mode = AccountMode::Login;
        AccountStatus status = AccountStatus::Ok;
    };

    class AccountProtocol {
    public:
        static bool isValidUsername(const std::string &username);
        static bool isValidPassword(const std::string &password);

        static void sendRequest(ClientManager &c, AccountMode mode,
            const std::string &username, const std::string &password);
        static void manageRequest(const Header &header,
            const std::vector<uint8_t> &payload, Context &ctx);
        static void sendResult(ServerManager &s, uint32_t clientId,
            AccountMode mode, AccountStatus status);
        static void manageResult(const Header &header,
            const std::vector<uint8_t> &payload, Context &ctx);
    };

} // namespace RType

#endif /* !ACCOUNTPROTOCOL_HPP_ */
