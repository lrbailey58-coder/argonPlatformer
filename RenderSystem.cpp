// File: RenderSystem.cpp
#include "RenderSystem.h"
#include <algorithm>
#include <cmath>

void RenderSystem::Render(const WorldState& world) {
    BeginDrawing();
    ClearBackground(SKYBLUE);

    // 1. Render Scrolling 2D World
    BeginMode2D(world.camera.cam);
        DrawLevel(world.level);
        DrawHoops(world.level);
        DrawPlayer(world.player);
        DrawBallAndChargeBar(world);
    EndMode2D();

    // 2. Render Fixed Screen UI (Scaled dynamically to current screen resolution)
    DrawHUD(world);

    EndDrawing();
}

void RenderSystem::DrawLevel(const LevelData& level) {
    for (int x = 0; x <= (int)level.mapWidth; x += 100) {
        DrawLine(x, 0, x, (int)level.mapHeight, Fade(WHITE, 0.25f));
    }
    for (int y = 0; y <= (int)level.mapHeight; y += 100) {
        DrawLine(0, y, (int)level.mapWidth, y, Fade(WHITE, 0.25f));
    }

    for (const auto& plat : level.platforms) {
        DrawRectangleRec(plat, DARKGRAY);
        DrawRectangleLinesEx(plat, 2, BLACK);
    }
}

void RenderSystem::DrawHoops(const LevelData& level) {
    for (const auto& hoop : level.hoops) {
        if (hoop.cleared) continue;

        Vector2 leftRim  = hoop.GetLeftRimPos();
        Vector2 rightRim = hoop.GetRightRimPos();
        Vector2 tangent  = hoop.GetTangent();
        Vector2 shotDir  = hoop.GetShotDirection();

        DrawLineEx(leftRim, rightRim, 3.0f, Fade(PURPLE, 0.45f));

        Vector2 arrowStart = { hoop.center.x - shotDir.x * 20.0f, hoop.center.y - shotDir.y * 20.0f };
        Vector2 arrowTip   = { hoop.center.x + shotDir.x * 16.0f, hoop.center.y + shotDir.y * 16.0f };
        DrawLineEx(arrowStart, arrowTip, 2.5f, GOLD);

        Vector2 wingBase = { arrowTip.x - shotDir.x * 8.0f, arrowTip.y - shotDir.y * 8.0f };
        Vector2 wingLeft  = { wingBase.x - tangent.x * 6.0f, wingBase.y - tangent.y * 6.0f };
        Vector2 wingRight = { wingBase.x + tangent.x * 6.0f, wingBase.y + tangent.y * 6.0f };
        DrawLineEx(arrowTip, wingLeft, 2.5f, GOLD);
        DrawLineEx(arrowTip, wingRight, 2.5f, GOLD);

        DrawCircleV(leftRim, hoop.rimRadius, PURPLE);
        DrawCircleLines((int)leftRim.x, (int)leftRim.y, hoop.rimRadius, BLACK);

        DrawCircleV(rightRim, hoop.rimRadius, PURPLE);
        DrawCircleLines((int)rightRim.x, (int)rightRim.y, hoop.rimRadius, BLACK);
    }
}

void RenderSystem::DrawPlayer(const PlayerData& player) {
    DrawRectangleRec(player.rect, RED);
}

void RenderSystem::DrawBallAndChargeBar(const WorldState& world) {
    const BallData& ball = world.ball;
    const PlayerData& player = world.player;

    // 1. Draw 25-Dot Trajectory Preview Arc while Charging
    if (ball.isHeld && player.isCharging) {
        Vector2 playerCenter = {
            player.rect.x + player.rect.width / 2.0f,
            player.rect.y + player.rect.height / 2.0f
        };
        Vector2 dir = {
            world.input.mouseWorldPos.x - playerCenter.x,
            world.input.mouseWorldPos.y - playerCenter.y
        };
        float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
        if (len > 0.0001f) {
            dir.x /= len;
            dir.y /= len;
        } else {
            dir = { 1.0f, 0.0f };
        }

        float powerRatio = std::clamp(ball.chargeTimer / ball.maxPowerTime, 0.0f, 1.0f);
        float throwSpeed = ball.minThrowSpeed + powerRatio * (ball.maxThrowSpeed - ball.minThrowSpeed);
        Vector2 launchVel = { dir.x * throwSpeed, dir.y * throwSpeed };

        const int numDots = 25;
        const float timeStep = 0.035f;

        for (int i = 1; i <= numDots; i++) {
            float t = i * timeStep;

            float speedX = std::fabs(launchVel.x);
            float signX  = (launchVel.x >= 0.0f) ? 1.0f : -1.0f;
            float stopTimeX = (ball.airDrag > 0.0f) ? (speedX / ball.airDrag) : t;
            float effectiveT = std::min(t, stopTimeX);

            float offsetX = signX * (speedX * effectiveT - 0.5f * ball.airDrag * effectiveT * effectiveT);

            Vector2 samplePos = {
                ball.pos.x + offsetX,
                ball.pos.y + launchVel.y * t + 0.5f * ball.gravity * t * t
            };

            bool hitObstacle = false;
            for (const auto& plat : world.level.platforms) {
                if (CheckCollisionCircleRec(samplePos, ball.radius * 0.6f, plat)) {
                    hitObstacle = true;
                    break;
                }
            }
            if (hitObstacle) break;

            float alpha = 0.85f * (1.0f - (float)i / (float)(numDots + 2));
            float dotRadius = 3.5f - (1.2f * ((float)i / (float)numDots));

            DrawCircleV(samplePos, dotRadius, Fade(WHITE, alpha));
            DrawCircleLines((int)samplePos.x, (int)samplePos.y, dotRadius, Fade(DARKBLUE, alpha));
        }
    }

    // 2. Draw the Ball
    DrawCircleV(ball.pos, ball.radius, LIME);
    DrawCircleLines((int)ball.pos.x, (int)ball.pos.y, ball.radius, DARKGREEN);

    // 3. Draw Charge Bar above the player
    if (ball.isHeld && player.isCharging) {
        float barWidth = 44.0f;
        float barHeight = 7.0f;
        float barX = player.rect.x + (player.rect.width - barWidth) / 2.0f;
        float barY = player.rect.y - 16.0f;

        float powerRatio = std::clamp(ball.chargeTimer / ball.maxPowerTime, 0.0f, 1.0f);

        DrawRectangle((int)barX, (int)barY, (int)barWidth, (int)barHeight, DARKGRAY);

        Color barColor = (powerRatio >= 1.0f) ? ORANGE : YELLOW;
        if (world.input.pauseChargeHeld) {
            barColor = SKYBLUE;
        }

        DrawRectangle((int)barX, (int)barY, (int)(barWidth * powerRatio), (int)barHeight, barColor);
        DrawRectangleLines((int)barX, (int)barY, (int)barWidth, (int)barHeight, BLACK);
    }
}

void RenderSystem::DrawHUD(const WorldState& world) {
    // Dynamic UI scale factor based on 960x600 base resolution
    float uiScale = (float)GetScreenWidth() / 960.0f;
    if (uiScale < 1.0f) uiScale = 1.0f; // Ensure text never shrinks below base size

    int fontSize = (int)(18.0f * uiScale);
    int padX = (int)(20.0f * uiScale);
    int padY = (int)(15.0f * uiScale);

    // Top instruction text
    DrawText("Move: A/D | Crouch: S | Jump: W/Space | L-Click: Charge | R-Click: Lock | F11: Fullscreen", padX, padY, fontSize, DARKBLUE);

    // Hoop Progress Counter Box (scaled proportionally)
    int boxX = (int)(20.0f * uiScale);
    int boxY = (int)(48.0f * uiScale);
    int boxW = (int)(280.0f * uiScale);
    int boxH = (int)(38.0f * uiScale);

    DrawRectangle(boxX, boxY, boxW, boxH, Fade(BLACK, 0.55f));
    DrawRectangleLines(boxX, boxY, boxW, boxH, PURPLE);
    DrawText(
        TextFormat("Hoops Cleared: %d / %d", world.level.hoopsCleared, world.level.totalHoops),
        boxX + (int)(12.0f * uiScale), boxY + (int)(8.0f * uiScale), (int)(20.0f * uiScale), GOLD
    );

    // Victory Screen Overlay
    if (world.level.levelWon) {
        int sw = GetScreenWidth();
        int sh = GetScreenHeight();
        DrawRectangle(0, 0, sw, sh, Fade(BLACK, 0.6f));

        int titleFontSize = (int)(40.0f * uiScale);
        int subFontSize = (int)(22.0f * uiScale);

        const char* titleText = "ALL HOOPS CLEARED! YOU WIN!";
        const char* subText = "Press 'R' to Play Again or 'ESC' to Quit";

        int titleWidth = MeasureText(titleText, titleFontSize);
        int subWidth = MeasureText(subText, subFontSize);

        DrawText(titleText, (sw - titleWidth) / 2, sh / 2 - (int)(45.0f * uiScale), titleFontSize, GREEN);
        DrawText(subText, (sw - subWidth) / 2, sh / 2 + (int)(15.0f * uiScale), subFontSize, WHITE);
    }
}