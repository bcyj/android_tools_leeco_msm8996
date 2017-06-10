AM_CFLAGS = -Wundef \
        -Wstrict-prototypes \
        -Wno-trigraphs \
        -g -O0 \
        -fno-inline \
        -fno-short-enums \
        -fpic

AM_CPPFLAGS = \
        -I../inc

h_sources =
requiredlibs =

c_sources = qmi_idl_accessor.c \
        qmi_idl_lib.c

lib_LTLIBRARIES = libqmi_encdec.la
libqmi_encdec_la_CC = @CC@
libqmi_encdec_la_SOURCES = $(c_sources) $(h_sources)
libqmi_encdec_la_CFLAGS = $(AM_CFLAGS)
libqmi_encdec_la_CPPFLAGS = $(AM_CPPFLAGS)
libqmi_encdec_la_LDFLAGS = -shared -version-info 1:0:0
libqmi_encdec_la_LIBADD = $(requiredlibs)
