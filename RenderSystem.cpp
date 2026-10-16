// File: RenderSystem.cpp
#include "RenderSystem.h"
#include <algorithm>

void RenderSystem::Render(const WorldState& world) {
    BeginDrawing();
    ClearBackground(SKYBLUE);

    // 1. Render Scrolling 2D World
    BeginMode2D(world.camera.cam);
        DrawLevel(world.level);
        DrawHoops(world.level);
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
}

void RenderSystem::DrawHoops(const LevelData& level) {
    for (const auto& hoop : level.hoops) {
        // Cleared hoops turn completely invisible
        if (hoop.cleared) continue;

        Vector2 leftRim  = hoop.GetLeftRimPos();
        Vector2 rightRim = hoop.GetRightRimPos();
        Vector2 tangent  = hoop.GetTangent();
        Vector2 shotDir  = hoop.GetShotDirection();

        // 1. Draw subtle rim line connecting the opening
        DrawLineEx(leftRim, rightRim, 3.0f, Fade(PURPLE, 0.45f));

        // 2. Draw directional arrow indicating the required entry & shot direction
        Vector2 arrowStart = { hoop.center.x - shotDir.x * 20.0f, hoop.center.y - shotDir.y * 20.0f };
        Vector2 arrowTip   = { hoop.center.x + shotDir.x * 16.0f, hoop.center.y + shotDir.y * 16.0f };
        DrawLineEx(arrowStart, arrowTip, 2.5f, GOLD);

        // Arrowhead wings
        Vector2 wingBase = { arrowTip.x - shotDir.x * 8.0f, arrowTip.y - shotDir.y * 8.0f };
        Vector2 wingLeft  = { wingBase.x - tangent.x * 6.0f, wingBase.y - tangent.y * 6.0f };
        Vector2 wingRight = { wingBase.x + tangent.x * 6.0f, wingBase.y + tangent.y * 6.0f };
        DrawLineEx(arrowTip, wingLeft, 2.5f, GOLD);
        DrawLineEx(arrowTip, wingRight, 2.5f, GOLD);

        // 3. Draw the two Purple Rim Dots
        DrawCircleV(leftRim, hoop.rimRadius, PURPLE);
        DrawCircleLines((int)leftRim.x, (int)leftRim.y, hoop.rimRadius, BLACK);

        DrawCircleV(rightRim, hoop.rimRadius, PURPLE);
        DrawCircleLines((int)rightRim.x, (int)rightRim.y, hoop.rimRadius, BLACK);
    }
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
    DrawText("Move: A/D | Crouch: S | Jump: W/Space | Hold Left Click: Charge & Throw Ball", 20, 15, 20, DARKBLUE);

    // Hoop Progress Counter Box
    DrawRectangle(20, 45, 250, 36, Fade(BLACK, 0.55f));
    DrawRectangleLines(20, 45, 250, 36, PURPLE);
    DrawText(
        TextFormat("Hoops Cleared: %d / %d", world.level.hoopsCleared, world.level.totalHoops),
        32, 53, 20, GOLD
    );

    if (world.level.levelWon) {
        int sw = (int)world.camera.screenWidth;
        int sh = (int)world.camera.screenHeight;
        DrawRectangle(0, 0, sw, sh, Fade(BLACK, 0.6f));
        DrawText("ALL HOOPS CLEARED! YOU WIN!", sw / 2 - 300, sh / 2 - 40, 40, GREEN);
        DrawText("Press 'R' to Play Again or 'ESC' to Quit", sw / 2 - 200, sh / 2 + 20, 20, WHITE);
    }
}