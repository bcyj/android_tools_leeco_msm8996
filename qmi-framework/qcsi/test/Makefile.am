AM_CFLAGS = -Wundef \
        -Wstrict-prototypes \
        -Wno-trigraphs \
        -g -O0 \
        -fno-inline \
        -fno-short-enums \
        -fpic

AM_CPPFLAGS = \
        -I../../inc \
        -I../inc \
        -I../../common/inc

h_sources =

requiredlibs = ../src/libqmi_csi.la \
        ../../common/src/libqmi_common_so.la \
        ../../encdec/libqmi_encdec.la

bin_PROGRAMS = qmi_ping_svc
qmi_ping_svc_SOURCES = qmi_ping_api_v01.c qmi_ping_svc.c qmi_ping_svc_ipc_router_main.c
qmi_ping_svc_LDADD = $(requiredlibs)
qmi_ping_svc_CC = @CC@
