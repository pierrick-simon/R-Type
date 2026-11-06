/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** tests_score
*/

#include <criterion/criterion.h>

#include "Components.hpp"
#include "Enemies/EnemyManager.hpp"
#include "Enemies/EnemySystems.hpp"
#include "GameState.hpp"
#include "Player/Hud.hpp"
#include "SpriteSheet2D.hpp"
#include "Text2D.hpp"
#include "Weapons/Force.hpp"

using namespace RType;
using namespace RType::Server;

namespace {

    Entity addPlayer(GameManager &gm, std::size_t slot = 0)
    {
        const Entity e = gm.createEntity();

        gm.addComponent<Player>(e, Player {.slot = slot});
        gm.addComponent<Health>(e, Health {PLAYER_HP, PLAYER_HP});
        gm.addComponent<Hitbox>(e, Hitbox {PLAYER_WIDTH, PLAYER_HEIGHT});
        gm.addComponent<Position2D>(e, 100.f, 100.f);
        gm.addComponent<Velocity2D>(e);
        gm.addComponent<SpriteSheet2D>(e);
        gm.addComponent<Scorer>(e);
        return e;
    }

    void shoot(GameManager &gm, Entity owner, float x, float y)
    {
        const Entity shot = gm.createEntity();

        gm.addComponent<Projectile>(shot, Projectile {5, true, false, owner});
        gm.addComponent<Position2D>(shot, x, y);
        gm.addComponent<Hitbox>(shot, Hitbox {10.f, 10.f});
    }

    std::string labelOf(GameManager &gm, std::optional<Entity> player)
    {
        for (auto [_, label, text] : gm.view<ScoreLabel, Text2D>().getVector())
            if (label.player == player)
                return text.str;
        return "";
    }

} // namespace

Test(score, kills_credit_the_shooter_and_the_team)
{
    GameManager gm;
    EnemyDirector director;
    const Entity a = addPlayer(gm);
    const Entity b = addPlayer(gm, 1);

    const Entity scout = spawnEnemy(gm, EnemyType::Scout, 1000.f, 600.f);
    const auto &pos = gm.getComponent<Position2D>(scout);
    const auto &box = gm.getComponent<Hitbox>(scout);

    shoot(gm,
        b,
        pos.x + box.offsetX + box.width / 2.f,
        pos.y + box.offsetY + box.height / 2.f);
    systemEnemyCombat(gm, director, {});
    cr_assert_gt(director.score, 0);
    cr_assert_eq(gm.getComponent<Scorer>(b).score, director.score);
    cr_assert_eq(gm.getComponent<Scorer>(a).score, 0);
}

Test(score, force_kills_credit_its_owner)
{
    GameManager gm;
    EnemyDirector director;
    const Entity player = addPlayer(gm);

    giveForce(gm, player, LaserType::Reflect);
    const auto &pos = gm.getComponent<Position2D>(*findForce(gm, player));

    spawnEnemy(gm, EnemyType::Scout, pos.x + 40.f, pos.y + 30.f);
    systemEnemyCombat(gm, director, {});
    cr_assert_gt(gm.getComponent<Scorer>(player).score, 0);
    cr_assert_eq(gm.getComponent<Scorer>(player).score, director.score);
}

Test(score, hud_shows_the_team_and_each_player)
{
    GameManager gm;
    const Entity player = addPlayer(gm, 1);
    const Entity director = gm.createEntity();

    gm.addComponent<EnemyDirector>(director);
    gm.getComponent<EnemyDirector>(director).score = 1500;
    gm.getComponent<Scorer>(player).score = 400;
    spawnTeamScore(gm);
    spawnPlayerScore(gm, player, gm.getComponent<Player>(player));
    scoreHudSystem(gm);
    cr_assert_str_eq(labelOf(gm, std::nullopt).c_str(), "SCORE 001500");
    cr_assert_str_eq(labelOf(gm, player).c_str(), "2P 000400");
}

Test(score, restart_resets_the_scores)
{
    GameManager gm;
    const Entity player = addPlayer(gm);

    gm.getComponent<Scorer>(player).score = 900;
    restartGame(gm);
    cr_assert_eq(gm.getComponent<Scorer>(player).score, 0);
}
