// File: Ball.cpp
#include "Ball.h"
#include <cmath>
#include <algorithm>

Ball::Ball() {
    radius = 7.8f; // 1.3x bigger than 6.0f
    isHeld = true;
    pos = { 0.0f, 0.0f };
    velocity = { 0.0f, 0.0f };

    chargeTimer = 0.0f;
    maxPowerTime = 1.4f;    // Max power reached at 1.4 seconds
    maxHoldTime = 3.0f;     // Auto-fire at 3 seconds
    minThrowSpeed = 250.0f;
    maxThrowSpeed = 1100.0f;

    hasLeftPlayer = false;

    gravity = 950.0f;
    bounciness = 0.7f;     // Basketball bounce restitution
    groundFriction = 750.0f; // 20% higher ground friction
    airDrag = 70.0f;
}

void Ball::Reset(const Player& player) {
    isHeld = true;
    hasLeftPlayer = false;
    chargeTimer = 0.0f;
    velocity = { 0.0f, 0.0f };
    pos = {
        player.rect.x + player.rect.width / 2.0f,
        player.rect.y + player.rect.height / 2.0f
    };
}

void Ball::SyncWithPlayer(const Player& player, Vector2 mouseWorldPos, const std::vector<Rectangle>& platforms) {
    if (!isHeld) return;

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

    // Step outward from the player's center up to 18.0f, stopping early if the indicator hits a floor or wall
    float maxOrbitRadius = 18.0f;
    Vector2 safePos = playerCenter;

    for (float dist = 1.0f; dist <= maxOrbitRadius; dist += 1.0f) {
        Vector2 testPos = {
            playerCenter.x + dir.x * dist,
            playerCenter.y + dir.y * dist
        };
        Rectangle testBox = { testPos.x - radius, testPos.y - radius, radius * 2.0f, radius * 2.0f };

        bool hitsEnvironment = false;
        for (const auto& plat : platforms) {
            if (CheckCollisionRecs(testBox, plat)) {
                hitsEnvironment = true;
                break;
            }
        }

        if (hitsEnvironment) {
            break; // Stop right before clipping into the floor/wall
        }
        safePos = testPos;
    }

    pos = safePos;
}

Rectangle Ball::GetBoundingBox() const {
    return { pos.x - radius, pos.y - radius, radius * 2.0f, radius * 2.0f };
}

void Ball::ResolvePlatformCollision(const Rectangle& box) {
    Rectangle ballBox = GetBoundingBox();
    if (!CheckCollisionRecs(ballBox, box)) return;

    // Calculate how far the ball is overlapping the platform on all 4 sides
    float overlapLeft   = (pos.x + radius) - box.x;
    float overlapRight  = (box.x + box.width) - (pos.x - radius);
    float overlapTop    = (pos.y + radius) - box.y;
    float overlapBottom = (box.y + box.height) - (pos.y - radius);

    float minOverlapX = std::min(overlapLeft, overlapRight);
    float minOverlapY = std::min(overlapTop, overlapBottom);

    // Resolve along whichever axis has the smaller overlap (prevents teleporting to platform edges!)
    if (minOverlapX < minOverlapY) {
        if (overlapLeft < overlapRight) {
            pos.x = box.x - radius;
            if (velocity.x > 0.0f) velocity.x = -velocity.x * bounciness;
        } else {
            pos.x = box.x + box.width + radius;
            if (velocity.x < 0.0f) velocity.x = -velocity.x * bounciness;
        }
        if (std::fabs(velocity.x) < 15.0f) velocity.x = 0.0f;
    } else {
        if (overlapTop < overlapBottom) {
            pos.y = box.y - radius;
            if (velocity.y > 0.0f) velocity.y = -velocity.y * bounciness;
            // Settle onto the floor when vertical bounces become tiny
            if (std::fabs(velocity.y) < 45.0f) {
                velocity.y = 0.0f;
            }
        } else {
            pos.y = box.y + box.height + radius;
            if (velocity.y < 0.0f) velocity.y = -velocity.y * bounciness;
        }
    }
}

void Ball::Update(float dt, Player& player, const std::vector<Rectangle>& platforms, Vector2 mouseWorldPos) {
    if (isHeld) {
        // 1. Update indicator position around the player (clamped so it never clips into floors/walls)
        SyncWithPlayer(player, mouseWorldPos, platforms);

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

        // 2. Charging & Firing with Left Mouse Button
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            player.isCharging = true;
            chargeTimer += dt;

            // Automatically fire from the indicator's position if held for maxHoldTime (5 seconds)
            if (chargeTimer >= maxHoldTime) {
                isHeld = false;
                player.isCharging = false;
                hasLeftPlayer = !CheckCollisionCircleRec(pos, radius, player.rect);

                velocity = { dir.x * maxThrowSpeed, dir.y * maxThrowSpeed };
                chargeTimer = 0.0f;
            }
        } else if (player.isCharging) {
            // Left click was released before 5 seconds -> Fire from the indicator's current pos!
            float powerRatio = std::clamp(chargeTimer / maxPowerTime, 0.0f, 1.0f);
            float throwSpeed = minThrowSpeed + powerRatio * (maxThrowSpeed - minThrowSpeed);

            isHeld = false;
            player.isCharging = false;
            hasLeftPlayer = !CheckCollisionCircleRec(pos, radius, player.rect);

            velocity = { dir.x * throwSpeed, dir.y * throwSpeed };
            chargeTimer = 0.0f;
        }
    } else {
        // Ball is in the world (not held)
        player.isCharging = false;

        // Apply Gravity
        velocity.y += gravity * dt;

        // Apply horizontal friction (stronger when rolling on a floor, light drag in the air)
        bool rollingOnFloor = (std::fabs(velocity.y) < 1.0f);
        float decel = (rollingOnFloor ? groundFriction : airDrag) * dt;
        if (std::fabs(velocity.x) <= decel) {
            velocity.x = 0.0f;
        } else {
            velocity.x -= (velocity.x > 0.0f ? decel : -decel);
        }

        // Sub-step physics into 8 mini-steps per frame for ultra-reliable high-speed bounces
        const int subSteps = 8;
        float subDt = dt / (float)subSteps;

        for (int step = 0; step < subSteps; step++) {
            pos.x += velocity.x * subDt;
            pos.y += velocity.y * subDt;

            // Bounce off floors, ceilings, and walls using minimum-penetration resolution
            for (const auto& plat : platforms) {
                ResolvePlatformCollision(plat);
            }

            // Track when the ball has completely exited the player's hitbox
            if (!hasLeftPlayer) {
                if (!CheckCollisionCircleRec(pos, radius, player.rect)) {
                    hasLeftPlayer = true;
                }
            } else {
                // Once it has left the player's hitbox, touching the player picks it back up!
                if (CheckCollisionCircleRec(pos, radius, player.rect)) {
                    isHeld = true;
                    hasLeftPlayer = false;
                    velocity = { 0.0f, 0.0f };
                    chargeTimer = 0.0f;
                    SyncWithPlayer(player, mouseWorldPos, platforms);
                    break;
                }
            }
        }
    }
}

void Ball::Draw(const Player& player) const {
    // Draw the light green ball with a dark green outline
    DrawCircleV(pos, radius, LIME);
    DrawCircleLines((int)pos.x, (int)pos.y, radius, DARKGREEN);

    // Draw a charge bar above the player while holding Left Click
    if (isHeld && player.isCharging) {
        float barWidth = 44.0f;
        float barHeight = 7.0f;
        float barX = player.rect.x + (player.rect.width - barWidth) / 2.0f;
        float barY = player.rect.y - 16.0f;

        float powerRatio = std::clamp(chargeTimer / maxPowerTime, 0.0f, 1.0f);

        DrawRectangle((int)barX, (int)barY, (int)barWidth, (int)barHeight, DARKGRAY);
        Color barColor = (powerRatio >= 1.0f) ? ORANGE : YELLOW;
        DrawRectangle((int)barX, (int)barY, (int)(barWidth * powerRatio), (int)barHeight, barColor);
        DrawRectangleLines((int)barX, (int)barY, (int)barWidth, (int)barHeight, BLACK);
    }
}