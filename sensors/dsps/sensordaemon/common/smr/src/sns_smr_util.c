#define __SNS_MODULE__ SNS_SMR

#ifndef SNS_SMR_C
#define SNS_SMR_C

/*============================================================================

  @file sns_smr_util.c

  @brief

  Copyright (c) 2010-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/

/*============================================================================

                                INCLUDE FILES

============================================================================*/
#include "sns_osa.h"
#include "sns_memmgr.h"
#include "sns_smr_util.h"
#include "sns_common.h"
#include "sns_debug_api.h"
#include "sns_queue.h"
#include "sns_em.h"
#include "qmi_idl_lib.h"
#include <stddef.h>
#include <string.h>
#include <stdbool.h>

/*===========================================================================

                              PREPROCESSOR DEFINITIONS AND CONSTANTS

===========================================================================*/

#define SMR_MAX_MSG_LEN         4096        /* 4KB */
#define SMR_RSVD_HEADER_LEN     64
#define SMR_MAX_BODY_LEN        (SMR_MAX_MSG_LEN - SMR_RSVD_HEADER_LEN) /* The max body length
                                                     allowed for sns_smr_msg_alloc() */
#define SNS_SMR_QMI_CLI_SIG 0x04
#define SNS_SMR_QMI_TIMER_SIG 0x08

/*===========================================================================

                              STATIC VARIABLES

===========================================================================*/
/* The max length of QMI encoded message */
static uint16_t sns_smr_qmi_max_encode_len = 0;

/*===========================================================================

                              Function Definitions

===========================================================================*/

/*===========================================================================

  FUNCTION:   sns_smr_get_qmi_service_info

===========================================================================*/
sns_err_code_e sns_smr_get_qmi_service_info (
    qmi_idl_service_object_type const *svc_obj,
    uint32_t timeout_ms, qmi_service_info *svc_info)
{
  qmi_client_type notifier_handle;
  qmi_client_error_type qmi_err;
  qmi_cci_os_signal_type os_params_tmp;
  qmi_service_info svc_info_array[ SMR_MAX_QMI_SVC_CNT ];
  qmi_service_instance instance_id;
  sns_err_code_e rv = SNS_SUCCESS;
  uint32_t svc_num = -1,
           num_entries = SMR_MAX_QMI_SVC_CNT,
           num_services,
           i,
           max_iid,
           max_idx;

  qmi_idl_get_service_id( *svc_obj, &svc_num );

#if defined(SNS_DSPS_BUILD) || defined(SNS_PCSIM) || defined(QDSP6)
  os_params_tmp.ext_signal = NULL;
  os_params_tmp.sig = SNS_SMR_QMI_CLI_SIG;
  os_params_tmp.timer_sig = SNS_SMR_QMI_TIMER_SIG;
#endif

  qmi_err = qmi_client_notifier_init( *svc_obj, &os_params_tmp, &notifier_handle );
  if( QMI_NO_ERR != qmi_err )
  {
    SNS_PRINTF_STRING_ERROR_1( SMR_DBG_MODULE_ID,
        "qmi_client_notifier_init error %i", qmi_err );
    rv = SNS_ERR_FAILED;
  }
  else
  {
    QMI_CCI_OS_SIGNAL_WAIT( &os_params_tmp, timeout_ms );
    QMI_CCI_OS_SIGNAL_CLEAR( &os_params_tmp );
    if( os_params_tmp.timed_out )
    {
      SNS_PRINTF_STRING_ERROR_2( SMR_DBG_MODULE_ID,
              "Unable to initialize service %d with QCCI, timed out (%d ms)",
              svc_num, timeout_ms );
      rv = SNS_ERR_FAILED;
    }
    else if( QMI_NO_ERR != ( qmi_err =
          qmi_client_get_service_list( *svc_obj, svc_info_array,
                                       &num_entries, &num_services ) ) )
    {
      SNS_PRINTF_STRING_ERROR_1( SMR_DBG_MODULE_ID,
              "Unable to initialize service %d with QCCI",
              svc_num );
      rv = SNS_ERR_FAILED;
    }
    else if( num_entries > 0 )
    {
      if( num_services != num_entries )
      {
        SNS_PRINTF_STRING_LOW_2( SMR_DBG_MODULE_ID,
          "Too many service instances found (%i / %i)", num_services, num_entries );
      }
      for( i = 0, max_idx = 0, max_iid = 0; i < num_entries; i++ )
      {
        if( QMI_NO_ERR == ( qmi_err =
              qmi_client_get_instance_id( &svc_info_array[ i ], &instance_id) ))
        {
          // Choose the service with the largest instance ID
          if( max_iid < instance_id )
          {
            max_idx = i;
            max_iid = instance_id;
          }
        }
        else
        {
          SNS_PRINTF_STRING_MEDIUM_2( SMR_DBG_MODULE_ID,
              "Instance ID not found for service %i (%i)",
              svc_num, qmi_err );
        }
      }

      *svc_info = svc_info_array[ max_idx ];
    }
    else
    {
      SNS_PRINTF_STRING_ERROR_2( SMR_DBG_MODULE_ID,
              "No services found for num %i (%i)",
              svc_num, qmi_err );
      rv = SNS_ERR_FAILED;
    }

    qmi_client_release( notifier_handle );
  }
  return rv;
}

/*===========================================================================

  FUNCTION:   sns_smr_get_max_msg_len

===========================================================================*/
/*!
  @brief

  @return
   - Maximum message length of an sensors service

*/
/*=========================================================================*/
uint32_t sns_smr_get_max_msg_len()
{
#ifdef  SMR_ENCODE_ON
  return sns_smr_get_qmi_max_encode_msg_len() + sizeof(sns_smr_header_s);
#else
  return SMR_MAX_BODY_LEN + sizeof(sns_smr_header_s);
#endif
}

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
sns_err_code_e sns_smr_set_hdr(const sns_smr_header_s * header_type_ptr, void * body_ptr)
{
  SNS_OS_MEMCOPY((void*)GET_SMR_MSG_HEADER_PTR(body_ptr), (const void *)header_type_ptr,
         sizeof(sns_smr_header_s));
  return SNS_SUCCESS;
}

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
sns_err_code_e sns_smr_get_hdr(sns_smr_header_s * header_type_ptr, const void * body_ptr)
{
  SNS_OS_MEMCOPY((void *)header_type_ptr, (const void*)GET_SMR_MSG_HEADER_PTR(body_ptr),
         sizeof(sns_smr_header_s));
  return SNS_SUCCESS;
}

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
uint16_t sns_smr_get_qmi_max_encode_msg_len (void)
{
  if( 0 == sns_smr_qmi_max_encode_len )
  {
    uint8_t  i;
    uint32_t svc_len, max_svc_len = 0;
    for (i = 0; i < SNS_SMR_RTB_SIZE; i++)
    {
      /* if the entry has a qmi_svc_obj, the entry is for a valied service id */
      if (NULL != sns_rtb[i].qmi_svc_obj)
      {
        qmi_idl_get_max_service_len(sns_rtb[i].qmi_svc_obj, &svc_len);
        max_svc_len = MAX(svc_len, max_svc_len);
      }
    }
    sns_smr_qmi_max_encode_len = (uint16_t)max_svc_len;
  }

  return sns_smr_qmi_max_encode_len;
}


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
uint16_t sns_smr_get_qmi_max_decode_msg_len (void)
{
  return SMR_MAX_BODY_LEN;
}

/*===========================================================================

  FUNCTION:   sns_smr_msg_alloc

===========================================================================*/
/*!
  @brief this function allocates message body and header, and returns the body pointer.

  @param[i] body_size is the message body size to be allocated

  @return
  NULL if failed, or a pointer to the newly allocated message body

*/
/*=========================================================================*/
void * sns_smr_msg_alloc (sns_debug_module_id_e src_module, uint16_t body_size)
{
  UNREFERENCED_PARAMETER(src_module);
  smr_msg_s * msg_ptr;

  if ( SMR_MAX_BODY_LEN < body_size )
  {
    SNS_PRINTF_STRING_ID_HIGH_2(SMR_DBG_MODULE_ID,DBG_SMR_ALLOC_ERR,
                                body_size, SMR_MAX_BODY_LEN);
    SNS_ASSERT ( false );
  }

  msg_ptr = (smr_msg_s*)
  SNS_OS_MALLOC(src_module, (uint16_t)(SMR_MSG_HEADER_BLK_SIZE + body_size));
  if (msg_ptr != NULL)
  {
    sns_q_link(msg_ptr, &msg_ptr->q_link);
#ifdef SNS_DEBUG
    msg_ptr->msg_marker = SMR_MSG_MARKER;
#endif
    SNS_OS_MEMSET((void *)msg_ptr->body, 0, body_size);
    return ((void *)msg_ptr->body);
  }
  else
  {
    return NULL;
  }
}

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
void sns_smr_msg_free (void * body_ptr)
{
  if( NULL != body_ptr )
  {
#ifdef SNS_DEBUG
    SNS_ASSERT(IS_SMR_MSG_MARKER(((smr_msg_s *)(GET_SMR_MSG_PTR(body_ptr)))->msg_marker));
#endif
    SNS_OS_FREE( GET_SMR_MSG_PTR(body_ptr) );
  }
}

/*===========================================================================

  FUNCTION:  smr_set_qmi_service_obj

===========================================================================*/
/**
  @brief set service object into the routing table

  @detail

  @return None

*/
/*=========================================================================*/
void smr_set_qmi_service_obj (void)
{
  uint8_t i;
  for (i = 0; i < SNS_SMR_RTB_SIZE; i++)
  {
    SNS_ASSERT (i == sns_rtb[i].qmi_svc_num);
    if ( NULL != sns_rtb[i].svc_map.get_svc_obj )
    {
      sns_rtb[i].qmi_svc_obj = sns_rtb[i].svc_map.get_svc_obj(
          sns_rtb[i].svc_map.maj_ver,
          sns_rtb[i].svc_map.min_ver,
          sns_rtb[i].svc_map.tool_ver);
    }
  }
}

/*===========================================================================

  FUNCTION:   sns_smr_get_svc_obj

===========================================================================*/
/*!
  @brief This function returns a service object by using the routing table.

  @param[i] svc_num: The service number which determines the service object

  @return
   The service object or NULL if the svc_num is not defined or invalid

*/
/*=========================================================================*/
qmi_idl_service_object_type  sns_smr_get_svc_obj (uint8_t svc_num)
{
  if ( svc_num < SNS_SMR_RTB_SIZE )
  {
    return sns_rtb[svc_num].qmi_svc_obj;
  }
  else
  {
    return NULL;
  }
}

/*===========================================================================

  FUNCTION:   sns_smr_get_rtb_size

===========================================================================*/
/**
  @brief Get the number of items in the SMR routing table.

  @return Service count

*/
/*=========================================================================*/
uint32_t sns_smr_get_rtb_size (void)
{
  return SNS_SMR_RTB_SIZE;
}

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
uint8_t  sns_smr_get_module_id (uint8_t svc_num)
{
  if ( svc_num < SNS_SMR_RTB_SIZE )
  {
    return sns_rtb[svc_num].module_id;
  }
  else
  {
    return 0xFF;
  }
}

#endif /* SNS_SMR_C */
