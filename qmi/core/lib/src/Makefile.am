ACLOCAL_AMFLAGS = -I m4

AM_CFLAGS = -Wundef \
	    -Wstrict-prototypes \
	    -Wno-trigraphs

AM_CPPFLAGS = -D__packed__= \
	      -DFEATURE_LE_DIAG \
	      -I../inc \
	      -I../../../inc \
	      -I../../../platform

h_sources =
requiredlibs =
library_includedir =
library_include_HEADERS =

c_sources = qmi_idl_lib.c \
        qmi_idl_accessor.c

lib_LTLIBRARIES = libqmiidl.la
libqmiidl_la_CC = @CC@
libqmiidl_la_SOURCES = $(c_sources) $(h_sources)
libqmiidl_la_CFLAGS = $(AM_CFLAGS) -fPIC
libqmiidl_la_CPPFLAGS = $(AM_CPPFLAGS) $(DATA_INCLUDES)
libqmiidl_la_LDFLAGS = -shared -version-info 1:0:0
libqmiidl_la_LIBADD = $(requiredlibs)
