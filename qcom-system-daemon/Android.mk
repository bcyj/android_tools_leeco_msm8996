ifeq ($(call is-vendor-board-platform,QCOM),true)
LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE := qcom-system-daemon
LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include \
		    $(TARGET_OUT_HEADERS)/common/inc/ \
		    $(TARGET_OUT_HEADERS)/diag/src \
		    $(TARGET_OUT_HEADERS)/diag/include \
		    $(TARGET_OUT_HEADERS)/subsystem_control/inc \
		    $(TARGET_OUT_HEADERS)/qmi-framework/common/inc \
		    $(TARGET_OUT_HEADERS)/qmi-framework/inc \
		    $(TARGET_OUT_HEADERS)/qmi/inc \
		    $(TARGET_OUT_HEADERS)/qmi/platform \
		    $(TARGET_OUT_HEADERS)/qmi-framework/qcsi/inc \
		    $(TARGET_OUT_HEADERS)/qmi-framework/qcci/inc \
		    external/libxml2/include \
		    external/icu/icu4c/source/common
LOCAL_ADDITIONAL_DEPENDENCIES += ${TARGET_OUT_INTERMEDIATES}/KERNEL_OBJ/usr
LOCAL_SRC_FILES := qcom_system_daemon.c
LOCAL_SHARED_LIBRARIES := libc libcutils libutils libdiag libxml2 libsubsystem_control libqmi_common_so libqmi_cci  libqmi_encdec libqmiservices
LOCAL_MODULE_TAGS := optional
LOCAL_CFLAGS += -Wall
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_MODULE       := carrier_regional_pack_conf.xml
LOCAL_MODULE_TAGS  := optional
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_PATH  := $(TARGET_OUT)/vendor/speccfg
LOCAL_SRC_FILES    := $(LOCAL_MODULE)
include $(BUILD_PREBUILT)

endif
