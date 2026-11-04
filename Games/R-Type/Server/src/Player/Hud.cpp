/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Hud
*/

#include "Player/Hud.hpp"

#include "Assets.hpp"
#include "Config.hpp"
#include "Player/Destroy.hpp"
#include "SpriteSheet2D.hpp"
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

} // namespace RType::Server
