AM_CFLAGS = -Wundef \
        -Wstrict-prototypes \
        -Wno-trigraphs \
        -g -O0 \
        -fno-inline \
        -fno-short-enums \
        -fpic


AM_CPPFLAGS = -D__packed__= \
        -I$(top_srcdir)/dsutils/inc \
        $(DIAG_CFLAGS) \
        -I$(WORKSPACE)/common/inc

requiredlibs = -lpthread -lrt

library_includedir = $(pkgincludedir)

ipacmdiag_sources = ipacmdiag_main.c

bin_PROGRAMS = ipacmdiag
ipatest_CC = @CC@
ipacmdiag_SOURCES = $(ipacmdiag_sources)
if USE_GLIB
ipacmdiag_CFLAGS  = -Dstrlcpy=strncpy -Dstrlcat=strncat -DUSE_GLIB $(AM_CFLAGS) @GLIB_CFLAGS@
ipacmdiag_LDFLAGS  = $(DIAG_LIBS) -lpthread @GLIB_LIBS@ -shared -version-info 1:0:0
ipacmdiag_CPPFLAGS = -Dstrlcpy=strncpy -Dstrlcat=strncat -DUSE_GLIB $(AM_CFLAGS) $(AM_CPPFLAGS) @GLIB_CFLAGS@
else
ipacmdiag_CFLAGS = -Dstrlcpy=strncpy -Dstrlcat=strncat $(AM_CFLAGS) $(DIAG_CFLAGS) -fPIC
ipacmdiag_LDFLAGS = -lpthread $(DIAG_LIBS) -shared -version-info 1:0:0
ipacmdiag_CPPFLAGS = -Dstrlcpy=strncpy -Dstrlcat=strncat $(AM_CPPFLAGS)
endif
ipacmdiag_CPPFLAGS = $(AM_CPPFLAGS)
ipacmdiag_LDADD = $(requiredlibs)

init_ddir = ${sysconfdir}/init.d
init_d_SCRIPTS = start_ipacmdiag_le