ifeq ($(call is-vendor-board-platform,QCOM),true)

ROOT_DIR := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_HTTP_PATH := $(ROOT_DIR)

include $(LOCAL_HTTP_PATH)/IPStream/Common/Network/Android_Network.mk

include $(LOCAL_HTTP_PATH)/IPStream/Common/StreamUtils/Android_StreamUtils.mk

include $(LOCAL_HTTP_PATH)/IPStream/Protocol/HTTP/Android_Protocol.mk

include $(LOCAL_HTTP_PATH)/IPStream/Source/HTTP/Android_Source.mk

include $(LOCAL_HTTP_PATH)/IPStream/MMI/HTTP/Android_MMI.mk

include $(LOCAL_HTTP_PATH)/AAL/Android_AAL.mk

ifeq ($(call is-board-platform-in-list, msm8994),true)
include $(LOCAL_HTTP_PATH)/DEAL/Android_DEAL.mk
endif

endif  #is-vendor-board-platform
