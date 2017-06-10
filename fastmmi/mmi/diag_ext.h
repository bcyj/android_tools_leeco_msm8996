/*
 * Copyright (c) 2013-2015, Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __SYSTEM_CORE_MMI_DIAG__
#define __SYSTEM_CORE_MMI_DIAG__
#include <log.h>
#include "msg.h"
#include "diag_lsm.h"
#include "diagpkt.h"
#include "diag.h"
#include "diagcmd.h"
#include "utils.h"

#ifdef LOG_TAG
#undef LOG_TAG
#define LOG_TAG   "mmid"
#endif

#define DIRECTORY_FILENAME  "directory.txt"
#define TESTLIST_FILENAME  "testlist.txt"
#define PTT_SOCKET_BIN "/system/bin/ptt_socket_app"
#define FTMDAEMON_BIN "/system/bin/ftmdaemon"
#define CHGDISABLED_PROP "persist.usb.chgdisabled"

#define PACK_SIZE 1024

#define NV_PKT_SIZE 134
#define BYTE_PTR( var )    ( (byte *) &(var) )


/* Subsystem command codes for the test app  */
#define FTM_AP_C  52

#define WRITE_FILENAME_OFFSET 10
#define WRITE_APPEND_DATA_OFFSET(filename) (WRITE_FILENAME_OFFSET + strlen(filename) + 1)
#define WRITE_ISIZE_OFFSET(filename) (WRITE_APPEND_DATA_OFFSET(filename) + 1)
#define WRITE_DATA_OFFSET(filename) (WRITE_ISIZE_OFFSET(filename) + 2)


#define FTM_AP_LOG_PKT_ID 0x1125    /* FTM Sublog code under FTM_LOG_V2 (0x117C) */

enum {
    FTM_AP_TEST_APP_STATUS = 0,
    FTM_AP_SELECT_SEQUENCE = 1,
    FTM_AP_CLEAR_RESULTS = 2,
    FTM_AP_EXECUTE_SINGLE_TEST = 3,
    FTM_AP_EXECUTE_ALL_TESTS = 4,
    FTM_AP_READ_FILE = 5,
    FTM_AP_WRITE_FILE = 6,
    FTM_AP_ERASE_ALL_FILES = 7,
    FTM_AP_TEST_LIST_TO_FILE = 8,
    FTM_AP_DIR_TO_FILE = 9,
    FTM_AP_EXIT_TEST = 10,
    FTM_AP_UTILITY = 11
};

/* Subsystem command codes for the test app  */
#define FTM_DEVICE_INFO  54
enum {
    FTM_USB_SERIALNUM = 0,
};

typedef enum {
    APP_IDLE = 0,
    APP_EXECUTING
} app_status;


/* FTM Error code for FTM_AP_EXECUTE_SINGLE_TEST and FTM_AP_EXECUTE_ALL_TESTS*/
enum {
    FTM_NO_ERROR = 0,
    FTM_ERROR_TEST_CASE_NOT_FOUND,
    FTM_ERROR_MMI_IN_EXECUTION,
    FTM_ERROR_UNKNOWN,
};

/*  Read command
 0 = Success
 1 = Error
 2 = Error: Offset is invalid
 3 = Error: File does not exist
 4 = Error: File exists, but cannot be opened.
 */

enum {
    READ_SUCCESS = 0,
    READ_ERR = 1,
    READ_BAD_OFFSET_ERR = 2,
    READ_FILE_NOT_EXIST_ERR = 3,
    READ_OPEN_ERR = 4,
};

/*	Write command
0 = Success
1 = Error
2 = Error: File cannot be opened
3 = Warning: The previous file has not been closed.
*/
enum {
    WRITE_SUCCESS = 0,
    WRITE_ERR = 1,
    WRITE_OPEN_ERR = 2,
    WRITE_FILE_CLOSED_ERR = 3,
};

enum {
    CFG_SUCCESS = 0,
    CFG_NOT_FOUND,
    CFG_FORMAT_ERROR,
    CFG_CASE_ERROR,
    CFG_DEFUAL_NOT_FOUND,
};

enum {
    ID_AUTOSTART = 0,
    ID_WLAN,
    ID_BT,
    ID_CHARGER,
    ID_NFC,
};

enum {
    OPS_OFF = 0,
    OPS_ON,
};

typedef struct {
    char app_name[64];
    int32_t proc_id;
    bool activated;
    mutex_locker lock;
} app_status_t;

typedef struct {
    uint8_t cmd_code;
    uint8_t sub_sys_id;
    uint16_t sub_sys_cmd_code;
    uint16_t ftm_cmd_id;
    uint16_t ftm_data_len;
    uint16_t ftm_rsp_pkt_size;
} __attribute__ ((packed)) ftm_header;

/*read command request
CMD_CODE                         Unsigned / 1 byte    Command ID  - Set CMD_CODE to 75
SUB_SYS_ID                       Unsigned / 1 byte    Subsystem ID - FTM ID is 11
SUBSYS_CMD_CODE                  Unsigned / 2 bytes   FTM Mode ID - FTM_AP_C (52)
FTM_CMD_ID                       Unsigned / 2 bytes   5 - FTM_AP_READ_FILE
FTM_DATA_LEN                     Unsigned / 2 bytes   Unused, set to 0
FTM_RSP_PKT_SIZE                 Unsigned / 2 bytes   Unused, set to 0
OFFSET                           Unsigned / 4 bytes   The offset of the file location
MAX_SIZE                         Unsigned / 2 bytes   The maximum number of bytes to transfer.  The size is subject to the diag buffer limitation
FILENAME                         Variable ASCII       Null terminated The file to be read. The filename cannot contain any path "/"
*/
typedef struct {
    ftm_header sftm_header;
    uint32_t offset;
    uint16_t max_size;
    char filename[256];
} __attribute__ ((packed)) ftm_read_file_req;

/*single test command request
CMD_CODE                        Unsigned / 1 byte      Command ID - Set CMD_CODE to 75
SUB_SYS_ID                      Unsigned / 1 byte      Subsystem ID - FTM ID is 11
SUBSYS_CMD_CODE                 Unsigned / 2 bytes     FTM Mode ID - FTM_AP_C (52)
FTM_CMD_ID                      Unsigned / 2 bytes     3  - FTM_ AP_EXECUTE_SINGLE_TEST
FTM_DATA_LEN                    Unsigned / 2 bytes     Unused, set to 0
FTM_RSP_PKT_SIZE                Unsigned / 2 bytes     Unused, set to 0
TEST_CASE                       Variable ASCII         Null terminated String name of the test case defined in sequence file (.seq)
*/
typedef struct {
    ftm_header sftm_header;
    char test_case[256];
} __attribute__ ((packed)) ftm_single_test_case_req;

/*select sequence command request
CMD_CODE                        Unsigned / 1 byte       Command ID - Set CMD_CODE to 75
SUB_SYS_ID                      Unsigned / 1 byte       Subsystem ID - FTM ID is 11
SUBSYS_CMD_CODE                 Unsigned / 2 bytes      FTM Mode ID - FTM_AP_C (52)
FTM_CMD_ID                      Unsigned / 2 bytes      1  - FTM_AP_SELECT_SEQUENCE
FTM_DATA_LEN                    Unsigned / 2 bytes      Unused, set to 0
FTM_RSP_PKT_SIZE                Unsigned / 2 bytes      Unused, set to 0
FILENAME                        Variable ASCII          Null terminated String name of the test case defined in sequence file (.seq)
*/
typedef struct {
    ftm_header sftm_header;
    char filename[256];
} __attribute__ ((packed)) ftm_select_seq_req;

/*exit test case with result request
CMD_CODE                        Unsigned / 1 byte      Command ID - Set CMD_CODE to 75
SUB_SYS_ID                      Unsigned / 1 byte      Subsystem ID - FTM ID is 11
SUBSYS_CMD_CODE                 Unsigned / 2 bytes     FTM Mode ID - FTM_AP_C (52)
FTM_CMD_ID                      Unsigned / 2 bytes     10  - FTM_ AP_EXIT_TEST
FTM_DATA_LEN                    Unsigned / 2 bytes     Unused, set to 0
FTM_RSP_PKT_SIZE                Unsigned / 2 bytes     Unused, set to 0
FILENAME                        Variable ASCII         Null terminated String name of the test case defined in sequence file (.seq)
*/
typedef struct {
    ftm_header sftm_header;
    char result;
} __attribute__ ((packed)) ftm_exit_test_req;


typedef struct {
    ftm_header sftm_header;
    uint16_t ftm_error_code;
    uint32_t file_size;
    uint32_t offset;
    uint16_t size;
    uint8_t Data[PACK_SIZE];
} __attribute__ ((packed)) ftm_read_file_response;

typedef struct {
    ftm_header sftm_header;
    uint16_t ftm_error_code;
    uint8_t state;              //   "   0 = Idle   "   1 = Executing
    uint32_t fail_count;        //Returns the number of test cases that have failed since the status was resetted by FTM_MMIFTM_AP_CLEAR_ALL_STATUS.
} __attribute__ ((packed)) ftm_ap_status_response;

typedef struct {
    ftm_header sftm_header;
    uint16_t ftm_error_code;
} __attribute__ ((packed)) ftm_common_response;

typedef struct {
    ftm_header sftm_header;
    uint16_t ftm_error_code;
    uint16_t iNumTests;         // num of test from the sequence
} __attribute__ ((packed)) ftm_select_sequence_response;

typedef struct {
    ftm_header sftm_header;
    uint16_t ftm_error_code;
    char serialnum[0];
} __attribute__ ((packed)) ftm_usb_serialnum_response;

typedef struct {
    log_hdr_type hdr;
    word ftm_log_id;            /* FTM log id */
    int32_t test_result;        /* result (0==PASS), else Error Code */
    byte data[SIZE_512];        /* Variable length payload, test_case nam look at FTM log id for contents */
} __attribute__ ((packed)) ftm_ap_log_pkt_type;

/*FTM AP utility
CMD_CODE                         Unsigned / 1 byte       Command ID - Set CMD_CODE to 75
SUB_SYS_ID                       Unsigned / 1 byte       Subsystem ID - FTM ID is 11
SUBSYS_CMD_CODE                  Unsigned / 2 bytes      FTM Mode ID - FTM_AP_C (52)
FTM_CMD_ID                       Unsigned / 2 bytes      11  - FTM_ AP_UTILITY
FTM_DATA_LEN                     Unsigned / 2 bytes      Unused, set to 0
FTM_RSP_PKT_SIZE                 Unsigned / 2 bytes      Unused, set to 0
FTM_UTILITY_ID                   Unsigned / 1 bytes      ID LIST:      0=AutoStart   1=WLAN   2=BT  3=CHARGER  4=NFC
FTM_UTILITY_OPS                  Unsigned / 1 bytes      0-ON  1-OFF
*/
typedef struct {
    ftm_header sftm_header;
    uint8_t utility_id;
    uint8_t utility_ops;
} __attribute__ ((packed)) ftm_ap_utility_req;

int diag_init(void);
int diag_deinit(void);
void send_cmd(const char *subcmd);
void send_cmd(const char *subcmd, const char *params, uint32_t size);
void log_msg(const char *str);
#endif
