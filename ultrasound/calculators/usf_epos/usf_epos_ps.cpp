/*===========================================================================
                           usf_epos_ps.c

DESCRIPTION: Implementation of the "power save"/"US detection" in EPOS daemon.

Copyright (c) 2011-2013 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
=============================================================================*/
#define LOG_TAG "usf_epos_ps"

/*----------------------------------------------------------------------------
  Include files
----------------------------------------------------------------------------*/
#include "usf_log.h"
#include <stdlib.h>
#include <unistd.h>
#include <ual.h>
#include <ual_util.h>
#include "usf_epos_dynamic_lib_proxy.h"
#include <usf_epos_feedback_handlers.h>

/*----------------------------------------------------------------------------
  Defines
----------------------------------------------------------------------------*/

extern bool usf_epos_get_points(bool& b_us);

extern bool is_usf_epos_running(void);

/*-----------------------------------------------------------------------------
  Typedefs
-----------------------------------------------------------------------------*/

/**
  Transition function from one "power save" state to other.
*/
typedef bool (*ps_transit_func)(work_params_type&);

/**
  Work function type.
*/
typedef bool (*usf_work_func) (work_params_type&, ps_transit_enum&);


/**
  Information for each "power save" state.
*/
typedef struct
{
  // The current "power save" state.
  ps_state_enum  src_state;
  // The next "power save" state.
  ps_state_enum  dest_state;
  // The time (sec; without US), causing transition to the next state.
  uint32_t       dest_timeout;
  // Work function in the current state
  usf_work_func  pwf;

  // US detection info
  us_detect_info_type *us_detect_info;
} ps_state_info_type;

/*-----------------------------------------------------------------------------
  Static + Externs Variable Definitions
-----------------------------------------------------------------------------*/

extern EposParams eposParams;

extern int send_event(
  int event_num,
  int extra1,
  int extra2
);

// Inits epos should be called when going into active state
extern void epos_init(bool first_time);

// For pen dynamic library
extern UsfEposDynamicLibProxy* epos_lib_proxy;

/**
  Number of the "power save" (Active, Standby, Idle) states, can
  be used as source state.
*/
static const uint16_t PS_SRC_STATES_NUMBER = 3;

/**
  Number of all "power save" (Active, Standby, Idle, OFF)
  states, can be used as destination state.
*/
static const uint16_t PS_DST_STATES_NUMBER = 4;

/**
  The US detect info for the Standby state.
*/
static us_detect_info_type standby_us_detect_info;

/**
  The US detect info for the Idle state.
*/
static us_detect_info_type idle_us_detect_info;

/**
  The current PS state.
*/
static ps_state_enum s_ps_current_state = PS_STATE_ACTIVE;

/**
  Is PS supported (true).
*/
static bool sb_ps = false;

/**
  PS state transition timer isn't activated.
*/
static uint32_t PS_NO_TRANSIT_TIME = 0;

/**
  Time (in sec since the Epoch) to start PS state transition
*/
static uint32_t s_transit_time = PS_NO_TRANSIT_TIME;

/**
  US detection port in idle state.
*/
static uint32_t s_ps_idle_detect_port;

/**
 US detection period (in sec) in idle state
 Value 0 - contininue detection mode
 Other value - one shot detection mode
*/
static uint32_t s_ps_idle_detect_period;

/**
  Signal to start PS transition (!PS_NO_TRANSIT).
*/
static ps_transit_enum s_ps_transit = PS_NO_TRANSIT;

/**
  Gently exists in case of critical error
*/
extern int epos_exit(int status);

/**
  Work function in the PS Active state.
*/
extern bool usf_act_work_func(work_params_type&, ps_transit_enum&);

/**
  Work function in the PS Standby state.
*/
static bool ps_standby_work_func(work_params_type&, ps_transit_enum&);

/**
  Work function in the PS Idle state.
*/
static bool ps_idle_work_func(work_params_type&, ps_transit_enum&);

/**
  The "power save" (Active, Standby, Idle) states info container
  act_work_func() should be provided by configure_ps()
*/
static ps_state_info_type s_ps_state_container[PS_SRC_STATES_NUMBER] =
{
  {PS_STATE_ACTIVE,  PS_STATE_UNDEF, 0, usf_act_work_func,    NULL},
  {PS_STATE_STANDBY, PS_STATE_UNDEF, 0, ps_standby_work_func, &standby_us_detect_info},
  {PS_STATE_IDLE,    PS_STATE_UNDEF, 0, ps_idle_work_func,    &idle_us_detect_info},
};

/*----------------------------------------------------------------------------
  Function definitions
----------------------------------------------------------------------------*/

/*==============================================================================
  FUNCTION:  check_ps_state
==============================================================================*/
/**
  Checks if ps state is one of the source states.
*/
static inline void check_ps_state
(
  ps_state_enum ps_state
)
{
  if (ps_state >= PS_SRC_STATES_NUMBER)
  {
    LOGE("%s: Unexpected state for ps %d",
         __FUNCTION__,
         ps_state);
    epos_exit(1);
  }
}

/*==============================================================================
  FUNCTION:  leave_active_state
==============================================================================*/
/**
  Called when leaving active state
 */
static void leave_active_state()
{
  // Notifies feedback handlers that we have left active state
  usf_epos_notify_active_state_exit();

  // Release EPOS library
  epos_lib_proxy->release_dsp(eposParams.m_epos_workspace,
                              NULL);
}

/*============================================================================
  FUNCTION:  ps_act2standby
============================================================================*/
/**
  The "power save" Active to Standby transition function.
*/
static bool ps_act2standby(work_params_type& work_params)
{

  if (NULL == s_ps_state_container[PS_STATE_STANDBY].us_detect_info)
  {
    LOGE("%s: Standby: wrong detect info",
         __FUNCTION__);
    return false;
  }

  s_ps_state_container[PS_STATE_STANDBY].us_detect_info->detect_timeout =
    USF_NO_WAIT_TIMEOUT;
  s_ps_state_container[PS_STATE_STANDBY].us_detect_info->us_detect_mode =
    US_DETECT_CONTINUE_MODE;


  bool rc = ual_start_us_detection(
    *(s_ps_state_container[PS_STATE_STANDBY].us_detect_info));

  if (rc)
  {
    s_ps_current_state = PS_STATE_STANDBY;
    leave_active_state();
  }
  else
  {
      LOGE("%s: us detection failed, rc=%d",
         __FUNCTION__,
         rc);
      epos_exit(EXIT_FAILURE);
  }

  LOGD("%s: detect timeout=%d; rc=%d",
       __FUNCTION__,
       s_ps_state_container[PS_STATE_STANDBY].us_detect_info->detect_timeout,
       rc);

  return rc;
} // ps_act2standby

/*============================================================================
  FUNCTION:  ps_act2idle
============================================================================*/
/**
  The "power save" Active to Idle transition function.
*/
static bool ps_act2idle(work_params_type& work_params)
{
  if (NULL == s_ps_state_container[PS_STATE_IDLE].us_detect_info)
  {
    LOGE("%s: wrong detect info",
         __FUNCTION__);
    return false;
  }

  s_ps_state_container[PS_STATE_IDLE].us_detect_info->detect_timeout =
                        s_ps_state_container[PS_STATE_IDLE].dest_timeout;
  s_ps_current_state = PS_STATE_IDLE;
  LOGD("%s: detect timeout=%d",
       __FUNCTION__,
       s_ps_state_container[PS_STATE_IDLE].us_detect_info->detect_timeout);

  leave_active_state();

  return true;
} // ps_act2idle

/*============================================================================
  FUNCTION:  ps_standby2idle
============================================================================*/
/**
  The "power save" Standby to Idle transition function.
*/
static bool ps_standby2idle(work_params_type& work_params)
{
  if (NULL == s_ps_state_container[PS_STATE_IDLE].us_detect_info)
  {
    LOGE("%s: wrong detect info",
         __FUNCTION__);
    return false;
  }

  s_ps_state_container[PS_STATE_IDLE].us_detect_info->detect_timeout =
                        s_ps_state_container[PS_STATE_IDLE].dest_timeout;
  s_ps_current_state = PS_STATE_IDLE;
  LOGD("%s: detect timeout=%d",
       __FUNCTION__,
       s_ps_state_container[PS_STATE_IDLE].us_detect_info->detect_timeout);
  return true;
} // ps_standby2idle

/*============================================================================
  FUNCTION:  ps_X2off
============================================================================*/
/**
  The "power save" Active/Standby/Idle to Off transition function.
*/
static bool ps_X2off(work_params_type& work_params)
{
  LOGI("%s: entry",
       __FUNCTION__);

  return false; // it causes to cancel the calculator
} // ps_X2off


/*============================================================================
  FUNCTION:  enter_active_state
============================================================================*/
/**
  Call when entering active state
*/
static void enter_active_state(work_params_type& work_params)
{
  s_transit_time = PS_NO_TRANSIT_TIME;
  s_ps_current_state = PS_STATE_ACTIVE;
  // Init epos lib (false means "not the first time call")
  epos_init(false);
  work_params.bActZone = true;
}

/*============================================================================
  FUNCTION:  ps_standby2act
============================================================================*/
/**
  The "power save" Standby to Active transition function.
*/
static bool ps_standby2act(work_params_type& work_params)
{
  LOGI("%s: entry",
       __FUNCTION__);

  enter_active_state(work_params);

  return true;
} // ps_standby2act

/*============================================================================
  FUNCTION:  ps_idle2act
============================================================================*/
/**
  The "power save" Idle to Active transition function.
*/
static bool ps_idle2act(work_params_type& work_params)
{
  LOGI("%s: entry",
       __FUNCTION__);

  bool rc = ual_stop_TX();
  LOGD("%s: stop TX of Idle; rc=%d",
       __FUNCTION__,
       rc);
  if (!rc)
  {
    return false;
  }

  // US TX should support the three TSC event types
  // Backup event_type from the configuration file
  uint32_t temp_event_type = work_params.paramsStruct.usf_event_type;
  work_params.paramsStruct.usf_event_type = USF_TSC_EVENT |
                                            USF_TSC_PTR_EVENT |
                                            USF_TSC_EXT_EVENT;

  int ret = ual_util_tx_config(&work_params.paramsStruct,
                              (char *)CLIENT_NAME);
  if (-1 == ret)
  {
    return false;
  }

  // Restore event_type from the configuration file
  work_params.paramsStruct.usf_event_type = temp_event_type;

  // Enter active state
  enter_active_state(work_params);

  return true;
} // ps_idle2act

/**
  The "power save" (Active, Standby, Idle) transition functions.
*/
static ps_transit_func
      ps_transit_func_table[PS_SRC_STATES_NUMBER][PS_DST_STATES_NUMBER] =
{
  {NULL,           ps_act2standby, ps_act2idle,     ps_X2off},
  {ps_standby2act, NULL,           ps_standby2idle, ps_X2off},
  {ps_idle2act,    NULL,           NULL,            ps_X2off},
};


/*==============================================================================
  FUNCTION:  configure_ps
==============================================================================*/
/**
  Configure PS mechanism.
*/
void configure_ps(us_all_info& paramsStruct)
{
  char *pStateParamsStr[PS_SRC_STATES_NUMBER] =
  {
    paramsStruct.ps_act_state_params,
    paramsStruct.ps_standby_state_params,
    paramsStruct.ps_idle_state_params
  };

  for (uint16_t ind=0; ind<PS_SRC_STATES_NUMBER; ++ind)
  {
    if (NULL != pStateParamsStr[ind])
    {
      int ret = sscanf(pStateParamsStr[ind],
                       "%d ,%d",
                       (int*)(&s_ps_state_container[ind].dest_state),
                       &s_ps_state_container[ind].dest_timeout);
      if ((2 != ret) ||
          (PS_STATE_OFF < s_ps_state_container[ind].dest_state) ||
          (s_ps_state_container[ind].src_state >
           s_ps_state_container[ind].dest_state))
      {
        LOGE("%s: wrong PS state(%d) info: [%s]",
             __FUNCTION__,
             ind,
             pStateParamsStr[ind]);
        LOGD("%s: Power save is disabled",
             __FUNCTION__);
        return;
      }
      if (0 == s_ps_state_container[ind].dest_timeout)
      { // minimal value of the timeout is 1
        s_ps_state_container[ind].dest_timeout = 1;
      }
    }
  } // PS state info loop

  if ((PS_STATE_ACTIVE == s_ps_state_container[PS_STATE_ACTIVE].dest_state) ||
      (paramsStruct.usf_epos_timeout_to_coord_rec > 0))
  {
    // ps is disabled also If calibration is done
    LOGI("%s: Power save is disabled",
         __FUNCTION__);
    return;
  }

  // US detect info only for the Standby & Idle states
  char *pDetectParamsStr[PS_SRC_STATES_NUMBER-1] =
  {
    paramsStruct.ps_standby_detect_info,
    paramsStruct.ps_idle_detect_info,
  };

  for (uint16_t ind=1; ind<PS_SRC_STATES_NUMBER; ++ind)
  {
    if ((NULL != pDetectParamsStr[ind-1]) &&
       (NULL != s_ps_state_container[ind].us_detect_info))
    {
      int ret = sscanf(
                 pDetectParamsStr[ind-1],
                 "%d ,%d",
                 (int*)&s_ps_state_container[ind].us_detect_info->us_detector,
                 &s_ps_state_container[ind].us_detect_info->skip_time);

      if ((2 != ret) ||
          (US_DETECT_FW <
           s_ps_state_container[ind].us_detect_info->us_detector))
      {
        LOGE("%s: wrong US detect info(%d): [%s]",
             __FUNCTION__,
             ind,
             pDetectParamsStr[ind-1]);
        LOGD("%s: Power save is disabled",
             __FUNCTION__);
        return;
      }
    }
  } // detect info loop

  // US detect calibration only for the Standby & Idle states
  char *pDetectCalibStr[PS_SRC_STATES_NUMBER-1] =
  {
    paramsStruct.ps_standby_detect_calibration,
    paramsStruct.ps_idle_detect_calibration,
  };

  for (uint16_t ind=1; ind<PS_SRC_STATES_NUMBER; ++ind)
  {
    if ((NULL != pDetectCalibStr[ind-1]) &&
       (NULL != s_ps_state_container[ind].us_detect_info))
    {
      uint32_t calibSize = 0;
      void* pCalibration = ual_util_malloc_read(pDetectCalibStr[ind-1], calibSize);
      if (NULL == pCalibration)
      {
        LOGW("%s: No calibration for state ind=%d; file=%s",
             __FUNCTION__,
             ind,
             pDetectCalibStr[ind-1]);
        continue;
      }
      s_ps_state_container[ind].us_detect_info->params_data_size = calibSize;
      s_ps_state_container[ind].us_detect_info->params_data = (uint8_t*)pCalibration;
    }
  } // detect calibration loop

  s_ps_idle_detect_port = paramsStruct.ps_idle_detect_port;
  s_ps_idle_detect_period = paramsStruct.ps_idle_detect_period;

  sb_ps = true;
  LOGD("%s:  Power save is enabled",
       __FUNCTION__);

  return;
} // configure_ps


/*==============================================================================
  FUNCTION:  ps_standby_work_func
==============================================================================*/
/**
  Work function in the PS Standby state.
*/
bool ps_standby_work_func(work_params_type& work_params, ps_transit_enum& ps_transit)
{
  ual_data_type data;

  LOGI("%s: entry",
       __FUNCTION__);

  // To drop non-US frames from the shared memory
  bool rc = ual_read(&data,
                     NULL,
                     0,
                     USF_NO_WAIT_TIMEOUT);
  LOGD("%s: ual_read() for drop: rc=%d; size=%d",
       __FUNCTION__,
       rc,
       data.region[0].data_buf_size);

  rc = ual_read(&data,
                NULL,
                0,
                s_ps_state_container[PS_STATE_STANDBY].dest_timeout);
  if (!rc)
  {
    LOGE("%s: ual_read failed rc=%d",
         __FUNCTION__,
         rc);
    epos_exit(EXIT_FAILURE);
  }

  if (0 == data.region[0].data_buf_size)
  { // Time to transit from the Standby, as there is no US.
    LOGD("%s: Time to transit from the Standby, as there is no US",
         __FUNCTION__);
    ps_transit = PS_NO_US_TRANSIT;
    return true;
  }

  // Assuming, US exists - flag isn't used here
  bool b_us = true;

  for (int r = 0; r < work_params.num_of_regions; r++)
  {
    work_params.numberOfFrames = data.region[r].data_buf_size /
                                    work_params.frame_size_in_bytes;
    work_params.nextPacket =  data.region[r].data_buf;

    rc = usf_epos_get_points(b_us);
    if (!rc)
    {
      return false;
    }
  } // End for r (regions)

  ps_transit = PS_US_TRANSIT;

  return true;
} // ps_standby_work_func

/*==============================================================================
  FUNCTION:  ps_idle_work_func
==============================================================================*/
/**
  Work function in the PS Idle state.
*/
bool ps_idle_work_func(work_params_type& work_params,
                       ps_transit_enum& ps_transit)
{
  const char* PS_IDLE_CLIENT = "epos_us_detect";
  const uint32_t PS_IDLE_TX_BUF_SIZE = 1044; // see cfg/readme.txt
  uint32_t transp_data = 0x00010001; // skip=group=1
  us_tx_info_type tx_info;
  uint32_t elapsed_time = 0;
  us_detect_info_type *p_detect_info =
                      s_ps_state_container[PS_STATE_IDLE].us_detect_info;

  LOGI("%s: entry",
       __FUNCTION__);

  if (NULL == p_detect_info)
  {
    LOGE("%s: wrong detect info",
         __FUNCTION__);
    return false;
  }

  // TX path configuration prepare
  tx_info.us_xx_info.client_name = PS_IDLE_CLIENT;
  tx_info.us_xx_info.dev_id =
    work_params.paramsStruct.usf_device_id;

  tx_info.us_xx_info.stream_format =
    work_params.paramsStruct.usf_tx_data_format;

  tx_info.us_xx_info.sample_rate =
    work_params.paramsStruct.usf_tx_sample_rate;

  tx_info.us_xx_info.buf_num = 1;
  tx_info.us_xx_info.port_cnt = 1;
  tx_info.us_xx_info.port_id[0] = s_ps_idle_detect_port;

  tx_info.us_xx_info.bits_per_sample =
    work_params.paramsStruct.usf_tx_sample_width;

  tx_info.input_info.event_types = USF_NO_EVENT;

  // TX transparent data
  tx_info.us_xx_info.params_data_size =
    work_params.paramsStruct.usf_tx_transparent_data_size;
  tx_info.us_xx_info.params_data = reinterpret_cast<uint8_t *>(&transp_data);

  tx_info.us_xx_info.buf_size = PS_IDLE_TX_BUF_SIZE;

  uint16_t try_num = 1; // for continue detection mode
  if (0 < s_ps_idle_detect_period)
  { // one shot detection mode
    p_detect_info->us_detect_mode = US_DETECT_SHOT_MODE;
    try_num = (p_detect_info->detect_timeout + s_ps_idle_detect_period-1)/
              s_ps_idle_detect_period; // round up
    // In one-shot mode QDSP6 detects US "immediately",
    // independent of (p_detect_info->detect_timeout) value
  }
  else
  { // continue detection mode
    p_detect_info->us_detect_mode = US_DETECT_CONTINUE_MODE;
  }

  ps_transit = PS_NO_US_TRANSIT;
  bool rc = true;
  for (uint16_t itry=0; itry<try_num; ++itry)
  {
    rc = is_usf_epos_running();
    if (!rc)
    {
      LOGD("%s: usf_epos isn't running; itry=%d",
           __FUNCTION__,
           itry);
      break;
    }

    rc = ual_stop_TX();
    LOGD("%s: stop TX of Act/Standby; rc=%d; itry=%d",
         __FUNCTION__,
         rc,
         itry);
    if (!rc)
    {
      break;
    }

    uint32_t ret = sleep(s_ps_idle_detect_period);
    if (ret)
    {
      rc = false;
      LOGD("%s: usf_epos got signal to stop; itry=%d",
           __FUNCTION__,
           itry);
      break;
    }

    rc = ual_configure_TX(&tx_info);
    if (!rc)
    {
      break;
    }

    rc = ual_start_TX();
    LOGD("%s: start TX of Idle; rc=%d; itry=%d",
         __FUNCTION__,
         rc,
         itry);
    if (!rc)
    {
      break;
    }

    rc = ual_start_us_detection(*p_detect_info);
    LOGD("%s: start US detect; rc=%d; is_us=%d; itry=%d",
         __FUNCTION__,
         rc,
         p_detect_info->is_us,
         itry);
    if (!rc)
    {
      LOGE("%s: us detection failed, rc=%d",
           __FUNCTION__,
           rc);
      epos_exit(EXIT_FAILURE);
    }

    if (p_detect_info->is_us)
    {
      ps_transit = PS_US_TRANSIT;
      break;
    }

  } //detection tries loop

  return rc;
} // ps_idle_work_func

/*==============================================================================
  FUNCTION:  usf_ps_act_set_transit_timer
==============================================================================*/
/**
  Control transit timer in the PS Active state.
*/
void  usf_ps_act_set_transit_timer(bool b_us, ps_transit_enum& ps_transit)
{
  if (sb_ps)
  { // PS is supported
    // US signal check upon the last fb_info report
    if (b_us)
    { // There is US - stop transition timing
      if (PS_NO_TRANSIT_TIME != s_transit_time)
      {
        s_transit_time = PS_NO_TRANSIT_TIME;
        LOGD("%s: Stop no_us timer in the Active state",
             __FUNCTION__);
      }
      ps_transit = PS_NO_TRANSIT;
    }
    else
    { // No US - handle the transition timer
      if (PS_NO_TRANSIT_TIME != s_transit_time)
      { // Check elapsed time
        uint32_t current_time = time(NULL);
        if (current_time >= s_transit_time)
        { // It's time for the PS state transition
          LOGD("%s: Transmission, current_time=%d; s_transit_time=%d",
               __FUNCTION__,
               current_time,
               s_transit_time);
          ps_transit = PS_NO_US_TRANSIT;
        }
      }
      else
      { // start the transition timer
        s_transit_time = time(NULL) +
                          s_ps_state_container[PS_STATE_ACTIVE].dest_timeout;
        LOGD("%s: Start no_us timer (%d) in Active state; transit_time=%d",
             __FUNCTION__,
             s_ps_state_container[PS_STATE_ACTIVE].dest_timeout,
             s_transit_time);
      }
    }
  }
} // ps_act_set_transit_timer

/*==============================================================================
  FUNCTION:  usf_ps_transit
==============================================================================*/
/**
  Transit from the current state to the next one, upon ps_transit value
*/
static bool usf_ps_transit(work_params_type& work_params, ps_transit_enum& ps_transit)
{

  // Checks the current state (exits if the state is wrong)
  check_ps_state(s_ps_current_state);

  if (PS_NO_TRANSIT == ps_transit)
  {
    return true;
  }

  bool rc = true;
  ps_state_enum dst_state = PS_STATE_ACTIVE; // for PS_US_TRANSIT

  if (PS_NO_US_TRANSIT == ps_transit)
  {
    dst_state = s_ps_state_container[s_ps_current_state].dest_state;
  }

  if ((PS_DST_STATES_NUMBER > dst_state) &&
      (s_ps_current_state != dst_state) )
  {
    ps_transit_func pstf=ps_transit_func_table[s_ps_current_state][dst_state];
    if (NULL != pstf)
    {
      ps_state_enum prev_state = s_ps_current_state;
      rc = (*pstf)(work_params);
      if (rc)
      {
        LOGD("Sending power state changed event");
        send_event(POWER_STATE_CHANGED,
                   s_ps_current_state,
                   prev_state);
      }
    }
  }

  return rc;
} // usf_ps_transit

/*==============================================================================
  FUNCTION:  usf_ps_work
==============================================================================*/
/**
  Dispatch control among PS state's work functions
*/
bool usf_ps_work(work_params_type& work_params)
{
  // Checks the current state (exits if the state is wrong)
  check_ps_state(s_ps_current_state);

  // s_ps_current_state may be changed during the PS transitions
  // s_ps_current_state is trusty valid
  // s_ps_state_container[s_ps_current_state].pwf is trusty not NULL
  usf_work_func pwf = s_ps_state_container[s_ps_current_state].pwf;

  bool rc = (*pwf)(work_params, s_ps_transit);

  if (sb_ps && rc)
  {
    rc = usf_ps_transit(work_params, s_ps_transit);
    s_ps_transit = PS_NO_TRANSIT;
  }

  return rc;
} // usf_ps_work

/*==============================================================================
  FUNCTION:  usf_ps_is_active
==============================================================================*/
/**
  Is the calculator in the Active PS state?
*/
bool usf_ps_is_active(void)
{
  return ((PS_NO_TRANSIT_TIME == s_transit_time) &&
          (s_ps_current_state == PS_STATE_ACTIVE));
}
