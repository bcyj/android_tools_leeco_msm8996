AM_CFLAGS = -Wundef \
        -Wstrict-prototypes \
        -Wno-trigraphs \
        -g -O0 \
        -fno-inline \
        -fno-short-enums \
        -fpic

AM_CPPFLAGS = -I../inc \
               $(AUDIOALSA_CFLAGS) \
               $(AUDCAL_CFLAGS) \
              -I.

h_sources = ../inc/acdb-rtac.h

c_sources = acdb-rtac.c

library_includedir = $(pkgincludedir)
library_include_HEADERS = $(h_sources)

lib_LTLIBRARIES = libacdbrtac.la
libacdbrtac_la_CC = @CC@
libacdbrtac_la_SOURCES = $(c_sources) $(h_sources)
libacdbrtac_la_CFLAGS = $(AM_CFLAGS) -DUSE_GLIB @GLIB_CFLAGS@
libacdbrtac_la_CPPFLAGS = $(AM_CPPFLAGS) -DUSE_GLIB @GLIB_CFLAGS@
libacdbrtac_la_LDFLAGS = $(AUDCAL_LIBS) @GLIB_LIBS@ -shared -version-info 1:0:0
