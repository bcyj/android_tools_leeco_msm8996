AM_CFLAGS = -Wundef \
        -Wstrict-prototypes \
        -Wno-trigraphs \
        -g -O0 \
        -fno-inline \
        -fno-short-enums \
        -fpic

AM_CPPFLAGS = -D__packed__= \
        -DFEATURE_XMLLIB \
        -I../inc

h_sources = ../inc/xmllib_tok.h \
        ../inc/xmllib_common.h \
        ../inc/xmllib_parser.h

c_sources = xmllib_decl.c \
        xmllibi_decl_utf8.c \
        xmllibi_utf8_util.c \
        xmllib_generator.c \
        xmllibi_tok_ascii.c \
        xmllib_parser.c \
        xmllibi_decl_ascii.c \
        xmllibi_tok_utf8.c \
        xmllib_tok.c

requiredlibs = -lpthread

library_includedir = $(pkgincludedir)
library_include_HEADERS = $(h_sources)

lib_LTLIBRARIES = libxml.la
libxml_la_CC = @CC@
libxml_la_SOURCES = $(c_sources)
if USE_GLIB
libxml_la_CFLAGS  = -DUSE_GLIB $(AM_CFLAGS) $(DIAG_CFLAGS) @GLIB_CFLAGS@
libxml_la_LDFLAGS  = $(DIAG_LIBS) -lpthread @GLIB_LIBS@
else
libxml_la_CFLAGS = $(AM_CFLAGS) $(DIAG_CFLAGS) -fPIC
libxml_la_LDFLAGS = -lpthread $(DIAG_LIBS)
endif
libxml_la_CPPFLAGS = $(AM_CPPFLAGS)
libxml_la_LIBADD = $(requiredlibs)
