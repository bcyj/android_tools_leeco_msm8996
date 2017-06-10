/**
  @file
  dun_kevents.c

  @brief
  Monitor kernel events for getting USB state
*/

/*===========================================================================

  Copyright (c) 2011-2014 Qualcomm Technologies, Inc. All Rights Reserved

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

#include <sys/socket.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <net/if.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <sys/wait.h>
#include <pthread.h>
#include <termios.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <signal.h>

#include "port_bridge.h"

#define DUN_USB_STATE_FILE                   "/sys/class/android_usb/android0/state"
#define DUN_USB_FUNCTION_FILE                "/sys/class/android_usb/android0/functions"
#define DUN_USB_MODE_SERIAL                  "serial"

#define DUN_USB_ACTION                       "ACTION="
#define DUN_USB_ACTION_LEN                   (sizeof(DUN_USB_ACTION) - 1)
#define DUN_USB_ACTION_ADD                   "add"
#define DUN_USB_ACTION_REMOVE                "remove"
#define DUN_USB_ACTION_CHANGE                "change"

#define DUN_USB_SUBSYSTEM                    "SUBSYSTEM="
#define DUN_USB_SUBSYSTEM_LEN                (sizeof(DUN_USB_SUBSYSTEM) - 1)
#define DUN_USB_SUBSYSTEM_VAL                "android_usb"

#define DUN_USB_STATE                        "USB_STATE="
#define DUN_USB_STATE_LEN                    (sizeof(DUN_USB_STATE) - 1)
#define DUN_USB_STATE_CONNECTED              "CONNECTED"
#define DUN_USB_STATE_DISCONNECTED           "DISCONNECTED"
#define DUN_USB_STATE_CONFIGURED             "CONFIGURED"

#define DUN_MAX_STR_LEN                      (100)
#define DUN_MAX_UEVENT_BUFSIZE               (1024)
#define DUN_INVALID_SOCK_FD                  (-1)

static int  uevent_netlink_sock = DUN_INVALID_SOCK_FD;
static char *uevent_buf         = NULL;

/*===========================================================================
  FUNCTION  dun_get_usbserial_mode
============================================================================*/
/*!
@brief
  Reads sysfs file to determine if USB supports serial function

@param
  usb_dev   Will be filled with USB state information

@return
  None
*/
/*===========================================================================*/
static void dun_get_usbserial_mode(dun_usb_t *usb_dev)
{
  int fd = 0;
  int rc = 0;
  char buffer[DUN_MAX_STR_LEN];

  do
  {
    if (NULL == usb_dev)
    {
      LOGE("dun_get_usbserial_mode: Invalid parameters received!\n");
      break;
    }

    /* Check if USB is in serial mode from sysfs */
    fd = open(DUN_USB_FUNCTION_FILE, O_RDONLY);
    if (fd < 0)
    {
      LOGE("dun_get_usbserial_mode: Error reading USB function file [%s]\n",
          strerror(errno));
      usb_dev->usb_mode = DUN_USBMODE_NOT_SERIAL;
      break;
    }

    memset(buffer, 0, DUN_MAX_STR_LEN);

    if ( ( rc = (int)read(fd, buffer, sizeof(buffer)) ) < 0 )
    {
      LOGE("dun_get_usbserial_mode: Error reading from USB function file [%s]\n",
           strerror(errno));
      usb_dev->usb_mode = DUN_USBMODE_INVALID;
      break;
    }

    LOGI("dun_get_usbserial_mode: USB function file Bytes read [%d] Function [%s]",
         rc, buffer);
    /* Adding NULL at the end of string*/
    if (rc < (DUN_MAX_STR_LEN -1))
    {
      buffer[rc] = 0;
    }
    else
    {
      buffer[DUN_MAX_STR_LEN -1] = 0;
    }
    /* Check if 'serial' is part of the function file */
    if (NULL != strstr(buffer, DUN_USB_MODE_SERIAL))
    {
      /* USB is configured in serial mode */
      usb_dev->usb_mode = DUN_USBMODE_SERIAL;
    }
    else
    {
      /* USB is not configured in serial mode */
      usb_dev->usb_mode = DUN_USBMODE_NOT_SERIAL;
    }
  } while (0);

  if (close(fd) < 0)
  {
    LOGE("dun_get_usbserial_mode: Error closing USB function file [%s]\n",
    strerror(errno));
  }
}

/*===========================================================================
  FUNCTION  dun_getusbinfo_fromsys
============================================================================*/
/*!
@brief
  Reads the sysfs file to determine the usb state and mode

@param
  usb_dev   Will be filled with USB state information

@return
  None
*/
/*===========================================================================*/
static void dun_getusbinfo_fromsys(dun_usb_t *usb_dev)
{
  int fd_state = 0;
  int rc = 0;
  char buf_state[DUN_MAX_STR_LEN];

  do
  {
    if (NULL == usb_dev)
    {
      /* Invalid argument */
      LOGE("dun_getusbinfo_fromsys: Invalid argument!\n");
      break;
    }

    fd_state = open(DUN_USB_STATE_FILE, O_RDONLY);
    if (fd_state < 0)
    {
      /* USB may not be configured yet */
      LOGE("dun_getusbinfo_fromsys: Error opening USB state file [%s]\n",
           strerror(errno));
      usb_dev->usb_state = DUN_USBSTATE_UNPLUG;
      usb_dev->usb_mode  = DUN_USBMODE_NOT_SERIAL;
      break;
    }

    memset(buf_state, 0, sizeof(buf_state));

    if ((rc = (int)read(fd_state, buf_state, sizeof(buf_state))) < 0)
    {
      LOGE("dun_getusbinfo_fromsys: Error reading from USB state file [%s]\n",
           strerror(errno));
      usb_dev->usb_state = DUN_USBSTATE_ERROR;
      usb_dev->usb_mode  = DUN_USBMODE_INVALID;
      break;
    }

    /* Adding NULL at the end of string*/
    if (rc < (DUN_MAX_STR_LEN -1))
    {
      buf_state[rc] = 0;
    }
    else
    {
      buf_state[DUN_MAX_STR_LEN -1] = 0;
    }

    LOGI("dun_getusbinfo_fromsys: USB state file Bytes read [%d] State [%s]",
         rc, buf_state);

    if (NULL != strstr(buf_state, DUN_USB_STATE_CONFIGURED))
    {
      /* USB is connected */
      LOGI("dun_getusbinfo_fromsys: USB is connected...\n");
      usb_dev->usb_state = DUN_USBSTATE_PLUG;

      /* Proceed to check if USB has serial function enabled */
      dun_get_usbserial_mode(usb_dev);
    }
    else
    {
      /* USB is not connected */
      usb_dev->usb_state = DUN_USBSTATE_UNPLUG;
      usb_dev->usb_mode  = DUN_USBMODE_NOT_SERIAL;
    }
  } while (0);

  /* Close the sysfs state and function files */
  if (close(fd_state) < 0)
  {
    LOGE("dun_getusbinfo_fromsys: Error closing USB state file [%s]\n",
         strerror(errno));
  }
}

/*===========================================================================
  FUNCTION  dun_parse_usbevent
============================================================================*/
/*!
@brief
  This function parses USB state from uevent netlink message

@param
  buf      The message received from netlink socket
  size     The length of the message read
  usb_dev  out parameter for capturing USB state

@return
  None
*/
/*===========================================================================*/
static void dun_parse_usbevent(char *buf, int size, dun_usb_t *usb_dev)
{
  char *uevent_msg_start                 = NULL;
  char *uevent_msg_end                   = NULL;
  char uevent_action[DUN_MAX_STR_LEN]    = "";
  char uevent_subsystem[DUN_MAX_STR_LEN] = "";
  char uevent_usbstate[DUN_MAX_STR_LEN]  = "";

  int fd = 0;
  char buffer[DUN_MAX_STR_LEN];

  if (NULL == usb_dev || size <= 0)
  {
    LOGE("dun_parse_usbevent: Error, invalid arguments\n");
    return;
  }

  /* Log the event */
  uevent_msg_start = buf;
  uevent_msg_end = uevent_msg_start + size;

  LOGI("dun_parse_usbevent: Uevent Message\n");
  while (uevent_msg_start < uevent_msg_end)
  {
    LOGI("dun_parse_usbevent: %s\n", uevent_msg_start);
    uevent_msg_start += (strlen(uevent_msg_start) + 1);
  }

  memset(uevent_action, 0, sizeof(uevent_action));
  memset(uevent_subsystem, 0, sizeof(uevent_subsystem));
  memset(uevent_usbstate, 0, sizeof(uevent_usbstate));

  /* Parse uevent message */
  uevent_msg_start = buf;
  uevent_msg_end = uevent_msg_start + size;

  while (uevent_msg_start < uevent_msg_end)
  {
    if ( 0 == strncasecmp(uevent_msg_start, DUN_USB_ACTION,
                          DUN_USB_ACTION_LEN) )
    {
      strlcpy(uevent_action,
              (uevent_msg_start + DUN_USB_ACTION_LEN),
              DUN_MAX_STR_LEN);
      LOGI("dun_parse_usbevent: ACTION [%s]\n", uevent_action);
    }
    else if ( 0 == strncasecmp(uevent_msg_start, DUN_USB_SUBSYSTEM,
                               DUN_USB_SUBSYSTEM_LEN) )
    {
      strlcpy(uevent_subsystem,
              (uevent_msg_start + DUN_USB_SUBSYSTEM_LEN),
              DUN_MAX_STR_LEN);
      LOGI("dun_parse_usbevent: SUBSYSTEM [%s]\n", uevent_subsystem);
    }
    else if ( 0 == strncasecmp(uevent_msg_start, DUN_USB_STATE,
                               DUN_USB_STATE_LEN) )
    {
      strlcpy(uevent_usbstate,
              (uevent_msg_start + DUN_USB_STATE_LEN),
              DUN_MAX_STR_LEN);
      LOGI("dun_parse_uevent: USB_STATE [%s]\n", uevent_usbstate);
    }

     uevent_msg_start += (strlen(uevent_msg_start) + 1);
  }

  if (0 == strncasecmp(uevent_action, DUN_USB_ACTION_ADD,
                       (strlen(DUN_USB_ACTION_ADD) + 1)))
  {
    /* A device has been added, check if it is USB */
    if (0 == strncasecmp(uevent_subsystem, DUN_USB_SUBSYSTEM_VAL,
                         (strlen(DUN_USB_SUBSYSTEM_VAL) + 1)))
    {
      /* USB has been added */
      usb_dev->usb_state = DUN_USBSTATE_PLUG;

      /* Check if USB is in serial mode from sysfs */
      dun_get_usbserial_mode(usb_dev);
    }
  }
  else if(0 == strncasecmp(uevent_action, DUN_USB_ACTION_CHANGE,
                           (strlen(DUN_USB_ACTION_CHANGE) + 1)))
  {
    LOGI("dun_parse_uevent: Action change received...");
    /* Android receives [ACTION=change] event for USB plugging and unplugging
     * USB_STATE field of the uevent will determine this.
     * If USB_STATE is DISCONNECTED, USB is unplugged
     * If USB_STATE is CONFIGURED, USB is plugged */
    if (0 == strncasecmp(uevent_subsystem, DUN_USB_SUBSYSTEM_VAL,
                         (strlen(DUN_USB_SUBSYSTEM_VAL) + 1)))
    {
      /* Check USB_STATE */
      if (0 == strncasecmp(uevent_usbstate, DUN_USB_STATE_DISCONNECTED,
                           (strlen(DUN_USB_STATE_DISCONNECTED) + 1)))
      {
        /* USB has been removed, set state as unplug */
        usb_dev->usb_state = DUN_USBSTATE_UNPLUG;
        usb_dev->usb_mode  = DUN_USBMODE_NOT_SERIAL;
      }
      else if(0 == strncasecmp(uevent_usbstate, DUN_USB_STATE_CONFIGURED,
                               (strlen(DUN_USB_STATE_CONFIGURED) + 1)))
      {
        usb_dev->usb_state = DUN_USBSTATE_PLUG;

        /* Check if USB is in serial mode from sysfs */
        dun_get_usbserial_mode(usb_dev);
      }
    }
  }
  else if (0 == strncasecmp(uevent_action, DUN_USB_ACTION_REMOVE,
                            (strlen(DUN_USB_ACTION_REMOVE) + 1)))
  {
    /* A device has been removed, check if it is USB */
    if (0 == strncasecmp(uevent_subsystem, DUN_USB_SUBSYSTEM_VAL,
                         (strlen(DUN_USB_SUBSYSTEM_VAL) + 1)))
    {
      /* USB has been removed, set state as unplug */
      usb_dev->usb_state = DUN_USBSTATE_UNPLUG;
      usb_dev->usb_mode  = DUN_USBMODE_NOT_SERIAL;
    }
  }
}

/*===========================================================================
  FUNCTION  dun_usb_thread_exit_handler
============================================================================*/
/*!
@brief
  Exit handler for USB event listener thread

@return
  None
*/
/*===========================================================================*/
static void dun_usb_thread_exit_handler(int sig)
{
  LOGD("dun_usb_thread_exit_handler: Received signal in USB listener thread %d",
       sig);

  /* Close the netlink socket */
  if (close(uevent_netlink_sock) < 0)
  {
    LOGE("dun_usb_thread_exit_handler: Error closing uevent netlink socket [%s]",
         strerror(errno));
  }

  /* Free uevent buffer memory */
  free(uevent_buf);

  /* Exit the thread */
  pthread_exit(NULL);
}

/*===========================================================================
  FUNCTION  dun_monitor_usbkevents
============================================================================*/
/*!
@brief
  Thread to monitor kernel events to detect USB plug/unplug events

@param
  arg  Argument passed as part of thread creation

@return
  None
*/
/*===========================================================================*/
void *dun_monitor_usbkevents(void *arg)
{
  dun_usb_t             usb_dev;
  dun_usb_t             prev_usb_dev;
  int                   cnt;
  struct sockaddr_nl    socknladdr;
  struct sockaddr_nl* __attribute__((__may_alias__)) socknladdr_ptr;
  struct msghdr         msg;
  struct iovec          iov;
  struct sigaction actions;

  (void) arg; /* To suppress compiler warning on unused variable */
  do
  {
    /* Register signal handler */
    memset(&actions, 0, sizeof(actions));
    sigemptyset(&actions.sa_mask);
    actions.sa_flags = 0;
    actions.sa_handler = dun_usb_thread_exit_handler;

    if (sigaction(SIGUSR1, &actions, NULL) < 0)
    {
      LOGE("dun_monitor_usbkevents: Error in registering signal handler"
           " in USB event listener thread [%s]\n",
           strerror(errno));
      break;
    }

    /* Create USB event listener */
    memset(&socknladdr, 0, sizeof(socknladdr));
    socknladdr.nl_family = AF_NETLINK;
    socknladdr.nl_pid    = (uint32_t)getpid();
    socknladdr.nl_groups = 0xffffffff;

    if ((uevent_netlink_sock = socket(PF_NETLINK, SOCK_DGRAM,
                              NETLINK_KOBJECT_UEVENT)) < 0)
    {
      LOGE("dun_monitor_usbkevents: Unable to create uevent socket [%s]\n",
           strerror(errno));
      break;
    }

    socknladdr_ptr = &socknladdr;
    if (bind(uevent_netlink_sock, (struct sockaddr *)socknladdr_ptr,
             sizeof(socknladdr)) < 0)
    {
      LOGE("dun_monitor_usbkevents: Unable to bind uevent socket [%s]\n",
           strerror(errno));
      break;
    }

    uevent_buf = (char *) malloc(DUN_MAX_UEVENT_BUFSIZE);
    if (NULL == uevent_buf)
    {
      LOGE("dun_monitor_usbkevents: Out of memory\n");
      break;
    }

    memset(&usb_dev, 0x0, sizeof(usb_dev));
    usb_dev.usb_state = DUN_USBSTATE_UNPLUG;
    usb_dev.usb_mode = DUN_USBMODE_NOT_SERIAL;

    /* During bootup we will loose the uevent netlink message. We need to read
     * the initial USB state from sysfs files */
    dun_getusbinfo_fromsys(&usb_dev);

    if (DUN_USBSTATE_ERROR == usb_dev.usb_state)
    {
      /* USB is in an undefined state, we will assume it is unplugged */
      dun_post_event(DUN_EVENT_USB_UNPLUG);
    }
    else if (DUN_USBSTATE_PLUG == usb_dev.usb_state
             && DUN_USBMODE_SERIAL == usb_dev.usb_mode)
    {
      /* If the USB is plugged and in serial mode, post USB PLUGGED event */
      dun_post_event(DUN_EVENT_USB_PLUG);
    }
    else
    {
      /* USB is in UNPLUG state */
      dun_post_event(DUN_EVENT_USB_UNPLUG);
    }

    /* Save the current USB config */
    memcpy(&prev_usb_dev, &usb_dev, sizeof(usb_dev));

    /* Wait for netlink uevents */
    do
    {
      memset(uevent_buf, 0, DUN_MAX_UEVENT_BUFSIZE);
      cnt = (int)recv(uevent_netlink_sock, uevent_buf, DUN_MAX_UEVENT_BUFSIZE, 0);

      if (cnt < 0)
      {
        LOGE("dun_monitor_usbkevents: Error receiving uevent [%s]",
             strerror(errno));
        sleep(SLEEP_INTERVAL);
        /* Continue netlink loop */
        continue;
      }

      /* Check if uevent is from USB */
      dun_parse_usbevent(uevent_buf, cnt, &usb_dev);

      if (prev_usb_dev.usb_state != usb_dev.usb_state
          || prev_usb_dev.usb_mode != usb_dev.usb_mode)
      {
        /* Post events only if there is a change in USB configuration
         * from previous state */
        if (DUN_USBSTATE_PLUG == usb_dev.usb_state
             && DUN_USBMODE_SERIAL == usb_dev.usb_mode)
        {
          /* If the USB is plugged and in serial mode, post USB PLUGGED event */
          dun_post_event(DUN_EVENT_USB_PLUG);
        }
        else
        {
          /* USB is in UNPLUG state */
         dun_post_event(DUN_EVENT_USB_UNPLUG);
        }

        LOGI("dun_monitor_usbkevents: Posted change to port-bridge SM");

        /* Save the current USB config */
        memcpy(&prev_usb_dev, &usb_dev, sizeof(usb_dev));
      }
      else
      {
        LOGI("dun_monitor_usbkevents: No change in USB state...");
      }
    } while (1);
  } while (0);

  /* Cleanup */
  if (uevent_buf)
  {
    free(uevent_buf);
  }

  close(uevent_netlink_sock);
  return NULL;
}
