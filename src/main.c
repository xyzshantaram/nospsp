#include "../wic/include/wic.h"
#include "callbacks.h"
#include "controls.h"
#include "glib2d.h"
#include "handle_mu.h"
#include "intraFont.h"
#include "microui.h"
#include "net.h"
#include "ui.h"
#include "util.h"
#include <pspmoduleinfo.h>
#include <pspthreadman.h>
#include <psputility.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

PSP_MODULE_INFO("NostrStation Portable", 0, 1, 1);
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER | THREAD_ATTR_VFPU);
PSP_MAIN_THREAD_STACK_SIZE_KB(3072);
PSP_HEAP_SIZE_KB(-4096);

intraFont *fnt;

static int text_width(mu_Font fnt, const char *text, int len) {
    if (len == -1) {
        len = strlen(text);
    }
    return intraFontMeasureTextEx(fnt, text, len);
}

static int text_height(mu_Font font) { return fnt->size; }

void mainloop(GameState *state, InputState *s, mu_Context *ctx) {
    mu_Command *cmd = NULL;
    g2dClear(G2D_HEX(0x007cdfff));
    mu_begin(ctx);
    mu_demo(ctx);
    mu_end(ctx);

    while (mu_next_command(ctx, &cmd)) {
        switch (cmd->type) {
        case MU_COMMAND_TEXT:
            handle_mu_text(fnt, cmd);
            break;
        case MU_COMMAND_RECT:
            handle_mu_rect(cmd);
            break;
        case MU_COMMAND_ICON:
            handle_mu_icon(cmd);
            break;
        default:
            break;
        case MU_COMMAND_CLIP: {
            mu_Rect r = cmd->clip.rect;
            g2dSetScissor(r.x, r.y, r.w, r.h);
            break;
        }
        }
    }

    draw_cursor(s);
    process_controls(ctx, s, fnt);
    g2dFlip(G2D_VSYNC);
}

int main() {
    g2dInit();
    intraFontInit();
    load_latin_font(&fnt, 0);
    callbacks_setup();
    sceCtrlSetSamplingCycle(0);
    sceCtrlSetSamplingMode(PSP_CTRL_MODE_ANALOG);
    sceUtilityLoadNetModule(PSP_NET_MODULE_COMMON);
    sceUtilityLoadNetModule(PSP_NET_MODULE_INET);

    SceUID nthread = sceKernelCreateThread(
        "User Mode Thread", net_thread,
        0x11,       // default priority
        512 * 1024, // stack size (256KB is regular default)
        PSP_THREAD_ATTR_USER, NULL);

    // start user thread, then wait for it to do everything else
    sceKernelStartThread(nthread, 0, NULL);
    sceKernelWaitThreadEnd(nthread, NULL);

    GameState state = GS_RUNNING;

    mu_Context *ctx = malloc(sizeof *ctx);
    InputState *s = malloc(sizeof *s);
    s->mouse_x = SCREEN_WIDTH / 2;
    s->mouse_y = SCREEN_HEIGHT / 2;
    s->last_mouse_x = 0;
    s->last_mouse_y = 0;
    s->is_keyboard_active = 0;
    mu_init(ctx);
    ctx->text_width = text_width;
    ctx->text_height = text_height;

    while (state != GS_EXITED) {
        mainloop(&state, s, ctx);
    }

    free(ctx);
    free(s);
    sceKernelExitGame();
}