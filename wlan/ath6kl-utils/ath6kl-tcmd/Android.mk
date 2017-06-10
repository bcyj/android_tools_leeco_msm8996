LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := athtestcmd

LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/libtcmd

ifeq ($(BOARD_HAS_ATH_WLAN_AR6004),true)
LOCAL_CFLAGS+= -DCONFIG_AR6002_REV6
LOCAL_SRC_FILES:= \
    athtestcmd_ar6004.c
else
LOCAL_SRC_FILES:= \
    athtestcmd.c  \
    sinit_common.c
endif

LOCAL_MODULE_TAGS := optional
LOCAL_LDLIBS += -lpthread -lrt

LOCAL_SHARED_LIBRARIES += libcutils
LOCAL_SHARED_LIBRARIES += libnl_2
LOCAL_STATIC_LIBRARIES += libtcmd

LOCAL_MODULE_OWNER := qcom

include $(BUILD_EXECUTABLE)


ifneq ($(BOARD_HAS_ATH_WLAN_AR6004),true)
include $(CLEAR_VARS)

LOCAL_MODULE := psatUtil

LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/libtcmd

LOCAL_SRC_FILES:= \
    psatUtil.c    \
    sinit_common.c

LOCAL_LDLIBS += -lpthread -lrt

LOCAL_MODULE_TAGS := optional

LOCAL_SHARED_LIBRARIES += libcutils
LOCAL_SHARED_LIBRARIES += libnl_2
LOCAL_STATIC_LIBRARIES += libtcmd

LOCAL_MODULE_OWNER := qcom

include $(BUILD_EXECUTABLE)
endif
