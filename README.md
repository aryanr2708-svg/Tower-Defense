# Tower Defense

A small Clash-of-Clans-style defense game written in C with [raylib](https://www.raylib.com/).
Place your core, buy towers with rupees, and survive 60 seconds of enemy attacks.

## How to play
1. Pick a map (Meadow, Lake or Canyon) with 1/2/3 or a click.
2. Click a tile to place your **core**.
3. Pick a tower (1/2/3 or the build bar) and click to build. Faint dots show enemy routes.
4. Press **SPACE** to start the 60-second battle.
5. Survive until the timer hits 0:00 with your core alive.

Press **M** to return to the menu at any time. After a game, **ENTER** goes to the menu and **R** replays the map.

| Tower  | Cost  | Role                              |
|--------|-------|-----------------------------------|
| Archer | Rs 30 | Fast, cheap, single target        |
| Cannon | Rs 60 | Slow, splash damage               |
| Decoy  | Rs 20 | Never shoots, soaks up enemy hits |

Towers expire after 25 seconds and enemies can destroy them. At most 8 towers can be active at once.

## Build (Windows)
Install raylib with its Windows installer (it bundles the w64devkit compiler), then run in PowerShell:

    .\build.bat run

If raylib is not in `C:\raylib`, edit the two paths at the top of `build.bat`.

## Project layout
- `main.c`: window and main loop
- `game.c`: state machine, input, spawn director
- `enemy.c`, `tower.c`, `projectile.c`: entity logic and the data tables to tune
- `path.c`: shortest-route calculation so enemies walk around rock and water
- `maps.c`: the three map layouts
- `ui.c`: all drawing
- `config.h`: tuning numbers; `types.h`: shared structs