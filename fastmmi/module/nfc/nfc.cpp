/*
 * Copyright (c) 2014, Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
#include <errno.h>
#include <hardware/nfc.h>
#include <hardware/hardware.h>

#include "mmi_module.h"

/*==========================================================================*
*                             Defnitions                                  *
*==========================================================================*/
#define TRUE                      1
#define FALSE                     0
#define FTM_MODE                  1

enum {
    NCI_HAL_INIT,
    NCI_HAL_WRITE,
    NCI_HAL_READ,
    NCI_HAL_DEINIT,
    NCI_HAL_ASYNC_LOG,
    NCI_HAL_ERROR
};

/*Data buffer linked list*/

typedef struct asyncdata {
    unsigned char *response_buff;
    uint16_t async_datalen;
    struct asyncdata *next;
} asyncdata_t;

struct nfc_cmd {
    const unsigned char *cmd;
    unsigned char len;
};

/*==========================================================================*
*                             Declarations                                  *
*==========================================================================*/
static hash_map < string, string > *temp;
static pthread_t processThreadPid;

static nfc_nci_device_t *dev = NULL;
static unsigned char hal_state = NCI_HAL_INIT;
static unsigned char *nfc_cmd_buff = NULL, len = 0;
static unsigned short res_len = 0;
static unsigned char *response_buff = NULL;
static unsigned char hal_opened = FALSE, wait_rsp = FALSE;
static asyncdata_t *buff = NULL;
static asyncdata_t *start = NULL;
static int init_finish = 0;

/*I2C read/write*/
static pthread_mutex_t nfc_mutex = PTHREAD_MUTEX_INITIALIZER;

/* Reader thread handle */
static pthread_t nfc_thread_handle;
static sem_t semaphore_halcmd_complete;
static sem_t semaphore_nfccmd_complete;
static sem_t semaphore_nfcdetect_complete;
static sem_t semaphore_nfchal_ready;

/* NFC detection commands*/
const unsigned char nfc_test_cmd_1[] = { 0x20, 0x00, 0x01, 0x01 };
const unsigned char nfc_test_cmd_2[] = { 0x2F, 0x01, 0x09, 0x04, 0x00, 0x00, 0x07, 0x72, 0x00, 0x00, 0x00, 0x01 };
const unsigned char nfc_test_cmd_3[] =
    { 0x2F, 0x01, 0x0D, 0x10, 0x00, 0x01, 0x00, 0x00, 0x01, 0x0A, 0xF6, 0x00, 0x02, 0x00, 0x00, 0x00 };

struct nfc_cmd nfc_cmd_list[] = {
    {nfc_test_cmd_1, sizeof(nfc_test_cmd_1) / sizeof(nfc_test_cmd_1[0])}
    ,
    {nfc_test_cmd_2, sizeof(nfc_test_cmd_2) / sizeof(nfc_test_cmd_2[0])}
    ,
    {nfc_test_cmd_3, sizeof(nfc_test_cmd_3) / sizeof(nfc_test_cmd_3[0])}
    ,
};

const unsigned char nfc_test_result_mark[] = { 0x6f, 0x01, 0x0d, 0x10 };

/**
* Defined case run in mmi mode,this mode support UI.
*
*/
static int is_nfc_test_result(unsigned char *p_data) {
    unsigned int i = 0;

    for(i = 0; i < (sizeof(nfc_test_result_mark) / sizeof(nfc_test_result_mark[0])); i++) {
        if((p_data[i] == nfc_test_result_mark[i]) &&
           (i = sizeof(nfc_test_result_mark) / sizeof(nfc_test_result_mark[0]) - 1)) {
            return 1;
        } else {
            break;
        }
    }
    return 0;
}

/*====================================================================================
FUNCTION   nfc_cback

DESCRIPTION
  This is the call back function which will indicate if the nfc hal open is successful
  or failed.

DEPENDENCIES
  NIL

RETURN VALUE
none

SIDE EFFECTS
  NONE

=====================================================================================*/
static void nfc_cback(unsigned char event, unsigned char status) {
    switch (event) {
    case HAL_NFC_OPEN_CPLT_EVT:
        if(status == HAL_NFC_STATUS_OK) {
            /* Release semaphore to indicate that hal open is done
               and change the state to write. */
            hal_opened = TRUE;
            ALOGI("HAL Open Success..state changed to Write  \n");
            sem_post(&semaphore_nfchal_ready);
        } else {
            ALOGE("HAL Open Failed \n");
            hal_state = NCI_HAL_ERROR;
            hal_opened = FALSE;
            sem_post(&semaphore_halcmd_complete);
        }
        break;
    case HAL_NFC_CLOSE_CPLT_EVT:
        ALOGI("HAL_NFC_CLOSE_CPLT_EVT recieved..\n");
        break;
    default:
        ALOGI("nfc_hal_cback unhandled event %x \n", event);
        break;
    }
}

/*==========================================================================================
FUNCTION   fill_async_data

DESCRIPTION
This function will store all the incoming async msgs( like ntfs and data from QCA1990)
in to a list to be committed further.

DEPENDENCIES
  NIL

RETURN VALUE
  NONE

SIDE EFFECTS
  NONE

==============================================================================================*/
static void fill_async_data(unsigned short data_len, unsigned char *p_data) {
    asyncdata_t *next_node = NULL;

    /* Initialize a list which will store all async message untill they are sent */
    if(buff == NULL) {
        /* first node creation */
        buff = (asyncdata_t *) malloc(sizeof(asyncdata_t));
        if(buff) {
            start = buff;
            buff->response_buff = (unsigned char *) malloc(data_len);
            if(buff->response_buff) {
                memcpy(buff->response_buff, p_data, data_len);
                buff->async_datalen = data_len;
                buff->next = NULL;
            } else {
                ALOGE("mem allocation failed while storing asysnc msg \n");
            }
        } else {
            ALOGE("mem allocation failed while trying to make the async list \n");
        }
    } else {
        /* this is the case when some data is already present in the list which has not been sent yet */
        next_node = (asyncdata_t *) malloc(sizeof(asyncdata_t));
        if(next_node) {
            next_node->response_buff = (unsigned char *) malloc(data_len);
            if(next_node->response_buff) {
                memcpy(next_node->response_buff, p_data, data_len);
                next_node->async_datalen = data_len;
                next_node->next = NULL;
                while(buff->next != NULL) {
                    buff = buff->next;
                }
                buff->next = next_node;
            } else {
                ALOGE("mem allocation failed while storing asysnc msg \n");
            }
        } else {
            ALOGE("mem allocation failed while trying to make the async list \n");
        }
    }
}

/*======================================================================================================
FUNCTION   nfc_data_cback

DESCRIPTION
  This is the call back function which will provide back incoming data from the QCA1990
  to nfc ftm.

DEPENDENCIES
  NIL

RETURN VALUE
  NONE

SIDE EFFECTS
  NONE

========================================================================================================*/
static void nfc_data_cback(uint16_t data_len, unsigned char *p_data) {
    if((data_len == 0x00) || (p_data == NULL)) {
        ALOGE("Error case : wrong data lentgh or buffer revcieved \n");
        return;
    }
    if(((p_data[0] & 0xF0) == 0x60 /*ntf packets */ ) || ((p_data[0] & 0xF0) == 0x00) /*data packet rsps */ ) {
        pthread_mutex_lock(&nfc_mutex);
        fill_async_data(data_len, p_data);
        pthread_mutex_unlock(&nfc_mutex);
        if((wait_rsp == FALSE) || ((p_data[0] == 0x60) && (p_data[1] == 0x00))) {
            /*This is the case when ntf receieved after rsp is logged to pc app */
            ALOGI("Sending async msg to logging subsystem \n");
            if(is_nfc_test_result(p_data)) {
                ALOGI("nfc_data_cback :: nfc test result");
                sem_post(&semaphore_nfcdetect_complete);
            } else {
                hal_state = NCI_HAL_ASYNC_LOG;
                sem_post(&semaphore_halcmd_complete);
            }
        }
    } else {
        if(response_buff || res_len) {
            return;
        }

        response_buff = (unsigned char *) malloc(data_len);
        if(response_buff) {
            memcpy(response_buff, p_data, data_len);
            res_len = data_len;
            hal_state = NCI_HAL_READ;
            sem_post(&semaphore_halcmd_complete);
        } else {
            ALOGE("Mem allocation failed in nfc_data_cback \n");
        }
    }
}

/*===========================================================================
FUNCTION   nfc_hal_open

DESCRIPTION
   This function will open the nfc hal for ftm nfc command processing.

DEPENDENCIES
  NIL

RETURN VALUE
  void

SIDE EFFECTS
  NONE

===============================================================================*/
static unsigned char nfc_hal_open(void) {
    unsigned char ret = 0;
    const hw_module_t *hw_module = NULL;

    ret = hw_get_module(NFC_NCI_HARDWARE_MODULE_ID, &hw_module);
    if(ret == 0) {
        dev = (nfc_nci_device_t *) malloc(sizeof(nfc_nci_device_t));
        if(!dev) {
            ALOGE("NFC FFBM : mem allocation failed \n");
            return FALSE;
        } else {
            ret = nfc_nci_open(hw_module, &dev);
            if(ret != 0) {
                ALOGE("NFC FFBM : nfc_nci_open fail \n");
                free(dev);
                return FALSE;
            } else {
                ALOGI("NFC FFBM : opening NCI HAL \n");
                dev->common.reserved[0] = FTM_MODE;
                dev->open(dev, nfc_cback, nfc_data_cback);
                sem_wait(&semaphore_halcmd_complete);
            }
        }
    } else {
        ALOGE("NFC FFBM : hw_get_module() call failed \n");
        return FALSE;
    }
    return TRUE;
}

/*=================================================================================================
FUNCTION   nfc_log_send_msg

DESCRIPTION
This function will log the asynchronous messages(NTFs and data packets) to the logging subsystem
 of DIAG.

DEPENDENCIES

RETURN VALUE
TRUE if data logged successfully and FALSE if failed.

SIDE EFFECTS
  None

==================================================================================================*/
static int nfc_update_status(void) {

    asyncdata_t *node = NULL;

    buff = start;
    if(buff != NULL) {
        do {
            node = buff;
            buff = buff->next;
            free(node);
        } while(buff != NULL);
        ALOGI("all msgs committed \n");
        return TRUE;
    } else {
        ALOGI("No async message left to be logged \n");
    }

    return FALSE;
}

/*===========================================================================
FUNCTION   nfc_thread

DESCRIPTION
  Thread Routine to perfom asynchrounous handling of events coming from
  NFCC. It will perform read and write for all type of commands/data.

DEPENDENCIES

RETURN VALUE
  RETURN NIL

SIDE EFFECTS
  None

===========================================================================*/
static void *nfc_thread(void *) {
    while(1) {
        ALOGI("Waiting for Cmd/Rsp \n");
        sem_wait(&semaphore_halcmd_complete);

        switch (hal_state) {
        case NCI_HAL_INIT:
            ALOGI("NFC FFBM : HAL Open request recieved..\n");
            if(nfc_hal_open() == FALSE) {
                hal_state = NCI_HAL_ERROR;
                hal_opened = FALSE;
            } else {
                break;
            }
        case NCI_HAL_ERROR:
            /* HAL open failed.Post sem and handle error case */
            sem_post(&semaphore_nfccmd_complete);
            break;
        case NCI_HAL_WRITE:
            if(dev != NULL) {
                ALOGI("NFC FFBM : Cmd recieved for nfc..sending.\n");
                /* send data to the NFCC */
                ALOGI("cmd request arrived \n");
                wait_rsp = TRUE;
                dev->write(dev, len, nfc_cmd_buff);
            } else {
                ALOGI("dev is null \n");
            }
            break;
        case NCI_HAL_READ:
            /* indicate to ftm that response is avilable now */
            sem_post(&semaphore_nfccmd_complete);
            ALOGI("NFC FFBM : State changed to READ \n");
            break;
        case NCI_HAL_ASYNC_LOG:
            /* indicate to ftm that response is avilable now */
            ALOGI("NFC FFBM : State changed to NCI_HAL_ASYNC_LOG.Logging aysnc message \n");
            pthread_mutex_lock(&nfc_mutex);
            if(nfc_update_status()) {
                ALOGI("async msgs commited to the log system..changing HAL state to write \n");
            } else {
                ALOGE("async msgs commit failed..changing HAL state to write \n");
            }
            hal_state = NCI_HAL_WRITE;
            pthread_mutex_unlock(&nfc_mutex);
            break;
        default:
            break;
        }
    }
    return NULL;
}

/*===========================================================================
FUNCTION   nfc_send_command

DESCRIPTION
This is the function which will be called by the NFC FFBM layer callback function
registered with the DIAG service./

DEPENDENCIES

RETURN VALUE
  RETURN rsp pointer(containing the NFCC rsp packets) to the callback function
  (subsequently for DIAG service)

SIDE EFFECTS
  None

===========================================================================*/
static int nfc_send_command(struct nfc_cmd *command) {
    struct timespec time_sec;
    int sem_status;

    ALOGI("NFC FFBM : nfc mode requested \n");

    if(command == NULL) {
        ALOGE("Error : NULL packet recieved from DIAG \n");
        return -1;
    }

    /*copy command to send it further to QCA1990 */
    len = command->len;
    nfc_cmd_buff = (unsigned char *) command->cmd;
    hal_state = NCI_HAL_WRITE;
    /*send the command */
    sem_post(&semaphore_halcmd_complete);

    ALOGI("waiting for nfc response \n");

    if(clock_gettime(CLOCK_REALTIME, &time_sec) == -1) {
        ALOGI("get clock_gettime error");
    }
    time_sec.tv_sec += 5;

    sem_status = sem_timedwait(&semaphore_nfccmd_complete, &time_sec);

    if(sem_status == -1) {
        ALOGE("nfc command timed out\n");
        return -1;
    }

    if(!hal_opened) {
        /*Hal open is failed */
        hal_state = NCI_HAL_INIT;
        return -1;
    }

    ALOGI("\n\n *****Framing the response to send back to Diag service******** \n\n");
    /* Frame the response as per the cmd request */
    ALOGI("Framing the response for FTM_NFC_NFCC_COMMAND cmd \n");
    if(response_buff && res_len) {
        free(response_buff);
        response_buff = 0;
        res_len = 0;
    }

    wait_rsp = FALSE;

    return 0;
}

static int init(hash_map < string, string > *params) {
    int ret = -1;

    temp = params;
    hal_state = NCI_HAL_INIT;
    hal_opened = FALSE;
    wait_rsp = FALSE;
    res_len = 0;
    len = 0;

    /* Start nfc_thread which will process all requests as per
       state machine flow. By Default First state will be NCI_HAL_INIT */
    if(sem_init(&semaphore_halcmd_complete, 0, 1) != 0) {
        ALOGE("NFC FFBM :semaphore_halcmd_complete creation failed \n");
        return -1;
    }
    if(sem_init(&semaphore_nfccmd_complete, 0, 0) != 0) {
        ALOGE("NFC FFBM :semaphore_nfccmd_complete creation failed \n");
        return -1;
    }
    if(sem_init(&semaphore_nfcdetect_complete, 0, 0) != 0) {
        ALOGE("NFC FFBM :semaphore_nfcdetect_complete creation failed \n");
        return -1;
    }
    if(sem_init(&semaphore_nfchal_ready, 0, 0) != 0) {
        ALOGE("NFC FFBM :semaphore_nfcdetect_complete creation failed \n");
        return -1;
    }

    ret = pthread_create(&nfc_thread_handle, NULL, nfc_thread, NULL);
    if(ret < 0) {
        ALOGE("can't create pthread: %s\n", strerror(errno));
        return -1;
    }

    init_finish = 1;
    return 0;
}

static void *run_test(void *mod) {
    mmi_module_t *module = (mmi_module_t *) mod;
    unsigned int i = 0;
    char str[128] = { 0 };
    char str_temp[128] = { 0 };
    asyncdata_t *psync = NULL;
    int ret = -1;


    signal(SIGUSR1, signal_handler);
    sem_wait(&semaphore_nfchal_ready);

    module->cb_print(NULL, SUBCMD_MMI, "start to send command", 21, PRINT);
    /* Send command to NFC chip */
    for(i = 0; i < sizeof(nfc_cmd_list) / sizeof(nfc_cmd_list[0]); i++) {
        ret = nfc_send_command(&nfc_cmd_list[i]);
        if(ret != 0) {
            return NULL;
        }
    }

    module->cb_print(NULL, SUBCMD_MMI, "succeed to send command", 23, PRINT);

    sem_wait(&semaphore_nfcdetect_complete);
    if(buff != NULL)
        psync = buff->next;

    if(psync != NULL && psync->response_buff != NULL) {
        if(psync->async_datalen > 4 && psync->response_buff[4] == 0)
            snprintf(str, sizeof(str), "%s: %s", "uicc_detect", "detected\n");
        else
            snprintf(str, sizeof(str), "%s: %s", "uicc_detect", "not_detected\n");

        if(psync->async_datalen > 8 && psync->response_buff[8] == 0)
            snprintf(str_temp, sizeof(str), "%s: %s", "antenna_detect", "detected\n");
        else
            snprintf(str_temp, sizeof(str), "%s: %s", "antenna_detect", "not_detected\n");
        strlcat(str, str_temp, sizeof(str));

        if(psync->async_datalen > 12 && psync->response_buff[12] == 0)
            snprintf(str_temp, sizeof(str), "%s: %s", "carrier_detect", "detected");
        else
            snprintf(str_temp, sizeof(str), "%s: %s", "carrier_detect", "not_detected");
        strlcat(str, str_temp, sizeof(str));
    }
    module->cb_print(NULL, SUBCMD_MMI, str, strlen(str), PRINT_DATA);

    hal_state = NCI_HAL_ASYNC_LOG;
    sem_post(&semaphore_halcmd_complete);

    while(1);

    return NULL;
}

static void nfc_hal_close(void) {

    hal_state = NCI_HAL_INIT;
    hal_opened = FALSE;
    wait_rsp = FALSE;
    if(dev != NULL) {
        if(nfc_nci_close(dev) != 0)
            ALOGE("NFC FFBM : nfc_nci_close fail \n");
    }
}

static void mem_free(void) {

    if(response_buff != NULL) {
        free(response_buff);
        response_buff = NULL;
    }

    nfc_cmd_buff = NULL;
    buff = NULL;
    start = NULL;
}

static void deinit(const mmi_module_t * mod) {
    nfc_hal_close();
    mem_free();
    wait_rsp = false;
    init_finish = 0;
}

static int32_t module_run_mmi(const mmi_module_t * module, hash_map < string, string > &params) {
    ALOGI("%s start ", __FUNCTION__);
    int ret = FAILED;

    if(!init_finish) {
        ret = init(&params);
        if(ret != SUCCESS) {
            ALOGE("FFBM NFC : fail to initialize");
            return ret;
        }
    }

    ret = pthread_create((pthread_t *) & module->run_pid, NULL, run_test, (void *) module);
    if(ret < 0) {
        ALOGE("%s:Can't create pthread: %s\n", __FUNCTION__, strerror(errno));
        return FAILED;
    } else {
        pthread_join(module->run_pid, NULL);
    }

    deinit(module);

    return ret;
}

/**
* Defined case run in PCBA mode, fully automatically.
*
*/
static int32_t module_run_pcba() {
    ALOGI("%s start", __FUNCTION__);
    return ERR_UNKNOW;
}

static int32_t module_init(const mmi_module_t * module, hash_map < string, string > &params) {
    ALOGI("%s start ", __FUNCTION__);
    int ret = 0;

    if(module == NULL) {
        ALOGE("%s NULL point  received ", __FUNCTION__);
        return FAILED;
    }
    if(!init_finish)
        ret = init(&params);

    ALOGE("FFBM NFC : initialize finished with rc = %d", ret);

    return ret;
}

static int32_t module_deinit(const mmi_module_t * module) {
    ALOGI("%s start.", __FUNCTION__);
    if(module == NULL) {
        ALOGE("%s NULL point  received ", __FUNCTION__);
        return FAILED;
    }
    return SUCCESS;
}

static int32_t module_stop(const mmi_module_t * module) {
    ALOGI("%s start.", __FUNCTION__);
    if(module == NULL) {
        ALOGE("%s NULL point  received ", __FUNCTION__);
        return FAILED;
    }

    pthread_kill(module->run_pid, SIGUSR1);
    return SUCCESS;
}

/**
* Before call Run function, caller should call module_init first to initialize the module.
* the "cmd" passd in MUST be defined in cmd_list ,mmi_agent will validate the cmd before run.
*
*/
static int32_t module_run(const mmi_module_t * module, const char *cmd, hash_map < string, string > &params) {
    int ret = FAILED;

    if(!module || !cmd) {
        ALOGE("%s NULL point  received ", __FUNCTION__);
        return FAILED;
    }
    ALOGI("%s start.command : %s", __FUNCTION__, cmd);

    if(!strcmp(cmd, SUBCMD_MMI))
        ret = module_run_mmi(module, params);
    else if(!strcmp(cmd, SUBCMD_PCBA))
        ret = module_run_pcba();
    else {
        ALOGE("%s Invalid command: %s  received ", __FUNCTION__, cmd);
        ret = FAILED;
    }

   /** Default RUN mmi*/
    return ret;
}

/**
* Methods must be implemented by module.
*/
static struct mmi_module_methods_t module_methods = {
    .module_init = module_init,
    .module_deinit = module_deinit,
    .module_run = module_run,
    .module_stop = module_stop,
};

/**
* Every mmi module must have a data structure named MMI_MODULE_INFO_SYM
* and the fields of this data structure must be initialize in strictly sequence as definition,
* please don't change the sequence as g++ not supported in CPP file.
*/
mmi_module_t MMI_MODULE_INFO_SYM = {
    .version_major = 1,
    .version_minor = 0,
    .name = "NFC",
    .author = "Qualcomm Technologies, Inc.",
    .methods = &module_methods,
    .module_handle = NULL,
    .supported_cmd_list = NULL,
    .supported_cmd_list_size = 0,
    .cb_print = NULL, /**it is initialized by mmi agent*/
    .run_pid = -1,
};
