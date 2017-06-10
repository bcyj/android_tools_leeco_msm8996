AM_CFLAGS = -Wundef \
        -Wstrict-prototypes \
        -Wno-trigraphs \
        -g -O0 \
        -fno-inline \
        -fno-short-enums \
        -fpic

AM_CPPFLAGS = \
        -I../../inc \
        -I../../common/inc

h_sources = \
        ../../inc/qmi_sap_target_ext.h \
        ../../inc/qmi_sap.h \
        ../../inc/service_access_proxy_v01.h

requiredlibs = -lpthread \
        ../../qcci/src/libqmi_cci.la

library_includedir = $(pkgincludedir)
library_include_HEADERS = $(h_sources)

c_sources = qmi_sap_client.c service_access_proxy_v01.c

lib_LTLIBRARIES = libqmi_sap.la
libqmi_sap_la_CC = @CC@
libqmi_sap_la_SOURCES = $(c_sources) $(h_sources)
libqmi_sap_la_CFLAGS = $(AM_CFLAGS)
libqmi_sap_la_CPPFLAGS = $(AM_CPPFLAGS)
libqmi_sap_la_LDFLAGS = -shared -version-info 1:0:0
libqmi_sap_la_LIBADD = $(requiredlibs)
