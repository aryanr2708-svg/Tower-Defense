#include <time.h>
#include "raylib.h"
#include "config.h"
#include "types.h"
#include "game.h"
#include "ui.h"

static Game game;     // one big struct holding all game state (static = zero-initialised)

int main(void)
{
    InitWindow(SCREEN_W, SCREEN_H, "Tower Defense");
    SetWindowPosition(50, 50);
    SetTargetFPS(60);
    SetRandomSeed((unsigned int)time(NULL));   // different random spawns every run

    game_init(&game);

    while (!WindowShouldClose())
    {
        float dt = GetFrameTime();
        if (dt > 0.05f) dt = 0.05f;            // don't let a lag spike teleport enemies

        game_update(&game, dt);

        BeginDrawing();
        ClearBackground(BLACK);
        ui_draw(&game);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
