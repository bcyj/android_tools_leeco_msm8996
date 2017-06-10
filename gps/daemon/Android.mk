ifneq ($(TARGET_NO_RPC),true)

#Temporarily disable gps Daemon for this release
ifeq ($(FEATURE_GNSS_BIT_API), true)

LOCAL_PATH:=$(call my-dir)

ifeq ($(call is-board-platform,msm8660),true)
  MODEM_APIS_DIR := msm8660_surf
else
  MODEM_APIS_DIR := $(TARGET_BOARD_PLATFORM)
endif

GPSONE_BIT_API_DIR = ../../modem-apis/$(MODEM_APIS_DIR)/api/libs/remote_apis/gpsone_bit_api

# test_loc_api_client
#

include $(CLEAR_VARS)
LOC_API_DIR = ../../../../../hardware/qcom/gps/loc_api/libloc_api_50001

LOCAL_CFLAGS+= \
        -DDEBUG_DMN_LOC_API -D_ANDROID_

LOCAL_C_INCLUDES:=$(LOCAL_PATH)
LOCAL_C_INCLUDES+= \
        $(TARGET_OUT_HEADERS)/common/inc \
        $(TARGET_OUT_HEADERS)/oncrpc/inc \
        $(TARGET_OUT_HEADERS)/diag/include \
        $(LOCAL_PATH)/$(GPSONE_BIT_API_DIR)/rpc/inc \
        $(LOCAL_PATH)/$(GPSONE_BIT_API_DIR)/inc \
        $(LOCAL_PATH)/$(LOC_API_DIR)

LOCAL_SRC_FILES:= \
        test/test_loc_api_main.cpp \
        $(LOC_API_DIR)/loc_eng_dmn_conn.cpp \
        $(LOC_API_DIR)/loc_eng_dmn_conn_handler.cpp \
        $(LOC_API_DIR)/loc_eng_dmn_conn_thread_helper.c \
        $(LOC_API_DIR)/loc_eng_dmn_conn_glue_msg.c \
        $(LOC_API_DIR)/loc_eng_dmn_conn_glue_pipe.c

LOCAL_SHARED_LIBRARIES := \
        libutils

LOCAL_MODULE:=test_loc_api_client
LOCAL_MODULE_TAGS := optional
# include $(BUILD_EXECUTABLE)

# test_bit
#

include $(CLEAR_VARS)

# FEATURE_GNSS_BIT_API is used by gpsone_bit_api.c only
LOCAL_CFLAGS+= \
        -D_ANDROID_ \
        -DFEATURE_GNSS_BIT_API

LOCAL_C_INCLUDES:=$(LOCAL_PATH)
LOCAL_C_INCLUDES+= \
        $(LOCAL_PATH)/test/platform \
        $(TARGET_OUT_HEADERS)/common/inc \
        $(TARGET_OUT_HEADERS)/oncrpc/inc \
        $(TARGET_OUT_HEADERS)/diag/include \
        $(TARGET_OUT_HEADERS)/qmi/inc \
        $(LOCAL_PATH)/../loc_api/libloc-rpc/rpc_inc \
        $(LOCAL_PATH)/bit_api/rpc/inc \
        $(LOCAL_PATH)/$(GPSONE_BIT_API_DIR)/rpc/inc \
        $(LOCAL_PATH)/$(GPSONE_BIT_API_DIR)/inc \
        $(LOCAL_PATH)/test/bit_api/inc \
        $(LOCAL_PATH)/test/platform \
        $(LOCAL_PATH)/$(PDAPI_DIR)/inc

LOCAL_SRC_FILES+= \
        test/test_bit_main.c \
        test/test_bit_cases.c \
        test/test_script_parser.c \
        gpsone_thread_helper.c \
        test/bit_api/src/gpsone_bit_api.c \
        test/bit_api/rpc/src/gpsone_bit_api_svc.c \
        $(GPSONE_BIT_API_DIR)/src/gpsone_bit_api_xdr.c

LOCAL_SHARED_LIBRARIES := \
    liboncrpc

LOCAL_MODULE:=test_bit
LOCAL_MODULE_TAGS := optional
# include $(BUILD_EXECUTABLE)

# gpsone_daemon
#

include $(CLEAR_VARS)

LOCAL_CFLAGS+= \
        -D_ANDROID_

LOCAL_C_INCLUDES:=$(LOCAL_PATH)
LOCAL_C_INCLUDES+= \
        $(TARGET_OUT_HEADERS)/common/inc \
        $(TARGET_OUT_HEADERS)/oncrpc/inc \
        $(TARGET_OUT_HEADERS)/diag/include \
        $(TARGET_OUT_HEADERS)/qmi/inc \
        $(TARGET_OUT_HEADERS)/data/inc \
        $(LOCAL_PATH)/../loc_api/libloc-rpc/rpc_inc \
        $(LOCAL_PATH)/$(GPSONE_BIT_API_DIR)/rpc/inc \
        $(LOCAL_PATH)/$(GPSONE_BIT_API_DIR)/inc \
        $(LOCAL_PATH)/$(PDAPI_DIR)/inc \
        $(LOCAL_PATH)/$(PDSM_ATL_DIR)/inc

LOCAL_SRC_FILES+= \
    gpsone_daemon_manager.c \
    gpsone_daemon_manager_handler.c \
    gpsone_net.c \
    gpsone_conn_bridge.c \
    gpsone_conn_bridge_proc.c \
    gpsone_udp_modem.c \
    gpsone_udp_modem_proc.c \
    gpsone_thread_helper.c \
    gpsone_bit_forward.c \
    gpsone_glue_data_service.c \
    gpsone_conn_client.c \
    gpsone_glue_msg.c \
    gpsone_glue_pipe.c \
    gpsone_glue_rpc.c

LOCAL_SHARED_LIBRARIES := \
    libutils \
    liboncrpc \
    libdsi_netctrl \
    libgpsone_bit_api \
    libqmi

LOCAL_MODULE:=gpsone_daemon
LOCAL_MODULE_TAGS := optional
include $(BUILD_EXECUTABLE)

# test_agps_server
#

include $(CLEAR_VARS)

LOCAL_CFLAGS+= \
        -D_ANDROID_

LOCAL_C_INCLUDES:=$(LOCAL_PATH)
LOCAL_C_INCLUDES+= \
        $(TARGET_OUT_HEADERS)/common/inc \
        $(TARGET_OUT_HEADERS)/oncrpc/inc \
        $(TARGET_OUT_HEADERS)/diag/include \
        $(LOCAL_PATH)/../loc_api/libloc-rpc/rpc_inc \
        $(LOCAL_PATH)/$(GPSONE_BIT_API_DIR)/rpc/inc \
        $(LOCAL_PATH)/$(GPSONE_BIT_API_DIR)/inc \

LOCAL_SRC_FILES:= \
    test/test_agps_server.c \
    test/test_agps_server_handler.c \
    test/test_script_parser.c \
    test/test_socket_server_helper.c \
    gpsone_glue_data_service.c

LOCAL_SHARED_LIBRARIES := \
    libutils

LOCAL_MODULE:=test_agps_server
LOCAL_MODULE_TAGS := optional
# include $(BUILD_EXECUTABLE)

endif # FEATURE_GNSS_BIT_API

else
# not TARGET_NO_RPC := true
# QMI Compilation
ifneq ($(BUILD_TINY_ANDROID),true)

ifeq ($(FEATURE_GNSS_BIT_API), true)

LOCAL_PATH:=$(call my-dir)
#TODO: need to fix the depdendency below for RPC header file
GPSONE_BIT_API_DIR =qmi_local

include $(CLEAR_VARS)

LOCAL_CFLAGS += \
    -fno-short-enums \
    -D_ANDROID_ \
    -DFEATURE_QMI \
    -DFEATURE_DORMANCY_DISABLE
#TODO: Need to fix Dormancy issue in Data Services

LOCAL_C_INCLUDES:=$(LOCAL_PATH)
LOCAL_C_INCLUDES+= \
        $(TARGET_OUT_HEADERS)/common/inc \
        $(TARGET_OUT_HEADERS)/diag/include \
        $(TARGET_OUT_HEADERS)/data/inc \
        $(LOCAL_PATH)/$(GPSONE_BIT_API_DIR) \
        $(TOP)/vendor/qcom/proprietary/qmi-framework/inc \
        $(TOP)/vendor/qcom/proprietary/qmi-framework/qcci/inc \
        $(TOP)/vendor/qcom/proprietary/qmi-framework/common/inc \
        $(TARGET_OUT_HEADERS)/qmi/inc \
        $(TARGET_OUT_HEADERS)/libloc_eng \
        $(TARGET_OUT_HEADERS)/libloc_core \
        $(LOCAL_PATH)/test/platform


LOCAL_SRC_FILES+= \
    gpsone_daemon_manager.c \
    gpsone_daemon_manager_handler.c \
    gpsone_net.c \
    gpsone_conn_bridge.c \
    gpsone_conn_bridge_proc.c \
    gpsone_udp_modem.c \
    gpsone_udp_modem_proc.c \
    gpsone_thread_helper.c \
    gpsone_bit_forward_qmi.c \
    gpsone_glue_data_service.c \
    gpsone_conn_client.c \
    gpsone_glue_msg.c \
    gpsone_glue_pipe.c \
    gpsone_glue_qmi.c \
    bearer_independent_transport_v01.c

LOCAL_SHARED_LIBRARIES := \
    libutils \
    libdsi_netctrl \
    libcutils \
    libqmi_cci \
    libqmi_csi \
    libqmi_common_so \
    libqmi

LOCAL_MODULE:=gpsone_daemon
LOCAL_MODULE_TAGS := optional
include $(BUILD_EXECUTABLE)

endif #GNSS BIT API feature

endif #BUILD_TINY_ANDROID

endif #QMI Compilation
