# Argon Platformer

A custom 2D physics platformer built in C++ using **Raylib**. Navigate across obstacles and ascending platforms, use your bouncing ball, and reach the green victory flag at the summit.

---

## Controls

| Action | Keys / Mouse |
| :--- | :--- |
| **Move Left / Right** | `A` / `D` or `Left Arrow` / `Right Arrow` |
| **Crouch** | `S` or `Down Arrow` *(50% height and slower ground speed)* |
| **Jump** | `W`, `Spacebar`, or `Up Arrow` *(Tap for a short hop, hold for a full jump)* |
| **Aim Ball** | Move Mouse *(The held green ball orbits toward your cursor)* |
| **Charge & Throw Ball** | Hold `Left Click` to charge (reaches max power at 2s; auto-fires at 5s), release to throw |
| **Restart (After Winning)** | `R` |
| **Quit Game** | `ESC` |

---

## Gameplay Mechanics

* **Air Speed Preservation:** Your horizontal air speed is locked to the movement speed you had when leaving the ground. Jumping at full standing speed and crouching in mid-air lets you squeeze through tight elevated gaps at full speed.
* **Coyote Time & Jump Buffering:** Includes a `0.12s` forgiveness window when running off ledges or pressing jump right before touching the ground.
* **Basketball Physics:** You have a single light-green ball. While charging a throw on the ground, horizontal movement is locked (though you can still jump or preserve existing air momentum). Once thrown, the ball bounces off walls, ceilings, and floors. Walk into the ball after it clears your hitbox to pick it back up.
* **Smooth Deadzone Camera:** The camera drifts gently toward the player and accelerates if you move outside the center box, clamping cleanly to the edges of the `2400x1200` map.

---

## Prerequisites & Setup (Windows 11)

1. **GNU C++ Compiler (`g++`) via MSYS2**
2. **Raylib Library:** If not already installed in your MSYS2 UCRT64 environment, run:
   ```bash
   pacman -S mingw-w64-ucrt-x86_64-raylib

## Project Files

Ensure all six source files are in the same directory:

* main.cpp -- Main game loop, window initialization, and UI overlay

* Player.h / Player.cpp -- Player movement, crouch/jump physics, coyote time, and collisions

* Ball.h / Ball.cpp -- Ball aiming orbit, charge shot logic, and sub-stepped bounce physics

* Level.h -- Level 1 platform layout, boundaries, and victory flag

* GameCamera.h -- Smooth 2D camera controller with center-box deadzone and boundary clamping

# How to Compile and Run

Open PowerShell (or your MSYS2 terminal) inside the project folder and run:
```PowerShell

g++ main.cpp Player.cpp Ball.cpp -o game.exe -lraylib -lopengl32 -lgdi32 -lwinmm

```

(Note: Add -static to the end of the command if you want a standalone executable that bundles all required .dll files).

Launch the game:

```PowerShell

./game.exe
```