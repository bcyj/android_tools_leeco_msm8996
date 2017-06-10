LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

# ---------------------------------------------------------------------------------
#            Common definitons
# ---------------------------------------------------------------------------------


LOCAL_CFLAGS := -D_ANDROID_
ifeq ($(call is-android-codename,ICECREAM_SANDWICH),true)
LOCAL_CFLAGS += -DWFD_ICS
endif

# ---------------------------------------------------------------------------------
#            WFD COMMON UTILS SRC
# ---------------------------------------------------------------------------------

LOCAL_SRC_FILES := \
	src/wfd_cfg_parser.cpp \
	src/wfd_netutils.cpp\
        src/WFDMMThreads.cpp\
        src/WFDUtils.cpp\
        src/WFDMMSourceStatistics.cpp

# ---------------------------------------------------------------------------------
#             WFD COMMON UTILS INC
# ---------------------------------------------------------------------------------

LOCAL_C_INCLUDES := $(LOCAL_PATH)/./inc
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../uibc/interface/inc
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../mm/interface/inc
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/common/inc
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/mm-osal/include
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/mm-core/omxcore
LOCAL_C_INCLUDES += external/connectivity/stlport/stlport

# ---------------------------------------------------------------------------------
#            WFD COMMON UTILS SHARED LIB
# ---------------------------------------------------------------------------------

LOCAL_SHARED_LIBRARIES := libmmosal
LOCAL_SHARED_LIBRARIES += liblog
LOCAL_SHARED_LIBRARIES += libutils
LOCAL_SHARED_LIBRARIES += libcutils
LOCAL_LDLIBS += -llog

LOCAL_MODULE_TAGS := optional

LOCAL_MODULE:= libwfdcommonutils

LOCAL_MODULE_OWNER := qcom
LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_SHARED_LIBRARY)

include $(call all-makefiles-under,$(LOCAL_PATH))

# ---------------------------------------------------------------------------------
#            END
# ---------------------------------------------------------------------------------
