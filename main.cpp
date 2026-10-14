// File: main.cpp
#include "raylib.h"
#include "Player.h"
#include "Level.h"
#include "GameCamera.h"

int main() {
    const int screenWidth = 960;
    const int screenHeight = 600;

    InitWindow(screenWidth, screenHeight, "2D Platformer - Level 1");
    SetTargetFPS(60);

    Level level1;
    Player player(80, 1000); // Start near the bottom-left of the map
    GameCamera camera(screenWidth, screenHeight, { player.rect.x, player.rect.y });
    
    // Snap camera inside map bounds on the first frame
    camera.Update(0.0f, player, level1.mapWidth, level1.mapHeight);

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();

        if (!level1.levelWon) {
            player.Update(dt, level1.platforms);
            camera.Update(dt, player, level1.mapWidth, level1.mapHeight);
            level1.CheckWin(player.rect);
        } else {
            if (IsKeyPressed(KEY_R)) {
                level1.Reset();
                player = Player(80, 1000);
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
        EndMode2D();

        // 2. Draw Fixed UI Overlay (Stays on screen regardless of camera position)
        DrawText("Move: A/D or Left/Right | Crouch: S/Down | Jump: W/Up/Space", 20, 20, 20, DARKBLUE);

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