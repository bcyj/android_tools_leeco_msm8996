/*============================================================================
  @file sns_acm.c

  @brief
    This implements the sensor1 APIs, and contains the Application Client
    Manager (ACM). It bridges the SM and application clients.

  <br><br>

  DEPENDENCIES: This uses OS services defined in sns_osa.h.
    It uses events/timers defined in sns_em.h.

  Copyright (c) 2010-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ============================================================================*/

/*============================================================================
  INCLUDE FILES
  ============================================================================*/
#include "sensor1.h"
#include "sns_acm.h"
#include "sns_common.h"
#include "sns_common_v01.h"
#ifdef SNS_BLAST
#include "sns_debug_str_mdm.h"
#else
#include "sns_debug_str.h"
#endif
#include "sns_em.h"
#include "sns_init.h"
#include "sns_log_api.h"
#include "sns_memmgr.h"
#include "sns_osa.h"
#include "sns_pwr.h"
#include "sns_queue.h"
#include "sns_smr_util.h"
#include "sns_debug_api.h"
#include "sns_smgr_api_v01.h"
#include "sns_sam_amd_v01.h"
#include "sns_sam_rmd_v01.h"
#include "sns_sam_vmd_v01.h"
#include "sns_acm_priv.h"
#include "qmi_idl_lib.h"

#include <stdbool.h>
#include <stdint.h>

/*============================================================================
  Preprocessor Definitions and Constants
  ============================================================================*/

#define SNS_ACM_WRITABLE_POLL_USEC 100000 /* 100ms */

//#define SNS_ACM_DEBUG
#ifdef SNS_ACM_DEBUG
#  define SNS_ACM_DEBUG0( level, msg )          \
  SNS_PRINTF_STRING_##level##_0( SNS_MOD, (msg) )
#  define SNS_ACM_DEBUG1( level, msg, p1 )            \
  SNS_PRINTF_STRING_##level##_1( SNS_MOD, (msg), p1 )
#  define SNS_ACM_DEBUG2( level, msg, p1, p2 )        \
  SNS_PRINTF_STRING_##level##_2( SNS_MOD, (msg), p1, p2 )
#  define SNS_ACM_DEBUG3( level, msg, p1, p2, p3 )          \
  SNS_PRINTF_STRING_##level##_3( SNS_MOD, (msg), p1, p2, p3 )
#else /* SNS_ACM_DEBUG */
#  define SNS_ACM_DEBUG0( level, msg )
#  define SNS_ACM_DEBUG1( level, msg, p1 )
#  define SNS_ACM_DEBUG2( level, msg, p1, p2 )
#  define SNS_ACM_DEBUG3( level, msg, p1, p2, p3 )
#endif /* SNS_ACM_DEBUG */

/* Macros used for accessing bits in a bit array */
#define ISBITSET(array,i) (((array)[(i)>>3] &  (1<<((i)&7)))!=0)
#define SETBIT(array,i)     (array)[(i)>>3] |= (1<<((i)&7));
#define CLEARBIT(array,i)   (array)[(i)>>3] &= (1<<((i)&7))^0xFF;

/* Macros used to determine type of message */
#define SNS_ACM_IS_SMGR_SAM_CANCEL_RESP( sn, mid )    \
  ( ( (SNS_SAM_AMD_SVC_ID_V01 == (sn)) ||             \
      (SNS_SAM_RMD_SVC_ID_V01 == (sn)) ||             \
      (SNS_SAM_RMD_SVC_ID_V01 == (sn)) ||             \
      (SNS_SMGR_SVC_ID_V01 == (sn)) )                 \
    &&                                                \
    ( 0 == mid ) )

#define SNS_ACM_IS_SAM_ENABLE_REQ( sn, mid ) \
  ((SNS_SAM_AMD_SVC_ID_V01 == sn && SNS_SAM_AMD_ENABLE_REQ_V01 == mid) || \
   (SNS_SAM_RMD_SVC_ID_V01 == sn && SNS_SAM_RMD_ENABLE_REQ_V01 == mid) || \
   (SNS_SAM_RMD_SVC_ID_V01 == sn && SNS_SAM_VMD_ENABLE_REQ_V01 == mid) )


#define SNS_ACM_IS_SAM_ENABLE_RESP( sn, mid ) \
  ((SNS_SAM_AMD_SVC_ID_V01 == sn && SNS_SAM_AMD_ENABLE_RESP_V01 == mid) || \
   (SNS_SAM_RMD_SVC_ID_V01 == sn && SNS_SAM_RMD_ENABLE_RESP_V01 == mid) || \
   (SNS_SAM_RMD_SVC_ID_V01 == sn && SNS_SAM_VMD_ENABLE_RESP_V01 == mid) )

#define SNS_ACM_IS_SAM_DISABLE_REQ( sn, mid ) \
  ((SNS_SAM_AMD_SVC_ID_V01 == sn && SNS_SAM_AMD_DISABLE_REQ_V01 == mid) || \
   (SNS_SAM_RMD_SVC_ID_V01 == sn && SNS_SAM_RMD_DISABLE_REQ_V01 == mid) || \
   (SNS_SAM_RMD_SVC_ID_V01 == sn && SNS_SAM_VMD_DISABLE_REQ_V01 == mid) )


#define SNS_ACM_IS_SMGR_REPORT_REQ( sn, mid ) \
   ( SNS_SMGR_SVC_ID_V01 == sn && SNS_SMGR_REPORT_REQ_V01 == mid )

#define SNS_ACM_IS_SMGR_REPORT_RESP( sn, mid ) \
   ( SNS_SMGR_SVC_ID_V01 == sn && SNS_SMGR_REPORT_RESP_V01 == mid )

/*============================================================================
  Type Declarations
  ============================================================================*/

/**
 * This type is used to track a client's open/closed state
 */
typedef enum sns_acm_cli_state_e {
  SNS_ACM_CLI_STATE_CLOSED,
  SNS_ACM_CLI_STATE_OPENED,
} sns_acm_cli_state_e;

/**
 * When creating a rate node, this is the action performed by the message.
 */
typedef enum sns_acm_rate_state_e {
  SNS_ACM_RATE_OTHER,
  SNS_ACM_RATE_SAM_ENABLE,
  SNS_ACM_RATE_SAM_DISABLE,
  SNS_ACM_RATE_SMGR_ADD,
  SNS_ACM_RATE_SMGR_DELETE
} sns_acm_rate_state_e;

/**
 * This type is used to in the timer callback to determine what type of
 * timer has expired.
 */
typedef enum sns_acm_tmr_e {
  SNS_ACM_TMR_WRITABLE
} sns_acm_tmr_e;

/**
 * This contains data needed to call a client's write callback function.
 * The write callback is de-registered immediately before the callback has been
 * called.
 */
typedef struct sns_acm_write_cb_info_s {
  sensor1_write_cb_t               write_cb;
  intptr_t                         cb_data;
  uint32_t                         service_id;
  struct sns_acm_write_cb_info_s  *next;
} sns_acm_write_cb_info_s;


/**
 * This is used to maintain a sorted list of the report rates of the various
 * sensor1 clients
 */
typedef struct sns_acm_rate_s {
  uint32_t  service_number;
  int32_t   msg_id;
  uint8_t   txn_id;

  /* smgr uses report_id and sam uses instance id */
  uint8_t   report_id;
  uint32_t  report_rate;
  bool      confirmed;
  struct sns_acm_rate_s* next;
} sns_acm_rate_s;


/**
 * A client handle. Contains all necessary information to keep track of
 * an external client.
 */
struct sensor1_handle_s {
  sns_acm_cli_state_e       cli_state;
  sensor1_notify_data_cb_t  notify_cb;
  intptr_t                  notify_data;
  sns_acm_write_cb_info_s  *write_cb_info;
  int32_t                   outstanding_reqs;

  /* The following are used to determine which services a client
   * has used. This will be used to clean up when the client closes the
   * connection.
   */
  int32_t                   max_svc_used;
  uint8_t                   svcs_used[ (SNS_ACM_MAX_SVC_ID+7)/8 ];

  /* list of report rates associated with the client */
  sns_acm_rate_s*           rate_ptr;
};


/*============================================================================
 * Forward Function Declarations
 ============================================================================*/
static int32_t sns_acm_get_client_index( const sensor1_handle_s* );

/*============================================================================
  Static Variable Definitions
  ============================================================================*/
/**
 * The list of clients.
 */
static sensor1_handle_s sns_acm_db[SNS_ACM_MAX_CLIENTS];

/**
 * Mutex protecting the global list of clients
 */
static OS_EVENT *sns_acm_db_mutex = (OS_EVENT*) NULL;

/**
 * Global variable to see if CM has been initializied.
 */
static bool sns_acm_initialized = false;

/**
 * When this timer expires, it is time to callback all of the writable
 * callbacks.
 */
static sns_em_timer_obj_t sns_acm_writable_tmr_ptr = (sns_em_timer_obj_t) NULL;

/**
 * The max report rate across all ACM clients
 */
static int32_t sns_acm_max_rate = -1;

/**
 * Mutex which ensures proper log order in write
 */
static OS_EVENT *sns_acm_log_mutex = (OS_EVENT*) NULL;

/*============================================================================
  Timer handlers -- called from interrupt/signal context
  ============================================================================*/
/*===========================================================================

  FUNCTION:   sns_acm_timer_cb

  ===========================================================================*/
/*!
  @brief Handles timer callbacks for the CM.

  @param[i] cb_data: data used when registering the callback. Defines
  which timer has expired. Of type sns_acm_tmr_e.

  @return
  None.

  @sideeffects
  May sigal (flags) the ACM task to handle the work of timer expry.
*/
/*=========================================================================*/
static void
sns_acm_timer_cb( void* cb_data )
{
  uint8_t err;
  sns_acm_tmr_e tmr_type = (sns_acm_tmr_e)cb_data;

  if( SNS_ACM_TMR_WRITABLE == tmr_type ) {
    SNS_ACM_DEBUG0( LOW, "Setting ACM writable sig" );
    sns_os_sigs_post( sns_acm_flag_grp,
                      SNS_ACM_WRITABLE_FLAG,
                      OS_FLAG_SET, &err );
  }
}

/*============================================================================
  Static Function Definitions and Documentation
  ============================================================================*/
static int32_t sns_acm_max_clients() { return SNS_ACM_MAX_CLIENTS; }
static sensor1_handle_s *sns_acm_client_handle( int i ) { return &sns_acm_db[ i ]; }

/*===========================================================================

  FUNCTION:   sns_acm_recalculate_latency

  ===========================================================================*/
/*!
  @brief  Calculates the overall latency requirement for the sensor subsystem

  @return
  None.

  @sideeffects
  Sets the CPU latency to the newly calculated value
*/
/*=========================================================================*/
static void
sns_acm_recalculate_latency( void )
{
  int i;
  int max_rate =  -1;

  if( sns_acm_max_rate != -1 ) {
    for( i = 0; i < SNS_ACM_MAX_CLIENTS; i++ ) {
      if( SNS_ACM_CLI_STATE_CLOSED != sns_acm_client_handle( i )->cli_state &&
          sns_acm_client_handle( i )->rate_ptr != NULL ) {
        if( (int)sns_acm_client_handle( i )->rate_ptr->report_rate > max_rate ) {
          max_rate = sns_acm_client_handle( i )->rate_ptr->report_rate;
        }
      }
    }
    if( max_rate != sns_acm_max_rate ) {
      /* todo - algo behavior i.e. if rate is 0 set a timer for 10 msec */
      sns_acm_max_rate = max_rate;
      sns_pwr_set_cpu_latency( sns_acm_max_rate );
    }
  }
}

/*===========================================================================

  FUNCTION:   sns_acm_remove_all_rate_nodes

  ===========================================================================*/
/*!
  @brief  Removes all report rate nodes associated with a sensor1 client.

  @param c_ptr: handle to identify the sensor1 client

  @note sns_acm_db_mutex should be acquired before calling this function

  @sideeffects
  Recalculates the CPU latency
*/
/*=========================================================================*/
static void
sns_acm_remove_all_rate_nodes( sensor1_handle_s *c_ptr )
{
  sns_acm_rate_s *_ptr1, *_ptr2;
  _ptr2 = c_ptr->rate_ptr;

  while( _ptr2 != NULL )
  {
    _ptr1 = _ptr2;
    _ptr2 = _ptr2->next;

    SNS_OS_FREE( _ptr1 );
  }
  c_ptr->rate_ptr = NULL;
  sns_acm_recalculate_latency();
}

/*===========================================================================

  FUNCTION:   sns_acm_cancel_rate_node

  ===========================================================================*/
/*!
  @brief Removes all a report rate tracking node for a particular service

  @param head_ptr: Head ptr to the list of rates for a specific client
  @param service_number: To identify the service number (SNS_SAM_AMD_SVC_ID_V01, etc)

  @note sns_acm_db_mutex should be acquired before calling this function

*/
/*=========================================================================*/
static void
sns_acm_cancel_rate_node( sensor1_handle_s* client_ptr,
                          uint32_t service_number )
{
  sns_acm_rate_s *_ptr1, *_ptr2;
  _ptr1 = NULL;
  _ptr2 = client_ptr->rate_ptr;

  while( NULL != _ptr2 && _ptr2->service_number == service_number ) {
    /* head node */
    client_ptr->rate_ptr = _ptr2->next;
    SNS_OS_FREE( _ptr2 );
    _ptr2 = client_ptr->rate_ptr;
  }

  while( NULL != _ptr2 ) {
    _ptr1 = _ptr2;
    _ptr2 = _ptr2->next;
    if( NULL != _ptr2 && _ptr2->service_number == service_number ) {
      _ptr1->next = _ptr2->next;
      SNS_OS_FREE( _ptr2 );
      _ptr2 = _ptr1;
    }
  }
  sns_acm_recalculate_latency();
}


/*===========================================================================

  FUNCTION:   sns_acm_remove_rate_node

  ===========================================================================*/
/*!
  @brief Removes a report rate tracking node

  @param head_ptr: Head ptr to the list of rates for a specific client
  @param unconfirmed: If true, uses txn_id as an identifier to locate a node
                      for which no response has been receieve. If false, uses
                      report_id as an identifier
  @param identifier: To identify the instance(sam), report(smgr) or txn_id(unconfirmed)
                     being deleted
  @param service_number: To identify the service number (SNS_SAM_AMD_SVC_ID_V01, etc)
  @param recalc_latency: recalculate the latency if the head node (fastest rate) is
                         deleted.

  @note sns_acm_db_mutex should be acquired before calling this function

*/
/*=========================================================================*/
static void
sns_acm_remove_rate_node( sensor1_handle_s* client_ptr,
                          bool unconfirmed,
                          uint8_t identifier,
                          uint32_t service_number,
                          bool recalc_latency )
{
  sns_acm_rate_s *_ptr1, *_ptr2;
  _ptr1 = NULL;
  _ptr2 = client_ptr->rate_ptr;

  if( NULL != _ptr2 &&
      _ptr2->service_number == service_number &&
      ((unconfirmed == false && _ptr2->report_id == identifier) ||
       (unconfirmed == true && _ptr2->confirmed == false && _ptr2->txn_id == identifier)) ) {
    /* head node */
    client_ptr->rate_ptr = _ptr2->next;
    SNS_OS_FREE( _ptr2 );
    /* only need to do this if the head node is deleted */
    if( recalc_latency ) {
      sns_acm_recalculate_latency();
    }
  } else {
    while( NULL != _ptr2 ) {
      _ptr1 = _ptr2;
      _ptr2 = _ptr2->next;
      if( NULL != _ptr2 &&
          _ptr2->service_number == service_number &&
          ((unconfirmed == false && _ptr2->report_id == identifier) ||
           (unconfirmed == true && _ptr2->confirmed == false && _ptr2->txn_id == identifier)) ) {
        _ptr1->next = _ptr2->next;
        SNS_OS_FREE( _ptr2 );
        break;
      }
    }
  }
}


/*===========================================================================

  FUNCTION:   sns_acm_insert_rate_node

  ===========================================================================*/
/*!
  @brief Insert a node to track another report

  @note sns_acm_db_mutex should be acquired before calling this function

  @param head_ptr: Head ptr to the list of rates for a specific client
  @param rate_ptr: Node to insert
*/
/*=========================================================================*/
static void
sns_acm_insert_rate_node( sensor1_handle_s* client_ptr,
                          sns_acm_rate_s* rate_ptr )
{
  sns_acm_rate_s *_ptr1, *_ptr2;

  /* Remove an existing rate node for this service/ID if it exists */
  sns_acm_remove_rate_node( client_ptr,
                            false, /* Assume any existing rate has been confirmed */
                            rate_ptr->report_id,
                            rate_ptr->service_number,
                            false );

  _ptr1 = client_ptr->rate_ptr;

  if( _ptr1 == NULL ||
      _ptr1->report_rate < rate_ptr->report_rate ) {
    client_ptr->rate_ptr = rate_ptr;
    rate_ptr->next = _ptr1;
  }
  else {
    _ptr2 = _ptr1->next;
    while( _ptr1 ) {
      if( _ptr2 == NULL ||
          _ptr2->report_rate < rate_ptr->report_rate ) {
        _ptr1->next = rate_ptr;
        rate_ptr->next = _ptr2;
        break;
      }
      _ptr1 = _ptr1->next;
      _ptr2 = _ptr2->next;
    }
  }
}

/*===========================================================================

  FUNCTION:   sns_acm_peek_resp_error

  ===========================================================================*/
/*!
  @brief  Peeks into a SAM/SMGR error response

  @param client_ptr: client handle
  @param msg_hdr: pointer to message header

*/
/*=========================================================================*/
static void
sns_acm_peek_resp_error( sensor1_handle_s* client_ptr,
                         sensor1_msg_header_s *msg_hdr )
{
  if( SNS_ACM_IS_SMGR_REPORT_REQ(msg_hdr->service_number, msg_hdr->msg_id) ||
      SNS_ACM_IS_SAM_ENABLE_REQ(msg_hdr->service_number, msg_hdr->msg_id) ) {
    sns_acm_remove_rate_node( client_ptr,
                              true,
                              msg_hdr->txn_id,
                              msg_hdr->service_number,
                              true );
  }
}


/*===========================================================================

  FUNCTION:   sns_acm_peek_response

  ===========================================================================*/
/*!
  @brief  Peeks into a SAM or SMGR response to a previous request

  This is needed to determine the instance id of the algorithm (SAM and to
  ensure that there was no error. In case of error the rate node is
  deleted.

  @param client_ptr: client handle
  @param msg_hdr: pointer to message header
  @param msg_ptr: pointer to message

*/
/*=========================================================================*/
static void
sns_acm_peek_response( sensor1_handle_s* client_ptr,
                       sensor1_msg_header_s *msg_hdr,
                       void* msg_ptr )
{
  const sns_common_resp_s_v01*  crsp_ptr = (sns_common_resp_s_v01*) msg_ptr;

  if( SNS_ACM_IS_SMGR_REPORT_RESP(msg_hdr->service_number, msg_hdr->msg_id) ||
      SNS_ACM_IS_SAM_ENABLE_RESP(msg_hdr->service_number, msg_hdr->msg_id) ) {

    if( crsp_ptr->sns_result_t != 0 ||
        crsp_ptr->sns_err_t != SENSOR1_SUCCESS ) {
      /* the report request failed */
      sns_acm_remove_rate_node( client_ptr, true, msg_hdr->txn_id,
                                msg_hdr->service_number, true );
    } else {
      /* the report request succeeded */
      sns_acm_rate_s* _ptr1 = client_ptr->rate_ptr;
      while( _ptr1 ) {
        if( _ptr1->service_number == msg_hdr->service_number &&
            _ptr1->confirmed == false &&
            _ptr1->txn_id == msg_hdr->txn_id ) {
          _ptr1->confirmed = true;
          if( SNS_ACM_IS_SAM_ENABLE_RESP( msg_hdr->service_number, msg_hdr->msg_id ) ) {
            /* ok to use qmd type since all algos have the instance id in the same place */
            _ptr1->report_id = ((sns_sam_qmd_enable_resp_msg_v01*) msg_ptr)->instance_id;
          } else if( SNS_ACM_IS_SMGR_REPORT_RESP(msg_hdr->service_number, msg_hdr->msg_id) ) {
            /* todo .. can look at the acknack field to make sure that the rate is
               what was asked for */
          }
          break;
        }
        _ptr1 = _ptr1->next;
      }
    }
  } else if ( SNS_ACM_IS_SMGR_SAM_CANCEL_RESP( msg_hdr->service_number, msg_hdr->msg_id ) ) {
    sns_acm_cancel_rate_node( client_ptr, msg_hdr->service_number );
  }
}

/*===========================================================================

  FUNCTION:   sns_acm_apply_rate_node

  ===========================================================================*/
/*!
  @brief  Applies the given rate node.

  @param[i] client_index: Index of the client in the ACM db
  @param[i] rate_ptr: Pointer to the rate node to use.
  @param[i] rate_state: How to use the given rate node.
*/
/*=========================================================================*/
static void
sns_acm_apply_rate_node( int32_t const client_index, sns_acm_rate_s * const rate_ptr,
                         sns_acm_rate_state_e rate_state )
{
  if( SNS_ACM_RATE_SMGR_ADD == rate_state ||
      SNS_ACM_RATE_SAM_ENABLE == rate_state ) {
    sns_acm_rate_s *rate_node = (sns_acm_rate_s*)SNS_OS_MALLOC(
        SNS_DBG_MOD_APPS_ACM, sizeof(sns_acm_rate_s));
    if( NULL == rate_node ) {
      SNS_PRINTF_STRING_ERROR_0( SNS_DBG_MOD_ACM,
          "Can't allocate rate pointer" );
      return ;
    }

    SNS_OS_MEMCOPY( rate_node, rate_ptr, sizeof(sns_acm_rate_s) );
    sns_acm_insert_rate_node( sns_acm_client_handle( client_index ), rate_node );

    /* update the frequency .. if needed */
    if( (int32_t)rate_ptr->report_rate > sns_acm_max_rate ) {
      sns_acm_max_rate = rate_ptr->report_rate;
      sns_pwr_set_cpu_latency( sns_acm_max_rate );
    }
  }
  else if( SNS_ACM_RATE_SMGR_DELETE == rate_state ||
           SNS_ACM_RATE_SAM_DISABLE == rate_state ) {
    sns_acm_remove_rate_node( sns_acm_client_handle( client_index ), false,
                              rate_ptr->report_id, rate_ptr->service_number, true );
  }
  else if( SNS_ACM_RATE_OTHER != rate_state ) {
    SNS_PRINTF_STRING_ERROR_1( SNS_DBG_MOD_ACM, "Invalid rate state %i", rate_state );
  }
}

/*===========================================================================

  FUNCTION:   sns_acm_peek_request

  ===========================================================================*/
/*!
  @brief  Peeks into SMGR and SAM enable and disable requests. Used to track
          all the various report and their associated rates.


  @param[i]  msg_hdr: pointer to message header
  @param[i]  msg_ptr: pointer to message
  @param[io] rate_ptr: Pre-allocated rate pointer.

  @return    The action interpreted from msg_ptr and msg_hdr.
*/
/*=========================================================================*/
static sns_acm_rate_state_e
sns_acm_peek_request( sensor1_msg_header_s const * const msg_hdr,
                      void const * const msg_ptr, sns_acm_rate_s * const rate_ptr )
{
  sns_acm_rate_state_e rv = SNS_ACM_RATE_OTHER;

  if( SNS_ACM_IS_SMGR_REPORT_REQ( msg_hdr->service_number, msg_hdr->msg_id ) ) {
    const sns_smgr_periodic_report_req_msg_v01 *smgr_req = msg_ptr;

    rate_ptr->report_id = smgr_req->ReportId;
    rate_ptr->service_number = msg_hdr->service_number;
    rv = SNS_ACM_RATE_SMGR_ADD;

    if( SNS_SMGR_REPORT_ACTION_ADD_V01 == smgr_req->Action ) {
      rate_ptr->txn_id = msg_hdr->txn_id;
      rate_ptr->next = NULL;
      rate_ptr->report_id = smgr_req->ReportId;
      rate_ptr->report_rate = smgr_req->ReportRate;
      rate_ptr->confirmed = false;

      rv = SNS_ACM_RATE_SMGR_DELETE;
    }
  }
  else if( SNS_ACM_IS_SAM_ENABLE_REQ( msg_hdr->service_number, msg_hdr->msg_id ) ) {
    const sns_sam_qmd_enable_req_msg_v01 *sam_req = msg_ptr;

    rate_ptr->service_number = msg_hdr->service_number;
    rate_ptr->txn_id = msg_hdr->txn_id;
    rate_ptr->report_id = 0xFF; /* this gets filled up in the response */
    rate_ptr->next = NULL;

    /* convert fom q16 (sec) to Hz. */
    rate_ptr->report_rate = (uint32_t)(65536.0/sam_req->report_period);
    rate_ptr->confirmed = false;

    rv = SNS_ACM_RATE_SAM_ENABLE;
  }
  else if( SNS_ACM_IS_SAM_DISABLE_REQ( msg_hdr->service_number, msg_hdr->msg_id ) ) {
    const sns_sam_qmd_disable_req_msg_v01*  sam_req = msg_ptr;
    rate_ptr->report_id = sam_req->instance_id;
    rate_ptr->service_number = msg_hdr->service_number;

    rv = SNS_ACM_RATE_SAM_DISABLE;
  }

  return rv;
}

/*===========================================================================

  FUNCTION:   sns_acm_init_client_handle

  ===========================================================================*/
/*!
  @brief Initializes an individual client handle.

  @param[i] c_ptr: Pointer to the handle to init.

  @return
  None

  @sideeffects
  None
*/
/*=========================================================================*/
static void
sns_acm_init_client_handle( sensor1_handle_s *c_ptr )
{
  size_t i;
  c_ptr->cli_state = SNS_ACM_CLI_STATE_CLOSED;
  c_ptr->notify_cb = NULL;
  c_ptr->write_cb_info = NULL;
  c_ptr->outstanding_reqs = 0;

  c_ptr->max_svc_used = -1;
  for( i = 0; i < sizeof(c_ptr->svcs_used); i++ ) {
    c_ptr->svcs_used[i] = 0;
  }
  c_ptr->rate_ptr = NULL;
}

/*===========================================================================

  FUNCTION:   sns_acm_client_count

  ===========================================================================*/
/*!
  @brief Returns the count of non-closed clients.

  @return
  Count of clients in-use.
  Negative value: error

*/
/*=========================================================================*/
static int_fast8_t
sns_acm_client_count( void )
{
  int_fast8_t i;
  int_fast8_t rv;
  uint8_t err;

  rv = -1;

  sns_os_mutex_pend( sns_acm_db_mutex, 0, &err );

  if( 0 == err ) {
    rv = 0;
    for( i = 0; i < SNS_ACM_MAX_CLIENTS; i++ ) {
      if( SNS_ACM_CLI_STATE_CLOSED != sns_acm_client_handle( i )->cli_state ) {
        rv ++;
      }
    }
    sns_os_mutex_post( sns_acm_db_mutex );
  } else {
    SNS_PRINTF_STRING_ERROR_1( SNS_DBG_MOD_ACM,
                               "Error %d getting mutex", (int32_t)err );
  }
  return rv;
}

/*===========================================================================

  FUNCTION:   sns_acm_new_client_handle

  ===========================================================================*/
/*!
  @brief Allocates and initializes a new client handle.

  @return
  Pointer to the client handle.

  @sideeffects
  Updates global list of client handles.
  Turns on the DSPS if this is the first client.
*/
/*=========================================================================*/
static sensor1_handle_s*
sns_acm_new_client_handle( void )
{
  int_fast8_t i;
  sensor1_handle_s *c_ptr = NULL;
  uint8_t err;

  if( 0 == sns_acm_client_count() ) {
    sns_pwr_on( SNS_PWR_VOTE_ACM );
  }

  sns_os_mutex_pend( sns_acm_db_mutex, 0, &err );

  if( 0 == err ) {
    for( i = 0; i < SNS_ACM_MAX_CLIENTS; i++ ) {
      if( SNS_ACM_CLI_STATE_CLOSED == sns_acm_client_handle( i )->cli_state ) {
        c_ptr = sns_acm_client_handle( i );
        sns_acm_init_client_handle( c_ptr );
        c_ptr->cli_state = SNS_ACM_CLI_STATE_OPENED;
        break;
      }
    }
    sns_os_mutex_post( sns_acm_db_mutex );
  } else {
    SNS_PRINTF_STRING_ERROR_1( SNS_DBG_MOD_ACM,
                               "Error %d getting mutex", (int32_t)err );
  }


  return c_ptr;
}

/*===========================================================================

  FUNCTION:   sns_acm_free_client_handle

  ===========================================================================*/
/*!
  @brief De-allocates a client handle, releasing resources allocated to this
  client.  Caller must hold sns_acm_db_mutex.

  @param[i] c_ptr: pointer to the client handle.

  @return
  None.

  @sideeffects
  Updates the global client list.
  Turns off the DSPS if there are no clients remaining.
*/
/*=========================================================================*/
static void
sns_acm_free_client_handle( sensor1_handle_s *c_ptr )
{
  sns_acm_write_cb_info_s *write_cb_info_ptr;
  int msg_removed_cnt,
      client_index;

  // TODO: Free memory allocated by this client handle
  while( NULL != c_ptr->write_cb_info ) {
    write_cb_info_ptr = c_ptr->write_cb_info;
    c_ptr->write_cb_info = c_ptr->write_cb_info->next;
    SNS_OS_FREE(write_cb_info_ptr);
  }

  // Release the client handle
  client_index = sns_acm_get_client_index( c_ptr );
  sns_acm_mr_close( client_index );
  msg_removed_cnt = sns_acm_mr_queue_clean( client_index );

  if( -1 == msg_removed_cnt )
  {
    SNS_PRINTF_STRING_ERROR_1( SNS_DBG_MOD_ACM,
                               "Error cleaning msg queue %i",
                               client_index );
  }
  else
  {
    c_ptr->outstanding_reqs -= msg_removed_cnt;
  }

  if( 0 != c_ptr->outstanding_reqs )
  {
    SNS_PRINTF_STRING_HIGH_1( SNS_DBG_MOD_ACM,
                              "Close with outstanding reqs %i",
                              c_ptr->outstanding_reqs );
  }

  c_ptr->cli_state = SNS_ACM_CLI_STATE_CLOSED;

  if( 0 == sns_acm_client_count() ) {
    sns_pwr_off( SNS_PWR_VOTE_ACM );
  }

  sns_acm_remove_all_rate_nodes( c_ptr );
}

/*===========================================================================

  FUNCTION:   sns_acm_get_client_index

  ===========================================================================*/
/*!
  @brief Given a pointer to a client handle, returns the index into the global
  client table for that client.

  @param[i] c_ptr: Client to look up.

  @return
  Index of the client handle
  -1 if no client is found.

  @sideeffects
  None.
*/
/*=========================================================================*/
static int32_t
sns_acm_get_client_index( const sensor1_handle_s *c_ptr )
{
  uintptr_t offset = (uintptr_t)c_ptr - (uintptr_t)sns_acm_db;

  if( (c_ptr != NULL) && (offset % sizeof(*sns_acm_db)) == 0 ) {
    return offset / sizeof(*sns_acm_db);
  }
  return -1;
}

/*===========================================================================

  FUNCTION:   sns_acm_verify_client_handle

  ===========================================================================*/
/*!
  @brief Verifies that the pointer to a client handle actually lies within the
  global client list, and the client is "in_use".

  @param[i] c_ptr: Pointer to the client handle to verify.

  @return
  - true: Handle is in the database
  - false: Handle is not in the database.

  @sideeffects
  None.
*/
/*=========================================================================*/
static bool
sns_acm_verify_client_handle( const sensor1_handle_s *c_ptr )
{
  uintptr_t offset = (uintptr_t)c_ptr - (uintptr_t)sns_acm_db;

  if( (c_ptr != NULL) &&
      (c_ptr >= sns_acm_db) &&
      ( (uintptr_t)c_ptr
        <= ((uintptr_t)sns_acm_db + sizeof(*sns_acm_db)*SNS_ACM_MAX_CLIENTS)) &&
      (offset % sizeof(*sns_acm_db)) == 0 ) {
    return (SNS_ACM_CLI_STATE_OPENED == c_ptr->cli_state);
  }
  return false;
}

/*===========================================================================

  FUNCTION:   sns_acm_add_client_write_cb_info

  ===========================================================================*/
/*!
  @brief Adds a write callback structure to a speficied client.

  By the current design, a client can have multiple callbacks -- but onle one
  for each service ID.

  @param[i] c_ptr: Pointer to the client

  @param[i]. w_ptr: Write callback structure to add to the client.

  @return
  - true: Callback structure successfully added.
  - false: Callback not added -- service ID already registered
*/
/*=========================================================================*/
static bool
sns_acm_add_client_write_cb_info( sensor1_handle_s *c_ptr,
                                  sns_acm_write_cb_info_s *w_ptr )
{
  bool return_value = false;
  uint8_t err;

  if( NULL == w_ptr ) {
    return false;
  }

  if( NULL == c_ptr->write_cb_info ) {
    c_ptr->write_cb_info = w_ptr;
    w_ptr->next = NULL;
    return_value = true;
  } else {

    sns_os_mutex_pend( sns_acm_db_mutex, 0, &err );

    if( 0 == err ) {
      sns_acm_write_cb_info_s *index_ptr;
      for( index_ptr = c_ptr->write_cb_info;
           index_ptr != NULL;
           index_ptr = index_ptr->next )
      {
        if( index_ptr->service_id == w_ptr->service_id ) {
          return_value = false;
          break;
        } else if( NULL == index_ptr->next ) {
          index_ptr->next = w_ptr;
          w_ptr->next = NULL;
          return_value = true;
          break;
        }
      }
      sns_os_mutex_post( sns_acm_db_mutex );
    } else {
      SNS_PRINTF_STRING_ERROR_1( SNS_DBG_MOD_ACM,
                                 "Error %d getting mutex", (int32_t)err );
    }
  }

  return return_value;
}

/*===========================================================================

  FUNCTION:   sns_acm_log_api_call

  ===========================================================================*/
/*!
  @brief Logs a call to sensor1_open, sensor1_write, or sensor1_close

  @param[i] sensor1_fn: As defined in sns_log_sensor1_api_e. Open/Write/Close
  @param[i] sensor1_rv: Return value from sensor1 API call
  @param[i] context_handle: pointer to the client handle
  @param[i] msg_hdr: pointer to the sensor1 header for this message
  @param[i] log_ptr: pointer to allocated log message with body.

  @return
  None

  @sideeffects
  None
*/
/*=========================================================================*/
static void
sns_acm_log_api_call( sns_log_sensor1_api_e             sensor1_fn,
                      int32_t                           sensor1_rv,
                      const sensor1_handle_s            *c_ptr,
                      const sensor1_msg_header_s        *msg_hdr,
                      sns_log_sensor1_request_s * const log_ptr )
{
  sns_err_code_e err;
  uint32_t       logpkt_size;

  if( NULL != log_ptr ) {
    /* Fill in the log packet */
    log_ptr->version = SNS_LOG_STRUCT_VERSION;

#if defined(SNS_BLAST)
    log_ptr->logging_processor = 1;
#else
    log_ptr->logging_processor = 2;
#endif /* defined(SNS_BLAST) */

    err = sns_em_get_timestamp64(&(log_ptr->timestamp));

    if( SNS_SUCCESS != err ) {
      // Fall back to 32-bit timestamp
      log_ptr->timestamp = sns_em_get_timestamp();
    }
    log_ptr->sensor1_fn = (uint8_t)sensor1_fn;
    log_ptr->sensor1_rv = sensor1_rv;
    log_ptr->ext_clnt_id = (uint8_t)sns_acm_get_client_index(c_ptr);

    if( NULL != c_ptr && sns_acm_verify_client_handle( c_ptr ) ) {
      log_ptr->cb_data = (uint64_t)(c_ptr->notify_data);
    } else {
      log_ptr->cb_data = 0;
    }

    log_ptr->context_handle = (uintptr_t)c_ptr;
    log_ptr->msg_type = SENSOR1_MSG_TYPE_REQ;

    if( NULL == msg_hdr ) {
      log_ptr->svc_num = 0;
      log_ptr->msg_id = 0;
      log_ptr->txn_id = 0;
    } else {
      log_ptr->svc_num = (uint8_t)msg_hdr->service_number;
      log_ptr->msg_id = (uint16_t)msg_hdr->msg_id;
      log_ptr->txn_id = msg_hdr->txn_id;
    }

    if( NULL == log_ptr->request || NULL == msg_hdr || 0 == msg_hdr->msg_size ) {
      log_ptr->request_size = 0;
    }

    logpkt_size = ( sizeof(sns_log_sensor1_request_s) +
                    log_ptr->request_size ) - 1;
    err = sns_logpkt_shorten( (void*)log_ptr, logpkt_size );

    err = sns_logpkt_commit( SNS_LOG_SENSOR1_REQUEST,
                             (void*)log_ptr );
  }
}

/*===========================================================================

  FUNCTION:   sns_acm_log_resp_or_ind

  ===========================================================================*/
/*!
  @brief Logs a response sent to a client.

  @param[i] context_handle: pointer to the client handle
  @param[i] msg_hdr: pointer to the sensor1 message header
  @param[i] msg_body: pointer to the cType for this message

  @return
  None

  @sideeffects
  None
*/
/*=========================================================================*/
static void
sns_acm_log_resp_or_ind( const sensor1_handle_s     *c_ptr,
                         sensor1_msg_type_e          msg_type,
                         const sensor1_msg_header_s *msg_hdr,
                         const void                 *msg_body )
{
  sns_log_sensor1_resp_ind_s  *log_ptr;
  sns_err_code_e               err;
  int32_t                      encode_result;
  sns_log_id_e                 log_type;
  qmi_idl_type_of_message_type qmi_type;
  uint32_t                     logpkt_size;

  logpkt_size = SNS_LOG_MAX_SIZE + sizeof(sns_log_sensor1_resp_ind_s);

  switch( msg_type ) {
    case SENSOR1_MSG_TYPE_RESP:
      log_type = SNS_LOG_SENSOR1_RESPONSE;
      qmi_type = QMI_IDL_RESPONSE;
      break;
    case SENSOR1_MSG_TYPE_IND:
      log_type = SNS_LOG_SENSOR1_INDICATION;
      qmi_type = QMI_IDL_INDICATION;
      break;
    case SENSOR1_MSG_TYPE_RESP_INT_ERR:
      log_type = SNS_LOG_SENSOR1_RESPONSE;
      qmi_type = QMI_IDL_NUM_MSG_TYPES; /* Invalid QMI type */
      msg_body = NULL;
      break;
    default:
      SNS_PRINTF_STRING_ERROR_1( SNS_DBG_MOD_ACM,
                                 "Logging invalid msg type %d",
                                 (int32_t)msg_type );
      return;
  }

  err = sns_logpkt_malloc( log_type,
                           logpkt_size,
                           (void**)(&log_ptr) );

  if( (err == SNS_SUCCESS) && (NULL != log_ptr) ) {
    /* Fill in the log packet */
    log_ptr->version = SNS_LOG_STRUCT_VERSION;

#if defined(SNS_BLAST)
    log_ptr->logging_processor = 1;
#else
    log_ptr->logging_processor = 2;
#endif /* defined(SNS_BLAST) */

    err = sns_em_get_timestamp64(&(log_ptr->timestamp));

    if( SNS_SUCCESS != err ) {
      // Fall back to 32-bit timestamp
      log_ptr->timestamp = sns_em_get_timestamp();
    }
    if( sns_acm_verify_client_handle( c_ptr ) ) {
      log_ptr->cb_data = (uint64_t)(c_ptr->notify_data);
    } else {
      log_ptr->cb_data = 0;
    }
    log_ptr->context_handle = (uintptr_t)c_ptr;
    log_ptr->msg_type = msg_type;
    log_ptr->ext_clnt_id = (uint8_t)sns_acm_get_client_index(c_ptr);

    if( NULL == msg_hdr ) {
      log_ptr->svc_num = 0;
      log_ptr->msg_id = 0;
      log_ptr->txn_id = 0;
    } else {
      log_ptr->svc_num = (uint8_t)msg_hdr->service_number;
      log_ptr->msg_id = (uint16_t)msg_hdr->msg_id;
      log_ptr->txn_id = msg_hdr->txn_id;
    }

    if( NULL == msg_body || NULL == msg_hdr ||
        0 == msg_hdr->msg_size ) {
      log_ptr->resp_ind_size = 0;
    } else {
      qmi_idl_service_object_type svc_obj;
      uint32_t resp_ind_size;

      svc_obj = sns_smr_get_svc_obj( (uint8_t)(msg_hdr->service_number) );
      encode_result = qmi_idl_message_encode( svc_obj,
                                              qmi_type,
                                              (uint16_t)(msg_hdr->msg_id),
                                              msg_body,
                                              msg_hdr->msg_size,
                                              (void*)log_ptr->resp_ind,
                                              SNS_LOG_MAX_SIZE,
                                              &resp_ind_size );
      log_ptr->resp_ind_size = (uint16_t)resp_ind_size;
    }
    logpkt_size = ( sizeof(sns_log_sensor1_request_s) +
                    log_ptr->resp_ind_size ) - 1;
    err = sns_logpkt_shorten( (void*)log_ptr, logpkt_size );

    err = sns_logpkt_commit( log_type, (void*)log_ptr );
  }
}

/*============================================================================
  CM thread
  These functions are called from the CM thread.
  ============================================================================*/
/*===========================================================================

  FUNCTION:   sns_acm_handle_rx

  ===========================================================================*/
/*!
  @brief Handles a receive indication from the message router

  @return
  None.

  @sideeffects
  Most likely calls an external client notify data callback function.
*/
/*=========================================================================*/
void
sns_acm_handle_rx( void * msg_ptr )
{
  sns_smr_header_s smr_header;
  sensor1_msg_header_s msg_hdr;
  sensor1_handle_s *client_ptr;
  sensor1_msg_type_e msg_type;
  uint8_t err;

  sns_smr_get_hdr( &smr_header, msg_ptr );

  if( sns_acm_max_clients() <= smr_header.ext_clnt_id ) {
    SNS_PRINTF_STRING_ERROR_1( SNS_DBG_MOD_ACM, "bad client ID %d\n",
                                (int32_t)smr_header.ext_clnt_id );
    sns_smr_msg_free( msg_ptr );
    return;
  }
  client_ptr = sns_acm_client_handle( smr_header.ext_clnt_id );

  msg_hdr.service_number = smr_header.svc_num;
  msg_hdr.msg_id = smr_header.msg_id;
  msg_hdr.txn_id = smr_header.txn_id;
  msg_hdr.msg_size = smr_header.body_len;

  sns_os_mutex_pend( sns_acm_db_mutex, 0, &err );
  if( 0 == err ) {
    if ( smr_header.msg_type == SNS_SMR_MSG_TYPE_RESP ) {
      msg_type = SENSOR1_MSG_TYPE_RESP;
      client_ptr->outstanding_reqs--;
      sns_acm_peek_response( client_ptr, &msg_hdr, msg_ptr );
    } else if ( smr_header.msg_type == SNS_SMR_MSG_TYPE_RESP_INT_ERR ) {
      msg_type = SENSOR1_MSG_TYPE_RESP_INT_ERR;
      client_ptr->outstanding_reqs--;
      sns_acm_peek_resp_error( client_ptr, &msg_hdr );
    } else {
      msg_type = SENSOR1_MSG_TYPE_IND;
    }

    if( (SNS_ACM_CLI_STATE_OPENED == client_ptr->cli_state) &&
        (NULL != client_ptr->notify_cb) ) {
      SNS_ACM_DEBUG1( LOW, "Delivering message to client %d",
                      smr_header.ext_clnt_id );

      sns_os_mutex_pend( sns_acm_log_mutex, 0, &err );
      if( 0 == err )
      {
        sns_acm_log_resp_or_ind( client_ptr, msg_type, &msg_hdr, msg_ptr );
        sns_os_mutex_post( sns_acm_log_mutex );
      } else {
        SNS_PRINTF_STRING_ERROR_1( SNS_DBG_MOD_ACM,
                                    "Error %d getting log mutex", (int32_t)err );
      }

      client_ptr->notify_cb( client_ptr->notify_data,
                             &msg_hdr,
                             msg_type,
                             msg_ptr );
    } else {
      sns_smr_msg_free( msg_ptr );
    }
    sns_os_mutex_post( sns_acm_db_mutex );
  } else {
    SNS_PRINTF_STRING_ERROR_1( SNS_DBG_MOD_ACM,
                                "Error %d getting mutex", (int32_t)err );
    sns_smr_msg_free( msg_ptr );
  }
}

/*===========================================================================

  FUNCTION:   sns_acm_handle_writable_timer

  ===========================================================================*/
/*!
  @brief Handles the writable flag being set (presumably by the writabl
  timer callback function).

  @sideeffects
  Removes all writable callbacks from the list of registered callbacks

  @return
  None.
*/
/*=========================================================================*/
static void
sns_acm_handle_writable_timer( void )
{
  int_fast8_t i;
  sns_acm_write_cb_info_s *w_info_ptr;
  sns_acm_write_cb_info_s *w_tmp_ptr;
  uint8_t err;

  SNS_ACM_DEBUG0( LOW, "Calling writable callbacks" );
  sns_os_mutex_pend( sns_acm_db_mutex, 0, &err );

  if( 0 == err ) {
    for( i = 0; i < SNS_ACM_MAX_CLIENTS; i++ ) {
      if( (SNS_ACM_CLI_STATE_OPENED == sns_acm_client_handle( i )->cli_state) &&
          (NULL != sns_acm_db[i].write_cb_info) ) {

        w_info_ptr = sns_acm_client_handle( i )->write_cb_info;
        sns_acm_client_handle( i )->write_cb_info = NULL;

        while( w_info_ptr != NULL ) {
          w_tmp_ptr = w_info_ptr;
          w_info_ptr = w_info_ptr->next;
          SNS_ACM_DEBUG2( LOW, "Calling writable cb. Client %d, svc_id %d",
                          i ,w_tmp_ptr->service_id );
          w_tmp_ptr->write_cb( w_tmp_ptr->cb_data, w_tmp_ptr->service_id );
          SNS_OS_FREE(w_tmp_ptr);

        }
      }
    }
    sns_os_mutex_post( sns_acm_db_mutex );
  } else {
    SNS_PRINTF_STRING_ERROR_1( SNS_DBG_MOD_ACM,
                               "Error %d getting mutex", (int32_t)err );
  }
}
/*===========================================================================

  FUNCTION:   sns_acm_rx_thread

  ===========================================================================*/
/*!
  @brief Main thread loop for the ACM receive functionality.

  @param[i] p_arg: Argument passed in during thread creation. Unused.
*/
/*=========================================================================*/
static void
sns_acm_rx_thread( void *p_arg ) {
  uint8_t err;
  OS_FLAGS flags;

  UNREFERENCED_PARAMETER( p_arg );

  sns_pwr_off( SNS_PWR_VOTE_ACM );

  SNS_PRINTF_STRING_MEDIUM_0( SNS_DBG_MOD_ACM,
                              "Initialization complete" );
  sns_init_done();

  for(;;) {
    flags = sns_os_sigs_pend( sns_acm_flag_grp,
                              SNS_ACM_SMR_RX_FLAG | SNS_ACM_WRITABLE_FLAG,
                              OS_FLAG_WAIT_SET_ANY | OS_FLAG_CONSUME,
                              0, &err );

    if( 0 != err ) {
      SNS_PRINTF_STRING_ERROR_1( SNS_DBG_MOD_ACM,
                                 "Error in sigs_pend %u", (uint32_t)err );

    }
    if( false == sns_acm_initialized ) {
      break;
    }
    if( flags & SNS_ACM_SMR_RX_FLAG ) {
      sns_acm_mr_handle();
    }
    if( flags & SNS_ACM_WRITABLE_FLAG ) {
      sns_acm_handle_writable_timer();
    }
  }
  sns_os_task_del( SNS_MODULE_PRI_ACM );
}

/*============================================================================
  Externalized Function Definitions
  ============================================================================*/

/*============================================================================
  One time init function
  ============================================================================*/
/*===========================================================================

  FUNCTION:   sns_acm_init

  ===========================================================================*/
/*!
  @brief Initializes the ACM functionality.

  Creates EM timer.
  Creates ACM RX thread.
  Registers with SMR.

  @return
  None.
*/
/*=========================================================================*/
sns_err_code_e
sns_acm_init( void )
{
  sns_err_code_e error_code;
  int32_t        i;
  uint8_t        err;
  int32_t        max_clients = sns_acm_max_clients();

  if( true == sns_acm_initialized ) {
    return SNS_SUCCESS;
  }

  error_code = sns_em_create_timer_obj( sns_acm_timer_cb,
                                        (void*)(intptr_t)SNS_ACM_TMR_WRITABLE,
                                        SNS_EM_TIMER_TYPE_ONESHOT,
                                        &sns_acm_writable_tmr_ptr);

  if( SNS_SUCCESS != error_code ) {
    SNS_PRINTF_STRING_FATAL_0( SNS_DBG_MOD_ACM,
                               "init: can't create EM timer object" );
    return SNS_ERR_NOMEM;
  }

  sns_acm_flag_grp = sns_os_sigs_create( 0, &err );
  if( sns_acm_flag_grp == NULL ) {
    sns_em_delete_timer_obj( sns_acm_writable_tmr_ptr );
    SNS_PRINTF_STRING_FATAL_0( SNS_DBG_MOD_ACM,
                               "init: can't create OS signal" );
    return SNS_ERR_NOMEM;
  }

  for( i = 0; i < max_clients; i++ ) {
    sns_acm_init_client_handle( sns_acm_client_handle( i ) );
  }

  sns_acm_db_mutex = sns_os_mutex_create( SNS_MODULE_PRI_ACM_MUTEX,
                                          &err );
  if( 0 != err ) {
    SNS_PRINTF_STRING_FATAL_0( SNS_DBG_MOD_ACM,
                               "init: can't create db mutex" );

    sns_em_delete_timer_obj( sns_acm_writable_tmr_ptr );
    return SNS_ERR_FAILED;
  }

  sns_acm_log_mutex = sns_os_mutex_create( SNS_MODULE_PRI_ACM_MUTEX,
                                          &err );
  if( 0 != err ) {
    SNS_PRINTF_STRING_FATAL_0( SNS_DBG_MOD_ACM,
                               "init: can't create log mutex" );

    sns_em_delete_timer_obj( sns_acm_writable_tmr_ptr );
    sns_os_mutex_del( sns_acm_db_mutex, 0, &err );
    return SNS_ERR_FAILED;
  }

  error_code = sns_acm_mr_init( sns_acm_flag_grp, SNS_ACM_SMR_RX_FLAG );

  if( SNS_SUCCESS != error_code ) {
    sns_em_delete_timer_obj( sns_acm_writable_tmr_ptr );
    sns_os_mutex_del( sns_acm_db_mutex, 0, &err );
    sns_os_mutex_del( sns_acm_log_mutex, 0, &err );

    SNS_PRINTF_STRING_FATAL_0( SNS_DBG_MOD_ACM,
                               "init: can't register with SMR" );

    return SNS_ERR_FAILED;
  }

  error_code = sns_os_task_create( sns_acm_rx_thread,
                                   NULL,
                                   NULL,
                                   SNS_MODULE_PRI_ACM );
  if( 0 != error_code ) {

    SNS_PRINTF_STRING_FATAL_0( SNS_DBG_MOD_ACM,
                               "init: can't create thread" );
    sns_em_delete_timer_obj( sns_acm_writable_tmr_ptr );
    sns_os_mutex_del( sns_acm_db_mutex, 0, &err );
    sns_os_mutex_del( sns_acm_log_mutex, 0, &err );
    // TODO: it is not possible to deregister from SMR

    return SNS_ERR_FAILED;
  }

  sns_acm_initialized = true;
  return SNS_SUCCESS;
}

/*===========================================================================

  FUNCTION:   sns_acm_deinit

  ===========================================================================*/
/*!
  @brief Remove the ACM functionality.

  Ensure all client resources are freed.
  Delete EM timer.
  Terminate ACM RX thread.

  @return
  None.
*/
/*=========================================================================*/
sns_err_code_e
sns_acm_deinit( void )
{
  uint8_t        err     = 0;
  uint8_t        index   = 0;
  sns_err_code_e sns_err = SNS_SUCCESS;

  // Close all clients
  for(index=0; index<SNS_ACM_MAX_CLIENTS; index++) {
    if(sns_acm_verify_client_handle(&sns_acm_db[index]))
    {
      SNS_PRINTF_STRING_ERROR_0( SNS_DBG_MOD_ACM,
                                 "sns_acm_deinit: Closing existing client" );

      if(SENSOR1_SUCCESS != sensor1_close(&sns_acm_db[index]))
      {
        SNS_PRINTF_STRING_ERROR_0( SNS_DBG_MOD_ACM,
                                   "sns_acm_deinit: failed to close client handle" );
      }
    }
  }

  sns_acm_initialized = false;

  // signal thread to exit
  if( NULL != sns_acm_flag_grp ) {
    sns_os_sigs_post( sns_acm_flag_grp,
                      SNS_ACM_WRITABLE_FLAG,
                      OS_FLAG_SET, &err );

    sns_os_task_del_req(SNS_MODULE_PRI_ACM);

    sns_os_sigs_del(sns_acm_flag_grp, 0, &err);
    sns_acm_flag_grp = (OS_FLAG_GRP*)NULL;
  }

  // Deinit MR
  sns_err = sns_acm_mr_deinit();
  if(SNS_SUCCESS != sns_err)
  {
    SNS_PRINTF_STRING_ERROR_1( SNS_DBG_MOD_ACM, "sns_acm_mr_deinit: failed %d", sns_err );
  }

  if(NULL != sns_acm_writable_tmr_ptr)
  {
    sns_em_delete_timer_obj( sns_acm_writable_tmr_ptr );
    sns_acm_writable_tmr_ptr = (sns_em_timer_obj_t)NULL;
  }

  if(NULL != sns_acm_db_mutex)
  {
    sns_os_mutex_del( sns_acm_db_mutex, 0, &err );
    sns_acm_db_mutex = (OS_EVENT*)NULL;
  }

  if(NULL != sns_acm_log_mutex)
  {
    sns_os_mutex_del( sns_acm_log_mutex, 0, &err );
    sns_acm_log_mutex = (OS_EVENT*)NULL;
  }

  return SNS_SUCCESS;
}

/*============================================================================
  - - - - - - - - - - - - - - - - - -
  SENSOR1 API
  - - - - - - - - - - - - - - - - - -
  ============================================================================*/

/*===========================================================================

  FUNCTION:   sensor1_open
  - Documented in sensor1.h

  ===========================================================================*/
sensor1_error_e
sensor1_open( sensor1_handle_s **hndl,
              sensor1_notify_data_cb_t notify_cb,
              intptr_t cb_data )
{
  sensor1_error_e           rv; // Return value
  sensor1_handle_s          *c_ptr;
  sns_log_sensor1_request_s *log_ptr;

  c_ptr = sns_acm_new_client_handle();

  SNS_ACM_DEBUG1( LOW, "sensor1_open: allocating client %d",
                  sns_acm_get_client_index( c_ptr ) );

  if( NULL == c_ptr ) {
    hndl = NULL;
    SNS_PRINTF_STRING_ERROR_0( SNS_DBG_MOD_ACM,
                               "sensor1_open: can't allocate client handle" );
    rv = SENSOR1_ENOMEM;
  } else {
    c_ptr->notify_cb   = notify_cb;
    c_ptr->notify_data = cb_data;

    *hndl = c_ptr;
    rv = SENSOR1_SUCCESS;
  }

  /* Generate and send log message */
  sns_logpkt_malloc( SNS_LOG_SENSOR1_REQUEST,
                     sizeof(sns_log_sensor1_request_s),
                     (void**)(&log_ptr) );
  if( NULL != log_ptr )
  {
    sns_acm_log_api_call( SNS_LOG_SENSOR1_API_OPEN,
                          (int32_t)rv,
                          c_ptr,
                          NULL, log_ptr );
  }
  return rv;
}

/*===========================================================================

  FUNCTION:   sensor1_close
  - Documented in sensor1.h

  ===========================================================================*/
sensor1_error_e sensor1_close( sensor1_handle_s *hndl_ptr )
{
  sensor1_error_e           rv;
  uint8_t                   err;
  sns_log_sensor1_request_s *log_ptr;
  int                       i;

  SNS_ACM_DEBUG1( LOW, "sensor1_close: freeing client %d",
                  sns_acm_get_client_index( hndl_ptr ) );

  if( false == sns_acm_verify_client_handle( hndl_ptr )) {
    SNS_PRINTF_STRING_ERROR_0( SNS_DBG_MOD_ACM,
                               "sensor1_close: invalid client handle" );

    rv = SENSOR1_EINVALID_CLIENT;
  } else {
    rv = SENSOR1_SUCCESS;
  }

  /* Generate and send log message */
  sns_logpkt_malloc( SNS_LOG_SENSOR1_REQUEST,
                     sizeof(sns_log_sensor1_request_s),
                     (void**)(&log_ptr) );
  if( NULL != log_ptr )
  {
    sns_acm_log_api_call( SNS_LOG_SENSOR1_API_CLOSE,
                          (int32_t)rv,
                          hndl_ptr,
                          NULL, log_ptr );
  }

  // TODO: Re-enable
  if( SENSOR1_SUCCESS == rv ) {
    uint8_t clnt_idx = (uint8_t)sns_acm_get_client_index( hndl_ptr );

    sns_os_mutex_pend( sns_acm_db_mutex, 0, &err );
    if( 0 == err ) {
      for( i = 0; i <= hndl_ptr->max_svc_used; i++ ) {
        if( ISBITSET( hndl_ptr->svcs_used, i ) ) {
          sns_acm_mr_cancel( clnt_idx, i );
        }
      }
      sns_acm_free_client_handle( hndl_ptr );
      sns_os_mutex_post( sns_acm_db_mutex );
    } else {
      SNS_PRINTF_STRING_ERROR_1( SNS_DBG_MOD_ACM,
                                 "Error %d getting mutex", (int32_t)err );
      rv = SENSOR1_EUNKNOWN;
    }
  }
  return rv;
}

/*===========================================================================

  FUNCTION:   sensor1_write
  - Documented in sensor1.h

  ===========================================================================*/
sensor1_error_e sensor1_write( sensor1_handle_s *hndl_ptr,
                               sensor1_msg_header_s *msg_hdr,
                               void* msg_ptr )
{
  sensor1_error_e  rv = SENSOR1_SUCCESS;
  sns_err_code_e   smr_err = SNS_SUCCESS;
  sns_smr_header_s smr_header;
  int32_t          client_index = -1;
  uint8_t          err;
  sns_log_sensor1_request_s *log_ptr = NULL;
  sns_acm_rate_s   peek_info;
  sns_acm_rate_state_e rate_state = SNS_ACM_RATE_OTHER;
  uint32_t         request_size = 0;

  SNS_ACM_DEBUG1( LOW, "sensor1_write: client %d",
                  sns_acm_get_client_index( hndl_ptr ) );

  sns_os_mutex_pend( sns_acm_log_mutex, 0, &err );
  if( 0 == err )
  {
    if( false == sns_acm_verify_client_handle( hndl_ptr ) ) {
      SNS_PRINTF_STRING_ERROR_0( SNS_DBG_MOD_ACM,
                                 "sensor1_write: invalid client handle" );

      rv = SENSOR1_EINVALID_CLIENT;
    } else {
      client_index = sns_acm_get_client_index(hndl_ptr);

      if( -1 == client_index ) {
        SNS_PRINTF_STRING_ERROR_0( SNS_DBG_MOD_ACM,
                                   "sensor1_write: invalid client handle" );
        rv = SENSOR1_EINVALID_CLIENT;
      } else if( NULL == msg_ptr ) {
        SNS_PRINTF_STRING_ERROR_0( SNS_DBG_MOD_ACM,
                                   "sensor1_write: invalid message pointer" );
        rv = SENSOR1_EBAD_PTR;
      } else {
        qmi_idl_service_object_type svc_obj;

        // TODO: Verify this msg_ptr was allocated with the sns msg allocator
        smr_header.dst_module  = 0;
        smr_header.src_module  = SNS_MODULE_ACM;
        smr_header.priority    = SNS_SMR_MSG_PRI_LOW;
        smr_header.txn_id      = msg_hdr->txn_id;
        smr_header.ext_clnt_id = (uint8_t)client_index;
        smr_header.msg_type    = SNS_SMR_MSG_TYPE_REQ;
        smr_header.svc_num     = (uint8_t)(msg_hdr->service_number);
        smr_header.msg_id      = (uint16_t)(msg_hdr->msg_id);
        smr_header.body_len    = msg_hdr->msg_size;

        sns_smr_set_hdr( &smr_header, msg_ptr );

        /* Peek into this request and populate rate_ptr structure required for PM
         * QoS and wakelock purposes */
        rate_state = sns_acm_peek_request( msg_hdr, msg_ptr, &peek_info );

        /* Generate log message to be sent after message is sent */
        smr_err = sns_logpkt_malloc( SNS_LOG_SENSOR1_REQUEST,
                           SNS_LOG_MAX_SIZE + sizeof(sns_log_sensor1_request_s),
                           (void**)(&log_ptr) );

        if( SNS_SUCCESS == smr_err && NULL != log_ptr
            && NULL != msg_ptr && 0 != msg_hdr->msg_size )
        {
          /* Generate qmi_encoded message for log */
          svc_obj = sns_smr_get_svc_obj( (uint8_t)(msg_hdr->service_number) );

          qmi_idl_message_encode( svc_obj, QMI_IDL_REQUEST,
                                  (uint16_t)msg_hdr->msg_id, msg_ptr,
                                  msg_hdr->msg_size, (void*)log_ptr->request,
                                  SNS_LOG_MAX_SIZE, &request_size );
          log_ptr->request_size = (uint16_t)request_size;
        }

        smr_err = sns_acm_mr_send( msg_ptr );
      }
    }

    sns_acm_log_api_call( SNS_LOG_SENSOR1_API_WRITE, (int32_t)rv, hndl_ptr,
                          msg_hdr, log_ptr );
    sns_os_mutex_post( sns_acm_log_mutex );
  }
  else
  {
    SNS_PRINTF_STRING_ERROR_1( SNS_DBG_MOD_ACM,
                               "Error %d getting log mutex",
                               (int32_t)err );
    rv = SENSOR1_EUNKNOWN;
  }

  if( SNS_SUCCESS == smr_err && SENSOR1_SUCCESS == rv ) {

    /* successfully sent a message to a service. Record that this service
     * has been contacted by this client */
    sns_os_mutex_pend( sns_acm_db_mutex, 0, &err );

    if( 0 == err ) {
      sns_acm_apply_rate_node( client_index, &peek_info, rate_state );

      hndl_ptr->outstanding_reqs++;
      if( SNS_ACM_MAX_SVC_ID > msg_hdr->service_number ) {
        SETBIT( hndl_ptr->svcs_used, msg_hdr->service_number );
        hndl_ptr->max_svc_used = MAX( (int)(hndl_ptr->max_svc_used),
                                      (int)(msg_hdr->service_number) );
      }
      sns_os_mutex_post( sns_acm_db_mutex );
    } else {
      SNS_PRINTF_STRING_ERROR_1( SNS_DBG_MOD_ACM,
                                 "Error %d getting mutex", (int32_t)err );
    }
  }

  return ( SENSOR1_SUCCESS == rv ) ? smr_err : rv;
}

/*===========================================================================

  FUNCTION:   sensor1_writable
  - Documented in sensor1.h

  ===========================================================================*/
sensor1_error_e sensor1_writable( sensor1_handle_s *hndl_ptr,
                                  sensor1_write_cb_t cbf,
                                  intptr_t cb_data,
                                  uint32_t service_id )
{
  sns_acm_write_cb_info_s *w_cb_ptr;

  SNS_ACM_DEBUG2( LOW, "sensor1_writable: client %d service %d",
                  sns_acm_get_client_index( hndl_ptr ), service_id );

  if( false == sns_acm_verify_client_handle( hndl_ptr ) ) {
    SNS_PRINTF_STRING_ERROR_0( SNS_DBG_MOD_ACM,
                               "sensor1_writable: invalid client handle" );

    return SENSOR1_EINVALID_CLIENT;
  }

  w_cb_ptr = (sns_acm_write_cb_info_s*)
    SNS_OS_MALLOC(SNS_DBG_MOD_APPS_ACM,sizeof(sns_acm_write_cb_info_s));

  if( NULL == w_cb_ptr ) {
    SNS_PRINTF_STRING_ERROR_0( SNS_DBG_MOD_ACM,
                               "sensor1_writable: can't allocate write cb struct" );

    return SENSOR1_ENOMEM;
  }

  w_cb_ptr->write_cb   = cbf;
  w_cb_ptr->cb_data    = cb_data;
  w_cb_ptr->service_id = service_id;
  w_cb_ptr->next = NULL;

  sns_acm_add_client_write_cb_info( hndl_ptr, w_cb_ptr );

  sns_em_register_timer( sns_acm_writable_tmr_ptr,
                         sns_em_convert_usec_to_localtick(SNS_ACM_WRITABLE_POLL_USEC) );

  return SENSOR1_SUCCESS;
}

/*===========================================================================

  FUNCTION:   sensor1_alloc_msg_buf
  - Documented in sensor1.h

  ===========================================================================*/
sensor1_error_e sensor1_alloc_msg_buf(sensor1_handle_s *hndl_ptr,
                                      uint16_t size,
                                      void** buffer )
{
  UNREFERENCED_PARAMETER(hndl_ptr);

  SNS_ACM_DEBUG2( LOW, "sensor1_alloc: client %d, size %d",
                  sns_acm_get_client_index( hndl_ptr ), size );

  if( buffer == NULL ) {
    SNS_PRINTF_STRING_ERROR_0( SNS_DBG_MOD_ACM,
                               "sensor1_alloc_msg_buf: NULL buffer" );
    return SENSOR1_EBAD_PTR;
  }
  // TODO: track client memory usage

  *buffer = sns_smr_msg_alloc( SNS_DBG_MOD_APPS_ACM, size );
  if( NULL == *buffer ) {
    SNS_PRINTF_STRING_ERROR_1( SNS_DBG_MOD_ACM,
                               "sensor1_alloc_msg_buf: can't allocate %d bytes",
                               size );

    return SENSOR1_ENOMEM;
  }
  return SENSOR1_SUCCESS;
}

/*===========================================================================

  FUNCTION:   sensor1_free_msg_buf
  - Documented in sensor1.h

  ===========================================================================*/
sensor1_error_e sensor1_free_msg_buf(sensor1_handle_s *hndl_ptr,
                                     void* msg_buf )
{
  UNREFERENCED_PARAMETER(hndl_ptr);

  SNS_ACM_DEBUG1( LOW, "sensor1_free_msg_buf: client %d",
                  sns_acm_get_client_index( hndl_ptr ) );

  if( NULL == msg_buf ) {
    SNS_PRINTF_STRING_ERROR_0( SNS_DBG_MOD_ACM,
                               "sensor1_free_msg_buf: NULL ptr" );
    return SENSOR1_EBAD_PTR;
  }
  // TODO: track client memory usage
  sns_smr_msg_free( msg_buf );
  return SENSOR1_SUCCESS;
}

/*===========================================================================

  FUNCTION:   sensor1_init
  - Documented in sensor1.h

  ===========================================================================*/
sensor1_error_e sensor1_init( void )
{
#if defined( SNS_BLAST )
  return SENSOR1_SUCCESS;
#else
  return (sensor1_error_e) sns_init();
#endif
}

/*===========================================================================

  FUNCTION:   sensor1_deinit
  - Documented in sensor1.h

  ===========================================================================*/
sensor1_error_e sensor1_deinit( void )
{
#if defined(_WIN32)
  return (sensor1_error_e)sns_deinit();
#else
  // TODO: Implement on Linux
  return SENSOR1_SUCCESS;
#endif
}
