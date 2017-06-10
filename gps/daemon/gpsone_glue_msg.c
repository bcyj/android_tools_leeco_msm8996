/* Copyright (c) 2010, Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
#include <linux/stat.h>
#include <fcntl.h>

#include <linux/types.h>

#include "gpsone_glue_msg.h"
#include "gpsone_glue_pipe.h"
#include "gpsone_daemon_dbg.h"

/*===========================================================================
FUNCTION    gpsone_glue_msgget

DESCRIPTION
   This function get a message queue

   q_path - name path of the message queue
   mode -

DEPENDENCIES
   None

RETURN VALUE
   message queue id

SIDE EFFECTS
   N/A

===========================================================================*/
int gpsone_glue_msgget(const char * q_path, int mode)
{
    int msgqid;
    msgqid = gpsone_glue_pipeget(q_path, mode);
    return msgqid;
}

/*===========================================================================
FUNCTION    gpsone_glue_msgremove

DESCRIPTION
   remove a message queue

   q_path - name path of the message queue
   msgqid - message queue id

DEPENDENCIES
   None

RETURN VALUE
   0: success or negative value for failure

SIDE EFFECTS
   N/A

===========================================================================*/
int gpsone_glue_msgremove(const char * q_path, int msgqid)
{
    int result;
    result = gpsone_glue_piperemove(q_path, msgqid);
    return result;
}

/*===========================================================================
FUNCTION    gpsone_glue_msgsnd

DESCRIPTION
   Send a message

   msgqid - message queue id
   msgp - pointer to the message to be sent
   msgsz - size of the message

DEPENDENCIES
   None

RETURN VALUE
   number of bytes sent out or negative value for failure

SIDE EFFECTS
   N/A

===========================================================================*/
int gpsone_glue_msgsnd(int msgqid, const void * msgp, size_t msgsz)
{
    int result;
    struct msgbuf *pmsg = (struct msgbuf *) msgp;
    pmsg->msgsz = msgsz;

    result = gpsone_glue_pipewrite(msgqid, msgp, msgsz);
    if (result != (int) msgsz) {
        GPSONE_DMN_PR_ERR("%s:%d] pipe broken %d, msgsz = %d\n", __func__, __LINE__, result, (int) msgsz);
        return -1;
    }

    return result;
}

/*===========================================================================
FUNCTION    gpsone_glue_msgrcv

DESCRIPTION
   receive a message

   msgqid - message queue id
   msgp - pointer to the buffer to hold the message
   msgsz - size of the buffer

DEPENDENCIES
   None

RETURN VALUE
   number of bytes received or negative value for failure

SIDE EFFECTS
   N/A

===========================================================================*/
int gpsone_glue_msgrcv(int msgqid, void *msgp, size_t msgbufsz)
{
    int result;
    struct msgbuf *pmsg = (struct msgbuf *) msgp;

    result = gpsone_glue_piperead(msgqid, &(pmsg->msgsz), sizeof(pmsg->msgsz));
    if (result != sizeof(pmsg->msgsz)) {
        GPSONE_DMN_PR_ERR("%s:%d] pipe broken %d\n", __func__, __LINE__, result);
        return -1;
    }

    if (msgbufsz < pmsg->msgsz) {
        GPSONE_DMN_PR_ERR("%s:%d] msgbuf is too small %d < %d\n", __func__, __LINE__, (int) msgbufsz, (int) pmsg->msgsz);
        return -1;
    }

    result = gpsone_glue_piperead(msgqid, (char *) msgp + sizeof(pmsg->msgsz), pmsg->msgsz - sizeof(pmsg->msgsz));
    if (result != (int) (pmsg->msgsz - sizeof(pmsg->msgsz))) {
        GPSONE_DMN_PR_ERR("%s:%d] pipe broken %d, msgsz = %d\n", __func__, __LINE__, result, (int) pmsg->msgsz);
        return -1;
    }

    return pmsg->msgsz;
}

/*===========================================================================
FUNCTION    gpsone_glue_msgunblock

DESCRIPTION
   unblock a message queue

   msgqid - message queue id

DEPENDENCIES
   None

RETURN VALUE
   0: success

SIDE EFFECTS
   N/A

===========================================================================*/
int gpsone_glue_msgunblock(int msgqid)
{
    return gpsone_glue_pipeunblock(msgqid);
}

/*===========================================================================
FUNCTION    gpsone_glue_msgflush

DESCRIPTION
   flush out the message in a queue

   msgqid - message queue id

DEPENDENCIES
   None

RETURN VALUE
   number of bytes that are flushed out.

SIDE EFFECTS
   N/A

===========================================================================*/
int gpsone_glue_msgflush(int msgqid)
{
    int length;
    char buf[128];

    do {
        length = gpsone_glue_piperead(msgqid, buf, 128);
        GPSONE_DMN_DBG("%s:%d] %s\n", __func__, __LINE__, buf);
    } while(length);
    return length;
}

