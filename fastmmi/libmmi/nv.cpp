/*
 * Copyright (c) 2013-2014, Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
#include <pthread.h>
#include <stdlib.h>
#include <semaphore.h>
#include "diagcmd.h"
#include "utils.h"
#include "nv.h"

static rsp_t rspItem;
static sem_t sem_nv_rw_complete;
static mutex_locker m_lock;

/* Callback for the primary processor */
int process_diag_data(unsigned char *ptr, int len, void *context_data) {
    int i;
    unsigned int cmd_code = 0;


    ALOGI(" Process diag reply data");
    for(i = 0; i < len; i++) {
        ALOGI("%02x\t", ptr[i]);
    }

    if(len <= 0)
        return -1;

    cmd_code = ptr[0];
    if((cmd_code == DIAG_NV_READ_F) || (cmd_code == DIAG_NV_WRITE_F)) {
        ALOGI("NV read / write command: %d\n", cmd_code);
        rspItem.item_id = ptr[1] | ptr[2] << 8;
    }

    rspItem.length = len;
    memcpy(&rspItem.data, ptr, sizeof(rspItem.data));
    ALOGI(" rsp cmd_code,len (%d,%d) \n", cmd_code, rspItem.length);
    sem_post(&sem_nv_rw_complete);

    return 0;
}

static void process_reply(unsigned char *buf_ptr, int len) {

    int data_size = 0;

    sem_wait(&sem_nv_rw_complete);

    data_size = rspItem.length > len ? len : rspItem.length;
    ALOGI("\n process_reply cmd id %d\n", rspItem.data[0]);
    memcpy(buf_ptr, rspItem.data, data_size);
}

static void send_comand(unsigned char *sendbuf, int send_len, unsigned char *replybuf, int reply_len) {

    mutex_locker::autolock _L(m_lock);
    /*swith to CALLBACK MODE, so diag response will process by register callback function */
    diag_switch_logging(CALLBACK_MODE, NULL);

    /*Initial global response data */
    memset(&rspItem, 0, sizeof(rsp_t));

    /*send the command to diag framework */
    diag_callback_send_data(0, sendbuf, send_len);

    /*waiting the command callback complete */
    process_reply(replybuf, reply_len);

    /*swith back to USB mode, so pc tool can use diag */
    diag_switch_logging(USB_MODE, NULL);
}

static void efs_sync() {
    efs2_diag_sync_no_wait_req_type req;
    efs2_diag_sync_no_wait_rsp_type resp;

     /*SYNC*/ req.hdr.cmd_code = DIAG_SUBSYS_CMD_F;
    req.hdr.subsys_id = DIAG_SUBSYS_FS;
    req.hdr.subsys_cmdcode = 48;

    req.sequence_number = rand() % 100;
    req.path[0] = '/';
    req.path[1] = '\0';

    send_comand((unsigned char *) &req, sizeof(req), (unsigned char *) &resp, sizeof(resp));
}

void register_callback(void) {
    int data_primary = 100;

    sem_init(&sem_nv_rw_complete, 0, 0);

    /* Register the callback for the primary processor */
    diag_register_callback(&process_diag_data, &data_primary);

    /* This is required to get the data through this app */
    ALOGI("diag: register callback\n");

}

/*===========================================================================

FUNCTION DIAG_NV_READ

DESCRIPTION
  This procedure encapsulates an NV read operation for the diag task.

DEPENDENCIES
  Assumes that diag_init has set up the NV command buffer, ncmd.

RETURN VALUE
  Status of NV operation.

SIDE EFFECTS
  None.

===========================================================================*/

int diag_nv_read(nv_items_enum_type item,   /*!< Which NV item to read */
                 unsigned char *data_ptr,   /*!< buffer pointer to put the read data */
                 int len) {
    unsigned char nv_read[NV_PKT_SIZE];

    memset(&nv_read, 0, sizeof(nv_read));
    nv_read[0] = DIAG_NV_READ_F;
    nv_read[1] = BYTE_PTR(item)[0];
    nv_read[2] = BYTE_PTR(item)[1];

    send_comand(nv_read, NV_PKT_SIZE, data_ptr, len);

    return 0;
}

/*===========================================================================

FUNCTION DIAG_NV_WRITE
DESCRIPTION
  This procedure encapsulates an NV write operation for the diag task.

DEPENDENCIES
  Assumes that diag_init has set up the NV command buffer, ncmd.

RETURN VALUE
  Status of NV operation.

SIDE EFFECTS
  None.

===========================================================================*/

int diag_nv_write(nv_items_enum_type item, unsigned char *data_ptr, int len) {
    unsigned char nv_write[NV_PKT_SIZE];
    int data_size = NV_PKT_SIZE - 3;

    if(len < NV_PKT_SIZE - 3)
        data_size = len;

    memset(&nv_write, 0, sizeof(nv_write));
    nv_write[0] = DIAG_NV_WRITE_F;
    nv_write[1] = BYTE_PTR(item)[0];
    nv_write[2] = BYTE_PTR(item)[1];
    memcpy(&nv_write[3], data_ptr, data_size);

    send_comand(nv_write, data_size + 3, data_ptr, len);

    /*sync */
    efs_sync();
    return 0;
}

/*===========================================================================

FUNCTION DIAG_NV_WRITE
DESCRIPTION
  This procedure encapsulates an stream diag command.

DEPENDENCIES
  Assumes that diag_init has set up.

RETURN VALUE
  None

SIDE EFFECTS
  None.

===========================================================================*/

int diag_send_cmd(unsigned char cmd, unsigned char *data_ptr, int len) {
    unsigned char buf[MAX_PKT_SIZE];
    int data_size = MAX_PKT_SIZE - 3;

    if(len < MAX_PKT_SIZE - 3)
        data_size = len;

    memset(&buf, 0, sizeof(buf));
    buf[0] = cmd;
    memcpy(&buf[2], data_ptr, data_size);

    send_comand(buf, data_size + 1, data_ptr, len);
    return 0;
}
