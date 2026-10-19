/*
** EPITECH PROJECT, 2026
** R-Type
** File description:
** Enemy preview window
*/

#include <array>
#include <cstddef>

#include <raylib.h>

#include "EnemyDefs.hpp"
#include "Texture2D.hpp"

namespace {

    constexpr int WINDOW_WIDTH = 1280;
    constexpr int WINDOW_HEIGHT = 720;
    constexpr int COLUMNS = 4;
    constexpr float DISPLAY_SCALE = 4.f;

    struct PreviewEnemy {
        RType::Texture2D texture;
        const RType::Game::EnemyDef &definition;
        float animationTime = 0.f;
    };

    [[nodiscard]] Rectangle spriteFrame(const PreviewEnemy &enemy)
    {
        const auto &sheet = *enemy.definition.sheet;
        int frame = int(enemy.animationTime * 1000.f / float(sheet.frameMs)) %
            sheet.frames;

        return {float(frame * sheet.cellW),
            0.f,
            float(sheet.cellW),
            float(sheet.cellH)};
    }

} // namespace

int main()
{
    using namespace RType;
    using namespace RType::Game;

    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "R-Type - Enemy preview");
    SetTargetFPS(60);

    std::array<PreviewEnemy, std::size(ENEMY_DEFS)> enemies = [&] {
        std::array<PreviewEnemy, std::size(ENEMY_DEFS)> result = {{
            {RType::Texture2D(ENEMY_DEFS[0].sheet->file), ENEMY_DEFS[0]},
            {RType::Texture2D(ENEMY_DEFS[1].sheet->file), ENEMY_DEFS[1]},
            {RType::Texture2D(ENEMY_DEFS[2].sheet->file), ENEMY_DEFS[2]},
            {RType::Texture2D(ENEMY_DEFS[3].sheet->file), ENEMY_DEFS[3]},
            {RType::Texture2D(ENEMY_DEFS[4].sheet->file), ENEMY_DEFS[4]},
            {RType::Texture2D(ENEMY_DEFS[5].sheet->file), ENEMY_DEFS[5]},
            {RType::Texture2D(ENEMY_DEFS[6].sheet->file), ENEMY_DEFS[6]},
            {RType::Texture2D(ENEMY_DEFS[7].sheet->file), ENEMY_DEFS[7]},
            {RType::Texture2D(ENEMY_DEFS[8].sheet->file), ENEMY_DEFS[8]},
            {RType::Texture2D(ENEMY_DEFS[9].sheet->file), ENEMY_DEFS[9]},
            {RType::Texture2D(ENEMY_DEFS[10].sheet->file), ENEMY_DEFS[10]},
        }};
        return result;
    }();

    while (!WindowShouldClose()) {
        float elapsed = GetFrameTime();

        BeginDrawing();
        ClearBackground(Color {16, 20, 33, 255});
        DrawText("Enemy preview - close with Escape", 24, 20, 26, RAYWHITE);

        for (std::size_t index = 0; index < enemies.size(); ++index) {
            auto &enemy = enemies[index];
            const auto &sheet = *enemy.definition.sheet;
            int column = int(index % COLUMNS);
            int row = int(index / COLUMNS);
            float centerX = 160.f + float(column) * 320.f;
            float centerY = 135.f + float(row) * 180.f;
            Rectangle source = spriteFrame(enemy);
            Rectangle destination = {centerX,
                centerY,
                float(sheet.cellW) * DISPLAY_SCALE,
                float(sheet.cellH) * DISPLAY_SCALE};

            DrawText(enemy.definition.name,
                int(centerX - 100.f),
                int(centerY - 75.f),
                20,
                LIGHTGRAY);
            DrawTexturePro(enemy.texture,
                source,
                destination,
                {destination.width / 2.f, destination.height / 2.f},
                0.f,
                WHITE);
            enemy.animationTime += elapsed;
        }
        EndDrawing();
    }
    CloseWindow();
    return 0;
}
