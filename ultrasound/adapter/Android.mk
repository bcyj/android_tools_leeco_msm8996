ifneq ($(BUILD_TINY_ANDROID),true)


ROOT_DIR := $(call my-dir)
LOCAL_PATH := $(ROOT_DIR)

# ---------------------------------------------------------------------------------
#                               Common definitons
# ---------------------------------------------------------------------------------


# ---------------------------------------------------------------------------------
#                       Make the Shared library (libgesadapter)
# ---------------------------------------------------------------------------------

include $(CLEAR_VARS)

LOCAL_MODULE            := libgesadapter
LOCAL_MODULE_TAGS       := optional
LOCAL_CFLAGS            := -DLOG_NIDEBUG=0

LOCAL_C_INCLUDES        := $(LOCAL_PATH)/../ual \
                           $(LOCAL_PATH)/../ual_util \
                           $(LOCAL_PATH)/../../mm-gestures/gesture-bus2 \
                           $(LOCAL_PATH)/../../mm-gestures/gesture-core \
                           $(LOCAL_PATH)/../calculators/stubs
LOCAL_SRC_FILES         := gs_bus_adapter.cpp \
                           gesture_gs_bus_adapter.cpp \
                           listener.cpp
LOCAL_SHARED_LIBRARIES  := libmmgesture-bus2 \
                           liblog

LOCAL_MODULE_OWNER := qcom
LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_SHARED_LIBRARY)

# ---------------------------------------------------------------------------------
#                       Make the Shared library (libsyncgesadapter)
# ---------------------------------------------------------------------------------

include $(CLEAR_VARS)

LOCAL_MODULE            := libsyncgesadapter
LOCAL_MODULE_TAGS       := optional
LOCAL_CFLAGS            := -DLOG_NIDEBUG=0

LOCAL_C_INCLUDES        := $(LOCAL_PATH)/../ual \
                           $(LOCAL_PATH)/../ual_util \
                           $(LOCAL_PATH)/../../mm-gestures/gesture-bus2 \
                           $(LOCAL_PATH)/../../mm-gestures/gesture-core \
                           $(LOCAL_PATH)/../calculators/stubs
LOCAL_SRC_FILES         := gs_bus_adapter.cpp \
                           sync_gesture_gs_bus_adapter.cpp \
                           listener.cpp
LOCAL_SHARED_LIBRARIES  := libmmgesture-bus2 \
                           liblog

LOCAL_MODULE_OWNER := qcom
LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_SHARED_LIBRARY)

# ---------------------------------------------------------------------------------
#                       Make the Shared library (libproxadapter)
# ---------------------------------------------------------------------------------

include $(CLEAR_VARS)

LOCAL_MODULE            := libproxadapter
LOCAL_MODULE_TAGS       := optional
LOCAL_CFLAGS            := -DLOG_NIDEBUG=0 -ILOG_NIDEBUG=0

LOCAL_C_INCLUDES        := $(LOCAL_PATH)/../ual \
                           $(LOCAL_PATH)/../ual_util \
                           $(LOCAL_PATH)/../../mm-gestures/gesture-bus2 \
                           $(LOCAL_PATH)/../../mm-gestures/gesture-core \
                           $(LOCAL_PATH)/../calculators/stubs
LOCAL_SRC_FILES         := gs_bus_adapter.cpp \
                           proximity_gs_bus_adapter.cpp \
                           listener.cpp
LOCAL_SHARED_LIBRARIES  := libmmgesture-bus2 \
                           liblog

LOCAL_MODULE_OWNER := qcom
LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_SHARED_LIBRARY)

# ---------------------------------------------------------------------------------
#                       Make the Shared library (libgessockadapter)
# ---------------------------------------------------------------------------------

include $(CLEAR_VARS)

LOCAL_MODULE            := libgessockadapter
LOCAL_MODULE_TAGS       := optional
LOCAL_CFLAGS            := -DLOG_NIDEBUG=0

LOCAL_C_INCLUDES        := $(LOCAL_PATH)/../ual \
                           $(LOCAL_PATH)/../ual_util
LOCAL_SRC_FILES         := gesture_socket_adapter.cpp
LOCAL_SHARED_LIBRARIES  := liblog \
                           libual \
                           libualutil

LOCAL_MODULE_OWNER := qcom
LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_SHARED_LIBRARY)

# ---------------------------------------------------------------------------------
#                       Make the Shared library (libgessyncsockadapter)
# ---------------------------------------------------------------------------------

include $(CLEAR_VARS)

LOCAL_MODULE            := libgessyncsockadapter
LOCAL_MODULE_TAGS       := optional
LOCAL_CFLAGS            := -DLOG_NIDEBUG=0

LOCAL_C_INCLUDES        := $(LOCAL_PATH)/../ual \
                           $(LOCAL_PATH)/../ual_util
LOCAL_SRC_FILES         := sync_gesture_socket_adapter.cpp
LOCAL_SHARED_LIBRARIES  := liblog \
                           libual \
                           libualutil

LOCAL_MODULE_OWNER := qcom
LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_SHARED_LIBRARY)

endif #BUILD_TINY_ANDROID
