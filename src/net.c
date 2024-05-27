#ifndef __NET_H
#define __NET_H

#include "net.h"
#include "../wic/include/wic.h"
#include "tlse/tls_root_ca.h"
#include "tlse/tlse.h"
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>

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
    fprintf(stderr, "Inside net_thread\n");

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

int tls_socket(SSL **clientssl, char *hostname, int port) {
    int sockfd, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    int ret;
    char msg[] = "GET %s HTTP/1.1\r\nHost: %s:%i\r\nConnection: close\r\n\r\n";
    char msg_buffer[0xFF];
    char buffer[0xFFF];
    *clientssl = SSL_CTX_new(SSLv3_client_method());

    if (!clientssl) {
        fprintf(stderr, "Error initializing client context\n");
        return -1;
    }

    tls_load_root_certificates(*clientssl, ROOT_CA_DEF, ROOT_CA_DEF_LEN);

    sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    errno = 0;
    if (sockfd < 0) {
        fprintf(stderr, "ERROR opening socket\n");
        perror("socket");
        return -2;
    }
    server = gethostbyname(hostname);
    if (server == NULL) {
        fprintf(stderr, "ERROR, no such host\n");
        return -3;
    }
    memset((char *)&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    memcpy((char *)&serv_addr.sin_addr.s_addr, (char *)server->h_addr,
           server->h_length);
    serv_addr.sin_port = htons(port);
    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        fprintf(stderr, "ERROR connecting to");
        return -4;
    }

    // starting from here is identical with libssl
    SSL_set_fd(*clientssl, sockfd);

    // set sni
    tls_sni_set(*clientssl, hostname);

    if ((ret = SSL_connect(*clientssl)) != 1) {
        fprintf(stderr, "Handshake Error %i\n", ret);
        return -5;
    }
    // SSL usage sample
    /*     ret = SSL_write(clientssl, msg_buffer, strlen(msg_buffer));
        if (ret < 0) {
            fprintf(stderr, "SSL write error %i\n", ret);
            return -6;
        }
        while ((ret = SSL_read(clientssl, buffer, sizeof(buffer))) > 0) {
            fwrite(buffer, ret, 1, stdout);
        }
        if (ret < 0) fprintf(stderr, "SSL read error %i\n", ret); */

    // SSL shutdown
    /*     SSL_shutdown(clientssl);
        close(sockfd);
        SSL_CTX_free(clientssl); */
    return 0;
}

#endif