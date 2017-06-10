AM_CFLAGS = -Wundef \
        -Wstrict-prototypes \
        -Wno-trigraphs \
        -g -O0 \
        -fno-inline \
        -fno-short-enums \
        -fpic

#refer to Androik.mk for staging header files
AM_CPPFLAGS = -D__packed__= \
        -DFEATURE_Q_NO_SELF_QPTR \
        -DFEATURE_DS_LINUX_NO_RPC \
        -DFEATURE_DATA_LINUX \
        -DFEATURE_DATA_LINUX_LE \
        $(DIAG_CFLAGS) \
        -I../inc

h_sources = ../inc/ds_cmdq.h \
        ../inc/ds_list.h \
        ../inc/ds_string.h \
        ../inc/ds_util.h \
        ../inc/ds_sl_list.h \
        ../inc/queue.h \
        ../inc/stm2.h \
        ../inc/stm2_os.h \
        ../inc/ds_trace.h

requiredlibs = -lpthread

library_includedir = $(pkgincludedir)
library_include_HEADERS = $(h_sources)

c_sources = ds_cmdq.c \
        ds_list.c \
        ds_util.c \
        ds_sl_list.c \
        stm2.c \
        ds_trace.c

lib_LTLIBRARIES = libdsutils.la
libdsutils_la_CC = @CC@
libdsutils_la_SOURCES = $(c_sources) $(h_sources)
if USE_GLIB
libdsutils_la_CFLAGS  = -DUSE_GLIB $(AM_CFLAGS) @GLIB_CFLAGS@
libdsutils_la_LDFLAGS  = -lpthread $(DIAG_LIBS) @GLIB_LIBS@ -shared -version-info 1:0:0
else
libdsutils_la_CFLAGS = $(AM_CFLAGS)
libdsutils_la_LDFLAGS = -lpthread $(DIAG_LIBS) -shared -version-info 1:0:0
endif
libdsutils_la_CPPFLAGS = $(AM_CPPFLAGS)
libdsutils_la_LIBADD = $(requiredlibs)
