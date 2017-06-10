ifeq ($(TARGET_USES_QCA_NFC),true)

# function to find all *.cpp files under a directory
define all-cpp-files-under
$(patsubst ./%,%, \
  $(shell cd $(LOCAL_PATH) ; \
          find $(1) -name "*.cpp" -and -not -name ".*") \
 )
endef


LOCAL_PATH:= $(call my-dir)
NFA := src/nfa
NFC := src/nfc
HAL := src/hal
UDRV := src/udrv
HALIMPL := halimpl/ncihal
D_CFLAGS := -DANDROID -DBUILDCFG=1

# compile in DTA support
D_CFLAGS += -DDTA


######################################
# Build shared library system/vendor/lib/libqnfc-nci.so for stack code.

include $(CLEAR_VARS)
LOCAL_PROPRIETARY_MODULE := true
LOCAL_PRELINK_MODULE := false
LOCAL_ARM_MODE := arm
LOCAL_MODULE := libqnfc-nci
LOCAL_MODULE_TAGS := optional
LOCAL_SHARED_LIBRARIES := libhardware_legacy libcutils liblog libdl libhardware
LOCAL_CFLAGS := $(D_CFLAGS)
LOCAL_C_INCLUDES := external/connectivity/stlport/stlport \
    $(LOCAL_PATH)/src/include \
    $(LOCAL_PATH)/src/gki/ulinux \
    $(LOCAL_PATH)/src/gki/common \
    $(LOCAL_PATH)/$(NFA)/include \
    $(LOCAL_PATH)/$(NFA)/int \
    $(LOCAL_PATH)/$(NFC)/include \
    $(LOCAL_PATH)/$(NFC)/int \
    $(LOCAL_PATH)/src/hal/include \
    $(LOCAL_PATH)/src/hal/int
LOCAL_SRC_FILES := \
    $(call all-c-files-under, $(NFA)/ce $(NFA)/dm $(NFA)/ee) \
    $(call all-c-files-under, $(NFA)/hci $(NFA)/int $(NFA)/p2p $(NFA)/rw $(NFA)/sys) \
    $(call all-c-files-under, $(NFC)/int $(NFC)/llcp $(NFC)/nci $(NFC)/ndef $(NFC)/nfc $(NFC)/tags) \
    $(call all-c-files-under, src/adaptation) \
    $(call all-cpp-files-under, src/adaptation) \
    $(call all-c-files-under, src/gki) \
    src/nfca_version.c
include $(BUILD_SHARED_LIBRARY)


######################################
# Build shared library system/vendor/lib/hw/nfc_nci.*.so for Hardware Abstraction Layer.
# Android's generic HAL (libhardware.so) dynamically loads this shared library.

include $(CLEAR_VARS)
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_RELATIVE_PATH := hw
LOCAL_MODULE := nfc_nci.$(TARGET_BOARD_PLATFORM)
#necessary as libhardware.so loads this name
LOCAL_SRC_FILES := $(call all-c-files-under, $(HALIMPL)) \
    $(call all-cpp-files-under, $(HALIMPL)) \
    src/adaptation/CrcChecksum.cpp \
    src/nfca_version.c
LOCAL_SHARED_LIBRARIES := liblog libcutils libhardware_legacy
LOCAL_SHARED_LIBRARIES += libril libqmi_cci libqmi_common_so libqmiservices
LOCAL_MODULE_TAGS := optional
LOCAL_C_INCLUDES := external/connectivity/stlport/stlport \
    $(LOCAL_PATH)/$(HALIMPL)/include \
    $(LOCAL_PATH)/$(HALIMPL)/gki/ulinux \
    $(LOCAL_PATH)/$(HALIMPL)/gki/common \
    $(LOCAL_PATH)/$(HALIMPL)/DT_Include \
    $(LOCAL_PATH)/$(HAL)/include \
    $(LOCAL_PATH)/$(HAL)/int \
    $(LOCAL_PATH)/src/include \
    $(LOCAL_PATH)/$(NFC)/include \
    $(LOCAL_PATH)/$(NFA)/include \
    $(LOCAL_PATH)/$(UDRV)/include \
    $(TARGET_OUT_HEADERS)/qmi/inc \
    $(TARGET_OUT_HEADERS)/qmi-framework/inc
LOCAL_CFLAGS := $(D_CFLAGS) -DNFC_HAL_TARGET=TRUE -DNFC_RW_ONLY=TRUE
LOCAL_CPPFLAGS := $(LOCAL_CFLAGS)
include $(BUILD_SHARED_LIBRARY)


######################################
include $(call all-makefiles-under,$(LOCAL_PATH))
endif
