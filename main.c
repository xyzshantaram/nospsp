#include "callbacks.h"
#include "glib2d.h"
#include "intraFont.h"
#include "util.h"
#include <pspctrl.h>
#include <pspdisplay.h>
#include <pspge.h>
#include <pspgu.h>
#include <pspkernel.h>
#include <pspnet.h>
#include <pspnet_apctl.h>
#include <pspnet_inet.h>
#include <psprtc.h>
#include <psputility.h>
#include <stdio.h>
#include <string.h>

#define TARGET_FPS 60

PSP_MODULE_INFO("NostrStation Portable", 0, 1, 1);
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER | THREAD_ATTR_VFPU);
PSP_HEAP_SIZE_KB(-16384);
PSP_HEAP_THRESHOLD_SIZE_KB(1024);
PSP_MAIN_THREAD_STACK_SIZE_KB(1024);

#define PSP_BUF_WIDTH (512)
#define PSP_SCR_WIDTH (480)
#define PSP_SCR_HEIGHT (272)

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
    SceCtrlLatch changed;
    SceCtrlData bstates;
} InputState;

int read_controls(InputState *s) {
    SceCtrlLatch latch;
    int ret = sceCtrlReadLatch(&latch);
    if (ret < 0) goto exit;
    SceCtrlData data;
    ret = sceCtrlReadBufferPositive(&data, 1);
    if (ret < 0) goto exit;

    s->changed = latch;
    s->bstates = data;
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

int main() {
    callbacks_setup();
    g2dInit();
    intraFontInit();
    intraFont *latin_fonts[16];
    load_latin_fonts(&latin_fonts);

    sceGeEdramSetSize(0x400000);
    sceUtilityLoadNetModule(PSP_NET_MODULE_COMMON);
    sceUtilityLoadNetModule(PSP_NET_MODULE_INET);

    SceUID thid = sceKernelCreateThread(
        "User Mode Thread", user_main,
        0x11,       // default priority
        512 * 1024, // stack size (256KB is regular default)
        PSP_THREAD_ATTR_USER, NULL);

    // start user thread, then wait for it to do everything else
    sceKernelStartThread(thid, 0, NULL);
    sceKernelWaitThreadEnd(thid, NULL);

    u64 last_tick;
    sceRtcGetCurrentTick(&last_tick);
    int cr = 20;
    int cx = cr / 2;
    int cy = PSP_SCR_HEIGHT / 2;
    int dx = 2;
    InputState s;

    GameState state = GS_RUNNING;

    while (state != GS_EXITED) {
        // Framelimit code
        u32 res = sceRtcGetTickResolution();
        double min_delta = (float)res / TARGET_FPS;
        u64 this_tick;
        sceRtcGetCurrentTick(&this_tick);
        double delta = this_tick - last_tick;
        if (delta < min_delta) continue;
        last_tick = this_tick;
        g2dClear(G2D_HEX(0x007cdfff));
        stroke_circle(cx, cy, cr, G2D_HEX(0x27ffffff));
        cx += dx;
        if (cx <= cr / 2 || cx >= PSP_SCR_WIDTH - cr / 2) dx *= -1;
        read_controls(&s);
        if (s.bstates.Buttons & PSP_CTRL_CIRCLE) {
            state = GS_EXITED;
        }

        g2dFlip(G2D_VSYNC);
    }
    g2dTerm();
    unload_fonts(&latin_fonts);
    intraFontShutdown();
    sceKernelExitGame();
}