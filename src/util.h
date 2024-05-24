#ifndef __UTIL_H
#define __UTIL_H

#include "glib2d.h"
#include "intraFont.h"
#include "microui.h"
#include <stdint.h>

#define FONT_LOCATION "flash0:/font/ltn%d.pgf"

#define true 1
#define false 0
#define SCREEN_WIDTH 480
#define SCREEN_HEIGHT 272
#define MIN(x, y) ((x) < (y) ? (x) : (y))
#define MAX(x, y) ((x) > (y) ? (x) : (y))
#define CLAMP(v, l, u) (MAX((l), MIN((v), (u))))
#define CH_TO_STR(x) ((char[2]){x, '\0'})

typedef enum game_state_t {
    GS_EXITED = 0,
    GS_RUNNING,
} GameState;

int clamp(int val, int min, int max);
void fill_rect(int x, int y, int w, int h, g2dColor color);
void stroke_rect(int x, int y, int w, int h, g2dColor color);
void fill_circle(int x, int y, int r, g2dColor color);
void stroke_circle(int cx, int cy, int r, g2dColor color);
void add_point(int x, int y, g2dColor color);
_Bool circle_rect_collision(int cx, int cy, int r, int rx, int ry, int w,
                            int h);
void load_latin_font(intraFont **fnt, int idx);
void unload_font(intraFont **fnt);
void draw_line(int x0, int y0, int x1, int y1, g2dColor color);
void iF_draw_text(intraFont *fnt, float xpos, float ypos, const char *msg,
                  uint32_t color, float size);
void draw_icon(uint8_t *icon, int x, int y, uint8_t w, uint8_t h, mu_Color c);

#endif