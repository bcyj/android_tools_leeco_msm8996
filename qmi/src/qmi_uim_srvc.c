/******************************************************************************
  @file    qmi_uim_srvc.c
  @brief   The QMI UIM service layer.

  $Id$

  DESCRIPTION
  QMI UIM service routines.

  INITIALIZATION AND SEQUENCING REQUIREMENTS
  qmi_uim_srvc_init_client() needs to be called before sending or receiving of any
  QoS service messages

  ---------------------------------------------------------------------------
  Copyright (c) 2010-2014 Qualcomm Technologies, Inc.
  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
******************************************************************************/


#include "qmi_i.h"
#include "qmi_service.h"
#include "qmi_uim_srvc.h"
#include "qmi_util.h"

#define QMI_UIM_STD_MSG_SIZE                            QMI_MAX_STD_MSG_SIZE

/* Indication message IDs */
#define QMI_UIM_STATUS_CHANGE_IND_MSG_ID                0x0032
#define QMI_UIM_REFRESH_IND_MSG_ID                      0x0033

/* Request Response Message IDs */
#define QMI_UIM_RESET_MSG_ID                            0x0000
#define QMI_UIM_READ_TRANSPARENT_MSG_ID                 0x0020
#define QMI_UIM_READ_RECORD_MSG_ID                      0x0021
#define QMI_UIM_WRITE_TRANSPARENT_MSG_ID                0x0022
#define QMI_UIM_WRITE_RECORD_MSG_ID                     0x0023
#define QMI_UIM_GET_FILE_ATTRIBUTES_MSG_ID              0x0024
#define QMI_UIM_SET_PIN_PROTECTION_MSG_ID               0x0025
#define QMI_UIM_VERIFY_PIN_MSG_ID                       0x0026
#define QMI_UIM_UNBLOCK_PIN_MSG_ID                      0x0027
#define QMI_UIM_CHANGE_PIN_MSG_ID                       0x0028
#define QMI_UIM_DEPERSONALIZATION_MSG_ID                0x0029
#define QMI_UIM_REFRESH_REGISTER_MSG_ID                 0x002A
#define QMI_UIM_REFRESH_OK_MSG_ID                       0x002B
#define QMI_UIM_REFRESH_COMPLETE_MSG_ID                 0x002C
#define QMI_UIM_REFRESH_GET_LAST_EVENT_MSG_ID           0x002D
#define QMI_UIM_EVENT_REG_MSG_ID                        0x002E
#define QMI_UIM_GET_CARD_STATUS_MSG_ID                  0x002F
#define QMI_UIM_POWER_DOWN_MSG_ID                       0x0030
#define QMI_UIM_POWER_UP_MSG_ID                         0x0031
#define QMI_UIM_AUTHENTICATE_MSG_ID                     0x0034
#define QMI_UIM_CLOSE_SESSION_MSG_ID                    0x0035
#define QMI_UIM_GET_SERVICE_STATUS_MSG_ID               0x0036
#define QMI_UIM_SET_SERVICE_STATUS_MSG_ID               0x0037
#define QMI_UIM_CHANGE_PROVISIONING_SESSION_MSG_ID      0x0038
#define QMI_UIM_GET_LABEL_MSG_ID                        0x0039
#define QMI_UIM_SEND_APDU_MSG_ID                        0x003B
#define QMI_UIM_SAP_CONNECTION_MSG_ID                   0x003C
#define QMI_UIM_SAP_REQUEST_MSG_ID                      0x003D
#define QMI_UIM_LOGICAL_CHANNEL_MSG_ID                  0x003F
#define QMI_UIM_GET_ATR_MSG_ID                          0x0041

/* Request TLV IDs */
#define QMI_UIM_MANDATORY_1_TLV_ID                      0x01
#define QMI_UIM_MANDATORY_2_TLV_ID                      0x02
#define QMI_UIM_MANDATORY_3_TLV_ID                      0x03
#define QMI_UIM_MANDATORY_4_TLV_ID                      0x04
#define QMI_UIM_MANDATORY_5_TLV_ID                      0x05
#define QMI_UIM_OPTIONAL_1_TLV_ID                       0x10
#define QMI_UIM_OPTIONAL_2_TLV_ID                       0x11
#define QMI_UIM_OPTIONAL_3_TLV_ID                       0x12
#define QMI_UIM_OPTIONAL_4_TLV_ID                       0x13
#define QMI_UIM_OPTIONAL_5_TLV_ID                       0x14

/* Response TLV IDs */
#define QMI_UIM_RESULT_CODE_TLV_ID                      0x02
#define QMI_UIM_OPTIONAL_1_RESP_TLV_ID                  0x10
#define QMI_UIM_OPTIONAL_2_RESP_TLV_ID                  0x11
#define QMI_UIM_OPTIONAL_3_RESP_TLV_ID                  0x12
#define QMI_UIM_OPTIONAL_4_RESP_TLV_ID                  0x13
#define QMI_UIM_OPTIONAL_5_RESP_TLV_ID                  0x14

/* TLV Sizes */
#define QMI_UIM_SESSION_INFORMATION_MIN_TLV_SIZE        (2)
#define QMI_UIM_SESSION_INFORMATION_MAX_TLV_SIZE        (QMI_UIM_SESSION_INFORMATION_MIN_TLV_SIZE + \
                                                         QMI_UIM_MAX_AID_LEN)
#define QMI_UIM_FILE_ID_MIN_TLV_SIZE                    (3)
#define QMI_UIM_FILE_ID_MAX_TLV_SIZE                    (QMI_UIM_FILE_ID_MIN_TLV_SIZE + \
                                                         QMI_UIM_MAX_FILE_PATH)
#define QMI_UIM_READ_TRANSPARENT_MAX_TLV_SIZE           (4)
#define QMI_UIM_READ_RECORD_MAX_TLV_SIZE                (4)
#define QMI_UIM_WRITE_TRANSPARENT_MIN_TLV_SIZE          (4)
#define QMI_UIM_WRITE_RECORD_MIN_TLV_SIZE               (4)
#define QMI_UIM_SET_PIN_PROTECTION_MIN_TLV_SIZE         (3)
#define QMI_UIM_SET_PIN_PROTECTION_MAX_TLV_SIZE         (QMI_UIM_SET_PIN_PROTECTION_MIN_TLV_SIZE + \
                                                         QMI_UIM_MAX_PIN_LEN)
#define QMI_UIM_VERIFY_PIN_MIN_TLV_SIZE                 (2)
#define QMI_UIM_VERIFY_PIN_MAX_TLV_SIZE                 (QMI_UIM_VERIFY_PIN_MIN_TLV_SIZE + \
                                                         QMI_UIM_MAX_PIN_LEN)
#define QMI_UIM_ENCRYPTED_PIN_MIN_TLV_SIZE              (1)
#define QMI_UIM_ENCRYPTED_PIN_MAX_TLV_SIZE              (QMI_UIM_ENCRYPTED_PIN_MIN_TLV_SIZE + \
                                                         QMI_UIM_MAX_PIN_LEN)
#define QMI_UIM_UNBLOCK_PIN_MIN_TLV_SIZE                (3)
#define QMI_UIM_UNBLOCK_PIN_MAX_TLV_SIZE                (QMI_UIM_UNBLOCK_PIN_MIN_TLV_SIZE + \
                                                         (QMI_UIM_MAX_PIN_LEN * 2))
#define QMI_UIM_CHANGE_PIN_MIN_TLV_SIZE                 (3)
#define QMI_UIM_CHANGE_PIN_MAX_TLV_SIZE                 (QMI_UIM_CHANGE_PIN_MIN_TLV_SIZE + \
                                                         (QMI_UIM_MAX_PIN_LEN * 2))
#define QMI_UIM_DEPERSONALIZATION_MIN_TLV_SIZE          (3)
#define QMI_UIM_DEPERSONALIZATION_MAX_TLV_SIZE          (QMI_UIM_DEPERSONALIZATION_MIN_TLV_SIZE + \
                                                         QMI_UIM_MAX_CK_LEN)
#define QMI_UIM_AUTH_DATA_MIN_TLV_SIZE                  (3)
#define QMI_UIM_SLOT_MAX_TLV_SIZE                       (1)
#define QMI_UIM_EVENT_REG_MAX_TLV_SIZE                  (4)
#define QMI_UIM_CAPABILITIES_MASK_MAX_TLV_SIZE          (4)
#define QMI_UIM_SERVICE_STATUS_MAX_TLV_SIZE             (1)
#define QMI_UIM_APP_INFORMATION_MIN_TLV_SIZE            (2)
#define QMI_UIM_APP_INFORMATION_MAX_TLV_SIZE            (QMI_UIM_APP_INFORMATION_MIN_TLV_SIZE + \
                                                         QMI_UIM_MAX_AID_LEN)
#define QMI_UIM_REFRESH_REGISTER_MIN_TLV_SIZE           (4)
#define QMI_UIM_REFRESH_OK_MAX_TLV_SIZE                 (1)
#define QMI_UIM_REFRESH_COMPLETE_MAX_TLV_SIZE           (1)
#define QMI_UIM_SESSION_CHANGE_MAX_TLV_SIZE             (2)
#define QMI_UIM_SAP_CONNECT_MAX_TLV_SIZE                (2)
#define QMI_UIM_SAP_DISCONNECT_MODE_MAX_TLV_SIZE        (1)
#define QMI_UIM_CHANNEL_ID_MAX_TLV_SIZE                 (1)
#define QMI_UIM_APDU_LENGTH_MIN_TLV_SIZE                (2)
#define QMI_UIM_SAP_REQUEST_MAX_TLV_SIZE                (2)
#define QMI_UIM_LOGICAL_CHANNEL_OPEN_MIN_TLV_SIZE       (1)
#define QMI_UIM_LOGICAL_CHANNEL_OPEN_MAX_TLV_SIZE       (QMI_UIM_LOGICAL_CHANNEL_OPEN_MIN_TLV_SIZE + \
                                                         QMI_UIM_MAX_AID_LEN)
#define QMI_UIM_LOGICAL_CHANNEL_CLOSE_MAX_TLV_SIZE      (QMI_UIM_CHANNEL_ID_MAX_TLV_SIZE)

/* TLV Sizes per message */
#define QMI_UIM_SET_PIN_PROTECTION_ALL_TLV_SIZE         (QMI_UIM_SESSION_INFORMATION_MAX_TLV_SIZE + \
                                                         QMI_UIM_SET_PIN_PROTECTION_MAX_TLV_SIZE)

#define QMI_UIM_READ_TRANSPARENT_ALL_TLV_SIZE           (QMI_UIM_SESSION_INFORMATION_MAX_TLV_SIZE + \
                                                         QMI_UIM_FILE_ID_MAX_TLV_SIZE + \
                                                         QMI_UIM_READ_TRANSPARENT_MAX_TLV_SIZE)

#define QMI_UIM_READ_RECORD_ALL_TLV_SIZE                (QMI_UIM_SESSION_INFORMATION_MAX_TLV_SIZE + \
                                                         QMI_UIM_FILE_ID_MAX_TLV_SIZE + \
                                                         QMI_UIM_READ_RECORD_MAX_TLV_SIZE)

#define QMI_UIM_GET_FILE_ATTRIBUTES_ALL_TLV_SIZE        (QMI_UIM_SESSION_INFORMATION_MAX_TLV_SIZE + \
                                                         QMI_UIM_FILE_ID_MAX_TLV_SIZE)

#define QMI_UIM_REFRESH_REGISTER_ALL_TLV_SIZE           (QMI_UIM_SESSION_INFORMATION_MAX_TLV_SIZE + \
                                                         QMI_UIM_REFRESH_REGISTER_MIN_TLV_SIZE)

#define QMI_UIM_REFRESH_OK_ALL_TLV_SIZE                 (QMI_UIM_SESSION_INFORMATION_MAX_TLV_SIZE + \
                                                         QMI_UIM_REFRESH_OK_MAX_TLV_SIZE)

#define QMI_UIM_REFRESH_COMPLETE_ALL_TLV_SIZE           (QMI_UIM_SESSION_INFORMATION_MAX_TLV_SIZE + \
                                                         QMI_UIM_REFRESH_COMPLETE_MAX_TLV_SIZE)

#define QMI_UIM_REFRESH_GET_LASTEVENT_ALL_TLV_SIZE      (QMI_UIM_SESSION_INFORMATION_MAX_TLV_SIZE)

#define QMI_UIM_VERIFY_PIN_ALL_TLV_SIZE                 (QMI_UIM_SESSION_INFORMATION_MAX_TLV_SIZE + \
                                                         QMI_UIM_VERIFY_PIN_MAX_TLV_SIZE + \
                                                         QMI_UIM_ENCRYPTED_PIN_MAX_TLV_SIZE)

#define QMI_UIM_UNBLOCK_PIN_ALL_TLV_SIZE                (QMI_UIM_SESSION_INFORMATION_MAX_TLV_SIZE + \
                                                         QMI_UIM_UNBLOCK_PIN_MAX_TLV_SIZE)

#define QMI_UIM_CHANGE_PIN_ALL_TLV_SIZE                 (QMI_UIM_SESSION_INFORMATION_MAX_TLV_SIZE + \
                                                         QMI_UIM_CHANGE_PIN_MAX_TLV_SIZE)

#define QMI_UIM_DEPERSONALIZATION_ALL_TLV_SIZE          (QMI_UIM_DEPERSONALIZATION_MAX_TLV_SIZE)

#define QMI_UIM_POWER_DOWN_ALL_TLV_SIZE                 (QMI_UIM_SLOT_MAX_TLV_SIZE)

#define QMI_UIM_POWER_UP_ALL_TLV_SIZE                   (QMI_UIM_SLOT_MAX_TLV_SIZE)

#define QMI_UIM_GET_SERVICE_STATUS_ALL_TLV_SIZE         (QMI_UIM_SESSION_INFORMATION_MAX_TLV_SIZE + \
                                                         QMI_UIM_CAPABILITIES_MASK_MAX_TLV_SIZE)

#define QMI_UIM_SET_SERVICE_STATUS_ALL_TLV_SIZE         (QMI_UIM_SESSION_INFORMATION_MAX_TLV_SIZE + \
                                                         QMI_UIM_SERVICE_STATUS_MAX_TLV_SIZE)

#define QMI_UIM_CHANGE_PROV_SESSION_ALL_TLV_SIZE        (QMI_UIM_SESSION_CHANGE_MAX_TLV_SIZE + \
                                                         QMI_UIM_APP_INFORMATION_MAX_TLV_SIZE)

#define QMI_UIM_GET_LABEL_ALL_TLV_SIZE                  (QMI_UIM_APP_INFORMATION_MAX_TLV_SIZE)

#define QMI_UIM_CLOSE_SESSION_ALL_TLV_SIZE              (QMI_UIM_SESSION_INFORMATION_MAX_TLV_SIZE)

#define QMI_UIM_SAP_CONNECTION_ALL_TLV_SIZE             (QMI_UIM_SAP_CONNECT_MAX_TLV_SIZE + \
                                                         QMI_UIM_SAP_DISCONNECT_MODE_MAX_TLV_SIZE)

#define QMI_UIM_LOGICAL_CHANNEL_ALL_TLV_SIZE            (QMI_UIM_SLOT_MAX_TLV_SIZE + \
                                                         QMI_UIM_LOGICAL_CHANNEL_OPEN_MAX_TLV_SIZE + \
                                                         QMI_UIM_LOGICAL_CHANNEL_CLOSE_MAX_TLV_SIZE)

#define QMI_UIM_GET_ATR_ALL_TLV_SIZE                    (QMI_UIM_SLOT_MAX_TLV_SIZE)

static int uim_service_initialized = FALSE;


/*---------------------------------------------------------------------------
  TLV Read/Write functions
---------------------------------------------------------------------------*/

/*===========================================================================
  FUNCTION  qmi_uim_write_read_transparent_tlv
===========================================================================*/
/*!
@brief
  Takes the input parameter data for read transparent, and writes it in TLV
  form to the passed buffer.  Buffer pointers and length indicators are
  adjusted to reflect new TLVs.

@return
  QMI_INTERNAL_ERR if an error occurred, QMI_NO_ERR if not

@note

  - Dependencies
    - None.

  - Side Effects
    - Writes TLV to input buffer and updates pointers and byte count
*/
/*=========================================================================*/
static int qmi_uim_write_read_transparent_tlv
(
  unsigned char                                 ** msg,
  int                                           *  msg_size,
  const qmi_uim_read_transparent_params_type    *  params
)
{
  unsigned char     param_buf [QMI_UIM_READ_TRANSPARENT_ALL_TLV_SIZE];
  unsigned char   * param_ptr = param_buf;
  unsigned long     size = QMI_UIM_SESSION_INFORMATION_MIN_TLV_SIZE;
  unsigned short    temp_path = 0;
  unsigned short    i = 0;

  /* First write Session info TLV (mandatory) */
  WRITE_8_BIT_VAL (param_ptr, params->session_info.session_type);
  WRITE_8_BIT_VAL (param_ptr, params->session_info.aid.data_len);
  if(params->session_info.aid.data_len > 0)
  {
    size += params->session_info.aid.data_len;
    if(size > QMI_UIM_SESSION_INFORMATION_MAX_TLV_SIZE)
    {
      return QMI_INTERNAL_ERR;
    }
    memcpy (param_ptr, (char *)params->session_info.aid.data_ptr,
            params->session_info.aid.data_len);
    param_ptr += params->session_info.aid.data_len;
  }
  if (qmi_util_write_std_tlv (msg,
                              msg_size,
                              QMI_UIM_MANDATORY_1_TLV_ID,
                              size,
                              (void *)param_buf) < 0)
  {
    return QMI_INTERNAL_ERR;
  }

  /* Next is File ID TLV (mandatory) */
  WRITE_16_BIT_VAL (param_ptr, params->file_id.file_id);
  WRITE_8_BIT_VAL (param_ptr, params->file_id.path.data_len);
  /* Construct File path which is a sequence of 2 byte blocks */
  for(i = 0; i < params->file_id.path.data_len; )
  {
    temp_path = (unsigned short)((*(params->file_id.path.data_ptr + i) << 8) |
                                 (*(params->file_id.path.data_ptr + i + 1)));
    WRITE_16_BIT_VAL (param_ptr, temp_path);
    i = (unsigned short)(i + 2);
  }

  if (qmi_util_write_std_tlv (msg,
                              msg_size,
                              QMI_UIM_MANDATORY_2_TLV_ID,
                              QMI_UIM_FILE_ID_MIN_TLV_SIZE + (unsigned long)params->file_id.path.data_len,
                              (void *)(param_buf + size)) < 0)
  {
    return QMI_INTERNAL_ERR;
  }
  size = size + QMI_UIM_FILE_ID_MIN_TLV_SIZE + params->file_id.path.data_len;

  if(size > QMI_UIM_SESSION_INFORMATION_MAX_TLV_SIZE + QMI_UIM_FILE_ID_MAX_TLV_SIZE)
  {
    return QMI_INTERNAL_ERR;
  }
  /* Next is Read transparent TLV (mandatory) */
  WRITE_16_BIT_VAL (param_ptr, params->offset);
  WRITE_16_BIT_VAL (param_ptr, params->length);
  if (qmi_util_write_std_tlv (msg,
                              msg_size,
                              QMI_UIM_MANDATORY_3_TLV_ID,
                              QMI_UIM_READ_TRANSPARENT_MAX_TLV_SIZE,
                              (void *)(param_buf + size)) < 0)
  {
    return QMI_INTERNAL_ERR;
  }
  return QMI_NO_ERR;
} /* qmi_uim_write_read_transparent_tlv */


/*===========================================================================
  FUNCTION  qmi_uim_write_read_record_tlv
===========================================================================*/
/*!
@brief
  Takes the input parameter data for read record, and writes it in TLV
  form to the passed buffer.  Buffer pointers and length indicators are
  adjusted to reflect new TLVs.

@return
  QMI_INTERNAL_ERR if an error occurred, QMI_NO_ERR if not

@note

  - Dependencies
    - None.

  - Side Effects
    - Writes TLV to input buffer and updates pointers and byte count
*/
/*=========================================================================*/
static int qmi_uim_write_read_record_tlv
(
  unsigned char                                 ** msg,
  int                                           *  msg_size,
  const qmi_uim_read_record_params_type         *  params
)
{
  unsigned char     param_buf [QMI_UIM_READ_RECORD_ALL_TLV_SIZE];
  unsigned char   * param_ptr = param_buf;
  unsigned long     size = QMI_UIM_SESSION_INFORMATION_MIN_TLV_SIZE;
  unsigned short    temp_path = 0;
  int               i = 0;

  /* First write Session info TLV (mandatory) */
  WRITE_8_BIT_VAL (param_ptr, params->session_info.session_type);
  WRITE_8_BIT_VAL (param_ptr, params->session_info.aid.data_len);
  if(params->session_info.aid.data_len > 0)
  {
    size += params->session_info.aid.data_len;
    if(size > QMI_UIM_SESSION_INFORMATION_MAX_TLV_SIZE)
    {
      return QMI_INTERNAL_ERR;
    }
    memcpy (param_ptr, (char *)params->session_info.aid.data_ptr,
            params->session_info.aid.data_len);
    param_ptr += params->session_info.aid.data_len;
  }
  if (qmi_util_write_std_tlv (msg,
                              msg_size,
                              QMI_UIM_MANDATORY_1_TLV_ID,
                              size,
                              (void *)param_buf) < 0)
  {
    return QMI_INTERNAL_ERR;
  }

  /* Next is File ID TLV (mandatory) */
  WRITE_16_BIT_VAL (param_ptr, params->file_id.file_id);
  WRITE_8_BIT_VAL (param_ptr, params->file_id.path.data_len);
  /* Construct File path which is a sequence of 2 byte blocks */
  for(i = 0; i < params->file_id.path.data_len; i+=2)
  {
    temp_path = (unsigned short)((*(params->file_id.path.data_ptr + i) << 8) |
                                 (*(params->file_id.path.data_ptr + i + 1)));
    WRITE_16_BIT_VAL (param_ptr, temp_path);
  }

  if (qmi_util_write_std_tlv (msg,
                              msg_size,
                              QMI_UIM_MANDATORY_2_TLV_ID,
                              QMI_UIM_FILE_ID_MIN_TLV_SIZE + (unsigned long)params->file_id.path.data_len,
                              (void *)(param_buf + size)) < 0)
  {
    return QMI_INTERNAL_ERR;
  }
  size = size + QMI_UIM_FILE_ID_MIN_TLV_SIZE + params->file_id.path.data_len;

  if(size > QMI_UIM_SESSION_INFORMATION_MAX_TLV_SIZE + QMI_UIM_FILE_ID_MAX_TLV_SIZE)
  {
    return QMI_INTERNAL_ERR;
  }
  /* Next is Read Record TLV (mandatory) */
  WRITE_16_BIT_VAL (param_ptr, params->record);
  WRITE_16_BIT_VAL (param_ptr, params->length);
  if (qmi_util_write_std_tlv (msg,
                              msg_size,
                              QMI_UIM_MANDATORY_3_TLV_ID,
                              QMI_UIM_READ_RECORD_MAX_TLV_SIZE,
                              (void *)(param_buf + size)) < 0)
  {
    return QMI_INTERNAL_ERR;
  }

  return QMI_NO_ERR;
} /* qmi_uim_write_read_record_tlv */


/*===========================================================================
  FUNCTION  qmi_uim_write_write_transparent_tlv
===========================================================================*/
/*!
@brief
  Takes the input parameter data for write transparent, and writes it in TLV
  form to the passed buffer.  Buffer pointers and length indicators are
  adjusted to reflect new TLVs.

@return
  QMI_INTERNAL_ERR if an error occurred, QMI_NO_ERR if not

@note

  - Dependencies
    - None.

  - Side Effects
    - Writes TLV to input buffer and updates pointers and byte count
*/
/*=========================================================================*/
static int qmi_uim_write_write_transparent_tlv
(
  unsigned char                                 ** msg,
  int                                           *  msg_size,
  const qmi_uim_write_transparent_params_type   *  params
)
{
  // Todo - change msg below to handle size dynamically (more than 2K bytes)
  unsigned char     param_buf [QMI_SRVC_PDU_SIZE(QMI_UIM_STD_MSG_SIZE)];
  unsigned char   * param_ptr  = param_buf;
  unsigned long     size = QMI_UIM_SESSION_INFORMATION_MIN_TLV_SIZE;
  unsigned short    temp_path = 0;
  int               i = 0;

  /* First write Session info TLV (mandatory) */
  WRITE_8_BIT_VAL (param_ptr, params->session_info.session_type);
  WRITE_8_BIT_VAL (param_ptr, params->session_info.aid.data_len);
  if(params->session_info.aid.data_len > 0)
  {
    memcpy (param_ptr, (char *)params->session_info.aid.data_ptr,
            params->session_info.aid.data_len);
    param_ptr += params->session_info.aid.data_len;
    size += params->session_info.aid.data_len;
  }
  if(size >= QMI_SRVC_PDU_SIZE(QMI_UIM_STD_MSG_SIZE))
  {
    return QMI_INTERNAL_ERR;
  }
  if (qmi_util_write_std_tlv (msg,
                              msg_size,
                              QMI_UIM_MANDATORY_1_TLV_ID,
                              size,
                              (void *)param_buf) < 0)
  {
    return QMI_INTERNAL_ERR;
  }

  /* Next is File ID TLV (mandatory) */
  WRITE_16_BIT_VAL (param_ptr, params->file_id.file_id);
  WRITE_8_BIT_VAL (param_ptr, params->file_id.path.data_len);
  /* Construct File path which is a sequence of 2 byte blocks */
  for(i = 0; i < params->file_id.path.data_len; i+=2)
  {
    temp_path = (unsigned short)((*(params->file_id.path.data_ptr + i) << 8) |
                                 (*(params->file_id.path.data_ptr + i + 1)));
    WRITE_16_BIT_VAL (param_ptr, temp_path);
  }

  if (qmi_util_write_std_tlv (msg,
                              msg_size,
                              QMI_UIM_MANDATORY_2_TLV_ID,
                              QMI_UIM_FILE_ID_MIN_TLV_SIZE + (unsigned long)params->file_id.path.data_len,
                              (void *)(param_buf + size)) < 0)
  {
    return QMI_INTERNAL_ERR;
  }
  size = size + QMI_UIM_FILE_ID_MIN_TLV_SIZE + params->file_id.path.data_len;

  if(size >= QMI_SRVC_PDU_SIZE(QMI_UIM_STD_MSG_SIZE))
  {
    return QMI_INTERNAL_ERR;
  }
  /* Next is Write Transparent TLV (mandatory) */
  WRITE_16_BIT_VAL (param_ptr, params->offset);
  WRITE_16_BIT_VAL (param_ptr, params->data.data_len);
  if(params->data.data_len > 0)
  {
    memcpy (param_ptr, (char *)params->data.data_ptr, params->data.data_len);
  }
  if (qmi_util_write_std_tlv (msg,
                              msg_size,
                              QMI_UIM_MANDATORY_3_TLV_ID,
                              QMI_UIM_WRITE_TRANSPARENT_MIN_TLV_SIZE + (unsigned long)params->data.data_len,
                              (void *)(param_buf + size)) < 0)
  {
    return QMI_INTERNAL_ERR;
  }
  return QMI_NO_ERR;
} /* qmi_uim_write_write_transparent_tlv */


/*===========================================================================
  FUNCTION  qmi_uim_write_write_record_tlv
===========================================================================*/
/*!
@brief
  Takes the input parameter data for write record, and writes it in TLV
  form to the passed buffer.  Buffer pointers and length indicators are
  adjusted to reflect new TLVs.

@return
  QMI_INTERNAL_ERR if an error occurred, QMI_NO_ERR if not

@note

  - Dependencies
    - None.

  - Side Effects
    - Writes TLV to input buffer and updates pointers and byte count
*/
/*=========================================================================*/
static int qmi_uim_write_write_record_tlv
(
  unsigned char                                 ** msg,
  int                                           *  msg_size,
  const qmi_uim_write_record_params_type        *  params
)
{
  // Todo - change msg below to handle size dynamically (more than 2K bytes)
  unsigned char     param_buf [QMI_SRVC_PDU_SIZE(QMI_UIM_STD_MSG_SIZE)];
  unsigned char   * param_ptr  = param_buf;
  unsigned long     size = QMI_UIM_SESSION_INFORMATION_MIN_TLV_SIZE;
  unsigned short    temp_path = 0;
  int               i = 0;

  /* First write Session info TLV (mandatory) */
  WRITE_8_BIT_VAL (param_ptr, params->session_info.session_type);
  WRITE_8_BIT_VAL (param_ptr, params->session_info.aid.data_len);
  if(params->session_info.aid.data_len > 0)
  {
    memcpy (param_ptr, (char *)params->session_info.aid.data_ptr,
            params->session_info.aid.data_len);
    param_ptr += params->session_info.aid.data_len;
    size += params->session_info.aid.data_len;
  }
  if(size >= QMI_SRVC_PDU_SIZE(QMI_UIM_STD_MSG_SIZE))
  {
    return QMI_INTERNAL_ERR;
  }
  if (qmi_util_write_std_tlv (msg,
                              msg_size,
                              QMI_UIM_MANDATORY_1_TLV_ID,
                              size,
                              (void *)param_buf) < 0)
  {
    return QMI_INTERNAL_ERR;
  }

  /* Next is File ID TLV (mandatory) */
  WRITE_16_BIT_VAL (param_ptr, params->file_id.file_id);
  WRITE_8_BIT_VAL (param_ptr, params->file_id.path.data_len);
  /* Construct File path which is a sequence of 2 byte blocks */
  for(i = 0; i < params->file_id.path.data_len; i+=2)
  {
    temp_path = (unsigned short)((*(params->file_id.path.data_ptr + i) << 8) |
                                 (*(params->file_id.path.data_ptr + i + 1)));
    WRITE_16_BIT_VAL (param_ptr, temp_path);
  }

  if (qmi_util_write_std_tlv (msg,
                              msg_size,
                              QMI_UIM_MANDATORY_2_TLV_ID,
                              QMI_UIM_FILE_ID_MIN_TLV_SIZE + (unsigned long)params->file_id.path.data_len,
                              (void *)(param_buf + size)) < 0)
  {
    return QMI_INTERNAL_ERR;
  }
  size = size + QMI_UIM_FILE_ID_MIN_TLV_SIZE + params->file_id.path.data_len;

  if(size >= QMI_SRVC_PDU_SIZE(QMI_UIM_STD_MSG_SIZE))
  {
    return QMI_INTERNAL_ERR;
  }
  /* Next is Write Record TLV (mandatory) */
  WRITE_16_BIT_VAL (param_ptr, params->record);
  WRITE_16_BIT_VAL (param_ptr, params->data.data_len);
  if(params->data.data_len > 0)
  {
    memcpy (param_ptr, (char *)params->data.data_ptr, params->data.data_len);
  }
  if (qmi_util_write_std_tlv (msg,
                              msg_size,
                              QMI_UIM_MANDATORY_3_TLV_ID,
                              QMI_UIM_WRITE_RECORD_MIN_TLV_SIZE + (unsigned long)params->data.data_len,
                              (void *)(param_buf + size)) < 0)
  {
    return QMI_INTERNAL_ERR;
  }
  return QMI_NO_ERR;
} /* qmi_uim_write_write_record_tlv */


/*===========================================================================
  FUNCTION  qmi_uim_write_get_file_attributes_tlv
===========================================================================*/
/*!
@brief
  Takes the input parameter data for get file attributes, and writes it in
  TLV form to the passed buffer. Buffer pointers and length indicators are
  adjusted to reflect new TLVs.

@return
  QMI_INTERNAL_ERR if an error occurred, QMI_NO_ERR if not

@note

  - Dependencies
    - None.

  - Side Effects
    - Writes TLV to input buffer and updates pointers and byte count
*/
/*=========================================================================*/
static int qmi_uim_write_get_file_attributes_tlv
(
  unsigned char                                     ** msg,
  int                                               *  msg_size,
  const qmi_uim_get_file_attributes_params_type     *  params
)
{
  unsigned char     param_buf [QMI_UIM_GET_FILE_ATTRIBUTES_ALL_TLV_SIZE];
  unsigned char   * param_ptr = param_buf;
  unsigned long     size = QMI_UIM_SESSION_INFORMATION_MIN_TLV_SIZE;
  unsigned short    temp_path = 0;
  int               i;

  /* First write Session info TLV (mandatory) */
  WRITE_8_BIT_VAL (param_ptr, params->session_info.session_type);
  WRITE_8_BIT_VAL (param_ptr, params->session_info.aid.data_len);
  if(params->session_info.aid.data_len > 0)
  {
    memcpy (param_ptr, (char *)params->session_info.aid.data_ptr,
            params->session_info.aid.data_len);
    param_ptr += params->session_info.aid.data_len;
    size += params->session_info.aid.data_len;
  }
  if(size >= QMI_UIM_GET_FILE_ATTRIBUTES_ALL_TLV_SIZE)
  {
    return QMI_INTERNAL_ERR;
  }
  if (qmi_util_write_std_tlv (msg,
                              msg_size,
                              QMI_UIM_MANDATORY_1_TLV_ID,
                              size,
                              (void *)param_buf) < 0)
  {
    return QMI_INTERNAL_ERR;
  }

  /* Next is File ID TLV (mandatory) */
  WRITE_16_BIT_VAL (param_ptr, params->file_id.file_id);
  WRITE_8_BIT_VAL (param_ptr, params->file_id.path.data_len);
  /* Construct File path which is a sequence of 2 byte blocks */
  for(i = 0; i < params->file_id.path.data_len; i+=2)
  {
    temp_path = (unsigned short)((*(params->file_id.path.data_ptr + i) << 8) |
                                 (*(params->file_id.path.data_ptr + i + 1)));
    WRITE_16_BIT_VAL (param_ptr, temp_path);
  }

  if (qmi_util_write_std_tlv (msg,
                              msg_size,
                              QMI_UIM_MANDATORY_2_TLV_ID,
                              QMI_UIM_FILE_ID_MIN_TLV_SIZE + (unsigned long)params->file_id.path.data_len,
                              (void *)(param_buf + size)) < 0)
  {
    return QMI_INTERNAL_ERR;
  }
  return QMI_NO_ERR;
} /* qmi_uim_write_get_file_attributes_tlv */


/*===========================================================================
  FUNCTION  qmi_uim_write_refresh_register_tlv
===========================================================================*/
/*!
@brief
  Takes the input parameter data for refresh register, and writes it in
  TLV form to the passed buffer. Buffer pointers and length indicators are
  adjusted to reflect new TLVs.

@return
  QMI_INTERNAL_ERR if an error occurred, QMI_NO_ERR if not

@note

  - Dependencies
    - None.

  - Side Effects
    - Writes TLV to input buffer and updates pointers and byte count
*/
/*=========================================================================*/
static int qmi_uim_write_refresh_register_tlv
(
  unsigned char                                     ** msg,
  int                                               *  msg_size,
  const qmi_uim_refresh_register_params_type        *  params,
  unsigned char                                     *  param_ptr,
  unsigned int                                         param_size
)
{
  unsigned int      size = 0;
  unsigned int      total_len = 0;
  unsigned short    temp_path = 0;
  int               i = 0, j = 0;

  /* Allocate buffer based on requested number of files */
  if ((params == NULL) || (param_ptr == NULL))
  {
    return QMI_INTERNAL_ERR;
  }

  /* First write Session info TLV (mandatory) */
  size = QMI_UIM_SESSION_INFORMATION_MIN_TLV_SIZE;
  WRITE_8_BIT_VAL (param_ptr, params->session_info.session_type);
  WRITE_8_BIT_VAL (param_ptr, params->session_info.aid.data_len);
  if(params->session_info.aid.data_len > 0)
  {
    memcpy (param_ptr,
            (char *)params->session_info.aid.data_ptr,
            params->session_info.aid.data_len);
    param_ptr += params->session_info.aid.data_len;
    size += params->session_info.aid.data_len;
  }

  if(size >= param_size)
  {
    return QMI_INTERNAL_ERR;
  }

  if (qmi_util_write_std_tlv (msg,
                              msg_size,
                              QMI_UIM_MANDATORY_1_TLV_ID,
                              size,
                              (void *)(param_ptr - size)) < 0)
  {
    return QMI_INTERNAL_ERR;
  }

  /* Next is Register refresh TLV (mandatory) */
  WRITE_8_BIT_VAL (param_ptr, params->reg_for_refresh);
  WRITE_8_BIT_VAL (param_ptr, params->vote_for_init);
  WRITE_16_BIT_VAL (param_ptr, params->num_files);

  /* Write individual files */
  for (i = 0; i < params->num_files; i++)
  {
    WRITE_16_BIT_VAL (param_ptr, params->files_ptr[i].file_id);
    WRITE_8_BIT_VAL (param_ptr, params->files_ptr[i].path.data_len);

    if((params->files_ptr[i].path.data_len > 0) &&
       (params->files_ptr[i].path.data_len <= QMI_UIM_MAX_FILE_PATH))
    {
      /* Construct File path which is a sequence of 2 byte blocks */
      for(j = 0; j < params->files_ptr[i].path.data_len; j+=2)
      {
        temp_path = (unsigned short)((*(params->files_ptr[i].path.data_ptr + j) << 8) |
                                     (*(params->files_ptr[i].path.data_ptr + j + 1)));
        WRITE_16_BIT_VAL (param_ptr, temp_path);
      }
      total_len += params->files_ptr[i].path.data_len;
    }
    else
    {
      return QMI_INTERNAL_ERR;
    }
  }
  /* Calculate total size of TLV */
  total_len += (unsigned int)(QMI_UIM_REFRESH_REGISTER_MIN_TLV_SIZE +
                              (QMI_UIM_FILE_ID_MIN_TLV_SIZE * params->num_files));

  if(total_len >= (param_size - size))
  {
    return QMI_INTERNAL_ERR;
  }

  if (qmi_util_write_std_tlv (msg,
                              msg_size,
                              QMI_UIM_MANDATORY_2_TLV_ID,
                              total_len,
                              (void *)(param_ptr - total_len)) < 0)
  {
    return QMI_INTERNAL_ERR;
  }

  return QMI_NO_ERR;
}
/* qmi_uim_write_refresh_register_tlv */


/*===========================================================================
  FUNCTION  qmi_uim_write_refresh_ok_tlv
===========================================================================*/
/*!
@brief
  Takes the input parameter data for refresh ok, and writes it in
  TLV form to the passed buffer. Buffer pointers and length indicators are
  adjusted to reflect new TLVs.

@return
  QMI_INTERNAL_ERR if an error occurred, QMI_NO_ERR if not

@note

  - Dependencies
    - None.

  - Side Effects
    - Writes TLV to input buffer and updates pointers and byte count
*/
/*=========================================================================*/
static int qmi_uim_write_refresh_ok_tlv
(
  unsigned char                               ** msg,
  int                                         *  msg_size,
  const qmi_uim_refresh_ok_params_type        *  params
)
{
    unsigned char     param_buf [QMI_UIM_REFRESH_OK_ALL_TLV_SIZE];
    unsigned char   * param_ptr = param_buf;
    unsigned long     size = QMI_UIM_SESSION_INFORMATION_MIN_TLV_SIZE;
    unsigned short    temp_len = 0;

    /* First write Session info TLV (mandatory) */
    WRITE_8_BIT_VAL (param_ptr, params->session_info.session_type);
    temp_len = params->session_info.aid.data_len;
    WRITE_8_BIT_VAL (param_ptr, temp_len);
    if(temp_len > 0)
    {
      memcpy (param_ptr, (char *)params->session_info.aid.data_ptr, temp_len);
      param_ptr += temp_len;
      size += temp_len;
    }
    if(size >= QMI_UIM_REFRESH_OK_ALL_TLV_SIZE)
    {
      return QMI_INTERNAL_ERR;
    }
    if (qmi_util_write_std_tlv (msg,
                                msg_size,
                                QMI_UIM_MANDATORY_1_TLV_ID,
                                size,
                                (void *)param_buf) < 0)
    {
      return QMI_INTERNAL_ERR;
    }

    /* Next is Register OK TLV (mandatory) */
    WRITE_8_BIT_VAL (param_ptr, params->ok_to_refresh);

    if (qmi_util_write_std_tlv (msg,
                                msg_size,
                                QMI_UIM_MANDATORY_2_TLV_ID,
                                QMI_UIM_REFRESH_OK_MAX_TLV_SIZE,
                                (void *)(param_buf + size)) < 0)
    {
      return QMI_INTERNAL_ERR;
    }

  return QMI_NO_ERR;
}
/* qmi_uim_write_refresh_ok_tlv */


/*===========================================================================
  FUNCTION  qmi_uim_write_refresh_complete_tlv
===========================================================================*/
/*!
@brief
  Takes the input parameter data for refresh complete command, and writes it
  in TLV form to the passed buffer. Buffer pointers and length indicators are
  adjusted to reflect new TLVs.

@return
  QMI_INTERNAL_ERR if an error occurred, QMI_NO_ERR if not

@note

  - Dependencies
    - None.

  - Side Effects
    - Writes TLV to input buffer and updates pointers and byte count
*/
/*=========================================================================*/
static int qmi_uim_write_refresh_complete_tlv
(
  unsigned char                                   ** msg,
  int                                             *  msg_size,
  const qmi_uim_refresh_complete_params_type      *  params
)
{
    unsigned char     param_buf [QMI_UIM_REFRESH_COMPLETE_ALL_TLV_SIZE];
    unsigned char   * param_ptr = param_buf;
    unsigned long     size = QMI_UIM_SESSION_INFORMATION_MIN_TLV_SIZE;
    unsigned short    temp_len = 0;

    /* First write Session info TLV (mandatory) */
    WRITE_8_BIT_VAL (param_ptr, params->session_info.session_type);
    temp_len = params->session_info.aid.data_len;
    WRITE_8_BIT_VAL (param_ptr, temp_len);
    if(temp_len > 0)
    {
      memcpy (param_ptr, (char *)params->session_info.aid.data_ptr, temp_len);
      param_ptr += temp_len;
      size += temp_len;
    }
    if(size >= QMI_UIM_REFRESH_COMPLETE_ALL_TLV_SIZE)
    {
      return QMI_INTERNAL_ERR;
    }
    if (qmi_util_write_std_tlv (msg,
                                msg_size,
                                QMI_UIM_MANDATORY_1_TLV_ID,
                                size,
                                (void *)param_buf) < 0)
    {
      return QMI_INTERNAL_ERR;
    }

    /* Next is Register complete TLV (mandatory) */
    WRITE_8_BIT_VAL (param_ptr, params->refresh_success);

    if (qmi_util_write_std_tlv (msg,
                                msg_size,
                                QMI_UIM_MANDATORY_2_TLV_ID,
                                QMI_UIM_REFRESH_COMPLETE_MAX_TLV_SIZE,
                                (void *)(param_buf + size)) < 0)
    {
      return QMI_INTERNAL_ERR;
    }

  return QMI_NO_ERR;
}
/* qmi_uim_write_refresh_complete_tlv */


/*===========================================================================
  FUNCTION  qmi_uim_write_refresh_get_last_event_tlv
===========================================================================*/
/*!
@brief
  Takes the input parameter data for refresh ge tlast event command, and
  writes it in TLV form to the passed buffer. Buffer pointers and length
  indicators are adjusted to reflect new TLVs.

@return
  QMI_INTERNAL_ERR if an error occurred, QMI_NO_ERR if not

@note

  - Dependencies
    - None.

  - Side Effects
    - Writes TLV to input buffer and updates pointers and byte count
*/
/*=========================================================================*/
static int qmi_uim_write_refresh_get_last_event_tlv
(
  unsigned char                                         ** msg,
  int                                                   *  msg_size,
  const qmi_uim_refresh_get_last_event_params_type      *  params
)
{
    unsigned char     param_buf [QMI_UIM_REFRESH_GET_LASTEVENT_ALL_TLV_SIZE];
    unsigned char   * param_ptr = param_buf;
    unsigned long     size = QMI_UIM_SESSION_INFORMATION_MIN_TLV_SIZE;
    unsigned short    temp_len = 0;

    /* First write Session info TLV (mandatory) */
    WRITE_8_BIT_VAL (param_ptr, params->session_info.session_type);
    temp_len = params->session_info.aid.data_len;
    WRITE_8_BIT_VAL (param_ptr, temp_len);
    if(temp_len > 0)
    {
      memcpy (param_ptr, (char *)params->session_info.aid.data_ptr, temp_len);
    }
    if (qmi_util_write_std_tlv (msg,
                                msg_size,
                                QMI_UIM_MANDATORY_1_TLV_ID,
                                size + temp_len,
                                (void *)param_buf) < 0)
    {
      return QMI_INTERNAL_ERR;
    }

  return QMI_NO_ERR;
}
/* qmi_uim_write_refresh_get_last_event_tlv */


/*===========================================================================
  FUNCTION  qmi_uim_write_pin_protection_tlv
===========================================================================*/
/*!
@brief
  Takes the input pin protection parameter data, and writes it in TLV form
  to the passed buffer.  Buffer pointers and length indicators are adjusted
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
static int qmi_uim_write_pin_protection_tlv
(
  unsigned char                                   ** msg,
  int                                             *  msg_size,
  const qmi_uim_set_pin_protection_params_type    *  params
)
{
  unsigned char     param_buf [QMI_UIM_SET_PIN_PROTECTION_ALL_TLV_SIZE];
  unsigned char   * param_ptr = param_buf;
  unsigned long     size = QMI_UIM_SESSION_INFORMATION_MIN_TLV_SIZE;

  /* First write Session Info TLV */
  WRITE_8_BIT_VAL (param_ptr, params->session_info.session_type);
  WRITE_8_BIT_VAL (param_ptr, params->session_info.aid.data_len);
  if(params->session_info.aid.data_len > 0)
  {
    size += params->session_info.aid.data_len;
    if(size > QMI_UIM_SESSION_INFORMATION_MAX_TLV_SIZE)
    {
      return QMI_INTERNAL_ERR;
    }
    memcpy (param_ptr, (char *)params->session_info.aid.data_ptr,
            params->session_info.aid.data_len);
    param_ptr += params->session_info.aid.data_len;
  }
  if (qmi_util_write_std_tlv (msg,
                              msg_size,
                              QMI_UIM_MANDATORY_1_TLV_ID,
                              size,
                              (void *)param_buf) < 0)
  {
    return QMI_INTERNAL_ERR;
  }

  /* Next is the pin protection TLV */
  WRITE_8_BIT_VAL (param_ptr, params->pin_id);
  WRITE_8_BIT_VAL (param_ptr, params->pin_operation);
  WRITE_8_BIT_VAL (param_ptr, params->pin_data.data_len);
  if(params->pin_data.data_len <= QMI_UIM_MAX_PIN_LEN)
  {
    memcpy (param_ptr, (char *)params->pin_data.data_ptr,
            params->pin_data.data_len);
  }
  else
  {
    return QMI_INTERNAL_ERR;
  }
  if (qmi_util_write_std_tlv (msg,
                              msg_size,
                              QMI_UIM_MANDATORY_2_TLV_ID,
                              QMI_UIM_SET_PIN_PROTECTION_MIN_TLV_SIZE + (unsigned long)params->pin_data.data_len,
                              (void *)(param_buf + size)) < 0)
  {
    return QMI_INTERNAL_ERR;
  }

  return QMI_NO_ERR;
} /* qmi_uim_write_pin_protection_tlv */


/*===========================================================================
  FUNCTION  qmi_uim_write_verify_pin_tlv
===========================================================================*/
/*!
@brief
  Takes the input verify pin parameter data, and writes it in TLV form
  to the passed buffer.  Buffer pointers and length indicators are adjusted
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
static int qmi_uim_write_verify_pin_tlv
(
  unsigned char                             ** msg,
  int                                       *  msg_size,
  const qmi_uim_verify_pin_params_type      *  params
)
{
  unsigned char     param_buf [QMI_UIM_VERIFY_PIN_ALL_TLV_SIZE];
  unsigned char   * param_ptr = param_buf;
  unsigned long     size = QMI_UIM_SESSION_INFORMATION_MIN_TLV_SIZE;
  unsigned long     offset = 0;

  /* First write Session Info TLV */
  WRITE_8_BIT_VAL (param_ptr, params->session_info.session_type);
  WRITE_8_BIT_VAL (param_ptr, params->session_info.aid.data_len);
  if(params->session_info.aid.data_len > 0)
  {
    size += params->session_info.aid.data_len;
    if(size > QMI_UIM_SESSION_INFORMATION_MAX_TLV_SIZE)
    {
      return QMI_INTERNAL_ERR;
    }
    memcpy (param_ptr, (char *)params->session_info.aid.data_ptr,
            params->session_info.aid.data_len);
    param_ptr += params->session_info.aid.data_len;
  }
  if (qmi_util_write_std_tlv (msg,
                              msg_size,
                              QMI_UIM_MANDATORY_1_TLV_ID,
                              size,
                              (void *)param_buf) < 0)
  {
    return QMI_INTERNAL_ERR;
  }

  /* Next is the verify pin TLV */
  offset += size;
  WRITE_8_BIT_VAL (param_ptr, params->pin_id);
  if(params->pin_data.data_len <= QMI_UIM_MAX_PIN_LEN)
  {
    if(!params->is_pin1_encrypted)
    {
      WRITE_8_BIT_VAL (param_ptr, params->pin_data.data_len);
      memcpy (param_ptr, (char *)params->pin_data.data_ptr,
              params->pin_data.data_len);
      param_ptr += params->pin_data.data_len;
      size = QMI_UIM_VERIFY_PIN_MIN_TLV_SIZE + (unsigned long)params->pin_data.data_len;
    }
    else
    {
      /* If PIN1 value is encrypted, send the value in optional TLV */
      WRITE_8_BIT_VAL (param_ptr, QMI_UIM_MIN_PIN_LEN);
      memcpy (param_ptr, "0000", QMI_UIM_MIN_PIN_LEN);
      param_ptr += QMI_UIM_MIN_PIN_LEN;
      size = QMI_UIM_VERIFY_PIN_MIN_TLV_SIZE + QMI_UIM_MIN_PIN_LEN;
    }
  }
  else
  {
    return QMI_INTERNAL_ERR;
  }
  if (qmi_util_write_std_tlv (msg,
                              msg_size,
                              QMI_UIM_MANDATORY_2_TLV_ID,
                              size,
                              (void *)(param_buf + offset)) < 0)
  {
    return QMI_INTERNAL_ERR;
  }

  /* Populate the optional TLV with encrypted value if PIN1 is encrypted */
  if(params->is_pin1_encrypted)
  {
    offset += size;
    size = QMI_UIM_ENCRYPTED_PIN_MIN_TLV_SIZE + (unsigned long)params->pin_data.data_len;
    WRITE_8_BIT_VAL (param_ptr, params->pin_data.data_len);;

    memcpy (param_ptr, (char *)params->pin_data.data_ptr,
            params->pin_data.data_len);

    if (size + offset >= (unsigned long)QMI_UIM_VERIFY_PIN_ALL_TLV_SIZE)
    {
      return QMI_INTERNAL_ERR;
    }

    if (qmi_util_write_std_tlv (msg,
                                msg_size,
                                QMI_UIM_OPTIONAL_1_TLV_ID,
                                size,
                                (void *)(param_buf + offset)) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }

  return QMI_NO_ERR;
} /* qmi_uim_write_verify_pin_tlv */


/*===========================================================================
  FUNCTION  qmi_uim_write_unblock_pin_tlv
===========================================================================*/
/*!
@brief
  Takes the input unblock pin parameter data, and writes it in TLV form
  to the passed buffer.  Buffer pointers and length indicators are adjusted
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
static int qmi_uim_write_unblock_pin_tlv
(
  unsigned char                             ** msg,
  int                                       *  msg_size,
  const qmi_uim_unblock_pin_params_type     *  params
)
{
  unsigned char     param_buf [QMI_UIM_UNBLOCK_PIN_ALL_TLV_SIZE];
  unsigned char   * param_ptr = param_buf;
  unsigned long     size = QMI_UIM_SESSION_INFORMATION_MIN_TLV_SIZE;
  unsigned long     offset = 0;

  /* First write Session Info TLV */
  WRITE_8_BIT_VAL (param_ptr, params->session_info.session_type);
  WRITE_8_BIT_VAL (param_ptr, params->session_info.aid.data_len);
  if(params->session_info.aid.data_len > 0)
  {
    size += params->session_info.aid.data_len;
    if(size > QMI_UIM_SESSION_INFORMATION_MAX_TLV_SIZE)
    {
      return QMI_INTERNAL_ERR;
    }
    memcpy (param_ptr, (char *)params->session_info.aid.data_ptr,
            params->session_info.aid.data_len);
    param_ptr += params->session_info.aid.data_len;
  }
  if (qmi_util_write_std_tlv (msg,
                              msg_size,
                              QMI_UIM_MANDATORY_1_TLV_ID,
                              size,
                              (void *)param_buf) < 0)
  {
    return QMI_INTERNAL_ERR;
  }

  offset = size;
  size = QMI_UIM_UNBLOCK_PIN_MIN_TLV_SIZE;

  /* Next is the Unblock pin TLV */
  WRITE_8_BIT_VAL (param_ptr, params->pin_id);
  WRITE_8_BIT_VAL (param_ptr, params->puk_data.data_len);
  if(params->puk_data.data_len <= QMI_UIM_MAX_PIN_LEN)
  {
    memcpy (param_ptr, (char *)params->puk_data.data_ptr,
            params->puk_data.data_len);
    param_ptr += params->puk_data.data_len;
    size += params->puk_data.data_len;
  }
  else
  {
    return QMI_INTERNAL_ERR;
  }

  WRITE_8_BIT_VAL (param_ptr, params->new_pin_data.data_len);
  if(params->new_pin_data.data_len <= QMI_UIM_MAX_PIN_LEN)
  {
    memcpy (param_ptr, (char *)params->new_pin_data.data_ptr,
            params->new_pin_data.data_len);
    size += params->new_pin_data.data_len;
  }
  else
  {
    return QMI_INTERNAL_ERR;
  }

  if (qmi_util_write_std_tlv (msg,
                              msg_size,
                              QMI_UIM_MANDATORY_2_TLV_ID,
                              size,
                              (void *)(param_buf + offset)) < 0)
  {
    return QMI_INTERNAL_ERR;
  }

  return QMI_NO_ERR;
} /* qmi_uim_write_unblock_pin_tlv */


/*===========================================================================
  FUNCTION  qmi_uim_write_change_pin_tlv
===========================================================================*/
/*!
@brief
  Takes the input change pin parameter data, and writes it in TLV form
  to the passed buffer.  Buffer pointers and length indicators are adjusted
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
static int qmi_uim_write_change_pin_tlv
(
  unsigned char                             ** msg,
  int                                       *  msg_size,
  const qmi_uim_change_pin_params_type      *  params
)
{
  unsigned char     param_buf [QMI_UIM_CHANGE_PIN_ALL_TLV_SIZE];
  unsigned char   * param_ptr = param_buf;
  unsigned long     size = QMI_UIM_SESSION_INFORMATION_MIN_TLV_SIZE;
  unsigned long     offset = 0;

  /* First write Session Info TLV */
  WRITE_8_BIT_VAL (param_ptr, params->session_info.session_type);
  WRITE_8_BIT_VAL (param_ptr, params->session_info.aid.data_len);

  if(params->session_info.aid.data_len > 0)
  {
    size += params->session_info.aid.data_len;
    if(size > QMI_UIM_SESSION_INFORMATION_MAX_TLV_SIZE)
    {
      return QMI_INTERNAL_ERR;
    }
    memcpy (param_ptr, (char *)params->session_info.aid.data_ptr,
            params->session_info.aid.data_len);
    param_ptr += params->session_info.aid.data_len;
  }
  if (qmi_util_write_std_tlv (msg,
                              msg_size,
                              QMI_UIM_MANDATORY_1_TLV_ID,
                              size,
                              (void *)param_buf) < 0)
  {
    return QMI_INTERNAL_ERR;
  }

  offset = size;
  size = QMI_UIM_CHANGE_PIN_MIN_TLV_SIZE;

  /* Next is the Change pin TLV */
  WRITE_8_BIT_VAL (param_ptr, params->pin_id);
  WRITE_8_BIT_VAL (param_ptr, params->old_pin_data.data_len);
  if(params->old_pin_data.data_len <= QMI_UIM_MAX_PIN_LEN)
  {
    memcpy (param_ptr, (char *)params->old_pin_data.data_ptr,
            params->old_pin_data.data_len);
    param_ptr += params->old_pin_data.data_len;
    size += params->old_pin_data.data_len;
  }
  else
  {
    return QMI_INTERNAL_ERR;
  }

  WRITE_8_BIT_VAL (param_ptr, params->new_pin_data.data_len);
  if(params->new_pin_data.data_len <= QMI_UIM_MAX_PIN_LEN)
  {
    memcpy (param_ptr, (char *)params->new_pin_data.data_ptr,
            params->new_pin_data.data_len);
    size += params->new_pin_data.data_len;
  }
  else
  {
    return QMI_INTERNAL_ERR;
  }

  if (qmi_util_write_std_tlv (msg,
                              msg_size,
                              QMI_UIM_MANDATORY_2_TLV_ID,
                              size,
                              (void *)(param_buf + offset)) < 0)
  {
    return QMI_INTERNAL_ERR;
  }

  return QMI_NO_ERR;
} /* qmi_uim_write_change_pin_tlv */


/*===========================================================================
  FUNCTION  qmi_uim_write_depersonalization_tlv
===========================================================================*/
/*!
@brief
  Takes the input depersonalization parameter data, and writes it in TLV form
  to the passed buffer.  Buffer pointers and length indicators are adjusted
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
static int qmi_uim_write_depersonalization_tlv
(
  unsigned char                                   ** msg,
  int                                             *  msg_size,
  const qmi_uim_depersonalization_params_type     *  params
)
{
  unsigned char     param_buf [QMI_UIM_DEPERSONALIZATION_ALL_TLV_SIZE];
  unsigned char   * param_ptr = param_buf;
  unsigned long     size = QMI_UIM_DEPERSONALIZATION_MIN_TLV_SIZE;
  unsigned short    temp_len = 0;

  /* Write Deperso TLV */
  WRITE_8_BIT_VAL (param_ptr, params->perso_feature);
  WRITE_8_BIT_VAL (param_ptr, params->perso_operation);
  temp_len = params->ck_data.data_len;
  WRITE_8_BIT_VAL (param_ptr, temp_len);
  if(temp_len > 0)
  {
    memcpy (param_ptr, (char *)params->ck_data.data_ptr, temp_len);
    size += temp_len;
  }
  if (qmi_util_write_std_tlv (msg,
                              msg_size,
                              QMI_UIM_MANDATORY_1_TLV_ID,
                              size,
                              (void *)param_buf) < 0)
  {
    return QMI_INTERNAL_ERR;
  }
  return QMI_NO_ERR;
} /* qmi_uim_write_depersonalization_tlv */


/*===========================================================================
  FUNCTION  qmi_uim_write_authenticate_tlv
===========================================================================*/
/*!
@brief
  Takes the input authenticate parameter data, and writes it in TLV form
  to the passed buffer.  Buffer pointers and length indicators are adjusted
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
static int qmi_uim_write_authenticate_tlv
(
  unsigned char                                 ** msg,
  int                                           *  msg_size,
  const qmi_uim_authenticate_params_type        *  params
)
{
  unsigned char     param_buf [QMI_SRVC_PDU_SIZE(QMI_UIM_STD_MSG_SIZE)];
  unsigned char   * param_ptr  = param_buf;
  unsigned long     size       = QMI_UIM_SESSION_INFORMATION_MIN_TLV_SIZE;
  unsigned long     offset     = 0;

  /* First write Session info TLV (mandatory) */
  WRITE_8_BIT_VAL (param_ptr, params->session_info.session_type);
  WRITE_8_BIT_VAL (param_ptr, params->session_info.aid.data_len);
  if(params->session_info.aid.data_len > 0)
  {
    size += params->session_info.aid.data_len;
    if(size > QMI_UIM_SESSION_INFORMATION_MAX_TLV_SIZE)
    {
      return QMI_INTERNAL_ERR;
    }
    memcpy (param_ptr, (char *)params->session_info.aid.data_ptr,
            params->session_info.aid.data_len);
    param_ptr += params->session_info.aid.data_len;
  }
  if (qmi_util_write_std_tlv (msg,
                              msg_size,
                              QMI_UIM_MANDATORY_1_TLV_ID,
                              size,
                              (void *)param_buf) < 0)
  {
    return QMI_INTERNAL_ERR;
  }

  /* Next is Authentication data TLV (mandatory) */
  offset = size;
  size = QMI_UIM_AUTH_DATA_MIN_TLV_SIZE;
  WRITE_8_BIT_VAL (param_ptr, params->auth_context);
  WRITE_16_BIT_VAL (param_ptr, params->auth_data.data_len);
  if(params->auth_data.data_len > 0)
  {
    memcpy (param_ptr, (char *)params->auth_data.data_ptr, params->auth_data.data_len);
    size += params->auth_data.data_len;
  }
  if(size >= QMI_SRVC_PDU_SIZE(QMI_UIM_STD_MSG_SIZE) -
             QMI_UIM_SESSION_INFORMATION_MAX_TLV_SIZE)
  {
    return QMI_INTERNAL_ERR;
  }
  if (qmi_util_write_std_tlv (msg,
                              msg_size,
                              QMI_UIM_MANDATORY_2_TLV_ID,
                              size,
                              (void *)(param_buf + offset)) < 0)
  {
    return QMI_INTERNAL_ERR;
  }
  return QMI_NO_ERR;
} /* qmi_uim_write_authenticate_tlv */


/*===========================================================================
  FUNCTION  qmi_uim_write_get_service_status_tlv
===========================================================================*/
/*!
@brief
  Takes the input get service status parameter data, and writes it in TLV form
  to the passed buffer.  Buffer pointers and length indicators are adjusted
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
static int qmi_uim_write_get_service_status_tlv
(
  unsigned char                                 ** msg,
  int                                           *  msg_size,
  const qmi_uim_get_service_status_params_type  *  params
)
{
  unsigned char     param_buf [QMI_UIM_GET_SERVICE_STATUS_ALL_TLV_SIZE];
  unsigned char   * param_ptr = param_buf;
  unsigned long     size = QMI_UIM_SESSION_INFORMATION_MIN_TLV_SIZE;
  unsigned short    temp_len = 0;

  /* First write Session Info TLV */
  WRITE_8_BIT_VAL (param_ptr, params->session_info.session_type);
  temp_len = params->session_info.aid.data_len;
  WRITE_8_BIT_VAL (param_ptr, temp_len);
  if(temp_len > 0)
  {
    size += temp_len;
    if(size > QMI_UIM_SESSION_INFORMATION_MAX_TLV_SIZE)
    {
      return QMI_INTERNAL_ERR;
    }
    memcpy (param_ptr, (char *)params->session_info.aid.data_ptr, temp_len);
    param_ptr += temp_len;
  }
  if (qmi_util_write_std_tlv (msg,
                              msg_size,
                              QMI_UIM_MANDATORY_1_TLV_ID,
                              size,
                              (void *)param_buf) < 0)
  {
    return QMI_INTERNAL_ERR;
  }

  /* Next is the Capabilities mask TLV */
  WRITE_32_BIT_VAL(param_ptr, params->mask);

  if (qmi_util_write_std_tlv (msg,
                              msg_size,
                              QMI_UIM_MANDATORY_2_TLV_ID,
                              QMI_UIM_CAPABILITIES_MASK_MAX_TLV_SIZE,
                              (void *) (param_buf + size)) < 0)
  {
    return QMI_INTERNAL_ERR;
  }

  return QMI_NO_ERR;
} /* qmi_uim_write_get_service_status_tlv */


/*===========================================================================
  FUNCTION  qmi_uim_write_set_service_status_tlv
===========================================================================*/
/*!
@brief
  Takes the input set service status parameter data, and writes it in TLV form
  to the passed buffer.  Buffer pointers and length indicators are adjusted
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
static int qmi_uim_write_set_service_status_tlv
(
  unsigned char                                 ** msg,
  int                                           *  msg_size,
  const qmi_uim_set_service_status_params_type  *  params
)
{
  unsigned char     param_buf [QMI_UIM_SET_SERVICE_STATUS_ALL_TLV_SIZE];
  unsigned char   * param_ptr = param_buf;
  unsigned long     size = QMI_UIM_SESSION_INFORMATION_MIN_TLV_SIZE;
  unsigned short    temp_len = 0;

  /* First write Session Info TLV */
  WRITE_8_BIT_VAL (param_ptr, params->session_info.session_type);
  temp_len = params->session_info.aid.data_len;
  WRITE_8_BIT_VAL (param_ptr, temp_len);
  if(temp_len > 0)
  {
    memcpy (param_ptr, (char *)params->session_info.aid.data_ptr, temp_len);
    param_ptr += temp_len;
    size += temp_len;
  }
  if(size >= QMI_UIM_SET_SERVICE_STATUS_ALL_TLV_SIZE)
  {
    return QMI_INTERNAL_ERR;
  }
  if (qmi_util_write_std_tlv (msg,
                              msg_size,
                              QMI_UIM_MANDATORY_1_TLV_ID,
                              size,
                              (void *)param_buf) < 0)
  {
    return QMI_INTERNAL_ERR;
  }

  /* Next is the FDN status TLV */
  WRITE_8_BIT_VAL(param_ptr, params->fdn_status);

  if (qmi_util_write_std_tlv (msg,
                              msg_size,
                              QMI_UIM_OPTIONAL_1_TLV_ID,
                              QMI_UIM_SERVICE_STATUS_MAX_TLV_SIZE,
                              (void *) (param_buf + size)) < 0)
  {
    return QMI_INTERNAL_ERR;
  }

  return QMI_NO_ERR;
} /* qmi_uim_write_set_service_status_tlv */


/*===========================================================================
  FUNCTION  qmi_uim_write_change_prov_session_tlv
===========================================================================*/
/*!
@brief
  Takes the input change provisioning session parameter data, and writes it
  in TLV form to the passed buffer. Buffer pointers and length indicators
  are adjusted to reflect new TLV.

@return
  QMI_INTERNAL_ERR if an error occurred, QMI_NO_ERR if not

@note

  - Dependencies
    - None.

  - Side Effects
    - Writes TLV to input buffer and updates pointers and byte count
*/
/*=========================================================================*/
static int qmi_uim_write_change_prov_session_tlv
(
  unsigned char                                     ** msg,
  int                                               *  msg_size,
  const qmi_uim_change_prov_session_params_type     *  params
)
{
  unsigned char     param_buf [QMI_UIM_CHANGE_PROV_SESSION_ALL_TLV_SIZE];
  unsigned char   * param_ptr = param_buf;
  unsigned long     size = QMI_UIM_APP_INFORMATION_MIN_TLV_SIZE;

  /* First write Session Change TLV */
  WRITE_8_BIT_VAL (param_ptr, params->session_type);
  WRITE_8_BIT_VAL (param_ptr, params->activation_type);
  if (qmi_util_write_std_tlv (msg,
                              msg_size,
                              QMI_UIM_MANDATORY_1_TLV_ID,
                              QMI_UIM_SESSION_CHANGE_MAX_TLV_SIZE,
                              (void *)param_buf) < 0)
  {
    return QMI_INTERNAL_ERR;
  }

  /* Application Info TLV is needed only for activate request */
  if (params->activation_type == QMI_UIM_SESSION_ACTIVATE)
  {
    WRITE_8_BIT_VAL (param_ptr, params->app_info.slot);
    WRITE_8_BIT_VAL (param_ptr, params->app_info.aid.data_len);
    if(params->app_info.aid.data_len > 0)
    {
      memcpy (param_ptr, (char *)params->app_info.aid.data_ptr,
              params->app_info.aid.data_len);
      size += params->app_info.aid.data_len;
    }
    if (qmi_util_write_std_tlv (msg,
                                msg_size,
                                QMI_UIM_OPTIONAL_1_TLV_ID,
                                size,
                                (void *) (param_buf+QMI_UIM_SESSION_CHANGE_MAX_TLV_SIZE)) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }

  return QMI_NO_ERR;
} /* qmi_uim_write_change_prov_session_tlv */


/*===========================================================================
  FUNCTION  qmi_uim_write_get_label_tlv
===========================================================================*/
/*!
@brief
  Takes the input get label parameter data, and writes it in TLV form
  to the passed buffer.  Buffer pointers and length indicators are adjusted
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
static int qmi_uim_write_get_label_tlv
(
  unsigned char                                 ** msg,
  int                                           *  msg_size,
  const qmi_uim_get_label_params_type           *  params
)
{
  unsigned char     param_buf [QMI_UIM_GET_LABEL_ALL_TLV_SIZE];
  unsigned char   * param_ptr = param_buf;
  unsigned long     size = QMI_UIM_APP_INFORMATION_MIN_TLV_SIZE;

  /* Write Application Info TLV */
  WRITE_8_BIT_VAL (param_ptr, params->app_info.slot);
  WRITE_8_BIT_VAL (param_ptr, params->app_info.aid.data_len);
  if(params->app_info.aid.data_len > 0)
  {
    memcpy (param_ptr, (char *)params->app_info.aid.data_ptr,
            params->app_info.aid.data_len);
    param_ptr += params->app_info.aid.data_len;
    size += params->app_info.aid.data_len;
  }
  if (qmi_util_write_std_tlv (msg,
                              msg_size,
                              QMI_UIM_MANDATORY_1_TLV_ID,
                              size,
                              (void *)param_buf) < 0)
  {
    return QMI_INTERNAL_ERR;
  }
  return QMI_NO_ERR;
} /* qmi_uim_write_get_label_tlv */


/*===========================================================================
  FUNCTION  qmi_uim_write_close_session_tlv
===========================================================================*/
/*!
@brief
  Takes the input close session parameter data, and writes it in TLV form
  to the passed buffer.  Buffer pointers and length indicators are adjusted
  to reflect new TLV.

@return
  QMI_INTERNAL_ERR if an error occurred, QMI_NO_ERR if not

@note

  - Dependencies
    - None.

  - Side Effects
    - Writes TLV to input buffer and updates pointers and byte count
*/
/*=========================================================================*/
static int qmi_uim_write_close_session_tlv
(
  unsigned char                                 ** msg,
  int                                           *  msg_size,
  const qmi_uim_close_session_params_type       *  params
)
{
  unsigned char     param_buf [QMI_UIM_CLOSE_SESSION_ALL_TLV_SIZE];
  unsigned char   * param_ptr = param_buf;
  unsigned long     size = QMI_UIM_SESSION_INFORMATION_MIN_TLV_SIZE;

  /* Write Session Info TLV */
  WRITE_8_BIT_VAL (param_ptr, params->session_info.session_type);
  WRITE_8_BIT_VAL (param_ptr, params->session_info.aid.data_len);
  if ((params->session_info.aid.data_len > 0) &&
      (params->session_info.aid.data_len <= (QMI_UIM_CLOSE_SESSION_ALL_TLV_SIZE -
                                             QMI_UIM_SESSION_INFORMATION_MIN_TLV_SIZE)))
  {
    memcpy (param_ptr, (char *)params->session_info.aid.data_ptr,
            params->session_info.aid.data_len);
    size += params->session_info.aid.data_len;
  }

  if(size > QMI_UIM_CLOSE_SESSION_ALL_TLV_SIZE)
  {
    return QMI_INTERNAL_ERR;
  }

  if (qmi_util_write_std_tlv (msg,
                              msg_size,
                              QMI_UIM_MANDATORY_1_TLV_ID,
                              size,
                              (void *)param_buf) < 0)
  {
    return QMI_INTERNAL_ERR;
  }
  return QMI_NO_ERR;
} /* qmi_uim_write_close_session_tlv */


/*===========================================================================
  FUNCTION  qmi_uim_write_send_apdu_tlv
===========================================================================*/
/*!
@brief
  Takes the input send apdu parameter data, and writes it in TLV form to
  the passed buffer. Buffer pointers and length indicators are adjusted
  to reflect new TLV.

@return
  QMI_INTERNAL_ERR if an error occurred, QMI_NO_ERR if not

@note

  - Dependencies
    - None.

  - Side Effects
    - Writes TLV to input buffer and updates pointers and byte count
*/
/*=========================================================================*/
static int qmi_uim_write_send_apdu_tlv
(
  unsigned char                                 ** msg,
  int                                           *  msg_size,
  const qmi_uim_send_apdu_params_type           *  params
)
{
  unsigned char     param_buf [QMI_SRVC_PDU_SIZE(QMI_UIM_STD_MSG_SIZE)];
  unsigned char   * param_ptr = param_buf;
  unsigned long     size = QMI_UIM_APDU_LENGTH_MIN_TLV_SIZE;
  unsigned long     offset = 0;

  /* First is the Slot TLV */
  WRITE_8_BIT_VAL (param_ptr, params->slot);
  if (qmi_util_write_std_tlv (msg,
                              msg_size,
                              QMI_UIM_MANDATORY_1_TLV_ID,
                              QMI_UIM_SLOT_MAX_TLV_SIZE,
                              (void *)param_buf) < 0)
  {
    return QMI_INTERNAL_ERR;
  }

  /* Next is APDU TLV */
  offset = QMI_UIM_SLOT_MAX_TLV_SIZE;
  WRITE_16_BIT_VAL (param_ptr, params->apdu.data_len);
  if(params->apdu.data_len > 0)
  {
    size += params->apdu.data_len;
    if(size > QMI_SRVC_PDU_SIZE(QMI_UIM_STD_MSG_SIZE) - QMI_UIM_SLOT_MAX_TLV_SIZE)
    {
      return QMI_INTERNAL_ERR;
    }
    memcpy (param_ptr,
            (char *)params->apdu.data_ptr,
            params->apdu.data_len);
    param_ptr += params->apdu.data_len;
  }

  if (qmi_util_write_std_tlv (msg,
                              msg_size,
                              QMI_UIM_MANDATORY_2_TLV_ID,
                              size,
                              (void *) (param_buf + offset)) < 0)
  {
    return QMI_INTERNAL_ERR;
  }

  /* Write the optional channel id TLV if present */
  offset += size;
  if (params->channel_id_present)
  {
    if(offset >= QMI_SRVC_PDU_SIZE(QMI_UIM_STD_MSG_SIZE))
    {
      return QMI_INTERNAL_ERR;
    }
    WRITE_8_BIT_VAL (param_ptr, params->channel_id);
    if (qmi_util_write_std_tlv (msg,
                                msg_size,
                                QMI_UIM_OPTIONAL_1_TLV_ID,
                                QMI_UIM_CHANNEL_ID_MAX_TLV_SIZE,
                                (void *) (param_buf + offset)) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }

  return QMI_NO_ERR;
} /* qmi_uim_write_send_apdu_tlv */


/*===========================================================================
  FUNCTION  qmi_uim_write_sap_connection_tlv
===========================================================================*/
/*!
@brief
  Takes the input SAP connection parameter data, and writes it in TLV form
  to the passed buffer. Buffer pointers and length indicators are adjusted
  to reflect new TLV.

@return
  QMI_INTERNAL_ERR if an error occurred, QMI_NO_ERR if not

@note

  - Dependencies
    - None.

  - Side Effects
    - Writes TLV to input buffer and updates pointers and byte count
*/
/*=========================================================================*/
static int qmi_uim_write_sap_connection_tlv
(
  unsigned char                                 ** msg,
  int                                           *  msg_size,
  const qmi_uim_sap_connection_params_type      *  params
)
{
  unsigned char     param_buf [QMI_UIM_SAP_CONNECTION_ALL_TLV_SIZE];
  unsigned char   * param_ptr = param_buf;

  /* First is the Connect TLV */
  WRITE_8_BIT_VAL (param_ptr, params->operation_type);
  WRITE_8_BIT_VAL (param_ptr, params->slot);
  if (qmi_util_write_std_tlv (msg,
                              msg_size,
                              QMI_UIM_MANDATORY_1_TLV_ID,
                              QMI_UIM_SAP_CONNECT_MAX_TLV_SIZE,
                              (void *)param_buf) < 0)
  {
    return QMI_INTERNAL_ERR;
  }

  /* If operation type is disconnect, add disconnect mode TLV */
  if (params->operation_type == QMI_UIM_SAP_CONNECTION_DISCONNECT)
  {
    /* Write Open logical channel TLV */
    WRITE_8_BIT_VAL (param_ptr, params->disconnect_mode);
    if (qmi_util_write_std_tlv (msg,
                                msg_size,
                                QMI_UIM_OPTIONAL_1_TLV_ID,
                                QMI_UIM_SAP_DISCONNECT_MODE_MAX_TLV_SIZE,
                                (void *) (param_buf + QMI_UIM_SAP_CONNECT_MAX_TLV_SIZE)) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }

  return QMI_NO_ERR;
} /* qmi_uim_write_sap_connection_tlv */


/*===========================================================================
  FUNCTION  qmi_uim_write_sap_request_tlv
===========================================================================*/
/*!
@brief
  Takes the input SAP resuest parameter data, and writes it in TLV form
  to the passed buffer. Buffer pointers and length indicators are adjusted
  to reflect new TLV.

@return
  QMI_INTERNAL_ERR if an error occurred, QMI_NO_ERR if not

@note

  - Dependencies
    - None.

  - Side Effects
    - Writes TLV to input buffer and updates pointers and byte count
*/
/*=========================================================================*/
static int qmi_uim_write_sap_request_tlv
(
  unsigned char                                 ** msg,
  int                                           *  msg_size,
  const qmi_uim_sap_request_params_type         *  params
)
{
  unsigned char     param_buf [QMI_SRVC_PDU_SIZE(QMI_UIM_STD_MSG_SIZE)];
  unsigned char   * param_ptr = param_buf;
  unsigned long     size = QMI_UIM_APDU_LENGTH_MIN_TLV_SIZE;

  /* First is the SAP request TLV */
  WRITE_8_BIT_VAL (param_ptr, params->request_type);
  WRITE_8_BIT_VAL (param_ptr, params->slot);
  if (qmi_util_write_std_tlv (msg,
                              msg_size,
                              QMI_UIM_MANDATORY_1_TLV_ID,
                              QMI_UIM_SAP_REQUEST_MAX_TLV_SIZE,
                              (void *)param_buf) < 0)
  {
    return QMI_INTERNAL_ERR;
  }

  /* If operation type is send_apdu, add APDU TLV */
  if (params->request_type == QMI_UIM_SAP_REQUEST_OP_SEND_APDU)
  {
    WRITE_16_BIT_VAL (param_ptr, params->apdu.data_len);
    if(params->apdu.data_len > 0)
    {
      memcpy (param_ptr,
              (char *)params->apdu.data_ptr,
              params->apdu.data_len);
      size += params->apdu.data_len;
    }

    if(size >= (QMI_SRVC_PDU_SIZE(QMI_UIM_STD_MSG_SIZE) - QMI_UIM_SAP_REQUEST_MAX_TLV_SIZE))
    {
      return QMI_INTERNAL_ERR;
    }

    if (qmi_util_write_std_tlv (msg,
                                msg_size,
                                QMI_UIM_OPTIONAL_1_TLV_ID,
                                size,
                                (void *) (param_buf + QMI_UIM_SAP_REQUEST_MAX_TLV_SIZE)) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }

  return QMI_NO_ERR;
} /* qmi_uim_write_sap_request_tlv */


/*===========================================================================
  FUNCTION  qmi_uim_write_logical_channel_tlv
===========================================================================*/
/*!
@brief
  Takes the input logical channel parameter data, and writes it in TLV form
  to the passed buffer. Buffer pointers and length indicators are adjusted
  to reflect new TLV.

@return
  QMI_INTERNAL_ERR if an error occurred, QMI_NO_ERR if not

@note

  - Dependencies
    - None.

  - Side Effects
    - Writes TLV to input buffer and updates pointers and byte count
*/
/*=========================================================================*/
static int qmi_uim_write_logical_channel_tlv
(
  unsigned char                                 ** msg,
  int                                           *  msg_size,
  const qmi_uim_logical_channel_params_type     *  params
)
{
  unsigned char     param_buf [QMI_UIM_LOGICAL_CHANNEL_ALL_TLV_SIZE];
  unsigned char   * param_ptr = param_buf;
  unsigned long     size = QMI_UIM_LOGICAL_CHANNEL_OPEN_MIN_TLV_SIZE;

  /* First is the Slot TLV */
  WRITE_8_BIT_VAL (param_ptr, params->slot);
  if (qmi_util_write_std_tlv (msg,
                              msg_size,
                              QMI_UIM_MANDATORY_1_TLV_ID,
                              QMI_UIM_SLOT_MAX_TLV_SIZE,
                              (void *)param_buf) < 0)
  {
    return QMI_INTERNAL_ERR;
  }

  /* Depending upon the request, it is either -
     Open channel or close channel request */
  if (params->operation_type == QMI_UIM_LOGICAL_CHANNEL_OPEN)
  {
    /* Write Open logical channel TLV */
    WRITE_8_BIT_VAL (param_ptr, params->channel_data.aid.data_len);
    if(params->channel_data.aid.data_len > 0)
    {
      memcpy (param_ptr,
              (char *)params->channel_data.aid.data_ptr,
              params->channel_data.aid.data_len);
      size += params->channel_data.aid.data_len;
    }
    if (qmi_util_write_std_tlv (msg,
                                msg_size,
                                QMI_UIM_OPTIONAL_1_TLV_ID,
                                size,
                                (void *) (param_buf + QMI_UIM_SLOT_MAX_TLV_SIZE)) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }
  else if (params->operation_type == QMI_UIM_LOGICAL_CHANNEL_CLOSE)
  {
    /* Write Close logical channel TLV */
    WRITE_8_BIT_VAL (param_ptr, params->channel_data.channel_id);
    if (qmi_util_write_std_tlv (msg,
                                msg_size,
                                QMI_UIM_OPTIONAL_2_TLV_ID,
                                QMI_UIM_LOGICAL_CHANNEL_CLOSE_MAX_TLV_SIZE,
                                (void *) (param_buf + QMI_UIM_SLOT_MAX_TLV_SIZE)) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }

  return QMI_NO_ERR;
} /* qmi_uim_write_logical_channel_tlv */

/*===========================================================================
  FUNCTION  qmi_uim_read_read_transparent_tlv
===========================================================================*/
/*!
@brief
  Parses the raw TLV for read transparent response data, and writes
  it to the passed response pointer. Input Buffer pointer value_ptr is
  adjusted as the TLV is parsed.

@return
  None.

@note

  - Dependencies
    - None.

  - Side Effects
    - Writes TLV to input structure pointer and updates TLV buffer pointer
*/
/*=========================================================================*/
static void qmi_uim_read_read_transparent_tlv
(
  unsigned char                             * value_ptr,
  qmi_uim_read_transparent_rsp_type         * rsp
)
{
  /* We expect only 1 optional_2 TLV for read transparent, any more will be rejected */
  if (rsp->content.data_ptr)
  {
    return;
  }

  READ_16_BIT_VAL (value_ptr, rsp->content.data_len);

  if (rsp->content.data_len > 0)
  {
    rsp->content.data_ptr = (unsigned char *) malloc (rsp->content.data_len);
    if (rsp->content.data_ptr)
    {
      memcpy (rsp->content.data_ptr,
              value_ptr,
              rsp->content.data_len);
    }
  }
} /* qmi_uim_read_read_transparent_tlv */


/*===========================================================================
  FUNCTION  qmi_uim_read_read_record_tlv
===========================================================================*/
/*!
@brief
  Parses the raw TLV input for read record response data, and writes
  it to the passed response pointer. Input Buffer pointer value_ptr is
  adjusted as the TLV is parsed.

@return
  None.

@note

  - Dependencies
    - None.

  - Side Effects
    - Writes TLV to input structure pointer and updates TLV buffer pointer
*/
/*=========================================================================*/
static void qmi_uim_read_read_record_tlv
(
  unsigned char                             * value_ptr,
  qmi_uim_read_record_rsp_type              * rsp
)
{
  /* We expect only 1 optional_2 TLV for read record, any more will be rejected */
  if (rsp->content.data_ptr)
  {
    return;
  }

  READ_16_BIT_VAL (value_ptr, rsp->content.data_len);

  if (rsp->content.data_len > 0)
  {
    /* Note that in case of a Sync call, Client needs to free this up,
       and in case of an ASync call, QMI_UIM service will free it up */
    rsp->content.data_ptr = (unsigned char *) malloc (rsp->content.data_len);
    if (rsp->content.data_ptr)
    {
      memcpy (rsp->content.data_ptr,
              value_ptr,
              rsp->content.data_len);
    }
  }
} /* qmi_uim_read_read_record_tlv */


/*===========================================================================
  FUNCTION  qmi_uim_write_security_attributes
===========================================================================*/
/*!
@brief
  Writes security attributes to the passed pointer after checking with
  the respective attribute masks.

@return
  None.

@note

  - Dependencies
    - None.

  - Side Effects
    - None.
*/
/*=========================================================================*/
static void qmi_uim_write_security_attributes
(
  unsigned short                temp_val_word,
  qmi_uim_security_type       * sec_type
)
{
  /*lint --e{774} */
  if (temp_val_word & QMI_UIM_SECURITY_ATTR_MASK_PIN1)
  {
    sec_type->pin1 = QMI_UIM_TRUE;
  }
  if (temp_val_word & QMI_UIM_SECURITY_ATTR_MASK_PIN2)
  {
    sec_type->pin2 = QMI_UIM_TRUE;
  }
  if (temp_val_word & QMI_UIM_SECURITY_ATTR_MASK_UPIN)
  {
    sec_type->upin = QMI_UIM_TRUE;
  }
  if (temp_val_word & QMI_UIM_SECURITY_ATTR_MASK_ADM)
  {
    sec_type->adm = QMI_UIM_TRUE;
  }
} /* qmi_uim_write_security_attributes */


/*===========================================================================
  FUNCTION  qmi_uim_read_get_file_attributes_tlv
===========================================================================*/
/*!
@brief
  Parses the raw TLV input for get file attributes response data, and writes
  it to the passed response pointer. Input Buffer pointer value_ptr is
  adjusted as the TLV is parsed.

@return
  None.

@note

  - Dependencies
    - None.

  - Side Effects
    - Writes TLV to input structure pointer and updates TLV buffer pointer
*/
/*=========================================================================*/
static void qmi_uim_read_get_file_attributes_tlv
(
  unsigned char                             * value_ptr,
  const unsigned long                       * type,
  qmi_uim_get_file_attributes_rsp_type      * rsp
)
{
  if (*type == QMI_UIM_OPTIONAL_1_RESP_TLV_ID)
  {
    READ_8_BIT_VAL (value_ptr, rsp->sw1);
    READ_8_BIT_VAL (value_ptr, rsp->sw2);
  }
  else if (*type == QMI_UIM_OPTIONAL_2_RESP_TLV_ID)
  {
    unsigned char  temp_val = 0;
    unsigned short temp_val_word = 0;

    /* We expect only 1 optional_2 TLV for get file attr, any more will be rejected */
    if (rsp->raw_value.data_ptr)
    {
      return;
    }

    READ_16_BIT_VAL (value_ptr, rsp->file_size);
    READ_16_BIT_VAL (value_ptr, rsp->file_id);
    READ_8_BIT_VAL (value_ptr, temp_val);
    rsp->file_type = (qmi_uim_file_type)temp_val;
    READ_16_BIT_VAL (value_ptr, rsp->record_size);
    READ_16_BIT_VAL (value_ptr, rsp->record_count);

    READ_8_BIT_VAL (value_ptr, temp_val);
    rsp->read_security.security_value = (qmi_uim_security_value_type)temp_val;
    READ_16_BIT_VAL (value_ptr, temp_val_word);
    qmi_uim_write_security_attributes(temp_val_word, &rsp->read_security);

    READ_8_BIT_VAL (value_ptr, temp_val);
    rsp->write_security.security_value = (qmi_uim_security_value_type)temp_val;
    READ_16_BIT_VAL (value_ptr, temp_val_word);
    qmi_uim_write_security_attributes(temp_val_word, &rsp->write_security);

    READ_8_BIT_VAL (value_ptr, temp_val);
    rsp->increase_security.security_value = (qmi_uim_security_value_type)temp_val;
    READ_16_BIT_VAL (value_ptr, temp_val_word);
    qmi_uim_write_security_attributes(temp_val_word, &rsp->increase_security);

    READ_8_BIT_VAL (value_ptr, temp_val);
    rsp->deactivate_security.security_value = (qmi_uim_security_value_type)temp_val;
    READ_16_BIT_VAL (value_ptr, temp_val_word);
    qmi_uim_write_security_attributes(temp_val_word, &rsp->deactivate_security);

    READ_8_BIT_VAL (value_ptr, temp_val);
    rsp->activate_security.security_value = (qmi_uim_security_value_type)temp_val;
    READ_16_BIT_VAL (value_ptr, temp_val_word);
    qmi_uim_write_security_attributes(temp_val_word, &rsp->activate_security);

    READ_16_BIT_VAL (value_ptr, rsp->raw_value.data_len);

    if (rsp->raw_value.data_len > 0)
    {
      rsp->raw_value.data_ptr = (unsigned char *) malloc (rsp->raw_value.data_len);
      /* Note that in case of a Sync call, Client needs to free this up,
       * and in case of an ASync call, QMI_UIM service will free it up */
      if (rsp->raw_value.data_ptr)
      {
        memcpy (rsp->raw_value.data_ptr,
                value_ptr,
                rsp->raw_value.data_len);
      }
    }
  }
} /* qmi_uim_read_get_file_attributes_tlv */


/*===========================================================================
  FUNCTION  qmi_uim_read_refresh_event_tlv
===========================================================================*/
/*!
@brief
  Parses the raw TLV input for refresh event data, and writes it to the
  passed response pointer. Input Buffer pointer value_ptr is adjusted as
  the TLV is parsed.

@return
  None.

@note

  - Dependencies
    - None.

  - Side Effects
    - Writes TLV to input structure pointer and updates TLV buffer pointer
*/
/*=========================================================================*/
static void qmi_uim_read_refresh_event_tlv
(
  unsigned char                             * value_ptr,
  qmi_uim_refresh_event_type                * rsp
)
{
  unsigned char     temp_val = 0;
  int               i = 0, j = 0;

  /* Process the TLV */
  memset (rsp, 0, sizeof(qmi_uim_refresh_event_type));

  READ_8_BIT_VAL (value_ptr, temp_val);
  rsp->refresh_stage = (qmi_uim_refresh_stage)temp_val;

  READ_8_BIT_VAL (value_ptr, temp_val);
  rsp->refresh_mode = (qmi_uim_refresh_mode)temp_val;

  READ_8_BIT_VAL (value_ptr, temp_val);
  rsp->session_type = (qmi_uim_session_type)temp_val;

  READ_8_BIT_VAL (value_ptr, rsp->aid_len);
  if((rsp->aid_len > 0) && (rsp->aid_len <= QMI_UIM_MAX_AID_LEN))
  {
    memcpy (rsp->aid_value, (char *)value_ptr, rsp->aid_len);
    value_ptr += rsp->aid_len;
  }

  READ_16_BIT_VAL (value_ptr, rsp->num_files);

  if (rsp->num_files > 0)
  {
    /* Note that in case of a Sync call, Client needs to free this memory,
       and in case of an Indication call, QMI_UIM service will free it up */
    rsp->files_ptr = (qmi_uim_refresh_file_id_type *)
                      malloc (rsp->num_files * sizeof(qmi_uim_refresh_file_id_type));

    if (rsp->files_ptr)
    {
      /* Fill the file list up */
      memset (rsp->files_ptr, 0x00,
              rsp->num_files * sizeof(qmi_uim_refresh_file_id_type));

      for (i = 0; (i < rsp->num_files) && (value_ptr != NULL); i++)
      {
        /* Read file ID & path length */
        READ_16_BIT_VAL (value_ptr, rsp->files_ptr[i].file_id);
        READ_8_BIT_VAL (value_ptr, rsp->files_ptr[i].path_len);

        /* Update file path */
        if((rsp->files_ptr[i].path_len > 0) &&
           (rsp->files_ptr[i].path_len <= QMI_UIM_MAX_FILE_PATH))
        {
          /* Construct file path which is a sequence of 2 byte blocks */
          for(j = 0; j < rsp->files_ptr[i].path_len; j+=2)
          {
            rsp->files_ptr[i].path_value[j] = *(value_ptr+j+1);
            rsp->files_ptr[i].path_value[j+1] = *(value_ptr+j);
          }
          /* Update source pointer for the next file details */
          value_ptr += rsp->files_ptr[i].path_len;
        }
        else
        {
          return;
        }
      }
    }
  }
} /* qmi_uim_read_refresh_event_tlv */


/*===========================================================================
  FUNCTION  qmi_uim_read_set_pin_protection_tlv
===========================================================================*/
/*!
@brief
  Parses the raw TLV input for set pin protection response data, and writes
  it to the passed response pointer. Input Buffer pointer value_ptr is
  adjusted as the TLV is parsed.

@return
  None.

@note

  - Dependencies
    - None.

  - Side Effects
    - Writes TLV to input structure pointer and updates TLV buffer pointer
*/
/*=========================================================================*/
static void qmi_uim_read_set_pin_protection_tlv
(
  unsigned char                             * value_ptr,
  const unsigned long                       * type,
  qmi_uim_set_pin_protection_rsp_type       * pin_protection_rsp_ptr
)
{
  /* Update respone data with based on appropriate TLV */
  if (*type == QMI_UIM_OPTIONAL_1_RESP_TLV_ID)
  {
    READ_8_BIT_VAL (value_ptr, pin_protection_rsp_ptr->num_retries);
    READ_8_BIT_VAL (value_ptr, pin_protection_rsp_ptr->num_unblock_retries);
  }
  else if(*type == QMI_UIM_OPTIONAL_2_RESP_TLV_ID)
  {
    /* We expect only 1 optional_2 TLV for encrypted pin, any more will be rejected */
    if (pin_protection_rsp_ptr->encr_pin_data.data_ptr)
    {
      return;
    }

    READ_8_BIT_VAL (value_ptr, pin_protection_rsp_ptr->encr_pin_data.data_len);
    if (pin_protection_rsp_ptr->encr_pin_data.data_len > 0)
    {
      pin_protection_rsp_ptr->encr_pin_data.data_ptr =
          (unsigned char *) malloc (pin_protection_rsp_ptr->encr_pin_data.data_len);
      /* Note that in case of a Sync call, Client needs to free this up,
         and in case of an Async call, QMI_UIM service will free it up */
      if (pin_protection_rsp_ptr->encr_pin_data.data_ptr)
      {
        memcpy (pin_protection_rsp_ptr->encr_pin_data.data_ptr,
                value_ptr,
                pin_protection_rsp_ptr->encr_pin_data.data_len);
      }
    }
  }
} /* qmi_uim_read_set_pin_protection_tlv */


/*===========================================================================
  FUNCTION  qmi_uim_read_verify_pin_tlv
===========================================================================*/
/*!
@brief
  Parses the raw TLV input for verify pin response data, and writes
  it to the passed response pointer. Input Buffer pointer value_ptr is
  adjusted as the TLV is parsed.

@return
  None.

@note

  - Dependencies
    - None.

  - Side Effects
    - Writes TLV to input structure pointer and updates TLV buffer pointer
*/
/*=========================================================================*/
static void qmi_uim_read_verify_pin_tlv
(
  unsigned char                             * value_ptr,
  const unsigned long                       * type,
  qmi_uim_verify_pin_rsp_type               * verify_pin_rsp_ptr
)
{
  /* Update respone data with based on appropriate TLV */
  if (*type == QMI_UIM_OPTIONAL_1_RESP_TLV_ID)
  {
    READ_8_BIT_VAL (value_ptr, verify_pin_rsp_ptr->num_retries);
    READ_8_BIT_VAL (value_ptr, verify_pin_rsp_ptr->num_unblock_retries);
  }
  else if(*type == QMI_UIM_OPTIONAL_2_RESP_TLV_ID)
  {
    /* We expect only 1 optional_2 TLV for encrypted pin, any more will be rejected */
    if (verify_pin_rsp_ptr->encr_pin_data.data_ptr)
    {
      return;
    }

    READ_8_BIT_VAL (value_ptr, verify_pin_rsp_ptr->encr_pin_data.data_len);
    if (verify_pin_rsp_ptr->encr_pin_data.data_len > 0)
    {
      verify_pin_rsp_ptr->encr_pin_data.data_ptr =
          (unsigned char *) malloc (verify_pin_rsp_ptr->encr_pin_data.data_len);
      /* Note that in case of a Sync call, Client needs to free this up,
         and in case of an Async call, QMI_UIM service will free it up */
      if (verify_pin_rsp_ptr->encr_pin_data.data_ptr)
      {
        memcpy (verify_pin_rsp_ptr->encr_pin_data.data_ptr,
                value_ptr,
                verify_pin_rsp_ptr->encr_pin_data.data_len);
      }
    }
  }
} /* qmi_uim_read_verify_pin_tlv */


/*===========================================================================
  FUNCTION  qmi_uim_read_unblock_pin_tlv
===========================================================================*/
/*!
@brief
  Parses the raw TLV input for unblock pin response data, and writes
  it to the passed response pointer. Input Buffer pointer value_ptr is
  adjusted as the TLV is parsed.

@return
  None.

@note

  - Dependencies
    - None.

  - Side Effects
    - Writes TLV to input structure pointer and updates TLV buffer pointer
*/
/*=========================================================================*/
static void qmi_uim_read_unblock_pin_tlv
(
  unsigned char                             * value_ptr,
  const unsigned long                       * type,
  qmi_uim_unblock_pin_rsp_type              * unblock_pin_rsp_ptr
)
{
  /* Update respone data with based on appropriate TLV */
  if (*type == QMI_UIM_OPTIONAL_1_RESP_TLV_ID)
  {
    READ_8_BIT_VAL (value_ptr, unblock_pin_rsp_ptr->num_retries);
    READ_8_BIT_VAL (value_ptr, unblock_pin_rsp_ptr->num_unblock_retries);
  }
  else if(*type == QMI_UIM_OPTIONAL_2_RESP_TLV_ID)
  {
    /* We expect only 1 optional_2 TLV for encrypted pin, any more will be rejected */
    if (unblock_pin_rsp_ptr->encr_pin_data.data_ptr)
    {
      return;
    }

    READ_8_BIT_VAL (value_ptr, unblock_pin_rsp_ptr->encr_pin_data.data_len);
    if (unblock_pin_rsp_ptr->encr_pin_data.data_len > 0)
    {
      unblock_pin_rsp_ptr->encr_pin_data.data_ptr =
          (unsigned char *) malloc (unblock_pin_rsp_ptr->encr_pin_data.data_len);
      /* Note that in case of a Sync call, Client needs to free this up,
         and in case of an Async call, QMI_UIM service will free it up */
      if (unblock_pin_rsp_ptr->encr_pin_data.data_ptr)
      {
        memcpy (unblock_pin_rsp_ptr->encr_pin_data.data_ptr,
                value_ptr,
                unblock_pin_rsp_ptr->encr_pin_data.data_len);
      }
    }
  }
} /* qmi_uim_read_unblock_pin_tlv */


/*===========================================================================
  FUNCTION  qmi_uim_read_change_pin_tlv
===========================================================================*/
/*!
@brief
  Parses the raw TLV input for change pin response data, and writes
  it to the passed response pointer. Input Buffer pointer value_ptr is
  adjusted as the TLV is parsed.

@return
  None.

@note

  - Dependencies
    - None.

  - Side Effects
    - Writes TLV to input structure pointer and updates TLV buffer pointer
*/
/*=========================================================================*/
static void qmi_uim_read_change_pin_tlv
(
  unsigned char                             * value_ptr,
  const unsigned long                       * type,
  qmi_uim_change_pin_rsp_type               * change_pin_rsp_ptr
)
{
  /* Update respone data with based on appropriate TLV */
  if (*type == QMI_UIM_OPTIONAL_1_RESP_TLV_ID)
  {
    READ_8_BIT_VAL (value_ptr, change_pin_rsp_ptr->num_retries);
    READ_8_BIT_VAL (value_ptr, change_pin_rsp_ptr->num_unblock_retries);
  }
  else if(*type == QMI_UIM_OPTIONAL_2_RESP_TLV_ID)
  {
    /* We expect only 1 optional_2 TLV for encrypted pin, any more will be rejected */
    if (change_pin_rsp_ptr->encr_pin_data.data_ptr)
    {
      return;
    }

    READ_8_BIT_VAL (value_ptr, change_pin_rsp_ptr->encr_pin_data.data_len);
    if (change_pin_rsp_ptr->encr_pin_data.data_len > 0)
    {
      change_pin_rsp_ptr->encr_pin_data.data_ptr =
          (unsigned char *) malloc (change_pin_rsp_ptr->encr_pin_data.data_len);
      /* Note that in case of a Sync call, Client needs to free this up,
         and in case of an Async call, QMI_UIM service will free it up */
      if (change_pin_rsp_ptr->encr_pin_data.data_ptr)
      {
        memcpy (change_pin_rsp_ptr->encr_pin_data.data_ptr,
                value_ptr,
                change_pin_rsp_ptr->encr_pin_data.data_len);
      }
    }
  }
} /* qmi_uim_read_change_pin_tlv */


/*===========================================================================
  FUNCTION  qmi_uim_read_card_status_tlv
===========================================================================*/
/*!
@brief
  Parses the raw card status TLV and writes it to the passed response pointer.
  Input Buffer pointer value_ptr is adjusted as the TLV is parsed.

@return
  None.

@note

  - Dependencies
    - None.

  - Side Effects
    - Writes TLV to input structure pointer and updates TLV buffer pointer
*/
/*=========================================================================*/
static void qmi_uim_read_card_status_tlv
(
  unsigned char                             * value_ptr,
  qmi_uim_card_status_type                  * rsp_data
)
{
  int i = 0;
  int j = 0;
  int slots = 0;

  /* Process the TLV */
  memset (rsp_data, 0, sizeof(qmi_uim_card_status_type));

  READ_16_BIT_VAL (value_ptr, rsp_data->index_gw_pri_prov);
  READ_16_BIT_VAL (value_ptr, rsp_data->index_1x_pri_prov);
  READ_16_BIT_VAL (value_ptr, rsp_data->index_gw_sec_prov);
  READ_16_BIT_VAL (value_ptr, rsp_data->index_1x_sec_prov);
  READ_8_BIT_VAL (value_ptr, rsp_data->num_slots);

  slots = (rsp_data->num_slots <= QMI_UIM_MAX_CARD_COUNT)
           ? rsp_data->num_slots : QMI_UIM_MAX_CARD_COUNT;

  /* Fill Card specific data */
  for (i = 0; i < slots; i++)
  {
    unsigned char temp_val;
    READ_8_BIT_VAL (value_ptr, temp_val);
    rsp_data->card[i].card_state = (qmi_uim_card_state_type)temp_val;
    READ_8_BIT_VAL (value_ptr, temp_val);
    rsp_data->card[i].upin_state = (qmi_uim_pin_status_type)temp_val;
    READ_8_BIT_VAL (value_ptr, rsp_data->card[i].upin_num_retries);
    READ_8_BIT_VAL (value_ptr, rsp_data->card[i].upuk_num_retries);
    READ_8_BIT_VAL (value_ptr, temp_val);
    rsp_data->card[i].card_error = (qmi_uim_card_error_type)temp_val;
    READ_8_BIT_VAL (value_ptr, rsp_data->card[i].num_app);

    /* Fill App specific data per card */
    for (j = 0; j < rsp_data->card[i].num_app
             && j < QMI_UIM_MAX_APP_PER_CARD_COUNT; j++)
    {
      READ_8_BIT_VAL (value_ptr, temp_val);
      rsp_data->card[i].application[j].app_type = (qmi_uim_app_type)temp_val;
      READ_8_BIT_VAL (value_ptr, temp_val);
      rsp_data->card[i].application[j].app_state = (qmi_uim_app_state_type)temp_val;
      READ_8_BIT_VAL (value_ptr, temp_val);
      rsp_data->card[i].application[j].perso_state = (qmi_uim_perso_state_type)temp_val;
      READ_8_BIT_VAL (value_ptr, temp_val);
      rsp_data->card[i].application[j].perso_feature = (qmi_uim_perso_feature_id_type)temp_val;
      READ_8_BIT_VAL (value_ptr, rsp_data->card[i].application[j].perso_retries);
      READ_8_BIT_VAL (value_ptr, rsp_data->card[i].application[j].perso_unblock_retries);
      READ_8_BIT_VAL (value_ptr, rsp_data->card[i].application[j].aid_len);
      if(rsp_data->card[i].application[j].aid_len <= QMI_UIM_MAX_AID_LEN)
      {
        memcpy (rsp_data->card[i].application[j].aid_value,
                (void *)value_ptr,
                rsp_data->card[i].application[j].aid_len);
        value_ptr += rsp_data->card[i].application[j].aid_len;
      }

      READ_8_BIT_VAL (value_ptr, rsp_data->card[i].application[j].univ_pin);
      READ_8_BIT_VAL (value_ptr, temp_val);
      rsp_data->card[i].application[j].pin1_state = (qmi_uim_pin_status_type)temp_val;
      READ_8_BIT_VAL (value_ptr, rsp_data->card[i].application[j].pin1_num_retries);
      READ_8_BIT_VAL (value_ptr, rsp_data->card[i].application[j].puk1_num_retries);
      READ_8_BIT_VAL (value_ptr, temp_val);
      rsp_data->card[i].application[j].pin2_state = (qmi_uim_pin_status_type)temp_val;
      READ_8_BIT_VAL (value_ptr, rsp_data->card[i].application[j].pin2_num_retries);
      READ_8_BIT_VAL (value_ptr, rsp_data->card[i].application[j].puk2_num_retries);
    }
  }
} /* qmi_uim_read_card_status_tlv */


/*===========================================================================
  FUNCTION  qmi_uim_read_card_status_validity_tlv
===========================================================================*/
/*!
@brief
  Parses the raw card status validity TLV and writes it to the passed
  response pointer. Input Buffer pointer value_ptr is adjusted as the TLV
  is parsed.

@return
  None.

@note

  - Dependencies
    - None.

  - Side Effects
    - Writes TLV to input structure pointer and updates TLV buffer pointer
*/
/*=========================================================================*/
static void qmi_uim_read_card_status_validity_tlv
(
  unsigned char                             * value_ptr,
  qmi_uim_card_status_validity_type         * rsp_data
)
{
  int           i        = 0;
  int           slots    = 0;
  unsigned char temp_val = 0;

  /* Error check */
  if (rsp_data == NULL)
  {
    return;
  }

  memset (rsp_data, 0, sizeof(qmi_uim_card_status_validity_type));

  /* Process the TLV */
  READ_8_BIT_VAL (value_ptr, temp_val);

  slots = (temp_val <= QMI_UIM_MAX_CARD_COUNT) ? temp_val : QMI_UIM_MAX_CARD_COUNT;

  /* Fill Card specific data */
  for (i = 0; i < slots; i++)
  {
    READ_8_BIT_VAL (value_ptr, temp_val);
    rsp_data->card_status_invalid[i] = (temp_val == 0) ? QMI_UIM_TRUE : QMI_UIM_FALSE;
  }
} /* qmi_uim_read_card_status_validity_tlv */


/*===========================================================================
  FUNCTION  qmi_uim_read_auth_response_tlv
===========================================================================*/
/*!
@brief
  Parses the raw TLV for authentication response data, and writes it to the
  passed response pointer. Input Buffer pointer value_ptr is adjusted as the
  TLV is parsed.

@return
  None.

@note

  - Dependencies
    - None.

  - Side Effects
    - Writes TLV to input structure pointer and updates TLV buffer pointer
*/
/*=========================================================================*/
static void qmi_uim_read_auth_response_tlv
(
  unsigned char                             * value_ptr,
  qmi_uim_authenticate_rsp_type             * rsp
)
{
  /* We expect only 1 optional_2 TLV for auth response, any more will be rejected */
  if (rsp->auth_response.data_ptr)
  {
    return;
  }

  READ_16_BIT_VAL (value_ptr, rsp->auth_response.data_len);

  if (rsp->auth_response.data_len > 0)
  {
    /* Note that in case of a Sync call, Client needs to free this up,
       and in case of an ASync call, QMI_UIM service will free it up */
    rsp->auth_response.data_ptr = (unsigned char *) malloc (rsp->auth_response.data_len);
    if (rsp->auth_response.data_ptr)
    {
      memcpy (rsp->auth_response.data_ptr,
              value_ptr,
              rsp->auth_response.data_len);
    }
  }
} /* qmi_uim_read_auth_response_tlv */


/*===========================================================================
  FUNCTION  qmi_uim_read_send_apdu_tlv
===========================================================================*/
/*!
@brief
  Parses the raw TLV for send apdu response data, and writes
  it to the passed response pointer. Input Buffer pointer value_ptr is
  adjusted as the TLV is parsed.

@return
  None.

@note

  - Dependencies
    - None.

  - Side Effects
    - Writes TLV to input structure pointer and updates TLV buffer pointer
*/
/*=========================================================================*/
static void qmi_uim_read_send_apdu_tlv
(
  unsigned char                             * value_ptr,
  qmi_uim_send_apdu_rsp_type                * rsp
)
{
  /* We expect only 1 optional_1 TLV for send apdu, any more will be rejected */
  if (rsp->apdu_response.data_ptr)
  {
    return;
  }

  READ_16_BIT_VAL (value_ptr, rsp->apdu_response.data_len);

  if (rsp->apdu_response.data_len > 0)
  {
    /* Note that in case of a Sync call, Client needs to free this up,
       and in case of an ASync call, QMI_UIM service will free it up */
    rsp->apdu_response.data_ptr = (unsigned char *) malloc (rsp->apdu_response.data_len);
    if (rsp->apdu_response.data_ptr)
    {
      memcpy (rsp->apdu_response.data_ptr,
              value_ptr,
              rsp->apdu_response.data_len);
    }
  }
} /* qmi_uim_read_send_apdu_tlv */


/*===========================================================================
  FUNCTION  qmi_uim_read_sap_response_tlv
===========================================================================*/
/*!
@brief
  Parses the raw TLV for SAP response data, and writes it to passed response
  pointer. Input Buffer pointer value_ptr is adjusted as the TLV is parsed.

@return
  None.

@note

  - Dependencies
    - None.

  - Side Effects
    - Writes TLV to input structure pointer and updates TLV buffer pointer
*/
/*=========================================================================*/
static void qmi_uim_read_sap_response_tlv
(
  unsigned long                               type,
  unsigned char                             * value_ptr,
  qmi_uim_sap_request_rsp_type              * rsp
)
{
  /* We expect only 1 optional TLV for any of the sap requests, any more will be rejected */
  if (rsp->sap_response.data_ptr)
  {
    return;
  }

  /* Optional 1 TLV is for ATR value, optional 3 is for Card reader status */
  if ((type == QMI_UIM_OPTIONAL_1_RESP_TLV_ID) ||
      (type == QMI_UIM_OPTIONAL_3_RESP_TLV_ID))
  {
    /* Parse ATR value TLV or Card reader status TLV */
    READ_8_BIT_VAL (value_ptr, rsp->sap_response.data_len);
  }
  /* Optional 2 TLV is for APDU result TLV */
  else if(type == QMI_UIM_OPTIONAL_2_RESP_TLV_ID)
  {
    /* Parse APDU result TLV */
    READ_16_BIT_VAL (value_ptr, rsp->sap_response.data_len);
  }

  if (rsp->sap_response.data_len > 0)
  {
    /* Note that in case of a Sync call, Client needs to free this up,
       and in case of an ASync call, QMI_UIM service will free it up */
    rsp->sap_response.data_ptr = (unsigned char *) malloc (rsp->sap_response.data_len);
    if (rsp->sap_response.data_ptr)
    {
      memcpy (rsp->sap_response.data_ptr,
              value_ptr,
              rsp->sap_response.data_len);
    }
  }
} /* qmi_uim_read_sap_response_tlv */


/*===========================================================================
  FUNCTION  qmi_uim_read_get_atr_tlv
===========================================================================*/
/*!
@brief
  Parses the raw TLV for get ATR response data, and writes
  it to the passed response pointer. Input Buffer pointer value_ptr is
  adjusted as the TLV is parsed.

@return
  None.

@note

  - Dependencies
    - None.

  - Side Effects
    - Writes TLV to input structure pointer and updates TLV buffer pointer
*/
/*=========================================================================*/
static void qmi_uim_read_get_atr_tlv
(
  unsigned char                             * value_ptr,
  qmi_uim_get_atr_rsp_type                  * rsp
)
{
  /* We expect only 1 optional_1 TLV for send apdu, any more will be rejected */
  if (rsp->atr_response.data_ptr)
  {
    return;
  }

  READ_8_BIT_VAL (value_ptr, rsp->atr_response.data_len);

  if (rsp->atr_response.data_len > 0)
  {
    /* Note that in case of a Sync call, Client needs to free this up,
       and in case of an ASync call, QMI_UIM service will free it up */
    rsp->atr_response.data_ptr = (unsigned char *) malloc (rsp->atr_response.data_len);
    if (rsp->atr_response.data_ptr)
    {
      memcpy (rsp->atr_response.data_ptr,
              value_ptr,
              rsp->atr_response.data_len);
    }
  }
} /* qmi_uim_read_get_atr_tlv */


/*---------------------------------------------------------------------------
  Response functions to the Client
---------------------------------------------------------------------------*/

/*===========================================================================
  FUNCTION  qmi_uim_handle_read_rsp
===========================================================================*/
/*!
@brief
  This function decodes the response TLVs for both read transparent as well
  as read record request synchronous function calls.

@return
  QMI_INTERNAL_ERR if an error was encountered, QMI_NO_ERR otherwise.

*/
/*=========================================================================*/
static int qmi_uim_handle_read_rsp
(
  unsigned char                             * rx_msg,
  int                                         rx_msg_size,
  qmi_uim_rsp_data_type                     * rsp_data
)
{
  unsigned long type;
  unsigned long length;
  unsigned char *value_ptr;

  rsp_data->sys_err_code = QMI_NO_ERR;

  /* Loop to read the TLVs */
  while (rx_msg_size > 0)
  {
    if (qmi_util_read_std_tlv (&rx_msg,
                               &rx_msg_size,
                               &type,
                               &length,
                               &value_ptr) < 0)
    {
      return QMI_INTERNAL_ERR;
    }

    /* Process the TLV and update response data */
    switch (type)
    {
      case QMI_UIM_OPTIONAL_1_RESP_TLV_ID:
        if (rsp_data->rsp_id == QMI_UIM_SRVC_READ_TRANSPARENT_RSP_MSG)
        {
          READ_8_BIT_VAL (value_ptr,
                          rsp_data->rsp_data.read_transparent_rsp.sw1);
          READ_8_BIT_VAL (value_ptr,
                          rsp_data->rsp_data.read_transparent_rsp.sw2);
        }
        else if (rsp_data->rsp_id == QMI_UIM_SRVC_READ_RECORD_RSP_MSG)
        {
          READ_8_BIT_VAL (value_ptr,
                          rsp_data->rsp_data.read_record_rsp.sw1);
          READ_8_BIT_VAL (value_ptr,
                          rsp_data->rsp_data.read_record_rsp.sw2);
        }
        break;

      case QMI_UIM_OPTIONAL_2_RESP_TLV_ID:
        if (rsp_data->rsp_id == QMI_UIM_SRVC_READ_TRANSPARENT_RSP_MSG)
        {
          qmi_uim_read_read_transparent_tlv(value_ptr,
                                            &rsp_data->rsp_data.read_transparent_rsp);
        }
        else if (rsp_data->rsp_id == QMI_UIM_SRVC_READ_RECORD_RSP_MSG)
        {
          qmi_uim_read_read_record_tlv(value_ptr,
                                       &rsp_data->rsp_data.read_record_rsp);
        }
        break;

      default:
          QMI_ERR_MSG_1 ("qmi_uim_handle_read_rsp: unknown TLV type = %x",
                        (unsigned int)type);
        break;
    }
  }
  return QMI_NO_ERR;
} /* qmi_uim_handle_read_rsp */


/*===========================================================================
  FUNCTION  qmi_uim_handle_write_rsp
===========================================================================*/
/*!
@brief
  This function decodes the response TLVs for both write transparent as well
  as write record request synchronous function calls.

@return
  QMI_INTERNAL_ERR if an error was encountered, QMI_NO_ERR otherwise.

*/
/*=========================================================================*/
static int qmi_uim_handle_write_rsp
(
  unsigned char                             * rx_msg,
  int                                         rx_msg_size,
  qmi_uim_rsp_data_type                     * rsp_data
)
{
  unsigned long type;
  unsigned long length;
  unsigned char *value_ptr;

  /* Read the TLV */
  if (qmi_util_read_std_tlv (&rx_msg,
                             &rx_msg_size,
                             &type,
                             &length,
                             &value_ptr) < 0)
  {
    return QMI_INTERNAL_ERR;
  }
  /* Verify that it is the right TLV */
  if (type != QMI_UIM_OPTIONAL_1_RESP_TLV_ID)
  {
    return QMI_INTERNAL_ERR;
  }

  rsp_data->sys_err_code = QMI_NO_ERR;

  if(rsp_data->rsp_id == QMI_UIM_SRVC_WRITE_TRANSPARENT_RSP_MSG)
  {
    READ_8_BIT_VAL (value_ptr,
                    rsp_data->rsp_data.write_transparent_rsp.sw1);
    READ_8_BIT_VAL (value_ptr,
                    rsp_data->rsp_data.write_transparent_rsp.sw2);
  }
  else if(rsp_data->rsp_id == QMI_UIM_SRVC_WRITE_RECORD_RSP_MSG)
  {
    READ_8_BIT_VAL (value_ptr,
                    rsp_data->rsp_data.write_record_rsp.sw1);
    READ_8_BIT_VAL (value_ptr,
                    rsp_data->rsp_data.write_record_rsp.sw1);
  }

  return QMI_NO_ERR;
} /* qmi_uim_handle_write_rsp */


/*===========================================================================
  FUNCTION  qmi_uim_handle_get_file_attributes_rsp
===========================================================================*/
/*!
@brief
  This function decodes the response TLVs for get file attributes request
  synchronous function call.

@return
  QMI_INTERNAL_ERR if an error was encountered, QMI_NO_ERR otherwise.

*/
/*=========================================================================*/
static int qmi_uim_handle_get_file_attributes_rsp
(
  unsigned char                             * rx_msg,
  int                                         rx_msg_size,
  qmi_uim_rsp_data_type                     * rsp_data
)
{
  unsigned long type;
  unsigned long length;
  unsigned char *value_ptr;

  rsp_data->sys_err_code = QMI_NO_ERR;

  /* Loop to read the TLVs */
  while (rx_msg_size > 0)
  {
    if (qmi_util_read_std_tlv (&rx_msg,
                               &rx_msg_size,
                               &type,
                               &length,
                               &value_ptr) < 0)
    {
      return QMI_INTERNAL_ERR;
    }

    /* Process the TLV and update response data */
    qmi_uim_read_get_file_attributes_tlv(value_ptr,
                                         &type,
                                         &rsp_data->rsp_data.get_file_attributes_rsp);
  }
  return QMI_NO_ERR;
} /* qmi_uim_handle_get_file_attributes_rsp */


/*===========================================================================
  FUNCTION  qmi_uim_handle_refresh_event_rsp
===========================================================================*/
/*!
@brief
  This function decodes the response TLV for refresh event. This is the same
  function that is called by both indication callback & the get last event
  command request.

@return
  QMI_INTERNAL_ERR if an error was encountered, QMI_NO_ERR otherwise.

*/
/*=========================================================================*/
static int qmi_uim_handle_refresh_event_rsp
(
  unsigned char                          * rx_msg,
  int                                      rx_msg_size,
  void                                   * rsp_data,
  qmi_service_msg_type                     msg_type
)
{
  unsigned long                     type;
  unsigned long                     length;
  unsigned char                   * value_ptr;
  qmi_uim_rsp_data_type           * rsp_msg = NULL;
  qmi_uim_indication_data_type    * ind_msg = NULL;

  /* Read the TLV */
  if (qmi_util_read_std_tlv (&rx_msg,
                             &rx_msg_size,
                             &type,
                             &length,
                             &value_ptr) < 0)
  {
    return QMI_INTERNAL_ERR;
  }

  /* Verify that it is the right TLV */
  if (type != QMI_UIM_OPTIONAL_1_RESP_TLV_ID)
  {
    return QMI_INTERNAL_ERR;
  }

  /* Cast appropriately */
  if (msg_type == QMI_SERVICE_RESPONSE_MSG)
  {
    rsp_msg = (qmi_uim_rsp_data_type*)rsp_data;
    rsp_msg->sys_err_code = QMI_NO_ERR;
    rsp_data = (qmi_uim_refresh_event_type*)&rsp_msg->rsp_data.refresh_get_last_event_rsp.refresh_event;
  }
  else if (msg_type == QMI_SERVICE_INDICATION_MSG)
  {
    ind_msg = (qmi_uim_indication_data_type*)rsp_data;
    rsp_data = (qmi_uim_refresh_event_type*)&ind_msg->refresh_ind.refresh_event;
  }
  else
  {
    return QMI_INTERNAL_ERR;
  }

  /* Process the TLV and update response data */
  qmi_uim_read_refresh_event_tlv(value_ptr, rsp_data);

  return QMI_NO_ERR;
} /* qmi_uim_handle_refresh_event_rsp */


/*===========================================================================
  FUNCTION  qmi_uim_handle_pin_operation_rsp
===========================================================================*/
/*!
@brief
  This function decodes the response TLV for all the pin operation requests
  synchronous function call.

@return
  QMI_INTERNAL_ERR if an error was encountered, QMI_NO_ERR otherwise.

*/
/*=========================================================================*/
static int qmi_uim_handle_pin_operation_rsp
(
  unsigned char                             * rx_msg,
  int                                         rx_msg_size,
  qmi_uim_rsp_data_type                     * rsp_data_ptr
)
{
  unsigned long type;
  unsigned long length;
  unsigned char *value_ptr;

  /* Loop to read the TLVs */
  while (rx_msg_size > 0)
  {
    if (qmi_util_read_std_tlv (&rx_msg,
                               &rx_msg_size,
                               &type,
                               &length,
                               &value_ptr) < 0)
    {
      return QMI_INTERNAL_ERR;
    }

    /* Process the TLV for the respective response type */
    switch (rsp_data_ptr->rsp_id)
    {
      case QMI_UIM_SRVC_SET_PIN_PROTECTION_RSP_MSG:
        qmi_uim_read_set_pin_protection_tlv(value_ptr,
                                            &type,
                                            &rsp_data_ptr->rsp_data.set_pin_protection_rsp);
        break;
      case QMI_UIM_SRVC_VERIFY_PIN_RSP_MSG:
        qmi_uim_read_verify_pin_tlv(value_ptr,
                                    &type,
                                    &rsp_data_ptr->rsp_data.verify_pin_rsp);
        break;
      case QMI_UIM_SRVC_UNBLOCK_PIN_RSP_MSG:
        qmi_uim_read_unblock_pin_tlv(value_ptr,
                                     &type,
                                     &rsp_data_ptr->rsp_data.unblock_pin_rsp);
        break;
      case QMI_UIM_SRVC_CHANGE_PIN_RSP_MSG:
        qmi_uim_read_change_pin_tlv(value_ptr,
                                    &type,
                                    &rsp_data_ptr->rsp_data.change_pin_rsp);
        break;
      default:
        return QMI_INTERNAL_ERR;
    }
  }
  return QMI_NO_ERR;
} /* qmi_uim_handle_pin_operation_rsp */


/*===========================================================================
  FUNCTION  qmi_uim_handle_depersonalization_rsp
===========================================================================*/
/*!
@brief
  This function decodes the response TLV for depersonalization request
  synchronous  function call.

@return
  QMI_INTERNAL_ERR if an error was encountered, QMI_NO_ERR otherwise.

*/
/*=========================================================================*/
static int qmi_uim_handle_depersonalization_rsp
(
  unsigned char                             * rx_msg,
  int                                         rx_msg_size,
  qmi_uim_rsp_data_type                     * rsp_data
)
{
  unsigned long type;
  unsigned long length;
  unsigned char *value_ptr;

  /* Read the TLV */
  if (qmi_util_read_std_tlv (&rx_msg,
                             &rx_msg_size,
                             &type,
                             &length,
                             &value_ptr) < 0)
  {
    return QMI_INTERNAL_ERR;
  }

  /* Verify that it is the right TLV */
  if (type != QMI_UIM_OPTIONAL_1_RESP_TLV_ID)
  {
    return QMI_INTERNAL_ERR;
  }

  /* Process the TLV and update response data */
  READ_8_BIT_VAL (value_ptr,
                  rsp_data->rsp_data.depersonalization_rsp.num_retries);
  READ_8_BIT_VAL (value_ptr,
                  rsp_data->rsp_data.depersonalization_rsp.num_unblock_retries);

  return QMI_NO_ERR;
} /* qmi_uim_handle_depersonalization_rsp */


/*===========================================================================
  FUNCTION  qmi_uim_handle_get_card_status_rsp
===========================================================================*/
/*!
@brief
  This function decodes the response TLVs for Get Card Status command.

@return
  QMI_INTERNAL_ERR if an error was encountered, QMI_NO_ERR otherwise.

*/
/*=========================================================================*/
static int qmi_uim_handle_get_card_status_rsp
(
  unsigned char                             * rx_msg,
  int                                         rx_msg_size,
  void                                      * rsp_data,
  qmi_service_msg_type                        msg_type
)
{
  unsigned long                          type;
  unsigned long                          length;
  unsigned char                        * value_ptr;
  qmi_uim_rsp_data_type                * rsp_msg         = NULL;
  qmi_uim_indication_data_type         * ind_msg         = NULL;
  qmi_uim_card_status_type             * card_status_ptr = NULL;
  qmi_uim_card_status_validity_type    * validity_ptr    = NULL;

  /* Find out the type of request */
  if (msg_type == QMI_SERVICE_RESPONSE_MSG)
  {
    rsp_msg         = (qmi_uim_rsp_data_type*)rsp_data;
    card_status_ptr = (qmi_uim_card_status_type*)&rsp_msg->rsp_data.get_card_status_rsp.card_status;
    validity_ptr    = (qmi_uim_card_status_validity_type*)&rsp_msg->rsp_data.get_card_status_rsp.card_status_validity;
  }
  else if (msg_type == QMI_SERVICE_INDICATION_MSG)
  {
    ind_msg         = (qmi_uim_indication_data_type*)rsp_data;
    card_status_ptr = (qmi_uim_card_status_type*)&ind_msg->status_change_ind;
  }
  else
  {
    return QMI_INTERNAL_ERR;
  }

  /* loop to read the TLVs */
  while (rx_msg_size > 0)
  {
    if (qmi_util_read_std_tlv (&rx_msg,
                               &rx_msg_size,
                               &type,
                               &length,
                               &value_ptr) < 0)
    {
      return QMI_INTERNAL_ERR;
    }

    /* Process the TLVS and update response data */
    switch (type)
    {
      case QMI_UIM_OPTIONAL_1_RESP_TLV_ID:
        qmi_uim_read_card_status_tlv(value_ptr, card_status_ptr);
        break;

      case QMI_UIM_OPTIONAL_3_RESP_TLV_ID:
        qmi_uim_read_card_status_validity_tlv(value_ptr, validity_ptr);
        break;

      default:
        QMI_ERR_MSG_1 ("qmi_uim_handle_get_card_status_rsp: unknown TLV type = %x",
                      (unsigned int)type);
        break;
    }
  }

  return QMI_NO_ERR;
} /* qmi_uim_handle_get_card_status_rsp */


/*===========================================================================
  FUNCTION  qmi_uim_handle_event_reg_rsp
===========================================================================*/
/*!
@brief
  This function decodes the event reg response TLV.

@return
  QMI_INTERNAL_ERR if an error was encountered, QMI_NO_ERR otherwise.

*/
/*=========================================================================*/
static int qmi_uim_handle_event_reg_rsp
(
  unsigned char                             * rx_msg,
  int                                         rx_msg_size,
  qmi_uim_rsp_data_type                     * rsp_data
)
{
  unsigned long   type;
  unsigned long   length;
  unsigned char * value_ptr;

  /* Read a TLV */
  if (qmi_util_read_std_tlv (&rx_msg,
                             &rx_msg_size,
                             &type,
                             &length,
                             &value_ptr) < 0)
  {
    return QMI_INTERNAL_ERR;
  }

  /* Verify that it is the right TLV */
  if (type != QMI_UIM_OPTIONAL_1_RESP_TLV_ID)
  {
    return QMI_INTERNAL_ERR;
  }

  rsp_data->rsp_id = QMI_UIM_SRVC_EVENT_REG_RSP_MSG;

  /* Process the TLV */
  READ_32_BIT_VAL (value_ptr, rsp_data->rsp_data.event_reg_rsp.event_mask);

  return QMI_NO_ERR;
} /* qmi_uim_handle_event_reg_rsp */


/*===========================================================================
  FUNCTION  qmi_uim_handle_authenticate_rsp
===========================================================================*/
/*!
@brief
  This function decodes the response TLVs for authentication request
  synchronous function call.

@return
  QMI_INTERNAL_ERR if an error was encountered, QMI_NO_ERR otherwise.

*/
/*=========================================================================*/
static int qmi_uim_handle_authenticate_rsp
(
  unsigned char                             * rx_msg,
  int                                         rx_msg_size,
  qmi_uim_rsp_data_type                     * rsp_data
)
{
  unsigned long type;
  unsigned long length;
  unsigned char *value_ptr;

  rsp_data->rsp_id = QMI_UIM_SRVC_AUTHENTICATE_RSP_MSG;

  /* Loop to read the TLVs */
  while (rx_msg_size > 0)
  {
    if (qmi_util_read_std_tlv (&rx_msg,
                               &rx_msg_size,
                               &type,
                               &length,
                               &value_ptr) < 0)
    {
      return QMI_INTERNAL_ERR;
    }

    /* Process the TLVS and update response data */
    switch (type)
    {
      case QMI_UIM_OPTIONAL_1_RESP_TLV_ID:
        READ_8_BIT_VAL (value_ptr,
                        rsp_data->rsp_data.authenticate_rsp.sw1);
        READ_8_BIT_VAL (value_ptr,
                        rsp_data->rsp_data.authenticate_rsp.sw2);
        break;

      case QMI_UIM_OPTIONAL_2_RESP_TLV_ID:
        qmi_uim_read_auth_response_tlv(value_ptr,
                                       &rsp_data->rsp_data.authenticate_rsp);
        break;

      default:
        QMI_ERR_MSG_1 ("qmi_uim_handle_authenticate_rsp: unknown TLV type = %x",
                      (unsigned int)type);
        break;
    }
  }
  return QMI_NO_ERR;
} /* qmi_uim_handle_authenticate_rsp */


/*===========================================================================
  FUNCTION  qmi_uim_handle_get_service_status_rsp
===========================================================================*/
/*!
@brief
  This function decodes the get service status response TLV.

@return
  QMI_INTERNAL_ERR if an error was encountered, QMI_NO_ERR otherwise.

*/
/*=========================================================================*/
static int qmi_uim_handle_get_service_status_rsp
(
  unsigned char                             * rx_msg,
  int                                         rx_msg_size,
  qmi_uim_rsp_data_type                     * rsp_data
)
{
  unsigned long   type;
  unsigned long   length;
  unsigned char * value_ptr;

  /* Read a TLV */
  if (qmi_util_read_std_tlv (&rx_msg,
                             &rx_msg_size,
                             &type,
                             &length,
                             &value_ptr) < 0)
  {
    return QMI_INTERNAL_ERR;
  }

  /* Read the appropriate TLV */
  if (type == QMI_UIM_OPTIONAL_1_RESP_TLV_ID)
  {
    READ_8_BIT_VAL (value_ptr, rsp_data->rsp_data.get_service_status_rsp.fdn_status);
    rsp_data->rsp_data.get_service_status_rsp.fdn_status_valid = QMI_UIM_TRUE;
  }
  else if (type == QMI_UIM_OPTIONAL_2_RESP_TLV_ID)
  {
    READ_8_BIT_VAL (value_ptr, rsp_data->rsp_data.get_service_status_rsp.hidden_key_status);
    rsp_data->rsp_data.get_service_status_rsp.hidden_key_status_valid = QMI_UIM_TRUE;
  }
  else if (type == QMI_UIM_OPTIONAL_3_RESP_TLV_ID)
  {
    READ_8_BIT_VAL (value_ptr, rsp_data->rsp_data.get_service_status_rsp.index);
    rsp_data->rsp_data.get_service_status_rsp.index_valid = QMI_UIM_TRUE;
  }
  else if (type == QMI_UIM_OPTIONAL_4_RESP_TLV_ID)
  {
    READ_8_BIT_VAL (value_ptr, rsp_data->rsp_data.get_service_status_rsp.esn_status);
    rsp_data->rsp_data.get_service_status_rsp.esn_status_valid = QMI_UIM_TRUE;
  }
  else if (type == QMI_UIM_OPTIONAL_5_RESP_TLV_ID)
  {
    READ_8_BIT_VAL (value_ptr, rsp_data->rsp_data.get_service_status_rsp.acl_status);
    rsp_data->rsp_data.get_service_status_rsp.acl_status_valid = QMI_UIM_TRUE;
  }
  else
  {
    return QMI_INTERNAL_ERR;
  }

  rsp_data->rsp_id = QMI_UIM_SRVC_GET_SERVICE_STATUS_RSP_MSG;

  return QMI_NO_ERR;
} /* qmi_uim_handle_get_service_status_rsp */


/*===========================================================================
  FUNCTION  qmi_uim_handle_get_label_rsp
===========================================================================*/
/*!
@brief
  This function decodes the get service status response TLV.

@return
  QMI_INTERNAL_ERR if an error was encountered, QMI_NO_ERR otherwise.

*/
/*=========================================================================*/
static int qmi_uim_handle_get_label_rsp
(
  unsigned char                             * rx_msg,
  int                                         rx_msg_size,
  qmi_uim_rsp_data_type                     * rsp_data
)
{
  unsigned long   type;
  unsigned long   length;
  unsigned char * value_ptr;

  /* Read a TLV */
  if (qmi_util_read_std_tlv (&rx_msg,
                             &rx_msg_size,
                             &type,
                             &length,
                             &value_ptr) < 0)
  {
    return QMI_INTERNAL_ERR;
  }

  /* Verify that it is the right TLV */
  if (type != QMI_UIM_OPTIONAL_1_RESP_TLV_ID)
  {
    return QMI_INTERNAL_ERR;
  }

  rsp_data->rsp_id = QMI_UIM_SRVC_GET_LABEL_RSP_MSG;

  /* Process the TLV */
  READ_8_BIT_VAL (value_ptr, rsp_data->rsp_data.get_label_rsp.label_len);
  if(rsp_data->rsp_data.get_label_rsp.label_len <= QMI_UIM_MAX_LABEL_LEN)
  {
    memcpy (rsp_data->rsp_data.get_label_rsp.label_value,
            (void *)value_ptr,
            rsp_data->rsp_data.get_label_rsp.label_len);
  }

  return QMI_NO_ERR;
} /* qmi_uim_handle_get_label_rsp */


/*===========================================================================
  FUNCTION  qmi_uim_handle_send_apdu_rsp
===========================================================================*/
/*!
@brief
  This function decodes the send apdu response TLV.

@return
  QMI_INTERNAL_ERR if an error was encountered, QMI_NO_ERR otherwise.

*/
/*=========================================================================*/
static int qmi_uim_handle_send_apdu_rsp
(
  unsigned char                             * rx_msg,
  int                                         rx_msg_size,
  qmi_uim_rsp_data_type                     * rsp_data
)
{
  unsigned long   type;
  unsigned long   length;
  unsigned char * value_ptr;

  /* Update response id */
  rsp_data->rsp_id = QMI_UIM_SRVC_SEND_APDU_RSP_MSG;

  /* Read a TLV */
  if (qmi_util_read_std_tlv (&rx_msg,
                             &rx_msg_size,
                             &type,
                             &length,
                             &value_ptr) >= 0)
  {
    /* Verify that it is the right TLV */
    if (type != QMI_UIM_OPTIONAL_1_RESP_TLV_ID)
    {
      return QMI_INTERNAL_ERR;
    }

    /* Process the TLV */
    qmi_uim_read_send_apdu_tlv(value_ptr,
                               &rsp_data->rsp_data.send_apdu_rsp);
  }

  return QMI_NO_ERR;
} /* qmi_uim_handle_send_apdu_rsp */


/*===========================================================================
  FUNCTION  qmi_uim_handle_sap_connection_rsp
===========================================================================*/
/*!
@brief
  This function decodes the SAP connection response TLV.

@return
  QMI_INTERNAL_ERR if an error was encountered, QMI_NO_ERR otherwise.

*/
/*=========================================================================*/
static int qmi_uim_handle_sap_connection_rsp
(
  unsigned char                             * rx_msg,
  int                                         rx_msg_size,
  qmi_uim_rsp_data_type                     * rsp_data
)
{
  unsigned long   type;
  unsigned long   length;
  unsigned char * value_ptr;

  /* Update response id */
  rsp_data->rsp_id = QMI_UIM_SRVC_SAP_CONNECTION_RSP_MSG;

  /* Read optional TLV if present */
  if (qmi_util_read_std_tlv (&rx_msg,
                             &rx_msg_size,
                             &type,
                             &length,
                             &value_ptr) >= 0)
  {
    /* Verify that it is the right TLV */
    if (type != QMI_UIM_OPTIONAL_1_RESP_TLV_ID)
    {
      return QMI_INTERNAL_ERR;
    }

    /* Process the TLV */
    READ_8_BIT_VAL (value_ptr, rsp_data->rsp_data.sap_connection_rsp.connection_status);
  }

  return QMI_NO_ERR;
} /* qmi_uim_handle_sap_connection_rsp */


/*===========================================================================
  FUNCTION  qmi_uim_handle_sap_request_rsp
===========================================================================*/
/*!
@brief
  This function decodes the SAP request response TLV.

@return
  QMI_INTERNAL_ERR if an error was encountered, QMI_NO_ERR otherwise.

*/
/*=========================================================================*/
static int qmi_uim_handle_sap_request_rsp
(
  unsigned char                             * rx_msg,
  int                                         rx_msg_size,
  qmi_uim_rsp_data_type                     * rsp_data
)
{
  unsigned long   type;
  unsigned long   length;
  unsigned char * value_ptr;

  /* Update response id */
  rsp_data->rsp_id = QMI_UIM_SRVC_SAP_REQUEST_RSP_MSG;

  /* We could get any of the 3 optional TLVs */
  if (qmi_util_read_std_tlv (&rx_msg,
                             &rx_msg_size,
                             &type,
                             &length,
                             &value_ptr) >= 0)
  {
    /* Process the TLV */
    qmi_uim_read_sap_response_tlv(type,
                                  value_ptr,
                                  &rsp_data->rsp_data.sap_response_rsp);
  }
  return QMI_NO_ERR;
} /* qmi_uim_handle_sap_request_rsp */


/*===========================================================================
  FUNCTION  qmi_uim_handle_logical_channel_rsp
===========================================================================*/
/*!
@brief
  This function decodes the logical channel response TLV.

@return
  QMI_INTERNAL_ERR if an error was encountered, QMI_NO_ERR otherwise.

*/
/*=========================================================================*/
static int qmi_uim_handle_logical_channel_rsp
(
  unsigned char                             * rx_msg,
  int                                         rx_msg_size,
  qmi_uim_rsp_data_type                     * rsp_data
)
{
  unsigned long   type;
  unsigned long   length;
  unsigned char * value_ptr;

  /* Update response id */
  rsp_data->rsp_id = QMI_UIM_SRVC_LOGICAL_CHANNEL_RSP_MSG;

  /* Read optional TLV if present */
  if (qmi_util_read_std_tlv (&rx_msg,
                             &rx_msg_size,
                             &type,
                             &length,
                             &value_ptr) >= 0)
  {
    /* Verify that it is the right TLV */
    if (type != QMI_UIM_OPTIONAL_1_RESP_TLV_ID)
    {
      return QMI_INTERNAL_ERR;
    }

    /* Process the TLV */
    READ_8_BIT_VAL (value_ptr, rsp_data->rsp_data.logical_channel_rsp.channel_id);
  }

  return QMI_NO_ERR;
} /* qmi_uim_handle_logical_channel_rsp */


/*===========================================================================
  FUNCTION  qmi_uim_handle_get_atr_rsp
===========================================================================*/
/*!
@brief
  This function decodes the get ATR request response TLV.

@return
  QMI_INTERNAL_ERR if an error was encountered, QMI_NO_ERR otherwise.

*/
/*=========================================================================*/
static int qmi_uim_handle_get_atr_rsp
(
  unsigned char                             * rx_msg,
  int                                         rx_msg_size,
  qmi_uim_rsp_data_type                     * rsp_data
)
{
  unsigned long   type;
  unsigned long   length;
  unsigned char * value_ptr;

  /* Update response id */
  rsp_data->rsp_id = QMI_UIM_SRVC_GET_ATR_RSP_MSG;

  /* Read optional TLV if present */
  if (qmi_util_read_std_tlv (&rx_msg,
                             &rx_msg_size,
                             &type,
                             &length,
                             &value_ptr) >= 0)
  {
    /* Verify that it is the right TLV */
    if (type != QMI_UIM_OPTIONAL_1_RESP_TLV_ID)
    {
      return QMI_INTERNAL_ERR;
    }

    /* Process the TLV */
    qmi_uim_read_get_atr_tlv(value_ptr, &rsp_data->rsp_data.get_atr_rsp);
  }

  return QMI_NO_ERR;
} /* qmi_uim_handle_get_atr_rsp */


/*===========================================================================
  FUNCTION  qmi_uim_free_tlv_data
===========================================================================*/
/*!
@brief
  This function frees any allocated memory by the QMI UIM service. Note that
  this is applicable only for the Async QMI API calls.

@return
  None.

*/
/*=========================================================================*/
static void qmi_uim_free_tlv_data
(
  qmi_uim_rsp_data_type    * rsp_data
)
{
  if (rsp_data == NULL)
  {
    return;
  }

  switch (rsp_data->rsp_id)
  {
    case QMI_UIM_SRVC_GET_FILE_ATTRIBUTES_RSP_MSG:
      if (rsp_data->rsp_data.get_file_attributes_rsp.raw_value.data_ptr != NULL)
      {
        free(rsp_data->rsp_data.get_file_attributes_rsp.raw_value.data_ptr);
        rsp_data->rsp_data.get_file_attributes_rsp.raw_value.data_ptr = NULL;
      }
      break;

    case QMI_UIM_SRVC_READ_TRANSPARENT_RSP_MSG:
      if (rsp_data->rsp_data.read_transparent_rsp.content.data_ptr != NULL)
      {
        free(rsp_data->rsp_data.read_transparent_rsp.content.data_ptr);
        rsp_data->rsp_data.read_transparent_rsp.content.data_ptr = NULL;
      }
      break;

    case QMI_UIM_SRVC_READ_RECORD_RSP_MSG:
      if (rsp_data->rsp_data.read_transparent_rsp.content.data_ptr != NULL)
      {
        free(rsp_data->rsp_data.read_transparent_rsp.content.data_ptr);
        rsp_data->rsp_data.read_transparent_rsp.content.data_ptr = NULL;
      }
      break;

    case QMI_UIM_SRVC_SET_PIN_PROTECTION_RSP_MSG:
      if (rsp_data->rsp_data.set_pin_protection_rsp.encr_pin_data.data_ptr != NULL)
      {
        free(rsp_data->rsp_data.set_pin_protection_rsp.encr_pin_data.data_ptr);
        rsp_data->rsp_data.set_pin_protection_rsp.encr_pin_data.data_ptr = NULL;
      }
      break;

    case QMI_UIM_SRVC_VERIFY_PIN_RSP_MSG:
      if (rsp_data->rsp_data.verify_pin_rsp.encr_pin_data.data_ptr != NULL)
      {
        free(rsp_data->rsp_data.verify_pin_rsp.encr_pin_data.data_ptr);
        rsp_data->rsp_data.verify_pin_rsp.encr_pin_data.data_ptr = NULL;
      }
      break;

    case QMI_UIM_SRVC_UNBLOCK_PIN_RSP_MSG:
      if (rsp_data->rsp_data.unblock_pin_rsp.encr_pin_data.data_ptr != NULL)
      {
        free(rsp_data->rsp_data.unblock_pin_rsp.encr_pin_data.data_ptr);
        rsp_data->rsp_data.unblock_pin_rsp.encr_pin_data.data_ptr = NULL;
      }
      break;

    case QMI_UIM_SRVC_CHANGE_PIN_RSP_MSG:
      if (rsp_data->rsp_data.change_pin_rsp.encr_pin_data.data_ptr != NULL)
      {
        free(rsp_data->rsp_data.change_pin_rsp.encr_pin_data.data_ptr);
        rsp_data->rsp_data.change_pin_rsp.encr_pin_data.data_ptr = NULL;
      }
      break;

    case QMI_UIM_SRVC_AUTHENTICATE_RSP_MSG:
      if (rsp_data->rsp_data.authenticate_rsp.auth_response.data_ptr != NULL)
      {
        free(rsp_data->rsp_data.authenticate_rsp.auth_response.data_ptr);
        rsp_data->rsp_data.authenticate_rsp.auth_response.data_ptr = NULL;
      }
      break;

    case QMI_UIM_SRVC_SEND_APDU_RSP_MSG:
      if (rsp_data->rsp_data.send_apdu_rsp.apdu_response.data_ptr != NULL)
      {
        free(rsp_data->rsp_data.send_apdu_rsp.apdu_response.data_ptr);
        rsp_data->rsp_data.send_apdu_rsp.apdu_response.data_ptr = NULL;
      }
      break;

    case QMI_UIM_SRVC_SAP_REQUEST_RSP_MSG:
      if (rsp_data->rsp_data.sap_response_rsp.sap_response.data_ptr != NULL)
      {
        free(rsp_data->rsp_data.sap_response_rsp.sap_response.data_ptr);
        rsp_data->rsp_data.sap_response_rsp.sap_response.data_ptr = NULL;
      }
      break;

    case QMI_UIM_SRVC_GET_ATR_RSP_MSG:
      if (rsp_data->rsp_data.get_atr_rsp.atr_response.data_ptr != NULL)
      {
        free(rsp_data->rsp_data.get_atr_rsp.atr_response.data_ptr);
        rsp_data->rsp_data.get_atr_rsp.atr_response.data_ptr = NULL;
      }
      break;

    case QMI_UIM_SRVC_NONE_RSP_MSG:
    case QMI_UIM_SRVC_WRITE_TRANSPARENT_RSP_MSG:
    case QMI_UIM_SRVC_WRITE_RECORD_RSP_MSG:
    case QMI_UIM_SRVC_REFRESH_REGISTER_RSP_MSG:
    case QMI_UIM_SRVC_REFRESH_GET_LAST_EVENT_RSP_MSG:
    case QMI_UIM_SRVC_DEPERSONALIZATION_RSP_MSG:
    case QMI_UIM_SRVC_POWER_DOWN_RSP_MSG:
    case QMI_UIM_SRVC_POWER_UP_RSP_MSG:
    case QMI_UIM_SRVC_GET_CARD_STATUS_RSP_MSG:
    case QMI_UIM_SRVC_EVENT_REG_RSP_MSG:
    case QMI_UIM_SRVC_CLOSE_SESSION_RSP_MSG:
    case QMI_UIM_SRVC_GET_SERVICE_STATUS_RSP_MSG:
    case QMI_UIM_SRVC_SET_SERVICE_STATUS_RSP_MSG:
    case QMI_UIM_SRVC_CHANGE_PROV_SESSION_RSP_MSG:
    case QMI_UIM_SRVC_GET_LABEL_RSP_MSG:
    case QMI_UIM_SRVC_SAP_CONNECTION_RSP_MSG:
    case QMI_UIM_SRVC_LOGICAL_CHANNEL_RSP_MSG:
      /* Nothing to do */
      break;
  }
} /* qmi_uim_free_tlv_data */


/*---------------------------------------------------------------------------
  Callback functions
---------------------------------------------------------------------------*/

/*===========================================================================
  FUNCTION  qmi_uim_srvc_indication_cb
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

  - Side Effects
*/
/*=========================================================================*/
static void qmi_uim_srvc_indication_cb
(
  int                                 user_handle,
  qmi_service_id_type                 service_id,
  unsigned long                       msg_id,
  void                              * user_ind_msg_hdlr,
  void                              * user_ind_msg_hdlr_user_data,
  unsigned char                     * rx_msg_buf,
  int                                 rx_msg_len
)
{
  int                             rc = 0;
  qmi_uim_indication_id_type      ind_id;
  qmi_uim_indication_data_type    ind_data;
  qmi_uim_indication_hdlr_type    user_ind_hdlr;

  QMI_DEBUG_MSG_1("qmi_uim_srvc_indication_cb: ind = 0x%x", msg_id);

  if (user_ind_msg_hdlr == NULL)
  {
    return;
  }

  /* Initialize data */
  memset((void*)&ind_data, 0x0, sizeof(ind_data));

  /* Determine the indication ID and process appropriately */
  switch (msg_id)
  {
    case QMI_UIM_STATUS_CHANGE_IND_MSG_ID:
      ind_id = QMI_UIM_SRVC_STATUS_CHANGE_IND_MSG;
      rc = qmi_uim_handle_get_card_status_rsp (rx_msg_buf,
                                               rx_msg_len,
                                               &ind_data,
                                               QMI_SERVICE_INDICATION_MSG);
      if (rc < 0)
      {
        QMI_ERR_MSG_1 ("qmi_uim_srvc_indication_cb::qmi_uim_handle_get_card_status_rsp returned error %d\n ",
                       rc);
        return;
      }
      break;

    case QMI_UIM_REFRESH_IND_MSG_ID:
      ind_id = QMI_UIM_SRVC_REFRESH_IND_MSG;
      rc = qmi_uim_handle_refresh_event_rsp (rx_msg_buf,
                                             rx_msg_len,
                                             &ind_data,
                                             QMI_SERVICE_INDICATION_MSG);
      if (rc < 0)
      {
        QMI_ERR_MSG_1 ("qmi_uim_srvc_indication_cb::qmi_uim_handle_refresh_ind returned error %d\n ",
                       rc);
        return;
      }
      break;

    default:
      QMI_ERR_MSG_1 ("qmi_uim_srvc_indication_cb::Invalid indication msg_id received %lx\n ",msg_id);
      return;
  } /* Switch */

  /*lint -e{611} */
  user_ind_hdlr = (qmi_uim_indication_hdlr_type) user_ind_msg_hdlr;

  /* Call user registered handler */
  user_ind_hdlr (user_handle,
                 service_id,
                 user_ind_msg_hdlr_user_data,
                 ind_id,
                 &ind_data);

  /* Free up any memory allocated */
  if (msg_id == QMI_UIM_REFRESH_IND_MSG_ID)
  {
    if (ind_data.refresh_ind.refresh_event.files_ptr != NULL)
    {
      free(ind_data.refresh_ind.refresh_event.files_ptr);
      ind_data.refresh_ind.refresh_event.files_ptr = NULL;
    }
  }
} /* qmi_uim_srvc_indication_cb */


/*===========================================================================
  FUNCTION  qmi_uim_convert_msg_id
===========================================================================*/
/*!
@brief
  Converts message id to the response id.

@return
  Error codes.

@note

  - Dependencies
    - None.

  - Side Effects
    - None.
*/
/*=========================================================================*/
static int qmi_uim_convert_msg_id
(
  unsigned long               msg_id,
  qmi_uim_rsp_id_type       * rsp_id
)
{
  switch (msg_id)
  {
    case QMI_UIM_READ_TRANSPARENT_MSG_ID:
      *rsp_id = QMI_UIM_SRVC_READ_TRANSPARENT_RSP_MSG;
      break;

    case QMI_UIM_READ_RECORD_MSG_ID:
      *rsp_id = QMI_UIM_SRVC_READ_RECORD_RSP_MSG;
      break;

    case QMI_UIM_WRITE_TRANSPARENT_MSG_ID:
      *rsp_id = QMI_UIM_SRVC_WRITE_TRANSPARENT_RSP_MSG;
      break;

    case QMI_UIM_WRITE_RECORD_MSG_ID:
      *rsp_id = QMI_UIM_SRVC_WRITE_RECORD_RSP_MSG;
      break;

    case QMI_UIM_GET_FILE_ATTRIBUTES_MSG_ID:
      *rsp_id = QMI_UIM_SRVC_GET_FILE_ATTRIBUTES_RSP_MSG;
      break;

    case QMI_UIM_REFRESH_REGISTER_MSG_ID:
      *rsp_id = QMI_UIM_SRVC_REFRESH_REGISTER_RSP_MSG;
      break;

    case QMI_UIM_SET_PIN_PROTECTION_MSG_ID:
      *rsp_id = QMI_UIM_SRVC_SET_PIN_PROTECTION_RSP_MSG;
      break;

    case QMI_UIM_VERIFY_PIN_MSG_ID:
      *rsp_id = QMI_UIM_SRVC_VERIFY_PIN_RSP_MSG;
      break;

    case QMI_UIM_UNBLOCK_PIN_MSG_ID:
      *rsp_id = QMI_UIM_SRVC_UNBLOCK_PIN_RSP_MSG;
      break;

    case QMI_UIM_CHANGE_PIN_MSG_ID:
      *rsp_id = QMI_UIM_SRVC_CHANGE_PIN_RSP_MSG;
      break;

    case QMI_UIM_DEPERSONALIZATION_MSG_ID:
      *rsp_id = QMI_UIM_SRVC_DEPERSONALIZATION_RSP_MSG;
      break;

    case QMI_UIM_POWER_DOWN_MSG_ID:
      *rsp_id = QMI_UIM_SRVC_POWER_DOWN_RSP_MSG;
      break;

    case QMI_UIM_POWER_UP_MSG_ID:
      *rsp_id = QMI_UIM_SRVC_POWER_UP_RSP_MSG;
      break;

    case QMI_UIM_GET_CARD_STATUS_MSG_ID:
      *rsp_id = QMI_UIM_SRVC_GET_CARD_STATUS_RSP_MSG;
      break;

    case QMI_UIM_AUTHENTICATE_MSG_ID:
      *rsp_id = QMI_UIM_SRVC_AUTHENTICATE_RSP_MSG;
      break;

    case QMI_UIM_CLOSE_SESSION_MSG_ID:
      *rsp_id = QMI_UIM_SRVC_CLOSE_SESSION_RSP_MSG;
      break;

    case QMI_UIM_GET_SERVICE_STATUS_MSG_ID:
      *rsp_id = QMI_UIM_SRVC_GET_SERVICE_STATUS_RSP_MSG;
      break;

    case QMI_UIM_SET_SERVICE_STATUS_MSG_ID:
      *rsp_id = QMI_UIM_SRVC_SET_SERVICE_STATUS_RSP_MSG;
      break;

    case QMI_UIM_CHANGE_PROVISIONING_SESSION_MSG_ID:
	    *rsp_id = QMI_UIM_SRVC_CHANGE_PROV_SESSION_RSP_MSG;
	    break;

    case QMI_UIM_GET_LABEL_MSG_ID:
	    *rsp_id = QMI_UIM_SRVC_GET_LABEL_RSP_MSG;
	    break;

    case QMI_UIM_SEND_APDU_MSG_ID:
      *rsp_id = QMI_UIM_SRVC_SEND_APDU_RSP_MSG;
      break;

    case QMI_UIM_SAP_CONNECTION_MSG_ID:
      *rsp_id = QMI_UIM_SRVC_SAP_CONNECTION_RSP_MSG;
      break;

    case QMI_UIM_SAP_REQUEST_MSG_ID:
      *rsp_id = QMI_UIM_SRVC_SAP_REQUEST_RSP_MSG;
      break;

    case QMI_UIM_LOGICAL_CHANNEL_MSG_ID:
      *rsp_id = QMI_UIM_SRVC_LOGICAL_CHANNEL_RSP_MSG;
      break;

    case QMI_UIM_GET_ATR_MSG_ID:
      *rsp_id = QMI_UIM_SRVC_GET_ATR_RSP_MSG;
      break;

    default:
      return QMI_INTERNAL_ERR;
  }
  return QMI_NO_ERR;
} /* qmi_uim_convert_msg_id */


/*===========================================================================
  FUNCTION  qmi_uim_srvc_async_cb
===========================================================================*/
/*!
@brief
  This is the callback function that will be called by the generic
  services layer to report asynchronous responses.  This function will
  process the asynchronous TLV's and then call the user registered
  functions with the reply data

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
static void qmi_uim_srvc_async_cb
(
  int                       user_handle,
  qmi_service_id_type       service_id,
  unsigned long             msg_id,
  int                       rsp_rc,
  int                       qmi_err_code,
  unsigned char           * reply_msg_data,
  int                       reply_msg_size,
  void                    * srvc_async_cb_data,
  void                    * user_async_cb_fn,
  void                    * user_async_cb_data
)
{
  qmi_uim_user_async_cb_type    user_cb;
  qmi_uim_rsp_data_type         rsp_data;
  unsigned long                 type;
  unsigned long                 length;
  unsigned char               * value_ptr;

  (void) srvc_async_cb_data;

  QMI_DEBUG_MSG_1("qmi_uim_srvc_async_cb: msg_id = 0x%x", msg_id);

  memset((void*)&rsp_data, 0x0, sizeof(rsp_data));

  /* Update the response error codes */
  rsp_data.sys_err_code = rsp_rc;
  rsp_data.qmi_err_code = qmi_err_code;

  /* Update response ID */
  if(qmi_uim_convert_msg_id(msg_id, &rsp_data.rsp_id) < 0)
  {
    QMI_ERR_MSG_2 ("Unknown async reply msg, msg_id=%x, user=%x",
                  (unsigned int) msg_id, (unsigned int) user_handle);
    return;
  }

  /* Loop to read the TLVs, if present */
  while (reply_msg_size > 0)
  {
    if (qmi_util_read_std_tlv (&reply_msg_data,
                               &reply_msg_size,
                               &type,
                               &length,
                               &value_ptr) < 0)
    {
      QMI_ERR_MSG_0 ("qmi_uim_srvc_async_cb: Malformed message, discarding");
      rsp_data.sys_err_code = QMI_SERVICE_ERR;
      rsp_data.qmi_err_code = QMI_SERVICE_ERR_MALFORMED_MSG;
      break;
    }

    switch (rsp_data.rsp_id)
    {
      case QMI_UIM_SRVC_READ_TRANSPARENT_RSP_MSG:
        if (type == QMI_UIM_OPTIONAL_1_RESP_TLV_ID)
        {
          READ_8_BIT_VAL (value_ptr,
                          rsp_data.rsp_data.read_transparent_rsp.sw1);
          READ_8_BIT_VAL (value_ptr,
                          rsp_data.rsp_data.read_transparent_rsp.sw2);
        }
        else if (type == QMI_UIM_OPTIONAL_2_RESP_TLV_ID)
        {
          qmi_uim_read_read_transparent_tlv(value_ptr,
                                            &rsp_data.rsp_data.read_transparent_rsp);
        }
        break;

      case QMI_UIM_SRVC_READ_RECORD_RSP_MSG:
        if (type == QMI_UIM_OPTIONAL_1_RESP_TLV_ID)
        {
          READ_8_BIT_VAL (value_ptr,
                          rsp_data.rsp_data.read_record_rsp.sw1);
          READ_8_BIT_VAL (value_ptr,
                          rsp_data.rsp_data.read_record_rsp.sw2);
        }
        else if (type == QMI_UIM_OPTIONAL_2_RESP_TLV_ID)
        {
          qmi_uim_read_read_record_tlv(value_ptr,
                                       &rsp_data.rsp_data.read_record_rsp);
        }
        break;

      case QMI_UIM_SRVC_WRITE_TRANSPARENT_RSP_MSG:
        if (type == QMI_UIM_OPTIONAL_1_RESP_TLV_ID)
        {
          READ_8_BIT_VAL (value_ptr,
                          rsp_data.rsp_data.write_transparent_rsp.sw1);
          READ_8_BIT_VAL (value_ptr,
                          rsp_data.rsp_data.write_transparent_rsp.sw2);
        }
        break;

      case QMI_UIM_SRVC_WRITE_RECORD_RSP_MSG:
        if (type == QMI_UIM_OPTIONAL_1_RESP_TLV_ID)
        {
          READ_8_BIT_VAL (value_ptr,
                          rsp_data.rsp_data.write_record_rsp.sw1);
          READ_8_BIT_VAL (value_ptr,
                          rsp_data.rsp_data.write_record_rsp.sw2);
        }
        break;

      case QMI_UIM_SRVC_GET_FILE_ATTRIBUTES_RSP_MSG:
        qmi_uim_read_get_file_attributes_tlv(value_ptr,
                                             &type,
                                             &rsp_data.rsp_data.get_file_attributes_rsp);
        break;

      case QMI_UIM_SRVC_REFRESH_REGISTER_RSP_MSG:
        break;

      case QMI_UIM_SRVC_SET_PIN_PROTECTION_RSP_MSG:
        qmi_uim_read_set_pin_protection_tlv(value_ptr,
                                            &type,
                                            &rsp_data.rsp_data.set_pin_protection_rsp);
        break;

      case QMI_UIM_SRVC_VERIFY_PIN_RSP_MSG:
		qmi_uim_read_verify_pin_tlv(value_ptr,
                                    &type,
                                    &rsp_data.rsp_data.verify_pin_rsp);
        break;

      case QMI_UIM_SRVC_UNBLOCK_PIN_RSP_MSG:
		qmi_uim_read_unblock_pin_tlv(value_ptr,
                                     &type,
                                     &rsp_data.rsp_data.unblock_pin_rsp);
        break;

      case QMI_UIM_SRVC_CHANGE_PIN_RSP_MSG:
		qmi_uim_read_change_pin_tlv(value_ptr,
                                    &type,
                                    &rsp_data.rsp_data.change_pin_rsp);
        break;

      case QMI_UIM_SRVC_DEPERSONALIZATION_RSP_MSG:
        if (type == QMI_UIM_OPTIONAL_1_RESP_TLV_ID)
        {
          READ_8_BIT_VAL (value_ptr,
                          rsp_data.rsp_data.depersonalization_rsp.num_retries);
          READ_8_BIT_VAL (value_ptr,
                          rsp_data.rsp_data.depersonalization_rsp.num_unblock_retries);
        }
        break;

      case QMI_UIM_SRVC_GET_CARD_STATUS_RSP_MSG:
        if (type == QMI_UIM_OPTIONAL_1_RESP_TLV_ID)
        {
          qmi_uim_read_card_status_tlv(value_ptr,
                                     &rsp_data.rsp_data.get_card_status_rsp.card_status);
        }
        else if (type == QMI_UIM_OPTIONAL_3_RESP_TLV_ID)
        {
          qmi_uim_read_card_status_validity_tlv(value_ptr,
                                                &rsp_data.rsp_data.get_card_status_rsp.card_status_validity);
        }
        break;

      case QMI_UIM_SRVC_CLOSE_SESSION_RSP_MSG:
        break;

      case QMI_UIM_SRVC_AUTHENTICATE_RSP_MSG:
        if (type == QMI_UIM_OPTIONAL_1_RESP_TLV_ID)
        {
          READ_8_BIT_VAL (value_ptr, rsp_data.rsp_data.authenticate_rsp.sw1);
          READ_8_BIT_VAL (value_ptr, rsp_data.rsp_data.authenticate_rsp.sw2);
        }
        else if (type == QMI_UIM_OPTIONAL_2_RESP_TLV_ID)
        {
          qmi_uim_read_auth_response_tlv(value_ptr,
                                         &rsp_data.rsp_data.authenticate_rsp);
        }
        break;

      case QMI_UIM_SRVC_GET_SERVICE_STATUS_RSP_MSG:
        if (type == QMI_UIM_OPTIONAL_1_RESP_TLV_ID)
        {
          READ_8_BIT_VAL (value_ptr,
                          rsp_data.rsp_data.get_service_status_rsp.fdn_status);
          rsp_data.rsp_data.get_service_status_rsp.fdn_status_valid = QMI_UIM_TRUE;
        }
        else if (type == QMI_UIM_OPTIONAL_2_RESP_TLV_ID)
        {
          READ_8_BIT_VAL (value_ptr,
                          rsp_data.rsp_data.get_service_status_rsp.hidden_key_status);
          rsp_data.rsp_data.get_service_status_rsp.hidden_key_status_valid = QMI_UIM_TRUE;
        }
        else if (type == QMI_UIM_OPTIONAL_3_RESP_TLV_ID)
        {
          READ_8_BIT_VAL (value_ptr,
                          rsp_data.rsp_data.get_service_status_rsp.index);
          rsp_data.rsp_data.get_service_status_rsp.index_valid = QMI_UIM_TRUE;
        }
        else if (type == QMI_UIM_OPTIONAL_4_RESP_TLV_ID)
        {
          READ_8_BIT_VAL (value_ptr,
                          rsp_data.rsp_data.get_service_status_rsp.esn_status);
          rsp_data.rsp_data.get_service_status_rsp.esn_status_valid = QMI_UIM_TRUE;
        }
        else if (type == QMI_UIM_OPTIONAL_5_RESP_TLV_ID)
        {
          READ_8_BIT_VAL (value_ptr,
                          rsp_data.rsp_data.get_service_status_rsp.acl_status);
          rsp_data.rsp_data.get_service_status_rsp.acl_status_valid = QMI_UIM_TRUE;
        }
        break;

      case QMI_UIM_SRVC_CHANGE_PROV_SESSION_RSP_MSG:
        break;

      case QMI_UIM_SRVC_GET_LABEL_RSP_MSG:
        if (type == QMI_UIM_OPTIONAL_1_RESP_TLV_ID)
        {
          READ_8_BIT_VAL (value_ptr, rsp_data.rsp_data.get_label_rsp.label_len);
          if(rsp_data.rsp_data.get_label_rsp.label_len <= QMI_UIM_MAX_LABEL_LEN)
          {
            memcpy (rsp_data.rsp_data.get_label_rsp.label_value,
                   (void *)value_ptr,
    	           rsp_data.rsp_data.get_label_rsp.label_len);
          }
        }
        break;

      case QMI_UIM_SRVC_SEND_APDU_RSP_MSG:
        if (type == QMI_UIM_OPTIONAL_1_RESP_TLV_ID)
        {
          qmi_uim_read_send_apdu_tlv(value_ptr,
                                     &rsp_data.rsp_data.send_apdu_rsp);
        }
        break;

      case QMI_UIM_SRVC_SAP_CONNECTION_RSP_MSG:
        if (type == QMI_UIM_OPTIONAL_1_RESP_TLV_ID)
        {
          READ_8_BIT_VAL (value_ptr, rsp_data.rsp_data.sap_connection_rsp.connection_status);
        }
        break;

      case QMI_UIM_SRVC_SAP_REQUEST_RSP_MSG:
        qmi_uim_read_sap_response_tlv(type,
                                      value_ptr,
                                      &rsp_data.rsp_data.sap_response_rsp);
        break;

      case QMI_UIM_SRVC_LOGICAL_CHANNEL_RSP_MSG:
        if (type == QMI_UIM_OPTIONAL_1_RESP_TLV_ID)
        {
          READ_8_BIT_VAL (value_ptr, rsp_data.rsp_data.logical_channel_rsp.channel_id);
        }
        break;

      case QMI_UIM_SRVC_GET_ATR_RSP_MSG:
        if (type == QMI_UIM_OPTIONAL_1_RESP_TLV_ID)
        {
          qmi_uim_read_get_atr_tlv(value_ptr, &rsp_data.rsp_data.get_atr_rsp);
        }
        break;

      default:
        QMI_ERR_MSG_1 ("qmi_uim_srvc_async_cb: unknown TLV type = %x",
                       (unsigned int)type);
        break;
    }
  }

  /* If the reply indicates that the command was aborted, don't
  ** call the user callback */
  if ((rsp_rc == QMI_SERVICE_ERR) &&
      (qmi_err_code == QMI_SERVICE_ERR_ABORTED))
  {
    QMI_DEBUG_MSG_2 ("Discarding aborted reply, msg_id=%x, user=%x",(unsigned int) msg_id,
                                                                  (unsigned int) user_handle);
  }
  else
  {
    /*lint -e{611} */
    user_cb = (qmi_uim_user_async_cb_type) user_async_cb_fn;

    /* Call the user callback */
    user_cb (user_handle,
             service_id,
             &rsp_data,
             user_async_cb_data);
  }

  /* Free up any memory allocated */
  qmi_uim_free_tlv_data(&rsp_data);

} /* qmi_uim_srvc_async_cb */


/*---------------------------------------------------------------------------
  Init/Deinit functions
---------------------------------------------------------------------------*/

/*===========================================================================
  FUNCTION  qmi_uim_srvc_init
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
int qmi_uim_srvc_init (void)
{
  int  rc = QMI_NO_ERR;
  if (!uim_service_initialized)
  {

    rc = qmi_service_set_srvc_functions (QMI_UIM_SERVICE,
                                         qmi_uim_srvc_indication_cb);
    if (rc != QMI_NO_ERR)
    {
      QMI_ERR_MSG_1("qmi_uim_srvc_init: set srvc functions returns err=%d\n",rc);
    }
    else
    {
      QMI_DEBUG_MSG_0("qmi_uim_srvc_init: UIM successfully initialized");
      uim_service_initialized = TRUE;
    }
  }
  else
  {
    QMI_ERR_MSG_0 ("qmi_uim_srvc_init: Init failed, UIM already initialized");
  }
  return rc;
} /* qmi_uim_srvc_init */


/*===========================================================================
  FUNCTION  qmi_uim_srvc_release
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
int qmi_uim_srvc_release (void)
{
  int  rc = QMI_NO_ERR;
  if (uim_service_initialized)
  {

    rc = qmi_service_set_srvc_functions (QMI_UIM_SERVICE,NULL);
    if (rc != QMI_NO_ERR)
    {
      QMI_ERR_MSG_1("qmi_uim_srvc_release: set srvc functions returns err=%d\n",rc);
    }
    else
    {
      QMI_DEBUG_MSG_0("qmi_nas_srvc_release: UIM successfully released");
      uim_service_initialized = FALSE;
    }
  }
  else
  {
    QMI_ERR_MSG_0 ("qmi_nas_srvc_release: Release failed, UIM not initialized");
  }
  return rc;
} /* qmi_uim_srvc_release */


/*===========================================================================
  FUNCTION  qmi_uim_srvc_init_client
===========================================================================*/
/*!
@brief
  This function is called to initialize the UIM service.  This function
  must be called prior to calling any other UIM service functions.
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
qmi_client_handle_type qmi_uim_srvc_init_client
(
  const char                    * dev_id,
  qmi_uim_indication_hdlr_type    user_rx_ind_msg_hdlr,
  void                          * user_rx_ind_msg_hdlr_user_data,
  int                           * qmi_err_code
)
{
  qmi_client_handle_type client_handle;
  qmi_connection_id_type conn_id;

  if ((conn_id = QMI_PLATFORM_DEV_NAME_TO_CONN_ID(dev_id)) == QMI_CONN_ID_INVALID)
  {
    return QMI_INTERNAL_ERR;
  }

  /* Call common service layer initialization function */
  /*lint -e{611} */
  client_handle =  qmi_service_init (conn_id,
                                   QMI_UIM_SERVICE,
                                   (void *) user_rx_ind_msg_hdlr,
                                   user_rx_ind_msg_hdlr_user_data,
                                   qmi_err_code);

  return client_handle;
} /* qmi_uim_srvc_init_client */


/*===========================================================================
  FUNCTION  qmi_uim_srvc_release_client
===========================================================================*/
/*!
@brief
  This function is called to release a client created by the
  qmi_uim_srvc_init_client() function.  This function should be called
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
qmi_uim_srvc_release_client
(
  int        user_handle,
  int      * qmi_err_code
)
{
  int rc;
  rc = qmi_service_release (user_handle, qmi_err_code);
  return rc;
} /* qmi_uim_srvc_release_client */


/*---------------------------------------------------------------------------
   Command handling functions from the Client
---------------------------------------------------------------------------*/

/*===========================================================================
  FUNCTION  qmi_uim_reset
===========================================================================*/
/*!
@brief
  Resets UIM service.  If the user_cb function pointer is set to NULL,
  then this function will be invoked synchronously, otherwise it will
  be invoked asynchronously.

@return
  In the synchronous case, if return code < 0, the operation failed.  In
  the failure case, if the return code is QMI_SERVICE_ERR, then the
  qmi_err_code value in rsp_data will give you the QMI error reason.
  Otherwise, qmi_err_code will have meaningless data.

  In the asynchronous case, if the return code < 0, the operation failed and
  you will not get an asynchronous response.  If the operation doesn't fail
  (return code >=0), the returned value will be a transaction handle which
  can be used to cancel the transaction via the qmi_uim_abort() command.

@note

  - Dependencies
    - qmi_uim_srvc_init_client() must be called before calling this.

  - Side Effects
    - Resets UIM service
*/
/*=========================================================================*/
int qmi_uim_reset
(
  int                           user_handle,
  qmi_uim_user_async_cb_type    user_cb,
  void                        * user_data,
  int                         * qmi_err_code
)
{
  unsigned char msg[QMI_UIM_STD_MSG_SIZE];
  int           msg_size;
  int rc;

  /* Set message size to the complete buffer minus the header size */
  msg_size = QMI_SRVC_PDU_SIZE(QMI_UIM_STD_MSG_SIZE);

  /* Send message, do async if user_cb is non-NULL, sync otherwise */
  if (user_cb)
  {
    *qmi_err_code = QMI_SERVICE_ERR_NONE;
    /*lint -e{611} */
    rc = qmi_service_send_msg_async (user_handle,
                                     QMI_UIM_SERVICE,
                                     QMI_UIM_RESET_MSG_ID,
                                     QMI_SRVC_PDU_PTR(msg),
                                     0,
                                     qmi_uim_srvc_async_cb,
                                     NULL,
                                     (void *)user_cb,
                                     user_data);
  }
  else
  {
    rc = qmi_service_send_msg_sync (user_handle,
                                    QMI_UIM_SERVICE,
                                    QMI_UIM_RESET_MSG_ID,
                                    QMI_SRVC_PDU_PTR(msg),
                                    (int)QMI_SRVC_PDU_SIZE(QMI_UIM_STD_MSG_SIZE) - msg_size,
                                    msg,
                                    &msg_size,
                                    QMI_UIM_STD_MSG_SIZE,
                                    QMI_SYNC_MSG_DEFAULT_TIMEOUT,
                                    qmi_err_code);
  }
  return rc;
} /* qmi_uim_reset */


/*===========================================================================
  FUNCTION  qmi_uim_read_transparent
===========================================================================*/
/*!
@brief
  Reads a transparent file from the card.  If the user_cb function
  pointer is set to NULL, this function will be invoked synchronously.
  Otherwise it will be invoked asynchronously.

@return
  In the synchronous case, if return code < 0, the operation failed.  In
  the failure case, if the return code is QMI_SERVICE_ERR, then the
  qmi_err_code value in rsp_data will give you the QMI error reason.
  Otherwise, qmi_err_code will have meaningless data.

  In the asynchronous case, if the return code < 0, the operation failed and
  you will not get an asynchronous response.  If the operation doesn't fail
  (return code >=0), the returned value will be a transaction handle which
  can be used to cancel the transaction via the qmi_uim_abort() command.

@note
  - Dependencies
    - qmi_uim_srvc_init_client() must be called before calling this.

  - Side Effects
    - None
*/
/*=========================================================================*/
int qmi_uim_read_transparent
(
  int                                           client_handle,
  const qmi_uim_read_transparent_params_type  * params,
  qmi_uim_user_async_cb_type                    user_cb,
  void                                        * user_data,
  qmi_uim_rsp_data_type                       * rsp_data
)
{
  unsigned char    *msg = NULL;
  int               msg_size;
  unsigned char    *tmp_msg_ptr;
  unsigned int      max_msg_size = 0;
  int rc;

  if (!params)
  {
    return QMI_SERVICE_ERR;
  }

  if (!user_cb)
  {
    if (!rsp_data)
    {
      return QMI_SERVICE_ERR;
    }
  }

  /* Calculate max buffer size - note that this max size should satify
     as both request & response buffers (read data + headers) */
  max_msg_size = (params->length <= 0) ? QMI_MAX_MSG_SIZE :
                 ((unsigned int)params->length + QMI_UIM_STD_MSG_SIZE);

  /* Allocate buffer for the message dynamically since it could be very large */
  msg = (unsigned char *) malloc (max_msg_size);
  if (!msg)
  {
    QMI_ERR_MSG_0 ("Dynamic memory allocation failed for qmi_uim_read_transparent\n");
    return QMI_INTERNAL_ERR;
  }

  QMI_DEBUG_MSG_1("qmi_uim_read_transparent: %d bytes allocated for msg", max_msg_size);

  /* Set tmp_msg_ptr to beginning of message-specific TLV portion of
  ** message buffer */
  tmp_msg_ptr = QMI_SRVC_PDU_PTR(msg);

  /* Set message size to the complete buffer minus the header size */
  msg_size = (int)QMI_SRVC_PDU_SIZE(max_msg_size);

  /* Construct TLV for Read Transparent request */
  if (qmi_uim_write_read_transparent_tlv (&tmp_msg_ptr,
                                          &msg_size,
                                          params) < 0)
  {
    free(msg);
    return QMI_INTERNAL_ERR;
  }

  /* Send message, do async if user_cb is non-NULL, sync otherwise */
  if (user_cb)
  {
    /*lint -e{611} */
    rc = qmi_service_send_msg_async (client_handle,
                                     QMI_UIM_SERVICE,
                                     QMI_UIM_READ_TRANSPARENT_MSG_ID,
                                     QMI_SRVC_PDU_PTR(msg),
                                     (int)QMI_SRVC_PDU_SIZE(max_msg_size) - msg_size,
                                     qmi_uim_srvc_async_cb,
                                     NULL,
                                     (void *)user_cb,
                                     user_data);
  }
  else
  {
    rc = qmi_service_send_msg_sync (client_handle,
                                    QMI_UIM_SERVICE,
                                    QMI_UIM_READ_TRANSPARENT_MSG_ID,
                                    QMI_SRVC_PDU_PTR(msg),
                                    (int)QMI_SRVC_PDU_SIZE(max_msg_size) - msg_size,
                                    msg,
                                    &msg_size,
                                    (int)max_msg_size,
                                    QMI_SYNC_MSG_DEFAULT_TIMEOUT,
                                    &rsp_data->qmi_err_code);
    /* Prepare the response data */
    if (rc == QMI_NO_ERR)
    {
      rsp_data->rsp_id = QMI_UIM_SRVC_READ_TRANSPARENT_RSP_MSG;
      memset (&rsp_data->rsp_data, 0, sizeof(qmi_uim_read_transparent_rsp_type));
      if (qmi_uim_handle_read_rsp (msg, msg_size, rsp_data) < 0)
      {
        QMI_ERR_MSG_0 ("qmi_uim_read_transparent: qmi_uim_handle_read_rsp returned error");
        rc = QMI_INTERNAL_ERR;
        rsp_data->sys_err_code = QMI_INTERNAL_ERR;
      }
    }
  }

  free(msg);
  return rc;
} /* qmi_uim_read_transparent */


/*===========================================================================
  FUNCTION  qmi_uim_read_record
===========================================================================*/
/*!
@brief
  Read a record from a linear/cyclic file from the card.
  If the user_cb function pointer is set to NULL, this function will
  be invoked synchronously. Otherwise it will be invoked asynchronously.

@return
  In the synchronous case, if return code < 0, the operation failed.  In
  the failure case, if the return code is QMI_SERVICE_ERR, then the
  qmi_err_code value in rsp_data will give you the QMI error reason.
  Otherwise, qmi_err_code will have meaningless data.

  In the asynchronous case, if the return code < 0, the operation failed and
  you will not get an asynchronous response.  If the operation doesn't fail
  (return code >= 0), the returned value will be a transaction handle which
  can be used to cancel the transaction via the qmi_uim_abort() command.

@note
  - Dependencies
    - qmi_uim_srvc_init_client() must be called before calling this.

  - Side Effects
    - None
*/
/*=========================================================================*/
int qmi_uim_read_record
(
  int                                           client_handle,
  const qmi_uim_read_record_params_type       * params,
  qmi_uim_user_async_cb_type                    user_cb,
  void                                        * user_data,
  qmi_uim_rsp_data_type                       * rsp_data
)
{
  unsigned char     msg[QMI_UIM_STD_MSG_SIZE];
  int               msg_size;
  unsigned char    *tmp_msg_ptr;
  int rc;

  if (!params)
  {
    return QMI_SERVICE_ERR;
  }

  if (!user_cb)
  {
    if (!rsp_data)
    {
      return QMI_SERVICE_ERR;
    }
  }

  /* Set tmp_msg_ptr to beginning of message-specific TLV portion of
  ** message buffer */
  tmp_msg_ptr = QMI_SRVC_PDU_PTR(msg);

  /* Set message size to the complete buffer minus the header size */
  msg_size = QMI_SRVC_PDU_SIZE(QMI_UIM_STD_MSG_SIZE);

  /* Construct TLV for Read Record request */
  if (qmi_uim_write_read_record_tlv (&tmp_msg_ptr,
                                     &msg_size,
                                     params) < 0)
  {
    return QMI_INTERNAL_ERR;
  }

  /* Send message, do async if user_cb is non-NULL, sync otherwise */
  if (user_cb)
  {
    /*lint -e{611} */
    rc = qmi_service_send_msg_async (client_handle,
                                     QMI_UIM_SERVICE,
                                     QMI_UIM_READ_RECORD_MSG_ID,
                                     QMI_SRVC_PDU_PTR(msg),
                                     (int)QMI_SRVC_PDU_SIZE(QMI_UIM_STD_MSG_SIZE) - msg_size,
                                     qmi_uim_srvc_async_cb,
                                     NULL,
                                     (void *)user_cb,
                                     user_data);
  }
  else
  {
    rc = qmi_service_send_msg_sync (client_handle,
                                    QMI_UIM_SERVICE,
                                    QMI_UIM_READ_RECORD_MSG_ID,
                                    QMI_SRVC_PDU_PTR(msg),
                                    (int)QMI_SRVC_PDU_SIZE(QMI_UIM_STD_MSG_SIZE) - msg_size,
                                    msg,
                                    &msg_size,
                                    QMI_UIM_STD_MSG_SIZE,
                                    QMI_SYNC_MSG_DEFAULT_TIMEOUT,
                                    &rsp_data->qmi_err_code);
    /* Prepare the response data */
    if (rc == QMI_NO_ERR)
    {
      rsp_data->rsp_id = QMI_UIM_SRVC_READ_RECORD_RSP_MSG;
      memset (&rsp_data->rsp_data, 0, sizeof(qmi_uim_read_record_rsp_type));
      if (qmi_uim_handle_read_rsp (msg, msg_size, rsp_data) < 0)
      {
        QMI_ERR_MSG_0 ("qmi_uim_read_record: qmi_uim_handle_read_rsp returned error");
        rc = QMI_INTERNAL_ERR;
        rsp_data->sys_err_code = QMI_INTERNAL_ERR;
      }
    }
  }
  return rc;
} /* qmi_uim_read_record */


/*===========================================================================
  FUNCTION  qmi_uim_write_transparent
===========================================================================*/
/*!
@brief
  Writes a transparent file to the card.  If the user_cb function
  pointer is set to NULL, this function will be invoked synchronously.
  Otherwise it will be invoked asynchronously.

@return
  In the synchronous case, if return code < 0, the operation failed.  In
  the failure case, if the return code is QMI_SERVICE_ERR, then the
  qmi_err_code value will give you the QMI error reason.  Otherwise,
  qmi_err_code will have meaningless data.

  In the asynchronous case, if the return code < 0, the operation failed and
  you will not get an asynchronous response.  If the operation doesn't fail
  (return code >=0), the returned value will be a transaction handle which
  can be used to cancel the transaction via the qmi_uim_abort() command.

@note
  - Dependencies
    - qmi_uim_srvc_init_client() must be called before calling this.

  - Side Effects
    - None
*/
/*=========================================================================*/
int qmi_uim_write_transparent
(
  int                                           client_handle,
  const qmi_uim_write_transparent_params_type * params,
  qmi_uim_user_async_cb_type                    user_cb,
  void                                        * user_data,
  qmi_uim_rsp_data_type                       * rsp_data
)
{
  // Todo - change msg below to handle size dynamically (more than 2K bytes)
  unsigned char     msg[QMI_UIM_STD_MSG_SIZE];
  int               msg_size;
  unsigned char    *tmp_msg_ptr;
  int               rc;

  if (!params)
  {
    return QMI_SERVICE_ERR;
  }

  if (!user_cb)
  {
    if (!rsp_data)
    {
      return QMI_SERVICE_ERR;
    }
  }

  /* Set tmp_msg_ptr to beginning of message-specific TLV portion of
  ** message buffer */
  tmp_msg_ptr = QMI_SRVC_PDU_PTR(msg);

  /* Set message size to the complete buffer minus the header size */
  msg_size = QMI_SRVC_PDU_SIZE(QMI_UIM_STD_MSG_SIZE);

  /* Construct TLV for Write Transparent request */
  if (qmi_uim_write_write_transparent_tlv (&tmp_msg_ptr,
                                           &msg_size,
                                           params) < 0)
  {
    return QMI_INTERNAL_ERR;
  }

  /* Send message, do async if user_cb is non-NULL, sync otherwise */
  if (user_cb)
  {
    /*lint -e{611} */
    rc = qmi_service_send_msg_async (client_handle,
                                     QMI_UIM_SERVICE,
                                     QMI_UIM_WRITE_TRANSPARENT_MSG_ID,
                                     QMI_SRVC_PDU_PTR(msg),
                                     (int)QMI_SRVC_PDU_SIZE(QMI_UIM_STD_MSG_SIZE) - msg_size,
                                     qmi_uim_srvc_async_cb,
                                     NULL,
                                     (void *)user_cb,
                                     user_data);
  }
  else
  {
    rc = qmi_service_send_msg_sync (client_handle,
                                    QMI_UIM_SERVICE,
                                    QMI_UIM_WRITE_TRANSPARENT_MSG_ID,
                                    QMI_SRVC_PDU_PTR(msg),
                                    (int)QMI_SRVC_PDU_SIZE(QMI_UIM_STD_MSG_SIZE) - msg_size,
                                    msg,
                                    &msg_size,
                                    QMI_UIM_STD_MSG_SIZE,
                                    QMI_SYNC_MSG_DEFAULT_TIMEOUT,
                                    &rsp_data->qmi_err_code);
    /* Prepare the response data */
    if (rc == QMI_NO_ERR)
    {
      rsp_data->rsp_id = QMI_UIM_SRVC_WRITE_TRANSPARENT_RSP_MSG;
      memset (&rsp_data->rsp_data, 0, sizeof(qmi_uim_write_transparent_rsp_type));
      if (qmi_uim_handle_write_rsp (msg, msg_size, rsp_data) < 0)
      {
        QMI_ERR_MSG_0 ("qmi_uim_write_transparent: qmi_uim_handle_write_rsp returned error");
        rc = QMI_INTERNAL_ERR;
        rsp_data->sys_err_code = QMI_INTERNAL_ERR;
      }
    }
  }
  return rc;
} /* qmi_uim_write_transparent */


/*===========================================================================
  FUNCTION  qmi_uim_write_record
===========================================================================*/
/*!
@brief
  Writes a record to a linear/cyclic file from the card.  If the user_cb
  function pointer is set to NULL, this function will be invoked synchronously.
  Otherwise it will be invoked asynchronously.

@return
  In the synchronous case, if return code < 0, the operation failed.  In
  the failure case, if the return code is QMI_SERVICE_ERR, then the
  qmi_err_code value will give you the QMI error reason.  Otherwise,
  qmi_err_code will have meaningless data.

  In the asynchronous case, if the return code < 0, the operation failed and
  you will not get an asynchronous response.  If the operation doesn't fail
  (return code >=0), the returned value will be a transaction handle which
  can be used to cancel the transaction via the qmi_uim_abort() command.

@note
  - Dependencies
    - qmi_uim_srvc_init_client() must be called before calling this.

  - Side Effects
    - None
*/
/*=========================================================================*/
int qmi_uim_write_record
(
  int                                           client_handle,
  const qmi_uim_write_record_params_type      * params,
  qmi_uim_user_async_cb_type                    user_cb,
  void                                        * user_data,
  qmi_uim_rsp_data_type                       * rsp_data
)
{
  // Todo - change msg below to handle size dynamically (more than 2K bytes)
  unsigned char     msg[QMI_UIM_STD_MSG_SIZE];
  int               msg_size;
  unsigned char    *tmp_msg_ptr;
  int rc;

  if (!params)
  {
    return QMI_SERVICE_ERR;
  }

  if (!user_cb)
  {
    if (!rsp_data)
    {
      return QMI_SERVICE_ERR;
    }
  }

  /* Set tmp_msg_ptr to beginning of message-specific TLV portion of
  ** message buffer */
  tmp_msg_ptr = QMI_SRVC_PDU_PTR(msg);

  /* Set message size to the complete buffer minus the header size */
  msg_size = QMI_SRVC_PDU_SIZE(QMI_UIM_STD_MSG_SIZE);

  /* Construct TLV for Write Record request */
  if (qmi_uim_write_write_record_tlv (&tmp_msg_ptr,
                                      &msg_size,
                                      params) < 0)
  {
    return QMI_INTERNAL_ERR;
  }

  /* Send message, do async if user_cb is non-NULL, sync otherwise */
  if (user_cb)
  {
    /*lint -e{611} */
    rc = qmi_service_send_msg_async (client_handle,
                                     QMI_UIM_SERVICE,
                                     QMI_UIM_WRITE_RECORD_MSG_ID,
                                     QMI_SRVC_PDU_PTR(msg),
                                     (int)QMI_SRVC_PDU_SIZE(QMI_UIM_STD_MSG_SIZE) - msg_size,
                                     qmi_uim_srvc_async_cb,
                                     NULL,
                                     (void *)user_cb,
                                     user_data);
  }
  else
  {
    rc = qmi_service_send_msg_sync (client_handle,
                                    QMI_UIM_SERVICE,
                                    QMI_UIM_WRITE_RECORD_MSG_ID,
                                    QMI_SRVC_PDU_PTR(msg),
                                    (int)QMI_SRVC_PDU_SIZE(QMI_UIM_STD_MSG_SIZE) - msg_size,
                                    msg,
                                    &msg_size,
                                    QMI_UIM_STD_MSG_SIZE,
                                    QMI_SYNC_MSG_DEFAULT_TIMEOUT,
                                    &rsp_data->qmi_err_code);
    /* Prepare the response data */
    if (rc == QMI_NO_ERR)
    {
      rsp_data->rsp_id = QMI_UIM_SRVC_WRITE_RECORD_RSP_MSG;
      memset (&rsp_data->rsp_data, 0, sizeof(qmi_uim_write_record_rsp_type));
      if (qmi_uim_handle_write_rsp (msg, msg_size, rsp_data) < 0)
      {
        QMI_ERR_MSG_0 ("qmi_uim_write_record: qmi_uim_handle_write_rsp returned error");
        rc = QMI_INTERNAL_ERR;
        rsp_data->sys_err_code = QMI_INTERNAL_ERR;
      }
    }
  }
  return rc;
} /* qmi_uim_write_record */


/*===========================================================================
  FUNCTION  qmi_uim_get_file_attributes
===========================================================================*/
/*!
@brief
  Gets the file Attributes for a file on the card.  If the user_cb
  function pointer is set to NULL, this function will be invoked synchronously.
  Otherwise it will be invoked asynchronously.

@return
  In the synchronous case, if return code < 0, the operation failed.  In
  the failure case, if the return code is QMI_SERVICE_ERR, then the
  qmi_err_code value will give you the QMI error reason.  Otherwise,
  qmi_err_code will have meaningless data.

  In the asynchronous case, if the return code < 0, the operation failed and
  you will not get an asynchronous response.  If the operation doesn't fail
  (return code >=0), the returned value will be a transaction handle which
  can be used to cancel the transaction via the qmi_uim_abort() command.

@note
  - Dependencies
    - qmi_uim_srvc_init_client() must be called before calling this.

  - Side Effects
    - None
*/
/*=========================================================================*/
int qmi_uim_get_file_attributes
(
  int                                                 client_handle,
  const qmi_uim_get_file_attributes_params_type     * params,
  qmi_uim_user_async_cb_type                          user_cb,
  void                                              * user_data,
  qmi_uim_rsp_data_type                             * rsp_data
)
{
  unsigned char     msg[QMI_UIM_STD_MSG_SIZE];
  int               msg_size;
  unsigned char    *tmp_msg_ptr;
  int rc;

  if (!params)
  {
    return QMI_SERVICE_ERR;
  }

  if (!user_cb)
  {
    if (!rsp_data)
    {
      return QMI_SERVICE_ERR;
    }
  }

  /* Set tmp_msg_ptr to beginning of message-specific TLV portion of
  ** message buffer */
  tmp_msg_ptr = QMI_SRVC_PDU_PTR(msg);

  /* Set message size to the complete buffer minus the header size */
  msg_size = QMI_SRVC_PDU_SIZE(QMI_UIM_STD_MSG_SIZE);

  /* Construct TLV for Get file attributes request */
  if (qmi_uim_write_get_file_attributes_tlv (&tmp_msg_ptr,
                                             &msg_size,
                                             params) < 0)
  {
    return QMI_INTERNAL_ERR;
  }

  /* Send message, do async if user_cb is non-NULL, sync otherwise */
  if (user_cb)
  {
    /*lint -e{611} */
    rc = qmi_service_send_msg_async (client_handle,
                                     QMI_UIM_SERVICE,
                                     QMI_UIM_GET_FILE_ATTRIBUTES_MSG_ID,
                                     QMI_SRVC_PDU_PTR(msg),
                                     (int)QMI_SRVC_PDU_SIZE(QMI_UIM_STD_MSG_SIZE) - msg_size,
                                     qmi_uim_srvc_async_cb,
                                     NULL,
                                     (void *)user_cb,
                                     user_data);
  }
  else
  {
    rc = qmi_service_send_msg_sync (client_handle,
                                    QMI_UIM_SERVICE,
                                    QMI_UIM_GET_FILE_ATTRIBUTES_MSG_ID,
                                    QMI_SRVC_PDU_PTR(msg),
                                    (int)QMI_SRVC_PDU_SIZE(QMI_UIM_STD_MSG_SIZE) - msg_size,
                                    msg,
                                    &msg_size,
                                    QMI_UIM_STD_MSG_SIZE,
                                    QMI_SYNC_MSG_DEFAULT_TIMEOUT,
                                    &rsp_data->qmi_err_code);
    /* Prepare the response data */
    if (rc == QMI_NO_ERR)
    {
      rsp_data->rsp_id = QMI_UIM_SRVC_GET_FILE_ATTRIBUTES_RSP_MSG;
      memset (&rsp_data->rsp_data, 0, sizeof(qmi_uim_get_file_attributes_rsp_type));
      if (qmi_uim_handle_get_file_attributes_rsp (msg, msg_size, rsp_data) < 0)
      {
        QMI_ERR_MSG_0 ("qmi_uim_get_file_attributes: qmi_uim_handle_get_file_attributes_rsp returned error");
        rc = QMI_INTERNAL_ERR;
        rsp_data->sys_err_code = QMI_INTERNAL_ERR;
      }
    }
  }
  return rc;
} /* qmi_uim_get_file_attributes */


/*===========================================================================
  FUNCTION  qmi_uim_refresh_register
===========================================================================*/
/*!
@brief
  Registers for file change notifications triggered by the card.
  This function is supported only synchronously.

@return
  If return code < 0, the operation failed. In the failure case, if the
  return code is QMI_SERVICE_ERR, then the qmi_err_code value will give you
  the QMI error reason.  Otherwise, qmi_err_code will have meaningless data.

@note
  - Dependencies
    - qmi_uim_srvc_init_client() must be called before calling this.

  - Side Effects
    - None
*/
/*=========================================================================*/
int qmi_uim_refresh_register
(
  int                                               client_handle,
  const qmi_uim_refresh_register_params_type      * params,
  qmi_uim_user_async_cb_type                        user_cb,
  void                                            * user_data,
  qmi_uim_rsp_data_type                           * rsp_data
)
{
  unsigned char    *msg_ptr = NULL;
  int               msg_size;
  unsigned int      max_msg_size = 0;
  unsigned char    *tmp_msg_ptr;
  unsigned char    *param_ptr = NULL;
  unsigned int      param_size = 0;
  int               rc;

  if (!params)
  {
    return QMI_SERVICE_ERR;
  }

  if (!user_cb)
  {
    if (!rsp_data)
    {
      return QMI_SERVICE_ERR;
    }
  }

  /* Calculate max buffer size needed - TLV data + headers) */
  max_msg_size = QMI_UIM_STD_MSG_SIZE +
                   (QMI_UIM_FILE_ID_MAX_TLV_SIZE * (unsigned int)params->num_files);

  /* Allocate buffer for the message dynamically */
  msg_ptr = (unsigned char *) malloc ((size_t)max_msg_size);
  if (msg_ptr == NULL)
  {
    QMI_ERR_MSG_0 ("Dynamic memory allocation failed for qmi_uim_refresh_register\n");
    return QMI_INTERNAL_ERR;
  }

  QMI_DEBUG_MSG_1("qmi_uim_refresh_register: %d bytes allocated for msg", max_msg_size);

  /* Set tmp_msg_ptr to beginning of message-specific TLV portion of
  ** message buffer */
  tmp_msg_ptr = QMI_SRVC_PDU_PTR(msg_ptr);

  /* Set message size to the complete buffer minus the header size */
  msg_size = (int)QMI_SRVC_PDU_SIZE(max_msg_size);

  /* Allocate memory for all TLVs needed for refresh register */
  param_size = QMI_UIM_REFRESH_REGISTER_ALL_TLV_SIZE +
               (QMI_UIM_FILE_ID_MAX_TLV_SIZE * (unsigned int)params->num_files);
  param_ptr = (unsigned char *) malloc (param_size);
  if (param_ptr == NULL)
  {
    rc = QMI_INTERNAL_ERR;
    goto refresh_reg_ret;
  }

  /* Construct TLV for Refresh register request */
  if (qmi_uim_write_refresh_register_tlv (&tmp_msg_ptr,
                                          &msg_size,
                                          params,
                                          param_ptr,
                                          param_size) < 0)
  {
    rc = QMI_INTERNAL_ERR;
    goto refresh_reg_ret;
  }

  /* Send message, do async if user_cb is non-NULL, sync otherwise */
  if (user_cb)
  {
    /*lint -e{611} */
    rc = qmi_service_send_msg_async (client_handle,
                                     QMI_UIM_SERVICE,
                                     QMI_UIM_REFRESH_REGISTER_MSG_ID,
                                     QMI_SRVC_PDU_PTR(msg_ptr),
                                     (int)QMI_SRVC_PDU_SIZE(max_msg_size) - msg_size,
                                     qmi_uim_srvc_async_cb,
                                     NULL,
                                     (void *)user_cb,
                                     user_data);
  }
  else
  {
    rc = qmi_service_send_msg_sync (client_handle,
                                    QMI_UIM_SERVICE,
                                    QMI_UIM_REFRESH_REGISTER_MSG_ID,
                                    QMI_SRVC_PDU_PTR(msg_ptr),
                                    (int)QMI_SRVC_PDU_SIZE(max_msg_size) - msg_size,
                                    msg_ptr,
                                    &msg_size,
                                    (int)max_msg_size,
                                    QMI_SYNC_MSG_DEFAULT_TIMEOUT,
                                    &rsp_data->qmi_err_code);
  }

refresh_reg_ret:
  /* Free memory allocated above for TLVs */
  if (param_ptr)
  {
    free(param_ptr);
    param_ptr = NULL;
  }

  if (msg_ptr)
  {
    free(msg_ptr);
    msg_ptr = NULL;
  }

  return rc;
} /* qmi_uim_refresh_register */


/*===========================================================================
  FUNCTION  qmi_uim_refresh_ok
===========================================================================*/
/*!
@brief
  Enables the client to indicate if it is OK to start the refresh procedure.
  This function is supported only synchronously.

@return
  If return code < 0, the operation failed. In the failure case, if the
  return code is QMI_SERVICE_ERR, then the qmi_err_code value will give you
  the QMI error reason.  Otherwise, qmi_err_code will have meaningless data.

@note
  - Dependencies
    - qmi_uim_srvc_init_client() must be called before calling this.

  - Side Effects
    - None
*/
/*=========================================================================*/
int qmi_uim_refresh_ok
(
  int                                               client_handle,
  const qmi_uim_refresh_ok_params_type            * params,
  qmi_uim_rsp_data_type                           * rsp_data
)
{
  unsigned char     msg[QMI_UIM_STD_MSG_SIZE];
  int               msg_size;
  unsigned char    *tmp_msg_ptr;
  int rc;

  if((!params) || (!rsp_data))
  {
    return QMI_SERVICE_ERR;
  }

  /* Set tmp_msg_ptr to beginning of message-specific TLV portion of
  ** message buffer */
  tmp_msg_ptr = QMI_SRVC_PDU_PTR(msg);

  /* Set message size to the complete buffer minus the header size */
  msg_size = QMI_SRVC_PDU_SIZE(QMI_UIM_STD_MSG_SIZE);

  /* Construct TLV for Refresh register request */
  if (qmi_uim_write_refresh_ok_tlv (&tmp_msg_ptr,
                                    &msg_size,
                                    params) < 0)
  {
    return QMI_INTERNAL_ERR;
  }

  /* Send message, only sync if supported */
  rc = qmi_service_send_msg_sync (client_handle,
                                  QMI_UIM_SERVICE,
                                  QMI_UIM_REFRESH_OK_MSG_ID,
                                  QMI_SRVC_PDU_PTR(msg),
                                  (int)QMI_SRVC_PDU_SIZE(QMI_UIM_STD_MSG_SIZE) - msg_size,
                                  msg,
                                  &msg_size,
                                  QMI_UIM_STD_MSG_SIZE,
                                  QMI_SYNC_MSG_DEFAULT_TIMEOUT,
                                  &rsp_data->qmi_err_code);
  return rc;
} /* qmi_uim_refresh_ok */


/*===========================================================================
  FUNCTION  qmi_uim_refresh_complete
===========================================================================*/
/*!
@brief
  Indicates to the modem that the client has finished the refresh procedure
  after re-reading all the cached files. This function is supported only
  synchronously.

@return
  If return code < 0, the operation failed. In the failure case, if the
  return code is QMI_SERVICE_ERR, then the qmi_err_code value will give you
  the QMI error reason.  Otherwise, qmi_err_code will have meaningless data.

@note
  - Dependencies
    - qmi_uim_srvc_init_client() must be called before calling this.

  - Side Effects
    - None
*/
/*=========================================================================*/
int qmi_uim_refresh_complete
(
  int                                               client_handle,
  const qmi_uim_refresh_complete_params_type      * params,
  qmi_uim_rsp_data_type                           * rsp_data
)
{
  unsigned char     msg[QMI_UIM_STD_MSG_SIZE];
  int               msg_size;
  unsigned char    *tmp_msg_ptr;
  int rc;

  if((!params) || (!rsp_data))
  {
    return QMI_SERVICE_ERR;
  }

  /* Set tmp_msg_ptr to beginning of message-specific TLV portion of
  ** message buffer */
  tmp_msg_ptr = QMI_SRVC_PDU_PTR(msg);

  /* Set message size to the complete buffer minus the header size */
  msg_size = QMI_SRVC_PDU_SIZE(QMI_UIM_STD_MSG_SIZE);

  /* Construct TLV for Refresh register request */
  if (qmi_uim_write_refresh_complete_tlv (&tmp_msg_ptr,
                                          &msg_size,
                                          params) < 0)
  {
    return QMI_INTERNAL_ERR;
  }

  /* Send message, only sync if supported */
  rc = qmi_service_send_msg_sync (client_handle,
                                  QMI_UIM_SERVICE,
                                  QMI_UIM_REFRESH_COMPLETE_MSG_ID,
                                  QMI_SRVC_PDU_PTR(msg),
                                  (int)QMI_SRVC_PDU_SIZE(QMI_UIM_STD_MSG_SIZE) - msg_size,
                                  msg,
                                  &msg_size,
                                  QMI_UIM_STD_MSG_SIZE,
                                  QMI_SYNC_MSG_DEFAULT_TIMEOUT,
                                  &rsp_data->qmi_err_code);
  return rc;
} /* qmi_uim_refresh_complete */


/*===========================================================================
  FUNCTION  qmi_uim_refresh_get_last_event
===========================================================================*/
/*!
@brief
  Retreives the latest refresh event. This function is supported only
  synchronously.

@return
  If return code < 0, the operation failed. In the failure case, if the
  return code is QMI_SERVICE_ERR, then the qmi_err_code value will give you
  the QMI error reason.  Otherwise, qmi_err_code will have meaningless data.

@note
  - Dependencies
    - qmi_uim_srvc_init_client() must be called before calling this.

  - Side Effects
    - None
*/
/*=========================================================================*/
int qmi_uim_refresh_get_last_event
(
  int                                                 client_handle,
  const qmi_uim_refresh_get_last_event_params_type  * params,
  qmi_uim_rsp_data_type                             * rsp_data
)
{
  unsigned char     msg[QMI_UIM_STD_MSG_SIZE];
  int               msg_size;
  unsigned char    *tmp_msg_ptr;
  int rc;

  if((!params) || (!rsp_data))
  {
    return QMI_SERVICE_ERR;
  }

  /* Set tmp_msg_ptr to beginning of message-specific TLV portion of
  ** message buffer */
  tmp_msg_ptr = QMI_SRVC_PDU_PTR(msg);

  /* Set message size to the complete buffer minus the header size */
  msg_size = QMI_SRVC_PDU_SIZE(QMI_UIM_STD_MSG_SIZE);

  /* Construct TLV for Refresh register request */
  if (qmi_uim_write_refresh_get_last_event_tlv (&tmp_msg_ptr,
                                                &msg_size,
                                                params) < 0)
  {
    return QMI_INTERNAL_ERR;
  }

  /* Send message, only sync if supported */
  rc = qmi_service_send_msg_sync (client_handle,
                                  QMI_UIM_SERVICE,
                                  QMI_UIM_REFRESH_GET_LAST_EVENT_MSG_ID,
                                  QMI_SRVC_PDU_PTR(msg),
                                  (int)QMI_SRVC_PDU_SIZE(QMI_UIM_STD_MSG_SIZE) - msg_size,
                                  msg,
                                  &msg_size,
                                  QMI_UIM_STD_MSG_SIZE,
                                  QMI_SYNC_MSG_DEFAULT_TIMEOUT,
                                  &rsp_data->qmi_err_code);
  /* Prepare the response data */
  if (rc == QMI_NO_ERR)
  {
    rsp_data->rsp_id = QMI_UIM_SRVC_REFRESH_GET_LAST_EVENT_RSP_MSG;
    memset (&rsp_data->rsp_data, 0, sizeof(qmi_uim_refresh_get_last_evt_rsp_type));
    if (qmi_uim_handle_refresh_event_rsp (msg, msg_size, rsp_data,
                                          QMI_SERVICE_RESPONSE_MSG) < 0)
    {
      QMI_ERR_MSG_0 ("qmi_uim_refresh_get_last_event: qmi_uim_handle_refresh_event_rsp returned error");
      rc = QMI_INTERNAL_ERR;
      rsp_data->sys_err_code = QMI_INTERNAL_ERR;
    }
  }
  return rc;
} /* qmi_uim_refresh_get_last_event */


/*===========================================================================
  FUNCTION  qmi_uim_set_pin_protection
===========================================================================*/
/*!
@brief
  Enables or disables specified pin on the card.  If the user_cb function
  pointer is set to NULL, this function will be invoked synchronously.
  Otherwise it will be invoked asynchronously.

@return
  In the synchronous case, if return code < 0, the operation failed.  In
  the failure case, if the return code is QMI_SERVICE_ERR, then the
  qmi_err_code value will give you the QMI error reason.  Otherwise,
  qmi_err_code will have meaningless data.

  In the asynchronous case, if the return code < 0, the operation failed and
  you will not get an asynchronous response.  If the operation doesn't fail
  (return code >=0), the returned value will be a transaction handle which
  can be used to cancel the transaction via the qmi_uim_abort() command.

@note

  - Dependencies
    - qmi_uim_srvc_init_client() must be called before calling this.

  - Side Effects
    - None
*/
/*=========================================================================*/
int qmi_uim_set_pin_protection
(
  int                                               client_handle,
  const qmi_uim_set_pin_protection_params_type    * params,
  qmi_uim_user_async_cb_type                        user_cb,
  void                                            * user_data,
  qmi_uim_rsp_data_type                           * rsp_data
)
{
  unsigned char     msg[QMI_UIM_STD_MSG_SIZE];
  int               msg_size;
  unsigned char    *tmp_msg_ptr;
  int rc;

  if (!params)
  {
    return QMI_SERVICE_ERR;
  }

  if (!user_cb)
  {
    if (!rsp_data)
    {
      return QMI_SERVICE_ERR;
    }
  }

  /* Set tmp_msg_ptr to beginning of message-specific TLV portion of
   * message buffer */
  tmp_msg_ptr = QMI_SRVC_PDU_PTR(msg);

  /* Set the message size to the complete buffer minus the header size */
  msg_size = QMI_SRVC_PDU_SIZE(QMI_UIM_STD_MSG_SIZE);

  /* Construct TLV for Set Pin protection request */
  if (qmi_uim_write_pin_protection_tlv (&tmp_msg_ptr,
                                        &msg_size,
                                        params) < 0)
  {
    return QMI_INTERNAL_ERR;
  }

  /* Send message, do async if user_cb is non-NULL, sync otherwise */
  if (user_cb)
  {
    /*lint -e{611} */
    rc = qmi_service_send_msg_async (client_handle,
                                     QMI_UIM_SERVICE,
                                     QMI_UIM_SET_PIN_PROTECTION_MSG_ID,
                                     QMI_SRVC_PDU_PTR(msg),
                                     (int)QMI_SRVC_PDU_SIZE(QMI_UIM_STD_MSG_SIZE) - msg_size,
                                     qmi_uim_srvc_async_cb,
                                     NULL,
                                     (void *)user_cb,
                                     user_data);
  }
  else
  {
    rc = qmi_service_send_msg_sync (client_handle,
                                    QMI_UIM_SERVICE,
                                    QMI_UIM_SET_PIN_PROTECTION_MSG_ID,
                                    QMI_SRVC_PDU_PTR(msg),
                                    (int)QMI_SRVC_PDU_SIZE(QMI_UIM_STD_MSG_SIZE) - msg_size,
                                    msg,
                                    &msg_size,
                                    QMI_UIM_STD_MSG_SIZE,
                                    QMI_SYNC_MSG_DEFAULT_TIMEOUT,
                                    &rsp_data->qmi_err_code);

    rsp_data->rsp_id = QMI_UIM_SRVC_SET_PIN_PROTECTION_RSP_MSG;

    /* Process optional TLVs (encrypted PIN1 or retires left) if any */
    memset (&rsp_data->rsp_data, 0, sizeof(qmi_uim_set_pin_protection_rsp_type));
    if (qmi_uim_handle_pin_operation_rsp (msg, msg_size, rsp_data) < 0)
    {
      QMI_ERR_MSG_0 ("qmi_uim_set_pin_protection: qmi_uim_handle_pin_operation_rsp returned error");
      rc = QMI_INTERNAL_ERR;
      rsp_data->sys_err_code = QMI_INTERNAL_ERR;
    }
  }
  return rc;
} /* qmi_uim_set_pin_protection */


/*===========================================================================
  FUNCTION  qmi_uim_verify_pin
===========================================================================*/
/*!
@brief
  Verifies the specified pin on the card.  If the user_cb function
  pointer is set to NULL, this function will be invoked synchronously.
  Otherwise it will be invoked asynchronously.

@return
  In the synchronous case, if return code < 0, the operation failed.  In
  the failure case, if the return code is QMI_SERVICE_ERR, then the
  qmi_err_code value will give you the QMI error reason.  Otherwise,
  qmi_err_code will have meaningless data.

  In the asynchronous case, if the return code < 0, the operation failed and
  you will not get an asynchronous response.  If the operation doesn't fail
  (return code >=0), the returned value will be a transaction handle which
  can be used to cancel the transaction via the qmi_uim_abort() command.

@note
  - Dependencies
    - qmi_uim_srvc_init_client() must be called before calling this.

  - Side Effects
    - None
*/
/*=========================================================================*/
int qmi_uim_verify_pin
(
  int                                                client_handle,
  const qmi_uim_verify_pin_params_type              * params,
  qmi_uim_user_async_cb_type                         user_cb,
  void                                             * user_data,
  qmi_uim_rsp_data_type                            * rsp_data
)
{
  unsigned char     msg[QMI_UIM_STD_MSG_SIZE];
  int               msg_size;
  unsigned char    *tmp_msg_ptr;
  int rc;

  if (!params)
  {
    return QMI_SERVICE_ERR;
  }

  if (!user_cb)
  {
    if (!rsp_data)
    {
      return QMI_SERVICE_ERR;
    }
  }

  /* Set tmp_msg_ptr to beginning of message-specific TLV portion of
   * message buffer */
  tmp_msg_ptr = QMI_SRVC_PDU_PTR(msg);

  /* Set the message size to the complete buffer minus the header size */
  msg_size = QMI_SRVC_PDU_SIZE(QMI_UIM_STD_MSG_SIZE);

  /* Construct TLV for Verify Pin request */
  if (qmi_uim_write_verify_pin_tlv (&tmp_msg_ptr,
                                    &msg_size,
                                    params) < 0)
  {
    return QMI_INTERNAL_ERR;
  }

  /* Send message, do async if user_cb is non-NULL, sync otherwise */
  if (user_cb)
  {
    /*lint -e{611} */
    rc = qmi_service_send_msg_async (client_handle,
                                     QMI_UIM_SERVICE,
                                     QMI_UIM_VERIFY_PIN_MSG_ID,
                                     QMI_SRVC_PDU_PTR(msg),
                                     (int)QMI_SRVC_PDU_SIZE(QMI_UIM_STD_MSG_SIZE) - msg_size,
                                     qmi_uim_srvc_async_cb,
                                     NULL,
                                     (void *)user_cb,
                                     user_data);
  }
  else
  {
    rc = qmi_service_send_msg_sync (client_handle,
                                    QMI_UIM_SERVICE,
                                    QMI_UIM_VERIFY_PIN_MSG_ID,
                                    QMI_SRVC_PDU_PTR(msg),
                                    (int)QMI_SRVC_PDU_SIZE(QMI_UIM_STD_MSG_SIZE) - msg_size,
                                    msg,
                                    &msg_size,
                                    QMI_UIM_STD_MSG_SIZE,
                                    QMI_SYNC_MSG_DEFAULT_TIMEOUT,
                                    &rsp_data->qmi_err_code);

    rsp_data->rsp_id = QMI_UIM_SRVC_VERIFY_PIN_RSP_MSG;

    /* Process optional TLVs (encrypted PIN1 or retires left) if any */
    memset (&rsp_data->rsp_data, 0, sizeof(qmi_uim_verify_pin_rsp_type));
    if (qmi_uim_handle_pin_operation_rsp (msg, msg_size, rsp_data) < 0)
    {
      QMI_ERR_MSG_0 ("qmi_uim_verify_pin: qmi_uim_handle_pin_operation_rsp returned error");
      rc = QMI_INTERNAL_ERR;
      rsp_data->sys_err_code = QMI_INTERNAL_ERR;
    }
  }
  return rc;
} /* qmi_uim_verify_pin */


/*===========================================================================
  FUNCTION  qmi_uim_unblock_pin
===========================================================================*/
/*!
@brief
  Unblocks the specified pin on the card.  If the user_cb function
  pointer is set to NULL, this function will be invoked synchronously.
  Otherwise it will be invoked asynchronously.

@return
  In the synchronous case, if return code < 0, the operation failed.  In
  the failure case, if the return code is QMI_SERVICE_ERR, then the
  qmi_err_code value will give you the QMI error reason.  Otherwise,
  qmi_err_code will have meaningless data.

  In the asynchronous case, if the return code < 0, the operation failed and
  you will not get an asynchronous response.  If the operation doesn't fail
  (return code >=0), the returned value will be a transaction handle which
  can be used to cancel the transaction via the qmi_uim_abort() command.

@note
  - Dependencies
    - qmi_uim_srvc_init_client() must be called before calling this.

  - Side Effects
    - None
*/
/*=========================================================================*/
int qmi_uim_unblock_pin
(
  int                                                 client_handle,
  const qmi_uim_unblock_pin_params_type             * params,
  qmi_uim_user_async_cb_type                          user_cb,
  void                                              * user_data,
  qmi_uim_rsp_data_type                             * rsp_data
)
{
  unsigned char     msg[QMI_UIM_STD_MSG_SIZE];
  int               msg_size;
  unsigned char    *tmp_msg_ptr;
  int rc;

  if(!params)
  {
    return QMI_SERVICE_ERR;
  }

  if (!user_cb)
  {
    if (!rsp_data)
    {
      return QMI_SERVICE_ERR;
    }
  }

  /* Set tmp_msg_ptr to beginning of message-specific TLV portion of
   * message buffer */
  tmp_msg_ptr = QMI_SRVC_PDU_PTR(msg);

  /* Set the message size to the complete buffer minus the header size */
  msg_size = QMI_SRVC_PDU_SIZE(QMI_UIM_STD_MSG_SIZE);

  /* Construct TLV for Unblock Pin request */
  if (qmi_uim_write_unblock_pin_tlv (&tmp_msg_ptr,
                                     &msg_size,
                                     params) < 0)
  {
    return QMI_INTERNAL_ERR;
  }

  /* Send message, do async if user_cb is non-NULL, sync otherwise */
  if (user_cb)
  {
    /*lint -e{611} */
    rc = qmi_service_send_msg_async (client_handle,
                                     QMI_UIM_SERVICE,
                                     QMI_UIM_UNBLOCK_PIN_MSG_ID,
                                     QMI_SRVC_PDU_PTR(msg),
                                     (int)QMI_SRVC_PDU_SIZE(QMI_UIM_STD_MSG_SIZE) - msg_size,
                                     qmi_uim_srvc_async_cb,
                                     NULL,
                                     (void *)user_cb,
                                     user_data);
  }
  else
  {
    rc = qmi_service_send_msg_sync (client_handle,
                                    QMI_UIM_SERVICE,
                                    QMI_UIM_UNBLOCK_PIN_MSG_ID,
                                    QMI_SRVC_PDU_PTR(msg),
                                    (int)QMI_SRVC_PDU_SIZE(QMI_UIM_STD_MSG_SIZE) - msg_size,
                                    msg,
                                    &msg_size,
                                    QMI_UIM_STD_MSG_SIZE,
                                    QMI_SYNC_MSG_DEFAULT_TIMEOUT,
                                    &rsp_data->qmi_err_code);

    rsp_data->rsp_id = QMI_UIM_SRVC_UNBLOCK_PIN_RSP_MSG;

    /* Process optional TLVs (encrypted PIN1 or retires left) if any */
    memset (&rsp_data->rsp_data, 0, sizeof(qmi_uim_unblock_pin_rsp_type));
    if (qmi_uim_handle_pin_operation_rsp (msg, msg_size, rsp_data) < 0)
    {
      QMI_ERR_MSG_0 ("qmi_uim_unblock_pin: qmi_uim_handle_pin_operation_rsp returned error");
      rc = QMI_INTERNAL_ERR;
      rsp_data->sys_err_code = QMI_INTERNAL_ERR;
    }
  }
  return rc;
} /* qmi_uim_unblock_pin */


/*===========================================================================
  FUNCTION  qmi_uim_change_pin
===========================================================================*/
/*!
@brief
  Changes the specified pin on the card.  If the user_cb function
  pointer is set to NULL, this function will be invoked synchronously.
  Otherwise it will be invoked asynchronously.

@return
  In the synchronous case, if return code < 0, the operation failed.  In
  the failure case, if the return code is QMI_SERVICE_ERR, then the
  qmi_err_code value will give you the QMI error reason.  Otherwise,
  qmi_err_code will have meaningless data.

  In the asynchronous case, if the return code < 0, the operation failed and
  you will not get an asynchronous response.  If the operation doesn't fail
  (return code >=0), the returned value will be a transaction handle which
  can be used to cancel the transaction via the qmi_uim_abort() command.

@note
  - Dependencies
    - qmi_uim_srvc_init_client() must be called before calling this.

  - Side Effects
    - None
*/
/*=========================================================================*/
int qmi_uim_change_pin
(
  int                                               client_handle,
  const qmi_uim_change_pin_params_type            * params,
  qmi_uim_user_async_cb_type                        user_cb,
  void                                            * user_data,
  qmi_uim_rsp_data_type                           * rsp_data
)
{
  unsigned char     msg[QMI_UIM_STD_MSG_SIZE];
  int               msg_size;
  unsigned char    *tmp_msg_ptr;
  int rc;

  if(!params)
  {
    return QMI_SERVICE_ERR;
  }

  if (!user_cb)
  {
    if (!rsp_data)
    {
      return QMI_SERVICE_ERR;
    }
  }

  /* Set tmp_msg_ptr to beginning of message-specific TLV portion of
   * message buffer */
  tmp_msg_ptr = QMI_SRVC_PDU_PTR(msg);

  /* Set the message size to the complete buffer minus the header size */
  msg_size = QMI_SRVC_PDU_SIZE(QMI_UIM_STD_MSG_SIZE);

  /* Construct TLV for Change Pin request */
  if (qmi_uim_write_change_pin_tlv (&tmp_msg_ptr,
                                    &msg_size,
                                    params) < 0)
  {
    return QMI_INTERNAL_ERR;
  }

  /* Send message, do async if user_cb is non-NULL, sync otherwise */
  if (user_cb)
  {
    /*lint -e{611} */
    rc = qmi_service_send_msg_async (client_handle,
                                     QMI_UIM_SERVICE,
                                     QMI_UIM_CHANGE_PIN_MSG_ID,
                                     QMI_SRVC_PDU_PTR(msg),
                                     (int)QMI_SRVC_PDU_SIZE(QMI_UIM_STD_MSG_SIZE) - msg_size,
                                     qmi_uim_srvc_async_cb,
                                     NULL,
                                     (void *)user_cb,
                                     user_data);
  }
  else
  {
    rc = qmi_service_send_msg_sync (client_handle,
                                    QMI_UIM_SERVICE,
                                    QMI_UIM_CHANGE_PIN_MSG_ID,
                                    QMI_SRVC_PDU_PTR(msg),
                                    (int)QMI_SRVC_PDU_SIZE(QMI_UIM_STD_MSG_SIZE) - msg_size,
                                    msg,
                                    &msg_size,
                                    QMI_UIM_STD_MSG_SIZE,
                                    QMI_SYNC_MSG_DEFAULT_TIMEOUT,
                                    &rsp_data->qmi_err_code);

    rsp_data->rsp_id = QMI_UIM_SRVC_CHANGE_PIN_RSP_MSG;

    /* Process optional TLVs (encrypted PIN1 or retires left) if any */
    memset (&rsp_data->rsp_data, 0, sizeof(qmi_uim_change_pin_rsp_type));
    if (qmi_uim_handle_pin_operation_rsp (msg, msg_size, rsp_data) < 0)
    {
      QMI_ERR_MSG_0 ("qmi_uim_change_pin: qmi_uim_handle_pin_operation_rsp returned error");
      rc = QMI_INTERNAL_ERR;
      rsp_data->sys_err_code = QMI_INTERNAL_ERR;
    }
  }
  return rc;
} /* qmi_uim_change_pin */


/*===========================================================================
  FUNCTION  qmi_uim_depersonalization
===========================================================================*/
/*!
@brief
  Deactivates or unblocks specified personalization on the phone.  If the
  user_cb function pointer is set to NULL, this function will be invoked
  synchronously. Otherwise it will be invoked asynchronously.

@return
  In the synchronous case, if return code < 0, the operation failed.  In
  the failure case, if the return code is QMI_SERVICE_ERR, then the
  qmi_err_code value will give you the QMI error reason.  Otherwise,
  qmi_err_code will have meaningless data.

  In the asynchronous case, if the return code < 0, the operation failed and
  you will not get an asynchronous response.  If the operation doesn't fail
  (return code >=0), the returned value will be a transaction handle which
  can be used to cancel the transaction via the qmi_uim_abort() command.

@note
  - Dependencies
    - qmi_uim_srvc_init_client() must be called before calling this.

  - Side Effects
    - None
*/
/*=========================================================================*/
int qmi_uim_depersonalization
(
  int                                                 client_handle,
  const qmi_uim_depersonalization_params_type       * params,
  qmi_uim_user_async_cb_type                          user_cb,
  void                                              * user_data,
  qmi_uim_rsp_data_type                             * rsp_data
)
{
  unsigned char     msg[QMI_UIM_STD_MSG_SIZE];
  int               msg_size;
  unsigned char    *tmp_msg_ptr;
  int rc;

  if(!params)
  {
    return QMI_SERVICE_ERR;
  }

  if (!user_cb)
  {
    if (!rsp_data)
    {
      return QMI_SERVICE_ERR;
    }
  }

  /* Set tmp_msg_ptr to beginning of message-specific TLV portion of
   * message buffer */
  tmp_msg_ptr = QMI_SRVC_PDU_PTR(msg);

  /* Set the message size to the complete buffer minus the header size */
  msg_size = QMI_SRVC_PDU_SIZE(QMI_UIM_STD_MSG_SIZE);

  /* Construct TLV for Deperso request */
  if (qmi_uim_write_depersonalization_tlv (&tmp_msg_ptr,
                                           &msg_size,
                                           params) < 0)
  {
    return QMI_INTERNAL_ERR;
  }

  /* Send message, do async if user_cb is non-NULL, sync otherwise */
  if (user_cb)
  {
    /*lint -e{611} */
    rc = qmi_service_send_msg_async (client_handle,
                                     QMI_UIM_SERVICE,
                                     QMI_UIM_DEPERSONALIZATION_MSG_ID,
                                     QMI_SRVC_PDU_PTR(msg),
                                     (int)QMI_SRVC_PDU_SIZE(QMI_UIM_STD_MSG_SIZE) - msg_size,
                                     qmi_uim_srvc_async_cb,
                                     NULL,
                                     (void *)user_cb,
                                     user_data);
  }
  else
  {
    rc = qmi_service_send_msg_sync (client_handle,
                                    QMI_UIM_SERVICE,
                                    QMI_UIM_DEPERSONALIZATION_MSG_ID,
                                    QMI_SRVC_PDU_PTR(msg),
                                    (int)QMI_SRVC_PDU_SIZE(QMI_UIM_STD_MSG_SIZE) - msg_size,
                                    msg,
                                    &msg_size,
                                    QMI_UIM_STD_MSG_SIZE,
                                    QMI_SYNC_MSG_DEFAULT_TIMEOUT,
                                    &rsp_data->qmi_err_code);

    rsp_data->rsp_id = QMI_UIM_SRVC_DEPERSONALIZATION_RSP_MSG;

    /* Response data is filled up only in case of Service Error
     * which in this case has the optional TLV - retires left */
    if (rc != QMI_NO_ERR)
    {
      memset (&rsp_data->rsp_data, 0, sizeof(qmi_uim_change_pin_rsp_type));
      if (qmi_uim_handle_depersonalization_rsp (msg, msg_size, rsp_data) < 0)
      {
        QMI_ERR_MSG_0 ("qmi_uim_depersonalization: qmi_uim_handle_depersonalization_rsp returned error");
        rc = QMI_INTERNAL_ERR;
        rsp_data->sys_err_code = QMI_INTERNAL_ERR;
      }
    }
  }
  return rc;
} /* qmi_uim_depersonalization */


/*===========================================================================
  FUNCTION  qmi_uim_power_down
===========================================================================*/
/*!
@brief
  Powers down the card.  If the user_cb function pointer is set to NULL,
  this function will be invoked synchronously. Otherwise it will be invoked
  asynchronously.

@return
  In the synchronous case, if return code < 0, the operation failed.  In
  the failure case, if the return code is QMI_SERVICE_ERR, then the
  qmi_err_code value will give you the QMI error reason.  Otherwise,
  qmi_err_code will have meaningless data.

  In the asynchronous case, if the return code < 0, the operation failed and
  you will not get an asynchronous response.  If the operation doesn't fail
  (return code >=0), the returned value will be a transaction handle which
  can be used to cancel the transaction via the qmi_uim_abort() command.

@note
  - Dependencies
    - qmi_uim_srvc_init_client() must be called before calling this.

  - Side Effects
    - None
*/
/*=========================================================================*/
int qmi_uim_power_down
(
  int                                               client_handle,
  const qmi_uim_power_down_params_type            * params,
  qmi_uim_user_async_cb_type                        user_cb,
  void                                            * user_data,
  qmi_uim_rsp_data_type                           * rsp_data
)
{
  unsigned char     msg[QMI_UIM_STD_MSG_SIZE];
  int               msg_size;
  unsigned char    *tmp_msg_ptr;
  int               rc;
  unsigned char     param_buf [QMI_UIM_POWER_DOWN_ALL_TLV_SIZE];
  unsigned char     *param_ptr = param_buf;

  if(!params)
  {
    return QMI_SERVICE_ERR;
  }

  if (!user_cb)
  {
    if (!rsp_data)
    {
      return QMI_SERVICE_ERR;
    }
  }

  /* Set tmp_msg_ptr to beginning of message-specific TLV portion of
   * message buffer */
  tmp_msg_ptr = QMI_SRVC_PDU_PTR(msg);

  /* Set the message size to the complete buffer minus the header size */
  msg_size = QMI_SRVC_PDU_SIZE(QMI_UIM_STD_MSG_SIZE);

  /* Construct TLV for Power down request */
  WRITE_8_BIT_VAL (param_ptr, params->slot);
  if (qmi_util_write_std_tlv (&tmp_msg_ptr,
                              &msg_size,
                              QMI_UIM_MANDATORY_1_TLV_ID,
                              QMI_UIM_SLOT_MAX_TLV_SIZE,
                              (void *)param_buf) < 0)
  {
    return QMI_INTERNAL_ERR;
  }

  /* Send message, do async if user_cb is non-NULL, sync otherwise */
  if (user_cb)
  {
    /*lint -e{611} */
    rc = qmi_service_send_msg_async (client_handle,
                                     QMI_UIM_SERVICE,
                                     QMI_UIM_POWER_DOWN_MSG_ID,
                                     QMI_SRVC_PDU_PTR(msg),
                                     (int)QMI_SRVC_PDU_SIZE(QMI_UIM_STD_MSG_SIZE) - msg_size,
                                     qmi_uim_srvc_async_cb,
                                     NULL,
                                     (void *)user_cb,
                                     user_data);
  }
  else
  {
    rc = qmi_service_send_msg_sync (client_handle,
                                    QMI_UIM_SERVICE,
                                    QMI_UIM_POWER_DOWN_MSG_ID,
                                    QMI_SRVC_PDU_PTR(msg),
                                    (int)QMI_SRVC_PDU_SIZE(QMI_UIM_STD_MSG_SIZE) - msg_size,
                                    msg,
                                    &msg_size,
                                    QMI_UIM_STD_MSG_SIZE,
                                    QMI_SYNC_MSG_DEFAULT_TIMEOUT,
                                    &rsp_data->qmi_err_code);

    rsp_data->rsp_id = QMI_UIM_SRVC_POWER_DOWN_RSP_MSG;
  }
  return rc;
} /* qmi_uim_power_down */


/*===========================================================================
  FUNCTION  qmi_uim_power_up
===========================================================================*/
/*!
@brief
  Powers up the card.  If the user_cb function pointer is set to NULL,
  this function will be invoked synchronously. Otherwise it will be invoked
  asynchronously.

@return
  In the synchronous case, if return code < 0, the operation failed.  In
  the failure case, if the return code is QMI_SERVICE_ERR, then the
  qmi_err_code value will give you the QMI error reason.  Otherwise,
  qmi_err_code will have meaningless data.

  In the asynchronous case, if the return code < 0, the operation failed and
  you will not get an asynchronous response.  If the operation doesn't fail
  (return code >=0), the returned value will be a transaction handle which
  can be used to cancel the transaction via the qmi_uim_abort() command.

@note
  - Dependencies
    - qmi_uim_srvc_init_client() must be called before calling this.

  - Side Effects
    - None
*/
/*=========================================================================*/
int qmi_uim_power_up
(
  int                                                 client_handle,
  const qmi_uim_power_up_params_type                * params,
  qmi_uim_user_async_cb_type                          user_cb,
  void                                              * user_data,
  qmi_uim_rsp_data_type                             * rsp_data
)
{
  unsigned char     msg[QMI_UIM_STD_MSG_SIZE];
  int               msg_size;
  unsigned char    *tmp_msg_ptr;
  int               rc;
  unsigned char     param_buf [QMI_UIM_POWER_UP_ALL_TLV_SIZE];
  unsigned char    *param_ptr = param_buf;

  if(!params)
  {
    return QMI_SERVICE_ERR;
  }

  if (!user_cb)
  {
    if (!rsp_data)
    {
      return QMI_SERVICE_ERR;
    }
  }

  /* Set tmp_msg_ptr to beginning of message-specific TLV portion of
   * message buffer */
  tmp_msg_ptr = QMI_SRVC_PDU_PTR(msg);

  /* Set the message size to the complete buffer minus the header size */
  msg_size = QMI_SRVC_PDU_SIZE(QMI_UIM_STD_MSG_SIZE);

  /* Construct TLV for Power up request */
  WRITE_8_BIT_VAL (param_ptr, params->slot);

  if (qmi_util_write_std_tlv (&tmp_msg_ptr,
                              &msg_size,
                              QMI_UIM_MANDATORY_1_TLV_ID,
                              QMI_UIM_SLOT_MAX_TLV_SIZE,
                              (void *)param_buf) < 0)
  {
    return QMI_INTERNAL_ERR;
  }

  /* Send message, do async if user_cb is non-NULL, sync otherwise */
  if (user_cb)
  {
    /*lint -e{611} */
    rc = qmi_service_send_msg_async (client_handle,
                                     QMI_UIM_SERVICE,
                                     QMI_UIM_POWER_UP_MSG_ID,
                                     QMI_SRVC_PDU_PTR(msg),
                                     (int)QMI_SRVC_PDU_SIZE(QMI_UIM_STD_MSG_SIZE) - msg_size,
                                     qmi_uim_srvc_async_cb,
                                     NULL,
                                     (void *)user_cb,
                                     user_data);
  }
  else
  {
    rc = qmi_service_send_msg_sync (client_handle,
                                    QMI_UIM_SERVICE,
                                    QMI_UIM_POWER_UP_MSG_ID,
                                    QMI_SRVC_PDU_PTR(msg),
                                    (int)QMI_SRVC_PDU_SIZE(QMI_UIM_STD_MSG_SIZE) - msg_size,
                                    msg,
                                    &msg_size,
                                    QMI_UIM_STD_MSG_SIZE,
                                    QMI_SYNC_MSG_DEFAULT_TIMEOUT,
                                    &rsp_data->qmi_err_code);

    rsp_data->rsp_id = QMI_UIM_SRVC_POWER_UP_RSP_MSG;
  }
  return rc;
} /* qmi_uim_power_up */


/*===========================================================================
  FUNCTION  qmi_uim_get_card_status
===========================================================================*/
/*!
@brief
  Retreives the Card status along with status of all the available applications
  on the card. If the user_cb function pointer is set to NULL,
  this function will be invoked synchronously. Otherwise it will be invoked
  asynchronously.

@return
  In the synchronous case, if return code < 0, the operation failed.  In
  the failure case, if the return code is QMI_SERVICE_ERR, then the
  qmi_err_code value will give you the QMI error reason.  Otherwise,
  qmi_err_code will have meaningless data.

  In the asynchronous case, if the return code < 0, the operation failed and
  you will not get an asynchronous response.  If the operation doesn't fail
  (return code >=0), the returned value will be a transaction handle which
  can be used to cancel the transaction via the qmi_uim_abort() command.

@note

  - Dependencies
    - qmi_uim_srvc_init_client() must be called before calling this.

  - Side Effects
    - None
*/
/*=========================================================================*/
int qmi_uim_get_card_status
(
  int                                       client_handle,
  qmi_uim_user_async_cb_type                user_cb,
  void                                    * user_data,
  qmi_uim_rsp_data_type                   * rsp_data
)
{
  unsigned char     msg[QMI_UIM_STD_MSG_SIZE];
  int               msg_size;
  int rc;

  if (!user_cb)
  {
    if (!rsp_data)
    {
      return QMI_SERVICE_ERR;
    }
  }

  /* Set the message size to the complete buffer minus the header size */
  msg_size = QMI_SRVC_PDU_SIZE(QMI_UIM_STD_MSG_SIZE);

  /* Send message, do async if user_cb is non-NULL, sync otherwise */
  if (user_cb)
  {
    /*lint -e{611} */
    rc = qmi_service_send_msg_async (client_handle,
                                     QMI_UIM_SERVICE,
                                     QMI_UIM_GET_CARD_STATUS_MSG_ID,
                                     QMI_SRVC_PDU_PTR(msg),
                                     (int)QMI_SRVC_PDU_SIZE(QMI_UIM_STD_MSG_SIZE) - msg_size,
                                     qmi_uim_srvc_async_cb,
                                     NULL,
                                     (void *)user_cb,
                                     user_data);
  }
  else
  {
    rc = qmi_service_send_msg_sync (client_handle,
                                    QMI_UIM_SERVICE,
                                    QMI_UIM_GET_CARD_STATUS_MSG_ID,
                                    QMI_SRVC_PDU_PTR(msg),
                                    (int)QMI_SRVC_PDU_SIZE(QMI_UIM_STD_MSG_SIZE) - msg_size,
                                    msg,
                                    &msg_size,
                                    QMI_UIM_STD_MSG_SIZE,
                                    QMI_SYNC_MSG_DEFAULT_TIMEOUT,
                                    &rsp_data->qmi_err_code);

    rsp_data->rsp_id = QMI_UIM_SRVC_GET_CARD_STATUS_RSP_MSG;

    /* Response data is filled up only in case of no Error */
    if (rc == QMI_NO_ERR)
    {
      if (qmi_uim_handle_get_card_status_rsp (msg, msg_size, rsp_data,
                                              QMI_SERVICE_RESPONSE_MSG) < 0)
      {
        QMI_ERR_MSG_0 ("qmi_uim_get_card_status: qmi_uim_handle_get_card_status_rsp returned error");
        rc = QMI_INTERNAL_ERR;
        rsp_data->sys_err_code = QMI_INTERNAL_ERR;
      }
    }
  }

  return rc;
} /* qmi_uim_get_card_status */


/*===========================================================================
  FUNCTION  qmi_uim_event_reg
===========================================================================*/
/*!
@brief
  Issues the Event registration command for the client. Note that this is a
  synchronous-only function call.

@return
  If return code < 0, the operation failed.  In the failure case, if the
  return code is QMI_SERVICE_ERR, then the qmi_err_code value will give the
  QMI error reason.  Otherwise, qmi_err_code will have meaningless data.

@note

  - Dependencies
    - qmi_uim_srvc_init_client() must be called before calling this.

  - Side Effects
    - None
*/
/*=========================================================================*/
int qmi_uim_event_reg
(
  int                                             client_handle,
  const qmi_uim_event_reg_params_type           * params,
  qmi_uim_rsp_data_type                         * rsp_data
)
{
  unsigned char     msg[QMI_UIM_STD_MSG_SIZE];
  unsigned char     *msg_ptr;
  int               msg_size;
  unsigned char     param_buf [QMI_UIM_EVENT_REG_MAX_TLV_SIZE];
  unsigned char     *param_ptr = param_buf;
  unsigned long     mask       = 0;
  int               rc;

  if((!params) || (!rsp_data))
  {
    return QMI_SERVICE_ERR;
  }

  /* Prepare the request message */
  msg_ptr = QMI_SRVC_PDU_PTR(msg);
  msg_size = QMI_SRVC_PDU_SIZE(QMI_UIM_STD_MSG_SIZE);

  memset (param_buf, 0, sizeof(param_buf));

  /* Prepare the TLV */
  if(params->card_status)
  {
    mask |= QMI_UIM_EVENT_MASK_CARD_STATUS;
  }
  if(params->sap_connection)
  {
    mask |= QMI_UIM_EVENT_MASK_SAP_CONNECTION;
  }

  WRITE_32_BIT_VAL(param_ptr, mask);

  if (qmi_util_write_std_tlv (&msg_ptr,
                              &msg_size,
                              QMI_UIM_MANDATORY_1_TLV_ID,
                              QMI_UIM_EVENT_REG_MAX_TLV_SIZE,
                              (void *)param_buf) < 0)
  {
    return QMI_INTERNAL_ERR;
  }

  rc = qmi_service_send_msg_sync (client_handle,
                                  QMI_UIM_SERVICE,
                                  QMI_UIM_EVENT_REG_MSG_ID,
                                  QMI_SRVC_PDU_PTR(msg),
                                  (int)QMI_SRVC_PDU_SIZE(QMI_UIM_STD_MSG_SIZE) - msg_size,
                                  msg,
                                  &msg_size,
                                  QMI_UIM_STD_MSG_SIZE,
                                  QMI_SYNC_MSG_DEFAULT_TIMEOUT,
                                  &rsp_data->qmi_err_code);
  if (rc == QMI_NO_ERR)
  {
    rsp_data->rsp_id = QMI_UIM_SRVC_EVENT_REG_RSP_MSG;
    if (qmi_uim_handle_event_reg_rsp (msg, msg_size, rsp_data) < 0)
    {
      QMI_ERR_MSG_0 ("qmi_uim_get_card_status: qmi_uim_handle_event_reg_rsp returned error");
      rc = QMI_INTERNAL_ERR;
    }
  }

  return rc;
} /* qmi_uim_event_reg */


/*===========================================================================
  FUNCTION  qmi_uim_authenticate
===========================================================================*/
/*!
@brief
  Issues the authenticate request on the card. If the user_cb function
  pointer is set to NULL, this function will be invoked synchronously.
  Otherwise it will be invoked asynchronously.

@return
  In the synchronous case, if return code < 0, the operation failed.  In
  the failure case, if the return code is QMI_SERVICE_ERR, then the
  qmi_err_code value will give you the QMI error reason.  Otherwise,
  qmi_err_code will have meaningless data.

  In the asynchronous case, if the return code < 0, the operation failed and
  you will not get an asynchronous response.  If the operation doesn't fail
  (return code >=0), the returned value will be a transaction handle which
  can be used to cancel the transaction via the qmi_uim_abort() command.

@note

  - Dependencies
    - qmi_uim_srvc_init_client() must be called before calling this.

  - Side Effects
    - None
*/
/*=========================================================================*/
int qmi_uim_authenticate
(
  int                                               client_handle,
  const qmi_uim_authenticate_params_type          * params,
  qmi_uim_user_async_cb_type                        user_cb,
  void                                            * user_data,
  qmi_uim_rsp_data_type                           * rsp_data
)
{
  unsigned char     msg[QMI_UIM_STD_MSG_SIZE];
  int               msg_size;
  unsigned char    *tmp_msg_ptr;
  int rc;

  if (!params)
  {
    return QMI_SERVICE_ERR;
  }

  if (!user_cb)
  {
    if (!rsp_data)
    {
      return QMI_SERVICE_ERR;
    }
  }

  /* Set tmp_msg_ptr to beginning of message-specific TLV portion of
  ** message buffer */
  tmp_msg_ptr = QMI_SRVC_PDU_PTR(msg);

  /* Set message size to the complete buffer minus the header size */
  msg_size = QMI_SRVC_PDU_SIZE(QMI_UIM_STD_MSG_SIZE);

  /* Construct TLV for Authenticate request */
  if (qmi_uim_write_authenticate_tlv (&tmp_msg_ptr,
                                      &msg_size,
                                      params) < 0)
  {
    return QMI_INTERNAL_ERR;
  }

  /* Send message, do async if user_cb is non-NULL, sync otherwise */
  if (user_cb)
  {
    /*lint -e{611} */
    rc = qmi_service_send_msg_async (client_handle,
                                     QMI_UIM_SERVICE,
                                     QMI_UIM_AUTHENTICATE_MSG_ID,
                                     QMI_SRVC_PDU_PTR(msg),
                                     (int)QMI_SRVC_PDU_SIZE(QMI_UIM_STD_MSG_SIZE) - msg_size,
                                     qmi_uim_srvc_async_cb,
                                     NULL,
                                     (void *)user_cb,
                                     user_data);
  }
  else
  {
    rc = qmi_service_send_msg_sync (client_handle,
                                    QMI_UIM_SERVICE,
                                    QMI_UIM_AUTHENTICATE_MSG_ID,
                                    QMI_SRVC_PDU_PTR(msg),
                                    (int)QMI_SRVC_PDU_SIZE(QMI_UIM_STD_MSG_SIZE) - msg_size,
                                    msg,
                                    &msg_size,
                                    QMI_UIM_STD_MSG_SIZE,
                                    QMI_SYNC_MSG_DEFAULT_TIMEOUT,
                                    &rsp_data->qmi_err_code);
    /* Prepare the response data */
    if (rc == QMI_NO_ERR)
    {
      memset (&rsp_data->rsp_data, 0, sizeof(qmi_uim_authenticate_rsp_type));
      if (qmi_uim_handle_authenticate_rsp (msg, msg_size, rsp_data) < 0)
      {
        QMI_ERR_MSG_0 ("qmi_uim_authenticate: qmi_uim_handle_authenticate_rsp returned error");
        rc = QMI_INTERNAL_ERR;
        rsp_data->sys_err_code = QMI_INTERNAL_ERR;
      }
    }
  }
  return rc;
} /* qmi_uim_authenticate */


/*===========================================================================
  FUNCTION  qmi_uim_get_service_status
===========================================================================*/
/*!
@brief
  Issues the Service status command that queries for the status of particular
  services in the card e.g FDN, BDN etc. Note that currently only FDN query
  is supported. If the user_cb function pointer is set to NULL,
  this function will be invoked synchronously. Otherwise it will be invoked
  asynchronously.

@return
  In the synchronous case, if return code < 0, the operation failed.  In
  the failure case, if the return code is QMI_SERVICE_ERR, then the
  qmi_err_code value will give you the QMI error reason.  Otherwise,
  qmi_err_code will have meaningless data.

  In the asynchronous case, if the return code < 0, the operation failed and
  you will not get an asynchronous response.  If the operation doesn't fail
  (return code >=0), the returned value will be a transaction handle which
  can be used to cancel the transaction via the qmi_uim_abort() command.

@note

  - Dependencies
    - qmi_uim_srvc_init_client() must be called before calling this.

  - Side Effects
    - None
*/
/*=========================================================================*/
int qmi_uim_get_service_status
(
  int                                               client_handle,
  const qmi_uim_get_service_status_params_type    * params,
  qmi_uim_user_async_cb_type                        user_cb,
  void                                            * user_data,
  qmi_uim_rsp_data_type                           * rsp_data
)
{
  unsigned char     msg[QMI_UIM_STD_MSG_SIZE];
  int               msg_size;
  unsigned char    *tmp_msg_ptr;
  int rc;

  if (!params)
  {
    return QMI_SERVICE_ERR;
  }

  if (!user_cb)
  {
    if (!rsp_data)
    {
      return QMI_SERVICE_ERR;
    }
  }

  /* Set tmp_msg_ptr to beginning of message-specific TLV portion of
  ** message buffer */
  tmp_msg_ptr = QMI_SRVC_PDU_PTR(msg);

  /* Set message size to the complete buffer minus the header size */
  msg_size = QMI_SRVC_PDU_SIZE(QMI_UIM_STD_MSG_SIZE);

  /* Construct TLV for Get Service Status request */
  if (qmi_uim_write_get_service_status_tlv (&tmp_msg_ptr,
                                            &msg_size,
                                            params) < 0)
  {
    return QMI_INTERNAL_ERR;
  }

  /* Send message, do async if user_cb is non-NULL, sync otherwise */
  if (user_cb)
  {
    /*lint -e{611} */
    rc = qmi_service_send_msg_async (client_handle,
                                     QMI_UIM_SERVICE,
                                     QMI_UIM_GET_SERVICE_STATUS_MSG_ID,
                                     QMI_SRVC_PDU_PTR(msg),
                                     (int)QMI_SRVC_PDU_SIZE(QMI_UIM_STD_MSG_SIZE) - msg_size,
                                     qmi_uim_srvc_async_cb,
                                     NULL,
                                     (void *)user_cb,
                                     user_data);
  }
  else
  {
    rc = qmi_service_send_msg_sync (client_handle,
                                    QMI_UIM_SERVICE,
                                    QMI_UIM_GET_SERVICE_STATUS_MSG_ID,
                                    QMI_SRVC_PDU_PTR(msg),
                                    (int)QMI_SRVC_PDU_SIZE(QMI_UIM_STD_MSG_SIZE) - msg_size,
                                    msg,
                                    &msg_size,
                                    QMI_UIM_STD_MSG_SIZE,
                                    QMI_SYNC_MSG_DEFAULT_TIMEOUT,
                                    &rsp_data->qmi_err_code);
    /* Prepare the response data */
    if (rc == QMI_NO_ERR)
    {
      rsp_data->rsp_id = QMI_UIM_SRVC_GET_SERVICE_STATUS_RSP_MSG;
      memset (&rsp_data->rsp_data, 0, sizeof(qmi_uim_get_service_status_rsp_type));
      if (qmi_uim_handle_get_service_status_rsp (msg, msg_size, rsp_data) < 0)
      {
        QMI_ERR_MSG_0 ("qmi_uim_get_service_status: qmi_uim_handle_get_service_status_rsp returned error");
        rc = QMI_INTERNAL_ERR;
        rsp_data->sys_err_code = QMI_INTERNAL_ERR;
      }
    }
  }
  return rc;
} /* qmi_uim_get_service_status */


/*===========================================================================
  FUNCTION  qmi_uim_set_service_status
===========================================================================*/
/*!
@brief
  Issues the Service status command that sets the status of particular
  services in the card e.g FDN, BDN etc. Note that currently only FDN
  service is supported. If the user_cb function pointer is set to NULL,
  this function will be invoked synchronously. Otherwise it will be invoked
  asynchronously.

@return
  In the synchronous case, if return code < 0, the operation failed.  In
  the failure case, if the return code is QMI_SERVICE_ERR, then the
  qmi_err_code value will give you the QMI error reason.  Otherwise,
  qmi_err_code will have meaningless data.

  In the asynchronous case, if the return code < 0, the operation failed and
  you will not get an asynchronous response.  If the operation doesn't fail
  (return code >=0), the returned value will be a transaction handle which
  can be used to cancel the transaction via the qmi_uim_abort() command.

@note

  - Dependencies
    - qmi_uim_srvc_init_client() must be called before calling this.

  - Side Effects
    - None
*/
/*=========================================================================*/
int qmi_uim_set_service_status
(
  int                                               client_handle,
  const qmi_uim_set_service_status_params_type    * params,
  qmi_uim_user_async_cb_type                        user_cb,
  void                                            * user_data,
  qmi_uim_rsp_data_type                           * rsp_data
)
{
  unsigned char     msg[QMI_UIM_STD_MSG_SIZE];
  int               msg_size;
  unsigned char    *tmp_msg_ptr;
  int rc;

  if (!params)
  {
    return QMI_SERVICE_ERR;
  }

  if (!user_cb)
  {
    if (!rsp_data)
    {
      return QMI_SERVICE_ERR;
    }
  }

  /* Set tmp_msg_ptr to beginning of message-specific TLV portion of
  ** message buffer */
  tmp_msg_ptr = QMI_SRVC_PDU_PTR(msg);

  /* Set message size to the complete buffer minus the header size */
  msg_size = QMI_SRVC_PDU_SIZE(QMI_UIM_STD_MSG_SIZE);

  /* Construct TLV for Get Service Status request */
  if (qmi_uim_write_set_service_status_tlv (&tmp_msg_ptr,
                                            &msg_size,
                                            params) < 0)
  {
    return QMI_INTERNAL_ERR;
  }

  /* Send message, do async if user_cb is non-NULL, sync otherwise */
  if (user_cb)
  {
    /*lint -e{611} */
    rc = qmi_service_send_msg_async (client_handle,
                                     QMI_UIM_SERVICE,
                                     QMI_UIM_SET_SERVICE_STATUS_MSG_ID,
                                     QMI_SRVC_PDU_PTR(msg),
                                     (int)QMI_SRVC_PDU_SIZE(QMI_UIM_STD_MSG_SIZE) - msg_size,
                                     qmi_uim_srvc_async_cb,
                                     NULL,
                                     (void *)user_cb,
                                     user_data);
  }
  else
  {
    rc = qmi_service_send_msg_sync (client_handle,
                                    QMI_UIM_SERVICE,
                                    QMI_UIM_SET_SERVICE_STATUS_MSG_ID,
                                    QMI_SRVC_PDU_PTR(msg),
                                    (int)QMI_SRVC_PDU_SIZE(QMI_UIM_STD_MSG_SIZE) - msg_size,
                                    msg,
                                    &msg_size,
                                    QMI_UIM_STD_MSG_SIZE,
                                    QMI_SYNC_MSG_DEFAULT_TIMEOUT,
                                    &rsp_data->qmi_err_code);
    rsp_data->rsp_id = QMI_UIM_SRVC_SET_SERVICE_STATUS_RSP_MSG;
  }
  return rc;
} /* qmi_uim_set_service_status */


/*===========================================================================
  FUNCTION  qmi_uim_change_provisioning_session
===========================================================================*/
/*!
@brief
  Issues the change provisioning session command that is used to activate,
  deactivate or switch the provisioning sessions. If the user_cb function
  pointer is set to NULL, this function will be invoked synchronously.
  Otherwise it will be invoked asynchronously.

@return
  In the synchronous case, if return code < 0, the operation failed.  In
  the failure case, if the return code is QMI_SERVICE_ERR, then the
  qmi_err_code value will give you the QMI error reason.  Otherwise,
  qmi_err_code will have meaningless data.

  In the asynchronous case, if the return code < 0, the operation failed and
  you will not get an asynchronous response.  If the operation doesn't fail
  (return code >=0), the returned value will be a transaction handle which
  can be used to cancel the transaction via the qmi_uim_abort() command.

@note

  - Dependencies
    - qmi_uim_srvc_init_client() must be called before calling this.

  - Side Effects
    - None
*/
/*=========================================================================*/
int qmi_uim_change_provisioning_session
(
  int                                                 client_handle,
  const qmi_uim_change_prov_session_params_type     * params,
  qmi_uim_user_async_cb_type                          user_cb,
  void                                              * user_data,
  qmi_uim_rsp_data_type                             * rsp_data
)
{
  unsigned char     msg[QMI_UIM_STD_MSG_SIZE];
  int               msg_size;
  unsigned char    *tmp_msg_ptr;
  int rc;

  if (!params)
  {
    return QMI_SERVICE_ERR;
  }

  if (!user_cb)
  {
    if (!rsp_data)
    {
      return QMI_SERVICE_ERR;
    }
  }

  /* Set tmp_msg_ptr to beginning of message-specific TLV portion of
  ** message buffer */
  tmp_msg_ptr = QMI_SRVC_PDU_PTR(msg);

  /* Set message size to the complete buffer minus the header size */
  msg_size = QMI_SRVC_PDU_SIZE(QMI_UIM_STD_MSG_SIZE);

  /* Construct TLV for Get Service Status request */
  if (qmi_uim_write_change_prov_session_tlv (&tmp_msg_ptr,
                                               &msg_size,
                                               params) < 0)
  {
    return QMI_INTERNAL_ERR;
  }

  /* Send message, do async if user_cb is non-NULL, sync otherwise */
  if (user_cb)
  {
    /*lint -e{611} */
    rc = qmi_service_send_msg_async (client_handle,
                                     QMI_UIM_SERVICE,
                                     QMI_UIM_CHANGE_PROVISIONING_SESSION_MSG_ID,
                                     QMI_SRVC_PDU_PTR(msg),
                                     (int)QMI_SRVC_PDU_SIZE(QMI_UIM_STD_MSG_SIZE) - msg_size,
                                     qmi_uim_srvc_async_cb,
                                     NULL,
                                     (void *)user_cb,
                                     user_data);
  }
  else
  {
    rc = qmi_service_send_msg_sync (client_handle,
                                    QMI_UIM_SERVICE,
                                    QMI_UIM_CHANGE_PROVISIONING_SESSION_MSG_ID,
                                    QMI_SRVC_PDU_PTR(msg),
                                    (int)QMI_SRVC_PDU_SIZE(QMI_UIM_STD_MSG_SIZE) - msg_size,
                                    msg,
                                    &msg_size,
                                    QMI_UIM_STD_MSG_SIZE,
                                    QMI_SYNC_MSG_DEFAULT_TIMEOUT,
                                    &rsp_data->qmi_err_code);
    rsp_data->rsp_id = QMI_UIM_SRVC_CHANGE_PROV_SESSION_RSP_MSG;
  }
  return rc;
} /* qmi_uim_change_provisioning_session */


/*===========================================================================
  FUNCTION  qmi_uim_get_label
===========================================================================*/
/*!
@brief
  Issues the get label command that retrieves the label of an application
  from EF-DIR on the UICC card. Note that it will return an error if used on
  an ICC card. If the user_cb function pointer is set to NULL, this function
  will be invoked synchronously. Otherwise it will be invoked asynchronously.

@return
  In the synchronous case, if return code < 0, the operation failed.  In
  the failure case, if the return code is QMI_SERVICE_ERR, then the
  qmi_err_code value will give you the QMI error reason.  Otherwise,
  qmi_err_code will have meaningless data.

  In the asynchronous case, if the return code < 0, the operation failed and
  you will not get an asynchronous response.  If the operation doesn't fail
  (return code >=0), the returned value will be a transaction handle which
  can be used to cancel the transaction via the qmi_uim_abort() command.

@note

  - Dependencies
    - qmi_uim_srvc_init_client() must be called before calling this.

  - Side Effects
    - None
*/
/*=========================================================================*/
int qmi_uim_get_label
(
  int                                               client_handle,
  const qmi_uim_get_label_params_type             * params,
  qmi_uim_user_async_cb_type                        user_cb,
  void                                            * user_data,
  qmi_uim_rsp_data_type                           * rsp_data
)
{
  unsigned char     msg[QMI_UIM_STD_MSG_SIZE];
  int               msg_size;
  unsigned char    *tmp_msg_ptr;
  int rc;

  if (!params)
  {
    return QMI_SERVICE_ERR;
  }

  if (!user_cb)
  {
    if (!rsp_data)
    {
      return QMI_SERVICE_ERR;
    }
  }

  /* Set tmp_msg_ptr to beginning of message-specific TLV portion of
  ** message buffer */
  tmp_msg_ptr = QMI_SRVC_PDU_PTR(msg);

  /* Set message size to the complete buffer minus the header size */
  msg_size = QMI_SRVC_PDU_SIZE(QMI_UIM_STD_MSG_SIZE);

  /* Construct TLV for Get Service Status request */
  if (qmi_uim_write_get_label_tlv (&tmp_msg_ptr,
                                   &msg_size,
                                   params) < 0)
  {
    return QMI_INTERNAL_ERR;
  }

  /* Send message, do async if user_cb is non-NULL, sync otherwise */
  if (user_cb)
  {
    /*lint -e{611} */
    rc = qmi_service_send_msg_async (client_handle,
                                     QMI_UIM_SERVICE,
                                     QMI_UIM_GET_LABEL_MSG_ID,
                                     QMI_SRVC_PDU_PTR(msg),
                                     (int)QMI_SRVC_PDU_SIZE(QMI_UIM_STD_MSG_SIZE) - msg_size,
                                     qmi_uim_srvc_async_cb,
                                     NULL,
                                     (void *)user_cb,
                                     user_data);
  }
  else
  {
    rc = qmi_service_send_msg_sync (client_handle,
                                    QMI_UIM_SERVICE,
                                    QMI_UIM_GET_LABEL_MSG_ID,
                                    QMI_SRVC_PDU_PTR(msg),
                                    (int)QMI_SRVC_PDU_SIZE(QMI_UIM_STD_MSG_SIZE) - msg_size,
                                    msg,
                                    &msg_size,
                                    QMI_UIM_STD_MSG_SIZE,
                                    QMI_SYNC_MSG_DEFAULT_TIMEOUT,
                                    &rsp_data->qmi_err_code);
    /* Prepare the response data */
    if (rc == QMI_NO_ERR)
    {
      rsp_data->rsp_id = QMI_UIM_SRVC_GET_LABEL_RSP_MSG;
      memset (&rsp_data->rsp_data, 0, sizeof(qmi_uim_get_label_rsp_type));
      if (qmi_uim_handle_get_label_rsp (msg, msg_size, rsp_data) < 0)
      {
        QMI_ERR_MSG_0 ("qmi_uim_get_label: qmi_uim_handle_get_label_rsp returned error");
        rc = QMI_INTERNAL_ERR;
        rsp_data->sys_err_code = QMI_INTERNAL_ERR;
      }
    }
  }
  return rc;
} /* qmi_uim_get_label */


/*===========================================================================
  FUNCTION  qmi_uim_close_session
===========================================================================*/
/*!
@brief
  Issues the close session command for a non-provisioning session that may
  have been opened before by the client. This function is supported only
  synchronously.

@return
  If return code < 0, the operation failed. In the failure case, if the
  return code is QMI_SERVICE_ERR, then the qmi_err_code value will give you
  the QMI error reason.  Otherwise, qmi_err_code will have meaningless data.

@note

  - Dependencies
    - qmi_uim_srvc_init_client() must be called before calling this.

  - Side Effects
    - None
*/
/*=========================================================================*/
int qmi_uim_close_session
(
  int                                               client_handle,
  const qmi_uim_close_session_params_type         * params,
  qmi_uim_rsp_data_type                           * rsp_data
)
{
  unsigned char     msg[QMI_UIM_STD_MSG_SIZE];
  int               msg_size;
  unsigned char    *tmp_msg_ptr;
  int rc;

  if((!params) || (!rsp_data))
  {
    return QMI_SERVICE_ERR;
  }

  /* Set tmp_msg_ptr to beginning of message-specific TLV portion of
  ** message buffer */
  tmp_msg_ptr = QMI_SRVC_PDU_PTR(msg);

  /* Set message size to the complete buffer minus the header size */
  msg_size = QMI_SRVC_PDU_SIZE(QMI_UIM_STD_MSG_SIZE);

  /* Construct TLV for Close session request */
  if (qmi_uim_write_close_session_tlv (&tmp_msg_ptr,
                                       &msg_size,
                                       params) < 0)
  {
    return QMI_INTERNAL_ERR;
  }

  /* Send message */
  rc = qmi_service_send_msg_sync (client_handle,
                                  QMI_UIM_SERVICE,
                                  QMI_UIM_CLOSE_SESSION_MSG_ID,
                                  QMI_SRVC_PDU_PTR(msg),
                                  (int)QMI_SRVC_PDU_SIZE(QMI_UIM_STD_MSG_SIZE) - msg_size,
                                  msg,
                                  &msg_size,
                                  QMI_UIM_STD_MSG_SIZE,
                                  QMI_SYNC_MSG_DEFAULT_TIMEOUT,
                                  &rsp_data->qmi_err_code);
  /* No other parameters to receive */
  rsp_data->rsp_id = QMI_UIM_SRVC_CLOSE_SESSION_RSP_MSG;

  return rc;
} /* qmi_uim_close_session */


/*===========================================================================
  FUNCTION  qmi_uim_send_apdu
===========================================================================*/
/*!
@brief
  Issues the request for sending raw APDUs to the card. An optional channel
  id parameter can be used when a logical channel is already opened previously
  using the qmi_uim_logical_channel command. If the user_cb function pointer
  is set to NULL, this function will be invoked synchronously. Otherwise it
  will be invoked asynchronously.

@return
  In the synchronous case, if return code < 0, the operation failed.  In
  the failure case, if the return code is QMI_SERVICE_ERR, then the
  qmi_err_code value will give you the QMI error reason.  Otherwise,
  qmi_err_code will have meaningless data.

  In the asynchronous case, if the return code < 0, the operation failed and
  you will not get an asynchronous response.  If the operation doesn't fail
  (return code >=0), the returned value will be a transaction handle which
  can be used to cancel the transaction via the qmi_uim_abort() command.

@note

  - Dependencies
    - qmi_uim_srvc_init_client() must be called before calling this.

  - Side Effects
    - None
*/
/*=========================================================================*/
EXTERN int qmi_uim_send_apdu
(
  int                                               client_handle,
  const qmi_uim_send_apdu_params_type             * params,
  qmi_uim_user_async_cb_type                        user_cb,
  void                                            * user_data,
  qmi_uim_rsp_data_type                           * rsp_data
)
{
  unsigned char     msg[QMI_UIM_STD_MSG_SIZE];
  int               msg_size;
  unsigned char    *tmp_msg_ptr;
  int rc;

  if (!params)
  {
    return QMI_SERVICE_ERR;
  }

  if (!user_cb)
  {
    if (!rsp_data)
    {
      return QMI_SERVICE_ERR;
    }
  }

  /* Set tmp_msg_ptr to beginning of message-specific TLV portion of
  ** message buffer */
  tmp_msg_ptr = QMI_SRVC_PDU_PTR(msg);

  /* Set message size to the complete buffer minus the header size */
  msg_size = QMI_SRVC_PDU_SIZE(QMI_UIM_STD_MSG_SIZE);

  /* Construct TLV for Get Service Status request */
  if (qmi_uim_write_send_apdu_tlv (&tmp_msg_ptr,
                                   &msg_size,
                                   params) < 0)
  {
    return QMI_INTERNAL_ERR;
  }

  /* Send message, do async if user_cb is non-NULL, sync otherwise */
  if (user_cb)
  {
    /*lint -e{611} */
    rc = qmi_service_send_msg_async (client_handle,
                                     QMI_UIM_SERVICE,
                                     QMI_UIM_SEND_APDU_MSG_ID,
                                     QMI_SRVC_PDU_PTR(msg),
                                     (int)QMI_SRVC_PDU_SIZE(QMI_UIM_STD_MSG_SIZE) - msg_size,
                                     qmi_uim_srvc_async_cb,
                                     NULL,
                                     (void *)user_cb,
                                     user_data);
  }
  else
  {
    rc = qmi_service_send_msg_sync (client_handle,
                                    QMI_UIM_SERVICE,
                                    QMI_UIM_SEND_APDU_MSG_ID,
                                    QMI_SRVC_PDU_PTR(msg),
                                    (int)QMI_SRVC_PDU_SIZE(QMI_UIM_STD_MSG_SIZE) - msg_size,
                                    msg,
                                    &msg_size,
                                    QMI_UIM_STD_MSG_SIZE,
                                    QMI_SYNC_MSG_DEFAULT_TIMEOUT,
                                    &rsp_data->qmi_err_code);
    /* Prepare the response data */
    if (rc == QMI_NO_ERR)
    {
      memset (&rsp_data->rsp_data, 0, sizeof(qmi_uim_send_apdu_rsp_type));
      if (qmi_uim_handle_send_apdu_rsp (msg, msg_size, rsp_data) < 0)
      {
        QMI_ERR_MSG_0 ("qmi_uim_send_apdu: qmi_uim_handle_send_apdu_rsp returned error");
        rc = QMI_INTERNAL_ERR;
        rsp_data->sys_err_code = QMI_INTERNAL_ERR;
      }
    }
  }
  return rc;
} /* qmi_uim_send_apdu */


/*===========================================================================
  FUNCTION  qmi_uim_sap_connection
===========================================================================*/
/*!
@brief
  Issues the request for establishing or releasing a SAP connection to the
  UIM module on the modem. If the user_cb function pointer is set to NULL,
  this function will be invoked synchronously. Otherwise it will be invoked
  asynchronously.

@return
  In the synchronous case, if return code < 0, the operation failed.  In
  the failure case, if the return code is QMI_SERVICE_ERR, then the
  qmi_err_code value will give you the QMI error reason.  Otherwise,
  qmi_err_code will have meaningless data.

  In the asynchronous case, if the return code < 0, the operation failed and
  you will not get an asynchronous response.  If the operation doesn't fail
  (return code >=0), the returned value will be a transaction handle which
  can be used to cancel the transaction via the qmi_uim_abort() command.

@note

  - Dependencies
    - qmi_uim_srvc_init_client() must be called before calling this.

  - Side Effects
    - None
*/
/*=========================================================================*/
EXTERN int qmi_uim_sap_connection
(
  int                                               client_handle,
  const qmi_uim_sap_connection_params_type        * params,
  qmi_uim_user_async_cb_type                        user_cb,
  void                                            * user_data,
  qmi_uim_rsp_data_type                           * rsp_data
)
{
  unsigned char     msg[QMI_UIM_STD_MSG_SIZE];
  int               msg_size;
  unsigned char    *tmp_msg_ptr;
  int rc;

  if (!params)
  {
    return QMI_SERVICE_ERR;
  }

  if (!user_cb)
  {
    if (!rsp_data)
    {
      return QMI_SERVICE_ERR;
    }
  }

  /* Set tmp_msg_ptr to beginning of message-specific TLV portion of
  ** message buffer */
  tmp_msg_ptr = QMI_SRVC_PDU_PTR(msg);

  /* Set message size to the complete buffer minus the header size */
  msg_size = QMI_SRVC_PDU_SIZE(QMI_UIM_STD_MSG_SIZE);

  /* Construct TLV for SAP connection request */
  if (qmi_uim_write_sap_connection_tlv (&tmp_msg_ptr,
                                         &msg_size,
                                         params) < 0)
  {
    return QMI_INTERNAL_ERR;
  }

  /* Send message, do async if user_cb is non-NULL, sync otherwise */
  if (user_cb)
  {
    /*lint -e{611} */
    rc = qmi_service_send_msg_async (client_handle,
                                     QMI_UIM_SERVICE,
                                     QMI_UIM_SAP_CONNECTION_MSG_ID,
                                     QMI_SRVC_PDU_PTR(msg),
                                     (int)QMI_SRVC_PDU_SIZE(QMI_UIM_STD_MSG_SIZE) - msg_size,
                                     qmi_uim_srvc_async_cb,
                                     NULL,
                                     (void *)user_cb,
                                     user_data);
  }
  else
  {
    rc = qmi_service_send_msg_sync (client_handle,
                                    QMI_UIM_SERVICE,
                                    QMI_UIM_SAP_CONNECTION_MSG_ID,
                                    QMI_SRVC_PDU_PTR(msg),
                                    (int)QMI_SRVC_PDU_SIZE(QMI_UIM_STD_MSG_SIZE) - msg_size,
                                    msg,
                                    &msg_size,
                                    QMI_UIM_STD_MSG_SIZE,
                                    QMI_SYNC_MSG_DEFAULT_TIMEOUT,
                                    &rsp_data->qmi_err_code);
    /* Prepare the response data */
    if (rc == QMI_NO_ERR)
    {
      memset (&rsp_data->rsp_data, 0, sizeof(qmi_uim_sap_connection_rsp_type));
      if (qmi_uim_handle_sap_connection_rsp (msg, msg_size, rsp_data) < 0)
      {
        QMI_ERR_MSG_0 ("qmi_uim_sap_connection: qmi_uim_handle_sap_connection_rsp returned error");
        rc = QMI_INTERNAL_ERR;
        rsp_data->sys_err_code = QMI_INTERNAL_ERR;
      }
    }
  }
  return rc;
} /* qmi_uim_sap_connection */


/*===========================================================================
  FUNCTION  qmi_uim_sap_request
===========================================================================*/
/*!
@brief
  Issues the various types of SAP requests after a SAP connection is
  successfully established to the UIM module on the modem. If the user_cb
  function pointer is set to NULL, this function will be invoked
  synchronously. Otherwise it will be invoked asynchronously.

@return
  In the synchronous case, if return code < 0, the operation failed.  In
  the failure case, if the return code is QMI_SERVICE_ERR, then the
  qmi_err_code value will give you the QMI error reason.  Otherwise,
  qmi_err_code will have meaningless data.

  In the asynchronous case, if the return code < 0, the operation failed and
  you will not get an asynchronous response.  If the operation doesn't fail
  (return code >=0), the returned value will be a transaction handle which
  can be used to cancel the transaction via the qmi_uim_abort() command.

@note

  - Dependencies
    - qmi_uim_srvc_init_client() must be called before calling this.

  - Side Effects
    - None
*/
/*=========================================================================*/
EXTERN int qmi_uim_sap_request
(
  int                                               client_handle,
  const qmi_uim_sap_request_params_type           * params,
  qmi_uim_user_async_cb_type                        user_cb,
  void                                            * user_data,
  qmi_uim_rsp_data_type                           * rsp_data
)
{
  unsigned char     msg[QMI_UIM_STD_MSG_SIZE];
  int               msg_size;
  unsigned char    *tmp_msg_ptr;
  int rc;

  if (!params)
  {
    return QMI_SERVICE_ERR;
  }

  if (!user_cb)
  {
    if (!rsp_data)
    {
      return QMI_SERVICE_ERR;
    }
  }

  /* Set tmp_msg_ptr to beginning of message-specific TLV portion of
  ** message buffer */
  tmp_msg_ptr = QMI_SRVC_PDU_PTR(msg);

  /* Set message size to the complete buffer minus the header size */
  msg_size = QMI_SRVC_PDU_SIZE(QMI_UIM_STD_MSG_SIZE);

  /* Construct TLV for SAP request */
  if (qmi_uim_write_sap_request_tlv (&tmp_msg_ptr,
                                     &msg_size,
                                     params) < 0)
  {
    return QMI_INTERNAL_ERR;
  }

  /* Send message, do async if user_cb is non-NULL, sync otherwise */
  if (user_cb)
  {
    /*lint -e{611} */
    rc = qmi_service_send_msg_async (client_handle,
                                     QMI_UIM_SERVICE,
                                     QMI_UIM_SAP_REQUEST_MSG_ID,
                                     QMI_SRVC_PDU_PTR(msg),
                                     (int)QMI_SRVC_PDU_SIZE(QMI_UIM_STD_MSG_SIZE) - msg_size,
                                     qmi_uim_srvc_async_cb,
                                     NULL,
                                     (void *)user_cb,
                                     user_data);
  }
  else
  {
    /* Send sync message */
    rc = qmi_service_send_msg_sync (client_handle,
                                    QMI_UIM_SERVICE,
                                    QMI_UIM_SAP_REQUEST_MSG_ID,
                                    QMI_SRVC_PDU_PTR(msg),
                                    (int)QMI_SRVC_PDU_SIZE(QMI_UIM_STD_MSG_SIZE) - msg_size,
                                    msg,
                                    &msg_size,
                                    QMI_UIM_STD_MSG_SIZE,
                                    QMI_SYNC_MSG_DEFAULT_TIMEOUT,
                                    &rsp_data->qmi_err_code);
    /* Prepare the response data */
    if (rc == QMI_NO_ERR)
    {
      memset (&rsp_data->rsp_data, 0, sizeof(qmi_uim_sap_request_rsp_type));
      if (qmi_uim_handle_sap_request_rsp (msg, msg_size, rsp_data) < 0)
      {
        QMI_ERR_MSG_0 ("qmi_uim_sap_request: qmi_uim_handle_sap_request_rsp returned error");
        rc = QMI_INTERNAL_ERR;
        rsp_data->sys_err_code = QMI_INTERNAL_ERR;
      }
    }
  }
  return rc;
} /* qmi_uim_sap_request */


/*===========================================================================
  FUNCTION  qmi_uim_logical_channel
===========================================================================*/
/*!
@brief
  Issues the request for open or close logical channel on a particlar
  application on the UICC card. Note that it will return an error if used on
  an ICC card. If the user_cb function pointer is set to NULL, this function
  will be invoked synchronously. Otherwise it will be invoked asynchronously.

@return
  In the synchronous case, if return code < 0, the operation failed.  In
  the failure case, if the return code is QMI_SERVICE_ERR, then the
  qmi_err_code value will give you the QMI error reason.  Otherwise,
  qmi_err_code will have meaningless data.

  In the asynchronous case, if the return code < 0, the operation failed and
  you will not get an asynchronous response.  If the operation doesn't fail
  (return code >=0), the returned value will be a transaction handle which
  can be used to cancel the transaction via the qmi_uim_abort() command.

@note

  - Dependencies
    - qmi_uim_srvc_init_client() must be called before calling this.

  - Side Effects
    - None
*/
/*=========================================================================*/
int qmi_uim_logical_channel
(
  int                                               client_handle,
  const qmi_uim_logical_channel_params_type       * params,
  qmi_uim_user_async_cb_type                        user_cb,
  void                                            * user_data,
  qmi_uim_rsp_data_type                           * rsp_data
)
{
  unsigned char     msg[QMI_UIM_STD_MSG_SIZE];
  int               msg_size;
  unsigned char    *tmp_msg_ptr;
  int rc;

  if (!params)
  {
    return QMI_SERVICE_ERR;
  }

  if (!user_cb)
  {
    if (!rsp_data)
    {
      return QMI_SERVICE_ERR;
    }
  }

  /* Set tmp_msg_ptr to beginning of message-specific TLV portion of
  ** message buffer */
  tmp_msg_ptr = QMI_SRVC_PDU_PTR(msg);

  /* Set message size to the complete buffer minus the header size */
  msg_size = QMI_SRVC_PDU_SIZE(QMI_UIM_STD_MSG_SIZE);

  /* Construct TLV for Get Service Status request */
  if (qmi_uim_write_logical_channel_tlv (&tmp_msg_ptr,
                                         &msg_size,
                                         params) < 0)
  {
    return QMI_INTERNAL_ERR;
  }

  /* Send message, do async if user_cb is non-NULL, sync otherwise */
  if (user_cb)
  {
    /*lint -e{611} */
    rc = qmi_service_send_msg_async (client_handle,
                                     QMI_UIM_SERVICE,
                                     QMI_UIM_LOGICAL_CHANNEL_MSG_ID,
                                     QMI_SRVC_PDU_PTR(msg),
                                     (int)QMI_SRVC_PDU_SIZE(QMI_UIM_STD_MSG_SIZE) - msg_size,
                                     qmi_uim_srvc_async_cb,
                                     NULL,
                                     (void *)user_cb,
                                     user_data);
  }
  else
  {
    rc = qmi_service_send_msg_sync (client_handle,
                                    QMI_UIM_SERVICE,
                                    QMI_UIM_LOGICAL_CHANNEL_MSG_ID,
                                    QMI_SRVC_PDU_PTR(msg),
                                    (int)QMI_SRVC_PDU_SIZE(QMI_UIM_STD_MSG_SIZE) - msg_size,
                                    msg,
                                    &msg_size,
                                    QMI_UIM_STD_MSG_SIZE,
                                    QMI_SYNC_MSG_DEFAULT_TIMEOUT,
                                    &rsp_data->qmi_err_code);
    /* Prepare the response data */
    if (rc == QMI_NO_ERR)
    {
      memset (&rsp_data->rsp_data, 0, sizeof(qmi_uim_logical_channel_rsp_type));
      if (qmi_uim_handle_logical_channel_rsp (msg, msg_size, rsp_data) < 0)
      {
        QMI_ERR_MSG_0 ("qmi_uim_logical_channel: qmi_uim_handle_logical_channel_rsp returned error");
        rc = QMI_INTERNAL_ERR;
        rsp_data->sys_err_code = QMI_INTERNAL_ERR;
      }
    }
  }
  return rc;
} /* qmi_uim_logical_channel */


/*===========================================================================
  FUNCTION  qmi_uim_get_atr
===========================================================================*/
/*!
@brief
  Issues the request to retrieve the ATR of the specified card. If the
  user_cb function pointer is set to NULL, this function will be invoked
  synchronously. Otherwise it will be invoked asynchronously.

@return
  In the synchronous case, if return code < 0, the operation failed.  In
  the failure case, if the return code is QMI_SERVICE_ERR, then the
  qmi_err_code value will give you the QMI error reason.  Otherwise,
  qmi_err_code will have meaningless data.

  In the asynchronous case, if the return code < 0, the operation failed and
  you will not get an asynchronous response.  If the operation doesn't fail
  (return code >=0), the returned value will be a transaction handle which
  can be used to cancel the transaction via the qmi_uim_abort() command.

@note

  - Dependencies
    - qmi_uim_srvc_init_client() must be called before calling this.

  - Side Effects
    - None
*/
/*=========================================================================*/
EXTERN int qmi_uim_get_atr
(
  int                                               client_handle,
  const qmi_uim_get_atr_params_type               * params,
  qmi_uim_user_async_cb_type                        user_cb,
  void                                            * user_data,
  qmi_uim_rsp_data_type                           * rsp_data
)
{
  unsigned char     msg[QMI_UIM_STD_MSG_SIZE];
  int               msg_size;
  unsigned char    *tmp_msg_ptr;
  unsigned char     param_buf [QMI_UIM_GET_ATR_ALL_TLV_SIZE];
  unsigned char    *param_ptr = param_buf;
  int rc;

  if (!params)
  {
    return QMI_SERVICE_ERR;
  }

  if (!user_cb)
  {
    if (!rsp_data)
    {
      return QMI_SERVICE_ERR;
    }
  }

  /* Set tmp_msg_ptr to beginning of message-specific TLV portion of
  ** message buffer */
  tmp_msg_ptr = QMI_SRVC_PDU_PTR(msg);

  /* Set message size to the complete buffer minus the header size */
  msg_size = QMI_SRVC_PDU_SIZE(QMI_UIM_STD_MSG_SIZE);

  /* Construct TLV for get ATR request */
  WRITE_8_BIT_VAL (param_ptr, params->slot);
  if (qmi_util_write_std_tlv (&tmp_msg_ptr,
                              &msg_size,
                              QMI_UIM_MANDATORY_1_TLV_ID,
                              QMI_UIM_SLOT_MAX_TLV_SIZE,
                              (void *)param_buf) < 0)
  {
    return QMI_INTERNAL_ERR;
  }

  /* Send message, do async if user_cb is non-NULL, sync otherwise */
  if (user_cb)
  {
    /*lint -e{611} */
    rc = qmi_service_send_msg_async (client_handle,
                                     QMI_UIM_SERVICE,
                                     QMI_UIM_GET_ATR_MSG_ID,
                                     QMI_SRVC_PDU_PTR(msg),
                                     (int)QMI_SRVC_PDU_SIZE(QMI_UIM_STD_MSG_SIZE) - msg_size,
                                     qmi_uim_srvc_async_cb,
                                     NULL,
                                     (void *)user_cb,
                                     user_data);
  }
  else
  {
    rc = qmi_service_send_msg_sync (client_handle,
                                    QMI_UIM_SERVICE,
                                    QMI_UIM_GET_ATR_MSG_ID,
                                    QMI_SRVC_PDU_PTR(msg),
                                    (int)QMI_SRVC_PDU_SIZE(QMI_UIM_STD_MSG_SIZE) - msg_size,
                                    msg,
                                    &msg_size,
                                    QMI_UIM_STD_MSG_SIZE,
                                    QMI_SYNC_MSG_DEFAULT_TIMEOUT,
                                    &rsp_data->qmi_err_code);
    /* Prepare the response data */
    if (rc == QMI_NO_ERR)
    {
      memset (&rsp_data->rsp_data, 0, sizeof(qmi_uim_get_atr_rsp_type));
      if (qmi_uim_handle_get_atr_rsp (msg, msg_size, rsp_data) < 0)
      {
        QMI_ERR_MSG_0 ("qmi_uim_logical_channel: qmi_uim_handle_logical_channel_rsp returned error");
        rc = QMI_INTERNAL_ERR;
        rsp_data->sys_err_code = QMI_INTERNAL_ERR;
      }
    }
  }
  return rc;
} /* qmi_uim_get_atr */

