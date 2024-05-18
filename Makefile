TARGET = NostrPSP
BUILD_PRX = 1

OBJS = main.o glib2d.o callbacks.o main.o util.o gutil.o
LIBS = -lintrafont_psp -ljpeg -lpng -lz -lpspgum -lpspgu -lpsprtc -lm -lpspvram

CFLAGS = -O2 -G0 -Wall
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

EXTRA_TARGETS = EBOOT.PBP
PSP_EBOOT_TITLE = NostrStation Portable
PSP_EBOOT_ICON = icon144x80.png

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak

run: clean all
	PPSSPPSDL ./EBOOT.PBP