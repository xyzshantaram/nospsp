#include "icons/check.c"
#include "icons/close.c"
#include "icons/collapsed.c"
#include "icons/expanded.c"
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
