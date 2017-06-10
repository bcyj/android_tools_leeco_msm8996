#
# Build Listen Java library for either full SVA or ST variant
#

ifeq ($(strip $(AUDIO_FEATURE_ENABLED_LISTEN)),true)
# Build Listen JNI for all of SVA
BUILD_SVA_JAVA := true
BUILD_FULL_SVA := true
endif

ifeq ($(strip $(BOARD_SUPPORTS_SOUND_TRIGGER)),true)
# Build Listen Java for SoundTrigger implementation
# subset of SVA 2.x built
BUILD_SVA_JAVA := true
BUILD_FULL_SVA := false
endif

ifeq ($(BUILD_SVA_JAVA),true)

LOCAL_PATH := $(call my-dir)

#=============================================
#  Listen API lib
#=============================================

include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
    src/com/qualcomm/listen/ListenTypes.java \
    src/com/qualcomm/listen/ListenSoundModel.java

ifeq ($(BUILD_FULL_SVA),true)
LOCAL_SRC_FILES += \
    src/com/qualcomm/listen/IListenEventProcessor.java \
    src/com/qualcomm/listen/ListenMasterControl.java \
    src/com/qualcomm/listen/ListenReceiver.java \
    src/com/qualcomm/listen/ListenVoiceWakeupSession.java
endif

LOCAL_MODULE_TAGS := optional

LOCAL_MODULE := Listen

include $(BUILD_STATIC_JAVA_LIBRARY)

endif
