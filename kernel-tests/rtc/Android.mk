ifneq (,$(filter arm aarch64 arm64, $(TARGET_ARCH)))

WRITE_DISABLE_PRODUCT_LIST := msm8660
WRITE_DISABLE_PRODUCT_LIST += msm8960

ALARM_DISABLE_PRODUCT_LIST := msm7627a
ALARM_DISABLE_PRODUCT_LIST += msm7627_surf
ALARM_DISABLE_PRODUCT_LIST += msm7630_fusion
ALARM_DISABLE_PRODUCT_LIST += msm7630_surf

TARGET_WRITE_DISABLE := $(call is-board-platform-in-list,$(WRITE_DISABLE_PRODUCT_LIST))
TARGET_ALARM_DISABLE := $(call is-board-platform-in-list,$(ALARM_DISABLE_PRODUCT_LIST))

LOCAL_PATH := $(call my-dir)
commonSources :=

include $(CLEAR_VARS)

LOCAL_MODULE := rtc_test
LOCAL_C_INCLUDES := $(TARGET_OUT_HEADERS)/common/inc
LOCAL_C_FLAGS := -lstringl
LOCAL_SRC_FILES += $(commonSources) rtc_test.c
ifneq (,$(strip $(TARGET_WRITE_DISABLE)))
	LOCAL_CFLAGS := -DDISABLE_WRITE_TEST
endif

ifneq (,$(strip $(TARGET_ALARM_DISABLE)))
	LOCAL_CFLAGS += -DDISABLE_ALARM_READ
endif

LOCAL_SHARED_LIBRARIES := \
        libc \
        libcutils \
        libutils

LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT_DATA)/kernel-tests
LOCAL_MODULE_OWNER := qcom
include $(BUILD_EXECUTABLE)

endif
