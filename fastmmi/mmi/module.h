/*
 * Copyright (c) 2014, Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
#ifndef __MUDULE_H
#define __MUDULE_H


/* Should be the same name as config file, all those modules
 *  need to test in MMI process for event dispatch and draw screen
 *  did not support in Modules.
 */
#define LOCAL_LCD "mmi_lcd.so"
#define LOCAL_TOUCH "mmi_touch.so"
#define LOCAL_KEY "mmi_key.so"
#define LOCAL_KEY_HEADSET "mmi_headset.so"
#define MODULE_CAMERA "mmi_camera.so"
#define MODULE_SENSOR "mmi_sensor.so"
#define MODULE_BLUETOOTH "mmi_bluetooth.so"
#define MODULE_WIFI "mmi_wifi.so"
#define MODULE_GPS "mmi_gps.so"

/**Module export function*/
void send_run_extra(module_info * mod, const char *subcmd);

int fork_launch_module(module_info * mod);
void module_exec_ui(module_info * mod);
void module_exec_pcba(module_info * mod);
void update_main_status();
void do_run_all(void *m);

void launch_main();
void initial_screen(module_info * mod);
void switch_module(void *m);
void module_cleanup(module_info * mod);

/**Local module export function*/
bool is_local_module(module_info * mod);
void launch_lcd_function(module_info * mod);
void launch_touch_function(module_info * mod);

int create_module_thread(module_info * mod);
void do_next(void *m);

#endif
