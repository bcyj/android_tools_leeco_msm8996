VOB_COMPONENTS := vendor/qcom/proprietary/nfc/libqnfc-nci/src
NFA := $(VOB_COMPONENTS)/nfa
NFC := $(VOB_COMPONENTS)/nfc

LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_PRELINK_MODULE := false

ifneq ($(NCI_VERSION),)
LOCAL_CFLAGS += -DNCI_VERSION=$(NCI_VERSION) -O0 -g
endif

LOCAL_CFLAGS += -Wall -Wextra

# DTA
LOCAL_CFLAGS += -DDTA

define all-cpp-files-under
$(patsubst ./%,%, \
  $(shell cd $(LOCAL_PATH) ; \
          find $(1) -name "*.cpp" -and -not -name ".*") \
 )
endef

LOCAL_SRC_FILES:= $(call all-cpp-files-under, .)

LOCAL_C_INCLUDES += \
    external/connectivity/stlport/stlport \
    external/libxml2/include \
    external/icu/icu4c/source/common \
    frameworks/native/include \
    libcore/include \
    $(NFA)/include \
    $(NFA)/brcm \
    $(NFC)/include \
    $(NFC)/brcm \
    $(NFC)/int \
    $(VOB_COMPONENTS)/hal/include \
    $(VOB_COMPONENTS)/hal/int \
    $(VOB_COMPONENTS)/include \
    $(VOB_COMPONENTS)/gki/ulinux \
    $(VOB_COMPONENTS)/gki/common

LOCAL_SHARED_LIBRARIES := \
    libicuuc \
    libnativehelper \
    libcutils \
    libutils \
    liblog \
    libqnfc-nci \
    libxml2

LOCAL_MODULE := libqnfc_nci_jni
LOCAL_MODULE_TAGS := optional

include $(BUILD_SHARED_LIBRARY)
