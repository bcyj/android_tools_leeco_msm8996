
AM_CFLAGS = -Wundef \
        -Wstrict-prototypes \
        -Wno-trigraphs \
        -g -O0 \
        -fno-inline \
        -fno-short-enums \
        -fpic

AM_CPPFLAGS = \
        -I../../inc \
        -I../inc

h_sources = ../inc/qmi_common.h

requiredlibs = -lpthread
library_includedir = $(pkgincludedir)
library_include_HEADERS = $(h_sources)

c_sources = common_v01.c

lib_LTLIBRARIES = libqmi_common_so.la
libqmi_common_so_la_CC = @CC@
libqmi_common_so_la_SOURCES = $(c_sources) $(h_sources)
libqmi_common_so_la_CFLAGS = $(AM_CFLAGS)
libqmi_common_so_la_CPPFLAGS = $(AM_CPPFLAGS)
libqmi_common_so_la_LDFLAGS = -shared -version-info 1:0:0
libqmi_common_so_la_LIBADD = $(requiredlibs)
