TARGET = NostrPSP
BUILD_PRX = 1

CSRCS = $(shell echo src/*.c)
OBJS = $(CSRCS:.c=.o) wic/src/http_parser.o wic/src/wic.o src/tlse/tlse.o
LIBS = -lm -ljpeg -lpng -lz -lintrafont_psp -lpspgum -lpspgu -lpsprtc -lpspvram -lpspnet -lpspnet_apctl

CFLAGS = -O2 -G0 -Wall -I wic/include
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

EXTRA_TARGETS = EBOOT.PBP
PSP_EBOOT_TITLE = NostrStation Portable
PSP_EBOOT_ICON = icon144x80.png

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak

run: clean all
	PPSSPPSDL ./EBOOT.PBP