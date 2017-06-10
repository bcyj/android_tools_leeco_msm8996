/**
  @file
  port_bridge.c

  @brief
  Main port_bridge application which bridges communication
  between an external host port and a modem port.
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

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <sys/wait.h>
#include <pthread.h>
#include <termios.h>
#include <unistd.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <net/if.h>
#include <termios.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <signal.h>

#include "port_bridge.h"

int dun_ctrl_pipefds[2];

char *DUN_EVENT_STR[] = {
  "DUN_EVENT_ERROR",
  "DUN_EVENT_USB_UNPLUG",
  "DUN_EVENT_USB_PLUG",
  "DUN_EVENT_START",
  "DUN_EVENT_STOP"
};

char *DUN_STATE_STR[] = {
  "DUN_STATE_ERROR",
  "DUN_STATE_USB_UNPLUG",
  "DUN_STATE_IDLE",
  "DUN_STATE_CONNECTED"
};

dun_state_t       dun_state;
dun_portparams_t  dun_portparams;
pthread_t         kevent_thread;
pthread_mutex_t   dun_post_event_mutex;

/*===========================================================================
  FUNCTION  dun_post_event
============================================================================*/
/*!
@brief
  Posts event to the DUN state machine

@param
  event  The event to be posted

@return
  None
*/
/*===========================================================================*/
void dun_post_event(dun_event_t event)
{
   dun_event_msg_t msg;

   pthread_mutex_lock (&dun_post_event_mutex);
   msg.event = event;
   msg.unused = NULL;
   LOGI("dun_post_event: Writing message to control pipe");
   write(dun_ctrl_pipefds[1], &msg, sizeof(msg));
   pthread_mutex_unlock (&dun_post_event_mutex);

   return;
}

/*===========================================================================
  FUNCTION  dun_process_state_usb_unplug
============================================================================*/
/*!
@brief
  Process events when port bridge is in USB UNPLUG state

@param
  msg   DUN event

@return
  None
*/
/*===========================================================================*/
static void dun_process_state_usb_unplug(dun_event_msg_t msg)
{
    if (msg.event < DUN_EVENT_MAX
        && dun_state < DUN_STATE_MAX)
    {
      LOGI("dun_process_state_usb_unplug: Received event(%s) in state(%s)\n",
             DUN_EVENT_STR[msg.event+1], DUN_STATE_STR[dun_state+1]);
    }

    switch(msg.event)
    {
       case DUN_EVENT_USB_PLUG:
            dun_state = DUN_STATE_IDLE;
            dun_start_ports_threads(&dun_portparams);
            if (dun_state < DUN_STATE_MAX)
            {
              LOGI("dun_process_state_usb_unplug: Moved to state(%s)\n",
                   DUN_STATE_STR[dun_state+1]);
            }
            break;

       case DUN_EVENT_ERROR:
       case DUN_EVENT_STOP:
       case DUN_EVENT_START:
       case DUN_EVENT_USB_UNPLUG:
       default:
            break;
    }

    return;
}

/*===========================================================================
  FUNCTION  dun_process_state_idle
============================================================================*/
/*!
@brief
  Process events when port bridge is in IDLE state

@param
  msg   DUN event

@return
  None
*/
/*============================================================================*/
static void dun_process_state_idle(dun_event_msg_t msg)
{
  if (msg.event < DUN_EVENT_MAX
        && dun_state < DUN_STATE_MAX)
  {
    LOGI("dun_process_state_usb_idle: Received event(%s) in state(%s)\n",
          DUN_EVENT_STR[msg.event+1], DUN_STATE_STR[dun_state+1]);
  }

  switch(msg.event)
  {
   case DUN_EVENT_USB_UNPLUG:
        dun_stop_ports_threads(&dun_portparams);
        dun_state = DUN_STATE_USB_UNPLUG;
        if (dun_state < DUN_STATE_MAX)
        {
          LOGI("dun_process_state_idle: Moved to state(%s)\n",
               DUN_STATE_STR[dun_state+1]);
        }
        break;
   case DUN_EVENT_START:
        dun_state = DUN_STATE_CONNECTED;
        if (dun_state < DUN_STATE_MAX)
        {
          LOGI("dun_process_state_idle: Moved to state(%s)\n",
               DUN_STATE_STR[dun_state+1]);
        }
        break;
   case DUN_EVENT_ERROR:
        /* There was a fatal error which is non-recoverable, exit the process */
        LOGI("dun_process_state_idle: Fatal error! Exiting...");
        exit(EXIT_FAILURE);
        break;
   case DUN_EVENT_STOP:
   case DUN_EVENT_USB_PLUG:
   default:
        break;
  }

  return;
}

/*===========================================================================
  FUNCTION  dun_process_state_connected
============================================================================*/
/*!
@brief
  Process events when port bridge is in CONNECTED state

@param
  msg   DUN event

@return
  None
*/
/*============================================================================*/
static void dun_process_state_connected(dun_event_msg_t msg)
{
  if (msg.event < DUN_EVENT_MAX
      && dun_state < DUN_STATE_MAX)
  {
    LOGI("dun_process_state_usb_connected: Received event(%s) in state(%s)\n",
           DUN_EVENT_STR[msg.event+1], DUN_STATE_STR[dun_state+1]);
  }

  switch(msg.event)
  {
    case DUN_EVENT_USB_UNPLUG:
    case DUN_EVENT_STOP:
         if (DUN_STATE_USB_UNPLUG != dun_state)
         {
           dun_state = DUN_STATE_USB_UNPLUG;
           dun_stop_ports_threads(&dun_portparams);
           if (dun_state < DUN_STATE_MAX)
           {
             LOGI("dun_process_state_connected: Moved to state(%s)\n",
                  DUN_STATE_STR[dun_state+1]);
           }
         }
         else
         {
           LOGI("dun_process_state_connected: Already in UNPLUG state, ignoring this event");
         }
         break;
    case DUN_EVENT_ERROR:
    case DUN_EVENT_START:
    case DUN_EVENT_USB_PLUG:
    default:
         break;
  }
  if (dun_state < DUN_STATE_MAX)
  {
    LOGI("Moved to state(%s)\n", DUN_STATE_STR[dun_state + 1]);
  }
  else
  {
    LOGI("Invalid state (%d)\n", dun_state);
  }
  return;
}

/*===========================================================================
  FUNCTION  dun_process_event
============================================================================*/
/*!
@brief
  Call appropriate DUN event handlers based on the DUN state

@param
  msg   DUN event

@return
  None
*/
/*===========================================================================*/
static void dun_process_event(dun_event_msg_t msg)
{
  switch(dun_state) {
    case DUN_STATE_USB_UNPLUG:
         dun_process_state_usb_unplug(msg);
         break;

    case DUN_STATE_IDLE:
         dun_process_state_idle(msg);
         break;

    case DUN_STATE_CONNECTED:
         dun_process_state_connected(msg);
         break;

    default:
         break;
  }
  return;
}

/*===========================================================================
  FUNCTION  dun_cleanup
============================================================================*/
/*!
@brief
  Performs cleanup of resources

@return
  None
*/
/*===========================================================================*/
void dun_cleanup(void)
{
  LOGI("dun_cleanup: Port bridge is closing, cleaning up resources...");
  /* Close the control pipe */
  LOGI("dun_cleanup: Closing control pipe...");
  if (close(dun_ctrl_pipefds[0]) < 0)
  {
    LOGE("dun_cleanup: Error closing read ctrl pipe [%s]", strerror(errno));
  }

  if (close(dun_ctrl_pipefds[1]) < 0)
  {
    LOGE("dun_cleanup: Error closing write ctrl pipe [%s]", strerror(errno));
  }

  /* Destroy the mutex */
  pthread_mutex_destroy(&dun_post_event_mutex);
}

/*===========================================================================
  FUNCTION  main
============================================================================*/
/*!
@brief
  Entry point of port bridge application
*/
/*===========================================================================*/
int main(int argc, char *argv[])
{
  dun_event_msg_t event_msg;

  if(argc < 3)
  {
    LOGE("Usage: %s <SMD port dev node> <USB serial port dev node>\n",
         argv[0]);
    return DUN_ERROR;
  }

  atexit(dun_cleanup);

  memset(dun_portparams.modem, 0, sizeof(dun_portparams.modem));
  memset(dun_portparams.host, 0, sizeof(dun_portparams.host));
  strlcpy(dun_portparams.modem, argv[1], sizeof(dun_portparams.modem));
  strlcpy(dun_portparams.host, argv[2], sizeof(dun_portparams.host));

  /* create pipe for IPC */
  if(0 > pipe(dun_ctrl_pipefds))
  {
    LOGE("Error while creating pipe for IPC : %s \n", strerror(errno));
    return DUN_ERROR;
  }

  /* Initialize events processing mutex */
  if (0 != pthread_mutex_init(&dun_post_event_mutex, NULL))
  {
    LOGE("Unable to create event mutex : %s\n", strerror(errno));
    return DUN_ERROR;
  }

  /* create kernel events  thread */
  if( pthread_create(&kevent_thread, NULL, dun_monitor_usbkevents,
                     (void *)NULL) < 0)
  {
    LOGE("Unable to create usb event thread : %s\n", strerror(errno));
    return DUN_ERROR;
  }

  /* initial DUN system state */
  dun_state = DUN_STATE_USB_UNPLUG;

  while(1)
  {
    /* read messages from pipe */
    LOGI("Read message from control pipe\n");
    if(read(dun_ctrl_pipefds[0], &event_msg, sizeof(event_msg))
       != sizeof(event_msg))
    {
      LOGE("Error while reading the message from pipe : %s\n",
           strerror(errno));
      /* Retry after a minute */
      usleep(SLEEP_INTERVAL);
      continue;
    }

    if(event_msg.event > DUN_EVENT_MAX)
    {
     LOGE("Error: event_msg.event does not exist : %s\n",
          strerror(errno));
     usleep(SLEEP_INTERVAL);
     continue;
   }

   dun_process_event(event_msg);
  }

  pthread_mutex_destroy(&dun_post_event_mutex);

  return DUN_SUCCESS;
}
