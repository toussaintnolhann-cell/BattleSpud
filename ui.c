#include "ui.h"
#include "game.h"
#include <stdio.h>

void ui_draw_hud(GameState *gs) {
    if (!gs->local.alive) {
        printf("\x1b[10;10H  GAME OVER  ");
        printf("\x1b[11;10H  Alive: %d  ", gs->alive_count);
        return;
    }
    printf("\x1b[1;1H HP: %.0f/100   ", gs->local.hp);
    printf("\x1b[2;1H AMMO: %d       ", gs->local.ammo);
    printf("\x1b[3;1H ALIVE: %d/%d   ", gs->alive_count, MAX_BOTS+1);
    printf("\x1b[4;1H STORM: stage %d ", gs->storm_stage);
    if (gs->alive_count == 1 && gs->local.alive)
        printf("\x1b[10;8H  VICTORY ROYALE!  ");
}
