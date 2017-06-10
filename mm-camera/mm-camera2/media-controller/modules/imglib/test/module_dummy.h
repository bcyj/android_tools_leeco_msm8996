/**********************************************************************
* Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved. *
* Qualcomm Technologies Proprietary and Confidential.                 *
**********************************************************************/

#ifndef __MODULE_DUMMY_H__
#define __MODULE_DUMMY_H__

#include "img_common.h"
#include "img_comp.h"
#include "img_comp_factory.h"
#include "module_imglib_common.h"
#include "camera_dbg.h"
#include "modules.h"

#define IMGLIB_MOD_NAME "dummy"
#define MAX_NUM_FRAMES 7

#define MAX_DUMMY_STATIC_PORTS 5

typedef struct {
  pthread_mutex_t mutex;
  pthread_cond_t cond;
  pthread_t threadid;
  uint32_t identity;
  mct_frame_t frame[MAX_NUM_FRAMES];
  int is_ready;
  int status;
  mct_port_t *p_srcport;
} dummy_client_t;

typedef struct {
  int client_cnt;
  pthread_mutex_t mutex;
  pthread_cond_t cond;
  mct_list_t *test_client;
} module_dummy_t;

#endif //__MODULE_DUMMY_H__
