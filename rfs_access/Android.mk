ifeq ($(call is-board-platform-in-list,msm8974 msm8226 msm8610 apq8084 msm8916 msm8994),true)
LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE		:= rfs_access
LOCAL_MODULE_TAGS	:= optional

LOCAL_C_INCLUDES	:= $(TARGET_OUT_HEADERS)/qmi-framework/inc \
				$(TARGET_OUT_HEADERS)/common/inc \
				$(LOCAL_PATH)/src \
				$(LOCAL_PATH)/src/qmi \
				$(LOCAL_PATH)/src/util

LOCAL_SRC_FILES		+= src/rfsa_server.c \
				src/rfsa_vtl_server.c \
				src/qmi/rfsa_qmi_server.c \
				src/qmi/rfsa_v01.c \
				src/util/rfsa_event.c \
				src/util/rfsa_list.c \
				src/util/rfsa_lock.c \
				src/util/rfsa_thread.c

LOCAL_C_FLAGS		:= -Wall -DLOG_NIDEBUG=0

LOCAL_PRELINK_MODULE	:= false
LOCAL_SHARED_LIBRARIES	:= libqmi_csi libqmi_common_so libcutils

LOCAL_MODULE_OWNER := qcom
include $(BUILD_EXECUTABLE)

# Commenting out Folder creation logic from rfs_access, since tftp_server
# make file will now create the folder structure.

#$(shell rm -rf $(TARGET_OUT)/rfs/)

#$(shell mkdir -p $(TARGET_OUT)/rfs/msm/mpss/readonly)
#$(shell mkdir -p $(TARGET_OUT)/rfs/msm/adsp/readonly)

#To be enabled when prepopulation support is needed for the read_write folder
# $(shell rm -rf  $(TARGET_OUT_DATA)/rfs/)
# $(shell mkdir -p $(TARGET_OUT_DATA)/rfs/msm/mpss/)
# $(shell mkdir -p $(TARGET_OUT_DATA)/rfs/msm/adsp/)

#$(shell ln -s /data/tombstones/modem $(TARGET_OUT)/rfs/msm/mpss/ramdumps)
#$(shell ln -s /data/rfs/msm/mpss $(TARGET_OUT)/rfs/msm/mpss/readwrite)
#$(shell ln -s /data/rfs/shared $(TARGET_OUT)/rfs/msm/mpss/shared)
#$(shell ln -s /firmware $(TARGET_OUT)/rfs/msm/mpss/readonly/firmware)
#
#$(shell ln -s /data/tombstones/lpass $(TARGET_OUT)/rfs/msm/adsp/ramdumps)
#$(shell ln -s /data/rfs/msm/adsp $(TARGET_OUT)/rfs/msm/adsp/readwrite)
#$(shell ln -s /data/rfs/shared $(TARGET_OUT)/rfs/msm/adsp/shared)
#$(shell ln -s /firmware $(TARGET_OUT)/rfs/msm/adsp/readonly/firmware)

endif
