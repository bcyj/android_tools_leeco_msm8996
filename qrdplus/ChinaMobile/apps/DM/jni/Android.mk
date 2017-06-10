
LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional

# This is the target being built.
LOCAL_MODULE:= libdmjni

# All of the source files that we will compile.
LOCAL_SRC_FILES := $(call all-subdir-c-files)

# All of the shared libraries we link against.
LOCAL_SHARED_LIBRARIES := \
        liblog \
        libc \
        libutils
LOCAL_LDLIBS := -llog -lc -lutils

LOCAL_MULTILIB := 32

# No static libraries.
LOCAL_STATIC_LIBRARIES :=

# Also need the JNI headers.
LOCAL_C_INCLUDES += \
    $(JNI_H_INCLUDE)

# No special compiler flags.
LOCAL_CFLAGS += -w

LOCAL_PRELINK_MODULE := false

# This will install the file in /system/vendor/ChinaMobile
#LOCAL_MODULE_PATH := $(TARGET_OUT)/vendor/ChinaMobile/system/lib
#LOCAL_MODULE_RELATIVE_PATH := ../vendor/lib
LOCAL_MODULE_OWNER := qti
LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_SHARED_LIBRARY)

