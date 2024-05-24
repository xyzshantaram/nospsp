#include "handle_mu.h"
#include "icons/check.h"
#include "icons/close.h"
#include "icons/collapsed.h"
#include "icons/expanded.h"
#include "intraFont.h"
#include "util.h"

#define SET_ICON(icon_type)                                                    \
    do {                                                                       \
        w = w_##icon_type##_ATL;                                               \
        h = h_##icon_type##_ATL;                                               \
        icon = icon_type##_ATL;                                                \
    } while (0)

void handle_mu_text(intraFont *fnt, mu_Command *cmd) {
    mu_Color c = cmd->text.color;
    iF_draw_text(fnt, cmd->text.pos.x, cmd->text.pos.y + 4, cmd->text.str,
                 G2D_RGBA(c.r, c.g, c.b, c.a), 0.6f);
}

void handle_mu_rect(mu_Command *cmd) {
    mu_Color c = cmd->rect.color;
    fill_rect(cmd->rect.rect.x, cmd->rect.rect.y, cmd->rect.rect.w,
              cmd->rect.rect.h, G2D_RGBA(c.r, c.g, c.b, c.a));
}

void handle_mu_icon(mu_Command *cmd) {
    int w, h;
    const uint8_t *icon = NULL;
    switch (cmd->icon.id) {
    case MU_ICON_CHECK: {
        SET_ICON(MU_ICON_CHECK);
        break;
    }
    case MU_ICON_CLOSE: {
        SET_ICON(MU_ICON_CLOSE);
        break;
    }
    case MU_ICON_EXPANDED: {
        SET_ICON(MU_ICON_EXPANDED);
        break;
    }
    case MU_ICON_COLLAPSED:
        SET_ICON(MU_ICON_COLLAPSED);
        break;
    default:
        break;
    }

    if (icon) {
        mu_Rect rect = cmd->icon.rect;
        draw_icon(icon, rect.x, rect.y, w, h, cmd->icon.color);
    }
}
