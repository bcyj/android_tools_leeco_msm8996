LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
commonSources  :=
commonIncludes := $(TARGET_OUT_HEADERS)/common/inc
commonIncludes += $(TARGET_OUT_HEADERS)/qmi-framework/inc
commonIncludes += $(TARGET_OUT_HEADERS)/sensors/inc
commonIncludes += $(TARGET_OUT_HEADERS)/diag/include

LOCAL_C_INCLUDES := $(commonIncludes)
LOCAL_C_INCLUDES += $(LOCAL_PATH)/inc
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../sensordaemon/common/inc
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../sensordaemon/common/idl/inc
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../sensordaemon/ucos-ii_posix/inc

LOCAL_MODULE:=sensorrdiag
LOCAL_MODULE_OWNER := qcom

LOCAL_SHARED_LIBRARIES += \
    libdiag \
    liblog  \
    libqmi_encdec  \
    libsensor1

LOCAL_SRC_FILES += \
	src/sensorrdiag.c

#include IDL files
IDL_H_FILES := $(shell find $(LOCAL_PATH)/../sensordaemon/common/idl/inc/ -type f -name '*_v[0-9][0-9].h' -print )
IDL_FILES := $(notdir $(IDL_H_FILES))
IDL_SRC_FILES := $(IDL_FILES:.h=.c)

LOCAL_SRC_FILES += $(addprefix ../sensordaemon/common/idl/src/,$(IDL_SRC_FILES))

LOCAL_CFLAGS += -D_GNU_SOURCE -DSNS_RDIAG_DEBUG -DSNS_LA
LOCAL_CFLAGS += -Wno-missing-field-initializers -Werror

LOCAL_PRELINK_MODULE := false
LOCAL_MODULE_TAGS := eng
include $(BUILD_EXECUTABLE)
