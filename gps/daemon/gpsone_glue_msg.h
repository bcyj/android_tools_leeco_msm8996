/* Copyright (c) 2010, Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
#ifndef GPSONE_GLUE_MSG_H
#define GPSONE_GLUE_MSG_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include <linux/types.h>
#include "gpsone_glue_pipe.h"
/* key_id
 *     'm': daemon manager ctrl task
 *     'b': connection bridge ctrl task
 *     'l': loc api
 */

/* All structures used with the message queue must start
   with a length field. You can expand on this structure.
 */
struct msgbuf {
    size_t msgsz;
};

int gpsone_glue_msgget(const char * q_path, int mode);
int gpsone_glue_msgremove(const char * q_path, int msgqid);
int gpsone_glue_msgsnd(int msgqid, const void * msgp, size_t msgsz);
int gpsone_glue_msgrcv(int msgqid, void *msgp, size_t msgsz);
int gpsone_glue_msgflush(int msgqid);
int gpsone_glue_msgunblock(int msgqid);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* GPSONE_GLUE_MSG_H */
