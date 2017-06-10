/******************************************************************************
  ---------------------------------------------------------------------------

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
******************************************************************************/

#ifndef CRI_NAS
#define CRI_NAS

#include "utils_common.h"
#include "cri_core.h"
#include "cri_data.h"
#include "network_access_service_v01.h"

#define CRI_NAS_MCC_MNC_MAX (3)
#define CRI_NAS_LAC_MAX (10)
#define CRI_NAS_CID_MAX (10)
#define CRI_NAS_PSC_MAX (10)
#define CRI_NAS_TAC_MAX (10)
#define CRI_NAS_SID_MAX (10)
#define CRI_NAS_NID_MAX (10)
#define CRI_NAS_IS856_MAX (16)
#define CRI_NAS_DEF_ROAM_IND_MAX (5)

#define CRI_NAS_MCC_MNC_SIZE 6

#define CRI_NAS_ECIO_DELTA   20
#define CRI_NAS_RSSI_DELTA   10
#define CRI_NAS_IO_DELTA     10
#define CRI_NAS_SNR_DELTA    10
#define CRI_NAS_SINR_DELTA   10
#define CRI_NAS_RSRP_DELTA   50
#define CRI_NAS_RSCP_DELTA   50
#define CRI_NAS_RSRQ_DELTA   50


#define CRI_NAS_OPERATOR_MAX_EONS_LEN (512)
#define CRI_NAS_OPERATOR_MAX_MCC_MNC_LEN (16)

#define CRI_NAS_3GPP_NETWORK_INFO_LIST_MAX 40

#define QMI_CODING_SCHEME_CELL_BROADCAST_DATA 0x00
#define QMI_CODING_SCHEME_UCS2                0x01


#define CRI_NAS_NW_SCAN_RES_ENTRY_PREFERRED              ( (uint8_t) 1 << 6 )
#define CRI_NAS_NW_SCAN_RES_ENTRY_NOT_PREFERRED          ( (uint8_t) 1 << 7 )
#define CRI_NAS_NW_SCAN_RES_ENTRY_FORBIDDEN              ( (uint8_t) 1 << 4 )
#define CRI_NAS_NW_SCAN_RES_ENTRY_NOT_FORBIDDEN          ( (uint8_t) 1 << 5 )
#define CRI_NAS_NW_SCAN_RES_ENTRY_HOME                   ( (uint8_t) 1 << 2 )
#define CRI_NAS_NW_SCAN_RES_ENTRY_ROAM                   ( (uint8_t) 1 << 3 )
#define CRI_NAS_NW_SCAN_RES_ENTRY_CUR_SERVING            ( (uint8_t) 1 << 0 )
#define CRI_NAS_NW_SCAN_RES_ENTRY_AVAILABLE              ( (uint8_t) 1 << 1 )


#define NAS_RADIO_IF_NONE           0x00 // - None (no service)
#define NAS_RADIO_IF_CDMA2000       0x01 // - cdma2000 1X
#define NAS_RADIO_IF_CDMA2000_HRPD  0x02 // - cdma2000 HRPD (1xEV-DO)
#define NAS_RADIO_IF_AMPS           0x03 // - AMPS
#define NAS_RADIO_IF_GSM            0x04 // - GSM
#define NAS_RADIO_IF_UMTS           0x05 // - UMTS
#define NAS_RADIO_IF_LTE            0x08 // - LTE
#define NAS_RADIO_IF_TDSCDMA        0x09 // - TDSCDMA



#define CRI_NAS_MANAGED_ROAMING_CAUSE (10)

typedef enum {
    CRI_NAS_RTE_UNKNOWN = 0,
    CRI_NAS_RTE_GSM,
    CRI_NAS_RTE_WCDMA,
    CRI_NAS_RTE_TDSCDMA,
    CRI_NAS_RTE_LTE,
    CRI_NAS_RTE_CDMA,
    CRI_NAS_RTE_HDR,
    CRI_NAS_RTE_MAX
} cri_nas_rte_type;

typedef enum {
    CRI_NAS_RTE_SERVICE_VOICE = 0,
    CRI_NAS_RTE_SERVICE_DATA,
    CRI_NAS_RTE_SERVICE_MAX
} cri_nas_rte_service_type;

typedef enum {
    CRI_NAS_RTE_REG_CONFIDENCE_UNKNOWN = 0,
    CRI_NAS_RTE_REG_CONFIDENCE_FULL,
    CRI_NAS_RTE_REG_CONFIDENCE_LIMITED,
    CRI_NAS_RTE_REG_CONFIDENCE_NONE
} cri_nas_rte_reg_confidence_type;


typedef enum {
    CRI_NAS_TECH_UNKNOWN = 0,
    CRI_NAS_TECH_GSM,
    CRI_NAS_TECH_GPRS,
    CRI_NAS_TECH_EDGE,
    CRI_NAS_TECH_WCDMA,
    CRI_NAS_TECH_TDSCDMA,
    CRI_NAS_TECH_HSDPA,
    CRI_NAS_TECH_HSUPA,
    CRI_NAS_TECH_HSPA,
    CRI_NAS_TECH_HSPA_PLUS,
    CRI_NAS_TECH_LTE,
    CRI_NAS_TECH_IS95A,
    CRI_NAS_TECH_IS95B,
    CRI_NAS_TECH_1xRTT,
    CRI_NAS_TECH_EVDO_0,
    CRI_NAS_TECH_EVDO_A,
    CRI_NAS_TECH_EVDO_B,
    CRI_NAS_TECH_EHRPD
} cri_nas_tech_type;


typedef enum
{
    CRI_NAS_TECH_REG_STATUS_UNKNOWN = 0,
    CRI_NAS_TECH_REG_STATUS_REGISTERED_HOME,
    CRI_NAS_TECH_REG_STATUS_REGISTERED_ROAMING,
    CRI_NAS_TECH_REG_STATUS_NOT_REGISTERED_LTD_SRV,
    CRI_NAS_TECH_REG_STATUS_NOT_REGISTERED_SEARCHING,
    CRI_NAS_TECH_REG_STATUS_DENIED_LTD_SRV,
    CRI_NAS_TECH_REG_STATUS_DENIED,
    CRI_NAS_TECH_REG_STATUS_NOT_REGISTERED_NOT_SEARCHING
} cri_nas_tech_reg_status_type;


typedef enum
{
    CRI_NAS_NW_SELECT_NONE = 0,
    CRI_NAS_NW_SELECT_SETTING_PREF,
    CRI_NAS_NW_SELECT_SETTING_PREF_REGD,
    CRI_NAS_NW_SELECT_SETTING_PREF_LIMITED_REGD,
    CRI_NAS_NW_SELECT_AWAITING_REG,
    CRI_NAS_NW_SELECT_CANCELLING_REQ,
    CRI_NAS_NW_SELECT_ROLLBACK,
    CRI_NAS_NW_SELECT_DONE
} cri_nas_nw_select_state_type;

typedef enum
{
    CRI_NAS_ROAMING_PREF_OFF = 0,
    CRI_NAS_ROAMING_PREF_NOT_OFF,
    CRI_NAS_ROAMING_PREF_ANY,
} cri_nas_nw_roaming_pref_type;


typedef struct cri_nas_rte_common_reg_info_type
{
    cri_nas_rte_reg_confidence_type reg_confidence;
    cri_nas_tech_type tech;
    cri_nas_tech_reg_status_type reg_status;

    int roam_status;
    int reg_reject_reason;

    char mcc[CRI_NAS_MCC_MNC_MAX + 1];
    char mnc[CRI_NAS_MCC_MNC_MAX + 1];
}cri_nas_rte_common_reg_info_type;

typedef struct cri_nas_rte_gsm_reg_info_type
{
    cri_nas_rte_common_reg_info_type reg_info;
    char lac[CRI_NAS_LAC_MAX + 1];
    char cid[CRI_NAS_CID_MAX + 1];
    int8_t gsm_sig_info;
    nas_cell_access_status_e_type_v01 cell_access_status;
}cri_nas_rte_gsm_reg_info_type;

typedef struct cri_nas_rte_wcdma_reg_info_type
{
    cri_nas_rte_common_reg_info_type reg_info;
    char lac[CRI_NAS_LAC_MAX + 1];
    char cid[CRI_NAS_CID_MAX + 1];
    char psc[CRI_NAS_PSC_MAX + 1];
    nas_common_sig_info_param_type_v01 wcdma_sig_info;
    nas_cell_access_status_e_type_v01 cell_access_status;
}cri_nas_rte_wcdma_reg_info_type;

typedef struct cri_nas_rte_tdscdma_reg_info_type
{
    cri_nas_rte_common_reg_info_type reg_info;
    char lac[CRI_NAS_LAC_MAX + 1];
    char cid[CRI_NAS_CID_MAX + 1];
    nas_tdscdma_sig_info_type_v01 tdscdma_sig_info;
    nas_cell_access_status_e_type_v01 cell_access_status;
}cri_nas_rte_tdscdma_reg_info_type;

typedef struct cri_nas_rte_lte_reg_info_type
{
    cri_nas_rte_common_reg_info_type reg_info;
    char lac[CRI_NAS_LAC_MAX + 1];
    char cid[CRI_NAS_CID_MAX + 1];
    char tac[CRI_NAS_TAC_MAX + 1];
    nas_lte_sig_info_type_v01 lte_sig_info;
    nas_cell_access_status_e_type_v01 cell_access_status;
}cri_nas_rte_lte_reg_info_type;

typedef struct cri_nas_rte_cdma_reg_info_type
{
    cri_nas_rte_common_reg_info_type reg_info;
    char sid[CRI_NAS_SID_MAX + 1];
    char nid[CRI_NAS_NID_MAX + 1];
    char def_roam_ind[CRI_NAS_DEF_ROAM_IND_MAX + 1];
    nas_common_sig_info_param_type_v01 cdma_sig_info;
}cri_nas_rte_cdma_reg_info_type;

typedef struct cri_nas_rte_hdr_reg_info_type
{
    cri_nas_rte_common_reg_info_type reg_info;
    char is856_sys_id[CRI_NAS_IS856_MAX + 1];
    char def_roam_ind[CRI_NAS_DEF_ROAM_IND_MAX + 1];
    nas_hdr_sig_info_type_v01 hdr_sig_info;
}cri_nas_rte_hdr_reg_info_type;


typedef struct cri_nas_nw_selection_info_type
{
    uint8_t is_automatic;
    uint32_t mcc;
    uint32_t mnc;
    uint32_t rat;
    cri_nas_nw_select_state_type curr_state;
    cri_nas_nw_select_state_type final_state;
    cri_core_error_type error;
}cri_nas_nw_selection_info_type;


typedef struct
{
    uint8_t is_automatic;
    uint32_t mcc;
    uint32_t mnc;
    uint32_t rat;
}cri_nas_nw_selection_request_type;


qmi_error_type_v01 cri_nas_init_client(hlos_ind_cb_type hlos_ind_cb);
void cri_nas_release_client(int qmi_service_client_id);
void cri_nas_async_resp_handler(int qmi_service_client_id,
                                unsigned long message_id,
                                void *resp_data,
                                int resp_data_len,
                                cri_core_context_type cri_core_context);
void cri_nas_unsol_ind_handler(int qmi_service_client_id,
                                unsigned long message_id,
                                void *ind_data,
                                int ind_data_len);

void* cri_nas_retrieve_voice_rte(cri_nas_rte_type *rte);
void* cri_nas_retrieve_data_rte(cri_nas_rte_type *rte);
void cri_nas_update_data_system_status();

#endif
