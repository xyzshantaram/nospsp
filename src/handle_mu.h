#ifndef __HANDLE_MU_H
#define __HANDLE_MU_H
#include "intraFont.h"
#include "microui.h"

void handle_mu_text(intraFont *fnt, mu_Command *cmd);
void handle_mu_rect(mu_Command *cmd);
void handle_mu_icon(mu_Command *cmd);
#endif