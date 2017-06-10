ifeq ($(call is-vendor-board-platform,QCOM),true)
CORE_PATH := $(call my-dir)
LOCAL_PATH := $(CORE_PATH)
include $(CLEAR_VARS)

LOCAL_COPY_HEADERS_TO := mm-core/mmi
LOCAL_COPY_HEADERS    := mmi/mmiDeviceApi.h

include $(BUILD_COPY_HEADERS)

include $(CORE_PATH)/display/Android.mk
endif
