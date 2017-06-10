ifeq ($(call is-vendor-board-platform,QCOM),true)

LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_C_INCLUDES := $(TARGET_OUT_HEADERS)/common/inc
LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include


LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr

LOCAL_SRC_FILES:= \
        Qualcomm_FM_Test_HAL_Layer.cpp \
        FmRadioController_qcom.cpp \
        ConfigFmThs.cpp \
        FmPerformanceParams.cpp \
        QcomFmIoctlsInterface.cpp \
        ConfFileParser.cpp \

LOCAL_MODULE:= hal_ss_test_manual
LOCAL_MODULE_TAGS := debug
LOCAL_SHARED_LIBRARIES  += libcutils

include $(BUILD_EXECUTABLE)

endif # filter
