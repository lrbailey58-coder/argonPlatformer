// File: PhysicsSystem.h
#ifndef PHYSICS_SYSTEM_H
#define PHYSICS_SYSTEM_H

#include "WorldState.h"

class PhysicsSystem {
public:
    static void UpdateWorld(float dt, WorldState& world);
    static void UpdateCamera(float dt, WorldState& world);

private:
    static void UpdatePlayer(float dt, PlayerData& player, const InputState& input, const std::vector<Rectangle>& platforms);
    static void UpdateBall(float dt, BallData& ball, PlayerData& player, const InputState& input, const std::vector<Rectangle>& platforms);
    static void SyncBallWithPlayer(BallData& ball, const PlayerData& player, Vector2 mouseWorldPos, const std::vector<Rectangle>& platforms);
    static void ResolveBallPlatformCollision(BallData& ball, const Rectangle& box);
};

#endif