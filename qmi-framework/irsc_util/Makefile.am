AM_CFLAGS = -Wundef \
        -Wstrict-prototypes \
        -Wno-trigraphs \
        -g -O0 \
        -fno-inline \
        -fno-short-enums \
        -fpic \
        -I../inc

c_sources = irsc_util.c

bin_PROGRAMS = irsc_util

irsc_util_SOURCES = $(c_sources)
irsc_util_CFLAGS = $(AM_CFLAGS)

if USE_GLIB
irsc_util_CFLAGS += -DUSE_GLIB @GLIB_CFLAGS@
irsc_util_LDFLAGS = @GLIB_LIBS@
endif
