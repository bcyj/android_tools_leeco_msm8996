/*
 *Copyright (C) 2013-2014 Qualcomm Technologies, Inc. All rights reserved.
 *        Qualcomm Technologies Proprietary and Confidential.
 *
 *qcom_system_daemon.h : Header file for qcom-system-daemon
 */
#include "msg.h"
#include "diag_lsm.h"
#include "diagpkt.h"
#include "diagcmd.h"
#include "subsystem_control.h"
#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "QCOMSysDaemon"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <cutils/log.h>
#include <cutils/properties.h>
#include <cutils/android_reboot.h>
#include <errno.h>
#include <pthread.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/reboot.h>
#include "qmi_cci_target.h"
#include "qmi_client.h"
#include "qmi_idl_lib.h"
#include "qmi_cci_common.h"
#include "persistent_device_configuration_v01.h"

#define CONFIG_FRAME_SIZE 1500
#define QMI_PDC_SEND_MSG_TIME_OUT 500

/* FTM MODE ID 75 11 53*/
#define FTM_FFBM_CMD_CODE	53
#define EDL_RESET_CMD_CODE	1

#define MISC_PARTITION_LOCATION_ALT "/dev/block/platform/msm_sdcc.1/by-name/misc"
#define MISC_PARTITION_LOCATION "/dev/block/bootdevice/by-name/misc"
#define BLOCK_DEVICE_NODE "/dev/block/bootdevice/by-name/bootselect"
#define BLOCK_DEVICE_NODE_ALT "/dev/block/platform/msm_sdcc.1/by-name/bootselect"
#define MODE_FFBM "ffbm-01"
#define MODE_NORMAL "normal"
#define MBN_PATH_MAX_SIZE 128

#define RET_SUCCESS 0
#define RET_FAILED 1

#define REBOOT_CMD "reboot"
#define EDL_REBOOT_CMD "edl-reboot"
#define SWITCH_OS_CMD "switchos"
#define FFBM_COMMAND_BUFFER_SIZE 20

/* Subsystem command codes for image version */

#define VERSION_DIAGPKT_PROCID            0x80              // VERSION_PROCID 128
#define VERSION_DIAGPKT_SUBSYS            0x63              // VERSION_SUBSYS 99
#define VERSION_DIAGPKT_PREFIX            0x00              // VERSION_PREFIX 0

#define SELECT_IMAGE_FILE		"/sys/devices/soc0/select_image"
#define IMAGE_VERSION_FILE		"/sys/devices/soc0/image_version"
#define IMAGE_VARIANT_FILE		"/sys/devices/soc0/image_variant"
#define IMAGE_CRM_VERSION_FILE		"/sys/devices/soc0/image_crm_version"
#define SOS_FIFO			"/data/app/qsysdaemon"
#define BOOTSELECT_FACTORY		(1 << 30)
#define BOOTSELECT_SIGNATURE		('B' | ('S' << 8) | ('e' << 16) | ('l' << 24))
#define BOOTSELECT_VERSION		0x00010001

/* Size of version table stored in smem */
#define VERSION_TABLE_S 4096
#define IMAGE_VERSION_SINGLE_BLOCK_SIZE 128
#define IMAGE_VERSION_NAME_SIZE 75
#define IMAGE_VERSION_VARIANT_SIZE 20
#define IMAGE_VERSION_OEM_SIZE 32

/* QDSS defines */
#define QDSSDIAG_PROCESSOR_APPS   0x0100
#define QDSS_DIAG_PROC_ID QDSSDIAG_PROCESSOR_APPS

#define QDSS_QUERY_STATUS              0x00
#define QDSS_TRACE_SINK                0x01
#define QDSS_FILTER_ETM                0x02
#define QDSS_FILTER_STM                0x03
#define QDSS_FILTER_HWEVENT_ENABLE     0x04

#define QDSS_FILTER_HWEVENT_CONFIGURE  0x31

#define   TMC_TRACESINK_ETB   0
#define   TMC_TRACESINK_RAM   1
#define   TMC_TRACESINK_TPIU  2
#define   TMC_TRACESINK_USB   3
#define   TMC_TRACESINK_USB_BUFFERED   4
#define   TMC_TRACESINK_SD    6

#define QDSS_RSP_SUCCESS  0
#define QDSS_RSP_FAIL  1

#define QDSS_ETB_SINK_FILE "/sys/bus/coresight/devices/coresight-tmc-etf/curr_sink"
#define QDSS_ETR_SINK_FILE "/sys/bus/coresight/devices/coresight-tmc-etr/curr_sink"
#define QDSS_ETR_OUTMODE_FILE "/sys/bus/coresight/devices/coresight-tmc-etr/out_mode"
#define QDSS_TPIU_SINK_FILE "/sys/bus/coresight/devices/coresight-tpiu/curr_sink"
#define QDSS_TPIU_OUTMODE_FILE "/sys/bus/coresight/devices/coresight-tpiu/out_mode"
#define QDSS_STM_FILE "/sys/bus/coresight/devices/coresight-stm/enable"
#define QDSS_HWEVENT_FILE "/sys/bus/coresight/devices/coresight-hwevent/enable"
#define QDSS_STM_HWEVENT_FILE "/sys/bus/coresight/devices/coresight-stm/hwevent_enable"
#define QDSS_HWEVENT_SET_REG_FILE "/sys/bus/coresight/devices/coresight-hwevent/setreg"

typedef enum
{
	PDC_NO_ERROR = 0,	/* Success */
	PDC_NO_SERVICE,	    /* Invalid parameters */
	PDC_INIT_FAIL,
	PDC_ERROR,
	PDC_STATUS_MAX = 0x7FFFFFFF,   /* force 32 bite. */
} pdc_err_status;

typedef struct
{
	uint8_t config_id[PDC_CONFIG_ID_SIZE_MAX_V01];
	uint32_t config_id_len;
	uint32_t conf_size;
	uint32_t load_size;
	int conf_fd;
	pthread_mutex_t        pdc_config_mutex;
	pthread_mutexattr_t    pdc_config_mutex_attr;
} pdc_config_info_type;

typedef enum
{
	FTM_FFBM_SET_MODE   =     0,
	FTM_FFBM_GET_MODE   =     1,
	FTM_FFBM_SET_REGIONAL_PACK =    2,
	FTM_FFBM_GET_REGIONAL_PACK =    3,
	FTM_FFBM_HANDLE_PDC =    4
}FFBM_CMD_CODE;

typedef enum
{
	FTM_FFBM_SUCCESS = 0,
	FTM_FFBM_FAIL = 1
}FTM_ERROR_CODE;

typedef enum
{
	BOOT_MODE_HLOS = 0,
	BOOT_MODE_FFBM = 1
}BOOT_MODE;

typedef enum
{
	PDC_CONFIG_LOAD,
	PDC_CONFIG_SET_SELECT,
	PDC_CONFIG_ACTIVATE,
	PDC_CONFIG_LOAD_SELECT_ACTIVATE
}PDC_OPERATION_TYPE;

typedef PACKED struct
{
	diagpkt_cmd_code_type              cmd_code;
	diagpkt_subsys_id_type             subsys_id;
	diagpkt_subsys_cmd_code_type       subsys_cmd_code;
	uint16                             ffbm_cmd_code;
	uint16                             reserved;
	uint16                             reserved1;
}__attribute__((packed))ffbm_pkt_type;

typedef PACKED struct
 {
        ffbm_pkt_type       ftm_header;
        uint8		    regional_pack_id;
        uint8               mbn_operation_type;
 }__attribute__((packed))ffbm_set_mbn_req_type;

typedef PACKED struct
 {
        ffbm_pkt_type       ftm_header;
        uint16              iFTM_Error_Code;
 }__attribute__((packed))ffbm_set_mbn_rsp_type;

 /* FFBM Set Regional Packet command request packet */
typedef PACKED struct
{
       ffbm_pkt_type        ftm_header;
       uint8                regional_pack_id;
}__attribute__((packed))ffbm_set_region_pack_req_type;

typedef PACKED struct
{
       ffbm_pkt_type        ftm_header;
       uint16               iFTM_Error_Code;
}__attribute__((packed))ffbm_set_region_pack_rsp_type;

typedef PACKED struct
{
       ffbm_pkt_type        ftm_header;
       uint16               iFTM_Error_Code;
       uint8                current_regional_pack_id;
       uint8                next_regional_pack_id;
}__attribute__((packed))ffbm_get_region_pack_rsp_type;

/* FFBM Set Mode command request packet */
typedef PACKED struct
{
	ffbm_pkt_type        ftm_header;
	uint8                iNextBootMode;
	uint8                iNextBootSubMode;
}__attribute__((packed))ffbm_set_mode_req_type;

/* FFBM Set Mode command respond packet */
typedef PACKED struct
{
	ffbm_pkt_type        ftm_header;
	uint16               iFTM_Error_Code;
}__attribute__((packed))ffbm_set_mode_rsq_type;

/* FFBM Get Mode command request packet */
typedef PACKED struct
{
	ffbm_pkt_type        ftm_header;
	uint16               iFTM_Error_Code;
	uint8                iCurrentBootMode;
	uint8                iCurrentBootSubMode;
	uint8                iNextBootMode;
	uint8                iNextBootSubMode;
}__attribute__((packed))ffbm_get_mode_rsq_type;
/* bootselect partition format structure */
typedef struct {
	uint32_t signature;                // Contains value BOOTSELECT_SIGNATURE defined above
	uint32_t version;
	uint32_t boot_partition_selection; // Decodes which partitions to boot: 0-Windows,1-Android
	uint32_t state_info;               // Contains factory and format bit as definded above
} boot_selection_info;

void * ffbm_dispatch(ffbm_pkt_type *ffbm_pkt);
void * ffbm_get_mode();
void * ffbm_set_mode(ffbm_set_mode_req_type *ffbm_pkt);
void * ffbm_set_regional_pack(ffbm_set_region_pack_req_type *ffbm_pkt);
void * ffbm_get_regional_pack();

/* QDSS */
typedef PACK(struct)
{
  uint8 cmdCode;        // Diag Message ID
  uint8 subsysId;       // Subsystem ID (DIAG_SUBSYS_QDSS)
  uint16 subsysCmdCode; // Subsystem command code
} qdss_diag_pkt_hdr;

typedef PACK(struct)
{
  qdss_diag_pkt_hdr hdr;
} qdss_diag_pkt_req;

typedef PACK(struct)
{
  qdss_diag_pkt_hdr hdr; // Header
  uint8 result;          //See QDSS_CMDRESP_... definitions
} qdss_diag_pkt_rsp;

typedef PACK(struct)
{
  qdss_diag_pkt_hdr hdr;
  uint8 trace_sink;
} qdss_trace_sink_req;

typedef qdss_diag_pkt_rsp qdss_trace_sink_rsp; //generic response

typedef PACK(struct)
{
  qdss_diag_pkt_hdr hdr;
  uint8  state;
} qdss_filter_etm_req;

typedef qdss_diag_pkt_rsp qdss_filter_etm_rsp;

typedef PACK(struct)
{
  qdss_diag_pkt_hdr hdr;
  uint8  state;
} qdss_filter_stm_req;

typedef qdss_diag_pkt_rsp qdss_filter_stm_rsp;

typedef PACK(struct)
{
  qdss_diag_pkt_hdr hdr;
  uint8  state;
} qdss_filter_hwevents_req;

typedef qdss_diag_pkt_rsp qdss_filter_hwevents_rsp;

typedef PACK(struct)
{
  qdss_diag_pkt_hdr hdr;
  uint32 register_addr;
  uint32 register_value;
} qdss_filter_hwevents_configure_req;

typedef qdss_diag_pkt_rsp qdss_filter_hwevents_configure_rsp;

typedef PACK(struct)
{
  qdss_diag_pkt_hdr hdr;
} qdss_query_status_req;

typedef PACK(struct)
{
  qdss_diag_pkt_hdr hdr;
  uint8 trace_sink;
  uint8 stm_enabled;
  uint8 hw_events_enabled;
} qdss_query_status_rsp;

