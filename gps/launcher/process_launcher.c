/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*D

  Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
=============================================================================*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>
#include <utils/Log.h>
#include <loc_target.h>
#include <private/android_filesystem_config.h>
#include <loc_cfg.h>
#include <log_util.h>
#include <sys/prctl.h>
#include <loc_misc_utils.h>
#include <sys/capability.h>
#include <cutils/properties.h>
#include "izat_modes.h"
#define LOG_TAG "LocSvc_launcher"

#define GPS_CONF_FILE "/etc/gps.conf"
#define IZAT_CONF_FILE "/etc/izat.conf"

#define MAX_NUM_GROUPS 20
#define MAX_CHILD_PROC_RESTARTS 5
#define MAX_PARAM_NAME_LENGTH 64
/*Length of process arguments in conf file*/
#define MAX_PARAM_ARGUMENT_LENGTH 100

#define MAX_NUM_STRINGS   20
#define MAX_NUM_ARGS      25

//We can have 8 masks for now
#define CONFIG_MASK_TARGET_ALL           0X01
#define CONFIG_MASK_TARGET_FOUND         0X02
#define CONFIG_MASK_TARGET_CHECK         0X03
#define CONFIG_MASK_BASEBAND_ALL         0X04
#define CONFIG_MASK_BASEBAND_FOUND       0X08
#define CONFIG_MASK_BASEBAND_CHECK       0x0c

#define IZAT_FEATURE_MASK_GTP_WIFI_BASIC            0x01
#define IZAT_FEATURE_MASK_GTP_WIFI_PREMIUM          0X02
#define IZAT_FEATURE_MASK_GTP_CELL_BASIC            0X04
#define IZAT_FEATURE_MASK_GTP_CELL_PREMIUM          0X08
#define IZAT_FEATURE_MASK_GTP_AP_CELL_BASIC         IZAT_FEATURE_MASK_GTP_CELL_BASIC
#define IZAT_FEATURE_MASK_GTP_AP_CELL_PREMIUM       IZAT_FEATURE_MASK_GTP_CELL_PREMIUM
#define IZAT_FEATURE_MASK_PIP_BASIC                 0X10
#define IZAT_FEATURE_MASK_PIP_PREMIUM               0X20
#define IZAT_FEATURE_MASK_SAP_BASIC                 0x40
#define IZAT_FEATURE_MASK_SAP_PREMIUM               0X80
#define IZAT_FEATURE_MASK_GTP_WAA_BASIC             0X100
#define IZAT_FEATURE_MASK_GTP_WAA_PREMIUM           0x200
#define IZAT_FEATURE_MASK_GTP_MODEM_CELL_BASIC      0X400
#define IZAT_FEATURE_MASK_GTP_MODEM_CELL_PREMIUM    0X800

#define ONE_DAY_SECONDS 86400

static unsigned char sigchld_received=0;
static unsigned char sigalrm_received=0;

typedef enum {
    ENABLED,
    RUNNING,
    DISABLED,
    DISABLED_FROM_CONF
}e_process_status;

typedef struct {
    e_process_status proc_status;
    pid_t proc_id;
    unsigned char num_restarts;
    char *name[2];
    gid_t group_list[MAX_NUM_GROUPS];
    unsigned char num_groups;
    char *args[MAX_NUM_ARGS];
    char argumentString[MAX_PARAM_ARGUMENT_LENGTH];
}s_proc_info;

static void sig_handler_chld(int signal_num)
{
    if(signal_num == SIGCHLD) {
        LOC_LOGE("%s:%d]: Received SIGCHLD\n", __func__, __LINE__);
        sigchld_received = 1;
    }
    else {
        LOC_LOGE("%s:%d]: Received signal %s\n", __func__, __LINE__, strsignal(signal_num));
    }
}

static void sig_handler_alrm(int signal_num)
{
    if(signal_num == SIGALRM) {
        LOC_LOGE("%s:%d]: Received SIGALRM\n", __func__, __LINE__);
        sigalrm_received = 1;
    }
    else {
        LOC_LOGE("%s:%d]: Received signal %s\n", __func__, __LINE__, strsignal(signal_num));
    }
}

int main(int argc, char **argv)
{
    //To be passed in to waitpid
    int child_status;
    int child_exit_status=0;

    struct sigaction sigact_chld, sigact_alrm;
    s_proc_info *child_proc;
    volatile int i=0;
    unsigned int j=0, k=0;
    gid_t gid_list[MAX_NUM_GROUPS];
    sigset_t newmask, oldmask;
    char *split_strings[MAX_NUM_STRINGS];
    int name_length=0, group_list_length=0, platform_length=0, baseband_length=0, ngroups=0, ret=0;
    int group_index=0, nstrings=0, status_length=0;
    FILE* conf_fp;
    char platform_name[PROPERTY_VALUE_MAX], baseband_name[PROPERTY_VALUE_MAX];
    int lean_target=0;
    unsigned int izat_service_mask=0;
    char config_mask = 0;
    unsigned char proc_list_length=0;
    char conf_proc_name[MAX_PARAM_NAME_LENGTH];
    char conf_proc_argument[MAX_PARAM_ARGUMENT_LENGTH];
    char conf_proc_status[MAX_PARAM_NAME_LENGTH];
    char conf_group_list[MAX_PARAM_NAME_LENGTH];
    unsigned int conf_premium_feature=0;
    unsigned int conf_izat_feature_mask=0;
    char conf_platform_list[MAX_PARAM_NAME_LENGTH];
    char conf_baseband[MAX_PARAM_NAME_LENGTH];
    char conf_lean_targets[MAX_PARAM_NAME_LENGTH];
    unsigned int conf_sglte_target=0;
    char conf_feature_gtp_cell_proc[MAX_PARAM_NAME_LENGTH];
    char conf_feature_gtp_waa[MAX_PARAM_NAME_LENGTH];
    char conf_feature_gtp_cell[MAX_PARAM_NAME_LENGTH];
    char conf_feature_gtp_wifi[MAX_PARAM_NAME_LENGTH];
    char conf_feature_pip[MAX_PARAM_NAME_LENGTH];
    char conf_feature_sap[MAX_PARAM_NAME_LENGTH];
    char arg_gtp_waa[IZAT_MAX_ARG_STR_LENGTH] = "--";
    char arg_gtp_ap_cell[IZAT_MAX_ARG_STR_LENGTH] = "--";
    char arg_gtp_modem_cell[IZAT_MAX_ARG_STR_LENGTH] = "--";
    char arg_gtp_wifi[IZAT_MAX_ARG_STR_LENGTH] = "--";
    char arg_pip[IZAT_MAX_ARG_STR_LENGTH] = "--";
    char arg_sap[IZAT_MAX_ARG_STR_LENGTH] = "--";
    char arg_disabled[IZAT_MAX_ARG_STR_LENGTH] = IZAT_MODE_DISABLED;
    char arg_basic[IZAT_MAX_ARG_STR_LENGTH] = IZAT_MODE_BASIC;
    char arg_premium[IZAT_MAX_ARG_STR_LENGTH] = IZAT_MODE_PREMIUM;
    int gtp_cell_ap_enabled = 0;

    /* gps.conf Parameter spec table */
    loc_param_s_type gps_conf_parameter_table[] =
    {
        {"SGLTE_TARGET",        &conf_sglte_target,           NULL, 'n'},
    };
    /* izat.conf feature mode table*/
    loc_param_s_type izat_conf_feature_table[] =
    {
        {"GTP_CELL_PROC", &conf_feature_gtp_cell_proc, NULL, 's'},
        {"GTP_CELL",      &conf_feature_gtp_cell,      NULL, 's'},
        {"GTP_WIFI",      &conf_feature_gtp_wifi,      NULL, 's'},
        {"GTP_WAA",       &conf_feature_gtp_waa,       NULL, 's'},
        {"PIP",           &conf_feature_pip,           NULL, 's'},
        {"SAP",           &conf_feature_sap,           NULL, 's'}
    };
    /* izat.conf Parameter spec table */
    loc_param_s_type izat_conf_parameter_table[] =
    {
        {"PROCESS_NAME",        &conf_proc_name,           NULL, 's'},
        {"PROCESS_ARGUMENT",    &conf_proc_argument,       NULL, 's'},
        {"PROCESS_STATE",       &conf_proc_status,         NULL, 's'},
        {"PROCESS_GROUPS",      &conf_group_list,          NULL, 's'},
        {"PREMIUM_FEATURE",     &conf_premium_feature,     NULL, 'n'},
        {"IZAT_FEATURE_MASK",   &conf_izat_feature_mask,   NULL, 'n'},
        {"PLATFORMS",           &conf_platform_list,       NULL, 's'},
        {"BASEBAND",            &conf_baseband,            NULL, 's'},
        {"LEAN_TARGETS",        &conf_lean_targets,        NULL, 's'},
    };

    struct __user_cap_header_struct cap_hdr = {0};
    struct __user_cap_data_struct cap_data = {0};

    cap_hdr.version = _LINUX_CAPABILITY_VERSION;
    cap_hdr.pid = 0;

    if(prctl(PR_SET_KEEPCAPS, 1) < 0) {
        LOC_LOGE("%s:%d]: Error: prctl failed. %s\n",
                 __func__, __LINE__, strerror(errno));
        ret = -1;
        goto err_capset;
    }
    if(setuid(AID_GPS) == -1) {
        LOC_LOGE("%s:%d]: Error: setuid failed. %s\n",
                 __func__, __LINE__, strerror(errno));
        ret = -1;
        goto err_capset;
    }
    //Read gps.conf and fill parameter table
    UTIL_READ_CONF(GPS_CONF_FILE, gps_conf_parameter_table);

    cap_data.permitted = (1<<CAP_SETUID) | (1<<CAP_SETGID);
    cap_data.effective = cap_data.permitted;

    LOC_LOGD("cap_data.permitted: %d", (int)cap_data.permitted);

    if(capset(&cap_hdr, &cap_data)) {
        LOC_LOGE("%s:%d]: Error: capset failed. %s\n",
                 __func__, __LINE__, strerror(errno));
        ret = -1;
        goto err_capset;
    }

    LOC_LOGD("Parent- pid: %d, uid: %d, euid: %d, gid: %d, egid: %d\n",
             getpid(), getuid(), geteuid(), getgid(), getegid());
    //Form argument strings
    strncat(arg_gtp_waa, IZAT_FEATURE_GTP_WAA, IZAT_MAX_ARG_STR_LENGTH-3);
    strncat(arg_gtp_ap_cell, IZAT_FEATURE_GTP_AP_CELL, IZAT_MAX_ARG_STR_LENGTH-3);
    strncat(arg_gtp_modem_cell, IZAT_FEATURE_GTP_MODEM_CELL, IZAT_MAX_ARG_STR_LENGTH-3);
    strncat(arg_gtp_wifi, IZAT_FEATURE_GTP_WIFI, IZAT_MAX_ARG_STR_LENGTH-3);
    strncat(arg_pip, IZAT_FEATURE_PIP, IZAT_MAX_ARG_STR_LENGTH-3);
    strncat(arg_sap, IZAT_FEATURE_SAP, IZAT_MAX_ARG_STR_LENGTH-3);

    //Get platform name from ro.board.platform property
    loc_get_platform_name(platform_name, sizeof(platform_name));
    //Get baseband name from ro.baseband property
    loc_get_target_baseband(baseband_name, sizeof(baseband_name));
    //Identify if this is a lean and mean target from ro.lean property
    lean_target = loc_identify_lean_target(lean_target, sizeof(lean_target));
    LOC_LOGD("%s:%d]: lean target:%d", __func__, __LINE__, lean_target);

    UTIL_READ_CONF(IZAT_CONF_FILE, izat_conf_feature_table);

    //Set service mask for GTP_WIFI
    if(strcmp(conf_feature_gtp_wifi, "DISABLED") == 0) {
        LOC_LOGD("%s:%d]: GTP WIFI DISABLED", __func__, __LINE__);
    }
    else if(strcmp(conf_feature_gtp_wifi, "BASIC") == 0) {
        LOC_LOGD("%s:%d]: Setting GTP WIFI to mode: BASIC", __func__, __LINE__);
        izat_service_mask |= IZAT_FEATURE_MASK_GTP_WIFI_BASIC;
    }
    //izat.conf has a garbage value
    else {
        LOC_LOGE("%s:%d]: Unrecognized value for GTP WIFI Mode."\
                 " Setting GTP WIFI to default mode: BASIC", __func__, __LINE__);
        izat_service_mask |= IZAT_FEATURE_MASK_GTP_WIFI_BASIC;
    }

    //Set service mask for GTP_CELL
    //Using a temp variable here to indicate wheter GTP cell is
    //enabled on the AP or modem. This variable will be used in
    //further checks below. An alternative was to compare the
    //string again in each place which would've been more expensive
    if(strcmp(conf_feature_gtp_cell_proc, "AP") == 0) {
        gtp_cell_ap_enabled = 1;
    }

    if(strcmp(conf_feature_gtp_cell, "PREMIUM") == 0) {
        LOC_LOGE("%s:%d]: Error: Izat feature GTP CELL does not support PREMIUM mode" \
                 " available modes are BASIC and DISABLED. Starting feature in BASIC mode",
                 __func__, __LINE__);
        if(gtp_cell_ap_enabled) {
            izat_service_mask |= IZAT_FEATURE_MASK_GTP_AP_CELL_BASIC;
        }
        else {
            izat_service_mask |= IZAT_FEATURE_MASK_GTP_MODEM_CELL_BASIC;
        }
    }
    else if(strcmp(conf_feature_gtp_cell, "BASIC") == 0) {
            LOC_LOGD("%s:%d]: Setting GTP CELL to mode: BASIC", __func__, __LINE__);
        if(gtp_cell_ap_enabled) {
            izat_service_mask |= IZAT_FEATURE_MASK_GTP_AP_CELL_BASIC;
        }
        else {
            izat_service_mask |= IZAT_FEATURE_MASK_GTP_MODEM_CELL_BASIC;
        }
    }
    else if(strcmp(conf_feature_gtp_cell, "DISABLED") == 0) {
            LOC_LOGD("%s:%d]: GTP CELL DISABLED", __func__, __LINE__);
    }
    //izat.conf has a garbage value
    else {
        LOC_LOGE("%s:%d]: Unrecognized value for GTP CELL Mode."        \
                 " Setting GTP CELL to default mode: BASIC", __func__, __LINE__);
        if(gtp_cell_ap_enabled) {
            izat_service_mask |= IZAT_FEATURE_MASK_GTP_AP_CELL_BASIC;
        }
        else {
            izat_service_mask |= IZAT_FEATURE_MASK_GTP_MODEM_CELL_BASIC;
        }
    }

    //Set service mask for GTP_WAA
    if(strcmp(conf_feature_gtp_waa, "PREMIUM") == 0) {
        LOC_LOGE("%s:%d]: Error: Izat feature GTP WAA does not support PREMIUM mode" \
                 " available modes are BASIC and DISABLED. Starting feature in BASIC mode",
                 __func__, __LINE__);
        izat_service_mask |= IZAT_FEATURE_MASK_GTP_WAA_BASIC;
    }
    else if(strcmp(conf_feature_gtp_waa, "BASIC") == 0) {
        LOC_LOGD("%s:%d]: Setting GTP WAA to mode: BASIC", __func__, __LINE__);
        izat_service_mask |= IZAT_FEATURE_MASK_GTP_WAA_BASIC;
    }
    else if(strcmp(conf_feature_gtp_waa, "DISABLED") == 0) {
        LOC_LOGD("%s:%d]: GTP WAA DISABLED", __func__, __LINE__);
    }
    //izat.conf has a garbage value
    else {
        LOC_LOGE("%s:%d]: Unrecognized value for GTP WAA Mode."\
                 " Setting GTP WAA to default mode: DISABLED", __func__, __LINE__);
    }

    //Set service mask for PIP
    if(strcmp(conf_feature_pip, "PREMIUM") == 0) {
        LOC_LOGD("%s:%d]: Setting PIP to mode: PREMIUM", __func__, __LINE__);
        izat_service_mask |= IZAT_FEATURE_MASK_PIP_PREMIUM;
    }
    else if(strcmp(conf_feature_pip, "BASIC") == 0) {
        LOC_LOGE("%s:%d]: Error: Izat feature PIP does not support BASIC mode"\
                 " available modes are PREMIUM and DISABLED. Default to mode: DISABLED",
                 __func__, __LINE__);
    }
    else if(strcmp(conf_feature_pip, "DISABLED") == 0) {
        LOC_LOGD("%s:%d]: PIP DISABLED", __func__, __LINE__);
    }
    else {
       LOC_LOGE("%s:%d]: Unrecognized value for PIP Mode."\
                " Setting PIP to default mode: DISABLED", __func__, __LINE__);
    }

    //Set service mask for SAP
    if(strcmp(conf_feature_sap, "PREMIUM") == 0) {
        LOC_LOGD("%s:%d]: Setting SAP to mode: PREMIUM", __func__, __LINE__);
        izat_service_mask |= IZAT_FEATURE_MASK_SAP_PREMIUM;
    }
    else if(strcmp(conf_feature_sap, "BASIC") == 0) {
        LOC_LOGD("%s:%d]: Setting SAP to mode: BASIC", __func__, __LINE__);
        izat_service_mask |= IZAT_FEATURE_MASK_SAP_BASIC;
    }
    else if(strcmp(conf_feature_sap, "DISABLED") == 0) {
        LOC_LOGD("%s:%d]: Setting SAP to mode: DISABLED", __func__, __LINE__);
    }
    else {
       LOC_LOGE("%s:%d]: Unrecognized value for SAP Mode."\
                " Setting SAP to default mode: BASIC", __func__, __LINE__);
       izat_service_mask |= IZAT_FEATURE_MASK_SAP_BASIC;
    }

    LOC_LOGD("%s:%d]: izat_service_mask: %x\n", __func__, __LINE__, izat_service_mask);

    //Set signal handler for SIGCHLD
    memset(&sigact_chld, 0, sizeof(struct sigaction));
    sigact_chld.sa_flags = SA_SIGINFO;
    sigemptyset(&sigact_chld.sa_mask);
    sigact_chld.sa_handler = sig_handler_chld;
    if(sigaction(SIGCHLD, &sigact_chld, NULL) == -1) {
        LOC_LOGE("Error setting handler for SIGCHLD\n");
    }
    //Set signal handler for SIGALRM
    memset(&sigact_alrm, 0, sizeof(struct sigaction));
    sigact_alrm.sa_flags = SA_SIGINFO;
    sigemptyset(&sigact_alrm.sa_mask);
    sigact_alrm.sa_handler = sig_handler_alrm;
    if(sigaction(SIGALRM, &sigact_alrm, NULL) == -1) {
        LOC_LOGE("Error setting handler for SIGALRM\n");
    }

    //Read current group subscriptions
    memset(gid_list, 0, sizeof(gid_list));
    ngroups = getgroups(MAX_NUM_GROUPS, gid_list);
    if(ngroups == -1) {
        LOC_LOGE("Could not find groups. ngroups:%d\n", ngroups);
    }
    else {
        IF_LOC_LOGD {
            LOC_LOGD("GIDs from parent: ");
            for(ngroups = 0; ngroups<MAX_NUM_GROUPS; ngroups++) {
                if(gid_list[ngroups]) {
                    LOC_LOGD("%d ", gid_list[ngroups]);
                }
            }
            LOC_LOGD("\n");
        }
    }

    if((conf_fp = fopen(IZAT_CONF_FILE, "r")) == NULL) {
        LOC_LOGE("%s:%d]: Error opening %s %s\n", __func__,
                 __LINE__, IZAT_CONF_FILE, strerror(errno));
        ret = -1;
        goto err_conf_file;
    }

    //Parse through the file to find out how many processes are to be launched
    proc_list_length = 0;
    do {
        conf_proc_name[0] = 0;
        //Here note that the 3rd parameter is passed as 1.
        //This is so that only the first parameter in the table which is "PROCESS_NAME"
        //is read. We do not want to read the entire block of parameters at this time
        //since we are only counting the number of processes to launch.
        //Therefore, only counting the occurrences of PROCESS_NAME parameter
        //should suffice
        if(loc_read_conf_r(conf_fp, izat_conf_parameter_table, 1)) {
            LOC_LOGE("%s:%d]: Unable to read conf file. Failing\n", __func__, __LINE__);
            ret = -1;
            goto err_conf_file;
        }
        name_length=(int)strlen(conf_proc_name);
        if(name_length) {
            proc_list_length++;
            LOC_LOGD("Process name:%s", conf_proc_name);
        }
    } while(name_length);
    LOC_LOGD("proc_list_length = %d\n", proc_list_length);

    child_proc = (s_proc_info *)calloc(proc_list_length, sizeof(s_proc_info));
    if(child_proc == NULL) {
        LOC_LOGE("%s:%d]: ERROR: Malloc returned NULL\n", __func__, __LINE__);
        ret = -1;
        goto err_capset;
    }

    //Move file descriptor to the beginning of the file
    //so that the parameters can be read
    rewind(conf_fp);

    for(j=0; j<proc_list_length; j++) {
        //Set defaults for all the child process structs
        child_proc[j].proc_status = DISABLED;
        child_proc[j].num_restarts = 0;
        memset(child_proc[j].group_list, 0, sizeof(child_proc[j].group_list));
        memset(child_proc[j].args, 0, sizeof(child_proc[j].args));
        config_mask=0;
        if(loc_read_conf_r(conf_fp, izat_conf_parameter_table,
                           sizeof(izat_conf_parameter_table)/sizeof(izat_conf_parameter_table[0]))) {
            LOC_LOGE("%s:%d]: Unable to read conf file. Failing\n", __func__, __LINE__);
            ret = -1;
            goto err;
        }

        name_length=(int)strlen(conf_proc_name);
        group_list_length=(int)strlen(conf_group_list);
        platform_length = (int)strlen(conf_platform_list);
        baseband_length = (int)strlen(conf_baseband);
        status_length = (int)strlen(conf_proc_status);

        if(!name_length || !group_list_length || !platform_length ||
           !baseband_length || !status_length) {
            LOC_LOGE("%s:%d]: Error: i: %d; One of the parameters not specified in conf file",
                     __func__, __LINE__, i);
            continue;
        }

        if(access(conf_proc_name, X_OK)) {
            LOC_LOGV("The binary %s is not present or is not executable. Marking as DISABLED",
                     conf_proc_name);
            child_proc[j].proc_status = DISABLED;
            continue;
        }
        if(strcmp(conf_proc_status, "DISABLED") == 0) {
            LOC_LOGD("%s:%d]: Process %s is disabled in conf file",
                     __func__, __LINE__, conf_proc_name);
            child_proc[j].proc_status = DISABLED_FROM_CONF;
            continue;
        }
        else if(strcmp(conf_proc_status, "ENABLED") == 0) {
            LOC_LOGD("%s:%d]: Process %s is enabled in conf file",
                     __func__, __LINE__, conf_proc_name);
        }

        child_proc[j].name[0] = (char *)malloc(MAX_PARAM_NAME_LENGTH);
        if(child_proc[j].name[0] == NULL) {
            LOC_LOGE("%s:%d]: ERROR: Malloc returned NULL\n", __func__, __LINE__);
            ret = -1;
            goto err;
        }
        //Since strlcpy copies length-1 characters, we add 1 to name_length
        if((name_length+1) > MAX_PARAM_NAME_LENGTH) {
            LOC_LOGE("%s:%d]: i: %d; Length of name parameter too long. Max length: %d",
                     __func__, __LINE__, i, MAX_PARAM_NAME_LENGTH);
            continue;
        }
        strlcpy(child_proc[j].name[0], conf_proc_name, MAX_PARAM_NAME_LENGTH);
        child_proc[j].name[1] = NULL;

        child_proc[j].num_groups = 0;
        ngroups = loc_util_split_string(conf_group_list, split_strings, MAX_NUM_STRINGS, ' ');
        for(i=0; i<ngroups; i++) {
            for(k=0; k<android_id_count; k++) {
                if(strcmp(android_ids[k].name, split_strings[i]) == 0) {
                    child_proc[j].group_list[i] = android_ids[k].aid;
                    child_proc[j].num_groups++;
                    LOC_LOGD("%s:%d]:Group %s = %d matches child_group: %d\n",
                             __func__, __LINE__, split_strings[i],
                             android_ids[k].aid,child_proc[j].group_list[i]);
                }
            }
        }

        nstrings = loc_util_split_string(conf_platform_list, split_strings, MAX_NUM_STRINGS, ' ');
        if(strcmp("all", split_strings[0]) == 0) {
            LOC_LOGD("%s:%d]: Enabled for all targets\n", __func__, __LINE__);
            config_mask |= CONFIG_MASK_TARGET_ALL;
        }
        else {
            for(i=0; i<nstrings; i++) {
                if(strcmp(platform_name, split_strings[i]) == 0) {
                    LOC_LOGD("%s:%d]: Matched platform: %s\n",
                             __func__, __LINE__, split_strings[i]);
                    config_mask |= CONFIG_MASK_TARGET_FOUND;
                    break;
                }
            }
        }

        nstrings = loc_util_split_string(conf_baseband, split_strings, MAX_NUM_STRINGS, ' ');
        if(strcmp("all", split_strings[0]) == 0) {
            LOC_LOGD("%s:%d]: Enabled for all basebands\n", __func__, __LINE__);
            config_mask |= CONFIG_MASK_BASEBAND_ALL;
        }
        else {
            for(i=0; i<nstrings; i++) {
                if(strcmp(baseband_name, split_strings[i]) == 0) {
                    LOC_LOGD("%s:%d]: Matched baseband: %s\n",
                             __func__, __LINE__, split_strings[i]);
                    config_mask |= CONFIG_MASK_BASEBAND_FOUND;
                    break;
                }
                //Since ro.baseband is not a reliable source for detecting sglte
                //the alternative is to read the SGLTE_TARGET parameter from gps.conf
                //this parameter is read into conf_sglte_target
                else if((strcmp("sglte", split_strings[i]) == 0 ) && conf_sglte_target) {
                    LOC_LOGD("%s:%d]: Matched baseband SGLTE\n", __func__, __LINE__);
                    config_mask |= CONFIG_MASK_BASEBAND_FOUND;
                    break;
                }
            }
        }

        nstrings = loc_util_split_string(conf_lean_targets, split_strings, MAX_NUM_STRINGS, ' ');
        if(!strcmp("DISABLED", split_strings[0]) && lean_target) {
            LOC_LOGD("%s:%d]: Disabled for lean targets\n", __func__, __LINE__);
            child_proc[j].proc_status = DISABLED;
            continue;
        }

        if((config_mask & CONFIG_MASK_TARGET_CHECK) &&
           (config_mask & CONFIG_MASK_BASEBAND_CHECK) &&
           (child_proc[j].proc_status != DISABLED_FROM_CONF)) {

            //Set args
            //The first argument passed through argv is usually the name of the
            //binary when started from commandline.
            //getopt() seems to ignore this first argument and hence we assign it
            //to the process name for consistency with command line args
            i = 0;
            child_proc[j].args[i++] = child_proc[j].name[0];

            if(conf_premium_feature) {
               if(conf_izat_feature_mask & izat_service_mask) {
                    LOC_LOGD("%s:%d]: Enabled. %s has service mask: %x\n",
                             __func__, __LINE__, child_proc[j].name[0], conf_izat_feature_mask);
                    child_proc[j].proc_status = ENABLED;

                    if(conf_izat_feature_mask &
                       (IZAT_FEATURE_MASK_GTP_WIFI_BASIC | IZAT_FEATURE_MASK_GTP_WIFI_PREMIUM)) {
                        if(izat_service_mask & IZAT_FEATURE_MASK_GTP_WIFI_BASIC) {
                            child_proc[j].args[i++] = arg_gtp_wifi;
                            child_proc[j].args[i++] = arg_basic;
                        }
                        else if(izat_service_mask & IZAT_FEATURE_MASK_GTP_WIFI_PREMIUM) {
                            child_proc[j].args[i++] = arg_gtp_wifi;
                            child_proc[j].args[i++] = arg_premium;
                        }
                    }
                    if(conf_izat_feature_mask &
                       (IZAT_FEATURE_MASK_GTP_CELL_BASIC | IZAT_FEATURE_MASK_GTP_CELL_PREMIUM )) {
                        if(izat_service_mask & IZAT_FEATURE_MASK_GTP_AP_CELL_BASIC){
                            child_proc[j].args[i++] = arg_gtp_ap_cell;
                            child_proc[j].args[i++] = arg_basic;
                            child_proc[j].args[i++] = arg_gtp_modem_cell;
                            child_proc[j].args[i++] = arg_disabled;
                        }
                        else if(izat_service_mask & IZAT_FEATURE_MASK_GTP_AP_CELL_PREMIUM){
                            child_proc[j].args[i++] = arg_gtp_ap_cell;
                            child_proc[j].args[i++] = arg_premium;
                            child_proc[j].args[i++] = arg_gtp_modem_cell;
                            child_proc[j].args[i++] = arg_disabled;
                        }
                        else if(izat_service_mask & IZAT_FEATURE_MASK_GTP_MODEM_CELL_BASIC) {
                            child_proc[j].args[i++] = arg_gtp_modem_cell;
                            child_proc[j].args[i++] = arg_basic;
                            child_proc[j].args[i++] = arg_gtp_ap_cell;
                            child_proc[j].args[i++] = arg_disabled;
                        }
                        else if(izat_service_mask & IZAT_FEATURE_MASK_GTP_MODEM_CELL_PREMIUM) {
                            child_proc[j].args[i++] = arg_gtp_modem_cell;
                            child_proc[j].args[i++] = arg_premium;
                            child_proc[j].args[i++] = arg_gtp_ap_cell;
                            child_proc[j].args[i++] = arg_disabled;
                        }
                        else {
                             child_proc[j].args[i++] = arg_gtp_ap_cell;
                             child_proc[j].args[i++] = arg_disabled;
                             child_proc[j].args[i++] = arg_gtp_modem_cell;
                             child_proc[j].args[i++] = arg_disabled;
                       }
                    }
                    if(conf_izat_feature_mask &
                       (IZAT_FEATURE_MASK_GTP_WAA_BASIC | IZAT_FEATURE_MASK_GTP_WAA_PREMIUM)) {
                        if(izat_service_mask & IZAT_FEATURE_MASK_GTP_WAA_BASIC) {
                            child_proc[j].args[i++] = arg_gtp_waa;
                            child_proc[j].args[i++] = arg_basic;
                        }
                        else if(izat_service_mask & IZAT_FEATURE_MASK_GTP_WAA_PREMIUM) {
                            child_proc[j].args[i++] = arg_gtp_waa;
                            child_proc[j].args[i++] = arg_premium;
                        }
                    }
                    if(conf_izat_feature_mask &
                       (IZAT_FEATURE_MASK_PIP_BASIC | IZAT_FEATURE_MASK_PIP_PREMIUM)) {
                        if(izat_service_mask & IZAT_FEATURE_MASK_PIP_BASIC) {
                            child_proc[j].args[i++] = arg_pip;
                            child_proc[j].args[i++] = arg_basic;
                        }
                        else if(izat_service_mask & IZAT_FEATURE_MASK_PIP_PREMIUM) {
                            child_proc[j].args[i++] = arg_pip;
                            child_proc[j].args[i++] = arg_premium;
                        }
                    }
                    if(conf_izat_feature_mask &
                       (IZAT_FEATURE_MASK_SAP_BASIC | IZAT_FEATURE_MASK_SAP_PREMIUM)) {
                        if(izat_service_mask & IZAT_FEATURE_MASK_SAP_BASIC) {
                            child_proc[j].args[i++] = arg_sap;
                            child_proc[j].args[i++] = arg_basic;
                        }
                        else if(izat_service_mask & IZAT_FEATURE_MASK_SAP_PREMIUM) {
                            child_proc[j].args[i++] = arg_sap;
                            child_proc[j].args[i++] = arg_premium;
                        }
                    }
                    IF_LOC_LOGD {
                        LOC_LOGD("%s:%d]: %s args\n", __func__, __LINE__, child_proc[j].name[0]);
                        for(i=0; i<MAX_NUM_ARGS; i++) {
                            if(child_proc[j].args[i]) {
                                LOC_LOGD("%s:%d]: i: %d, %s\n", __func__, __LINE__, i,
                                         child_proc[j].args[i]);
                            }
                        }
                        LOC_LOGD("%s:%d]: \n", __func__, __LINE__);
                    }
                }
                else {
                    LOC_LOGD("%s:%d]: Disabled. %s has service mask:  %x \n",
                             __func__, __LINE__, child_proc[j].name[0], conf_izat_feature_mask);
                }
            }
            else {
                LOC_LOGD("%s:%d]: %s not a premium feature. Enabled\n",
                         __func__, __LINE__, child_proc[j].name[0]);
                child_proc[j].proc_status = ENABLED;
            }

            /*Fill up the remaining arguments from configuration file*/
            LOC_LOGD("%s] Parsing Process_Arguments from Configuration: %s \n",
                      __func__, conf_proc_argument);
            if(0 != conf_proc_argument[0])
            {
                /**************************************
                ** conf_proc_argument is shared by all the programs getting launched,
                ** hence copy to process specific argument string and parse the same.
                ***************************************/
                strncpy(child_proc[j].argumentString,conf_proc_argument,strlen(conf_proc_argument));
                loc_util_split_string(child_proc[j].argumentString, &child_proc[j].args[i],
                                      (MAX_NUM_ARGS - i), ' ');
            }
        }
        else {
            LOC_LOGD("%s:%d]: Process %s is disabled\n",
                     __func__, __LINE__, child_proc[j].name[0]);
        }
    }

    fclose(conf_fp);
    //Set an alarm to be woken up after 24 hours to clear
    //the number of restarts for active child processes
    alarm(ONE_DAY_SECONDS);

    //Start recurring loop. This loop does the following things:
    //1. Check if SIGCHLD is received and re-enable/disable child processes
    //2. Check if SIGALRM is received and clear restart counter
    //3. Launch enabled child processes
    while(1) {
        //Check if any of the child processes have exited
        if(sigchld_received) {
            //Loop through the list of child processes and figure out which
            //process has exited
            for(i=0; i<proc_list_length; i++) {
                child_status = 0;
                child_exit_status = 0;
                if(child_proc[i].proc_status == RUNNING) {
                    LOC_LOGD("%s:%d]: Child_id: %d; child name: %s\n",
                             __func__, __LINE__, child_proc[i].proc_id, child_proc[i].name[0]);
                    if(waitpid(child_proc[i].proc_id, &child_status, WNOHANG) ==
                       child_proc[i].proc_id) {
                        if(WIFEXITED(child_status)) {
                            child_exit_status = WEXITSTATUS(child_status);
                            if(child_exit_status) {
                                LOC_LOGE("%s:%d]: Exit status: %d. Restarting\n",
                                         __func__, __LINE__, child_exit_status);
                                if(child_proc[i].num_restarts == MAX_CHILD_PROC_RESTARTS) {
                                    child_proc[i].proc_status = DISABLED;
                                    LOC_LOGE("%s:%d]: %s Restarted too many times. Grounded!\n",
                                             __func__, __LINE__, child_proc[i].name[0]);
                                }
                                else {
                                    child_proc[i].num_restarts++;
                                    LOC_LOGE("%s:%d]: Restarting %s. Number of restarts: %d\n",
                                             __func__, __LINE__, child_proc[i].name[0],
                                             child_proc[i].num_restarts);
                                    child_proc[i].proc_status = ENABLED;
                                }
                            }
                            else {
                                LOC_LOGE("%s:%d]: Exit status 0. Not Restarting\n",
                                         __func__, __LINE__);
                                child_proc[i].proc_status = DISABLED;
                            }
                        }
                        else if(WIFSIGNALED(child_status)) {
                            LOC_LOGE("%s:%d]: Child exited due to signal: %s\n",
                                     __func__, __LINE__, strsignal(WTERMSIG(child_status)));
                            if(child_proc[i].num_restarts == MAX_CHILD_PROC_RESTARTS) {
                                child_proc[i].proc_status = DISABLED;
                                LOC_LOGE("%s:%d]: %s Restarted too many times. Grounded!\n",
                                         __func__, __LINE__, child_proc[i].name[0]);
                            }
                            else {
                                child_proc[i].num_restarts++;
                                LOC_LOGE("%s:%d]: Restarting %s. Number of restarts: %d\n",
                                         __func__, __LINE__, child_proc[i].name[0],
                                         child_proc[i].num_restarts);
                                child_proc[i].proc_status = ENABLED;
                            }
                        }
                        else {
                            LOC_LOGE("%s:%d]:No Reason found\n", __func__, __LINE__);
                        }
                    }
                    else {
                        LOC_LOGD("%s:%d]: Either the %s did not exit or waitpid returned error\n",
                                 __func__, __LINE__, child_proc[i].name[0]);
                    }
                }
            }
            //Clear the flag to be set in the signal handler
            sigchld_received = 0;
        }
        if(sigalrm_received) {
            LOC_LOGD("%s:%d]:sigalrm_received\n",__func__, __LINE__);
            for(i=0; i<proc_list_length; i++) {
                if((child_proc[i].proc_status == RUNNING) ||
                   (child_proc[i].proc_status == ENABLED)) {
                    LOC_LOGD("%s:%d]: Child_id: %d; child name: %s; num_restarts: %d\n",
                             __func__, __LINE__, child_proc[i].proc_id, child_proc[i].name[0],
                             child_proc[i].num_restarts);
                    child_proc[i].num_restarts = 0;
                }
            }
            sigalrm_received = 0;
            alarm(ONE_DAY_SECONDS);
        }

        //Start making children
        for(i=0; i<proc_list_length;i++) {
            if(child_proc[i].proc_status == ENABLED) {
                LOC_LOGD("%s:%d]: Forking child: %d, name: %s",
                         __func__, __LINE__, i, child_proc[i].name[0]);
                if((child_proc[i].proc_id = fork()) < 0) {
                    LOC_LOGE("%s: fork of child failed:%s\n",
                          child_proc[i].name[0], strerror(errno));
                    exit(1);
                }
                //This block executed by child process
                else if(!child_proc[i].proc_id) {
                    LOC_LOGD("Child born- uid: %d, euid: %d, gid: %d, egid: %d\n",
                             getuid(), geteuid(), getgid(), getegid());
                    ngroups = setgroups(child_proc[i].num_groups, child_proc[i].group_list);
                    if(ngroups == -1) {
                        LOC_LOGE("Could not set groups. errno:%d, %s\n",
                              errno, strerror(errno));
                    }
                    LOC_LOGD("%s:%d]: Groups set\n", __func__, __LINE__);

                    memset(gid_list, 0, sizeof(gid_list));
                    ngroups = getgroups(MAX_NUM_GROUPS, gid_list);
                    if(ngroups == -1) {
                        LOC_LOGE("Could not find groups. ngroups:%d\n", ngroups);
                    }
                    else {
                        IF_LOC_LOGD {
                            LOC_LOGD("GIDs from child:\n");
                            for(ngroups = 0; ngroups<MAX_NUM_GROUPS; ngroups++) {
                                if(gid_list[ngroups]) {
                                    LOC_LOGD("%d ", gid_list[ngroups]);
                                }
                            }
                            LOC_LOGD("\n");
                        }
                    }
                    //Set UID of the child process to that of the GPS group
                    if(setuid(AID_GPS) == -1) {
                        LOC_LOGE("%s:%d]: Error: setuid failed; %s; Exiting process\n",
                                 __func__, __LINE__, strerror(errno));
                        exit(1);
                    }
                    ngroups=0;
                    //Set up the child process to receive exit when it receives
                    //a SIGHUP.
                    //SIGHUP is sent to the child process when the parent process
                    //exits. We want all child processes to exit when the launcher
                    //exits so that restarting the launcher will re-launch all the
                    //child processes.
                    if(prctl(PR_SET_PDEATHSIG, SIGHUP, 0, 0, 0) == -1) {
                        LOC_LOGE("%s:%d]: prctl failed. %s", __func__, __LINE__, strerror(errno));
                    }
                    execve(child_proc[i].name[0], child_proc[i].args, NULL);
                    LOC_LOGE("\nError! execve() returned. Something is wrong. Could not start %s\n",
                             child_proc[i].name[0]);
                }
                //This block executed by parent process
                else {
                    LOC_LOGD("Child %s created with pid: %d\n",
                             child_proc[i].name[0], child_proc[i].proc_id);
                    child_proc[i].proc_status = RUNNING;
                }
            }
        }

        //For handling race condition between when the child process crashes
        //and the parent goes to sleep without checking whether SIGCHLD is received
        sigemptyset(&newmask);
        sigaddset(&newmask, SIGCHLD);
        sigaddset(&newmask, SIGALRM);
        if(sigprocmask(SIG_BLOCK, &newmask, &oldmask) < 0) {
            LOC_LOGE("%s:%d]: SIG_BLOCK returned error\n", __func__, __LINE__);
        }
        if(sigchld_received || sigalrm_received)
            continue;
        LOC_LOGD("Parent sleeping. Will wake up when one of the children mess up\n");
        sigsuspend(&oldmask);
        if(sigprocmask(SIG_UNBLOCK, &newmask, NULL) < 0) {
            LOC_LOGE("%s:%d]: SIG_BLOCK returned error\n", __func__, __LINE__);
        }
        LOC_LOGE("Parent woken up! %s\n", strerror(errno));
    }
err:
    if(child_proc) {
        for(j=0; j<proc_list_length; j++)
        {
            if(child_proc[j].proc_id)
                kill(child_proc[j].proc_id, SIGHUP);
            if(child_proc[j].name[0]) {
                free(child_proc[j].name[0]);
            }
        }
        free(child_proc);
    }
err_conf_file:
err_capset:
    LOC_LOGE("%s:%d]: Exiting main. ret: %d", __func__, __LINE__, ret);
    return ret;
}
