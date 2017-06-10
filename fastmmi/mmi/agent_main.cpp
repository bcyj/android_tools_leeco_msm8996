/*
 * Copyright (c) 2014-2015, Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
#include "agent.h"

static mmi_module_t *g_module = NULL;
static char *g_module_name = NULL;
static int g_sock = -1;
static msg_queue_t g_msg_queue;
static sem_t g_msg_sem;

/**
 * Load the file defined by the variant and if successful
 * return the dlopen handle and the mmi.
 * @return 0 = success, !0 = failure.
 */
static int load_module(const char *path, mmi_module_t ** pModule) {
    int ret;
    void *handle;
    mmi_module_t *module;
    char const *errstr;
    const char *sym = MMI_MODULE_INFO_SYM_AS_STR;

    if(path == NULL) {
        ALOGE("%s: error parameter module name is null", __FUNCTION__);
        return -1;
    }

    if(access(path, R_OK)) {
        ALOGE("%s not found\n", path);
        return -1;
    }

    ALOGI("%s: start with: lib:%s", __FUNCTION__, path);
    /**
     * load the symbols resolving undefined symbols before
     * dlopen returns. Since RTLD_GLOBAL is not or'd in with
     * RTLD_NOW the external symbols will not be global
     */
    handle = dlopen(path, RTLD_NOW);
    if(handle == NULL) {
        errstr = dlerror();
        ALOGE("load fail: module path =%s\n%s, can not open dl", path, errstr ? errstr : "unknown");
        ret = -EINVAL;
        goto done;
    }

    /** Get the address of the struct mmi_module_info. */
    module = (mmi_module_t *) dlsym(handle, sym);
    if(module == NULL) {
        ALOGE("load fail: couldn't find symbol %s", sym);
        ret = -EINVAL;
        goto done;
    }

    module->module_handle = handle;

    /** success */
    ret = 0;

  done:
    ALOGI("load done dl with ret=%d", ret);
    if(ret != 0) {
        module = NULL;
        if(handle != NULL) {
            ALOGE("load fail: close dl");
            dlclose(handle);
            handle = NULL;
        }
    } else {
        ALOGI("loaded mmi module  path=%s module=%p handle=%p", path, *pModule, handle);
    }

    *pModule = module;
    if(module != NULL) {
        ALOGI("%s:Load module %s: %s (%p,name:%s,author:%s,handle:%p)", __FUNCTION__, path,
              ret ? "Fail" : "Success", module, module->name, module->author, module->module_handle);
    }

    return ret;
}

/**
 * cmd handler
 * @return 0 = success, !0 = failure.
 */
int print_msg(const char *module_name, const char *subcmd, const char *str, uint32_t size, print_type_t type) {

    msg_t msg;

    if(str == NULL || strlen(str) == 0) {
        ALOGI("%s,No message need to send, current sock=%d", __FUNCTION__, g_sock);
        return -1;
    }
    memset(&msg, 0, sizeof(msg));

    if(module_name == NULL || strlen(module_name) == 0)
        strlcpy(msg.module, g_module_name, sizeof(msg.module));
    else
        strlcpy(msg.module, module_name, sizeof(msg.module));

    if(subcmd != NULL)
        strlcpy(msg.subcmd, subcmd, sizeof(msg.subcmd));

    strlcpy(msg.msg, str, sizeof(msg.msg));
    msg.length = size;
    msg.msg_id = type;          //Use the field
    msg.cmd = CMD_PRINT;

    ALOGI("%s[%s,cmd:%s],message(%s) through sock fd:%d", __FUNCTION__, msg.module, msg.subcmd, str,g_sock);

    send_msg(g_sock, &msg);
    return 0;
}

/**
 * cmd handler
 * @return 0 = success, !0 = failure.
 */
static void *msg_process_thread(void *req) {

    int ret = 0;

    if(req == NULL)
        return NULL;

    msg_t *preq = (msg_t *) req;
    msg_t resp;

    /**Initial resp header*/
    memcpy(&resp, preq, sizeof(msg_t));
    ALOGI("%s,Process command: (%s, %d,%s), param:%s)", __FUNCTION__, preq->module, preq->cmd, preq->subcmd, preq->msg);

    switch (preq->cmd) {
    case CMD_INIT:
        ret = handle_init(g_module, preq, &resp);
        break;
    case CMD_DEINIT:
        ret = handle_deinit(g_module, preq, &resp);
        break;

    case CMD_QUERY:
        ret = handle_query(g_module, preq, &resp);
        break;

    case CMD_RUN:
        ret = handle_run(g_module, preq, &resp);
        break;

    case CMD_STOP:
        ret = handle_stop(g_module, preq, &resp);
        break;

    default:
        ret = -1;
        resp.result = FAILED;
        ALOGE("%s,Invalid command received(%d) is invalid)", __FUNCTION__, preq->cmd);
        break;
    }

    send_msg(g_sock, &resp);

     /**Free the req momery*/
    if(preq != NULL)
        free(preq);

    return NULL;
}

/**
 * Handle message thread to read message from pending message queue
 * @Never return.
 */
static void *msg_handle_thread(void *) {

    list < msg_t * >::iterator iter;
    pthread_t ptid;
    int ret = -1;
    msg_t *msg = NULL;

    while(1) {

        sem_wait(&g_msg_sem);

        dequeue_msg(&g_msg_queue, &msg);
        ALOGI("Get message, module:%s", msg->module);
        if(msg != NULL) {
             /**Start one single thread for each request */
            ret = pthread_create(&ptid, NULL, msg_process_thread, msg);
            if(ret < 0) {
                ALOGE("%s:Can't create msg_dispatcher pthread for cmd(%s,%d)  %s\n", __FUNCTION__, msg->module,
                      msg->cmd, strerror(errno));
                continue;
            }
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
    const char *usage = "mmi_agent usage:\n\
                                    mmi_agent -m [module path] \n\
                                    mmi_agent -p [module parameters] \n";

    ALOGI("%s\n", usage);
}

static void sigterm_handler(int sig) {
    /**Quit*/
    if(g_module != NULL) {
        g_module->methods->module_stop(g_module);
        g_module->methods->module_deinit(g_module);
    }
}

int main(int argc, char *argv[]) {

    int ch = 0, ret = 0;
    const char *lib_path = NULL;

    hash_map < string, string > module_params;
    pthread_t pid_wait, pid_handle;

    signal(SIGTERM, sigterm_handler);

    ALOGI("%s mmi_agent start", __FUNCTION__);
    sem_init(&g_msg_sem, 0, 0);

    while((ch = getopt(argc, argv, "m:p:")) != EOF) {
        switch (ch) {
        case 'm':
            g_module_name = strdup(optarg);
            if(g_module_name == NULL) {
                ALOGE("%s: out of memory", __FUNCTION__);
                exit(-1);
            }
            ALOGI("%s: g_module_name(%s)", __FUNCTION__, g_module_name);
            break;
        case 'p':
            parse_parameter(optarg, module_params);
            ALOGI("%s: get module_params", __FUNCTION__);
            break;
        default:
            usage();
            exit(-1);
            break;
        }
    }

    if(g_module_name == NULL) {
        ALOGE("%s:Please specify module", __FUNCTION__);
        goto out;
    }

    lib_path = module_params["lib_path"].c_str();
    if(lib_path == NULL || strlen(lib_path) == 0) {
        ALOGE("%s:Please specify module lib", __FUNCTION__);
        goto out;
    }
    ALOGE("%s:lib path: %s ", __FUNCTION__, lib_path);

    ret = load_module(lib_path, &g_module);
    if(ret < 0)
        goto out;

    /** Initial Callback function*/
    g_module->cb_print = print_msg;

    /** Connect to MMI server via socket*/
    g_sock = connect_server(MMI_SOCKET);
    if(g_sock < 0)
        goto out;

     /**Ready, say hello to server*/
    ret = say_hello(g_sock, g_module_name);
    if(ret) {
        ALOGE("%s:Say Hello Fail", __FUNCTION__);
        goto out;
    }

    /** Call initialize function when module start*/
    ret = g_module->methods->module_init(g_module, module_params);
    if(ret) {
        ALOGE("%s: Call module_init fail (%s)", __FUNCTION__, lib_path);
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
    usleep(rand() % 10000 * 1000);
     /**Something wrong happen, sleep random time let the father process
      could handle SIGCHLD for all childrens. */
    if(g_module_name != NULL) {
        ALOGI("%s: mmi agent exit to clear resource for %s", __FUNCTION__, g_module_name);
        free(g_module_name);
    }
    sem_close(&g_msg_sem);
    exit(1);
}
