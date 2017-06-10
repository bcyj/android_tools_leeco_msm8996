AM_CFLAGS = -Wundef \
        -Wstrict-prototypes \
        -Wno-trigraphs \
        -g -O0 \
        -fno-inline \
        -fno-short-enums \
        -fpic

AM_CPPFLAGS = -I../inc \
              -I. \
              $(QMIF_CFLAGS) \
              $(ALSAINTF_CFLAGS) \
              $(ACDBLOADER_CFLAGS)

c_sources = core_sound_driver_v01.c csd_alsa.c csd_server.c

library_includedir = $(pkgincludedir)
library_include_HEADERS = $(h_sources)

init_ddir = $(sysconfdir)/init.d
init_d_SCRIPTS = csdserver

bin_PROGRAMS = csd_server
csd_server_CC = @CC@
csd_server_SOURCES = $(c_sources) $(h_sources)
csd_server_CFLAGS = $(AM_CFLAGS) -DUSE_GLIB @GLIB_CFLAGS@
csd_server_CPPFLAGS = $(AM_CPPFLAGS) -DUSE_GLIB @GLIB_CFLAGS@
csd_server_LDFLAGS = -lpthread $(ALSAINTF_LIBS) $(ACDBLOADER_LIBS) $(QMIF_LIBS) @GLIB_LIBS@ -shared -version-info 1:0:0
