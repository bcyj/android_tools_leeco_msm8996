ACLOCAL_AMFLAGS = -I m4

AM_CFLAGS = -Wundef \
            -Wstrict-prototypes \
            -Wno-trigraphs

AM_CPPFLAGS = -D__packed__= \
              -DFEATURE_LE_DIAG \
              $(QMI_CFLAGS)

libqcmap_ipc_h_sources =
requiredlibs =
library_includedir = $(pkgincludedir)
library_include_HEADERS = \
                qualcomm_mobile_access_point_msgr_v01.h

library_includedir = $(pkgincludedir)

libqcmap_ipc_c_sources = \
                qualcomm_mobile_access_point_msgr_v01.c

lib_LTLIBRARIES = libqcmapipc.la
libqcmapipc_la_CC = @CC@
libqcmapipc_la_SOURCES = $(libqcmap_ipc_c_sources) $(libqcmap_ipc_h_sources)
libqcmapipc_la_CFLAGS = $(AM_CFLAGS) $(DIAG_CFLAGS) -fPIC
libqcmapipc_la_CPPFLAGS = $(AM_CPPFLAGS)
libqcmapipc_la_LDFLAGS = -shared -version-info 1:0:0
libqcmapipc_la_LIBADD = $(requiredlibs)
