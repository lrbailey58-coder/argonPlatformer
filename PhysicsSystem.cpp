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

    // 1. Update Ball state (charging, firing, bouncing, and hoop scoring)
    UpdateBall(dt, world.ball, world.player, world.level, world.input);

    // 2. Update Player physics and movement (Player does not collide with hoops)
    UpdatePlayer(dt, world.player, world.input, world.level.platforms);

    // 3. Re-sync held ball indicator to the player's new post-movement position
    SyncBallWithPlayer(world.ball, world.player, world.input.mouseWorldPos, world.level.platforms);

    // 4. Update Smooth Deadzone Camera
    UpdateCamera(dt, world);
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

void PhysicsSystem::ResolveBallCircleCollision(BallData& ball, Vector2 circleCenter, float circleRadius) {
    float dx = ball.pos.x - circleCenter.x;
    float dy = ball.pos.y - circleCenter.y;
    float distSq = dx * dx + dy * dy;
    float minDist = ball.radius + circleRadius;

    if (distSq < minDist * minDist) {
        float dist = std::sqrt(distSq);
        Vector2 normal = (dist > 0.0001f) ? Vector2{ dx / dist, dy / dist } : Vector2{ 0.0f, -1.0f };

        // Push the ball out of the purple rim circle
        float overlap = minDist - dist;
        ball.pos.x += normal.x * overlap;
        ball.pos.y += normal.y * overlap;

        // Reflect velocity vector off the circular rim
        float velDotNormal = ball.velocity.x * normal.x + ball.velocity.y * normal.y;
        if (velDotNormal < 0.0f) {
            ball.velocity.x -= (1.0f + ball.bounciness) * velDotNormal * normal.x;
            ball.velocity.y -= (1.0f + ball.bounciness) * velDotNormal * normal.y;
        }
    }
}

void PhysicsSystem::HandleHoopInteraction(BallData& ball, Vector2 prevBallPos, HoopData& hoop, LevelData& level) {
    if (hoop.cleared) return;

    // 1. Bounce the ball off the two circular purple rim dots
    ResolveBallCircleCollision(ball, hoop.GetLeftRimPos(), hoop.rimRadius);
    ResolveBallCircleCollision(ball, hoop.GetRightRimPos(), hoop.rimRadius);

    // 2. Compute ball coordinates in the hoop's local rotated reference frame
    Vector2 tangent = hoop.GetTangent();
    Vector2 shotDir = hoop.GetShotDirection(); // Valid scoring direction

    Vector2 relPrev = { prevBallPos.x - hoop.center.x, prevBallPos.y - hoop.center.y };
    Vector2 relCurr = { ball.pos.x - hoop.center.x, ball.pos.y - hoop.center.y };

    // Signed distance along the shot direction (negative = entry/top side, positive = exit/bottom side)
    float prevNormalDist = relPrev.x * shotDir.x + relPrev.y * shotDir.y;
    float currNormalDist = relCurr.x * shotDir.x + relCurr.y * shotDir.y;

    // Lateral distance from the center of the hoop opening
    float currTangentDist = relCurr.x * tangent.x + relCurr.y * tangent.y;
    float halfOpening = hoop.openingWidth / 2.0f;

    // Check if the ball is laterally within the hoop's opening
    if (std::fabs(currTangentDist) <= halfOpening) {
        // Case A: Ball crosses the hoop center-line in the VALID direction (from <= 0 to > 0)
        if (prevNormalDist <= 0.0f && currNormalDist > 0.0f) {
            hoop.cleared = true;
            level.hoopsCleared++;
            if (level.hoopsCleared >= level.totalHoops) {
                level.levelWon = true;
            }
            return;
        }

        // Case B: One-way block! If the ball approaches from the WRONG side (currNormalDist > 0)
        // and tries to enter the hoop opening moving against shotDir, bounce it back out!
        float velAlongShot = ball.velocity.x * shotDir.x + ball.velocity.y * shotDir.y;
        if (prevNormalDist > 0.0f && currNormalDist < ball.radius && velAlongShot < 0.0f) {
            // Push ball flush to the back/bottom of the hoop plane
            float pushOut = ball.radius - currNormalDist;
            ball.pos.x += shotDir.x * pushOut;
            ball.pos.y += shotDir.y * pushOut;

            // Reflect velocity component along the hoop normal
            ball.velocity.x -= (1.0f + ball.bounciness) * velAlongShot * shotDir.x;
            ball.velocity.y -= (1.0f + ball.bounciness) * velAlongShot * shotDir.y;
        }
    }
}

void PhysicsSystem::UpdateBall(float dt, BallData& ball, PlayerData& player, LevelData& level, const InputState& input) {
    const auto& platforms = level.platforms;

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
            Vector2 prevBallPos = ball.pos;

            ball.pos.x += ball.velocity.x * subDt;
            ball.pos.y += ball.velocity.y * subDt;

            // 1. Platform Collisions
            for (const auto& plat : platforms) {
                ResolveBallPlatformCollision(ball, plat);
            }

            // 2. Hoop Collisions & One-Way Scoring (Only when ball is thrown!)
            for (auto& hoop : level.hoops) {
                HandleHoopInteraction(ball, prevBallPos, hoop, level);
            }

            // 3. Player Hitbox-Exit & Pickup Check
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

    float minCamX = camData.screenWidth / 2.0f;
    float maxCamX = world.level.mapWidth - (camData.screenWidth / 2.0f);
    float minCamY = camData.screenHeight / 2.0f;
    float maxCamY = world.level.mapHeight - (camData.screenHeight / 2.0f);

    if (maxCamX >= minCamX) camData.cam.target.x = std::clamp(camData.cam.target.x, minCamX, maxCamX);
    if (maxCamY >= minCamY) camData.cam.target.y = std::clamp(camData.cam.target.y, minCamY, maxCamY);
}