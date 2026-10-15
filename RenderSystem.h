// File: RenderSystem.h
#ifndef RENDER_SYSTEM_H
#define RENDER_SYSTEM_H

#include "WorldState.h"

class RenderSystem {
public:
    static void Render(const WorldState& world);

private:
    static void DrawLevel(const LevelData& level);
    static void DrawPlayer(const PlayerData& player);
    static void DrawBallAndChargeBar(const BallData& ball, const PlayerData& player);
    static void DrawHUD(const WorldState& world);
};

#endif