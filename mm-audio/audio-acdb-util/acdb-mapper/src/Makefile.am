AM_CFLAGS = -Wundef \
        -Wstrict-prototypes \
        -Wno-trigraphs \
        -g -O0 \
        -fno-inline \
        -fno-short-enums \
        -fpic

AM_CPPFLAGS = -I../inc \
               $(AUDIOALSA_CFLAGS) \
              -I.

h_sources = ../inc/acdb-id-mapper.h

c_sources = acdb-id-mapper.c

library_includedir = $(pkgincludedir)
library_include_HEADERS = $(h_sources)

lib_LTLIBRARIES = libacdbmapper.la
libacdbmapper_la_CC = @CC@
libacdbmapper_la_SOURCES = $(c_sources) $(h_sources)
libacdbmapper_la_CFLAGS = $(AM_CFLAGS) -DUSE_GLIB @GLIB_CFLAGS@
libacdbmapper_la_CPPFLAGS = $(AM_CPPFLAGS) -DUSE_GLIB @GLIB_CFLAGS@
libacdbmapper_la_LDFLAGS = @GLIB_LIBS@ -shared -version-info 1:0:0
