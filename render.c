#include "render.h"
#include "game.h"
#include <citro3d.h>
#include <math.h>

#define COL_GRASS  0x3a7d44ff
#define COL_WALL   0x5c4a2aff
#define COL_LOOT   0xffd700ff
#define COL_WATER  0x1e90ffff
#define COL_PLAYER 0x00ff88ff
#define COL_BOT    0xff4444ff

#define VIEW_TILES_X 25
#define VIEW_TILES_Y 15

static void draw_rect(float sx, float sy, float sw, float sh, u32 col) {
    C3D_ImmDrawBegin(GPU_TRIANGLE_STRIP);
        C3D_ImmSendAttrib(sx,    sy,    0.5f, 0); C3D_ImmSendAttrib(0,0,0,0);
        C3D_ImmSendAttrib(sx+sw, sy,    0.5f, 0); C3D_ImmSendAttrib(0,0,0,0);
        C3D_ImmSendAttrib(sx,    sy+sh, 0.5f, 0); C3D_ImmSendAttrib(0,0,0,0);
        C3D_ImmSendAttrib(sx+sw, sy+sh, 0.5f, 0); C3D_ImmSendAttrib(0,0,0,0);
    C3D_ImmDrawEnd();
    (void)col;
}

void render_world(GameState *gs) {
    float cam_x = gs->local.x / TILE_SIZE - VIEW_TILES_X / 2.0f;
    float cam_y = gs->local.y / TILE_SIZE - VIEW_TILES_Y / 2.0f;
    float tw = 400.0f / VIEW_TILES_X;
    float th = 240.0f / VIEW_TILES_Y;
    for (int ty = 0; ty < VIEW_TILES_Y; ty++) {
        for (int tx = 0; tx < VIEW_TILES_X; tx++) {
            int wx = (int)(cam_x + tx);
            int wy = (int)(cam_y + ty);
            if (wx < 0 || wy < 0 || wx >= MAP_W || wy >= MAP_H) continue;
            u32 col;
            switch (gs->tiles[wy][wx]) {
                case TILE_WALL:  col = COL_WALL;  break;
                case TILE_LOOT:  col = COL_LOOT;  break;
                case TILE_WATER: col = COL_WATER; break;
                default:         col = COL_GRASS; break;
            }
            draw_rect(tx*tw, ty*th, tw-1, th-1, col);
        }
    }
}

void render_players(GameState *gs) {
    float cam_x = gs->local.x / TILE_SIZE - VIEW_TILES_X / 2.0f;
    float cam_y = gs->local.y / TILE_SIZE - VIEW_TILES_Y / 2.0f;
    float tw = 400.0f / VIEW_TILES_X;
    float th = 240.0f / VIEW_TILES_Y;
    if (gs->local.alive) {
        float sx = (gs->local.x/TILE_SIZE - cam_x)*tw - tw*0.4f;
        float sy = (gs->local.y/TILE_SIZE - cam_y)*th - th*0.4f;
        draw_rect(sx, sy, tw*0.8f, th*0.8f, COL_PLAYER);
    }
    for (int i = 0; i < MAX_BOTS; i++) {
        if (!gs->bots[i].alive) continue;
        float sx = (gs->bots[i].x/TILE_SIZE - cam_x)*tw - tw*0.4f;
        float sy = (gs->bots[i].y/TILE_SIZE - cam_y)*th - th*0.4f;
        draw_rect(sx, sy, tw*0.8f, th*0.8f, COL_BOT);
    }
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (!gs->bullets[i].active) continue;
        float sx = (gs->bullets[i].x/TILE_SIZE - cam_x)*tw;
        float sy = (gs->bullets[i].y/TILE_SIZE - cam_y)*th;
        draw_rect(sx-2, sy-2, 4, 4, 0xffffffff);
    }
}
