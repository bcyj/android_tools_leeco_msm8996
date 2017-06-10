LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := libAniAsf
LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES +=  \
			aniAsfTimer.c \
			aniAsfIpc.c \
			aniAsfEvent.c \
			aniAsfLog.c \
			aniAsfProcessUtils.c \
			aniAsfUtils.c \
			aniAsfMisc.c  \
			aniAsfHashTable.c \
			aniAsfDict.c \
			aniAsfPacket.c \
		  aniAsfPortMap.c

LOCAL_CFLAGS += \
	-fno-short-enums 

LOCAL_C_INCLUDES:= \
	$(LOCAL_PATH)/../inc

LOCAL_PRELINK_MODULE := false

LOCAL_MODULE_OWNER := qcom

include $(BUILD_STATIC_LIBRARY)
