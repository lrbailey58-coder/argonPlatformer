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
    static void UpdateBall(float dt, BallData& ball, PlayerData& player, LevelData& level, const InputState& input);
    static void SyncBallWithPlayer(BallData& ball, const PlayerData& player, Vector2 mouseWorldPos, const std::vector<Rectangle>& platforms);
    static void ResolveBallPlatformCollision(BallData& ball, const Rectangle& box);
    static void ResolveBallCircleCollision(BallData& ball, Vector2 circleCenter, float circleRadius);
    static void HandleHoopInteraction(BallData& ball, Vector2 prevBallPos, HoopData& hoop, LevelData& level);
};

#endif