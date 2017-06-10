LOCAL_PATH:= $(call my-dir)

#Server
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
    PeripheralManagerServer.cpp \
    Client.cpp \
    Peripheral.cpp \
    ../subsystem-request/subsystem_request_v01.c

LOCAL_C_INCLUDES:= \
    external/connectivity/stlport/stlport \
    $(TARGET_OUT_HEADERS)/qmi-framework/inc \
    $(TARGET_OUT_HEADERS)/qmi/inc \
    $(TARGET_OUT_HEADERS)/libmdmdetect/inc/ \
    $(LOCAL_PATH)/../subsystem-request \
    $(LOCAL_PATH)/../ \
    $(LOCAL_PATH)/../libpmclient

LOCAL_SHARED_LIBRARIES:= \
    libc \
    libcutils \
    libutils \
    libbinder \
    libqmi_cci \
    libqmi_common_so \
    libqmi_encdec \
    libqmi_csi \
    libmdmdetect \
    libperipheral_client

LOCAL_MODULE:= pm-service
LOCAL_CFLAGS = -Wall -Werror
LOCAL_MODULE_OWNER := qti

include $(BUILD_EXECUTABLE)