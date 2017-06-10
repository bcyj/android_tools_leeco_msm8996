CC = gcc
C++ = g++
FEATURE_QMI =1
BIN = gpsone_daemon test_agps_server test_udp_client
# since we are suing GARDEn for simulating HAL for ATL requests
# BIN += test_loc_api
#Since 8960 builds do not have gpsone_bit_api directories
TARGET_PRODUCT = msm8660_surf
# BIN += test_bit
ROOT_DIR = ../../../../../..
QC_PROP_DIR = $(ROOT_DIR)/vendor/qcom/proprietary
ifeq ($(FEATURE_QMI), 1)
#QMI specific
BIN += libbit_api_v01.so libqmi_common_so.so libqmi_cci.so libqmi_csi.so libqmi_encdec.so bit_svc
GPSONE_BIT_API_DIR = ../qmi_local
else
GPSONE_BIT_API_DIR = ../../../modem-apis/$(TARGET_PRODUCT)/api/libs/remote_apis/gpsone_bit_api/inc
endif
LOC_API_DIR = ../../../../../../hardware/qcom/gps/loc_api/libloc_api_50001
LOC_HAL_UTILS_DIR = ../../../../../../hardware/qcom/gps/utils
FRAMEWORKS_DIR = ../../../../../../frameworks/base/include/
QMI_ROOT = $(QC_PROP_DIR)/qmi
QMI_PLATFORM_ROOT = $(QMI_ROOT)/platform
QMI_ON_TARGET_ROOT = $(QC_PROP_DIR)/qmi-framework
QMI_QCSI_ON_TARGET_ROOT = $(QMI_ON_TARGET_ROOT)/qcsi
QMI_QCSI_OFF_TARGET_ROOT = $(QC_PROP_DIR)/gps-noship/GARDEn/qmi_loc_off_target/qcsi/linux
QMI_QCCI_ON_TARGET_ROOT = $(QMI_ON_TARGET_ROOT)/qcci
QMI_QCCI_OFF_TARGET_ROOT = $(QC_PROP_DIR)/gps-noship/GARDEn/qmi_loc_off_target/qcci/linux


# bit_api/inc/gpsone_bit_local_api.h
# bit_api/src/gpsone_bit_api.c
GPSONE_BIT_MODEM_DIR = bit_api

ifeq ($(FEATURE_QMI), 1)
PLATFORM_INC = \
          -I$(QMI_ROOT)/inc \

          -I$(QMI_PLATFORM_ROOT) \
          -I$(QMI_ON_TARGET_ROOT)/inc \
          -I$(QMI_ON_TARGET_ROOT)/common/inc \

          -I$(QMI_ON_TARGET_ROOT)/smem_log \
          -I$(QMI_QCSI_ON_TARGET_ROOT)/inc \
          -I$(QMI_QCCI_ON_TARGET_ROOT)/inc
CFLAG += -DFEATURE_QMI

endif

CFLAG = -I$(GPSONE_BIT_API_DIR) -I$(GPSONE_BIT_MODEM_DIR)/inc -I../ -I./ -I$(LOC_API_DIR) -Iplatform -I$(FRAMEWORKS_DIR) -DDEBUG_X86 -DDEBUG_DMN_LOC_API
CFLAG += -I$(LOC_HAL_UTILS_DIR) $(PLATFORM_INC)

LIBS = \
    -lm \
    -lc \
    -lpthread \
    -lrt \
    -lstdc++ \
	-ldl \

LIBS_LFLAGS = \
    -g \
    -L. \

LFLAG = -Wall -g -L.

LIBS_CFLAGS = \
    $(CFLAG) \
    -fPIC
# LFLAG = -m32 -Wall -g

RPC_CLIENT_OBJS = \
        $(GPSONE_BIT_API_DIR)/src/gpsone_bit_api_cb_clnt.o \
        $(GPSONE_BIT_API_DIR)/src/gpsone_bit_api_cb_xdr.o \
        $(GPSONE_BIT_API_DIR)/src/gpsone_bit_api_clnt.o \
        $(GPSONE_BIT_API_DIR)/src/gpsone_bit_api_xdr.o \
        $(GPSONE_BIT_API_DIR)/src/gpsone_bit_api_common_xdr.o

RPC_SERVER_OBJS =  \
        $(GPSONE_BIT_API_DIR)/src/gpsone_bit_api_cb_svc.o \
        $(GPSONE_BIT_API_DIR)/src/gpsone_bit_api_cb_xdr.o \
        $(GPSONE_BIT_API_DIR)/src/gpsone_bit_api_svc.o \
        $(GPSONE_BIT_API_DIR)/src/gpsone_bit_api_xdr.o

BIT_MODEM_OBJS = \
        $(GPSONE_BIT_MODEM_DIR)/src/gpsone_bit_api.o

all: $(BIN)

GPSONE_DAEMON_OBJS:= \
        ../gpsone_daemon_manager.o \
        ../gpsone_daemon_manager_handler.o \
        ../gpsone_net.o \
        ../gpsone_conn_bridge.o \
        ../gpsone_conn_bridge_proc.o \
        ../gpsone_udp_modem.o \
        ../gpsone_udp_modem_proc.o \
        ../gpsone_thread_helper.o \
        ../gpsone_glue_data_service.o \
        ../gpsone_conn_client.o \
        ../gpsone_glue_msg.o \
        ../gpsone_glue_pipe.o \

ifneq ($(FEATURE_QMI), 1)
GPSONE_DAEMON_OBJS += ../gpsone_bit_forward.o \
                      ../gpsone_glue_rpc.o
endif

TEST_AGPS_SERVER_OBJS:= \
        test_agps_server.o \
        test_agps_server_handler.o \
        test_script_parser.o \
        test_socket_server_helper.o \
        ../gpsone_glue_data_service.o

TEST_UDP_CLIENT_OBJS:= \
        test_udp_client.o

TEST_BIT_OBJS:= \
        test_bit_main.o \
        test_bit_cases.o \
        test_script_parser.o

TEST_LOC_API_OBJS:= \
        test_loc_api_main.o \
        $(LOC_API_DIR)/loc_eng_dmn_conn.o \
        $(LOC_API_DIR)/loc_eng_dmn_conn_handler.o \
        $(LOC_API_DIR)/loc_eng_dmn_conn_thread_helper.o \
        $(LOC_API_DIR)/loc_eng_dmn_conn_glue_msg.o \
        $(LOC_API_DIR)/loc_eng_dmn_conn_glue_pipe.o

LIB_QMI_ENC_DEC_OBJS = \
    $(QMI_ON_TARGET_ROOT)/encdec/qmi_idl_lib.o \
    $(QMI_ON_TARGET_ROOT)/encdec/qmi_idl_accessor.o

LIB_QMI_QCSI_OBJS = \
    $(QMI_QCSI_OFF_TARGET_ROOT)/../qmi_csi_common_fixed.o \
    $(QMI_QCSI_OFF_TARGET_ROOT)/qmi_csi_xport_udp.o \
    $(QMI_ON_TARGET_ROOT)/smem_log/smem_log.o

LIB_QMI_QCCI_OBJS = \
    $(QMI_QCCI_OFF_TARGET_ROOT)/../qmi_cci_common.o \
    $(QMI_QCCI_OFF_TARGET_ROOT)/qmi_cci_xport_udp.o \
    $(QMI_ON_TARGET_ROOT)/smem_log/smem_log.o

LIB_QMI_COMMON_OBJS = \
    $(QMI_ON_TARGET_ROOT)/common/src/common_v01.o

LIB_BIT_API_V01_OBJS = \
   ../gpsone_bit_forward_qmi.o \
   ../gpsone_glue_qmi.o \
   ../bearer_independent_transport_v01.o

BIT_SVC_OBJS = \
    qmi_bit_svc_udp_main.o \
    qmi_bit_svc.o \
    ../bearer_independent_transport_v01.o

GPSONE_DAEMON_OBJS += $(BIT_MODEM_OBJS)
GPSONE_DAEMON_OBJS += $(TEST_BIT_OBJS)

ifeq ($(FEATURE_QMI), 1)
gpsone_daemon: $(GPSONE_DAEMON_OBJS) libbit_api_v01.so
	$(CC) -lbit_api_v01 $(LFLAG) -lpthread -lrt -o $@ $(GPSONE_DAEMON_OBJS) -lbit_api_v01
else
gpsone_daemon: $(GPSONE_DAEMON_OBJS)
	$(CC) -Wall -g -lpthread -lrt -o $@ $(GPSONE_DAEMON_OBJS)
endif

test_bit: $(TEST_BIT_OBJS)
	$(CC) $(LFLAG) -o $@ $(TEST_BIT_OBJS) $(RPC_SERVER_OBJS)

test_loc_api: $(TEST_LOC_API_OBJS)
	$(C++) $(LFLAG) -lpthread -lrt -o $@ $(TEST_LOC_API_OBJS)

test_agps_server: $(TEST_AGPS_SERVER_OBJS)
	$(CC) $(LFLAG) -o $@ $(TEST_AGPS_SERVER_OBJS)

test_udp_client: $(TEST_UDP_CLIENT_OBJS)
	$(CC) $(LFLAG) -lpthread -L./ -o $@ $(TEST_UDP_CLIENT_OBJS)

bit_svc: $(BIT_SVC_OBJS) libqmi_cci.so
	$(C++) -o $@ -L. $(LIBS) $(BIT_SVC_OBJS) -lqmi_encdec -lqmi_csi -lqmi_common_so -lqmi_cci

libbit_api_v01.so: $(LIB_BIT_API_V01_OBJS) libqmi_cci.so libqmi_csi.so libqmi_common_so.so
	$(C++) $(LIBS_CFLAGS) -Xlinker -rpath . -shared -W1,-soname,libbit_api_v01.so -o $@ -L. $(LIBS_LFLAGS) $(LIBS) $(LIB_BIT_API_V01_OBJS) -lqmi_cci -lqmi_csi -lqmi_common_so

libqmi_common_so.so: $(LIB_QMI_COMMON_OBJS)
	$(C++) -fPIC -shared -W1,-soname,libqmi_common_so.so -o $@ -L. $(LIBS_LFLAGS) $(LIBS) $(LIB_QMI_COMMON_OBJS)

libqmi_cci.so: $(LIB_QMI_QCCI_OBJS) libqmi_common_so.so libqmi_encdec.so
	$(C++) $(LIBS_CFLAGS) -Xlinker -rpath . -fPIC -shared -W1,-soname,libqmi_cci.so -o $@ -L. $(LIBS_LFLAGS) $(LIBS) $(LIB_QMI_QCCI_OBJS) -lqmi_common_so -lqmi_encdec

libqmi_csi.so: $(LIB_QMI_QCSI_OBJS) libqmi_common_so.so libqmi_encdec.so
	$(C++) -fPIC -shared -W1,-soname,libqmi_csi.so -o $@ -L. $(LIBS_LFLAGS) $(LIBS) $(LIB_QMI_QCSI_OBJS) -lqmi_common_so -lqmi_encdec

libqmi_encdec.so: $(LIB_QMI_ENC_DEC_OBJS)
	$(C++) -fPIC -shared -W1,-soname,libqmi_encdec.so -o $@ -L. $(LIBS_LFLAGS) $(LIBS) $(LIB_QMI_ENC_DEC_OBJS)


# FEATURE_GNSS_BIT_API is used by gpsone_bit_api.c only
$(BIT_MODEM_OBJS) : $(patsubst %.o,%.c,$@)
	$(CC) $(LFLAG) $(CFLAG) -DFEATURE_GNSS_BIT_API -c $(patsubst %.o,%.c,$@) -o $@

%.o : %.c
	$(CC) $(LFLAG) -fPIC $(CFLAG) -c $(patsubst %.o,%.c,$@) -o $@

%.o : %.cpp
	$(C++) $(LFLAG) $(CFLAG) -c $(patsubst %.o,%.cpp,$@) -o $@

-include $(LIB_BIT_API_V01_OBJS:.o=.d)
-include $(LIB_QMI_COMMON_OBJS:.o=.d)
-include $(LIB_QMI_QCCI_OBJS:.o=.d)
-include $(LIB_QMI_QCSI_OBJS:.o=.d)
-include $(LIB_QMI_ENC_DEC_OBJS:.o=.d)
-include $(BIT_SVC_OBJS:.o=.d)

clean cleanup:
	rm -f $(BIN) $(GPSONE_DAEMON_OBJS) $(TEST_BIT_OBJS) $(TEST_LOC_API_OBJS) $(TEST_AGPS_SERVER_OBJS) $(TEST_UDP_CLIENT_OBJS) \
		$(LIB_QMI_COMMON_OBJS) $(patsubst %.o,%.d,$(LIB_QMI_COMMON_OBJS)) \
		$(LIB_QMI_QCCI_OBJS) $(patsubst %.o,%.d,$(LIB_QMI_QCCI_OBJS)) \
		$(LIB_QMI_QCSI_OBJS) $(patsubst %.o,%.d,$(LIB_QMI_QCSI_OBJS)) \
		$(LIB_QMI_ENC_DEC_OBJS) $(patsubst %.o,%.d,$(LIB_QMI_ENC_DEC_OBJS)) \
		$(LIB_BIT_API_V01_OBJS) $(patsubst %.o,%.d,$(LIB_BIT_API_V01_OBJS)) \
		$(BIT_SVC_OBJS) $(patsubst %.o,%.d,$(BIT_SVC_OBJS))

