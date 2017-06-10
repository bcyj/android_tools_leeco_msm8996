
/* sensor.c
 *
 * Copyright (c) 2012-2013 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#include <stdio.h>
#include <dlfcn.h>
#include <asm-generic/errno-base.h>
#include <poll.h>
#include "sensor.h"
#include "mct_pipeline.h"
#include "sensor_thread.h"
#include "camera_dbg.h"
#include <sys/syscall.h>
#include <sys/prctl.h>
#include "server_debug.h"

static long long sensor_current_timestamp(void)
{
  struct    timeval te;
  long long milliseconds;
  gettimeofday(&te, NULL);
  /* Calculate milliseconds */
  milliseconds = te.tv_sec*1000LL + te.tv_usec/1000;

  return milliseconds;
}

boolean cancel_autofocus = FALSE;
/** sensor_cancel_autofocus:
 *
 *  Return:
 *
 *  This function cancels the autofocus polling loop **/
void sensor_cancel_autofocus_loop()
{
   cancel_autofocus = TRUE;
}
/** sensor_process_thread_message:
 *
 *  Return:
 *
 *  This function processes the thread message **/
void sensor_process_thread_message(sensor_thread_msg_t *msg)
{
  mct_bus_msg_t bus_msg;
  mct_bus_msg_af_status_t af_msg;
  enum sensor_af_t status;
  int i=0,ret = -1;
  cam_focus_distances_info_t fdistance;

  SHIGH("Processing Pipe message %d", msg->msgtype);
  long long start = sensor_current_timestamp();

  switch(msg->msgtype){
  case SET_AUTOFOCUS:
        status = SENSOR_AF_NOT_FOCUSSED;
        while  ( i<20 ) {
        ret = ioctl(msg->fd, VIDIOC_MSM_SENSOR_GET_AF_STATUS, &status);
        if (ret < 0) {
         SERR("failed");
        }
        if(status ==  SENSOR_AF_FOCUSSED)
        break;
        if(cancel_autofocus) {
        cancel_autofocus = FALSE;
        break;
        }
        usleep(10000);
        i++;
       }
        // Send the AF call back
      switch (status) {
            case SENSOR_AF_FOCUSSED:
                 af_msg.focus_state = CAM_AF_FOCUSED;
                 break;
            default:
                 af_msg.focus_state = CAM_AF_NOT_FOCUSED;
             break;
        }
       af_msg.f_distance = fdistance;
       bus_msg.type = MCT_BUS_MSG_SENSOR_AF_STATUS;
       bus_msg.msg = (void *)&af_msg;
       bus_msg.sessionid = msg->sessionid;
       mct_module_post_bus_msg(msg->module,&bus_msg);
       cancel_autofocus = FALSE;
       SHIGH("%s: Setting Auto Focus message received",__func__);
       break;

  case OFFLOAD_FUNC: {
    if (msg->offload_func) {
      msg->offload_func(msg->param1, msg->param2, msg->param3, msg->param4);
    } else {
      SERR("msg->offload_func is null");
    }
  }
       break;

  default:
    break;
  }

  long long end = sensor_current_timestamp();
  SHIGH("%s: move lens took %llu ms", __func__, end - start);
}
/** sensor_thread_func: sensor_thread_func
 *
 *  Return:
 *
 *  This is the main thread function **/

void* sensor_thread_func(void *data)
{
   sensor_thread_t *thread = (sensor_thread_t*)data;
   int readfd, writefd;
   pthread_mutex_lock(&thread->mutex);
   thread->is_thread_started = TRUE;
   readfd = thread->readfd;
   writefd = thread->writefd;
   pthread_cond_signal(&thread->cond);
   pthread_mutex_unlock(&thread->mutex);
   struct pollfd pollfds;
   int num_of_fds = 1;
   boolean thread_exit = FALSE;
   int ready = 0;
   pollfds.fd = readfd;
   pollfds.events = POLLIN | POLLPRI;
   cancel_autofocus = FALSE;

   SHIGH("%s thread_id is %d\n",__func__, syscall(SYS_gettid));
   prctl(PR_SET_NAME, "sensor_thread", 0, 0, 0);
   while(!thread_exit){
     ready = poll(&pollfds, (nfds_t)num_of_fds, -1);
      if(ready > 0)
          {
             ALOGE("Got some events");
             if(pollfds.revents & (POLLIN | POLLPRI)){
             int nread = 0;
             sensor_thread_msg_t msg;
             nread = read(pollfds.fd, &msg, sizeof(sensor_thread_msg_t));
                if(nread < 0) {
                  SERR("%s: Unable to read the message", __func__);
                }
             if(msg.stop_thread) {
             break;
             }
             sensor_process_thread_message(&msg);
             }
          }
      else{
          SERR("%s: Unable to ple exiting the thread",__func__);
          break;
      }

   }
   SHIGH("%s:Sensor thread is exiting",__func__);
   close(readfd);
   close(writefd);
   pthread_exit(0);
   return NULL;
}
/** sensor_thread_create: sensor_thread_create
 *
 *  Return:
 *
 *  This function creates sensor thread **/
int32_t sensor_thread_create(mct_module_t *module)
{
  int ret = 0;
  sensor_thread_t thread;
  pthread_attr_t attr;
  module_sensor_ctrl_t *ctrl =(module_sensor_ctrl_t*)module->module_private;
  if(pipe(ctrl->pfd) < 0) {
     SERR("%s: Error in creating the pipe",__func__);
  }
  if ((ctrl->pfd[0]) >= MAX_FD_PER_PROCESS) {
    dump_list_of_daemon_fd();
    ctrl->pfd[0] = -1;
  }

   pthread_attr_init(&attr);
   pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
   pthread_mutex_init(&thread.mutex, NULL);
   pthread_cond_init(&thread.cond, NULL);
   thread.is_thread_started = FALSE;
   thread.readfd = ctrl->pfd[0];
   thread.writefd = ctrl->pfd[1];
   ret = pthread_create(&thread.td, &attr, sensor_thread_func, &thread );
   if(0 != ret) {
     SERR("%s: Failed to create af_status thread, ret = 0x%x",__func__, ret);
     return ret;
   }
   pthread_setname_np(thread.td, "CAM_sensor");
   pthread_mutex_lock(&thread.mutex);
    while(thread.is_thread_started == FALSE) {
       pthread_cond_wait(&thread.cond, &thread.mutex);
    }
   pthread_mutex_unlock(&thread.mutex);
   return ret;
}

