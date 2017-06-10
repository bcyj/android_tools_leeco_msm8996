/******************************************************************************
  @file    qcril_data.h
  @brief

  DESCRIPTION


  INITIALIZATION AND SEQUENCING REQUIREMENTS
  N/A

  ---------------------------------------------------------------------------
  Copyright (C) 2009 - 2010, 2013, 2014 Qualcomm Technologies, Inc.
  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
******************************************************************************/

/*===========================================================================

                        EDIT HISTORY FOR MODULE

This section contains comments describing changes made to the module.
Notice that changes are listed in reverse chronological order.

$Header: $

when       who     what, where, why
--------   ---     ----------------------------------------------------------
05/18/09   fc      Changes to log debug messages to Diag directly instead
                   of through logcat.
04/05/09   fc      Cleanup log macros.
01/26/08   fc      Logged assertion info.
12/23/08   asn     Added handling on MO call-end and IP addr issue
12/15/08   asn     Additional macros
08/08/08   asn     Initial version

===========================================================================*/
#ifndef QCRIL_DATA_H
#define QCRIL_DATA_H

/*===========================================================================

                           INCLUDE FILES

===========================================================================*/

#include "assert.h"
#include "ril.h"
#include "qcril_log.h"
#include <pthread.h> /* mutex */


/*===========================================================================

                        DEFINITIONS AND TYPES

===========================================================================*/
#define QCRIL_CDMA_STRING "0"
#define QCRIL_UMTS_STRING "1"

/* The ril_tech string provided by Android Telephony is RIL_RadioTechnology+2 */
#define QCRIL_DATA_CONVERT_RIL_RADIOTECH(x)  (x+2)

#define QCRIL_DATA_NUM_RIL_RADIO_TECHS  (QCRIL_DATA_RIL_RADIO_TECH_LAST-2)

/* More specific radio technology values provided by Android Telephony */
typedef enum
{
  QCRIL_DATA_RIL_RADIO_TECH_UNKNOWN = QCRIL_DATA_CONVERT_RIL_RADIOTECH(RADIO_TECH_UNKNOWN),
  QCRIL_DATA_RIL_RADIO_TECH_GPRS    = QCRIL_DATA_CONVERT_RIL_RADIOTECH(RADIO_TECH_GPRS),
  QCRIL_DATA_RIL_RADIO_TECH_EDGE    = QCRIL_DATA_CONVERT_RIL_RADIOTECH(RADIO_TECH_EDGE),
  QCRIL_DATA_RIL_RADIO_TECH_UMTS    = QCRIL_DATA_CONVERT_RIL_RADIOTECH(RADIO_TECH_UMTS),
  QCRIL_DATA_RIL_RADIO_TECH_IS95A   = QCRIL_DATA_CONVERT_RIL_RADIOTECH(RADIO_TECH_IS95A),
  QCRIL_DATA_RIL_RADIO_TECH_IS95B   = QCRIL_DATA_CONVERT_RIL_RADIOTECH(RADIO_TECH_IS95B),
  QCRIL_DATA_RIL_RADIO_TECH_1xRTT   = QCRIL_DATA_CONVERT_RIL_RADIOTECH(RADIO_TECH_1xRTT),
  QCRIL_DATA_RIL_RADIO_TECH_EVDO_0  = QCRIL_DATA_CONVERT_RIL_RADIOTECH(RADIO_TECH_EVDO_0),
  QCRIL_DATA_RIL_RADIO_TECH_EVDO_A  = QCRIL_DATA_CONVERT_RIL_RADIOTECH(RADIO_TECH_EVDO_A),
  QCRIL_DATA_RIL_RADIO_TECH_HSDPA   = QCRIL_DATA_CONVERT_RIL_RADIOTECH(RADIO_TECH_HSDPA),
  QCRIL_DATA_RIL_RADIO_TECH_HSUPA   = QCRIL_DATA_CONVERT_RIL_RADIOTECH(RADIO_TECH_HSUPA),
  QCRIL_DATA_RIL_RADIO_TECH_HSPA    = QCRIL_DATA_CONVERT_RIL_RADIOTECH(RADIO_TECH_HSPA),
  QCRIL_DATA_RIL_RADIO_TECH_EVDO_B  = QCRIL_DATA_CONVERT_RIL_RADIOTECH(RADIO_TECH_EVDO_B),
  QCRIL_DATA_RIL_RADIO_TECH_EHRPD   = QCRIL_DATA_CONVERT_RIL_RADIOTECH(RADIO_TECH_EHRPD),
  QCRIL_DATA_RIL_RADIO_TECH_LTE     = QCRIL_DATA_CONVERT_RIL_RADIOTECH(RADIO_TECH_LTE),
  QCRIL_DATA_RIL_RADIO_TECH_HSPAP   = QCRIL_DATA_CONVERT_RIL_RADIOTECH(RADIO_TECH_HSPAP),
  QCRIL_DATA_RIL_RADIO_TECH_GSM     = QCRIL_DATA_CONVERT_RIL_RADIOTECH(RADIO_TECH_GSM),
  QCRIL_DATA_RIL_RADIO_TECH_TDSCDMA = QCRIL_DATA_CONVERT_RIL_RADIOTECH(RADIO_TECH_TD_SCDMA),

  /* This should be the last entry */
  QCRIL_DATA_RIL_RADIO_TECH_LAST

} qcril_data_ril_radio_tech_t;

#define QCRIL_PROFILE_IDX_MAX_LEN 4

#ifndef PDP_FAIL_PARTIAL_RETRY_FAIL
#define PDP_FAIL_PARTIAL_RETRY_FAIL -7
#endif

/*===========================================================================

                    MACROs

===========================================================================*/

#define QCRIL_DS_LOG_DBG_MEM( msg, val )  \
  QCRIL_LOG_DEBUG( ">>>Mem debug>>> %s [%p]", msg, (unsigned int *)val );

#define QCRIL_DS_LOG_DBG_DSI_HNDL( dsi_hndl ) \
  QCRIL_LOG_DEBUG( "dsi_hndl is [%p]", (unsigned int *) dsi_hndl );

/*---------------------------------------------------------------------------
   Utility
---------------------------------------------------------------------------*/
#define MINIMUM(a,b)    ((a) < (b) ? (a) : (b))
#define MAXIMUM(a,b)    ((a) > (b) ? (a) : (b))
/*---------------------------------------------------------------------------
   Helper macro for logging function entry and exit
---------------------------------------------------------------------------*/
#define QCRIL_DS_FUNC_ENTRY(msg)    \
  QCRIL_LOG_VERBOSE( "%s:%s Entering func:%s! [%s]", __func__, msg )

#define QCRIL_DS_FUNC_EXIT(msg)    \
  QCRIL_LOG_VERBOSE( "%s:%s Entering func:%s! [%s]", __func__, msg )

/* Soft ASSERT */
#define QCRIL_DS_ASSERT( expr, msg )                                                   \
    if ( !( expr ) ) {                                                                 \
      QCRIL_LOG_ERROR( "%s", "*****ASSERTION FAILED (soft)*****" ); \
      QCRIL_LOG_ERROR( "Cond %s [%s]", #expr, msg );                                \
      QCRIL_LOG_ERROR( "%s", "*********************************" ); \
    }

/* Hard ASSERT */
#define QCRIL_DS_ASSERT_H( expr, msg )                                                 \
    if ( !( expr ) ) {                                                                 \
      QCRIL_LOG_ERROR( "%s", "*****ASSERTION FAILED (hard)*****" ); \
      QCRIL_LOG_ERROR( "Cond %s [%s]", #expr, msg );                                \
      QCRIL_LOG_ERROR( "%s", "*********************************" ); \
      assert(0);                                                                       \
    }

/*---------------------------------------------------------------------------
   Helper macro for locking and unlocking mutex with debug msgs
---------------------------------------------------------------------------*/

#define QCRIL_DATA_MUTEX_LOCK( m ) \
    do { \
      int lock_err = 0; \
      QCRIL_LOG_INFO(">>>>>> LOCK QCRIL_DATA MUTEX %x", m); \
      lock_err = pthread_mutex_lock(m); \
      QCRIL_LOG_INFO("LOCK QCRIL_DATA MUTEX %x result=%d", m, lock_err); \
      QCRIL_DS_ASSERT_H( lock_err == 0, "pthread_mutex_lock failed" ); \
    } while (0)

#define QCRIL_DATA_MUTEX_UNLOCK( m ) \
    do { \
      int unlock_err = 0; \
      QCRIL_LOG_INFO("<<<<<< UNLOCK QCRIL_DATA MUTEX %x", m); \
      unlock_err= pthread_mutex_unlock(m); \
      QCRIL_LOG_INFO("UNLOCK QCRIL_DATA MUTEX %x result=%d", m, unlock_err); \
      QCRIL_DS_ASSERT_H( unlock_err == 0, "pthread_muxtex_unlock failed" ); \
    } while (0)

#define QCRIL_DATA_MUTEX_TRYLOCK( m, result ) \
    do { \
      QCRIL_LOG_INFO(">>>>>> TRY LOCK QCRIL_DATA MUTEX %x", m); \
      result = pthread_mutex_trylock(m); \
      QCRIL_LOG_INFO("TRYLOCK QCRIL_DATA MUTEX %x result=%d", m, result); \
    } while (0)

/* Error codes */
#define QCRIL_DS_SUCCESS 0
#define QCRIL_DS_ERROR  -1

/*Switch to toggle Dormancy indications*/
typedef enum
{
  DORMANCY_INDICATIONS_MIN = 0x01,
  DORMANCY_INDICATIONS_OFF = DORMANCY_INDICATIONS_MIN,
  DORMANCY_INDICATIONS_ON  = 0x02,
  DORMANCY_INDICATIONS_MAX = DORMANCY_INDICATIONS_ON
}qcril_data_dormancy_ind_switch_type;

/* Switch to toggle data limited system status indications */
typedef enum
{
  LIMITED_SYS_INDICATIONS_MIN = 0x01,
  LIMITED_SYS_INDICATIONS_OFF = LIMITED_SYS_INDICATIONS_MIN,
  LIMITED_SYS_INDICATIONS_ON  = 0x02,
  LIMITED_SYS_INDICATIONS_MAX = LIMITED_SYS_INDICATIONS_ON
}qcril_data_limited_sys_ind_switch_type;
/*=========================================================================*/


/*===========================================================================

                    EXTERNAL FUNCTION PROTOTYPES

===========================================================================*/
/*===========================================================================

  FUNCTION:  qcril_data_process_screen_state_change

===========================================================================*/
/*!
    @brief
    Update qcril_data about the current screen status. QCRIL data can perform
    further optimization processing as part of this indication.

    @args[in] Screen state:
              0 - screen OFF
              1 - screen ON

    @return QCRIL_DS_SUCCESS on success
    @return QCRIL_DS_ERROR on failure.
*/
/*=========================================================================*/
int
qcril_data_process_screen_state_change
(
  boolean screen_state
);

/*===========================================================================

  FUNCTION:  qcril_data_toggle_dormancy_indications

===========================================================================*/
/*!
    @brief

    Handles request to turn ON/OFF dormancy indications. Typically called to
    turn off indications when in power save mode  and turn back on when out
    of power save mode

    @return QCRIL_DS_SUCCESS on success and QCRIL_DS_ERROR on failure.
*/
/*=========================================================================*/
int
qcril_data_toggle_dormancy_indications
(
  qcril_data_dormancy_ind_switch_type       dorm_ind_switch
);

/*===========================================================================

  FUNCTION:  qcril_data_toggle_limited_sys_indications

===========================================================================*/
/*!
    @brief

    Handles request to turn ON/OFF limited data system status change
    indications. Typically called to TURN-ON limited indications when
    in screen-off state. In screen-on state, this is TURNED-OFF so
    full data system status indications can be received.

    @return QCRIL_DS_SUCCESS on success and QCRIL_DS_ERROR on failure.
*/
/*=========================================================================*/
int
qcril_data_toggle_limited_sys_indications
(
  qcril_data_limited_sys_ind_switch_type       limited_sys_ind_switch
);

#ifdef RIL_REQUEST_SET_INITIAL_ATTACH_APN
/*===========================================================================

  FUNCTION: qcril_data_request_set_lte_attach_profile

===========================================================================*/
/*!
  @brief
  Handles RIL_REQUEST_SET_INITIAL_ATTACH_APN.

  @return
  None.

*/
/*===========================================================================*/
RIL_Errno qcril_data_request_set_lte_attach_profile
(
  RIL_InitialAttachApn* attachInfo
);
#endif /* RIL_REQUEST_SET_INITIAL_ATTACH_APN */

/*===========================================================================

  FUNCTION:  qcril_data_update_mtu

===========================================================================*/
/*!
    @brief
    Changes MTU value on all active calls
    @return
    NONE
*/
  /*===========================================================================*/
void qcril_data_update_mtu
(
  unsigned int mtu
);
/*===========================================================================

  FUNCTION:  qcril_data_set_is_data_enabled

===========================================================================*/
/*!
    @brief
    Sets the data enabled to true or false via DSD indication passed by
    upper layers
    @return
    SUCCESS :- If QMI DSD returns rc = 0
    FAILURE :- If QMI DSD returns rc = negative value
*/
  /*===========================================================================*/

RIL_Errno qcril_data_set_is_data_enabled
(
  boolean is_data_enabled
);
/*===========================================================================

  FUNCTION:  qcril_data_set_is_data_roaming_enabled

===========================================================================*/
/*!
    @brief
    Sets the data roaming enabled to true or false via DSD indication passed
    by upper layers
    @return
    SUCCESS:- If QMI DSD returns rc = 0
    FAILURE:- If QMI DSD returns rc = negative value
*/
  /*===========================================================================*/

RIL_Errno qcril_data_set_is_data_roaming_enabled
(
  boolean is_data_roaming_enabled
);
/*===========================================================================

  FUNCTION:  qcril_data_set_apn_info

===========================================================================*/
/*!
    @brief
    Sets the APN type and APN name via DSD indication passed by upper layers
    @return
    SUCCESS:- If QMI DSD returns rc = 0
    FAILURE:- If QMI DSD returns rc = negative value
*/
  /*===========================================================================*/

RIL_Errno qcril_data_set_apn_info
(
  char *type,
  char *name,
  int32 is_apn_valid
);

#endif /* QCRIL_DATA_H */
