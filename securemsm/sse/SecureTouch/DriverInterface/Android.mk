# =============================================================================
#
# Module: Secure Touch Driver Interface
#
# =============================================================================

LOCAL_PATH          := $(call my-dir)
SECUREMSM_PATH      := vendor/qcom/proprietary/securemsm

include $(CLEAR_VARS)

ifeq ($(call is-board-platform,msm8974),true)
  ST_TARGET_CFLAGS := -DST_TARGET_MSM8974
else ifeq ($(call is-board-platform,apq8084),true)
  ST_TARGET_CFLAGS := -DST_TARGET_APQ8084
else ifeq ($(call is-board-platform,msm8994),true)
  ST_TARGET_CFLAGS := -DST_TARGET_MSM8994
else ifeq ($(call is-board-platform,msm8916),true)
  ST_TARGET_CFLAGS := -DST_TARGET_MSM8916
endif

LOCAL_MODULE        := libStDrvInt

LOCAL_MODULE_TAGS := optional

LOCAL_C_INCLUDES   := \
                    $(LOCAL_PATH)/include \
                    $(TARGET_OUT_HEADERS)/common/inc

LOCAL_CFLAGS        := $(ST_TARGET_CFLAGS) -g -fdiagnostics-show-option

LOCAL_SRC_FILES     := \
                    src/StDrvInt.c \

LOCAL_SHARED_LIBRARIES := liblog

LOCAL_MODULE_OWNER := qti
LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_SHARED_LIBRARY)
