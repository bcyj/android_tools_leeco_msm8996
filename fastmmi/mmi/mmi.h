/*
 * Copyright (c) 2013-2015, Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
#ifndef __MMI_H
#define MMI_H
#include "common.h"
#include "layout.h"

#ifdef LOG_TAG
#undef LOG_TAG
#define LOG_TAG   "mmi"
#endif

extern hash_map < string, layout * >g_layout_map;
extern char g_res_file[PATH_MAX];
extern hash_map < string, module_info * >g_modules_map;
extern hash_map < string, module_info * >g_controller_map;
extern sem_t g_data_print_sem;
extern sem_t g_result_sem;

#define MMI_AGENT  "/system/bin/mmi_agent"
#define MMI_AGENT32  "/system/bin/mmi_agent32"
#define MMI_AGENT64  "/system/bin/mmi_agent64"

#define MMI_DIAG  "/system/bin/mmi_diag"

#define MMI_LIB_PATH64  "/system/vendor/lib64/"
#define MMI_LIB_PATH32  "/system/vendor/lib/"

#define WAKE_LOCK "/sys/power/wake_lock"
#define WAKE_UNLOCK "/sys/power/wake_unlock"

int handle_print(msg_t * msg, module_info * mod);
int handle_query(msg_t * msg, module_info * mod);
int handle_run(msg_t * msg, module_info * mod);

void start_all(bool automation) ;
void do_run_all(void *m);
int reconfig(const char *cfg);
layout *acquire_cur_layout();
void release_cur_layout();
layout *switch_cur_layout_locked(layout * lay, module_info * mod);
void set_main_module(module_info *mod);
module_info *get_main_module();
void flush_result() ;
void set_boot_mode(boot_mode_type mode);

#endif
