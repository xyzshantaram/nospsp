#ifndef GUTIL_H
#define GUTIL_H

#define PSP_BUF_WIDTH (512)
#define PSP_SCR_WIDTH (480)
#define PSP_SCR_HEIGHT (272)

void init_graphics();
void frame_start();
void frame_end();
void terminate_graphics();

#endif