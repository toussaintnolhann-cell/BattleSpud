#include <3ds.h>
#include <citro3d.h>
#include "game.h"
#include "render.h"
#include "ui.h"

#define SCREEN_TOP_W  400
#define SCREEN_TOP_H  240
#define SCREEN_BOT_W  320
#define SCREEN_BOT_H  240

int main(void) {
    gfxInitDefault();
    C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
    consoleInit(GFX_BOTTOM, NULL);

    C3D_RenderTarget *top = C3D_RenderTargetCreate(SCREEN_TOP_H, SCREEN_TOP_W,
                                                    GPU_RB_RGB8, GPU_RB_DEPTH16);
    C3D_RenderTargetSetOutput(top, GFX_TOP, GFX_LEFT, GX_TRANSFER_FMT(GX_TRANSFER_FMT_RGB8, 0, 0, 0, 0));

    GameState gs;
    game_init(&gs);

    while (aptMainLoop()) {
        hidScanInput();
        u32 kDown = hidKeysDown();
        u32 kHeld = hidKeysHeld();
        circlePosition cpad;
        hidCircleRead(&cpad);

        if (kDown & KEY_START) break;

        game_update(&gs, kDown, kHeld, &cpad);

        C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
        C3D_RenderTargetClear(top, C3D_CLEAR_ALL, 0x1a1a2eff, 0);
        C3D_FrameDrawOn(top);
        render_world(&gs);
        render_players(&gs);
        C3D_FrameEnd(0);

        consoleClear();
        ui_draw_hud(&gs);
    }

    game_free(&gs);
    C3D_Fini();
    gfxExit();
    return 0;
}
