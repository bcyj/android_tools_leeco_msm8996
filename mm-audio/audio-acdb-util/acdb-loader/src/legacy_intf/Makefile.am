AM_CFLAGS = -Wundef \
        -Wstrict-prototypes \
        -Wno-trigraphs \
        -g -O0 \
        -fno-inline \
        -fno-short-enums \
        -fpic

AM_CPPFLAGS = -I../../inc \
              -I../../inc/8960 \
              $(ACDBMAPPER_CFLAGS) \
              $(AUDCAL_CFLAGS) \
              -I.

h_sources = ../../inc/8960/acdb-loader.h \
            ../../inc/acdb-anc-tabla.h \
            ../../inc/8960/acdb-loader-def.h \
            ../../inc/acdb-anc-general.h

c_sources = acdb-loader.c

library_includedir = $(pkgincludedir)
library_include_HEADERS = $(h_sources)

lib_LTLIBRARIES = libacdbloader.la
libacdbloader_la_CC = @CC@
libacdbloader_la_SOURCES = $(c_sources) $(h_sources)
libacdbloader_la_CFLAGS = $(AM_CFLAGS) -DUSE_GLIB @GLIB_CFLAGS@ -D WCD9310_ANC_BIN_PATH=\"/data/misc/audio/wcd9310_anc.bin\" -D MBHC_BIN_PATH=\"/data/misc/audio/mbhc.bin\"
libacdbloader_la_CPPFLAGS = $(AM_CPPFLAGS) -DUSE_GLIB @GLIB_CFLAGS@
libacdbloader_la_LDFLAGS = $(ACDBMAPPER_LIBS) $(AUDCAL_LIBS) @GLIB_LIBS@ -shared -version-info 1:0:0
