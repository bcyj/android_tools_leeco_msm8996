ifeq ($(call is-vendor-board-platform,QCOM),true)

MUX_DISABLE_PLATFORM_LIST := msm8610 mpq8092 msm_bronze

ifneq ($(call is-board-platform-in-list,$(MUX_DISABLE_PLATFORM_LIST)),true)

ROOT_DIR := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_PATH := $(ROOT_DIR)
# ---------------------------------------------------------------------------------
#         Common definitons
# ---------------------------------------------------------------------------------

SECURE_MUX = 0

libmm-mux-def := -O3
libmm-mux-def += -D_ANDROID_
libmm-mux-def += -D_ANDROID_LOG_
libmm-mux-def += -D_ANDROID_LOG_ERROR
libmm-mux-def += -D_ANDROID_LOG_PROFILE
libmm-mux-def += -Du32="unsigned int"
libmm-mux-def += -DENABLE_MUX_STATS
ifeq ($(SECURE_MUX),1)
libmm-mux-def += -DFEATURE_FILEMUX_SECURE_MP2
else
libmm-mux-def += -UFEATURE_FILEMUX_SECURE_MP2
endif

# ---------------------------------------------------------------------------------
#    Make the apps
# ---------------------------------------------------------------------------------

include $(CLEAR_VARS)

mm-mux-inc := $(LOCAL_PATH)/main/FilemuxInternalDefs/inc
mm-mux-inc += $(LOCAL_PATH)/main/MuxBaseLib/inc
mm-mux-inc += $(LOCAL_PATH)/main/FileMuxLib/inc
mm-mux-inc += $(LOCAL_PATH)/main/FileMuxLib/src
mm-mux-inc += $(LOCAL_PATH)/main/MP2BaseFileLib/inc
ifeq ($(SECURE_MUX),1)
mm-mux-inc += $(LOCAL_PATH)/main/MP2SecureFilelib/inc
mm-mux-inc += $(LOCAL_PATH)/main/Transcode/
endif
ifeq ($(call is-android-codename,ICECREAM_SANDWICH),true)
mm-mux-inc += $(TOP)/frameworks/base/include/utils
else ifeq ("$(PLATFORM_VERSION)","4.3")
mm-mux-inc += $(TOP)/frameworks/native/include/utils
else
mm-mux-inc += $(TOP)/system/core/include/utils
endif
mm-mux-inc += $(TARGET_OUT_HEADERS)/mm-osal/include
mm-mux-inc += $(TARGET_OUT_HEADERS)/mm-parser/include
mm-mux-inc += $(LOCAL_PATH)/include
mm-mux-inc += $(TARGET_OUT_HEADERS)/common/inc
ifeq ($(SECURE_MUX),1)
mm-mux-inc += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include \
mm-mux-inc += $(LOCAL_PATH)/../securemsm/QSEEComAPI
endif
LOCAL_MODULE := libFileMux
LOCAL_CFLAGS := $(libmm-mux-def)
LOCAL_C_INCLUDES := $(mm-mux-inc)
LOCAL_PRELINK_MODULE := false
LOCAL_SHARED_LIBRARIES := libbinder
LOCAL_SHARED_LIBRARIES += libmmosal
LOCAL_SHARED_LIBRARIES += liblog
LOCAL_SHARED_LIBRARIES += libutils
LOCAL_SHARED_LIBRARIES += libwfdcommonutils
LOCAL_SHARED_LIBRARIES += libcutils
ifeq ($(SECURE_MUX),1)
LOCAL_SHARED_LIBRARIES += libQSEEComAPI
endif
LOCAL_SRC_FILES := main/FileMuxLib/src/MuxQueue.cpp
LOCAL_SRC_FILES += main/MuxBaseLib/src/muxbase.cpp
LOCAL_SRC_FILES += main/FileMuxLib/src/filemux.cpp
LOCAL_SRC_FILES += main/MP2BaseFileLib/src/MP2BaseFile.cpp
ifeq ($(SECURE_MUX),1)
LOCAL_SRC_FILES += main/MP2SecureFilelib/src/MP2BaseFileSecure.cpp
LOCAL_SRC_FILES += main/MP2SecureFilelib/src/smux_mem.c
LOCAL_SRC_FILES += main/MP2SecureFilelib/src/smux_tz_commands.c
endif
LOCAL_SRC_FILES += main/MuxBaseLib/src/qmmList.c

ifeq ($(SECURE_MUX),1)
LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr
endif

LOCAL_COPY_HEADERS_TO :=  mm-mux
LOCAL_COPY_HEADERS += main/MuxBaseLib/inc/qmmList.h
LOCAL_COPY_HEADERS += main/MuxBaseLib/inc/muxbase.h
LOCAL_COPY_HEADERS += main/MuxBaseLib/inc/isucceedfail.h
LOCAL_COPY_HEADERS += main/FilemuxInternalDefs/inc/filemuxinternaldefs.h
LOCAL_COPY_HEADERS += main/FileMuxLib/inc/filemux.h
LOCAL_COPY_HEADERS += main/FileMuxLib/inc/filemuxtypes.h

LOCAL_MODULE_TAGS := optional


LOCAL_MODULE_OWNER := qcom
LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_SHARED_LIBRARY)

endif # is-board-platform
endif #is-vendor-board-platform



