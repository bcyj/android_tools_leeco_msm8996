AM_CFLAGS = -Wundef \
        -Wstrict-prototypes \
        -Wno-trigraphs \
        -g -O0 \
        -fno-inline \
        -fno-short-enums \
        -fpic


AM_CPPFLAGS = -D__packed__= \
        -DIMAGE_APPS_PROC \
        -DFEATURE_Q_NO_SELF_QPTR \
        -DAEEINTERFACE_CPLUSPLUS \
        -DFEATURE_SMD \
        -DFEATURE_DIAG_MP_MASTER_APPS \
        -DFEATURE_DS_LINUX_ANDROID \
        -DFEATURE_DSS_LINUX \
        -DFEATURE_LINUX \
        -DFEATURE_MULTIPROCESSOR \
        -DFEATURE_DSUTILS_OFFTARGET \
        -DFEATURE_NO_REX \
        -DFEATURE_XMLLIB \
        -I$(top_srcdir)/dsutils/inc \
        -I$(top_srcdir)/netmgr/inc \
        -I../inc \
        -I../../ipc \
        $(QMIFRAMEWORK_CFLAGS) \
        $(QMI_CFLAGS) \
        $(DIAG_CFLAGS) \
        $(DSUTILS_CFLAGS) \
        $(XMLLIB_CFLAGS) \
        $(CFLAGS)

library_includedir = $(pkgincludedir)
library_include_HEADERS = ./../inc/QCMAP_Client.h \
                          ./../inc/qcmap_client_util.h \
						  ./../inc/qcmap_firewall_util.h

libqcmap_client_h_sources = QCMAP_Client.h qcmap_client_util.h
libqcmap_client_c_sources = QCMAP_Client.cpp

requiredlibs = -lpthread -lrt \
               ../../ipc/libqcmapipc.la

library_includedir = $(pkgincludedir)

#libqcmap_client_la_CC = @CC@
libqcmap_client_la_SOURCES = $(libqcmap_client_c_sources) $(libqcmap_client_h_sources)
if USE_GLIB
libqcmap_client_la_CFLAGS  = -Dstrlcpy=strncpy -Dstrlcat=strncat -DUSE_GLIB $(AM_CFLAGS) @GLIB_CFLAGS@
libqcmap_client_la_LDFLAGS  = $(DIAG_LIBS) $(DSUTILS_LIBS) $(XMLLIB_LIBS) -lpthread -lqmi_cci  @GLIB_LIBS@ -shared -version-info 1:0:0
libqcmap_client_la_CPPFLAGS = -Dstrlcpy=strncpy -Dstrlcat=strncat -DUSE_GLIB $(AM_CFLAGS) $(AM_CPPFLAGS) @GLIB_CFLAGS@
else
libqcmap_client_la_CFLAGS = -Dstrlcpy=strncpy -Dstrlcat=strncat $(AM_CFLAGS) $(DIAG_CFLAGS) -fPIC
libqcmap_client_la_LDFLAGS = -lpthread $(DIAG_LIBS) $(DSUTILS_LIBS) $(XMLLIB_LIBS) -lqmi_cci -shared -version-info 1:0:0
libqcmap_client_la_CPPFLAGS = -Dstrlcpy=strncpy -Dstrlcat=strncat $(AM_CPPFLAGS)
endif
libqcmap_client_la_LIBADD = $(requiredlibs)

#Create and Install libraries
lib_LTLIBRARIES = libqcmap_client.la

#####################QCMAP_Client##################

QCMAP_CLI_sources = qcmap_cli_main.cpp 

#QCMAP_CLI_h_sources =  

bin_PROGRAMS = QCMAP_CLI
QCMAP_CLI_CC = @CC@
QCMAP_CLI_SOURCES = $(QCMAP_CLI_sources)
if USE_GLIB
QCMAP_CLI_CFLAGS = -Dstrlcpy=strncpy -Dstrlcat=strncat -DUSE_GLIB $(AM_CFLAGS) @GLIB_CFLAGS@
QCMAP_CLI_LDFLAGS = -lpthread $(DSUTILS_LIBS) $(XMLLIB_LIBS) @GLIB_LIBS@
else
QCMAP_CLI_CFLAGS = -Dstrlcpy=strncpy -Dstrlcat=strncat $(AM_CFLAGS) -fPIC
QCMAP_CLI_LDFLAGS = -lpthread $(DSUTILS_LIBS) $(XMLLIB_LIBS)
endif
QCMAP_CLI_CPPFLAGS = -Dstrlcpy=strncpy -Dstrlcat=strncat $(AM_CPPFLAGS)
QCMAP_CLI_LDADD = libqcmap_client.la $(requiredlibs)
