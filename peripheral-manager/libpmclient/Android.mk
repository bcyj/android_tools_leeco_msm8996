LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)
ifeq ($(TARGET_PER_MGR_ENABLED),true)
	LOCAL_CFLAGS += -DPER_MGR_SUPPORTED
endif
LOCAL_SRC_FILES:= \
    PeripheralManagerClient.cpp \
    IPeripheralManager.cpp

LOCAL_C_INCLUDES := \
    $(LOCAL_PATH)/../pm-server \
    $(LOCAL_PATH)/.. \
    external/connectivity/stlport/stlport \
    $(TARGET_OUT_HEADERS)/libmdmdetect/inc/

LOCAL_SHARED_LIBRARIES:= libcutils libutils libbinder libmdmdetect
LOCAL_MODULE:= libperipheral_client
LOCAL_MODULE_TAGS := optional
LOCAL_PROPRIETARY_MODULE := true
LOCAL_CFLAGS += -Wall -Werror
LOCAL_MODULE_OWNER := qti
LOCAL_COPY_HEADERS_TO := libperipheralclient/inc
LOCAL_COPY_HEADERS := ../pm-service.h
include $(BUILD_SHARED_LIBRARY)
