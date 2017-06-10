/* Copyright (c) 2010, Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
#include <stdio.h>
#include <time.h>

#include "gpsone_thread_helper.h"
#include "gpsone_daemon_dbg.h"

#define MAX_TASK_COMM_LEN 15

/*===========================================================================
FUNCTION    thelper_signal_init

DESCRIPTION
   This function will initialize the conditional variable resources.

   thelper - thelper instance

DEPENDENCIES
   None

RETURN VALUE
   0: success or negative value for failure

SIDE EFFECTS
   N/A

===========================================================================*/
int thelper_signal_init(struct gpsone_thelper * thelper)
{
    int result;
    thelper->thread_ready = 0;
    result = pthread_cond_init( &thelper->thread_cond, NULL);
    if (result) {
        return result;
    }

    result = pthread_mutex_init(&thelper->thread_mutex, NULL);
    if (result) {
        pthread_cond_destroy(&thelper->thread_cond);
    }
    return result;
}

/*===========================================================================
FUNCTION    

DESCRIPTION
   This function will destroy the conditional variable resources

    thelper - pointer to thelper instance

DEPENDENCIES
   None

RETURN VALUE
   0: success or negative value for failure

SIDE EFFECTS
   N/A

===========================================================================*/
int thelper_signal_destroy(struct gpsone_thelper * thelper)
{
    int result, ret_result = 0;
    result = pthread_cond_destroy( &thelper->thread_cond);
    if (result) {
        ret_result = result;
    }

    result = pthread_mutex_destroy(&thelper->thread_mutex);
    if (result) {
        ret_result = result;
    }

    return ret_result;
}

/*===========================================================================
FUNCTION    thelper_signal_wait

DESCRIPTION
   This function will be blocked on the conditional variable until thelper_signal_ready
   is called

    thelper - pointer to thelper instance

DEPENDENCIES
   None

RETURN VALUE
   0: success or negative value for failure

SIDE EFFECTS
   N/A

===========================================================================*/
int thelper_signal_wait(struct gpsone_thelper * thelper)
{
    int result = -1;

    pthread_mutex_lock(&thelper->thread_mutex);
    if (!thelper->thread_ready) {
        result = pthread_cond_wait(&thelper->thread_cond, &thelper->thread_mutex);
    }
    pthread_mutex_unlock(&thelper->thread_mutex);

    return result;
}

int thelper_signal_timedwait(struct gpsone_thelper * thelper, int tv_sec)
{
    int result = -1;
    struct timespec ts;

    pthread_mutex_lock(&thelper->thread_mutex);
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec += tv_sec;

    if (!thelper->thread_ready) {
        result = pthread_cond_timedwait(&thelper->thread_cond, &thelper->thread_mutex, &ts);
    }
    pthread_mutex_unlock(&thelper->thread_mutex);

    return result;
}

/*===========================================================================
FUNCTION     thelper_signal_ready

DESCRIPTION
   This function will wake up the conditional variable

    thelper - pointer to thelper instance

DEPENDENCIES
   None

RETURN VALUE
   0: success or negative value for failure

SIDE EFFECTS
   N/A

===========================================================================*/
int thelper_signal_ready(struct gpsone_thelper * thelper)
{
    int result;

    GPSONE_DMN_DBG("%s:%d] 0x%lx\n", __func__, __LINE__, (long) thelper);

    pthread_mutex_lock(&thelper->thread_mutex);
    thelper->thread_ready = 1;
    result = pthread_cond_signal(&thelper->thread_cond);
    pthread_mutex_unlock(&thelper->thread_mutex);

    return result;
}

/*===========================================================================
FUNCTION    thelper_main

DESCRIPTION
   This function is the main thread. It will be launched as a child thread

    data - pointer to the instance

DEPENDENCIES
   None

RETURN VALUE
   NULL

SIDE EFFECTS
   N/A

===========================================================================*/
static void * thelper_main(void *data)
{
    int result = 0;
    struct gpsone_thelper * thelper = (struct gpsone_thelper *) data;

    if (thelper->thread_proc_init) {
        result = thelper->thread_proc_init(thelper->thread_context);
        if (result < 0) {
            thelper->thread_exit = 1;
            thelper_signal_ready(thelper);
            GPSONE_DMN_PR_ERR("%s:%d] error: 0x%lx\n", __func__, __LINE__, (long) thelper);
            return NULL;
        }
    }

    thelper_signal_ready(thelper);

    if (thelper->thread_proc_pre) {
        result = thelper->thread_proc_pre(thelper->thread_context);
        if (result < 0) {
            thelper->thread_exit = 1;
            GPSONE_DMN_PR_ERR("%s:%d] error: 0x%lx\n", __func__, __LINE__, (long) thelper);
            return NULL;
        }
    }

    do {
        if (thelper->thread_proc) {
            result = thelper->thread_proc(thelper->thread_context);
            if (result < 0) {
                thelper->thread_exit = 1;
                GPSONE_DMN_PR_ERR("%s:%d] error: 0x%lx\n", __func__, __LINE__, (long) thelper);
            }
        }
    } while (thelper->thread_exit == 0);

    if (thelper->thread_proc_post) {
        result = thelper->thread_proc_post(thelper->thread_context);
    }

    if (result != 0) {
        GPSONE_DMN_PR_ERR("%s:%d] error: 0x%lx\n", __func__, __LINE__, (long) thelper);
    }
    return NULL;
}

/*===========================================================================
FUNCTION    gpsone_launch_thelper

DESCRIPTION
   This function will initialize the thread context and launch the thelper_main

    thelper - pointer to thelper instance
    thread_proc_init - The initialization function pointer
    thread_proc_pre  - The function to call before task loop and after initialization
    thread_proc      - The task loop
    thread_proc_post - The function to call after the task loop
    context          - the context for the above four functions

DEPENDENCIES
   None

RETURN VALUE
   0: success or negative value for failure

SIDE EFFECTS
   N/A

===========================================================================*/
int gpsone_launch_thelper_w_name(struct gpsone_thelper * thelper,
    int (*thread_proc_init) (void * context),
    int (*thread_proc_pre) (void * context),
    int (*thread_proc) (void * context),
    int (*thread_proc_post) (void * context),
    void * context, char* name)
{
    int result;

    thelper->thread_exit  = 0;
    thelper_signal_init(thelper);

    if (context) {
        thelper->thread_context    = context;
    }

    thelper->thread_proc_init  = thread_proc_init;
    thelper->thread_proc_pre   = thread_proc_pre;
    thelper->thread_proc       = thread_proc;
    thelper->thread_proc_post  = thread_proc_post;

    GPSONE_DMN_DBG("%s:%d] 0x%lx call pthread_create\n", __func__, __LINE__, (long) thelper);
    result = pthread_create(&thelper->thread_id, NULL,
        thelper_main, (void *)thelper);

    if (result != 0) {
        GPSONE_DMN_PR_ERR("%s:%d] 0x%lx\n", __func__, __LINE__, (long) thelper);
        return -1;
    }

    if (NULL != name) {
        char lname[MAX_TASK_COMM_LEN+1];
        memcpy(lname, name, MAX_TASK_COMM_LEN);
        lname[MAX_TASK_COMM_LEN] = 0;
        result = pthread_setname_np(thelper->thread_id, lname);
        if (0 != result) {
            GPSONE_DMN_DBG("%s:%d] setname failure - %d\n", __func__, __LINE__, result);
        }
    }

    GPSONE_DMN_DBG("%s:%d] 0x%lx pthread_create done\n", __func__, __LINE__, (long) thelper);

    thelper_signal_wait(thelper);

    GPSONE_DMN_DBG("%s:%d] 0x%lx pthread ready\n", __func__, __LINE__, (long) thelper);
    return thelper->thread_exit;
}

/*===========================================================================
FUNCTION    gpsone_unblock_thelper

DESCRIPTION
   This function unblocks thelper_main to release the thread

    thelper - pointer to thelper instance

DEPENDENCIES
   None

RETURN VALUE
   0: success

SIDE EFFECTS
   N/A

===========================================================================*/
int gpsone_unblock_thelper(struct gpsone_thelper * thelper)
{
    GPSONE_DMN_DBG("%s:%d] 0x%lx\n", __func__, __LINE__, (long) thelper);
    thelper->thread_exit = 1;
    return 0;
}

/*===========================================================================
FUNCTION    gpsone_join_thelper

    thelper - pointer to thelper instance

DESCRIPTION
   This function will wait for the thread of thelper_main to finish

DEPENDENCIES
   None

RETURN VALUE
   0: success or negative value for failure

SIDE EFFECTS
   N/A

===========================================================================*/
int gpsone_join_thelper(struct gpsone_thelper * thelper)
{
    int result;

    GPSONE_DMN_DBG("%s:%d] 0x%lx\n", __func__, __LINE__, (long) thelper);
    result = pthread_join(thelper->thread_id, NULL);
    if (result != 0) {
        GPSONE_DMN_PR_ERR("%s:%d] 0x%lx\n", __func__, __LINE__, (long) thelper);
    }
    GPSONE_DMN_DBG("%s:%d] 0x%lx\n", __func__, __LINE__, (long) thelper);

    thelper_signal_destroy(thelper);

    return result;
}

