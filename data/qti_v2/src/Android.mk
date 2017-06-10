LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_CFLAGS += -DFEATURE_DATA_LOG_QXDM

filetoadd = bionic/libc/kernel/arch-arm/asm/posix_types.h
LOCAL_CFLAGS += $(shell if [ -a $(filetoadd) ] ; then echo -include $(filetoadd) ; fi ;)

LOCAL_SHARED_LIBRARIES += libc
LOCAL_SHARED_LIBRARIES += librmnetctl
LOCAL_SHARED_LIBRARIES += libqmi_cci
LOCAL_SHARED_LIBRARIES += libdsutils
LOCAL_SHARED_LIBRARIES += libdiag
LOCAL_SHARED_LIBRARIES += libqmiservices
LOCAL_SHARED_LIBRARIES += libqmi
LOCAL_SHARED_LIBRARIES += libnetutils
LOCAL_SHARED_LIBRARIES += libcutils
LOCAL_SHARED_LIBRARIES += libqmi_client_qmux
LOCAL_SHARED_LIBRARIES += libqmi_encdec

LOCAL_C_INCLUDES := $(LOCAL_PATH)
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../diag/include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../qmi/services
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../common/inc
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../qmi-framework/inc
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../qmi/inc
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../qmi/src
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../qmi/platform
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../dsutils/inc
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../dss_new/inc
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../dss_new/src/utils/inc
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../dss_new/src/platform/inc
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/dataservices/rmnetctl
LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include
LOCAL_ADDITIONAL_DEPENDENCIES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr
LOCAL_C_INCLUDES += system/core/include/



LOCAL_SRC_FILES := qti_main.c \
                   qti_rmnet_peripheral.c \
                   qti_rmnet_modem.c \
                   qti_rmnet_data.c \
                   qti_cmdq.c \
                   qti_rmnet_qmi.c \
                   qti_rmnet_dpm.c


LOCAL_MODULE := qti
LOCAL_MODULE_TAGS := optional

LOCAL_MODULE_OWNER := qcom
LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_EXECUTABLE)
