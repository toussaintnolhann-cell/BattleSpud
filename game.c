#include "game.h"
#include <string.h>
#include <stdlib.h>
#include <math.h>

#define STORM_SHRINK_TICKS  600
#define PLAYER_SPEED        0.08f
#define BULLET_SPEED        0.6f
#define BOT_SPEED           0.05f
#define PLAYER_HP           100.0f
#define BULLET_DMG          25.0f
#define STORM_DMG_TICK      0.3f

static bool tile_solid(GameState *gs, int tx, int ty) {
    if (tx < 0 || ty < 0 || tx >= MAP_W || ty >= MAP_H) return true;
    return gs->tiles[ty][tx] == TILE_WALL || gs->tiles[ty][tx] == TILE_WATER;
}

static void spawn_map(GameState *gs) {
    srand(0xDEADBEEF);
    for (int y = 0; y < MAP_H; y++)
        for (int x = 0; x < MAP_W; x++) {
            int r = rand() % 100;
            if (r < 15)      gs->tiles[y][x] = TILE_WALL;
            else if (r < 20) gs->tiles[y][x] = TILE_LOOT;
            else if (r < 23) gs->tiles[y][x] = TILE_WATER;
            else             gs->tiles[y][x] = TILE_GRASS;
        }
    for (int i = 0; i < MAP_W; i++) {
        gs->tiles[0][i]       = TILE_WALL;
        gs->tiles[MAP_H-1][i] = TILE_WALL;
    }
    for (int i = 0; i < MAP_H; i++) {
        gs->tiles[i][0]       = TILE_WALL;
        gs->tiles[i][MAP_W-1] = TILE_WALL;
    }
}

void game_init(GameState *gs) {
    memset(gs, 0, sizeof(*gs));
    spawn_map(gs);
    gs->local.x    = MAP_W / 2.0f * TILE_SIZE;
    gs->local.y    = MAP_H / 2.0f * TILE_SIZE;
    gs->local.hp   = PLAYER_HP;
    gs->local.ammo = 30;
    gs->local.alive    = true;
    gs->local.is_local = true;
    srand(12345);
    for (int i = 0; i < MAX_BOTS; i++) {
        int tx, ty;
        do { tx = 4 + rand()%(MAP_W-8); ty = 4 + rand()%(MAP_H-8); }
        while (tile_solid(gs, tx, ty));
        gs->bots[i].x    = tx * TILE_SIZE;
        gs->bots[i].y    = ty * TILE_SIZE;
        gs->bots[i].hp   = PLAYER_HP;
        gs->bots[i].ammo = 20;
        gs->bots[i].alive = true;
    }
    gs->storm_cx     = MAP_W / 2.0f * TILE_SIZE;
    gs->storm_cy     = MAP_H / 2.0f * TILE_SIZE;
    gs->storm_radius = MAP_W * TILE_SIZE * 0.5f;
    gs->alive_count  = MAX_BOTS + 1;
}

static void move_player(GameState *gs, u32 kHeld, circlePosition *cpad) {
    float dx = cpad->dx / 156.0f * PLAYER_SPEED;
    float dy = -(cpad->dy / 156.0f) * PLAYER_SPEED;
    float nx = gs->local.x + dx * TILE_SIZE;
    float ny = gs->local.y + dy * TILE_SIZE;
    int tx = (int)(nx / TILE_SIZE), ty = (int)(ny / TILE_SIZE);
    if (!tile_solid(gs, tx, (int)(gs->local.y / TILE_SIZE))) gs->local.x = nx;
    if (!tile_solid(gs, (int)(gs->local.x / TILE_SIZE), ty)) gs->local.y = ny;
    int lx = (int)(gs->local.x / TILE_SIZE), ly = (int)(gs->local.y / TILE_SIZE);
    if (gs->tiles[ly][lx] == TILE_LOOT) {
        gs->local.ammo += 10;
        if (gs->local.hp < PLAYER_HP) gs->local.hp += 20.0f;
        if (gs->local.hp > PLAYER_HP) gs->local.hp = PLAYER_HP;
        gs->tiles[ly][lx] = TILE_GRASS;
    }
}

static void fire_bullet(GameState *gs, float aim_dx, float aim_dy, int owner) {
    if (owner == -1 && gs->local.ammo <= 0) return;
    if (owner >= 0  && gs->bots[owner].ammo <= 0) return;
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (!gs->bullets[i].active) {
            float ox  = (owner == -1) ? gs->local.x : gs->bots[owner].x;
            float oy  = (owner == -1) ? gs->local.y : gs->bots[owner].y;
            float len = sqrtf(aim_dx*aim_dx + aim_dy*aim_dy);
            if (len < 0.001f) return;
            gs->bullets[i].x  = ox;
            gs->bullets[i].y  = oy;
            gs->bullets[i].vx = (aim_dx/len) * BULLET_SPEED;
            gs->bullets[i].vy = (aim_dy/len) * BULLET_SPEED;
            gs->bullets[i].active = true;
            gs->bullets[i].owner  = owner;
            if (owner == -1) gs->local.ammo--;
            else gs->bots[owner].ammo--;
            return;
        }
    }
}

static void update_bullets(GameState *gs) {
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (!gs->bullets[i].active) continue;
        gs->bullets[i].x += gs->bullets[i].vx * TILE_SIZE;
        gs->bullets[i].y += gs->bullets[i].vy * TILE_SIZE;
        int tx = (int)(gs->bullets[i].x / TILE_SIZE);
        int ty = (int)(gs->bullets[i].y / TILE_SIZE);
        if (tile_solid(gs, tx, ty)) { gs->bullets[i].active = false; continue; }
        if (gs->bullets[i].owner != -1 && gs->local.alive) {
            float ddx = gs->bullets[i].x - gs->local.x;
            float ddy = gs->bullets[i].y - gs->local.y;
            if (ddx*ddx + ddy*ddy < (TILE_SIZE*0.8f)*(TILE_SIZE*0.8f)) {
                gs->local.hp -= BULLET_DMG;
                gs->bullets[i].active = false;
                if (gs->local.hp <= 0) { gs->local.alive = false; gs->alive_count--; }
            }
        }
        for (int b = 0; b < MAX_BOTS; b++) {
            if (!gs->bots[b].alive) continue;
            if (gs->bullets[i].owner == b) continue;
            float ddx = gs->bullets[i].x - gs->bots[b].x;
            float ddy = gs->bullets[i].y - gs->bots[b].y;
            if (ddx*ddx + ddy*ddy < (TILE_SIZE*0.8f)*(TILE_SIZE*0.8f)) {
                gs->bots[b].hp -= BULLET_DMG;
                gs->bullets[i].active = false;
                if (gs->bots[b].hp <= 0) { gs->bots[b].alive = false; gs->alive_count--; }
                break;
            }
        }
    }
}

static void update_bots(GameState *gs) {
    for (int i = 0; i < MAX_BOTS; i++) {
        if (!gs->bots[i].alive || !gs->local.alive) continue;
        float dx   = gs->local.x - gs->bots[i].x;
        float dy   = gs->local.y - gs->bots[i].y;
        float dist = sqrtf(dx*dx + dy*dy);
        if (dist > TILE_SIZE * 6) {
            float nx = gs->bots[i].x + (dx/dist)*BOT_SPEED*TILE_SIZE;
            float ny = gs->bots[i].y + (dy/dist)*BOT_SPEED*TILE_SIZE;
            int tx = (int)(nx/TILE_SIZE), ty = (int)(ny/TILE_SIZE);
            if (!tile_solid(gs, tx, (int)(gs->bots[i].y/TILE_SIZE))) gs->bots[i].x = nx;
            if (!tile_solid(gs, (int)(gs->bots[i].x/TILE_SIZE), ty)) gs->bots[i].y = ny;
        }
        if (dist < TILE_SIZE * 10 && (gs->tick % 90) == (u64)(i * 13))
            fire_bullet(gs, dx, dy, i);
    }
}

static void update_storm(GameState *gs) {
    gs->storm_timer++;
    if (gs->storm_timer >= STORM_SHRINK_TICKS && gs->storm_stage < STORM_STAGES) {
        gs->storm_stage++;
        gs->storm_timer = 0;
        gs->storm_radius *= 0.72f;
    }
    if (gs->local.alive) {
        float dx = gs->local.x - gs->storm_cx;
        float dy = gs->local.y - gs->storm_cy;
        if (sqrtf(dx*dx+dy*dy) > gs->storm_radius) {
            gs->local.hp -= STORM_DMG_TICK;
            if (gs->local.hp <= 0) { gs->local.alive = false; gs->alive_count--; }
        }
    }
    for (int i = 0; i < MAX_BOTS; i++) {
        if (!gs->bots[i].alive) continue;
        float dx = gs->bots[i].x - gs->storm_cx;
        float dy = gs->bots[i].y - gs->storm_cy;
        if (sqrtf(dx*dx+dy*dy) > gs->storm_radius) {
            gs->bots[i].hp -= STORM_DMG_TICK;
            if (gs->bots[i].hp <= 0) { gs->bots[i].alive = false; gs->alive_count--; }
        }
    }
}

void game_update(GameState *gs, u32 kDown, u32 kHeld, circlePosition *cpad) {
    if (!gs->local.alive) { gs->tick++; return; }
    move_player(gs, kHeld, cpad);
    float aim_dx = 0, aim_dy = 0;
    if (kHeld & KEY_DRIGHT) aim_dx =  1;
    if (kHeld & KEY_DLEFT)  aim_dx = -1;
    if (kHeld & KEY_DDOWN)  aim_dy =  1;
    if (kHeld & KEY_DUP)    aim_dy = -1;
    if ((kDown & KEY_A) && (aim_dx != 0 || aim_dy != 0))
        fire_bullet(gs, aim_dx, aim_dy, -1);
    update_bullets(gs);
    update_bots(gs);
    update_storm(gs);
    gs->tick++;
}

void game_free(GameState *gs) { (void)gs; }
