// File: Ball.h
#ifndef BALL_H
#define BALL_H

#include "raylib.h"
#include "Player.h"
#include <vector>

class Ball {
public:
    Vector2 pos;
    Vector2 velocity;
    float radius;
    bool isHeld;

    // Throw charging settings
    float chargeTimer;
    float maxPowerTime;    // 2.0 seconds to reach 100% throw power
    float maxHoldTime;     // 5.0 seconds before auto-firing
    float minThrowSpeed;
    float maxThrowSpeed;

    // Hitbox-exit pickup tracking
    bool hasLeftPlayer;    // Becomes true once the ball fully exits the player's hitbox

    // Basketball physics properties
    float gravity;
    float bounciness;
    float groundFriction;
    float airDrag;

    Ball();
    void Reset(const Player& player);
    void SyncWithPlayer(const Player& player, Vector2 mouseWorldPos, const std::vector<Rectangle>& platforms);
    void Update(float dt, Player& player, const std::vector<Rectangle>& platforms, Vector2 mouseWorldPos);
    void Draw(const Player& player) const;

private:
    Rectangle GetBoundingBox() const;
    void ResolvePlatformCollision(const Rectangle& box);
};

#endif