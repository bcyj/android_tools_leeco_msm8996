/*
 * Copyright (c) 2014-2015, Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
#ifndef __COMMON_H
#define __COMMON_H
#include <cutils/log.h>
#include <cutils/properties.h>
#include <ctype.h>
#include <dlfcn.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <hash_map>
#include <inttypes.h>
#include <list>
#include <private/android_filesystem_config.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <sys/mman.h>
using namespace std;

/**Function*/
#define BITS_PER_LONG  (sizeof(unsigned long) * 8)
#define BITS_TO_LONGS(x)  (((x) + BITS_PER_LONG - 1) / BITS_PER_LONG)
#define test_bit(bit, array)   ((array)[(bit)/BITS_PER_LONG] & (1L << ((bit) % BITS_PER_LONG)))

typedef void (*cb_t) (void *);
typedef void *(*thread_func_t) (void *);

/**MMI*/
#define MMI_VERSION "Version 2.0"

#define MMI_SOCKET "/dev/socket/mmi"
#define MAIN_MODULE  "MMI"
#define MMI_BASE_DIR "/cache/FTM_AP/"
#define TEMP_MMAP_FILE  MMI_BASE_DIR".mmi"
#define AUTOSTART_CONFIG MMI_BASE_DIR".autostart"

#define MMI_RES_BASE_DIR "/system/etc/mmi/"
#define MMI_LAYOUT_BASE_DIR MMI_RES_BASE_DIR"layout/"
#define MMI_CONFIG MMI_RES_BASE_DIR"mmi.cfg"

#define LAYOUT_CONFIRM  "layout_confirm.xml"
#define LAYOUT_REBOOT  "layout_reboot.xml"
#define LAYOUT_PCBA  "layout_pcba.xml"
#define LAYOUT_REPORT "layout_report.xml"

#define SIZE_512 512
#define BLOCK_SIZE 512
#define SIZE_1K 1024
#define SIZE_2K (SIZE_1K*2)
#define SIZE_8K (SIZE_1K*8)
#define SIZE_1M (SIZE_1K*SIZE_1K)
#define SIZE_1G (SIZE_1K*SIZE_1K*SIZE_1K)

/**Font size config, percent of LCD density*/
#define FONT_SIZE_SMALL 3
#define FONT_SIZE_NORMAL 4
#define FONT_SIZE_LARGE 6
#define FONT_SIZE_HUGE 8

/**Camera preview dimenstion config*/
#define DIMENSION_SMALL_WIDTH 288
#define DIMENSION_SMALL_HEIGHT 352
#define DIMENSION_NORMAL_WIDTH 480
#define DIMENSION_NORMAL_HEIGHT 640
#define DIMENSION_LARGE_WIDTH  960
#define DIMENSION_LARGE_HEIGHT 1280

#define SUBCMD_MMI "mmi"
#define SUBCMD_PCBA "pcba"


/**PCBA test level*/
#define PCBA_STATNDARD "standard"
#define PCBA_FUNCTION "function"

/**Config file Key words*/
#define KEY_AUTOMATION "automation"
#define KEY_TESTMODE "test_mode"
#define KEY_STR_LANGUAGE "language"
#define KEY_FONTSIZE "font_size"
#define KEY_LIB_NAME "lib_name"
#define KEY_DISPALY_NAME "display_name"
#define KEY_ENABLE "enable"
#define KEY_LAYOUT "layout"
#define KEY_PARAMETER "parameter"

/**Result file keywords*/
#define KEY_TIMESTAMP_WORDS "Timestamp"
#define KEY_RESULT_WORDS "Result"
#define KEY_TESTTIME_WORDS "TestTime_Sec"

/**Button name string list*/
#define KEY_MAIN_STATUS "main_status"
#define KEY_MAIN_ALL "main_all"
#define KEY_MAIN_FAILED "main_failed"
#define KEY_MAIN_RUNALL "main_runall"
#define KEY_DISPLAY "display"
#define KEY_DISPLAY_PCBA "display_pcba"
#define KEY_REBOOT_ANDROID "reboot_android"
#define KEY_REBOOT_POWEROFF "reboot_poweroff"
#define KEY_REBOOT_FFBM "reboot_ffbm"

#define KEY_STR_EXTRACMD "extracmd"
#define KEY_STR_EXTRACMDSIZE "extracmd_size"
#define KEY_BTN "btn"
#define KEY_PASS "pass"
#define KEY_FAIL "fail"
#define KEY_STR_TITLE "title"
#define KEY_STR_INDICATION "indication"
#define KEY_STR_FRONT "front"
#define KEY_STR_BACK "back"
#define KEY_STR_LEFT "left"
#define KEY_STR_RIGHT "right"
#define KEY_STR_CENTER "center"
#define KEY_STR_HOME "home"
#define KEY_STR_MENU "menu"
#define KEY_STR_BACK "back"
#define KEY_STR_VOLUMEUP "volumeup"
#define KEY_STR_VOLUMEDOWN "volumedown"
#define KEY_STR_SNAPSHOT "snapshot"
#define KEY_STR_POWER "power"
#define KEY_STR_HEADPHONE_INSERT "headphone_insert"
#define KEY_STR_MICROPHONE_INSERT "microphone_insert"
#define KEY_STR_HANGUP "hangup"

/**Parameter keys*/
#define KEY_DIMENSION "dimension"
#define KEY_SMALL "small"
#define KEY_NORMAL "normal"
#define KEY_LARGE "large"
#define KEY_HUGE "huge"
#define KEY_MIN_LIMINT "min_limit"
#define KEY_MAX_LIMINT "max_limit"

/**LANGUAGE KEYWORDS*/
#define KEY_TITLE_PCBA "pcba_title"
#define KEY_POWER_OFF_NOTICE "power_off_notice"
#define KEY_RESET_NOTICE "reset_notice"
#define KEY_REBOOT_NOTICE "reboot_notice"

#define MAX_CAM_PREVIEW_BUFFERS 5

/**Diag control mmi commands list*/
#define CLIENT_DIAG_NAME "DIAG"
#define CLIENT_DEBUG_NAME "DEBUG"

#define SUBCMD_STAT "stat"                          /**Query the MMI status,include fail case number*/
#define SUBCMD_RECONFIG "reconfig"        /**Reconfig MMI using one specified CFG file*/
#define SUBCMD_CLEAR "clear"                     /**Clear the current result file*/
#define SUBCMD_RUNCASE "runcase"           /**Run specified case*/
#define SUBCMD_RUNALL "runall"                 /** Start Run all test cases*/
#define SUBCMD_LISTCASE "listcase"            /** List all the test cases into specified file*/
#define SUBCMD_EXITCASE "exitcase"          /** Exit the current test case*/

/**Keyword used in parameters*/
#define KEY_MMI_STAT "state"
#define KEY_FAIL_COUNT "fail_count"
#define KEY_CFG_PATH "cfg_path"
#define KEY_TESTLIST_PATH "testlist_path"
#define KEY_CASE_NUM "case_num"
#define KEY_CASE_NAME "case_name"
#define KEY_EXIT_RESULT "exit_result"

/**Common keywords***/
#define KEY_ASCII_TRUE "54525535"   //ASCII for "TRUE"

/**Color value*/
#define COLOR_RED 0xff0000ff
#define COLOR_GREEN 0x00ff00ff
#define COLOR_BLUE 0x0000ffff
#define COLOR_WHITE 0xffffffff
#define COLOR_BLACK 0x000000ff

/**Magnetometer compass value*/
#define ROUND_ANGLE 360
#define ROUND_POINT_HALF_SIDE 4
#define ROUND_POINT_SIDE 8
#define POINT_HALF_SIDE 16
#define POINT_SIDE 32

#define IPC_SYNC_USE_SEM_T

enum {
    CONFIG_SUCCESS = 0,
    CONFIG_NOT_FOUND_ERR = 1,
    CONFIG_FORTMAT_ERR = 2,
    CONFIG_TEST_CASE_ERR = 3,
    CONFIG_NO_DEFAULT_CFG_ERR = 4,
};

typedef enum {
    BOOT_MODE_NORMAL = 0,
    BOOT_MODE_PCBA = 1,
    BOOT_MODE_UI = 2,
    MAX_BOOT_MODE,
} boot_mode_type;

typedef enum {
    REBOOT_NONE = 0,
    REBOOT_POWEROFF = 1,
    REBOOT_FFBM = 2,
    REBOOT_ANDROID = 3,
    MAX_REBOOT,
} reboot_opt_t;

typedef enum {
    LEVEL_NONE = 0,
    LEVEL_STANDARD = 1,
    LEVEL_FUNCTION = 2,
} level_t;

enum {
    FAILED = -1,
    SUCCESS = 0,
    ERR_UNKNOW = INT_MAX
};

enum {
    FTM_SUCCESS = 0,
    FTM_FAIL = 1,
};

enum {
    MMI_IDLE = 0x0,
    MMI_BUSY = 0x1,
};

enum {
    // server to client
    CMD_INIT = 0x1,
    CMD_DEINIT = 0x02,
    CMD_QUERY = 0x03,
    CMD_RUN = 0x04,
    CMD_STOP = 0x05,
    CMD_NV_READ = 0X06,
    CMD_NV_WRITE = 0X07,
    CMD_RESULT = 0x08,

    // client to server
    CMD_PRINT = 0x101,          // for display/log message
    CMD_HELLO = 0x103,          // for display/log message
    CMD_CTRL = 0x104,           //Diag control mmi behavior
};

typedef enum {
    PRINT = 0,                  //only print in screen
    DATA,                       //only save data in file
    PRINT_DATA,                 //print in screen and save data
} print_type_t;

/**MMI test mode*/
typedef enum {
    TEST_MODE_NONE = 0,
    TEST_MODE_PCBA = 1,
    TEST_MODE_UI = 2,
    MAX_TEST_MODE
} module_mode_t;

/*
 * communication header between process
 * */
typedef enum {
    BUF_STATE_EMPTY,
    BUF_STATE_DONE
} buffer_state_t;

typedef struct {
    volatile int32_t buf_state[MAX_CAM_PREVIEW_BUFFERS];
    sem_t buf_mutex_sem;
} __attribute__ ((__packed__)) ipc_header_t;

/*
 * communication message between mmi core and client
 * */
typedef struct {
    char module[32];
    int msg_id;
    int cmd;
    char subcmd[32];
    int length;
    int result;
    char msg[SIZE_1K];          //para:xxx;  capture:normal:para
} __attribute__ ((__packed__)) msg_t;

/**Queue List*/
typedef struct {
    list < msg_t * >queue;
    pthread_mutex_t lock;
} msg_queue_t;

typedef struct {
    bool is_valid;
    uint32_t size;
      list < string > cmd_list;
} extra_cmd_t;

class module_info {
  public:
    char module[32];
    int socket_fd;
    int result;
    pid_t pid;
    int mode;
    int running_state;
    extra_cmd_t extracmd;
    time_t start_time;          //start test time
    double duration;            //test duration
    time_t last_time;           //last time to modify test result data
    char data[SIZE_512];        //module test data

      hash_map < string, string > config_list;
      module_info(char *mod) {
        if(mod != NULL)
            strlcpy(module, mod, sizeof(module));

        memset(data, 0, sizeof(data));
        result = INT_MAX;
        pid = -1;
        socket_fd = -1;
        extracmd.is_valid = false;
    }
};

typedef struct {
    cb_t cb;
    void *module;
} runnable_t;

typedef struct {
    list < runnable_t * >queue;
    pthread_mutex_t lock;
} runnable_queue_t;

typedef struct {
    char r;
    char g;
    char b;
    char a;
} color_t;

typedef struct {
    int x;
    int y;
    int w;
    int h;
} rect_t;

typedef struct {
    char name[32];
    cb_t cb;
} func_map_t;

class point_t {
  public:
    int x;
    int y;

    point_t() {
    }
    point_t(int x, int y) {
        this->x = x;
        this->y = y;
    }
    void operator=(const point_t & point) {
        this->x = point.x;
        this->y = point.y;
    }

    bool operator==(const point_t & point) {
        if(this->x == point.x && this->y == point.y)
            return true;
        return false;
    }
};


/**************************/
class mutex_locker {
    pthread_mutex_t m_mutex;
  public:
      class autolock {
        mutex_locker & locker;
      public:
        inline autolock(mutex_locker & locker):locker(locker) {
            locker.lock();
        }
        inline ~ autolock() {
            locker.unlock();
        }
    };
    inline mutex_locker() {
        pthread_mutex_init(&m_mutex, 0);
    }
    inline ~ mutex_locker() {
        pthread_mutex_destroy(&m_mutex);
    }
    inline void lock() {
        pthread_mutex_lock(&m_mutex);
    }
    inline void unlock() {
        pthread_mutex_unlock(&m_mutex);
    }
};

typedef struct exec_cmd_t {
    const char *cmd;
    char **params;
    char *result;
    char *exit_str;
    int pid;
    int size;
} exec_cmd_t;

typedef void (*callback) (char *, int);

typedef struct {
    const char *key_name;
    class button *key_btn;
    int key_code;
    bool exist;
    bool tested;
} key_map_t;
#endif
