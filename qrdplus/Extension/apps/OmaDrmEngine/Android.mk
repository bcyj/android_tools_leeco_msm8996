
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

base := frameworks/base
av := frameworks/av

# Determine whether the DRM framework uses 64-bit data types for file offsets and do the same.
ifneq ($(shell grep -c 'off64_t offset' $(av)/drm/libdrmframework/plugins/common/include/IDrmEngine.h), 0)
LOCAL_CFLAGS += -DUSE_64BIT_DRM_API
endif

LOCAL_SRC_FILES:= \
    src/OmaDrmEngine.cpp

LOCAL_MODULE := libomadrmengine

LOCAL_STATIC_LIBRARIES := \
    libdrmutility \
    libdrmframeworkcommon

LOCAL_SHARED_LIBRARIES := \
    libicui18n \
    libicuuc \
    libutils \
    libdl \
    libandroid_runtime \
    libnativehelper \
    libcrypto \
    libssl \
    libdrmframework \
    libdrm1 \
    libdrm1_jni \
    libcutils


LOCAL_C_INCLUDES += \
    $(JNI_H_INCLUDE) \
    $(av)/include/drm \
    $(av)/drm/libdrmframework/plugins/common/include \
    $(av)/drm/libdrmframework/plugins/common/util/include \
    $(base)/media/libdrm/mobile1/include \
    $(LOCAL_PATH)/include \
    external/openssl/include


LOCAL_MODULE_PATH := $(TARGET_OUT_VENDOR_SHARED_LIBRARIES)/drm 

LOCAL_MODULE_TAGS := optional

include $(BUILD_SHARED_LIBRARY)
