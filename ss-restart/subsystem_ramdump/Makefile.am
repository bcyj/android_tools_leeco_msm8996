AM_CFLAGS = -Wall \
        -Wundef \
        -Wstrict-prototypes \
        -Wno-trigraphs \
	-Werror

subsystem_ramdump_SOURCES = subsystem_ramdump.c

if USE_GLIB
subsystem_ramdump_CFLAGS = $(AM_CFLAGS) -DUSE_GLIB @GLIB_CFLAGS@
subsystem_ramdump_LDFLAGS = -lpthread -lrt @GLIB_LIBS@
else
subsystem_ramdump_LDFLAGS = -lpthread -lrt
endif

bin_PROGRAMS = subsystem_ramdump
