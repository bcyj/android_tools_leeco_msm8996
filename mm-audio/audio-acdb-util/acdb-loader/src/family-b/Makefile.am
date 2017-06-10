AM_CFLAGS = -Wundef \
        -Wstrict-prototypes \
        -Wno-trigraphs \
        -g -O0 \
        -fno-inline \
        -fno-short-enums \
        -fpic

AM_CPPFLAGS = -I../../inc \
              -I../../inc/8974 \
              $(ACDBMAPPER_CFLAGS) \
              $(ACDBRTAC_CFLAGS) \
              $(AUDCAL_CFLAGS) \
              -I.

h_sources = ../../inc/8974/acdb-loader.h \
	    ../../inc/acdb-anc-taiko.h \
            ../../inc/acdb-anc-general.h

c_sources = acdb-loader.c

library_includedir = $(pkgincludedir)
library_include_HEADERS = $(h_sources)

lib_LTLIBRARIES = libacdbloader.la
libacdbloader_la_CC = @CC@
libacdbloader_la_SOURCES = $(c_sources) $(h_sources)
libacdbloader_la_CFLAGS = $(AM_CFLAGS) -DUSE_GLIB @GLIB_CFLAGS@ -D WCD9320_ANC_BIN_PATH=\"/data/misc/audio/wcd9320_anc.bin\" -D MBHC_BIN_PATH=\"/data/misc/audio/mbhc.bin\" -D MAD_BIN_PATH=\"/data/misc/audio/wcd9320_mad_audio.bin\" -D ETC_ROOT_PATH=\"/etc/\" -D ACDB_BIN_PATH=\"/etc/acdbdata/\" -D DEFAULT_BOARD=\"MTP\"
libacdbloader_la_CPPFLAGS = $(AM_CPPFLAGS) -DUSE_GLIB @GLIB_CFLAGS@
libacdbloader_la_LDFLAGS = $(ACDBMAPPER_LIBS) $(ACDBRTAC_LIBS) $(AUDCAL_LIBS) @GLIB_LIBS@ -shared -version-info 1:0:0
