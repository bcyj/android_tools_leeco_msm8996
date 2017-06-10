#/******************************************************************************
#*@file Android.mk
#* brief Rules for compiling native code associated with VZW Native
#* GPS Location Provider
#*******************************************************************************/
LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

# This is the target being built.
LOCAL_MODULE:= libloc_ext
LOCAL_MODULE_TAGS := optional

# the name of LIBLOC has been changed from "libloc_api" to "libloc"
# the wildcard function tries to find any directory named libloc_api or libloc
# the notdir removes the long path which is not necessary

# All of the source files that we will compile.
LOCAL_SRC_FILES:= \
  jni_bridge.cpp \
  loc_ext.cpp

# All of the shared libraries we link against.
LOCAL_SHARED_LIBRARIES := \
        libutils \
        libcutils \
        libloc_eng \
        libloc_core \
        libgps.utils \
        libandroid_runtime

# Also need the JNI headers.
LOCAL_C_INCLUDES += \
        $(JNI_H_INCLUDE) \
        $(TARGET_OUT_HEADERS)/libloc_eng \
        $(TARGET_OUT_HEADERS)/libloc_core \
        $(TARGET_OUT_HEADERS)/gps.utils \
    $(TOP)/hardware/qcom/gps/loc_api/ulp/inc

LOCAL_CFLAGS+=$(GPS_FEATURES)

#add QMI libraries for QMI targets
QMI_BOARD_PLATFORM_LIST := msm8960

ifeq ($(call is-board-platform-in-list,$(QMI_BOARD_PLATFORM_LIST)),true)
LOCAL_C_INCLUDES += \
    $(TOP)/vendor/qcom/opensource/location/loc_api/loc_api_v02 \
    $(TARGET_OUT_HEADERS)/qmi-framework/inc

LOCAL_CFLAGS+= -DUSE_QMI
endif #($(call is-board-platform-in-list,$(QMI_BOARD_PLATFORM_LIST)),true)

ifneq ($(TARGET_NO_RPC),true)

LOCAL_C_INCLUDES += \
    $(TARGET_OUT_HEADERS)/libloc_api-rpc-qc/rpc_inc \
    $(TARGET_OUT_HEADERS)/libcommondefs/rpcgen/inc \
    $(TARGET_OUT_HEADERS)/loc_api/rpcgen/inc \
    $(TOP)/hardware/msm7k/librpc

LOCAL_CFLAGS+= -DUSE_RPC

endif #TARGET_NO_RPC

# Don't prelink this library.  For more efficient code, you may want
# to add this library to the prelink map and set this to true. However,
# it's difficult to do this for applications that are not supplied as
# part of a system image.

LOCAL_PRELINK_MODULE := false

LOCAL_MODULE_OWNER := qcom
LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_SHARED_LIBRARY)
