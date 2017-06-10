AM_CFLAGS = \
    -DFEATURE_LOC_API_RPCGEN \
    -DDEBUG \
    -DFEATURE_GPS_DCM_ALWAYS_ON \
    -DUSE_QCOM_AUTO_RPC \
    -DFEATURE_RPC_CHAR_ARRAY \
    $(LOCAPI_CFLAGS)

requiredlibs = \
    $(LOCAPI_LIBS)

c_sources = \
    loc_api_test.c \
    loc_api_data.c \
    loc_api_cmd.c \
    loc_api_ini.c \
    loc_api_cb_hub.c \
    loc_api_cb_log.c \
    loc_api_cb_tests.c \
    loc_api_cb_ni.c \
    loc_api_xtra_time.c \
    loc_api_xtra_bin.c \
    loc_api_gui.c \
    loc_api_cb_ds_on.c \
    loc_parameter.ini \
    loc_api_test_main.c


loc_api_app_SOURCES = $(c_sources)
if USE_GLIB
loc_api_app_CFLAGS = -DUSE_GLIB $(AM_CFLAGS) @GLIB_CFLAGS@
loc_api_app_LDFLAGS = -lstdc++ -lpthread @GLIB_LIBS@ -shared -version-info 1:0:0
loc_api_app_CPPFLAGS = -DUSE_GLIB $(AM_CFLAGS) $(AM_CPPFLAGS) @GLIB_CFLAGS@
else
loc_api_app_CFLAGS = $(AM_CFLAGS)
loc_api_app_LDFLAGS = -lpthread -shared -version-info 1:0:0
loc_api_app_CPPFLAGS = $(AM_CFLAGS) $(AM_CPPFLAGS)
endif

loc_api_app_LDADD = -lcutils $(requiredlibs)
bin_PROGRAMS = loc_api_app

