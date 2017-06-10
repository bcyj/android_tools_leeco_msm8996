/* Copyright (c) 2010 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 * Copyright (c) 2012 Qualcomm Atheros, Inc.
 * All Rights Reserved.
 * Qualcomm Atheros Confidential and Proprietary
 */
#include <string.h>
#include <unistd.h>
#include <errno.h>

// #include <linux/stat.h>
#include <fcntl.h>
// #include <linux/types.h>
#include <sys/types.h>
#include <sys/stat.h>


#include "gpsone_glue_pipe.h"
#include "gpsone_daemon_dbg.h"

/*===========================================================================
FUNCTION    gpsone_glue_pipeget

DESCRIPTION
   create a named pipe.

   pipe_name - pipe name path
   mode - mode

DEPENDENCIES
   None

RETURN VALUE
   0: success or negative value for failure

SIDE EFFECTS
   N/A

===========================================================================*/
int gpsone_glue_pipeget(const char * pipe_name, int mode)
{
    int fd;
    int result;

    GPSONE_DMN_DBG("%s:%d] %s, mode = %d\n", __func__, __LINE__, pipe_name, mode);
    result = mkfifo(pipe_name, 0660);

    if ((result == -1) && (errno != EEXIST)) {
        GPSONE_DMN_PR_ERR("%s:%d] pipe_name: %s failed: %d\n", __func__, __LINE__, pipe_name, errno);
        return result;
    }
    //To ensure the right permissions are set for this path
    result = chmod (pipe_name, 0660);
    if (result != 0)
    {
        GPSONE_DMN_PR_ERR("%s:%d] failed to change mode for %s, error = %s\n", __func__, __LINE__, pipe_name, strerror(errno));
    }

    fd = open(pipe_name, mode);
    if (fd <= 0)
    {
        GPSONE_DMN_PR_ERR("%s:%d] pipe_name: %s failed: %d\n", __func__, __LINE__, pipe_name, errno);
    }
    GPSONE_DMN_DBG("%s:%d] fd = %d, %s\n", __func__, __LINE__, fd, pipe_name);
    return fd;
}

/*===========================================================================
FUNCTION    gpsone_glue_piperemove

DESCRIPTION
   remove a pipe

    pipe_name - pipe name path
    fd - fd for the pipe

DEPENDENCIES
   None

RETURN VALUE
   0: success

SIDE EFFECTS
   N/A

===========================================================================*/
int gpsone_glue_piperemove(const char * pipe_name, int fd)
{
    close(fd);
    if (pipe_name) unlink(pipe_name);
    GPSONE_DMN_DBG("%s:%d] fd = %d, %s\n", __func__, __LINE__, fd, pipe_name);
    return 0;
}

/*===========================================================================
FUNCTION    gpsone_glue_pipewrite

DESCRIPTION
   write to a pipe

   fd - fd of a pipe
   buf - buffer for the data to write
   sz - size of the data in buffer

DEPENDENCIES
   None

RETURN VALUE
   number of bytes written or negative value for failure

SIDE EFFECTS
   N/A

===========================================================================*/
int gpsone_glue_pipewrite(int fd, const void * buf, size_t sz)
{
    int result;

    GPSONE_DMN_DBG("%s:%d] fd = %d, buf = 0x%lx, size = %d\n", __func__, __LINE__, fd, (long) buf, (int) sz);

    result = write(fd, buf, sz);

    /* @todo check for non EINTR & EAGAIN, shall not do select again, select_tut Law 7) */

    GPSONE_DMN_DBG("%s:%d] fd = %d, result = %d\n", __func__, __LINE__, fd, result);
    return result;
}

/*===========================================================================
FUNCTION    gpsone_glue_piperead

DESCRIPTION
   read from a pipe

   fd - fd for the pipe
   buf - buffer to hold the data read from pipe
   sz - size of the buffer

DEPENDENCIES
   None

RETURN VALUE
   number of bytes read from pipe or negative value for failure

SIDE EFFECTS
   N/A

===========================================================================*/
int gpsone_glue_piperead(int fd, void * buf, size_t sz)
{
    int len;

    GPSONE_DMN_DBG("%s:%d] fd = %d, buf = 0x%lx, size = %d\n", __func__, __LINE__, fd, (long) buf, (int) sz);

    len = read(fd, buf, sz);

    /* @todo check for non EINTR & EAGAIN, shall not do select again, select_tut Law 7) */

    GPSONE_DMN_DBG("%s:%d] fd = %d, len = %d\n", __func__, __LINE__, fd, len);
    return len;
}

/*===========================================================================
FUNCTION    gpsone_glue_pipeunblock

DESCRIPTION
   unblock a pipe

   fd - fd for the pipe

DEPENDENCIES
   None

RETURN VALUE
   0 for success or negative value for failure

SIDE EFFECTS
   N/A

===========================================================================*/
int gpsone_glue_pipeunblock(int fd)
{
    int result;
    struct flock flock_v;
    GPSONE_DMN_DBG("%s:%d]\n", __func__, __LINE__);
//    result = fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NDELAY);
    flock_v.l_type = F_UNLCK;
    flock_v.l_len = 32;
    result = fcntl(fd, F_SETLK, &flock_v);
    if (result < 0) {
perror("why failed?");
        GPSONE_DMN_DBG("%s:%d] fcntl failure, %s\n", __func__, __LINE__, strerror(errno));
    }

    return result;
}
