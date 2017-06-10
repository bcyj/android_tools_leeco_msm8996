AM_CFLAGS = -Wundef \
        -Wstrict-prototypes \
        -Wno-trigraphs \
        -g -O0 \
        -fno-inline \
        -fno-short-enums \
        -fpic

AM_CPPFLAGS = -I../inc \
              -I.

h_sources = ../inc/control.h

c_sources = hw.c

library_includedir = $(pkgincludedir)
library_include_HEADERS = $(h_sources)

lib_LTLIBRARIES = libaudioalsa.la
libaudioalsa_la_CC = @CC@
libaudioalsa_la_SOURCES = $(c_sources) $(h_sources)
libaudioalsa_la_CFLAGS = $(AM_CFLAGS) -DUSE_GLIB @GLIB_CFLAGS@ -DQDSP6V2
libaudioalsa_la_CPPFLAGS = $(AM_CPPFLAGS) -DUSE_GLIB @GLIB_CFLAGS@
libaudioalsa_la_LDFLAGS = -$(LIBACDBMAPPER_LIBS) $(DIAG_LIBS) @GLIB_LIBS@ -shared -version-info 1:0:0
