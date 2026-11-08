/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** UserDB
*/

#include "UserDB.hpp"

#include <array>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>

#include "AccountProtocol.hpp"
#include "ServerManager.hpp"

namespace RType::Server {

    namespace {

        constexpr std::array<uint32_t, 64> SHA256_K = {0x428a2f98,
            0x71374491,
            0xb5c0fbcf,
            0xe9b5dba5,
            0x3956c25b,
            0x59f111f1,
            0x923f82a4,
            0xab1c5ed5,
            0xd807aa98,
            0x12835b01,
            0x243185be,
            0x550c7dc3,
            0x72be5d74,
            0x80deb1fe,
            0x9bdc06a7,
            0xc19bf174,
            0xe49b69c1,
            0xefbe4786,
            0x0fc19dc6,
            0x240ca1cc,
            0x2de92c6f,
            0x4a7484aa,
            0x5cb0a9dc,
            0x76f988da,
            0x983e5152,
            0xa831c66d,
            0xb00327c8,
            0xbf597fc7,
            0xc6e00bf3,
            0xd5a79147,
            0x06ca6351,
            0x14292967,
            0x27b70a85,
            0x2e1b2138,
            0x4d2c6dfc,
            0x53380d13,
            0x650a7354,
            0x766a0abb,
            0x81c2c92e,
            0x92722c85,
            0xa2bfe8a1,
            0xa81a664b,
            0xc24b8b70,
            0xc76c51a3,
            0xd192e819,
            0xd6990624,
            0xf40e3585,
            0x106aa070,
            0x19a4c116,
            0x1e376c08,
            0x2748774c,
            0x34b0bcb5,
            0x391c0cb3,
            0x4ed8aa4a,
            0x5b9cca4f,
            0x682e6ff3,
            0x748f82ee,
            0x78a5636f,
            0x84c87814,
            0x8cc70208,
            0x90befffa,
            0xa4506ceb,
            0xbef9a3f7,
            0xc67178f2};

        uint32_t rotr(uint32_t x, int n)
        {
            return (x >> n) | (x << (32 - n));
        }

        void sha256Block(std::array<uint32_t, 8> &h, const uint8_t *block)
        {
            std::array<uint32_t, 64> w {};

            for (int i = 0; i < 16; i++)
                w[i] = uint32_t(block[i * 4]) << 24 |
                    uint32_t(block[i * 4 + 1]) << 16 |
                    uint32_t(block[i * 4 + 2]) << 8 |
                    uint32_t(block[i * 4 + 3]);
            for (int i = 16; i < 64; i++) {
                const uint32_t s0 =
                    rotr(w[i - 15], 7) ^ rotr(w[i - 15], 18) ^ (w[i - 15] >> 3);
                const uint32_t s1 =
                    rotr(w[i - 2], 17) ^ rotr(w[i - 2], 19) ^ (w[i - 2] >> 10);
                w[i] = w[i - 16] + s0 + w[i - 7] + s1;
            }
            uint32_t a = h[0], b = h[1], c = h[2], d = h[3];
            uint32_t e = h[4], f = h[5], g = h[6], k = h[7];

            for (int i = 0; i < 64; i++) {
                const uint32_t s1 = rotr(e, 6) ^ rotr(e, 11) ^ rotr(e, 25);
                const uint32_t ch = (e & f) ^ (~e & g);
                const uint32_t t1 = k + s1 + ch + SHA256_K[i] + w[i];
                const uint32_t s0 = rotr(a, 2) ^ rotr(a, 13) ^ rotr(a, 22);
                const uint32_t maj = (a & b) ^ (a & c) ^ (b & c);
                const uint32_t t2 = s0 + maj;

                k = g;
                g = f;
                f = e;
                e = d + t1;
                d = c;
                c = b;
                b = a;
                a = t1 + t2;
            }
            h[0] += a;
            h[1] += b;
            h[2] += c;
            h[3] += d;
            h[4] += e;
            h[5] += f;
            h[6] += g;
            h[7] += k;
        }

    } // namespace

    std::string sha256Hex(const std::string &data)
    {
        std::array<uint32_t, 8> h = {0x6a09e667,
            0xbb67ae85,
            0x3c6ef372,
            0xa54ff53a,
            0x510e527f,
            0x9b05688c,
            0x1f83d9ab,
            0x5be0cd19};
        std::string message = data;
        const uint64_t bits = uint64_t(data.size()) * 8;
        static const char *hex = "0123456789abcdef";
        std::string out;

        message += static_cast<char>(0x80);
        while (message.size() % 64 != 56)
            message += '\0';
        for (int i = 7; i >= 0; i--)
            message += static_cast<char>((bits >> (i * 8)) & 0xff);
        for (std::size_t i = 0; i < message.size(); i += 64)
            sha256Block(h, reinterpret_cast<const uint8_t *>(&message[i]));
        for (const uint32_t word : h)
            for (int i = 28; i >= 0; i -= 4)
                out += hex[(word >> i) & 0xf];
        return out;
    }

    UserDB::UserDB(std::string path) :
        _path(std::move(path))
    {
        load();
    }

    std::string UserDB::hashPassword(
        const std::string &username, const std::string &password)
    {
        return sha256Hex(username + ":" + password);
    }

    void UserDB::load()
    {
        std::ifstream file(_path);
        std::string username;
        std::string hash;
        std::string score;

        while (std::getline(file, username) && std::getline(file, hash) &&
            std::getline(file, score)) {
            User user {hash, 0};

            try {
                user.highestScore = std::stol(score);
            } catch (const std::exception &) {
            }
            if (!username.empty())
                _users[username] = user;
        }
    }

    bool UserDB::exists(const std::string &username) const
    {
        return _users.contains(username);
    }

    bool UserDB::check(
        const std::string &username, const std::string &password) const
    {
        const auto it = _users.find(username);

        return it != _users.end() &&
            it->second.passwordHash == hashPassword(username, password);
    }

    bool UserDB::add(const std::string &username, const std::string &password)
    {
        if (exists(username))
            return false;
        const User user {hashPassword(username, password), 0};
        std::ofstream file(_path, std::ios::app);

        if (!file) {
            std::cerr << "[Master] Cannot write " << _path << std::endl;
            return false;
        }
        file << username << '\n'
             << user.passwordHash << '\n'
             << user.highestScore << '\n';
        file.flush();
        _users[username] = user;
        return true;
    }

    bool UserDB::recordScore(const std::string &username, long score)
    {
        const auto it = _users.find(username);

        if (it == _users.end() || score <= it->second.highestScore)
            return false;
        it->second.highestScore = score;
        save();
        return true;
    }

    void UserDB::save() const
    {
        const std::string tmp = _path + ".tmp";
        {
            std::ofstream file(tmp, std::ios::trunc);

            if (!file) {
                std::cerr << "[Master] Cannot write " << tmp << std::endl;
                return;
            }
            for (const auto &[name, user] : _users)
                file << name << '\n'
                     << user.passwordHash << '\n'
                     << user.highestScore << '\n';
        }
        std::error_code ec;

        std::filesystem::rename(tmp, _path, ec);
        if (ec)
            std::cerr << "[Master] Cannot replace " << _path << ": "
                      << ec.message() << std::endl;
    }

    namespace {

        AccountStatus answer(UserDB &db, const AccountRequest &request)
        {
            if (!AccountProtocol::isValidUsername(request.username))
                return request.mode == AccountMode::Create
                    ? AccountStatus::InvalidUsername
                    : AccountStatus::WrongCredentials;
            if (request.mode == AccountMode::Login)
                return db.check(request.username, request.password)
                    ? AccountStatus::Ok
                    : AccountStatus::WrongCredentials;
            if (!AccountProtocol::isValidPassword(request.password))
                return AccountStatus::InvalidPassword;
            if (db.exists(request.username))
                return AccountStatus::UsernameTaken;
            return db.add(request.username, request.password)
                ? AccountStatus::Ok
                : AccountStatus::InvalidUsername;
        }

        void handleRequest(ServerManager &server, UserDB &db,
            const uint32_t clientId, AccountRequest &request)
        {
            request.pending = false;
            if (request.mode == AccountMode::Logout) {
                request.loggedUser.clear();
                request.answered = false;
                return;
            }
            const bool resent = request.answered &&
                request.lastMode == request.mode &&
                request.lastUsername == request.username &&
                request.lastPassword == request.password;

            if (!resent) {
                request.lastStatus = answer(db, request);
                request.lastMode = request.mode;
                request.lastUsername = request.username;
                request.lastPassword = request.password;
                request.answered = true;
                if (request.lastStatus == AccountStatus::Ok)
                    std::cout << "[Master] " << request.username
                              << (request.mode == AccountMode::Create
                                         ? " created an account"
                                         : " logged in")
                              << std::endl;
            }
            if (request.lastStatus == AccountStatus::Ok)
                request.loggedUser = request.lastUsername;
            try {
                AccountProtocol::sendResult(
                    server, clientId, request.lastMode, request.lastStatus);
            } catch (const std::exception &e) {
                std::cerr << "[Master] " << e.what() << std::endl;
            }
        }

    } // namespace

    void systemMasterAccounts(GameManager &gm)
    {
        gm.view<ServerManager, UserDB>().each(
            [&gm](Entity, ServerManager &server, UserDB &db) {
                gm.view<DefaulClient, AccountRequest>().each(
                    [&](Entity, DefaulClient &client, AccountRequest &request) {
                        if (request.pending)
                            handleRequest(server, db, client.getId(), request);
                    });
            });
    }

} // namespace RType::Server
