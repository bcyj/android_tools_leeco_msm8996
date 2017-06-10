/*
 * Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 * Not a Contribution.
 * Apache license notifications and license are retained
 * for attribution purposes only.
 */

/******************************************************************************
* Copyright (c) 2013, The Linux Foundation. All rights reserved.
* Not a Contribution.
 ******************************************************************************/

/******************************************************************************
 *
 *  Copyright (C) 1999-2012 Broadcom Corporation
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at:
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 ******************************************************************************/
#include "OverrideLog.h"
#include <string.h>
#include "gki.h"
#include "nfc_hal_api.h"
#include "nfc_hal_int.h"
#include "userial.h"
#include "nfc_target.h"

#include <pthread.h>
#include <termios.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <gki_int.h>
#include <poll.h>
#include "hw.h"
#include "config.h"

#include <DT_Nfc_link.h>
#include <DT_Nfc_types.h>
#include <DT_Nfc_status.h>
#include <DT_Nfc_i2c.h>
#include <DT_Nfc_log.h>
#include <DT_Nfc.h>
#include <semaphore.h>
#include <sys/mman.h>

#ifdef DTA // <DTA>
#include "dta_flag.h"
#endif // </DTA>

#define HCISU_EVT                           EVENT_MASK(APPL_EVT_0)
#define MAX_ERROR                           10

#define NUM_RESET_ATTEMPTS                  5

#ifndef BTE_APPL_MAX_USERIAL_DEV_NAME
#define BTE_APPL_MAX_USERIAL_DEV_NAME       (256)
#endif
extern UINT8 appl_trace_level;
extern char current_mode;
extern UINT8 reset_status;
/* Mapping of USERIAL_PORT_x to linux */
extern UINT32 ScrProtocolTraceFlag;
static int current_nfc_wake_state = 0;

int uart_port  = 0;
int isLowSpeedTransport = 0;
int nfc_wake_delay = 0;
int nfc_write_delay = 0;
int gPowerOnDelay = 300;
static int gPrePowerOffDelay = 0;    // default value
static int gPostPowerOffDelay = 0;     // default value
static pthread_mutex_t close_thread_mutex = PTHREAD_MUTEX_INITIALIZER;
UINT8 nci_wake_done = 0;

char userial_dev[BTE_APPL_MAX_USERIAL_DEV_NAME+1];
char power_control_dev[BTE_APPL_MAX_USERIAL_DEV_NAME+1];
tSNOOZE_MODE_CONFIG gSnoozeModeCfg = {
    NFC_HAL_LP_SNOOZE_MODE_SPI_I2C,     /* Sleep Mode (0=Disabled 1=UART 8=SPI/I2C) */
    NFC_HAL_LP_IDLE_THRESHOLD_HOST,     /* Idle Threshold Host */
    NFC_HAL_LP_IDLE_THRESHOLD_HC,       /* Idle Threshold HC */
    NFC_HAL_LP_ACTIVE_LOW,              /* NFC Wake active mode (0=ActiveLow 1=ActiveHigh) */
    NFC_HAL_LP_ACTIVE_HIGH              /* Host Wake active mode (0=ActiveLow 1=ActiveHigh) */
};

#define USERIAL_Debug_verbose     ((ScrProtocolTraceFlag & 0x80000000) == 0x80000000)

#include <sys/socket.h>
#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "NfcDt"

static UINT8 spi_negotiation[10] = { 0xF0, /* CMD */
                                    0x00, /* SPI PARM Negotiation */
                                    0x01, /* SPI Version */
                                    0x00, /* SPI Mode:0, SPI_INT active low */
                                    0x00, /* 8Bit, MSB first, Little Endian byte order */
                                    0x00, /* Reserved */
                                    0xFF, /* Sleep timeout Lower Byte */
                                    0xFF, /* Sleep timeout Upper Byte */
                                    0x00, /* Reserved */
                                    0x00 /* Reserved */
};
static UINT8 spi_nego_res[20];

#include <ctype.h>

/* use tc interface to change baudrate instead of close/open sequence which can fail on some platforms
 * due to tx line movement when opeing/closing the UART. the 43xx do not like this. */
#ifndef USERIAL_USE_TCIO_BAUD_CHANGE
#define USERIAL_USE_TCIO_BAUD_CHANGE FALSE
#endif

#ifndef USERIAL_USE_IO_BT_WAKE
#define USERIAL_USE_IO_BT_WAKE FALSE
#endif

/* this are the ioctl values used for bt_wake ioctl via UART driver. you may need to redefine at for
 * you platform! Logically they need to be unique and not colide with existing uart ioctl's.
 */
#ifndef USERIAL_IO_BT_WAKE_ASSERT
#define USERIAL_IO_BT_WAKE_ASSERT   0x8003
#endif
#ifndef USERIAL_IO_BT_WAKE_DEASSERT
#define USERIAL_IO_BT_WAKE_DEASSERT 0x8004
#endif
#ifndef USERIAL_IO_BT_WAKE_GET_ST
#define USERIAL_IO_BT_WAKE_GET_ST   0x8005
#endif

/* the read limit in this current implementation depends on the GKI_BUF3_SIZE
 * It would be better to use some ring buffer from the USERIAL_Read() is reading
 * instead of putting it into GKI buffers.
 */
#define READ_LIMIT                  (USERIAL_POOL_BUF_SIZE-BT_HDR_SIZE)
/*
 * minimum buffer size requirement to read a full sized packet from NFCC = 255 + 4 byte header
 */
#define MIN_BUFSIZE                 259
#define POLL_TIMEOUT                1000
/* priority of the reader thread */
#define USERIAL_READ_TRHEAD_PRIO    90
/* time (ms) to wait before trying to allocate again a GKI buffer */
#define NO_GKI_BUFFER_RECOVER_TIME  100
#define MAX_SERIAL_PORT             (USERIAL_PORT_15 + 1)
#define DT_POOL_ID                 GKI_POOL_ID_0

#define NFC_PAGE_SIZE           (0x1000)    //4K Pages
#define PAGES                   (0x8)       //8 Pages for now, giving 32 KBytes
#define BLOCK_START             (0x0)
#define BYTE_SHIFT              (8)
#define NFC_HEADER              (3)

#define DTA_POLL_WAKE_DELAY     5
#define NCI_WAKE_DELAY          10
#define P2P_TARGET_WAKE_DELAY   1
#define WRITE_DELAY             1
#define NFCC_RESET_DELAY        20
#define NFCC_INIT_DELAY         20

#define SET_NFCC_INIT           2
#define SET_NFCC_OFF            1
#define SET_NFCC_ON             0

extern void dumpbin(const char* data, int size);
extern UINT8 *scru_dump_hex (UINT8 *p, char *p_title, UINT32 len, UINT32 trace_layer, UINT32 trace_type);

static pthread_t      worker_thread1 = 0;

typedef struct  {
    volatile unsigned long bt_wake_state;
    int             sock;
    tUSERIAL_CBACK  *ser_cb;
    UINT16      baud;
    UINT8       data_bits;
    UINT16      parity;
    UINT8       stop_bits;
    UINT8       port;
    tUSERIAL_OPEN_CFG open_cfg;
    int         sock_power_control;
    int         client_device_address;
    struct timespec write_time;
} tLINUX_CB;

static tLINUX_CB linux_cb;  /* case of multipel port support use array : [MAX_SERIAL_PORT] */



void DT_Nfc_close_thread(UINT32 params);

static UINT8 device_name[BTE_APPL_MAX_USERIAL_DEV_NAME+1];
static int  bSerialPortDevice = FALSE;
static int _timeout = POLL_TIMEOUT;
static BOOLEAN is_close_thread_is_waiting = FALSE;


int   perf_log_every_count = 0;
typedef struct {
    const char* label;
    long    lapse;
    long    bytes;
    long    count;
    long    overhead;
} tPERF_DATA;
#define USING_POLL_WAIT    /* If we use polled wait rather than SIGIO approach */

/*structure holds members related for both read and write operations*/
typedef struct DT_RdWr_st
{
    char                ReaderThreadAlive;

    long                nReadMode;              /* Is this a solicited/unsolicited READ */
    char                *block_zero_base;       /* This is mapped to page aligned buffer written
                                                    to by kernel driver. Base Address */
    char                *block_absolute;        /* Base Address(above) + Current Block No. + Offset */
    signed short        block_no_handler;       /* The Current Block No. we're reading from in
                                                      shared mem - in handler context */
    signed short        block_no_deferred;      /* Block no. being read from client context - it may lag
                                                       but unlikely since data rate veerry slow */
    signed short        block_lag;              /* Latency in client servicing */
    unsigned short      block_wrap_handler_cnt; /* The block buffer is cyclic so need to know when the
                                                       the block_no_handler wraps back to zero. This should
                                                       NEVER be allowed to  == 2, otherwise we have a lag
                                                       at least equal to the total number of block so we
                                                       are over writing data. */
    BOOLEAN             blocks_available;       /* If we have new block(s) to service. */
    UINT8               uiPoolID;               /* Memory pool ID generated when pool created */
    char                WaitOnWrite;
    char                WriteBusy;
} DT_Nfc_RdWr_t;



static DT_Nfc_RdWr_t            RdWrContext;
static DT_Nfc_Phy_select_t      dTransport;
static DT_Nfc_sConfig_t         DriverConfig;
int pdTransportHandle;

static sem_t                    data_available;
extern char shut_down_reason;

uint8_t DT_Nfc_RamdumpPerformed = FALSE;


/*******************************************************************************
**
** Function         perf_reset
**
** Description      reset performance measurement data
**
** Returns          none
**
*******************************************************************************/
void perf_reset(tPERF_DATA* t)
{
    t->count =
    t->bytes =
    t->lapse = 0;
}

/*******************************************************************************
**
** Function         perf_log
**
** Description      produce a log entry of cvurrent performance data
**
** Returns          none
**
*******************************************************************************/
void perf_log(tPERF_DATA* t)
{

    if (t->lapse)
    {
        if (t->bytes)
            ALOGD( "%s:%s, bytes=%ld, lapse=%ld (%d.%02d kbps) (bus data rate %d.%02d kbps) overhead %d(%d percent)\n",
                    __func__,
                    t->label, t->bytes, t->lapse,
                    (int)(8 * t->bytes / t->lapse), (int)(800 * t->bytes / (t->lapse)) % 100,
                    (int)(9 * (t->bytes + t->count * t->overhead) / t->lapse), (int)(900 * (t->bytes + t->count * t->overhead) / (t->lapse)) % 100,
                    (int)(t->count * t->overhead), (int)(t->count * t->overhead * 100 / t->bytes)
                    );
        else
            ALOGD( "%s:%s, lapse=%ld (average %ld)\n", __func__,
                    t->label, t->lapse, (int)t->lapse / t->count
                    );
    }
    perf_reset(t);
}

/*******************************************************************************
**
** Function         perf_update
**
** Description      update perforamnce measurement data
**
** Returns          none
**
*******************************************************************************/
void perf_update(tPERF_DATA* t, long lapse, long bytes)
{
    if (!perf_log_every_count)
        return;
    // round to nearest ms
    lapse += 500;
    lapse /= 1000;
    t->count++;
    t->bytes += bytes;
    t->lapse += lapse;
    if (t->count == perf_log_every_count)
        perf_log(t);
}

static tPERF_DATA   perf_poll = {"USERIAL_Poll", 0, 0, 0, 0};
static tPERF_DATA   perf_read = {"USERIAL_Read", 0, 0, 0, 9};
static tPERF_DATA   perf_write = {"USERIAL_Write", 0, 0, 0, 3};
static tPERF_DATA   perf_poll_2_poll = {"USERIAL_Poll_to_Poll", 0, 0, 0, 0};
static clock_t      _poll_t0 = 0;

static UINT32 userial_baud_tbl[] =
{
    300,        /* USERIAL_BAUD_300          0 */
    600,        /* USERIAL_BAUD_600          1 */
    1200,       /* USERIAL_BAUD_1200         2 */
    2400,       /* USERIAL_BAUD_2400         3 */
    9600,       /* USERIAL_BAUD_9600         4 */
    19200,      /* USERIAL_BAUD_19200        5 */
    57600,      /* USERIAL_BAUD_57600        6 */
    115200,     /* USERIAL_BAUD_115200       7 */
    230400,     /* USERIAL_BAUD_230400       8 */
    460800,     /* USERIAL_BAUD_460800       9 */
    921600,     /* USERIAL_BAUD_921600       10 */
    1000000,    /* USERIAL_BAUD_1M           11 */
    1500000,    /* USERIAL_BAUD_1_5M         12 */
    2000000,    /* USERIAL_BAUD_2M           13 */
    3000000,    /* USERIAL_BAUD_3M           14 */
    4000000     /* USERIAL_BAUD_4M           15 */
};

int DT_Set_Power(int state)
{
    int ret = 0;
    if (state >= 0)
    {
        if (state == NFCC_REG_WAKE)
        {
           nci_wake_done = 1;
#ifdef DISP_NCI_FAKE
            UINT8 pp[] = {0x2f,0x03,0x01,0xFF}; /* perform NCI wake */
            UINT8 len = 4;
            DISP_NCI_FAKE(pp, len, FALSE);
#endif
        }
        ret = dTransport.rst(state);
#ifdef DISP_NCI_FAKE
        if(state == NFCC_REG_WAKE)
        {
            /*
             * if wakeup succeeds, kernel reports length of wakeup register (1)
             * if wakeup does not succeed, kernel reports error -EIO
             */
            UINT8 pp_res[] = {0x4f,0x03,0x01,0x00};
            /* error code is in -ret */
            pp_res[3] = (ret < 0) ? -ret : 0x00;
            UINT8 len_res = 4;
            DISP_NCI_FAKE(pp_res, len_res, TRUE);
        }
#endif
    }
    return ret;
}


/* Method to return NFCC hardware version */
int DT_Get_Nfcc_Version(int field)
{
    int retversion = 0xFF;
    if (field >= 0)
    {
        retversion = dTransport.version(field);
    }
    return (retversion);
}

/* Method to return NFCC hardware is fused or unfused */
int DT_Get_Nfcc_efuse()
{
    int retversion = 0xFF;
    retversion = dTransport.efuse();
    return (retversion);
}

/* Method to return if initial NFCC CORE_RESET_NTF has arrived or not.
 * 0xFF is for the caller to determine if the call went through or not. */
int DT_Get_Nfcc_initial_ntf()
{
    int ret = 0xFF;
    ret = dTransport.ntf();
    return (ret);
}
/*******************************************************************************
**
** Function           setWriteDelay
**
** Description        Record a delay for the next write operation
**
** Input Parameter    delay in milliseconds
**
** Comments           use this function to register a delay before next write,
**                    This is used in three instances: power up delay, wake delay
**                    and write delay
**
*******************************************************************************/
static void setWriteDelay(int delay)
{
    if (delay <= 0) {
        // Set a minimum delay of 5ms between back-to-back writes
        delay = 5;
    }

    clock_gettime(CLOCK_MONOTONIC, &linux_cb.write_time);
    if (delay > 1000)
    {
        linux_cb.write_time.tv_sec += delay / 1000;
        delay %= 1000;
    }
    unsigned long write_delay = delay * 1000 * 1000;
    linux_cb.write_time.tv_nsec += write_delay;
    if (linux_cb.write_time.tv_nsec > 1000*1000*1000)
    {
        linux_cb.write_time.tv_nsec -= 1000*1000*1000;
        linux_cb.write_time.tv_sec++;
    }
}

/*******************************************************************************
**
** Function           doWriteDelay
**
** Description        Execute a delay as registered in setWriteDelay()
**
** Output Parameter   none
**
** Returns            none
**
** Comments           This function calls GKI_Delay to execute a delay to fulfill
**                    the delay registered earlier.
**
*******************************************************************************/
static void doWriteDelay()
{
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    long delay = 0;

    if (now.tv_sec > linux_cb.write_time.tv_sec)
        return;
    else if (now.tv_sec == linux_cb.write_time.tv_sec)
    {
        if (now.tv_nsec > linux_cb.write_time.tv_nsec)
            return;
        delay = (linux_cb.write_time.tv_nsec - now.tv_nsec) / 1000000;
    }
    else
        delay = (linux_cb.write_time.tv_sec - now.tv_sec) * 1000 + linux_cb.write_time.tv_nsec / 1000000 - now.tv_nsec / 1000000;

    if (delay > 0 && delay < 1000)
    {
        ALOGD_IF((appl_trace_level>=BT_TRACE_LEVEL_DEBUG), "doWriteDelay() delay %ld ms", delay);
        GKI_delay(delay);
    }
}

/*******************************************************************************
**
** Function         create_signal_fds
**
** Description      create a socketpair for read thread to use
**
** Returns          file descriptor
**
*******************************************************************************/

static int signal_fds[2];
static inline int create_signal_fds(struct pollfd* set)
{
    if (signal_fds[0] == 0 && socketpair(AF_UNIX, SOCK_STREAM, 0, signal_fds) < 0)
    {
        ALOGE("%s create_signal_sockets:socketpair failed, errno: %d", __func__, errno);
        return -1;
    }
    set->fd = signal_fds[0];
    return signal_fds[0];
}

/*******************************************************************************
**
** Function         close_signal_fds
**
** Description      close the socketpair
**
** Returns          none
**
*******************************************************************************/
static inline void close_signal_fds()
{
    close(signal_fds[0]);
    signal_fds[0] = 0;

    close(signal_fds[1]);
    signal_fds[1] = 0;
}

/*******************************************************************************
**
** Function         send_wakeup_signal
**
** Description      send a one byte data to the socket as signal to the read thread
**                  for it to stop
**
** Returns          number of bytes sent, or error no
**
*******************************************************************************/
static inline int send_wakeup_signal()
{
    char sig_on = 1;
    return send(signal_fds[1], &sig_on, sizeof(sig_on), 0);
}

/*******************************************************************************
**
** Function         reset_signal
**
** Description      read the one byte data from the socket
**
** Returns          received data
**
*******************************************************************************/
static inline int reset_signal()
{
    char sig_recv = 0;
    recv(signal_fds[0], &sig_recv, sizeof(sig_recv), MSG_WAITALL);
    return (int)sig_recv;
}

/*******************************************************************************
**
** Function         is_signaled
**
** Description      test if there's data waiting on the socket
**
** Returns          TRUE is data is available
**
*******************************************************************************/
static inline int is_signaled(struct pollfd* set)
{
    return ((set->revents & POLLIN) == POLLIN) || ((set->revents & POLLRDNORM) == POLLRDNORM);
}

/******************************************************************************/

typedef unsigned char uchar;

BUFFER_Q Userial_in_q;

/*******************************************************************************
 **
 ** Function           USERIAL_GetLineSpeed
 **
 ** Description        This function convert USERIAL baud to line speed.
 **
 ** Output Parameter   None
 **
 ** Returns            line speed
 **
 *******************************************************************************/
UDRV_API extern UINT32 USERIAL_GetLineSpeed(UINT8 baud)
{
    return (baud <= USERIAL_BAUD_4M) ?
            userial_baud_tbl[baud-USERIAL_BAUD_300] : 0;
}

/*******************************************************************************
 **
 ** Function           USERIAL_GetBaud
 **
 ** Description        This function convert line speed to USERIAL baud.
 **
 ** Output Parameter   None
 **
 ** Returns            line speed
 **
 *******************************************************************************/
UDRV_API extern UINT8 USERIAL_GetBaud(UINT32 line_speed)
{
    UINT8 i;
    for (i = USERIAL_BAUD_300; i <= USERIAL_BAUD_921600; i++)
    {
        if (userial_baud_tbl[i-USERIAL_BAUD_300] == line_speed)
            return i;
    }

    return USERIAL_BAUD_AUTO;
}

/*******************************************************************************
**
** Function           USERIAL_Init
**
** Description        This function initializes the  serial driver.
**
** Output Parameter   None
**
** Returns            Nothing
**
*******************************************************************************/

UDRV_API void    USERIAL_Init(void * p_cfg)
{
    ALOGI(__FUNCTION__);

    //if userial_close_thread() is waiting to run; let it go first;
    //let it finish; then continue this function
    while (TRUE)
    {
        pthread_mutex_lock(&close_thread_mutex);
        if (is_close_thread_is_waiting)
        {
            pthread_mutex_unlock(&close_thread_mutex);
            ALOGI("USERIAL_Init(): wait for close-thread");
            sleep (1);
        }
        else
            break;
    }

    memset(&linux_cb, 0, sizeof(linux_cb));
    linux_cb.sock = -1;
    linux_cb.ser_cb = NULL;
    linux_cb.sock_power_control = -1;
    linux_cb.client_device_address = 0;
    GKI_init_q(&Userial_in_q);
    pthread_mutex_unlock(&close_thread_mutex);
}

/*******************************************************************************
 **
 ** Function           my_read
 **
 ** Description        This function read a packet from driver.
 **
 ** Output Parameter   None
 **
 ** Returns            number of bytes in the packet or error code
 **
 *******************************************************************************/
int my_read(int fd, uchar *pbuf, int len)
{
    struct pollfd fds[2];

    int n = 0;
    int ret = 0;
    int count = 0;
    int offset = 0;
    clock_t t1, t2;

    if (!isLowSpeedTransport && _timeout != POLL_TIMEOUT)
        ALOGD_IF((appl_trace_level>=BT_TRACE_LEVEL_DEBUG), "%s: enter, pbuf=%lx, len = %d\n", __func__, (unsigned long)pbuf, len);
    memset(pbuf, 0, len);
    /* need to use select in order to avoid collistion between read and close on same fd */
    /* Initialize the input set */
    fds[0].fd = fd;
    fds[0].events = POLLIN | POLLERR | POLLRDNORM;
    fds[0].revents = 0;

    create_signal_fds(&fds[1]);
    fds[1].events = POLLIN | POLLERR | POLLRDNORM;
    fds[1].revents = 0;
    t1 = clock();
    n = poll(fds, 2, _timeout);
    t2 = clock();
    perf_update(&perf_poll, t2 - t1, 0);
    if (_poll_t0)
        perf_update(&perf_poll_2_poll, t2 - _poll_t0, 0);

    _poll_t0 = t2;
    /* See if there was an error */
    if (n < 0)
    {
        ALOGD( "select failed; errno = %d\n", errno);
        return -errno;
    }
    else if (n == 0)
        return -EAGAIN;

    if (is_signaled(&fds[1]))
    {
        ALOGD( "%s: exit signal received\n", __func__);
        reset_signal();
        return -1;
    }
    if (!bSerialPortDevice || len < MIN_BUFSIZE)
        count = len;
    else
        count = 1;
    do {
        t2 = clock();
        ret = read(fd, pbuf+offset, (size_t)count);
        if (ret > 0)
            perf_update(&perf_read, clock()-t2, ret);

        if (ret <= 0 || !bSerialPortDevice || len < MIN_BUFSIZE)
            break;

        if (isLowSpeedTransport)
            goto done;

        if (offset == 0)
        {
            if (pbuf[offset] == HCIT_TYPE_NFC)
                count = 3;
            else if (pbuf[offset] == HCIT_TYPE_EVENT)
                count = 2;
            else
            {
                ALOGD( "%s: unknown HCIT type header pbuf[%d] = %x\n", __func__, offset, pbuf[offset]);
                break;
            }
            offset = 1;
        }
        else if (offset == 1)
        {
            offset += count;
            count = pbuf[offset-1];

            if (count > (len - offset)) //if (count > (remaining buffer size))
                count = len - offset; //only read what the remaining buffer size can hold

        }
        else
        {
            offset += ret;
            count -= ret;
        }
        if (count == 0)
        {
            ret = offset;
            break;
        }
    } while (count > 0);
 #if VALIDATE_PACKET
/*
 * vallidate the packet structure
 */
    if (ret > 0 && len >= MIN_BUFSIZE)
    {
        count = 0;
        while (count < ret)
        {
            if (pbuf[count] == HCIT_TYPE_NFC)
            {
                if (USERIAL_Debug_verbose)
                    scru_dump_hex(pbuf+count, NULL, pbuf[count+3]+4, 0, 0);
                count += pbuf[count+3]+4;
            }
            else if (pbuf[count] == HCIT_TYPE_EVENT)
            {
                if (USERIAL_Debug_verbose)
                    scru_dump_hex(pbuf+count, NULL, pbuf[count+2]+3, 0, 0);
                count += pbuf[count+2]+3;
            }
            else
            {
                ALOGD( "%s: unknown HCIT type header pbuf[%d] = %x, remain %d bytes\n", __func__, count, pbuf[count], ret-count);
                scru_dump_hex(pbuf+count, NULL, ret - count, 0, 0);
                break;
            }
        } /* while*/
    }
#endif
done:
    if (!isLowSpeedTransport)
        ALOGD_IF((appl_trace_level>=BT_TRACE_LEVEL_DEBUG), "%s: return %d(0x%x) bytes, errno=%d count=%d, n=%d, timeout=%d\n", __func__,
            ret, ret, errno, count, n, _timeout);
    if (_timeout == POLL_TIMEOUT)
        _timeout = -1;
    return ret;
}
extern BOOLEAN gki_chk_buf_damage(void *p_buf);
static int sRxLength = 0;


/*******************************************************************************
 **
 ** Function           DT_read_thread
 **
 ** Description        entry point of read thread.
 **
 ** Output Parameter   None
 **
 ** Returns            0
 **
 *******************************************************************************/
UINT32 DT_read_thread(UINT32 arg)
{
    int rx_length;
    int error_count = 0;
    int bErrorReported = 0;
    int iMaxError = MAX_ERROR;
    BT_HDR *p_buf = NULL;
    UINT8 *p1 = NULL;
    UINT8 mt = 0, pbf = 0, gid = 0, op_code = 0 , payload_len = 0;
    BOOLEAN ignore_nfccevent = FALSE;

    worker_thread1 = pthread_self();

    ALOGD( "start userial_read_thread, id=%lx", worker_thread1);
    _timeout = POLL_TIMEOUT;

    for (;linux_cb.sock > 0;)
    {
        BT_HDR *p_buf;
        UINT8 *current_packet;
        p_buf = (BT_HDR *) GKI_getpoolbuf( DT_POOL_ID );
        if (p_buf != NULL)
        {
            p_buf->offset = 0;
            p_buf->layer_specific = 0;

            current_packet = (UINT8 *) (p_buf + 1);
            if (current_packet != NULL)
            {
                rx_length = my_read(linux_cb.sock, current_packet, READ_LIMIT);
            }
            else
            {
                ALOGE( "DT_read_thread(): unable to get buffer from GKI p_buf = %p poolid = %d\n", p_buf, DT_POOL_ID);
                rx_length = 0;  /* paranoia setting */
                GKI_freebuf( p_buf );
                GKI_delay( NO_GKI_BUFFER_RECOVER_TIME );
                continue;
            }
        }
        else
        {
            ALOGE( "DT_read_thread(): unable to get buffer from GKI p_buf = %p poolid = %d\n", p_buf, DT_POOL_ID);
            rx_length = 0;  /* paranoia setting */
            GKI_delay( NO_GKI_BUFFER_RECOVER_TIME );
            continue;
        }
        if (rx_length > 0)
        {
            bErrorReported = 0;
            error_count = 0;
            iMaxError = 3;
            if (rx_length > sRxLength)
                sRxLength = rx_length;
            p_buf->len = (UINT16)rx_length;
            /*  QNCI_FEATURE_UI_SCREEN_ERR_HANDLE */
            p1 = current_packet;
            if (p1 != NULL)
            {
                NCI_MSG_PRS_HDR0 (p1, mt, pbf, gid);
                if ((mt == NCI_MT_NTF) && (gid == NCI_GID_RF_MANAGE))
                {
                    NCI_MSG_PRS_HDR1 (p1, op_code);
                    payload_len = *p1++;
                    if ( op_code == NCI_MSG_RF_INTF_ACTIVATED)
                    {
                        if ((payload_len > 3) &&
                           (*(p1 + 1) != NCI_INTERFACE_EE_DIRECT_RF) &&
                           (((*(p1 + 3)) & 0x80 )== 0x00))
                        {
                            if (nfc_hal_cb.sent_uioff == TRUE)
                            {
                                ignore_nfccevent = TRUE;
                            }
                            else
                            {
                                nfc_hal_cb.poll_activated_event = TRUE;
                            }
                        }
                    }
                    else if ( op_code == NCI_MSG_RF_DISCOVER )
                    {
                        nfc_hal_cb.poll_activated_event = TRUE;
                    }

                    else if (op_code == NCI_MSG_RF_DEACTIVATE)
                    {
                        nfc_hal_cb.poll_activated_event = FALSE;
                    }
                }
            }
            else
            {
                ALOGE( "DT_read_thread(): null data");
            }
            /*  QNCI_FEATURE_UI_SCREEN_ERR_HANDLE */
            if (ignore_nfccevent == FALSE)
            {
                GKI_enqueue(&Userial_in_q, p_buf);
                if (!isLowSpeedTransport)
                    ALOGD_IF((appl_trace_level>=BT_TRACE_LEVEL_DEBUG), "DT_read_thread(): enqueued p_buf=%p, count=%d, length=%d\n",
                                p_buf, Userial_in_q.count, rx_length);

                GKI_send_event (NFC_HAL_TASK, NFC_HAL_TASK_EVT_DATA_RDY);
            }
            else
            {
                ignore_nfccevent = FALSE;
                GKI_freebuf( p_buf );
                ALOGE( "DT_read_thread(): Ignore act ntf uicmd off/lock");
            }
        }
        else
        {
            GKI_freebuf( p_buf );
            if (rx_length == -EAGAIN)
                continue;
            else if (rx_length == -1)
            {
                ALOGD( "DT_read_thread(): exiting\n");
                break;
            }
            else if (rx_length == 0 && (!nfc_hal_dm_is_nfcc_awake()))
            {
                continue;
            }
            ++error_count;
            if (rx_length <= 0 && ((error_count > 0) && ((error_count % iMaxError) == 0)))
            {
                if (bErrorReported == 0)
                {
                    ALOGE( "DT_read_thread(): my_read returned (%d) error count = %d, errno=%d return USERIAL_ERR_EVT\n",
                            rx_length, error_count, errno);
                    if (linux_cb.ser_cb != NULL)
                        (*linux_cb.ser_cb)(linux_cb.port, USERIAL_ERR_EVT, (tUSERIAL_EVT_DATA *)p_buf);

                    GKI_send_event(USERIAL_HAL_TASK, HCISU_EVT);
                    ++bErrorReported;
                }
                if (sRxLength == 0)
                {
                    ALOGE( "DT_read_thread(): my_read returned (%d) error count = %d, errno=%d exit read thread\n",
                            rx_length, error_count, errno);
                    break;
                }
            }
        }
    } /* for */







    ALOGD( "DT READ THREAD: EXITING TASK\n");

    return 0;
}

UINT16 DT_Unprocessed_Data()
{
    UINT16 len = 0;
    len = GKI_queue_length (&Userial_in_q);
    return len;
}

/*******************************************************************************
 **
 ** Function           userial_to_tcio_baud
 **
 ** Description        helper function converts USERIAL baud rates into TCIO conforming baud rates
 **
 ** Output Parameter   None
 **
 ** Returns            TRUE - success
 **                    FALSE - unsupported baud rate, default of 115200 is used
 **
 *******************************************************************************/
BOOLEAN userial_to_tcio_baud(UINT8 cfg_baud, UINT32 * baud)
{
    if (cfg_baud == USERIAL_BAUD_600)
        *baud = B600;
    else if (cfg_baud == USERIAL_BAUD_1200)
        *baud = B1200;
    else if (cfg_baud == USERIAL_BAUD_9600)
        *baud = B9600;
    else if (cfg_baud == USERIAL_BAUD_19200)
        *baud = B19200;
    else if (cfg_baud == USERIAL_BAUD_57600)
        *baud = B57600;
    else if (cfg_baud == USERIAL_BAUD_115200)
        *baud = B115200 | CBAUDEX;
    else if (cfg_baud == USERIAL_BAUD_230400)
        *baud = B230400;
    else if (cfg_baud == USERIAL_BAUD_460800)
        *baud = B460800;
    else if (cfg_baud == USERIAL_BAUD_921600)
        *baud = B921600;
    else if (cfg_baud == USERIAL_BAUD_1M)
        *baud = B1000000;
    else if (cfg_baud == USERIAL_BAUD_2M)
        *baud = B2000000;
    else if (cfg_baud == USERIAL_BAUD_3M)
        *baud = B3000000;
    else if (cfg_baud == USERIAL_BAUD_4M)
        *baud = B4000000;
    else
    {
        ALOGE( "USERIAL_Open: unsupported baud idx %i", cfg_baud );
        *baud = B115200;
        return FALSE;
    }
    return TRUE;
}

#if (USERIAL_USE_IO_BT_WAKE==TRUE)
/*******************************************************************************
 **
 ** Function           userial_io_init_bt_wake
 **
 ** Description        helper function to set the open state of the bt_wake if ioctl
 **                    is used. it should not hurt in the rfkill case but it might
 **                    be better to compile it out.
 **
 ** Returns            none
 **
 *******************************************************************************/
void userial_io_init_bt_wake( int fd, unsigned long * p_wake_state )
{
    /* assert BT_WAKE for ioctl. should NOT hurt on rfkill version */
    ioctl( fd, USERIAL_IO_BT_WAKE_ASSERT, NULL);
    ioctl( fd, USERIAL_IO_BT_WAKE_GET_ST, p_wake_state );
    if ( *p_wake_state == 0)
        ALOGI("\n***userial_io_init_bt_wake(): Ooops, asserted BT_WAKE signal, but still got BT_WAKE state == to %d\n",
             *p_wake_state );

    *p_wake_state = 1;
}
#endif

/*******************************************************************************
**
** Function           DT_Nfc_Open
**
** Description        Configure the physical interface to controller.
**
** Output Parameter   None
**
** Returns            Nothing
**
*******************************************************************************/
NFC_RETURN_CODE DT_Nfc_Open(DT_Nfc_sConfig_t *pDriverConfig, int *pdTransportHandle, void (*nci_cb) )
{
   NFC_RETURN_CODE retstatus = NFC_SUCCESS;
   UINT16 chip_version, region2_info = 0;
   UINT16 chip_revid;
   UINT8 chip_version_major = 0, chip_version_minor = 0;
   UINT16 metal_version = 0;
   UINT16 Cfg;
   UINT32 debug_enable = 0;
   UINT8 stored_restart_reason = 0,efuse_value=0;
   /* if userial_close_thread() is waiting to run; let it go first;
      let it finish; then continue this function */

   while (TRUE){
       pthread_mutex_lock(&close_thread_mutex);
       if (is_close_thread_is_waiting){
           pthread_mutex_unlock(&close_thread_mutex);
           ALOGI("DT_Nfc_Open(): wait for close-thread");
           sleep (1);
        }
        else {
            break;
        }
   }

   stored_restart_reason = nfc_hal_retrieve_info();

   HAL_TRACE_DEBUG0 ("DT:DT_Nfc_Open");

   if ((pDriverConfig == NULL) || (pdTransportHandle == NULL))
   {
       HAL_TRACE_DEBUG0 ("DT:DT_Nfc_Open : phwref == NULL");
       retstatus = NFC_FAILED;
       goto done_open;
   }

   if (pDriverConfig->devFile == NULL) {
       HAL_TRACE_DEBUG0 ("DT:DT_Nfc_Open : devFile NULL");
       retstatus = NFC_FAILED;
       goto done_open;
   }

   memset(&dTransport, 0, sizeof(DT_Nfc_Phy_select_t));
   memcpy(&DriverConfig, pDriverConfig, sizeof(DT_Nfc_sConfig_t));

   switch(pDriverConfig->phyType)
   {
      case ENUM_LINK_TYPE_I2C:
      {
         dTransport.init        = DT_Nfc_i2c_initialize;
         dTransport.close       = DT_Nfc_i2c_close;
         dTransport.setup       = DT_Nfc_i2c_setup;
         dTransport.rst         = DT_Nfc_i2c_reset;
         dTransport.version     = DT_Nfc_i2c_version;
         dTransport.efuse       = DT_Nfc_i2c_efuse_type;
         dTransport.ntf         = DT_Nfc_check_nfcc_initial_core_reset_ntf;
         break;
      }

      case ENUM_LINK_TYPE_UART:
      case ENUM_LINK_TYPE_USB:
      default:
      {
         HAL_TRACE_DEBUG0 ("DT:DT_Nfc_Open : phyType not supported");
         retstatus = NFC_FAILED;
         goto done_open;
      }
   }

   if(dTransport.init == NULL)
   {
      HAL_TRACE_DEBUG0 ("DT:DT_Nfc_Open : dTransport.init() not initialized");
      retstatus = NFC_FAILED;
      goto done_open;
   }

   dTransport.init();
   retstatus = dTransport.setup(pDriverConfig, pdTransportHandle);

   if (retstatus != NFC_SUCCESS)
   {
       HAL_TRACE_DEBUG0 ("NFC:DT:DT_Nfc_Open **FAIL**: NFC DEVICE NOT LOADED or NFCC HARDWARE NOT FOUND");
       retstatus = NFC_NO_HW;
       goto done_open;
   } else if (stored_restart_reason == NFCSERVICE_GIVE_UP) {
       HAL_TRACE_DEBUG0 ("NFC:DT:DT_Nfc_Open **FAIL**: Several restart attempts failed, deactivate nfc until user restarts.");
       retstatus = NFC_NO_HW;
       nfc_hal_store_info(REMOVE_INFO);
       goto done_open;
   }

   if ((int) (*pdTransportHandle) == 0){
         HAL_TRACE_DEBUG0 ("DT:DT_Nfc_Open : Open but assigned 0 close it down !!! ...");
         HAL_TRACE_DEBUG0 ("DT:DT_Nfc_Open : Try to reassign ...");
         retstatus = dTransport.setup(pDriverConfig, pdTransportHandle);
         HAL_TRACE_DEBUG1("DT:DT_Nfc_Open (second):  New Handle = %d ...\n",(int) (*pdTransportHandle) );
   }

   if (retstatus != NFC_SUCCESS)
   {
       ALOGE ("NFC:DT:DT_Nfc_Open FAIL (second): can't open DT ...");
       retstatus = NFC_FAILED;
       goto done_open;
   }

   /* Store context here now */
   linux_cb.sock = (int) (*pdTransportHandle);      /* cast back to integer */
   linux_cb.ser_cb     = nci_cb;                    /* was p_cback; */

   RdWrContext.uiPoolID = DT_POOL_ID;

   /* Initialise semaphore */
   if(sem_init(&data_available, 0, 0) == -1){
        ALOGE ("DT:DT_Nfc_Open : NFC Init Semaphore creation Error");
        retstatus = NFC_FAILED;
        goto done_open;
   }

   HAL_TRACE_DEBUG2 ("DT_Nfc_Open().stored_restart_reason = %d current_mode=%d",stored_restart_reason,current_mode);

   if(((reset_status == TRUE) && (stored_restart_reason != NFCSERVICE_WATCHDOG_TIMER_EXPIRED))|| (current_mode == FTM_MODE))
   {
       DT_Set_Power( SET_NFCC_OFF );
       GKI_delay (NFCC_RESET_DELAY);
       DT_Set_Power( SET_NFCC_ON );
       /* Invoke kernel routine to re-initialise NFCC as ALL config will be lost */
       retstatus = DT_Set_Power( SET_NFCC_INIT );
       if (retstatus != NFC_SUCCESS)
       {
           ALOGE ("DT:DT_Nfc_Open : can't initialise NFCC hardware, assume none present.");
           retstatus = NFC_NO_HW;
           goto done_open;
       }
       nfc_hal_store_info(REMOVE_INFO);
       GKI_delay( NFCC_INIT_DELAY );
   }

   chip_version = DT_Get_Nfcc_Version( NFCC_CHIP_VERSION_REG );
   chip_version_minor = (chip_version & 0x0F);

   /* Chip version minor will be 1 in case of QCA1990A and 0 in case of QCA1990 always*/
   if(chip_version_minor == QCA1990_CHIP_VERSION_MINOR)
   {
       /*Read chip version and metal revision as per QCA1990 chip way*/
       /*[7:4] - Major and [3:0] - Minor*/
       chip_version_major = ((chip_version >> 4)&(0xF));
       nfc_hal_cb.dev_cb.nfcc_chip_type = QCA1990;
       ALOGD("DT:DT_Nfc_Open : chip is QCA1990 \n");
   }
   else if(chip_version_minor == QCA1990A_CHIP_VERSION_MINOR)
   {
       /*Read chip version and metal revision as per QCA1990A chip way*/
       /*[7:4] - Minor and [3:0] - Major*/
       chip_version_major = chip_version & 0x0F;
       nfc_hal_cb.dev_cb.nfcc_chip_type = QCA1990A;
       ALOGD("DT:DT_Nfc_Open : chip is QCA1990A \n");
   }
   GKI_delay( WRITE_DELAY );
   chip_revid = DT_Get_Nfcc_Version( NFCC_CHIP_REVID_REG );
   metal_version = (chip_revid & (0xF));
   ALOGD("DT:DT_Nfc_Open : chip version = %d.%d\n", chip_version_major, metal_version);
   GKI_delay( WRITE_DELAY );
   /*Store chip version and metal revision in HAL CB.Used in patch propogation*/
   nfc_hal_cb.dev_cb.nfcc_chip_version = chip_version_major;
   nfc_hal_cb.dev_cb.nfcc_chip_metal_version = metal_version;
   efuse_value = DT_Get_Nfcc_efuse();
   ALOGD("DT:DT_Nfc_Open : efuse_value = %02x and chip is %s\n",efuse_value,((1 << 1) & efuse_value) ? "FUSED" : "UNFUSED" );
   nfc_hal_cb.dev_cb.efuse_value = ((1 << 1) & efuse_value) ? 1 : 0;
   /* Create reader thread */
   GKI_create_task ((TASKPTR)DT_read_thread, USERIAL_HAL_TASK, (INT8*)"USERIAL_HAL_TASK", 0, 0, (pthread_cond_t*)NULL, NULL);

   /* Initialise the DT context */
   /* Reset the Reader Thread values to NULL */
   memset((void *)&RdWrContext,0,sizeof(RdWrContext));
   RdWrContext.ReaderThreadAlive    = TRUE;
   RdWrContext.WriteBusy            = FALSE;
   RdWrContext.WaitOnWrite          = FALSE;

   /* Configure Read Mode for NFCC - set to unsolicited */
   RdWrContext.nReadMode = UNSOLICITED;


   /* Shared Mem Access */
   if (RdWrContext.block_zero_base == NULL){
       RdWrContext.block_zero_base = mmap(NULL, (NFC_PAGE_SIZE * PAGES), 0x1|0x2, 0 | 0x01, linux_cb.sock, 0);
   }

   /* Now enable handler Interrupts - For Reads */
#ifndef USING_POLL_WAIT
   reg_int_handler(linux_cb.sock);
#endif
done_open:
   pthread_mutex_unlock(&close_thread_mutex);
   ALOGI("DT_Nfc_Open(): exit retstatus = %d ",retstatus);
   return retstatus;
}
/*******************************************************************************
**
** Function           DT_Nfc_Read
**
** Description        Read data from a serial port using byte buffers.
**
** Output Parameter   None
**
** Returns            Number of bytes actually read from the serial port and
**                    copied into p_data.  This may be less than len.
**
*******************************************************************************/

static BT_HDR *pbuf_dt_Read = NULL;

UDRV_API UINT16  DT_Nfc_Read(tUSERIAL_PORT port, UINT8 *p_data, UINT16 len)
{
    UINT16 total_len = 0;
    UINT16 copy_len = 0;
    UINT8 * current_packet = NULL;

    if (pbuf_dt_Read == NULL && (total_len < len))
        pbuf_dt_Read = (BT_HDR *)GKI_dequeue(&Userial_in_q);

    if (pbuf_dt_Read != NULL)
    {
        current_packet = ((UINT8 *)(pbuf_dt_Read + 1)) + (pbuf_dt_Read->offset);

        if ((pbuf_dt_Read->len) <= (len - total_len))
            copy_len = pbuf_dt_Read->len;
        else
            copy_len = (len - total_len);

        memcpy((p_data + total_len), current_packet, copy_len);

        total_len += copy_len;

        pbuf_dt_Read->offset += copy_len;
        pbuf_dt_Read->len -= copy_len;

        if (pbuf_dt_Read->len == 0)
        {
            GKI_freebuf(pbuf_dt_Read);
            pbuf_dt_Read = NULL;
        }
    }

    return total_len;
}
/*******************************************************************************
**
** Function           USERIAL_Readbuf
**
** Description        Read data from a serial port using GKI buffers.
**
** Output Parameter   Pointer to a GKI buffer which contains the data.
**
** Returns            Nothing
**
** Comments           The caller of this function is responsible for freeing the
**                    GKI buffer when it is finished with the data.  If there is
**                    no data to be read, the value of the returned pointer is
**                    NULL.
**
*******************************************************************************/
UDRV_API void    USERIAL_ReadBuf(tUSERIAL_PORT port, BT_HDR **p_buf)
{

}
/*******************************************************************************
**
** Function           USERIAL_WriteBuf
**
** Description        Write data to a serial port using a GKI buffer.
**
** Output Parameter   None
**
** Returns            TRUE  if buffer accepted for write.
**                    FALSE if there is already a buffer being processed.
**
** Comments           The buffer will be freed by the serial driver.  Therefore,
**                    the application calling this function must not free the
**                    buffer.
**
*******************************************************************************/
UDRV_API BOOLEAN USERIAL_WriteBuf(tUSERIAL_PORT port, BT_HDR *p_buf)
{
    return FALSE;
}
/*******************************************************************************
**
** Function           DT_Nfc_Write
**
** Description        Write data to a serial port using a byte buffer.
**
** Output Parameter   None
**
** Returns            Number of bytes actually written to the transport.  This
**                    may be less than len.
**
*******************************************************************************/
UDRV_API UINT16  DT_Nfc_Write(tUSERIAL_PORT port, UINT8 *p_data, UINT16 len)
{
    int ret = 0, total = 0;
    int i = 0;
    UINT8 dontsend = FALSE; /*  QNCI_FEATURE_UI_SCREEN_ERR_HANDLE */
    clock_t t;

    ALOGD_IF((appl_trace_level>=BT_TRACE_LEVEL_DEBUG), "DT_Nfc_Write: (%d bytes) - \n", len);

    if(nci_wake_done)
    {
#ifdef DTA // <DTA>
        if(nfc_hal_in_dta_mode()) {
            if (nfc_hal_cb.listen_mode_activated == FALSE) {
                setWriteDelay(DTA_POLL_WAKE_DELAY);
                HAL_TRACE_DEBUG0("DT_Nfc_Write : DTA POLL MODE\n");
            } else {
                HAL_TRACE_DEBUG0("DT_Nfc_Write : DTA LISTEN MODE\n");
            }
        } else {
#endif // </DTA>
        /* In case of p2p target we don't have to set POLL_WAKE_DELAY */
        if ((nfc_hal_cb.listen_mode_activated == TRUE) && (nfc_hal_cb.act_interface == NCI_INTERFACE_NFC_DEP)) {
            setWriteDelay(P2P_TARGET_WAKE_DELAY);
        } else {
            setWriteDelay(NCI_WAKE_DELAY);
        }
        nci_wake_done = 0;
#ifdef DTA // <DTA>
    }
#endif
    }
    else
    {
            nfc_write_delay = WRITE_DELAY;
    }
    pthread_mutex_lock(&close_thread_mutex);
    doWriteDelay();
    t = clock();
    if((current_mode == FTM_MODE) && (nfc_hal_cb.dev_cb.patch_applied))
    {
        if(p_data[0] == 0xFF)
        {
            p_data = p_data + 1;
            len--;
            DT_Nfc_set_controller_mode(2);
            HAL_TRACE_DEBUG3("FTM_MODE : len = %X p_data[0]=%x p_data[1] = %x",len,p_data[0],p_data[1]);
        }
        else
        {
            HAL_TRACE_DEBUG0("FTM_MODE : Setting mode to 0");
            DT_Nfc_set_controller_mode(0);
        }
    }
    /* if prop UI state command from JNI */
    if((p_data[0] == NCI_PROP_CMD) &&
            (p_data[1] == NCI_MSG_PROP_GENERIC) &&
            (p_data[2] == 0x03)/* payload length */)
    {
        if (p_data[3] == NCI_MSG_PROP_GENERIC_SUBCMD_UI_STATE)
        {
            HAL_TRACE_DEBUG4 ("Got screen state cmd,%d,%d,%d,%d" , p_data[5], nfc_hal_cb.poll_activated_event, nfc_hal_cb.sent_uioff, nfc_hal_cb.poll_locked_mode);
             /*  QNCI_FEATURE_UI_SCREEN_ERR_HANDLE */
            if (nfc_hal_cb.poll_activated_event == TRUE)
            {
                if ((nfc_hal_cb.poll_locked_mode == TRUE) &&
                    (p_data[5] == NCI_PROP_NFCC_UI_STATE_OFF))
                {
                    /* Got screen cmd and poll activation at the same time to service poll activation first
                       Pend screen cmd and handle poll activation first*/
                    dontsend = TRUE;
                    HAL_TRACE_DEBUG0 ("Set pending screen cmd");
                    nfc_hal_cb.pending_screencmd = p_data[5];
                }
                else if ((nfc_hal_cb.poll_locked_mode == FALSE) &&
                         ((p_data[5] == NCI_PROP_NFCC_UI_STATE_OFF) ||
                          (p_data[5] == NCI_PROP_NFCC_UI_STATE_LOCKED)))
                {
                    dontsend = TRUE;
                    HAL_TRACE_DEBUG0 ("Set pending screen cmd");
                    nfc_hal_cb.pending_screencmd = p_data[5];
                }
                else
                {
                    if (nfc_hal_cb.pending_screencmd != 0xFF)
                    {
                        dontsend = TRUE;
                    }
                }
            }
            else
            {
                if ((nfc_hal_cb.poll_locked_mode == TRUE) &&
                    (p_data[5] != NCI_PROP_NFCC_UI_STATE_OFF))
                {
                    /* Got screen unlocked cmd ignore the pending screen off cmd*/
                    if (nfc_hal_cb.pending_screencmd != 0xFF)
                    {
                        nfc_hal_cb.pending_screencmd = 0xFF;
                    }
                    nfc_hal_cb.sent_uioff = FALSE;
                }
                else if ((nfc_hal_cb.poll_locked_mode == FALSE) &&
                    (p_data[5] == NCI_PROP_NFCC_UI_STATE_UNLOCKED))
                {
                    /* Got screen unlocked cmd ignore the pending screen off cmd*/
                    if (nfc_hal_cb.pending_screencmd != 0xFF)
                    {
                        nfc_hal_cb.pending_screencmd = 0xFF;
                    }
                    nfc_hal_cb.sent_uioff = FALSE;
                }
                else
                {
                    nfc_hal_cb.sent_uioff = TRUE;
                }
            }
        }
        else if (p_data[3] == NCI_MSG_PROP_GENERIC_HAL_CMD)
        {
            if (p_data[4] == NCI_PROP_HAL_POLLING_IN_LOCKED)
            {
                if (p_data[5] == NCI_PROP_HAL_ENABLE_POLL)
                {
                    nfc_hal_cb.poll_locked_mode = TRUE;
                    HAL_TRACE_DEBUG0 ("enable poll locked");
                }
                else
                {
                    nfc_hal_cb.poll_locked_mode = FALSE;
                    HAL_TRACE_DEBUG0 ("disable poll locked");
                }
            }
            dontsend = TRUE;
        }
        /*  QNCI_FEATURE_UI_SCREEN_ERR_HANDLE */
    }
    if (dontsend == FALSE)
    {
        while (len != 0 && linux_cb.sock != -1)
        {
            ret = write(linux_cb.sock, p_data + total, len);
            if (ret < 0)
            {
                ALOGE("USERIAL_Write len = %d, ret = %d, errno = %d", len, ret, errno);
                break;
            }
            else
            {
                ALOGD_IF((appl_trace_level>=BT_TRACE_LEVEL_DEBUG), "USERIAL_Write len = %d, ret = %d", len, ret);
            }
            total += ret;
            len -= ret;
        }
        perf_update(&perf_write, clock() - t, total);

        ALOGD_IF((appl_trace_level>=BT_TRACE_LEVEL_DEBUG), "DT_Nfc_Write len = %d, ret =  %d, errno = %d\n", len, ret, errno);

        /* register a delay for next write
         */
        setWriteDelay(total * nfc_write_delay / 1000);
    }
    pthread_mutex_unlock(&close_thread_mutex);
    return ((UINT16)total);
}
/*******************************************************************************
**
** Function           DT_Nfc_close_port
**
** Description        close the transport driver
**
** Returns            Nothing
**
*******************************************************************************/
void DT_Nfc_close_port( void )
{
    DT_Nfc_Close(linux_cb.port);
}

/*******************************************************************************
**
** Function         USERIAL_SetPowerOffDelays
**
** Description      Set power off delays used during USERIAL_Close().  The
**                  values in the conf. file setting override these if set.
**
** Returns          None.
**
*******************************************************************************/
UDRV_API void USERIAL_SetPowerOffDelays(int pre_poweroff_delay, int post_poweroff_delay)
{
    gPrePowerOffDelay = pre_poweroff_delay;
    gPostPowerOffDelay = post_poweroff_delay;
}
/*******************************************************************************
**
** Function           DT_Nfc_Close
**
** Description        Close the Physical Transport
**
** Output Parameter   None
**
** Returns            Nothing
**
*******************************************************************************/
void DT_Nfc_Close(DT_Nfc_sConfig_t *pDriverConfig)
{
    UINT32 debug_enable = 0,region2_enable = 0;
    pthread_attr_t attr;
    pthread_t      close_thread;

    ALOGD ("%s: enter", __FUNCTION__);

    // check to see if thread is already running
    if (pthread_mutex_trylock(&close_thread_mutex) == 0)
    {
        // mutex aquired so thread is not running
        is_close_thread_is_waiting = TRUE;
        pthread_mutex_unlock(&close_thread_mutex);

        // close transport in a new thread so we don't block the caller
        // make thread detached, no other thread will join
        pthread_attr_init(&attr);
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

        /* We do not want to enter region 2 whilst in ramdump state as it means not receiving RSPs for CMDs sent */
        if (DT_Nfc_RamdumpPerformed == FALSE)
        {
            if (!nfc_hal_cb.ncit_cb.hw_error)
            {
                if((shut_down_reason == NFC_DISABLED_FROM_UI) || (shut_down_reason == NFC_DISABLED_BY_AIRPLANEMODE))
                {
                    GetNumValue("REGION2_DEBUG_ENABLE_FLAG", &debug_enable, sizeof(debug_enable));
                    if((debug_enable == TRUE) && (shut_down_reason != NFC_DISABLED_BY_AIRPLANEMODE))
                    {
                        HAL_TRACE_DEBUG0("User Disabled NFC..Region 2 Debug ON ..Sending region2 eable command");
                        if(current_mode != FTM_MODE)
                        {
                            /* Send region2_enable cmd with debug disable first to send chip in region2 After the
                               chip has gone in region2 then region 2 enable command with debug on can be sent*/
                            nfc_hal_dm_send_prop_nci_region2_enable_cmd(REGION2_DEBUG_DISABLE);
                            nfc_hal_dm_set_nfc_wake (NFC_HAL_ASSERT_NFC_WAKE);
                            GKI_delay(10);
                            nfc_hal_cb.ncit_cb.nci_wait_rsp = NFC_HAL_WAIT_RSP_NONE;
                            nfc_hal_dm_send_prop_nci_region2_enable_cmd(REGION2_DEBUG_ENABLE);
                            /*Store this information that NFCC has gone in region 2 in /data/nfc/*/
                            nfc_hal_store_info(STORE_INFO_DEBUG_ENABLE);
                        }
                    }
                    else
                    {
                        HAL_TRACE_DEBUG0("***** User Disabled NFC :Region 2 Debug OFF :  Send Sleep cmd *******");
                        /*Store this information that NFCC has been disabled from settings UI in /data/nfc/*/
                        nfc_hal_store_info(STORE_INFO_NFC_DISABLED);
                        nfc_hal_dm_set_nfc_wake (NFC_HAL_DEASSERT_NFC_WAKE);
                        /*Put 100 ms delay so that Sleep rsp may be recieved before reader thread is closed*/
                        GKI_delay (100);
                    }
                }
                else if(shut_down_reason == NFCSERVICE_WATCHDOG_TIMER_EXPIRED)
                {
                    HAL_TRACE_DEBUG0(" nfc service watchdog timer expired..stor info");
                    nfc_hal_store_info(NFCSERVICE_WATCHDOG_TIMER_EXPIRED);
                }
                else
                {
                    /*Shut down reason is other than disable from UI so send region 2 enable command*/
                    /* Check if Region2 enable in libnfc-nci.conf file*/
                    GetNumValue("REGION2_ENABLE", &region2_enable, sizeof(region2_enable));
                    if(region2_enable)
                    {
                        if(current_mode != FTM_MODE)
                        {
                            HAL_TRACE_DEBUG0("NFC enabled..Device being shut down..sending region2 enable command");
                            /* Send region2_enable cmd with debug disable first to send chip in region2 After the
                               chip has gone in region2 then region 2 enable command with debug on can be sent*/
                            nfc_hal_dm_send_prop_nci_region2_enable_cmd(REGION2_DEBUG_DISABLE);
                            GetNumValue("REGION2_DEBUG_ENABLE_FLAG", &debug_enable, sizeof(debug_enable));
                            if(debug_enable)
                            {
                                nfc_hal_cb.ncit_cb.nci_wait_rsp = NFC_HAL_WAIT_RSP_NONE;
                                nfc_hal_dm_send_prop_nci_region2_enable_cmd(REGION2_DEBUG_ENABLE);
                                /*If region 2 debug is disabled then write 2 in nv file*/
                            }
                            /*Store this information that NFCC has gone in region 2 in /data/nfc/*/
                            nfc_hal_store_info(DEVICE_POWER_CYCLED);
                        }
                    }
                }
            }

        }
        else
        {
           HAL_TRACE_DEBUG0("DT_Nfc_Close: RAMDUMP - In ramdump state so not entering Region 2 code!!!");
           DT_Nfc_RamdumpPerformed = FALSE;
        }
        pthread_create( &close_thread, &attr, (void *)DT_Nfc_close_thread, (void*)pDriverConfig);
        pthread_attr_destroy(&attr);
        nfc_hal_cb.ncit_cb.hw_error = FALSE;
        /* Are we attempting to close down the current connection */
        if (!strcmp(pDriverConfig->devFile, DriverConfig.devFile)){
            DriverConfig.nRef       = 0;
            DriverConfig.phyType    = ENUM_LINK_TYPE_NONE;
        }
    }
    else
    {
        // mutex not aquired to thread is already running
        ALOGD( "DT_Nfc_Close(): already closing \n");
    }
    ALOGD ("%s: exit", __FUNCTION__);
}
/*******************************************************************************
**
** Function         DT_Nfc_close_thread
**
** Description      Close the DT reader thread
**
** Returns          None.
**
*******************************************************************************/
void DT_Nfc_close_thread(UINT32 params)
{
    tUSERIAL_PORT port = (tUSERIAL_PORT )params;
    BT_HDR                  *p_buf = NULL;

    int result;

    ALOGD( "%s: closing transport (%d)\n", __FUNCTION__, linux_cb.sock);
    pthread_mutex_lock(&close_thread_mutex);
    is_close_thread_is_waiting = FALSE;

    if (linux_cb.sock < 0)
    {
        ALOGD( "%s: already closed (%d)\n", __FUNCTION__, linux_cb.sock);
        pthread_mutex_unlock(&close_thread_mutex);
        return;
    }

    send_wakeup_signal();

    result = pthread_join( worker_thread1, NULL );
    if ( result < 0 )
        ALOGE( "%s: pthread_join() FAILED: result: %d", __FUNCTION__, result );
    else
        ALOGD( "%s: pthread_join() joined: result: %d", __FUNCTION__, result );

    result = close(linux_cb.sock);
    if (result<0)
        ALOGD("%s: close return %d", __FUNCTION__, result);

    linux_cb.sock_power_control = -1;
    linux_cb.sock = -1;
    close_signal_fds();
    pthread_mutex_unlock(&close_thread_mutex);
    ALOGD("%s: exiting", __FUNCTION__);
}
/*******************************************************************************
**
** Function           USERIAL_Feature
**
** Description        Check whether a feature of the serial API is supported.
**
** Output Parameter   None
**
** Returns            TRUE  if the feature is supported
**                    FALSE if the feature is not supported
**
*******************************************************************************/
UDRV_API BOOLEAN USERIAL_Feature(tUSERIAL_FEATURE feature)
{
    switch (feature)
    {
    case USERIAL_FEAT_PORT_1:
    case USERIAL_FEAT_PORT_2:
    case USERIAL_FEAT_PORT_3:
    case USERIAL_FEAT_PORT_4:

    case USERIAL_FEAT_BAUD_600:
    case USERIAL_FEAT_BAUD_1200:
    case USERIAL_FEAT_BAUD_9600:
    case USERIAL_FEAT_BAUD_19200:
    case USERIAL_FEAT_BAUD_57600:
    case USERIAL_FEAT_BAUD_115200:

    case USERIAL_FEAT_STOPBITS_1:
    case USERIAL_FEAT_STOPBITS_2:

    case USERIAL_FEAT_PARITY_NONE:
    case USERIAL_FEAT_PARITY_EVEN:
    case USERIAL_FEAT_PARITY_ODD:

    case USERIAL_FEAT_DATABITS_5:
    case USERIAL_FEAT_DATABITS_6:
    case USERIAL_FEAT_DATABITS_7:
    case USERIAL_FEAT_DATABITS_8:

    case USERIAL_FEAT_FC_HW:
    case USERIAL_FEAT_BUF_BYTE:

    case USERIAL_FEAT_OP_FLUSH_RX:
    case USERIAL_FEAT_OP_FLUSH_TX:
        return TRUE;
    default:
        return FALSE;
    }

    return FALSE;
}

UDRV_API BOOLEAN USERIAL_IsClosed()
{
    return (linux_cb.sock == -1) ? TRUE : FALSE;
}
