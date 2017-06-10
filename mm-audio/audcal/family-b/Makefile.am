ACLOCAL_AMFLAGS = -I m4
pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = acdbloader.pc
EXTRA_DIST = $(pkgconfig_DATA)

AM_CFLAGS = -Wundef \
        -Wstrict-prototypes \
        -Wno-trigraphs \
        -g -O0 \
        -fno-inline \
        -fno-short-enums \
        -fpic

AM_CPPFLAGS = -Iacdb/inc \
              -Iacph/inc \
              -Iacdb/src \
              $(DIAG_CFLAGS) \
              $(AUDIOALSA_CFLAGS)

h_sources = ..acdb/inc/acdb.h \
            ..acph/inc/acph_update_for_rtc.h \
            ..acdb/inc/acdb_os_includes.h

c_sources = acdb/src/acdb.c \
            acdb/src/acdb_command.c \
            acdb/src/acdb_data_mgr.c \
            acdb/src/acdb_file_mgr.c \
            acdb/src/acdb_init.c \
            acdb/src/acdb_init_utility.c \
            acdb/src/acdb_parser.c \
            acdb/src/acdb_translation.c \
            acdb/src/acdb_utility.c

library_includedir = $(pkgincludedir)
library_include_HEADERS = $(h_sources)

lib_LTLIBRARIES = libaudcal.la
libaudcal_la_CC = @CC@
libaudcal_la_SOURCES = $(c_sources) $(h_sources)
libaudcal_la_CFLAGS = $(AM_CFLAGS) -DUSE_GLIB @GLIB_CFLAGS@
libaudcal_la_CPPFLAGS = $(AM_CPPFLAGS) -DUSE_GLIB @GLIB_CFLAGS@
libaudcal_la_LDFLAGS = $(AUDIOALSA_LIBS) $(DIAG_LIBS) @GLIB_LIBS@ -shared -version-info 1:0:0
