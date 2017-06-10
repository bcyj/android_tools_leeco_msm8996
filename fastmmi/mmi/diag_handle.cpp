/*
 * Copyright (c) 2013-2015, Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
#include "diag_ext.h"

extern msg_t g_msg;
static app_status_t ptt_socket_status;
static app_status_t ftmdaemon_status;

PACK(void *) ftm_ap_dispatch(PACK(void *)req_pkt, uint16 pkt_len);

     static const diagpkt_user_table_entry_type ftm_fastmmi_diag_func_table[] = {   /* susbsys_cmd_code lo = 0 , susbsys_cmd_code hi = 0, call back function */
         {FTM_AP_C, FTM_AP_C, ftm_ap_dispatch},
     };


/*  FFBM related function */
static bool is_file(const char *path) {
    struct stat st;

    if(lstat(path, &st) == 0)
        return S_ISREG(st.st_mode) != 0;
    return false;
}

/**Erase all files under FTM_AP */
static int erase_all_files() {

    struct dirent *de;
    DIR *dir;
    char filepath[PATH_MAX];

    dir = opendir(MMI_BASE_DIR);
    if(dir != 0) {
        while((de = readdir(dir))) {
            if((strncmp(de->d_name, ".", 2) == 0)
               || (strncmp(de->d_name, "..", 3) == 0))
                continue;
            snprintf(filepath, sizeof(filepath), "%s%s", MMI_BASE_DIR, de->d_name);
            if(is_file(filepath))
                remove(filepath);
        }
        closedir(dir);
    }
    return FTM_SUCCESS;
}

/*List the FTM_AP directory contents to a file directory.Txt,
in the FTM_AP directory(see 3.3.1.1).  The directory.Txt file
can later be retrieved to desktop with FTM_AP_READ_FILE command.*/
static int dir_list_filename_to_file(const char *filename) {
    struct dirent *de;
    DIR *dir;
    FILE *fp = NULL;
    char filepath[PATH_MAX];
    char buf[255];

    memset(buf, 0, 255);

    snprintf(filepath, sizeof(filepath), "%s%s", MMI_BASE_DIR, filename);
    ALOGI("%s: filepath(%s)", __FUNCTION__, filepath);
    fp = fopen(filepath, "w");
    if(!fp)
        return FTM_FAIL;

    dir = opendir(MMI_BASE_DIR);
    if(dir != 0) {
        while((de = readdir(dir))) {
            if(!strncmp(de->d_name, ".", 1)  || !strcmp(de->d_name, ".."))
                continue;

            ALOGI("%s \n", de->d_name);
            snprintf(buf, sizeof(buf), "%s\n", de->d_name);
            fwrite(buf, strlen(buf), 1, fp);
        }
        closedir(dir);
    }
    fclose(fp);
    return FTM_SUCCESS;
}

PACKED void *ftm_app_status(PACK(void *)req_pkt, uint16 pkt_len) {
    ftm_ap_status_response *rsp = NULL;

    hash_map < string, string > params;
    /* Allocate the same length as the request. */
    rsp = (ftm_ap_status_response *) diagpkt_subsys_alloc(DIAG_SUBSYS_FTM, FTM_AP_C, sizeof(ftm_ap_status_response));

    if(rsp != NULL) {
        ALOGE("%s: diagpkt_subsys_alloc succeeded", __FUNCTION__);
    } else {
        ALOGE("%s: diagpkt_subsys_alloc failed", __FUNCTION__);
        return NULL;
    }

    send_cmd(SUBCMD_STAT);      /*success */
    rsp->ftm_error_code = g_msg.result;

    parse_parameter(g_msg.msg, params);

    rsp->fail_count = atoi(params[KEY_FAIL_COUNT].c_str());
    rsp->state = atoi(params[KEY_MMI_STAT].c_str());
    ALOGI("ftm_app_status(state=%ld,fail_count=%d)", rsp->fail_count, rsp->state);

    rsp->sftm_header.cmd_code = 75;
    rsp->sftm_header.sub_sys_id = DIAG_SUBSYS_FTM;
    rsp->sftm_header.sub_sys_cmd_code = FTM_AP_C;
    rsp->sftm_header.ftm_cmd_id = FTM_AP_TEST_APP_STATUS;
    rsp->sftm_header.ftm_data_len = 0;
    rsp->sftm_header.ftm_rsp_pkt_size = 0;

    return rsp;
}

PACKED void *ftm_select_sequence(PACK(void *)req_pkt, uint16 pkt_len) {

    ftm_select_sequence_response *rsp = NULL;
    ftm_select_seq_req *req = NULL;
    char filepath[PATH_MAX] = { 0 };

    hash_map < string, string > params;

    req = (ftm_select_seq_req *) req_pkt;

    /* Allocate the same length as the request. */
    rsp = (ftm_select_sequence_response *) diagpkt_subsys_alloc(DIAG_SUBSYS_FTM, FTM_AP_C, sizeof(ftm_common_response));
    ALOGI("ftm_select_sequence: start \n");
    if(rsp != NULL) {
        ALOGE("%s: diagpkt_subsys_alloc succeeded", __FUNCTION__);
    } else {
        ALOGE("%s: diagpkt_subsys_alloc failed", __FUNCTION__);
        return NULL;
    }

    snprintf(filepath, sizeof(filepath), "%s:%s%s", KEY_CFG_PATH, MMI_BASE_DIR, req->filename);
    send_cmd(SUBCMD_RECONFIG, filepath, strlen(filepath));
    rsp->ftm_error_code = g_msg.result;
    parse_parameter(g_msg.msg, params);

    rsp->sftm_header.cmd_code = 75;
    rsp->sftm_header.sub_sys_id = DIAG_SUBSYS_FTM;
    rsp->sftm_header.sub_sys_cmd_code = FTM_AP_C;
    rsp->sftm_header.ftm_cmd_id = FTM_AP_SELECT_SEQUENCE;
    rsp->sftm_header.ftm_data_len = 0;
    rsp->iNumTests = atoi(params[KEY_CASE_NUM].c_str());
    rsp->sftm_header.ftm_rsp_pkt_size = 0;
    ALOGI("ftm_select_sequence: end \n");

    return rsp;
}

PACKED void *ftm_clear_results(PACK(void *)req_pkt, uint16 pkt_len) {
    ftm_common_response *rsp = NULL;

    /* Allocate the same length as the request. */
    rsp = (ftm_common_response *) diagpkt_subsys_alloc(DIAG_SUBSYS_FTM, FTM_AP_C, sizeof(ftm_common_response));

    if(rsp != NULL) {
        ALOGE("%s: diagpkt_subsys_alloc succeeded", __FUNCTION__);
    } else {
        ALOGE("%s: diagpkt_subsys_alloc failed", __FUNCTION__);
        return NULL;
    }

    send_cmd(SUBCMD_CLEAR);
    rsp->ftm_error_code = g_msg.result;

    rsp->sftm_header.cmd_code = 75;
    rsp->sftm_header.sub_sys_id = DIAG_SUBSYS_FTM;
    rsp->sftm_header.sub_sys_cmd_code = FTM_AP_C;
    rsp->sftm_header.ftm_cmd_id = FTM_AP_CLEAR_RESULTS;
    rsp->sftm_header.ftm_data_len = 0;
    rsp->sftm_header.ftm_rsp_pkt_size = 0;
    return rsp;
}

PACKED void *ftm_execute_single_test(PACK(void *)req_pkt, uint16 pkt_len) {
    ftm_common_response *rsp = NULL;
    ftm_single_test_case_req *req = NULL;
    char case_name[NAME_MAX] = { 0 };

    req = (ftm_single_test_case_req *) req_pkt;
    /* Allocate the same length as the request. */
    rsp = (ftm_common_response *) diagpkt_subsys_alloc(DIAG_SUBSYS_FTM, FTM_AP_C, sizeof(ftm_common_response));

    if(rsp != NULL) {
        ALOGE("%s: diagpkt_subsys_alloc succeeded", __FUNCTION__);
    } else {
        ALOGE("%s: diagpkt_subsys_alloc failed", __FUNCTION__);
        return NULL;
    }

    snprintf(case_name, sizeof(case_name), "%s:%s", KEY_CASE_NAME, req->test_case);

    send_cmd(SUBCMD_RUNCASE, case_name, strlen(case_name)); /*success */
    rsp->ftm_error_code = g_msg.result;

    rsp->sftm_header.cmd_code = 75;
    rsp->sftm_header.sub_sys_id = DIAG_SUBSYS_FTM;
    rsp->sftm_header.sub_sys_cmd_code = FTM_AP_C;
    rsp->sftm_header.ftm_cmd_id = FTM_AP_EXECUTE_SINGLE_TEST;
    rsp->sftm_header.ftm_data_len = 0;
    rsp->sftm_header.ftm_rsp_pkt_size = 0;

    return rsp;
}


PACKED void *ftm_execute_all_tests(PACK(void *)req_pkt, uint16 pkt_len) {
    ftm_common_response *rsp = NULL;

    /* Allocate the same length as the request. */
    rsp = (ftm_common_response *) diagpkt_subsys_alloc(DIAG_SUBSYS_FTM, FTM_AP_C, sizeof(ftm_common_response));

    if(rsp != NULL) {
        ALOGE("%s: diagpkt_subsys_alloc succeeded", __FUNCTION__);
    } else {
        ALOGE("%s: diagpkt_subsys_alloc failed", __FUNCTION__);
        return NULL;
    }

    send_cmd(SUBCMD_RUNALL);    /*success */
    rsp->ftm_error_code = g_msg.result;

    rsp->sftm_header.cmd_code = 75;
    rsp->sftm_header.sub_sys_id = DIAG_SUBSYS_FTM;
    rsp->sftm_header.sub_sys_cmd_code = FTM_AP_C;
    rsp->sftm_header.ftm_cmd_id = FTM_AP_EXECUTE_ALL_TESTS;
    rsp->sftm_header.ftm_data_len = 0;
    rsp->sftm_header.ftm_rsp_pkt_size = 0;

    return rsp;
}


PACKED void *ftm_read_file(PACK(void *)req_pkt, uint16 pkt_len) {

    ftm_read_file_response *rsp = NULL;
    ftm_read_file_req *req = NULL;
    FILE *fp = NULL;
    char filepath[255] = { 0 };
    uint16_t iSize = 0;
    uint8_t Data[PACK_SIZE];

    req = (ftm_read_file_req *) req_pkt;

    /*parse parameter */
    char *pFileName = req->filename;
    uint32_t offset = req->offset;
    uint16_t max_size = req->max_size;

    ALOGI("ftm_read_file: pFileName=%s,%d\n", pFileName, offset);
    /* Allocate the same length as the request. */
    rsp = (ftm_read_file_response *) diagpkt_subsys_alloc(DIAG_SUBSYS_FTM, FTM_AP_C, sizeof(ftm_read_file_response));
    if(rsp != NULL) {
        ALOGI("%s: diagpkt_subsys_alloc succeeded", __FUNCTION__);
    } else {
        ALOGE("%s: diagpkt_subsys_alloc failed", __FUNCTION__);
        return NULL;
    }

    /*get dir */
    memset(Data, 0, sizeof(Data));

    snprintf(filepath, sizeof(filepath), "%s%s", MMI_BASE_DIR, pFileName);

    fp = fopen(filepath, "rb");
    if(fp) {
        fseek(fp, 0, SEEK_END); /*non-portable */
        rsp->file_size = ftell(fp);

        ALOGI("ftm_read_file: offset=%d,filesize=%ld \n", offset, rsp->file_size);

        if(offset < rsp->file_size) {
            fseek(fp, offset, SEEK_SET);
            iSize = fread(Data, 1, max_size, fp);
            rsp->ftm_error_code = READ_SUCCESS;
        } else
            rsp->ftm_error_code = READ_BAD_OFFSET_ERR;

        fclose(fp);
        fp = NULL;
    } else {
        rsp->ftm_error_code = READ_FILE_NOT_EXIST_ERR;
    }

    rsp->sftm_header.cmd_code = 75;
    rsp->sftm_header.sub_sys_id = DIAG_SUBSYS_FTM;
    rsp->sftm_header.sub_sys_cmd_code = FTM_AP_C;
    rsp->sftm_header.ftm_cmd_id = FTM_AP_READ_FILE;
    rsp->sftm_header.ftm_data_len = 0;
    rsp->sftm_header.ftm_rsp_pkt_size = 0;

    rsp->offset = offset;
    rsp->size = iSize;
    ALOGI("ftm_read_file: iSize=%d", iSize);
    memcpy(rsp->Data, Data, sizeof(rsp->Data));

    return rsp;
}

/* write file request struct
CMD_CODE	          Unsigned / 1 byte	            Command ID - Set CMD_CODE to 75
SUB_SYS_ID	          Unsigned / 1 byte	            Subsystem ID - FTM ID is 11
SUBSYS_CMD_CODE	          Unsigned / 2 bytes	            FTM Mode ID - FTM_AP_C (52)
FTM_CMD_ID	          Unsigned / 2 bytes	            6 - FTM_AP_WRITE_FILE
FTM_DATA_LEN	          Unsigned / 2 bytes	            Unused, set to 0
FTM_RSP_PKT_SIZE	  Unsigned / 2 bytes	            Unused, set to 0
FILENAME	          Variable length ASCII             Null terminated  The file to be read.
                                                            The filename cannot contain any path "/"
MORE_DATA	          Unsigned / 1 bytes	            0 = no more data   1 = more data
SIZE	                  Unsigned / 2 bytes	            The actual number of bytes transfer in DATA portion
DATA	                  Variable length binary            The data stream
*/
PACKED void *ftm_write_file(PACK(void *)req_pkt, uint16 pkt_len) {
    ftm_common_response *rsp = NULL;

    char filepath[255] = { 0 };
    static FILE *fp = NULL;
    uint8_t *pReq = (unsigned char *) req_pkt;
    char *filename = (char *) (pReq + WRITE_FILENAME_OFFSET);

    uint8_t append_data = *(unsigned char *) (pReq + WRITE_APPEND_DATA_OFFSET(filename));
    uint16_t i_size = *(unsigned short *) (pReq + WRITE_ISIZE_OFFSET(filename));
    unsigned char *pData = (unsigned char *) &pReq[WRITE_DATA_OFFSET(filename)];


    /* Allocate the same length as the request. */
    rsp = (ftm_common_response *) diagpkt_subsys_alloc(DIAG_SUBSYS_FTM, FTM_AP_C, sizeof(ftm_common_response));

    if(rsp != NULL) {
        ALOGE("%s: diagpkt_subsys_alloc succeeded", __FUNCTION__);
    } else {
        ALOGE("%s: diagpkt_subsys_alloc failed", __FUNCTION__);
        return NULL;
    }

    snprintf(filepath, sizeof(filepath), "%s%s", MMI_BASE_DIR, filename);

    if(!append_data)
        fp = fopen(filepath, "w");
    else
        fp = fopen(filepath, "a+");

    if(fp) {
        if(i_size > 0)
            fwrite(pData, 1, i_size, fp);

        ALOGI("write file: i_size=%d", i_size);
        rsp->ftm_error_code = WRITE_SUCCESS;
        fclose(fp);
        fp = NULL;
    } else {
        rsp->ftm_error_code = WRITE_OPEN_ERR;
    }

    rsp->sftm_header.cmd_code = 75;
    rsp->sftm_header.sub_sys_id = DIAG_SUBSYS_FTM;
    rsp->sftm_header.sub_sys_cmd_code = FTM_AP_C;
    rsp->sftm_header.ftm_cmd_id = FTM_AP_WRITE_FILE;
    rsp->sftm_header.ftm_data_len = 0;
    rsp->sftm_header.ftm_rsp_pkt_size = 0;

    return rsp;
}


PACKED void *ftm_test_list_to_file(PACK(void *)req_pkt, uint16 pkt_len) {
    ftm_common_response *rsp = NULL;
    char filepath[PATH_MAX];

    /* Allocate the same length as the request. */
    rsp = (ftm_common_response *) diagpkt_subsys_alloc(DIAG_SUBSYS_FTM, FTM_AP_C, sizeof(ftm_common_response));

    if(rsp != NULL) {
        ALOGE("%s: diagpkt_subsys_alloc succeeded", __FUNCTION__);
    } else {
        ALOGE("%s: diagpkt_subsys_alloc failed", __FUNCTION__);
        return NULL;
    }

    snprintf(filepath, sizeof(filepath), "%s:%s%s", KEY_TESTLIST_PATH, MMI_BASE_DIR, TESTLIST_FILENAME);
    send_cmd(SUBCMD_LISTCASE, filepath, strlen(filepath));
    rsp->ftm_error_code = g_msg.result;

    rsp->sftm_header.cmd_code = 75;
    rsp->sftm_header.sub_sys_id = DIAG_SUBSYS_FTM;
    rsp->sftm_header.sub_sys_cmd_code = FTM_AP_C;
    rsp->sftm_header.ftm_cmd_id = FTM_AP_TEST_LIST_TO_FILE;
    rsp->sftm_header.ftm_data_len = 0;
    rsp->sftm_header.ftm_rsp_pkt_size = 0;
    return rsp;
}


PACKED void *ftm_erase_all_files(PACK(void *)req_pkt, uint16 pkt_len) {
    ftm_common_response *rsp = NULL;

    /* Allocate the same length as the request. */
    rsp = (ftm_common_response *) diagpkt_subsys_alloc(DIAG_SUBSYS_FTM, FTM_AP_C, sizeof(ftm_common_response));

    if(rsp != NULL) {
        ALOGE("%s: diagpkt_subsys_alloc succeeded", __FUNCTION__);
    } else {
        ALOGE("%s: diagpkt_subsys_alloc failed", __FUNCTION__);
        return NULL;
    }

    rsp->ftm_error_code = erase_all_files();    /*success */

    rsp->sftm_header.cmd_code = 75;
    rsp->sftm_header.sub_sys_id = DIAG_SUBSYS_FTM;
    rsp->sftm_header.sub_sys_cmd_code = FTM_AP_C;
    rsp->sftm_header.ftm_cmd_id = FTM_AP_ERASE_ALL_FILES;
    rsp->sftm_header.ftm_data_len = 0;
    rsp->sftm_header.ftm_rsp_pkt_size = 0;
    return rsp;
}

PACKED void *ftm_dir_to_file(PACK(void *)req_pkt, uint16 pkt_len) {
    ftm_common_response *rsp = NULL;

    /* Allocate the same length as the request. */
    rsp = (ftm_common_response *) diagpkt_subsys_alloc(DIAG_SUBSYS_FTM, FTM_AP_C, sizeof(ftm_common_response));

    if(rsp != NULL) {
        ALOGE("%s: diagpkt_subsys_alloc succeeded", __FUNCTION__);
    } else {
        ALOGE("%s: diagpkt_subsys_alloc failed", __FUNCTION__);
        return NULL;
    }

    rsp->ftm_error_code = dir_list_filename_to_file(DIRECTORY_FILENAME);    /*success */

    rsp->sftm_header.cmd_code = 75;
    rsp->sftm_header.sub_sys_id = DIAG_SUBSYS_FTM;
    rsp->sftm_header.sub_sys_cmd_code = FTM_AP_C;
    rsp->sftm_header.ftm_cmd_id = FTM_AP_DIR_TO_FILE;
    rsp->sftm_header.ftm_data_len = 0;
    rsp->sftm_header.ftm_rsp_pkt_size = 0;
    return rsp;
}

PACKED void *ftm_exit_test(PACK(void *)req_pkt, uint16 pkt_len) {
    ftm_common_response *rsp = NULL;
    ftm_exit_test_req *req = NULL;
    char params[256] = { 0 };
    req = (ftm_exit_test_req *) req_pkt;
    /* Allocate the same length as the request. */
    rsp = (ftm_common_response *) diagpkt_subsys_alloc(DIAG_SUBSYS_FTM, FTM_AP_C, sizeof(ftm_common_response));

    if(rsp != NULL) {
        ALOGE("%s: diagpkt_subsys_alloc succeeded", __FUNCTION__);
    } else {
        ALOGE("%s: diagpkt_subsys_alloc failed", __FUNCTION__);
        return NULL;
    }
    ALOGI("get test result from host: %d \n", req->result);
    snprintf(params, sizeof(params), "%s:%d", KEY_EXIT_RESULT, req->result);

    send_cmd(SUBCMD_EXITCASE, params, strlen(params));
    rsp->ftm_error_code = g_msg.result;

    rsp->sftm_header.cmd_code = 75;
    rsp->sftm_header.sub_sys_id = DIAG_SUBSYS_FTM;
    rsp->sftm_header.sub_sys_cmd_code = FTM_AP_C;
    rsp->sftm_header.ftm_cmd_id = FTM_AP_EXIT_TEST;
    rsp->sftm_header.ftm_data_len = 0;
    rsp->sftm_header.ftm_rsp_pkt_size = 0;
    return rsp;
}

static uint16_t do_utility(uint8_t id, uint8_t ops) {

    ALOGI("%s: id:%d,ops:%d \n", __FUNCTION__, id, ops);
    uint16_t ret = FTM_SUCCESS;

    switch (id) {
    case ID_AUTOSTART:
        if(ops == OPS_ON)
            ret = write_file(AUTOSTART_CONFIG, KEY_ASCII_TRUE);
        else
            ret = clear_file(AUTOSTART_CONFIG);
        break;

    case ID_WLAN:
        if(ops == OPS_ON) {
            mutex_locker::autolock _L(ptt_socket_status.lock);
            if(!ptt_socket_status.activated) {
                char *args[5] = { PTT_SOCKET_BIN, "-f", "-d", "-v", NULL };
                ret = fork_daemon(PTT_SOCKET_BIN, args, &ptt_socket_status.proc_id);
                if(!ret)
                    ptt_socket_status.activated = true;
            }
        } else {
            mutex_locker::autolock _L(ptt_socket_status.lock);
            if(ptt_socket_status.activated) {
                kill_proc(ptt_socket_status.proc_id);
                ptt_socket_status.activated = false;
            }
        }
        break;

    case ID_BT:
    case ID_NFC:
        if(ops == OPS_ON) {
            mutex_locker::autolock _L(ftmdaemon_status.lock);
            if(!ftmdaemon_status.activated) {
                char *args[3] = { FTMDAEMON_BIN, "-n", NULL };
                ret = fork_daemon(FTMDAEMON_BIN, args, &ftmdaemon_status.proc_id);
                if(!ret)
                    ftmdaemon_status.activated = true;
            }
        } else {
            mutex_locker::autolock _L(ftmdaemon_status.lock);
            if(ftmdaemon_status.activated) {
                kill_proc(ftmdaemon_status.proc_id);
                ftmdaemon_status.activated = false;
            }
        }

        break;

    case ID_CHARGER:
        if(ops == OPS_ON) {
            property_set(CHGDISABLED_PROP, "0");
        } else {
            property_set(CHGDISABLED_PROP, "1");
        }
        break;

    default:
        break;
    }

    return ret == 0 ? FTM_SUCCESS : FTM_FAIL;
}

PACKED void *ftm_utility(PACK(void *)req_pkt, uint16 pkt_len) {
    ftm_common_response *rsp = NULL;
    ftm_ap_utility_req *req = NULL;
    char params[256] = { 0 };
    req = (ftm_ap_utility_req *) req_pkt;
    /* Allocate the same length as the request. */
    rsp = (ftm_common_response *) diagpkt_subsys_alloc(DIAG_SUBSYS_FTM, FTM_AP_C, sizeof(ftm_common_response));

    if(req != NULL && rsp != NULL) {
        ALOGE("%s: diagpkt_subsys_alloc succeeded", __FUNCTION__);
    } else {
        ALOGE("%s: diagpkt_subsys_alloc failed", __FUNCTION__);
        return NULL;
    }

    rsp->ftm_error_code = do_utility(req->utility_id, req->utility_ops);

    rsp->sftm_header.cmd_code = 75;
    rsp->sftm_header.sub_sys_id = DIAG_SUBSYS_FTM;
    rsp->sftm_header.sub_sys_cmd_code = FTM_AP_C;
    rsp->sftm_header.ftm_cmd_id = FTM_AP_UTILITY;
    rsp->sftm_header.ftm_data_len = 0;
    rsp->sftm_header.ftm_rsp_pkt_size = 0;
    return rsp;
}

PACKED void log_msg(const char *str) {
    int result = log_status(LOG_FTM_VER_2_C);

    ALOGI("ftm_ap_send_log_msg start");
    ftm_ap_log_pkt_type *pkt_ptr = (ftm_ap_log_pkt_type *) log_alloc(LOG_FTM_VER_2_C, sizeof(ftm_ap_log_pkt_type));

    if(result == 1) {
        if(pkt_ptr != NULL && str != NULL) {
            ALOGI("ftm_ap_send_log_msg :%s", pkt_ptr);
            pkt_ptr->ftm_log_id = FTM_AP_LOG_PKT_ID;
            pkt_ptr->test_result = 0;
            /*copy the case name to log data */
            strlcpy((char *) pkt_ptr->data, str, sizeof(pkt_ptr->data));
            log_commit(pkt_ptr);
        }
    }
}

/*===========================================================================*/

/* ftm_ap_dispatch registered functions */

/*===========================================================================*/

PACK(void *) ftm_ap_dispatch(PACK(void *)req_pkt, uint16 pkt_len) {
    PACK(void *) rsp = NULL;

    ftm_header *pheader = (ftm_header *) req_pkt;
    uint16_t iCmd = pheader->ftm_cmd_id;

    ALOGI("\n  start to process FTM_CMD_ID iCmd=%d\n", iCmd);
    switch (iCmd) {
    case FTM_AP_TEST_APP_STATUS:
        rsp = ftm_app_status(req_pkt, pkt_len);
        break;
    case FTM_AP_SELECT_SEQUENCE:
        rsp = ftm_select_sequence(req_pkt, pkt_len);
        break;
    case FTM_AP_CLEAR_RESULTS:
        rsp = ftm_clear_results(req_pkt, pkt_len);
        break;
    case FTM_AP_EXECUTE_SINGLE_TEST:
        rsp = ftm_execute_single_test(req_pkt, pkt_len);
        break;
    case FTM_AP_EXECUTE_ALL_TESTS:
        rsp = ftm_execute_all_tests(req_pkt, pkt_len);
        break;
    case FTM_AP_READ_FILE:
        rsp = ftm_read_file(req_pkt, pkt_len);
        break;
    case FTM_AP_WRITE_FILE:
        rsp = ftm_write_file(req_pkt, pkt_len);
        break;
    case FTM_AP_TEST_LIST_TO_FILE:
        rsp = ftm_test_list_to_file(req_pkt, pkt_len);
        break;
    case FTM_AP_ERASE_ALL_FILES:
        rsp = ftm_erase_all_files(req_pkt, pkt_len);
        break;
    case FTM_AP_DIR_TO_FILE:
        rsp = ftm_dir_to_file(req_pkt, pkt_len);
        break;
    case FTM_AP_EXIT_TEST:
        rsp = ftm_exit_test(req_pkt, pkt_len);
        break;
    case FTM_AP_UTILITY:
        rsp = ftm_utility(req_pkt, pkt_len);
        break;

    default:
        break;
    }
    ALOGI("\n  end of process FTM_CMD_ID iCmd=%d\n", iCmd);

    return rsp;
}

int diag_deinit(void) {
    return Diag_LSM_DeInit();
}

/*===========================================================================*/

/* Main Function. This initializes Diag_LSM, calls the tested APIs and exits. */

/*===========================================================================*/
int diag_init(void) {

    ALOGI("\n\t\t=====================");
    ALOGI("\n\t\tStarting fastmmi Test App Calling LSM init ");
    ALOGI("\n\t\t=====================");

    /* Calling LSM init  */
    if(!Diag_LSM_Init(NULL)) {
        ALOGE("fastmmi Test App: Diag_LSM_Init() failed.");
        return -1;
    }

    /* Registering diag packet with no subsystem id. This is so
     * that an empty request to the app. gets a response back
     * and we can ensure that the diag is working as well as the app. is
     * responding subsys id = 11, table = test_tbl_2,
     * To execute on QXDM :: "send_data 75 11 0 0 0 0 0 0"
     OR
     * To execute on QXDM :: "send_data 75 11 3 0 0 0 0 0"
     */
    DIAGPKT_DISPATCH_TABLE_REGISTER(DIAG_SUBSYS_FTM, ftm_fastmmi_diag_func_table);

    return 0;
}
