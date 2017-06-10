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
        -DFEATURE_DSS_LINUX \
        -DFEATURE_LINUX \
        -DFEATURE_MULTIPROCESSOR \
        -DFEATURE_DSUTILS_OFFTARGET \
        -DFEATURE_NO_REX \
        -DFEATURE_XMLLIB \
        -DFEATURE_MDM_LE \
        -I$(WORKSPACE)/qmi/services \
        -I$(top_srcdir)/dsutils/inc \
        -I$(top_srcdir)/netmgr/inc \
        -I$(top_srcdir)/mobileap_v2/ipc \
        -I$(WORKSPACE)/qmi-framework/inc \
        -I$(WORKSPACE)/qmi-framework/qcci/inc \
        $(QMI_CFLAGS) \
        $(DIAG_CFLAGS) \
        $(DSUTILS_CFLAGS) \
        $(XMLLIB_CFLAGS) \
        $(QMIFRAMEWORK_CFLAGS) \
        $(DATAOSS_CFLAGS) \
        -I$(WORKSPACE)/common/inc

requiredlibs = -lpthread -lrt \
         ../../netmgr/src/libnetmgr.la \
         ../../mobileap_v2/ipc/libqcmapipc.la

library_includedir = $(pkgincludedir)

qti_sources = qti_main.c \
              qti_netlink.c \
              qti_qcmap.c \
              qti_cmdq.c \
              qti_rmnet_peripheral.c \
              qti_rmnet_modem.c \
              qti_rmnet_dpm.c \
              qti_rmnet_data.c \
              qti_rmnet_qmi.c

qti_h_sources = qti.h \
                qti_cmdq.h


init_ddir = ${sysconfdir}/init.d
init_d_SCRIPTS = start_qti_le start_stop_qti_ppp_le

bin_PROGRAMS = qti qti_ppp
qti_CC = @CC@
qti_SOURCES = $(qti_sources) $(qti_h_sources)
if USE_GLIB
qti_CFLAGS  = -DUSE_GLIB $(AM_CFLAGS) @GLIB_CFLAGS@
qti_LDFLAGS  = $(DIAG_LIBS) $(XMLLIB_LIBS) $(DATAOSS_LIBS) -lpthread -lqmi_cci -lqmiservices @GLIB_LIBS@ -shared -version-info 1:0:0
qti_CPPFLAGS = -DUSE_GLIB $(AM_CFLAGS) $(AM_CPPFLAGS) @GLIB_CFLAGS@
else
qti_CFLAGS = $(AM_CFLAGS) $(DIAG_CFLAGS) -fPIC
qti_LDFLAGS = -lpthread $(DIAG_LIBS) $(XMLLIB_LIBS) $(DATAOSS_LIBS) -lqmi_cci -lqmiservices -shared -version-info 1:0:0
qti_CPPFLAGS = $(AM_CPPFLAGS)
endif
qti_CPPFLAGS = $(AM_CPPFLAGS)
qti_LDADD = $(requiredlibs)

qti_ppp_sources = qti_ppp.c \
        qti_ppp_netlink.c \
        qti_ppp_qcmap.c \
        qti_ppp_usb_tty.c \
        qti_ppp_smd_tty.c \
        qti_ppp_wds_client.c

qti_ppp_h_sources = qti_ppp.h

qti_ppp_CC = @CC@
qti_ppp_SOURCES = $(qti_ppp_sources) $(qti_ppp_h_sources)
if USE_GLIB
qti_ppp_CFLAGS  = -DUSE_GLIB $(AM_CFLAGS) @GLIB_CFLAGS@
qti_ppp_LDFLAGS  = $(DIAG_LIBS) $(XMLLIB_LIBS) -lpthread -lqmi_cci -lqmiservices @GLIB_LIBS@ -shared -version-info 1:0:0
qti_ppp_CPPFLAGS = -DUSE_GLIB $(AM_CFLAGS) $(AM_CPPFLAGS) @GLIB_CFLAGS@
else
qti_ppp_CFLAGS = $(AM_CFLAGS) $(DIAG_CFLAGS) -fPIC
qti_ppp_LDFLAGS = -lpthread $(DIAG_LIBS) $(XMLLIB_LIBS) -lqmi_cci -lqmiservices -shared -version-info 1:0:0
qti_ppp_CPPFLAGS = $(AM_CPPFLAGS)
endif
qti_ppp_CPPFLAGS = $(AM_CPPFLAGS)
qti_ppp_LDADD = $(requiredlibs)
