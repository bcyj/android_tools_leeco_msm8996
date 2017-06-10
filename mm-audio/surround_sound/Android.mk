ifeq ($(call is-board-platform-in-list, msm8960 msm8974 msm8610 msm8226 msm8610 copper apq8084 msm8916 msm8909 msm8994),true)
ifneq ($(BUILD_TINY_ANDROID),true)

LOCAL_PATH:= $(call my-dir)

########################################################
#                deploy SSR library                   #
########################################################
include $(CLEAR_VARS)
LOCAL_MODULE             := libsurround_proc
LOCAL_MODULE_TAGS        := optional
LOCAL_SRC_FILES          := lib/libsurround_proc.so
LOCAL_MODULE_CLASS       := SHARED_LIBRARIES
LOCAL_MODULE_SUFFIX      := .so
LOCAL_MODULE_OWNER       := qcom
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MULTILIB           := 32

include $(BUILD_PREBUILT)

########################################################
#                deploy SSR headers                    #
########################################################
include $(CLEAR_VARS)
LOCAL_COPY_HEADERS_TO   := mm-audio/surround_sound
LOCAL_COPY_HEADERS      := ola_filters/surround_filters_interface.h
LOCAL_COPY_HEADERS      += libsurround_proc/profile.h
LOCAL_C_INCLUDES        := $(TARGET_OUT_HEADERS)/common/inc
LOCAL_CFLAGS            := -include $(TARGET_OUT_HEADERS)/common/inc/comdef.h
LOCAL_MODULE:= surround_sound_headers
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_OWNER := qcom
LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_SHARED_LIBRARY)

########################################################
#                deploy SSR filters                    #
########################################################
include $(CLEAR_VARS)
LOCAL_MODULE       := filter1i.pcm
LOCAL_MODULE_TAGS  := optional
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_PATH  := $(TARGET_OUT_ETC)/surround_sound/
LOCAL_SRC_FILES    := filters/filter1i.pcm
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE       := filter1r.pcm
LOCAL_MODULE_TAGS  := optional
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_PATH  := $(TARGET_OUT_ETC)/surround_sound/
LOCAL_SRC_FILES    := filters/filter1r.pcm
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE       := filter2i.pcm
LOCAL_MODULE_TAGS  := optional
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_PATH  := $(TARGET_OUT_ETC)/surround_sound/
LOCAL_SRC_FILES    := filters/filter2i.pcm
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE       := filter2r.pcm
LOCAL_MODULE_TAGS  := optional
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_PATH  := $(TARGET_OUT_ETC)/surround_sound/
LOCAL_SRC_FILES    := filters/filter2r.pcm
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE       := filter3i.pcm
LOCAL_MODULE_TAGS  := optional
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_PATH  := $(TARGET_OUT_ETC)/surround_sound/
LOCAL_SRC_FILES    := filters/filter3i.pcm
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE       := filter3r.pcm
LOCAL_MODULE_TAGS  := optional
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_PATH  := $(TARGET_OUT_ETC)/surround_sound/
LOCAL_SRC_FILES    := filters/filter3r.pcm
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE       := filter4i.pcm
LOCAL_MODULE_TAGS  := optional
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_PATH  := $(TARGET_OUT_ETC)/surround_sound/
LOCAL_SRC_FILES    := filters/filter4i.pcm
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE       := filter4r.pcm
LOCAL_MODULE_TAGS  := optional
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_PATH  := $(TARGET_OUT_ETC)/surround_sound/
LOCAL_SRC_FILES    := filters/filter4r.pcm
include $(BUILD_PREBUILT)

endif # BUILD_TINY_ANDROID
endif # is-board-platform-in-list
