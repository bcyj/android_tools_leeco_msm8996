AM_CFLAGS = -Wundef \
        -Wstrict-prototypes \
        -Wno-trigraphs \
        -g -O0 \
        -fno-inline \
        -fno-short-enums \
        -fpic \
        -Dstrlcpy=g_strlcpy \
        -Dstrlcat=g_strlcat \
        $(DIAG_CFLAGS) \
        $(XMLLIB_CFLAGS) \
        $(DSUTILS_CFLAGS)

#refer to Androik.mk for staging header files
AM_CPPFLAGS = -D__packed__= \
        -DFEATURE_XMLLIB \
        -I../inc \
        -Dstrlcpy=g_strlcpy \
        -Dstrlcat=g_strlcat

c_sources = configdb.c \
        configdb_xml.c \
        configdb_util.c

requiredlibs = -lpthread

h_sources = ../inc/configdb.h
library_includedir = $(pkgincludedir)
library_include_HEADERS = $(h_sources)

lib_LTLIBRARIES = libconfigdb.la
libconfigdb_la_CC = @CC@
libconfigdb_la_SOURCES = $(c_sources)
libconfigdb_la_CPPFLAGS = $(AM_CPPFLAGS)
libconfigdb_la_LIBADD = $(requiredlibs) -lxml
if USE_GLIB
libconfigdb_la_CFLAGS  = -DUSE_GLIB $(AM_CFLAGS) @GLIB_CFLAGS@
libconfigdb_la_LDFLAGS = -lpthread @GLIB_LIBS@ $(XMLLIB_LIBS) $(DSUTILS_LIBS) $(DIAG_LIBS)
else
libconfigdb_la_CFLAGS = $(AM_CFLAGS)
libconfigdb_la_LDFLAGS = -lpthread $(XMLLIB_LIBS) $(DSUTILS_LIBS) $(DIAG_LIBS)
endif

