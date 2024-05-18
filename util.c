#include "util.h"
#include <stdlib.h>

int clamp(int val, int min, int max) {
    if (val < min) return min;
    if (val > max) return max;
    return val;
}

void add_point(int x, int y, g2dColor color) {
    g2dSetColor(color);
    g2dSetCoordXY(x, y);
    g2dAdd();
}

void fill_rect(int x, int y, int w, int h, g2dColor color) {
    g2dBeginRects(NULL); // No texture
    g2dSetColor(color);
    g2dSetScaleWH(w, h);
    g2dSetCoordXY(x, y);
    g2dAdd();
    g2dEnd();
}

void stroke_rect(int x, int y, int w, int h, g2dColor color) {}

void fill_circle(int cx, int cy, int r, g2dColor color) {
    g2dBeginPoints();
    for (int y = -r; y <= r; y++)
        for (int x = -r; x <= r; x++)
            if (x * x + y * y < r * r + r) add_point(cx + x, cy + y, color);
    g2dEnd();
}

void circle_points(int cx, int cy, int x, int y, int pix) {
    if (x == 0) {
        add_point(cx, cy + y, pix);
        add_point(cx, cy - y, pix);
        add_point(cx + y, cy, pix);
        add_point(cx - y, cy, pix);
    } else if (x == y) {
        add_point(cx + x, cy + y, pix);
        add_point(cx - x, cy + y, pix);
        add_point(cx + x, cy - y, pix);
        add_point(cx - x, cy - y, pix);
    } else if (x < y) {
        add_point(cx + x, cy + y, pix);
        add_point(cx - x, cy + y, pix);
        add_point(cx + x, cy - y, pix);
        add_point(cx - x, cy - y, pix);
        add_point(cx + y, cy + x, pix);
        add_point(cx - y, cy + x, pix);
        add_point(cx + y, cy - x, pix);
        add_point(cx - y, cy - x, pix);
    }
}

void stroke_circle(int cx, int cy, int r, g2dColor color) {
    g2dBeginPoints();
    int x = 0;
    int y = r;
    int p = (5 - r * 4) / 4;

    circle_points(cx, cy, x, y, color);
    while (x < y) {
        x++;
        if (p < 0) {
            p += 2 * x + 1;
        } else {
            y--;
            p += 2 * (x - y) + 1;
        }
        circle_points(cx, cy, x, y, color);
    }
    g2dEnd();
}

g2dColor rgba(uint32_t color) {
    uint64_t r = (0xff000000 & color) >> 24;
    uint64_t g = (0xff0000 & color) >> 16;
    uint64_t b = (0xff00 & color) >> 8;
    uint64_t a = (0xff & color);

    return G2D_RGBA(r, g, b, a);
}

_Bool circle_rect_collision(int cx, int cy, int r, int rx, int ry, int w,
                            int h) {
    int dx = abs(cx - rx);
    int dy = abs(cy - ry);

    if (dx > (w / 2 + r)) return false;
    if (dy > (h / 2 + r)) return false;

    if (dx <= (w / 2)) return true;
    if (dy <= (h / 2)) return true;

    int d_sq = (dx - w / 2) ^ 2 + (dy - h / 2) ^ 2;

    return (d_sq < (r ^ 2));
}