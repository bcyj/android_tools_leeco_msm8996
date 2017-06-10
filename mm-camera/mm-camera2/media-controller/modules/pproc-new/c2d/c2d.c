/*
  Copyright (c) 2013 - 2015 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
*/

#include <unistd.h>
#include <stdio.h>
#include <dlfcn.h>
#include <linux/msm_kgsl.h>
#include "c2d.h"
#include "c2d_interface.h"
#include "c2d_caps.h"
#include "camera_dbg.h"

#if 0
#undef CDBG
#define CDBG LOGE
#endif

#undef C2D_COPY_FRAME
//#define C2D_COPY_FRAME
#define CEILING16(X) (((X) + 0x000F) & 0xFFF0)

static C2D_YUV_FORMAT c2d_get_frame_format(cam_format_t cam_fmt);
static int32_t c2d_destroy_default_yuv_surface(c2d_module_ctrl_t *ctrl, void *data)
{
	uint32_t id = *(uint32_t *)data;
	ctrl->c2d_lib.c2dDestroySurface(id);
	return PPROC_SUCCESS;
}

static int32_t c2d_create_default_yuv_surface(c2d_module_ctrl_t *ctrl, void *data)
{
  int32_t c2d_rc = C2D_STATUS_OK;
  c2d_libparams *c2d_lib_params = (c2d_libparams *)data;
  CDBG("%s:%d\n", __func__, __LINE__);

  /* Create Source Surface */
  //c2d_lib_params->format = c2d_get_frame_format(c2d_lib_params->format);
  c2d_lib_params->surface_def.format = c2d_get_frame_format(c2d_lib_params->format);
  c2d_lib_params->surface_def.width = 1 * 4;
  c2d_lib_params->surface_def.height = 1 * 4;
  c2d_lib_params->surface_def.plane0 = (void*)0xaaaaaaaa;
  c2d_lib_params->surface_def.phys0 = (void*)0xaaaaaaaa;
  c2d_lib_params->surface_def.stride0 = 1 * 4;
  c2d_lib_params->surface_def.plane1 = (void*)0xaaaaaaaa;
  c2d_lib_params->surface_def.phys1 = (void*)0xaaaaaaaa;
  c2d_lib_params->surface_def.stride1 = 1 * 4;
  c2d_lib_params->surface_def.plane2 = (void*)0xaaaaaaaa;
  c2d_lib_params->surface_def.phys2 = (void*)0xaaaaaaaa;
  c2d_lib_params->surface_def.stride2 = 1 * 4;

  c2d_rc = ctrl->c2d_lib.c2dCreateSurface(&c2d_lib_params->id,
    c2d_lib_params->surface_bit,
    (C2D_SURFACE_TYPE)(C2D_SURFACE_YUV_HOST | C2D_SURFACE_WITH_PHYS |
    C2D_SURFACE_WITH_PHYS_DUMMY), &c2d_lib_params->surface_def);
  if (c2d_rc != C2D_STATUS_OK) {
    CDBG_ERROR("%s:%d failed. c2d_rc = %d\n", __func__, __LINE__, c2d_rc);
    return PPROC_FAILURE;
  }

  CDBG("%s:%d\n", __func__, __LINE__);
  return PPROC_SUCCESS;
} /* c2d_create_default_yuv_surface */

static int32_t c2d_update_surface(c2d_module_ctrl_t *ctrl,
  c2d_libparams *c2d_lib_params, c2d_frame *c2d_buffer)
{
  int32_t c2d_rc = C2D_STATUS_OK;
  C2D_YUV_SURFACE_DEF *surface_def = NULL;
  c2d_gpu_buf_t *gpu_buf = NULL;
  if (!ctrl || !c2d_lib_params || !c2d_buffer) {
      CDBG_ERROR("%s:%d failed\n", __func__, __LINE__);
      return PPROC_FAILURE;
  }

  surface_def = &c2d_lib_params->surface_def;
  gpu_buf = &c2d_lib_params->gpu_buf;
  c2d_lib_params->gpu_buf.num_planes = c2d_buffer->num_planes;

  if (c2d_buffer->num_planes == 1) {
    CDBG("%s: Single Plane Surface", __func__);

    CDBG("%s:%d fd %d vaddr %ld length %d addr offset %d\n", __func__, __LINE__,
      c2d_buffer->mp[0].fd, c2d_buffer->mp[0].vaddr, c2d_buffer->mp[0].length,
      c2d_buffer->mp[0].addr_offset);
    surface_def->plane0 = (void *)c2d_buffer->mp[0].vaddr;
    CDBG("%s:%d before map gpu %x\n", __func__, __LINE__, gpu_buf->sp_gAddr);
    c2d_rc = ctrl->c2d_lib.c2dMapAddr(c2d_buffer->mp[0].fd,
      (void *)c2d_buffer->mp[0].vaddr, c2d_buffer->mp[0].length,
      c2d_buffer->mp[0].addr_offset, KGSL_USER_MEM_TYPE_ION,
      (void **)&gpu_buf->sp_gAddr);
    if (c2d_rc != C2D_STATUS_OK) {
      CDBG_ERROR("%s:%d failed c2d_rc %d\n", __func__, __LINE__, c2d_rc);
      return PPROC_FAILURE;
    }
    CDBG("%s:%d after map gpu %x\n", __func__, __LINE__, gpu_buf->sp_gAddr);
    surface_def->phys0 = (void *)gpu_buf->sp_gAddr;
    surface_def->stride0 = c2d_buffer->stride * 2;
  } else if (c2d_buffer->num_planes > 1) {
    CDBG("%s: Multi Plane Surface", __func__);

    surface_def->plane0 = (void *)(c2d_buffer->mp[0].vaddr +
      c2d_buffer->mp[0].data_offset);
    c2d_rc = ctrl->c2d_lib.c2dMapAddr(c2d_buffer->mp[0].fd,
      (void *)c2d_buffer->mp[0].vaddr, c2d_buffer->mp[0].length +
      c2d_buffer->mp[1].length, c2d_buffer->mp[0].addr_offset,
      KGSL_USER_MEM_TYPE_ION, (void **)&gpu_buf->mp_gAddr[0]);
    if (c2d_rc != C2D_STATUS_OK) {
      CDBG_ERROR("%s:%d failed c2d_rc %d\n", __func__, __LINE__, c2d_rc);
      return PPROC_FAILURE;
    }
    surface_def->phys0 = (void *)(gpu_buf->mp_gAddr[0] +
      c2d_buffer->mp[0].data_offset);
    surface_def->stride0 = c2d_buffer->stride;
    surface_def->plane1 = (void *)(c2d_buffer->mp[1].vaddr +
      c2d_buffer->mp[1].data_offset);
    gpu_buf->mp_gAddr[1] = gpu_buf->mp_gAddr[0] +
      (c2d_buffer->mp[1].vaddr - c2d_buffer->mp[0].vaddr);
    if (!gpu_buf->mp_gAddr[1]) {
      CDBG_ERROR("%s:%d failed rc\n", __func__, __LINE__);
      return PPROC_FAILURE;
    }
    surface_def->phys1 = (void *)(gpu_buf->mp_gAddr[1] +
      c2d_buffer->mp[1].data_offset);
    surface_def->stride1 = c2d_buffer->stride;

   if(c2d_buffer->num_planes == 3) {
   surface_def->plane2 = (void *)(c2d_buffer->mp[2].vaddr +
      c2d_buffer->mp[2].data_offset);
   gpu_buf->mp_gAddr[2] = gpu_buf->mp_gAddr[1] +
     (c2d_buffer->mp[2].vaddr - c2d_buffer->mp[1].vaddr);

     if (!gpu_buf->mp_gAddr[2]) {
      CDBG_ERROR("%s:%d failed rc\n", __func__, __LINE__);
      return PPROC_FAILURE;
    }
    surface_def->phys2 = (void *)(gpu_buf->mp_gAddr[2] +
      c2d_buffer->mp[2].data_offset);
    surface_def->stride2 = CEILING16(c2d_buffer->stride/2);
    surface_def->stride1 = CEILING16(c2d_buffer->stride/2);
   }

  }
    else {
    CDBG_ERROR("%s: Invalid number of planes %d", __func__,
      c2d_buffer->num_planes);
    return PPROC_FAILURE;
  }

  CDBG("%s: surface_id: %d surface_type: %d\n", __func__, c2d_lib_params->id,
    c2d_lib_params->surface_type);
  CDBG("%s: plane0: 0x%x phys0: 0x%x\n", __func__,
    (uint32_t)surface_def->plane0, (uint32_t)surface_def->phys0);
  CDBG("%s: plane1: 0x%x phys1: 0x%x\n", __func__,
    (uint32_t)surface_def->plane1, (uint32_t)surface_def->phys1);
  CDBG("%s: plane2: 0x%x phys2: 0x%x\n", __func__,
    (uint32_t)surface_def->plane2, (uint32_t)surface_def->phys2);
  CDBG("%s: stride0: %d stride1: %d stride2: %d\n", __func__,
    surface_def->stride0, surface_def->stride1, surface_def->stride2);

  c2d_rc = ctrl->c2d_lib.c2dUpdateSurface(c2d_lib_params->id,
    c2d_lib_params->surface_type,
    (C2D_SURFACE_TYPE)(C2D_SURFACE_YUV_HOST | C2D_SURFACE_WITH_PHYS),
    (void *)surface_def);

  if (c2d_rc != C2D_STATUS_OK) {
    CDBG_ERROR("%s: c2dUpdateSurface failed. c2d_rc = %d\n", __func__, c2d_rc);
    return PPROC_FAILURE;
  }

  return PPROC_SUCCESS;
} /* c2d_update_surface */

#ifdef C2D_COPY_FRAME
static int32_t c2d_copy_frame(c2d_process_frame_buffer *c2d_process_buffer)
{
  int32_t rc = PPROC_SUCCESS;

  CDBG("%s:%d Enter\n", __func__, __LINE__);
  if (!c2d_process_buffer) {
    CDBG_ERROR("%s:%d failed", __func__, __LINE__);
    return PPROC_FAILURE;
  }

  /* Dummy copy */
  memcpy((void *)c2d_process_buffer->c2d_output_buffer->mp[0].vaddr,
    (void *)c2d_process_buffer->c2d_input_buffer->mp[0].vaddr,
    c2d_process_buffer->c2d_output_buffer->ion_alloc[0].len);

  return rc;
}
#endif

static C2D_YUV_FORMAT c2d_get_frame_format(cam_format_t cam_fmt)
{
  switch (cam_fmt) {
    case CAM_FORMAT_YUV_420_NV12:
    case CAM_FORMAT_YUV_420_NV12_VENUS:
      return C2D_COLOR_FORMAT_420_NV12;
    case CAM_FORMAT_YUV_420_NV21:
      return C2D_COLOR_FORMAT_420_NV21;
    case CAM_FORMAT_YUV_420_YV12:
      return C2D_COLOR_FORMAT_420_YV12;
    case CAM_FORMAT_YUV_422_NV16:
      return C2D_COLOR_FORMAT_422_UYVY;
    case CAM_FORMAT_YUV_422_NV61:
      return C2D_COLOR_FORMAT_422_UYVY;
    case CAM_FORMAT_YUV_RAW_8BIT_YUYV:
      return C2D_COLOR_FORMAT_422_YUYV;
    case CAM_FORMAT_YUV_RAW_8BIT_YVYU:
      return C2D_COLOR_FORMAT_422_YVYU;
    case CAM_FORMAT_YUV_RAW_8BIT_UYVY:
      return C2D_COLOR_FORMAT_422_UYVY;
    case CAM_FORMAT_YUV_RAW_8BIT_VYUY:
      return C2D_COLOR_FORMAT_422_VYUY;
    default:
      CDBG_ERROR("%s: Invalid cam_fmt = %d", __func__, cam_fmt);
      return C2D_COLOR_FORMAT_420_NV12;
  }
} /* c2d_get_frame_format */

static uint32_t c2d_get_target_rotation(uint32_t type, c2d_flip_type flip)
{
  uint32_t target_config = 0;
  switch (type) {
    case ROTATION_90:
      target_config |= C2D_TARGET_ROTATE_90;
      break;
    case ROTATION_180:
      target_config |= C2D_TARGET_ROTATE_180;
      break;
    case ROTATION_270:
      target_config |= C2D_TARGET_ROTATE_270;
      break;
    default:
      target_config |= C2D_TARGET_ROTATE_0;
      break;
  }
  switch (flip) {
  case C2D_FLIP_H:
    target_config |= C2D_TARGET_MIRROR_H;
    break;
  case C2D_FLIP_V:
    target_config |= C2D_TARGET_MIRROR_V;
    break;
  }
  return target_config;
} /* c2d_get_target_rotation */

static int32_t c2d_process_frame(c2d_module_ctrl_t *ctrl, void *data)
{
  int32_t rc = PPROC_SUCCESS;
  int32_t c2d_rc = C2D_STATUS_OK;
  /* TODO: C2D_DRIVER_INFO is not present on A family */
  //C2D_DRIVER_INFO driver_info;
  C2D_STATUS c2d_status = 0;
  c2d_process_frame_buffer *c2d_process_buffer =
    (c2d_process_frame_buffer *)data;

  CDBG("%s:%d Enter\n", __func__, __LINE__);
  if (!ctrl || !c2d_process_buffer) {
    CDBG_ERROR("%s:%d failed ctrl %p, c2d_process_buffer %p\n", __func__,
      __LINE__, ctrl, c2d_process_buffer);
    return PPROC_FAILURE;
  }

  if (!c2d_process_buffer->c2d_input_buffer ||
    !c2d_process_buffer->c2d_output_buffer) {
    CDBG_ERROR("%s:%d failed input buffer %p, output buffer %p\n", __func__,
      __LINE__, c2d_process_buffer->c2d_input_buffer,
      c2d_process_buffer->c2d_output_buffer);
    return PPROC_FAILURE;
  }

  CDBG("%s:%d\n", __func__, __LINE__);

#ifdef C2D_COPY_FRAME
  /* Call c2d copy frame to copy the frame */
  c2d_copy_frame(c2d_process_buffer);
  return rc;
#endif

  CDBG("%s:%d\n", __func__, __LINE__);

  ctrl->c2d_input_lib_params = c2d_process_buffer->c2d_input_lib_params;
  ctrl->c2d_output_lib_params = c2d_process_buffer->c2d_output_lib_params;
  ctrl->c2d_input_lib_params.surface_type = C2D_SOURCE;
  /* C2D create target surface */
  ctrl->c2d_output_lib_params.surface_type = C2D_TARGET;
  /* C2D map gpu addr and update source surface */
  ctrl->c2d_input_lib_params.surface_def.width =
    c2d_process_buffer->c2d_input_buffer->width;
  ctrl->c2d_input_lib_params.surface_def.height =
    c2d_process_buffer->c2d_input_buffer->height;
  ctrl->c2d_input_lib_params.surface_def.format =
    c2d_get_frame_format(c2d_process_buffer->c2d_input_buffer->format);
  rc = c2d_update_surface(ctrl, &ctrl->c2d_input_lib_params,
    c2d_process_buffer->c2d_input_buffer);
  if (rc < 0) {
    CDBG_ERROR("%s:%d failed", __func__, __LINE__);
    goto ERROR2;
  }

  CDBG("%s:%d\n", __func__, __LINE__);
  /* Rotation in C2D is counter-clockwise, whereas
     rotation value set from HAL expects clockwise,
     therefore swap rotation set for C2D */
  switch(c2d_process_buffer->rotation)
  {
    case ROTATION_90:
      c2d_process_buffer->rotation = ROTATION_270;
      break;
    case ROTATION_270:
      c2d_process_buffer->rotation = ROTATION_90;
      break;
    default:
      break;
  }

  ctrl->c2d_output_lib_params.surface_def.width =
    c2d_process_buffer->c2d_output_buffer->width;
  ctrl->c2d_output_lib_params.surface_def.height =
    c2d_process_buffer->c2d_output_buffer->height;
  ctrl->c2d_output_lib_params.surface_def.format =
    c2d_get_frame_format(c2d_process_buffer->c2d_output_buffer->format);

  /* C2D map gpu addr and update target surface */
  rc = c2d_update_surface(ctrl, &ctrl->c2d_output_lib_params,
    c2d_process_buffer->c2d_output_buffer);
  if (rc < 0) {
    CDBG_ERROR("%s:%d failed", __func__, __LINE__);
    goto ERROR2;
  }

  CDBG("%s:%d\n", __func__, __LINE__);
  /* Check if we process EIS 2.0 */
  if (c2d_process_buffer->c2d_input_buffer->lens_correction_cfg.use_LC) {
    ctrl->LC_params.target_id = ctrl->c2d_output_lib_params.id;
    ctrl->LC_params.lenscorrect_obj.srcId = ctrl->c2d_input_lib_params.id;
    ctrl->LC_params.lenscorrect_obj.blitSize.x =
        c2d_process_buffer->c2d_input_buffer->roi_cfg.x << 16;
    ctrl->LC_params.lenscorrect_obj.blitSize.y =
        c2d_process_buffer->c2d_input_buffer->roi_cfg.y << 16;
    ctrl->LC_params.lenscorrect_obj.blitSize.width =
        c2d_process_buffer->c2d_input_buffer->roi_cfg.width << 16;
    ctrl->LC_params.lenscorrect_obj.blitSize.height =
        c2d_process_buffer->c2d_input_buffer->roi_cfg.height << 16;
    ctrl->LC_params.lenscorrect_obj.gridSize.x =
        c2d_process_buffer->c2d_input_buffer->roi_cfg.x << 16;
    ctrl->LC_params.lenscorrect_obj.gridSize.y =
        c2d_process_buffer->c2d_input_buffer->roi_cfg.y << 16;
    ctrl->LC_params.lenscorrect_obj.gridSize.width =
        c2d_process_buffer->c2d_input_buffer->roi_cfg.width << 16;
    ctrl->LC_params.lenscorrect_obj.gridSize.height =
        c2d_process_buffer->c2d_input_buffer->roi_cfg.height << 16;
    ctrl->LC_params.lenscorrect_obj.offsetX = 0;
    ctrl->LC_params.lenscorrect_obj.offsetY = 0;
    ctrl->LC_params.lenscorrect_obj.source_rect.x =
      (c2d_process_buffer->c2d_input_buffer->x_border / 2) << 16;
    ctrl->LC_params.lenscorrect_obj.source_rect.y =
      (c2d_process_buffer->c2d_input_buffer->y_border / 2) << 16;
    ctrl->LC_params.lenscorrect_obj.source_rect.width =
      (c2d_process_buffer->c2d_input_buffer->width -
       c2d_process_buffer->c2d_input_buffer->x_border) << 16;
    ctrl->LC_params.lenscorrect_obj.source_rect.height =
      (c2d_process_buffer->c2d_input_buffer->height -
      c2d_process_buffer->c2d_input_buffer->y_border) << 16;
    ctrl->LC_params.lenscorrect_obj.target_rect.x = 0;
    ctrl->LC_params.lenscorrect_obj.target_rect.y = 0;
    ctrl->LC_params.lenscorrect_obj.target_rect.width =
      c2d_process_buffer->c2d_output_buffer->width  << 16;
    ctrl->LC_params.lenscorrect_obj.target_rect.height =
      c2d_process_buffer->c2d_output_buffer->height  << 16;
    ctrl->LC_params.lenscorrect_obj.transformType =
        c2d_process_buffer->c2d_input_buffer->lens_correction_cfg.transform_type;
    ctrl->LC_params.lenscorrect_obj.transformMatrices =
        c2d_process_buffer->c2d_input_buffer->lens_correction_cfg.transform_mtx;

    c2d_rc = ctrl->c2d_lib.c2dLensCorrection(ctrl->LC_params.target_id,
      &ctrl->LC_params.lenscorrect_obj);
    if (c2d_rc != C2D_STATUS_OK) {
      CDBG_ERROR("%s:%d failed.c2d_rc %d\n", __func__, __LINE__, c2d_rc);
      goto ERROR2;
    }
  } else {
    /* C2D update frame params */
    /* Update source params */
    ctrl->draw_params.draw_obj.config_mask = 0;
    ctrl->draw_params.draw_obj.surface_id = ctrl->c2d_input_lib_params.id;
    ctrl->draw_params.draw_obj.source_rect.x =
      c2d_process_buffer->c2d_input_buffer->roi_cfg.x << 16;
    ctrl->draw_params.draw_obj.source_rect.y =
        c2d_process_buffer->c2d_input_buffer->roi_cfg.y << 16;
    ctrl->draw_params.draw_obj.source_rect.width =
       c2d_process_buffer->c2d_input_buffer->roi_cfg.width << 16;
    ctrl->draw_params.draw_obj.source_rect.height =
      c2d_process_buffer->c2d_input_buffer->roi_cfg.height << 16;
    ctrl->draw_params.draw_obj.config_mask |=
         (C2D_SOURCE_RECT_BIT | C2D_ALPHA_BLEND_NONE);

    switch (c2d_process_buffer->flip) {
      case C2D_FLIP_H :
        ctrl->draw_params.draw_obj.config_mask |= C2D_MIRROR_H_BIT;
        break;
      case C2D_FLIP_V :
        ctrl->draw_params.draw_obj.config_mask |= C2D_MIRROR_V_BIT;
        break;
      default:
        break;
    }

    CDBG("%s:%d\n", __func__, __LINE__);
     /* Update target params */
    ctrl->draw_params.target_config = 0;
    ctrl->draw_params.target_id = ctrl->c2d_output_lib_params.id;
    ctrl->draw_params.target_scissor = NULL;
    ctrl->draw_params.target_mask_id = 0;
    ctrl->draw_params.target_color_key = 0;
    ctrl->draw_params.target_config |= c2d_get_target_rotation(
      c2d_process_buffer->rotation, c2d_process_buffer->flip);
    ctrl->draw_params.target_config |= C2D_ALPHA_BLEND_NONE;

    CDBG("%s:%d\n", __func__, __LINE__);
   /* C2D draw */
    c2d_rc = ctrl->c2d_lib.c2dDraw(ctrl->draw_params.target_id,
      ctrl->draw_params.target_config, ctrl->draw_params.target_scissor,
      ctrl->draw_params.target_mask_id, ctrl->draw_params.target_color_key,
      &ctrl->draw_params.draw_obj, 1);
    if (c2d_rc != C2D_STATUS_OK) {
      CDBG_ERROR("%s:%d failed.c2d_rc %d\n", __func__, __LINE__, c2d_rc);
      goto ERROR2;
    }
  }

  CDBG("%s:%d\n", __func__, __LINE__);
  /* C2D finish */
  c2d_rc = ctrl->c2d_lib.c2dFinish(ctrl->c2d_output_lib_params.id);
  if (c2d_rc != C2D_STATUS_OK) {
    CDBG_ERROR("%s:%d failed.c2d_rc %d\n", __func__, __LINE__, c2d_rc);
    goto ERROR2;
  }

  CDBG("%s:%d\n", __func__, __LINE__);
  /* C2D unmap gpu addr */
  if (ctrl->c2d_input_lib_params.gpu_buf.num_planes == 1) {
    ctrl->c2d_lib.c2dUnMapAddr(
      (void *)ctrl->c2d_input_lib_params.gpu_buf.sp_gAddr);
    ctrl->c2d_lib.c2dUnMapAddr(
      (void *)ctrl->c2d_output_lib_params.gpu_buf.sp_gAddr);
  } else {
    ctrl->c2d_lib.c2dUnMapAddr(
      (void *)ctrl->c2d_input_lib_params.gpu_buf.mp_gAddr[0]);
    ctrl->c2d_lib.c2dUnMapAddr(
      (void *)ctrl->c2d_output_lib_params.gpu_buf.mp_gAddr[0]);
  }

  CDBG("%s:%d Exit\n", __func__, __LINE__);
  return rc;

ERROR2:
  CDBG("%s:%d\n", __func__, __LINE__);
ERROR1:
  CDBG("%s:%d\n", __func__, __LINE__);
  return rc;
}

/** c2d_open: function to open c2d library and create control
 *  structure
 *
 *  @lib_private: address of pointer to control structure to
 *              store c2d control structure pointer
 *
 *  Return: 0 for success and negative error on failure
 *
 *  This function handles loads c2d library and allocates memory
 *  for c2d control structure to store its state variables **/

static int32_t c2d_open(void **lib_private)
{
  c2d_module_ctrl_t *ctrl = NULL;
  int32_t            value = 0;

  CDBG("%s:%d E\n", __func__, __LINE__);

  /* Validate input parameters */
  if (!lib_private) {
    CDBG_ERROR("%s:%d failed\n", __func__, __LINE__);
    return PPROC_FAILURE;
  }

  /* Allcate memory for c2d control structure */
  ctrl = (c2d_module_ctrl_t *)malloc(sizeof(c2d_module_ctrl_t));
  if (!ctrl) {
    CDBG_ERROR("%s: Malloc Error", __func__);
    return PPROC_FAILURE;
  }
  memset(ctrl, 0, sizeof(c2d_module_ctrl_t));

  /* Open C2D library*/
  ctrl->c2d_lib.ptr = dlopen("libC2D2.so", RTLD_NOW);
  if (!ctrl->c2d_lib.ptr) {
    CDBG_ERROR("%s ERROR: couldn't dlopen libc2d2.so: %s", __func__, dlerror());
    goto ERROR1;
  }

  CDBG("%s:%d\n", __func__, __LINE__);
  /* Get function pointer for functions supported by C2D */
  *(void **)&ctrl->c2d_lib.c2dCreateSurface =
      dlsym(ctrl->c2d_lib.ptr, "c2dCreateSurface");
  *(void **)&ctrl->c2d_lib.c2dUpdateSurface =
      dlsym(ctrl->c2d_lib.ptr, "c2dUpdateSurface");
  *(void **)&ctrl->c2d_lib.c2dLensCorrection =
      dlsym(ctrl->c2d_lib.ptr, "c2dLensCorrection");
  *(void **)&ctrl->c2d_lib.c2dDraw =
      dlsym(ctrl->c2d_lib.ptr, "c2dDraw");
  *(void **)&ctrl->c2d_lib.c2dFinish =
      dlsym(ctrl->c2d_lib.ptr, "c2dFinish");
  *(void **)&ctrl->c2d_lib.c2dDestroySurface =
      dlsym(ctrl->c2d_lib.ptr, "c2dDestroySurface");
  *(void **)&ctrl->c2d_lib.c2dMapAddr =
      dlsym(ctrl->c2d_lib.ptr, "c2dMapAddr");
  *(void **)&ctrl->c2d_lib.c2dUnMapAddr =
      dlsym(ctrl->c2d_lib.ptr, "c2dUnMapAddr");
  /* c2dGetDriverCapabilities is not present in A family */
  /* *(void **)&ctrl->c2d_lib.c2dGetDriverCapabilities =
      dlsym(ctrl->c2d_lib.ptr, "c2dGetDriverCapabilities"); */

  CDBG("%s:%d\n", __func__, __LINE__);
  /* Validate function pointers for C2D library */
  if (!ctrl->c2d_lib.c2dCreateSurface || !ctrl->c2d_lib.c2dUpdateSurface ||
    !ctrl->c2d_lib.c2dFinish || !ctrl->c2d_lib.c2dDestroySurface ||
    !ctrl->c2d_lib.c2dLensCorrection || !ctrl->c2d_lib.c2dDraw ||
    !ctrl->c2d_lib.c2dMapAddr || !ctrl->c2d_lib.c2dUnMapAddr) {
    CDBG_ERROR("%s: ERROR mapping symbols from libc2d2.so", __func__);
    goto ERROR2;
  }
  CDBG("%s:%d\n", __func__, __LINE__);

  /* All succeeded, expose control structure pointer to higher layer */
  *lib_private = (void *)ctrl;

  CDBG("%s:%d X\n", __func__, __LINE__);
  return PPROC_SUCCESS;

ERROR2:
  dlclose(ctrl->c2d_lib.ptr);
ERROR1:
  free(ctrl);
  return PPROC_FAILURE;
}

static int32_t c2d_process(void *lib_private, pproc_interface_event_t event,
  void *data)
{
  int32_t rc = PPROC_SUCCESS;
  c2d_module_ctrl_t *ctrl = (c2d_module_ctrl_t *)lib_private;
  if (!ctrl || !data) {
    CDBG_ERROR("%s:%d failed ctrl %p data %p\n", __func__, __LINE__, ctrl,
      data);
    return PPROC_FAILURE;
  }
  switch (event) {
  case PPROC_IFACE_PROCESS_FRAME:
    CDBG("%s:%d data %p\n", __func__, __LINE__, data);
    rc = c2d_process_frame(ctrl, data);
    break;
  case PPROC_IFACE_CREATE_SURFACE:
    rc = c2d_create_default_yuv_surface(ctrl, data);
    break;
  case PPROC_IFACE_DESTROY_SURFACE:
    rc = c2d_destroy_default_yuv_surface(ctrl, data);
    break;
  default:
    break;
  }
  return rc;
}

static int32_t c2d_close(void *lib_private)
{
  int32_t rc = PPROC_SUCCESS;
  c2d_module_ctrl_t *ctrl = (c2d_module_ctrl_t *)lib_private;
  int32_t value;
  CDBG("%s:%d Enter\n", __func__, __LINE__);

  if (NULL == ctrl)
    return PPROC_FAILURE;

  /* Initialize thread exit variable */
  ctrl->thread_exit = TRUE;

  dlclose(ctrl->c2d_lib.ptr);

  free(ctrl);
  CDBG("%s:%d Exit\n", __func__, __LINE__);
  return rc;
}

static pproc_interface_func_tbl_t c2d_func_tbl = {
  .open = &c2d_open,
  .process = &c2d_process,
  .close = &c2d_close,
};

static pproc_interface_lib_params_t c2d_lib_ptr = {
  .func_tbl = &c2d_func_tbl,
};

pproc_interface_lib_params_t *pproc_library_init(void)
{
  return &c2d_lib_ptr;
}
