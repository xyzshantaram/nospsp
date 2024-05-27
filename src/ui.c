
#include "ui.h"
#include <stdio.h>
#include <stdlib.h>

static int uint8_slider(mu_Context *ctx, unsigned char *value, int low,
                        int high) {
    static float tmp;
    mu_push_id(ctx, &value, sizeof(value));
    tmp = *value;
    int res = mu_slider_ex(ctx, &tmp, low, high, 0, "%.0f", MU_OPT_ALIGNCENTER);
    *value = tmp;
    mu_pop_id(ctx);
    return res;
}

void mu_demo(mu_Context *ctx, char **logbuf, int *buf_updated) {
    if (mu_begin_window(ctx, "HTTPS?", mu_rect(0, 0, 480, 272))) {
        mu_layout_row(ctx, 1, (int[]){-1}, -1);
        mu_begin_panel(ctx, "Log Output");
        mu_Container *panel = mu_get_current_container(ctx);
        mu_layout_row(ctx, 1, (int[]){-1}, -1);
        mu_text(ctx, *logbuf);
        mu_end_panel(ctx);
        if (*buf_updated) {
            panel->scroll.y = panel->content_size.y;
            *buf_updated = 0;
        }

        mu_end_window(ctx);
    }
}