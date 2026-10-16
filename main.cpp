// File: main.cpp
#include "raylib.h"
#include "WorldState.h"
#include "InputSystem.h"
#include "PhysicsSystem.h"
#include "RenderSystem.h"

int main() {
    const int windowedWidth = 960;
    const int windowedHeight = 600;

    InitWindow(windowedWidth, windowedHeight, "Argon Platformer - Level 1");
    SetTargetFPS(60);

    // Launch immediately in smooth Windows 11 Fullscreen (press F11 anytime to toggle windowed)
    ToggleBorderlessWindowed();

    // 1. Initialize the Model (WorldState) using the current screen resolution
    WorldState world;
    world.Init((float)GetScreenWidth(), (float)GetScreenHeight());
    PhysicsSystem::UpdateCamera(0.0f, world);

    // 2. Main Game Loop: Input -> Physics (Controller) -> Render (View)
    while (!WindowShouldClose()) {
        float dt = GetFrameTime();

        InputSystem::PollInput(world);
        PhysicsSystem::UpdateWorld(dt, world);
        RenderSystem::Render(world);
    }

    CloseWindow();
    return 0;
}