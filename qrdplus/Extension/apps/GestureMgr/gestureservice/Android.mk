LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
	gestureservice.c \
	gesturebinder.cpp

LOCAL_SHARED_LIBRARIES := \
	liblog \
	libutils \
	libbinder

LOCAL_MODULE:= gestureservice

include $(BUILD_EXECUTABLE)

