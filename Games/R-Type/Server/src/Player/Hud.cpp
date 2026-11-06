/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Hud
*/

#include "Player/Hud.hpp"

#include <algorithm>
#include <string>
#include <tuple>
#include <vector>

#include "Assets.hpp"
#include "Config.hpp"
#include "Enemies/EnemyComponents.hpp"
#include "Enemies/EnemyManager.hpp"
#include "GameState.hpp"
#include "Player/Destroy.hpp"
#include "SpriteSheet2D.hpp"
#include "Text2D.hpp"
#include "UpdateClientEntity.hpp"

namespace RType::Server {

    Position2D lifeIconPosition(std::size_t slot, int index)
    {
        return {HUD_MARGIN + float(index) * (LIFE_ICON_WIDTH + HUD_SPACING),
            HUD_MARGIN + float(slot) * (LIFE_ICON_HEIGHT + HUD_SPACING)};
    }

    void spawnLifeIcons(GameManager &gm, Entity player, const Player &state)
    {
        const AssetId sheet =
            Asset::PLAYER_SHEET_1 + AssetId(state.slot % PLAYER_COLORS);

        for (int i = 0; i < PLAYER_LIVES; ++i) {
            auto icon = gm.createEntity();

            gm.addComponent<LifeIcon>(icon, LifeIcon {player, i});
            gm.addComponent<Position2D>(icon, lifeIconPosition(state.slot, i));
            gm.addComponent<SpriteSheet2D>(icon,
                Size2D(1, PLAYER_FRAMES),
                FRAME_NEUTRAL,
                PLAYER_FRAME_MS,
                LIFE_ICON_SCALE,
                false);
            gm.addComponent<UpdateClientEntity>(
                icon, EntityType::SPRITESHEET, sheet, gm, icon);
        }
    }

    void livesHudSystem(GameManager &gm)
    {
        for (auto [icon, life, sheet] :
            gm.view<LifeIcon, SpriteSheet2D>().getVector()) {
            if (!gm.hasComponent<Player>(life.owner)) {
                destroyNetworked(gm, icon);
                continue;
            }
            const auto &player = gm.getComponent<Player>(life.owner);

            sheet.color.a = life.index < player.lives ? 255 : 0;
        }
    }

    std::string formatScore(const std::string &label, int score)
    {
        std::string digits = std::to_string(std::max(score, 0));

        if (digits.size() < SCORE_DIGITS)
            digits.insert(0, SCORE_DIGITS - digits.size(), '0');
        return label + " " + digits;
    }

    std::string playerLabel(const Player &state)
    {
        return std::to_string(state.slot + 1) + "P";
    }

    void spawnScoreLabel(GameManager &gm, std::optional<Entity> player,
        const Position2D &pos, const raylib::Color &color,
        const std::string &str)
    {
        const Entity label = gm.createEntity();
        Text2D text;

        text.fontSize = SCORE_FONT_SIZE;
        text.str = str;
        text.color = color;
        gm.addComponent<ScoreLabel>(label, ScoreLabel {player});
        gm.addComponent<Position2D>(label, pos);
        gm.addComponent<Text2D>(label, text);
        gm.addComponent<UpdateClientEntity>(
            label, EntityType::TEXT_ENTITY, Asset::FONT, gm, label);
    }

    void spawnTeamScore(GameManager &gm)
    {
        spawnScoreLabel(gm,
            std::nullopt,
            {HUD_MARGIN, SCORE_Y},
            raylib::Color::White(),
            formatScore("SCORE", 0));
    }

    void spawnPlayerScore(GameManager &gm, Entity player, const Player &state)
    {
        const auto &rgb = PLAYER_SCORE_COLORS[state.slot % PLAYER_COLORS];

        spawnScoreLabel(gm,
            player,
            {SCORE_PLAYERS_X + float(state.slot) * SCORE_SPACING, SCORE_Y},
            raylib::Color(rgb[0], rgb[1], rgb[2], 255),
            formatScore(playerLabel(state), 0));
    }

    float centeredX(const Text2D &text)
    {
        return (WINDOW_WIDTH - float(text.str.size()) * text.fontSize) / 2.f;
    }

    /** @brief On the game over screen, the team score is big and centered,
     * the players' are listed under it. Along the bottom otherwise. */
    void layoutTeamScore(Text2D &text, Position2D &pos)
    {
        if (!isGameOverScreen()) {
            text.fontSize = SCORE_FONT_SIZE;
            text.color = raylib::Color::White();
            pos = {HUD_MARGIN, SCORE_Y};
            return;
        }
        text.fontSize = GAME_OVER_SCORE_SIZE;
        text.color = raylib::Color(255, 215, 0, 255);
        pos = {centeredX(text), GAME_OVER_SCORE_Y};
    }

    void layoutPlayerScore(
        Text2D &text, Position2D &pos, std::size_t slot, std::size_t rank)
    {
        if (!isGameOverScreen()) {
            text.fontSize = SCORE_FONT_SIZE;
            pos = {SCORE_PLAYERS_X + float(slot) * SCORE_SPACING, SCORE_Y};
            return;
        }
        text.fontSize = GAME_OVER_PLAYER_SIZE;
        pos = {centeredX(text),
            GAME_OVER_PLAYERS_Y + float(rank) * GAME_OVER_PLAYER_SPACING};
    }

    void scoreHudSystem(GameManager &gm)
    {
        int team = 0;
        std::vector<std::tuple<std::size_t, Text2D *, Position2D *>> players;

        for (auto [_, director] : gm.view<EnemyDirector>().getVector())
            team = director.score;
        for (auto [label, score, text, pos] :
            gm.view<ScoreLabel, Text2D, Position2D>().getVector()) {
            if (!score.player) {
                text.str = formatScore("SCORE", team);
                layoutTeamScore(text, pos);
                continue;
            }
            if (!gm.hasComponent<Player>(*score.player)) {
                destroyNetworked(gm, label);
                continue;
            }
            const auto &player = gm.getComponent<Player>(*score.player);
            const int points = gm.hasComponent<Scorer>(*score.player)
                ? gm.getComponent<Scorer>(*score.player).score
                : 0;

            text.str = formatScore(playerLabel(player), points);
            players.emplace_back(player.slot, &text, &pos);
        }
        std::ranges::sort(
            players, {}, [](const auto &p) { return std::get<0>(p); });
        for (std::size_t rank = 0; rank < players.size(); ++rank) {
            auto [slot, text, pos] = players[rank];

            layoutPlayerScore(*text, *pos, slot, rank);
        }
    }

} // namespace RType::Server
