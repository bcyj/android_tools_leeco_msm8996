/*============================================================================

   Copyright (c) 2010-2011 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/ioctl.h>
struct file;
struct inode;
#include <linux/android_pmem.h>
#include <sys/mman.h>
#include <errno.h>
#include <sys/time.h>
#include <string.h>
#include "camera.h"
#include "camera_dbg.h"
#include "mm_camera_interface.h"

/* Stats variables */
static int cam_stats_fd[2];
static pthread_t camstats_thread_id = -1;
static int is_camstats_thread_ready;
static int camstats_exit;
static int camstats_thread_running;
pthread_mutex_t camstats_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  camstats_cond  = PTHREAD_COND_INITIALIZER;

/* Static functions */
static void* start_camstats_thread(void*);

/*===========================================================================
FUNCTION      is_camstats_thread_running

DESCRIPTION   Function for checking whether the stats thread is running
===========================================================================*/
int is_camstats_thread_running(void)
{
  return camstats_thread_running;
}

/*===========================================================================
FUNCTION      launch_camstats_thread

DESCRIPTION   Cam stats thread create function
===========================================================================*/
int launch_camstats_thread(void)
{
  CDBG("launch_camstats_thread(), start %d\n", camstats_exit);

  pthread_mutex_lock(&camstats_mutex);
  if (camstats_thread_running) {
     pthread_mutex_unlock(&camstats_mutex);
     CDBG("launch_camstats_thread thread exit failure!\n");
     return FALSE;
  }

  is_camstats_thread_ready = 0;
  camstats_exit = 0;
  camstats_thread_id = -1;
  pthread_mutex_unlock(&camstats_mutex);

  if (pthread_create(&camstats_thread_id, NULL,
    start_camstats_thread, NULL) != 0) {
    CDBG("launch_camstats_thread(), create thread failed\n");
    return FALSE;
  }

  /* Waiting for launching sub thread ready signal. */
  CDBG("launch_camstats_thread(), call pthread_cond_wait %d\n",
    (int32_t)camstats_thread_id);

  pthread_mutex_lock(&camstats_mutex);
  camstats_thread_running = TRUE;
  if (!is_camstats_thread_ready) {
    pthread_cond_wait(&camstats_cond, &camstats_mutex);
  }
  pthread_mutex_unlock(&camstats_mutex);

  CDBG("launch_camstats_thread(), call pthread_cond_wait done\n");
  return TRUE;
}

/*===========================================================================
FUNCTION      release_camstats_thread

DESCRIPTION   Cam stats thread exit function
===========================================================================*/
void release_camstats_thread(void)
{
  camstats_msg msg;
  msg.msg_type = CAM_STATS_MSG_EXIT;
  CDBG("release_camstats_thread enter!\n");
  pthread_mutex_lock(&camstats_mutex);
  if (!camstats_thread_running) {
    pthread_mutex_unlock(&camstats_mutex);
    CDBG("release_camstats_thread thread not running \n");
    return;
  }
  camstats_exit = 1;
  pthread_mutex_unlock(&camstats_mutex);

  int rc = write(cam_stats_fd[1], &msg, sizeof(msg));
  if (rc < 0)
    CDBG("release_camstats_thread write Failed\n");

  close(cam_stats_fd[0]);
  close(cam_stats_fd[1]);
  pthread_join(camstats_thread_id, NULL);
  pthread_mutex_lock(&camstats_mutex);
  camstats_thread_running = FALSE;
  pthread_mutex_unlock(&camstats_mutex);
  CDBG("release_camstats_thread exit!\n");
}

/*===========================================================================
FUNCTION      send_camstats

DESCRIPTION   Generic function for sending stats message to stats thread
===========================================================================*/
int8_t send_camstats(camstats_type msg_type, void* data, int size)
{
  CDBG("send_camstats %d\n", msg_type);
  camstats_msg msg;
  switch(msg_type) {
    case CAM_STATS_TYPE_HIST: {
      CDBG("send_camstats hist buffer %p size %d\n",
        msg.msg_data.hist_data.buffer, size);
      memcpy((void *)msg.msg_data.hist_data.buffer, data, size );
        msg.msg_type = CAM_STATS_MSG_HIST;
      break;
    }
    default: {
      CDBG("send_camstats invalid message\n");
      return TRUE;
    }
  }

  int rc = write(cam_stats_fd[1], &msg, sizeof(msg));
  if(rc < 0) {
    CDBG("send_camstats write Failed\n");
    return FALSE;
  }
  return TRUE;
}

/*===========================================================================
FUNCTION      send_camstats

DESCRIPTION   Generic function for sending stats message to stats thread
===========================================================================*/
int8_t send_camstats_msg(camstats_type stats_type, camstats_msg* p_msg)
{
  CDBG("send_camstats %d\n", stats_type);
  switch(stats_type) {
    case CAM_STATS_TYPE_HIST: {
      CDBG("send_camstats hist p_msg %p \n", p_msg);
      p_msg->msg_type = CAM_STATS_MSG_HIST;
      break;
    }
    default:
      CDBG("send_camstats invalid message\n");
      return TRUE;
  }

  int rc = write(cam_stats_fd[1], p_msg, sizeof(camstats_msg));
  if (rc < 0) {
    CDBG("send_camstats write Failed\n");
    return FALSE;
  }
  return TRUE;
}

/*===========================================================================
FUNCTION      start_camstats_thread

DESCRIPTION   Handler function for stats messaged from config thread
===========================================================================*/
void* start_camstats_thread(void* arg)
{
  fd_set fds;
  int len = 0;
  camstats_msg msg;

  if (pipe(cam_stats_fd) < 0) {
    CDBG("start_camstats_thread : pipe creation failed\n");
    return NULL;
  }
  CDBG("start_camstats_thread : pipe cam_stats_fd %d %d\n", cam_stats_fd[0],
    cam_stats_fd[1]);

  CDBG("start_camstats_thread(), signal the main thread\n");
  pthread_mutex_lock(&camstats_mutex);
  is_camstats_thread_ready = 1;
  pthread_cond_signal(&camstats_cond);
  pthread_mutex_unlock(&camstats_mutex);

  do {
    CDBG("start_camstats_thread(), start read %d\n", sizeof(msg));
    len = read(cam_stats_fd[0], &msg, sizeof(msg));
    //CDBG("start_camstats_thread(), read len %d %p\n", len, msg.msg_data );

    pthread_mutex_lock(&camstats_mutex);
    if (camstats_exit) {
      pthread_mutex_unlock(&camstats_mutex);
      break;
    }
    pthread_mutex_unlock(&camstats_mutex);

   if (len != sizeof(msg)) {
     CDBG("start_camstats_thread improper data received");
     continue;
   }

    CDBG("start_camstats_thread(), msg_type %d\n", msg.msg_type);
    switch(msg.msg_type) {
      case CAM_STATS_MSG_HIST: {
        CDBG("start_camstats_thread CAM_STATS_MSG_HIST %p",
          msg.msg_data.hist_data.buffer);
        if (get_notify_obj()->camstats_cb) {
          get_notify_obj()->camstats_cb(CAM_STATS_TYPE_HIST,
            &(msg.msg_data.hist_data));
        }
      }
      break;
      case CAM_STATS_MSG_EXIT: {
        camstats_exit = 1;
      }
      break;
    }
  } while (!camstats_exit);
  pthread_mutex_lock(&camstats_mutex);
  camstats_thread_id = -1;
  pthread_mutex_unlock(&camstats_mutex);
  CDBG("start_camstats_thread(), Exit\n");
  return NULL;
}
