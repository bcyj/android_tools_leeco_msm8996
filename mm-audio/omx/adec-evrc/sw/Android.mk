ifeq ($(call is-vendor-board-platform,QCOM),true)
ifneq ($(BUILD_TINY_ANDROID),true)

ROOT_DIR := $(call my-dir)
LOCAL_PATH:= $(ROOT_DIR)
include $(CLEAR_VARS)

# ---------------------------------------------------------------------------------
#                 Common definitons
# ---------------------------------------------------------------------------------

libOmxEvrcDec-def :=  -g -O3
libOmxEvrcDec-def +=  -DQC_MODIFIED
libOmxEvrcDec-def +=  -D_ANDROID_
libOmxEvrcDec-def +=  -DPROFILE_DECODER
libOmxEvrcDec-def +=  -DVERBOSE
libOmxEvrcDec-def +=  -D_DEBUG

ifeq ($(call is-board-platform,msm8660),true)
AUDIO_V2 := true
libOmxEvrcDec-def += -DAUDIOV2
endif
ifeq ($(call is-chipset-in-board-platform,msm7630),true)
AUDIO_V2 := true
libOmxEvrcDec-def += -DAUDIOV2
endif

# ---------------------------------------------------------------------------------
#             Make the apps-test (mm-adec-omxevrc-test)
# ---------------------------------------------------------------------------------

include $(CLEAR_VARS)

LOCAL_PATH:= $(ROOT_DIR)

mm-evrc-dec-test-inc    := $(LOCAL_PATH)/inc
mm-evrc-dec-test-inc    += $(LOCAL_PATH)/test
mm-evrc-dec-test-inc    += $(TARGET_OUT_HEADERS)/mm-core/omxcore
mm-evrc-dec-test-inc    += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include
ifeq ($(call is-board-platform,msm8960),true)
mm-evrc-dec-test-inc    += $(AUDIO_ROOT)/audio-alsa/inc
endif
ifeq ($(call is-board-platform,msm8660),true)
mm-evrc-dec-test-inc    += $(AUDIO_ROOT)/audio-alsa/inc
endif
ifeq ($(call is-chipset-in-board-platform,msm7630),true)
mm-evrc-dec-test-inc    += $(AUDIO_ROOT)/audio-alsa/inc
endif
LOCAL_MODULE            := mm-adec-omxevrc-test
LOCAL_MODULE_TAGS       := optional
LOCAL_CFLAGS            := $(libOmxEvrcDec-def)
LOCAL_C_INCLUDES        := $(mm-evrc-dec-test-inc)
LOCAL_PRELINK_MODULE    := false
LOCAL_SHARED_LIBRARIES  := libmm-omxcore
ifeq ($(call is-board-platform,msm8960),true)
LOCAL_SHARED_LIBRARIES  += libaudioalsa
endif
ifeq ($(call is-board-platform,msm8660),true)
LOCAL_SHARED_LIBRARIES  += libaudioalsa
endif
ifeq ($(call is-chipset-in-board-platform,msm7630),true)
LOCAL_SHARED_LIBRARIES  += libaudioalsa
endif
LOCAL_SHARED_LIBRARIES  += libOmxEvrcDec

LOCAL_SRC_FILES         := test/omx_evrc_dec_test.c

LOCAL_ADDITIONAL_DEPENDENCIES  := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr

include $(BUILD_EXECUTABLE)

endif #BUILD_TINY_ANDROID
endif # is-vendor-board-platform
# ---------------------------------------------------------------------------------
#                     END
# ---------------------------------------------------------------------------------

