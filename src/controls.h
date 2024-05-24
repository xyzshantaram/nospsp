#ifndef __CONTROLS_H
#define __CONTROLS_H

#include "intraFont.h"
#include "microui.h"
#include <pspctrl.h>
#include <stdint.h>

#define MOUSE_DEADZONE 20
#define MOUSE_MAX_SPEED 8
#define MOUSE_SPEED_MODIFIER (127 / MOUSE_MAX_SPEED)

typedef struct input_state_t {
    int mouse_x;
    int mouse_y;
    uint8_t last_mouse_x;
    uint8_t last_mouse_y;
    int is_keyboard_active;
} InputState;

extern const char keymaps[3][3][4];

int process_controls(mu_Context *ctx, InputState *s, intraFont *fnt);
void draw_cursor(InputState *s);

#endif