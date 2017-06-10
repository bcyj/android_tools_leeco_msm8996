/*===========================================================================
                           usf_epos_feedback_handlers.h

DESCRIPTION: This header file exposes function for handling feedback requests
from the EPOS library

Copyright (c) 2012-2013 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
=============================================================================*/

#ifndef _USF_EPOS_FEEDBACK_HANDLERS_
#define _USF_EPOS_FEEDBACK_HANDLERS_

/*----------------------------------------------------------------------------
  Include files
----------------------------------------------------------------------------*/
#include "eposexports.h"

/*-----------------------------------------------------------------------------
  Consts and macros
-----------------------------------------------------------------------------*/

// ERROR return values
#define BAD_ARGS  1
#define RX_OPEN_FAILED  2
#define RX_CLOSE_FAILED 3
#define CLIENT_NAME "digitalpen"

/*------------------------------------------------------------------------------
  Function declarations
------------------------------------------------------------------------------*/

/*==============================================================================
  FUNCTION:  handle_feedback
==============================================================================*/
/**
 * Handles the feedback got from EPOS lib, sends a command back to EPOS lib
 * when needed.
 */
void usf_epos_handle_feedback(void* InWorkspace,
                              FeedbackInfo *OutFeedback);


/*==============================================================================
  FUNCTION:  active_leave_notify
==============================================================================*/
/**
 * Notifies the feedbacks that we have left active state
 */
void usf_epos_notify_active_state_exit();

/*==============================================================================
  FUNCTION:  usf_epos_notify_RX_stop
==============================================================================*/
/**
  Notifies the Epos library about RX stop
 */
void usf_epos_notify_RX_stop(void *InWorkspace);

#endif // _USF_EPOS_FEEDBACK_HANDLERS_
