AM_CFLAGS = -Wundef \
        -Wstrict-prototypes \
        -Wno-trigraphs \
        -g -O0 \
        -fno-inline \
        -fno-short-enums \
        -fpic \
        -Dstrlcpy=g_strlcpy \
        -Dstrlcat=g_strlcat

AM_CPPFLAGS = -D__packed__= \
        -DIMAGE_APPS_PROC \
        -DFEATURE_Q_NO_SELF_QPTR \
        -DFEATURE_DSUTILS_OFFTARGET \
        -DDSI_NETCTRL_OFFTARGET \
        -DFEATURE_DS_LINUX_NO_RPC \
        -DFEATURE_DATA_LINUX_LE \
        -DFEATURE_DATA_SOFTAP_V6 \
        -DFEATURE_LINUX \
        -DFEATURE_DS_SVLTE1 \
        -DFEATURE_NO_REX \
        -DFEATURE_DSI_MULTIMODEM_ROUTELOOKUP \
        $(QMI_CFLAGS) \
        $(DIAG_CFLAGS) \
        -I../inc \
        -I../../qdi/inc \
        -I../../netmgr/inc \
        -I../../dsutils/inc \
        -I../../configdb/inc \
        -Dstrlcpy=g_strlcpy \
        -Dstrlcat=g_strlcat

libdsi_netctrl_c_sources = dsi_netctrl.c \
        dsi_netctrl_init.c \
        dsi_netctrli.c \
        dsi_netctrl_mni_cb.c \
        dsi_netctrl_mni.c \
        dsi_netctrl_multimodem.c \
        dsi_netctrl_netmgr.c \
        dsi_netctrl_platform.c \
        dsi_netctrl_cb_thrd.c \
        dsi_netctrl_qos.c \
        dsi_netctrl_embms.c \
        dsi_config.c

requiredlibs = -lpthread

library_includedir = $(pkgincludedir)

lib_LTLIBRARIES = libdsi_netctrl.la
libdsi_netctrl_la_CC = @CC@
libdsi_netctrl_la_SOURCES = $(libdsi_netctrl_c_sources)
if USE_GLIB
libdsi_netctrl_la_CPPFLAGS = -DUSE_GLIB $(AM_CPPFLAGS)
libdsi_netctrl_la_CFLAGS  = -DUSE_GLIB $(AM_CFLAGS) @GLIB_CFLAGS@
libdsi_netctrl_la_LDFLAGS  = $(DSUTILS_LIBS)  -lpthread -lqmi @GLIB_LIBS@
else
libdsi_netctrl_la_CFLAGS = $(AM_CFLAGS) -fPIC
libdsi_netctrl_la_LDFLAGS = -lpthread -lqmi $(DSUTILS_LIBS)
endif
libdsi_netctrl_la_CPPFLAGS = $(AM_CPPFLAGS)
libdsi_netctrl_la_LIBADD = $(requiredlibs) ../../qdi/src/libqdi.la ../../netmgr/src/libnetmgr.la
