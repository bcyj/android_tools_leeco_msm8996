#/******************************************************************************
#*@file Android.mk
#* brief Rules for compiling native code associated with libulp library
#*  ******************************************************************************/
ifneq ($(BUILD_TINY_ANDROID),true)

LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)
ULP2_ROOT_DIR = ../

# This is the target being built.
LOCAL_MODULE:= libulp2

# All of the source files that we will compile.
LOCAL_SRC_FILES:= \
    ulp_main.cpp \
    ulp_data.cpp \
    ulp_msg.cpp \
    ulp_gnss.cpp \
    ulp_gnp.cpp \
    ulp_quipc.cpp \
    ulp_xtwifi.cpp \
    ulp_brain.cpp \
    ulp_monitor.cpp \
    ulp_debug.cpp \
    ulp_log.cpp \
    LocUlpProxy.cpp \
    ulp_zpp.cpp

# All of the shared libraries we link against.
LOCAL_SHARED_LIBRARIES := \
    libutils \
    libgps.utils \
    libdl \
    libloc_core \
    libizat_core \
    liblog \
    liblbs_core

ifeq ($(FEATURE_QUIPC_DEBUG_INFO_MAKE),true)
LOCAL_SHARED_LIBRARIES += libquipc_os_api
endif

LOCAL_C_INCLUDES += \
  $(TARGET_OUT_HEADERS)/gps.utils \
  $(TARGET_OUT_HEADERS)/libloc_core \
  $(TARGET_OUT_HEADERS)/libizat_core \
  $(TARGET_OUT_HEADERS)/liblocationservice \
  $(TARGET_OUT_HEADERS)/liblbs_core \
  $(LOCAL_PATH)/$(ULP2_ROOT_DIR)/inc

LOCAL_CFLAGS+=$(GPS_FEATURES) \
    -D_ANDROID_ \

LOCAL_LDFLAGS += -Wl,--export-dynamic
LOCAL_MODULE_TAGS := optional

LOCAL_PRELINK_MODULE := false
LOCAL_MODULE_OWNER := qcom
LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_SHARED_LIBRARY)

endif # not BUILD_TINY_ANDROID



