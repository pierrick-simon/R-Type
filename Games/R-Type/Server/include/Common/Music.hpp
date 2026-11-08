/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Music
*/

#ifndef MUSIC_HPP_
#define MUSIC_HPP_

#include <optional>
#include "Assets.hpp"
#include "GameManager.hpp"

namespace RType::Server {

    struct Music {
        AssetId track = 0;
    };

    [[nodiscard]] std::optional<AssetId> levelMusic(GameManager &gm);

    void playMusic(GameManager &gm, std::optional<AssetId> track);

    void musicSystem(GameManager &gm);

} // namespace RType::Server

#endif
