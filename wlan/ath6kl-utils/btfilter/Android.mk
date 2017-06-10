LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

include $(CLEAR_VARS)

LOCAL_SRC_FILES := abtfilt_core.c \
		abtfilt_main.c \
		abtfilt_utils.c \
		abtfilt_wlan.c \
		btfilter_action.c \
		nl80211_utils.c \
		btfilter_core.c

ifeq ($(BOARD_HAVE_BLUETOOTH_BLUEZ),true)
LOCAL_SRC_FILES += alljoyn_dbus.c \
		   alljoyn_intf.cpp
else
#LOCAL_SRC_FILES += abtfilt_bluedroid.c
LOCAL_SRC_FILES += alljoyn_dbus.c
endif

ifeq ($(BT_ALT_STACK),true)
LOCAL_SRC_FILES += abtfilt_bluez_hciutils.c
else
LOCAL_CFLAGS += -DCONFIG_NO_HCILIBS
endif

LOCAL_SHARED_LIBRARIES := \
		libcutils \
		libnl_2 \
		libdl

LOCAL_CFLAGS += -Wno-psabi -Wno-write-strings -DANDROID_NDK
LOCAL_CFLAGS += -DTARGET_ANDROID -DLINUX -DQCC_OS_GROUP_POSIX
LOCAL_CFLAGS += -DQCC_OS_ANDROID -DQCC_CPU_ARM -DANDROID

LOCAL_CFLAGS += -include bionic/libc/kernel/arch-arm/asm/posix_types.h
LOCAL_CFLAGS += -include bionic/libc/kernel/common/linux/socket.h

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH) \
	$(LOCAL_PATH)/include \
	$(LOCAL_PATH)/linux/include \
	$(LOCAL_PATH)/common/include \
	$(LOCAL_PATH)/os/linux/include

ifeq ($(BOARD_HAVE_BLUETOOTH_BLUEZ),true)
ALLJOYN_DIST := external/alljoyn/alljoyn_core
LOCAL_C_INCLUDES += $(ALLJOYN_DIST)/inc
LOCAL_C_INCLUDES += $(ALLJOYN_DIST)/autogen
LOCAL_C_INCLUDES += external/alljoyn/common/inc

LOCAL_C_INCLUDES += system/bluetooth/bluez-clean-headers/bluetooth
LOCAL_CFLAGS+=-DBLUEZ4_3
LOCAL_SHARED_LIBRARIES += liballjoyn
endif

LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include
LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr

LOCAL_C_INCLUDES += external/libnl-headers

LOCAL_CFLAGS+= -DABF_DEBUG

LOCAL_CFLAGS+= -DCONFIG_LIBNL20

LOCAL_MODULE := abtfilt
LOCAL_MODULE_TAGS := optional

ifeq ($(BOARD_HAS_ATH_WLAN_AR6004), true)
LOCAL_CFLAGS+= -DSEND_WMI_BY_IOCTL
LOCAL_CFLAGS+= -DMULTI_WLAN_CHAN_SUPPORT
LOCAL_CFLAGS+= -DHID_PROFILE_SUPPORT
endif

ifeq ($(BOARD_HAVE_BLUETOOTH_BLUEZ), false)
LOCAL_CFLAGS+= -DBLUETOOTH_BLUEDROID
endif

LOCAL_MODULE_OWNER := qcom

include $(BUILD_EXECUTABLE)
