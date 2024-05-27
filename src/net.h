#include "glib2d.h"
#include "tlse/tlse.h"
#include <pspnet_apctl.h>
#include <psputility.h>
#include <stdio.h>
#include <string.h>

int psp_DisplayNetDialog(void);
int net_thread(SceSize args, void *argp);
int tls_socket(SSL **clientssl, char *hostname, int port);