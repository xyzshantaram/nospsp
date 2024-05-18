#include "gutil.h"
#include <pspdisplay.h>
#include <pspgu.h>

static unsigned int __attribute__((aligned(16))) list[262144];

// Get Memory Size
static unsigned int get_memory_size(unsigned int width, unsigned int height,
                                    unsigned int psm) {
    switch (psm) {
    case GU_PSM_T4:
        return (width * height) >> 1;

    case GU_PSM_T8:
        return width * height;

    case GU_PSM_5650:
    case GU_PSM_5551:
    case GU_PSM_4444:
    case GU_PSM_T16:
        return 2 * width * height;

    case GU_PSM_8888:
    case GU_PSM_T32:
        return 4 * width * height;

    default:
        return 0;
    }
}

// Vram Buffer Request
void *get_static_vram_buffer(unsigned int width, unsigned int height,
                             unsigned int psm) {
    static unsigned int staticOffset = 0;
    unsigned int memSize = get_memory_size(width, height, psm);
    void *result = (void *)staticOffset;
    staticOffset += memSize;

    return result;
}

// Vram Texture Request
void *get_static_vram_texture(unsigned int width, unsigned int height,
                              unsigned int psm) {
    void *result = get_static_vram_buffer(width, height, psm);
    return (void *)(((unsigned int)result) +
                    ((unsigned int)sceGeEdramGetAddr()));
}

// Initialize Graphics
void init_graphics() {
    void *fbp0 =
        get_static_vram_buffer(PSP_BUF_WIDTH, PSP_SCR_HEIGHT, GU_PSM_8888);
    void *fbp1 =
        get_static_vram_buffer(PSP_BUF_WIDTH, PSP_SCR_HEIGHT, GU_PSM_8888);
    void *zbp =
        get_static_vram_buffer(PSP_BUF_WIDTH, PSP_SCR_HEIGHT, GU_PSM_4444);

    sceGuInit();

    sceGuStart(GU_DIRECT, list);
    sceGuDrawBuffer(GU_PSM_8888, fbp0, PSP_BUF_WIDTH);
    sceGuDispBuffer(PSP_SCR_WIDTH, PSP_SCR_HEIGHT, fbp1, PSP_BUF_WIDTH);
    sceGuDepthBuffer(zbp, PSP_BUF_WIDTH);
    sceGuOffset(2048 - (PSP_SCR_WIDTH / 2), 2048 - (PSP_SCR_HEIGHT / 2));
    sceGuViewport(2048, 2048, PSP_SCR_WIDTH, PSP_SCR_HEIGHT);
    sceGuDepthRange(65535, 0);
    sceGuScissor(0, 0, PSP_SCR_WIDTH, PSP_SCR_HEIGHT);
    sceGuEnable(GU_SCISSOR_TEST);
    sceGuDepthFunc(GU_GEQUAL);
    sceGuEnable(GU_DEPTH_TEST);
    sceGuFrontFace(GU_CW);
    sceGuShadeModel(GU_SMOOTH);
    sceGuEnable(GU_CULL_FACE);
    sceGuEnable(GU_TEXTURE_2D);
    sceGuEnable(GU_CLIP_PLANES);
    sceGuFinish();
    sceGuSync(0, 0);

    sceDisplayWaitVblankStart();
    sceGuDisplay(GU_TRUE);
}

// Start Frame
void frame_start() { sceGuStart(GU_DIRECT, list); }

// End Frame
void frame_end() {
    sceGuFinish();
    sceGuSync(0, 0);
    sceDisplayWaitVblankStart();
    sceGuSwapBuffers();
}

// End Graphics
void terminate_graphics() { sceGuTerm(); }