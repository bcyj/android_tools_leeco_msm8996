# LOCAL_PATH needs to be hard-coded because wmsts is inside
# the qcril directory (bug in the Android makefile system).
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

libwmsts_api_specific_defines := -DFEATURE_EXPORT_WMS
libwmsts_api_specific_defines += -DFEATURE_CDSMS
libwmsts_api_specific_defines += -DFEATURE_GWSMS
libwmsts_api_specific_defines += -DFEATURE_SMS_UDH
libwmsts_api_specific_defines += -DFEATURE_GWSMS_BROADCAST
libwmsts_api_specific_defines += -DFEATURE_CDSMS_BROADCAST
libwmsts_api_specific_defines += -DPACKED=

#common_libwmsts_cflags := -g
#common_libwmsts_cflags += -O0
#common_libwmsts_cflags += -fno-inline
#common_libwmsts_cflags += -fno-short-enums
common_libwmsts_cflags += $(libwmsts_api_specific_defines)

libwmsts_includes := $(TARGET_OUT_HEADERS)/oncrpc/inc
libwmsts_includes += $(TARGET_OUT_HEADERS)/wms/inc
libwmsts_includes += $(LOCAL_PATH)/../qcril
libwmsts_includes += $(TARGET_OUT_HEADERS)/common/inc
libwmsts_includes += hardware/ril/include/telephony

LOCAL_CFLAGS += $(common_libwmsts_cflags)
LOCAL_CFLAGS += -include $(TARGET_OUT_HEADERS)/oncrpc/inc/oncrpc/err.h

LOCAL_C_INCLUDES := $(libwmsts_includes)

LOCAL_SRC_FILES := wmsts.c
LOCAL_SRC_FILES += wmstscdma.c
LOCAL_SRC_FILES += bit.c

LOCAL_SHARED_LIBRARIES := libcutils libutils 

LOCAL_LDLIBS += -lpthread

LOCAL_MODULE := libwmsts

LOCAL_MODULE_TAGS := optional debug

LOCAL_PRELINK_MODULE := false

LOCAL_MODULE_OWNER := qcom
LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_SHARED_LIBRARY)
