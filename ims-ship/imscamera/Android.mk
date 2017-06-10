ifneq ($(TARGET_USES_AOSP),true)

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SHARED_LIBRARIES := libcutils
LOCAL_SHARED_LIBRARIES += libutils
LOCAL_SHARED_LIBRARIES += liblog


LOCAL_SHARED_LIBRARIES += libnativehelper
LOCAL_SHARED_LIBRARIES += libandroid_runtime
LOCAL_SHARED_LIBRARIES += libgui
LOCAL_SHARED_LIBRARIES += libui
LOCAL_SHARED_LIBRARIES += libmedia
LOCAL_SHARED_LIBRARIES += libbinder
LOCAL_SHARED_LIBRARIES += libcameraservice
LOCAL_SHARED_LIBRARIES += libcamera_client
LOCAL_SHARED_LIBRARIES += libhardware


LOCAL_SRC_FILES += src/ImsCameraImplementation.cpp

LOCAL_C_INCLUDES := $(LOCAL_PATH)/src/
LOCAL_C_INCLUDES += $(LOCAL_PATH)/inc
LOCAL_C_INCLUDES += frameworks/base/include/binder
LOCAL_C_INCLUDES += frameworks/base/include/gui
LOCAL_C_INCLUDES += frameworks/native/include/binder
LOCAL_C_INCLUDES += frameworks/av/include/camera
LOCAL_C_INCLUDES += system/media/camera/include


ifeq ($(call is-board-platform,msm8974),true)
LOCAL_CFLAGS +=  -DB_FAMILY
LOCAL_CPPFLAGS +=  -DB_FAMILY
endif

ifeq ($(call is-board-platform,msm8226),true)
LOCAL_CFLAGS +=  -DB_FAMILY
LOCAL_CPPFLAGS +=  -DB_FAMILY
endif

ifeq ($(call is-board-platform,apq8084),true)
LOCAL_CFLAGS +=  -DB_FAMILY
LOCAL_CPPFLAGS +=  -DB_FAMILY
endif


ifeq ($(call is-android-codename,ICECREAM_SANDWICH),true)

LOCAL_C_INCLUDES += $(TOP)/hardware/qcom/media/libstagefrighthw
LOCAL_C_INCLUDES += $(TOP)/frameworks/base/media/libstagefright

#Include sanitized header start
#LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include
LOCAL_CFLAGS += -include bionic/libc/kernel/arch-arm/asm/posix_types.h
LOCAL_CFLAGS += -include bionic/libc/kernel/common/linux/socket.h
#End of inclusion of sanitized headers

endif


ifeq ($(call is-android-codename,JELLY_BEAN),true)
#LOCAL_C_INCLUDES += $(TOP)/hardware/qcom/media/libstagefrighthw
#LOCAL_C_INCLUDES += $(TOP)/frameworks/av/media/libstagefright
#LOCAL_C_INCLUDES += $(TOP)/hardware/qcom/display/libgralloc
LOCAL_C_INCLUDES += $(TOP)/frameworks/native/include/media/hardware
LOCAL_C_INCLUDES += $(TOP)/frameworks/native/include/media/openmax
LOCAL_C_INCLUDES += $(TOP)/frameworks/native/include/utils
#Include sanitized header start
LOCAL_CFLAGS += -include bionic/libc/kernel/arch-arm/asm/posix_types.h
LOCAL_CFLAGS += -include bionic/libc/kernel/common/linux/socket.h
LOCAL_CFLAGS += -DANDROID_MEDIA_JB
#End of inclusion of sanitized headers
endif

ifeq ($(PLATFORM_SDK_VERSION),17)
LOCAL_CFLAGS += -DFAREND_WITH_SURFACETEXTURE
endif

LOCAL_C_INCLUDES += frameworks/base/include/media
LOCAL_C_INCLUDES += hardware/libhardware/modules/gralloc/
LOCAL_C_INCLUDES += $(JNI_H_INCLUDE)

LOCAL_C_INCLUDES += frameworks/av/services/camera/libcameraservice/
ifneq ($(call is-android-codename,JELLY_BEAN),true)
LOCAL_C_INCLUDES += frameworks/av/services/camera/libcameraservice/device1/
endif
LOCAL_C_INCLUDES += hardware/libhardware/include/hardware/


LOCAL_MODULE:= lib-imscamera

LOCAL_MODULE_TAGS:=optional

LOCAL_PRELINK_MODULE := false

LOCAL_MODULE_OWNER := qcom
LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_SHARED_LIBRARY)

endif
