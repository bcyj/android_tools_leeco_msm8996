/******************************************************************************
  @file    ds_profile_os_linux.c
  @brief   

  DESCRIPTION
  This file implements the modem (AMSS) specific routines

  INITIALIZATION AND SEQUENCING REQUIREMENTS
  N/A

  ---------------------------------------------------------------------------
  Copyright (C) 2010 Qualcomm Technologies, Inc.
  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
******************************************************************************/
/*===========================================================================

                        EDIT HISTORY FOR MODULE

  This section contains comments describing changes made to the module.
  Notice that changes are listed in reverse chronological order.

  $Header: //source/qcom/qct/modem/data/1x/707/main/latest/src/ds707_data_session_profile.c#32 $ $DateTime: 2009/09/11 10:21:08 $ $Author: lipings $

when       who     what, where, why
--------   ---     ----------------------------------------------------------
09/30/09   mg      Created the module. First version of the file.
===========================================================================*/

#include <stdarg.h>
#include <assert.h>
#include "memory.h"
#include "err.h"
#include "ds_profile_os_linux.h"
#include "ds_profile_tech_common.h"


/*---------------------------------------------------------------------------
 					 PUBLIC FUNCTION IMPLEMENTATIONS
   Logging and locking functions for DS profile library
---------------------------------------------------------------------------*/

int ds_profile_log_init( 
  char *lib_instance
)
{
  boolean ret_val = FALSE;
  ret_val = Diag_LSM_Init(NULL);

  (void)lib_instance;
  return DSI_SUCCESS;
}

#ifdef TEST_FRAMEWORK
int ds_profile_lock_init( 
  plm_lock_type *lock 
)
{
  qmutex_attr_t attr;

  qmutex_attr_init( &attr );
  if(qmutex_create(lock, &attr) != 0)
    return DSI_FAILURE;
  else
    return DSI_SUCCESS;
}

int ds_profile_lock_acq( plm_lock_type lock )
{
  if(qmutex_lock( lock ) != 0)
    return DSI_FAILURE;
  else
    return DSI_SUCCESS;
}

int ds_profile_lock_rel( plm_lock_type lock )
{
  if(qmutex_unlock( lock ) != 0)
    return DSI_FAILURE;
  else
    return DSI_SUCCESS;
}

int plm_lock_try( plm_lock_type lock )
{
  if(qmutex_lock( lock ) != 0)
    return DSI_FAILURE;
  else
    return DSI_SUCCESS;
}

#else /* TEST_FRAMEWORK */
int ds_profile_lock_init( 
  plm_lock_type *lock 
)
{
  int ret_val = DSI_FAILURE ;

  if ((ret_val = pthread_mutex_init(lock,NULL)) == 0)
  {
    return DSI_SUCCESS;
  }

  return ret_val;
}

int ds_profile_lock_acq( plm_lock_type *lock )
{
  int ret_val = DSI_FAILURE ;

  if ((ret_val = pthread_mutex_lock(lock)) == 0)
  {
    return DSI_SUCCESS;
  }

  return ret_val;
}

int ds_profile_lock_rel( plm_lock_type *lock )
{
  int ret_val = DSI_FAILURE ;

  if ((ret_val = pthread_mutex_unlock(lock)) == 0)
  {
    return DSI_SUCCESS;
  }

  return ret_val;
}

int plm_lock_try( plm_lock_type *lock )
{
  int ret_val = DSI_FAILURE ;

  if ((ret_val = pthread_mutex_trylock(lock)) == 0)
  {
    return DSI_SUCCESS;
  }

  return ret_val;
}

/*=========================================================================
  FUNCTION:  ds_profile_format_log_msg

===========================================================================*/
/*!
    @brief
    Format debug message for logging.

    @return
    None
*/
/*=========================================================================*/
void ds_profile_format_log_msg
(
  char *buf_ptr,
  int buf_size,
  char *fmt,
  ...
)
{
  va_list ap;

  /*-----------------------------------------------------------------------*/

  assert( buf_ptr != NULL );
  assert( buf_size > 0 );

  /*-----------------------------------------------------------------------*/

  va_start( ap, fmt );

  vsnprintf( buf_ptr, buf_size, fmt, ap );

  va_end( ap );

} /* ds_profile_format_log_msg */

#endif /* TEST_FRAMEWORK */


