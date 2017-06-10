/*========================================================================


*//** @file jpeg_q5_helper.c

@par EXTERNALIZED FUNCTIONS
  (none)

@par INITIALIZATION AND SEQUENCING REQUIREMENTS
  (none)

Copyright (C) 2008-09 Qualcomm Technologies, Inc.
All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
*//*====================================================================== */

/*========================================================================
                             Edit History

$Header:

when       who     what, where, why
--------   ---     -------------------------------------------------------
07/16/09   zhiminl Aborted event listener by ADSP_IOCTL_ABORT_EVENT_READ.
06/23/09   zhiminl Fixed pmem_fd < 0 when invalid, not <= 0.
04/16/09   vma     PMEM unregistration added.
01/22/09   vma     PMEM registration added.
09/19/08   vma     QDSP queue update
07/29/08   vma     Created file.

========================================================================== */

/* =======================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */

#include "jpeg_q5_helper.h"
#include "jpegerr.h"
#include "jpeglog.h"

#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/msm_adsp.h>

/* -----------------------------------------------------------------------
** Type Declarations
** ----------------------------------------------------------------------- */
// Structure of PMEM info for registering and unregistering to driver,
// which uses struct adsp_pmem_info
typedef struct
{
    int    fd;    // File descriptor of the PMEM buffer
    void * vaddr; // Virtual address of the buffer

} pmem_info_t;

/* -----------------------------------------------------------------------
** Global Object Definitions
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Local Object Definitions
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Forward Declarations
** ----------------------------------------------------------------------- */
static void* jpeg_q5_event_listener(void *arg);

/* =======================================================================
**                          Macro Definitions
** ======================================================================= */

/* Command Queue Indexes */
#if 1

#define QDSP_JPEG_ACTION_CMD_Q         22
#define QDSP_JPEG_CFG_CMD_Q            23

#else

#define QDSP_JPEG_ACTION_CMD_Q         20
#define QDSP_JPEG_CFG_CMD_Q            21

#endif

/* =======================================================================
**                          Function Definitions
** ======================================================================= */
int jpeg_q5_helper_init(jpeg_q5_helper_t        *p_helper,
                        void                    *p_client,
                        jpeg_q5_event_handler_t  p_handler)
{
    if (!p_helper || !p_handler)
    {
        return JPEGERR_ENULLPTR;
    }

    // Initialize individual fields
    STD_MEMSET(p_helper, 0, sizeof(jpeg_q5_helper_t));

    // Open for descriptor
    p_helper->fd = open("/dev/adsp/JPEGTASK", O_RDWR);
    if (p_helper->fd < 0)
    {
        p_helper->fd = open("/dev/JPEGTASK", O_RDWR);
        if (p_helper->fd < 0)
        {
            JPEG_DBG_LOW("jpeg_q5_helper_init: adsp driver open failed\n");
            return JPEGERR_EFAILED;
        }
    }

    // Enable adsp
    if (ioctl(p_helper->fd, ADSP_IOCTL_ENABLE) < 0)
    {
        close(p_helper->fd);
        return JPEGERR_EFAILED;
    }

    p_helper->p_client  = p_client;
    p_helper->p_handler = p_handler;
    os_mutex_init(&(p_helper->mutex));
    os_cond_init(&(p_helper->cond));

    // Create thread to listen for kernel event
    os_mutex_lock(&(p_helper->mutex));
    os_thread_create(&(p_helper->thread),
                   jpeg_q5_event_listener, (void *)p_helper);
    os_cond_wait(&(p_helper->cond), &(p_helper->mutex));
    os_mutex_unlock(&(p_helper->mutex));

    // Set state to ready
    p_helper->state = JPEG_Q5_HELPER_READY;
    return JPEGERR_SUCCESS;
}

int jpeg_q5_helper_register_pmem(jpeg_q5_helper_t *p_helper,
                                 jpeg_buf_t       *p_buf)
{
    int rc = JPEGERR_EFAILED;

    if (p_helper && p_buf && p_buf->pmem_fd && p_buf->ptr)
    {
        pmem_info_t info;
        info.fd    = p_buf->pmem_fd;
        info.vaddr = p_buf->ptr;
        rc = ioctl(p_helper->fd, ADSP_IOCTL_REGISTER_PMEM, &info);
    }
    return rc;
}

int jpeg_q5_helper_unregister_pmem(jpeg_q5_helper_t *p_helper)
{
    int rc = JPEGERR_EFAILED;

    if (p_helper)
    {
        rc = ioctl(p_helper->fd, ADSP_IOCTL_UNREGISTER_PMEM, NULL);
    }
    return rc;
}

int jpeg_q5_helper_send_cfg_command(jpeg_q5_helper_t *p_helper,
                                    uint8_t          *cmd_buf,
                                    uint32_t          cmd_len)
{
    struct adsp_command_t cmd;
    cmd.queue = QDSP_JPEG_CFG_CMD_Q;
    cmd.data  = cmd_buf;
    cmd.len   = cmd_len;
    if (ioctl(p_helper->fd, ADSP_IOCTL_WRITE_COMMAND, &cmd) < 0)
    {
        JPEG_DBG_HIGH("jpeg_q5_helper_send_cfg_command: failed calling ioctl (%d - %s)\n",
                      errno, strerror(errno));
        return JPEGERR_EFAILED;
    }
    return JPEGERR_SUCCESS;
}

int jpeg_q5_helper_send_action_command(jpeg_q5_helper_t *p_helper,
                                       uint8_t          *cmd_buf,
                                       uint32_t          cmd_len)
{
    struct adsp_command_t cmd;
    cmd.queue = QDSP_JPEG_ACTION_CMD_Q;
    cmd.data  = cmd_buf;
    cmd.len   = cmd_len;
    if (ioctl(p_helper->fd, ADSP_IOCTL_WRITE_COMMAND, &cmd) < 0)
    {
        JPEG_DBG_HIGH("jpeg_q5_helper_send_action_command: failed calling ioctl (%d - %s)\n",
                      errno, strerror(errno));
        return JPEGERR_EFAILED;
    }
    return JPEGERR_SUCCESS;
}

void jpeg_q5_helper_destroy(jpeg_q5_helper_t *p_helper)
{
    JPEG_DBG_LOW("jpeg_q5_helper_destroy\n");
    if (p_helper->state != JPEG_Q5_HELPER_UNINITIALIZED)
    {
        os_mutex_lock(&(p_helper->mutex));
        if (p_helper->state == JPEG_Q5_HELPER_READY)
        {
            p_helper->state = JPEG_Q5_HELPER_QUITING;
            os_mutex_unlock(&(p_helper->mutex));

            // Abort event listener
            (void)ioctl(p_helper->fd, ADSP_IOCTL_ABORT_EVENT_READ, NULL);
            os_thread_join(&p_helper->thread, NULL);
        }
        else
        {
            os_mutex_unlock(&(p_helper->mutex));
        }
        os_mutex_destroy(&(p_helper->mutex));
        os_cond_destroy(&(p_helper->cond));
        if (p_helper->fd >= 0)
            (void)close(p_helper->fd);
    }
}

static void* jpeg_q5_event_listener(void *arg)
{
    int       rc;
    uint8_t   buf[200];

    struct adsp_event_t  event;

    jpeg_q5_helper_t *p_helper = (jpeg_q5_helper_t *)arg;

    // Unblocks the init function
    os_mutex_lock(&(p_helper->mutex));
    os_cond_signal(&(p_helper->cond));
    os_mutex_unlock(&(p_helper->mutex));

    // Infinite loop to listen for dsp events
    for (;;)
    {
        event.data       = buf;
        event.timeout_ms = 500;
        event.len        = sizeof(buf);

        rc = ioctl(p_helper->fd, ADSP_IOCTL_GET_EVENT, &event);
        if (p_helper->state == JPEG_Q5_HELPER_QUITING)
        {
            break;
        }
        if (rc < 0)
        {
            if (errno != ETIMEDOUT)
            {
                JPEG_DBG_ERROR("jpeg_q5_event_listener: dsp ioctl failed (%d - %s)\n",
                               errno, strerror(errno));
                // Throw error event
                p_helper->p_handler(p_helper->p_client, JPEG_Q5_IOCTL_ERROR, NULL, 0);

                os_mutex_lock(&(p_helper->mutex));
                p_helper->state = JPEG_Q5_HELPER_QUITING;
                os_mutex_unlock(&(p_helper->mutex));
                break;
            }
            else
            {
                JPEG_DBG_LOW("jpeg_q5_event_listener: timeout\n");
            }
        }
        else if (event.type == 0)
        {
            // dsp message
            uint32_t len = event.len / 2;

            if (event.flags)
                len /= 2;

            p_helper->p_handler(p_helper->p_client,
                                event.msg_id,
                                (void *)event.data, len);
        }
        else if (event.type == 1)
        {
        }
    }

    return NULL;
}
