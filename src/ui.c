
#include "ui.h"
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

void mu_demo(mu_Context *ctx) {
    static struct {
        const char *label;
        int idx;
    } colors[] = {{"text:", MU_COLOR_TEXT},
                  {"border:", MU_COLOR_BORDER},
                  {"windowbg:", MU_COLOR_WINDOWBG},
                  {"titlebg:", MU_COLOR_TITLEBG},
                  {"titletext:", MU_COLOR_TITLETEXT},
                  {"panelbg:", MU_COLOR_PANELBG},
                  {"button:", MU_COLOR_BUTTON},
                  {"buttonhover:", MU_COLOR_BUTTONHOVER},
                  {"buttonfocus:", MU_COLOR_BUTTONFOCUS},
                  {"base:", MU_COLOR_BASE},
                  {"basehover:", MU_COLOR_BASEHOVER},
                  {"basefocus:", MU_COLOR_BASEFOCUS},
                  {"scrollbase:", MU_COLOR_SCROLLBASE},
                  {"scrollthumb:", MU_COLOR_SCROLLTHUMB},
                  {NULL}};

    if (mu_begin_window(ctx, "Style Editor", mu_rect(0, 0, 480, 272))) {
        int sw = mu_get_current_container(ctx)->body.w * 0.14;
        mu_layout_row(ctx, 6, (int[]){80, sw, sw, sw, sw, -1}, 0);
        for (int i = 0; colors[i].label; i++) {
            mu_label(ctx, colors[i].label);
            uint8_slider(ctx, &ctx->style->colors[i].r, 0, 255);
            uint8_slider(ctx, &ctx->style->colors[i].g, 0, 255);
            uint8_slider(ctx, &ctx->style->colors[i].b, 0, 255);
            uint8_slider(ctx, &ctx->style->colors[i].a, 0, 255);
            mu_draw_rect(ctx, mu_layout_next(ctx), ctx->style->colors[i]);
        }
        mu_end_window(ctx);
    }
}