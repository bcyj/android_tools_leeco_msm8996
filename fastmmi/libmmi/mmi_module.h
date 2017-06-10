/*
 * Copyright (c) 2014, Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef MMI_MODULE_INCLUDE_H
#define MMI_MODULE_INCLUDE_H

#include "common.h"
#include "utils.h"

#ifdef LOG_TAG
#undef LOG_TAG
#define LOG_TAG   "mmim"
#endif

#define KEY_MODULE_NAME  "module"

/**
 * Name of the mmi_module_info
 */
#define MMI_MODULE_INFO_SYM         MMI
/**
 * Name of the mmi_module_info as a string
 */
#define MMI_MODULE_INFO_SYM_AS_STR  "MMI"

/** Wifi resource */
#define WIFI_DRIVER "/system/lib/modules/wlan.ko"
#define WIFI_IFCONFIG  "/system/bin/ifconfig"
#define WIFI_IWLIST  "/system/bin/iwlist"

/** Bluetooth resource */
#define BIN_BDT  "/system/bin/bdt"

/** Audio and FM resource */
#define AUDIO_FTM "/system/bin/mm-audio-ftm"
#define AUDIO_FTM_CONFIG "/etc/ftm_test_config"
#define FM_APP "/system/bin/fmfactorytest"
#define FILE_AUDIO_RECORD "/data/ftm_pcm_record.wav"
#define FILE_AUDIO_MUSIC "/etc/mmi/qualsound.wav"
#define FILE_AUDIO_PLAY_MUSIC MMI_RES_BASE_DIR"qualsound.wav"

/** Flashlight resource */
#define SYS_FLASHLIGHT "/sys/class/leds/torch-light0/brightness"

/** Sdcard resource */
#define DEV_MMCBLK1 "/dev/block/mmcblk1"
#define SYS_MMCBLK0_SIZE "/sys/class/block/mmcblk0/size"
#define CMD_SDCARD_MOUNT "mount -t vfat /dev/block/mmcblk1p1 /mnt"
#define CMD_SDCARD_UNMOUNT  "umount /mnt"

/** Simcard resource */
#define BIN_RIL_TEST "/system/bin/qmi_simple_ril_test"
#define SIM_INPUT MMI_BASE_DIR"sim_input.txt"
#define SIM_INPUT_PARAM "input="SIM_INPUT
#define SIM_OUTPUT MMI_BASE_DIR"sim_output.txt"
#define SIM_OUTPUT_PARAM "output="SIM_OUTPUT


/** GPS resource */
#define BIN_GARDEN_APP  "/system/bin/garden_app"

/**Battery*/
#define DEFAULT_SYS_VOLTAGE_NOW  "/sys/class/power_supply/battery/voltage_now"
#define DEFAULT_SYS_HEALTH  "/sys/class/power_supply/battery/health"
#define DEFAULT_SYS_STATUS  "/sys/class/power_supply/battery/status"

/**Memory*/
#define PROC_MEMINFO  "/proc/meminfo"
#define CPUINFO_MAX_FREQ  "/sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_max_freq"

struct mmi_module_t;
typedef int32_t(*cb_print_t) (const char *module_name, const char *subcmd, const char *str, uint32_t size,
                              print_type_t type);

typedef struct mmi_module_methods_t {

    /** Open a specific device */
    int32_t(*module_init) (const mmi_module_t * module, hash_map < string, string > &params);

    /** Close this device */
    int32_t(*module_deinit) (const mmi_module_t * module);

    /** Run one cmd with param */
    int32_t(*module_run) (const mmi_module_t * module, const char *cmd, hash_map < string, string > &params);

    /** Stop the current run command */
    int32_t(*module_stop) (const mmi_module_t * module);

} mmi_module_methods_t;

/**
 * Every mmi module must have a data structure named MMI_MODULE_INFO_SYM
 * and the fields of this data structure must begin with mmi_module_t
 * followed by module specific information.
 */
typedef struct mmi_module_t {
    /**  Module version */
    uint16_t version_major;
    uint16_t version_minor;

    /** Name of module */
    const char *name;

    /** Author/owner/implementor of the module */
    const char *author;

    /** Module's methods */
    mmi_module_methods_t *methods;

    /** Module's handle */
    void *module_handle;

    /** Module's supported command list and it's size */
    const char **supported_cmd_list;
    uint32_t supported_cmd_list_size;

    /** Module's Callback function for send message to server
     *   The message will show in screen
     */
    cb_print_t cb_print;

    /** Module created Thread pid for Run command */
    pthread_t run_pid;
} mmi_module_t;

#endif                          /* MMI_MODULE_INCLUDE_H */
