LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
        mp-ctl.c

LOCAL_SHARED_LIBRARIES := \
        libcutils

LOCAL_C_INCLUDES += \
        device/qcom/common/power

LOCAL_CFLAGS += \
        -DSERVER \
        -DCLIENT \
	-DPERFD \
        -g0 \
        -Wall \
        #-DQC_DEBUG=1

ifeq ($(call is-android-codename,JELLY_BEAN),true)
    LOCAL_CFLAGS += -DANDROID_JELLYBEAN=1

    LOCAL_C_INCLUDES += \
    $(TARGET_OUT_HEADERS)/common/inc
endif

LOCAL_MODULE := libqti-perfd
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_OWNER := qti

include $(BUILD_STATIC_LIBRARY)


include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
        mp-ctl.c

LOCAL_SHARED_LIBRARIES := \
        libcutils

LOCAL_C_INCLUDES += \
        device/qcom/common/power

LOCAL_CFLAGS += \
        -DCLIENT \
        -g0 \
        -Wall \
        #-DQC_DEBUG=1

ifeq ($(call is-android-codename,JELLY_BEAN),true)
    LOCAL_CFLAGS += -DANDROID_JELLYBEAN=1
    LOCAL_C_INCLUDES += \
    $(TARGET_OUT_HEADERS)/common/inc
endif

LOCAL_MODULE := libqti-perfd-client
LOCAL_MODULE_TAGS := optional

LOCAL_MODULE_OWNER := qti
include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)


LOCAL_SRC_FILES := mp-ctl-d.c

LOCAL_SHARED_LIBRARIES := libcutils

LOCAL_CFLAGS += \
        -DPERFD \

LOCAL_MODULE := perfd

LOCAL_MODULE_TAGS := optional

LOCAL_STATIC_LIBRARIES += libqti-perfd

LOCAL_MODULE_OWNER := qti
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)

LOCAL_SRC_FILES := client.c

LOCAL_MODULE := libqti-perfd-client

LOCAL_MODULE_TAGS := optional

LOCAL_STATIC_LIBRARIES += libqti-perfd-client

LOCAL_MODULE_OWNER := qti
LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_SHARED_LIBRARY)
