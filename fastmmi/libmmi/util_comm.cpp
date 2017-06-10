/*
 * Copyright (c) 2014, Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
#include "common.h"
#include "utils.h"

int create_socket(const char *name) {
    struct sockaddr_un addr;
    int sockfd, ret;

    sockfd = socket(PF_UNIX, SOCK_STREAM, 0);
    if(sockfd < 0) {
        ALOGD("Failed to open socket '%s': %s\n", name, strerror(errno));
        return -1;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    snprintf(addr.sun_path, sizeof(addr.sun_path), "%s", name);

    ret = unlink(addr.sun_path);
    if(ret != 0 && errno != ENOENT) {
        ALOGD("Failed to unlink old socket '%s': %s\n", name, strerror(errno));
        close(sockfd);
        return -1;
    }

    ret = bind(sockfd, (struct sockaddr *) &addr, sizeof(addr));
    if(ret) {
        ALOGD("Failed to bind socket '%s': %s\n", name, strerror(errno));
        unlink(addr.sun_path);
        close(sockfd);
        return -1;
    }

    chmod(addr.sun_path, (mode_t) 0660);
    ALOGD("Created socket %s with sockfd=%d\n", addr.sun_path, sockfd);

    return sockfd;
}


/**
 * Connect to server, Never return if connect fail
 * @return Sock fd.
 */
int connect_server(const char *path) {

    struct sockaddr_un server;
    socklen_t alen = 0;
    int sock, ret = 0;

    if(path == NULL)
        return -1;

    sock = socket(PF_UNIX, SOCK_STREAM, 0);
    if(sock < 0) {
        ALOGE("Failed to open socket '%s': %s\n", path, strerror(errno));
        return -1;
    }
    /** Initialize address structure */
    memset(&server, 0, sizeof(struct sockaddr_un));

    /** Set address family to unix domain sockets */
    server.sun_family = AF_UNIX;

    /** Set address to the requested pathname */
    snprintf(server.sun_path, sizeof(server.sun_path), "%s", path);

    /** Get length of pathname */
    alen = strlen(server.sun_path) + sizeof(server.sun_family);

    while(1) {
        ret = connect(sock, (struct sockaddr *) &server, alen);
        if(ret == 0)
            break;

        sleep(1);
    }
    ALOGI("Connected to server socket '%s': sock=%d", path, sock);

    return sock;
}

/**
 * Say Hello to server
 * @return 0 = success, !0 = failure.
 */
int say_hello(int sock, const char *module_name) {
    return send_cmd(sock, module_name, CMD_HELLO, NULL, NULL, 0);
}

/**Static basical function for running specified cmd*/
int send_cmd(int sock, const char *module_name, int cmd, const char *subcmd, const char *params, int params_size) {

    msg_t msg;
    int ret = -1;

    if(sock < 0 || module_name == NULL) {
        ALOGE("%s,Invalid socket id(%d) or module name", __FUNCTION__, sock);
        return -1;
    }

    memset(&msg, 0, sizeof(msg_t));
    strlcpy(msg.module, module_name, sizeof(msg.module));
    msg.cmd = cmd;

    if(subcmd != NULL)
        strlcpy(msg.subcmd, subcmd, sizeof(msg.subcmd));

    if(params != NULL) {
        strlcpy(msg.msg, params, sizeof(msg.msg));
        msg.length = params_size;
    }

    ALOGI("SEND COMMAND [%s]: cmd[%d,%s],param[%s, %d],through sock:%d", msg.module, msg.cmd, msg.subcmd, msg.msg,
          msg.length, sock);

    ret = send_msg(sock, &msg);
    if(ret <= 0)
        return -1;

    return 0;
}

void enqueue_msg(msg_queue_t * queue, msg_t * msg) {

    ALOGI("%s: msg(module:%s,cmd:%d,subcmd:%s)", __FUNCTION__, msg->module, msg->cmd, msg->subcmd);
    pthread_mutex_lock(&queue->lock);
    queue->queue.push_back(msg);
    pthread_mutex_unlock(&queue->lock);
}

void dequeue_msg(msg_queue_t * queue, msg_t ** msg) {
    pthread_mutex_lock(&queue->lock);
    if(!queue->queue.empty()) {
        *msg = queue->queue.front();
        queue->queue.pop_front();
    }
    pthread_mutex_unlock(&queue->lock);
}

int send_msg(int fd, msg_t * msg) {
    int ret = 0;

    if(fd > 0) {
        ret = write(fd, msg, sizeof(msg_t));
    }
    return ret;
}

void signal_handler(int signal) {
    pthread_exit(NULL);
}

bool is_proc_exist(int pid) {
    return kill(pid, 0) == 0;
}

void kill_thread(pthread_t tid) {
    if(pthread_kill(tid, 0) == 0) {
        pthread_kill(tid, SIGUSR1);
    }
}

void kill_proc(int pid) {
    int stat;

    if(is_proc_exist(pid)) {
        kill(pid, SIGKILL);
        waitpid(pid, &stat, 0);
        if(WIFSIGNALED(stat))
            ALOGI("Child process received signal (%d).\n", WTERMSIG(stat));

    }
}
