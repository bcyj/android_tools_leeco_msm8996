LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH) \
	system/core/include/cutils/ \
	$(TARGET_OUT_HEADERS)/common/inc/ \
    $(TARGET_OUT_HEADERS)/qmi-framework/inc \
    $(TARGET_OUT_HEADERS)/qmi/inc \
	$(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include/linux/usb

LOCAL_SRC_FILES:= \
   usb_uicc_daemon.c\
   usb_uicc_daemon.h\
   usb_uicc_qmi.c\
   usb_uicc_qmi.h

LOCAL_ADDITIONAL_DEPENDENCIES := \
    $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr

LOCAL_SHARED_LIBRARIES := libutils libcutils libc libqmiservices libidl libqmi_common_so libqmi_cci

LOCAL_MODULE:= usb_uicc_client
LOCAL_MODULE_TAGS := optional
LOCAL_CFLAGS += \
        -DUSE_ANDROID_LOG \

include $(BUILD_EXECUTABLE)
