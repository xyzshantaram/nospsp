#include "net.h"
#include <pspnet.h>
#include <pspnet_inet.h>

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

int net_thread(SceSize args, void *argp) {
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