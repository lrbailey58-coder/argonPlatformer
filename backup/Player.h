// File: Player.h
#ifndef PLAYER_H
#define PLAYER_H

#include "raylib.h"
#include <vector>

class Player {
public:
    Rectangle rect;
    Vector2 velocity;
    bool isGrounded;
    bool isCrouching;
    bool isCharging;
    
    float normalHeight;
    float crouchHeight;
    float moveSpeed;
    float crouchSpeed;
    float airSpeed;
    float jumpForce;
    float gravity;

    // Coyote Time & Jump Buffering Timers
    float coyoteTimeDuration;
    float coyoteTimer;
    float jumpBufferDuration;
    float jumpBufferTimer;

    Player(float x, float y);
    void Update(float dt, const std::vector<Rectangle>& platforms);
    void Draw() const;
};

#endif