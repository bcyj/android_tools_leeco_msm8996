
LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional

# This is the target being built.
LOCAL_MODULE:= libapkscanner

# All of the source files that we will compile.
LOCAL_SRC_FILES := $(call all-subdir-c-files)

# All of the shared libraries we link against.
LOCAL_SHARED_LIBRARIES := \
        liblog \
        libc \
        libutils
LOCAL_LDLIBS := -llog -lc -lutils

# Also need the JNI headers.
LOCAL_C_INCLUDES += \
    $(JNI_H_INCLUDE)

# No special compiler flags.
LOCAL_CFLAGS += -w

include $(BUILD_SHARED_LIBRARY)

