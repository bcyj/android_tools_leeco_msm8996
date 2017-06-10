/*===========================================================================
                           usf_epos_feedback_handlers.cpp

DESCRIPTION: This file contains all the implementations of the feedback handlers
for the feedbacks EPOS library sends

Copyright (c) 2012-2013 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
=============================================================================*/

/*----------------------------------------------------------------------------
  Include files
----------------------------------------------------------------------------*/
#include "usf_log.h"
#include "usf_epos_dynamic_lib_proxy.h"
#include <usf_epos_feedback_handlers.h>
#include <ual_util.h>
#include <ual.h>

/*-----------------------------------------------------------------------------
  Consts and macros
-----------------------------------------------------------------------------*/
#define ENABLE 1
#define DISABLE 0

// Path for the dir which holds the pattern
#define PATTERN_DIR_PATH "/data/usf/epos/pattern/"

// Feedback number index in CommandOut parameter
#define FEEDBACK_INDEX 0

/*-----------------------------------------------------------------------------
  Typedefs
-----------------------------------------------------------------------------*/

/**
  Forward declerations
 */
static int usf_epos_spd_snd_req_handler(int args[],
                                        struct usf_epos_command *ret_cmd);

/**
  The feedback numbers we might get from EPOS lib
 */
enum usf_epos_feedback_number {
  NO_REQ    = 0,
  // Speed of sound request
  SPD_SND_REQ   = 30,
};

/**
  The feedback handler function pointer
 */
typedef int (*usf_epos_fb_hdlr_func)(int args[],
                                     struct usf_epos_command *ret_cmd);

/**
  The feedback handler container
 */
struct usf_epos_feedback_handler {
  // Feedback number
  enum usf_epos_feedback_number fb_num;
  // feedback handler function
  usf_epos_fb_hdlr_func func;
};

/*-----------------------------------------------------------------------------
  Externs
-----------------------------------------------------------------------------*/
// For RX config use
extern EposParams eposParams;
extern work_params_type s_work_params;

// Power save function, returns true if we are in active state
extern bool usf_ps_is_active(void);

// For pen dynamic library
extern UsfEposDynamicLibProxy* epos_lib_proxy;

/*-----------------------------------------------------------------------------
  Static
-----------------------------------------------------------------------------*/
/**
  This array maps each feedback number to a handler function
 */
static struct usf_epos_feedback_handler fb_handlers[] =
{
  {SPD_SND_REQ, usf_epos_spd_snd_req_handler},
};

// This variable value indicates if the RX path is turned on/off
static bool rx_enabled = false;

/*------------------------------------------------------------------------------
  Function declarations
------------------------------------------------------------------------------*/


/*==============================================================================
  FUNCTION:  usf_epos_start_RX
==============================================================================*/
/**
  Starts RX transmittion
  TODO: this function is shared for all daemons, need to write it only once in
        the ual_util file.
 */
static int usf_epos_start_RX()
{
  int rc;
  // config RX path, and starts it (when started DSP will send trash)
  if (ual_util_rx_config(&s_work_params.paramsStruct,
                       (char* )CLIENT_NAME))
  {
    LOGE("%s: ual_util_rx_config failed.",
         __FUNCTION__);
    return -RX_OPEN_FAILED;
  }

  // Pattern is taken from file named in the cfg file
  if (0 != s_work_params.paramsStruct.usf_rx_pattern[0])
  {
    if (NULL == eposParams.m_pPattern) {
      // Pattern has not been read from file yet
      LOGI("%s: Update pattern from file.",
           __FUNCTION__);

      // Allocate pattern
      eposParams.m_pPattern = (uint8_t *) malloc(eposParams.m_patternSize);
      if (NULL == eposParams.m_pPattern)
      {
        LOGE("%s: malloc(%d) failed.",
             __FUNCTION__, eposParams.m_patternSize);
        return -RX_OPEN_FAILED;
      }

      // Reads the pattern from the file
      rc = ual_util_read_pattern(eposParams.m_pPattern,
                                   &s_work_params.paramsStruct,
                                   (char *)PATTERN_DIR_PATH);
      if (rc)
      {
        free(eposParams.m_pPattern);
        eposParams.m_pPattern = NULL;
        LOGE("%s: ual_util_read_pattern failed.",
             __FUNCTION__);
        return -RX_OPEN_FAILED;
      }
    }

    // Pattern is transmitted only once. DSP transmits pattern in loop.
    rc = ual_write(eposParams.m_pPattern,
                   eposParams.m_patternSize);
    if (1 != rc)
    {
      LOGE("%s: ual_write failed.",
           __FUNCTION__);
      return -RX_OPEN_FAILED;
    }
  }
  return 0;
}


/*==============================================================================
  FUNCTION:  usf_epos_spd_snd_req_handler
==============================================================================*/
/**
  Takes care of speed of sound feedback request,
  opens/closes Rx path as needed.
  args[0] expected values:
    0 - Disable speed of sound transmittion
    1 - Enable speed of sound transmittion
 */
static int usf_epos_spd_snd_req_handler(int args[],
                                        struct usf_epos_command *ret_cmd)
{
  // Command type to be sent back to EPOS lib
  ret_cmd->cmd_type = CMD_TYPE_SET;
  // Command number to be sent back to EPOS lib
  ret_cmd->cmd_num = NO_CMD;

  switch (args[0])
  {
  case DISABLE:
    if (rx_enabled)
    {
      // Deactivates Rx
      if (!ual_stop_RX())
      {
        LOGW ("%s: Stop RX failed, could not stop RX",
              __FUNCTION__);
        return -RX_CLOSE_FAILED;
      }
      // Updates cmd back
      ret_cmd->cmd_num = CMD_SPD_SND_TRANSMIT;
      ret_cmd->args[0] = DISABLE;

      rx_enabled = false;
    }
    else
    {
      LOGW ("%s: Stop RX failed, RX not enabled",
            __FUNCTION__);
    }
    return 0;

  case ENABLE:

    if (!rx_enabled)
    {
      if (!usf_ps_is_active())
      {
        LOGW("%s: Start RX failed, current state is not active",
             __FUNCTION__);
        return 0;
      }

      // Activates RX
      if (usf_epos_start_RX())
      {
        (void)ual_stop_RX();
        LOGW("%s: Start RX failed, problem with system",
             __FUNCTION__);
        return -RX_OPEN_FAILED;
      }

      // Updates cmd back
      ret_cmd->cmd_num = CMD_SPD_SND_TRANSMIT;
      ret_cmd->args[0] = ENABLE;

      rx_enabled = true;
    }
    else
    {
      LOGW("%s: Start RX failed, RX already enabled",
           __FUNCTION__);
    }
    return 0;

  default:
    return -BAD_ARGS;
  }
}

/*==============================================================================
  FUNCTION:  handle_feedback
==============================================================================*/
/**
  Handles the feedback got from EPOS lib, sends a command back to EPOS lib
  when needed.
 */
void usf_epos_handle_feedback(void* InWorkspace,
                              FeedbackInfo *OutFeedback)
{
  if (NULL == OutFeedback)
  {
    LOGE("%s: OutFeedback is NULL",
         __FUNCTION__);
    return;
  }
  if ((NO_REQ == OutFeedback->CommandOut[FEEDBACK_INDEX]) ||
      (SPD_SND_REQ == OutFeedback->CommandOut[FEEDBACK_INDEX])) // no SoS
    return;


  LOGD("%s: Handling feedback: %d arg0: %d",
       __FUNCTION__,
       OutFeedback->CommandOut[0],
       OutFeedback->CommandOut[1]);

  // fb_handlers is an array found in usf_epos_feedback_handlers.h
  for (int i = 0; i < (int)ARRAY_SIZE(fb_handlers); ++i)
    // Find the appropiate feedback handler from the array
    if (fb_handlers[i].fb_num == OutFeedback->CommandOut[FEEDBACK_INDEX])
    {
      struct usf_epos_command ret_cmd;
      // Activate the handler function
      // CommandOut + 1 is the start of the args array (cur. 3 args)
      // Note: fb_handlers is an array we maintain, as long as it's this way,
      // we don't need to check for NULLity
      if (fb_handlers[i].func(OutFeedback->CommandOut + 1,
                              &ret_cmd))
      {
        LOGW("%s: Could not handle %d feedback",
             __FUNCTION__,
             OutFeedback->CommandOut[FEEDBACK_INDEX]);
        return;
      }

      if (NO_CMD != ret_cmd.cmd_num)
      {
        LOGD("%s: Sending command back: type:%ld num:%ld arg0: %ld",
             __FUNCTION__,
             ret_cmd.cmd_type,
             ret_cmd.cmd_num,
             ret_cmd.args[0]);
        // Send command back to EPOS lib
        epos_lib_proxy->command(InWorkspace,
                                (int32_t*)&ret_cmd);
      }
      return;
    }

   LOGW("%s: No handler found for %d feedback",
        __FUNCTION__,
        OutFeedback->CommandOut[FEEDBACK_INDEX]);
}


/*==============================================================================
  FUNCTION:  active_leave_notify
==============================================================================*/
/**
  Notifies the feedbacks that we have left active state
 */
void usf_epos_notify_active_state_exit()
{
  if (rx_enabled) {
    ual_stop_RX();
    rx_enabled = false;
  }
}

/*==============================================================================
  FUNCTION:  usf_epos_notify_RX_stop
==============================================================================*/
/**
  Notifies the Epos library about RX stop
 */
void usf_epos_notify_RX_stop(void *InWorkspace)
{
  if (rx_enabled) {
    struct usf_epos_command cmd;

    cmd.cmd_type = CMD_TYPE_SET;
    cmd.cmd_num = CMD_SPD_SND_TRANSMIT;
    cmd.args[0] = DISABLE;
    // Send command to EPOS lib
    epos_lib_proxy->command(InWorkspace,
                            (int32_t*)&cmd);
    rx_enabled = false;

    LOGD("%s",
        __FUNCTION__);
  }
}
