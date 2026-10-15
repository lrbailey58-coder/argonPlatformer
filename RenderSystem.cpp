// File: RenderSystem.cpp
#include "RenderSystem.h"
#include <algorithm>

void RenderSystem::Render(const WorldState& world) {
    BeginDrawing();
    ClearBackground(SKYBLUE);

    // 1. Render Scrolling 2D World
    BeginMode2D(world.camera.cam);
        DrawLevel(world.level);
        DrawPlayer(world.player);
        DrawBallAndChargeBar(world.ball, world.player);
    EndMode2D();

    // 2. Render Fixed Screen UI
    DrawHUD(world);

    EndDrawing();
}

void RenderSystem::DrawLevel(const LevelData& level) {
    // Subtle background grid
    for (int x = 0; x <= (int)level.mapWidth; x += 100) {
        DrawLine(x, 0, x, (int)level.mapHeight, Fade(WHITE, 0.25f));
    }
    for (int y = 0; y <= (int)level.mapHeight; y += 100) {
        DrawLine(0, y, (int)level.mapWidth, y, Fade(WHITE, 0.25f));
    }

    // Platforms
    for (const auto& plat : level.platforms) {
        DrawRectangleRec(plat, DARKGRAY);
        DrawRectangleLinesEx(plat, 2, BLACK);
    }

    // Victory Flag
    DrawRectangleRec(level.flagPole, GOLD);
    DrawTriangle(
        { level.flagPole.x + 10, level.flagPole.y },
        { level.flagPole.x + 10, level.flagPole.y + 35 },
        { level.flagPole.x + 55, level.flagPole.y + 17 },
        GREEN
    );
}

void RenderSystem::DrawPlayer(const PlayerData& player) {
    DrawRectangleRec(player.rect, RED);
}

void RenderSystem::DrawBallAndChargeBar(const BallData& ball, const PlayerData& player) {
    DrawCircleV(ball.pos, ball.radius, LIME);
    DrawCircleLines((int)ball.pos.x, (int)ball.pos.y, ball.radius, DARKGREEN);

    if (ball.isHeld && player.isCharging) {
        float barWidth = 44.0f;
        float barHeight = 7.0f;
        float barX = player.rect.x + (player.rect.width - barWidth) / 2.0f;
        float barY = player.rect.y - 16.0f;

        float powerRatio = std::clamp(ball.chargeTimer / ball.maxPowerTime, 0.0f, 1.0f);

        DrawRectangle((int)barX, (int)barY, (int)barWidth, (int)barHeight, DARKGRAY);
        Color barColor = (powerRatio >= 1.0f) ? ORANGE : YELLOW;
        DrawRectangle((int)barX, (int)barY, (int)(barWidth * powerRatio), (int)barHeight, barColor);
        DrawRectangleLines((int)barX, (int)barY, (int)barWidth, (int)barHeight, BLACK);
    }
}

void RenderSystem::DrawHUD(const WorldState& world) {
    DrawText("Move: A/D | Crouch: S | Jump: W/Space | Hold Left Click: Charge & Throw Ball", 20, 20, 20, DARKBLUE);

    if (world.level.levelWon) {
        int sw = (int)world.camera.screenWidth;
        int sh = (int)world.camera.screenHeight;
        DrawRectangle(0, 0, sw, sh, Fade(BLACK, 0.6f));
        DrawText("LEVEL 1 COMPLETE!", sw / 2 - 210, sh / 2 - 40, 40, GREEN);
        DrawText("Press 'R' to Play Again or 'ESC' to Quit", sw / 2 - 200, sh / 2 + 20, 20, WHITE);
    }
}