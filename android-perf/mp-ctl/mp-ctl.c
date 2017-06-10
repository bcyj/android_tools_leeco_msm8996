/******************************************************************************
  @file    mp-ctl.c
  @brief   Implementation of performance server module

  DESCRIPTION

  ---------------------------------------------------------------------------

  Copyright (c) 2011-2014 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
******************************************************************************/

#define ATRACE_TAG ATRACE_TAG_ALWAYS
#include "mp-ctl.h"

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include <signal.h>

#include <private/android_filesystem_config.h>
#include <cutils/properties.h>
#include <cutils/trace.h>

#ifdef SERVER
#include <dlfcn.h>
#endif

#define LOG_TAG           "ANDR-PERF-MPCTL"
#include <cutils/log.h>
#if defined(ANDROID_JELLYBEAN)
#include "common_log.h"
#endif
#include "performance.h"

#if QC_DEBUG
#define QLOGE(...)    ALOGE(__VA_ARGS__)
#define QLOGW(...)    ALOGW(__VA_ARGS__)
#define QLOGI(...)    ALOGI(__VA_ARGS__)
#define QLOGV(...)    ALOGV(__VA_ARGS__)
#else
#define QLOGE(...)
#define QLOGW(...)
#define QLOGI(...)
#define QLOGV(...)
#endif

#define MPCTL_VERSION      2
#define MPCTL_SOCKET       "/data/misc/perfd/mpctl"

#define MPCTL_MAGIC        0x4d502d43 /* "MP-C" */
#define SOCID_8994         207

#define MIN_FREQ_REQ      0
#define MAX_FREQ_REQ      1

typedef struct _mpctl_msg_t {
    uint32_t magic;
    uint8_t  version;
    uint8_t  seq;
    uint16_t data;
    uint32_t pl_handle;
    int pl_time;
    int pl_args[OPTIMIZATIONS_MAX];
    union {
        uint32_t msg;
        uint8_t  m[4];
    };
    pid_t    client_pid;
    pid_t    client_tid;
} mpctl_msg_t;

#define CMD_ENCODE(msg, cmd)      { (msg).msg = rand(); \
                                    (msg).m[1+((msg).seq & 1)] &= 0xE3; \
                                    (msg).m[1+((msg).seq & 1)] |= (cmd << 2); }
#define CMD_DECODE(msg)           (((msg).m[1+((msg).seq & 1)] & 0x1C) >> 2)

#define MAX_CPUS            8
#define TIME_MSEC_IN_SEC    1000
#define TIME_NSEC_IN_MSEC   1000000
#define ON                  1
#define LOCK_MIN            0x0
#define LOCK_MAX            0xF
#define MAX_LVL             0xFF
#define POLL_TIMER_MS       60000
#define TRACE_BUF_SZ        512
#define CO_NODE_MAX_LEN     32
#define FREQ_SIZE           10
#define MAX_FREQ            50
#define BILLION             1000000000L
#define ENABLE_PC_LATENCY   0
#define DISABLE_PC_LATENCY  1
#define RESET_SCHED_BOOST   0
#define ENABLE_PREFER_IDLE  1
#define WRITE_MIGRATE_COST  0
#ifdef SERVER
#define QLOGE(...) \
    if (perf_debug_output) \
        ALOGE(__VA_ARGS__)
#define QLOGW(...) \
    if (perf_debug_output) \
        ALOGW(__VA_ARGS__)
#define QLOGI(...) \
    if (perf_debug_output) \
        ALOGI(__VA_ARGS__)
#define QLOGV(...) \
    if (perf_debug_output) \
        ALOGV(__VA_ARGS__)

static int perf_debug_output = 0;

#ifdef PERFD
static int check_core_ctl_presence();
static int core_ctl_init();
#else
extern int mpdecision_toggle_power_collapse(int);
extern int mpdecision_lock_min_cores(int);
extern int mpdecision_lock_max_cores(int);
#endif

int core_status[MAX_CPUS] = {0};
char co_node[CO_NODE_MAX_LEN];
static int max_num_cpus = 0;
static int max_num_cpu_supt = 4;
static int max_num_cpu_clustr_0 = 4;
static int max_num_cpu_clustr_1 = 0;
static int cpu_count_clustr_0 = 4;
static int cpu_count_clustr_1 = 4;
static int comsoc = -1;
static int core_ctl_present = -1;    //1 represent core_ctl is present, 0 core ctl not present, -1 not initialized
static int kpm_hotplug_support = -1; //1 represent hotplug is supported through KPM, 0 hotplug not supported, -1 not initialized
static int core_ctl_cpu = -1;       //define for each chipset, based on their core control cpu
static struct sockaddr_un addr;

struct timespec m_accept;
struct timespec connected;
struct timespec recvd;
struct timespec ack;
struct timespec acq_completed;

uint64_t time_delta;
uint64_t time_delta2;

struct request {
    timer_t timer;
    pid_t pid;
    pid_t tid;
    int duration;
    int num_args;
    int opt[OPTIMIZATIONS_MAX];
};

struct q_node {
    struct request *handle;
    int level;
    struct q_node *next;
};

struct list_node {
    int return_handle;
    struct request *handle;
    struct list_node *next;
};

struct cpu_freq_resource {
    int ideal_cpu_id;
    int mapped_cpu;
};

struct cpu_freq_mapping {
    char * min;
    char * max;
    char * online;
    int in_use;
};

struct cpu_freq_resource_value {
    int avl_freq[MAX_FREQ];
    int count;
    int min_freq_pos;
    int valid;
};

#define ACTIVE_REQS_MAX         30

/* The reason this is defined as CPU7_MAX_FREQ+1 is because
 * we want an array where we can use the resource id of CPU
 * freq as the index so that we can find its ideal_cpu_id
 * and mapped_cpu easily.  Therefore we'll need an array
 * of at least 39+1 elements since CPU7_MAX_FREQ == 39.
 */
#define CPU_FREQ_RESOURCES_MAX  CPU7_MAX_FREQ+1
static struct list_node *active_list_head = NULL;
static int active_reqs = 0;
static struct q_node resource_qs[OPTIMIZATIONS_MAX];

static pthread_t mpctl_server_thread;
static mpctl_msg_t msg;
static int conn_socket;
static timer_t poll_timer;

static int min_cores = 0;
static int max_cores = 0xFF;

#ifdef PERFD
static int max_cores_clustr_0 = -1;
static int max_cores_clustr_1 = -1;
#endif


#define KPM_CPU_MIN_FREQ_NODE       "/sys/module/msm_performance/parameters/cpu_min_freq"
#define KPM_CPU_MAX_FREQ_NODE       "/sys/module/msm_performance/parameters/cpu_max_freq"
#define KPM_NUM_CLUSTERS        "/sys/module/msm_performance/parameters/num_clusters"
#define KPM_MAX_CPUS            "/sys/module/msm_performance/parameters/max_cpus"
#define KPM_MANAGED_CPUS        "/sys/module/msm_performance/parameters/managed_cpus"
#define NODE_MAX                150
#define SYSFS_PREFIX            "/sys/devices/system/"
#define CPU_SYSFS_PREFIX            (SYSFS_PREFIX"cpu/cpu0/cpufreq/interactive/")
#define AVL_FREQ_NODE          (SYSFS_PREFIX"cpu/cpu%d/cpufreq/scaling_available_frequencies")
#define CPUINFO_FREQ_NODE      (SYSFS_PREFIX"cpu/cpu%d/cpufreq/cpuinfo_%s_freq")
#define ONDEMAND_PARAMETER_NODE(param) (SYSFS_PREFIX"cpu/cpufreq/ondemand/"#param)
#define FREQLIST_MAX            32
#define KSM_PREFIX              "/sys/kernel/mm/ksm/"
#define KTM_ENABLE_NODE         "/sys/module/msm_thermal/core_control/enabled"
#define PM_QOS                "/sys/module/lpm_levels/parameters/sleep_disabled"
#define KPM_MAX_CPUS            "/sys/module/msm_performance/parameters/max_cpus"
#define KPM_MANAGED_CPUS        "/sys/module/msm_performance/parameters/managed_cpus"
#define KPM_MANAGED_CLUSTR_0    "0,1,2,3"
#define KPM_MANAGED_CLUSTR_1    "4,5,6,7"
#define KPM_MANAGED_CORES       "1,2,3,4,5,6,7"
#define SCHED_UPMIGRATE		"/proc/sys/kernel/sched_upmigrate"
#define SCHED_DOWNMIGRATE	"/proc/sys/kernel/sched_downmigrate"
#define SCHED_SMALL_TASK	"/proc/sys/kernel/sched_small_task"
#define SCHED_MOSTLY_IDLE_LOAD	"/proc/sys/kernel/sched_mostly_idle_load"
#define SCHED_MOSTLY_IDLE_NR_RUN"/proc/sys/kernel/sched_mostly_idle_nr_run"
#define SCHED_INIT_TASK_LOAD	"/proc/sys/kernel/sched_init_task_load"
#define SCHED_PREFER_IDLE_NODE  "/proc/sys/kernel/sched_prefer_idle"
#define SCHED_MIGRATE_COST_NODE "/proc/sys/kernel/sched_migration_cost_ns"
#define CORE_CTL_MIN_CPU	(SYSFS_PREFIX"cpu/cpu%d/core_ctl/min_cpus")
#define CORE_CTL_MAX_CPU	(SYSFS_PREFIX"cpu/cpu%d/core_ctl/max_cpus")

static char ksm_run_node[NODE_MAX] = { KSM_PREFIX };
static char ksm_param_sleeptime[NODE_MAX] = { KSM_PREFIX };
static char ksm_param_pages_to_scan[NODE_MAX] = { KSM_PREFIX };
static char ksm_sleep_millisecs[PROPERTY_VALUE_MAX] = "";
static char ksm_pages_to_scan[PROPERTY_VALUE_MAX] = "" ;
static int is_ksm_supported = -1;
static int PERF_SYSTRACE;
static char cores_online_prop[PROPERTY_VALUE_MAX];
static char trace_prop[PROPERTY_VALUE_MAX];
static char prop_name[NODE_MAX];
static char cpup[NODE_MAX] = { SYSFS_PREFIX }; //cpu present node
static char c0o[NODE_MAX] = { SYSFS_PREFIX };  // cpu0 online node
static char c1o[NODE_MAX] = { SYSFS_PREFIX };  // cpu1 online node
static char c2o[NODE_MAX] = { SYSFS_PREFIX };  // cpu2 online node
static char c3o[NODE_MAX] = { SYSFS_PREFIX };  // cpu3 online node
static char c4o[NODE_MAX] = { SYSFS_PREFIX };  // cpu4 online node
static char c5o[NODE_MAX] = { SYSFS_PREFIX };  // cpu5 online node
static char c6o[NODE_MAX] = { SYSFS_PREFIX };  // cpu6 online node
static char c7o[NODE_MAX] = { SYSFS_PREFIX };  // cpu7 online node
static char c00d[NODE_MAX];                    // cpu0 min freq default
static char c00dv[NODE_MAX];                   // cpu0 min freq default value
static char c00uv[NODE_MAX];
static char c01d[NODE_MAX];                    // cpu0 max freq default
static char c01dv[NODE_MAX];                   // cpu0 max freq default value
static char c00[NODE_MAX];                     // cpu0 min scaling freq node
static char c01[NODE_MAX];                     // cpu0 max scaling freq node
static char c10[NODE_MAX];                     // cpu1 min scaling freq node
static char c11[NODE_MAX];                     // cpu1 max scaling freq node
static char c20[NODE_MAX];                     // cpu2 min scaling freq node
static char c21[NODE_MAX];                     // cpu2 max scaling freq node
static char c30[NODE_MAX];                     // cpu3 min scaling freq node
static char c31[NODE_MAX];                     // cpu3 max scaling freq node
static char c40[NODE_MAX];                    // cpu4 min scaling freq node
static char c41[NODE_MAX];                     // cpu4 max scaling freq node
static char c50[NODE_MAX];                     // cpu5 min scaling freq node
static char c51[NODE_MAX];                    // cpu5 max scaling freq node
static char c60[NODE_MAX];                     // cpu6 min scaling freq node
static char c61[NODE_MAX];                     // cpu6 max scaling freq node
static char c70[NODE_MAX];                    // cpu7 min scaling freq node
static char c71[NODE_MAX];                    // cpu7 max scaling freq node
static char c0f[NODE_MAX];                     // cpu0 available freq
static char c4f[NODE_MAX];                    // cpu4 available freq
static char gp[16];                            // performance governor string
static char go[16];                            // ondemand governor string
static char gi[16];                            // interactive governor string
static char cps[NODE_MAX];                     // powersave_bias node
static char cib[NODE_MAX];                     // interactive boost node
static char cih[NODE_MAX];                     // interactive hispeed_freq node
static char g0[NODE_MAX];                      // cpu0 governor last setting
static char g1[NODE_MAX];                      // cpu1 governor last setting
static int  g0l     = -1;                      // g0 string length
static int  g1l     = -1;                      // g1 string length
static int  gihl    = -1;                      // hispeed freq length
static char cmf[NODE_MAX];                     // cpu min freq last setting
static int  cmfl    = -1;                      // cmf string length
static char gih[NODE_MAX];                     // high speed freq last setting
#define FREQLIST_STR            1024
static int  c0fL[FREQLIST_MAX];                // cpu0 list of available freq
static char c0fL_s[FREQLIST_STR];              // cpu0 list of available freq
static int  c0fL_n;                            // cpu0 #freq
static int  c4fL[FREQLIST_MAX];                // cpu4 list of available freq
static char c4fL_s[FREQLIST_STR];              // cpu4 list of available freq string
static int  c4fL_n;                            // cpu4 #freq
#define FREQMIN                 300000         // hardcoded minimum cpu freq
#define FREQMAX                9999999         // hardcoded maximum cpu freq
static char srate[NODE_MAX];                   // ondemand sampling rate storage
static int  sratel    = -1;                    // sampling rate string length
static char ondiob[NODE_MAX];                  // ondemand io_is_busy storage
static int  ondiobl = -1;                      // ondemand io_is_busy string length
static char ondsdf[NODE_MAX];                  // ondemand sampling_down_factor storage
static int  ondsdfl = -1;                      // ondemand sampling_down_factor string length
static char inttr[NODE_MAX];                   // interactive timer_rate storage
static int  inttrl  = -1;                      // interactive timer_rate string length
static char inthsf[NODE_MAX];                  // interactive hispeed_freq storage
static int  inthsfl = -1;                      // interactive hispeed_freq string length
static char intiob[NODE_MAX];                  // interactive io_is_busy storage
static int  intiobl = -1;                      // interactive io_is_busy string length
static char intghsl[NODE_MAX];                 // interactive go_hispeed_load storage
static int  intghsll = -1;                     // interactive go_hispeed_load string length
static char sf[NODE_MAX];                      // ondemand sync_freq storage
static int  sfl = -1;                          // ondemand sync_freq string length
static char of[NODE_MAX];                      // ondemand optimal_freq storage
static int  ofl = -1;                          // ondemand optimal_freq string length
static char sr[NODE_MAX];                      // sampling rate node string
static char oiib[NODE_MAX];                    // ondemand io is busy node string
static char osdf[NODE_MAX];                    // ondemand sampling down factor node string
static char ihl[NODE_MAX];                     // interactive hispeed load node string
static char itr[NODE_MAX];                     // interactive timer rate node string
static char iiib[NODE_MAX];                    // interactive io is busy node string
static char sfreq[NODE_MAX];                   // ondemand sync freq node string
static char ofreq[NODE_MAX];                   // ondemand optimal freq node string
static char sctm[NODE_MAX];                    // synchronize cores on foreground thread migration node
static char sctm_s[NODE_MAX];                  // synchronize cores on foreground thread migration node storage
static int  sctm_sl = -1;                      // synchronize cores on foreground thread migration node length
static char enbl_stp_up[NODE_MAX] = { ONDEMAND_PARAMETER_NODE(enable_stepup) }; //enable_stepupnode
static char enbl_stp_up_s[NODE_MAX];                  // enable_stepupnode storage
static int  enbl_stp_up_l = -1;                      // enable_stepupnode string length
static char max_int_steps[NODE_MAX] = { ONDEMAND_PARAMETER_NODE(max_intermediate_steps) }; //max_int_steps
static char max_int_steps_s[NODE_MAX];                  // max_int_steps storage
static int  max_int_steps_l = -1;                      // max_int_steps string length
static char schedb_n[NODE_MAX];                  // sched boost node string
static bool is_sched_boost_supported = false;
static bool is_sync_cores = true;               //Updating a single core freq, updates all cores freq.
static bool min_freq_prop_0_set = false;
static bool min_freq_prop_4_set = false;
static int  min_freq_prop_0_val = 0;
static int  min_freq_prop_4_val = 0;

static char iahd[NODE_MAX];                    // interactive pro above_hispeed_delay node string
static char ib[NODE_MAX];                      // interactive pro boost node
static char ibp[NODE_MAX];                     // interactive pro boostpulse node
static char ibpd[NODE_MAX];                    // interactive pro boostpulse_duration node
static char ighl[NODE_MAX];                    // interactive pro go_hispeed_load node
static char ihf[NODE_MAX];                     // interactive pro hispeed_freq node
static char iioib[NODE_MAX];                   // interactive pro io_is_busy node
static char imst[NODE_MAX];                    // interactive pro min_sample_time node
static char itl[NODE_MAX];                     // interactive pro target_loads node
static char itrate[NODE_MAX];                  // interactive pro timer_rate node
static char its[NODE_MAX];                     // interactive pro timer_slack node

static char ip_0_ahd_s[NODE_MAX];                 // interactive pro cpu 0 above_hispeed_delay storage
static int  ip_0_ahd_sl = -1;                     // interactive pro cpu 0 above_hispeed_delay string length
static char ip_0_b_s[NODE_MAX];                   // interactive pro cpu 0 boost storage
static int  ip_0_b_sl = -1;                       // interactive pro cpu 0 boost string length
static char ip_0_bpd_s[NODE_MAX];                 // interactive pro cpu 0 boostpulse_duration storage
static int  ip_0_bpd_sl = -1;                     // interactive pro cpu 0 boostpulse_duration string length
static char ip_0_ghl_s[NODE_MAX];                 // interactive pro cpu 0 go_hispeed_load storage
static int  ip_0_ghl_sl = -1;                     // interactive pro cpu 0 go_hispeed_load string length
static char ip_0_hf_s[NODE_MAX];                  // interactive pro cpu 0 hispeed_freq storage
static int  ip_0_hf_sl = -1;                      // interactive pro cpu 0 hispeed_freq string length
static char ip_0_ioib_s[NODE_MAX];                // interactive pro cpu 0 io_is_busy storage
static int  ip_0_ioib_sl = -1;                    // interactive pro cpu 0 io_is_busy string length
static char ip_0_mst_s[NODE_MAX];                 // interactive pro cpu 0 min_sample_time storage
static int  ip_0_mst_sl = -1;                     // interactive pro cpu 0 min_sample_time string length
static char ip_0_tl_s[NODE_MAX];                  // interactive pro cpu 0 target_loads storage
static int  ip_0_tl_sl = -1;                      // interactive pro cpu 0 target_loads string length
static char ip_0_tr_s[NODE_MAX];                  // interactive pro cpu 0 timer_rate storage
static int  ip_0_tr_sl = -1;                      // interactive pro cpu 0 timer_rate string length
static char ip_0_ts_s[NODE_MAX];                  // interactive pro cpu 0 timer_slack storage
static int  ip_0_ts_sl = -1;                      // interactive pro cpu 0 timer_slack string length

static char ip_4_ahd_s[NODE_MAX];                 // interactive pro cpu 4 above_hispeed_delay storage
static int  ip_4_ahd_sl = -1;                     // interactive pro cpu 4 above_hispeed_delay string length
static char ip_4_b_s[NODE_MAX];                   // interactive pro cpu 4 boost storage
static int  ip_4_b_sl = -1;                       // interactive pro cpu 4 boost string length
static char ip_4_bpd_s[NODE_MAX];                 // interactive pro cpu 4 boostpulse_duration storage
static int  ip_4_bpd_sl = -1;                     // interactive pro cpu 4 boostpulse_duration string length
static char ip_4_ghl_s[NODE_MAX];                 // interactive pro cpu 4 go_hispeed_load storage
static int  ip_4_ghl_sl = -1;                     // interactive pro cpu 4 go_hispeed_load string length
static char ip_4_hf_s[NODE_MAX];                  // interactive pro cpu 4 hispeed_freq storage
static int  ip_4_hf_sl = -1;                      // interactive pro cpu 4 hispeed_freq string length
static char ip_4_ioib_s[NODE_MAX];                // interactive pro cpu 4 io_is_busy storage
static int  ip_4_ioib_sl = -1;                    // interactive pro cpu 4 io_is_busy string length
static char ip_4_mst_s[NODE_MAX];                 // interactive pro cpu 4 min_sample_time storage
static int  ip_4_mst_sl = -1;                     // interactive pro cpu 4 min_sample_time string length
static char ip_4_tl_s[NODE_MAX];                  // interactive pro cpu 4 target_loads storage
static int  ip_4_tl_sl = -1;                      // interactive pro cpu 4 target_loads string length
static char ip_4_tr_s[NODE_MAX];                  // interactive pro cpu 4 timer_rate storage
static int  ip_4_tr_sl = -1;                      // interactive pro cpu 4 timer_rate string length
static char ip_4_ts_s[NODE_MAX];                  // interactive pro cpu 4 timer_slack storage
static int  ip_4_ts_sl = -1;                      // interactive pro cpu 4 timer_slack string length

/* support for sched params */

static char sch_upmig_s[NODE_MAX];                // sched upmigrate storage
static int  sch_upmig_sl = -1;                    // sched upmigrate string length

static char sch_dwnmig_s[NODE_MAX];               // sched downmigrate storage
static int  sch_dwnmig_sl = -1;                   // sched downmigrate string length

static char sch_sml_tsk_s[NODE_MAX];              // sched small task storage
static int  sch_sml_tsk_sl = -1;                  // sched small task string length

static char sch_mst_idl_ld_s[NODE_MAX];           // sched mostly idle load storage
static int  sch_mst_idl_ld_sl = -1;               // sched mostly idle load string length

static char sch_mst_idl_nr_rn_s[NODE_MAX];        // sched mostly idle nr run storage
static int  sch_mst_idl_nr_rn_sl = -1;            // sched mostly idle nr run string length

static char sch_init_tsk_ld_s[NODE_MAX];          // sched init task load storage
static int  sch_init_tsk_ld_sl = -1;              // sched init task load string length

static char schedpi[NODE_MAX];                 // sched_prefer_idle node storage
static int schedpil;                           // sched_prefer_idle node storage length
static char schedmc[NODE_MAX];                 //sched_migrate_cost node storage
static int schedmcl;                           // shced_migrate_cost node storage length

static char core_ctl_min_cpu[NODE_MAX];        // core control min_cpu storage
static char core_ctl_max_cpu[NODE_MAX];        // core control max_cpu storage

static char core_ctl_min_cpu_node[NODE_MAX];   //Actual path of min_cpu node based on target
static char core_ctl_max_cpu_node[NODE_MAX];   //Actual path of max_cpu node based on target

/* This array tells us information about each CPU freq resource such as:
 *     1. which ideal CPU to use
 *     1. which CPU is mapped
 */
static struct cpu_freq_resource cpu_freq_state[CPU_FREQ_RESOURCES_MAX];

/* This array tells us information about each CPU such as:
 *     1. what is the sysfs for min and max freq
 *     2. is there a mapping to it
 */
static struct cpu_freq_mapping cpu_freq_map[] = {
    {c00, c01, c0o},
    {c10, c11, c1o},
    {c20, c21, c2o},
    {c30, c31, c3o},
    {c40, c41, c4o},
    {c50, c51, c5o},
    {c60, c61, c6o},
    {c70, c71, c7o},
};
/* This array will be used to store max/min frequency
 * for each cpu.
 */
static struct cpu_freq_resource_value cpu_freq_val[MAX_CPUS];

#define FREAD_STR(fn, pstr, len, rc)    { int fd;                        \
                                          rc = -1;                       \
                                          fd = open(fn, O_RDONLY);       \
                                          if (fd >= 0) {                 \
                                              rc = read(fd, pstr, len);  \
                                              pstr[len-1] = '\0';        \
                                              close(fd); \
                                          } \
                                        }
#define FWRITE_STR(fn, pstr, len, rc)   { int fd;                        \
                                          rc = -1;                       \
                                          fd = open(fn, O_WRONLY);       \
                                          if (fd >= 0) {                 \
                                              rc = write(fd, pstr, len); \
                                              close(fd);                 \
                                          }                              \
                                        }
#define FOVRWT_STR(fn, pstr, len, rc)   { int fd;                        \
                                          rc = -1;                       \
                                          fd = open(fn, O_CREAT|O_WRONLY, S_IRUSR|S_IWUSR); \
                                          if (fd >= 0) {                 \
                                              rc = write(fd, pstr, len); \
                                              fsync(fd);                 \
                                              close(fd);                 \
                                          }                              \
                                        }

/* Function used to populate an array to keep track of core status
 * Warning: the array will be flushed on each call
 * Returns: the number of cores online or -1 on error
 */
inline int get_online_cpus (int * status_array, int array_sz) {
    int start = 0;
    int end = 0;
    int range = 0;
    int i = 0;
    int j = 0;
    int online_count = 0;
    char online[CO_NODE_MAX_LEN] = {0};
    int len = 0;
    int rc = 0;

    FREAD_STR(co_node, online, CO_NODE_MAX_LEN, rc);

    len = strlen(online);
    if (len < 1 || len > CO_NODE_MAX_LEN) {
        return -1;
    }

    memset(status_array, 0, sizeof(int)*array_sz);

    for (j = 0; j < len; j++) {
        if (isdigit(online[j])) {
            if (range) {
                end = online[j] - '0';
                for (i = start+1; i < end+1; i++) {
                    status_array[i] = 1;
                    online_count++;
                }
                range = 0;
            } else {
                start = online[j] - '0';
                status_array[start] = 1;
                online_count++;
            }
        }
        else if (online[j] == '-') {
           range = 1;
        }
    }
    return online_count;
}

static inline void write_to_file(FILE *output, char *input_node, char *input_val) {
    char buf[NODE_MAX];
    char tmp_s[NODE_MAX];
    int len = 0;
    int rc = 0;

    if (output == NULL)
        return;

    memset(buf, 0, NODE_MAX);
    FREAD_STR(input_val, buf, NODE_MAX, rc);
    if (rc >= 0) {
        snprintf(tmp_s, NODE_MAX, "%s:%s", input_node, buf);
        len = strlen(tmp_s);
        fwrite(tmp_s, sizeof(char), len, output);
        return;
    }
    QLOGW("Unable to write node value %s to default values file", input_node);
}

static inline void write_to_node(FILE *read_file, char *node_str, char *node) {
    char line[NODE_MAX];
    char *delimit = ":";
    char *token = NULL;
    int rc = 0;

    if (read_file == NULL)
        return;

    rewind(read_file);

    while (fgets(line, sizeof(line), read_file) != NULL) {
        if (strstr(line, node_str)) {
            token = strtok(line, delimit);
            token = strtok(NULL, delimit);
            if (token != NULL)
                FWRITE_STR(node, token, strlen(token), rc);
            return;
        }
    }
    QLOGW("Unable to read node value %s from default values file", node_str);
}
#ifdef PERFD
static void reset_cores_status() {
    char tmp_s[NODE_MAX];
    int rc = FAILED;

    if (core_ctl_present > 0) {
        if (property_get("ro.core_ctl_max_cpu", tmp_s, NULL) > 0) {
            FWRITE_STR(core_ctl_max_cpu_node, tmp_s, strlen(tmp_s), rc);
            if (rc < 1) {
                QLOGE("Warning: core_ctl_max_cpu not updated, write failed");
            } else
                QLOGI("Updating %s with %s and return value %d", core_ctl_max_cpu_node, tmp_s, rc);
        } else
            QLOGE("Warning: core_ctl_max_cpu property not defined, can not reset");

        if (property_get("ro.core_ctl_min_cpu", tmp_s, NULL) > 0) {
            FWRITE_STR(core_ctl_min_cpu_node, tmp_s, strlen(tmp_s), rc);
            if (rc < 1) {
                QLOGE("Warning: core_ctl_min_cpu not updated, write failed");
            } else
                QLOGI("Updating %s with %s and return value %d", core_ctl_min_cpu_node, tmp_s, rc);
        } else
            QLOGE("Warning: core_ctl_min_cpu property not defined, can not reset");
    }
    else if (kpm_hotplug_support > 0){
        snprintf(tmp_s, NODE_MAX, "%d:%d", max_cores_clustr_0, max_cores_clustr_1);
        FWRITE_STR(KPM_MAX_CPUS, tmp_s, strlen(tmp_s), rc);
        if (rc > 0) {
            QLOGE("Reset cores success");
        } else {
            QLOGE("Could not update the %s node \n", KPM_MANAGED_CPUS);
        }
    }
    else
        QLOGE("Error: KPM hoplug and core control both are not present, can not reset");
}
#endif

static void reset_freq_to_default() {
    int i = 0;
    int rc = -1;
    char maxf[NODE_MAX], minf[NODE_MAX];
    char freqnode[NODE_MAX];
#ifdef PERFD
    char tmp_s[NODE_MAX];
    int reqval = 0;
    snprintf(tmp_s, NODE_MAX, "0:%d 1:%d 2:%d 3:%d", reqval, reqval, reqval, reqval);
    QLOGI("reset_freq_to_default reset min freq req for CPUs 0-3: %s", tmp_s);
    FWRITE_STR(KPM_CPU_MIN_FREQ_NODE, tmp_s, strlen(tmp_s), rc);
    snprintf(tmp_s, NODE_MAX, "0:%u 1:%u 2:%u 3:%u", UINT_MAX, UINT_MAX, UINT_MAX, UINT_MAX);
    QLOGI("reset_freq_to_default reset max freq req for CPUs 0-3: %s", tmp_s);
    FWRITE_STR(KPM_CPU_MAX_FREQ_NODE, tmp_s, strlen(tmp_s), rc);
    if (max_num_cpu_clustr_1 > 0) {
       snprintf(tmp_s, NODE_MAX, "4:%d 5:%d 6:%d 7:%d", reqval, reqval, reqval, reqval);
       QLOGI("reset_freq_to_default reset min freq req for CPUs 4-7: %s", tmp_s);
       FWRITE_STR(KPM_CPU_MIN_FREQ_NODE, tmp_s, strlen(tmp_s), rc);
       snprintf(tmp_s, NODE_MAX, "4:%u 5:%u 6:%u 7:%u", UINT_MAX, UINT_MAX, UINT_MAX, UINT_MAX);
       QLOGI("reset_freq_to_default reset max freq req for CPUs 4-7: %s", tmp_s);
       FWRITE_STR(KPM_CPU_MAX_FREQ_NODE, tmp_s, strlen(tmp_s), rc);
    }
#else
    for ( i = 0; i < max_num_cpu_supt; i++) {
        maxf[0] = '\0';
        minf[0] = '\0';
        //read the max_freq from cpuinfo node.
        sprintf(freqnode, CPUINFO_FREQ_NODE, i,"max");
        FREAD_STR(freqnode, maxf, NODE_MAX, rc);
        if (rc > 0)
           maxf[rc-1] = '\0';
        else
           QLOGW("Could not read %s ", freqnode);
        //read the min_freq from cpuinfo node or property
        if (i < max_num_cpu_clustr_0) {
           if (min_freq_prop_0_set)
              snprintf(minf, NODE_MAX,"%d", min_freq_prop_0_val);
           else {
              sprintf(freqnode, CPUINFO_FREQ_NODE, i,"min");
              FREAD_STR(freqnode, minf, NODE_MAX, rc);
              if (rc > 0)
                 minf[rc-1] = '\0';
              else
                 QLOGW("Could not read %s ", freqnode);
            }
        } else if ((i > (max_num_cpu_clustr_0-1)) && (i <= max_num_cpu_clustr_1)) {
           if (min_freq_prop_4_set)
              snprintf(minf, NODE_MAX,"%d", min_freq_prop_4_val);
           else {
              sprintf(freqnode, CPUINFO_FREQ_NODE, i,"min");
              FREAD_STR(freqnode, minf, NODE_MAX, rc);
              if(rc > 0)
                 minf[rc-1] = '\0';
              else
                 QLOGW("Could not read %s ", freqnode);
            }
        }
        //Update the max node first.
        if (strlen(maxf) > 0) {
           FWRITE_STR(cpu_freq_map[i].max, maxf, strlen(maxf), rc);
           if (rc <=0)
              QLOGW("Initialization of %s with %s failed", cpu_freq_map[i].max, maxf);
        } else {
            QLOGW("Initialization of %s failed as freq invalid ", cpu_freq_map[i].max);
        }
        if (strlen(minf) > 0) {
           FWRITE_STR(cpu_freq_map[i].min, minf, strlen(minf), rc);
           if (rc <=0)
               QLOGW("Initialization of %s with %s failed", cpu_freq_map[i].min, minf);
        } else {
            QLOGW("Initialization of %s failed as freq invalid ", cpu_freq_map[i].min);
        }
  }//end of for
#endif
}//end of function

#ifdef PERFD
static void reset_pc() {
    char tmp_s[NODE_MAX];
    int rc = FAILED;
    snprintf(tmp_s, NODE_MAX, "%d", ENABLE_PC_LATENCY);
    FWRITE_STR(PM_QOS, tmp_s, strlen(tmp_s), rc);
}

static void reset_sched_boost() {
    char tmp_s[NODE_MAX];
    int rc = FAILED;
    snprintf(tmp_s, NODE_MAX, "%d", RESET_SCHED_BOOST);
    FWRITE_STR(schedb_n, tmp_s, strlen(tmp_s), rc);
}
#endif

static inline void reset_to_default_values() {
    FILE *defval;
    int rc = 0;

    defval = fopen("/data/system/perfd/default_values", "a+");
    if (defval == NULL) {
        QLOGE("Cannot open/create default values file");
        return;
    }

    fseek (defval, 0, SEEK_END);

    if (ftell(defval) == 0) {
        write_to_file(defval, "sr", sr);
        write_to_file(defval, "oiib", oiib);
        write_to_file(defval, "osdf", osdf);
        write_to_file(defval, "sfreq", sfreq);
        write_to_file(defval, "ofreq", ofreq);
        write_to_file(defval, "iiib", iiib);
        write_to_file(defval, "itr", itr);
        write_to_file(defval, "ihl", ihl);
        write_to_file(defval, "sctm", sctm);
        write_to_file(defval, "ksm_run_node", ksm_run_node);
        write_to_file(defval, "ksm_param_sleeptime", ksm_param_sleeptime);
        write_to_file(defval, "ksm_param_pages_to_scan", ksm_param_pages_to_scan);
        write_to_file(defval, "cib", cib);
        write_to_file(defval, "cih", cih);
    } else {
        reset_freq_to_default();
#ifdef PERFD
        reset_cores_status();
#endif
        write_to_node(defval, "sr", sr);
        write_to_node(defval, "oiib", oiib);
        write_to_node(defval, "osdf", osdf);
        write_to_node(defval, "sfreq", sfreq);
        write_to_node(defval, "ofreq", ofreq);
        write_to_node(defval, "iiib", iiib);
        write_to_node(defval, "itr", itr);
        write_to_node(defval, "ihl", ihl);
        write_to_node(defval, "sctm", sctm);
        write_to_node(defval, "ksm_run_node", ksm_run_node);
        write_to_file(defval, "ksm_param_sleeptime", ksm_param_sleeptime);
        write_to_file(defval, "ksm_param_pages_to_scan", ksm_param_pages_to_scan);
        write_to_node(defval, "cib", cib);
        write_to_node(defval, "cih", cih);
#ifdef PERFD
        reset_pc();
        reset_sched_boost();
#endif
    }
    fclose(defval);
}

/*
 * return number of available cores
 */
static inline int nocs(){
    static int nocs = 0;
    char b[100];
    int rc;
    if( nocs == 0 ){
        QLOGV("cpu/present file %s", cpup);
        FREAD_STR(cpup, b, 100, rc);
        if (rc > 2 && rc < 101) {
            QLOGV("cpu/present %s", b);
            //According to above spec, since we are not going to have
            //cores in the middle "not presented", so it will be just
            //last char +1
            nocs = b[rc - 2] - '0' + 1;
        } else {
            nocs = 1;
        }
    }
    return nocs;
}

/* toggle KSM
   0 -- KSM won't run
   1 -- KSM will run */
static int toggle_ksm_run(int run)
{
    int rc=0;
    if(is_ksm_supported == 0)
    {
        if(run == 0)
        {
            /* Disable KSM */
            FWRITE_STR(ksm_run_node, "0", 1, rc);
        }
        else if(run == 1)
        {
            /* Enable KSM */
            FWRITE_STR(ksm_run_node, "1", 1, rc);
        }
    }
    return rc;
}

/* Enable/disable power collapse on all CPUS for display off */
static int toggle_screen_power_collapse(uint16_t opt_data)
{
    int level = opt_data & 0xFF;

#ifdef PERFD
    return FAILED;
#else
    return mpdecision_toggle_power_collapse(level);
#endif
}
#define RELEASE_LOCK     0
/* Enable/disable power collapse on all CPUS */
static int toggle_power_collapse(uint16_t opt_data)
{
    int level = opt_data & 0xFF;
    char tmp_s[NODE_MAX];
    int rc = FAILED;

#ifdef PERFD
    if (level == RELEASE_LOCK) {
       snprintf(tmp_s, NODE_MAX, "%d", ENABLE_PC_LATENCY);
       FWRITE_STR(PM_QOS, tmp_s, strlen(tmp_s), rc);
       QLOGE("writing %s with %s and rc is %d", PM_QOS, tmp_s, rc);
       if (rc < 0)
          QLOGE("Releasing the power collapse perflock failed\n");
    } else {
       snprintf(tmp_s, NODE_MAX, "%d", DISABLE_PC_LATENCY);
       FWRITE_STR(PM_QOS, tmp_s, strlen(tmp_s), rc);
       QLOGE("writing %s with %s and rc is %d ", PM_QOS, tmp_s, rc);
       if (rc < 0)
          QLOGE("Acquiring the power collapse perflock failed\n");
    }
    return rc;
#else
    return mpdecision_toggle_power_collapse(level);
#endif
}

static int check_core_ctl_presence()
{
    int rc = -1;
    if (core_ctl_cpu == -1) {
        QLOGE("Error: core_ctl_cpu is not intilazed for this target");
        return -1;
    } else {
        snprintf(core_ctl_min_cpu_node,NODE_MAX,CORE_CTL_MIN_CPU,core_ctl_cpu);
        snprintf(core_ctl_max_cpu_node,NODE_MAX,CORE_CTL_MAX_CPU,core_ctl_cpu);
        FREAD_STR(core_ctl_min_cpu_node, core_ctl_min_cpu, NODE_MAX, rc);
        if (rc > 0) {
            core_ctl_present = 1;
        } else
            core_ctl_present = 0;
        return 1;
    }
}

static int core_ctl_init()
{
    int rc = -1;
    if(core_ctl_present > 0) {
       /* update min_cpu and max_cpus node */
       FREAD_STR(core_ctl_max_cpu_node, core_ctl_max_cpu, NODE_MAX, rc);
       if (rc > 0) {
           QLOGI("%s read with %s return value %d", core_ctl_max_cpu_node, core_ctl_max_cpu, rc);
           max_cores = atoi(core_ctl_max_cpu);
       } else {
           QLOGE("Unable to read %s", core_ctl_max_cpu_node);
       }

       FREAD_STR(core_ctl_min_cpu_node, core_ctl_min_cpu, NODE_MAX, rc);
       if (rc > 0) {
           QLOGI("%s read with %s return value %d", core_ctl_min_cpu_node, core_ctl_min_cpu, rc);
           min_cores = atoi(core_ctl_min_cpu);
       } else
           QLOGE("Unable to read %s", core_ctl_min_cpu_node);
       return 1;
    } else {
       QLOGE("Error: Core ctl not present");
       return -1;
    }
}

static int lock_min_cores(uint16_t opt_data)
{
    int reqval = (opt_data & 0xff);

#ifdef PERFD
    if (core_ctl_present > 0) {
        int rc = -1;
        int reqtype = (opt_data & 0xff00) >> 8;
        char tmp_s[NODE_MAX];
        if ((core_ctl_cpu == 0 && reqtype == CLUSTR_1_CPUS_ON) ||
           (core_ctl_cpu == 4 && reqtype == CLUSTR_0_CPUS_ON)) {
            QLOGE("Warning: Core control support not present for resource 0x%x on this target",reqtype);
            return FAILED;
        }

        if (reqval == 0x77 || reqval == RELEASE_LOCK) {
            min_cores = atoi(core_ctl_min_cpu);
            FWRITE_STR(core_ctl_min_cpu_node, core_ctl_min_cpu, strlen(core_ctl_min_cpu), rc);
            QLOGI("Updating %s with %s ", core_ctl_min_cpu_node, core_ctl_min_cpu);
            return rc;
        }

        if (reqval < 1) {
            return FAILED;
        }

        min_cores = reqval;
        if (CLUSTR_0_CPUS_ON == reqtype && min_cores > cpu_count_clustr_0) {
            min_cores = cpu_count_clustr_0;
        }
        if (CLUSTR_1_CPUS_ON == reqtype && min_cores > cpu_count_clustr_1) {
            min_cores = cpu_count_clustr_1;
        }

        if (min_cores > max_cores) {
            min_cores = max_cores;
        }
        snprintf(tmp_s, NODE_MAX, "%d", min_cores);
        FWRITE_STR(core_ctl_min_cpu_node, tmp_s, strlen(tmp_s), rc);
        QLOGI("Updating %s with %s ", core_ctl_min_cpu_node, tmp_s);
        return rc;
    }
    else
       return FAILED;
#else
    if (reqval == 0x77 || reqval == RELEASE_LOCK) {
        min_cores = 0;
        return mpdecision_lock_min_cores(min_cores);
    }

    if (reqval < 2) {
        return FAILED;
    }

    min_cores = reqval;

    if (min_cores > nocs()) {
        min_cores = nocs();
    }

    if (min_cores > max_cores) {
        min_cores = max_cores;
    }

    return mpdecision_lock_min_cores(min_cores);
#endif
}

static int lock_max_cores(uint16_t opt_data)
{
#ifdef PERFD
    char tmp_s[NODE_MAX];
    if (core_ctl_present > 0) {
       int rc = -1;
       int reqval = (opt_data & 0xf);
       int reqtype = (opt_data & 0xff00) >> 8;

        if ((core_ctl_cpu == 0 && reqtype == CLUSTR_1_MAX_CORES) ||
           (core_ctl_cpu == 4 && reqtype == CLUSTR_0_MAX_CORES)) {
            QLOGE("Warning: Core control support not present for resource 0x%x on this target",reqtype);
            return FAILED;
        }

       if (reqval == RELEASE_LOCK) {
           /* Update max_core first otherwise \
              min_core wiil not update */
           max_cores = atoi(core_ctl_max_cpu);
           FWRITE_STR(core_ctl_max_cpu_node, core_ctl_max_cpu, strlen(core_ctl_max_cpu), rc);
           QLOGI("Updating %s with %s ", core_ctl_max_cpu_node, core_ctl_max_cpu);
           if (min_cores > 0) {
               snprintf(tmp_s, NODE_MAX, "%d", min_cores);
               FWRITE_STR(core_ctl_min_cpu_node, tmp_s, strlen(tmp_s), rc);
               QLOGI("Updating %s with %s ", core_ctl_min_cpu_node, tmp_s);
           }
           return rc;
       }

       if ((0xF-reqval) < 1) {
            return FAILED;
       }

       max_cores = 0xF-reqval;
       snprintf(tmp_s, NODE_MAX, "%d", max_cores);
       FWRITE_STR(core_ctl_max_cpu_node, tmp_s, strlen(tmp_s), rc);
       QLOGI("Updating %s with %s ", core_ctl_max_cpu_node, tmp_s);
       return rc;
    }
    else if (kpm_hotplug_support > 0) {
       int rc = FAILED;
       int reqval = (opt_data & 0xff);
       int reqtype = (opt_data & 0xff00) >> 8;

       if (CLUSTR_1_MAX_CORES == reqtype && 0 == max_num_cpu_clustr_1) {
           QLOGW("Warning: As cluster 1 does not exist resource id 0x%X not supported", reqtype);
           return FAILED;
       }

       if (reqval == RELEASE_LOCK) {
           if (reqtype == CLUSTR_0_MAX_CORES) {
               max_cores_clustr_0 = -1;
           } else if (reqtype == CLUSTR_1_MAX_CORES ) {
               max_cores_clustr_1 = -1;
           }
       } else {
           max_cores = 0xFF-reqval;
           if (reqtype == CLUSTR_0_MAX_CORES) {
               if (max_cores > cpu_count_clustr_0) {
                   return FAILED;
               }
               max_cores_clustr_0 = max_cores;
           } else if (reqtype == CLUSTR_1_MAX_CORES ) {
               if (max_cores > cpu_count_clustr_1) {
                   return FAILED;
               }
               max_cores_clustr_1 = max_cores;
           }
       }
       snprintf(tmp_s, NODE_MAX, "%d:%d", max_cores_clustr_0, max_cores_clustr_1);
       QLOGE("Write %s into %s", tmp_s, KPM_MAX_CPUS);
       FWRITE_STR(KPM_MAX_CPUS, tmp_s, strlen(tmp_s), rc);
       return rc;
    }
    else
       return FAILED;
#else
    int reqval = (opt_data & 0xf);

    if (reqval == RELEASE_LOCK) {
        if (min_cores > 0) {
            mpdecision_lock_min_cores(min_cores);
        }
        max_cores = 0xFF;
        return mpdecision_lock_max_cores(max_cores);
    }

    max_cores = 0xF-reqval;
    return mpdecision_lock_max_cores(max_cores);
#endif
}

static int sched_prefer_idle(uint16_t data)
{
    int rc = 0;
    char tmp_s[NODE_MAX];
    static unsigned int stored_sched_prefer_idle = 0;
    int reqval = (data & 0xff);

    if (reqval == 0) {
        if (schedpil > 0) {
           FWRITE_STR(SCHED_PREFER_IDLE_NODE, schedpi, schedpil, rc);
           stored_sched_prefer_idle = 0;
           QLOGI("perf_lock_rel: updated %s with %s return value %d", SCHED_PREFER_IDLE_NODE, schedpi, rc);
        }
        return rc;
    }
    if (!stored_sched_prefer_idle) {
        FREAD_STR(SCHED_PREFER_IDLE_NODE, schedpi, NODE_MAX, rc);
        if (rc > 0) {
           QLOGI("%s read with %s return value %d", SCHED_PREFER_IDLE_NODE, schedpi, rc);
           schedpil = strlen(schedpi);
           stored_sched_prefer_idle = 1;
        }
       else {
            QLOGE("Failed to read %s ", SCHED_PREFER_IDLE_NODE);
       }
    }

    snprintf(tmp_s, NODE_MAX, "%d", ENABLE_PREFER_IDLE);
    FWRITE_STR(SCHED_PREFER_IDLE_NODE, tmp_s, strlen(tmp_s), rc);
    QLOGI("perf_lock_acq updated %s with %s return value %d", SCHED_PREFER_IDLE_NODE, tmp_s, rc);
    return rc;
}

static int sched_migrate_cost(uint16_t data)
{
    int rc = 0;
    char tmp_s[NODE_MAX];
    static unsigned int stored_sched_migrate_cost = 0;
    int reqval = (data & 0xff);
    if (reqval == 0) {
        if (schedmcl > 0) {
            FWRITE_STR(SCHED_MIGRATE_COST_NODE, schedmc, schedmcl, rc);
            stored_sched_migrate_cost = 0;
            QLOGI("perf_lock_rel: updated %s with %s return value %d", SCHED_MIGRATE_COST_NODE, schedmc, rc);
        }
        return rc;
    }
    if (!stored_sched_migrate_cost) {
        FREAD_STR(SCHED_MIGRATE_COST_NODE, schedmc, NODE_MAX, rc);
        if (rc > 0) {
            QLOGI("%s read with %s return value %d", SCHED_MIGRATE_COST_NODE, schedmc, rc);
            schedmcl = strlen(schedmc);
            stored_sched_migrate_cost = 1;
        }
        else {
             QLOGE("Failed to read %s ", SCHED_MIGRATE_COST_NODE);
        }
    }

    snprintf(tmp_s, NODE_MAX, "%d", WRITE_MIGRATE_COST);
    FWRITE_STR(SCHED_MIGRATE_COST_NODE, tmp_s, strlen(tmp_s), rc);
    QLOGI("perf_lock_acq updated %s with %s return value %d", SCHED_MIGRATE_COST_NODE, tmp_s, rc);
    return rc;
}


/* Set the sampling rate */
static int sampling_rate(uint16_t data)
{
    int rc = 0;
    char tmp_s[NODE_MAX];
    static unsigned int stored_sampling_rate = 0;
    int reqval = 0xff - (data & 0xff);

    if (reqval == 0xff) {
        if (sratel > 0) {
            FWRITE_STR(sr, srate, sratel, rc);
            stored_sampling_rate = 0;
        }
        return rc;
    }

    if (!stored_sampling_rate) {
        FREAD_STR(sr, srate, NODE_MAX, rc);
        if (rc >= 0) {
            sratel = strlen(srate);
        }
        stored_sampling_rate = 1;
    }
    reqval *= 10000;

    snprintf(tmp_s, NODE_MAX, "%d", reqval);
    FWRITE_STR(sr, tmp_s, strlen(tmp_s), rc);
    return rc;
}

/* Set io_is_busy */
static int ondemand_io_is_busy(uint16_t data)
{
    int rc = 0;
    char tmp_s[NODE_MAX];
    static unsigned int stored_ondemand_io_is_busy = 0;
    int reqval = (data & 0xff);

    if (reqval == 0xff) {
        if (ondiobl > 0) {
            FWRITE_STR(oiib, ondiob, ondiobl, rc);
            stored_ondemand_io_is_busy = 0;
        }
        return rc;
    }

    if (!stored_ondemand_io_is_busy) {
        FREAD_STR(oiib, ondiob, NODE_MAX, rc);
        if (rc >= 0) {
            ondiobl = strlen(ondiob);
        }
        stored_ondemand_io_is_busy = 1;
    }

    reqval = !!reqval; /* 0 or 1 */

    snprintf(tmp_s, NODE_MAX, "%d", reqval);
    FWRITE_STR(oiib, tmp_s, strlen(tmp_s), rc);
    return rc;
}

/* Set sampling_down_factor */
static int ondemand_sampling_down_factor(uint16_t data)
{
    int rc = 0;
    char tmp_s[NODE_MAX];
    static unsigned int stored_ondemand_sampling_down_factor = 0;
    int reqval = (data & 0xff);

    if (reqval == 0) {
        if (ondsdfl > 0) {
            FWRITE_STR(osdf, ondsdf, ondsdfl, rc);
            stored_ondemand_sampling_down_factor = 0;
        }
        return rc;
    }

    if (!stored_ondemand_sampling_down_factor) {
        FREAD_STR(osdf, ondsdf, NODE_MAX, rc);
        if (rc >= 0) {
            ondsdfl = strlen(ondsdf);
        }
        stored_ondemand_sampling_down_factor = 1;
    }

    snprintf(tmp_s, NODE_MAX, "%d", reqval);
    FWRITE_STR(osdf, tmp_s, strlen(tmp_s), rc);
    return rc;
}

/* Set go_hispeed_load */
static int interactive_hispeed_load(uint16_t data)
{
    int rc = 0;
    char tmp_s[NODE_MAX];
    static unsigned int stored_interactive_hispeed_load = 0;
    int reqval = (data & 0xff);

    if (reqval == 0) {
        if (intghsll > 0) {
            FWRITE_STR(ihl, intghsl, intghsll, rc);
            stored_interactive_hispeed_load = 0;
        }
        return rc;
    }

    if (!stored_interactive_hispeed_load) {
        FREAD_STR(ihl, intghsl, NODE_MAX, rc);
        if (rc >= 0) {
            intghsll = strlen(intghsl);
        }
        stored_interactive_hispeed_load = 1;
    }

    snprintf(tmp_s, NODE_MAX, "%d", reqval);
    FWRITE_STR(ihl, tmp_s, strlen(tmp_s), rc);
    return rc;
}

/* Set the timer rate. */
static int interactive_timer_rate(uint16_t data)
{
    int rc = 0;
    char tmp_s[NODE_MAX];
    static unsigned int stored_interactive_timer_rate = 0;
    int reqval = 0xff - (data & 0xff);

    if (reqval == 0xff) {
        if (inttrl > 0) {
            FWRITE_STR(itr, inttr, inttrl, rc);
            stored_interactive_timer_rate = 0;
        }
        return rc;
    }

    if (!stored_interactive_timer_rate) {
        FREAD_STR(itr, inttr, NODE_MAX, rc);
        if (rc >= 0) {
            inttrl = strlen(inttr);
        }
        stored_interactive_timer_rate = 1;
    }

    reqval *= 10000;

    snprintf(tmp_s, NODE_MAX, "%d", reqval);
    FWRITE_STR(itr, tmp_s, strlen(tmp_s), rc);
    return rc;
}

static int interactive_hispeed_freq(uint16_t data)
{
    int rc = 0;
    int i = 0;
    char tmp_s[NODE_MAX];
    int reqval = (data & 0xff);
    static unsigned int stored_interactive_hispeed_freq = 0;

    if (reqval == 0) {
        if (inthsfl > 0) {
            FWRITE_STR(cih, inthsf, inthsfl, rc);
            stored_interactive_hispeed_freq = 0;
        }
        return rc;
    }

    if (reqval > 0) {
        reqval *= 100000;
        for (i = 0; i < c0fL_n; i++) {
            if (c0fL[i] >= reqval) {
                reqval = c0fL[i];
                break;
            }
        }
        if (i == c0fL_n)
            reqval = c0fL[i - 1];
    }

    if (!stored_interactive_hispeed_freq) {
        FREAD_STR(cih, inthsf,
            NODE_MAX, rc);
        if (rc >= 0) {
            inthsfl = strlen(inthsf);
        }
        stored_interactive_hispeed_freq = 1;
    }

    snprintf(tmp_s, NODE_MAX, "%d", reqval);
    FWRITE_STR(cih, tmp_s, strlen(tmp_s), rc);
    return rc;
}

/* Set io_is_busy */
static int interactive_io_is_busy(uint16_t data)
{
    int rc = 0;
    char tmp_s[NODE_MAX];
    static unsigned int stored_interactive_io_is_busy = 0;
    int reqval = (data & 0xff);

    if (reqval == 0xff) {
        if (intiobl > 0) {
            FWRITE_STR(iiib, intiob, intiobl, rc);
            stored_interactive_io_is_busy = 0;
        }
        return rc;
    }

    if (!stored_interactive_io_is_busy) {
        FREAD_STR(iiib, intiob, NODE_MAX, rc);
        if (rc >= 0) {
            intiobl = strlen(intiob);
        }
        stored_interactive_io_is_busy = 1;
    }

    reqval = !!reqval; /* 0 or 1 */

    snprintf(tmp_s, NODE_MAX, "%d", reqval);
    FWRITE_STR(iiib, tmp_s, strlen(tmp_s), rc);
    return rc;
}

static int sync_freq(uint16_t data)
{
    int rc = 0;
    int i = 0;
    char tmp_s[NODE_MAX];
    int reqval = (data & 0xff);
    static unsigned int stored_sync_freq = 0;

    if (reqval == 0) {
        if (sfl > 0) {
            FWRITE_STR(sfreq, sf, sfl, rc);
            stored_sync_freq = 0;
        }
        return rc;
    }

    if (reqval > 0) {
        reqval *= 100000;
        for (i = 0; i < c0fL_n; i++) {
            if (c0fL[i] >= reqval) {
                reqval = c0fL[i];
                break;
            }
        }
        if (i == c0fL_n)
            reqval = c0fL[i - 1];
    }

    if (!stored_sync_freq) {
        FREAD_STR(sfreq, sf,
            NODE_MAX, rc);
        if (rc >= 0) {
            sfl = strlen(sf);
        }
        stored_sync_freq = 1;
    }

    snprintf(tmp_s, NODE_MAX, "%d", reqval);
    FWRITE_STR(sfreq, tmp_s, strlen(tmp_s), rc);
    return rc;
}

static int optimal_freq(uint16_t data)
{
    int rc = 0;
    int i = 0;
    char tmp_s[NODE_MAX];
    int reqval = (data & 0xff);
    static unsigned int stored_optimal_freq = 0;

    if (reqval == 0) {
        if (ofl > 0) {
            FWRITE_STR(ofreq, of, ofl, rc);
            stored_optimal_freq = 0;
        }
        return rc;
    }

    if (reqval > 0) {
        reqval *= 100000;
        for (i = 0; i < c0fL_n; i++) {
            if (c0fL[i] >= reqval) {
                reqval = c0fL[i];
                break;
            }
        }
        if (i == c0fL_n)
            reqval = c0fL[i - 1];
    }

    if (!stored_optimal_freq) {
        FREAD_STR(ofreq, of,
            NODE_MAX, rc);
        if (rc >= 0) {
            ofl = strlen(of);
        }
        stored_optimal_freq = 1;
    }

    snprintf(tmp_s, NODE_MAX, "%d", reqval);
    FWRITE_STR(ofreq, tmp_s, strlen(tmp_s), rc);
    return rc;
}

/* Set sched_upmigrate */
static int sched_upmigrate(uint16_t data)
{
    int rc = 0;
    char tmp_s[NODE_MAX];
    static unsigned int stored_sched_upmigrate = 0;
    int reqval = (data & 0xff);

    if (reqval == 0) {
        if (sch_upmig_sl > 0) {
            FWRITE_STR(SCHED_UPMIGRATE, sch_upmig_s, sch_upmig_sl, rc);
            stored_sched_upmigrate = 0;
        }
        return rc;
    }

    if (!stored_sched_upmigrate) {
        FREAD_STR(SCHED_UPMIGRATE, sch_upmig_s, NODE_MAX, rc);
        if (rc >= 0) {
            sch_upmig_sl = strlen(sch_upmig_s);
        }
        stored_sched_upmigrate = 1;
    }

    snprintf(tmp_s, NODE_MAX, "%d", reqval);
    FWRITE_STR(SCHED_UPMIGRATE, tmp_s, strlen(tmp_s), rc);
    return rc;
}

/* Set sched_downmigrate */
static int sched_downmigrate(uint16_t data)
{
    int rc = 0;
    char tmp_s[NODE_MAX];
    static unsigned int stored_sched_downmigrate = 0;
    int reqval = (data & 0xff);

    if (reqval == 0) {
        if (sch_upmig_sl > 0) {
            FWRITE_STR(SCHED_DOWNMIGRATE, sch_dwnmig_s, sch_dwnmig_sl, rc);
            stored_sched_downmigrate = 0;
        }
        return rc;
    }

    if (!stored_sched_downmigrate) {
        FREAD_STR(SCHED_DOWNMIGRATE, sch_dwnmig_s, NODE_MAX, rc);
        if (rc >= 0) {
            sch_dwnmig_sl = strlen(sch_dwnmig_s);
        }
        stored_sched_downmigrate = 1;
    }

    snprintf(tmp_s, NODE_MAX, "%d", reqval);
    FWRITE_STR(SCHED_DOWNMIGRATE, tmp_s, strlen(tmp_s), rc);
    return rc;
}

/* Set sched_small_task */
static int sched_small_task(uint16_t data)
{
    int rc = 0;
    char tmp_s[NODE_MAX];
    static unsigned int stored_sched_small_task = 0;
    int reqval = (data & 0xff);

    if (reqval == 0) {
        if (sch_sml_tsk_sl > 0) {
            FWRITE_STR(SCHED_SMALL_TASK, sch_sml_tsk_s, sch_sml_tsk_sl, rc);
            stored_sched_small_task = 0;
        }
        return rc;
    }

    if (!stored_sched_small_task) {
        FREAD_STR(SCHED_SMALL_TASK, sch_sml_tsk_s, NODE_MAX, rc);
        if (rc >= 0) {
            sch_sml_tsk_sl = strlen(sch_sml_tsk_s);
        }
        stored_sched_small_task = 1;
    }

    snprintf(tmp_s, NODE_MAX, "%d", reqval);
    FWRITE_STR(SCHED_SMALL_TASK, tmp_s, strlen(tmp_s), rc);
    return rc;
}

/* Set sched_mostly_idle_load */
static int sched_mostly_idle_load(uint16_t data)
{
    int rc = 0;
    char tmp_s[NODE_MAX];
    static unsigned int stored_sched_mostly_idle_load = 0;
    int reqval = (data & 0xff);

    if (reqval == 0) {
        if (sch_mst_idl_ld_sl > 0) {
            FWRITE_STR(SCHED_MOSTLY_IDLE_LOAD, sch_mst_idl_ld_s, sch_mst_idl_ld_sl, rc);
            stored_sched_mostly_idle_load = 0;
        }
        return rc;
    }

    if (!stored_sched_mostly_idle_load) {
        FREAD_STR(SCHED_MOSTLY_IDLE_LOAD, sch_mst_idl_ld_s, NODE_MAX, rc);
        if (rc >= 0) {
            sch_mst_idl_ld_sl = strlen(sch_mst_idl_ld_s);
        }
        stored_sched_mostly_idle_load = 1;
    }

    snprintf(tmp_s, NODE_MAX, "%d", reqval);
    FWRITE_STR(SCHED_MOSTLY_IDLE_LOAD, tmp_s, strlen(tmp_s), rc);
    return rc;
}

/* Set sched_mostly_idle_nr_run */
static int sched_mostly_idle_nr_run(uint16_t data)
{
    int rc = 0;
    char tmp_s[NODE_MAX];
    static unsigned int stored_sched_mostly_idle_nr_run = 0;
    int reqval = (data & 0xff);

    if (reqval == 0) {
        if (sch_mst_idl_nr_rn_sl > 0) {
            FWRITE_STR(SCHED_MOSTLY_IDLE_NR_RUN, sch_mst_idl_nr_rn_s, sch_mst_idl_nr_rn_sl, rc);
            stored_sched_mostly_idle_nr_run = 0;
        }
        return rc;
    }

    if (!stored_sched_mostly_idle_nr_run) {
        FREAD_STR(SCHED_MOSTLY_IDLE_NR_RUN, sch_mst_idl_nr_rn_s, NODE_MAX, rc);
        if (rc >= 0) {
            sch_mst_idl_nr_rn_sl = strlen(sch_mst_idl_nr_rn_s);
        }
        stored_sched_mostly_idle_nr_run = 1;
    }

    snprintf(tmp_s, NODE_MAX, "%d", reqval);
    FWRITE_STR(SCHED_MOSTLY_IDLE_NR_RUN, tmp_s, strlen(tmp_s), rc);
    return rc;
}

/* Set sched_init_task_load */
static int sched_init_task_load(uint16_t data)
{
    int rc = 0;
    char tmp_s[NODE_MAX];
    static unsigned int stored_sched_init_task_load = 0;
    int reqval = (data & 0xff);

    if (reqval == 0) {
        if (sch_init_tsk_ld_sl > 0) {
            FWRITE_STR(SCHED_INIT_TASK_LOAD, sch_init_tsk_ld_s, sch_init_tsk_ld_sl, rc);
            stored_sched_init_task_load = 0;
        }
        return rc;
    }

    if (!stored_sched_init_task_load) {
        FREAD_STR(SCHED_INIT_TASK_LOAD, sch_init_tsk_ld_s, NODE_MAX, rc);
        if (rc >= 0) {
            sch_init_tsk_ld_sl = strlen(sch_init_tsk_ld_s);
        }
        stored_sched_init_task_load = 1;
    }

    snprintf(tmp_s, NODE_MAX, "%d", reqval);
    FWRITE_STR(SCHED_INIT_TASK_LOAD, tmp_s, strlen(tmp_s), rc);
    return rc;
}

static int get_core_status(int cpu)
{
    if (cpu < 0 || cpu >= max_num_cpus) {
        return FAILED;
    }

    return core_status[cpu];
}

/*  Caller of the function should take care of the
 *  error condition when frequency is not found
 */
static int find_frequency_pos(int cpu, int freq)
{
  int i;
  for (i = 0; i < cpu_freq_val[cpu].count; i++) {
      if (cpu_freq_val[cpu].avl_freq[i] == freq)
             return i;
  }
  return -1;
}

static void check_min_freq_prop_set(int cpu)
{
  if ((cpu < max_num_cpu_clustr_0) && (min_freq_prop_0_set))
  {
     cpu_freq_val[cpu].min_freq_pos = find_frequency_pos(cpu, min_freq_prop_0_val);
     if (cpu_freq_val[cpu].min_freq_pos == -1) {
        QLOGW("Warning: min prop frequency not found setting it to default" );
        cpu_freq_val[cpu].min_freq_pos = 0;
       }
  }
  else if ((cpu > max_num_cpu_clustr_0-1) && (cpu < max_num_cpu_clustr_1) && min_freq_prop_4_set)
  {
     cpu_freq_val[cpu].min_freq_pos = find_frequency_pos(cpu, min_freq_prop_4_val);
     if (cpu_freq_val[cpu].min_freq_pos == -1) {
        QLOGW("Warning: min prop frequency not found setting it to default" );
        cpu_freq_val[cpu].min_freq_pos = 0;
       }
  }
}

static int init_available_freq(int cpu)
{
   int rc = -1;
   char listf[FREQLIST_STR];
   char *cfL = NULL, *pcfL = NULL;
   char avlnode[NODE_MAX];
   if (cpu < 0 || cpu >= max_num_cpu_supt) {
      QLOGE("Incorrect cpu%d" ,cpu);
      return 0;
   }
   sprintf(avlnode, AVL_FREQ_NODE, cpu);
   FREAD_STR(avlnode, listf, FREQLIST_STR, rc);
   if (rc > 0) {
      listf[rc - 1] = '\0';
      QLOGI("Initializing available freq for core %d as %s", cpu, listf);
      cfL = strtok_r(listf, " ", &pcfL);
      if (cfL) {
         cpu_freq_val[cpu].avl_freq[cpu_freq_val[cpu].count++] = atoi(cfL);
         while ((cfL = strtok_r(NULL, " ", &pcfL)) != NULL) {
               cpu_freq_val[cpu].avl_freq[cpu_freq_val[cpu].count++] = atoi(cfL);
               if(cpu_freq_val[cpu].count >= MAX_FREQ) {
                  QLOGE("Number of frequency is more than the size of the array.Exiting");
                  return FAILED;
               }
         }
         cpu_freq_val[cpu].valid = 1;
         cpu_freq_val[cpu].min_freq_pos = 0;
         check_min_freq_prop_set(cpu);
     }
   } else {
     QLOGW("Initialization of available freq for core %d failed, as %s not present", cpu, avlnode);
   }
   return SUCCESS;
}

static int find_next_cpu_frequency(int cpu, int freq)
{
   int i;
   if (cpu_freq_val[cpu].valid != 1)
      if(FAILED == init_available_freq(cpu))
         return FAILED;

   if (cpu_freq_val[cpu].valid != 1)
      return FAILED;

   for (i = cpu_freq_val[cpu].min_freq_pos; i < cpu_freq_val[cpu].count ; i++)
       if (cpu_freq_val[cpu].avl_freq[i] >= freq)
          return cpu_freq_val[cpu].avl_freq[i];

   if (i == cpu_freq_val[cpu].count)
      return cpu_freq_val[cpu].avl_freq[i-1];

   return FAILED;
}

static int get_reset_cpu_freq(int cpu, int ftype)
{
   if (cpu_freq_val[cpu].valid != 1) {
      QLOGE("Error: Perf-lock release for an un-initialized cpu %d", cpu);
      return FAILED;
   }
   else {
      if (ftype == MIN_FREQ_REQ)
         return cpu_freq_val[cpu].avl_freq[cpu_freq_val[cpu].min_freq_pos];
      else if (ftype == MAX_FREQ_REQ)
         return cpu_freq_val[cpu].avl_freq[cpu_freq_val[cpu].count-1];
   }
  return FAILED;
}

/* CPU options */
static int cpu_options(uint16_t data)
{
    int rc = 0;
    int i = 0;
    char tmp_s[NODE_MAX];
    int reqtype = (data & 0xff00) >> 8;
    int reqval  = (data & 0xff);
    char *cpu_node = NULL;
    int min_freq = ((reqtype <= CPU3_MIN_FREQ) || (reqtype >= CPU4_MIN_FREQ && reqtype <= CPU7_MIN_FREQ)) ? 1 : 0;
    int cpu = -1;
    int resource_id = 0;
    int freqtoupdate = 0;
    /* This is the gap value for each CPU between its scaling_min and scaling_max resource
     * This will be used to compute the resource id for scaling_max when given
     * resource for scaling_min and vice versa
     */
    const int min_max_gap = 19;
    int assoc_min_freq = 0;
    int assoc_max_freq = 0;
    cpu = cpu_freq_state[reqtype].ideal_cpu_id;

#ifdef PERFD
    int is_clustr_0 = ((cpu >= 0) && (cpu < max_num_cpu_clustr_0)) ? 1 : 0;

    if (!is_clustr_0 && !max_num_cpu_clustr_1) {
       QLOGW("Warning: As cluster 1 does not exists resource id 0x%X not supported", reqtype);
       return FAILED;
    }

    /* For sync_cores, where changing frequency for one core changes, ignore the requests
     * for resources other than for core0 and core4. This is to ensure that we may not end
     * up loosing concurrency support for cpufreq perflocks.
     */

    if (is_sync_cores && ((reqtype >= CPU1_MIN_FREQ && reqtype <= CPU3_MIN_FREQ) ||
                          (reqtype >= CPU1_MAX_FREQ && reqtype <= CPU3_MAX_FREQ))){
       QLOGW("Warning: Resource 0x%X not supported. Instead use resource for core0", reqtype);
       return FAILED;
    }
    if (is_sync_cores && ((reqtype >= CPU5_MIN_FREQ && reqtype <= CPU7_MIN_FREQ) ||
                          (reqtype >= CPU5_MAX_FREQ && reqtype <= CPU7_MAX_FREQ))){
       QLOGW("Warning: Resource 0x%X not supported. Instead use resource for core4", reqtype);
       return FAILED;
    }

    if (!reqval) {
        if (is_clustr_0) {
            if (min_freq) {
                snprintf(tmp_s, NODE_MAX, "0:%d 1:%d 2:%d 3:%d", reqval, reqval, reqval, reqval);
                QLOGI("reset min freq req for CPUs 0-3: %s", tmp_s);
                FWRITE_STR(KPM_CPU_MIN_FREQ_NODE, tmp_s, strlen(tmp_s), rc);
            } else {
                snprintf(tmp_s, NODE_MAX, "0:%u 1:%u 2:%u 3:%u", UINT_MAX, UINT_MAX, UINT_MAX, UINT_MAX);
                QLOGI("reset max freq req for CPUs 0-3: %s", tmp_s);
                FWRITE_STR(KPM_CPU_MAX_FREQ_NODE, tmp_s, strlen(tmp_s), rc);
            }
        } else {
            if (min_freq) {
                snprintf(tmp_s, NODE_MAX, "4:%d 5:%d 6:%d 7:%d", reqval, reqval, reqval, reqval);
                QLOGI("reset min freq req for CPUs 4-7: %s", tmp_s);
                FWRITE_STR(KPM_CPU_MIN_FREQ_NODE, tmp_s, strlen(tmp_s), rc);
            } else {
                snprintf(tmp_s, NODE_MAX, "4:%u 5:%u 6:%u 7:%u", UINT_MAX, UINT_MAX, UINT_MAX, UINT_MAX);
                QLOGI("reset max freq req for CPUs 4-7: %s", tmp_s);
                FWRITE_STR(KPM_CPU_MAX_FREQ_NODE, tmp_s, strlen(tmp_s), rc);
            }
        }
        return rc;
    }

    reqval *= 100000;
    reqval = find_next_cpu_frequency(cpu, reqval);
    if (reqval == FAILED)
       return FAILED;

    if (is_clustr_0) {
        snprintf(tmp_s, NODE_MAX, "0:%d 1:%d 2:%d 3:%d", reqval, reqval, reqval, reqval);
    } else {
        snprintf(tmp_s, NODE_MAX, "4:%d 5:%d 6:%d 7:%d", reqval, reqval, reqval, reqval);
    }

    if (min_freq) {
        QLOGI("set min freq req: %s", tmp_s);
        FWRITE_STR(KPM_CPU_MIN_FREQ_NODE, tmp_s, strlen(tmp_s), rc);
    } else {
        QLOGI("set max freq req: %s", tmp_s);
        FWRITE_STR(KPM_CPU_MAX_FREQ_NODE, tmp_s, strlen(tmp_s), rc);
    }

    return rc;
#else
    if (min_freq) {
        int invalid_min_freq = ((reqtype < CPU0_MIN_FREQ) || (reqtype > CPU3_MIN_FREQ)) ? 1 : 0;
        assoc_max_freq = reqtype+min_max_gap;
        int invalid_assoc_max_freq = ((assoc_max_freq < CPU0_MAX_FREQ) || (assoc_max_freq > CPU3_MAX_FREQ)) ? 1 : 0;
        if (invalid_min_freq || invalid_assoc_max_freq) {
            QLOGE("Invalid inputs for cpu min options");
            return FAILED;
        }
    } else {
        int invalid_max_freq = ((reqtype < CPU0_MAX_FREQ) || (reqtype > CPU3_MAX_FREQ)) ? 1 : 0;
        assoc_min_freq = reqtype-min_max_gap;
        int invalid_assoc_min_freq = ((assoc_min_freq < CPU0_MIN_FREQ) || (assoc_min_freq > CPU3_MIN_FREQ)) ? 1 : 0;
        if (invalid_max_freq || invalid_assoc_min_freq) {
            QLOGE("Invalid inputs for cpu max options");
            return FAILED;
        }
    }
    /* If the CPU requested is CPU0 we will immediately assign the proper sysfs
     * node to use based on whether its request for scaling_min or scaling_max
     * If not CPU 0 then we'll check to see if the designated
     * CPU for that resource is online. If it is not then we'll have to
     * cycle through the remaining available CPUs and check if its online.
     * Once we find an available CPU then we'll map the cpu to the resource
     * identifier and mark it as in_use.
     */
    if (reqtype == CPU0_MIN_FREQ) {
        cpu_node = c00;
    } else if (reqtype == CPU0_MAX_FREQ) {
        cpu_node = c01;
    } else {
        /* get the cpu to use for this resource */
        cpu = cpu_freq_state[reqtype].mapped_cpu;

        if (reqval) {
            /* we are acquiring the lock for CPU freq */

            if (!cpu) {
                /* there is no mapped cpu for the requested resource */

                if (1 > get_online_cpus(core_status, sizeof(core_status)/sizeof(core_status[0]))) {
                    QLOGE("Invalid number of cpus are online for frequency request");
                    return FAILED;
                }

                /* search for an available CPU */
                for (resource_id = CPU1_MIN_FREQ; resource_id < (CPU3_MIN_FREQ+1); resource_id++) {

                    cpu = cpu_freq_state[resource_id].ideal_cpu_id;

                    if (1 == get_core_status(cpu) && !cpu_freq_map[cpu].in_use) {
                        /* found a CPU, so use it */

                        cpu_freq_map[cpu].in_use = 1;
                        cpu_freq_state[reqtype].mapped_cpu = cpu;

                        if (min_freq) {
                            cpu_node = cpu_freq_map[cpu].min;
                        } else {
                            cpu_node = cpu_freq_map[cpu].max;
                        }
                        break;
                    }
                }

                if (cpu_node == NULL) {
                    QLOGE("Unable to find an available cpu");
                    return FAILED;
                }

            } else {
                /* there is already a mapped cpu for this request, so use it */
                if (min_freq) {
                    cpu_node = cpu_freq_map[cpu].min;
                } else {
                    cpu_node = cpu_freq_map[cpu].max;
                }
            }
        } else {
            /* we are releasing the lock on the CPU */

            if (!cpu) {
                /* there is no mapped resource for this release request */
                QLOGE("Unable to find the cpu assigned for this resource");
                return FAILED;
            }

            /* if there are no more requests for the CPU in use, unmap to release the CPU */
            if (min_freq) {
                cpu_node = cpu_freq_map[cpu].min;
                if (!resource_qs[assoc_max_freq].level) {
                    cpu_freq_state[reqtype].mapped_cpu = 0;
                    cpu_freq_state[assoc_max_freq].mapped_cpu = 0;
                    cpu_freq_map[cpu].in_use = 0;
                }
            } else {
                cpu_node = cpu_freq_map[cpu].max;
                if (!resource_qs[assoc_min_freq].level) {
                    cpu_freq_state[reqtype].mapped_cpu = 0;
                    cpu_freq_state[assoc_min_freq].mapped_cpu = 0;
                    cpu_freq_map[cpu].in_use = 0;
                }
            }
        }
    }
#endif
    if (!reqval) {
        if (min_freq) {
           freqtoupdate = get_reset_cpu_freq(cpu, MIN_FREQ_REQ);
           if (freqtoupdate == FAILED)
              return FAILED;
        } else {
           freqtoupdate = get_reset_cpu_freq(cpu, MAX_FREQ_REQ);
           if (freqtoupdate == FAILED)
               return FAILED;
        }
        snprintf(tmp_s, NODE_MAX, "%d", freqtoupdate);
        FWRITE_STR(cpu_node, tmp_s, strlen(tmp_s), rc);

        /* Core is offline while releasing perf-lock. Bring it online first */
        if ( rc <= 0 )  {
           char* cpu_online_node = cpu_freq_map[cpu].online;

           QLOGW("CPU %d is offline. Bring it online to reset to default min_freq", cpu);

           /* Disable KTM and bring core to online */
           FWRITE_STR(KTM_ENABLE_NODE, "0", 1, rc);
           FWRITE_STR(cpu_online_node, "1", 1, rc);

           /* Now, set the default min_freq */
           FWRITE_STR(cpu_node, tmp_s, strlen(tmp_s), rc);

           if ( rc <= 0 )
              QLOGW("Still, Unable to set default min_freq for CPU %d", cpu);

           /* Shutdown core and enable KTM back */
           FWRITE_STR(cpu_online_node, "0", 1, rc);
           FWRITE_STR(KTM_ENABLE_NODE, "1", 1, rc);

           if ( rc <= 0 )
              QLOGW("Unable to enable KTM core-control !");
        }
        return rc;
    }

    reqval *= 100000;
    reqval =  find_next_cpu_frequency(cpu, reqval);
    if (reqval == FAILED)
       return FAILED;
    snprintf(tmp_s, NODE_MAX, "%d", reqval);
    FWRITE_STR(cpu_node, tmp_s, strlen(tmp_s), rc);

    return rc;
}

static int toggle_thread_migration_sync(uint16_t opt_data)
{
    int rc = FAILED;
    int on = opt_data & 0xFF;

    if (on == 0xFF) {
        FWRITE_STR(sctm, sctm_s, 1, rc);
    } else if (on == 1) {
        FWRITE_STR(sctm, "1", 1, rc);
    } else {
        FWRITE_STR(sctm, "0", 1, rc);
    }

    return rc;
}

static int toggle_step_up(uint16_t opt_data)
{
    int rc = FAILED;
    int on = opt_data & 0x01;
    static unsigned int stored_step_up = 0;
    if (opt_data == 0xff && stored_step_up) {
        if (enbl_stp_up_l > 0) {
            FWRITE_STR(enbl_stp_up, enbl_stp_up_s, enbl_stp_up_l, rc);
            stored_step_up = 0;
        }
        return rc;
    }
    if (!stored_step_up) {
        FREAD_STR(enbl_stp_up, enbl_stp_up_s, NODE_MAX, rc);
        if (rc >= 0) {
            enbl_stp_up_l = strlen(enbl_stp_up_s);
        }
        stored_step_up = 1;
    }

    if (on) {
        FWRITE_STR(enbl_stp_up, "1", 1, rc);
    }
    else {
        FWRITE_STR(enbl_stp_up, "0", 1, rc);
    }

    return rc;
}

static int write_max_intr_steps(uint16_t opt_data)
{
    int rc = FAILED;
    char tmp_s[NODE_MAX];
    int max_steps = opt_data & 0xFF;
    static unsigned int stored_max_intr_steps = 0;
    if (opt_data == 0xff && stored_max_intr_steps) {
        if (max_int_steps_l > 0) {
            FWRITE_STR(max_int_steps, max_int_steps_s, max_int_steps_l, rc);
            stored_max_intr_steps = 0;
        }
        return rc;
    }
    if (!stored_max_intr_steps) {
        FREAD_STR(max_int_steps, max_int_steps_s, NODE_MAX, rc);
        if (rc >= 0) {
            max_int_steps_l = strlen(max_int_steps_s);
        }
        stored_max_intr_steps = 1;
    }
    snprintf(tmp_s, NODE_MAX, "%d", max_steps);
    FWRITE_STR(max_int_steps, tmp_s, strlen(tmp_s), rc);
    return rc;
}

/* Set sched boost */
static int sched_boost(uint16_t data)
{
    int rc = -1;
    char tmp_s[NODE_MAX];
    int reqval = (data & 0xff);

    if (!is_sched_boost_supported)
       return rc;

    reqval = !!reqval; /* 0 or 1 */
    snprintf(tmp_s, NODE_MAX, "%d", reqval);
    FWRITE_STR(schedb_n, tmp_s, strlen(tmp_s), rc);
    return rc;
}

static int interactive_pro_above_hispeed_delay(uint16_t data)
{
    int rc = 0;
    char tmp_s[NODE_MAX];
    int reqtype = (data & 0xff00) >> 8;
    int reqval = (data & 0xff);
    char *ipahd_s;
    int *ipahd_sl;

    if (reqtype == CPU0_INTERACTIVE_ABOVE_HISPEED_DELAY) {
        iahd[27] = '0';
        ipahd_s = ip_0_ahd_s;
        ipahd_sl = &ip_0_ahd_sl;
    } else {
        iahd[27] = '4';
        ipahd_s = ip_4_ahd_s;
        ipahd_sl = &ip_4_ahd_sl;
    }

    if (reqval == 0xff) {
        if (*ipahd_sl > 0) {
            FWRITE_STR(iahd, ipahd_s, *ipahd_sl, rc);
            *ipahd_sl = -1;
        }
        return rc;
    }

    if (*ipahd_sl == -1) {
        FREAD_STR(iahd, ipahd_s, NODE_MAX, rc);
        if (rc >= 0) {
            *ipahd_sl = strlen(ipahd_s);
        }
    }
    reqval *= 10000;
    snprintf(tmp_s, NODE_MAX, "%d", reqval);
    FWRITE_STR(iahd, tmp_s, strlen(tmp_s), rc);
    return rc;
}

static int interactive_pro_boost(uint16_t data)
{
    int rc = 0;
    char tmp_s[NODE_MAX];
    int reqtype = (data & 0xff00) >> 8;
    int reqval = (data & 0xff);
    char *ipb_s;
    int *ipb_sl;

    if (reqtype == CPU0_INTERACTIVE_BOOST) {
        ib[27] = '0';
        ipb_s = ip_0_b_s;
        ipb_sl = &ip_0_b_sl;
    } else {
        ib[27] = '4';
        ipb_s = ip_4_b_s;
        ipb_sl = &ip_4_b_sl;
    }

    if (reqval == 0xff) {
        if (*ipb_sl > 0) {
            FWRITE_STR(ib, ipb_s, *ipb_sl, rc);
            *ipb_sl = -1;
        }
        return rc;
    }

    if (*ipb_sl == -1) {
        FREAD_STR(ib, ipb_s, NODE_MAX, rc);
        if (rc >= 0) {
            *ipb_sl = strlen(ipb_s);
        }
    }

    reqval = !!reqval; /* 0 or 1 */

    snprintf(tmp_s, NODE_MAX, "%d", reqval);
    FWRITE_STR(ib, tmp_s, strlen(tmp_s), rc);
    return rc;
}

static int interactive_pro_boostpulse(uint16_t data)
{
    int rc = 0;
    char tmp_s[NODE_MAX];
    int reqtype = (data & 0xff00) >> 8;
    int reqval = (data & 0xff);

    if (reqtype == CPU0_INTERACTIVE_BOOSTPULSE) {
        ibp[27] = '0';
    } else {
        ibp[27] = '4';
    }

    snprintf(tmp_s, NODE_MAX, "%d", reqval);
    FWRITE_STR(ibp, tmp_s, strlen(tmp_s), rc);
    return rc;
}

static int interactive_pro_boostpulse_duration(uint16_t data)
{
    int rc = 0;
    char tmp_s[NODE_MAX];
    int reqtype = (data & 0xff00) >> 8;
    int reqval = (data & 0xff);
    char *ipbpd_s;
    int *ipbpd_sl;

    if (reqtype == CPU0_INTERACTIVE_BOOSTPULSE_DURATION) {
        ibpd[27] = '0';
        ipbpd_s = ip_0_bpd_s;
        ipbpd_sl = &ip_0_bpd_sl;
    } else {
        ibpd[27] = '4';
        ipbpd_s = ip_4_bpd_s;
        ipbpd_sl = &ip_4_bpd_sl;
    }

    if (reqval == 0) {
        if (*ipbpd_sl > 0) {
            FWRITE_STR(ibpd, ipbpd_s, *ipbpd_sl, rc);
            *ipbpd_sl = -1;
        }
        return rc;
    }

    if (*ipbpd_sl == -1) {
        FREAD_STR(ibpd, ipbpd_s, NODE_MAX, rc);
        if (rc >= 0) {
            *ipbpd_sl = strlen(ipbpd_s);
        }
    }

    reqval *= 10000;

    snprintf(tmp_s, NODE_MAX, "%d", reqval);
    FWRITE_STR(ibpd, tmp_s, strlen(tmp_s), rc);
    return rc;
}

/* Set go_hispeed_load */
static int interactive_pro_go_hispeed_load(uint16_t data)
{
    int rc = 0;
    char tmp_s[NODE_MAX];
    int reqtype = (data & 0xff00) >> 8;
    int reqval = (data & 0xff);
    char *ipghl_s;
    int *ipghl_sl;

    if (reqtype == CPU0_INTERACTIVE_GO_HISPEED_LOAD) {
        ighl[27] = '0';
        ipghl_s = ip_0_ghl_s;
        ipghl_sl = &ip_0_ghl_sl;
    } else {
        ighl[27] = '4';
        ipghl_s = ip_4_ghl_s;
        ipghl_sl = &ip_4_ghl_sl;
    }

    if (reqval == 0) {
        if (*ipghl_sl > 0) {
            FWRITE_STR(ighl, ipghl_s, *ipghl_sl, rc);
            *ipghl_sl = -1;
        }
        return rc;
    }

    if (*ipghl_sl == -1) {
        FREAD_STR(ighl, ipghl_s, NODE_MAX, rc);
        if (rc >= 0) {
            *ipghl_sl = strlen(ipghl_s);
        }
    }

    snprintf(tmp_s, NODE_MAX, "%d", reqval);
    FWRITE_STR(ighl, tmp_s, strlen(tmp_s), rc);
    return rc;
}

static int interactive_pro_hispeed_freq(uint16_t data)
{
    int rc = 0;
    int i = 0;
    char tmp_s[NODE_MAX];
    int reqtype = (data & 0xff00) >> 8;
    int reqval = (data & 0xff);
    char *iphf_s;
    int *iphf_sl;

    if (reqtype == CPU0_INTERACTIVE_HISPEED_FREQ) {
        ihf[27] = '0';
        iphf_s = ip_0_hf_s;
        iphf_sl = &ip_0_hf_sl;
    } else {
        ihf[27] = '4';
        iphf_s = ip_4_hf_s;
        iphf_sl = &ip_4_hf_sl;
    }

    if (reqval == 0) {
        if (*iphf_sl > 0) {
            FWRITE_STR(ihf, iphf_s, *iphf_sl, rc);
            *iphf_sl = -1;
        }
        return rc;
    }

    if (reqval > 0) {
        reqval *= 100000;
        reqval =  find_next_cpu_frequency(0, reqval);
    }

    if (*iphf_sl == -1) {
        FREAD_STR(ihf, iphf_s,
            NODE_MAX, rc);
        if (rc >= 0) {
            *iphf_sl = strlen(iphf_s);
        }
    }

    snprintf(tmp_s, NODE_MAX, "%d", reqval);
    FWRITE_STR(ihf, tmp_s, strlen(tmp_s), rc);
    return rc;
}

/* Set io_is_busy */
static int interactive_pro_io_is_busy(uint16_t data)
{
    int rc = 0;
    char tmp_s[NODE_MAX];
    int reqtype = (data & 0xff00) >> 8;
    int reqval = (data & 0xff);
    char *ipioib_s;
    int *ipioib_sl;

    if (reqtype == CPU0_INTERACTIVE_IO_IS_BUSY) {
        iioib[27] = '0';
        ipioib_s = ip_0_ioib_s;
        ipioib_sl = &ip_0_ioib_sl;
    } else {
        iioib[27] = '4';
        ipioib_s = ip_4_ioib_s;
        ipioib_sl = &ip_4_ioib_sl;
    }

    if (reqval == 0xff) {
        if (*ipioib_sl > 0) {
            FWRITE_STR(iioib, ipioib_s, *ipioib_sl, rc);
            *ipioib_sl = -1;
        }
        return rc;
    }

    if (*ipioib_sl == -1) {
        FREAD_STR(iioib, ipioib_s, NODE_MAX, rc);
        if (rc >= 0) {
            *ipioib_sl = strlen(ipioib_s);
        }
    }

    reqval = !!reqval; /* 0 or 1 */

    snprintf(tmp_s, NODE_MAX, "%d", reqval);
    FWRITE_STR(iioib, tmp_s, strlen(tmp_s), rc);
    return rc;
}
static int interactive_pro_min_sample_time(uint16_t data)
{
    int rc = 0;
    char tmp_s[NODE_MAX];
    int reqtype = (data & 0xff00) >> 8;
    int reqval = (data & 0xff);
    char *ipmst_s;
    int *ipmst_sl;

    if (reqtype == CPU0_INTERACTIVE_MIN_SAMPLE_TIME) {
        imst[27] = '0';
        ipmst_s = ip_0_mst_s;
        ipmst_sl = &ip_0_mst_sl;
    } else {
        imst[27] = '4';
        ipmst_s = ip_4_mst_s;
        ipmst_sl = &ip_4_mst_sl;
    }

    if (reqval == 0xFF) {
        if (*ipmst_sl > 0) {
            FWRITE_STR(imst, ipmst_s, *ipmst_sl, rc);
            *ipmst_sl = -1;
        }
        return rc;
    }

    if (*ipmst_sl == -1) {
        FREAD_STR(imst, ipmst_s, NODE_MAX, rc);
        if (rc >= 0) {
            *ipmst_sl = strlen(ipmst_s);
        }
    }

    reqval *= 10000;

    snprintf(tmp_s, NODE_MAX, "%d", reqval);
    FWRITE_STR(imst, tmp_s, strlen(tmp_s), rc);
    return rc;
}

static int interactive_pro_target_loads(uint16_t data)
{
    int rc = 0;
    char tmp_s[NODE_MAX];
    int reqtype = (data & 0xff00) >> 8;
    int reqval = (data & 0xff);
    char *iptl_s;
    int *iptl_sl;

    if (reqtype == CPU0_INTERACTIVE_TARGET_LOADS) {
        itl[27] = '0';
        iptl_s = ip_0_tl_s;
        iptl_sl = &ip_0_tl_sl;
    } else {
        itl[27] = '4';
        iptl_s = ip_4_tl_s;
        iptl_sl = &ip_4_tl_sl;
    }

    if (reqval == 0xff) {
        if (*iptl_sl > 0) {
            FWRITE_STR(itl, iptl_s, *iptl_sl, rc);
            *iptl_sl = -1;
        }
        return rc;
    }

    if (*iptl_sl == -1) {
        FREAD_STR(itl, iptl_s, NODE_MAX, rc);
        if (rc >= 0) {
            *iptl_sl = strlen(iptl_s);
        }
    }

    snprintf(tmp_s, NODE_MAX, "%d", reqval);
    FWRITE_STR(itl, tmp_s, strlen(tmp_s), rc);
    return rc;
}

/* Set the timer rate. */
static int interactive_pro_timer_rate(uint16_t data)
{
    int rc = 0;
    char tmp_s[NODE_MAX];
    int reqtype = (data & 0xff00) >> 8;
    int reqval = 0xff - (data & 0xff);
    char *iptr_s;
    int *iptr_sl;

    if (reqtype == CPU0_INTERACTIVE_TIMER_RATE) {
        itrate[27] = '0';
        iptr_s = ip_0_tr_s;
        iptr_sl = &ip_0_tr_sl;
    } else {
        itrate[27] = '4';
        iptr_s = ip_4_tr_s;
        iptr_sl = &ip_4_tr_sl;
    }

    if (reqval == 0xff) {
        if (*iptr_sl > 0) {
            FWRITE_STR(itrate, iptr_s, *iptr_sl, rc);
            *iptr_sl = -1;
        }
        return rc;
    }

    if (*iptr_sl == -1) {
        FREAD_STR(itrate, iptr_s, NODE_MAX, rc);
        if (rc >= 0) {
            *iptr_sl = strlen(iptr_s);
        }
    }

    reqval *= 10000;

    snprintf(tmp_s, NODE_MAX, "%d", reqval);
    FWRITE_STR(itrate, tmp_s, strlen(tmp_s), rc);
    return rc;
}

static int interactive_pro_timer_slack(uint16_t data)
{
    int rc = 0;
    char tmp_s[NODE_MAX];
    int reqtype = (data & 0xff00) >> 8;
    int reqval = 0xff - (data & 0xff);
    char *ipts_s;
    int *ipts_sl;

    if (reqtype == CPU0_INTERACTIVE_TIMER_SLACK) {
        its[27] = '0';
        ipts_s = ip_0_ts_s;
        ipts_sl = &ip_0_ts_sl;
    } else {
        its[27] = '4';
        ipts_s = ip_4_ts_s;
        ipts_sl = &ip_4_ts_sl;
    }

    if (reqval == 0xff) {
        if (*ipts_sl > 0) {
            FWRITE_STR(its, ipts_s, *ipts_sl, rc);
            *ipts_sl = -1;
        }
        return rc;
    }

    if (*ipts_sl == -1) {
        FREAD_STR(its, ipts_s, NODE_MAX, rc);
        if (rc >= 0) {
            *ipts_sl = strlen(ipts_s);
        }
    }

    reqval *= 10000;

    snprintf(tmp_s, NODE_MAX, "%d", reqval);
    FWRITE_STR(its, tmp_s, strlen(tmp_s), rc);
    return rc;
}

/* Set up the ideal CPU ids associated with each CPU freq resource
 * In turn set up the cpu freq resource mappings for each CPU
 */
void setup_cpu_freq_mappings() {
    int i;
    for (i = 0; i < CPU_FREQ_RESOURCES_MAX; i++){
        cpu_freq_state[i].ideal_cpu_id = -1;
    }
    cpu_freq_state[CPU0_MIN_FREQ].ideal_cpu_id = cpu_freq_state[CPU0_MAX_FREQ].ideal_cpu_id = 0;
    cpu_freq_state[CPU1_MIN_FREQ].ideal_cpu_id = cpu_freq_state[CPU1_MAX_FREQ].ideal_cpu_id = 1;
    cpu_freq_state[CPU2_MIN_FREQ].ideal_cpu_id = cpu_freq_state[CPU2_MAX_FREQ].ideal_cpu_id = 2;
    cpu_freq_state[CPU3_MIN_FREQ].ideal_cpu_id = cpu_freq_state[CPU3_MAX_FREQ].ideal_cpu_id = 3;
    cpu_freq_state[CPU4_MIN_FREQ].ideal_cpu_id = cpu_freq_state[CPU4_MAX_FREQ].ideal_cpu_id = 4;
    cpu_freq_state[CPU5_MIN_FREQ].ideal_cpu_id = cpu_freq_state[CPU5_MAX_FREQ].ideal_cpu_id = 5;
    cpu_freq_state[CPU6_MIN_FREQ].ideal_cpu_id = cpu_freq_state[CPU6_MAX_FREQ].ideal_cpu_id = 6;
    cpu_freq_state[CPU7_MIN_FREQ].ideal_cpu_id = cpu_freq_state[CPU7_MAX_FREQ].ideal_cpu_id = 7;
}

/* Allow all requestes of the reqtype until len
 * to be mapped to a single core specified with dstcpu
 */
void static_map_core(int reqtype, int len, int dstcpu) {
   int i = reqtype;
   for ( ; i <= reqtype+len; i++) {
       cpu_freq_state[i].ideal_cpu_id = dstcpu;
   }
}
/*=========================================================================================================================*/
/* PerfLock related functions                                                                                              */
/*=========================================================================================================================*/

void del_req_list()
{
    struct list_node *current;
    struct list_node *next;
    current = active_list_head;

    if (current == NULL) {
        return;
    }

    while (current != NULL)
    {
        next = current->next;
        free(current->handle);
        free(current);
        current = next;
    }
    active_list_head = NULL;
}

void del_req_q()
{
    int i = 0;
    struct q_node *current;
    struct q_node *next;

    for (i = 0; i < OPTIMIZATIONS_MAX; i++) {
        current = resource_qs[i].next;
        while (current != NULL) {
            next = current->next;
            free(current);
            current = next;
        }
    }
}

void reset()
{
    QLOGI("Initiating PerfLock reset");
    reset_to_default_values();
    del_req_list();
    del_req_q();
    memset(resource_qs, 0, sizeof(resource_qs));
    active_reqs = 0;
    QLOGI("PerfLock reset completed");
}

/* Wrapper functions for function pointers */
static int dummy(uint16_t __unused unused)
{
    return 0;
}

static int disable_ksm(uint16_t __unused unused)
{
    return toggle_ksm_run(0);
}

static int enable_ksm(uint16_t __unused unused)
{
    return toggle_ksm_run(1);
}

static int set_ksm_param(uint16_t __unused unused)
{
   int rc = 0;
   if(is_ksm_supported == 0)
   {
       char sleep_time[PROPERTY_VALUE_MAX];
       char scan_page[PROPERTY_VALUE_MAX];
       memset(sleep_time, 0, sizeof(sleep_time));
       memset(scan_page, 0, sizeof(scan_page));
       property_get("system.ksm_sleeptime", sleep_time, "20");
       property_get("system.ksm_scan_page", scan_page, "300");
       FWRITE_STR(ksm_param_sleeptime, sleep_time, strlen(sleep_time), rc);
       FWRITE_STR(ksm_param_pages_to_scan, scan_page, strlen(scan_page), rc);
   }
   return rc;
}

static int reset_ksm_param(uint16_t __unused unused)
{
   int rc = 0;
   if(is_ksm_supported == 0)
   {
      FWRITE_STR(ksm_param_sleeptime, ksm_sleep_millisecs, strlen(ksm_sleep_millisecs), rc);
      FWRITE_STR(ksm_param_pages_to_scan, ksm_pages_to_scan, strlen(ksm_pages_to_scan), rc);
   }
   return rc;
}

static void run_ksm_aggressive()
{
    int params[] = {0};
    params[0]=0x1D00;
    //Run KSM aggreddivly for 120 Sec
    internal_perf_lock_acq(0,120*1000,params, sizeof(params)/sizeof(params[0]), getpid(), gettid());
}


/* Set up function pointers for resources */
static int (*apply_opts[OPTIMIZATIONS_MAX])(uint16_t) = \
{
    dummy,
    toggle_power_collapse,
    cpu_options,
    cpu_options,
    cpu_options,
    cpu_options,
    dummy,
    lock_min_cores,
    lock_max_cores,
    dummy,
    dummy,
    sampling_rate,
    ondemand_io_is_busy,
    ondemand_sampling_down_factor,
    interactive_timer_rate,
    interactive_hispeed_freq,
    interactive_hispeed_load,
    sync_freq,
    optimal_freq,
    toggle_screen_power_collapse,
    toggle_thread_migration_sync,
    cpu_options,
    cpu_options,
    cpu_options,
    cpu_options,
    toggle_step_up,
    write_max_intr_steps,
    interactive_io_is_busy,
    disable_ksm,
    set_ksm_param,
    sched_boost,
    cpu_options,
    cpu_options,
    cpu_options,
    cpu_options,
    cpu_options,
    cpu_options,
    cpu_options,
    cpu_options,
    interactive_pro_above_hispeed_delay,
    interactive_pro_boost,
    interactive_pro_boostpulse,
    interactive_pro_boostpulse_duration,
    interactive_pro_go_hispeed_load,
    interactive_pro_hispeed_freq,
    interactive_pro_io_is_busy,
    interactive_pro_min_sample_time,
    interactive_pro_target_loads,
    interactive_pro_timer_rate,
    interactive_pro_timer_slack,
    interactive_pro_above_hispeed_delay,
    interactive_pro_boost,
    interactive_pro_boostpulse,
    interactive_pro_boostpulse_duration,
    interactive_pro_go_hispeed_load,
    interactive_pro_hispeed_freq,
    interactive_pro_io_is_busy,
    interactive_pro_min_sample_time,
    interactive_pro_target_loads,
    interactive_pro_timer_rate,
    interactive_pro_timer_slack,
    lock_max_cores,
    sched_prefer_idle,
    sched_migrate_cost,
    sched_small_task,
    sched_mostly_idle_load,
    sched_mostly_idle_nr_run,
    sched_init_task_load,
    dummy,
    dummy,
    dummy,
    dummy,
    dummy,
    dummy,
    dummy,
    dummy,
    dummy,
    dummy,
    sched_upmigrate,
    sched_downmigrate
};

static int (*reset_opts[OPTIMIZATIONS_MAX])(uint16_t) = \
{
    dummy,
    toggle_power_collapse,
    cpu_options,
    cpu_options,
    cpu_options,
    cpu_options,
    dummy,
    lock_min_cores,
    lock_max_cores,
    dummy,
    dummy,
    sampling_rate,
    ondemand_io_is_busy,
    ondemand_sampling_down_factor,
    interactive_timer_rate,
    interactive_hispeed_freq,
    interactive_hispeed_load,
    sync_freq,
    optimal_freq,
    toggle_screen_power_collapse,
    toggle_thread_migration_sync,
    cpu_options,
    cpu_options,
    cpu_options,
    cpu_options,
    toggle_step_up,
    write_max_intr_steps,
    interactive_io_is_busy,
    enable_ksm,
    reset_ksm_param,
    sched_boost,
    cpu_options,
    cpu_options,
    cpu_options,
    cpu_options,
    cpu_options,
    cpu_options,
    cpu_options,
    cpu_options,
    interactive_pro_above_hispeed_delay,
    interactive_pro_boost,
    interactive_pro_boostpulse,
    interactive_pro_boostpulse_duration,
    interactive_pro_go_hispeed_load,
    interactive_pro_hispeed_freq,
    interactive_pro_io_is_busy,
    interactive_pro_min_sample_time,
    interactive_pro_target_loads,
    interactive_pro_timer_rate,
    interactive_pro_timer_slack,
    interactive_pro_above_hispeed_delay,
    interactive_pro_boost,
    interactive_pro_boostpulse,
    interactive_pro_boostpulse_duration,
    interactive_pro_go_hispeed_load,
    interactive_pro_hispeed_freq,
    interactive_pro_io_is_busy,
    interactive_pro_min_sample_time,
    interactive_pro_target_loads,
    interactive_pro_timer_rate,
    interactive_pro_timer_slack,
    lock_max_cores,
    sched_prefer_idle,
    sched_migrate_cost,
    sched_small_task,
    sched_mostly_idle_load,
    sched_mostly_idle_nr_run,
    sched_init_task_load,
    dummy,
    dummy,
    dummy,
    dummy,
    dummy,
    dummy,
    dummy,
    dummy,
    dummy,
    dummy,
    sched_upmigrate,
    sched_downmigrate
};




static int reset_values[OPTIMIZATIONS_MAX] = \
{
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0xFF,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0xFF,
    0,
    0,
    0,
    0,
    0xFF,
    0xFF,
    0xFF,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0xFF,
    0xFF,
    0,
    0,
    0,
    0,
    0xFF,
    0xFF,
    0xFF,
    0,
    0,
    0xFF,
    0xFF,
    0,
    0,
    0,
    0,
    0xFF,
    0xFF,
    0xFF,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0
};

/* Always add new request to head of active list */
static int add_active(struct request *req, int req_handle)
{
    struct list_node *new;

    new = (struct list_node *)calloc(1, sizeof(struct list_node));
    if (new == NULL) {
        QLOGE("Failed to create active request");
        return FAILED;
    } else {
        new->handle = req;
        new->return_handle = req_handle;
        if (active_list_head == NULL) {
            new->next = NULL;
        } else {
            new->next = active_list_head;
        }
        active_list_head = new;
    }
    active_reqs++;
    return SUCCESS;
}

static void remove_active(struct request *req)
{
    struct list_node *del = NULL;
    struct list_node *iter = NULL;

    if (active_list_head == NULL)
        return;
    if (active_list_head->handle == req) {
        del = active_list_head;
        active_list_head = active_list_head->next;
    } else {
        iter = active_list_head;
        while ((iter->next != NULL) && (iter->next->handle != req)) {
            iter = iter->next;
        }
        del = iter->next;
        if (del != NULL)
            iter->next = del->next;
    }
    active_reqs--;
    /* Free list_node */
    if (del) {
        free(del);
    }
}

static struct request * is_in_list(int handle)
{
    struct list_node *iter;
    iter = active_list_head;

    if (iter == NULL || handle < 1) {
        return NULL;
    }

    while ((iter->next != NULL) && (iter->return_handle != handle)) {
        iter = iter->next;
    }
    if (iter->return_handle == handle) {
        return iter->handle;
    }
    return NULL;
}

/* Search for request in active list */
static struct request * is_active(int handle, int duration, int num_args, int list[])
{
    int i;
    struct list_node *iter;
    iter = active_list_head;

    if (iter == NULL || handle < 1 || num_args <= 0) {
        return NULL;
    }

    while ((iter->next != NULL) && (iter->return_handle != handle)) {
        iter = iter->next;
    }
    if (iter->return_handle == handle) {
        if (num_args != iter->handle->num_args || duration != iter->handle->duration) {
            return NULL;
        }
        i = num_args-1;
        do {
            if (list[i] != iter->handle->opt[i]) {
                return NULL;
            }
        } while (--i > -1);
        return iter->handle;
    }
    return NULL;
}

static void timer_callback(sigval_t pvData)
{
    if (pvData.sival_int != 0) {
        mpctl_send(MPCTL_CMD_PERFLOCKREL, pvData.sival_int);
    }
}

static int create_timer(struct request *req, int req_handle)
{
    int rc = 0;

    struct sigevent mSigEvent;
    mSigEvent.sigev_notify = SIGEV_THREAD;
    mSigEvent.sigev_notify_function = &timer_callback;
    mSigEvent.sigev_notify_attributes = NULL;
    mSigEvent.sigev_value.sival_int = req_handle;

    timer_t tTimerId;
    rc = timer_create(CLOCK_MONOTONIC, &mSigEvent, &tTimerId);
    if (rc != 0) {
        QLOGE("Failed to create timer");
        return rc;
    }
    req->timer = tTimerId;
    return rc;
}

static int set_timer(struct request *req)
{
    int rc = 0;
    int uTime = req->duration;

    struct itimerspec mTimeSpec;
    mTimeSpec.it_value.tv_sec = uTime / TIME_MSEC_IN_SEC;
    mTimeSpec.it_value.tv_nsec = (uTime % TIME_MSEC_IN_SEC) * TIME_NSEC_IN_MSEC;
    mTimeSpec.it_interval.tv_sec = 0;
    mTimeSpec.it_interval.tv_nsec = 0;

    rc = timer_settime(req->timer, 0, &mTimeSpec, NULL);
    if (rc != 0) {
        QLOGE("Failed to set timer");
    }
    return rc;
}

/* Remove request from pending queue */
static void rm_pending_request(struct request *req, int resource)
{
    struct q_node *del = NULL;
    struct q_node *iter = &resource_qs[resource];

    while ((iter->next != NULL) && (iter->next->handle != req)) {
        iter = iter->next;
    }
    del = iter->next;
    if (del != NULL)
        iter->next = del->next;
    /* Free q_node */
    if (del) {
        free(del);
    }
}

/* This is needed because when we are given the opt code
 * we only know what optimization value to apply,
 * but not what value to reset to.
 */
static int get_reset_opt_data(int resource, int level)
{
    int opt_data = reset_values[resource];

    if ((resource >= CPU0_MIN_FREQ && resource <= CPU3_MIN_FREQ) ||
        (resource >= CPU4_MIN_FREQ && resource <= CPU7_MIN_FREQ) ||
        (resource >= CPU0_MAX_FREQ && resource <= CPU3_MAX_FREQ) ||
        (resource >= CPU4_MAX_FREQ && resource <= CPU7_MAX_FREQ)) {
        opt_data = ((resource & 0xff) << 8) | (0 & 0xff);
    } else if (resource == POWER_COLLAPSE || resource == SCREEN_PWR_CLPS) {
        opt_data = (!level) & 0xff;
    } else if (resource >= CPU0_INTERACTIVE_ABOVE_HISPEED_DELAY && resource <= CPU4_INTERACTIVE_TIMER_SLACK) {
        opt_data = ((resource & 0xff) << 8) | (opt_data & 0xff);
    } else if (resource == CLUSTR_0_MAX_CORES || resource == CLUSTR_1_MAX_CORES) {
        opt_data = ((resource & 0xff) << 8) | (opt_data & 0xff);
    }

    return opt_data;
}

/* Iterate through all the resources, pend the current lock
 * and reset their levels but retain the current locked
 * request struct.
 */
static void pend_current_requests()
{
    int i = 0;
    int reset_opt_data = 0;
    int rc;
    struct q_node *current_resource, *pended;

    do {
        current_resource = &resource_qs[i];

        if (current_resource->handle != NULL) {
            /* resource is locked, pend the current req */
            pended = (struct q_node *)calloc(1, sizeof(struct q_node));
            if (pended == NULL) {
                QLOGW("Display off request, failed to pend existing request for resource=%d", i);
                continue;
            }
            *pended = *current_resource;
            current_resource->next = pended;

            reset_opt_data = get_reset_opt_data(i, current_resource->level);
            rc = (*reset_opts[i])(reset_opt_data);
            if (rc < 0) {
                QLOGW("Display off request, failed to apply optimization for resource=%d", i);
            }

            current_resource->level = 0;
        }

    } while (++i < OPTIMIZATIONS_MAX);
}

/* After all the requests for display off case
 * have been honored, lock the current resuorce
 * ensure display off request stays at top priority
 */
static void lock_current_state(struct request *req)
{
    int i = 0;
    struct q_node *current_resource;

    do {
        current_resource = &resource_qs[i];
        current_resource->handle = req;
        current_resource->level = MAX_LVL;
    } while (++i < OPTIMIZATIONS_MAX);
}

/* After we have released the lock on the resources
 * requested by the display off scenario, we can go
 * ahead and release all the other resources that were
 * occupied by the display off case.
 */
static void unlock_current_state(struct request *req)
{
    int i = 0;
    struct q_node *current_resource;
    struct q_node *pending;
    struct q_node *del = NULL;

    do {
        current_resource = &resource_qs[i];
        pending = resource_qs[i].next;
        if ((current_resource->handle != NULL) && (current_resource->handle == req)) {
            if (pending != NULL) {
                if (pending->level < current_resource->level) {
                    uint16_t opt_data;
                    opt_data = ((i & 0xff) << 8) | (pending->level & 0xff);
                    (*apply_opts[i])(opt_data);
                }
                del = pending;
                *current_resource = *(current_resource->next);
                /* Free q_node */
                if (del) {
                    free(del);
                }
            } else {
                current_resource->handle = NULL;
                current_resource->level = 0;
            }
        }
    } while (++i < OPTIMIZATIONS_MAX);
}

static void arm_poll_timer(int arm)
{
    int rc = -1;
    int uTime = 0;

    if (arm) {
        uTime = POLL_TIMER_MS;
    }

    struct itimerspec mTimeSpec;
    mTimeSpec.it_value.tv_sec = uTime / TIME_MSEC_IN_SEC;
    mTimeSpec.it_value.tv_nsec = (uTime % TIME_MSEC_IN_SEC) * TIME_NSEC_IN_MSEC;
    mTimeSpec.it_interval.tv_sec = 0;
    mTimeSpec.it_interval.tv_nsec = 0;

    rc = timer_settime(poll_timer, 0, &mTimeSpec, NULL);
    if (rc != 0) {
        QLOGE("Failed to arm poll timer");
    }
}

/* Timer callback function for polling clients */
static void poll_cb()
{
    mpctl_send(MPCTL_CMD_PERFLOCKPOLL);
    arm_poll_timer(1);
}

/* Poll to see if clients of PerfLock are still alive
 * If client is no longer active, release its lock
 */
static void poll_client_status()
{
    int rc = -1;
    struct list_node *iter;
    iter = active_list_head;
    char proc[NODE_MAX];          // "/proc/<pid>/status" string
    char p_stat[NODE_MAX];        // sysfs "/proc/<pid>/status" return string

    while (iter != NULL) {
        QLOGI("Active client: %d", iter->handle->pid);
        snprintf(proc, NODE_MAX, "/proc/%d/status", iter->handle->pid);
        FREAD_STR(proc, p_stat, 10, rc);
        /* Client does not exist */
        if ((rc < 0) && (errno == ENOENT)) {
            mpctl_send(MPCTL_CMD_PERFLOCKREL, iter->return_handle);
        }
        iter = iter->next;
    }

    return;
}

/* Main function used when PerfLock acquire is requested */
int internal_perf_lock_acq(int handle, int duration, int list[], int num_args, pid_t client_pid, pid_t client_tid)
{
    char trace_buf[TRACE_BUF_SZ];
    char trace_arg_buf[TRACE_BUF_SZ];
    char trace_args_buf[TRACE_BUF_SZ] = "list=";
    int arg = 0;
    static int handle_counter = 0;
    int rc = 0;
    int i = 0;
    int resource = 0;
    int level = 0;
    int status = 0;
    int *opt = list;
    struct request *req = NULL;
    struct request *exists = NULL;
    struct q_node *current_resource;
    struct q_node *pended, *new, *iter = NULL;

    if (num_args <= 0 || active_reqs == ACTIVE_REQS_MAX) {
        QLOGW("No optimizations performed");
        rc = send(conn_socket, &status, sizeof(status), 0);
        return rc;
    }

    /* Check if the number of arguments being requested is valid.
     * The maximum number of arguments supported is defined by
     * OPTIMIZATIONS_MAX.
     */
    if (num_args > OPTIMIZATIONS_MAX)
        num_args = OPTIMIZATIONS_MAX;

    for (arg = 0; arg < num_args; arg++) {
        snprintf(trace_arg_buf, TRACE_BUF_SZ, "0x%0X ", list[arg]);
        strlcat(trace_args_buf, trace_arg_buf, TRACE_BUF_SZ);
    }

    snprintf(trace_buf, TRACE_BUF_SZ, "perf_lock_acq: client_pid=%d, client_tid=%d, inupt handle=%d, duration=%d ms, num_args=%d, %s",\
             client_pid, client_tid, handle, duration, num_args, trace_args_buf);
    QLOGI("%s", trace_buf);

    if (PERF_SYSTRACE) {
        ATRACE_BEGIN(trace_buf);
    }

    exists = is_active(handle, duration, num_args, list);
    if (exists != NULL) {
        /* if the request exists, simply reset its timer and return handle to client */
        set_timer(exists);
        rc = send(conn_socket, &handle, sizeof(handle), 0);
        if (PERF_SYSTRACE) {
            ATRACE_END();
        }
        return rc;
    }

    /* request does not exist, create a new request */
    req = (struct request *)calloc(1, sizeof(struct request));
    if (req == NULL) {
        QLOGE("Failed to create new request, no optimizations performed");
        rc = send(conn_socket, &status, sizeof(status), 0);
        if (PERF_SYSTRACE) {
            ATRACE_END();
        }
        return rc;
    }

    handle_counter++;
    if (handle_counter > 2000000000)
        handle_counter = 1;

    snprintf(trace_buf, TRACE_BUF_SZ, "perf_lock_acq: output handle=%d", handle_counter);
    QLOGI("%s", trace_buf);

    if (PERF_SYSTRACE) {
        ATRACE_BEGIN(trace_buf);
        ATRACE_END();
    }

    clock_gettime(CLOCK_MONOTONIC, &ack);
    time_delta = (BILLION * (ack.tv_sec - connected.tv_sec)) + (ack.tv_nsec - connected.tv_nsec);
    QLOGI("time taken from connected to ack: %llu nanoseconds", (long long unsigned int)time_delta);
    time_delta2 = (BILLION * (ack.tv_sec - recvd.tv_sec)) + (ack.tv_nsec - recvd.tv_nsec);
    QLOGI("time taken from recvd to ack: %llu nanoseconds", (long long unsigned int)time_delta2);

    if (time_delta > 980000000) {
        /*Do not apply perflock*/
        if (req != NULL) {
            free(req);
        }
        QLOGE("Server exceeed timeout period to respond, no optimizations performed");
        rc = send(conn_socket, &status, sizeof(status), 0);
        return FAILED;
    }

    /* return handle back to client */
    rc = send(conn_socket, &handle_counter, sizeof(handle_counter), 0);

    req->duration = duration;
    req->num_args = num_args;
    req->pid = client_pid;
    req->tid = client_tid;

    memcpy(req->opt, list, sizeof(int)*num_args);

    /* handle display off case,
     * DISPLAY_OFF opcode is assumed to always
     * be the first argument of a display off request
     */
    if (*opt == DISPLAY_OFF) {
        pend_current_requests();
    }

    do {
        resource = (*(opt + i) & 0xFF00) >> 8;
        /* If resource requested is not supported, ignore it */
        if (resource < 0 || resource >= OPTIMIZATIONS_MAX)
            continue;
        level = *(opt + i) & 0xFF;
        current_resource = &resource_qs[resource];

        if (current_resource->level == 0) {
            /* resource is not being used by perflock */
            rc = (*apply_opts[resource])(*(opt + i));
            if (rc < 0) {
                QLOGE("Failed to apply optimization 0x%X", *(opt + i));
            }
            current_resource->handle = req;
            current_resource->level = level;

        } else if (level > current_resource->level) {
            /* new request is higher lvl than current */
            pended = (struct q_node *)calloc(1, sizeof(struct q_node));
            if (pended == NULL) {
                QLOGW("Failed to pend existing request");
                continue;
            }
            *pended = *current_resource;

            rc = (*apply_opts[resource])(*(opt + i));
            if (rc < 0) {
                QLOGE("Failed to apply optimization 0x%X", *(opt + i));
            }
            current_resource->handle = req;
            current_resource->level = level;
            current_resource->next = pended;

        } else {
            /* new is equal or lower lvl than current */
            new = (struct q_node *)calloc(1, sizeof(struct q_node));
            if (new == NULL) {
                QLOGW("Failed to pend new request for optimization 0x%X", *(opt + i));
                continue;
            }
            new->handle = req;
            new->level = level;

            iter = &resource_qs[resource];
            while ((iter->next != NULL) && (iter->next->level > level)) {
                iter = iter->next;
            }
            new->next = iter->next;
            iter->next = new;
        }
    } while (++i < req->num_args);

    arm_poll_timer(1);

    /* handle display off case */
    if (*opt == DISPLAY_OFF) {
        lock_current_state(req);
        arm_poll_timer(0);
    }

    add_active(req, handle_counter);

    if (req->duration != INDEFINITE_DURATION) {
        create_timer(req, handle_counter);
        set_timer(req);
    }

    if (PERF_SYSTRACE) {
        ATRACE_END();
    }

    clock_gettime(CLOCK_MONOTONIC, &acq_completed);
    time_delta = (BILLION * (acq_completed.tv_sec - connected.tv_sec)) + (acq_completed.tv_nsec - connected.tv_nsec);
    QLOGI("time taken from connected to acq_completed: %llu nanoseconds", (long long unsigned int)time_delta);

    return handle_counter;
}

/* Main function used when PerfLock release is requested */
int internal_perf_lock_rel(int handle)
{
    char trace_buf[TRACE_BUF_SZ];
    int rc = 0;
    int resource = 0;
    int level = 0;
    int reset_opt_data = 0;
    struct request *req = NULL;
    int *opt;
    int i;

    struct q_node *del = NULL;
    struct q_node *pending;
    struct q_node *current_resource;

    snprintf(trace_buf, TRACE_BUF_SZ, "perf_lock_rel: input handle=%d", handle);
    QLOGI("%s", trace_buf);
    if (PERF_SYSTRACE) {
        ATRACE_BEGIN(trace_buf);
    }

    req = is_in_list(handle);
    if (req == NULL) {
        QLOGW("Request does not exist, nothing released");
        if (PERF_SYSTRACE) {
            ATRACE_END();
        }
        return FAILED;
    }

    i = req->num_args-1;

    /* Check if the number of arguments being requested is valid.
     * The maximum number of arguments supported is defined by
     * OPTIMIZATIONS_MAX.
     */
    if (i >= OPTIMIZATIONS_MAX)
        i = OPTIMIZATIONS_MAX-1;

    opt = req->opt;

    do {
        resource = (*(opt + i) & 0xFF00) >> 8;
        /* If resource requested is not supported, ignore it */
        if (resource < 0 || resource >= OPTIMIZATIONS_MAX)
            continue;
        level = *(opt + i) & 0xFF;
        pending = resource_qs[resource].next;
        current_resource = &resource_qs[resource];

        if (current_resource->handle == req) {
            if (pending != NULL) {
                if (pending->level < current_resource->level) {
                    uint16_t opt_data;
                    opt_data = ((resource & 0xff) << 8) | (pending->level & 0xff);
                    rc = (*apply_opts[resource])(opt_data);
                    if (rc < 0) {
                        QLOGE("Failed to apply next pending optimization 0x%X", *(opt + i));
                    }
                }
                del = pending;
                *current_resource = *(current_resource->next);
                /* Free q_node */
                if (del) {
                    free(del);
                }
            }
            else {
                reset_opt_data = get_reset_opt_data(resource, level);
                rc = (*reset_opts[resource])(reset_opt_data);
                if (rc < 0) {
                    QLOGE("Failed to reset optimization 0x%X", *(opt + i));
                }
                current_resource->handle = NULL;
                current_resource->level = 0;
            }
        } else if (pending != NULL) {
            QLOGI("Removing pending requested optimization 0x%X", *(opt + i));
            rm_pending_request(req, resource);
        } else {
            QLOGE("Release error, resource optimization not found");
        }
    } while (--i > -1);

    /* handle display off case */
    if (*opt == DISPLAY_OFF) {
        unlock_current_state(req);
        arm_poll_timer(1);
    }

    if (req->duration != INDEFINITE_DURATION) {
        rc = timer_delete(req->timer);
        if (rc != 0) {
            QLOGE("Failed to delete timer");
        }
    }

    remove_active(req);

    if (req) {
        free(req);
    }

    if (0 == active_reqs)
        arm_poll_timer(0);

    if (PERF_SYSTRACE) {
        ATRACE_END();
    }
    return SUCCESS;
}

void check_display_status_lock_cores_online(int num_cpus) {
    int fd;
    char buf[20] = {0};
    int params[] = {0};

    if (num_cpus < 2 || num_cpus > nocs()) {
        return;
    }

    params[0] = ((CLUSTR_0_CPUS_ON & 0xff) << 8) | (num_cpus & 0xff);

    fd = open("/sys/class/graphics/fb0/mdp/power/runtime_status", O_RDONLY);
    if (fd >= 0) {
        if (read(fd, buf, sizeof(buf) - 1) == -1) {
            QLOGW("Unable to read display status");
        }
        else if (!(strncmp(buf, "active", 6)))
            internal_perf_lock_acq(0, 0, params, sizeof(params)/sizeof(params[0]), getpid(), gettid());

        close(fd);
    }
}

static int setup_cpu_freq_values()
{
  int i = 0;
  int rc;
  char min_freq_prop_0[PROPERTY_VALUE_MAX];
  char min_freq_prop_4[PROPERTY_VALUE_MAX];

  /* Check for min_freq_prop and if set read the min_freq*/
  if (property_get("ro.min_freq_0", min_freq_prop_0, NULL) > 0) {
     if (isdigit(min_freq_prop_0[0])) {
        min_freq_prop_0_val = atoi(min_freq_prop_0);
        min_freq_prop_0_set = true;
     }
  }

  if (property_get("ro.min_freq_4", min_freq_prop_4, NULL) > 0) {
     if (isdigit(min_freq_prop_4[0])) {
        min_freq_prop_4_val = atoi(min_freq_prop_4);
        min_freq_prop_4_set = true;
     }
  }

  for (i = 0; i < max_num_cpu_supt; i++ )
      if(FAILED == init_available_freq(i))
         return FAILED;

  return SUCCESS;
}

static int get_soc_id()
{
  int fd;
  int soc = -1;
  if (!access("/sys/devices/soc0/soc_id", F_OK)) {
       fd = open("/sys/devices/soc0/soc_id", O_RDONLY);
  } else {
       fd = open("/sys/devices/system/soc/soc0/id", O_RDONLY);
  }
  if (fd != -1)
  {
      char raw_buf[5];
      read(fd, raw_buf,4);
      raw_buf[4] = 0;
      soc = atoi(raw_buf);
      close(fd);
  }
  else
      close(fd);
  return soc;
}

static int is_msm8939(int socid)
{
  if((239 == socid) || (263 == socid) || (241 == socid) || \
     (268 == socid) || (269 == socid) || (270 == socid) || \
     (271 == socid))
    return 1;
  else
    return 0;
}

static int is_msm8916(int socid)
{
  if((206 == socid) || (247 == socid) || (248 == socid) || \
     (249 == socid) || (250 == socid))
    return 1;
  else
    return 0;
}

static int is_msm8909(int socid)
{
  if((245 == socid) || (258 == socid) || (259 == socid) || \
     (265 == socid) || (260 == socid) || (261 == socid) || \
     (262 == socid))
    return 1;
  else
    return 0;
}

static void initialize_target()
{
  if (is_msm8939(get_soc_id()))
  {
      is_sched_boost_supported = true;
      max_num_cpu_supt = 8;
      max_num_cpu_clustr_0 = 4;
      max_num_cpu_clustr_1 = max_num_cpu_supt;
      cpu_count_clustr_0 = 4;
      cpu_count_clustr_1 = 4;
      core_ctl_cpu = 0;
  }
  else if (SOCID_8994 == get_soc_id())
  {
      is_sched_boost_supported = true;
      max_num_cpu_supt = 8;
      max_num_cpu_clustr_0 = 4;
      max_num_cpu_clustr_1 = max_num_cpu_supt;
      cpu_count_clustr_1 = 4;
      cpu_count_clustr_1 = 4;
      core_ctl_cpu = 4;
  }
  else if (is_msm8916(get_soc_id()))
  {
      is_sched_boost_supported = false;
      max_num_cpu_supt = 4;
      max_num_cpu_clustr_0 = 4;
      max_num_cpu_clustr_1 = 0;
      cpu_count_clustr_0 = 4;
      cpu_count_clustr_1 = 0;
      core_ctl_cpu = 0;
  }
  else if (is_msm8909(get_soc_id()))
  {
      is_sched_boost_supported = false;
      max_num_cpu_supt = 4;
      max_num_cpu_clustr_0 = 4;
      max_num_cpu_clustr_1 = 0;
      cpu_count_clustr_0 = 4;
      cpu_count_clustr_1 = 0;
      core_ctl_cpu = 0;
  }
}

/*=========================================================================================================================*/

static void *mpctl_server(void *data)
{
    int rc, cmd, len, cpu_prefix_len;
    char tmp_s[NODE_MAX];
    char *cfL = NULL, *pcfL = NULL;
    struct sigevent mSigEvent;
    char kpm_max_cpus[NODE_MAX];

    if (comsoc < 0)
        return NULL;

    (void)data;

    memset(resource_qs, 0, sizeof(resource_qs));

    /* Inline everything here to make it more difficult to
     * disassemble due to code size.
     */

    /* Compose CPU node strings; add cpu/cpu1/online and frequencies
     *
     * /sys/kernel/mm/ksm/run
     * /sys/devices/system/cpu/present
     * /sys/devices/system/cpu/cpu1/online
     * /sys/devices/system/cpu/cpu2/online
     * /sys/devices/system/cpu/cpu3/online
     * /sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_min_freq
     * /sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_max_freq
     * /sys/devices/system/cpu/cpu0/cpufreq/scaling_min_freq
     * /sys/devices/system/cpu/cpu0/cpufreq/scaling_max_freq
     * /sys/devices/system/cpu/cpu1/cpufreq/scaling_min_freq
     * /sys/devices/system/cpu/cpu1/cpufreq/scaling_max_freq
     * /sys/devices/system/cpu/cpu2/cpufreq/scaling_min_freq
     * /sys/devices/system/cpu/cpu2/cpufreq/scaling_max_freq
     * /sys/devices/system/cpu/cpu3/cpufreq/scaling_min_freq
     * /sys/devices/system/cpu/cpu3/cpufreq/scaling_max_freq
     * /sys/devices/system/cpu/cpu0/cpufreq/scaling_available_frequencies
     * /sys/devices/system/cpu/cpufreq/ondemand/powersave_bias
     * /sys/devices/system/cpu/cpufreq/interactive/boost
     * /sys/devices/system/cpu/cpufreq/interactive/hispeed_freq
     * /sys/devices/system/cpu/cpufreq/ondemand/sampling_rate
     * /sys/devices/system/cpu/cpufreq/ondemand/io_is_busy
     * /sys/devices/system/cpu/cpufreq/ondemand/sampling_down_factor
     * /sys/devices/system/cpu/cpufreq/interactive/go_hispeed_load
     * /sys/devices/system/cpu/cpufreq/interactive/timer_rate
     * /sys/devices/system/cpu/cpufreq/interactive/io_is_busy
     * /sys/devices/system/cpu/cpufreq/ondemand/sync_freq
     * /sys/devices/system/cpu/cpufreq/ondemand/optimal_freq
     * /dev/cpuctl/apps/cpu.notify_on_migrate
     * /sys/devices/system/cpu/cpufreq/ondemand/enable_stepup
     * /sys/devices/system/cpu/cpufreq/ondemand/max_intermediate_steps
     *
     */

#   define SC1o(x)         c1o[len+x] = c2o[len+x] = c3o[len+x] = c4o[len+x] = c5o[len+x] = c6o[len+x] = c7o[len+x] = c0o[len+x]
#   define SC2o(x)         c2o[len+x]
#   define SC3o(x)         c3o[len+x]
#   define SC4o(x)         c4o[len+x]
#   define SC5o(x)         c5o[len+x]
#   define SC6o(x)         c6o[len+x]
#   define SC7o(x)         c7o[len+x]
#   define SC0o(x)         c0o[len+x]
#   define SCP(x)          cpup[len+x]
#   define SC11(x)         c11[len+x]
#   define SCPS(x)         cps[len+x]
#   define SC0F(x)         c0f[len+x] = c4f[len+x]
#   define SCIB(x)         cib[len+x]
#   define SCIH(x)         cih[len+x]
#   define SC00d(x)        c00d[len+x]
#   define SSR(x)          sr[len+x]
#   define SOIIB(x)        oiib[len+x]
#   define SOSDF(x)        osdf[len+x]
#   define SIHL(x)         ihl[len+x]
#   define SITR(x)         itr[len+x]
#   define SIIIB(x)        iiib[len+x]
#   define SSF(x)          sfreq[len+x]
#   define SOF(x)          ofreq[len+x]
#   define SSCTM(x)        sctm[x]
#   define IAHD(x)         iahd[cpu_prefix_len+x]
#   define IB(x)           ib[cpu_prefix_len+x]
#   define IBP(x)          ibp[cpu_prefix_len+x]
#   define IBPD(x)         ibpd[cpu_prefix_len+x]
#   define IGHL(x)         ighl[cpu_prefix_len+x]
#   define IHF(x)          ihf[cpu_prefix_len+x]
#   define IIOIB(x)        iioib[cpu_prefix_len+x]
#   define IMST(x)         imst[cpu_prefix_len+x]
#   define ITL(x)          itl[cpu_prefix_len+x]
#   define ITR(x)          itrate[cpu_prefix_len+x]
#   define ITS(x)          its[cpu_prefix_len+x]

    len = sizeof(SYSFS_PREFIX) - 1;
    cpu_prefix_len = sizeof(CPU_SYSFS_PREFIX) - 1;
    strlcpy(c11, c1o, NODE_MAX);
    SC1o(0) = SC1o(4) = \
        gp[9] = \
        SC11(0) = SC11(4) = SC11(9) = SC11(18) = \
        gi[6] = \
        SSCTM(5) = SSCTM(8) = SSCTM(17) = \
        prop_name[9] = \
        SCIB(18) = 'c';
    SC1o(14) = \
        SC11(14) = SC11(31) = \
        gi[3] = gi[10] = \
        SCIB(15) = SCIB(22) = \
        prop_name[1] = prop_name[10] = prop_name[13] = \
        SSCTM(2) = SSCTM(37) = \
        SCIH(28) = SCIH(29) = SCIH(34) = 'e';
    SC11(12) = SC11(29) = SSCTM(25) = \
        prop_name[15] = \
        gp[3] = 'f';
    SC1o(10) = SC1o(13) = \
        gp[8] = go[1] = go[6] = \
        SC11(22) = \
        SCIB(13) = \
        SSCTM(21) = SSCTM(29) = \
        gi[1] = 'n';
    SC1o(1) = SC1o(5) = \
        gp[0] = \
        SC11(1) = SC11(5) = SC11(10) = \
        prop_name[12] = \
        SSCTM(6) = SSCTM(13) = SSCTM(14) = SSCTM(18) = \
        'p';
    gi[2] = gi[7] = \
        SCIB(14) = SCIB(19) = SCIB(28) = \
        SSCTM(9) = SSCTM(23) = SSCTM(36) = \
        't';
    SC11(19) = SC11(26) = \
        gp[7] = go[5] = \
        gi[5] = \
        prop_name[8] = \
        SSCTM(12) = SSCTM(35) = \
        SCIB(17) = 'a';
    SC11(15) = SC11(32) = \
        SCIH(35) = 'q';
    SC1o(2) = SC1o(6) = \
        SC11(2) = SC11(6) = SC11(11) = \
        prop_name[3] = \
        SSCTM(7) = SSCTM(19) = 'u';
    SC1o(12) = \
        SC11(21) = \
        SSCTM(24) = SSCTM(32) = \
        gi[8] = gi[0] = 'i';
    SC1o(9) = \
        SSCTM(22) = SSCTM(28) = \
        SCIB(25) = SCIB(26) = 'o';
    SC11(23) = \
        SSCTM(33) = \
        prop_name[4] = \
        'g';
    SC11(24) = SC11(28) = \
        SSCTM(27) = SSCTM(30) = \
        SCIH(31) = '_';
    SC11(13) = SC11(30) = \
        gi[4] = \
        SCIB(16) = \
        SSCTM(34) = \
        SCIH(33) = 'r';
    SC1o(11) = \
        SC11(20) = \
        SSCTM(10) = \
        'l';
    SC11(27) = 'x';
    SC1o(15) = \
        gi[11] = \
        gp[11] = go[8] = \
        SC11(33) = '\0';
    go[2] = go[7] = \
        prop_name[0] = \
        SSCTM(1) = \
        SCIH(30) = 'd';
    SC1o(7) = \
        SC11(7) = '1';
    SC11(25) = \
        gp[6] = go[4] = \
        SSCTM(31) = \
        'm';
    SC11(17) = \
        SSCTM(15) = \
        SCIB(27) = 's';
    SC1o(3) = SC1o(8) = \
        SC11(3) = SC11(8) = SC11(16) = \
        SCIB(23) = '/';
    strlcpy(cps, c11, NODE_MAX);
    strlcpy(c10, c11, NODE_MAX);
    strlcpy(c21, c11, NODE_MAX);
    strlcpy(c31, c11, NODE_MAX);
    strlcpy(c41, c11, NODE_MAX);
    strlcpy(c51, c11, NODE_MAX);
    strlcpy(c61, c11, NODE_MAX);
    strlcpy(c71, c11, NODE_MAX);
    SCPS(7) = SCIH(32) = 'f';
    SCPS(10) = 'q';
        SCPS(11) = SCPS(20) = \
        SSCTM(0) = SSCTM(4) = SSCTM(11) = SSCTM(16) = '/';
    prop_name[5] = prop_name[11] = SSCTM(20) = '.';
    SCPS(14) = SCPS(19) = 'd';
        SCPS(16) = 'm';
    SCPS(17) = SCPS(27) = SCPS(33) = 'a';
    SCPS(21) = SCIH(27) = 'p';
    SCPS(23) = 'w';
    SCPS(26) = SCPS(34) = 's';
    SCPS(28) = \
        gi[9] = \
        SSCTM(3) = \
        SCIB(21) = 'v';
    SCPS(30) = '_';
    SCPS(31) = \
        prop_name[2] = \
        SCIB(24) = 'b';
    SCPS(35) = '\0';
    c10[len+26] = SCPS(32) = 'i';
    c10[len+27] = SCPS(13) = SCPS(18) = 'n';
    strlcpy(c00, c10, NODE_MAX);
    strlcpy(c00d, c10, NODE_MAX);
    strlcpy(c20, c10, NODE_MAX);
    strlcpy(c30, c10, NODE_MAX);
    strlcpy(c01, c11, NODE_MAX);
    strlcpy(c0f, c11, NODE_MAX);
    strlcpy(c40, c10, NODE_MAX);
    strlcpy(c50, c10, NODE_MAX);
    strlcpy(c60, c10, NODE_MAX);
    strlcpy(c70, c10, NODE_MAX);
    strlcpy(c4f, c11, NODE_MAX);
    SC00d(23) = \
        SCPS(12) = SCPS(22) = \
        gp[4] = go[0] = \
        'o';
    SC00d(19) = SC0F(39) = 'u';
    SC0F(38) = 'q';
    SC0F(45) = SCIH(26) = 's';
    SC0F(46) = SCIB(29) = \
    SCIH(36) = '\0';
    SC00d(22) = SC0F(35) = 'f';
    SC0F(26) = 'v';
    gp[1] = gp[10] = go[3] = \
    SC0F(33) = SC0F(37) = SC0F(40) = SC0F(44) = \
        SCPS(9) = SCPS(15) = SCPS(24) = SCPS(29) = 'e';
        gp[2] = gp[5] = \
        prop_name[7] = prop_name[14] = \
        SC0F(36) = \
        SCPS(8) = SCPS(25) = 'r';
    strlcpy(sr, cps, NODE_MAX);
    strlcpy(oiib, cps, NODE_MAX);
    strlcpy(osdf, cps, NODE_MAX);
    strlcpy(sfreq, cps, NODE_MAX);
    strlcpy(ofreq, cps, NODE_MAX);
    SIIIB(29) = SIIIB(26) = SOF(28) = SSF(25) = SOSDF(29) = SOSDF(34) = \
        SIHL(26) = SIHL(34) = SITR(29) = \
        SOIIB(23) = SOIIB(26) = SSR(29) = SC0F(34) = '_';
    SOSDF(27) = SOSDF(33) = SSR(27) = SC00d(21) = \
        SSF(23) = SC0F(41) = 'n';
    SIHL(37) = SOF(26) = SOSDF(22) = SOSDF(36) = SSR(31) = \
        SITR(31) = SSR(22) = SC0F(25) = \
        SC0F(27) = SC0F(30) = 'a';
    SOSDF(26) = SOIIB(21) = SOIIB(24) = \
        SIHL(28) = \
        SSR(26) = SC00d(20) = SC0F(28) = SC0F(43) = 'i';
    SSF(24) = SC00d(17) = SC0F(42) = 'c';
    SITR(26) = SOF(25) = SOSDF(23) = SSR(23) = 'm';
    SOSDF(28) = SSR(28) = 'g';
    SIHL(33) = SIHL(38) = SOSDF(30) = 'd';
    SIIIB(31) = SOIIB(28) = 'u';
    SIIIB(33) = SSF(22) = SOIIB(30) = SSCTM(26) = 'y';
    SOSDF(32) = 'w';
    SOF(29) = SSF(26) = SOSDF(35) = 'f';
    SOSDF(37) = 'c';
    SOF(32) = SSF(29) = 'q';
    SOSDF(25) = SSR(25) = SOF(27) = \
        SIHL(35) = SC0F(29) = SC0F(32) = 'l';
    SIIIB(30) = SOIIB(27) = SC0F(31) = 'b';
    SIIIB(25) = SOSDF(31) = SOSDF(39) = SOIIB(22) = SOF(21) = \
        SIHL(36) = SIHL(25) = 'o';
    c00[len+7] = c00d[len+7] = c01[len+7] = c0f[len+7] = SC0o(7) = '0';
    c4f[len+7] = '4';
    SC00d(18) = 'p';
    strlcpy(cib, cps, 33);
    strlcpy(c01d, c00d, NODE_MAX);
    strlcpy(iahd, CPU_SYSFS_PREFIX, NODE_MAX);
    strlcpy(ib, CPU_SYSFS_PREFIX, NODE_MAX);
    strlcpy(ighl, CPU_SYSFS_PREFIX, NODE_MAX);
    strlcpy(ihf, CPU_SYSFS_PREFIX, NODE_MAX);
    strlcpy(iioib, CPU_SYSFS_PREFIX, NODE_MAX);
    strlcpy(imst, CPU_SYSFS_PREFIX, NODE_MAX);
    strlcpy(itl, CPU_SYSFS_PREFIX, NODE_MAX);
    strlcpy(itrate, CPU_SYSFS_PREFIX, NODE_MAX);
    IAHD(0) = IAHD(17) = c01d[len+26] = 'a';
    IB(0) = IAHD(1) = 'b';
    IAHD(12) = IAHD(14) = 'd';
    c01d[len+27] = 'x';
    SIIIB(27) = SCIB(12) = SCIB(20) = SOF(24) = \
        SITR(25) = SCIH(25) = 'i';
    strlcpy(ihl, cib, 45);
    strlcpy(itr, cib, 45);
    strlcpy(iiib, cib, 45);
    SIHL(24) = 'g';
    IAHD(7) = SIIIB(24) = 'i';
    IAHD(16) = 'l';
    IB(1) = IB(2) = IAHD(2) = 'o';
    IAHD(9) = 'p';
    IAHD(3) = 'v';
    IB(4) = 't';
    IAHD(18) = 'y';
    IB(5) = IAHD(19) = '\0';
    IAHD(5) = IAHD(13) = '_';
    //Update different charactor for 2 online and 3 online
    c20[len+7] = c21[len+7] = SC2o(7)  = '2';
    c30[len+7] = c31[len+7] = SC3o(7)  = '3';
    c40[len+7] = c41[len+7] = SC4o(7)  = '4';
    c50[len+7] = c51[len+7] = SC5o(7)  = '5';
    c60[len+7] = c61[len+7] = SC6o(7)  = '6';
    c70[len+7] = c71[len+7] = SC7o(7)  = '7';
    strlcpy(cpup, c1o, NODE_MAX);
    SOF(22) = SOSDF(24) = SIHL(30) = \
        SSR(24) = SCP(4) = 'p';
    SSF(27) = SOSDF(40) = SOF(30) = \
        SITR(28) = SITR(30) = \
        SSR(30) = SCP(5) = 'r';
    IAHD(4) = IAHD(10) = IAHD(11) = IAHD(15) = \
        SIHL(31) = SIHL(32) = SSR(33) = SCP(6) = 'e';
    SIIIB(32) = SIIIB(28) = SOSDF(21) = SOIIB(25) = SSF(21) = \
        SIHL(29) = \
        IB(3) = IAHD(8) = \
        SOIIB(29) = SSR(21) = SCP(7) = 's';
    SOF(31) = SSF(28) = SCP(8) = \
        SITR(27) = SITR(33) = 'e';
    SCP(9) = 'n';
    SOSDF(38) = SSR(32) = SOF(23) = \
        prop_name[6] = \
        SITR(24) = SITR(32) = SCP(10) = 't';
    SOF(33) = SOSDF(41) = SOIIB(31) = SSF(30) = \
        prop_name[16] = \
        SITR(34) = SIHL(39) = SSR(34) = SCP(11) = '\0';
    strlcpy(cih, cib, 45);
    IAHD(6) = SIHL(27) = SCIH(24) = 'h';
    strlcpy(ibp, ib, NODE_MAX);
    IBP(5) = 'p';
    IBP(6) = 'u';
    IBP(7) = 'l';
    IBP(8) = 's';
    IBP(9) = 'e';
    IBP(10) = '\0';
    strlcpy(ibpd, ibp, NODE_MAX);
    IBPD(10) = '_';
    IBPD(11) = 'd';
    IBPD(12) = 'u';
    IBPD(13) = 'r';
    IBPD(14) = 'a';
    IBPD(15) = 't';
    IBPD(16) = 'i';
    IBPD(17) = 'o';
    IBPD(18) = 'n';
    IBPD(19) = '\0';
    co_node[0] = co_node[4] = co_node[12] = co_node[19] = co_node[23] = '/';
    co_node[1] = co_node[3] = co_node[11] = co_node[13] = co_node[15] = 's';
    co_node[6] = co_node[10] = co_node[17] = co_node[29] = 'e';
    co_node[8] = co_node[27] = 'i';
    co_node[9] = co_node[20] = 'c';
    co_node[2] = co_node[14] = 'y';
    co_node[28] = co_node[25] = 'n';
    co_node[5] = 'd';
    co_node[7] = 'v';
    co_node[16] = 't';
    co_node[21] = 'p';
    co_node[22] = 'u';
    co_node[18] = 'm';
    co_node[26] = 'l';
    co_node[24] = 'o';
    co_node[30] = '\0';

    IGHL(0) = 'g';
    IIOIB(1) = IGHL(1) = 'o';
    IGHL(2) = '_';
    IHF(0) = IGHL(3) = 'h';
    IIOIB(6) = 'b';
    IIOIB(7) = 'u';
    IIOIB(9) = 'y';
    ITL(3) = 'g';
    ITR(2) = 'm';
    ITL(1) = IMST(5) = ITR(7) = 'a';
    ITR(0) = ITR(8) = ITL(0) = ITL(5) = IMST(11) = 't';
    IMST(0) = IMST(6) = IMST(13) = 'm';
    IMST(2) = 'n';
    ITR(1) = IMST(1) = IMST(12) = IIOIB(0) = IIOIB(3) = IHF(1) = IGHL(4) = 'i';
    ITL(11) = IMST(4) = IIOIB(4) = IIOIB(8) = IHF(2) = IGHL(5) = 's';
    IMST(7) = IHF(3) = IGHL(6) = 'p';
    ITR(3) = ITL(4) = IMST(9) = IMST(14) = IHF(4) = IGHL(7) = 'e';
    IHF(5) = IGHL(8) = 'e';
    ITL(10) = IHF(6) = IGHL(9) = 'd';
    ITR(5) = ITL(6) = IMST(3) = IMST(10) = IIOIB(2) = \
        IIOIB(5) = IHF(7) = IGHL(10) = '_';
    ITL(7) = IMST(8) = IGHL(11) = 'l';
    ITL(8) = IGHL(12) = 'o';
    ITL(2) = ITL(9) = IGHL(13) = 'a';
    IGHL(14) = 'd';
    ITL(12) = IMST(15) = IIOIB(11) = IHF(12) = IGHL(15) = '\0';
    IHF(8) = 'f';
    ITR(6) = ITR(4) = ITL(2) = IHF(9) = 'r';
    ITR(9) = IHF(10) = 'e';
    IHF(11) = 'q';
    strlcpy(its, itrate, NODE_MAX);
    ITS(6) = 's';
    ITS(7) = 'l';
    ITS(8) = 'a';
    ITS(9) = 'c';
    ITS(10) = 'k';
    ITS(11) = '\0';

    ksm_run_node[19] = 'r';
    ksm_run_node[20] = 'u';
    ksm_run_node[21] = 'n';
    ksm_run_node[22] = '\0';

    ksm_param_sleeptime[19] = 's';
    ksm_param_sleeptime[20] = 'l';
    ksm_param_sleeptime[21] = 'e';
    ksm_param_sleeptime[22] = 'e';
    ksm_param_sleeptime[23] = 'p';
    ksm_param_sleeptime[24] = '_';
    ksm_param_sleeptime[25] = 'm';
    ksm_param_sleeptime[26] = 'i';
    ksm_param_sleeptime[27] = 'l';
    ksm_param_sleeptime[28] = 'l';
    ksm_param_sleeptime[29] = 'i';
    ksm_param_sleeptime[30] = 's';
    ksm_param_sleeptime[31] = 'e';
    ksm_param_sleeptime[32] = 'c';
    ksm_param_sleeptime[33] = 's';
    ksm_param_sleeptime[34] = '\0';

    ksm_param_pages_to_scan[19] = 'p';
    ksm_param_pages_to_scan[20] = 'a';
    ksm_param_pages_to_scan[21] = 'g';
    ksm_param_pages_to_scan[22] = 'e';
    ksm_param_pages_to_scan[23] = 's';
    ksm_param_pages_to_scan[24] = '_';
    ksm_param_pages_to_scan[25] = 't';
    ksm_param_pages_to_scan[26] = 'o';
    ksm_param_pages_to_scan[27] = '_';
    ksm_param_pages_to_scan[28] = 's';
    ksm_param_pages_to_scan[29] = 'c';
    ksm_param_pages_to_scan[30] = 'a';
    ksm_param_pages_to_scan[31] = 'n';
    ksm_param_pages_to_scan[32] = '\0';

    schedb_n[23] = 'b';
    schedb_n[4] = schedb_n[18] = 'c';
    schedb_n[21] = 'd';
    schedb_n[11] = schedb_n[14] = schedb_n[20] = 'e';
    schedb_n[19] = 'h';
    schedb_n[10] = 'k';
    schedb_n[15] = 'l';
    schedb_n[13] = 'n';
    schedb_n[3] = schedb_n[24] = schedb_n[25] = 'o';
    schedb_n[1] = 'p';
    schedb_n[2] = schedb_n[12] = 'r';
    schedb_n[6] = schedb_n[8] = schedb_n[17] = schedb_n[26] = 's';
    schedb_n[27] = 't';
    schedb_n[7] = 'y';
    schedb_n[22] = '_';
    schedb_n[0] = schedb_n[5] = schedb_n[9] = schedb_n[16] = '/';
    schedb_n[28] = '\0';

    /* Enable systraces by adding debug.trace.perf=1 into build.prop */
    if (property_get(prop_name, trace_prop, NULL) > 0) {
        if (trace_prop[0] == '1'){
            perf_debug_output = PERF_SYSTRACE = atoi(trace_prop);
        }
    }
    setup_cpu_freq_mappings();

    //target based initialization
    initialize_target();

    //check core control presence
    check_core_ctl_presence();

    //check for KPM hot plug support
    FREAD_STR(KPM_MAX_CPUS, kpm_max_cpus, NODE_MAX, rc);
    if (rc > 0) {
        kpm_hotplug_support = 1;
    } else
        kpm_hotplug_support = 0;
    if (core_ctl_present == 1 && kpm_hotplug_support == 1)
        QLOGE("Error: KPM hotplug support and core control both are present at the same time");

    if(FAILED == setup_cpu_freq_values()) {
      QLOGE("Frequency initialization failed. Exit perflock server");
      return NULL;
    }

#ifdef PERFD
    FREAD_STR(KPM_MANAGED_CPUS, tmp_s, NODE_MAX, rc);
    if(rc < 0) {
       QLOGE("Error reading KPM nodes. Does KPM exist\n");
    } else {
        if (max_num_cpu_clustr_1 > 0) {
           FWRITE_STR(KPM_NUM_CLUSTERS, "2", 1, rc);
           FWRITE_STR(KPM_MANAGED_CPUS, "0-3", 3, rc);
           FWRITE_STR(KPM_MANAGED_CPUS, "4-7", 3, rc);
           if (rc < 0)
              QLOGE("KPM initialization failed for multi cluster\n");
        } else {
            FWRITE_STR(KPM_NUM_CLUSTERS, "1", 1, rc);
            FWRITE_STR(KPM_MANAGED_CPUS, "0-3", 3, rc);
            if (rc < 0)
               QLOGE("KPM initialization failed for single cluster\n");
        }
    }
#endif

    reset_to_default_values();

    //initailize core control min/max copu nodes
    core_ctl_init();

    /* Parse list of available frequencies */
    FREAD_STR(c0f, c0fL_s, FREQLIST_STR, rc);
    if (rc > 0) {
        c0fL_s[rc - 1] = '\0';
        cfL = strtok_r(c0fL_s, " ", &pcfL);
        if (cfL) {
            c0fL[c0fL_n++] = atoi(cfL);
            while ((cfL = strtok_r(NULL, " ", &pcfL)) != NULL) {
                c0fL[c0fL_n++] = atoi(cfL);
            }
        }
    }

    /* create timer to poll client status */
    mSigEvent.sigev_notify = SIGEV_THREAD;
    mSigEvent.sigev_notify_function = &poll_cb;
    mSigEvent.sigev_notify_attributes = NULL;

    rc = timer_create(CLOCK_MONOTONIC, &mSigEvent, &poll_timer);
    if (rc != 0) {
        QLOGE("Failed to create timer for client poll");
    }

    // if a file exists (or is readable, writeable ) then return 0.
    is_ksm_supported = access(ksm_run_node, F_OK);
    if(is_ksm_supported == 0)
    {
        memset(ksm_sleep_millisecs, 0, sizeof(ksm_sleep_millisecs));
        memset(ksm_pages_to_scan, 0, sizeof(ksm_pages_to_scan));
        FREAD_STR(ksm_param_sleeptime, ksm_sleep_millisecs, sizeof(ksm_sleep_millisecs), rc);
        FREAD_STR(ksm_param_pages_to_scan, ksm_pages_to_scan, sizeof(ksm_pages_to_scan), rc);
    }

    if (property_get("ro.qualcomm.perf.cores_online", cores_online_prop, NULL) > 0) {
        if (isdigit(cores_online_prop[0])){
            check_display_status_lock_cores_online(atoi(cores_online_prop));
        }
    }

    //check if KSM us enabled or not if enable than run aggressivly
    if(is_ksm_supported == 0)
    {
        run_ksm_aggressive();
    }

    FREAD_STR(c00d, c00dv, 10, rc);
    if (rc == FAILED) {
        snprintf(c00dv, NODE_MAX, "%d", FREQMIN);
    }
    FREAD_STR(c01d, c01dv, 10, rc);
    if (rc == FAILED) {
        snprintf(c01dv, NODE_MAX, "%d", FREQMAX);
    }
    FREAD_STR(sctm, sctm_s, 2, rc);

    max_num_cpus = sysconf(_SC_NPROCESSORS_CONF);

    /* Enable KTM core_control, incase "perfd" crashed just after disabling it */
    FWRITE_STR(KTM_ENABLE_NODE, "1", 1, rc);

    /* Main loop */
    for (;;) {
        len = sizeof(struct sockaddr_un);

        clock_gettime(CLOCK_MONOTONIC, &m_accept);
        time_delta = (BILLION * (m_accept.tv_sec - connected.tv_sec)) + (m_accept.tv_nsec - connected.tv_nsec);
        QLOGI("time taken from connected to accept: %llu nanoseconds", (long long unsigned int)time_delta);

        conn_socket = accept(comsoc, (struct sockaddr *) &addr, &len);
        if (conn_socket == -1) {
            QLOGE("PERFLOCK mpctl server %s: accept failed, errno=%d (%s)", __func__, errno, strerror(errno));
            goto close_conn;
        }

        clock_gettime(CLOCK_MONOTONIC, &connected);

        rc = recv(conn_socket, &msg, sizeof(mpctl_msg_t), 0);
        if (rc < 0) {
            QLOGE("PERFLOCK mpctl server: failed to receive message, errno=%d (%s)", errno, strerror(errno));
            goto close_conn;
        }

        clock_gettime(CLOCK_MONOTONIC, &recvd);

        /* Descramble client id */
        msg.client_pid ^= msg.msg;
        msg.client_tid ^= msg.msg;

        QLOGV("Received len=%d, m=%u, v=%u, c=%u, s=%u, m=%u (0x%08x) d=%u",
              rc, msg.magic, msg.version, msg.client_pid, msg.seq, msg.msg, msg.msg, msg.data);

        if (rc != sizeof(mpctl_msg_t)) {
            QLOGE("Bad size");
        }
        else if (msg.magic != MPCTL_MAGIC) {
            QLOGE("Bad magic");
            continue;
        }
        else if (msg.version != MPCTL_VERSION) {
            QLOGE("Version mismatch");
            continue;
        }

        cmd = CMD_DECODE(msg);

        switch (cmd) {
            case MPCTL_CMD_PERFLOCKACQ:
                internal_perf_lock_acq(msg.pl_handle, msg.pl_time, msg.pl_args, msg.data, msg.client_pid, msg.client_tid);
                break;

            case MPCTL_CMD_PERFLOCKREL:
                internal_perf_lock_rel(msg.pl_handle);
                break;

            case MPCTL_CMD_PERFLOCKPOLL:
                poll_client_status();
                break;

            case MPCTL_CMD_PERFLOCKRESET:
                reset();
                break;

            default:
                QLOGE("Unknown command %d", cmd);
        }
close_conn:
    close(conn_socket);
    }

    QLOGI("MPCTL server thread exit due to rc=%d", rc);
    return NULL;
}

int mpctl_server_init()
{
    int rc = 0, stage = 0;
    char buf[PROPERTY_VALUE_MAX];

    QLOGI("MPCTL server starting");

    comsoc = socket(PF_UNIX, SOCK_SEQPACKET, 0);
    if (comsoc < 0) {
        stage = 1;
        goto error;
    }

    memset(&addr, 0, sizeof(struct sockaddr_un));
    addr.sun_family = AF_UNIX;
    snprintf(addr.sun_path, UNIX_PATH_MAX, MPCTL_SOCKET);

    /* Delete existing socket file if necessary */
    unlink(addr.sun_path);
    umask(0117);
    rc = bind(comsoc, (struct sockaddr *)&addr, sizeof(struct sockaddr_un));
    if (rc != 0) {
        stage = 2;
        goto error;
    }

    rc = listen(comsoc, SOMAXCONN);
    if (rc != 0) {
        stage = 3;
        goto error;
    }

    rc = pthread_create(&mpctl_server_thread, NULL, mpctl_server, NULL);
    if (rc != 0) {
        stage = 3;
        goto error;
    }
    return 1;

error:
    QLOGE("Unable to create control service (stage=%d, rc=%d)", stage, rc);
    if (comsoc != -1) {
        close(comsoc);
        comsoc = -1;

        /* Delete socket file */
        unlink(addr.sun_path);
    }
    return 0;
}

void mpctl_server_exit()
{
    pthread_join(mpctl_server_thread, NULL);
    if (comsoc != -1) {
        close(comsoc);
        comsoc = -1;
        unlink(MPCTL_SOCKET);
    }
}

#endif /* SERVER */

#ifdef CLIENT
static volatile int seq_num = 0;

static void mpctl_client_exit(int client_comsoc)
{
    if (client_comsoc >= 0) {
        close(client_comsoc);
    }
}

int mpctl_send(int control, ...)
{
    int rc, len;
    mpctl_msg_t msg;
    int handle = 0;
    int client_comsoc = -1;
    pid_t client_pid = 0;
    pid_t client_tid = 0;
    struct sockaddr_un client_addr;
    int ready = 0;
    fd_set readfds;
    struct timeval tv;

    tv.tv_sec = 1;
    tv.tv_usec = 0;

    QLOGI("MPCTL client send %d", control);

    client_comsoc = socket(PF_UNIX, SOCK_SEQPACKET, 0);
    if (client_comsoc < 0) {
        QLOGE("Failed to initialize control channel");
        return 0;
    }

    fcntl(client_comsoc, F_SETFL, O_NONBLOCK);

    memset(&client_addr, 0, sizeof(struct sockaddr_un));
    client_addr.sun_family = AF_UNIX;
    snprintf(client_addr.sun_path, UNIX_PATH_MAX, MPCTL_SOCKET);

    /* Prepare message for sending */
    msg.magic   = MPCTL_MAGIC;
    msg.version = MPCTL_VERSION;
    msg.seq     = seq_num++;
    msg.data    = 0;

    switch (control) {

        case MPCTL_CMD_PERFLOCKACQ:
        {
            int i = 0;
            va_list vargs;
            int data = 0;

            va_start(vargs, control);

            msg.pl_handle = va_arg(vargs, int);
            msg.pl_time = va_arg(vargs, int);
            data = va_arg(vargs, int);

            msg.data = data & 0xffff;

            if (msg.data > OPTIMIZATIONS_MAX)
                msg.data = OPTIMIZATIONS_MAX;

            int *pl_args  = va_arg(vargs, int*);
            for (i=0; i < msg.data; i++) {
                msg.pl_args[i] = *(pl_args + i) & 0xffff;
            }
            va_end(vargs);

            break;
        }

        case MPCTL_CMD_PERFLOCKREL:
        {
            int pl_handle = 0;
            va_list vargs;

            va_start(vargs, control);
            pl_handle = va_arg(vargs, int);
            va_end(vargs);

            msg.pl_handle = pl_handle & 0xffffffff;
            break;
        }

        case MPCTL_CMD_PERFLOCKPOLL:
        {
            break;
        }

        case MPCTL_CMD_PERFLOCKRESET:
        {
            break;
        }

        default:
            QLOGW("Bad parameter to mpctl_send %d", control);
            return 0;
    }

    CMD_ENCODE(msg, control);

    if (client_pid == 0) {
        client_pid = getpid();
    }
    if (client_tid == 0) {
        client_tid = gettid();
    }
    msg.client_pid  = client_pid ^ msg.msg;
    msg.client_tid  = client_tid ^ msg.msg;

    len = sizeof(struct sockaddr_un);
    rc = connect(client_comsoc, (struct sockaddr *) &client_addr, len);
    if (rc == -1) {
        QLOGE("PERFLOCK Failed to connect to socket: errno=%d (%s)", errno, strerror(errno));
        mpctl_client_exit(client_comsoc);
        return rc;
    }

    rc = send(client_comsoc, &msg, sizeof(mpctl_msg_t), 0);
    if (rc == -1) {
        QLOGE("PERFLOCK Failed to send mpctl: errno=%d (%s)", errno, strerror(errno));
        mpctl_client_exit(client_comsoc);
        return rc;
    }

    if (control == MPCTL_CMD_PERFLOCKACQ) {
        FD_ZERO(&readfds);
        FD_SET(client_comsoc, &readfds);

        ready = select(client_comsoc+1, &readfds, NULL, NULL, &tv);

        if (-1 == ready) {
            QLOGE("PERFLOCK Socket not ready to be read: errno=%d (%s)", errno, strerror(errno));
            rc = -1;
        } else if (ready) {
            if (FD_ISSET(client_comsoc, &readfds)) {
                rc = recv(client_comsoc, &handle, sizeof(handle), 0);
                rc = handle;
            } else {
                QLOGE("PERFLOCK File descriptor is not set");
                rc = -1;
            }
        } else {
            QLOGE("PERFLOCK Unknown socket error");
            rc = -1;
        }
        QLOGI("PERFLOCK client_comsoc: %d time left on select, secs: %d, usecs: %d", client_comsoc, tv.tv_sec, tv.tv_usec);
    }
    mpctl_client_exit(client_comsoc);

    return rc;
}
#endif /* CLIENT */
