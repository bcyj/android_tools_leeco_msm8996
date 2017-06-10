LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE_OWNER := qti

LOCAL_PROPRIETARY_MODULE := true

LOCAL_MODULE:= libmmi

LOCAL_SRC_FILES := util_comm.cpp \
                   util_string.cpp \
                   util_system.cpp \
                   util_ui.cpp \
                   textview.cpp \
                   button.cpp \
                   listview.cpp \
                   layout.cpp \
                   nv.cpp


LOCAL_C_INCLUDES := bootable/recovery/minui \
                    external/connectivity/stlport/stlport \
                    $(QC_PROP_ROOT)/diag/include \
                    $(QC_PROP_ROOT)/diag/src/ \
                    $(TARGET_OUT_HEADERS)/common/inc

LOCAL_MODULE_TAGS := optional
LOCAL_CFLAGS := -Wall
LOCAL_SHARED_LIBRARIES := libminui libcutils libdiag \
                          libft2 libutils libc

LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include
ifeq ($(TARGET_COMPILE_WITH_MSM_KERNEL),true)
LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr
endif

include $(BUILD_SHARED_LIBRARY)