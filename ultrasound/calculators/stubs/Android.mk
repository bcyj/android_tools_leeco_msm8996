ifneq ($(BUILD_TINY_ANDROID),true)

ROOT_DIR := $(call my-dir)
LOCAL_PATH := $(ROOT_DIR)
include $(CLEAR_VARS)

# ---------------------------------------------------------------------------------
#                               Common definitons
# ---------------------------------------------------------------------------------


# ---------------------------------------------------------------------------------
#                       Make the Shared library (libqcp2p)
# ---------------------------------------------------------------------------------

include $(CLEAR_VARS)
LOCAL_MODULE            := libqcp2p
LOCAL_MODULE_TAGS       := optional
LOCAL_SRC_FILES         := p2p_stub.cpp
LOCAL_SHARED_LIBRARIES  := liblog
LOCAL_C_INCLUDES        := $(LOCAL_PATH)/../../ual_util
LOCAL_MODULE_OWNER := qcom
LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_SHARED_LIBRARY)

# ---------------------------------------------------------------------------------
#                       Make the Shared library (libqchovering)
# ---------------------------------------------------------------------------------

include $(CLEAR_VARS)
LOCAL_MODULE            := libqchovering
LOCAL_MODULE_TAGS       := optional
LOCAL_SRC_FILES         := hovering_stub.cpp
LOCAL_SHARED_LIBRARIES  := liblog
LOCAL_C_INCLUDES        := $(LOCAL_PATH)/../../ual_util
LOCAL_MODULE_OWNER := qcom
LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_SHARED_LIBRARY)

# ---------------------------------------------------------------------------------
#                       Make the Shared library (libqcproximity)
# ---------------------------------------------------------------------------------

include $(CLEAR_VARS)
LOCAL_MODULE            := libqcproximity
LOCAL_MODULE_TAGS       := optional
LOCAL_SRC_FILES         := proximity_stub.cpp
LOCAL_SHARED_LIBRARIES  := liblog
LOCAL_MODULE_OWNER := qcom
LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_SHARED_LIBRARY)

# ---------------------------------------------------------------------------------
#                       Make the Shared library (libqcsyncgesture)
# ---------------------------------------------------------------------------------

include $(CLEAR_VARS)
LOCAL_MODULE            := libqcsyncgesture
LOCAL_MODULE_TAGS       := optional
LOCAL_SRC_FILES         := sync_gesture_stub.cpp
LOCAL_C_INCLUDES        := $(LOCAL_PATH)/../../ual_util
LOCAL_SHARED_LIBRARIES  := liblog
LOCAL_MODULE_OWNER := qcom
LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_SHARED_LIBRARY)

# ---------------------------------------------------------------------------------
#                       Make the Shared library (libmmgesture-bus2)
# ---------------------------------------------------------------------------------

include $(CLEAR_VARS)
LOCAL_MODULE            := libmmgesture-bus2
LOCAL_MODULE_TAGS       := optional
LOCAL_SRC_FILES         := gesture_bus_stub.cpp
LOCAL_SHARED_LIBRARIES  := liblog
LOCAL_C_INCLUDES        := $(LOCAL_PATH)/../../ual_util
LOCAL_MODULE_OWNER := qcom
LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_SHARED_LIBRARY)

# ---------------------------------------------------------------------------------
#                       Make the Shared library (libqcgesture)
# ---------------------------------------------------------------------------------

include $(CLEAR_VARS)
LOCAL_MODULE            := libqcgesture
LOCAL_MODULE_TAGS       := optional
LOCAL_SRC_FILES         := gesture_stub.cpp
LOCAL_SHARED_LIBRARIES  := liblog
LOCAL_C_INCLUDES        := $(LOCAL_PATH)/../../ual_util
LOCAL_MODULE_OWNER := qcom
LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_SHARED_LIBRARY)

endif #BUILD_TINY_ANDROID
