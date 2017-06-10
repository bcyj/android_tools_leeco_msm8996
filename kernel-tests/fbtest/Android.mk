ifneq (,$(filter arm aarch64 arm64, $(TARGET_ARCH)))
ifneq ($(TARGET_USES_AOSP),true)

LOCAL_PATH := $(call my-dir)
commonSources :=

#
# fbtest
#=======================================================
include $(CLEAR_VARS)
LOCAL_MODULE:= fbtest
CFLAGS += -DFEATURE_MEMORY_VIRTUAL
CFLAGS += -Duint32_t="unsigned int"
LOCAL_SRC_FILES := $(commonSources) \
                   fbtest.c \
                   fbtestUtils.c


ifeq ($(TARGET_USES_ION),true)
        LOCAL_CFLAGS := -DUSE_ION
endif

MDP3_PRODUCT_LIST := msm7625_surf
MDP3_PRODUCT_LIST += msm7625_ffa
MDP3_PRODUCT_LIST += msm7627_surf
MDP3_PRODUCT_LIST += msm7627_ffa
MDP3_PRODUCT_LIST += msm7627a
MDP3_PRODUCT_LIST += qsd8250_surf
MDP3_PRODUCT_LIST += qsd8250_ffa
MDP3_PRODUCT_LIST += qsd8650a_st1x
MDP3_PRODUCT_LIST += msm8610
MDP3_PRODUCT_LIST += msm8909
MDP4_PRODUCT_LIST := msm7630_fusion
MDP4_PRODUCT_LIST += msm7630_surf
MDP4_PRODUCT_LIST += msm8660
MDP4_PRODUCT_LIST += msm8660_csfb
MDP4_PRODUCT_LIST += msm8960

MDP5_PRODUCT_LIST := msm8974
MDP5_PRODUCT_LIST += msm8226
MDP5_PRODUCT_LIST += apq8084
MDP5_PRODUCT_LIST += mpq8092
MDP5_PRODUCT_LIST += msm8916
MDP5_PRODUCT_LIST += msm8994
MDP5_PRODUCT_LIST += msm8939

# MDP5 is backward compatible with MDP4
MDP4_PRODUCT_LIST += $(MDP5_PRODUCT_LIST)

QPIC_PRODUCT_LIST += mdm9625

TARGET_WITH_MDP3 := $(call is-board-platform-in-list,$(MDP3_PRODUCT_LIST))
TARGET_WITH_MDP4 := $(call is-board-platform-in-list,$(MDP4_PRODUCT_LIST))
TARGET_WITH_MDP5 := $(call is-board-platform-in-list,$(MDP5_PRODUCT_LIST))
TARGET_WITH_QPIC := $(call is-board-platform-in-list,$(QPIC_PRODUCT_LIST))

ifneq (,$(strip $(TARGET_WITH_MDP3)))
        LOCAL_CFLAGS += -DMDP3_FLAG
        LOCAL_SRC_FILES += mdp3.c
endif

ifneq (,$(strip $(TARGET_WITH_MDP4)))
        LOCAL_CFLAGS += -DMDP4_FLAG
        LOCAL_SRC_FILES += mdp4.c
endif

ifneq (,$(strip $(TARGET_WITH_MDP5)))
	LOCAL_CFLAGS += -DVENUS_COLOR_FORMAT
endif

ifneq (,$(strip $(TARGET_WITH_QPIC)))
        LOCAL_CFLAGS += -DQPIC_FLAG
endif

LOCAL_C_INCLUDES := system/core/include/cutils \
		$(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include/

LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr

LOCAL_SHARED_LIBRARIES := \
    libc \
    libcutils

ifeq (,$(strip $(BUILD_TINY_ANDROID)))
LOCAL_SHARED_LIBRARIES += libmm-abl
LOCAL_ADDITIONAL_DEPENDENCIES += \
    liblog libmm-abl-oem libcutils libdiag \
    libpowermanager libbinder libutils libqservice \
    libtinyxml
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/pp/inc
LOCAL_CFLAGS += -DENABLE_POSTPROC
LOCAL_SRC_FILES += postproctest.c
endif

LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT_DATA)/kernel-tests
LOCAL_MODULE_OWNER := qcom
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_MODULE := fbtest.sh
LOCAL_MODULE_CLASS := EXECUTABLES
LOCAL_SRC_FILES := fbtest.sh
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT_DATA)/kernel-tests
LOCAL_MODULE_OWNER := qcom
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := AR_LUT_1_0_B0
LOCAL_MODULE_CLASS := DATA
LOCAL_SRC_FILES := AR_LUT_1_0_B0
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT_DATA)/kernel-tests/postproc_cfg
LOCAL_MODULE_OWNER := qcom
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := AR_LUT_1_0_B
LOCAL_MODULE_CLASS := DATA
LOCAL_SRC_FILES := AR_LUT_1_0_B
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT_DATA)/kernel-tests/postproc_cfg
LOCAL_MODULE_OWNER := qcom
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := AR_LUT_1_0_G0
LOCAL_MODULE_CLASS := DATA
LOCAL_SRC_FILES := AR_LUT_1_0_G0
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT_DATA)/kernel-tests/postproc_cfg
LOCAL_MODULE_OWNER := qcom
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := AR_LUT_1_0_G
LOCAL_MODULE_CLASS := DATA
LOCAL_SRC_FILES := AR_LUT_1_0_G
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT_DATA)/kernel-tests/postproc_cfg
LOCAL_MODULE_OWNER := qcom
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := AR_LUT_1_0_R0
LOCAL_MODULE_CLASS := DATA
LOCAL_SRC_FILES := AR_LUT_1_0_R0
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT_DATA)/kernel-tests/postproc_cfg
LOCAL_MODULE_OWNER := qcom
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := r_only_igc
LOCAL_MODULE_CLASS := DATA
LOCAL_SRC_FILES := r_only_igc
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT_DATA)/kernel-tests/postproc_cfg
LOCAL_MODULE_OWNER := qcom
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := SanityCfg.cfg
LOCAL_MODULE_CLASS := DATA
LOCAL_SRC_FILES := SanityCfg.cfg
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT_DATA)/kernel-tests/postproc_cfg
LOCAL_MODULE_OWNER := qcom
include $(BUILD_PREBUILT)

endif
endif
