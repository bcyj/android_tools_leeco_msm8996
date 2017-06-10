/*****************************************************************************
* Copyright (c) 2011,2013-2014 Qualcomm Technologies, Inc. All Rights Reserved. *
* Qualcomm Technologies Proprietary and Confidential. *
*****************************************************************************/

#ifndef __PERFORMANCE_H__
#define __PERFORMANCE_H__

#ifdef __cplusplus
extern "C" {
#endif

#define FAILED                  -1
#define SUCCESS                 0
#define INDEFINITE_DURATION     0

enum SCREEN_DISPLAY_TYPE {
    DISPLAY_OFF = 0x00FF,
};

enum PWR_CLSP_TYPE {
    ALL_CPUS_PWR_CLPS_DIS = 0x100,
};

/* For CPUx min freq, the leftmost byte
 * represents the CPU and the
 * rightmost byte represents the frequency
 * All intermediate frequencies on the
 * device are supported. The hex value
 * passed into PerfLock will be multiplied
 * by 10^5. This frequency or the next
 * highest frequency available will be set
 *
 * For example, if 1.4 Ghz is required on
 * CPU0, use 0x20E
 *
 * If the highest available frequency
 * on the device is required, use
 * CPUx_MIN_FREQ_TURBO_MAX
 * where x represents the CPU
 */
enum CPU0_MIN_FREQ_LVL {
    CPU0_MIN_FREQ_NONTURBO_MAX = 0x20A,
    CPU0_MIN_FREQ_TURBO_MAX = 0x2FE,
};

enum CPU1_MIN_FREQ_LVL {
    CPU1_MIN_FREQ_NONTURBO_MAX = 0x30A,
    CPU1_MIN_FREQ_TURBO_MAX = 0x3FE,
};

enum CPU2_MIN_FREQ_LVL {
    CPU2_MIN_FREQ_NONTURBO_MAX = 0x40A,
    CPU2_MIN_FREQ_TURBO_MAX = 0x4FE,
};

enum CPU3_MIN_FREQ_LVL {
    CPU3_MIN_FREQ_NONTURBO_MAX = 0x50A,
    CPU3_MIN_FREQ_TURBO_MAX = 0x5FE,
};

enum CPUS_ONLINE_LVL {
    CPUS_ONLINE_MIN_2 = 0x702,
    CPUS_ONLINE_MIN_3 = 0x703,
    CPUS_ONLINE_MIN_4 = 0x704,
    CPUS_ONLINE_MAX = 0x704,
    CPUS_ONLINE_MAX_LIMIT_1 = 0x7FE,
    CPUS_ONLINE_MAX_LIMIT_2 = 0x7FD,
    CPUS_ONLINE_MAX_LIMIT_3 = 0x7FC,
    CPUS_ONLINE_MAX_LIMIT_4 = 0x7FB,
    CPUS_ONLINE_MAX_LIMIT_MAX = 0x7FB,
};

enum ALL_CPUS_FREQBOOST_LVL {
    ALL_CPUS_FREQ_NONTURBO_MAX = 0x90A,
    ALL_CPUS_FREQ_TURBO = 0x9FE,
};

enum SAMPLING_RATE_LVL {
    MS_500 = 0xBCD,
    MS_50 = 0xBFA,
    MS_20 = 0xBFD,
};

enum ONDEMAND_IO_BUSY_LVL {
    IO_BUSY_OFF = 0xC00,
    IO_BUSY_ON = 0xC01,
};

enum ONDEMAND_SAMPLING_DOWN_FACTOR_LVL {
    SAMPLING_DOWN_FACTOR_1 = 0xD01,
    SAMPLING_DOWN_FACTOR_4 = 0xD04,
};

enum INTERACTIVE_TIMER_RATE_LVL {
    TR_MS_500 = 0xECD,
    TR_MS_30 = 0xEFC,
    TR_MS_20 = 0xEFD,
};

enum INTERACTIVE_HISPEED_FREQ_LVL {
    HS_FREQ_1026 = 0xF0A,
};

enum INTERACTIVE_HISPEED_LOAD_LVL {
    HISPEED_LOAD_90 = 0x105A,
};

enum SYNC_FREQ_LVL {
    SYNC_FREQ_300 = 0x1103,
    SYNC_FREQ_600 = 0X1106,
    SYNC_FREQ_384 = 0x1103,
    SYNC_FREQ_NONTURBO_MAX = 0x110A,
    SYNC_FREQ_TURBO = 0x110F,
};

enum OPTIMAL_FREQ_LVL {
    OPTIMAL_FREQ_300 = 0x1203,
    OPTIMAL_FREQ_600 = 0x1206,
    OPTIMAL_FREQ_384 = 0x1203,
    OPTIMAL_FREQ_NONTURBO_MAX = 0x120A,
    OPTIMAL_FREQ_TURBO = 0x120F,
};

enum SCREEN_PWR_CLPS_LVL {
    PWR_CLPS_DIS = 0x1300,
    PWR_CLPS_ENA = 0x1301,
};

enum THREAD_MIGRATION_LVL {
    THREAD_MIGRATION_SYNC_OFF = 0x1400,
};
#ifdef __cplusplus
}
#endif

#endif // __PERFORMANCE_H__
