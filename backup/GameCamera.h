// File: GameCamera.h
#ifndef GAMECAMERA_H
#define GAMECAMERA_H

#include "raylib.h"
#include "Player.h"
#include <cmath>
#include <algorithm>

class GameCamera {
public:
    Camera2D cam;
    float screenWidth;
    float screenHeight;
    
    // Dimensions of the "center area" on screen
    float centerBoxWidth;
    float centerBoxHeight;
    
    // Speed settings
    float baseFollowSpeed;
    float borderPushSpeed;

    GameCamera(float screenW, float screenH, Vector2 startPos) {
        screenWidth = screenW;
        screenHeight = screenH;

        // Center area is 240x150 pixels in the middle of the screen
        centerBoxWidth = 240.0f;
        centerBoxHeight = 150.0f;

        baseFollowSpeed = 1.5f;  // Very slow drift when inside the center area
        borderPushSpeed = 10.0f; // Fast catch-up when moving outside the center area

        cam.offset = { screenWidth / 2.0f, screenHeight / 2.0f };
        cam.target = startPos;
        cam.rotation = 0.0f;
        cam.zoom = 1.0f;
    }

    void Update(float dt, const Player& player, float mapWidth, float mapHeight) {
        Vector2 playerCenter = {
            player.rect.x + player.rect.width / 2.0f,
            player.rect.y + player.rect.height / 2.0f
        };

        float diffX = playerCenter.x - cam.target.x;
        float diffY = playerCenter.y - cam.target.y;

        float halfBoxW = centerBoxWidth / 2.0f;
        float halfBoxH = centerBoxHeight / 2.0f;

        // --- Horizontal Camera Movement ---
        // Check if player is outside the center area AND moving away from the center
        bool movingAwayX = (diffX > halfBoxW && player.velocity.x > 0) ||
                           (diffX < -halfBoxW && player.velocity.x < 0);

        if (std::fabs(diffX) > halfBoxW) {
            float excessX = (diffX > 0) ? (diffX - halfBoxW) : (diffX + halfBoxW);
            float speedMultiplier = movingAwayX ? borderPushSpeed : (borderPushSpeed * 0.6f);
            cam.target.x += excessX * speedMultiplier * dt;
            
            // Hard safety clamp so the player can never outrun the center box
            float maxAllowedDistX = halfBoxW + 25.0f;
            if (diffX > maxAllowedDistX) cam.target.x = playerCenter.x - maxAllowedDistX;
            if (diffX < -maxAllowedDistX) cam.target.x = playerCenter.x + maxAllowedDistX;
        }
        // Always apply slow base drift toward the player's center
        cam.target.x += diffX * baseFollowSpeed * dt;

        // --- Vertical Camera Movement ---
        bool movingAwayY = (diffY > halfBoxH && player.velocity.y > 0) ||
                           (diffY < -halfBoxH && player.velocity.y < 0);

        if (std::fabs(diffY) > halfBoxH) {
            float excessY = (diffY > 0) ? (diffY - halfBoxH) : (diffY + halfBoxH);
            float speedMultiplier = movingAwayY ? borderPushSpeed : (borderPushSpeed * 0.6f);
            cam.target.y += excessY * speedMultiplier * dt;

            // Hard safety clamp vertically
            float maxAllowedDistY = halfBoxH + 25.0f;
            if (diffY > maxAllowedDistY) cam.target.y = playerCenter.y - maxAllowedDistY;
            if (diffY < -maxAllowedDistY) cam.target.y = playerCenter.y + maxAllowedDistY;
        }
        // Always apply slow base drift toward the player's center
        cam.target.y += diffY * baseFollowSpeed * dt;

        // --- Clamp Camera to Map Boundaries ---
        float minCamX = screenWidth / 2.0f;
        float maxCamX = mapWidth - (screenWidth / 2.0f);
        float minCamY = screenHeight / 2.0f;
        float maxCamY = mapHeight - (screenHeight / 2.0f);

        if (maxCamX >= minCamX) {
            cam.target.x = std::clamp(cam.target.x, minCamX, maxCamX);
        }
        if (maxCamY >= minCamY) {
            cam.target.y = std::clamp(cam.target.y, minCamY, maxCamY);
        }
    }
};

#endif