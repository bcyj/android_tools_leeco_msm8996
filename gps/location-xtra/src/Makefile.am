AM_CFLAGS = \
            -I../privinc \
            -I../pubinc \
            $(LOCAPI_CFLAGS) \
            -DHAVE_LIBLOG

requiredlibs = -lrt $(LOCAPI_LIBS)

h_sources = \
            ../privinc/xtra_log_api.h \
            ../privinc/xtra_config_api.h \
            ../privinc/xtra_defines.h \
            ../privinc/xtra.h \
            ../privinc/xtra_server_api.h \
            ../privinc/xtra_linux.h \
            ../pubinc/xtra_system_interface.h


c_sources = \
            xtra_config.c \
            xtra_servers.c \
            xtra_sntp_linux.c \
            xtra_http_linux.c \
            xtra_system_interface.c


library_includedir = $(pkgincludedir)
library_include_HEADERS = $(h_sources)

libloc_xtra_la_SOURCES = $(c_sources) $(h_sources)

if USE_GLIB
libloc_xtra_la_CFLAGS = -DUSE_GLIB $(AM_CFLAGS) @GLIB_CFLAGS@
libloc_xtra_la_LDFLAGS = -lstdc++ -lpthread @GLIB_LIBS@ -shared -version-info 1:0:0
libloc_xtra_la_CPPFLAGS = -DUSE_GLIB $(AM_CFLAGS) $(AM_CPPFLAGS) @GLIB_CFLAGS@
else
libloc_xtra_la_CFLAGS = $(AM_CFLAGS)
libloc_xtra_la_LDFLAGS = -shared -version-info 1:0:0
libloc_xtra_la_CPPFLAGS = $(AM_CFLAGS) $(AM_CPPFLAGS)
endif

libloc_xtra_la_LIBADD = $(requiredlibs)

lib_LTLIBRARIES = libloc_xtra.la
