## Check if GPS is unsupported
ifneq (true, $(strip $(GPS_NOT_SUPPORTED)))

LOCAL_PATH:=$(call my-dir)
include $(CLEAR_VARS)

LOC_API_APP_PATH:=$(LOCAL_PATH)

LOCAL_SRC_FILES:= \
    xtra_config.c \
    xtra_servers.c \
    xtra_sntp_linux.c \
    xtra_http_linux.c \
    xtra_system_interface.c

LOCAL_CFLAGS:= \
    -DDEBUG

LOCAL_C_INCLUDES:= \
    $(LOCAL_PATH)/../privinc \
    $(LOCAL_PATH)/../pubinc \
    $(TOP)/hardware/qcom/gps/utils \
    $(TOP)/hardware/qcom/gps/platform_lib_abstractions

LOCAL_SHARED_LIBRARIES := \
    libutils \
    libcutils \
    libgps.utils

LOCAL_PRELINK_MODULE:=false

LOCAL_CFLAGS+=$(GPS_FEATURES)

LOCAL_PROPRIETARY_MODULE := true

LOCAL_MODULE:=libloc_xtra
LOCAL_MODULE_TAGS := optional
include $(BUILD_SHARED_LIBRARY)

endif # GPS_NOT_SUPPORTED
