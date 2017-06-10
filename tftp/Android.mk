
LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE		:= tftp_server
LOCAL_MODULE_TAGS	:= optional

LOCAL_CFLAGS  := -Wall -Werror
LOCAL_CFLAGS  += -DFEATURE_TFTP_SERVER_BUILD -DTFTP_LA_BUILD
LOCAL_CFLAGS  += -DTFTP_ENABLE_DEBUG_MODE
LOCAL_CFLAGS  += -DTFTP_LOG_CONFIG_ENABLE_ERR_LOGGING
LOCAL_CFLAGS  += -DTFTP_LOG_CONFIG_ENABLE_WARN_LOGGING
LOCAL_CFLAGS  += -DTFTP_LOG_CONFIG_ENABLE_DEBUG_LOGGING
LOCAL_CFLAGS  += -DTFTP_LOG_CONFIG_ENABLE_INFO_LOGGING
LOCAL_CFLAGS  += -DTFTP_LOG_CONFIG_ENABLE_LINE_NO_LOGGING
LOCAL_CFLAGS  += -DTFTP_LOG_CONFIG_ENABLE_CONSOLE_LOGGING
LOCAL_CFLAGS  += -g
LOCAL_CFLAGS  += -O0

LOCAL_C_INCLUDES	:=  \
				$(TARGET_OUT_HEADERS)/common/inc \
				$(LOCAL_PATH)/common/inc \
				$(LOCAL_PATH)/os/inc \
				$(LOCAL_PATH)/server/inc

LOCAL_SRC_FILES		+= common/src/tftp_connection.c \
				common/src/tftp_log.c \
				common/src/tftp_malloc.c \
				common/src/tftp_pkt.c \
				common/src/tftp_protocol.c \
				os/src/tftp_file.c \
				os/src/tftp_msg_la.c \
				os/src/tftp_os.c \
				os/src/tftp_os_la.c \
				os/src/tftp_os_wakelocks_la.c \
				os/src/tftp_socket_ipcr_la.c \
				os/src/tftp_string_la.c \
				os/src/tftp_threads_la.c \
				os/src/tftp_utils_la.c \
				server/src/tftp_server.c \
				server/src/tftp_server_folders_la.c \
				server/src/tftp_server_main.c \
				server/src/tftp_server_utils.c

LOCAL_PRELINK_MODULE	:= false
LOCAL_SHARED_LIBRARIES	:= libqmi_csi libqmi_common_so libcutils

LOCAL_MODULE_OWNER := qti
include $(BUILD_EXECUTABLE)

#########################################################################
# Create Folder Structure
#########################################################################

$(shell rm -rf $(TARGET_OUT)/rfs/)

#To be enabled when prepopulation support is needed for the read_write folder
# $(shell rm -rf  $(TARGET_OUT_DATA)/rfs/)
# $(shell mkdir -p $(TARGET_OUT_DATA)/rfs/msm/mpss/)
# $(shell mkdir -p $(TARGET_OUT_DATA)/rfs/msm/adsp/)
# $(shell mkdir -p $(TARGET_OUT_DATA)/rfs/mdm/mpss/)
# $(shell mkdir -p $(TARGET_OUT_DATA)/rfs/mdm/adsp/)

#########################################################################
# MSM Folders
#########################################################################
$(shell mkdir -p $(TARGET_OUT)/rfs/msm/mpss/readonly)
$(shell mkdir -p $(TARGET_OUT)/rfs/msm/adsp/readonly)

$(shell ln -s /data/tombstones/modem $(TARGET_OUT)/rfs/msm/mpss/ramdumps)
$(shell ln -s /persist/rfs/msm/mpss $(TARGET_OUT)/rfs/msm/mpss/readwrite)
$(shell ln -s /persist/rfs/shared $(TARGET_OUT)/rfs/msm/mpss/shared)
$(shell ln -s /persist/hlos_rfs/shared $(TARGET_OUT)/rfs/msm/mpss/hlos)
$(shell ln -s /firmware $(TARGET_OUT)/rfs/msm/mpss/readonly/firmware)

$(shell ln -s /data/tombstones/lpass $(TARGET_OUT)/rfs/msm/adsp/ramdumps)
$(shell ln -s /persist/rfs/msm/adsp $(TARGET_OUT)/rfs/msm/adsp/readwrite)
$(shell ln -s /persist/rfs/shared $(TARGET_OUT)/rfs/msm/adsp/shared)
$(shell ln -s /persist/hlos_rfs/shared $(TARGET_OUT)/rfs/msm/adsp/hlos)
$(shell ln -s /firmware $(TARGET_OUT)/rfs/msm/adsp/readonly/firmware)

#########################################################################
# MDM Folders
#########################################################################
$(shell mkdir -p $(TARGET_OUT)/rfs/mdm/mpss/readonly)
$(shell mkdir -p $(TARGET_OUT)/rfs/mdm/adsp/readonly)
$(shell mkdir -p $(TARGET_OUT)/rfs/mdm/sparrow/readonly)

$(shell ln -s /data/tombstones/modem $(TARGET_OUT)/rfs/mdm/mpss/ramdumps)
$(shell ln -s /persist/rfs/mdm/mpss $(TARGET_OUT)/rfs/mdm/mpss/readwrite)
$(shell ln -s /persist/rfs/shared $(TARGET_OUT)/rfs/mdm/mpss/shared)
$(shell ln -s /persist/hlos_rfs/shared $(TARGET_OUT)/rfs/mdm/mpss/hlos)
$(shell ln -s /firmware $(TARGET_OUT)/rfs/mdm/mpss/readonly/firmware)

$(shell ln -s /data/tombstones/lpass $(TARGET_OUT)/rfs/mdm/adsp/ramdumps)
$(shell ln -s /persist/rfs/mdm/adsp $(TARGET_OUT)/rfs/mdm/adsp/readwrite)
$(shell ln -s /persist/rfs/shared $(TARGET_OUT)/rfs/mdm/adsp/shared)
$(shell ln -s /persist/hlos_rfs/shared $(TARGET_OUT)/rfs/mdm/adsp/hlos)
$(shell ln -s /firmware $(TARGET_OUT)/rfs/mdm/adsp/readonly/firmware)

$(shell ln -s /data/tombstones/sparrow $(TARGET_OUT)/rfs/mdm/sparrow/ramdumps)
$(shell ln -s /persist/rfs/mdm/sparrow $(TARGET_OUT)/rfs/mdm/sparrow/readwrite)
$(shell ln -s /persist/rfs/shared $(TARGET_OUT)/rfs/mdm/sparrow/shared)
$(shell ln -s /persist/hlos_rfs/shared $(TARGET_OUT)/rfs/mdm/sparrow/hlos)
$(shell ln -s /firmware $(TARGET_OUT)/rfs/mdm/sparrow/readonly/firmware)

#########################################################################
# APQ Folders
#########################################################################
$(shell mkdir -p $(TARGET_OUT)/rfs/apq/gnss/readonly)

$(shell ln -s /data/tombstones/modem $(TARGET_OUT)/rfs/apq/gnss/ramdumps)
$(shell ln -s /persist/rfs/apq/gnss $(TARGET_OUT)/rfs/apq/gnss/readwrite)
$(shell ln -s /persist/rfs/shared $(TARGET_OUT)/rfs/apq/gnss/shared)
$(shell ln -s /persist/hlos_rfs/shared $(TARGET_OUT)/rfs/apq/gnss/hlos)
$(shell ln -s /firmware $(TARGET_OUT)/rfs/apq/gnss/readonly/firmware)

