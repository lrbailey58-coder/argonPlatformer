// File: InputSystem.h
#ifndef INPUT_SYSTEM_H
#define INPUT_SYSTEM_H

#include "raylib.h"
#include "WorldState.h"

class InputSystem {
public:
    static void PollInput(WorldState& world) {
        world.input.moveLeft        = IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A);
        world.input.moveRight       = IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D);
        world.input.crouchHeld      = IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S);
        world.input.jumpPressed     = IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W) || IsKeyPressed(KEY_SPACE);
        world.input.jumpHeld        = IsKeyDown(KEY_UP) || IsKeyDown(KEY_W) || IsKeyDown(KEY_SPACE);
        world.input.chargeHeld      = IsMouseButtonDown(MOUSE_BUTTON_LEFT);
        world.input.pauseChargeHeld = IsMouseButtonDown(MOUSE_BUTTON_RIGHT);
        world.input.resetPressed    = IsKeyPressed(KEY_R);

        // Press F11 (or Alt+Enter) to toggle between Fullscreen and Windowed mode
        bool altEnter = (IsKeyDown(KEY_LEFT_ALT) || IsKeyDown(KEY_RIGHT_ALT)) && IsKeyPressed(KEY_ENTER);
        world.input.toggleFullscreenPressed = IsKeyPressed(KEY_F11) || altEnter;

        if (world.input.toggleFullscreenPressed) {
            ToggleBorderlessWindowed();
        }

        // Keep camera offset and zoom synced to current monitor/window dimensions
        world.camera.UpdateViewport((float)GetScreenWidth(), (float)GetScreenHeight());

        // Convert screen-space mouse cursor to scrolling world-space coordinates
        world.input.mouseWorldPos = GetScreenToWorld2D(GetMousePosition(), world.camera.cam);
    }
};

#endif