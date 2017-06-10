ifeq ($(strip $(BOARD_SUPPORTS_SOUND_TRIGGER)),true)

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_CFLAGS := -DPLATFORM_DEFAULT

ifneq ($(filter msm8974,$(TARGET_BOARD_PLATFORM)),)
    LOCAL_CFLAGS = -DPLATFORM_MSM8974
endif

ifneq ($(filter apq8084 msm8994,$(TARGET_BOARD_PLATFORM)),)
    LOCAL_CFLAGS = -DPLATFORM_APQ8084
endif

LOCAL_CFLAGS += -DSOUND_TRIGGER_PLATFORM=$(TARGET_BOARD_PLATFORM)

LOCAL_SRC_FILES := \
	sound_trigger_hw.c \
	sound_trigger_platform.c

LOCAL_C_INCLUDES += \
	external/tinyalsa/include \
        external/expat/lib \
	$(call include-path-for, audio-route)

LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include
LOCAL_ADDITIONAL_DEPENDENCIES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr

LOCAL_SHARED_LIBRARIES := \
	liblog \
	libcutils \
	libtinyalsa \
	libaudioroute \
	libdl \
	libexpat

LOCAL_MODULE := sound_trigger.primary.$(TARGET_BOARD_PLATFORM)
LOCAL_MODULE_RELATIVE_PATH := hw
LOCAL_MODULE_OWNER := qti
LOCAL_MODULE_TAGS := optional
LOCAL_32_BIT_ONLY := true
LOCAL_PROPRIETARY_MODULE := true

LOCAL_COPY_HEADERS_TO   := mm-audio/sound_trigger
LOCAL_COPY_HEADERS      := sound_trigger_prop_intf.h

include $(BUILD_SHARED_LIBRARY)

#
# to build sound_trigger_test executable
# uncomment the lines below
#
#include $(CLEAR_VARS)
#
#LOCAL_SRC_FILES := sound_trigger_test.c
#LOCAL_C_INCLUDES += \
#	system/core/include \
#	hardware/libhardware/include
#
#LOCAL_SHARED_LIBRARIES := \
#	libcutils \
#	libhardware
#
#LOCAL_MODULE := sound_trigger_test
#LOCAL_MODULE_OWNER := qti
#LOCAL_MODULE_TAGS := optional
#LOCAL_32_BIT_ONLY := true
#LOCAL_PROPRIETARY_MODULE := true
#
#include $(BUILD_EXECUTABLE)

endif
