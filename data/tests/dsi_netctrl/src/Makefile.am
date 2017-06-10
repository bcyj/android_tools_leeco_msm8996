AM_CFLAGS = -Wundef \
        -Wno-trigraphs \
        -g -O0 \
        -fno-inline \
        -fno-short-enums \
        -fpic \
        -Dstrlcpy=g_strlcpy \
        -Dstrlcat=g_strlcat

#refer to Androik.mk for staging header files
AM_CPPFLAGS = -D__packed__= \
        -DIMAGE_APPS_PROC \
        -DFEATURE_Q_NO_SELF_QPTR \
        -DAEEINTERFACE_CPLUSPLUS \
        -DFEATURE_SMD \
        -DFEATURE_DIAG_MP_MASTER_APPS \
        -DFEATURE_DATA_LINUX_LE \
        -DFEATURE_MULTIPROCESSOR \
        -DFEATURE_DSUTILS_OFFTARGET \
        -DFEATURE_NO_REX \
        -I../inc \
        -I$(top_srcdir)/netmgr/inc \
        -I$(top_srcdir)/dsi_netctrl/inc \
        $(QMI_CFLAGS) \
        $(DIAG_CFLAGS) \
        $(DSUTILS_CFLAGS) \
        -I$(WORKSPACE)/common/inc \
        -Dstrlcpy=g_strlcpy \
        -Dstrlcat=g_strlcat

requiredlibs = -lpthread \
        ../../../netmgr/src/libnetmgr.la \
        ../../../dsi_netctrl/src/libdsi_netctrl.la \
        ../../../qdi/src/libqdi.la

library_includedir = $(pkgincludedir)

testsdir = $(prefix)/tests

dsi_netctrl_test_CC = @CC@
dsi_netctrl_test_SOURCES = dsi_netctrl_test2.c
if USE_GLIB
dsi_netctrl_test_CFLAGS = -DUSE_GLIB $(AM_CFLAGS) @GLIB_CFLAGS@
dsi_netctrl_test_LDFLAGS = $(DIAG_LIBS) @GLIB_LIBS@
else
dsi_netctrl_test_CFLAGS = $(AM_CFLAGS) $(DIAG_CFLAGS) -fPIC
dsi_netctrl_test_LDFLAGS = $(DIAG_LIBS)
endif
dsi_netctrl_test_CPPFLAGS = $(AM_CPPFLAGS)
dsi_netctrl_test_LDADD = $(requiredlibs) $(DSUTILS_LIBS) $(DIAG_LIBS)

tests_PROGRAMS = dsi_netctrl_test
