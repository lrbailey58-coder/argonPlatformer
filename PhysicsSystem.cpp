// File: PhysicsSystem.cpp
#include "PhysicsSystem.h"
#include <cmath>
#include <algorithm>

void PhysicsSystem::UpdateWorld(float dt, WorldState& world) {
    if (world.level.levelWon) {
        if (world.input.resetPressed) {
            world.Init(world.camera.screenWidth, world.camera.screenHeight);
            UpdateCamera(0.0f, world);
        }
        return;
    }

    // 1. Update Ball state (charging, firing, or bouncing)
    UpdateBall(dt, world.ball, world.player, world.input, world.level.platforms);

    // 2. Update Player physics and movement
    UpdatePlayer(dt, world.player, world.input, world.level.platforms);

    // 3. Re-sync held ball indicator to the player's new post-movement position
    SyncBallWithPlayer(world.ball, world.player, world.input.mouseWorldPos, world.level.platforms);

    // 4. Update Smooth Deadzone Camera
    UpdateCamera(dt, world);

    // 5. Check Victory Condition
    if (CheckCollisionRecs(world.player.rect, world.level.flagPole)) {
        world.level.levelWon = true;
    }
}

void PhysicsSystem::UpdatePlayer(float dt, PlayerData& player, const InputState& input, const std::vector<Rectangle>& platforms) {
    // 1. Crouch Logic
    if (input.crouchHeld && !player.isCrouching) {
        player.rect.y += (player.normalHeight - player.crouchHeight);
        player.rect.height = player.crouchHeight;
        player.isCrouching = true;
    } else if (!input.crouchHeld && player.isCrouching) {
        Rectangle standRect = {
            player.rect.x,
            player.rect.y - (player.normalHeight - player.crouchHeight),
            player.rect.width,
            player.normalHeight
        };
        bool canStand = true;
        for (const auto& plat : platforms) {
            if (CheckCollisionRecs(standRect, plat)) {
                canStand = false;
                break;
            }
        }
        if (canStand) {
            player.rect.y -= (player.normalHeight - player.crouchHeight);
            player.rect.height = player.normalHeight;
            player.isCrouching = false;
        }
    }

    // 2. Coyote Timer & Grounded Takeoff Speed Memory
    if (player.isGrounded) {
        player.coyoteTimer = player.coyoteTimeDuration;
        player.airSpeed = player.isCrouching ? player.crouchSpeed : player.moveSpeed;
    } else {
        player.coyoteTimer -= dt;
    }

    // 3. Horizontal Movement (Locked on ground while charging; preserves air momentum)
    if (player.isCharging) {
        if (player.isGrounded) {
            player.velocity.x = 0.0f;
        }
    } else {
        float currentSpeed = player.isGrounded ? (player.isCrouching ? player.crouchSpeed : player.moveSpeed) : player.airSpeed;
        player.velocity.x = 0.0f;
        if (input.moveLeft)  player.velocity.x -= currentSpeed;
        if (input.moveRight) player.velocity.x += currentSpeed;
    }

    // 4. Jump Buffering, Coyote Jump & Variable Jump Height
    if (input.jumpPressed) {
        player.jumpBufferTimer = player.jumpBufferDuration;
    } else {
        player.jumpBufferTimer -= dt;
    }

    if (player.jumpBufferTimer > 0.0f && player.coyoteTimer > 0.0f) {
        player.velocity.y = player.jumpForce;
        player.isGrounded = false;
        player.coyoteTimer = 0.0f;
        player.jumpBufferTimer = 0.0f;
    }

    if (player.velocity.y < 0.0f && !input.jumpHeld) {
        player.velocity.y *= 0.5f;
    }

    // 5. Apply Gravity
    player.velocity.y += player.gravity * dt;

    // 6. Move Horizontally & Resolve X Collisions
    player.rect.x += player.velocity.x * dt;
    for (const auto& plat : platforms) {
        if (CheckCollisionRecs(player.rect, plat)) {
            if (player.velocity.x > 0.0f) {
                player.rect.x = plat.x - player.rect.width;
                if (player.isCharging) player.velocity.x = 0.0f;
            } else if (player.velocity.x < 0.0f) {
                player.rect.x = plat.x + plat.width;
                if (player.isCharging) player.velocity.x = 0.0f;
            }
        }
    }

    // 7. Move Vertically & Resolve Y Collisions
    player.rect.y += player.velocity.y * dt;
    player.isGrounded = false;
    for (const auto& plat : platforms) {
        if (CheckCollisionRecs(player.rect, plat)) {
            if (player.velocity.y > 0.0f) {
                player.rect.y = plat.y - player.rect.height;
                player.velocity.y = 0.0f;
                player.isGrounded = true;
            } else if (player.velocity.y < 0.0f) {
                player.rect.y = plat.y + plat.height;
                player.velocity.y = 0.0f;
            }
        }
    }
}

void PhysicsSystem::SyncBallWithPlayer(BallData& ball, const PlayerData& player, Vector2 mouseWorldPos, const std::vector<Rectangle>& platforms) {
    if (!ball.isHeld) return;

    Vector2 playerCenter = {
        player.rect.x + player.rect.width / 2.0f,
        player.rect.y + player.rect.height / 2.0f
    };

    Vector2 dir = { mouseWorldPos.x - playerCenter.x, mouseWorldPos.y - playerCenter.y };
    float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
    if (len > 0.0001f) {
        dir.x /= len;
        dir.y /= len;
    } else {
        dir = { 1.0f, 0.0f };
    }

    // Step outward up to 18.0f, stopping before clipping into any floor or wall
    float maxOrbitRadius = 18.0f;
    Vector2 safePos = playerCenter;

    for (float dist = 1.0f; dist <= maxOrbitRadius; dist += 1.0f) {
        Vector2 testPos = { playerCenter.x + dir.x * dist, playerCenter.y + dir.y * dist };
        Rectangle testBox = { testPos.x - ball.radius, testPos.y - ball.radius, ball.radius * 2.0f, ball.radius * 2.0f };

        bool hitsEnvironment = false;
        for (const auto& plat : platforms) {
            if (CheckCollisionRecs(testBox, plat)) {
                hitsEnvironment = true;
                break;
            }
        }
        if (hitsEnvironment) break;
        safePos = testPos;
    }

    ball.pos = safePos;
}

void PhysicsSystem::ResolveBallPlatformCollision(BallData& ball, const Rectangle& box) {
    Rectangle ballBox = { ball.pos.x - ball.radius, ball.pos.y - ball.radius, ball.radius * 2.0f, ball.radius * 2.0f };
    if (!CheckCollisionRecs(ballBox, box)) return;

    float overlapLeft   = (ball.pos.x + ball.radius) - box.x;
    float overlapRight  = (box.x + box.width) - (ball.pos.x - ball.radius);
    float overlapTop    = (ball.pos.y + ball.radius) - box.y;
    float overlapBottom = (box.y + box.height) - (ball.pos.y - ball.radius);

    float minOverlapX = std::min(overlapLeft, overlapRight);
    float minOverlapY = std::min(overlapTop, overlapBottom);

    if (minOverlapX < minOverlapY) {
        if (overlapLeft < overlapRight) {
            ball.pos.x = box.x - ball.radius;
            if (ball.velocity.x > 0.0f) ball.velocity.x = -ball.velocity.x * ball.bounciness;
        } else {
            ball.pos.x = box.x + box.width + ball.radius;
            if (ball.velocity.x < 0.0f) ball.velocity.x = -ball.velocity.x * ball.bounciness;
        }
        if (std::fabs(ball.velocity.x) < 15.0f) ball.velocity.x = 0.0f;
    } else {
        if (overlapTop < overlapBottom) {
            ball.pos.y = box.y - ball.radius;
            if (ball.velocity.y > 0.0f) ball.velocity.y = -ball.velocity.y * ball.bounciness;
            if (std::fabs(ball.velocity.y) < 45.0f) ball.velocity.y = 0.0f;
        } else {
            ball.pos.y = box.y + box.height + ball.radius;
            if (ball.velocity.y < 0.0f) ball.velocity.y = -ball.velocity.y * ball.bounciness;
        }
    }
}

void PhysicsSystem::UpdateBall(float dt, BallData& ball, PlayerData& player, const InputState& input, const std::vector<Rectangle>& platforms) {
    if (ball.isHeld) {
        SyncBallWithPlayer(ball, player, input.mouseWorldPos, platforms);

        Vector2 playerCenter = {
            player.rect.x + player.rect.width / 2.0f,
            player.rect.y + player.rect.height / 2.0f
        };
        Vector2 dir = { input.mouseWorldPos.x - playerCenter.x, input.mouseWorldPos.y - playerCenter.y };
        float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
        if (len > 0.0001f) {
            dir.x /= len;
            dir.y /= len;
        } else {
            dir = { 1.0f, 0.0f };
        }

        if (input.chargeHeld) {
            player.isCharging = true;
            ball.chargeTimer += dt;

            if (ball.chargeTimer >= ball.maxHoldTime) {
                ball.isHeld = false;
                player.isCharging = false;
                ball.hasLeftPlayer = !CheckCollisionCircleRec(ball.pos, ball.radius, player.rect);
                ball.velocity = { dir.x * ball.maxThrowSpeed, dir.y * ball.maxThrowSpeed };
                ball.chargeTimer = 0.0f;
            }
        } else if (player.isCharging) {
            float powerRatio = std::clamp(ball.chargeTimer / ball.maxPowerTime, 0.0f, 1.0f);
            float throwSpeed = ball.minThrowSpeed + powerRatio * (ball.maxThrowSpeed - ball.minThrowSpeed);

            ball.isHeld = false;
            player.isCharging = false;
            ball.hasLeftPlayer = !CheckCollisionCircleRec(ball.pos, ball.radius, player.rect);
            ball.velocity = { dir.x * throwSpeed, dir.y * throwSpeed };
            ball.chargeTimer = 0.0f;
        }
    } else {
        player.isCharging = false;
        ball.velocity.y += ball.gravity * dt;

        bool rollingOnFloor = (std::fabs(ball.velocity.y) < 1.0f);
        float decel = (rollingOnFloor ? ball.groundFriction : ball.airDrag) * dt;
        if (std::fabs(ball.velocity.x) <= decel) {
            ball.velocity.x = 0.0f;
        } else {
            ball.velocity.x -= (ball.velocity.x > 0.0f ? decel : -decel);
        }

        const int subSteps = 8;
        float subDt = dt / (float)subSteps;

        for (int step = 0; step < subSteps; step++) {
            ball.pos.x += ball.velocity.x * subDt;
            ball.pos.y += ball.velocity.y * subDt;

            for (const auto& plat : platforms) {
                ResolveBallPlatformCollision(ball, plat);
            }

            if (!ball.hasLeftPlayer) {
                if (!CheckCollisionCircleRec(ball.pos, ball.radius, player.rect)) {
                    ball.hasLeftPlayer = true;
                }
            } else {
                if (CheckCollisionCircleRec(ball.pos, ball.radius, player.rect)) {
                    ball.isHeld = true;
                    ball.hasLeftPlayer = false;
                    ball.velocity = { 0.0f, 0.0f };
                    ball.chargeTimer = 0.0f;
                    SyncBallWithPlayer(ball, player, input.mouseWorldPos, platforms);
                    break;
                }
            }
        }
    }
}

void PhysicsSystem::UpdateCamera(float dt, WorldState& world) {
    CameraData& camData = world.camera;
    const PlayerData& player = world.player;

    Vector2 playerCenter = {
        player.rect.x + player.rect.width / 2.0f,
        player.rect.y + player.rect.height / 2.0f
    };

    float diffX = playerCenter.x - camData.cam.target.x;
    float diffY = playerCenter.y - camData.cam.target.y;

    float halfBoxW = camData.centerBoxWidth / 2.0f;
    float halfBoxH = camData.centerBoxHeight / 2.0f;

    // Horizontal Camera Follow
    bool movingAwayX = (diffX > halfBoxW && player.velocity.x > 0) ||
                       (diffX < -halfBoxW && player.velocity.x < 0);

    if (std::fabs(diffX) > halfBoxW) {
        float excessX = (diffX > 0) ? (diffX - halfBoxW) : (diffX + halfBoxW);
        float speedMultiplier = movingAwayX ? camData.borderPushSpeed : (camData.borderPushSpeed * 0.6f);
        camData.cam.target.x += excessX * speedMultiplier * dt;

        float maxAllowedDistX = halfBoxW + 25.0f;
        if (diffX > maxAllowedDistX)  camData.cam.target.x = playerCenter.x - maxAllowedDistX;
        if (diffX < -maxAllowedDistX) camData.cam.target.x = playerCenter.x + maxAllowedDistX;
    }
    camData.cam.target.x += diffX * camData.baseFollowSpeed * dt;

    // Vertical Camera Follow
    bool movingAwayY = (diffY > halfBoxH && player.velocity.y > 0) ||
                       (diffY < -halfBoxH && player.velocity.y < 0);

    if (std::fabs(diffY) > halfBoxH) {
        float excessY = (diffY > 0) ? (diffY - halfBoxH) : (diffY + halfBoxH);
        float speedMultiplier = movingAwayY ? camData.borderPushSpeed : (camData.borderPushSpeed * 0.6f);
        camData.cam.target.y += excessY * speedMultiplier * dt;

        float maxAllowedDistY = halfBoxH + 25.0f;
        if (diffY > maxAllowedDistY)  camData.cam.target.y = playerCenter.y - maxAllowedDistY;
        if (diffY < -maxAllowedDistY) camData.cam.target.y = playerCenter.y + maxAllowedDistY;
    }
    camData.cam.target.y += diffY * camData.baseFollowSpeed * dt;

    // Clamp to Map Edges
    float minCamX = camData.screenWidth / 2.0f;
    float maxCamX = world.level.mapWidth - (camData.screenWidth / 2.0f);
    float minCamY = camData.screenHeight / 2.0f;
    float maxCamY = world.level.mapHeight - (camData.screenHeight / 2.0f);

    if (maxCamX >= minCamX) camData.cam.target.x = std::clamp(camData.cam.target.x, minCamX, maxCamX);
    if (maxCamY >= minCamY) camData.cam.target.y = std::clamp(camData.cam.target.y, minCamY, maxCamY);
}