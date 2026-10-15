// File: main.cpp
#include "raylib.h"
#include "WorldState.h"
#include "InputSystem.h"
#include "PhysicsSystem.h"
#include "RenderSystem.h"

int main() {
    const float screenWidth = 960.0f;
    const float screenHeight = 600.0f;

    InitWindow((int)screenWidth, (int)screenHeight, "Argon Platformer - Level 1");
    SetTargetFPS(60);

    // 1. Initialize the Model (WorldState)
    WorldState world;
    world.Init(screenWidth, screenHeight);
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