/* ========================================================================= *
   Purpose:  Shared object library used for fuzzing Camera HAL layer APIs

           -------------------------------------------------------
        Copyright © 2012 Qualcomm Technologies, Inc. All Rights Reserved.
               Qualcomm Technologies Proprietary and Confidential.
* ========================================================================= */
#ifndef ANDROID_HARDWARE_Q_CAMERA_H
#define ANDROID_HARDWARE_Q_CAMERA_H

#if 1
#define	TRUE	(1)
#define	FALSE	(!TRUE)

#include "camera.h"
#include <utils/Timers.h>

#ifdef HAVE_ANDROID_OS
#include <linux/android_pmem.h>
#endif
#else
#include "QualcommCamera.h"
#endif
  int Fuzzer_HAL_init();

  int Fuzzer_HAL_deinit();

  int camera_device_open(const char *id);

  int close_camera_device();

  int start_preview();

  int stop_preview();

  int set_callbacks(int cam_id);

  int set_preview_window();

  int enable_msg_type(int msg_type);

  int disable_msg_type(int msg_type);

  int msg_type_enabled(int msg_type);

  int preview_enabled();

  int store_meta_data_in_buffers(int enable);

  int start_recording();

  int stop_recording();

  int recording_enabled();

  int release_recording_frame(const void *opaque);

  int auto_focus();

  int cancel_auto_focus();

  int take_picture();

  int cancel_picture();

  int set_parameters(const char *parms, int use_default);

  char* get_parameters();

  int put_parameters(char *);

  int send_command(int cmd, int arg1, int arg2);

  int release();

#endif

