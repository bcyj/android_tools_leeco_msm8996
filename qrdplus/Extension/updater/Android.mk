LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

recovery_dir_common := $(call include-path-for, recovery)
to_src_root := ../../../../../..

LOCAL_SRC_FILES := \
        $(to_src_root)/$(recovery_dir_common)/roots.cpp \
        $(to_src_root)/$(recovery_dir_common)/ui.cpp \
        $(to_src_root)/$(recovery_dir_common)/asn1_decoder.cpp \
        $(to_src_root)/$(recovery_dir_common)/verifier.cpp \
        updater.cpp

LOCAL_STATIC_LIBRARIES += libminzip libmincrypt libminui libmtdutils
LOCAL_STATIC_LIBRARIES += libcutils liblog libc
LOCAL_STATIC_LIBRARIES += libfs_mgr libext4_utils_static libz

LOCAL_CFLAGS += -include $(recovery_dir_common)/ui.h
LOCAL_CFLAGS += -include $(recovery_dir_common)/common.h
LOCAL_CFLAGS += -include $(recovery_dir_common)/verifier.h

LOCAL_C_INCLUDES += \
    $(recovery_dir_common)/minzip \
    $(recovery_dir_common)/mtdutils

LOCAL_C_INCLUDES += \
    system/core/fs_mgr/include \
    system/extras/ext4_utils \
    system/vold

LOCAL_C_INCLUDES += external/openssl/include

LOCAL_MODULE := wrapper-updater
LOCAL_32_BIT_ONLY := true

LOCAL_FORCE_STATIC_EXECUTABLE := true

include $(BUILD_EXECUTABLE)
