
AM_CFLAGS = -Wundef \
        -Wstrict-prototypes \
        -Wno-trigraphs \
        -g -O0 \
        -fno-inline \
        -fno-short-enums \
        -fpic

AM_CPPFLAGS = -D__packed__= \
        -DIMAGE_APPS_PROC \
        -DFEATURE_DATA_LINUX_LE \
        -DFEATURE_XMLLIB \
        -DFEATURE_DSS_LINUX \
        -DFEATURE_LINUX \
        -DFEATURE_DIAG_MP_MASTER_APPS \
        -DFEATURE_MULTIPROCESSOR \
        -DFEATURE_DSUTILS_OFFTARGET \
        -I../inc \
        -I$(top_srcdir)/dss_new/src/platform/inc \
        -I$(top_srcdir)/dsi_netctrl/inc \
        -I$(top_srcdir)/dsutils/inc \
        -I$(top_srcdir)/mobileap_v2/ipc \
        $(QMIFRAMEWORK_CFLAGS) \
        $(QMI_CFLAGS) \
        $(DIAG_CFLAGS) \
        -I../../netmgr/inc \
        $(DSUTILS_CFLAGS) \
        $(XMLLIB_CFLAGS) \
        $(CFLAGS)


requiredlibs = ../../dsi_netctrl/src/libdsi_netctrl.la \
               ../../mobileap_v2/ipc/libqcmapipc.la

#####################QMI_IP##################

qmi_ip_c_sources = qmi_ip.c qmi_ip_multicast_listener.c qmi_ip_msg_hdlr.c qmi_ip_netlink.c qmi_ip_cmdq.c

qmi_ip_h_sources = qmi_ip.h qmi_ip_netlink.h qmi_ip_cmdq.h

bin_PROGRAMS = qmi_ip
qmi_ip_CC = @CC@
qmi_ip_SOURCES = $(qmi_ip_c_sources) $(qmi_ip_h_sources)
if USE_GLIB
qmi_ip_CFLAGS = -Dstrlcpy=strncpy -Dstrlcat=strncat -DUSE_GLIB $(AM_CFLAGS) $(DIAG_CFLAGS) @GLIB_CFLAGS@
qmi_ip_LDFLAGS = $(XMLLIB_LIBS) $(DSUTILS_LIBS) -lpthread -lssl -lcrypto -lqmi_csi -lqmi_cci @GLIB_LIBS@ -shared -version-info 1:0:0
else
qmi_ip_CFLAGS = -Dstrlcpy=strncpy -Dstrlcat=strncat $(AM_CFLAGS) $(DIAG_CFLAGS) -fPIC
qmi_ip_LDFLAGS = $(XMLLIB_LIBS) $(DSUTILS_LIBS) -lpthread -lssl -lcrypto -lqmi_csi -lqmi_cci -shared -version-info 1:0:0
endif
qmi_ip_CPPFLAGS = -Dstrlcpy=strncpy -Dstrlcat=strncat $(AM_CPPFLAGS)
qmi_ip_LDADD = $(requiredlibs)

etcdir = ${sysconfdir}

init_ddir = ${sysconfdir}/init.d
init_d_SCRIPTS = start_stop_qmi_ip


