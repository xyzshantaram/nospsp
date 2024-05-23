#include "callbacks.h"
#include "glib2d.h"
#include "icons/check.c"
#include "icons/close.c"
#include "icons/collapsed.c"
#include "icons/cursor.c"
#include "icons/expanded.c"
#include "icons/white.c"
#include "intraFont.h"
#include "microui.h"
#include "util.h"
#include <pspctrl.h>
#include <pspmoduleinfo.h>
#include <pspnet.h>
#include <pspnet_apctl.h>
#include <pspnet_inet.h>
#include <pspthreadman.h>
#include <psptypes.h>
#include <psputility.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TARGET_FPS 60
#define PSP_SCR_WIDTH 480
#define PSP_SCR_HEIGHT 272

PSP_MODULE_INFO("NostrStation Portable", 0, 1, 1);
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER | THREAD_ATTR_VFPU);
PSP_MAIN_THREAD_STACK_SIZE_KB(1024);
PSP_HEAP_SIZE_KB(-4096);

static void ConfigureDialog(pspUtilityDialogCommon *dialog,
                            size_t dialog_size) {
    memset(dialog, 0, sizeof(pspUtilityDialogCommon));

    dialog->size = dialog_size;
    sceUtilityGetSystemParamInt(PSP_SYSTEMPARAM_ID_INT_LANGUAGE,
                                &dialog->language); // Prompt language
    sceUtilityGetSystemParamInt(PSP_SYSTEMPARAM_ID_INT_UNKNOWN,
                                &dialog->buttonSwap); // X/O button swap
    dialog->graphicsThread = 0x11;
    dialog->accessThread = 0x13;
    dialog->fontThread = 0x12;
    dialog->soundThread = 0x10;
}

int psp_DisplayNetDialog(void) {
    int ret = 0, done = 0;
    pspUtilityNetconfData data;
    struct pspUtilityNetconfAdhoc adhocparam;

    memset(&adhocparam, 0, sizeof(adhocparam));
    memset(&data, 0, sizeof(pspUtilityNetconfData));

    ConfigureDialog(&data.base, sizeof(pspUtilityNetconfData));
    data.action = PSP_NETCONF_ACTION_CONNECTAP;
    data.hotspot = 0;
    data.adhocparam = &adhocparam;

    if ((ret = sceUtilityNetconfInitStart(&data)) < 0) {
        printf("sceUtilityNetconfInitStart() failed: 0x%08x", ret);
        return ret;
    }

    do {
        g2dDrawNetDialogBg();
        done = sceUtilityNetconfGetStatus();
        switch (done) {
        case PSP_UTILITY_DIALOG_VISIBLE:
            if ((ret = sceUtilityNetconfUpdate(1)) < 0) {
                printf("sceUtilityNetconfUpdate() failed: 0x%08x", ret);
            }
            break;

        case PSP_UTILITY_DIALOG_QUIT:
            if ((ret = sceUtilityNetconfShutdownStart()) < 0) {
                printf("sceUtilityNetconfShutdownStart() failed: 0x%08x", ret);
            }
            break;
        default:
            break;
        }
        g2dFlip(G2D_VSYNC_NO_FINISH);
    } while (done != PSP_UTILITY_DIALOG_NONE);

    done = PSP_NET_APCTL_STATE_DISCONNECTED;
    if ((ret = sceNetApctlGetState(&done)) < 0) {
        printf("sceNetApctlGetState() failed: 0x%08x", ret);
        return 0;
    }

    return (done == PSP_NET_APCTL_STATE_GOT_IP);
}

typedef struct input_state_t {
    int mouse_x;
    int mouse_y;
    uint8_t last_mouse_x;
    uint8_t last_mouse_y;
} InputState;

#define MOUSE_DEADZONE 20
#define MOUSE_MAX_SPEED 8
#define MOUSE_SPEED_MODIFIER (127 / MOUSE_MAX_SPEED)
#define MIN(x, y) ((x) < (y) ? (x) : (y))
#define MAX(x, y) ((x) > (y) ? (x) : (y))
#define CLAMP(v, l, u) (MAX((l), MIN((v), (u))))

int mouse_get_delta(uint8_t axis) {
    int delta_val = (int)axis - 127;
    int distance_without_deadzone =
        delta_val + ((delta_val < -MOUSE_DEADZONE) ? 1 : -1) * MOUSE_DEADZONE;

    if (delta_val > -MOUSE_DEADZONE && delta_val < MOUSE_DEADZONE) {
        distance_without_deadzone = 0;
    }

    return distance_without_deadzone / MOUSE_SPEED_MODIFIER;
}

int process_controls(mu_Context *ctx, InputState *s) {
    SceCtrlLatch latch;
    int ret = sceCtrlReadLatch(&latch);
    if (ret < 0) goto exit;

    SceCtrlData data;
    ret = sceCtrlReadBufferPositive(&data, 1);
    if (ret < 0) goto exit;

    int dx = mouse_get_delta(data.Lx);
    int dy = mouse_get_delta(data.Ly);
    if (!(data.Buttons & PSP_CTRL_TRIANGLE)) {
        s->mouse_x += dx;
        s->mouse_y += dy;
        s->mouse_x = CLAMP(s->mouse_x, 0, PSP_SCR_WIDTH);
        s->mouse_y = CLAMP(s->mouse_y, 0, PSP_SCR_HEIGHT);
    } else {
        mu_input_scroll(ctx, dx, dy);
    }

    if (s->mouse_x != s->last_mouse_x || s->mouse_y != s->last_mouse_y) {
        mu_input_mousemove(ctx, s->mouse_x, s->mouse_y);
    }

    s->last_mouse_x = s->mouse_x;
    s->last_mouse_y = s->mouse_y;

    if (latch.uiBreak & PSP_CTRL_CROSS) {
        mu_input_mouseup(ctx, s->mouse_x, s->mouse_y, MU_MOUSE_LEFT);
    }

    if (latch.uiMake & PSP_CTRL_CROSS) {
        mu_input_mousedown(ctx, s->mouse_x, s->mouse_y, MU_MOUSE_LEFT);
    }

exit:
    return ret;
}

int user_main(SceSize args, void *argp) {
    int ret = 0;

    if ((ret = sceNetInit(128 * 1024, 42, 4 * 1024, 42, 4 * 1024)) < 0) {
        printf("sceNetInit() failed: 0x%08x\n", ret);
        goto net_failed;
    }

    if ((ret = sceNetInetInit()) < 0) {
        printf("sceNetInetInit() failed: 0x%08x\n", ret);
        goto inet_failed;
    }

    if ((ret = sceNetApctlInit(0x8000, 48)) < 0) {
        printf("sceNetApctlInit() failed: 0x%08x\n", ret);
        goto apctl_failed;
    }

    psp_DisplayNetDialog();
apctl_failed:
    sceNetInetTerm();
inet_failed:
    sceNetTerm();
net_failed:
    sceUtilityUnloadNetModule(PSP_NET_MODULE_COMMON);
    sceUtilityUnloadNetModule(PSP_NET_MODULE_INET);

    return 0;
}

typedef enum game_state_t {
    GS_EXITED = 0,
    GS_RUNNING,
} GameState;

intraFont *fnt;

void iF_draw_text(float xpos, float ypos, const char *msg, uint32_t color) {
    if (!msg) return;
    intraFontSetStyle(fnt, 0.6f, color, 0, 0.0f, 0);
    intraFontPrint(fnt, xpos, ypos, msg);
}

static int text_width(mu_Font fnt, const char *text, int len) {
    if (len == -1) {
        len = strlen(text);
    }
    return intraFontMeasureTextEx(fnt, text, len);
}

static int text_height(mu_Font font) { return fnt->size; }

void draw_icon(uint8_t *icon, int x, int y, uint8_t w, uint8_t h, mu_Color c) {
    g2dBeginPoints();
    for (int i = 0; i < w; i++) {
        for (int j = 0; j < h; j++) {
            int b = icon[i + w * j];
            add_point(x + i + w / 4, y + j + h / 4,
                      G2D_RGBA((c.r * b) / 0xff, (c.g * b) / 0xff,
                               (c.b * b) / 0xff, (c.a * b) / 0xff));
        }
    }
    g2dEnd();
}

void mainloop(GameState *state, InputState *s, mu_Context *ctx) {
    mu_Command *cmd = NULL;
    g2dClear(G2D_HEX(0x007cdfff));
    mu_begin(ctx);
    process_controls(ctx, s);
    mu_demo(ctx);
    mu_end(ctx);

    while (mu_next_command(ctx, &cmd)) {
        switch (cmd->type) {
        case MU_COMMAND_TEXT:
            handle_mu_text(cmd);
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
    g2dFlip(G2D_VSYNC);
}

int main() {
    g2dInit();
    intraFontInit();
    load_latin_font(&fnt, 0);
    printf("loaded font %s\n", fnt->filename);

    sceCtrlSetSamplingCycle(0);
    sceCtrlSetSamplingMode(PSP_CTRL_MODE_ANALOG);

    callbacks_setup();

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
    s->mouse_x = PSP_SCR_WIDTH / 2;
    s->mouse_y = PSP_SCR_HEIGHT / 2;
    s->last_mouse_x = 0;
    s->last_mouse_y = 0;
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