AM_CFLAGS = -Wundef \
        -Wstrict-prototypes \
        -Wno-trigraphs \
        -g -O0 \
        -fno-inline \
        -fno-short-enums \
        -fpic \
        -Dstrlcpy=g_strlcpy \
        -Dstrlcat=g_strlcat \
        $(DIAG_CFLAGS) \
        $(QMI_CFLAGS) \
        $(DSUTILS_CFLAGS) \
        $(CONFIGDB_CFLAGS) \
        $(QMIFRAMEWORK_CFLAGS) \
        $(DATAOSS_CFLAGS)

#refer to Androik.mk for staging header files
AM_CPPFLAGS = -D__packed__= \
        -DIMAGE_APPS_PROC \
        -DFEATURE_Q_NO_SELF_QPTR \
        -DFEATURE_DSUTILS_OFFTARGET \
        -DFEATURE_CONFIGDB_OFFTARGET \
        -DFEATURE_DS_LINUX_NO_RPC \
        -DFEATURE_DATA_LINUX_LE \
        -DFEATURE_DATA_SOFTAP_V6 \
        -DFEATURE_LINUX \
        -DFEATURE_DS_SVLTE1 \
        -DFEATURE_NO_REX \
        -DFEATURE_DATA_PREFIX_DELEGATION \
        -I../inc \
        -I$(topdir)/dsutils/inc \
        -I$(topdir)/configdb/inc \
        -Dstrlcpy=g_strlcpy \
        -Dstrlcat=g_strlcat \
        $(QMIFRAMEWORK_CFLAGS) \
        $(DATAOSS_CFLAGS)

libnetmgr_c_sources = netmgr_client.c \
        netmgr_netlink.c \
        netmgr_util.c

requiredlibs = -lpthread

library_includedir = $(pkgincludedir)

lib_LTLIBRARIES = libnetmgr.la
libnetmgr_la_CC = @CC@
libnetmgr_la_SOURCES = $(libnetmgr_c_sources)
if USE_GLIB
libnetmgr_la_CFLAGS  = -DUSE_GLIB $(AM_CFLAGS) @GLIB_CFLAGS@
libnetmgr_la_LDFLAGS  = $(CONFIGDB_LIBS) $(DSUTILS_LIBS) $(QMIFRAMEWORK_LIBS) $(DATAOSS_LIBS) -lpthread @GLIB_LIBS@
else
libnetmgr_la_CFLAGS = $(AM_CFLAGS) -fPIC
libnetmgr_la_LDFLAGS = -lpthread $(CONFIGDB_LIBS) $(DSUTILS_LIBS) $(QMIFRAMEWORK_LIBS) $(DATAOSS_LIBS)
endif
libnetmgr_la_CPPFLAGS = $(AM_CPPFLAGS)
libnetmgr_la_LIBADD = $(requiredlibs)

#####################netmgrd##################

netmgrd_sources = netmgr.c \
        netmgr_exec.c \
        netmgr_kif.c \
        netmgr_main.c \
        netmgr_platform.c \
        netmgr_qmi.c \
        netmgr_sm.c \
        netmgr_test.c \
        netmgr_sm_int.c \
        netmgr_tc.c \
        netmgr_qmi_dpm.c \
        netmgr_qmi_wda.c \
        netmgr_qmi_dfs.c \
        netmgr_rmnet.c \
        netmgr_config.c \
        netmgr_cmdq.c

netmgrd_h_sources = ../inc/netmgr.h

bin_PROGRAMS = netmgrd
netmgrd_CC = @CC@
netmgrd_SOURCES = $(netmgrd_sources) $(netmgrd_h_sources)
if USE_GLIB
netmgrd_CFLAGS = -DUSE_GLIB $(AM_CFLAGS) @GLIB_CFLAGS@
netmgrd_LDFLAGS = $(DIAG_LIBS) $(CONFIGDB_LIBS) $(DSUTILS_LIBS) -lqmi -lpthread @GLIB_LIBS@
else
netmgrd_CFLAGS = $(AM_CFLAGS) -fPIC
netmgrd_LDFLAGS = $(DIAG_LIBS) $(CONFIGDB_LIBS) $(DSUTILS_LIBS) -lqmi
endif
netmgrd_CPPFLAGS = $(AM_CPPFLAGS)
netmgrd_LDADD = $(requiredlibs) libnetmgr.la
