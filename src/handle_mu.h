#ifndef __HANDLE_MU_H
#define __HANDLE_MU_H

#include "icons/check.h"
#include "icons/close.h"
#include "icons/collapsed.h"
#include "icons/expanded.h"
#include "microui.h"
#include "util.h"

void handle_mu_text(intraFont *fnt, mu_Command *cmd);
void handle_mu_rect(mu_Command *cmd);
void handle_mu_icon(mu_Command *cmd);

#define SET_ICON(icon_type)                                                    \
    do {                                                                       \
        w = w_##icon_type##_ATL;                                               \
        h = h_##icon_type##_ATL;                                               \
        icon = icon_type##_ATL;                                                \
    } while (0)
#endif