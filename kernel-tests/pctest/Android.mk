ifneq ($(call is-android-codename-in-list,JELLY_BEAN),true)
ifeq ($(TARGET_ARCH),arm)

PCTEST_BOARD_PLATFORM_LIST := msm8660
PCTEST_BOARD_PLATFORM_LIST += msm8960

ifeq ($(call is-board-platform-in-list,$(PCTEST_BOARD_PLATFORM_LIST)),true)

LOCAL_PATH := $(call my-dir)
#
# pctest
#=======================================================
include $(CLEAR_VARS)
LOCAL_MODULE := pctest
LOCAL_SRC_FILES := tools.c \
                   suspend_test_8x60.c \
                   idle_test_8x60.c \
                   main.c

LOCAL_SHARED_LIBRARIES := \
        libc \
        libcutils \
        libutils

LOCAL_CFLAGS +=		  \
	-DUSE_ANDROID_LOG	  \
LOCAL_C_INCLUDES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include/
LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr

LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT_DATA)/kernel-tests
LOCAL_MODULE_OWNER := qcom
include $(BUILD_EXECUTABLE)

else

LOCAL_PATH := $(call my-dir)
#
# pctest
#=======================================================
include $(CLEAR_VARS)
LOCAL_MODULE := pctest
LOCAL_SRC_FILES := tools.c \
                   suspend_test_legacy.c \
                   idle_test_legacy.c \
                   main.c

LOCAL_SHARED_LIBRARIES := \
        libc \
        libcutils \
        libutils
LOCAL_C_INCLUDES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include/
LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr

LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT_DATA)/kernel-tests
LOCAL_MODULE_OWNER := qcom
include $(BUILD_EXECUTABLE)

endif

#
# modem_test
#=======================================================
include $(CLEAR_VARS)
LOCAL_MODULE := modem_test
LOCAL_C_INCLUDES := vendor/qcom/proprietary/data/netmgr/src
LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include/
LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr

LOCAL_SRC_FILES := tools.c \
                   modem_test.c

LOCAL_SHARED_LIBRARIES := \
        libc \
        libcutils \
        libutils

LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT_DATA)/kernel-tests
LOCAL_MODULE_OWNER := qcom
include $(BUILD_EXECUTABLE)

#
# pc-compound-test.sh
#=======================================================
include $(CLEAR_VARS)
LOCAL_MODULE       := pc-compound-test.sh
LOCAL_SRC_FILES    := pc-compound-test.sh
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := EXECUTABLES
LOCAL_MODULE_PATH  := $(TARGET_OUT_DATA)/kernel-tests
LOCAL_MODULE_OWNER := qcom
include $(BUILD_PREBUILT)

endif
endif # JELLY BEAN
