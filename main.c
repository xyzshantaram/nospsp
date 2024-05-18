#include "gutil.h"
#include <pspctrl.h>
#include <pspdisplay.h>
#include <pspgu.h>
#include <pspkernel.h>
#include <pspnet.h>
#include <pspnet_apctl.h>
#include <pspnet_inet.h>
#include <psputility.h>
#include <stdio.h>
#include <string.h>

static char disp_list[0x10000] __attribute__((aligned(64)));

PSP_MODULE_INFO("NostrStation Portable", 0, 1, 1);
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER | THREAD_ATTR_VFPU);
PSP_HEAP_SIZE_KB(-1024);
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

static void drawDialogBackground(void) {
    sceGuStart(GU_DIRECT, disp_list);
    sceGuClearColor(0xFF686868);
    sceGuClearDepth(0);
    sceGuClear(GU_COLOR_BUFFER_BIT | GU_DEPTH_BUFFER_BIT);
    sceGuFinish();
    sceGuSync(0, 0);
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
        drawDialogBackground();

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
        }

        sceDisplayWaitVblankStart();
        sceGuSwapBuffers();
    } while (done != PSP_UTILITY_DIALOG_NONE);

    done = PSP_NET_APCTL_STATE_DISCONNECTED;
    if ((ret = sceNetApctlGetState(&done)) < 0) {
        printf("sceNetApctlGetState() failed: 0x%08x", ret);
        return 0;
    }

    return (done == PSP_NET_APCTL_STATE_GOT_IP);
}

typedef struct input_state_t {
    SceCtrlLatch latch;
    SceCtrlData data;
} InputState;

InputState read_controls() {
    SceCtrlLatch latch;
    sceCtrlReadLatch(&latch);
    SceCtrlData data;
    sceCtrlReadBufferPositive(&data, 1);

    InputState s = {.latch = latch, .data = data};
    return s;
}

int user_main(SceSize args, void *argp) {
    sceUtilityLoadNetModule(PSP_NET_MODULE_COMMON);
    sceUtilityLoadNetModule(PSP_NET_MODULE_INET);

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
    printf("here\n");

    while (1) {
        InputState s = read_controls();
        if (s.latch.uiBreak & PSP_CTRL_CIRCLE) {
            break;
        }
    }

apctl_failed:
    sceNetInetTerm();
inet_failed:
    sceNetTerm();
net_failed:
    sceUtilityUnloadNetModule(PSP_NET_MODULE_COMMON);
    sceUtilityUnloadNetModule(PSP_NET_MODULE_INET);

    return 0;
}

int main() {
    drawDialogBackground();
    init_graphics();

    SceUID thid = sceKernelCreateThread(
        "User Mode Thread", user_main,
        0x11,       // default priority
        512 * 1024, // stack size (256KB is regular default)
        PSP_THREAD_ATTR_USER, NULL);

    // start user thread, then wait for it to do everything else
    sceKernelStartThread(thid, 0, NULL);
    sceKernelWaitThreadEnd(thid, NULL);
    terminate_graphics();
    sceKernelExitGame();
}