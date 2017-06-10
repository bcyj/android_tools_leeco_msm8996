/******************************************************************************
  @file    qmi_qos_srvc.c
  @brief   The QMI QoS service layer.

  DESCRIPTION
  QMI QoS service routines.

  INITIALIZATION AND SEQUENCING REQUIREMENTS
  qmi_qos_srvc_init_client() needs to be called before sending or receiving of any
  QoS service messages

  ---------------------------------------------------------------------------
  Copyright (c) 2010-2012,2014 Qualcomm Technologies, Inc.
  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
******************************************************************************/

#include <stdlib.h>
#include "qmi_i.h"
#include "qmi_service.h"
#include "qmi_qos_srvc.h"
#include "qmi_qos_srvc_i.h"
#include "qmi_util.h"

#define QMI_QOS_STD_MSG_SIZE                            QMI_MAX_STD_MSG_SIZE

/* Message and TLV ID definitions */
#define QMI_QOS_REQUEST_QOS_MSG_ID                      0x0020
#define QMI_QOS_MODIFY_QOS_MSG_ID                       0x0024
#define QMI_QOS_RELEASE_QOS_MSG_ID                      0x0021
/*Request Response Message Ids*/
#define QMI_QOS_RESET_SRVC_VARIABLES_MSG_ID             0x0000
#define QMI_QOS_SET_EVENT_REPORT_STATE_MSG_ID           0x0001
#define QMI_QOS_SUSPEND_QOS_MSG_ID                      0x0022
#define QMI_QOS_RESUME_QOS_MSG_ID                       0x0023
#define QMI_QOS_GET_GRNTD_QOS_INFO_MSG_ID               0x0025
#define QMI_QOS_STATUS_REPORT_STATE_MSG_ID              0x0026
#define QMI_QOS_NW_STATUS_REPORT_STATE_MSG_ID           0x0027
#define QMI_QOS_NW_SUPPORTED_QOS_PROFILES_MSG_ID        0x0028
#define QMI_QOS_PERFORM_QOS_FLOW_OPERATION_MSG_ID       0xFFFE
#define QMI_QOS_SET_CLIENT_IP_FAMILY_PREF_MSG_ID        0x002A
#define QMI_QOS_BIND_MUX_DATA_PORT_MSG_ID               0x002B
#define QMI_QOS_GET_FILTER_PARAMS_MSG_ID                0x002C
#define QMI_QOS_BIND_SUBSCRIPTION_MSG_ID                0x002D
#define QMI_QOS_GET_BIND_SUBSCRIPTION_MSG_ID            0x002E


#define QMI_QOS_CLIENT_IP_PREF_REQ_TLV_ID                 0x01

#define QMI_QOS_SPEC_QOS_ID_REQ_RSP_TLV_ID                0x01
#define QMI_QOS_SPEC_INDEX_REQ_RSP_TLV_ID                 0x10
#define QMI_QOS_SPEC_REQ_RSP_TLV_ID                       0x10
#define QMI_QOS_TX_FLOW_REQ_RSP_TLV_ID                    0x11
#define QMI_QOS_RX_FLOW_REQ_RSP_TLV_ID                    0x12
#define QMI_QOS_TX_FILTER_REQ_RSP_TLV_ID                  0x13
#define QMI_QOS_RX_FILTER_REQ_RSP_TLV_ID                  0x14
#define QMI_QOS_FLOW_FILTER_ERR_VALS_TLV_ID               0x11
#define QMI_QOS_FLOW_FILTER_SPEC_ERR_RSP_TLV_ID           0x10

/* QMI_QOS_GET_FILTER_PARAMS message TLVs */
#define QMI_QOS_GET_FILTER_PARAMS_REQ_QOS_ID_TLV_ID       0x01
#define QMI_QOS_GET_FILTER_PARAMS_TX_FILTER_DATA_TLV_ID   0x10
#define QMI_QOS_GET_FILTER_PARAMS_RX_FILTER_DATA_TLV_ID   0x11

#define QMI_QOS_SPEC_ERR_TLV_ID                           0x10

/*STATUS REPORT TLV ID's*/
#define QMI_QOS_STATUS_INFO_TLV_ID                        0x01
#define QMI_QOS_STATUS_REQ_TLV_ID                         0x01
#define QMI_QOS_REASON_CODE_TLV_ID                        0x10
#define QMI_QOS_IDENTIFIERS_LIST_TLV_ID                   0x01

#define QMI_QOS_GET_GRANTED_PRIMARY_QOS_REQ_TLV_ID        0x10

#define QMI_QOS_FLOW_INDEX_REQ_RSP_TLV_ID                 0x10
#define QMI_QOS_FLOW_SPEC_REQ_RSP_TLV_ID                  0x10
#define QMI_QOS_FLOW_CDMA_PROFILE_ID_REQ_TLV_ID           0x1B
#define QMI_QOS_FLOW_UMTS_TC_REQ_RSP_TLV_ID               0x11
#define QMI_QOS_FLOW_UMTS_DATA_RATE_REQ_RSP_TLV_ID        0x12
#define QMI_QOS_FLOW_UMTS_BUCKET_INFO_REQ_RSP_TLV_ID      0x13
#define QMI_QOS_FLOW_UMTS_MAX_DELAY_REQ_RSP_TLV_ID        0x14
#define QMI_QOS_FLOW_UMTS_MAX_JITTER_REQ_RSP_TLV_ID       0x15
#define QMI_QOS_FLOW_UMTS_PKT_ERR_RATE_REQ_RSP_TLV_ID     0x16
#define QMI_QOS_FLOW_UMTS_MIN_POL_PKT_SZ_REQ_RSP_TLV_ID   0x17
#define QMI_QOS_FLOW_UMTS_MAX_ALLOW_PKT_SZ_REQ_RSP_TLV_ID 0x18
#define QMI_QOS_FLOW_UMTS_RESIDUAL_BER_REQ_RSP_TLV_ID     0x19
#define QMI_QOS_FLOW_UMTS_HANDLING_PRIO_REQ_RSP_TLV_ID    0x1A

#define QMI_QOS_FLOW_UMTS_3GPP2_IP_FLOW_PRIORITY_REQ_RSP_TLV_ID   0x1C
#define QMI_QOS_FLOW_UMTS_IM_CN_FLAG_REQ_RSP_TLV_ID               0x1D
#define QMI_QOS_FLOW_UMTS_3GPP_IP_FLOW_SIG_IND_REQ_RSP_TLV_ID     0x1E
#define QMI_QOS_FLOW_LTE_QCI_REQ_RSP_TLV_ID                       0x1F



#define QMI_QOS_TECH_PREF_REQ_TLV_ID                      0x10
#define QMI_QOS_PERFORM_FLOW_OP_QOS_ID_REQ_TLV_ID         0x01
#define QMI_QOS_PERFORM_FLOW_OP_REQUESTED_OP_TLV_ID       0x10
#define QMI_QOS_PERFORM_FLOW_SET_INACTIVITY_TIMER_TLV_ID  0x11
#define QMI_QOS_PERFORM_FLOW_PRIMARY_FLOW_OP_TLV_ID       0x12

#define QMI_QOS_NW_SUPP_QOS_PROFILES_RESP_TLV_ID        0x01
#define QMI_QOS_EXTENDED_ERROR_INFO_TLV_ID                0xE0
#define QMI_QOS_FILTER_INDEX_REQ_RSP_TLV_ID                 0x10
#define QMI_QOS_FILTER_SPEC_REQ_RSP_TLV_ID                  0x10
#define QMI_QOS_FILTER_IP_VER_REQ_RSP_TLV_ID                0x11
#define QMI_QOS_FILTER_SRC_ADDR_REQ_RSP_TLV_ID              0x12
#define QMI_QOS_FILTER_DEST_ADDR_REQ_RSP_TLV_ID             0x13
#define QMI_QOS_FILTER_TRANS_PROTOCOL_REQ_RSP_TLV_ID        0x14
#define QMI_QOS_FILTER_TOS_REQ_RSP_TLV_ID                   0x15
#define QMI_QOS_FILTER_IPV6_SRC_ADDR_REQ_RSP_TLV_ID         0x16
#define QMI_QOS_FILTER_IPV6_DST_ADDR_REQ_RSP_TLV_ID         0x17
#define QMI_QOS_FILTER_IPV6_TRAFFIC_CLASS_REQ_RSP_TLV_ID    0x19
#define QMI_QOS_FILTER_IPV6_FLOW_LABEL_REQ_RSP_TLV_ID       0x1A
#define QMI_QOS_FILTER_TCP_SRC_PORT_REQ_RSP_TLV_ID          0x1B
#define QMI_QOS_FILTER_TCP_DEST_PORT_REQ_RSP_TLV_ID         0x1C
#define QMI_QOS_FILTER_UDP_SRC_PORT_REQ_RSP_TLV_ID          0x1D
#define QMI_QOS_FILTER_UDP_DEST_PORT_REQ_RSP_TLV_ID         0x1E
#define QMI_QOS_FILTER_ESP_SECURITY_POLICY_REQ_RSP_TLV_ID   0x21
#define QMI_QOS_FILTER_PRECEDENCE_REQ_RSP_TLV_ID            0x22
#define QMI_QOS_FILTER_FILTER_ID_REQ_RSP_TLV_ID             0x23
#define QMI_QOS_FILTER_TRANSPORT_SRC_PORT_REQ_RSP_TLV_ID    0x24
#define QMI_QOS_FILTER_TRANSPORT_DEST_PORT_REQ_RSP_TLV_ID   0x25


#define QMI_QOS_REQUEST_OPCODE_TLV_ID                     0x11
#define QMI_QOS_MODIFY_PRIMARY_QOS_TLV_ID                 0x11

#define QMI_QOS_FLOW_OPERATION_FAILURE_RESP_TLV_ID        0xE1
#define QMI_QOS_FLOW_OP_TX_QUEUE_LEVEL_RESP_TLV_ID        0x11
#define QMI_QOS_FLOW_OP_RMAC3_INFO_RESP_TLV_ID            0x12
#define QMI_QOS_FLOW_OP_TX_INFO_INFO_RESP_TLV_ID          0x13
#define QMI_QOS_FLOW_OP_INACTIVITY_TIMER_RESP_TLV_ID      0x14


#define QMI_QOS_RELEASE_ID_LIST_TLV_ID                           0x01
#define QMI_QOS_EVENT_REPORT_GLOBAL_FLOW_STATE_TLV_ID            0x10
#define QMI_QOS_EVENT_REPORT_NW_SUPP_PROFILE_CHANGE_TLV_ID       0x11

/* Bind MUX data port TLVs */
#define QMI_QOS_BIND_MUX_DATA_PORT_EP_ID_TLV_ID                  0x10
#define QMI_QOS_BIND_MUX_DATA_PORT_MUX_ID_TLV_ID                 0x11
#define QMI_QOS_BIND_MUX_DATA_PORT_REVERSED_TLV_ID               0x12

#define QMI_QOS_BIND_SUBS_ID_REQ_TLV_ID                 0x01
#define QMI_QOS_GET_BIND_SUB_ID_TLV_ID     0x10

/* Event report related indication TLV ID's */
#define QMI_QOS_EVENT_FLOW_INFO_TLV_ID                    0x10
#define QMI_QOS_NW_SUPPORTED_QOS_PROFILES_TLV_ID          0x11
#define QMI_QOS_NW_STATUS_QOS_SUPPORTED_TLV_ID            0x01
#define QMI_QOS_NW_STATUS_IFACE_NAME_TLV_ID               0x10
#define QMI_QOS_STATUS_IND_TLV_ID                         0x01
#define QMI_QOS_STATUS_IND_TLV_ID                         0x01
#define QMI_QOS_STATUS_PRIMARY_QOS_MODIFY_RESULT_TLV_ID   0x01

#define QMI_QOS_EVENT_FLOW_STATE_TLV_ID                   0x10
#define QMI_QOS_EVENT_TRANS_FLOW_GRANTED_TLV_ID           0x11
#define QMI_QOS_EVENT_RECV_FLOW_GRANTED_TLV_ID            0x12
#define QMI_QOS_EVENT_TRANS_FILTER_GRANTED_TLV_ID         0x13
#define QMI_QOS_EVENT_RECV_FILTER_GRANTED_TLV_ID          0x14
#define QMI_QOS_EVENT_FLOW_TYPE_TLV_ID                    0x15

/* Indication message ID's */
#define QMI_QOS_EVENT_REPORT_IND_MSG_ID                    0x0001
#define QMI_QOS_STATUS_REPORT_IND_MSG_ID                   0x0026
#define QMI_QOS_NW_STATUS_REPORT_IND_MSG_ID                0x0027
#define QMI_QOS_PRIMARY_QOS_EVENT_IND_MSG_ID               0x0029

/* Some utility macros to make code a bit more readable */

/* Macro to write a TLV and add to the tlv_length variable */
#define QMI_QOSWRITE_TLV(tlv_id,size,data) \
do {\
  if ((rc = qmi_util_write_std_tlv (msg_buf, \
                                       msg_buf_size, \
                                       tlv_id, \
                                       size, \
                                       (void *)data)) < 0) \
  { \
    return QMI_INTERNAL_ERR; \
  } \
  tlv_length += rc; \
} while (0)


/* Macro to read a TLV and return error if read fails */
#define QMI_QOSREAD_TLV() \
do { \
  if (qmi_util_read_std_tlv (&msg_buf, \
                                &msg_buf_size, \
                                &type, \
                                &length, \
                                &value_ptr) < 0) \
  { \
    return QMI_INTERNAL_ERR; \
  } \
} while(0)



static int qos_service_initialized = FALSE;

/*===========================================================================
  FUNCTION  qmi_qos_srvc_event_prcss_nstd_flow_tlv
===========================================================================*/
/*!
@brief
  Processes a Flow Spec TLV and translates it into
  C structure

@return
  QMI_NO_ERR if no error occurred, QMI_INTERNAL_ERR otherwise

@note

*/
/*=========================================================================*/
static int
qmi_qos_srvc_event_prcss_nstd_flow_tlv
(
unsigned char                  *msg_buf,
int                            msg_buf_len,
qmi_qos_granted_flow_data_type          *flow_ind_data
)
{
  unsigned long type;
  unsigned long length;
  unsigned char *value_ptr;

  if (qmi_util_read_std_tlv (&msg_buf,
                                &msg_buf_len,
                                &type,
                                &length,
                                &value_ptr) < 0)
  {
    return QMI_INTERNAL_ERR;
  }

  {
    /*At this point value_ptr points to flow_spec
    **value portion containing the flow spec tlvs
   ** and length contains the total length of all
    ** tlvs */
    unsigned long tmp_type;
    unsigned long tmp_length;
    unsigned char *tmp_value_ptr;
    unsigned char tmp_char;
    int outer_tlv_length = (int) length;

    /*Set the Param mask to zero*/
    flow_ind_data->qos_flow_granted.umts_flow_desc.param_mask = 0;

    while (outer_tlv_length > 0)
    {
      if (qmi_util_read_std_tlv (&value_ptr,
                                  &outer_tlv_length,
                                    &tmp_type,
                                    &tmp_length,
                                    &tmp_value_ptr) < 0)
      {
        return QMI_INTERNAL_ERR;
      }
      /*use tmp_value_ptr to copy values to ind datastructure*/
      switch (tmp_type)
      {
        case QMI_QOS_FLOW_SPEC_REQ_RSP_TLV_ID:
          {
            READ_8_BIT_VAL (tmp_value_ptr, flow_ind_data->ip_flow_index);
          }
          break;

        case QMI_QOS_FLOW_CDMA_PROFILE_ID_REQ_TLV_ID:
          {
            flow_ind_data->qos_flow_granted.cdma_flow_desc.param_mask |= QMI_QOS_CDMA_FLOW_PARAM_PROFILE_ID;
            READ_16_BIT_VAL (tmp_value_ptr, flow_ind_data->qos_flow_granted.cdma_flow_desc.profile_id);
          }
          break;

        case QMI_QOS_FLOW_UMTS_TC_REQ_RSP_TLV_ID:
          {
            flow_ind_data->qos_flow_granted.umts_flow_desc.param_mask |= QMI_QOS_UMTS_FLOW_PARAM_TRAFFIC_CLASS;
            READ_8_BIT_VAL (tmp_value_ptr, tmp_char);
            flow_ind_data->qos_flow_granted.umts_flow_desc.traffic_class = (qmi_qos_umts_traffic_class_type) tmp_char;
          }
          break;

        case QMI_QOS_FLOW_UMTS_DATA_RATE_REQ_RSP_TLV_ID:
          {
            flow_ind_data->qos_flow_granted.umts_flow_desc.param_mask |= QMI_QOS_UMTS_FLOW_PARAM_DATA_RATE;
            READ_32_BIT_VAL (tmp_value_ptr, flow_ind_data->qos_flow_granted.umts_flow_desc.data_rate.max_rate);
            READ_32_BIT_VAL (tmp_value_ptr, flow_ind_data->qos_flow_granted.umts_flow_desc.data_rate.guaranteed_rate);
          }
          break;

        case QMI_QOS_FLOW_UMTS_BUCKET_INFO_REQ_RSP_TLV_ID:
          {
            flow_ind_data->qos_flow_granted.umts_flow_desc.param_mask |= QMI_QOS_UMTS_FLOW_PARAM_BUCKET_INFO;
            READ_32_BIT_VAL (tmp_value_ptr, flow_ind_data->qos_flow_granted.umts_flow_desc.bucket_info.peak_rate);
            READ_32_BIT_VAL (tmp_value_ptr, flow_ind_data->qos_flow_granted.umts_flow_desc.bucket_info.token_rate);
            READ_32_BIT_VAL (tmp_value_ptr, flow_ind_data->qos_flow_granted.umts_flow_desc.bucket_info.bucket_size);
          }
          break;

        case QMI_QOS_FLOW_UMTS_MAX_DELAY_REQ_RSP_TLV_ID:
          {
            flow_ind_data->qos_flow_granted.umts_flow_desc.param_mask |= QMI_QOS_UMTS_FLOW_PARAM_MAX_DELAY;
            READ_32_BIT_VAL (tmp_value_ptr, flow_ind_data->qos_flow_granted.umts_flow_desc.max_delay);
          }
          break;

        case QMI_QOS_FLOW_UMTS_MAX_JITTER_REQ_RSP_TLV_ID:
          {
            flow_ind_data->qos_flow_granted.umts_flow_desc.param_mask |= QMI_QOS_UMTS_FLOW_PARAM_MAX_JITTER;
            READ_32_BIT_VAL (tmp_value_ptr, flow_ind_data->qos_flow_granted.umts_flow_desc.max_jitter);
          }
          break;

        case QMI_QOS_FLOW_UMTS_PKT_ERR_RATE_REQ_RSP_TLV_ID:
          {
            flow_ind_data->qos_flow_granted.umts_flow_desc.param_mask |= QMI_QOS_UMTS_FLOW_PARAM_PKT_ERR_RATE;
            READ_16_BIT_VAL (tmp_value_ptr, flow_ind_data->qos_flow_granted.umts_flow_desc.pkt_err_rate.multiplier);
            READ_16_BIT_VAL (tmp_value_ptr, flow_ind_data->qos_flow_granted.umts_flow_desc.pkt_err_rate.exponent);
          }
          break;

        case QMI_QOS_FLOW_UMTS_MIN_POL_PKT_SZ_REQ_RSP_TLV_ID:
          {
            flow_ind_data->qos_flow_granted.umts_flow_desc.param_mask |= QMI_QOS_UMTS_FLOW_PARAM_MIN_POL_PKT_SZ;
            READ_32_BIT_VAL (tmp_value_ptr, flow_ind_data->qos_flow_granted.umts_flow_desc.min_policed_pkt_sz);
          }
          break;

        case QMI_QOS_FLOW_UMTS_MAX_ALLOW_PKT_SZ_REQ_RSP_TLV_ID:
          {
            flow_ind_data->qos_flow_granted.umts_flow_desc.param_mask |= QMI_QOS_UMTS_FLOW_PARAM_MAX_ALLOW_PKT_SZ;
            READ_32_BIT_VAL (tmp_value_ptr, flow_ind_data->qos_flow_granted.umts_flow_desc.max_allowed_pkt_sz);
          }
          break;

        case QMI_QOS_FLOW_UMTS_RESIDUAL_BER_REQ_RSP_TLV_ID:
          {
            flow_ind_data->qos_flow_granted.umts_flow_desc.param_mask |= QMI_QOS_UMTS_FLOW_PARAM_RESIDUAL_BER;
            READ_8_BIT_VAL (tmp_value_ptr, tmp_char);
            flow_ind_data->qos_flow_granted.umts_flow_desc.residual_ber = (qmi_qos_umts_residual_ber_type) tmp_char;
          }
          break;

        case QMI_QOS_FLOW_UMTS_HANDLING_PRIO_REQ_RSP_TLV_ID:
          {
            flow_ind_data->qos_flow_granted.umts_flow_desc.param_mask |= QMI_QOS_UMTS_FLOW_PARAM_HANDLING_PRIO;
            READ_8_BIT_VAL (tmp_value_ptr, tmp_char);
            flow_ind_data->qos_flow_granted.umts_flow_desc.handling_prio = (qmi_qos_umts_handling_prio_type) tmp_char;
          }
          break;
        case QMI_QOS_FLOW_UMTS_3GPP2_IP_FLOW_PRIORITY_REQ_RSP_TLV_ID:
          {
            flow_ind_data->qos_flow_granted.umts_flow_desc.param_mask |= QMI_QOS_UMTS_FLOW_PARAM_3GPP2_FLOW_PRIO;
            READ_8_BIT_VAL (tmp_value_ptr, tmp_char);
            flow_ind_data->qos_flow_granted.umts_flow_desc.flow_priority_3gpp2 = (unsigned char) tmp_char;
          }
          break;
        case QMI_QOS_FLOW_UMTS_IM_CN_FLAG_REQ_RSP_TLV_ID:
          {
            flow_ind_data->qos_flow_granted.umts_flow_desc.param_mask |= QMI_QOS_UMTS_FLOW_PARAM_IM_CN_FLAG;
            READ_8_BIT_VAL (tmp_value_ptr, tmp_char);
            flow_ind_data->qos_flow_granted.umts_flow_desc.im_cn_flag = (qmi_qos_bool_type) tmp_char;
          }
          break;
        case QMI_QOS_FLOW_UMTS_3GPP_IP_FLOW_SIG_IND_REQ_RSP_TLV_ID:
          {
            flow_ind_data->qos_flow_granted.umts_flow_desc.param_mask |= QMI_QOS_UMTS_FLOW_PARAM_IP_FLOW_SIG_IND;
            READ_8_BIT_VAL (tmp_value_ptr, tmp_char);
            flow_ind_data->qos_flow_granted.umts_flow_desc.ip_flow_sig_ind = tmp_char;
          }
          break;

        case QMI_QOS_FLOW_LTE_QCI_REQ_RSP_TLV_ID:
          {
            flow_ind_data->qos_flow_granted.umts_flow_desc.param_mask |= QMI_QOS_LTE_FLOW_PARAM_QCI_IND;
            READ_8_BIT_VAL (tmp_value_ptr, tmp_char);
            flow_ind_data->qos_flow_granted.umts_flow_desc.lte_qci = (qmi_qos_bool_type) tmp_char;
          }
          break;
        default:
          {
            QMI_DEBUG_MSG_2 ("qmi_qos_srvc_event_prcss_nstd_flow_tlv: Unknown TLV ID=%x, len=%d",
                             (unsigned int)tmp_type,(int)tmp_length);
          }
          break;
      }
    }
  }

  return QMI_NO_ERR;
}

/*===========================================================================
  FUNCTION  qmi_qos_srvc_event_prcss_sngl_nstd_filter_tlv
===========================================================================*/
/*!
@brief
  Processes a single filter spec TLV's and translates into
  a C data structure

@return
  QMI_NO_ERR if no error occurred, QMI_INTERNAL_ERR otherwise

@note

*/
/*=========================================================================*/
static int
qmi_qos_srvc_event_prcss_sngl_nstd_filter_tlv
(
unsigned char                  *msg_buf,
int                            msg_buf_len,
qmi_qos_granted_filter_data_type         *filter_ind_data
)
{
  unsigned long type;
  unsigned long length;
  unsigned char *value_ptr;
  unsigned char *tmp_char;
  int tlv_count = 0;

  unsigned char    *tmp_msg_buf = msg_buf;
  int               tmp_msg_buf_len = msg_buf_len;

  /*Set the Param mask*/
  filter_ind_data->qos_filter.filter_desc.param_mask = 0;


  while (tmp_msg_buf_len > 0)
  {
    if (qmi_util_read_std_tlv (&tmp_msg_buf,
                                  &tmp_msg_buf_len,
                                  &type,
                                  &length,
                                  &value_ptr) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
    tlv_count++;

    QMI_DEBUG_MSG_4 ("sngl_nstd_filter_tlv:  TLV ID=%x, len=%d value_ptr[0]=%d tlv_count=%d",
                  (unsigned int)type,(int)length,(int)value_ptr[0],tlv_count);

    switch (type)
    {
      case QMI_QOS_FILTER_INDEX_REQ_RSP_TLV_ID:
        {
          /*Event Mask set here*/
          READ_8_BIT_VAL (value_ptr, filter_ind_data->filter_index);
        }
        break;

      case QMI_QOS_FILTER_IP_VER_REQ_RSP_TLV_ID:
        {
          READ_8_BIT_VAL (value_ptr, tmp_char);
          filter_ind_data->qos_filter.ip_version = (qmi_qos_ip_version_type)((unsigned long)tmp_char);
        }
        break;

      case QMI_QOS_FILTER_SRC_ADDR_REQ_RSP_TLV_ID:
        {
          filter_ind_data->qos_filter.filter_desc.param_mask |= QMI_QOS_FILTER_PARAM_SRC_ADDR;
          READ_32_BIT_VAL (value_ptr, filter_ind_data->qos_filter.filter_desc.src_addr.ipv4_ip_addr);
          READ_32_BIT_VAL (value_ptr, filter_ind_data->qos_filter.filter_desc.src_addr.ipv4_subnet_mask);
        }
        break;

      case QMI_QOS_FILTER_DEST_ADDR_REQ_RSP_TLV_ID:
        {
          filter_ind_data->qos_filter.filter_desc.param_mask |= QMI_QOS_FILTER_PARAM_DEST_ADDR;
          READ_32_BIT_VAL (value_ptr, filter_ind_data->qos_filter.filter_desc.dest_addr.ipv4_ip_addr);
          READ_32_BIT_VAL (value_ptr, filter_ind_data->qos_filter.filter_desc.dest_addr.ipv4_subnet_mask);
        }
        break;

      case QMI_QOS_FILTER_TRANS_PROTOCOL_REQ_RSP_TLV_ID:
        {
          filter_ind_data->qos_filter.filter_desc.param_mask |= QMI_QOS_FILTER_PARAM_TRANS_PROTOCOL;
          READ_8_BIT_VAL (value_ptr, tmp_char);
          filter_ind_data->qos_filter.filter_desc.protocol = (qmi_qos_trans_protocol_filter_type)((unsigned long)tmp_char);
        }
        break;

      case QMI_QOS_FILTER_TOS_REQ_RSP_TLV_ID:
        {
          filter_ind_data->qos_filter.filter_desc.param_mask |= QMI_QOS_FILTER_PARAM_TOS;
          READ_8_BIT_VAL (value_ptr, filter_ind_data->qos_filter.filter_desc.tos.tos_value);
          READ_8_BIT_VAL (value_ptr, filter_ind_data->qos_filter.filter_desc.tos.tos_mask);
        }
        break;

      case QMI_QOS_FILTER_IPV6_SRC_ADDR_REQ_RSP_TLV_ID:
        {
          filter_ind_data->qos_filter.filter_desc.param_mask |= QMI_QOS_FILTER_PARAM_IPV6_SRC_ADDR;
          if (length <= sizeof(qmi_qos_ipv6_addr_filter_type))
          {
            memcpy(&filter_ind_data->qos_filter.filter_desc.ipv6_src_addr,value_ptr,length);
          }
        }
        break;

      case QMI_QOS_FILTER_IPV6_DST_ADDR_REQ_RSP_TLV_ID:
        {
          filter_ind_data->qos_filter.filter_desc.param_mask |= QMI_QOS_FILTER_PARAM_IPV6_DEST_ADDR;
          if (length <= sizeof(qmi_qos_ipv6_addr_filter_type))
          {
            memcpy(&filter_ind_data->qos_filter.filter_desc.ipv6_dest_addr,value_ptr,length);
          }
        }
        break;

      case QMI_QOS_FILTER_IPV6_TRAFFIC_CLASS_REQ_RSP_TLV_ID:
        {
          filter_ind_data->qos_filter.filter_desc.param_mask |= QMI_QOS_FILTER_PARAM_IPV6_TRAFFIC_CLASS;
          READ_8_BIT_VAL (value_ptr, filter_ind_data->qos_filter.filter_desc.ipv6_traffic_class.traffic_class_value);
          READ_8_BIT_VAL (value_ptr, filter_ind_data->qos_filter.filter_desc.ipv6_traffic_class.traffic_class_mask);
        }
        break;

      case QMI_QOS_FILTER_IPV6_FLOW_LABEL_REQ_RSP_TLV_ID:
        {
          filter_ind_data->qos_filter.filter_desc.param_mask |= QMI_QOS_FILTER_PARAM_IPV6_FLOW_LABEL;
          READ_32_BIT_VAL (value_ptr, filter_ind_data->qos_filter.filter_desc.ipv6_flow_label);
        }
        break;

      case QMI_QOS_FILTER_TCP_SRC_PORT_REQ_RSP_TLV_ID:
        {
          filter_ind_data->qos_filter.filter_desc.param_mask |= QMI_QOS_FILTER_PARAM_TCP_SRC_PORTS;
          READ_16_BIT_VAL (value_ptr, filter_ind_data->qos_filter.filter_desc.tcp_src_ports.start_port);
          READ_16_BIT_VAL (value_ptr, filter_ind_data->qos_filter.filter_desc.tcp_src_ports.range);
        }
        break;

      case QMI_QOS_FILTER_TCP_DEST_PORT_REQ_RSP_TLV_ID:
        {
          filter_ind_data->qos_filter.filter_desc.param_mask |= QMI_QOS_FILTER_PARAM_TCP_DEST_PORTS;
          READ_16_BIT_VAL (value_ptr, filter_ind_data->qos_filter.filter_desc.tcp_dest_ports.start_port);
          READ_16_BIT_VAL (value_ptr, filter_ind_data->qos_filter.filter_desc.tcp_dest_ports.range);
        }
        break;

      case QMI_QOS_FILTER_UDP_SRC_PORT_REQ_RSP_TLV_ID:
        {
          filter_ind_data->qos_filter.filter_desc.param_mask |= QMI_QOS_FILTER_PARAM_UDP_SRC_PORTS;
          READ_16_BIT_VAL (value_ptr, filter_ind_data->qos_filter.filter_desc.udp_src_ports.start_port);
          READ_16_BIT_VAL (value_ptr, filter_ind_data->qos_filter.filter_desc.udp_src_ports.range);
        }
        break;

      case QMI_QOS_FILTER_UDP_DEST_PORT_REQ_RSP_TLV_ID:
        {
          filter_ind_data->qos_filter.filter_desc.param_mask |= QMI_QOS_FILTER_PARAM_UDP_DEST_PORTS;
          READ_16_BIT_VAL (value_ptr, filter_ind_data->qos_filter.filter_desc.udp_dest_ports.start_port);
          READ_16_BIT_VAL (value_ptr, filter_ind_data->qos_filter.filter_desc.udp_dest_ports.range);
        }
        break;

      case QMI_QOS_FILTER_ESP_SECURITY_POLICY_REQ_RSP_TLV_ID:
        {
          filter_ind_data->qos_filter.filter_desc.param_mask |= QMI_QOS_FILTER_PARAM_ESP_SECURITY_POLICY;
          READ_32_BIT_VAL (value_ptr, filter_ind_data->qos_filter.filter_desc.esp_security_policy_index);
        }
        break;

      case QMI_QOS_FILTER_PRECEDENCE_REQ_RSP_TLV_ID:
        {
          filter_ind_data->qos_filter.filter_desc.param_mask |= QMI_QOS_FILTER_PARAM_PRECEDENCE;
          READ_8_BIT_VAL (value_ptr, filter_ind_data->qos_filter.filter_desc.precedence);
        }
        break;
      case QMI_QOS_FILTER_FILTER_ID_REQ_RSP_TLV_ID:
        {
          filter_ind_data->qos_filter.filter_desc.param_mask |= QMI_QOS_FILTER_PARAM_FILTER_ID;
          READ_8_BIT_VAL (value_ptr, filter_ind_data->qos_filter.filter_desc.filter_id);
        }
        break;
      case QMI_QOS_FILTER_TRANSPORT_SRC_PORT_REQ_RSP_TLV_ID:
        {
          filter_ind_data->qos_filter.filter_desc.param_mask |= QMI_QOS_FILTER_PARAM_TRANSPORT_SRC_PORTS;
          READ_16_BIT_VAL (value_ptr, filter_ind_data->qos_filter.filter_desc.transport_src_ports.start_port);
          READ_16_BIT_VAL (value_ptr, filter_ind_data->qos_filter.filter_desc.transport_src_ports.range);
        }
        break;
      case QMI_QOS_FILTER_TRANSPORT_DEST_PORT_REQ_RSP_TLV_ID:
        {
          filter_ind_data->qos_filter.filter_desc.param_mask |= QMI_QOS_FILTER_PARAM_TRANSPORT_DEST_PORTS;
          READ_16_BIT_VAL (value_ptr, filter_ind_data->qos_filter.filter_desc.transport_dest_ports.start_port);
          READ_16_BIT_VAL (value_ptr, filter_ind_data->qos_filter.filter_desc.transport_dest_ports.range);
        }
        break;

      default:
        {
          QMI_DEBUG_MSG_2 ("qmi_qos_srvc_event_prcss_sngl_nstd_filter_tlv: Unknown TLV ID=%x, len=%d",
                           (unsigned int)type,(int)length);
        }
        break;

    }
  }
  return QMI_NO_ERR;
}

/*===========================================================================
  FUNCTION  qmi_qos_srvc_event_prcss_nstd_filter_tlvs
===========================================================================*/
/*!
@brief
  Processes one or more filter spec TLV's and translates into
  a C data structure

@return
  QMI_NO_ERR if no error occurred, QMI_INTERNAL_ERR otherwise

@note

*/
/*=========================================================================*/
static int
qmi_qos_srvc_event_prcss_nstd_filter_tlvs
(
  unsigned char                        *msg_buf,
  int                                   msg_buf_len,
  qmi_qos_granted_filter_data_type     *filter_ind_data,
  unsigned int                         *filter_count
)
{
  unsigned long type;
  unsigned long length;
  unsigned char *value_ptr;

  unsigned int   filter_index = 0;

  while (msg_buf_len > 0)
  {
    if (qmi_util_read_std_tlv (&msg_buf,
                                  &msg_buf_len,
                                  &type,
                                  &length,
                                  &value_ptr) < 0)
    {
      return QMI_INTERNAL_ERR;
    }

    if (qmi_qos_srvc_event_prcss_sngl_nstd_filter_tlv (value_ptr,
                                                     (int)length,
                                                     &filter_ind_data[filter_index++]) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }

  /*Set the filter's count here*/
  *filter_count = filter_index;
  return QMI_NO_ERR;

}


/*===========================================================================
  FUNCTION  qmi_qos_srvc_process_event_report_ind
===========================================================================*/
/*!
@brief
  Processes a event report TLVs and translates it into
  C structure indication data

@return
  QMI_NO_ERR if no error occurred, QMI_INTERNAL_ERR otherwise

@note

*/
/*=========================================================================*/

static int
qmi_qos_srvc_process_event_report_ind
(
  unsigned char                 *rx_buf,
  int                            rx_buf_len,
  qmi_qos_event_report_type     *ind_data,
  short                          index
)
{
  unsigned long type;
  unsigned long length;
  unsigned char *value_ptr;
  unsigned char tmp_char;

  /*Error Check*/
  if ((rx_buf == NULL) || (ind_data == NULL) ||
      (index < 0) || (index >= QMI_QOS_MAX_FLOW_EVENTS))
  {
    return QMI_INTERNAL_ERR;
  }
  /*Setting the counts and Param masks to zero*/
  ind_data->flow_info[index].tx_granted_flow_data_is_valid = 0;
  ind_data->flow_info[index].rx_granted_flow_data_is_valid = 0;
  ind_data->flow_info[index].tx_filter_count = 0;
  ind_data->flow_info[index].rx_filter_count = 0;
  ind_data->flow_info[index].param_mask = 0;

  while (rx_buf_len > 0)
  {
    if (qmi_util_read_std_tlv (&rx_buf,
                               &rx_buf_len,
                               &type,
                               &length,
                               &value_ptr) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
    /*Process the nested TLVS in the else part here*/
    switch (type)
    {
      case QMI_QOS_EVENT_FLOW_STATE_TLV_ID:
        {
          READ_32_BIT_VAL (value_ptr, ind_data->flow_info[index].qos_flow_state.qos_identifier);
          READ_8_BIT_VAL  (value_ptr, ind_data->flow_info[index].qos_flow_state.new_flow);
          READ_8_BIT_VAL  (value_ptr, tmp_char);
          ind_data->flow_info[index].qos_flow_state.report_flow_state_chng = (global_flow_state_change) tmp_char;
          ind_data->flow_info[index].param_mask |= QMI_QOS_EVENT_REPORT_FLOW_INFO_FLOW_STATE_PARAM;
        }
        break;

      case QMI_QOS_EVENT_TRANS_FLOW_GRANTED_TLV_ID:
        {
          ind_data->flow_info[index].tx_granted_flow_data_is_valid = 1;
          if (qmi_qos_srvc_event_prcss_nstd_flow_tlv (value_ptr,
                                                     (int)length,
                                                     &ind_data->flow_info[index].tx_granted_flow_data) < 0)
          {
            return QMI_INTERNAL_ERR;
          }
        }
        break;

      case QMI_QOS_EVENT_RECV_FLOW_GRANTED_TLV_ID:
        {
          ind_data->flow_info[index].rx_granted_flow_data_is_valid = 1;
          if (qmi_qos_srvc_event_prcss_nstd_flow_tlv (value_ptr,
                                                     (int)length,
                                                     &ind_data->flow_info[index].rx_granted_flow_data) < 0)
          {
            return QMI_INTERNAL_ERR;
          }
        }
        break;

      case QMI_QOS_EVENT_TRANS_FILTER_GRANTED_TLV_ID:
        {
          if (qmi_qos_srvc_event_prcss_nstd_filter_tlvs (value_ptr,
                                                       (int)length,
                                                         ind_data->flow_info[index].tx_granted_filter_data,
                                                         &ind_data->flow_info[index].tx_filter_count) < 0)
          {
            return QMI_INTERNAL_ERR;
          }
        }
        break;

      case QMI_QOS_EVENT_RECV_FILTER_GRANTED_TLV_ID:
        {
          if (qmi_qos_srvc_event_prcss_nstd_filter_tlvs (value_ptr,
                                                       (int)length,
                                                         ind_data->flow_info[index].rx_granted_filter_data,
                                                         &ind_data->flow_info[index].rx_filter_count) < 0)
          {
            return QMI_INTERNAL_ERR;
          }
        }
        break;

      case QMI_QOS_EVENT_FLOW_TYPE_TLV_ID:
        {
          READ_8_BIT_VAL( value_ptr, ind_data->flow_info[index].flow_type );
          ind_data->flow_info[index].param_mask |= QMI_QOS_EVENT_REPORT_FLOW_INFO_FLOW_TYPE_PARAM;
        }
        break;
      default:
        {
          QMI_DEBUG_MSG_2 ("qmi_qos_srvc_process_event_report_ind: Unknown TLV ID=%x, len=%d",
                           (unsigned int)type,(int)length);
        }
        break;
    }
  }

  return QMI_NO_ERR;
}

/*===========================================================================
  FUNCTION  qmi_qos_srvc_process_status_report_ind
===========================================================================*/
/*!
@brief
  Processes a service report TLVs and translates it into
  C structure indication data

@return
  QMI_NO_ERR if no error occurred, QMI_INTERNAL_ERR otherwise

@note

*/
/*=========================================================================*/
static int
qmi_qos_srvc_process_status_report_ind
(
unsigned char                  *msg_buf,
int                            msg_len,
qmi_qos_status_report_type     *ind_data
)
{
  unsigned long type;
  unsigned long length;
  unsigned char *value_ptr;
  unsigned char tmp_char;

  ind_data->qos_reason_is_valid = 0;

  while (msg_len > 0)
  {
    if (qmi_util_read_std_tlv (&msg_buf,
                                  &msg_len,
                                  &type,
                                  &length,
                                  &value_ptr) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
    switch (type)
    {

      case QMI_QOS_STATUS_INFO_TLV_ID:
        {
          READ_32_BIT_VAL (value_ptr, ind_data->qos_status_information.qos_identifier);
          READ_8_BIT_VAL (value_ptr, tmp_char);
          ind_data->qos_status_information.qos_status = (qmi_qos_status_info) tmp_char;
          READ_8_BIT_VAL (value_ptr, tmp_char);
          ind_data->qos_status_information.qos_event = (qmi_qos_status_event) tmp_char;
        }
        break;

      case QMI_QOS_REASON_CODE_TLV_ID:
        {
          ind_data->qos_reason_is_valid = 1;
          READ_8_BIT_VAL (value_ptr, tmp_char);
          ind_data->qos_reason = (qmi_qos_reason_code) tmp_char;
        }
        break;

      default:
        {
          QMI_DEBUG_MSG_2 ("qmi_qos_srvc_process_status_report_ind: Unknown TLV ID=%x, len=%d",
                           (unsigned int)type,(int)length);

        }
        break;
    }

  }
  return QMI_NO_ERR;
}


/*===========================================================================
  FUNCTION  qmi_qos_srvc_process_nw_status_report
===========================================================================*/
/*!
@brief
  Processes a event report TLVs associated with the NW status report and
  adds to indication data.

@return
  QMI_NO_ERR if no error occurred, QMI_INTERNAL_ERR otherwise

@note

*/
/*=========================================================================*/
static int
qmi_qos_srvc_process_nw_status_report
(
unsigned char                 *rx_buf,
int                           rx_buf_len,
qmi_qos_nw_status_type        *reply_data
)
{
  unsigned long type;
  unsigned long length;
  unsigned char *value_ptr;

  /*Error Check*/
  if ((rx_buf == NULL) || (reply_data == NULL))
  {
    return QMI_INTERNAL_ERR;
  }

  reply_data->iface_name = QMI_QOS_INVALID_IFACE_NAME;

  while (rx_buf_len > 0)
  {
    if (qmi_util_read_std_tlv (&rx_buf,
                                  &rx_buf_len,
                                  &type,
                                  &length,
                                  &value_ptr) < 0)
    {
      return QMI_INTERNAL_ERR;
    }

    switch (type)
    {
      case QMI_QOS_NW_STATUS_QOS_SUPPORTED_TLV_ID:
      {
        unsigned char tmp_char;
        READ_8_BIT_VAL (value_ptr,tmp_char);
        reply_data->qos_supported = (qmi_qos_nw_support_status) tmp_char;
      }
      break;

      case QMI_QOS_NW_STATUS_IFACE_NAME_TLV_ID:
      {
        unsigned short tmp;
        READ_16_BIT_VAL (value_ptr,tmp);
        reply_data->iface_name = (qmi_qos_iface_name_type) tmp;
      }
      break;

      default:
      {
        QMI_DEBUG_MSG_2 ("qmi_qos_srvc_process_nw_status_report: Unknown TLV ID=%x, len=%d",
                         (unsigned int)type,(int)length);
      }
      break;
    }
  }

  return QMI_NO_ERR;
}



/*===========================================================================
  FUNCTION  qmi_qos_srvc_indication_cb
===========================================================================*/
/*!
@brief
  This is the callback function that will be called by the generic
  services layer to report asynchronous indications.  This function will
  process the indication TLV's and then call the user registered
  functions with the indication data.

@return
  None.

@note

  - Dependencies
    - qmi_connection_init() must be called for the associated port first.

  - Side Effects
    - Talks to modem processor
*/
/*=========================================================================*/

static void
qmi_qos_srvc_indication_cb
(
int                   user_handle,
qmi_service_id_type   service_id,
unsigned long         msg_id,
void                                *user_ind_msg_hdlr,
void                                *user_ind_msg_hdlr_user_data,
unsigned char         *rx_msg_buf,
int                   rx_msg_len
)
{

  qmi_qos_indication_id_type      ind_id;
  qmi_qos_indication_hdlr_type    user_ind_hdlr;
  qmi_qos_indication_data_type       ind_data;

  unsigned long type = 0;
  unsigned long length = 0;
  unsigned char *value_ptr = NULL;

  if (user_ind_msg_hdlr == NULL)
  {
    return;
  }

  memset((void*)&ind_data, 0x0, sizeof(ind_data));

  /* Get properly cast pointer to user indication handler */
  /*lint -e{611} */
  user_ind_hdlr = (qmi_qos_indication_hdlr_type) user_ind_msg_hdlr;

  switch (msg_id)
  {
    case QMI_QOS_EVENT_REPORT_IND_MSG_ID:
      {
        short  num_flows_counter = 0;

        ind_data.event_report.num_flows = 0;
        ind_id = QMI_QOS_SRVC_EVENT_REPORT_IND_MSG;
        while (rx_msg_len > 0)
        {
          if (qmi_util_read_std_tlv (&rx_msg_buf,
                                        &rx_msg_len,
                                        &type,
                                        &length,
                                        &value_ptr) < 0)
          {
            return ;
          }
          /*Check the correctness(type) of the tlv*/
          if (type == QMI_QOS_EVENT_FLOW_INFO_TLV_ID)
          {
            if (qmi_qos_srvc_process_event_report_ind (value_ptr,
                                                       (int)length,
                                                       &ind_data.event_report,
                                                       num_flows_counter) < 0)
            {
              QMI_ERR_MSG_0( "qmi_qos_srvc_indication_cb::QMI_ERR_SYSERR \n" );
              continue;
            }
            /*
              No Need to check for Buffer overflow here. Above API already
              checks that. Makes sure max of QMI_QOS_MAX_FLOW_EVENTS are
              filled in.
            */
            ind_data.event_report.param_mask |= QMI_QOS_EVENT_REPORT_GLOBAL_FLOW_INFO_PARAM;
            num_flows_counter++;
            ind_data.event_report.num_flows = num_flows_counter;
          }
          else if (type == QMI_QOS_NW_SUPPORTED_QOS_PROFILES_TLV_ID)
          {
            int num_instances = 0;
            int index;
            ind_data.event_report.param_mask |= QMI_QOS_EVENT_REPORT_NW_SUPPORTED_QOS_PROFILES_PARAM;
            READ_16_BIT_VAL (value_ptr, ind_data.event_report.nw_supported_qos_profiles.iface_type);
            READ_8_BIT_VAL (value_ptr,ind_data.event_report.nw_supported_qos_profiles.num_profiles);
            num_instances = ind_data.event_report.nw_supported_qos_profiles.num_profiles;
            for (index = 0; index < num_instances; index++)
            {
              READ_16_BIT_VAL(value_ptr,ind_data.event_report.nw_supported_qos_profiles.profile[index]);
            }
          }
          else
          {
            QMI_DEBUG_MSG_2 ("qmi_qos_srvc_indication_cb:: Unknown TLV ID=%x, len=%d",
                             (unsigned int)type,(int)length);
            continue;
          }
        } /* while */
      }
      break;

    case QMI_QOS_STATUS_REPORT_IND_MSG_ID:
      {
        ind_id =  QMI_QOS_SRVC_STATUS_REPORT_IND_MSG;

        if (qmi_qos_srvc_process_status_report_ind (rx_msg_buf,rx_msg_len,&ind_data.status_report) < 0)
        {
          QMI_DEBUG_MSG_0 ("qmi_qos_srvc_indication_cb::QMI_ERR_SYSERR \n");
          return;
        }
      }
      break;

    case QMI_QOS_NW_STATUS_REPORT_IND_MSG_ID:
      {
        ind_id = QMI_QOS_SRVC_NW_STATUS_REPORT_IND_MSG;

        if (qmi_qos_srvc_process_nw_status_report (rx_msg_buf,rx_msg_len,&ind_data.nw_report) < 0)
        {
          QMI_DEBUG_MSG_0 ("qmi_qos_srvc_indication_cb::QMI_ERR_SYSERR \n");
          return;
        }
      }
      break;

    case QMI_QOS_PRIMARY_QOS_EVENT_IND_MSG_ID:
      {
        int temp;
        ind_id = QMI_QOS_SRVC_PRIMARY_QOS_EVENT_IND_MSG;

        if (rx_msg_len && (qmi_util_read_std_tlv (&rx_msg_buf,
                                                     &rx_msg_len,
                                                     &type,
                                                     &length,
                                                     &value_ptr) < 0))
        {
          QMI_ERR_MSG_0( "qmi_qos_srvc_indication_cb::QMI_ERR_SYSERR \n" );
          return ;
        }

        if (type != QMI_QOS_STATUS_PRIMARY_QOS_MODIFY_RESULT_TLV_ID)
        {
          QMI_ERR_MSG_2( "qmi_qos_srvc_indication_cb:: Unknown TLV ID=%x, len=%d",
                         (unsigned int)type,(int)length);
          return;
        }

        READ_16_BIT_VAL (value_ptr,temp);
        ind_data.primary_qos_modify_result = (qmi_qos_primrary_qos_event_type) temp;
      }
      break;
    default:
      {
        QMI_DEBUG_MSG_1 ("qmi_qos_srvc_indication_cb: Unknown MsgId=%ld",msg_id);
        return;
      }
  }/*Switch*/

  user_ind_hdlr (user_handle,
                 service_id,
                 user_ind_msg_hdlr_user_data,
                 ind_id,
                 &ind_data);

}


/*===========================================================================
  FUNCTION  qmi_qos_write_flow_operation_request_tlvs
===========================================================================*/
/*!
@brief
  Takes the input UMTS profile data, and writes it in TLV form
  to the tx buffer.  Buffer pointers and length indicators are adjusted
  to reflect new TLV

@return
  QMI_INTERNAL_ERR if an error occurred, QMI_NO_ERR if not

@note

  - Dependencies
    - None.

  - Side Effects
    - Writes TLV to input buffer and updates pointers and byte count
*/
/*=========================================================================*/

static int
qmi_qos_write_flow_operation_request_tlvs
(
  unsigned char                        **tx_buf,
  int                                  *tx_buf_len,
  qmi_qos_perform_flow_op_req_type     *params
)
{
  if (params->params_mask & QMI_QOS_PERFORM_FLOW_OP_REQ_OPERATIONS_PARAM)
  {
    if (qmi_util_write_std_tlv (tx_buf,
                                   tx_buf_len,
                                   QMI_QOS_PERFORM_FLOW_OP_REQUESTED_OP_TLV_ID,
                                   4,
                                   (void *)&params->requested_operation) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }

  if (params->params_mask & QMI_QOS_PERFORM_FLOW_OP_SET_INACTIVITY_TIMER_PARAM)
  {
    if (qmi_util_write_std_tlv (tx_buf,
                                   tx_buf_len,
                                   QMI_QOS_PERFORM_FLOW_SET_INACTIVITY_TIMER_TLV_ID,
                                   4,
                                   (void *)&params->set_inactivity_timer) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }

  if (params->params_mask & QMI_QOS_PERFORM_FLOW_OP_PRIMARY_FLOW_OP_PARAM)
  {
    if (qmi_util_write_std_tlv (tx_buf,
                                   tx_buf_len,
                                   QMI_QOS_PERFORM_FLOW_PRIMARY_FLOW_OP_TLV_ID,
                                   1,
                                   (void *)&params->primary_flow_op) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }
  return QMI_NO_ERR;
}

/*===========================================================================
  FUNCTION  qmi_qos_srvc_init
===========================================================================*/
/*!
@brief
  This function is a callback that will be called once during client
  initialization

@return
  None.

@note

  - Dependencies
    - None.

  - Side Effects
    - None.
*/
/*=========================================================================*/
int qmi_qos_srvc_init (void)
{
  int  rc = QMI_NO_ERR;
  if (!qos_service_initialized)
  {

    rc = qmi_service_set_srvc_functions (QMI_QOS_SERVICE,
                                         qmi_qos_srvc_indication_cb);
    if (rc != QMI_NO_ERR)
    {
      QMI_ERR_MSG_1("qmi_qos_srvc_init: set srvc functions returns err=%d\n",rc);
    }
    else
    {
      QMI_DEBUG_MSG_0("qmi_qos_srvc_init: QOS successfully initialized");
      qos_service_initialized = TRUE;
    }
  }
  else
  {
    QMI_ERR_MSG_0 ("qmi_qos_srvc_init: Init failed, QOS already initialized");
  }
  return rc;
}


/*===========================================================================
  FUNCTION  qmi_qos_srvc_release
===========================================================================*/
/*!
@brief
  This function is a callback that will be called once during client
  release

@return
  None.

@note

  - Dependencies
    - None.

  - Side Effects
    - None.
*/
/*=========================================================================*/
int qmi_qos_srvc_release (void)
{
  int  rc = QMI_NO_ERR;
  if (qos_service_initialized)
  {

    rc = qmi_service_set_srvc_functions (QMI_QOS_SERVICE,NULL);
    if (rc != QMI_NO_ERR)
    {
      QMI_ERR_MSG_1("qmi_qos_srvc_release: set srvc functions returns err=%d\n",rc);
    }
    else
    {
      QMI_DEBUG_MSG_0("qmi_qos_srvc_release: QOS successfully released");
      qos_service_initialized = FALSE;
    }
  }
  else
  {
    QMI_ERR_MSG_0 ("qmi_qos_srvc_release: Release failed, QOS not initialized");
  }
  return rc;
}


/*===========================================================================
  FUNCTION  qmi_qos_srvc_init_client
===========================================================================*/
/*!
@brief
  This function is called to initialize the QoS service.  This function
  must be called prior to calling any other QoS service functions.
  For the time being, the indication handler callback and user data
  should be set to NULL until this is implemented.  Also note that this
  function may be called multiple times to allow for multiple, independent
  clients.

@return
  0 if abort operation was sucessful, < 0 if not.  If return code is
  QMI_INTERNAL_ERR, then the qmi_err_code will be valid and will
  indicate which QMI error occurred.

@note

  - Dependencies
    - qmi_connection_init() must be called for the associated port first.

  - Side Effects
    - Talks to modem processor
*/
/*=========================================================================*/
qmi_client_handle_type
qmi_qos_srvc_init_client
(
  const char                    *dev_id,
  qmi_qos_indication_hdlr_type  ind_hdlr,
  void                          *ind_hdlr_user_data,
  int                           *qmi_err_code
)
{
  int client_handle;
  qmi_connection_id_type conn_id;
  int mux_id = -1;
  int rc;
  int qmi_err;
  qmi_qos_bind_mux_data_port_params_type bind_params;
  int ep_type = -1;
  int epid = -1;

  if (QMI_CONN_ID_INVALID ==
      (conn_id = QMI_PLATFORM_DEV_NAME_TO_CONN_ID_EX(dev_id, &ep_type, &epid, &mux_id)))
  {
    return QMI_INTERNAL_ERR;
  }

  /* Call common service layer initialization function */
  /*lint -e{611} */
  client_handle =  qmi_service_init (conn_id,
                                   QMI_QOS_SERVICE,
                                   (void*)ind_hdlr,
                                   ind_hdlr_user_data,
                                   qmi_err_code);

  if (client_handle >= 0 && 0 < mux_id)
  {
    /* Prepare bind_params to bind the client to a EPID/mux id */
    memset(&bind_params, 0, sizeof(bind_params));
    bind_params.params_mask |= QMI_QOS_BIND_MUX_DATA_PORT_PARAMS_MUX_ID;
    bind_params.mux_id = (unsigned char) mux_id;

    if(ep_type != -1 )
    {
      /* We have a valid end point and a valid ep id */
      bind_params.params_mask |= QMI_QOS_BIND_MUX_DATA_PORT_PARAMS_EP_ID;
      bind_params.ep_id.ep_type = (qmi_qos_per_ep_type) ep_type;
      bind_params.ep_id.iface_id = (unsigned long) epid;
    }

    /* Bind QoS client to EPID/MUX-ID */
    (void) qmi_qos_bind_mux_data_port(client_handle, &bind_params, &qmi_err);
  }

  return client_handle;
}

/*===========================================================================
  FUNCTION  qmi_qos_srvc_release_client
===========================================================================*/
/*!
@brief
  This function is called to release a client created by the
  qmi_qos_srvc_init_client() function.  This function should be called
  for any client created when terminating a client process, especially
  if the modem processor is not reset.  The modem side QMI server has
  a limited number of clients that it will allocate, and if they are not
  released, we will run out.

@return
  0 if abort operation was sucessful, < 0 if not.  If return code is
  QMI_INTERNAL_ERR, then the qmi_err_code will be valid and will
  indicate which QMI error occurred.

@note

  - Dependencies
    - qmi_connection_init() must be called for the associated port first.

  - Side Effects
    - Talks to modem processor
*/
/*=========================================================================*/
int
qmi_qos_srvc_release_client
(
  qmi_client_handle_type  client_handle,
  int                     *qmi_err_code
)
{
  int rc;
  rc = qmi_service_release (client_handle, qmi_err_code);
  return rc;
}

/*===========================================================================
  FUNCTION  qmi_qos_bind_mux_data_port
===========================================================================*/
/*!
@brief
  Binds a QMI-QOS client to a MUX data port. In the case of QMAP, we will
  have a single control channel, so QOS clients need to specify which
  data port their actions need to be associated with.

@param[in]  user_handle: QMI QOS client handle.
@param[in]  params: Bind MUX data port specification
@param[out] qmi_err_code: QMI error code in case of failure.

@see
  qmi_qos_bind_mux_data_port_params_type

@return
  0 if operation was successful.
  < 0 If operation failed.  qmi_err_code will contain the reason.

@dependencies
  qmi_qos_srvc_init_client() must be called before calling this.

*/
/*=========================================================================*/
int
qmi_qos_bind_mux_data_port
(
  int                                     user_handle,
  qmi_qos_bind_mux_data_port_params_type *params,
  int                                    *qmi_err_code
)
{
  unsigned char     msg[QMI_QOS_STD_MSG_SIZE];
  int               msg_size;
  unsigned char    *tmp_msg_ptr;
  int rc;

  if (NULL == qmi_err_code || NULL == params)
  {
    QMI_ERR_MSG_0("qmi_qos_bind_mux_data_port(): Bad Input received\n");
    return QMI_INTERNAL_ERR;
  }

  *qmi_err_code = QMI_SERVICE_ERR_NONE;
  QMI_DEBUG_MSG_2("qmi_qos_bind_mux_data_port(): ENTRY: user handle %d, params_mask 0x%x",
      user_handle, params->params_mask);
  QMI_DEBUG_MSG_4("qmi_qos_bind_mux_data_port(): ENTRY: ep type %d, ep_id 0x%x, mux_id %d, reversed %d",
      params->ep_id.ep_type, params->ep_id.iface_id, params->mux_id, params->reversed);

  /* Set tmp_msg_ptr to beginning of message-specific TLV portion of
  ** message buffer */
  tmp_msg_ptr = QMI_SRVC_PDU_PTR(msg);

  /* Set the message size to the complete buffer minus the header size */
  msg_size = QMI_SRVC_PDU_SIZE(QMI_QOS_STD_MSG_SIZE);

  /* Write EP ID Type TLV (size 8 bytes) if appropriate */
  if (params->params_mask & QMI_QOS_BIND_MUX_DATA_PORT_PARAMS_EP_ID)
  {
    unsigned char tmp_buf[8];
    unsigned char *tmp_buf_ptr = tmp_buf;

    /* Set TLV based on parameter value */
    WRITE_32_BIT_VAL(tmp_buf_ptr, (unsigned long)(params->ep_id.ep_type));
    WRITE_32_BIT_VAL(tmp_buf_ptr, params->ep_id.iface_id);
    tmp_buf_ptr = tmp_buf;

    if (qmi_util_write_std_tlv (&tmp_msg_ptr,
                                &msg_size,
                                QMI_QOS_BIND_MUX_DATA_PORT_EP_ID_TLV_ID,
                                8,
                                (void *)tmp_buf_ptr) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }

  /* Write MUX ID Type TLV (size 1 byte) if appropriate */
  if (params->params_mask & QMI_QOS_BIND_MUX_DATA_PORT_PARAMS_MUX_ID)
  {
    if (qmi_util_write_std_tlv (&tmp_msg_ptr,
                                &msg_size,
                                QMI_QOS_BIND_MUX_DATA_PORT_MUX_ID_TLV_ID,
                                1,
                                (void *)&params->mux_id) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }

  /* Write Reversed TLV (size 1 byte) if appropriate */
  if (params->params_mask & QMI_QOS_BIND_MUX_DATA_PORT_PARAMS_REVERSED)
  {
    if (qmi_util_write_std_tlv (&tmp_msg_ptr,
                                &msg_size,
                                QMI_QOS_BIND_MUX_DATA_PORT_REVERSED_TLV_ID,
                                1,
                                (void *)&params->reversed) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }

  rc = qmi_service_send_msg_sync (user_handle,
                                  QMI_QOS_SERVICE,
                                  QMI_QOS_BIND_MUX_DATA_PORT_MSG_ID,
                                  QMI_SRVC_PDU_PTR(msg),
                                  (int)QMI_SRVC_PDU_SIZE(QMI_QOS_STD_MSG_SIZE) - msg_size,
                                  msg,
                                  &msg_size,
                                  QMI_QOS_STD_MSG_SIZE,
                                  QMI_SYNC_MSG_DEFAULT_TIMEOUT,
                                  qmi_err_code);
  return rc;
}

/*===========================================================================
  FUNCTION  qmi_qos_bind_subscription
===========================================================================*/
/*!
@brief
 Binds a QoS client to a subscription.

@return
  QMI_NO_ERR if operation was sucessful, < 0 if not.  If return code is
  QMI_SERVICE_ERR, then the qmi_err_code will be valid and will
  indicate which QMI error occurred.

@note
  - This function executes synchronously, there is not currently an
    asynchronous option for this functionality.

  - Dependencies
    - None.

  - Side Effects
    - None.
*/
/*=========================================================================*/
int
qmi_qos_bind_subscription
(
  int                          user_handle,
  qmi_qos_bind_subscription_type  subs_id,
  int                         *qmi_err_code
)
{
  unsigned char msg[QMI_QOS_STD_MSG_SIZE];
  int msg_size;
  unsigned char *pdu;

  if (!qmi_err_code)
  {
    QMI_ERR_MSG_1("qmi_qos_set_client_ip_pref bad parameters qmi_err_code=%p",
                  qmi_err_code);
    return QMI_INTERNAL_ERR;
  }

  pdu = QMI_SRVC_PDU_PTR(msg);
  msg_size = QMI_SRVC_PDU_SIZE(QMI_QOS_STD_MSG_SIZE);

  if (qmi_util_write_std_tlv(&pdu,
                             &msg_size,
                             QMI_QOS_BIND_SUBS_ID_REQ_TLV_ID,
                             4,
                             &subs_id) < 0)
  {
    return QMI_INTERNAL_ERR;
  }

  return qmi_service_send_msg_sync(user_handle,
                                   QMI_QOS_SERVICE,
                                   QMI_QOS_BIND_SUBSCRIPTION_MSG_ID,
                                   QMI_SRVC_PDU_PTR(msg),
                                   (int)QMI_SRVC_PDU_SIZE(QMI_QOS_STD_MSG_SIZE) - msg_size,
                                   msg,
                                   &msg_size,
                                   QMI_QOS_STD_MSG_SIZE,
                                   QMI_SYNC_MSG_DEFAULT_TIMEOUT,
                                   qmi_err_code);

}

/*===========================================================================
  FUNCTION  qmi_qos_get_bind_subscription
===========================================================================*/
/*!
@brief
  This message queries the current subscription the client is bound to.

@return
  QMI_NO_ERR if operation was sucessful, < 0 if not.  If return code is
  QMI_SERVICE_ERR, then the qmi_err_code will be valid and will
  indicate which QMI error occurred.

@note

  - This function executes synchronously, there is not currently an
    asynchronous option for this functionality.

  - Dependencies
    - None.

  - Side Effects
    - None.
*/
/*=========================================================================*/
int
qmi_qos_get_bind_subscription
(
  int                                          user_handle,
  qmi_qos_bind_subscription_type               *subs_id,
  int                                          *qmi_err_code
)
{
  unsigned char     msg[QMI_QOS_STD_MSG_SIZE];
  int               msg_size;
  unsigned char    *tmp_msg_ptr;
  int rc;
  int temp;

  if (!subs_id )
  {
    return QMI_INTERNAL_ERR;
  }
  if (!qmi_err_code )
  {
    return QMI_INTERNAL_ERR;
  }

  /* Set tmp_msg_ptr to beginning of message-specific TLV portion of
  ** message buffer
  */
  tmp_msg_ptr = QMI_SRVC_PDU_PTR(msg);

  /* Set the message size to the complete buffer minus the header size */
  msg_size = QMI_SRVC_PDU_SIZE(QMI_QOS_STD_MSG_SIZE);

  rc = qmi_service_send_msg_sync (user_handle,
                                  QMI_QOS_SERVICE,
                                  QMI_QOS_GET_BIND_SUBSCRIPTION_MSG_ID,
                                  QMI_SRVC_PDU_PTR(msg),
                                  (int)QMI_SRVC_PDU_SIZE(QMI_QOS_STD_MSG_SIZE) - msg_size,
                                  msg,
                                  &msg_size,
                                  QMI_QOS_STD_MSG_SIZE,
                                  QMI_SYNC_MSG_DEFAULT_TIMEOUT,
                                  qmi_err_code);

  if (rc == QMI_NO_ERR)
  {
    unsigned long type;
    unsigned long length;
    unsigned char *value_ptr;

    /* Set tmp_msg_ptr to return data */
    tmp_msg_ptr = msg;

    while (msg_size > 0)
    {
      if (qmi_util_read_std_tlv (&tmp_msg_ptr,
                                    &msg_size,
                                    &type,
                                    &length,
                                    &value_ptr) < 0)
      {
        return QMI_INTERNAL_ERR;
      }

      switch (type)
      {
        case QMI_QOS_GET_BIND_SUB_ID_TLV_ID:
          {
            READ_32_BIT_VAL (value_ptr, temp);
            *subs_id = (qmi_qos_bind_subscription_type) temp;
          }
          break;
        default:
          {
            QMI_ERR_MSG_1 ("qmi_qos_get_bind_subscription: unknown response TLV type = %x",(unsigned int)type);
          }
          break;
      }
    }
  }
  return rc;
}

static int
qmi_qos_format_single_flow_tlvs
(
unsigned char           **msg_buf,
int                     *msg_buf_size,
int                     flow_index,
qmi_qos_flow_req_type   *flow_data
)
{
  int rc;
  unsigned char tmp_buf[20],*tmp_buf_ptr;
  INIT_ENVELOPE_TLV_HDR (*msg_buf,*msg_buf_size);

  /* Add flow index TLV */
  QMI_QOSWRITE_TLV (QMI_QOS_FLOW_INDEX_REQ_RSP_TLV_ID,1,&flow_index);

  /* CDMA profile */
  if (flow_data->cdma_flow_desc.param_mask & QMI_QOS_CDMA_FLOW_PARAM_PROFILE_ID)
  {
    QMI_QOSWRITE_TLV (QMI_QOS_FLOW_CDMA_PROFILE_ID_REQ_TLV_ID,
                      2,&flow_data->cdma_flow_desc.profile_id);
  }

  /* UMTS profile */
    if (flow_data->umts_flow_desc.param_mask & QMI_QOS_UMTS_FLOW_PARAM_TRAFFIC_CLASS)
    {
      QMI_QOSWRITE_TLV (QMI_QOS_FLOW_UMTS_TC_REQ_RSP_TLV_ID,
                        1,&flow_data->umts_flow_desc.traffic_class);
    }
    if (flow_data->umts_flow_desc.param_mask & QMI_QOS_UMTS_FLOW_PARAM_DATA_RATE)
    {
      tmp_buf_ptr = tmp_buf;
      WRITE_32_BIT_VAL (tmp_buf_ptr, flow_data->umts_flow_desc.data_rate.max_rate);
      WRITE_32_BIT_VAL (tmp_buf_ptr, flow_data->umts_flow_desc.data_rate.guaranteed_rate);
      QMI_QOSWRITE_TLV (QMI_QOS_FLOW_UMTS_DATA_RATE_REQ_RSP_TLV_ID,
                        8,tmp_buf);
    }
    if (flow_data->umts_flow_desc.param_mask & QMI_QOS_UMTS_FLOW_PARAM_BUCKET_INFO)
    {
      tmp_buf_ptr = tmp_buf;
      WRITE_32_BIT_VAL (tmp_buf_ptr, flow_data->umts_flow_desc.bucket_info.peak_rate);
      WRITE_32_BIT_VAL (tmp_buf_ptr, flow_data->umts_flow_desc.bucket_info.token_rate);
      WRITE_32_BIT_VAL (tmp_buf_ptr, flow_data->umts_flow_desc.bucket_info.bucket_size);
      QMI_QOSWRITE_TLV (QMI_QOS_FLOW_UMTS_BUCKET_INFO_REQ_RSP_TLV_ID,
                        12,tmp_buf);
    }
    if (flow_data->umts_flow_desc.param_mask & QMI_QOS_UMTS_FLOW_PARAM_MAX_DELAY)
    {
      QMI_QOSWRITE_TLV (QMI_QOS_FLOW_UMTS_MAX_DELAY_REQ_RSP_TLV_ID,
                        4,&flow_data->umts_flow_desc.max_delay);
    }
    if (flow_data->umts_flow_desc.param_mask & QMI_QOS_UMTS_FLOW_PARAM_MAX_JITTER)
    {
      QMI_QOSWRITE_TLV (QMI_QOS_FLOW_UMTS_MAX_JITTER_REQ_RSP_TLV_ID,
                        4,&flow_data->umts_flow_desc.max_jitter);
    }
    if (flow_data->umts_flow_desc.param_mask & QMI_QOS_UMTS_FLOW_PARAM_PKT_ERR_RATE)
    {
      tmp_buf_ptr = tmp_buf;
      WRITE_16_BIT_VAL (tmp_buf_ptr, flow_data->umts_flow_desc.pkt_err_rate.multiplier);
      WRITE_16_BIT_VAL (tmp_buf_ptr, flow_data->umts_flow_desc.pkt_err_rate.exponent);
      QMI_QOSWRITE_TLV (QMI_QOS_FLOW_UMTS_PKT_ERR_RATE_REQ_RSP_TLV_ID,
                        4,tmp_buf);
    }
    if (flow_data->umts_flow_desc.param_mask & QMI_QOS_UMTS_FLOW_PARAM_MIN_POL_PKT_SZ)
    {
      QMI_QOSWRITE_TLV (QMI_QOS_FLOW_UMTS_MIN_POL_PKT_SZ_REQ_RSP_TLV_ID,
                        4,&flow_data->umts_flow_desc.min_policed_pkt_sz);
    }
    if (flow_data->umts_flow_desc.param_mask & QMI_QOS_UMTS_FLOW_PARAM_MAX_ALLOW_PKT_SZ)
    {
      QMI_QOSWRITE_TLV (QMI_QOS_FLOW_UMTS_MAX_ALLOW_PKT_SZ_REQ_RSP_TLV_ID,
                        4,&flow_data->umts_flow_desc.max_allowed_pkt_sz);
    }
    if (flow_data->umts_flow_desc.param_mask & QMI_QOS_UMTS_FLOW_PARAM_RESIDUAL_BER)
    {
      QMI_QOSWRITE_TLV (QMI_QOS_FLOW_UMTS_RESIDUAL_BER_REQ_RSP_TLV_ID,
                        1,&flow_data->umts_flow_desc.residual_ber);
    }
    if (flow_data->umts_flow_desc.param_mask & QMI_QOS_UMTS_FLOW_PARAM_HANDLING_PRIO)
    {
      QMI_QOSWRITE_TLV (QMI_QOS_FLOW_UMTS_HANDLING_PRIO_REQ_RSP_TLV_ID,
                        1,&flow_data->umts_flow_desc.handling_prio);
    }
    if (flow_data->umts_flow_desc.param_mask & QMI_QOS_UMTS_FLOW_PARAM_3GPP2_FLOW_PRIO)
    {
      QMI_QOSWRITE_TLV (QMI_QOS_FLOW_UMTS_3GPP2_IP_FLOW_PRIORITY_REQ_RSP_TLV_ID,
                        1,&flow_data->umts_flow_desc.flow_priority_3gpp2);
    }
    if (flow_data->umts_flow_desc.param_mask & QMI_QOS_UMTS_FLOW_PARAM_IM_CN_FLAG)
    {
      QMI_QOSWRITE_TLV (QMI_QOS_FLOW_UMTS_IM_CN_FLAG_REQ_RSP_TLV_ID,
                        1,&flow_data->umts_flow_desc.im_cn_flag);
    }
    if (flow_data->umts_flow_desc.param_mask & QMI_QOS_UMTS_FLOW_PARAM_IP_FLOW_SIG_IND)
    {
      QMI_QOSWRITE_TLV (QMI_QOS_FLOW_UMTS_3GPP_IP_FLOW_SIG_IND_REQ_RSP_TLV_ID,
                        1,&flow_data->umts_flow_desc.ip_flow_sig_ind);
    }

    if (flow_data->umts_flow_desc.param_mask & QMI_QOS_LTE_FLOW_PARAM_QCI_IND)
    {
      QMI_QOSWRITE_TLV (QMI_QOS_FLOW_LTE_QCI_REQ_RSP_TLV_ID,
                        1,&flow_data->umts_flow_desc.lte_qci);
    }

  SET_ENVELOPE_TLV_HDR (QMI_QOS_FLOW_SPEC_REQ_RSP_TLV_ID);
  return tlv_length;
}







static int
qmi_qos_format_all_flow_tlvs
(
unsigned char           **msg_buf,
int                     *msg_buf_size,
unsigned char           tlv_id,
int                     num_flow_req,
qmi_qos_flow_req_type   *flow_data
)
{
  int rc, flow_index;
  INIT_ENVELOPE_TLV_HDR (*msg_buf,*msg_buf_size);

  for (flow_index=0; flow_index < num_flow_req; flow_index++)
  {
    if ((rc = qmi_qos_format_single_flow_tlvs (msg_buf,
                                               msg_buf_size,
                                               flow_index,
                                               flow_data++)) < 0)
    {
      return QMI_INTERNAL_ERR;
    }

    tlv_length += rc;
  }

  SET_ENVELOPE_TLV_HDR (tlv_id);
  return tlv_length;
}




static int
qmi_qos_format_single_filter_tlvs
(
unsigned char           **msg_buf,
int                     *msg_buf_size,
int                     filter_index,
qmi_qos_filter_req_type   *filter_data
)
{
  int rc;
  unsigned char tmp_buf[20],*tmp_buf_ptr;
  INIT_ENVELOPE_TLV_HDR (*msg_buf,*msg_buf_size);

  /* Add filter index TLV */
  QMI_QOSWRITE_TLV (QMI_QOS_FILTER_INDEX_REQ_RSP_TLV_ID,1,&filter_index);

  /* Add filter version information */
  QMI_QOSWRITE_TLV (QMI_QOS_FILTER_IP_VER_REQ_RSP_TLV_ID,1,&filter_data->ip_version);

  if (filter_data->filter_desc.param_mask & QMI_QOS_FILTER_PARAM_SRC_ADDR)
  {
    tmp_buf_ptr = tmp_buf;
    WRITE_32_BIT_VAL (tmp_buf_ptr, filter_data->filter_desc.src_addr.ipv4_ip_addr);
    WRITE_32_BIT_VAL (tmp_buf_ptr, filter_data->filter_desc.src_addr.ipv4_subnet_mask);
    QMI_QOSWRITE_TLV (QMI_QOS_FILTER_SRC_ADDR_REQ_RSP_TLV_ID,
                      8,tmp_buf);
  }
  if (filter_data->filter_desc.param_mask & QMI_QOS_FILTER_PARAM_DEST_ADDR)
  {
    tmp_buf_ptr = tmp_buf;
    WRITE_32_BIT_VAL (tmp_buf_ptr, filter_data->filter_desc.dest_addr.ipv4_ip_addr);
    WRITE_32_BIT_VAL (tmp_buf_ptr, filter_data->filter_desc.dest_addr.ipv4_subnet_mask);
    QMI_QOSWRITE_TLV (QMI_QOS_FILTER_DEST_ADDR_REQ_RSP_TLV_ID,
                      8,tmp_buf);
  }
  if (filter_data->filter_desc.param_mask & QMI_QOS_FILTER_PARAM_TRANS_PROTOCOL)
  {
    QMI_QOSWRITE_TLV (QMI_QOS_FILTER_TRANS_PROTOCOL_REQ_RSP_TLV_ID,
                      1,&filter_data->filter_desc.protocol);
  }
  if (filter_data->filter_desc.param_mask & QMI_QOS_FILTER_PARAM_TOS)
  {
    tmp_buf_ptr = tmp_buf;
    WRITE_8_BIT_VAL (tmp_buf_ptr, filter_data->filter_desc.tos.tos_value);
    WRITE_8_BIT_VAL (tmp_buf_ptr, filter_data->filter_desc.tos.tos_mask);
    QMI_QOSWRITE_TLV (QMI_QOS_FILTER_TOS_REQ_RSP_TLV_ID,
                      2,tmp_buf);
  }
  if (filter_data->filter_desc.param_mask & QMI_QOS_FILTER_PARAM_IPV6_SRC_ADDR)
  {
    tmp_buf_ptr = tmp_buf;
    memcpy(tmp_buf_ptr,filter_data->filter_desc.ipv6_src_addr.ipv6_ip_addr,
           sizeof(qmi_qos_ipv6_addr_type));
    tmp_buf_ptr += sizeof(qmi_qos_ipv6_addr_type);
    WRITE_8_BIT_VAL (tmp_buf_ptr, filter_data->filter_desc.ipv6_src_addr.ipv6_filter_prefix_len);
    QMI_QOSWRITE_TLV (QMI_QOS_FILTER_IPV6_SRC_ADDR_REQ_RSP_TLV_ID,
                      17,tmp_buf);
  }
  if (filter_data->filter_desc.param_mask & QMI_QOS_FILTER_PARAM_IPV6_DEST_ADDR)
  {
    tmp_buf_ptr = tmp_buf;
    memcpy(tmp_buf_ptr,filter_data->filter_desc.ipv6_dest_addr.ipv6_ip_addr,
           sizeof(qmi_qos_ipv6_addr_type));
    tmp_buf_ptr += sizeof(qmi_qos_ipv6_addr_type);
    WRITE_8_BIT_VAL (tmp_buf_ptr, filter_data->filter_desc.ipv6_dest_addr.ipv6_filter_prefix_len);
    QMI_QOSWRITE_TLV (QMI_QOS_FILTER_IPV6_DST_ADDR_REQ_RSP_TLV_ID,
                      17,tmp_buf);
  }
  if (filter_data->filter_desc.param_mask & QMI_QOS_FILTER_PARAM_IPV6_TRAFFIC_CLASS)
  {
    tmp_buf_ptr = tmp_buf;
    WRITE_8_BIT_VAL (tmp_buf_ptr, filter_data->filter_desc.ipv6_traffic_class.traffic_class_value);
    WRITE_8_BIT_VAL (tmp_buf_ptr, filter_data->filter_desc.ipv6_traffic_class.traffic_class_mask);
    QMI_QOSWRITE_TLV (QMI_QOS_FILTER_IPV6_TRAFFIC_CLASS_REQ_RSP_TLV_ID,
                      2,tmp_buf);
  }
  if (filter_data->filter_desc.param_mask & QMI_QOS_FILTER_PARAM_IPV6_FLOW_LABEL)
  {
    tmp_buf_ptr = tmp_buf;
    WRITE_32_BIT_VAL (tmp_buf_ptr, filter_data->filter_desc.ipv6_flow_label);
    QMI_QOSWRITE_TLV (QMI_QOS_FILTER_IPV6_FLOW_LABEL_REQ_RSP_TLV_ID,
                      4,tmp_buf);
  }
  if (filter_data->filter_desc.param_mask & QMI_QOS_FILTER_PARAM_TCP_SRC_PORTS)
  {
    tmp_buf_ptr = tmp_buf;
    WRITE_16_BIT_VAL (tmp_buf_ptr, filter_data->filter_desc.tcp_src_ports.start_port);
    WRITE_16_BIT_VAL (tmp_buf_ptr, filter_data->filter_desc.tcp_src_ports.range);
    QMI_QOSWRITE_TLV (QMI_QOS_FILTER_TCP_SRC_PORT_REQ_RSP_TLV_ID,
                      4,tmp_buf);
  }
  if (filter_data->filter_desc.param_mask & QMI_QOS_FILTER_PARAM_TCP_DEST_PORTS)
  {
    tmp_buf_ptr = tmp_buf;
    WRITE_16_BIT_VAL (tmp_buf_ptr, filter_data->filter_desc.tcp_dest_ports.start_port);
    WRITE_16_BIT_VAL (tmp_buf_ptr, filter_data->filter_desc.tcp_dest_ports.range);
    QMI_QOSWRITE_TLV (QMI_QOS_FILTER_TCP_DEST_PORT_REQ_RSP_TLV_ID,
                      4,tmp_buf);
  }
  if (filter_data->filter_desc.param_mask & QMI_QOS_FILTER_PARAM_UDP_SRC_PORTS)
  {
    tmp_buf_ptr = tmp_buf;
    WRITE_16_BIT_VAL (tmp_buf_ptr, filter_data->filter_desc.udp_src_ports.start_port);
    WRITE_16_BIT_VAL (tmp_buf_ptr, filter_data->filter_desc.udp_src_ports.range);
    QMI_QOSWRITE_TLV (QMI_QOS_FILTER_UDP_SRC_PORT_REQ_RSP_TLV_ID,
                      4,tmp_buf);
  }
  if (filter_data->filter_desc.param_mask & QMI_QOS_FILTER_PARAM_UDP_DEST_PORTS)
  {
    tmp_buf_ptr = tmp_buf;
    WRITE_16_BIT_VAL (tmp_buf_ptr, filter_data->filter_desc.udp_dest_ports.start_port);
    WRITE_16_BIT_VAL (tmp_buf_ptr, filter_data->filter_desc.udp_dest_ports.range);
    QMI_QOSWRITE_TLV (QMI_QOS_FILTER_UDP_DEST_PORT_REQ_RSP_TLV_ID,
                      4,tmp_buf);
  }
  if (filter_data->filter_desc.param_mask & QMI_QOS_FILTER_PARAM_ESP_SECURITY_POLICY)
  {
    tmp_buf_ptr = tmp_buf;
    WRITE_32_BIT_VAL (tmp_buf_ptr, filter_data->filter_desc.esp_security_policy_index);
    QMI_QOSWRITE_TLV (QMI_QOS_FILTER_ESP_SECURITY_POLICY_REQ_RSP_TLV_ID,
                      4,tmp_buf);
  }
   if (filter_data->filter_desc.param_mask & QMI_QOS_FILTER_PARAM_PRECEDENCE)
  {
    tmp_buf_ptr = tmp_buf;
    WRITE_8_BIT_VAL (tmp_buf_ptr, filter_data->filter_desc.precedence);
    QMI_QOSWRITE_TLV (QMI_QOS_FILTER_PRECEDENCE_REQ_RSP_TLV_ID,
                      1,tmp_buf);
  }
   if (filter_data->filter_desc.param_mask & QMI_QOS_FILTER_PARAM_FILTER_ID)
  {
    tmp_buf_ptr = tmp_buf;
    WRITE_8_BIT_VAL (tmp_buf_ptr, filter_data->filter_desc.filter_id);
    QMI_QOSWRITE_TLV (QMI_QOS_FILTER_FILTER_ID_REQ_RSP_TLV_ID,
                      1,tmp_buf);
  }
   if (filter_data->filter_desc.param_mask & QMI_QOS_FILTER_PARAM_TRANSPORT_SRC_PORTS)
  {
    tmp_buf_ptr = tmp_buf;
    WRITE_16_BIT_VAL (tmp_buf_ptr, filter_data->filter_desc.transport_src_ports.start_port);
    WRITE_16_BIT_VAL (tmp_buf_ptr, filter_data->filter_desc.transport_src_ports.range);
    QMI_QOSWRITE_TLV (QMI_QOS_FILTER_TRANSPORT_SRC_PORT_REQ_RSP_TLV_ID,
                      4,tmp_buf);
  }
   if (filter_data->filter_desc.param_mask & QMI_QOS_FILTER_PARAM_TRANSPORT_DEST_PORTS)
  {
    tmp_buf_ptr = tmp_buf;
    WRITE_16_BIT_VAL (tmp_buf_ptr, filter_data->filter_desc.transport_dest_ports.start_port);
    WRITE_16_BIT_VAL (tmp_buf_ptr, filter_data->filter_desc.transport_dest_ports.range);
    QMI_QOSWRITE_TLV (QMI_QOS_FILTER_TRANSPORT_DEST_PORT_REQ_RSP_TLV_ID,
                      4,tmp_buf);
  }
  SET_ENVELOPE_TLV_HDR (QMI_QOS_FILTER_SPEC_REQ_RSP_TLV_ID);
  return tlv_length;
}




static int
qmi_qos_format_all_filter_tlvs
(
unsigned char           **msg_buf,
int                     *msg_buf_size,
unsigned char           tlv_id,
int                     num_filter_req,
qmi_qos_filter_req_type   *filter_data
)
{
  int rc, filter_index;
  INIT_ENVELOPE_TLV_HDR (*msg_buf,*msg_buf_size);

  for (filter_index=0; filter_index < num_filter_req; filter_index++)
  {
    if ((rc = qmi_qos_format_single_filter_tlvs (msg_buf,
                                               msg_buf_size,
                                               filter_index,
                                               filter_data++)) < 0)
    {
      return QMI_INTERNAL_ERR;
    }

    tlv_length += rc;
  }

  SET_ENVELOPE_TLV_HDR (tlv_id);
  return tlv_length;
}


#define QMI_QOS_REQUEST_QOS      0x01
#define QMI_QOS_MODIFY_QOS       0x02

static int
qmi_qos_format_spec_req_tlvs
(
unsigned char           **msg_buf,
int                     *msg_buf_size,
int                     spec_index,
qmi_qos_spec_type   *qos_spec,
unsigned char           req_modify
)
{
  int rc;
  INIT_ENVELOPE_TLV_HDR (*msg_buf,*msg_buf_size);

  /* Add spec index TLV */
  QMI_QOSWRITE_TLV (QMI_QOS_SPEC_INDEX_REQ_RSP_TLV_ID,1,&spec_index);

  if (req_modify == QMI_QOS_MODIFY_QOS)
    QMI_QOSWRITE_TLV (QMI_QOS_SPEC_QOS_ID_REQ_RSP_TLV_ID,4,&qos_spec->qos_identifier);

  /* Format TX flow TLV's if there are any */
  if (qos_spec->num_tx_flow_req >0)
  {
    if ((rc = qmi_qos_format_all_flow_tlvs (msg_buf,
                                            msg_buf_size,
                                            QMI_QOS_TX_FLOW_REQ_RSP_TLV_ID,
                                            (int)qos_spec->num_tx_flow_req,
                                            qos_spec->tx_flow_req_array)) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
    /* return from function is size of embedded TLV */
    tlv_length += rc;
  }

  /* Format RX flow TLV's if there are any */
  if (qos_spec->num_rx_flow_req > 0)
  {
    if ((rc = qmi_qos_format_all_flow_tlvs (msg_buf,
                                            msg_buf_size,
                                            QMI_QOS_RX_FLOW_REQ_RSP_TLV_ID,
                                            (int)qos_spec->num_rx_flow_req,
                                            qos_spec->rx_flow_req_array)) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
    /* return from function is size of embedded TLV */
    tlv_length += rc;
  }

  /* Format TX filter TLV's */
  if (qos_spec->num_tx_filter_req > 0)
  {
    if ((rc = qmi_qos_format_all_filter_tlvs (msg_buf,
                                            msg_buf_size,
                                            QMI_QOS_TX_FILTER_REQ_RSP_TLV_ID,
                                            (int)qos_spec->num_tx_filter_req,
                                            qos_spec->tx_filter_req_array)) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
    /* return from function is size of embedded TLV */
    tlv_length += rc;
  }

  /* Format RX filter TLV's */
  if (qos_spec->num_rx_filter_req > 0)
  {
    if ((rc = qmi_qos_format_all_filter_tlvs (msg_buf,
                                            msg_buf_size,
                                            QMI_QOS_RX_FILTER_REQ_RSP_TLV_ID,
                                            (int)qos_spec->num_rx_filter_req,
                                            qos_spec->rx_filter_req_array)) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
    /* return from function is size of embedded TLV */
    tlv_length += rc;
  }

  SET_ENVELOPE_TLV_HDR (QMI_QOS_SPEC_REQ_RSP_TLV_ID);
  return tlv_length;
}

static int
qmi_qos_process_qos_req_success_rsp
(
unsigned char   *msg_buf,
int             msg_buf_size,
int             num_qos_specs,
unsigned long   *qos_identifiers
)
{
  unsigned long type;
  unsigned long length;
  unsigned char *value_ptr;
  int           tlv_num_identifiers,i;
  unsigned char tmp_char;

  /* Read the result TLV */
  QMI_QOSREAD_TLV();

  READ_8_BIT_VAL (value_ptr, tmp_char);
  tlv_num_identifiers = (int) tmp_char;

  /* check to make sure we got the right number of QoS identifiers */
  if (tlv_num_identifiers != num_qos_specs)
  {
    QMI_DEBUG_MSG_2 ("qmi_qos_process_qos_req_success_rsp: num_identifiers don't match %d, %d",
                     (unsigned int) num_qos_specs, (unsigned int)tlv_num_identifiers );
    return QMI_INTERNAL_ERR;
  }

  /*lint -e{681) */
  for (i=0; i < tlv_num_identifiers; i++)
  {
    unsigned long identifier;
    READ_32_BIT_VAL (value_ptr, identifier);
    *qos_identifiers = identifier;
    qos_identifiers++;
  }

  return QMI_NO_ERR;
}



static int
qmi_qos_get_flow_errs
(
unsigned char     *msg_buf,
int                msg_buf_size,
unsigned short     *flow_err_masks
)
{
  unsigned long  type;
  unsigned long  length;
  unsigned char  *value_ptr;
  unsigned short err_mask = 0;
  int            index = -1;

  while (msg_buf_size > 0)
  {
    /* Get the FLOW/FILTER ERR TLV */
    QMI_QOSREAD_TLV();

    if (type == QMI_QOS_FLOW_INDEX_REQ_RSP_TLV_ID)
    {
      READ_8_BIT_VAL (value_ptr, index);
    }
    else if (type == QMI_QOS_FLOW_FILTER_ERR_VALS_TLV_ID)
    {
      int i;
      for (i = 0; i < (int)length; i++)
      {
        unsigned long tlv_id = 0;
        READ_8_BIT_VAL (value_ptr, tlv_id);
        switch (tlv_id)
        {

          case QMI_QOS_FLOW_CDMA_PROFILE_ID_REQ_TLV_ID:
            {
              err_mask |= QMI_QOS_CDMA_FLOW_PARAM_PROFILE_ID_ERR;
            }
            break;

          case QMI_QOS_FLOW_UMTS_TC_REQ_RSP_TLV_ID:
            {
              err_mask |= QMI_QOS_UMTS_FLOW_PARAM_TRAFFIC_CLASS;
            }
            break;

          case QMI_QOS_FLOW_UMTS_DATA_RATE_REQ_RSP_TLV_ID:
            {
              err_mask |= QMI_QOS_UMTS_FLOW_PARAM_DATA_RATE;
            }
            break;

          case QMI_QOS_FLOW_UMTS_BUCKET_INFO_REQ_RSP_TLV_ID:
            {
              err_mask |= QMI_QOS_UMTS_FLOW_PARAM_BUCKET_INFO;
            }
            break;

          case QMI_QOS_FLOW_UMTS_MAX_DELAY_REQ_RSP_TLV_ID:
            {
              err_mask |= QMI_QOS_UMTS_FLOW_PARAM_MAX_DELAY;
            }
            break;
          case QMI_QOS_FLOW_UMTS_MAX_JITTER_REQ_RSP_TLV_ID:
            {
              err_mask |= QMI_QOS_UMTS_FLOW_PARAM_MAX_JITTER;
            }
            break;

          case QMI_QOS_FLOW_UMTS_PKT_ERR_RATE_REQ_RSP_TLV_ID:
            {
              err_mask |= QMI_QOS_UMTS_FLOW_PARAM_PKT_ERR_RATE;
            }
            break;

          case QMI_QOS_FLOW_UMTS_MIN_POL_PKT_SZ_REQ_RSP_TLV_ID:
            {
              err_mask |= QMI_QOS_UMTS_FLOW_PARAM_MIN_POL_PKT_SZ;
            }
            break;

          case QMI_QOS_FLOW_UMTS_MAX_ALLOW_PKT_SZ_REQ_RSP_TLV_ID:
            {
              err_mask |= QMI_QOS_UMTS_FLOW_PARAM_MAX_ALLOW_PKT_SZ;
            }
            break;

          case QMI_QOS_FLOW_UMTS_RESIDUAL_BER_REQ_RSP_TLV_ID:
            {
              err_mask |= QMI_QOS_UMTS_FLOW_PARAM_RESIDUAL_BER;
            }
            break;

          case QMI_QOS_FLOW_UMTS_HANDLING_PRIO_REQ_RSP_TLV_ID:
            {
              err_mask |= QMI_QOS_UMTS_FLOW_PARAM_HANDLING_PRIO;
            }
            break;

          default:
            QMI_DEBUG_MSG_1 ("qmi_qos_get_flow_errs: unknown TLV type = %x",(unsigned int)type);
            break;
        }
      }
    }
  }

  if (index == -1)
  {
    return QMI_INTERNAL_ERR;
  }
  flow_err_masks[index]=err_mask;
  return QMI_NO_ERR;
}


static int
qmi_qos_get_filter_errs
(
unsigned char     *msg_buf,
int                msg_buf_size,
unsigned short     *filter_err_masks
)
{
  unsigned long  type;
  unsigned long  length;
  unsigned char  *value_ptr;
  unsigned short err_mask = 0;
  int            index = -1;

  while (msg_buf_size > 0)
  {
    /* Get the FLOW/FILTER ERR TLV */
    QMI_QOSREAD_TLV();

    if (type == QMI_QOS_FILTER_INDEX_REQ_RSP_TLV_ID)
    {
      READ_8_BIT_VAL (value_ptr, index);
    }
    else if (type == QMI_QOS_FLOW_FILTER_ERR_VALS_TLV_ID)
    {
      int i;
      for (i = 0; i < (int)length; i++)
      {
        unsigned long tlv_id = 0;
        READ_8_BIT_VAL (value_ptr, tlv_id);
        switch (tlv_id)
        {

          case QMI_QOS_FILTER_IP_VER_REQ_RSP_TLV_ID:
            {
              err_mask |= (unsigned short)QMI_QOS_FILTER_PARAM_IP_VER_ERR;
            }
            break;

          case QMI_QOS_FILTER_SRC_ADDR_REQ_RSP_TLV_ID:
            {
              err_mask |= (unsigned short)QMI_QOS_FILTER_PARAM_SRC_ADDR;
            }
            break;

          case QMI_QOS_FILTER_DEST_ADDR_REQ_RSP_TLV_ID:
            {
              err_mask |= (unsigned short)QMI_QOS_FILTER_PARAM_DEST_ADDR;
            }
            break;

          case QMI_QOS_FILTER_TRANS_PROTOCOL_REQ_RSP_TLV_ID:
            {
              err_mask |= (unsigned short)QMI_QOS_FILTER_PARAM_TRANS_PROTOCOL;
            }
            break;

          case QMI_QOS_FILTER_TOS_REQ_RSP_TLV_ID:
            {
              err_mask |= (unsigned short)QMI_QOS_FILTER_PARAM_TOS;
            }
            break;
          case QMI_QOS_FILTER_TCP_SRC_PORT_REQ_RSP_TLV_ID:
            {
              err_mask |= (unsigned short)QMI_QOS_FILTER_PARAM_TCP_SRC_PORTS;
            }
            break;

          case QMI_QOS_FILTER_TCP_DEST_PORT_REQ_RSP_TLV_ID:
            {
              err_mask |= (unsigned short)QMI_QOS_FILTER_PARAM_TCP_DEST_PORTS;
            }
            break;

          case QMI_QOS_FILTER_UDP_SRC_PORT_REQ_RSP_TLV_ID:
            {
              err_mask |= (unsigned short)QMI_QOS_FILTER_PARAM_UDP_SRC_PORTS;
            }
            break;

          case QMI_QOS_FILTER_UDP_DEST_PORT_REQ_RSP_TLV_ID:
            {
              err_mask |= (unsigned short)QMI_QOS_FILTER_PARAM_UDP_DEST_PORTS;
            }
            break;

          default:
            QMI_DEBUG_MSG_1 ("qmi_qos_get_filter_errs: unknown TLV type = %x",(unsigned int)type);
            break;
        }
      }
    }
  }

  if (index == -1)
  {
    return QMI_INTERNAL_ERR;
  }
  filter_err_masks[index]=err_mask;
  return QMI_NO_ERR;
}





static int
qmi_qos_get_spec_flow_filter_errs
(
unsigned char     *msg_buf,
int                msg_buf_size,
unsigned short     *err_masks,
int                flow
)
{
  unsigned long  type;
  unsigned long  length;
  unsigned char  *value_ptr;

  while (msg_buf_size > 0)
  {

    /* Get the FLOW/FILTER ERR TLV */
    QMI_QOSREAD_TLV();

    if (type != QMI_QOS_FLOW_FILTER_SPEC_ERR_RSP_TLV_ID)
    {
      return QMI_INTERNAL_ERR;
    }

    if (flow)
    {
      if (qmi_qos_get_flow_errs (value_ptr, (int)length, err_masks) < 0)
      {
        return QMI_INTERNAL_ERR;
      }
    }
    else
    { /* filter */
      if (qmi_qos_get_filter_errs (value_ptr, (int)length, err_masks) < 0)
      {
        return QMI_INTERNAL_ERR;
      }
    }
  }

  return QMI_NO_ERR;
}


static int
qmi_qos_get_qos_spec_req_errs
(
unsigned char          *msg_buf,
int                    msg_buf_size,
qmi_qos_err_rsp_type   *qos_spec_errs
)
{
  int qos_spec_index = 0;
  qmi_qos_err_rsp_type *this_spec_err = NULL;
  unsigned long type;
  unsigned long length;
  unsigned char *value_ptr;

  /* Get the index TLV... this should be first */
  if (msg_buf_size > 0)
  {
    QMI_QOSREAD_TLV();

    if (type != QMI_QOS_SPEC_INDEX_REQ_RSP_TLV_ID)
    {
      return QMI_INTERNAL_ERR;
    }
    READ_8_BIT_VAL (value_ptr, qos_spec_index);

    /* Set values and pointers based on returned index */
    this_spec_err = &qos_spec_errs[qos_spec_index];
    this_spec_err->errs_present = TRUE;
  }

  /* Get the flow and filter specific errors for this QoS spec */
  while (msg_buf_size > 0)
  {
    QMI_QOSREAD_TLV();

    switch (type)
    {
      case QMI_QOS_TX_FLOW_REQ_RSP_TLV_ID:
        {
          (void)qmi_qos_get_spec_flow_filter_errs (value_ptr,
                                           (int)length,
                                           this_spec_err->tx_flow_req_err_mask,
                                           TRUE);
        }
        break;

      case QMI_QOS_RX_FLOW_REQ_RSP_TLV_ID:
        {
          (void)qmi_qos_get_spec_flow_filter_errs (value_ptr,
                                           (int)length,
                                           this_spec_err->rx_flow_req_err_mask,
                                           TRUE);
        }
        break;

      case QMI_QOS_TX_FILTER_REQ_RSP_TLV_ID:
        {
          (void)qmi_qos_get_spec_flow_filter_errs (value_ptr,
                                           (int)length,
                                           this_spec_err->tx_filter_req_err_mask,
                                           FALSE);
        }
        break;

      case QMI_QOS_RX_FILTER_REQ_RSP_TLV_ID:
        {
          (void)qmi_qos_get_spec_flow_filter_errs (value_ptr,
                                           (int)length,
                                           this_spec_err->rx_filter_req_err_mask,
                                           FALSE);
        }
        break;

      default:
        QMI_DEBUG_MSG_1 ("qmi_qos_get_qos_spec_req_errs: unknown TLV type = %x",(unsigned int)type);
        return QMI_INTERNAL_ERR;
    }
  }
  return QMI_NO_ERR;
}

static int
qmi_qos_process_qos_req_failure_rsp
(
unsigned char          *msg_buf,
int                    msg_buf_size,
qmi_qos_err_rsp_type   *qos_spec_errs
)
{
  unsigned long type;
  unsigned long length;
  unsigned char *value_ptr;

  while (msg_buf_size > 0)
  {
    QMI_QOSREAD_TLV();

    if (type != QMI_QOS_SPEC_ERR_TLV_ID)
    {
      return QMI_INTERNAL_ERR;
    }

    if (qmi_qos_get_qos_spec_req_errs (value_ptr,
                                       (int)length,
                                       qos_spec_errs) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }
  return QMI_NO_ERR;
}


/*===========================================================================
  FUNCTION  qmi_qos_request_qos
===========================================================================*/
/*!
@brief
  This function sends a QoS request.  Note that for CDMA technology
  types, an array of up to 10 qos_specs may be specified at one time.
  For UMTS, only a single qos_spec can be specified at a time (num_qos_specs
  must be set to 1).  Upon successful return, the qos_identifiers pointer/array
  will contain identifiers for each of the qos_specs requested.  These
  identifiers will be used for any QoS operations/indications to follow on that
  particular session.

@return
  0 (QMI_NO_ERR) if operation was sucessful, < 0 if not.
  In the failure case when QMI_SERVICE_ERR is returned, the qmi_err_code
  will be valid and will contain the error code returned by QMI on the modem,
  and there may be more error information passed back in the qos_spec_errs data
  structure, depending on the nature of the error.

@note
    - Dependencies: qmi_qos_srvc_init_client() must be called before calling
                    this function.


    - Side Effects: Brings up a QoS session
*/
/*=========================================================================*/
int
qmi_qos_request_qos
(
  qmi_client_handle_type        client_handle,
  int                           num_qos_specs,
  qmi_qos_spec_type             *qos_spec_array,
  qmi_qos_req_opcode_type       req_opcode_type,
  unsigned long                 *qos_id_array,    /* Valid if rc == QMI_NO_ERR */
  qmi_qos_err_rsp_type          *qos_spec_errs,   /* Valid if rc == QMI_SERVICE_ERR */
  int                           *qmi_err_code
)
{
  unsigned char     *msg;
  int               msg_size;
  unsigned char    *tmp_msg_ptr;
  int              spec_index, rc;


  if ((num_qos_specs < 1) || (num_qos_specs > 10))
  {
    QMI_DEBUG_MSG_1 ("qmi_qos_request_qos: Invalid number of QoS specs for CDMA = %d",(unsigned int) num_qos_specs);
    return QMI_INTERNAL_ERR;
  }

  /* QoS reqest messages can be very large, so we dynamically allocate these */
  msg = (unsigned char *) malloc (QMI_MAX_MSG_SIZE);
  if (!msg)
  {
    QMI_ERR_MSG_0 ("Unable do dynamically allocate memory for QoS request\n");
    return QMI_INTERNAL_ERR;
  }


  /* Set tmp_msg_ptr to beginning of message-specific TLV portion of
  ** message buffer
  */
  tmp_msg_ptr = QMI_SRVC_PDU_PTR(msg);

  /* Set the message size to the complete buffer minus the header size */
  msg_size = QMI_SRVC_PDU_SIZE(QMI_MAX_MSG_SIZE);

  /* Initialize error structure flag */
  qos_spec_errs->errs_present = FALSE;

  /* Format all of the QOS specification request TLV's */
  for (spec_index=0; spec_index < num_qos_specs; spec_index++)
  {
    /* Initialize the error structures */
    memset ((void *) &qos_spec_errs[spec_index], 0, sizeof (qmi_qos_err_rsp_type));

    if ((rc = qmi_qos_format_spec_req_tlvs (&tmp_msg_ptr,
                                            &msg_size,
                                            spec_index,
                                            qos_spec_array++,
                                            QMI_QOS_REQUEST_QOS)) < 0)
    {
      QMI_DEBUG_MSG_0 ("qmi_qos_request_qos: Format of input QoS specs fails");
      free (msg);
      return QMI_INTERNAL_ERR;
    }
  }
  if ((int)req_opcode_type != 0x00)
  {
    unsigned char temp;
    temp = (unsigned char)req_opcode_type;

    if (qmi_util_write_std_tlv(&tmp_msg_ptr,
                               &msg_size,
                               QMI_QOS_REQUEST_OPCODE_TLV_ID,
                               1,
                               (void *)&temp) < 0)
    {
      free (msg);
      return QMI_INTERNAL_ERR;
    }
  }

  /* Send the QMI message to the modem processor */
  rc = qmi_service_send_msg_sync (client_handle,
                                  QMI_QOS_SERVICE,
                                  QMI_QOS_REQUEST_QOS_MSG_ID,
                                  QMI_SRVC_PDU_PTR(msg),
                                  (int)QMI_SRVC_PDU_SIZE(QMI_MAX_MSG_SIZE) - msg_size,
                                  msg,
                                  &msg_size,
                                  QMI_MAX_MSG_SIZE,
                                  QMI_SYNC_MSG_DEFAULT_TIMEOUT,
                                  qmi_err_code);

  /* Extract response information */
  if (rc == QMI_NO_ERR)
  {
    rc = qmi_qos_process_qos_req_success_rsp (msg,
                                              msg_size,
                                              num_qos_specs,
                                              qos_id_array);
  }
  else if (rc == QMI_SERVICE_ERR)
  {
    (void) qmi_qos_process_qos_req_failure_rsp (msg,
                                                msg_size,
                                                qos_spec_errs);
  }

  free (msg);
  return rc;
}



/*===========================================================================
  FUNCTION  qmi_qos_modify_secondary_qos
===========================================================================*/
/*!
@brief
  Modifies a secondary QoS.

@return
  0 (QMI_NO_ERR) if operation was sucessful, < 0 if not.
  In the failure case when QMI_SERVICE_ERR is returned, the qmi_err_code
  will be valid and will contain the error code returned by QMI on the modem.

@note
    - Dependencies: qmi_qos_srvc_init_client() must be called before calling
                    this function.
*/
/*=========================================================================*/
int
qmi_qos_modify_secondary_qos
(
  qmi_client_handle_type        client_handle,
  int                           num_qos_specs,
  qmi_qos_spec_type             *qos_spec_array,
  qmi_qos_err_rsp_type          *qos_spec_errs,   /* Valid if rc == QMI_SERVICE_ERR */
  int                           *qmi_err_code
)
{
  unsigned char     *msg;
  int               msg_size;
  unsigned char    *tmp_msg_ptr;
  int              spec_index, rc;


  /* Do some error checking */
  if ((num_qos_specs < 1) || (num_qos_specs > 10))
  {
    QMI_DEBUG_MSG_1 ("qmi_qos_modify_secondary_qos: Invalid number of QoS specs for CDMA = %d",(unsigned int) num_qos_specs);
    return QMI_INTERNAL_ERR;
  }

  /* QoS modify messages can be very large, so we dynamically allocate these */
  msg = (unsigned char *) malloc (QMI_MAX_MSG_SIZE);
  if (!msg)
  {
    QMI_ERR_MSG_0 ("Unable do dynamically allocate memory for QoS modify\n");
    return QMI_INTERNAL_ERR;
  }

  /* Set tmp_msg_ptr to beginning of message-specific TLV portion of
  ** message buffer
  */
  tmp_msg_ptr = QMI_SRVC_PDU_PTR(msg);

  /* Set the message size to the complete buffer minus the header size */
  msg_size = QMI_SRVC_PDU_SIZE(QMI_MAX_MSG_SIZE);

  /* Initialize error structure flag */
  qos_spec_errs->errs_present = FALSE;

  for (spec_index=0; spec_index < num_qos_specs; spec_index++)
  {
    /* Initialize the error structures */
    memset ((void *) &qos_spec_errs[spec_index], 0, sizeof (qmi_qos_err_rsp_type));

    if ((rc = qmi_qos_format_spec_req_tlvs (&tmp_msg_ptr,
                                            &msg_size,
                                            spec_index,
                                            qos_spec_array++,
                                            QMI_QOS_MODIFY_QOS)) < 0)
    {
      QMI_DEBUG_MSG_0 ("qmi_qos_modify_secondary_qos: Format of input QoS specs fails");
      free (msg);
      return QMI_INTERNAL_ERR;
    }
  }

  /* Send the QMI message to the modem processor */
  rc = qmi_service_send_msg_sync (client_handle,
                                  QMI_QOS_SERVICE,
                                  QMI_QOS_MODIFY_QOS_MSG_ID,
                                  QMI_SRVC_PDU_PTR(msg),
                                  (int)QMI_SRVC_PDU_SIZE(QMI_MAX_MSG_SIZE) - msg_size,
                                  msg,
                                  &msg_size,
                                  QMI_MAX_MSG_SIZE,
                                  QMI_SYNC_MSG_DEFAULT_TIMEOUT,
                                  qmi_err_code);

  if (rc == QMI_SERVICE_ERR)
  {
    (void) qmi_qos_process_qos_req_failure_rsp (msg,
                                                msg_size,
                                                qos_spec_errs);
  }
  free (msg);
  return rc;
}


/*===========================================================================
  FUNCTION  qmi_qos_modify_primary_qos
===========================================================================*/
/*!
@brief
  Modifies a primary QoS.

@return
  0 (QMI_NO_ERR) if operation was sucessful, < 0 if not.
  In the failure case when QMI_SERVICE_ERR is returned, the qmi_err_code
  will be valid and will contain the error code returned by QMI on the modem.

@note
    - Dependencies: qmi_qos_srvc_init_client() must be called before calling
                    this function.

*/
/*=========================================================================*/
int
qmi_qos_modify_primary_qos
(
  qmi_client_handle_type        client_handle,
  qmi_qos_spec_type             *qos_spec,
  qmi_qos_err_rsp_type          *qos_spec_err,   /* Valid if rc == QMI_SERVICE_ERR */
  int                           *qmi_err_code
)
{
  unsigned char     *msg;
  int               msg_size;
  unsigned char    *tmp_msg_ptr;
  int               rc;

  /* QoS modify messages can be very large, so we dynamically allocate these */
  msg = (unsigned char *) malloc (QMI_MAX_MSG_SIZE);
  if (!msg)
  {
    QMI_ERR_MSG_0 ("Unable do dynamically allocate memory for QoS modify\n");
    return QMI_INTERNAL_ERR;
  }

  /* Set tmp_msg_ptr to beginning of message-specific TLV portion of
  ** message buffer
  */
  tmp_msg_ptr = QMI_SRVC_PDU_PTR(msg);

  /* Set the message size to the complete buffer minus the header size */
  msg_size = QMI_SRVC_PDU_SIZE(QMI_MAX_MSG_SIZE);

  /* Initialize error structure flag */
  qos_spec_err->errs_present = FALSE;

  /* Initialize the error structures */
  memset ((void *) qos_spec_err, 0, sizeof (qmi_qos_err_rsp_type));

  if ((rc = qmi_qos_format_spec_req_tlvs (&tmp_msg_ptr,
                                          &msg_size,
                                          0,
                                          qos_spec,
                                          QMI_QOS_MODIFY_QOS)) < 0)
  {
    QMI_DEBUG_MSG_0 ("qmi_qos_modify_primary_qos: Format of input QoS specs fails");
    free (msg);
    return QMI_INTERNAL_ERR;
  }

  /* Add the flag indicating that this is a primary modify */
  {
    unsigned char temp = 1;
    if (qmi_util_write_std_tlv(&tmp_msg_ptr,
                                  &msg_size,
                                  QMI_QOS_MODIFY_PRIMARY_QOS_TLV_ID,
                                  1,
                                  (void *)&temp) < 0)
    {
      free (msg);
      return QMI_INTERNAL_ERR;
    }
  }

  /* Send the QMI message to the modem processor */
  rc = qmi_service_send_msg_sync (client_handle,
                                  QMI_QOS_SERVICE,
                                  QMI_QOS_MODIFY_QOS_MSG_ID,
                                  QMI_SRVC_PDU_PTR(msg),
                                  (int)QMI_SRVC_PDU_SIZE(QMI_MAX_MSG_SIZE) - msg_size,
                                  msg,
                                  &msg_size,
                                  QMI_MAX_MSG_SIZE,
                                  QMI_SYNC_MSG_DEFAULT_TIMEOUT,
                                  qmi_err_code);

  if (rc == QMI_SERVICE_ERR)
  {
    (void) qmi_qos_process_qos_req_failure_rsp (msg,
                                                msg_size,
                                                qos_spec_err);
  }
  free (msg);
  return rc;
}



/*===========================================================================
  FUNCTION  qmi_qos_release_qos
===========================================================================*/
/*!
@brief
  Tears down a requested QoS.  More that one QoS may be specified by passing
  in an array of qos_identifiers and setting the num_qos_identifiers
  accordingly.

@return
  0 (QMI_NO_ERR) if operation was sucessful, < 0 if not.
  In the failure case when QMI_SERVICE_ERR is returned, the qmi_err_code
  will be valid and will contain the error code returned by QMI on the modem.

@note
    - Dependencies: qmi_qos_srvc_init_client() must be called before calling
                    this function.
    - Side Effects:  Tears down a QoS
*/
/*=========================================================================*/
int
qmi_qos_release_qos
(
  qmi_client_handle_type   client_handle,
  int                      num_qos_ids,
  unsigned long            *qos_id_array,
  int                      *qmi_err_code
)
{
  unsigned char     msg[QMI_QOS_STD_MSG_SIZE], tmp_buf[50], *tmp_buf_ptr;
  int               msg_size;
  unsigned char    *tmp_msg_ptr = QMI_SRVC_PDU_PTR(msg);
  int               i, rc;


  if ((num_qos_ids < 1) || (num_qos_ids > 10))
  {
    return QMI_INTERNAL_ERR;
  }

  /* Set up tmp_buf_ptr to point to beginning of tmp_buf */
  tmp_buf_ptr = tmp_buf;

  WRITE_8_BIT_VAL (tmp_buf_ptr, num_qos_ids);

  for (i=0; i<num_qos_ids; i++)
  {
    WRITE_32_BIT_VAL (tmp_buf_ptr,*qos_id_array++);
  }

  /* Set tmp_msg_ptr to beginning of message-specific TLV portion of
  ** message buffer
  */
  tmp_msg_ptr = QMI_SRVC_PDU_PTR(msg);

  /* Set the message size to the complete buffer minus the header size */
  msg_size = QMI_SRVC_PDU_SIZE(QMI_QOS_STD_MSG_SIZE);

  /* Prepare the TLV */
  if ((rc = qmi_util_write_std_tlv (&tmp_msg_ptr,
                                       &msg_size,
                                       QMI_QOS_RELEASE_ID_LIST_TLV_ID,
                                       (unsigned long) ((num_qos_ids * 4) + 1),
                                       (void *)tmp_buf)) < 0)
  {
    return QMI_INTERNAL_ERR;
  }


  /* Synchronously send message to modem processor */
  rc = qmi_service_send_msg_sync (client_handle,
                                  QMI_QOS_SERVICE,
                                  QMI_QOS_RELEASE_QOS_MSG_ID,
                                  QMI_SRVC_PDU_PTR(msg),
                                  (int)QMI_SRVC_PDU_SIZE(QMI_QOS_STD_MSG_SIZE) - msg_size,
                                  msg,
                                  &msg_size,
                                  QMI_QOS_STD_MSG_SIZE,
                                  QMI_SYNC_MSG_DEFAULT_TIMEOUT,
                                  qmi_err_code);

  return rc;
}

/*===========================================================================
  FUNCTION  qmi_qos_set_event_report_state
===========================================================================*/
/*!
@brief
  Set the QoS event reporting state


@return

@note

  - Dependencies
    - qmi_qos_srvc_init_client() must be called before calling this.

  - Side Effects
    - Starts event reporting
*/
/*=========================================================================*/
int
qmi_qos_set_event_report_state
(
  int                               client_handle,
  qmi_qos_event_report_state_type   *report_state,
  int                               *qmi_err_code
)
{
  unsigned char     msg[QMI_QOS_STD_MSG_SIZE],*tmp_msg_ptr;
  int               msg_size, rc;

  /* Set tmp_msg_ptr to beginning of message-specific TLV portion of
  ** message buffer
  */
  tmp_msg_ptr = QMI_SRVC_PDU_PTR(msg);

  /* Set the message size to the complete buffer minus the header size */
  msg_size = QMI_SRVC_PDU_SIZE(QMI_QOS_STD_MSG_SIZE);

  /* Prepare the TLV */
  if (report_state->param_mask & QMI_QOS_GLOBAL_FLOW_REPORTING_STATE_PARAM)
  {
    if ((rc = qmi_util_write_std_tlv (&tmp_msg_ptr,
                                         &msg_size,
                                         QMI_QOS_EVENT_REPORT_GLOBAL_FLOW_STATE_TLV_ID,
                                         1,
                                         (void *)&report_state->flow_state)) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }

  if (report_state->param_mask & QMI_QOS_NW_SUPPORTED_PROFILE_CHANGE_STATE_PARAM)
  {
    unsigned char temp[3];
    unsigned char *temp_msg_ptr = &temp[1];
    short temp_16bit;

    temp[0] = (unsigned char)report_state->profile_change_state.profile_change_status;
    temp_16bit = (short)report_state->profile_change_state.iface_type;
    memcpy((void *)temp_msg_ptr, (void *)&temp_16bit,sizeof(short));
    if ((rc = qmi_util_write_std_tlv (&tmp_msg_ptr,
                                         &msg_size,
                                         QMI_QOS_EVENT_REPORT_NW_SUPP_PROFILE_CHANGE_TLV_ID,
                                         3,
                                         (void *)temp)) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }

  /* Synchronously send message to modem processor */
  rc = qmi_service_send_msg_sync (client_handle,
                                  QMI_QOS_SERVICE,
                                  QMI_QOS_SET_EVENT_REPORT_STATE_MSG_ID,
                                  QMI_SRVC_PDU_PTR(msg),
                                  (int)QMI_SRVC_PDU_SIZE(QMI_QOS_STD_MSG_SIZE) - msg_size,
                                  msg,
                                  &msg_size,
                                  QMI_QOS_STD_MSG_SIZE,
                                  QMI_SYNC_MSG_DEFAULT_TIMEOUT,
                                  qmi_err_code);

  return rc;
}



/*===========================================================================
  FUNCTION  qmi_qos_does_nw_support_qos
===========================================================================*/
/*!
@brief
  This function queries if the current network supports QOS!

@return

@note

  - Dependencies
    - qmi_qos_srvc_init_client() must be called before calling this.

  - Side Effects
    - Starts event reporting
*/
/*=========================================================================*/
int
qmi_qos_does_nw_support_qos
(
  int                               client_handle,
  qmi_qos_nw_status_type            *status_resp,
  int                               *qmi_err_code
)
{
  unsigned char msg[QMI_QOS_STD_MSG_SIZE];
  int           msg_size, rc;

  rc = qmi_service_send_msg_sync (client_handle,
                                  QMI_QOS_SERVICE,
                                  QMI_QOS_NW_STATUS_REPORT_STATE_MSG_ID,
                                  QMI_SRVC_PDU_PTR(msg),
                                  0,
                                  msg,
                                  &msg_size,
                                  QMI_QOS_STD_MSG_SIZE,
                                  QMI_SYNC_MSG_DEFAULT_TIMEOUT,
                                  qmi_err_code);

  if (rc == QMI_NO_ERR)
  {
    if ((rc = qmi_qos_srvc_process_nw_status_report (msg,msg_size,status_resp)) < 0)
    {
      QMI_DEBUG_MSG_0 ("qmi_qos_does_nw_support_qos::QMI_ERR_SYSERR \n");
    }
  }
  return rc;
}
/*===========================================================================
  FUNCTION  qmi_qos_get_status
===========================================================================*/
/*!
@brief
  This function queries if the current network supports QOS!


@return

@note

  - Dependencies
    - qmi_qos_srvc_init_client() must be called before calling this.

  - Side Effects
    - Starts event reporting
*/
/*=========================================================================*/
int
qmi_qos_get_status
(
  qmi_client_handle_type            client_handle,
  unsigned long                     qos_identifier,
  qmi_qos_status_info               *status_resp,
  int                               *qmi_err_code
)
{
  unsigned char msg[QMI_QOS_STD_MSG_SIZE], *tmp_msg_ptr;
  int           msg_size, rc;

  tmp_msg_ptr = QMI_SRVC_PDU_PTR(msg);

  msg_size = QMI_SRVC_PDU_SIZE(QMI_QOS_STD_MSG_SIZE);

  if ((rc = qmi_util_write_std_tlv(&tmp_msg_ptr,
                                      &msg_size,
                                      QMI_QOS_STATUS_REQ_TLV_ID,
                                      4,
                                      (void *)&qos_identifier)) < 0)
  {
    return QMI_INTERNAL_ERR;
  }

  rc = qmi_service_send_msg_sync (client_handle,
                                  QMI_QOS_SERVICE,
                                  QMI_QOS_STATUS_REPORT_STATE_MSG_ID,
                                  QMI_SRVC_PDU_PTR(msg),
                                  (int)QMI_SRVC_PDU_SIZE(QMI_QOS_STD_MSG_SIZE) - msg_size,
                                  msg,
                                  &msg_size,
                                  QMI_QOS_STD_MSG_SIZE,
                                  QMI_SYNC_MSG_DEFAULT_TIMEOUT,
                                  qmi_err_code);

  tmp_msg_ptr = msg;

  if (rc == QMI_NO_ERR)
  {
    unsigned long type;
    unsigned long length;
    unsigned char *value_ptr;
    unsigned char tmp_char;
    tmp_msg_ptr = msg;

    if (qmi_util_read_std_tlv (&tmp_msg_ptr,
                                  &msg_size,
                                  &type,
                                  &length,
                                  &value_ptr) < 0)
    {
      return QMI_INTERNAL_ERR;
    }

    if (type != QMI_QOS_STATUS_INFO_TLV_ID)
    {
      QMI_ERR_MSG_1( "qmi_qos_get_status:: Invalid TLV %lx",type );
      return QMI_INTERNAL_ERR;
    }

    READ_8_BIT_VAL (value_ptr, tmp_char);
    *status_resp = (qmi_qos_status_info) tmp_char;
  }

  return rc;
}

/*===========================================================================
  FUNCTION  qmi_qos_get_primary_granted_qos_info
===========================================================================*/
/*!
@brief
  This function if successful returns the qos filter parameters for a
  particular QOS flow.

@return

@note

  - Dependencies
    - qmi_qos_srvc_init_client() must be called before calling this.

  - Side Effects
    - Starts event reporting
*/
/*=========================================================================*/
static int
qmi_qos_get_filter_params
(
  qmi_client_handle_type                  client_handle,
  unsigned long                           qos_identifier,
  qmi_qos_granted_info_rsp_type           *granted_info,
  int                                     *qmi_err_code
)
{
  unsigned char msg[QMI_QOS_STD_MSG_SIZE], *tmp_msg_ptr;
  int           msg_size, rc;

  memset(msg, 0x0, QMI_QOS_STD_MSG_SIZE);
  tmp_msg_ptr = QMI_SRVC_PDU_PTR(msg);

  msg_size = QMI_SRVC_PDU_SIZE(QMI_QOS_STD_MSG_SIZE);

  if ((rc = qmi_util_write_std_tlv(&tmp_msg_ptr,
                                      &msg_size,
                                      QMI_QOS_GET_FILTER_PARAMS_REQ_QOS_ID_TLV_ID,
                                      4,
                                      (void *)&qos_identifier)) < 0)
  {
    return QMI_INTERNAL_ERR;
  }

  rc = qmi_service_send_msg_sync (client_handle,
                                  QMI_QOS_SERVICE,
                                  QMI_QOS_GET_FILTER_PARAMS_MSG_ID,
                                  QMI_SRVC_PDU_PTR(msg),
                                  QMI_SRVC_PDU_SIZE(QMI_QOS_STD_MSG_SIZE) - msg_size,
                                  msg,
                                  &msg_size,
                                  QMI_QOS_STD_MSG_SIZE,
                                  QMI_SYNC_MSG_DEFAULT_TIMEOUT,
                                  qmi_err_code);


  tmp_msg_ptr = msg;

  {
    unsigned long type;
    unsigned long length;
    unsigned char *value_ptr;

    tmp_msg_ptr = msg;

    while (msg_size > 0)
    {
      if (qmi_util_read_std_tlv (&tmp_msg_ptr,
                                    &msg_size,
                                    &type,
                                    &length,
                                    &value_ptr) < 0)
      {
        return QMI_INTERNAL_ERR;
      }

      switch (type)
      {
        case QMI_QOS_GET_FILTER_PARAMS_TX_FILTER_DATA_TLV_ID:
          {
            if (qmi_qos_srvc_event_prcss_nstd_filter_tlvs(value_ptr,
                                                          (int)length,
                                                          &granted_info->tx_granted_filter_data[0],
                                                          &granted_info->tx_filter_count) < 0)
            {
              return QMI_INTERNAL_ERR;
            }
          }
          break;

        case QMI_QOS_GET_FILTER_PARAMS_RX_FILTER_DATA_TLV_ID:
          {
            if (qmi_qos_srvc_event_prcss_nstd_filter_tlvs(value_ptr,
                                                          (int)length,
                                                          &granted_info->rx_granted_filter_data[0],
                                                          &granted_info->rx_filter_count) < 0)
            {
              return QMI_INTERNAL_ERR;
            }
          }
          break;

        default:
          {
            QMI_DEBUG_MSG_1 ("qmi_qos_get_filter_params: unknown TLV type = %x",(unsigned int)type);
          }
          break;
      }
    }
  }
  return rc;
}

/*===========================================================================
  FUNCTION  qmi_qos_get_primary_granted_qos_info
===========================================================================*/
/*!
@brief
  This function if successful returns the qos parameters for a
  particular QOS flow.


@return

@note

  - Dependencies
    - qmi_qos_srvc_init_client() must be called before calling this.

  - Side Effects
    - Starts event reporting
*/
/*=========================================================================*/
int
qmi_qos_get_primary_granted_qos_info
(
  qmi_client_handle_type                  client_handle,
  unsigned long                           qos_identifier,
  qmi_qos_granted_info_rsp_type           *granted_info,
  int                                     *qmi_err_code
)
{

  unsigned char msg[QMI_QOS_STD_MSG_SIZE], *tmp_msg_ptr;
  int           msg_size, rc;

  tmp_msg_ptr = QMI_SRVC_PDU_PTR(msg);

  msg_size = QMI_SRVC_PDU_SIZE(QMI_QOS_STD_MSG_SIZE);

  if ((rc = qmi_util_write_std_tlv(&tmp_msg_ptr,
                                      &msg_size,
                                      QMI_QOS_STATUS_REQ_TLV_ID,
                                      4,
                                      (void *)&qos_identifier)) < 0)
  {
    return QMI_INTERNAL_ERR;
  }

  /* Set TLV that indicates primary */
  {
    unsigned char temp = 1;
    if (qmi_util_write_std_tlv (&tmp_msg_ptr,
                                     &msg_size,
                                     QMI_QOS_GET_GRANTED_PRIMARY_QOS_REQ_TLV_ID,
                                     1,
                                     (void *)&temp) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }

  rc = qmi_service_send_msg_sync (client_handle,
                                  QMI_QOS_SERVICE,
                                  QMI_QOS_GET_GRNTD_QOS_INFO_MSG_ID,
                                  QMI_SRVC_PDU_PTR(msg),
                                  (int)QMI_SRVC_PDU_SIZE(QMI_QOS_STD_MSG_SIZE) - msg_size,
                                  msg,
                                  &msg_size,
                                  QMI_QOS_STD_MSG_SIZE,
                                  QMI_SYNC_MSG_DEFAULT_TIMEOUT,
                                  qmi_err_code);


  tmp_msg_ptr = msg;

  {
    unsigned long type;
    unsigned long length;
    unsigned char *value_ptr;

    memset(granted_info, 0, sizeof(*granted_info));

    granted_info->tx_granted_flow_data_is_valid = 0;
    granted_info->rx_granted_flow_data_is_valid = 0;
    granted_info->dss_errno_type.param_mask = 0;
    tmp_msg_ptr = msg;

    while (msg_size > 0)
    {
      if (qmi_util_read_std_tlv (&tmp_msg_ptr,
                                    &msg_size,
                                    &type,
                                    &length,
                                    &value_ptr) < 0)
      {
        return QMI_INTERNAL_ERR;
      }

      switch (type)
      {
        case QMI_QOS_TX_FLOW_REQ_RSP_TLV_ID:
          {
            if (qmi_qos_srvc_event_prcss_nstd_flow_tlv (value_ptr,
                                                       (int)length,
                                                       &granted_info->tx_granted_flow_data) < 0)
            {
              return QMI_INTERNAL_ERR;
            }
            granted_info->tx_granted_flow_data_is_valid = 1;
          }
          break;

        case QMI_QOS_RX_FLOW_REQ_RSP_TLV_ID:
          {
            if (qmi_qos_srvc_event_prcss_nstd_flow_tlv (value_ptr,
                                                       (int)length,
                                                       &granted_info->rx_granted_flow_data) < 0)
            {
              return QMI_INTERNAL_ERR;
            }
            granted_info->rx_granted_flow_data_is_valid = 1;
          }
          break;

        case QMI_QOS_EXTENDED_ERROR_INFO_TLV_ID:
          {
            granted_info->dss_errno_type.param_mask = QMI_QOS_EXTENDED_ERROR_INFO_PARAM;
            READ_16_BIT_VAL (value_ptr,granted_info->dss_errno_type.dss_errno);
          }
          break;
        default:
          {
            QMI_DEBUG_MSG_1 ("qmi_qos_get_primary_granted_qos_info: unknown TLV type = %x",(unsigned int)type);
          }
          break;
      }
    }
  }

  /* request modem for filter params for the qos flow */
  rc = qmi_qos_get_filter_params(client_handle,
                                 qos_identifier,
                                 granted_info,
                                 qmi_err_code);
  return rc;
}



/*===========================================================================
  FUNCTION  qmi_qos_get_secondary_granted_qos_info
===========================================================================*/
/*!
@brief
  This function if successful returns the qos parameters for a
  particular QOS flow.


@return

@note

  - Dependencies
    - qmi_qos_srvc_init_client() must be called before calling this.

  - Side Effects
    - Starts event reporting
*/
/*=========================================================================*/
int
qmi_qos_get_secondary_granted_qos_info
(
  qmi_client_handle_type                  client_handle,
  unsigned long                           qos_identifier,
  qmi_qos_granted_info_rsp_type           *granted_info,
  int                                     *qmi_err_code
)
{

  unsigned char msg[QMI_QOS_STD_MSG_SIZE], *tmp_msg_ptr;
  int           msg_size, rc;

  tmp_msg_ptr = QMI_SRVC_PDU_PTR(msg);

  msg_size = QMI_SRVC_PDU_SIZE(QMI_QOS_STD_MSG_SIZE);

  if ((rc = qmi_util_write_std_tlv(&tmp_msg_ptr,
                                      &msg_size,
                                      QMI_QOS_STATUS_REQ_TLV_ID,
                                      4,
                                      (void *)&qos_identifier)) < 0)
  {
    return QMI_INTERNAL_ERR;
  }

  rc = qmi_service_send_msg_sync (client_handle,
                                  QMI_QOS_SERVICE,
                                  QMI_QOS_GET_GRNTD_QOS_INFO_MSG_ID,
                                  QMI_SRVC_PDU_PTR(msg),
                                  (int)QMI_SRVC_PDU_SIZE(QMI_QOS_STD_MSG_SIZE) - msg_size,
                                  msg,
                                  &msg_size,
                                  QMI_QOS_STD_MSG_SIZE,
                                  QMI_SYNC_MSG_DEFAULT_TIMEOUT,
                                  qmi_err_code);


  tmp_msg_ptr = msg;

  {
    unsigned long type;
    unsigned long length;
    unsigned char *value_ptr;

    memset(granted_info, 0, sizeof(*granted_info));

    granted_info->tx_granted_flow_data_is_valid = 0;
    granted_info->rx_granted_flow_data_is_valid = 0;
    granted_info->dss_errno_type.param_mask = 0;
    tmp_msg_ptr = msg;

    while (msg_size > 0)
    {
      if (qmi_util_read_std_tlv (&tmp_msg_ptr,
                                    &msg_size,
                                    &type,
                                    &length,
                                    &value_ptr) < 0)
      {
        return QMI_INTERNAL_ERR;
      }

      switch (type)
      {
        case QMI_QOS_TX_FLOW_REQ_RSP_TLV_ID:
          {
            if (qmi_qos_srvc_event_prcss_nstd_flow_tlv (value_ptr,
                                                       (int)length,
                                                       &granted_info->tx_granted_flow_data) < 0)
            {
              return QMI_INTERNAL_ERR;
            }
            granted_info->tx_granted_flow_data_is_valid = 1;
          }
          break;

        case QMI_QOS_RX_FLOW_REQ_RSP_TLV_ID:
          {
            if (qmi_qos_srvc_event_prcss_nstd_flow_tlv (value_ptr,
                                                       (int)length,
                                                       &granted_info->rx_granted_flow_data) < 0)
            {
              return QMI_INTERNAL_ERR;
            }
            granted_info->rx_granted_flow_data_is_valid = 1;
          }
          break;

        case QMI_QOS_EXTENDED_ERROR_INFO_TLV_ID:
          {
            granted_info->dss_errno_type.param_mask = QMI_QOS_EXTENDED_ERROR_INFO_PARAM;
            READ_16_BIT_VAL (value_ptr,granted_info->dss_errno_type.dss_errno);
          }
          break;
        default:
          {
            QMI_DEBUG_MSG_1 ("qmi_qos_get_secondary_granted_qos_info: unknown TLV type = %x",(unsigned int)type);
          }
          break;
      }
    }
  }

  /* request modem for filter params for the qos flow */
  rc = qmi_qos_get_filter_params(client_handle,
                                 qos_identifier,
                                 granted_info,
                                 qmi_err_code);
  return rc;
}


/*===========================================================================
  FUNCTION  qmi_qos_reset_qos_srvc_variables
===========================================================================*/
/*!
@brief
  Resets the control points state which is kept by QOS.

@return

@note

  - Dependencies
    - qmi_qos_srvc_init_client() must be called before calling this.
    - The client handle of the requesting control point doesnot change.
    - The Control points state variables will be set to default values
     before the response.
  - Side Effects
    - Starts event reporting
*/
/*=========================================================================*/
int
qmi_qos_reset_qos_srvc_variables
(
int                               client_handle,
int                               *qmi_err_code
)
{

  int          rc;

  rc = qmi_service_send_msg_sync (client_handle,
                                  QMI_QOS_SERVICE,
                                  QMI_QOS_RESET_SRVC_VARIABLES_MSG_ID,
                                  NULL,
                                  0,
                                  NULL,
                                  NULL,
                                  QMI_QOS_STD_MSG_SIZE,
                                  QMI_SYNC_MSG_DEFAULT_TIMEOUT,
                                  qmi_err_code);

  return rc;
}


/*===========================================================================
  FUNCTION  qmi_qos_suspend_qos
===========================================================================*/
/*!
@brief
  This function is a request to suspend one or more existing QoS flows.
  Each QoS flow is identified with its QoS identifier

@return

@note

  - Dependencies
    - qmi_qos_srvc_init_client() must be called before calling this.
  - Side Effects
    - Starts event reporting
*/
/*=========================================================================*/
int
qmi_qos_suspend_qos
(
  qmi_client_handle_type    client_handle,
  unsigned char             num_qos_ids,
  unsigned long             *qos_id_array,
  int                       *qmi_err_code
)
{

  unsigned char msg[QMI_QOS_STD_MSG_SIZE], *tmp_msg_ptr = NULL;
  int           msg_size, rc,index;
  unsigned long tlv_length;
  unsigned char     tmp_buff[QMI_QOS_STD_MSG_SIZE];
  unsigned char     *tmp_buff_ptr = tmp_buff;
  tmp_msg_ptr = QMI_SRVC_PDU_PTR(msg);
  msg_size = QMI_SRVC_PDU_SIZE(QMI_QOS_STD_MSG_SIZE);

  tlv_length = sizeof(unsigned char) + (unsigned long) (num_qos_ids * 4);

  WRITE_8_BIT_VAL(tmp_buff_ptr,num_qos_ids);

  for (index = 0; index < num_qos_ids; index++)
  {
    WRITE_32_BIT_VAL(tmp_buff_ptr,qos_id_array[index]);
  }

  if (qmi_util_write_std_tlv (&tmp_msg_ptr,
                                 &msg_size,
                                 QMI_QOS_IDENTIFIERS_LIST_TLV_ID,
                                 tlv_length,
                                 (void *)tmp_buff) < 0)
  {
    return QMI_INTERNAL_ERR;
  }

  rc = qmi_service_send_msg_sync (client_handle,
                                  QMI_QOS_SERVICE,
                                  QMI_QOS_SUSPEND_QOS_MSG_ID,
                                  QMI_SRVC_PDU_PTR(msg),
                                  (int)QMI_SRVC_PDU_SIZE(QMI_QOS_STD_MSG_SIZE) - msg_size,
                                  msg,
                                  &msg_size,
                                  QMI_QOS_STD_MSG_SIZE,
                                  QMI_SYNC_MSG_DEFAULT_TIMEOUT,
                                  qmi_err_code);

  return rc;
}

/*===========================================================================
  FUNCTION  qmi_qos_resume_qos
===========================================================================*/
/*!
@brief
  This function is a request to resume one or more existing QoS flows.
  Each QoS flow is identified with its QoS identifier

@return

@note
  - Number of qos_identifiers for UMTS tech should always be 1
    (not more than that).
  - Dependencies
    - qmi_qos_srvc_init_client() must be called before calling this.
  - Side Effects
    - Starts event reporting
*/
/*=========================================================================*/
int
qmi_qos_resume_qos
(
  qmi_client_handle_type    client_handle,
  unsigned char             num_qos_ids,
  unsigned long             *qos_id_array,
  int                       *qmi_err_code
)
{
  unsigned char     msg[QMI_QOS_STD_MSG_SIZE], *tmp_msg_ptr = NULL;
  int               msg_size, rc,index;
  unsigned long     tlv_length;
  unsigned char     tmp_buff[QMI_QOS_STD_MSG_SIZE];
  unsigned char     *tmp_buff_ptr = tmp_buff;
  tmp_msg_ptr = QMI_SRVC_PDU_PTR(msg);
  msg_size = QMI_SRVC_PDU_SIZE(QMI_QOS_STD_MSG_SIZE);


  tlv_length = sizeof(unsigned char) + (unsigned long) (num_qos_ids * 4);

  WRITE_8_BIT_VAL(tmp_buff_ptr,(char)num_qos_ids);
  for (index = 0; index < num_qos_ids; index++)
  {
    WRITE_32_BIT_VAL(tmp_buff_ptr,qos_id_array[index]);
  }

  if (qmi_util_write_std_tlv (&tmp_msg_ptr,
                                 &msg_size,
                                 QMI_QOS_IDENTIFIERS_LIST_TLV_ID,
                                 tlv_length,
                                 (void *)tmp_buff) < 0)
  {
    return QMI_INTERNAL_ERR;
  }

  rc = qmi_service_send_msg_sync (client_handle,
                                  QMI_QOS_SERVICE,
                                  QMI_QOS_RESUME_QOS_MSG_ID,
                                  QMI_SRVC_PDU_PTR(msg),
                                  (int)QMI_SRVC_PDU_SIZE(QMI_QOS_STD_MSG_SIZE) - msg_size,
                                  msg,
                                  &msg_size,
                                  QMI_QOS_STD_MSG_SIZE,
                                  QMI_SYNC_MSG_DEFAULT_TIMEOUT,
                                  qmi_err_code);


  return rc;
}

/*===========================================================================
  FUNCTION  qmi_qos_get_nw_supported_qos_profiles
===========================================================================*/
/*!
@brief
  This function is a request to resume one or more existing QoS flows.
  Each QoS flow is identified with its QoS identifier

@return

@note
  - Number of qos_identifiers for UMTS tech should always be 1
    (not more than that).
  - Dependencies
    - qmi_qos_srvc_init_client() must be called before calling this.
  - Side Effects
    - Starts event reporting
*/
/*=========================================================================*/
int
qmi_qos_get_nw_supported_qos_profiles
(
  qmi_client_handle_type                         client_handle,
  qmi_qos_technology_type                        tech_pref,
  qmi_qos_nw_supported_qos_profiles_rsp_type     *rsp_data,
  int                                            *qmi_err_code
)
{
  unsigned char msg[QMI_QOS_STD_MSG_SIZE], *tmp_msg_ptr;
  int           msg_size, rc;

  tmp_msg_ptr = QMI_SRVC_PDU_PTR(msg);
  msg_size = QMI_SRVC_PDU_SIZE(QMI_QOS_STD_MSG_SIZE);

  if ((rc = qmi_util_write_std_tlv(&tmp_msg_ptr,
                                      &msg_size,
                                      QMI_QOS_TECH_PREF_REQ_TLV_ID,
                                      2,
                                      (void *)&tech_pref)) < 0)
  {
    return QMI_INTERNAL_ERR;
  }

  rc = qmi_service_send_msg_sync (client_handle,
                                  QMI_QOS_SERVICE,
                                  QMI_QOS_NW_SUPPORTED_QOS_PROFILES_MSG_ID,
                                  QMI_SRVC_PDU_PTR(msg),
                                  (int)QMI_SRVC_PDU_SIZE(QMI_QOS_STD_MSG_SIZE) - msg_size,
                                  msg,
                                  &msg_size,
                                  QMI_QOS_STD_MSG_SIZE,
                                  QMI_SYNC_MSG_DEFAULT_TIMEOUT,
                                  qmi_err_code);


  {
    unsigned long type;
    unsigned long length;
    unsigned char *value_ptr;
    rsp_data->dss_err.param_mask = 0;

    tmp_msg_ptr = msg;

    while (msg_size > 0)
    {
      if (qmi_util_read_std_tlv (&tmp_msg_ptr,
                                    &msg_size,
                                    &type,
                                    &length,
                                    &value_ptr) < 0)
      {
        return QMI_INTERNAL_ERR;
      }

      switch (type)
      {
        case QMI_QOS_NW_SUPP_QOS_PROFILES_RESP_TLV_ID:
          {
            int num_instances = 0;
            int index;
            READ_8_BIT_VAL (value_ptr,rsp_data->qos_profiles.num_instances);
            num_instances = rsp_data->qos_profiles.num_instances;
            for  (index = 0; index < num_instances; index++)
            {
              int num_profiles ;
              int j;
              READ_16_BIT_VAL(value_ptr, rsp_data->qos_profiles.profile_info[index].iface_type);
              READ_8_BIT_VAL(value_ptr,rsp_data->qos_profiles.profile_info[index].num_profiles);
              num_profiles = rsp_data->qos_profiles.profile_info[index].num_profiles;
              for (j = 0; j < num_profiles; j++)
              {
                READ_16_BIT_VAL(value_ptr,rsp_data->qos_profiles.profile_info[index].profile[j]);
              }
            }
          }
          break;
        case QMI_QOS_EXTENDED_ERROR_INFO_TLV_ID:
          {
            rsp_data->dss_err.param_mask = QMI_QOS_EXTENDED_ERROR_INFO_PARAM;
            READ_16_BIT_VAL (value_ptr,rsp_data->dss_err.dss_errno);
          }
          break;
        default:
          {
            QMI_ERR_MSG_1 ("qmi_qos_get_nw_supported_qos_profiles: unknown TLV type = %x",(unsigned int)type);
          }
          break;
      }
    }
  }
  return rc;
}
/*===========================================================================
  FUNCTION  qmi_qos_perform_flow_operation
===========================================================================*/
/*!
@brief
  This function is a request to resume one or more existing QoS flows.
  Each QoS flow is identified with its QoS identifier

@return

@note
  - Number of qos_identifiers for UMTS tech should always be 1
    (not more than that).
  - Dependencies
    - qmi_qos_srvc_init_client() must be called before calling this.
  - Side Effects
    - Starts event reporting
*/
/*=========================================================================*/
int
qmi_qos_perform_flow_operation
(
  int                                   client_handle,
  qmi_qos_perform_flow_op_req_type      *params,
  qmi_qos_perform_flow_op_resp_type     *resp_data,
  int                                   *qmi_err_code
)
{
  unsigned char     msg[QMI_QOS_STD_MSG_SIZE];
  int               msg_size;
  unsigned char    *tmp_msg_ptr;
  int rc;

  tmp_msg_ptr = QMI_SRVC_PDU_PTR(msg);

  msg_size = QMI_SRVC_PDU_SIZE(QMI_QOS_STD_MSG_SIZE);

  if (qmi_util_write_std_tlv (&tmp_msg_ptr,
                                 &msg_size,
                                 QMI_QOS_PERFORM_FLOW_OP_QOS_ID_REQ_TLV_ID,
                                 4,
                                 (void *)&params->qos_identifier) < 0)
  {
    return QMI_INTERNAL_ERR;
  }

  if (params && (params->params_mask != 0))
  {
    if (qmi_qos_write_flow_operation_request_tlvs (&tmp_msg_ptr,
                                                   &msg_size,
                                                   params) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }

  rc = qmi_service_send_msg_sync (client_handle,
                                  QMI_QOS_SERVICE,
                                  QMI_QOS_PERFORM_QOS_FLOW_OPERATION_MSG_ID,
                                  QMI_SRVC_PDU_PTR(msg),
                                  (int)QMI_SRVC_PDU_SIZE(QMI_QOS_STD_MSG_SIZE) - msg_size,
                                  msg,
                                  &msg_size,
                                  QMI_QOS_STD_MSG_SIZE,
                                  QMI_SYNC_MSG_DEFAULT_TIMEOUT,
                                  qmi_err_code);

  {
    unsigned long type;
    unsigned long length;
    unsigned char *value_ptr;
    resp_data->params_mask = 0;

    tmp_msg_ptr = msg;
    memset(resp_data, 0, sizeof(*resp_data));

    while (msg_size > 0)
    {
      if (qmi_util_read_std_tlv (&tmp_msg_ptr,
                                    &msg_size,
                                    &type,
                                    &length,
                                    &value_ptr) < 0)
      {
        return QMI_INTERNAL_ERR;
      }

      switch (type)
      {
        case QMI_QOS_FLOW_OPERATION_FAILURE_RESP_TLV_ID:
          {
            /*Set Param Mask*/

            int index;
            int num_instances = 0;
            resp_data->params_mask |= QMI_QOS_PERFORM_FLOW_OP_RESP_OP_FAILURE_PARAM;
            READ_8_BIT_VAL (value_ptr,resp_data->op_fail_info.num_failures);
            num_instances = resp_data->op_fail_info.num_failures;
            for (index = 0; index < num_instances; index++)
            {
              READ_8_BIT_VAL (value_ptr,resp_data->op_fail_info.fail_info[index].bit_number);
              READ_16_BIT_VAL (value_ptr,resp_data->op_fail_info.fail_info[index].dss_errno);
            }
          }
          break;
        case QMI_QOS_FLOW_OP_TX_QUEUE_LEVEL_RESP_TLV_ID:
          {
            /*Set Param Mask*/
            resp_data->params_mask |= QMI_QOS_PERFORM_FLOW_OP_RESP_TX_QUEUE_LEVEL_PARAM;
            READ_32_BIT_VAL(value_ptr,resp_data->tx_queue_level.current_new_data_cnt);
            READ_32_BIT_VAL(value_ptr,resp_data->tx_queue_level.wm_free_cnt);
            READ_32_BIT_VAL(value_ptr,resp_data->tx_queue_level.total_pending_cnt);
          }
          break;
        case QMI_QOS_FLOW_OP_RMAC3_INFO_RESP_TLV_ID:
          {
            /*Set Param Mask*/
            resp_data->params_mask |= QMI_QOS_PERFORM_FLOW_OP_RESP_RMAC3_INFO_PARAM;
            READ_16_BIT_VAL(value_ptr,resp_data->rmac3_info.ps_headroom_payload_size);
            READ_16_BIT_VAL(value_ptr,resp_data->rmac3_info.bucket_level_payload_size);
            READ_16_BIT_VAL(value_ptr,resp_data->rmac3_info.t2p_inflow_payload_size);
          }
          break;
        case QMI_QOS_FLOW_OP_TX_INFO_INFO_RESP_TLV_ID:
          {
            /*Set Param Mask*/
            unsigned char temp;
            resp_data->params_mask |= QMI_QOS_PERFORM_FLOW_OP_RESP_TX_STATUS_PARAM;
            READ_8_BIT_VAL(value_ptr,temp);
            resp_data->flow_status = (qmi_qos_perform_flow_op_tx_status)temp;
          }
          break;
        case QMI_QOS_FLOW_OP_INACTIVITY_TIMER_RESP_TLV_ID:
          {
            /*Set Param Mask*/
            resp_data->params_mask |= QMI_QOS_PERFORM_FLOW_OP_RESP_INACTIVITY_TIMER_PARAM;
            READ_8_BIT_VAL(value_ptr,resp_data->inactivity_timer);
          }
          break;
        default:
          {
            QMI_ERR_MSG_1 ("qmi_qos_perform_flow_operation: unknown TLV type = %x",(unsigned int)type);
          }
          break;
      }
    }
  }
  return rc;
}

/*===========================================================================
  FUNCTION  qmi_qos_set_client_ip_pref
===========================================================================*/
/*!
@brief
  This message sends the prefered client IP family preference to the modem
  to support dual-IP over single RmNet.

@return
  QMI_NO_ERR if operation was sucessful, < 0 if not.  If return code is
  QMI_SERVICE_ERR, then the qmi_err_code will be valid and will
  indicate which QMI error occurred.

@note
  - This function executes synchronously, there is not currently an
    asynchronous option for this functionality.

  - Dependencies
    - None.

  - Side Effects
    - None.
*/
/*=========================================================================*/
int
qmi_qos_set_client_ip_pref
(
  int                          user_handle,
  qmi_ip_family_pref_type      ip_family,
  int                         *qmi_err_code
)
{
  unsigned char msg[QMI_QOS_STD_MSG_SIZE];
  int msg_size;
  unsigned char *pdu;

  if (!qmi_err_code)
  {
    QMI_ERR_MSG_1("qmi_qos_set_client_ip_pref bad parameters qmi_err_code=%p",
                  qmi_err_code);
    return QMI_INTERNAL_ERR;
  }

  pdu = QMI_SRVC_PDU_PTR(msg);
  msg_size = QMI_SRVC_PDU_SIZE(QMI_QOS_STD_MSG_SIZE);

  if (qmi_util_write_std_tlv(&pdu,
                             &msg_size,
                             QMI_QOS_CLIENT_IP_PREF_REQ_TLV_ID,
                             1,
                             &ip_family) < 0)
  {
    return QMI_INTERNAL_ERR;
  }

  return qmi_service_send_msg_sync(user_handle,
                                   QMI_QOS_SERVICE,
                                   QMI_QOS_SET_CLIENT_IP_FAMILY_PREF_MSG_ID,
                                   QMI_SRVC_PDU_PTR(msg),
                                   (int)QMI_SRVC_PDU_SIZE(QMI_QOS_STD_MSG_SIZE) - msg_size,
                                   msg,
                                   &msg_size,
                                   QMI_QOS_STD_MSG_SIZE,
                                   QMI_SYNC_MSG_DEFAULT_TIMEOUT,
                                   qmi_err_code);
}
