#pragma once
#include <3ds.h>
#include <stdbool.h>

#define MAP_W        64
#define MAP_H        64
#define TILE_SIZE    8
#define MAX_BOTS     7
#define MAX_BULLETS  32
#define STORM_STAGES 6

typedef enum { TILE_GRASS=0, TILE_WALL, TILE_LOOT, TILE_WATER } TileType;

typedef struct {
    float x, y;
    float dx, dy;
    float hp;
    int   ammo;
    bool  alive;
    bool  is_local;
} Player;

typedef struct {
    float x, y;
    float vx, vy;
    bool  active;
    int   owner;
} Bullet;

typedef struct {
    TileType tiles[MAP_H][MAP_W];
    Player   local;
    Player   bots[MAX_BOTS];
    Bullet   bullets[MAX_BULLETS];
    float    storm_radius;
    float    storm_cx, storm_cy;
    int      storm_stage;
    u64      storm_timer;
    int      alive_count;
    u64      tick;
} GameState;

void game_init(GameState *gs);
void game_update(GameState *gs, u32 kDown, u32 kHeld, circlePosition *cpad);
void game_free(GameState *gs);
