/*
  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
*/

#ifndef __C2D_H__
#define __C2D_H__

#include <pthread.h>
#include <semaphore.h>
#include <media/msmb_pproc.h>
#include "c2d2.h"
#include "c2dExt.h"
#include "c2d_interface.h"
//#include "mtype.h"

#define PPROC_SUCCESS 0
#define PPROC_FAILURE -1

//#define MAX_PLANES 3

typedef struct {
  void *ptr;
  C2D_STATUS (*c2dCreateSurface)(uint32_t *surface_id, uint32_t surface_bits,
    C2D_SURFACE_TYPE surface_type, void *surface_definition);
  C2D_STATUS (*c2dUpdateSurface)(uint32_t surface_id, uint32_t surface_bits,
    C2D_SURFACE_TYPE surface_type, void *surface_definition);
  C2D_STATUS (*c2dDraw)(uint32_t target_id, uint32_t target_config,
    C2D_RECT *target_scissor, uint32_t target_mask_id,
    uint32_t target_color_key, C2D_OBJECT *objects_list, uint32_t num_objects);
  C2D_STATUS (*c2dLensCorrection)(uint32_t targetSurface,
    C2D_LENSCORRECT_OBJECT *sourceObject);
  C2D_STATUS (*c2dFinish)(uint32_t target_id);
  C2D_STATUS (*c2dDestroySurface)(uint32_t surface_id);
  C2D_STATUS (*c2dMapAddr)(int mem_fd, void *hostptr, uint32_t len,
    uint32_t offset, uint32_t flags, void **gpuaddr);
  C2D_STATUS (*c2dUnMapAddr)(void *gpuaddr);
  /* c2dGetDriverCapabilities is not present in A family */
  //C2D_STATUS (*c2dGetDriverCapabilities)(C2D_DRIVER_INFO * driver_info);
} c2d_lib_t;

typedef struct {
  uint32_t target_id;
  uint32_t target_config;
  C2D_RECT *target_scissor;
  uint32_t target_mask_id;
  uint32_t target_color_key;
  C2D_OBJECT draw_obj;
} c2d_draw_params_t;

typedef struct {
  uint32_t target_id;
  C2D_LENSCORRECT_OBJECT lenscorrect_obj;

}c2d_lens_correction_params_t;

typedef struct _c2d_queue_t c2d_queue_t;

typedef struct _c2d_queue_t {
  void             *data;
  c2d_queue_t      *next;
} c2d_queue_t;

typedef struct _c2d_module_ctrl_t {
  c2d_lib_t c2d_lib;
  c2d_libparams c2d_input_lib_params;
  c2d_libparams c2d_output_lib_params;
  c2d_draw_params_t draw_params;
  c2d_lens_correction_params_t LC_params;
  /* Thread id of CPP / C2D thread */
  pthread_mutex_t   mutex;
  /* Queue size */
  uint32_t          queue_size;
  c2d_queue_t      *c2d_queue;
  uint32_t          thread_exit;
} c2d_module_ctrl_t;

#endif
