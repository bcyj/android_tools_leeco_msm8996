LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES:= IPhoneService.cpp \
                  CommandApi.cpp

LOCAL_SHARED_LIBRARIES := libbinder libcutils libutils

LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/common/inc

LOCAL_COPY_HEADERS_TO := command-svc
LOCAL_COPY_HEADERS := ./CommandApi.h

LOCAL_MODULE:= libCommandSvc

LOCAL_MODULE_TAGS := optional
LOCAL_PRELINK_MODULE := false

LOCAL_MODULE_OWNER := qcom
LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_SHARED_LIBRARY)

