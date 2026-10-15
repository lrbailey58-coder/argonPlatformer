// File: Level.h
#ifndef LEVEL_H
#define LEVEL_H

#include "raylib.h"
#include <vector>

class Level {
public:
    std::vector<Rectangle> platforms;
    Rectangle flagPole;
    bool levelWon;
    
    float mapWidth;
    float mapHeight;

    Level() {
        Reset();
    }

    void Reset() {
        levelWon = false;
        platforms.clear();

        // Map Dimensions (2.5x wider and 2x taller than the window)
        mapWidth = 2400.0f;
        mapHeight = 1200.0f;

        // Main Floor across the bottom of the map
        platforms.push_back({ 0, 1100, mapWidth, 100 });

        // Map Outer Boundaries (Left, Right, and Top walls)
        platforms.push_back({ -20, 0, 20, mapHeight });
        platforms.push_back({ mapWidth, 0, 20, mapHeight });
        platforms.push_back({ 0, -20, mapWidth, 20 });

        // Section 1: Ground obstacles to jump over
        platforms.push_back({ 300, 1040, 50, 60 });
        platforms.push_back({ 550, 1000, 60, 100 });
        platforms.push_back({ 820, 960, 60, 140 });

        // Section 2: Low overhang (Crouch under or jump over)
        platforms.push_back({ 1050, 970, 140, 95 });

        // Section 3: Ascending stairs to test Vertical Camera Scrolling
        platforms.push_back({ 1300, 980, 140, 25 });
        platforms.push_back({ 1520, 860, 140, 25 });
        platforms.push_back({ 1300, 740, 140, 25 });
        platforms.push_back({ 1520, 620, 140, 25 });
        platforms.push_back({ 1750, 500, 180, 25 });

        // Section 4: High Victory Platform
        platforms.push_back({ 2020, 400, 300, 40 });

        // Victory Flag at the top-right summit
        flagPole = { 2220, 300, 10, 100 };
    }

    void CheckWin(const Rectangle& playerRect) {
        if (CheckCollisionRecs(playerRect, flagPole)) {
            levelWon = true;
        }
    }

    void Draw() const {
        // Draw a subtle background grid so camera movement is easy to see
        for (int x = 0; x <= (int)mapWidth; x += 100) {
            DrawLine(x, 0, x, (int)mapHeight, Fade(WHITE, 0.25f));
        }
        for (int y = 0; y <= (int)mapHeight; y += 100) {
            DrawLine(0, y, (int)mapWidth, y, Fade(WHITE, 0.25f));
        }

        // Draw all platforms and blocks
        for (const auto& plat : platforms) {
            DrawRectangleRec(plat, DARKGRAY);
            DrawRectangleLinesEx(plat, 2, BLACK);
        }

        // Draw the Flag Pole
        DrawRectangleRec(flagPole, GOLD);
        
        // Draw the Green Flag Banner
        DrawTriangle(
            { flagPole.x + 10, flagPole.y },
            { flagPole.x + 10, flagPole.y + 35 },
            { flagPole.x + 55, flagPole.y + 17 },
            GREEN
        );
    }
};

#endif