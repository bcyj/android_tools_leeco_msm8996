# Makefile.am - Automake script for data
#
ACLOCAL_AMFLAGS = -I m4

dirs :

dirs = netmgr/src
dirs += qdi/src
dirs += dsi_netctrl/src
dirs += tests/dsi_netctrl/src
dirs += qdp/src
dirs += radish
dirs += ipacm-diag/src

if TARGET_IS_9615
dirs += mobileap/src
dirs += qti/src
else
dirs += mobileap_v2/ipc
dirs += mobileap_v2/server/src mobileap_v2/client/src qmi_ip/src qti_v2/src
dirs += mobileap_v2/webserver/qcmap_web_client/src mobileap_v2/webserver/cgi
endif

SUBDIRS = $(dirs)

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = data.pc
EXTRA_DIST = $(pkgconfig_DATA)
