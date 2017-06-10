## Check if GPS is unsupported
ifneq (true, $(strip $(GPS_NOT_SUPPORTED)))

BUILD_LOC_API_APP:=1
COPY_LOC_PARAMETER_INI:=1

USE_NCURSES:=0

# At most one of the following can be selected
USE_DCM:=0
USE_DCM_ALWAYS_ON:=1

ifeq ($(BUILD_LOC_API_APP),1)

LOCAL_PATH:=$(call my-dir)
include $(CLEAR_VARS)

LOC_API_APP_PATH:=$(LOCAL_PATH)

LOC_API_DEBUG:=false

LOCAL_SRC_FILES:= \
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
  loc_api_gui.c

LOCAL_SRC_FILES+=loc_api_test_main.c

LOCAL_CFLAGS:= \
    -DFEATURE_LOC_API_RPCGEN \
    -DDEBUG

LOCAL_C_INCLUDES:= \
    $(LOCAL_PATH)/../../../common/inc \
    $(TARGET_OUT_HEADERS)/libcommondefs-rpc/inc \
    $(TARGET_OUT_HEADERS)/librpc \
    $(TARGET_OUT_HEADERS)/libloc_api-rpc-qc \
    $(TARGET_OUT_HEADERS)/libloc_api-rpc-qc/rpc_inc

ifeq ($(LOC_API_USE_LOCAL_RPC),1)
LOCAL_CFLAGS += -DUSE_LOCAL_RPC
endif
ifeq ($(LOC_API_USE_QCOM_AUTO_RPC),1)
LOCAL_CFLAGS += -DUSE_QCOM_AUTO_RPC
LOCAL_C_INCLUDES+= \
    $(TARGET_OUT_HEADERS)/loc_api/rpcgen/inc \
    $(TARGET_OUT_HEADERS)/libcommondefs/rpcgen/inc \
    $(API_SRCDIR)/libs/remote_apis/loc_api/rpcgen/inc \
    $(API_SRCDIR)/libs/remote_apis/commondefs/rpcgen/inc \
    hardware/msm7k/librpc
endif

LOCAL_SHARED_LIBRARIES := \
    librpc \
    libutils \
    libcutils \
    libloc_api-rpc-qc \
    libgps.utils

LOCAL_PRELINK_MODULE:=false

LOCAL_CFLAGS+=$(GPS_FEATURES)

ifeq ($(LOC_API_DEBUG),true)
LOCAL_CFLAGS += -DVERBOSE -DMM_DEBUG
endif

ifeq ($(USE_DCM),1)
LOCAL_CFLAGS += -DFEATURE_GPS_DCM_ENABLED
LOCAL_SRC_FILES+=loc_api_cb_ds.c
endif

ifeq ($(USE_DCM_ALWAYS_ON),1)
LOCAL_CFLAGS += -DFEATURE_GPS_DCM_ALWAYS_ON
LOCAL_SRC_FILES+=loc_api_cb_ds_on.c
endif

ifeq ($(USE_NCURSES),1)
LOCAL_CFLAGS += -DHAS_NCURSES
LOCAL_LDLIBS += -lncurses
endif

LOCAL_MODULE:=loc_api_app
LOCAL_MODULE_TAGS := optional
include $(BUILD_EXECUTABLE)

endif

ifeq ($(COPY_LOC_PARAMETER_INI),1)

# Copy configuration file
LOCAL_PATH:=$(LOC_API_APP_PATH)
include $(CLEAR_VARS)
LOCAL_MODULE       := loc_parameter.ini
LOCAL_MODULE_TAGS  := optional
LOCAL_MODULE_CLASS := ETC
LOCAL_SRC_FILES    := $(LOCAL_MODULE)
include $(BUILD_PREBUILT)
endif

endif # GPS_NOT_SUPPORTED
