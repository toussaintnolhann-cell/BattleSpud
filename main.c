
#include <3ds.h>
#include <citro3d.h>
#include <stdbool.h>

#include "game.h"
#include "render.h"
#include "ui.h"

#define SCREEN_TOP_W  400
#define SCREEN_TOP_H  240

#define DISPLAY_TRANSFER_FLAGS ( \
    GX_TRANSFER_FLIP_VERT(0) | \
    GX_TRANSFER_OUT_TILED(0) | \
    GX_TRANSFER_RAW_COPY(0) | \
    GX_TRANSFER_IN_FORMAT(GX_TRANSFER_FMT_RGB8) | \
    GX_TRANSFER_OUT_FORMAT(GX_TRANSFER_FMT_RGB8) | \
    GX_TRANSFER_SCALING(GX_TRANSFER_SCALE_NO) \
)

int main(void)
{
    // Initialisation de la Nintendo 3DS
    gfxInitDefault();

    if (!C3D_Init(C3D_DEFAULT_CMDBUF_SIZE)) {
        gfxExit();
        return 1;
    }

    consoleInit(GFX_BOTTOM, NULL);

    // Création de l'écran supérieur
    C3D_RenderTarget *top = C3D_RenderTargetCreate(
        SCREEN_TOP_H,
        SCREEN_TOP_W,
        GPU_RB_RGB8,
        GPU_RB_DEPTH16
    );

    if (top == NULL) {
        C3D_Fini();
        gfxExit();
        return 1;
    }

    C3D_RenderTargetSetOutput(
        top,
        GFX_TOP,
        GFX_LEFT,
        DISPLAY_TRANSFER_FLAGS
    );

    // Initialisation du jeu
    GameState gs;
    game_init(&gs);

    // Boucle principale
    while (aptMainLoop())
    {
        hidScanInput();

        u32 kDown = hidKeysDown();
        u32 kHeld = hidKeysHeld();

        circlePosition cpad;
        hidCircleRead(&cpad);

        // Quitter avec START
        if (kDown & KEY_START)
            break;

        // Mise à jour du jeu
        game_update(&gs, kDown, kHeld, &cpad);

        // Rendu graphique
        C3D_FrameBegin(C3D_FRAME_SYNCDRAW);

        C3D_RenderTargetClear(
            top,
            C3D_CLEAR_ALL,
            0x1a1a2eff,
            0
        );

        C3D_FrameDrawOn(top);

        render_world(&gs);
        render_players(&gs);

        C3D_FrameEnd(0);

        // Interface écran inférieur
        consoleClear();
        ui_draw_hud(&gs);
    }

    // Nettoyage
    game_free(&gs);

    C3D_RenderTargetDelete(top);
    C3D_Fini();
    gfxExit();

    return 0;
}
