/* Copyright (c) 2010, Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
#ifndef __GPSONE_THREAD_HELPER_H__
#define __GPSONE_THREAD_HELPER_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <pthread.h>

struct gpsone_thelper {
    unsigned char   thread_exit;
    unsigned char   thread_ready;
    pthread_cond_t  thread_cond;
    pthread_mutex_t thread_mutex;
    pthread_t       thread_id;
    void *          thread_context;
    int             (*thread_proc_init) (void * context);
    int             (*thread_proc_pre)  (void * context);
    int             (*thread_proc)      (void * context);
    int             (*thread_proc_post) (void * context);
};

#define gpsone_launch_thelper(helper, init, pre, proc, post, context) \
 gpsone_launch_thelper_w_name(helper, \
                              init, \
                              pre, \
                              proc, \
                              post, \
                              context, \
                              #proc)

int gpsone_launch_thelper_w_name(struct gpsone_thelper * thelper,
    int (*thread_proc_init) (void * context),
    int (*thread_proc_pre)  (void * context),
    int (*thread_proc)      (void * context),
    int (*thread_proc_post) (void * context),
    void * context,
    char* name);

int gpsone_unblock_thelper(struct gpsone_thelper * thelper);
int gpsone_join_thelper(struct gpsone_thelper * thelper);

/* if only need to use signal */
int thelper_signal_init(struct gpsone_thelper * thelper);
int thelper_signal_destroy(struct gpsone_thelper * thelper);
int thelper_signal_wait(struct gpsone_thelper * thelper);
int thelper_signal_timedwait(struct gpsone_thelper * thelper, int tv_sec);
int thelper_signal_ready(struct gpsone_thelper * thelper);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __GPSONE_THREAD_HELPER_H__ */
