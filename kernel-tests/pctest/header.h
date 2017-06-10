#define TEST_REVISION "00.00.08"

#include <pthread.h>

#define SYS_PM "/sys/module/pm/"
#define SYS_PM2 "/sys/module/pm2/"
#define SYS_PM_8x60 "/sys/module/pm_8x60/"
#define SYS_PM_8660 "/sys/module/pm_8660/"
#define SYS_PM_MODEM_TIMEOUT "parameters/modem_port_timeout"
#define SYS_PM_MODEM_STATUS "parameters/modem_port_status"
#define SLEEP_MODE_NODE "parameters/sleep_mode"
#define SLEEP_MODE_NODE_CORE_0 "modes/cpu0/power_collapse/suspend_enabled"
#define SLEEP_MODE_NODE_CORE_1 "modes/cpu1/power_collapse/suspend_enabled"
#define SLEEP_MODE_NODE_CORE_2 "modes/cpu2/power_collapse/suspend_enabled"
#define SLEEP_MODE_NODE_CORE_3 "modes/cpu3/power_collapse/suspend_enabled"

#define IDLE_SLEEP_MODE_NODE "parameters/idle_sleep_mode"
#define IDLE_SLEEP_MODE_NODE_CORE_0 "modes/cpu0/power_collapse/idle_enabled"
#define IDLE_SLEEP_MODE_NODE_CORE_1 "modes/cpu1/power_collapse/idle_enabled"
#define IDLE_SLEEP_MODE_NODE_CORE_2 "modes/cpu2/power_collapse/idle_enabled"
#define IDLE_SLEEP_MODE_NODE_CORE_3 "modes/cpu3/power_collapse/idle_enabled"

#define IDLE_SLEEP_MODE_NODE_STD_CORE_0 "modes/cpu0/standalone_power_collapse/idle_enabled"
#define IDLE_SLEEP_MODE_NODE_STD_CORE_1 "modes/cpu1/standalone_power_collapse/idle_enabled"
#define IDLE_SLEEP_MODE_NODE_STD_CORE_2 "modes/cpu2/standalone_power_collapse/idle_enabled"
#define IDLE_SLEEP_MODE_NODE_STD_CORE_3 "modes/cpu3/standalone_power_collapse/idle_enabled"

#define SLEEP_TIME_OVERRIDE_NODE "parameters/sleep_time_override"
#define PM_STATS_NODE "/proc/msm_pm_stats"
#define HOTPLUG_NODE_CORE_1 "/sys/devices/system/cpu/cpu1/online"
#define HOTPLUG_NODE_CORE_2 "/sys/devices/system/cpu/cpu2/online"
#define HOTPLUG_NODE_CORE_3 "/sys/devices/system/cpu/cpu3/online"

#define WAKELOCK_NODE "/proc/wakelocks"
#define PROC_NODE "/proc/"
#define WAKELOCK_LOCK_NODE "/sys/power/wake_lock"
#define WAKELOCK_UNLOCK_NODE "/sys/power/wake_unlock"

#define WAKELOCK_DEBUG_MASK_NODE "/sys/module/wakelock/parameters/debug_mask"
#define EARLYSUSPEND_DEBUG_MASK_NODE "/sys/module/earlysuspend/parameters/debug_mask"
#define IRQ_DEBUG_MASK_NODE "/sys/module/irq/parameters/debug_mask"
#define IRQ_DEBUG_MASK_NODE_8x60 "/sys/module/msm_show_resume_irq/parameters/debug_mask"

#define POWER_NODE "/sys/power/state"
#define POWER_STANDBY "standby\n"
#define POWER_ON "on\n"

#define SENSOR_SETTINGS "/persist/sensors/sensors_settings"

#define IDLEPC_WAKELOCK "idle-pc-test\n"
#define SUSPENDPC_WAKELOCK "suspend-pc-test\n"

#define INT_MAX  ((int)(~0U>>1))
#define ULONG_MAX (~0UL)

#ifdef USE_ANDROID_LOG
#define LOG_TAG "pctest"
#include "cutils/log.h"
#    define msg(format, ...)   LOGE(format, ## __VA_ARGS__)
#else
#    define msg(format, ...)   printf(format, ## __VA_ARGS__)
#endif

#define dbgmsg(format, ...) \
	if (debug_output) \
		msg(format, ## __VA_ARGS__)


extern int g_delay_test_sec;
extern int g_wakeup_sec;
extern int g_timeout_sec;
extern int g_num_iter;
extern int g_ignore_tcxo;

extern int g_wakelock_exists;
extern char *g_sys_pm;
extern char *g_resume_command;
extern char g_pm_stats[1024 * 16];
extern char g_wakelock_stats[1024 * 200];
extern int g_idle_interference_timer_ms;
extern pthread_mutex_t thread_mutex;
extern struct thread_args {
	int cpu;
	int ms;
};

int directory_exists(char *);
int file_exists(char *);
int file_exists_with_prefix(char *, char *);
int open_file(char *, char *, int);
int read_from_fd(int, char *, ssize_t);
int write_to_fd(int, char *, size_t);
int read_unsigned_int_from_fd(int);
int read_from_file(char *, char *, char *, size_t);
int read_unsigned_int_from_file(char *, char *);
int write_to_file(char *, char *, char *, size_t);
int write_string_to_file(char *, char *, char *);
int write_int_to_file(char *, char *, int);
int fork_exec(char *, char *, char *, char *, int);
int parse_pm_stats_count(char *, char *);
signed long long parse_wakelock_stats_for_active_wl(char *, char *);
int get_pm_stats_line_no(char *, char *);
int suspend_test();
int idle_power_collapse_test();

typedef int (*test_func_t)(void);
