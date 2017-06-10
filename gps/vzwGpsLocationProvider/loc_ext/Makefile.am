AM_CFLAGS = \
     $(LOCAPI_CFLAGS) \
     $(QMIF_CFLAGS)

libloc_ext_la_SOURCES = loc_ext.cpp

if USE_GLIB
libloc_ext_la_CFLAGS = -DUSE_GLIB $(AM_CFLAGS) @GLIB_CFLAGS@
libloc_ext_la_LDFLAGS = -lstdc++ -lpthread @GLIB_LIBS@ -shared -version-info 1:0:0
libloc_ext_la_CPPFLAGS = -DUSE_GLIB $(AM_CFLAGS) $(AM_CPPFLAGS) @GLIB_CFLAGS@
else
libloc_ext_la_CFLAGS = $(AM_CFLAGS)
libloc_ext_la_LDFLAGS = -lpthread -shared -version-info 1:0:0
libloc_ext_la_CPPFLAGS = $(AM_CFLAGS) $(AM_CPPFLAGS)
endif

library_includedir = $(includedir)/loc-api-app/vzwGpsLocationProvider/loc_ext

library_include_HEADERS = loc_vzw.h

libloc_ext_la_LIBADD = -lcutils $(LOCAPI_LIBS) $(QMIF_LIBS)

#Create and Install libraries
lib_LTLIBRARIES = libloc_ext.la

