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

h_sources =

requiredlibs = -lpthread -ldsutils

library_includedir = $(pkgincludedir)
library_include_HEADERS = $(h_sources)

c_sources  = ../platform/linux_qmi_qmux_if_client.c \
        ../platform/qmi_platform.c \
        ../src/qmi_qmux_if.c \
        ../src/qmi_util.c

lib_LTLIBRARIES = libqmi_client_qmux.la
libqmi_client_qmux_la_CC = @CC@
libqmi_client_qmux_la_SOURCES = $(c_sources) $(h_sources)
libqmi_client_qmux_la_CFLAGS = $(AM_CFLAGS) $(DIAG_CFLAGS) $(DSUTILS_CFLAGS) $(CONFIGDB_CFLAGS) -fPIC
libqmi_client_qmux_la_CPPFLAGS =$(AM_CPPFLAGS)
libqmi_client_qmux_la_LDFLAGS = $(DIAG_LIBS) $(DSUTILS_CFLAGS) $(CONFIGDB_LIBS) -shared -version-info 1:0:0
libqmi_client_qmux_la_LIBADD = $(requiredlibs)
