/******************************************************************************
  ---------------------------------------------------------------------------

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
******************************************************************************/

#include <stdlib.h>

#include "cri_data_core.h"
#include "cri_utils.h"

int data_client_id;
cri_data_system_status_info_type cri_data_core_radio_tech_info;

static qmi_error_type_v01 cri_data_init_client_state();
static void cri_data_cleanup_client_state();
static void cri_data_core_calculate_data_system_status(dsd_system_status_info_type_v01 *avail_sys);


int cri_data_core_retrieve_client_id()
{
    return data_client_id;
}

qmi_error_type_v01 cri_data_core_init_client(hlos_ind_cb_type hlos_ind_cb)
{
    qmi_error_type_v01 client_init_error;

    client_init_error = QMI_ERR_INTERNAL_V01;

    data_client_id = cri_core_create_qmi_service_client(QMI_DSD_SERVICE,
                                                        hlos_ind_cb);
    if(QMI_INTERNAL_ERR != data_client_id)
    {
        client_init_error = cri_data_init_client_state();
    }

    return client_init_error;
}

void cri_data_core_release_client(int qmi_service_client_id)
{
    cri_data_cleanup_client_state();
    cri_core_release_qmi_service_client(qmi_service_client_id);
    data_client_id = NIL;
}

qmi_error_type_v01 cri_data_core_fetch_system_status()
{
    qmi_error_type_v01 err_code;
    dsd_get_system_status_resp_msg_v01 *resp_msg;
    uint32_t iter_i;

    err_code = QMI_ERR_INTERNAL_V01;
    resp_msg = NULL;
    iter_i = 0;

    resp_msg = util_memory_alloc(sizeof(*resp_msg));

    if(resp_msg)
    {
        err_code = cri_core_qmi_send_msg_sync(cri_data_core_retrieve_client_id(),
                                              QMI_DSD_GET_SYSTEM_STATUS_REQ_V01,
                                              NULL,
                                              NIL,
                                              resp_msg,
                                              sizeof(*resp_msg),
                                              CRI_CORE_MINIMAL_TIMEOUT);

        if(CRI_ERR_NONE_V01 == cri_core_retrieve_err_code(err_code,
                                                          &resp_msg->resp))
        {
            UTIL_LOG_MSG( "available systems info valid %d",
                          resp_msg->avail_sys_valid );

            if( TRUE == resp_msg->avail_sys_valid )
            {
                UTIL_LOG_MSG("dsd_sys_status len=%d",
                             resp_msg->avail_sys_len);

                for(iter_i = 0; iter_i < resp_msg->avail_sys_len; iter_i++)
                {
                  UTIL_LOG_MSG("preferred %d - nw=0x%x, rat_value=0x%x, so_mask 0x%016llx",
                               (NIL == iter_i),
                               resp_msg->avail_sys[iter_i].technology,
                               resp_msg->avail_sys[iter_i].rat_value,
                               resp_msg->avail_sys[iter_i].so_mask);

                  if(NIL == iter_i)
                  {
                      cri_data_core_calculate_data_system_status(&resp_msg->avail_sys[iter_i]);
                  }
                }
            }
        }
        util_memory_free((void**)&resp_msg);
    }

    return err_code;
}

qmi_error_type_v01 cri_data_init_client_state()
{
    memset(&cri_data_core_radio_tech_info,
           NIL,
           sizeof(cri_data_core_radio_tech_info));
    cri_data_core_fetch_system_status();
    return QMI_ERR_NONE_V01;
}

void cri_data_cleanup_client_state()
{
    memset(&cri_data_core_radio_tech_info,
           NIL,
           sizeof(cri_data_core_radio_tech_info));
}

void cri_data_core_async_resp_handler(int qmi_service_client_id,
                                     unsigned long message_id,
                                     void *resp_data,
                                     int resp_data_len,
                                     cri_core_context_type cri_core_context)
{

    UTIL_LOG_MSG("entry");

    switch(message_id)
    {

        default:
            //no action
            break;
    }

    UTIL_LOG_MSG("exit");

}

void cri_data_core_system_status_ind_handler(int qmi_service_client_id,
                                             dsd_system_status_ind_msg_v01 *ind_msg)
{
    uint32_t iter_i;

    iter_i = 0;

    if( ind_msg )
    {
        UTIL_LOG_MSG( "available systems info valid %d", ind_msg->avail_sys_valid );

        if( TRUE == ind_msg->avail_sys_valid )
        {
            UTIL_LOG_MSG("dsd_sys_status len=%d", ind_msg->avail_sys_len);

            for(iter_i = 0; iter_i < ind_msg->avail_sys_len; iter_i++)
            {
              UTIL_LOG_MSG("preferred %d - nw=0x%x, rat_value=0x%x, so_mask 0x%016llx",
                           (NIL == iter_i),
                           ind_msg->avail_sys[iter_i].technology,
                           ind_msg->avail_sys[iter_i].rat_value,
                           ind_msg->avail_sys[iter_i].so_mask);

              if( NIL == iter_i )
              {
                  cri_data_core_calculate_data_system_status(&ind_msg->avail_sys[iter_i]);
              }
            }
        }
    }
}

void cri_data_core_unsol_ind_handler(int qmi_service_client_id,
                                        unsigned long message_id,
                                        void *ind_data,
                                        int ind_data_len)
{
    UTIL_LOG_MSG("entry %d", message_id);

    switch(message_id)
    {
        case QMI_DSD_SYSTEM_STATUS_IND_V01:
            cri_data_core_system_status_ind_handler(qmi_service_client_id,
                                                    ind_data);
            break;

        default:
            //no action
            break;
    }

    UTIL_LOG_MSG("exit");
}

void cri_data_core_calculate_data_system_status(dsd_system_status_info_type_v01 *avail_sys)
{
    if(avail_sys)
    {
        cri_data_core_radio_tech_info.is_dsd = TRUE;
        UTIL_LOG_MSG( "technology %d rat_value %x so_mask 0x%016llx",
                      avail_sys->technology,
                      avail_sys->rat_value,
                      avail_sys->so_mask);
        switch( avail_sys->rat_value )
        {
            case DSD_SYS_RAT_EX_NULL_BEARER_V01:
                cri_data_core_radio_tech_info.data_rte = CRI_NAS_RTE_UNKNOWN;
                cri_data_core_radio_tech_info.data_tech = CRI_NAS_TECH_UNKNOWN;
                break;

            case DSD_SYS_RAT_EX_3GPP_WCDMA_V01:
                if( (avail_sys->so_mask & QMI_DSD_3GPP_SO_MASK_HSDPAPLUS_V01)
                    || (avail_sys->so_mask & QMI_DSD_3GPP_SO_MASK_DC_HSDPAPLUS_V01)
                    || (avail_sys->so_mask & QMI_DSD_3GPP_SO_MASK_64_QAM_V01) )
                {
                    cri_data_core_radio_tech_info.data_tech = CRI_NAS_TECH_HSPA_PLUS;
                }
                else if( ((avail_sys->so_mask & QMI_DSD_3GPP_SO_MASK_HSDPA_V01)
                         && (avail_sys->so_mask & QMI_DSD_3GPP_SO_MASK_HSUPA_V01))
                         || (avail_sys->so_mask & QMI_DSD_3GPP_SO_MASK_HSPA_V01) )
                {
                    cri_data_core_radio_tech_info.data_tech = CRI_NAS_TECH_HSPA;
                }
                else if( (avail_sys->so_mask & QMI_DSD_3GPP_SO_MASK_HSDPA_V01) )
                {
                    cri_data_core_radio_tech_info.data_tech = CRI_NAS_TECH_HSDPA;
                }
                else if( (avail_sys->so_mask & QMI_DSD_3GPP_SO_MASK_HSUPA_V01) )
                {
                    cri_data_core_radio_tech_info.data_tech = CRI_NAS_TECH_HSUPA;
                }
                else if( (avail_sys->so_mask & QMI_DSD_3GPP_SO_MASK_WCDMA_V01) )
                {
                    cri_data_core_radio_tech_info.data_tech = CRI_NAS_TECH_WCDMA;
                }
                cri_data_core_radio_tech_info.data_rte = CRI_NAS_RTE_WCDMA;
                break;

            case DSD_SYS_RAT_EX_3GPP_GERAN_V01:
                if( (avail_sys->so_mask & QMI_DSD_3GPP_SO_MASK_EDGE_V01) )
                {
                    cri_data_core_radio_tech_info.data_tech = CRI_NAS_TECH_EDGE;
                }
                else if( (avail_sys->so_mask & QMI_DSD_3GPP_SO_MASK_GPRS_V01)
                         || (avail_sys->so_mask & QMI_DSD_3GPP_SO_MASK_GSM_V01) )
                {
                    cri_data_core_radio_tech_info.data_tech = CRI_NAS_TECH_GPRS;
                }
                cri_data_core_radio_tech_info.data_rte = CRI_NAS_RTE_GSM;
                break;

            case DSD_SYS_RAT_EX_3GPP_LTE_V01:
                cri_data_core_radio_tech_info.data_tech = CRI_NAS_TECH_LTE;
                cri_data_core_radio_tech_info.data_rte = CRI_NAS_RTE_LTE;
                break;

            case DSD_SYS_RAT_EX_3GPP_TDSCDMA_V01:
                cri_data_core_radio_tech_info.data_tech = CRI_NAS_TECH_TDSCDMA;
                if( (avail_sys->so_mask & QMI_DSD_3GPP_SO_MASK_HSDPAPLUS_V01)
                    || (avail_sys->so_mask & QMI_DSD_3GPP_SO_MASK_DC_HSDPAPLUS_V01)
                    || (avail_sys->so_mask & QMI_DSD_3GPP_SO_MASK_64_QAM_V01) )
                {
                    cri_data_core_radio_tech_info.data_tech = CRI_NAS_TECH_HSPA_PLUS;
                }
                else if( ((avail_sys->so_mask & QMI_DSD_3GPP_SO_MASK_HSDPA_V01)
                         && (avail_sys->so_mask & QMI_DSD_3GPP_SO_MASK_HSUPA_V01))
                         || (avail_sys->so_mask & QMI_DSD_3GPP_SO_MASK_HSPA_V01) )
                {
                    cri_data_core_radio_tech_info.data_tech = CRI_NAS_TECH_HSPA;
                }
                else if( (avail_sys->so_mask & QMI_DSD_3GPP_SO_MASK_HSDPA_V01) )
                {
                    cri_data_core_radio_tech_info.data_tech = CRI_NAS_TECH_HSDPA;
                }
                else if( (avail_sys->so_mask & QMI_DSD_3GPP_SO_MASK_HSUPA_V01) )
                {
                    cri_data_core_radio_tech_info.data_tech = CRI_NAS_TECH_HSUPA;
                }
                cri_data_core_radio_tech_info.data_rte = CRI_NAS_RTE_TDSCDMA;
                break;

            case DSD_SYS_RAT_EX_3GPP2_1X_V01:
                if( (avail_sys->so_mask & QMI_DSD_3GPP2_SO_MASK_1X_IS2000_REL_A_V01)
                    || (avail_sys->so_mask & QMI_DSD_3GPP2_SO_MASK_1X_IS2000_V01) )
                {
                    cri_data_core_radio_tech_info.data_tech = CRI_NAS_TECH_1xRTT;
                }
                else if( (avail_sys->so_mask & QMI_DSD_3GPP2_SO_MASK_1X_IS95_V01) )
                {
                    cri_data_core_radio_tech_info.data_tech = CRI_NAS_TECH_IS95A;
                }
                cri_data_core_radio_tech_info.data_rte = CRI_NAS_RTE_CDMA;
                break;

            case DSD_SYS_RAT_EX_3GPP2_HRPD_V01:
                if( (avail_sys->so_mask & QMI_DSD_3GPP2_SO_MASK_HDR_REVB_MMPA_V01)
                    || (avail_sys->so_mask & QMI_DSD_3GPP2_SO_MASK_HDR_REVB_EMPA_V01)
                    || (avail_sys->so_mask & QMI_DSD_3GPP2_SO_MASK_HDR_REVA_EMPA_V01)
                    || (avail_sys->so_mask & QMI_DSD_3GPP2_SO_MASK_HDR_EVDO_FMC_V01) )
                {
                    cri_data_core_radio_tech_info.data_tech = CRI_NAS_TECH_EHRPD;
                }
                else if( (avail_sys->so_mask & QMI_DSD_3GPP2_SO_MASK_HDR_REVB_DPA_V01)
                         || (avail_sys->so_mask & QMI_DSD_3GPP2_SO_MASK_HDR_REVB_MPA_V01) )
                {
                    cri_data_core_radio_tech_info.data_tech = CRI_NAS_TECH_EVDO_B;
                }
                else if( (avail_sys->so_mask & QMI_DSD_3GPP2_SO_MASK_HDR_REVA_DPA_V01)
                         || (avail_sys->so_mask & QMI_DSD_3GPP2_SO_MASK_HDR_REVA_MPA_V01) )
                {
                    cri_data_core_radio_tech_info.data_tech = CRI_NAS_TECH_EVDO_A;
                }
                else if( (avail_sys->so_mask & QMI_DSD_3GPP2_SO_MASK_HDR_REV0_DPA_V01) )
                {
                    cri_data_core_radio_tech_info.data_tech = CRI_NAS_TECH_EVDO_0;
                }
                cri_data_core_radio_tech_info.data_rte = CRI_NAS_RTE_HDR;
                break;

            case DSD_SYS_RAT_EX_3GPP2_EHRPD_V01:
                    cri_data_core_radio_tech_info.data_tech = CRI_NAS_TECH_EHRPD;
                    cri_data_core_radio_tech_info.data_rte = CRI_NAS_RTE_HDR;
                break;

            default: //no action
                break;
        }
        cri_nas_update_data_system_status();
    }
}

cri_data_system_status_info_type* cri_data_core_retrieve_data_system_status()
{
    return &cri_data_core_radio_tech_info;
}
