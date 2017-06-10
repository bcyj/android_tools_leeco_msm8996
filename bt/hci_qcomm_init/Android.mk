ifeq ($(call is-vendor-board-platform,QCOM),true)

LOCAL_PATH:= $(call my-dir)

include $(QC_PROP_ROOT)/common/build/remote_api_makefiles/target_api_enables.mk
include $(QC_PROP_ROOT)/common/build/remote_api_makefiles/remote_api_defines.mk

#If you want to use the legacy way(by using NV) of set the BT address
#You can set the value to 0
BT_QSOC_GET_ITEMS_FROM_PROPERTY = 0
BT_QSOC_GET_ITEMS_FROM_PERSIST = 0

include $(CLEAR_VARS)

LOCAL_COPY_HEADERS_TO := bt/hci_qcomm_init
LOCAL_COPY_HEADERS    := bt_nv.h
LOCAL_COPY_HEADERS    += btqsocnvm.h
LOCAL_COPY_HEADERS    += btqsocnvmutils.h

LOCAL_C_INCLUDES:= $(LOCAL_PATH)

LOCAL_SRC_FILES:=  bt_nv.cpp
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/common/inc \

LOCAL_MODULE:= libbtnv
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_OWNER := qcom
LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)

LOCAL_C_INCLUDES := $(LOCAL_PATH)
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/common/inc \

LOCAL_SRC_FILES:= btnvtool.cpp
LOCAL_SHARED_LIBRARIES = libbtnv

LOCAL_MODULE:= btnvtool
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_OWNER := qcom

include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)

LOCAL_CFLAGS   += $(remote_api_defines)
LOCAL_CFLAGS   += $(remote_api_enables)

LOCAL_CFLAGS += -DFEATURE_BT_QSOC

# LOCAL_CFLAGS += -DFEATURE_BT_QSOC_NVM_EFS_MODE

ifeq ($(call is-board-platform-in-list,msm8960 msm7627a msm7630_surf msm7630_fusion msm8660),true)
  LOCAL_CFLAGS += -DFEATURE_BT_QSOC_MARIMBA_A0
  LOCAL_CFLAGS += -DFEATURE_BT_QSOC_MARIMBA_B0
  LOCAL_CFLAGS += -DFEATURE_BT_QSOC_MARIMBA_B1
  LOCAL_CFLAGS += -DFEATURE_BT_QSOC_BAHAMA_A0
  LOCAL_CFLAGS += -DFEATURE_BT_QSOC_BAHAMA_B0
  LOCAL_CFLAGS += -DFEATURE_BT_QSOC_BAHAMA_B1
# LOCAL_CFLAGS += -DFEATURE_BT_QSOC_SLEEP
  LOCAL_CFLAGS += -DFEATURE_BT_QSOC_SW_INBAND_SLEEP
  LOCAL_CFLAGS += -DBT_NVM_MBA_B0_LOWER_TX_POWER
  BT_QSOC_REF_CLOCK = 19200000
# BT_QSOC_DISABLE_SLEEP_MODE = 1
  BT_QSOC_ENABLE_CLOCK_SHARING = 1
else
  LOCAL_CFLAGS += -DFEATURE_BT_QSOC_BTS4020_BDB0
  LOCAL_CFLAGS += -DFEATURE_BT_QSOC_BTS4020_BDB1
  LOCAL_CFLAGS += -DFEATURE_BT_QSOC_BTS4020_R3
  LOCAL_CFLAGS += -DFEATURE_BT_QSOC_BTS4021_B1
  LOCAL_CFLAGS += -DFEATURE_BT_QSOC_BTS4025_B0
  LOCAL_CFLAGS += -DFEATURE_BT_QSOC_BTS4025_B1
  LOCAL_CFLAGS += -DFEATURE_BT_QSOC_BTS4025_B2
  LOCAL_CFLAGS += -DFEATURE_BT_QSOC_BTS4025_B3
  BT_QSOC_REF_CLOCK = 32000000
  LOCAL_CFLAGS += -DFEATURE_BT_QSOC_SLEEP
# LOCAL_CFLAGS += -DFEATURE_BT_QSOC_SW_INBAND_SLEEP
# BT_QSOC_DISABLE_SLEEP_MODE = 1
# BT_QSOC_ENABLE_CLOCK_SHARING = 1
endif

ifeq ($(call is-board-platform-in-list,msm8960 msm7630_surf msm7630_fusion msm8660),true)
  LOCAL_CFLAGS += -DFEATURE_BT_SYSTEM_CLOCK_XO_SUPPORT
endif

#Enabled LE feature. hence using 4.0 spec related Bluetooth local feature mask
ifeq ($(call is-board-platform-in-list,msm8974 msm8960 msm7627a msm8226 msm8610 msm8916 msm8909),true)
  LOCAL_CFLAGS += -DFEATURE_BT_4_0
else
  LOCAL_CFLAGS += -DFEATURE_BT_3_0
endif

#Enable MSM8909 specific NVM tags
ifeq ($(call is-board-platform-in-list, msm8909),true)
  LOCAL_CFLAGS += -DMSM8909
endif

# configure defaults, last entry sets default

BT_QSOC_HCI_DEVICE = /dev/ttyS0
BT_QSOC_HCI_DEVICE = /dev/ttyMSM0
BT_QSOC_HCI_DEVICE = /dev/ttyHS0

BT_QSOC_HCI_BAUD_RATE = 115200
BT_QSOC_HCI_BAUD_RATE = 3000000

ifeq ($(BOARD_HAS_QCOM_WLAN),true)
BT_QSOC_WLAN_COEXISTENCE = 1
LOCAL_CFLAGS += -DFEATURE_BT_QSOC_WLAN_LIBRA
endif

ifeq ($(BOARD_HAS_ATH_WLAN),true)
BT_QSOC_WLAN_COEXISTENCE = 1
endif

ifeq ($(BT_QSOC_GET_ITEMS_FROM_PROPERTY), 1)
LOCAL_CFLAGS += -DBT_QSOC_GET_ITEMS_FROM_PROPERTY
else
ifeq ($(BT_QSOC_GET_ITEMS_FROM_PERSIST), 1)
LOCAL_CFLAGS += -DBT_QSOC_GET_ITEMS_FROM_PERSIST
else
LOCAL_CFLAGS += -DBT_QSOC_GET_ITEMS_FROM_NV
endif
endif

ifdef BT_QSOC_DISABLE_SLEEP_MODE
LOCAL_CFLAGS += -DBT_QSOC_DISABLE_SLEEP_MODE
endif

ifdef BT_QSOC_HCI_DEVICE
LOCAL_CFLAGS += '-DBT_QSOC_HCI_DEVICE="$(BT_QSOC_HCI_DEVICE)"'
endif

ifdef BT_QSOC_HCI_BAUD_RATE
LOCAL_CFLAGS += -DBT_QSOC_HCI_BAUD_RATE=$(BT_QSOC_HCI_BAUD_RATE)
endif

ifdef BT_QSOC_REF_CLOCK
LOCAL_CFLAGS += -DBT_QSOC_REF_CLOCK=$(BT_QSOC_REF_CLOCK)
endif

ifdef BT_QSOC_ENABLE_CLOCK_SHARING
LOCAL_CFLAGS += -DBT_QSOC_ENABLE_CLOCK_SHARING
endif

ifdef BT_QSOC_WLAN_COEXISTENCE
LOCAL_CFLAGS += -DFEATURE_BT_WLAN_COEXISTENCE
endif

LOCAL_CFLAGS += -DFEATURE_BT_QSOC_CLASS2

LOCAL_C_INCLUDES := $(LOCAL_PATH)

ifeq ($(BT_QSOC_GET_ITEMS_FROM_PERSIST),1)
LOCAL_SHARED_LIBRARIES := libbtnv
else
# add libbtnv library
LOCAL_SHARED_LIBRARIES := libbtnv
LOCAL_SHARED_LIBRARIES  += libqmiservices libqmi libqcci_legacy libqmi_client_qmux liblog
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/qmi/inc
endif

LOCAL_CFLAGS += -DANDROID

LOCAL_SHARED_LIBRARIES  += libcutils

#Enabling warnings to be treated as errors.
LOCAL_CFLAGS += -Werror -Wall -Wextra
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/common/inc \

LOCAL_SRC_FILES:= bthci_qcomm_linux.cpp bthci_qcomm_linux_uart.c
LOCAL_SRC_FILES += bthci_qcomm_common.c btqsocnvmplatform_linux.c
LOCAL_SRC_FILES += bt_qsoc_nvm_BTS4020_BDB0_19P2Mhz.c bt_qsoc_nvm_BTS4020_BDB0_32Mhz.c
LOCAL_SRC_FILES += bt_qsoc_nvm_BTS4020_BDB1_19P2Mhz.c bt_qsoc_nvm_BTS4020_BDB1_32Mhz.c
LOCAL_SRC_FILES += bt_qsoc_nvm_BTS4020_R3_19P2Mhz.c bt_qsoc_nvm_BTS4020_R3_32Mhz.c
LOCAL_SRC_FILES += bt_qsoc_nvm_BTS4021_B1_19P2Mhz.c bt_qsoc_nvm_BTS4021_B1_32Mhz.c
LOCAL_SRC_FILES += bt_qsoc_nvm_BTS4025_B0_19P2Mhz.c bt_qsoc_nvm_BTS4025_B0_32Mhz.c
LOCAL_SRC_FILES += bt_qsoc_nvm_BTS4025_B1_19P2Mhz.c bt_qsoc_nvm_BTS4025_B1_32Mhz.c
LOCAL_SRC_FILES += bt_qsoc_nvm_BTS4025_B2_19P2Mhz.c bt_qsoc_nvm_BTS4025_B2_32Mhz.c
LOCAL_SRC_FILES += bt_qsoc_nvm_BTS4025_B3_19P2Mhz.c bt_qsoc_nvm_BTS4025_B3_32Mhz.c
LOCAL_SRC_FILES += bt_qsoc_nvm_MARIMBA_A0.c bt_qsoc_nvm_MARIMBA_B0.c bt_qsoc_nvm_MARIMBA_B1.c
LOCAL_SRC_FILES += bt_qsoc_nvm_BAHAMA_A0.c bt_qsoc_nvm_BAHAMA_B0.c  bt_qsoc_nvm_BAHAMA_B1.c
LOCAL_SRC_FILES += btqsocnvm.c btqsocnvmefsmode.c
LOCAL_SRC_FILES += btqsocnvmtags.c btqsocnvmprsr.c btqsocnvmutils.c
LOCAL_SRC_FILES += bt_qmi_dms_client.c

LOCAL_MODULE:= hci_qcomm_init
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_OWNER := qcom

include $(BUILD_EXECUTABLE)

TEMP_LOCAL_PATH := $(LOCAL_PATH)

ifeq ($(BOARD_HAS_QCA_BT_AR3002),true)
    include $(TEMP_LOCAL_PATH)/ar3k/Android.mk
endif

ifeq ($(BOARD_HAS_QCA_BT_ROME),true)
    include $(TEMP_LOCAL_PATH)/rome/Android.mk
    include $(TEMP_LOCAL_PATH)/wcnss_filter/Android.mk
endif

endif # is-vendor-board-platform
