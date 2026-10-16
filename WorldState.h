// File: WorldState.h
#ifndef WORLD_STATE_H
#define WORLD_STATE_H

#include "raylib.h"
#include <vector>
#include <cmath>

struct InputState {
    bool moveLeft = false;
    bool moveRight = false;
    bool crouchHeld = false;
    bool jumpPressed = false;
    bool jumpHeld = false;
    bool chargeHeld = false;
    bool resetPressed = false;
    Vector2 mouseWorldPos = { 0.0f, 0.0f };
};

struct PlayerData {
    Rectangle rect;
    Vector2 velocity;
    bool isGrounded;
    bool isCrouching;
    bool isCharging;

    float normalHeight = 50.0f;
    float crouchHeight = 25.0f;
    float moveSpeed = 300.0f;
    float crouchSpeed = 150.0f;
    float airSpeed = 300.0f;
    float jumpForce = -540.0f;
    float gravity = 1100.0f;

    float coyoteTimeDuration = 0.12f;
    float coyoteTimer = 0.0f;
    float jumpBufferDuration = 0.12f;
    float jumpBufferTimer = 0.0f;

    void Init(float x, float y) {
        rect = { x, y, 40.0f, normalHeight };
        velocity = { 0.0f, 0.0f };
        isGrounded = false;
        isCrouching = false;
        isCharging = false;
        airSpeed = moveSpeed;
        coyoteTimer = 0.0f;
        jumpBufferTimer = 0.0f;
    }
};

struct BallData {
    Vector2 pos;
    Vector2 velocity;
    float radius = 7.8f; // Diameter = 15.6f
    bool isHeld = true;
    bool hasLeftPlayer = false;

    float chargeTimer = 0.0f;
    float maxPowerTime = 2.0f;
    float maxHoldTime = 5.0f;
    float minThrowSpeed = 250.0f;
    float maxThrowSpeed = 1100.0f;

    float gravity = 950.0f;
    float bounciness = 0.75f;
    float groundFriction = 420.0f;
    float airDrag = 20.0f;

    void Init(const PlayerData& player) {
        isHeld = true;
        hasLeftPlayer = false;
        chargeTimer = 0.0f;
        velocity = { 0.0f, 0.0f };
        pos = {
            player.rect.x + player.rect.width / 2.0f,
            player.rect.y + player.rect.height / 2.0f
        };
    }
};

struct HoopData {
    Vector2 center;
    float rotationDeg = 0.0f;  // 0 deg = horizontal hoop, shot from top to bottom
    float openingWidth = 24.0f; // ~1.54x the ball's 15.6f diameter
    float rimRadius = 5.0f;     // Radius of each purple rim dot
    bool cleared = false;

    // Unit vector along the hoop plane (from center toward right rim)
    Vector2 GetTangent() const {
        float rad = rotationDeg * DEG2RAD;
        return { std::cos(rad), std::sin(rad) };
    }

    // Unit vector pointing in the valid shot/scoring direction (top-to-bottom at 0 deg)
    Vector2 GetShotDirection() const {
        float rad = rotationDeg * DEG2RAD;
        return { -std::sin(rad), std::cos(rad) };
    }

    Vector2 GetLeftRimPos() const {
        Vector2 t = GetTangent();
        float dist = (openingWidth / 2.0f) + rimRadius;
        return { center.x - t.x * dist, center.y - t.y * dist };
    }

    Vector2 GetRightRimPos() const {
        Vector2 t = GetTangent();
        float dist = (openingWidth / 2.0f) + rimRadius;
        return { center.x + t.x * dist, center.y + t.y * dist };
    }
};

struct LevelData {
    std::vector<Rectangle> platforms;
    std::vector<HoopData> hoops;
    int hoopsCleared = 0;
    int totalHoops = 0;
    bool levelWon = false;
    float mapWidth = 2400.0f;
    float mapHeight = 1200.0f;

    void Init() {
        levelWon = false;
        platforms.clear();
        hoops.clear();

        // Main Floor
        platforms.push_back({ 0, 1100, mapWidth, 100 });

        // Map Outer Boundaries (Left, Right, and Top walls)
        platforms.push_back({ -20, 0, 20, mapHeight });
        platforms.push_back({ mapWidth, 0, 20, mapHeight });
        platforms.push_back({ 0, -20, mapWidth, 20 });

        // Section 1: Ground obstacles
        platforms.push_back({ 300, 1040, 50, 60 });
        platforms.push_back({ 550, 1000, 60, 100 });
        platforms.push_back({ 820, 960, 60, 140 });

        // Section 2: Low overhang
        platforms.push_back({ 1050, 970, 140, 95 });

        // Section 3: Ascending stairs
        platforms.push_back({ 1300, 980, 140, 25 });
        platforms.push_back({ 1520, 860, 140, 25 });
        platforms.push_back({ 1300, 740, 140, 25 });
        platforms.push_back({ 1520, 620, 140, 25 });
        platforms.push_back({ 1750, 500, 180, 25 });

        // Section 4: High Platform
        platforms.push_back({ 2020, 400, 300, 40 });

        // --- 4 Hoops to Clear ---
        // Hoop 1: Standard horizontal hoop floating above the ground obstacles (0 deg: shoot top -> bottom)
        hoops.push_back({ { 430.0f, 880.0f }, 0.0f, 24.0f, 5.0f, false });

        // Hoop 2: Tilted hoop past the low overhang (-35 deg: angled bank/arc shot from upper-left to lower-right)
        hoops.push_back({ { 1120.0f, 800.0f }, -35.0f, 24.0f, 5.0f, false });

        // Hoop 3: Tilted hoop mid-way up the ascending stairs (25 deg: shoot from upper-right to lower-left)
        hoops.push_back({ { 1420.0f, 640.0f }, 25.0f, 24.0f, 5.0f, false });

        // Hoop 4: Final hoop high above the summit platform (0 deg: classic basketball arc shot top -> bottom)
        hoops.push_back({ { 2180.0f, 240.0f }, 0.0f, 24.0f, 5.0f, false });

        hoopsCleared = 0;
        totalHoops = (int)hoops.size();
    }
};

struct CameraData {
    Camera2D cam;
    float screenWidth = 960.0f;
    float screenHeight = 600.0f;
    float centerBoxWidth = 240.0f;
    float centerBoxHeight = 150.0f;
    float baseFollowSpeed = 1.5f;
    float borderPushSpeed = 10.0f;

    void Init(float screenW, float screenH, Vector2 startPos) {
        screenWidth = screenW;
        screenHeight = screenH;
        cam.offset = { screenWidth / 2.0f, screenHeight / 2.0f };
        cam.target = startPos;
        cam.rotation = 0.0f;
        cam.zoom = 1.0f;
    }
};

struct WorldState {
    InputState input;
    PlayerData player;
    BallData ball;
    LevelData level;
    CameraData camera;

    void Init(float screenW, float screenH) {
        level.Init();
        player.Init(80.0f, 1000.0f);
        ball.Init(player);
        camera.Init(screenW, screenH, { player.rect.x, player.rect.y });
    }
};

#endif