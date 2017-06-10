/*
 * Copyright (c) 2014, Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
#include "common.h"
#include "utils.h"

static int g_sock = -1;
static msg_queue_t g_msg_queue;
static sem_t g_msg_sem;

#ifdef ALOGI
#undef ALOGI
#define ALOGI printf
#endif

#ifdef ALOGE
#undef ALOGE
#define ALOGE printf
#endif

/**
 * cmd handler
 * @return 0 = success, !0 = failure.
 */
void send_cmd(const char *subcmd, const char *params, uint32_t size) {
    send_cmd(g_sock, CLIENT_DEBUG_NAME, CMD_CTRL, subcmd, params, size);
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
static void msg_handle(msg_t * msg) {

    if(msg == NULL)
        return;

    switch (msg->cmd) {

    case CMD_RESULT:
        ALOGI("%s", msg->msg);

        break;
    default:

        ALOGE("%s,Invalid command received(%d) is invalid)", __FUNCTION__, msg->cmd);
        break;
    }
}

/**
 * Handle message thread to read message from pending message queue
 * @Never return.
 */
static void *console_thread(void *) {

    char user_input[32];
    char case_name[32];

    while(1) {

        fgets(user_input, sizeof(user_input), stdin);
        ALOGI("Get command from user: %s\n", user_input);
        if(!strcmp(user_input, SUBCMD_STAT)) {
            send_cmd(SUBCMD_STAT);
        } else if(!strncmp(user_input, "run", 3)) {
            snprintf(case_name, sizeof(case_name), "%s:%s", KEY_CASE_NAME, &user_input[3]);
            ALOGI("run case:%s(size:%lu)\n", case_name, strlen(case_name));
            send_cmd(SUBCMD_RUNCASE, case_name, strlen(case_name));
        } else if(!strcmp(user_input, "runall")) {
            send_cmd(SUBCMD_RUNALL);
        }
    }

    return NULL;
}

/**
 * Handle message thread to read message from pending message queue
 * @Never return.
 */
static void *msg_handle_thread(void *) {

    msg_t *msg = NULL;

    while(1) {

        sem_wait(&g_msg_sem);
        dequeue_msg(&g_msg_queue, &msg);
        if(msg != NULL) {
            /**Skip not diag module command*/
            if(strcmp(msg->module, CLIENT_DEBUG_NAME)) {
                ALOGI("%s,not diag command: (%s, %d,%s))", __FUNCTION__, msg->module, msg->cmd, msg->subcmd);
                free(msg);
                continue;
            }
            msg_handle(msg);
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

        /** Enquenue the request which handled in a single thread*/
        enqueue_msg(&g_msg_queue, msg);

        /**Notify the handle thread*/
        sem_post(&g_msg_sem);
    }

    return NULL;
}

static void usage() {
    const char *usage = "mmi_debug";

    ALOGI("%s\n", usage);
}

int main(int argc, char *argv[]) {

    int ret = 0;

    pthread_t pid_wait, pid_handle, pid_console;

    sem_init(&g_msg_sem, 0, 0);
    ALOGI("%s mmi_debug start\n", __FUNCTION__);

    /** Connect to MMI server via socket*/
    g_sock = connect_server(MMI_SOCKET);
    if(g_sock < 0)
        goto out;

     /**Ready, say hello to server*/
    say_hello(g_sock, CLIENT_DEBUG_NAME);
    ALOGI("%s mmi_debug connected to server , sock: %d \n", __FUNCTION__, g_sock);

    ret = pthread_create(&pid_console, NULL, console_thread, NULL);
    if(ret < 0) {
        ALOGE("%s:Can't create msg waiting pthread: %s\n", __FUNCTION__, strerror(errno));
        goto out;
    }

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
    sem_close(&g_msg_sem);
    exit(1);
}
