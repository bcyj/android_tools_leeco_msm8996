/* Copyright (c) 2012, Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#include <stdio.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <pthread.h>
#include <linux/types.h>
#include <media/msm_mercury.h>
#include "mercury_lib_hw.h"
#include "mercury_dbg.h"

typedef struct {
    int mcrfd;
    int mercury_sequential_op;
    int (*mercury_lib_event_handler) (mcr_obj_t, struct mercury_evt *,
        int);
    int (*mercury_lib_input_handler) (mcr_obj_t, struct mercury_buf *);
    int (*mercury_lib_output_handler) (mcr_obj_t, struct mercury_buf *);

    pthread_t event_thread_id;
    unsigned char event_thread_exit;
    pthread_mutex_t event_thread_ready_mutex;
    pthread_cond_t event_thread_ready_cond;
    uint8_t event_thread_is_ready;

    pthread_t input_thread_id;
    unsigned char input_thread_exit;
    pthread_mutex_t input_thread_ready_mutex;
    pthread_cond_t input_thread_ready_cond;
    uint8_t input_thread_is_ready;

    pthread_t output_thread_id;
    unsigned char output_thread_exit;
    pthread_mutex_t output_thread_ready_mutex;
    pthread_cond_t output_thread_ready_cond;
    uint8_t output_thread_is_ready;

} __mcr_obj_t;


static int mercury_fd = 0;
static int mercury_initialized = 0;

void mercury_lib_set_mercury_fd(int mcrfd)
{
    mercury_fd = mcrfd;
    mercury_lib_hw_setfd(mercury_fd);
}

int mercury_lib_get_mercury_fd(void)
{
    return mercury_fd;
}

int mercury_lib_configure_baseline(mercury_cmd_quant_cfg_t dqt_cfg,
    mercury_cmd_sof_cfg_t sof_cfg,
    mercury_cmd_huff_cfg_t huff_cfg, mercury_cmd_sos_cfg_t sos_cfg)
{
    int rc;
    rc = mercury_lib_hw_jpegd_core_reset();
    if (rc) {
        MCR_PR_ERR("\n(%d)%s()  Error Configuring JPEGD HW\n", __LINE__,
            __func__);
        return JPEGERR_EFAILED;
    }

    rc = mercury_lib_hw_jpeg_dqt(dqt_cfg);
    if (rc) {
        MCR_PR_ERR("\n(%d)%s()  Error Configuring JPEGD HW\n", __LINE__,
            __func__);
        return JPEGERR_EFAILED;
    }

    rc = mercury_lib_hw_jpeg_sof(sof_cfg);
    if (rc) {
        MCR_PR_ERR("\n(%d)%s()  Error Configuring JPEGD HW\n", __LINE__,
            __func__);
        return JPEGERR_EFAILED;
    }

    rc = mercury_lib_hw_jpeg_dht(huff_cfg);
    if (rc) {
        MCR_PR_ERR("\n(%d)%s()  Error Configuring JPEGD HW\n", __LINE__,
            __func__);
        return JPEGERR_EFAILED;
    }

    rc = mercury_lib_hw_jpeg_sos(sos_cfg);
    if (rc) {
        MCR_PR_ERR("\n(%d)%s()  Error Configuring JPEGD HW\n", __LINE__,
            __func__);
        return JPEGERR_EFAILED;
    }
    return rc;

}

int mercury_lib_decode_config(mercury_cmd_readbus_cfg_t bus_read_cfg ,
    mercury_cmd_writebus_cfg_t bus_write_cfg,
    mercury_cmd_control_cfg_t ctrl_cfg)
{
    if (mercury_lib_hw_bus_read_config(bus_read_cfg) ||
        mercury_lib_hw_bus_write_config(bus_write_cfg) ||
        mercury_lib_hw_bus_control_config(ctrl_cfg)) {
        MCR_PR_ERR ("\n(%d)%s()  Error Configuring JPEGD HW\n", __LINE__,
            __func__);
        return JPEGERR_EFAILED;
    }
    return 0;

}


int mercury_lib_output_buf_cfg(struct mercury_buf *buf)
{
    int rc;
    struct msm_mercury_buf msm_buf;

    msm_buf.type = buf->type;
    msm_buf.vaddr = buf->vaddr;
    msm_buf.fd = buf->fd;

    msm_buf.y_off = buf->y_off;
    msm_buf.y_len = buf->y_len;

    msm_buf.cbcr_off = buf->cbcr_off;
    msm_buf.cbcr_len = buf->cbcr_len;

    rc = ioctl(mercury_fd, MSM_MCR_IOCTL_OUTPUT_BUF_CFG, &msm_buf);
    if (rc  < 0) {
        MCR_PR_ERR ("(%d)%s  %s ioctl-%d error...\n",
            __LINE__, __FUNCTION__, MSM_MERCURY_NAME,
            _IOC_NR(MSM_MCR_IOCTL_OUTPUT_BUF_CFG));
        return 1;
    }

    return 0;
}

int mercury_lib_input_buf_cfg(struct mercury_buf *buf)
{
    int rc;
    struct msm_mercury_buf msm_buf;

    msm_buf.type = buf->type;
    msm_buf.vaddr = buf->vaddr;
    msm_buf.fd = buf->fd;

    msm_buf.y_off = buf->y_off;
    msm_buf.y_len = buf->y_len;

    rc = ioctl(mercury_fd, MSM_MCR_IOCTL_INPUT_BUF_CFG, &msm_buf);
    if (rc  < 0) {
        MCR_PR_ERR ("(%d)%s  %s ioctl-%d error...\n",
            __LINE__, __FUNCTION__, MSM_MERCURY_NAME,
            _IOC_NR(MSM_MCR_IOCTL_INPUT_BUF_CFG));
        return 1;
    }

    return 0;
}


int mercury_lib_output_buf_get(struct mercury_buf *buf)
{
    int rc;
    struct msm_mercury_buf msm_buf;
    rc = ioctl(mercury_fd, MSM_MCR_IOCTL_OUTPUT_GET, &msm_buf);
    if (rc  < 0) {
        MCR_PR_ERR ("(%d)%s  %s ioctl-%d error...\n",
            __LINE__, __FUNCTION__, MSM_MERCURY_NAME,
            _IOC_NR(MSM_MCR_IOCTL_OUTPUT_GET));
        return 1;
    }
    return 0;
}

int mercury_lib_input_buf_get(struct mercury_buf *buf)
{
    int rc;
    struct msm_mercury_buf msm_buf;
    rc = ioctl(mercury_fd, MSM_MCR_IOCTL_INPUT_GET, &msm_buf);
    if (rc  < 0) {
        MCR_PR_ERR ("(%d)%s  %s ioctl-%d error...\n",
            __LINE__, __FUNCTION__, MSM_MERCURY_NAME,
            _IOC_NR(MSM_MCR_IOCTL_INPUT_GET));
        return 1;
    }
    return 0;
}

int mercury_lib_event_get_unblock(void)
{
    struct msm_mercury_hw_cmd hw_cmd;
    int rc;

    MCR_PR_ERR("(%d)%s()\n", __LINE__, __func__);

    rc = ioctl(mercury_fd, MSM_MCR_IOCTL_EVT_GET_UNBLOCK, &hw_cmd);
    if (rc) {
        MCR_PR_ERR("(%d)%s() mercury MSM_MCR_IOCTL_EVT_GET_UNBLOCK "
            "fails (rc=%d)\n\n",
            __LINE__, __func__, rc);
        return -1;
    }
    return 0;
}

int mercury_lib_decode(void)
{
    uint32_t rc=0;
    rc = mercury_lib_hw_decode();
    return rc;
}

uint32_t mercury_lib_check_rd_ack_irq(void)
{
    uint32_t rc;
    rc = mercury_lib_hw_check_rd_ack_irq();
    return rc;
}

uint32_t mercury_lib_check_wr_ack_irq(void)
{
    uint32_t rc;
    rc = mercury_lib_hw_check_wr_ack_irq();
    return rc;
}

uint32_t mercury_lib_check_jpeg_status(void)
{
    uint32_t rc;
    rc = mercury_lib_hw_check_jpeg_status();
    return rc;
}

void mercury_lib_wait_thread_ready (__mcr_obj_t * mcr_obj_p,
    pthread_t* thread_id)
{
    if (*thread_id == mcr_obj_p->event_thread_id) {
        pthread_mutex_lock (&mcr_obj_p->event_thread_ready_mutex);
        MCR_DBG ("%s:%d], event thread ready %d\n", __func__, __LINE__,
            mcr_obj_p->event_thread_is_ready);
        if (!mcr_obj_p->event_thread_is_ready) {
            pthread_cond_wait (&mcr_obj_p->event_thread_ready_cond,
                &mcr_obj_p->event_thread_ready_mutex);
        }
        mcr_obj_p->event_thread_is_ready = 0;
        pthread_mutex_unlock (&mcr_obj_p->event_thread_ready_mutex);
    }
}

void mercury_lib_send_thread_ready (__mcr_obj_t * mcr_obj_p,
    pthread_t* thread_id)
{
    if (*thread_id == mcr_obj_p->event_thread_id) {
        pthread_mutex_lock (&mcr_obj_p->event_thread_ready_mutex);
        mcr_obj_p->event_thread_is_ready = 1;
        pthread_cond_signal (&mcr_obj_p->event_thread_ready_cond);
        pthread_mutex_unlock (&mcr_obj_p->event_thread_ready_mutex);
    }
}

int mercury_lib_wait_done (mcr_obj_t mcr_obj)
{
    __mcr_obj_t *mcr_obj_p = (__mcr_obj_t *) mcr_obj;

    if (mcr_obj_p->mercury_lib_event_handler) {
        MCR_DBG ("\n%s:%d] mercury_lib_wait_thread_ready; event_handler %d\n",
            __func__,
            __LINE__, (int)mcr_obj_p->event_thread_id);
        mercury_lib_wait_thread_ready (mcr_obj_p,
            &(mcr_obj_p->event_thread_id));
    }

    MCR_DBG ("\n%s:%d] mercury_lib_wait_done\n", __func__, __LINE__);
    return 0;
}

int mercury_lib_set_sequential_op (mcr_obj_t mcr_obj)
{
    __mcr_obj_t *mcr_obj_p = (__mcr_obj_t *) mcr_obj;

    MCR_DBG ("(%d)%s()\n", __LINE__, __func__);
    mcr_obj_p->mercury_sequential_op = 1;
    return 0;
}

int mercury_lib_clear_sequential_op (mcr_obj_t mcr_obj)
{
    __mcr_obj_t *mcr_obj_p = (__mcr_obj_t *) mcr_obj;

    MCR_DBG ("(%d)%s()\n", __LINE__, __func__);
    mcr_obj_p->mercury_sequential_op = 0;
    return 0;
}

void *mercury_lib_event_thread (void *context)
{
    __mcr_obj_t *mcr_obj_p = (__mcr_obj_t *) context;
    int mcrfd = mcr_obj_p->mcrfd;

    struct msm_mercury_ctrl_cmd mcrCtrlCmd;
    int rc = 0;

    MCR_DBG ("%s:%d] Enter threadid %ld\n", __func__, __LINE__,
        mcr_obj_p->event_thread_id);
    mercury_lib_send_thread_ready (mcr_obj_p,
        &(mcr_obj_p->event_thread_id));

    mcr_obj_p->event_thread_exit = 0;

    do {
        MCR_DBG ("\n  %s:%d] Sending MSM_MCR_IOCTL_EVT_GET \n",
            __func__, __LINE__);

        rc = ioctl (mcrfd, MSM_MCR_IOCTL_EVT_GET, &mcrCtrlCmd);

        if (mcrCtrlCmd.type == MSM_MERCURY_EVT_FRAMEDONE) {
            MCR_DBG ("(%d)%s() MSM_MERCURY_EVT_FRAMEDONE received (rc=%d)\n",
                __LINE__, __func__, rc);
            /* jpegd_engine_hw_event_handler()*/
            mcr_obj_p->mercury_lib_event_handler ((mcr_obj_t) mcr_obj_p,
                &mcrCtrlCmd, mcrCtrlCmd.type);
        } else if (mcrCtrlCmd.type == MSM_MERCURY_EVT_ERR) {
            MCR_DBG ("(%d)%s() MSM_MERCURY_EVT_ERR received (rc=%d)\n",
                __LINE__, __func__, rc);
            mcr_obj_p->mercury_lib_event_handler ((mcr_obj_t) mcr_obj_p,
                &mcrCtrlCmd, mcrCtrlCmd.type);
        } else if (mcrCtrlCmd.type == MSM_MERCURY_EVT_RESET) {
            MCR_DBG ("(%d)%s() MSM_MERCURY_EVT_RESET received (rc=%d)\n",
                __LINE__, __func__, rc);
        } else if (mcrCtrlCmd.type == MSM_MERCURY_EVT_UNBLOCK) {
            MCR_DBG ("(%d)%s() MSM_MERCURY_EVT_UNBLOCK received "
                "(event_thread_exit=%d)\n",
                __LINE__, __func__, mcr_obj_p->event_thread_exit);
        }

        mercury_lib_send_thread_ready (mcr_obj_p,
            &(mcr_obj_p->event_thread_id));
    } while (!mcr_obj_p->event_thread_exit);
    MCR_DBG ("%s:%d] Exit\n", __func__, __LINE__);
    return NULL;
}

int mercury_lib_init (mcr_obj_t * mcr_obj_p_p,
    int (*event_handler) (mcr_obj_t, struct mercury_evt *, int event) )
{
    __mcr_obj_t *mcr_obj_p;
    int mcrfd;
    int result;

    mcr_obj_p = malloc (sizeof (__mcr_obj_t));
    if (!mcr_obj_p) {
        MCR_PR_ERR("%s:%d] no mem\n", __func__, __LINE__);
        return -1;
    }
    memset (mcr_obj_p, 0, sizeof (__mcr_obj_t));

    mcrfd = open (MSM_MERCURY_NAME, O_RDWR);
    if (mcrfd < 0) {
        MCR_PR_ERR("Error opening device handle %s\n", MSM_MERCURY_NAME);
        goto mercury_init_err;
    }

    /* save file handle to a global variable*/
    mercury_lib_set_mercury_fd(mcrfd);

    mcr_obj_p->mercury_sequential_op = 0;
    mcr_obj_p->mcrfd = mcrfd;
    /* jpegd_engine_hw_event_handler()*/
    mcr_obj_p->mercury_lib_event_handler = event_handler;

    pthread_mutex_init(&mcr_obj_p->event_thread_ready_mutex, NULL);
    pthread_cond_init (&mcr_obj_p->event_thread_ready_cond, NULL);
    mcr_obj_p->event_thread_is_ready = 0;

    if (event_handler) {
        MCR_DBG("(%d)%s()  Create mercury_lib_event_thread\n",
            __LINE__, __func__);
        pthread_mutex_lock(&mcr_obj_p->event_thread_ready_mutex);
        result = pthread_create (&mcr_obj_p->event_thread_id, NULL,
            mercury_lib_event_thread, mcr_obj_p);
        if (result < 0) {
            MCR_PR_ERR ("%s event thread creation failed\n", __func__);
            pthread_mutex_unlock(&mcr_obj_p->event_thread_ready_mutex);
            goto mercury_init_err;
        }
        pthread_mutex_unlock(&mcr_obj_p->event_thread_ready_mutex);
    }

    MCR_DBG ("mercury create all threads success\n");
    mercury_lib_wait_done(mcr_obj_p);
    MCR_DBG ("mercury after starting all threads\n");
    *mcr_obj_p_p = mcr_obj_p;
    return mcrfd;

    mercury_init_err:
    if (mcr_obj_p) {
        free (mcr_obj_p);
    }
    return -1;

}

int mercury_lib_release(mcr_obj_t mcr_obj)
{
    __mcr_obj_t *mcr_obj_p = (__mcr_obj_t *) mcr_obj;
    int rc;

    if (mcr_obj_p->mercury_sequential_op) {
        MCR_DBG ("(%d)%s() Return prematurely...\n", __LINE__, __func__);
        return 0;
    }

    MCR_DBG ("(%d)%s() closing\n", __LINE__, __func__);

    mcr_obj_p->event_thread_exit = 1;

    if (mcr_obj_p->mercury_lib_event_handler) {

        rc = ioctl (mercury_fd, MSM_MCR_IOCTL_EVT_GET_UNBLOCK);
        MCR_DBG ("(rc=%d) %s:%d] mercury_lib_release\n",rc,__func__,
            __LINE__);

        if (pthread_join (mcr_obj_p->event_thread_id, NULL) != 0) {
            MCR_PR_ERR ("%s: failed %d\n", __func__, __LINE__);
        }
    }

    rc = close(mercury_fd);

    pthread_mutex_destroy (&mcr_obj_p->event_thread_ready_mutex);
    pthread_cond_destroy (&mcr_obj_p->event_thread_ready_cond);

    MCR_DBG ("(%d)%s() closing %s\n", __LINE__, __func__,
        MSM_MERCURY_NAME);

    return rc;
}
