ifeq ($(TARGET_ARCH),$(filter $(TARGET_ARCH),arm arm64))
ifeq ($(BOARD_HAVE_QCOM_FM),true)

LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

ifeq ($(call is-board-platform,msm8660),true)
  LOCAL_CFLAGS += -DBOARD_MSM8x60
endif

LOCAL_CFLAGS += -Werror -Wall -Wextra

LOCAL_MODULE := fm_qsoc_patches

LOCAL_C_INCLUDES :=  \
		     $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include \
		     $(TARGET_OUT_HEADERS)/qmi/inc \

LOCAL_SHARED_LIBRARIES := libqmi libqcci_legacy libqmiservices libcutils
LOCAL_SRC_FILES := fm_qsoc_patches.c
LOCAL_MODULE_TAGS := optional

LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr

LOCAL_MODULE_OWNER := qcom
include $(BUILD_EXECUTABLE)

endif

endif
