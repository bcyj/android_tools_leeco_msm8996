AM_CFLAGS = -Wundef \
        -Wstrict-prototypes \
        -Wno-trigraphs \
        -g -O0 \
        -fno-inline \
        -fno-short-enums \
        -fpic

AM_CPPFLAGS = -I../inc \
              -I../../acph/inc \
              -I../../actp/inc \
              -I../../acdb_hlos/inc \
              -I../../acph_online/inc \
              -I../../audtp/inc \
              -I../../audtp/src \
              -I../../acph/src \
              -I. \
              $(ACDBMAPPER_CFLAGS) \
              $(DIAG_CFLAGS) \
              $(AUDIOALSA_CFLAGS) \
              -I$(WORKSPACE)/common/inc

h_sources = ../inc/acdb.h \
			../inc/acdb_begin_pack.h \
			../inc/acdb_end_pack.h \
			../inc/acdb_init_utility.h \
			../../acph/inc/acph.h \
			../../acph/inc/acph_update_for_rtc.h \
			../../acdb_hlos/inc/acdb_os_includes.h \
			../../acph_online/inc/acph_online.h \
			../../actp/inc/actp.h \
			../../audtp/inc/audtp.h
			
			

c_sources = acdb.c \
            acdb_command.c \
            acdb_data_mgr.c \
            acdb_file_mgr.c \
            acdb_init.c \
            acdb_override.c \
            acdb_parser.c \
            acdb_linked_list.c \
            acdb_translation.c \
            acdb_utility.c \            
            ../../acph/src/acph.c \
            ../../acdb_hlos/src/acdb_init_utility.c \
            ../../acph_online/src/acph_online.c \
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
