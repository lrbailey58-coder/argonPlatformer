// File: Player.cpp
#include "Player.h"

Player::Player(float x, float y) {
    normalHeight = 50.0f;
    crouchHeight = 25.0f; // Half height when crouching
    rect = { x, y, 40.0f, normalHeight };
    velocity = { 0.0f, 0.0f };
    
    isGrounded = false;
    isCrouching = false;
    isCharging = false;
    
    moveSpeed = 300.0f;
    crouchSpeed = 150.0f; // Half speed while crouching on the ground
    airSpeed = moveSpeed; // Stores the speed cap from when the player left the ground
    jumpForce = -540.0f;
    gravity = 1100.0f;

    // 0.12 seconds (~7 frames at 60 FPS)
    coyoteTimeDuration = 0.12f;
    coyoteTimer = 0.0f;
    jumpBufferDuration = 0.12f;
    jumpBufferTimer = 0.0f;
}

void Player::Update(float dt, const std::vector<Rectangle>& platforms) {
    // 1. Crouch Input (Down Arrow or S)
    bool wantsToCrouch = IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S);
    if (wantsToCrouch && !isCrouching) {
        rect.y += (normalHeight - crouchHeight);
        rect.height = crouchHeight;
        isCrouching = true;
    } else if (!wantsToCrouch && isCrouching) {
        // Check if standing up would clip into a low ceiling above us
        Rectangle standRect = { rect.x, rect.y - (normalHeight - crouchHeight), rect.width, normalHeight };
        bool canStand = true;
        for (const auto& plat : platforms) {
            if (CheckCollisionRecs(standRect, plat)) {
                canStand = false;
                break;
            }
        }
        if (canStand) {
            rect.y -= (normalHeight - crouchHeight);
            rect.height = normalHeight;
            isCrouching = false;
        }
    }

    // 2. Update Coyote Timer & Grounded Takeoff Speed
    if (isGrounded) {
        coyoteTimer = coyoteTimeDuration;
        airSpeed = isCrouching ? crouchSpeed : moveSpeed;
    } else {
        coyoteTimer -= dt;
    }

    // 3. Horizontal Input
    if (isCharging) {
        // Cannot move on the ground while charging; keep existing momentum if airborne!
        if (isGrounded) {
            velocity.x = 0.0f;
        }
    } else {
        float currentSpeed = isGrounded ? (isCrouching ? crouchSpeed : moveSpeed) : airSpeed;

        velocity.x = 0.0f;
        if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A)) {
            velocity.x -= currentSpeed;
        }
        if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) {
            velocity.x += currentSpeed;
        }
    }

    // 4. Jump Buffering, Coyote Jump & Variable Jump Height (Still allowed while charging!)
    bool jumpPressed = IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W) || IsKeyPressed(KEY_SPACE);
    bool jumpHeld = IsKeyDown(KEY_UP) || IsKeyDown(KEY_W) || IsKeyDown(KEY_SPACE);

    if (jumpPressed) {
        jumpBufferTimer = jumpBufferDuration;
    } else {
        jumpBufferTimer -= dt;
    }

    // Trigger jump if a buffered jump input overlaps with grounded/coyote time
    if (jumpBufferTimer > 0.0f && coyoteTimer > 0.0f) {
        velocity.y = jumpForce;
        isGrounded = false;
        coyoteTimer = 0.0f;
        jumpBufferTimer = 0.0f;
    }

    // If moving upward and the player releases the jump key early, cut upward velocity for a short hop
    if (velocity.y < 0.0f && !jumpHeld) {
        velocity.y *= 0.5f;
    }

    // 5. Apply Gravity
    velocity.y += gravity * dt;

    // 6. Move Horizontally & Resolve X Collisions
    rect.x += velocity.x * dt;
    for (const auto& plat : platforms) {
        if (CheckCollisionRecs(rect, plat)) {
            if (velocity.x > 0) {
                rect.x = plat.x - rect.width;
                if (isCharging) velocity.x = 0.0f; // Stop air momentum if slamming into a wall
            } else if (velocity.x < 0) {
                rect.x = plat.x + plat.width;
                if (isCharging) velocity.x = 0.0f;
            }
        }
    }

    // 7. Move Vertically & Resolve Y Collisions
    rect.y += velocity.y * dt;
    isGrounded = false;
    for (const auto& plat : platforms) {
        if (CheckCollisionRecs(rect, plat)) {
            if (velocity.y > 0) {
                rect.y = plat.y - rect.height; // Landed on top of platform
                velocity.y = 0;
                isGrounded = true;
            } else if (velocity.y < 0) {
                rect.y = plat.y + plat.height; // Hit head on bottom of platform
                velocity.y = 0;
            }
        }
    }
}

void Player::Draw() const {
    DrawRectangleRec(rect, RED);
}