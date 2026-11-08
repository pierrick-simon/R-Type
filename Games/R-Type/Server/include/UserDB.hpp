/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** UserDB
*/

#ifndef USERDB_HPP_
#define USERDB_HPP_

#include <string>
#include <unordered_map>

#include <optional>
#include <vector>
#include "AccountProtocol.hpp"
#include "GameManager.hpp"

namespace RType::Server {

    inline constexpr const char *USER_DB_PATH = "userDB.txt";

    std::string sha256Hex(const std::string &data);

    class UserDB {
    public:
        struct User {
            std::string passwordHash;
            long highestScore = 0;
        };

        explicit UserDB(std::string path = USER_DB_PATH);

        [[nodiscard]] bool exists(const std::string &username) const;

        [[nodiscard]] bool check(
            const std::string &username, const std::string &password) const;

        bool add(const std::string &username, const std::string &password);

        /**
         * @brief Keep the best score of an account.
         *
         * Rewrites the file when @p score beats the saved one.
         *
         * @return true if the best score changed.
         */
        bool recordScore(const std::string &username, long score);

        /**
         * @brief The @p count best accounts, best first.
         *
         * Sorted by score, highest first; equal scores by username. Equal
         * scores share the same rank (1, 2, 2, 4...).
         */
        [[nodiscard]] std::vector<LeaderboardEntry> top(
            std::size_t count) const;

        /** @brief Rank and score of one account, if it exists. */
        [[nodiscard]] std::optional<LeaderboardEntry> entryOf(
            const std::string &username) const;

    private:
        static std::string hashPassword(
            const std::string &username, const std::string &password);
        void load();
        void save() const;

        std::string _path;
        std::unordered_map<std::string, User> _users;
    };

    void systemMasterAccounts(GameManager &gm);

} // namespace RType::Server

#endif /* !USERDB_HPP_ */
