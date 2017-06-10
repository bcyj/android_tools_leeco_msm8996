ifeq ($(call is-vendor-board-platform,QCOM),true)
ifeq ($(findstring true, $(BOARD_HAVE_QCOM_FM) $(BOARD_HAVE_BLUETOOTH)),true)

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

BDROID_DIR:= external/bluetooth/bluedroid

LOCAL_C_INCLUDES := $(TARGET_OUT_HEADERS)/common/inc
LOCAL_C_INCLUDES += $(BDROID_DIR)/hci/include

LOCAL_CFLAGS := -DANDROID

ifeq ($(BOARD_HAS_QCA_BT_ROME),true)
LOCAL_CFLAGS += -DBT_SOC_TYPE_ROME
endif

LOCAL_SRC_FILES := wds_main.c
LOCAL_SRC_FILES += wds_hci_pfal_linux.c

LOCAL_MODULE := wdsdaemon
LOCAL_MODULE_TAGS := optional
LOCAL_SHARED_LIBRARIES := libdiag
LOCAL_SHARED_LIBRARIES += libcutils \
                          libdl

include $(BUILD_EXECUTABLE)
endif  # filter
endif  # is-vendor-board-platform
