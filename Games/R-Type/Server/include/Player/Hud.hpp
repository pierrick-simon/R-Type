/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Hud
*/

#ifndef HUD_HPP_
#define HUD_HPP_

#include "Components.hpp"
#include "GameManager.hpp"

namespace RType::Server {

    void spawnLifeIcons(GameManager &gm, Entity player, const Player &state);
    void livesHudSystem(GameManager &gm);
    void spawnTeamScore(GameManager &gm);
    void spawnPlayerScore(GameManager &gm, Entity player, const Player &state);
    void scoreHudSystem(GameManager &gm);
    void spawnNameTag(GameManager &gm, Entity player, const Player &state);
    void nameTagSystem(GameManager &gm);

} // namespace RType::Server

#endif
