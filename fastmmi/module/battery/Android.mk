LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE_OWNER := qti
LOCAL_PROPRIETARY_MODULE := true
LOCAL_SRC_FILES := battery.cpp

LOCAL_MODULE := mmi_battery
LOCAL_MODULE_TAGS := optional
LOCAL_CFLAGS := -Wall
LOCAL_C_INCLUDES := $(QC_PROP_ROOT)/fastmmi/libmmi \
                    external/connectivity/stlport/stlport

LOCAL_SHARED_LIBRARIES := libcutils libutils libmmi

LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include
ifeq ($(TARGET_COMPILE_WITH_MSM_KERNEL),true)
LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr
endif

include $(BUILD_SHARED_LIBRARY)