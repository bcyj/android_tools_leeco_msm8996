ifeq ($(call is-vendor-board-platform,QCOM),true)
ifneq ($(TARGET_USES_AOSP),true)

MM_PARSER_PATH := $(call my-dir)
include $(call all-subdir-makefiles)

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

# ---------------------------------------------------------------------------------
#                 MM-Parser
# ---------------------------------------------------------------------------------

ifeq ($(TARGET_ENABLE_QC_AV_ENHANCEMENTS),true)
include $(LOCAL_PATH)/build_extended_parser_lib.mk
include $(LOCAL_PATH)/build.mk
endif #TARGET_ENABLE_QC_AV_ENHANCEMENTS

# ---------------------------------------------------------------------------------
#                 MM-Parser-lite
# ---------------------------------------------------------------------------------

COMPILE_MM_PARSER_LITE := false
COMPILE_MM_PARSER_LITE := $(shell if [ -f $(LOCAL_PATH)/../mm-parser-noship/build_lite.mk ] ; then echo true; fi)

ifeq ($(strip $(COMPILE_MM_PARSER_LITE)),true)

ifeq ($(TARGET_ENABLE_QC_AV_ENHANCEMENTS),true)
include $(LOCAL_PATH)/../mm-parser-noship/build_lite.mk
endif #TARGET_ENABLE_QC_AV_ENHANCEMENTS

endif #COMPILE_MM_PARSER_LITE

endif # Not (TARGET_USES_AOSP)
endif #is-vendor-board-platform
