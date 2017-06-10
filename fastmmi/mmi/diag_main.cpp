/*
 * Copyright (c) 2014, Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
#include "diag_ext.h"

static int g_sock = -1;
static msg_queue_t g_msg_queue;
static sem_t g_msg_sem;
static sem_t g_cmd_complete_sem;
msg_t g_msg;

/**
 * cmd handler
 * @return 0 = success, !0 = failure.
 */
void send_cmd(const char *subcmd, const char *params, uint32_t size) {
    send_cmd(g_sock, CLIENT_DIAG_NAME, CMD_CTRL, subcmd, params, size);
    sem_wait(&g_cmd_complete_sem);
}

/**
 * cmd handler
 * @return 0 = success, !0 = failure.
 */
void send_cmd(const char *subcmd) {
    send_cmd(subcmd, NULL, 0);
}

/**
 * cmd handler
 * @return 0 = success, !0 = failure.
 */
static int msg_handle(msg_t * msg) {

    int ret = -1;

    if(msg == NULL)
        return -1;

    switch (msg->cmd) {

    case CMD_CTRL:
        memcpy(&g_msg, msg, sizeof(msg_t));
        sem_post(&g_cmd_complete_sem);
        ret = 0;
        break;
    case CMD_RESULT:
        log_msg(msg->msg);
        break;
    default:
        ALOGE("%s,Invalid command received(%d) is invalid)", __FUNCTION__, msg->cmd);
        break;
    }

    return ret;
}

/**
 * Handle message thread to read message from pending message queue
 * @Never return.
 */
static void *msg_handle_thread(void *) {

    int ret = -1;
    msg_t *msg = NULL;

    while(1) {

        sem_wait(&g_msg_sem);
        dequeue_msg(&g_msg_queue, &msg);
        if(msg != NULL) {
            /**Skip not diag module command*/
            ALOGI("%s,Process command: (%s, %d,%s))", __FUNCTION__, msg->module, msg->cmd, msg->subcmd);
            if(strcmp(msg->module, CLIENT_DIAG_NAME)) {
                ALOGI("%s,not diag command: (%s, %d,%s))", __FUNCTION__, msg->module, msg->cmd, msg->subcmd);
                free(msg);
                continue;
            }
            ret = msg_handle(msg);
            if(ret < 0) {
                ALOGE("%s:Can't handle cmd(%s,%d)  %s\n", __FUNCTION__, msg->module, msg->cmd, strerror(errno));
            }
            free(msg);
        }
    }

    return NULL;
}

/**
 * Receive thread handle function
 * @Never return.
 */
static void *msg_waiting_thread(void *s) {

    int recv_size = -1, sock = -1;
    msg_t *msg = NULL;

    if(s == NULL) {
        ALOGE("%s: No sock id received", __FUNCTION__);
        return NULL;
    }

    sock = *(int *) s;

    while(1) {

        /** Receive a reply from the MMI server */
        msg = (msg_t *) zmalloc(sizeof(msg_t));
        if(msg == NULL) {
            ALOGE("%s:out of memory, abort the current request:(%s)\n", __FUNCTION__, strerror(errno));
            break;
        }

        if((recv_size = recv(sock, msg, sizeof(msg_t), MSG_WAITALL)) < 0) {
            ALOGE("%s:Recv fail: %s, received size:%d\n", __FUNCTION__, strerror(errno), recv_size);
            break;
        }

        ALOGI("%s:Received one message from server[%s,%d,%s],recv_size=%d", __FUNCTION__, msg->module, msg->cmd,
              msg->subcmd, recv_size);

        /** Enquenue the request which handled in a single thread*/
        enqueue_msg(&g_msg_queue, msg);

        /**Notify the handle thread*/
        sem_post(&g_msg_sem);
    }

    return NULL;
}

static void usage() {
    const char *usage = "mmi_diag";

    ALOGI("%s\n", usage);
}

int main(int argc, char *argv[]) {

    int ret = 0;

    pthread_t pid_wait, pid_handle;

    ALOGI("%s mmi_diag start", __FUNCTION__);
    sem_init(&g_msg_sem, 0, 0);
    sem_init(&g_cmd_complete_sem, 0, 0);

    ret = diag_init();
    if(ret < 0)
        goto out;

    /** Connect to MMI server via socket*/
    g_sock = connect_server(MMI_SOCKET);
    if(g_sock < 0)
        goto out;

     /**Ready, say hello to server*/
    say_hello(g_sock, CLIENT_DIAG_NAME);

    ret = pthread_create(&pid_wait, NULL, msg_waiting_thread, &g_sock);
    if(ret < 0) {
        ALOGE("%s:Can't create msg waiting pthread: %s\n", __FUNCTION__, strerror(errno));
        goto out;
    }

    ret = pthread_create(&pid_handle, NULL, msg_handle_thread, NULL);
    if(ret < 0) {
        ALOGE("%s:Can't create msg handle pthread: %s\n", __FUNCTION__, strerror(errno));
        goto out;
    }

    pthread_join(pid_wait, NULL);
    pthread_join(pid_handle, NULL);

  out:
    ALOGI("%s: mmi diag exit to clear resource", __FUNCTION__);
    diag_deinit();
    sem_close(&g_msg_sem);
    sem_close(&g_cmd_complete_sem);
    exit(1);
}
