#
# Build ListenJNI library for either full SVA or ST variant
#

ifeq ($(strip $(AUDIO_FEATURE_ENABLED_LISTEN)),true)
# Build Listen JNI for SVA
# all SVA including SVA 1.0 built
BUILD_SVA_JNI := true
BUILD_FULL_SVA := true
endif

ifeq ($(strip $(BOARD_SUPPORTS_SOUND_TRIGGER)),true)
# Build Listen JNI for SoundTrigger implementation
# subset of SVA 2.x built
BUILD_SVA_JNI := true
BUILD_FULL_SVA := false
endif

ifeq ($(BUILD_SVA_JNI),true)

LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_CFLAGS += -DAUDIO_LISTEN_ENABLED \
                -DLOG_NDEBUG

ifeq ($(BUILD_FULL_SVA),true)
LOCAL_CFLAGS +=  -DSVA1_SUPPORTED
LOCAL_CFLAGS +=  -DLISTEN_SERVICE_USED
endif

LOCAL_SRC_FILES:= \
    src/com_qualcomm_listen_ListenSoundModel.cpp \
    src/SoundModel.cpp

ifeq ($(BUILD_FULL_SVA),true)
LOCAL_SRC_FILES += \
    src/com_qualcomm_listen_ListenReceiver.cpp \
    src/com_qualcomm_listen_ListenMasterControl.cpp \
    src/com_qualcomm_listen_ListenVoiceWakeupSession.cpp
endif

LOCAL_SHARED_LIBRARIES := \
    libnativehelper \
    libutils \
    libbinder \
    libui \
    libcutils \
    libgui

ifeq ($(BUILD_FULL_SVA),true)
LOCAL_SHARED_LIBRARIES += liblisten
LOCAL_SHARED_LIBRARIES += liblistensoundmodel
endif
LOCAL_SHARED_LIBRARIES += liblistensoundmodel2

LOCAL_MODULE:= liblistenjni
LOCAL_MODULE_TAGS := optional
LOCAL_PROPRIETARY_MODULE := true

LOCAL_C_INCLUDES += \
    $(LOCAL_PATH)/inc \
    $(TOP)/hardware/libhardware/include \
    $(TOP)/hardware/libhardware_legacy/include \
    $(TARGET_OUT_HEADERS)/mm-audio/audio-listen

ifeq ($(BUILD_FULL_SVA),true)
LOCAL_C_INCLUDES += $(TOP)/vendor/qcom/proprietary/mm-audio/audio-listen/framework/service/inc
endif

include $(BUILD_SHARED_LIBRARY)

endif
