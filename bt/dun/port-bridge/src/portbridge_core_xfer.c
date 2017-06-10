/******************************************************************************

  @file    portbridge_core_xfer.c
  @brief   Data and Control Transfer Module

  DESCRIPTION
  Implementation of data and control transfer between smd and ext host.

 ******************************************************************************/
/*===========================================================================

  Copyright (c) 2010,2012-2013 Qualcomm Technologies, Inc. All Rights Reserved

  Qualcomm Technologies Proprietary

  Export of this technology or software is regulated by the U.S. Government.
  Diversion contrary to U.S. law prohibited.

  All ideas, data and information contained in or disclosed by
  this document are confidential and proprietary information of
  Qualcomm Technologies, Inc. and all rights therein are expressly reserved.
  By accepting this material the recipient agrees that this material
  and the information contained therein are held in confidence and in
  trust and will not be used, copied, reproduced in whole or in part,
  nor its contents revealed in any manner to others without the express
  written permission of Qualcomm Technologies, Inc.

  ===========================================================================*/

/*Header File Declarations*/
#include "portbridge_common.h"
#include "portbridge_core.h"
#include "portbridge_core_xfer.h"
#include "portbridge_ext_host_mon.h"

#include <termios.h>
#include <fcntl.h>
#include <pthread.h>
#include <errno.h>

#define CTRL_XFER_SLEEP_INTERVAL 200*1000
#define TIME_TO_TRANSFER_CD 500*1000
#define ORIGINAL_EXT_BITS 0x01
#define THROUGHPUT_INTERVAL 5

#undef LOGD
#undef LOGE
#undef LOGI
#undef LOGV

#define LOGD ALOGD
#define LOGI ALOGI
#define LOGE ALOGE
#define LOGV ALOGV


static pthread_t thrgpt_thread;

static unsigned long int count_bytes_to_smd_new = 0;
static unsigned long int count_bytes_to_ext_new = 0;

/*===========================================================================

FUNCTION     : notify_modem_status

DESCRIPTION  : Notifies the SMD modem status to the DUN App layer

DEPENDENCIES : None

RETURN VALUE : None

============================================================================*/
void notify_modem_status(int a_socket, char modem_bits)
{
    dun_ipc_msg ipc_msg;
    int ret = 0;
    unsigned short int len = DUN_IPC_HEADER_SIZE + DUN_IPC_MDM_STATUS_MSG_SIZE;
    ipc_msg.msg_type = DUN_IPC_MSG_MDM_STATUS;
    ipc_msg.msg_len  = DUN_IPC_MDM_STATUS_MSG_SIZE;


    port_log_err("notify_modem_status: msg_len %d modem_bits %d", ipc_msg.msg_len,
                                                                    modem_bits);
    ipc_msg.modem_status  = modem_bits;

    ret = send (a_socket, &ipc_msg, len, 0);
    if (ret < 0) {
        post_ext_host_event_to_core(DUN_EVENT_ERROR);
        port_log_err("Not able to send mesg to remote dev: %s", strerror(errno));
    }
    port_log_high("<-%s returns %d\n", __func__, ret);
}


/*===========================================================================

FUNCTION     : close_mdm

DESCRIPTION  : closes the smd port by sending the right command to mdm to
de-attach the data connection

DEPENDENCIES : None

RETURN VALUE : None

============================================================================*/

static void close_mdm(int *mdm_fd)
{
    port_log_high("->%s", __func__);
    pthread_mutex_lock(&fd_close_mutex);
    if(*mdm_fd >= 0) {
        if(!close(*mdm_fd)) {
            *mdm_fd = INVALID_SOCKET;
        }
        else {
            port_log_err(" Socket close failed %s\n",__func__);
        }
    }
    pthread_mutex_unlock(&fd_close_mutex);
    port_log_high("<-%s", __func__);
}

/*===========================================================================

FUNCTION     : thrgpt_thread_exit_handler

DESCRIPTION  : This function does thrgpt thread exit handling

DEPENDENCIES : None

RETURN VALUE : None

============================================================================*/

static void thrgpt_thread_exit_handler(int sig)
{
    if(sig == SIGUSR1) {
        pthread_exit(0);
    }
    else {
        port_log_err("Error in thrgpt thread exit handler! Sig value not SIGUSR1");
    }
}


/*===========================================================================

FUNCTION     : pb_log_throughput

DESCRIPTION  : This function logs the throughput of the dun call

DEPENDENCIES : None

RETURN VALUE : None

============================================================================*/
static void* pb_log_throughput()
{

    struct sigaction actions;
    static unsigned long int count_bytes_to_smd_prev = 0;
    static unsigned long int count_bytes_to_ext_prev = 0;

    memset(&actions, 0, sizeof(actions));
    sigemptyset(&actions.sa_mask);
    actions.sa_flags = 0;
    actions.sa_handler = thrgpt_thread_exit_handler;

    if (sigaction(SIGUSR1,&actions,NULL) < 0) {
        port_log_err("Error in sigaction in %s:  %s\n", __func__, strerror(errno));
    }

    while(1) {
        sleep(THROUGHPUT_INTERVAL);

        if(pb_dun_state == DUN_STATE_DISCONNECTED)
            return (void *)0;

        port_log_high("The number of bytes written (uplink) in %d seconds is %lu",
                THROUGHPUT_INTERVAL,
                abs(count_bytes_to_smd_new - count_bytes_to_smd_prev));

        port_log_high("The number of bytes written (dwnlink) in %d seconds is %lu",
                THROUGHPUT_INTERVAL,
                abs(count_bytes_to_ext_new - count_bytes_to_ext_prev));

        count_bytes_to_smd_prev = count_bytes_to_smd_new;
        count_bytes_to_ext_prev = count_bytes_to_ext_new;
    }
    return (void *)0;
}

/*===========================================================================

FUNCTION     : pb_start_thrgpt_thread

DESCRIPTION  : This function starts the thrgpt calculation thread

DEPENDENCIES : None

RETURN VALUE : None

============================================================================*/

void pb_start_thrgpt_thread(void)
{

    port_log_high("Starting thrgpt display thread");

    if(pthread_create(&(thrgpt_thread), NULL,
                pb_log_throughput, NULL) < 0) {
        port_log_err("Unable to create thrgpt thread : %s\n",
                strerror(errno));
    }
}

/*===========================================================================

FUNCTION     : pb_stop_thrgpt_thread

DESCRIPTION  : This function stops the thrgpt calculation thread

DEPENDENCIES : None

RETURN VALUE : None

============================================================================*/

void pb_stop_thrgpt_thread(void)
{
    int status;

    /* kill thrgpt displaying thread*/
    port_log_high("Stopping thrgpt display thread");
    if((status = pthread_kill(thrgpt_thread,
                    SIGUSR1)) < 0) {
        port_log_err("Error cancelling thread %d, error = %d (%s)",
                (int)thrgpt_thread, status, strerror(status));
        post_ext_host_event_to_core(DUN_EVENT_ERROR);
    }


    if((status = pthread_join(thrgpt_thread, NULL)) < 0) {
        port_log_err("Error joining thread %d, error = %d (%s)",
                (int)thrgpt_thread, status, strerror(status));
        post_ext_host_event_to_core(DUN_EVENT_ERROR);
    }

}

/*===========================================================================

FUNCTION     : pb_parse_for_atcmd

DESCRIPTION  : This function does parsing for atcmd

DEPENDENCIES : None

RETURN VALUE : DUN_ATCMD_E

============================================================================*/
static DUN_ATCMD_E pb_parse_for_atcmd(const unsigned char *buf)
{
    if(
            //#ifdef WCDMA
            (strncasecmp((const char *)buf,"ATDT*98", strlen("ATDT*98")) == 0)
            || (strncasecmp((const char *)buf,"ATDT*99", strlen("ATDT*99")) == 0)
            || (strncasecmp((const char *)buf,"ATD*98", strlen("ATD*98")) == 0)
            || (strncasecmp((const char *)buf,"ATD*99", strlen("ATD*99")) == 0)
            //#endif
            //#ifdef CDMA
            || (strncasecmp((const char *)buf,"ATDT#777", strlen("ATDT#777")) == 0)
            || (strncasecmp((const char *)buf,"ATD#777", strlen("ATD#777")) == 0)
            //#endif
      ) {
        return DUN_ATCMD_START;
    }
    else
        return DUN_ATCMD_INVALID;
}

/*===========================================================================

FUNCTION     : pb_smd_ext_ctrl_xfer

DESCRIPTION  : This function does smd to external port control
Read the smd port status bits and echo to external port
Convert DSR to DTR if it is set on SMD port

DEPENDENCIES : None

RETURN VALUE : status bits of the smd port

============================================================================*/
static int pb_smd_ext_ctrl_xfer(int init,
        int smd_old_status,
        dun_portparams_s *pportparams)
{
    int smd_new_status = 0;
    char rmt_status = 0;
    int cmd_bits;

    /*Get the status of the modem bits*/
    if( ioctl (pportparams->smdport_fd, TIOCMGET, &smd_new_status) < 0 ) {
        port_log_err("TIOCMGET for smd port failed - %s \n", strerror(errno));
        return -1;
    }

    /* Allow only MSR bits without CTS */
    smd_new_status &= TIOCM_RI | TIOCM_CD | TIOCM_DSR;

    /* Now set corresponding bits in physical interface if needed */
    if (init || (smd_new_status != smd_old_status)) {
        if(smd_new_status & TIOCM_RI)
            rmt_status |= RMT_MODEM_SIGNAL_RI;
        if(smd_new_status & TIOCM_CD)
            rmt_status |= RMT_MODEM_SIGNAL_DCD;
        if(smd_new_status & TIOCM_DSR)
            rmt_status |= RMT_MODEM_SIGNAL_DTRDSR;
        /* notify modem status bits to DUN App layer */
        notify_modem_status(pportparams->conn_sk, rmt_status);
    }
    return smd_new_status;
}

/*===========================================================================

FUNCTION    : pb_ext_smd_ctrl_xfer

DESCRIPTION : This function does external to smd port control
First, read the ext port status bits and echo to smd port
Convert DSR to DTR if it is set on external port

DEPENDENCIES : None

RETURN VALUE : status bits of the ext port

============================================================================*/
static int pb_ext_smd_ctrl_xfer(int init,
        int prev_modem_bits,
        dun_portparams_s *pportparams)
{
    int modem_bits = 0;
    char rmt_modem_bits = pportparams->rmt_mdm_bits;
    int smd_bits = 0;

    /* Allow only DTR bit */
    rmt_modem_bits  &= RMT_MODEM_SIGNAL_DTRDSR;
    if(rmt_modem_bits & RMT_MODEM_SIGNAL_DTRDSR)
        modem_bits |= TIOCM_DTR;
    /*
     * Set corresponding bits in physical interface for the first time or if
     * modem bits are set
     */

    if (init || (modem_bits != prev_modem_bits)) {
        smd_bits = modem_bits & (modem_bits ^ prev_modem_bits);
        if(smd_bits) {
            if(ioctl(pportparams->smdport_fd, TIOCMBIS, &smd_bits) < 0) {
                port_log_err("Error while setting SMD port bits : %s\n",
                        strerror(errno));
                return -1;
            }
        }

        smd_bits = ~modem_bits & (modem_bits ^ prev_modem_bits);

        if(smd_bits) {
            if(ioctl(pportparams->smdport_fd, TIOCMBIC, &smd_bits) < 0) {
                port_log_err("Error while clearing SMD port bits : %s\n",
                        strerror(errno));
                return -1;
            }
        }

        if(!(init || rmt_modem_bits)) {
            post_ext_host_event_to_core(DUN_EVENT_STOP_CALL);
        }
    }
    return modem_bits;
}


/*===========================================================================

FUNCTION     : pb_xfer_threads_exit_handler

DESCRIPTION  : This function does port monitor thread exit handling

DEPENDENCIES : None

RETURN VALUE : None

============================================================================*/

static void pb_xfer_threads_exit_handler(int sig)
{
    if(sig == SIGUSR1) {
        pthread_exit(0);
    }
    else if(sig == SIGPIPE) {
        port_log_err("Broken pipe: Socket is already closed, exiting monitor thread");
        pthread_exit(0);
    }
    else {
        port_log_err("Error in USB xfer exit handler! Sig value: %d", sig);
    }
}

/*===========================================================================

FUNCTION     : dun_port_monitor

DESCRIPTION  : This function does port monitors smd port and external port

DEPENDENCIES : None

RETURN VALUE : None

============================================================================*/
static void* pb_ctrl_xfer(void *arg)
{

    int prev_ext_bits =0;
    int prev_smd_bits =0;
    int init;
    struct sigaction actions;

    dun_portparams_s *pportparams = (dun_portparams_s *)arg;

    memset(&actions, 0, sizeof(actions));
    sigemptyset(&actions.sa_mask);
    actions.sa_flags = 0;
    actions.sa_handler = pb_xfer_threads_exit_handler;

    if (sigaction(SIGUSR1,&actions,NULL) < 0) {
        port_log_err("Error in sigaction in %s:  %s\n", __func__, strerror(errno));
    }

    if (sigaction(SIGPIPE,&actions,NULL) < 0) {
        port_log_err("Error in sigaction(SIGPIPE)in %s: %s", __func__, strerror(errno));
    }

    /* first time flag set*/
    init=1;

    while(pportparams->ctrl_running) {
        usleep(CTRL_XFER_SLEEP_INTERVAL);

        if(pb_dun_state == DUN_STATE_DISCONNECTED)
            break;

        prev_ext_bits = pb_ext_smd_ctrl_xfer(init, prev_ext_bits, pportparams);
        prev_smd_bits = pb_smd_ext_ctrl_xfer(init, prev_smd_bits, pportparams);
        if (prev_ext_bits < 0 || prev_smd_bits < 0) {
            post_ext_host_event_to_core(DUN_EVENT_ERROR);
            port_log_err("Error in %s \n", __func__);
            break;
        }
        init=0;
    }
    pportparams->ctrl_running = NO;
    pthread_exit(NULL);
    return (void *)0;
}

dun_internal_error handle_ctrl_request(int a_remote_sk, dun_ipc_ctrl_msg_type ctrl_msg)
{
    dun_internal_error err = DUN_ERR_NONE;

    switch(ctrl_msg) {
        case DUN_CRTL_MSG_DISCONNECT_REQ:
            port_log_high("%s: DUN Control DISCONNECT_REQ\n", __func__);
            post_ext_host_event_to_core(DUN_EVENT_EXT_HOST_DISCON);
            //disconnect_dun();
            break;
        default:
            err = DUN_ERR_ILLEGAL_ARG;
            port_log_high(" Invalid control Request: Req:%d", ctrl_msg);
            break;
    }
    return err;
}

dun_internal_error handle_dun_request (int a_remote_sk, dun_portparams_s *pportparams)
{
    dun_internal_error err = DUN_ERR_NONE;
    DUN_ATCMD_E atcmd;
    int num_read;
    unsigned char xfer_buf[DUN_MAXBUFSIZE];

    /* Data transfer from DUN App Socket to SMD port */
    num_read = read(pportparams->conn_sk, (void *)xfer_buf, DUN_MAXBUFSIZE);

    if(num_read < 0 || num_read > DUN_MAXBUFSIZE) {
        port_log_err("Ext port read failed: errno %s", strerror(errno));
        post_ext_host_event_to_core(DUN_EVENT_ERROR);
        /* close the smd fd, DUN App socket */
        close_mdm(&pportparams->smdport_fd);
        close_socket(&pportparams->conn_sk);
        return DUN_ERR_UNDEFINED;
    }
    else if (num_read != 0) {
        if (pb_dun_state == DUN_STATE_IDLE) {
            atcmd = pb_parse_for_atcmd(xfer_buf);
            if(atcmd == DUN_ATCMD_START) {
                port_log_high("ATCMD start arrived");
                /* When the core SM gets the ready_to_connect
                 * signal from the platform SM, it will signal this condition
                 * variable because of which the AT CMD will be passed onto
                 * the modem
                 */
                pthread_mutex_lock(&pb_signal_ext_host_mutex);
                /*Lock and then post the event to ext_host to avoid any race condition*/
                post_ext_host_event_to_core(DUN_EVENT_START_CALL);
                pthread_cond_wait(&pb_signal_ext_host, &pb_signal_ext_host_mutex);
                pthread_mutex_unlock(&pb_signal_ext_host_mutex);
            }
        }
        if (write (pportparams->smdport_fd, (void *)xfer_buf,
                    num_read) < 0) {
            port_log_err("Write to smd port failed in %s: %s\n",
                    __func__, strerror(errno));
            post_ext_host_event_to_core(DUN_EVENT_ERROR);
            return DUN_ERR_UNDEFINED;
        }

        /*For the throughput */
        count_bytes_to_smd_new = count_bytes_to_smd_new + num_read;
    }
    return err;
}


int do_read(int s, dun_portparams_s *pportparams)
{
    int ret = 0, len;
    dun_internal_error retval;
    char ipc_hdr[DUN_IPC_HEADER_SIZE];
    char buf[DUN_IPC_MDM_STATUS_MSG_SIZE];
    dun_ipc_msg_type ipc_msg_id;
    unsigned char dun_ipc_ctrl_msg;
    unsigned short int ipc_msg_len;

    port_log_high("Reading on DUN IPC header \n");
    len = recv(s, &ipc_hdr, DUN_IPC_HEADER_SIZE, 0);

    if (len < 0) {
        post_ext_host_event_to_core(DUN_EVENT_ERROR);
        port_log_err("%s: Not able to read from the socket: %s", __func__, strerror(errno));
        return -1;
    }

    port_log_high("Handling DUN IPC request");
    ipc_msg_id = (dun_ipc_msg_type)ipc_hdr[0];
    ipc_msg_len = *((unsigned short int *)(&ipc_hdr[1]));

    if((ipc_msg_len <= 0) || (ipc_msg_len > DUN_MAX_IPC_MSG_LEN)) {
        return -1;
    }

    if(ipc_msg_id == DUN_IPC_MSG_CTRL_REQUEST) {
        len = recv(s, &buf, DUN_IPC_CTRL_MSG_SIZE, 0);
        if (len < 0) {
            post_ext_host_event_to_core(DUN_EVENT_ERROR);
            port_log_err("%s: Not able to read from the socket: %s", __func__, strerror(errno));
            return -1;
        }

        port_log_high("Handling DUN control request");
        dun_ipc_ctrl_msg = (dun_ipc_ctrl_msg_type)buf[0];
        retval = handle_ctrl_request(s, dun_ipc_ctrl_msg);
        if (retval == DUN_ERR_ILLEGAL_ARG || retval == DUN_ERR_UNDEFINED) {
            ret = -1;
        }
    }
    else if(ipc_msg_id == DUN_IPC_MSG_DUN_REQUEST) {
        port_log_high("->%s(%x)", __func__, s);

        port_log_high("Handling DUN profile request");
        retval = handle_dun_request(s, pportparams);

        if (retval == DUN_ERR_ILLEGAL_ARG || retval == DUN_ERR_UNDEFINED) {
            ret = -1;
        }

        port_log_high("<-%s returns %d", __func__, ret);
    }
    else if(ipc_msg_id == DUN_IPC_MSG_MDM_STATUS) {
        len = recv(s, &buf, DUN_IPC_MDM_STATUS_MSG_SIZE, 0);
        if (len < 0) {
            post_ext_host_event_to_core(DUN_EVENT_ERROR);
            port_log_err("%s: Not able to read from the socket: %s", __func__, strerror(errno));
            return -1;
        }

        port_log_high("Handling DUN Modem status message");
        pportparams->rmt_mdm_bits = *((int *)(&buf[0]));
    }

    return ret;
}

/*===========================================================================

FUNCTION    : pb_dataxfr_ulink

DESCRIPTION : This function is responsible from reading from external port
and writing to the smd port

DEPENDENCIES : None

RETURN VALUE : Operation status: failure (-1) / success (0)

============================================================================*/
static void* pb_dataxfr_ulink(void *arg)
{
    struct sigaction actions;

    dun_portparams_s *pportparams = (dun_portparams_s *)arg;
    memset(&actions, 0, sizeof(actions));
    sigemptyset(&actions.sa_mask);
    actions.sa_flags = 0;
    actions.sa_handler = pb_xfer_threads_exit_handler;

    if (sigaction(SIGUSR1,&actions,NULL) < 0) {
        port_log_err("Error in sigaction in %s: %s \n", __func__, strerror(errno));
        post_ext_host_event_to_core(DUN_EVENT_ERROR);
        return (void *)-1;
    }

    while (pportparams->ulink_running) {

        if(do_read(pportparams->conn_sk, pportparams) < 0) {
            port_log_err(" error in uplink\n");
            break;
        }
    }
    post_core_event_to_platform(PLATFORM_EVENT_DUN_TERMINATED);
    port_log_dflt(" uplink thread exited \n");
    pportparams->ulink_running = NO;
    pthread_exit(NULL);
    return NULL;
}


/*===========================================================================

FUNCTION    : pb_dataxfr_dlink

DESCRIPTION : This function is responsible from reading from smd port
and writing to the ext port

DEPENDENCIES : None

RETURN VALUE : Operation status: failure (-1) / success (0)

============================================================================*/
static void* pb_dataxfr_dlink (void *arg)
{

    DUN_ATCMD_E atcmd;
    int num_read;
    dun_ipc_msg ipc_msg;
    struct sigaction actions;

    dun_portparams_s *pportparams = (dun_portparams_s *)arg;
    memset(&actions, 0, sizeof(actions));
    sigemptyset(&actions.sa_mask);
    actions.sa_flags = 0;
    actions.sa_handler = pb_xfer_threads_exit_handler;

    if (sigaction(SIGUSR1,&actions,NULL) < 0) {
        port_log_err("Error in sigaction in %s: %s \n", __func__, strerror(errno));
        post_ext_host_event_to_core(DUN_EVENT_ERROR);
        return (void *)-1;
    }

    while (pportparams->dlink_running) {
        /* Data transfer from SMD to External port*/
        if ((pb_dun_state == DUN_STATE_CONNECTED)||(pb_dun_state == DUN_STATE_IDLE)) {
            port_log_err("waiting for read in downlink\n");
            num_read = read(pportparams->smdport_fd, (void *) ipc_msg.xfer_buf,
                    (size_t)DUN_MAXBUFSIZE_DL);
            if (num_read < 0 || num_read > DUN_MAXBUFSIZE_DL) {
                port_log_err("Read from SMD port failed: %s\n", strerror(errno));
                post_ext_host_event_to_core(DUN_EVENT_ERROR);
                /* close the smd fd, DUN App socket */
                close_mdm(&pportparams->smdport_fd);
                close_socket(&pportparams->conn_sk);
                break;
            }

            if (num_read > 0) {
                port_log_err("pb_dataxfr_dlink num_read : %d\n",num_read);
                /* fill all DUN IPC parameters */
                ipc_msg.msg_type = DUN_IPC_MSG_DUN_RESPONSE;
                ipc_msg.msg_len  = num_read;
                if (write(pportparams->conn_sk, (void *) &ipc_msg, num_read + DUN_IPC_HEADER_SIZE) < 0) {
                    port_log_err("Write to external port failed: %s\n", strerror(errno));
                    post_ext_host_event_to_core(DUN_EVENT_ERROR);
                    break;
                }
            }

            /*For the throughput */
            count_bytes_to_ext_new = count_bytes_to_ext_new + num_read;
        }
    }
    port_log_dflt("downlink exited \n");
    pportparams->dlink_running = NO;
    pthread_exit(NULL);
    return NULL;
}


/*===========================================================================

FUNCTION     : pb_reset_ports

DESCRIPTION  : This function reset ports

DEPENDENCIES : None

RETURN VALUE : Operation status: failure (-1) / success (0)

============================================================================*/
static int pb_reset_ports(dun_portparams_s *pportparams)
{
    char orig_ext_bits = ORIGINAL_EXT_BITS;
    /* notify modem status bits to DUN App layer */
    notify_modem_status(pportparams->conn_sk, orig_ext_bits);
    return 0;
}


/*===========================================================================

FUNCTION     : pb_init_ports

DESCRIPTION  : This function init ports to raw mode

DEPENDENCIES : None

RETURN VALUE : Operation status: failure (-1) / success (0)

============================================================================*/
int pb_init_ports(dun_portparams_s *pportparams)
{
    struct termios term_params;
    struct termios term_params_smd;
    struct termios orig_term_params;
    int i =0;


    /* open the SMD port */
    if ((pportparams->smdport_fd = open(pportparams->smdportfname, O_RDWR)) < 0) {
        port_log_err("Unable to open SMD port %s : %s\n",
                pportparams->smdportfname, strerror(errno));
        return -1;
    }

    port_log_high("Dumping the TERMIOS for MDM port");

    if (tcgetattr (pportparams->smdport_fd, &term_params_smd) < 0) {
        port_log_err ("tcgetattr() of MDM port fails : %s\n", strerror(errno));
        close_mdm(&pportparams->smdport_fd);
        return -1;
    }
    port_log_high ("c_iflag: %0x", term_params_smd.c_iflag);
    port_log_high ("c_oflag: %0x", term_params_smd.c_oflag);
    port_log_high ("c_cflag: %0x", term_params_smd.c_cflag);
    port_log_high ("c_lflag: %0x", term_params_smd.c_lflag);

    for (i=0; i< NCCS; i++)
        port_log_high ("c_cc[%d]: %0x", i, term_params_smd.c_cc[i]);

    port_log_high ("Setting MDM port termios");
    term_params_smd.c_iflag = 0;
    term_params_smd.c_oflag = 0; /*Makes it RAW device*/
    term_params_smd.c_lflag = 0;
    term_params_smd.c_cflag = B38400 | CS8 | CREAD;

    if (tcsetattr (pportparams->smdport_fd,TCSAFLUSH, &term_params_smd) < 0) {
        port_log_err ("tcsetattr() of MDM port fails : %s\n", strerror(errno));
        close_mdm(&pportparams->smdport_fd);
        return -1;
    }

    if (tcgetattr (pportparams->smdport_fd, &term_params_smd) < 0) {
        port_log_err ("After set:tcgetattr of MDM port fails : %s\n",
                strerror(errno));
        close_mdm(&pportparams->smdport_fd);
        return -1;
    }
    port_log_high ("After Set:c_iflag: %0x", term_params_smd.c_iflag);
    port_log_high ("c_oflag: %0x", term_params_smd.c_oflag);
    port_log_high ("c_cflag: %0x", term_params_smd.c_cflag);
    port_log_high ("c_lflag: %0x", term_params_smd.c_lflag);

    for (i=0; i< NCCS; i++)
        port_log_high ("c_cc[%d]: %0x", i, term_params_smd.c_cc[i]);

    return 0;
}

/*===========================================================================

FUNCTION     : dun_start_port_threads

DESCRIPTION  : This function starts the control and data xfer threads

DEPENDENCIES : None

RETURN VALUE : None

============================================================================*/
int pb_start_xfer_threads(dun_portparams_s *pportparams)
{
    int status;

    if ( pportparams == NULL) {
        port_log_err("Null Port Parameters!");
        post_ext_host_event_to_core(DUN_EVENT_ERROR);
        return -1;
    }

    if (pb_reset_ports(pportparams) < 0) {
        port_log_err("Error while reseting ports!");
        post_ext_host_event_to_core(DUN_EVENT_ERROR);
        goto cleanup;
    }

    pportparams->ctrl_running = YES;
    /*create threads monitors external port and data transfer */
    if(pthread_create(&(pportparams->portctrlxfer_thread), NULL,
                pb_ctrl_xfer, (void *)pportparams) != 0) {
        port_log_err("Unable to create extportmonitor : %s\n",
                strerror(errno));
        post_ext_host_event_to_core(DUN_EVENT_ERROR);
        goto cleanup;
    }

    pportparams->ulink_running = YES;
    /* create uplink thread for port bridge*/
    if( pthread_create(&(pportparams->portdataxfr_thread_ulink), NULL,
                pb_dataxfr_ulink,  (void *)pportparams) != 0) {
        port_log_err("Unable to create extportread_thread : %s\n",
                strerror(errno));
        post_ext_host_event_to_core(DUN_EVENT_ERROR);
        close_mdm(&pportparams->smdport_fd);
        close_socket(&pportparams->conn_sk);
        /*Kill the ctrl xfer thread*/
        goto kill_ctrl_thread;
    }

    pportparams->dlink_running = YES;
    /* create downlink thread for port bridge*/
    if( pthread_create(&(pportparams->portdataxfr_thread_dlink), NULL,
                pb_dataxfr_dlink,  (void *)pportparams) != 0) {
        port_log_err("Unable to create extportread_thread : %s\n",
                strerror(errno));
        post_ext_host_event_to_core(DUN_EVENT_ERROR);
        close_mdm(&pportparams->smdport_fd);
        close_socket(&pportparams->conn_sk);
        goto kill_ctrl_data_threads;
    }
    return 0;

kill_ctrl_data_threads:
    if(pportparams->ulink_running == YES) {
        if((status = pthread_kill(pportparams->portdataxfr_thread_ulink,
                        SIGUSR1)) != 0){
            port_log_err("Error cancelling thread %d, error = %d (%s)",
                    (int)pportparams->portdataxfr_thread_ulink, status,
                    strerror(status));
        }
        if((status = pthread_join(pportparams->portdataxfr_thread_ulink, NULL)) != 0) {
            port_log_err("Error joining thread %d, error = %d (%s)",
                    (int)pportparams->portdataxfr_thread_ulink,
                    status, strerror(status));
        }
        else
            pportparams->ulink_running = NO;
    }

kill_ctrl_thread:
    if(pportparams->ctrl_running == YES) {
        if((status = pthread_kill(pportparams->portctrlxfer_thread,
                        SIGUSR1)) != 0) {
            port_log_err("Error cancelling thread %d, error = %d (%s)",
                    (int)pportparams->portctrlxfer_thread, status, strerror(status));
        }


        if((status = pthread_join(pportparams->portctrlxfer_thread, NULL)) != 0) {
            port_log_err("Error joining thread %d, error = %d (%s)",
                    (int)pportparams->portctrlxfer_thread, status, strerror(status));
        }
        else
            pportparams->ctrl_running = NO;
    }
    port_log_high("%s: killed all threads\n", __FUNCTION__);
    return -1;
cleanup:
    close_mdm(&pportparams->smdport_fd);
    close_socket(&pportparams->conn_sk);
    return -1;
}


/*===========================================================================

FUNCTION     : pb_stop_xfer_threads

DESCRIPTION  : This function stop threads

DEPENDENCIES : None

RETURN VALUE : None

============================================================================*/
void pb_stop_xfer_threads(dun_portparams_s *pportparams)
{
    int status;

    pb_close_fd_to_ports(&pb_dun_portparams);

    /* Allowing portsmonitor thread to transfer CD bit from SMD to ext */
    usleep(TIME_TO_TRANSFER_CD);


    if(pportparams->ctrl_running == YES) {
        /* kill thread which monitors external port bits */
        if((status = pthread_kill(pportparams->portctrlxfer_thread,
                        SIGUSR1)) != 0) {
            port_log_err("Error cancelling thread %d, error = %d (%s)",
                    (int)pportparams->portctrlxfer_thread, status, strerror(status));
        }
        if((status = pthread_join(pportparams->portctrlxfer_thread, NULL)) != 0) {
            port_log_err("Error joining thread %d, error = %d (%s)",
                    (int)pportparams->portctrlxfer_thread, status, strerror(status));
        }
        else
            pportparams->ctrl_running = NO;
    }

    if(pportparams->ulink_running == YES) {
        if((status = pthread_kill(pportparams->portdataxfr_thread_ulink,
                        SIGUSR1)) != 0){
            port_log_err("Error cancelling thread %d, error = %d (%s)",
                    (int)pportparams->portdataxfr_thread_ulink, status,
                    strerror(status));
        }
        if((status = pthread_join(pportparams->portdataxfr_thread_ulink, NULL)) != 0) {
            port_log_err("Error joining thread %d, error = %d (%s)",
                    (int)pportparams->portdataxfr_thread_ulink,
                    status, strerror(status));
        }
        else
            pportparams->ulink_running = NO;
    }

    if(pportparams->dlink_running == YES) {

        if((status = pthread_kill(pportparams->portdataxfr_thread_dlink,
                        SIGUSR1)) != 0){
            port_log_err("Error cancelling thread %d, error = %d (%s)",
                    (int)pportparams->portdataxfr_thread_dlink, status,
                    strerror(status));
        }
        if((status = pthread_join(pportparams->portdataxfr_thread_dlink, NULL)) != 0) {
            port_log_err("Error joining thread %d, error = %d (%s)",
                    (int)pportparams->portdataxfr_thread_dlink,
                    status, strerror(status));
        }
        else
            pportparams->dlink_running = NO;
    }
	port_log_high("%s: killed all threads\n", __FUNCTION__);

}

/*===========================================================================

FUNCTION     : pb_close_fd_to_ports

DESCRIPTION  : This function closes the fds to ext and smd ports

DEPENDENCIES : None

RETURN VALUE : Operation status: failure (-1) / success (0)

============================================================================*/
void pb_close_fd_to_ports(dun_portparams_s *pportparams)
{
    port_log_high("Closing %s", pportparams->smdportfname);
    close_mdm(&pportparams->smdport_fd);
    close_socket(&pportparams->conn_sk);
    disconnect_dun();
}

