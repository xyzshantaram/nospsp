#ifndef __UTIL_H__
#define __UTIL_H__

#include "glib2d.h"
#include "intraFont.h"
#include <stdint.h>

#define FONT_LOCATION "flash0:/font/ltn%d.pgf"

#define true 1
#define false 0
#define SCREEN_WIDTH 480
#define SCREEN_HEIGHT 272

int clamp(int val, int min, int max);
void fill_rect(int x, int y, int w, int h, g2dColor color);
void stroke_rect(int x, int y, int w, int h, g2dColor color);
void fill_circle(int x, int y, int r, g2dColor color);
void stroke_circle(int cx, int cy, int r, g2dColor color);
_Bool circle_rect_collision(int cx, int cy, int r, int rx, int ry, int w,
                            int h);
void load_latin_fonts(intraFont *(*arr)[16]);
void unload_fonts(intraFont *(*arr)[16]);
#endif