// File: main.cpp
#include "raylib.h"
#include "Player.h"
#include "Level.h"
#include "GameCamera.h"
#include "Ball.h"

int main() {
    const int screenWidth = 960;
    const int screenHeight = 600;

    InitWindow(screenWidth, screenHeight, "2D Platformer - Level 1");
    SetTargetFPS(60);

    Level level1;
    Player player(80, 1000); // Start near the bottom-left of the map
    Ball ball;
    ball.Reset(player);

    GameCamera camera(screenWidth, screenHeight, { player.rect.x, player.rect.y });
    camera.Update(0.0f, player, level1.mapWidth, level1.mapHeight);

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();

        if (!level1.levelWon) {
            // Convert screen mouse coordinates into scrolling world coordinates
            Vector2 mouseWorldPos = GetScreenToWorld2D(GetMousePosition(), camera.cam);

            // Update ball charge/throw state and player physics
            ball.Update(dt, player, level1.platforms, mouseWorldPos);
            player.Update(dt, level1.platforms);

            // Keep the held ball indicator synced to the player's updated position
            ball.SyncWithPlayer(player, mouseWorldPos, level1.platforms);

            camera.Update(dt, player, level1.mapWidth, level1.mapHeight);
            level1.CheckWin(player.rect);
        } else {
            if (IsKeyPressed(KEY_R)) {
                level1.Reset();
                player = Player(80, 1000);
                ball.Reset(player);
                camera = GameCamera(screenWidth, screenHeight, { player.rect.x, player.rect.y });
                camera.Update(0.0f, player, level1.mapWidth, level1.mapHeight);
            }
        }

        // --- Drawing Section ---
        BeginDrawing();
        ClearBackground(SKYBLUE);

        // 1. Draw the Scrolling Game World
        BeginMode2D(camera.cam);
            level1.Draw();
            player.Draw();
            ball.Draw(player);
        EndMode2D();

        // 2. Draw Fixed UI Overlay
        DrawText("Move: A/D | Crouch: S | Jump: W/Space | Hold Left Click: Charge & Throw Ball", 20, 20, 20, DARKBLUE);

        if (level1.levelWon) {
            DrawRectangle(0, 0, screenWidth, screenHeight, Fade(BLACK, 0.6f));
            DrawText("LEVEL 1 COMPLETE!", screenWidth / 2 - 210, screenHeight / 2 - 40, 40, GREEN);
            DrawText("Press 'R' to Play Again or 'ESC' to Quit", screenWidth / 2 - 200, screenHeight / 2 + 20, 20, WHITE);
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}