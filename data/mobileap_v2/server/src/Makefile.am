AM_CFLAGS = -Wundef \
        -Wstrict-prototypes \
        -Wno-trigraphs \
        -g -O0 \
        -fno-inline \
        -fno-short-enums \
        -fpic


AM_CPPFLAGS = -D__packed__= \
        -DIMAGE_APPS_PROC \
        -DFEATURE_Q_NO_SELF_QPTR \
        -DAEEINTERFACE_CPLUSPLUS \
        -DFEATURE_SMD \
        -DFEATURE_DIAG_MP_MASTER_APPS \
        -DFEATURE_DS_LINUX_ANDROID \
        -DFEATURE_DATA_PS \
        -DFEATURE_DATA_LINUX_LE \
        -DFEATURE_DSS_LINUX \
        -DFEATURE_LINUX \
        -DFEATURE_MULTIPROCESSOR \
        -DFEATURE_DSUTILS_OFFTARGET \
        -DFEATURE_NO_REX \
        -DFEATURE_XMLLIB \
        -I$(top_srcdir)/dsutils/inc \
        -I../inc \
        -I../../ipc \
        -I$(top_srcdir)/netmgr/inc \
        -I$(top_srcdir)/dsi_netctrl/inc \
        -I$(top_srcdir)/dsi_netctrl/src \
        $(QMIFRAMEWORK_CFLAGS) \
        $(QMI_CFLAGS) \
        $(DIAG_CFLAGS) \
        $(DSUTILS_CFLAGS) \
        $(XMLLIB_CFLAGS) \
        $(CFLAGS)


libqcmap_cm_c_sources = qcmap_cm.c qcmap_cm_xml.c

requiredlibs = -lpthread -lrt \
        ../../../netmgr/src/libnetmgr.la \
        ../../../dsi_netctrl/src/libdsi_netctrl.la \
        ../../../qdi/src/libqdi.la \
        ../../ipc/libqcmapipc.la

library_includedir = $(pkgincludedir)

#libqcmap_cm_la_CC = @CC@
libqcmap_cm_la_SOURCES = $(libqcmap_cm_c_sources)
if USE_GLIB
libqcmap_cm_la_CFLAGS  = -Dstrlcpy=strncpy -Dstrlcat=strncat -DUSE_GLIB $(AM_CFLAGS) @GLIB_CFLAGS@
libqcmap_cm_la_LDFLAGS  = $(DIAG_LIBS) $(DSUTILS_LIBS) $(XMLLIB_LIBS) -lpthread -lqmi_csi -lqmi_cci @GLIB_LIBS@ -shared -version-info 1:0:0
libqcmap_cm_la_CPPFLAGS = -Dstrlcpy=strncpy -Dstrlcat=strncat -DUSE_GLIB $(AM_CFLAGS) $(AM_CPPFLAGS) @GLIB_CFLAGS@
else
libqcmap_cm_la_CFLAGS = -Dstrlcpy=strncpy -Dstrlcat=strncat $(AM_CFLAGS) $(DIAG_CFLAGS) -fPIC
libqcmap_cm_la_LDFLAGS = -lpthread $(DIAG_LIBS) $(DSUTILS_LIBS) $(XMLLIB_LIBS) -lqmi_csi -lqmi_cci -shared -version-info 1:0:0
libqcmap_cm_la_CPPFLAGS = -Dstrlcpy=strncpy -Dstrlcat=strncat $(AM_CPPFLAGS)
endif
libqcmap_cm_la_LIBADD = $(requiredlibs)

#Create and Install libraries
lib_LTLIBRARIES = libqcmap_cm.la

#####################QCMAP_STA_Interface##################
QCMAP_StaInterface_sources = qcmap_sta_interface.c

#####################QCMAP_ConnectionManager##################

QCMAP_ConnectionManager_sources = QCMAP_ConnectionManager.cpp ds_qmi_qcmap_msgr.cpp qcmap_netlink.cpp

QCMAP_ConnectionManager_h_sources = QCMAP_ConnectionManager.h qcmap_netlink.h

bin_PROGRAMS = QCMAP_ConnectionManager QCMAP_StaInterface

etcdir = ${sysconfdir}
etc_SCRIPTS = mobileap_firewall.xml mobileap_cfg.xml mobileap_cfg.xsd

init_ddir = ${sysconfdir}/init.d
init_d_SCRIPTS = start_QCMAP_ConnectionManager_le

QCMAP_ConnectionManager_CC = @CC@
QCMAP_ConnectionManager_SOURCES = $(QCMAP_ConnectionManager_sources) $(QCMAP_ConnectionManager_h_sources)
if USE_GLIB
QCMAP_ConnectionManager_CFLAGS = -Dstrlcpy=strncpy -Dstrlcat=strncat -DUSE_GLIB $(AM_CFLAGS) @GLIB_CFLAGS@
QCMAP_ConnectionManager_LDFLAGS = -lpthread $(DSUTILS_LIBS) $(XMLLIB_LIBS) @GLIB_LIBS@
else
QCMAP_ConnectionManager_CFLAGS = -Dstrlcpy=strncpy -Dstrlcat=strncat $(AM_CFLAGS) -fPIC
QCMAP_ConnectionManager_LDFLAGS = -lpthread $(DSUTILS_LIBS) $(XMLLIB_LIBS)
endif
QCMAP_ConnectionManager_CPPFLAGS = -Dstrlcpy=strncpy -Dstrlcat=strncat $(AM_CPPFLAGS)
QCMAP_ConnectionManager_LDADD = libqcmap_cm.la $(requiredlibs)

QCMAP_StaInterface_CC = @CC@
QCMAP_StaInterface_SOURCES = $(QCMAP_StaInterface_sources)
if USE_GLIB
QCMAP_StaInterface_CFLAGS = -Dstrlcpy=strncpy -Dstrlcat=strncat -DUSE_GLIB $(AM_CFLAGS) @GLIB_CFLAGS@
QCMAP_StaInterface_LDFLAGS = -lpthread $(DSUTILS_LIBS) $(XMLLIB_LIBS) @GLIB_LIBS@ $(AM_CPPFLAGS)
else
QCMAP_StaInterface_CFLAGS = -Dstrlcpy=strncpy -Dstrlcat=strncat $(AM_CFLAGS) -fPIC
QCMAP_StaInterface_LDFLAGS = -lpthread $(DSUTILS_LIBS) $(XMLLIB_LIBS)
endif
QCMAP_StaInterface_LDADD = $(requiredlibs)
