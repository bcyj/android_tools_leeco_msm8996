/******************************************************************************
  @file    mp-ctl.h
  @brief   Header file for communication and actions for PerfLock

  DESCRIPTION

  ---------------------------------------------------------------------------

  Copyright (c) 2011,2014 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
******************************************************************************/

#ifndef __MP_CTL_H__
#define __MP_CTL_H__

/* Server functions */
int  mpctl_server_init(void);
void mpctl_server_exit(void);

/* Client function (up to 0-7)*/
#define MPCTL_CMD_PERFLOCKACQ 2
#define MPCTL_CMD_PERFLOCKREL 3
#define MPCTL_CMD_PERFLOCKPOLL 4
#define MPCTL_CMD_PERFLOCKRESET 5

/* Resource identifier */
enum resources {
    DISPLAY = 0,
    POWER_COLLAPSE,
    CPU0_MIN_FREQ,
    CPU1_MIN_FREQ,
    CPU2_MIN_FREQ,
    CPU3_MIN_FREQ,
    UNSUPPORTED_0,
    CLUSTR_0_CPUS_ON,
    CLUSTR_0_MAX_CORES,
    UNSUPPORTED_2,
    UNSUPPORTED_3,
    SAMPLING_RATE,
    ONDEMAND_IO_IS_BUSY,
    ONDEMAND_SAMPLING_DOWN_FACTOR,
    INTERACTIVE_TIMER_RATE,
    INTERACTIVE_HISPEED_FREQ,
    INTERACTIVE_HISPEED_LOAD,
    SYNC_FREQ,
    OPTIMAL_FREQ,
    SCREEN_PWR_CLPS,
    THREAD_MIGRATION,
    CPU0_MAX_FREQ,
    CPU1_MAX_FREQ,
    CPU2_MAX_FREQ,
    CPU3_MAX_FREQ,
    ONDEMAND_ENABLE_STEP_UP,
    ONDEMAND_MAX_INTERMEDIATE_STEPS,
    INTERACTIVE_IO_BUSY,
    KSM_RUN_STATUS,
    KSM_PARAMS,
    SCHED_BOOST,
    CPU4_MIN_FREQ,
    CPU5_MIN_FREQ,
    CPU6_MIN_FREQ,
    CPU7_MIN_FREQ,
    CPU4_MAX_FREQ,
    CPU5_MAX_FREQ,
    CPU6_MAX_FREQ,
    CPU7_MAX_FREQ,
    CPU0_INTERACTIVE_ABOVE_HISPEED_DELAY,
    CPU0_INTERACTIVE_BOOST,
    CPU0_INTERACTIVE_BOOSTPULSE,
    CPU0_INTERACTIVE_BOOSTPULSE_DURATION,
    CPU0_INTERACTIVE_GO_HISPEED_LOAD,
    CPU0_INTERACTIVE_HISPEED_FREQ,
    CPU0_INTERACTIVE_IO_IS_BUSY,
    CPU0_INTERACTIVE_MIN_SAMPLE_TIME,
    CPU0_INTERACTIVE_TARGET_LOADS,
    CPU0_INTERACTIVE_TIMER_RATE,
    CPU0_INTERACTIVE_TIMER_SLACK,
    CPU4_INTERACTIVE_ABOVE_HISPEED_DELAY,
    CPU4_INTERACTIVE_BOOST,
    CPU4_INTERACTIVE_BOOSTPULSE,
    CPU4_INTERACTIVE_BOOSTPULSE_DURATION,
    CPU4_INTERACTIVE_GO_HISPEED_LOAD,
    CPU4_INTERACTIVE_HISPEED_FREQ,
    CPU4_INTERACTIVE_IO_IS_BUSY,
    CPU4_INTERACTIVE_MIN_SAMPLE_TIME,
    CPU4_INTERACTIVE_TARGET_LOADS,
    CPU4_INTERACTIVE_TIMER_RATE,
    CPU4_INTERACTIVE_TIMER_SLACK,
    CLUSTR_1_MAX_CORES,
    SCHED_PREFER_IDLE, 
    SCHED_MIGRATE_COST,
    SCHED_SMALL_TASK,
    SCHED_MOSTLY_IDLE_LOAD,
    SCHED_MOSTLY_IDLE_NR_RUN,
    SCHED_INIT_TASK_LOAD,
    VIDEO_DECODE_PLAYBACK_HINT,
    DISPLAY_LAYER_HINT,
    VIDEO_ENCODE_PLAYBACK_HINT,
    CPUBW_HWMON_MIN_FREQ,
    CPUBW_HWMON_DECAY_RATE,
    CPUBW_HWMON_IO_PERCENT,
    CPU0_INTERACTIVE_MAX_FREQ_HYSTERESIS,
    CPU4_INTERACTIVE_MAX_FREQ_HYSTERESIS,
    GPU_DEFAULT_PWRLVL,
    CLUSTR_1_CPUS_ON,
    SCHED_UPMIGRATE,
    SCHED_DOWNMIGRATE,
    /* represents the maximum number of
     * optimizations allowed per
     * request and should always be
     * the last element
     */
    OPTIMIZATIONS_MAX
};

int mpctl_send(int control /* MPCTL_CMD_* */, ...);

#endif /* __MP_CTL_H__ */
