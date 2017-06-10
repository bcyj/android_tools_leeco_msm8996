AM_CFLAGS = -Wundef \
        -Wstrict-prototypes \
        -Wno-trigraphs \
        -g -O0 \
        -fno-inline \
        -fno-short-enums \
        -fpic

AM_CPPFLAGS = -I../inc \
              -I../../acdbdata/inc \
              -I../../acph/inc \
              -I../../actp/inc \
              -I../../audtp/inc \
              -I../../audtp/src \
              -I../../../8960/acdbdata/inc \
              -I../../../8960/acph/src \
              -I. \
              $(ACDBMAPPER_CFLAGS) \
              $(DIAG_CFLAGS) \
              $(AUDIOALSA_CFLAGS) \
              -I$(WORKSPACE)/common/inc

h_sources = ../inc/acdb.h \
            ../../acph/inc/acph.h \
            ../../../8960/acdbdata/inc/acdb_includes.h

c_sources = acdb.c \
            acdb_command.c \
            acdb_init.c \
            acdb_override.c \
            acdb_parser.c \
            acdb_linked_list.c \
            acdb_translation.c \
            ../la/src/acdb_init_utility.c \
            ../../../8960/acdbdata/src/acdb_default_data.c \
            ../../../8960/acph/src/acph.c \
            ../../../8960/acph/src/rtc_apps_intf.c \
            ../../../8960/acph/src/rtc_q6_intf.c \
            ../../actp/src/actp.c \
            ../../audtp/src/audtp.c

library_includedir = $(pkgincludedir)
library_include_HEADERS = $(h_sources)

lib_LTLIBRARIES = libaudcal.la
libaudcal_la_CC = @CC@
libaudcal_la_SOURCES = $(c_sources) $(h_sources)
libaudcal_la_CFLAGS = $(AM_CFLAGS) -DUSE_GLIB @GLIB_CFLAGS@
libaudcal_la_CPPFLAGS = $(AM_CPPFLAGS) -DUSE_GLIB @GLIB_CFLAGS@
libaudcal_la_LDFLAGS = $(AUDIOALSA_LIBS) $(ACDBMAPPER_LIBS) $(DIAG_LIBS) @GLIB_LIBS@ -shared -version-info 1:0:0
