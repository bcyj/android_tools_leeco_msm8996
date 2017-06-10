M_CFLAGS = -Wundef \
        -Wstrict-prototypes \
        -Wno-trigraphs \
        -g -O0 \
        -fno-inline \
        -fno-short-enums \
        -fpic

#refer to Androik.mk for staging header files
AM_CPPFLAGS = -D__packed__= \
        -DIMAGE_APPS_PROC \
        -DFEATURE_Q_NO_SELF_QPTR \
        -DFEATURE_DSUTILS_OFFTARGET \
        -DQDP_OFFTARGET \
        -DFEATURE_DS_LINUX_NO_RPC \
        -DFEATURE_DATA_LINUX_LE \
        -DFEATURE_DATA_SOFTAP_V6 \
        -DFEATURE_LINUX \
        -DFEATURE_DS_SVLTE1 \
        -DFEATURE_NO_REX \
        $(QMI_CFLAGS) \
        $(DSUTILS_CFLAGS) \
        $(DIAG_CFLAGS) \
        -I${WORKSPACE}/kernel/include \
        -I../inc \
        -I$(topdir)/dsutils/inc \
        -I../../dsi_netctrl/inc

INCPATHS  += -I../../../../../../system/core/include

libqdp_c_sources = qdp.c \
        qdp_platform.c

requiredlibs = -lpthread

library_includedir = $(pkgincludedir)

lib_LTLIBRARIES = libqdp.la
libqdp_la_CC = @CC@
libqdp_la_SOURCES = $(libqdp_c_sources)
if USE_GLIB
libqdp_la_CFLAGS  = -DUSE_GLIB $(AM_CFLAGS) @GLIB_CFLAGS@
libqdp_la_LDFLAGS  = $(DSUTILS_LIBS) -lpthread -lqmi @GLIB_LIBS@
else
libqdp_la_CFLAGS = $(AM_CFLAGS) -fPIC
libqdp_la_LDFLAGS = -lpthread -lqmi $(DSUTILS_LIBS)
endif
libqdp_la_CPPFLAGS = $(AM_CPPFLAGS)
libqdp_la_LIBADD = $(requiredlibs)

