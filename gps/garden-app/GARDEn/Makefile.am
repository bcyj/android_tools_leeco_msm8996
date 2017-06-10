AM_CFLAGS = \
    -DDEBUG \
    -MD \
    $(LOCAPI_CFLAGS) \
    $(LOCAPIAPP_CFLAGS) \
    $(LOCXTRA_CFLAGS)


garden_h_sources = \
     test_android_gps.h \
     android_runtime/AndroidRuntime.h \
     utils/Log.h \
     fake_logd_write.h

garden_c_sources = \
     test_android_gps.cpp \
     fake_android_runtime.cpp \
     fake_vzw_jni_bridage.cpp \
     fake_logd_write.c

garden_app_SOURCES = $(garden_c_sources) $(garden_h_sources)

if USE_GLIB
garden_app_CFLAGS = -DUSE_GLIB $(AM_CFLAGS) @GLIB_CFLAGS@
garden_app_LDFLAGS = -lstdc++ -lpthread @GLIB_LIBS@ -shared -version-info 1:0:0
garden_app_CPPFLAGS = -DUSE_GLIB $(AM_CFLAGS) $(AM_CPPFLAGS) @GLIB_CFLAGS@
else
garden_app_CFLAGS = $(AM_CFLAGS)
garden_app_LDFLAGS = -lpthread -shared -version-info 1:0:0
garden_app_CPPFLAGS = $(AM_CFLAGS) $(AM_CPPFLAGS)
endif

garden_app_LDADD = -lcutils $(LOCAPI_LIBS) $(LOCAPIAPP_LIBS) $(LOCXTRA_LIBS)

bin_PROGRAMS = garden_app
