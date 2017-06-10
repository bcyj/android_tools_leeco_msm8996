ifneq ($(BUILD_TINY_ANDROID),true)

ROOT_DIR := $(call my-dir)
LOCAL_PATH := $(ROOT_DIR)
BUILD_TEST_PROGRAMS := true

# ---------------------------------------------------------------------------------
#                 Common definitons
# ---------------------------------------------------------------------------------

rtsp-def := -DRTSP_DBG_ALL -D_ANDROID_
rtsp-inc := $(LOCAL_PATH)/rtsplib/inc
rtsp-inc += external/connectivity/stlport/stlport
rtsp-inc += $(TARGET_OUT_HEADERS)/mm-osal/include/
rtsp-inc += $(TARGET_OUT_HEADERS)/common/inc
# ---------------------------------------------------------------------------------
#             Make the apps
# ---------------------------------------------------------------------------------

include $(CLEAR_VARS)
LOCAL_MODULE := libwfdrtsp
LOCAL_MODULE_TAGS := optional
LOCAL_CFLAGS := $(rtsp-def)
LOCAL_C_INCLUDES := $(rtsp-inc)
LOCAL_PRELINK_MODULE := false
LOCAL_SHARED_LIBRARIES := libstlport
LOCAL_SRC_FILES := rtsplib/src/rtsp_base.cpp
LOCAL_SRC_FILES += rtsplib/src/rtsp_state.cpp
LOCAL_SRC_FILES += rtsplib/src/rtsp_source.cpp
LOCAL_SRC_FILES += rtsplib/src/rtsp_sink.cpp
LOCAL_SRC_FILES += rtsplib/src/rtsp_session.cpp
LOCAL_SRC_FILES += rtsplib/src/rtsp_linux.cpp
LOCAL_SRC_FILES += rtsplib/src/rtsp_api.cpp
LOCAL_SRC_FILES += rtsplib/src/rtsp_parser.cpp
LOCAL_SRC_FILES += rtsplib/src/rtsp_helper.cpp
LOCAL_SRC_FILES += rtsplib/src/rtsp_server.cpp
LOCAL_SRC_FILES += rtsplib/src/rtsp_client.cpp
LOCAL_SRC_FILES += rtsplib/src/rtsp_wfd.cpp
LOCAL_SRC_FILES += rtsplib/src/RTSPStringStream.cpp

LOCAL_SHARED_LIBRARIES += libcutils libutils libmmosal libwfdcommonutils
LOCAL_LDLIBS += -llog
LOCAL_MODULE_OWNER := qcom
LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_SHARED_LIBRARY)

ifeq ($(BUILD_TEST_PROGRAMS),true)

include $(CLEAR_VARS)
LOCAL_MODULE := rtspclient
LOCAL_MODULE_TAGS := optional
LOCAL_CFLAGS := $(rtsp-def)
LOCAL_C_INCLUDES := $(rtsp-inc)
LOCAL_PRELINK_MODULE := false
LOCAL_SHARED_LIBRARIES := libwfdrtsp libstlport libmmosal libwfdcommonutils
LOCAL_SRC_FILES    := rtsptest/sink/src/sink.cpp
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_MODULE := rtspserver
LOCAL_MODULE_TAGS := optional
LOCAL_CFLAGS := $(rtsp-def)
LOCAL_C_INCLUDES := $(rtsp-inc)
LOCAL_PRELINK_MODULE := false
LOCAL_SHARED_LIBRARIES := libwfdrtsp libstlport libmmosal libwfdcommonutils
LOCAL_SRC_FILES    := rtsptest/source/src/source.cpp
include $(BUILD_EXECUTABLE)

endif #BUILD_TEST_PROGRAMS
endif #BUILD_TINY_ANDROID

# ---------------------------------------------------------------------------------
#                     END
# ---------------------------------------------------------------------------------


