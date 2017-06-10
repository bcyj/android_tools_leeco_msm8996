#/******************************************************************************
#*@file Android.mk
#*brief Rules for compiling the source files
#*******************************************************************************/

# Current version of QDCM Mobile App is intended for mdp3 targets alone
TARGET_MDP3_LIST := msm8909
TARGET_MDP3 := $(call is-board-platform-in-list,$(TARGET_MDP3_LIST))

ifeq ($(TARGET_MDP3),true)
    LOCAL_PATH:= $(call my-dir)
    include $(CLEAR_VARS)

    LOCAL_MODULE_TAGS := optional

    LOCAL_SRC_FILES := $(call all-subdir-java-files)

    LOCAL_REQUIRED_MODULES := \
        com.qti.snapdragon.sdk.display
    LOCAL_JAVA_LIBRARIES := \
        com.qti.snapdragon.sdk.display

    LOCAL_PACKAGE_NAME := QDCMMobileApp
    LOCAL_CERTIFICATE := platform

    include $(BUILD_PACKAGE)

    include $(call all-makefiles-under,$(LOCAL_PATH))
endif

