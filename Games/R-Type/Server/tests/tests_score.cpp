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
#include "LobbyProtocol.hpp"
#include "Player/Hud.hpp"
#include "SpriteSheet2D.hpp"
#include "Text2D.hpp"
#include "UpdateText2D.hpp"
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

Test(score, team_score_is_big_on_the_game_over_screen)
{
    GameManager gm;
    const Entity director = gm.createEntity();

    gm.addComponent<EnemyDirector>(director);
    spawnTeamScore(gm);
    showGameOver(gm);
    scoreHudSystem(gm);
    for (auto [_, label, text, pos] :
        gm.view<ScoreLabel, Text2D, Position2D>().getVector()) {
        cr_assert_float_eq(text.fontSize, GAME_OVER_SCORE_SIZE, 0.01f);
        cr_assert_float_eq(
            pos.x + float(text.str.size()) * GAME_OVER_SCORE_SIZE / 2.f,
            WINDOW_WIDTH / 2.f,
            0.01f);
    }
    restartGame(gm);
    scoreHudSystem(gm);
    for (auto [_, label, text, pos] :
        gm.view<ScoreLabel, Text2D, Position2D>().getVector()) {
        cr_assert_float_eq(text.fontSize, SCORE_FONT_SIZE, 0.01f);
        cr_assert_float_eq(pos.y, SCORE_Y, 0.01f);
    }
}

Test(score, game_over_lists_every_player_under_the_total)
{
    GameManager gm;
    const Entity director = gm.createEntity();
    const Entity third = addPlayer(gm, 2);
    const Entity first = addPlayer(gm, 0);

    gm.addComponent<EnemyDirector>(director);
    spawnTeamScore(gm);
    spawnPlayerScore(gm, third, gm.getComponent<Player>(third));
    spawnPlayerScore(gm, first, gm.getComponent<Player>(first));
    showGameOver(gm);
    scoreHudSystem(gm);
    for (auto [_, label, text, pos] :
        gm.view<ScoreLabel, Text2D, Position2D>().getVector()) {
        cr_assert_float_eq(pos.x + float(text.str.size()) * text.fontSize / 2.f,
            WINDOW_WIDTH / 2.f,
            0.01f);
        if (!label.player)
            cr_assert_float_eq(pos.y, GAME_OVER_SCORE_Y, 0.01f);
        else
            cr_assert_float_eq(pos.y,
                GAME_OVER_PLAYERS_Y +
                    (*label.player == first ? 0.f : GAME_OVER_PLAYER_SPACING),
                0.01f);
    }
}

Test(score, lobby_state_carries_the_replay_votes)
{
    LobbyState state;

    state.started = true;
    state.gameOver = true;
    state.players = {{1, true, true, "a"}, {2, true, false, "b"}};
    const auto parsed =
        LobbyProtocol::parseState(LobbyProtocol::serializeState(state));

    cr_assert(parsed.has_value());
    cr_assert(parsed->gameOver);
    cr_assert_eq(parsed->replayVotes(), 1);
    cr_assert(parsed->players[0].ready);
    cr_assert(parsed->players[1].ready);
}

Test(score, a_score_change_waits_for_the_next_send)
{
    UpdateText2D update;
    Text2D text;

    UpdateText2D::systemUpdateText2D(update, text);
    update.setUpdateText(false);
    text.str = "SCORE 000000";
    UpdateText2D::systemUpdateText2D(update, text);
    UpdateText2D::systemUpdateText2D(update, text);
    cr_assert(update.getUpdateText());
}
