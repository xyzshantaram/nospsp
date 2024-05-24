#include "controls.h"
#include "icons/cursor.h"
#include "intraFont.h"
#include "util.h"

// clang-format off
const char keymaps[3][3][4] = {
{
    {' ', '-', '!', '?'}, 
    {'y', 'c', 'b', 'p'}, 
    {'@', '$', '(', ')'}
    },
{
    {'r', 'd', 'l', 'u'}, 
    {'e', 't', 'a', 'o'}, 
    {'m', 'w', 'f', 'g'}
    },
{
    {'k', 'v', 'j', 'x'}, 
    {'i', 'n', 's', 'h'}, 
    {'z', 'q', '.', ','}
    }
};
// clang-format on

void draw_cursor(InputState *s) {
    mu_Color c = {0xff, 0xff, 0xff, 0xff};
    draw_icon(ICON_MOUSE_ATL, s->mouse_x - w_ICON_MOUSE_ATL / 2,
              s->mouse_y - h_ICON_MOUSE_ATL / 2, w_ICON_MOUSE_ATL,
              h_ICON_MOUSE_ATL, c);
}

int mouse_get_delta(uint8_t axis) {
    int delta_val = (int)axis - 127;
    int distance_without_deadzone =
        delta_val + ((delta_val < -MOUSE_DEADZONE) ? 1 : -1) * MOUSE_DEADZONE;

    if (delta_val > -MOUSE_DEADZONE && delta_val < MOUSE_DEADZONE) {
        distance_without_deadzone = 0;
    }
    return distance_without_deadzone / MOUSE_SPEED_MODIFIER;
}

void render_keyboard(intraFont *fnt, bool shifted, uint8_t lx, uint8_t ly) {
    uint32_t bg = 0x99eeeeee;
    uint32_t black = 0xff000000;
    uint32_t gray = 0xff202020;
    uint32_t bg_brighter = 0xffffffff;
    int x = 124;
    int y = 20;
    int side = 232;
    int side_b3 = side / 3;
    int side_2b3 = 2 * side / 3;

    fill_rect(x, y, side, side, bg);
    fill_rect(x, y, side, side, bg);

    int tx = ((float)lx / 255) * side;
    int ty = ((float)ly / 255) * side;

    int qx = tx < side_b3 ? 0 : (tx > side_2b3 ? 2 : 1);
    int qy = ty < side_b3 ? 0 : (ty > side_2b3 ? 2 : 1);

    fill_rect(x + qx * side_b3, y + qy * side_b3, side / 3, side / 3,
              bg_brighter);

    draw_line(x, y + side_b3, x + side, y + side_b3, black);
    draw_line(x, y + side_2b3, x + side, y + side_2b3, black);
    draw_line(x + side_b3, y, x + side_b3, y + side, black);
    draw_line(x + side_2b3, y, x + side_2b3, y + side, black);

    int offset_half = side_b3 / 2 - 8;
    int offset_full = side_b3 - 15;
    float size = 0.8f;
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            int xpos = x + i * side_b3 + 4;
            int ypos = y + j * side_b3 + 12;
            int color = 0xff555555;
            if (qx == i && qy == j) color = BLACK;
            const char *keys = keymaps[i][j];

            iF_draw_text(fnt, xpos + offset_half, ypos + offset_full,
                         CH_TO_STR(keys[0]), color, size);
            iF_draw_text(fnt, xpos, ypos + offset_half, CH_TO_STR(keys[1]),
                         color, size);
            iF_draw_text(fnt, xpos + offset_half, ypos, CH_TO_STR(keys[2]),
                         color, size);
            iF_draw_text(fnt, xpos + offset_full, ypos + offset_half,
                         CH_TO_STR(keys[3]), color, size);
        }
    }
}

int process_controls(mu_Context *ctx, InputState *s, intraFont *fnt) {
    SceCtrlLatch latch;
    int ret = sceCtrlReadLatch(&latch);
    if (ret < 0) goto exit;

    SceCtrlData data;
    ret = sceCtrlReadBufferPositive(&data, 1);
    if (ret < 0) goto exit;

    int handled_select = 0;
    if (!handled_select && latch.uiBreak & PSP_CTRL_SELECT) {
        s->is_keyboard_active = !s->is_keyboard_active;
        handled_select = 1;
    }

    if (s->is_keyboard_active) {
        render_keyboard(fnt, false, data.Lx, data.Ly);
    } else {
        int dx = mouse_get_delta(data.Lx);
        int dy = mouse_get_delta(data.Ly);
        if (!(data.Buttons & PSP_CTRL_TRIANGLE)) {
            s->mouse_x += dx;
            s->mouse_y += dy;
            s->mouse_x = CLAMP(s->mouse_x, 0, SCREEN_WIDTH);
            s->mouse_y = CLAMP(s->mouse_y, 0, SCREEN_HEIGHT);
        } else {
            mu_input_scroll(ctx, dx, dy);
        }

        if (s->mouse_x != s->last_mouse_x || s->mouse_y != s->last_mouse_y) {
            mu_input_mousemove(ctx, s->mouse_x, s->mouse_y);
        }

        s->last_mouse_x = s->mouse_x;
        s->last_mouse_y = s->mouse_y;

        if (latch.uiBreak & PSP_CTRL_CROSS) {
            mu_input_mouseup(ctx, s->mouse_x, s->mouse_y, MU_MOUSE_LEFT);
        }

        if (latch.uiMake & PSP_CTRL_CROSS) {
            mu_input_mousedown(ctx, s->mouse_x, s->mouse_y, MU_MOUSE_LEFT);
        }
    }

exit:
    return ret;
}