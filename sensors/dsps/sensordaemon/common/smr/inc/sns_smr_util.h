#ifndef SNS_SMR_UTIL_H
#define SNS_SMR_UTIL_H

/*============================================================================

  @file sns_smr_util.h

  @brief

  Copyright (c) 2010-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/

/*===========================================================================

                   Include Files

===========================================================================*/
#include  "sns_queue.h"
#include  "sns_common.h"
#include  "sensor1.h"
#include  "qmi_idl_lib.h"
#include  "comdef.h" /* For PACK */
#include  "sns_debug_api.h"

#ifdef SNS_BLAST
#include "sns_debug_str_mdm.h"
#else
#include "sns_debug_str.h"
#endif

#include  "qmi_client.h"
#include  "qmi_csi.h"
#define SNS_PACK(X) PACK(X)

/*============================================================================

                        Type Declarations

=============================================================================*/
typedef enum {
  SNS_SMR_MSG_TYPE_REQ = SENSOR1_MSG_TYPE_REQ,
  SNS_SMR_MSG_TYPE_RESP = SENSOR1_MSG_TYPE_RESP,
  SNS_SMR_MSG_TYPE_IND = SENSOR1_MSG_TYPE_IND,
  SNS_SMR_MSG_TYPE_RESP_INT_ERR = SENSOR1_MSG_TYPE_RESP_INT_ERR,
  SNS_SMR_MSG_TYPE_LAST,
} sns_smr_msg_type_e;

typedef enum {
  SNS_SMR_MSG_PRI_LOW = 0,
  SNS_SMR_MSG_PRI_HIGH,
  SNS_SMR_MSG_PRI_LAST
} sns_smr_msg_pri_e;

/*
 * Services during their QMI registration may choose to set an "Instance ID".
 * This ID is used by Sensors clients as "priority" field; in case multiple
 * services exist, the one with the largest instance ID will be used.  In most
 * cases, services registered on the ADSP are done so with higher priority.
 */
typedef enum {
  SNS_SMR_SVC_PRI_LOW = 0,
  SNS_SMR_SVC_PRI_MED = 50,
  SNS_SMR_SVC_PRI_HIGH = 100,
} sns_smr_svc_pri_e;

#if defined(_WIN32)
#pragma pack(push,1)
#endif

typedef SNS_PACK(struct)
{
  uint8_t  dst_module;       /* Destination module ID within the Sensors framework */
  uint8_t  src_module;       /* Source module ID within the Sensors framework */
  uint8_t  priority;         /* The message priority */
  uint8_t  txn_id;           /* SMR client provided transaction ID */
  uint8_t  ext_clnt_id;      /* External client ID provided by a SMR client */
  uint8_t  msg_type;         /* The message type- refer sns_smr_msg_type_e */
  uint8_t  for_align0;       /* For alignment */
  uint8_t  svc_num;          /* The service number defined in the QMI header file */
  uint16_t msg_id;           /* The message identifier */
  uint16_t body_len;         /* The body length. i.e. header length is excluded */
#if ( defined(SNS_DSPS_BUILD) || defined(SNS_PCSIM) )
  void*    connection_handle;
  qmi_req_handle req_handle;
#endif
 } sns_smr_header_s;

#if defined(_WIN32)
#pragma pack(pop)
#endif

/* ----------------------------------------------------------------------------
 *  Definition of the message structure which includes header,body, and other stuff
 *  such as the queue link field.
 * --------------------------------------------------------------------------*/
typedef struct smr_msg_s
{
  sns_q_link_s        q_link;
#ifdef SNS_DEBUG
  uint32_t            msg_marker;
#endif
  sns_smr_header_s    header;
  uint8_t             body[1];
} smr_msg_s;

/*===========================================================================

                            MACROS

===========================================================================*/
#if defined(SNS_DSPS_BUILD) || defined(SNS_PCSIM)
#  define SMR_DBG_MODULE_ID SNS_DBG_MOD_DSPS_SMR
#elif defined(SNS_SMR_EXTRA_DEBUG)
#  define SMR_DBG_MODULE_ID SNS_DBG_MOD_APPS_SMR
#else
#  define SMR_DBG_MODULE_ID SNS_DBG_MOD_DSPS_SMR
#endif /* if defined(SNS_DSPS_BUILD) || defined(SNS_PCSIM) */

#define SMR_MSG_HEADER_BLK_SIZE ((uint32_t)offsetof(smr_msg_s, body))
#define GET_SMR_MSG_PTR(body_ptr) \
        ((smr_msg_s*)((uint8_t*)body_ptr - SMR_MSG_HEADER_BLK_SIZE))
#define GET_SMR_MSG_HEADER_PTR(body_ptr) \
        (&GET_SMR_MSG_PTR(body_ptr)->header)

#define SMR_MSG_MARKER 0xAAAA5555
#define IS_SMR_MSG_MARKER(marker) ((marker)==SMR_MSG_MARKER)

/* Maximum number of service instances to look-up */
#define SMR_MAX_QMI_SVC_CNT 5

/*===========================================================================

                    INTERNAL FUNCTION PROTOTYPES

===========================================================================*/

/*===========================================================================

  FUNCTION:   sns_smr_get_qmi_service_info

===========================================================================*/
/*!
  @brief Look-up the service info; if multiple are found, choose the service
         with the largest instance ID.

  @param[i] svc_obj: QMI service object for the sensor service to look-up.
  @param[i] timeout_ms: milliseconds to wait for the service.
  @param[o] svc_info: The QMI service info for a particular sensor service.

  @return
   - SNS_SUCCESS or error code (in which case svc_info should not be used)

*/
/*=========================================================================*/
sns_err_code_e sns_smr_get_qmi_service_info (qmi_idl_service_object_type const *svc_obj,
    uint32_t timeout_ms, qmi_service_info *svc_info);

/*===========================================================================

  FUNCTION:   sns_smr_get_max_msg_len

===========================================================================*/
/*!
  @brief

  @return
   - Maximum message length of an sensors service

*/
/*=========================================================================*/
uint32_t sns_smr_get_max_msg_len (void);

/*===========================================================================

  FUNCTION:  sns_smr_get_qmi_max_encode_msg_len

===========================================================================*/
/**
  @brief
    This function returns the maximum encoded message length for all services
    which are used in this framework.

  @detail

  @return the maximum encoded message length

*/
/*=========================================================================*/
uint16_t sns_smr_get_qmi_max_encode_msg_len (void);

/*===========================================================================

  FUNCTION:  sns_smr_get_qmi_max_decode_msg_len

===========================================================================*/
/**
  @brief
    This function returns the maximum decoded message length for all services
    which are used in this framework.

  @detail

  @return the maximum decoded message length

*/
/*=========================================================================*/
uint16_t sns_smr_get_qmi_max_decode_msg_len (void);

/*===========================================================================

  FUNCTION:   sns_smr_msg_alloc

===========================================================================*/
/*!
  @brief this function allocates message body and header and returns the body pointer.

  @param[i] body_size is the message body size to be allocated

  @return
  NULL if failed, or a pointer to the newly allocated message body

*/
/*=========================================================================*/
void * sns_smr_msg_alloc(sns_debug_module_id_e src_module, uint16_t body_size);

/*===========================================================================

  FUNCTION:   sns_smr_msg_free

===========================================================================*/
/*!
  @brief This function frees the message header and body allocated by sns_smr_msg_alloc().

  @param[i] body_ptr: A pointer variable to the message body to be freed

  @return
   None

*/
/*=========================================================================*/
void sns_smr_msg_free(void * body_ptr);

/*===========================================================================

  FUNCTION:   sns_smr_set_hdr

===========================================================================*/
/*!
  @brief This function sets message header information with the parameters delivered using
         sns_smr_header_s structure type.
         The address of the message header is calculated from body_ptr.

  @param[i] header_type_ptr: A pointer to the header structure type variable which includes all parameters.
            The message header is identified by body_ptr.
  @param[i] body_ptr: A pointer to the message body allocated by sns_smr_msg_alloc

  @return
   - SNS_SUCCESS if the message header was set successfully.
   - All other values indicate an error has occurred.

*/
/*=========================================================================*/
sns_err_code_e sns_smr_set_hdr(const sns_smr_header_s * header_type_ptr, void * body_ptr);


/*===========================================================================

  FUNCTION:   sns_smr_get_hdr

===========================================================================*/
/*!
  @brief This function gets message header information into sns_smr_header_s structure type.

  @param[o] header_type_ptr: A pointer to the header structure type in which
            the header informaiton will be retrieved
  @param[i] body_ptr: A pointer to the message body allocated by sns_smr_msg_alloc

  @return
   - SNS_SUCCESS if the message header was gotten successfully.
   - All other values indicate an error has occurred.
*/
/*=========================================================================*/
sns_err_code_e sns_smr_get_hdr(sns_smr_header_s * header_type_ptr, const void * body_ptr);

/*===========================================================================

  FUNCTION:   sns_smr_get_svc_obj

===========================================================================*/
/*!
  @brief This function returns a service object by using the routing table.

  @param[i] svc_num: The service number which determines the service object

  @return
   The service object or NULL if the svc_num is not defined

*/
/*=========================================================================*/
qmi_idl_service_object_type  sns_smr_get_svc_obj (uint8_t svc_num);

/*===========================================================================

  FUNCTION:   sns_smr_get_rtb_size

===========================================================================*/
/*!
  @brief  Retrieve the number of entries in the smr routing table

  @detail

  @return
   Number of entries in the table.

*/
/*=========================================================================*/
uint32_t sns_smr_get_rtb_size (void);

/*===========================================================================

  FUNCTION:   sns_smr_get_module_id

===========================================================================*/
/*!
  @brief This function returns the module_id from the routing table.

  @param[i] svc_num: The service number which determines the module id

  @return
   0xFF if svc_num is invalid

*/
/*=========================================================================*/
uint8_t sns_smr_get_module_id (uint8_t);

#endif /* SNS_SMR_UTIL_H */
