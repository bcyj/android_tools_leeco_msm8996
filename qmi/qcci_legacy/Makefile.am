VPATH += ../platform/

ACLOCAL_AMFLAGS = -I m4

AM_CFLAGS = -Wundef \
        -Wstrict-prototypes \
        -Wno-trigraphs

AM_CPPFLAGS = -D__packed__= \
        -DIMAGE_APPS_PROC \
        -DFEATURE_Q_SINGLE_LINK \
        -DFEATURE_Q_NO_SELF_QPTR \
        -DFEATURE_LINUX \
        -DFEATURE_NATIVELINUX \
        -I../core/lib/inc/ \
        -I../inc \
        -I../proxy \
        -I../platform \
        -I../src

h_sources = ../src/qmi_client.h

requiredlibs = -lpthread \
        ../core/lib/src/libqmiidl.la \
        ../src/libqmi.la \
        ../qmi_client_qmux/libqmi_client_qmux.la

library_includedir = $(pkgincludedir)
library_include_HEADERS = $(h_sources)

c_sources  = ../src/qmi_client.c

lib_LTLIBRARIES = libqcci_legacy.la
libqcci_legacy_la_CC = @CC@
libqcci_legacy_la_SOURCES = $(c_sources) $(h_sources)
libqcci_legacy_la_CFLAGS = $(AM_CFLAGS) $(DIAG_CFLAGS) $(CONFIGDB_CFLAGS) -fPIC
libqcci_legacy_la_CPPFLAGS =$(AM_CPPFLAGS)
libqcci_legacy_la_LDFLAGS = $(DIAG_LIBS) $(CONFIGDB_LIBS) -shared -version-info 1:0:0
libqcci_legacy_la_LIBADD = $(requiredlibs)
