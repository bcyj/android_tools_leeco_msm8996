# sources and intermediate files are separated
# The inclusion of qmi/core/lib/inc is a hack
# qmi/core and data/qmi have a circular depedency which
# needs to be resolved

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
        -DFEATURE_DSM_DUP_ITEMS \
        -I../inc \
        -I../src \
        -I../platform \
        -I../core/lib/inc \
        $(CONFIGDB_CFLAGS) \
        $(DSUTILS_CFLAGS)

h_sources = ../core/lib/inc/qmi_idl_lib.h \
        ../core/lib/inc/qmi_idl_lib_internal.h

requiredlibs = -lpthread -lconfigdb -ldsutils
library_includedir = $(pkgincludedir)
library_include_HEADERS = $(h_sources)

c_sources  = ../platform/qmi_platform.c \
      ../platform/linux_qmi_qmux_if_server.c \
      ../platform/qmi_platform_config.c \
      ../platform/qmi_platform_qmux_io.c \
      ../src/qmi_qmux.c \
      ../src/qmi_util.c

bin_PROGRAMS = qmuxd
qmuxd_CC = @CC@
qmuxd_SOURCES = $(c_sources) $(h_sources)

qmuxd_CFLAGS = -DUSE_GLIB -Dstrlcpy=g_strlcpy $(AM_CFLAGS) $(DIAG_CFLAGS) @GLIB_CFLAGS@
qmuxd_LDFLAGS = $(DIAG_LIBS) -shared @GLIB_LIBS@

qmuxd_CPPFLAGS =$(AM_CPPFLAGS)
qmuxd_LDADD = $(requiredlibs)

