/* ============================================================================
  Copyright (c) 2011 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
 ============================================================================*/

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/ioctl.h>
#include <string.h>
#include <errno.h>
#include <dlfcn.h>
#include "camera.h"
#include <linux/msm_kgsl.h>
#include "camera_dbg.h"
#include "config.h"
#include "config_proc.h"
#include "cam_mmap.h"
#include "c2d2.h"
#include "c2dExt.h"
#include "cam_list.h"
#include "stereocam.h"

#define DUMP_DUAL_FRAME 0
#define DUMP_C2D_FRAME 0

#define KGSL_DEVICE_NAME "/dev/kgsl-2d0"

#define PAD_2_2K(a, b) ({ \
  CDBG("%s: input a = %d\n", __func__, a); \
  (!b) ? a : (((a)+2047)& ~2047); \
})

#define ST_ABS(x) (((x) < 0) ? -(x) : (x))

typedef enum {
  DBG_DISPATCH_VIDEO,
  DBG_DISPATCH_SNAP_S,
  DBG_DISPATCH_SNAP_T,
} dbg_dispatch_mode_t;
static dbg_dispatch_mode_t dbg_dispatch_mode;

static pthread_t stereocam_dispatch_thread_id;
static int stereocam_dispatch_exit = 0;
static int is_stereocam_dispatch_thread_ready = 0;
pthread_cond_t stereocam_dispatch_thread_ready_cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t stereocam_dispatch_thread_ready_mutex =
  PTHREAD_MUTEX_INITIALIZER;

static int st_dispatch_terminate_fd[2];

void *libc2d = NULL;
int kgsl_fd;
#if 0
static C2D_STATUS (*LINK_c2dCreateSurface)(uint32_t *surface_id,
  uint32_t surface_bits, C2D_SURFACE_TYPE surface_type,
  void *surface_definition);

static C2D_STATUS (*LINK_c2dUpdateSurface)(uint32_t surface_id,
  uint32_t surface_bits, C2D_SURFACE_TYPE surface_type,
  void *surface_definition);

static C2D_STATUS (*LINK_c2dLensCorrection)(uint32_t targetSurface,
  C2D_LENSCORRECT_OBJECT *sourceObject);

static C2D_STATUS (*LINK_c2dFinish)(uint32_t target_id);

static C2D_STATUS (*LINK_c2dDestroySurface)(uint32_t surface_id);

static C2D_YUV_FORMAT c2d_format = C2D_COLOR_FORMAT_420_NV12;

static float unity_geo_correction[9] = {1, 0, 0,
                                        0, 1, 0,
                                        0, 0, 1};

static float main_geo_correction[9];

typedef struct c2d_addr {
  uint32_t vAddr0;
  uint32_t gAddr0;
  uint32_t vAddr1;
  uint32_t gAddr1;
} c2d_addr_t;

typedef struct c2d_context {
  C2D_YUV_SURFACE_DEF srcSurfaceDef;
  C2D_YUV_SURFACE_DEF dstSurfaceDef;
  c2d_addr_t src_addr;
  c2d_addr_t dst_addr;
  uint32_t src_id;
  uint32_t dst_id;
  C2D_LENSCORRECT_OBJECT lensCorrectObj;
} c2d_context_t;

static c2d_context_t c2d_obj;

typedef struct gpu_addr_list {
  struct cam_list list;
  struct {
    int fd;
    uint32_t vAddr;
    uint32_t gpuAddr;
    uint32_t origGpuAddr;
  } data;
} gpu_addr_list_t;

static gpu_addr_list_t g_list;

static struct st_analysis_param_t {
  float right_shift_old;
  float left_shift_old;
  float final_right_shift;
  float final_left_shift;
  s3d_state_t stereo_conv_state;
  stereo_conv_mode st_conv_mode;
  uint32_t comp_matrix_x_off;
  int comp_matrix_y_off;
} param_t;

/* QCOM: This defines the max CORRUPTED COLUMNS
 * introduced by VFE down scalar. Later we might
 * derive a formula for this but for LiteOn project
 * it should be fine. */
#define MAX_VFE_CORRUPTED_COLUMNS_IN_MIDDLE 2

static struct stereo_dim_t {
  uint32_t pad_l_w;
  uint32_t pad_l_h;
  uint32_t pad_r_w;
  uint32_t pad_r_h;
  uint32_t orig_l_w;
  uint32_t orig_l_h;
  uint32_t orig_r_w;
  uint32_t orig_r_h;
} dim_t;

static void *stereocam_dispatch (void *data);

int launch_stereocam_dispatch_thread(void *data)
{
  stereocam_dispatch_exit = 0;
  is_stereocam_dispatch_thread_ready = 0;

  return pthread_create(&stereocam_dispatch_thread_id, NULL, stereocam_dispatch,
    (void *)data);
} /* launch_stereocam_dispatch_thread */

int wait_stereocam_dispatch_ready() {
  pthread_mutex_lock(&stereocam_dispatch_thread_ready_mutex);

  if (!is_stereocam_dispatch_thread_ready)
    pthread_cond_wait(&stereocam_dispatch_thread_ready_cond,
      &stereocam_dispatch_thread_ready_mutex);

  pthread_mutex_unlock(&stereocam_dispatch_thread_ready_mutex);
  return stereocam_dispatch_exit;
} /* wait_stereocam_dispatch_ready */

int release_stereocam_dispatch_thread(void)
{
  int rc;
  char end = 'y';
  stereocam_dispatch_exit = 1;
  rc = write(st_dispatch_terminate_fd[1], &end, sizeof(end));

  if (rc <0)
    CDBG_HIGH("%s: failed : Failed\n", __func__);

  return pthread_join(stereocam_dispatch_thread_id, NULL);
} /* release_stereocam_dispatch_thread */

void stereocam_dispatch_ready_signal(void)
{
  /*
   * Send signal to config thread to indicate that stereocam_dispatch
   * thread is ready.
   */
  CDBG("stereocam_dispatch() is ready, call pthread_cond_signal\n");

  pthread_mutex_lock(&stereocam_dispatch_thread_ready_mutex);
  is_stereocam_dispatch_thread_ready = 1;
  pthread_cond_signal(&stereocam_dispatch_thread_ready_cond);
  pthread_mutex_unlock(&stereocam_dispatch_thread_ready_mutex);

  CDBG("stereocam_dispatch() is ready, call pthread_cond_signal done\n");
} /* stereocam_dispatch_ready_signal */

/*===========================================================================
 * FUNCTION    - init_c2d_lib -
 *
 * DESCRIPTION:  Load C2D library and link necessary procedures.
 *==========================================================================*/
static int init_c2d_lib()
{
  libc2d = dlopen("libC2D2.so", RTLD_NOW);
  if (!libc2d) {
    CDBG_ERROR("FATAL ERROR: could not dlopen libc2d2.so: %s", dlerror());
    return FALSE;
  }

  *(void **)&LINK_c2dCreateSurface = dlsym(libc2d, "c2dCreateSurface");
  *(void **)&LINK_c2dUpdateSurface = dlsym(libc2d, "c2dUpdateSurface");
  *(void **)&LINK_c2dFinish = dlsym(libc2d, "c2dFinish");
  *(void **)&LINK_c2dDestroySurface = dlsym(libc2d, "c2dDestroySurface");
  *(void **)&LINK_c2dLensCorrection = dlsym(libc2d, "c2dLensCorrection");

  if (!LINK_c2dCreateSurface || !LINK_c2dUpdateSurface || !LINK_c2dFinish ||
    !LINK_c2dDestroySurface || !LINK_c2dLensCorrection) {
    CDBG_ERROR("%s: dlsym ERROR", __func__);
    return FALSE;
  }
  return TRUE;
} /* init_c2d_lib */

/*===========================================================================
 * FUNCTION    - get_gpu_addr -
 *
 * DESCRIPTION: Get gpu address for C2D surface definition.
 *==========================================================================*/
static int get_gpu_addr(int fd, uint32_t len, uint32_t offset, uint32_t vAddr)
{
  int rc = 0;
  struct kgsl_map_user_mem param;
  memset(&param, 0, sizeof(param));
  param.fd = fd;
  param.len = (len + 4095) & ~(4095);
  param.offset = offset;
  param.hostptr = vAddr;
  param.memtype = KGSL_USER_MEM_TYPE_PMEM;

  rc = ioctl(kgsl_fd, IOCTL_KGSL_MAP_USER_MEM, (void *)&param,
             sizeof(param));

  if (!rc) {
    CDBG("%s: gpuAddress = 0x%x\n", __func__, param.gpuaddr);
    return param.gpuaddr;
  }

  CDBG_HIGH("%s: IOCTL_KGSL_MAP_USER_MEM failed with rc = %d.\n", __func__, rc);
  return 0;
} /* get_gpu_addr */

/*===========================================================================
 * FUNCTION    - unmap_gpu_addr -
 *
 * DESCRIPTION: unmap gpu address for C2D surface definition.
 *==========================================================================*/
static int unmap_gpu_addr(unsigned int gpuaddr)
{
  struct kgsl_sharedmem_free param;
  memset(&param, 0, sizeof(param));
  param.gpuaddr = gpuaddr;

  ioctl(kgsl_fd, IOCTL_KGSL_SHAREDMEM_FREE, (void *)&param, sizeof(param));
  return TRUE;
} /* unmap_gpu_addr */

/*===========================================================================
 * FUNCTION    - delete_gpu_addr_list -
 *
 * DESCRIPTION:  Delete gpu address list and free allocated memory.
 *==========================================================================*/
static void delete_gpu_addr_list()
{
  struct cam_list *pos1, *pos2;
  struct cam_list *head = &(g_list.list);
  gpu_addr_list_t *entry;

  for (pos1 = head->next, pos2 = pos1->next; pos1 != head; pos1 = pos2,
    pos2 = pos1->next) {
    entry = member_of(pos1, gpu_addr_list_t, list);
    unmap_gpu_addr(entry->data.origGpuAddr);
    cam_list_del_node(pos1);
    free(entry);
  }
} /* delete_gpu_addr_list */

/*===========================================================================
 * FUNCTION    - find_gpu_addr_item -
 *
 * DESCRIPTION:  Find pmem buffer and matching gpu address entry into list.
 *==========================================================================*/
static uint32_t find_gpu_addr_item(int fd, uint32_t vAddr)
{
  struct cam_list *pos;
  struct cam_list *head = &(g_list.list);
  gpu_addr_list_t *entry;

  CDBG("%s: Entry to find fd = %d, vAddr = 0x%x\n", __func__, fd, vAddr);
  for (pos = head->next; pos != head; pos = pos->next) {
    entry = member_of(pos, gpu_addr_list_t, list);
    CDBG("%s: Current Entry fd = %d vAddr = 0x%x gpuAddr = 0x%x\n", __func__,
      entry->data.fd, entry->data.vAddr, entry->data.gpuAddr);

    if ((entry->data.fd == fd) && (entry->data.vAddr == vAddr))
      return entry->data.gpuAddr;
  }

  CDBG("%s: entry not found\n", __func__);
  return 0;
} /* find_gpu_addr_item */

/*===========================================================================
 * FUNCTION    - add_gpu_addr_item -
 *
 * DESCRIPTION:  Add pmem buffer and matching gpu address entry into list.
 *==========================================================================*/
static void add_gpu_addr_item(int fd, uint32_t vAddr, uint32_t gpuAddr,
  uint32_t origGpuAddr)
{
  gpu_addr_list_t *entry;
  entry = (gpu_addr_list_t *)malloc(sizeof(gpu_addr_list_t));
  if (!entry) {
    CDBG_ERROR("%s: malloc error\n", __func__);
    exit(1);
  }

  entry->data.fd = fd;
  entry->data.vAddr = vAddr;
  entry->data.gpuAddr = gpuAddr;
  entry->data.origGpuAddr = origGpuAddr;

  cam_list_add_tail_node(&(entry->list), &(g_list.list));

  CDBG("%s: entry fd = %d, vAddr = 0x%x, gAddr = 0x%x\n", __func__,
    entry->data.fd, entry->data.vAddr, entry->data.gpuAddr);
} /* add_gpu_addr_item */

/*===========================================================================
 * FUNCTION    - init_kgsl -
 *
 * DESCRIPTION: Get the device fd of "/dev/kgsl-2d0".
 *==========================================================================*/
static int init_kgsl()
{
  kgsl_fd = open(KGSL_DEVICE_NAME, O_RDWR | O_SYNC);
  if (kgsl_fd < 0) {
    CDBG_HIGH("%s: kgsl_fd open failed", __func__);
    return FALSE;
  }

  return TRUE;
} /* init_kgsl */

/*===========================================================================
 * FUNCTION    - create_default_C2D_surface -
 *
 * DESCRIPTION: create C2D surface with default data.
 *==========================================================================*/
static void create_default_C2D_surface(C2D_YUV_SURFACE_DEF *surface,
  uint32_t *surface_id, C2D_SURFACE_BITS surface_bits)
{
  C2D_STATUS rc;

  /* Create Source Surface */
  surface->format = c2d_format;
  surface->width = 1 * 4;
  surface->height = 1 * 4;
  surface->plane0 = (void*)0xaaaaaaaa;
  surface->phys0 = (void*)0xaaaaaaaa;
  surface->stride0 = 1 * 4;
  surface->plane1 = (void*)0xaaaaaaaa;
  surface->phys1 = (void*)0xaaaaaaaa;
  surface->stride1 = 1 * 4;

  rc = LINK_c2dCreateSurface(surface_id, surface_bits,
    (C2D_SURFACE_TYPE)(C2D_SURFACE_YUV_HOST | C2D_SURFACE_WITH_PHYS),
    surface);

  if (rc != C2D_STATUS_OK)
    CDBG_HIGH("%s: c2dCreateSurface failed. rc = %d\n", __func__, rc);
} /* create_default_C2D_surface */

/*===========================================================================
 * FUNCTION    - update_C2D_surface -
 *
 * DESCRIPTION: update C2D surface.
 *==========================================================================*/
static void update_C2D_surface(C2D_YUV_SURFACE_DEF *surface, uint32_t width,
  uint32_t height, uint32_t surface_id, c2d_addr_t *addr, uint32_t stride,
  C2D_SURFACE_BITS surface_bits, C2D_YUV_FORMAT fmt)
{
  C2D_STATUS rc;

  /* Update Source Surface Parameteres*/
  surface->format = fmt;
  surface->width = width;
  surface->height = height;
  surface->plane0 = (void *)addr->vAddr0;
  surface->phys0 = (void *)addr->gAddr0;
  surface->stride0 = stride;
  surface->plane1 = (void *)addr->vAddr1;
  surface->phys1 = (void *)addr->gAddr1;
  surface->stride1 = stride;

  rc = LINK_c2dUpdateSurface(surface_id, surface_bits,
    (C2D_SURFACE_TYPE)(C2D_SURFACE_YUV_HOST | C2D_SURFACE_WITH_PHYS),
    (void *)surface);

  if (rc != C2D_STATUS_OK)
    CDBG_HIGH("%s: c2dUpdateSurface failed. rc = %d\n", __func__, rc);
} /* update_C2D_surface */

/*===========================================================================
 * FUNCTION    - release_c2d_buffer -
 *
 * DESCRIPTION:  unregister C2D buffer with kernel and deallocate.
 *==========================================================================*/
static int release_c2d_buffer(config_ctrl_t *ctrl, struct msm_pmem_info *info)
{
  C2D_STATUS rc;
  if (ioctl(ctrl->camfd, MSM_CAM_IOCTL_UNREGISTER_PMEM, info) < 0) {
    CDBG_HIGH("%s: ioctl MSM_CAM_IOCTL_UNREGISTER_PMEM is failed.\n", __func__);
    return FALSE;
  }

  if (do_munmap(info->fd, (void *)info->vaddr, info->cbcr_off * 3/2)) {
    CDBG_HIGH("%s: munmap failed\n", __func__);
    return FALSE;
  }

  rc = LINK_c2dDestroySurface(c2d_obj.src_id);
  if (rc != C2D_STATUS_OK)
    CDBG_HIGH("%s: LINK_c2dDestroySurface failed\n", __func__);

  rc = LINK_c2dDestroySurface(c2d_obj.dst_id);
  if (rc != C2D_STATUS_OK)
    CDBG_HIGH("%s: LINK_c2dDestroySurface failed\n", __func__);

  delete_gpu_addr_list();
  close(kgsl_fd);
  return TRUE;
} /* release_c2d_buffer */

/*===========================================================================
 * FUNCTION    - init_c2d_buffer -
 *
 * DESCRIPTION:  allocate and register C2D buffer with kernel.
 *==========================================================================*/
static int init_c2d_buffer(config_ctrl_t *ctrl, struct msm_pmem_info *info,
  stereo_frame_t* pStereoFrame, int bufPath)
{
  static int prevBufPath = -1;
  uint32_t frame_w = 0, frame_h = 0;
  uint32_t stride = 0;
  uint8_t pad_2K_bool = (ctrl->vfeCtrl.vfeMode == VFE_MODE_SNAPSHOT) ?
    FALSE : TRUE;

  if (prevBufPath == bufPath) {
    CDBG("%s: No need to create new C2D buffer\n", __func__);
    return TRUE;
  } else if (prevBufPath != -1) {
    CDBG("%s: Create new C2D buffer for bufPath = %d\n", __func__, bufPath);
    if (!release_c2d_buffer(ctrl, info))
      CDBG_HIGH("%s: release_c2d_buffer failed\n", __func__);
  }

  if (pStereoFrame->non_zoom_upscale) {
    frame_w = pStereoFrame->right_pack_dim.orig_w;
    frame_h = pStereoFrame->right_pack_dim.orig_h;
  } else {
    frame_w = pStereoFrame->right_pack_dim.modified_w;
    frame_h = pStereoFrame->right_pack_dim.modified_h;
  }

  info->vaddr = (void *)do_mmap(PAD_2_2K(frame_w * frame_h, pad_2K_bool) * 3/2,
    &(info->fd));

  if (!info->vaddr) {
    CDBG_ERROR("%s: mmap failed\n", __func__);
    return FALSE;
  }

  info->type = MSM_PMEM_C2D;
  info->y_off = 0;
  info->offset = 0;
  info->active = 0;
  info->cbcr_off = PAD_2_2K(frame_w * frame_h, pad_2K_bool);
  info->len = PAD_2_2K(frame_w * frame_h, pad_2K_bool);

  if (ioctl(ctrl->camfd, MSM_CAM_IOCTL_REGISTER_PMEM, info) < 0) {
    CDBG_ERROR("%s: ioctl MSM_CAM_IOCTL_REGISTER_PMEM is failed..\n", __func__);
    return FALSE;
  }

  if (!init_kgsl()) {
    CDBG_ERROR("%s: init_kgsl failed\n", __func__);
    return FALSE;
  }

  stride = frame_w;

  c2d_obj.dst_addr.vAddr0 = (uint32_t)info->vaddr;
  c2d_obj.dst_addr.gAddr0 = get_gpu_addr(info->fd,
    PAD_2_2K(frame_w * frame_h, pad_2K_bool) * 3/2, info->offset,
    (uint32_t)info->vaddr);

  c2d_obj.dst_addr.vAddr1 = c2d_obj.dst_addr.vAddr0 +
    PAD_2_2K(frame_w * frame_h, pad_2K_bool);
  c2d_obj.dst_addr.gAddr1 = c2d_obj.dst_addr.gAddr0 +
    PAD_2_2K(frame_w * frame_h, pad_2K_bool);

  cam_list_init(&(g_list.list));
  add_gpu_addr_item(info->fd, c2d_obj.dst_addr.vAddr0, c2d_obj.dst_addr.gAddr0,
    c2d_obj.dst_addr.gAddr0);

  create_default_C2D_surface(&(c2d_obj.srcSurfaceDef), &(c2d_obj.src_id),
    C2D_SOURCE);

  create_default_C2D_surface(&(c2d_obj.dstSurfaceDef), &(c2d_obj.dst_id),
    C2D_TARGET);

  /* C2D destination surface values will not change. */
  update_C2D_surface(&(c2d_obj.dstSurfaceDef), frame_w, frame_h, c2d_obj.dst_id,
    &(c2d_obj.dst_addr), stride, C2D_TARGET, c2d_format);

  prevBufPath = bufPath;

  return TRUE;
} /* init_c2d_buffer */

/*===========================================================================
 * FUNCTION    - stereo_geo_correction -
 *
 * DESCRIPTION: Apply geomatric correction to right frame using C2D
 *==========================================================================*/
static int stereo_geo_correction(uint32_t width, uint32_t height,
  stereo_frame_t *pStereoFrame)
{
  C2D_STATUS rc;
  c2d_obj.lensCorrectObj.srcId = c2d_obj.src_id;

  c2d_obj.lensCorrectObj.blitSize.x = 0;
  c2d_obj.lensCorrectObj.blitSize.y = 0;
  c2d_obj.lensCorrectObj.blitSize.width = width << 16;
  c2d_obj.lensCorrectObj.blitSize.height = height << 16;

  c2d_obj.lensCorrectObj.gridSize.width = width << 16;
  c2d_obj.lensCorrectObj.gridSize.height = height << 16;
  c2d_obj.lensCorrectObj.gridSize.x = 0;
  c2d_obj.lensCorrectObj.gridSize.y = 0;

  c2d_obj.lensCorrectObj.offsetX = 0;
  c2d_obj.lensCorrectObj.offsetY = 0;

  if (param_t.stereo_conv_state == S3D_STATE_2D)
    c2d_obj.lensCorrectObj.transformMatrices = unity_geo_correction;
  else
    c2d_obj.lensCorrectObj.transformMatrices = main_geo_correction;

/* For debug */
#ifdef PRINT_STEREO_MATRIX
  PRINT_1D_MATRIX(3, 3, c2d_obj.lensCorrectObj.transformMatrices);
#endif

  c2d_obj.lensCorrectObj.transformType =
    C2D_LENSCORRECT_PERSPECTIVE | C2D_LENSCORRECT_BILINEAR;

  CDBG("%s: Send Frame for c2dLensCorrection\n", __func__);
  rc = LINK_c2dLensCorrection(c2d_obj.dst_id, &(c2d_obj.lensCorrectObj));
  if (rc != C2D_STATUS_OK) {
    CDBG_ERROR("%s: c2dLensCorrection failed. rc = %d\n", __func__, rc);
    return FALSE;
  }

  rc = LINK_c2dFinish(c2d_obj.dst_id);
  if (rc != C2D_STATUS_OK) {
    CDBG_ERROR("%s: LINK_c2dFinish failed\n", __func__);
    return FALSE;
  }
  CDBG("%s: c2dLensCorrection done.\n", __func__);
  return TRUE;
} /* stereo_geo_correction */

/*===========================================================================
 * FUNCTION    - get_local_dimension -
 *
 * DESCRIPTION: Prepare local stereo dimension.
 *==========================================================================*/
static void get_local_dimension(stereo_frame_t *pStereoFrame)
{
  if (pStereoFrame->non_zoom_upscale) {
    dim_t.orig_r_w = pStereoFrame->right_pack_dim.modified_w;
    dim_t.orig_r_h = pStereoFrame->right_pack_dim.modified_h;
    dim_t.orig_l_w = pStereoFrame->left_pack_dim.modified_w;
    dim_t.orig_l_h = pStereoFrame->left_pack_dim.modified_h;

    dim_t.pad_r_w = pStereoFrame->right_pack_dim.orig_w;
    dim_t.pad_r_h = pStereoFrame->right_pack_dim.orig_h;
    dim_t.pad_l_w = pStereoFrame->left_pack_dim.orig_w;
    dim_t.pad_l_h = pStereoFrame->left_pack_dim.orig_h;
  } else {
    dim_t.orig_r_w = pStereoFrame->right_pack_dim.orig_w;
    dim_t.orig_r_h = pStereoFrame->right_pack_dim.orig_h;
    dim_t.orig_l_w = pStereoFrame->left_pack_dim.orig_w;
    dim_t.orig_l_h = pStereoFrame->left_pack_dim.orig_h;

    dim_t.pad_r_w = pStereoFrame->right_pack_dim.modified_w;
    dim_t.pad_r_h = pStereoFrame->right_pack_dim.modified_h;
    dim_t.pad_l_w = pStereoFrame->left_pack_dim.modified_w;
    dim_t.pad_l_h = pStereoFrame->left_pack_dim.modified_h;
  }

  CDBG("%s: orig left %dx%d\n", __func__, dim_t.orig_l_w, dim_t.orig_l_h);
  CDBG("%s: orig right %dx%d\n", __func__, dim_t.orig_r_w, dim_t.orig_r_h);
  CDBG("%s: padded left %dx%d\n", __func__, dim_t.pad_l_w, dim_t.pad_l_h);
  CDBG("%s: padded right %dx%d\n", __func__, dim_t.pad_r_w, dim_t.pad_r_h);
} /* get_local_dimension */

/*===========================================================================
 * FUNCTION    - stereocam_calculate_final_shift -
 *
 * DESCRIPTION: Calculate final shift values applied to L or R frame
 *              based on following criteria.
 *              * Analysis to Main Frame scaling
 *              * Zoom Scaling
 *              * Boundry hit conditions
 *==========================================================================*/
static void stereocam_calculate_final_shift(config_ctrl_t *ctrl,
  struct msm_st_frame *op_frame, float shift_scale_x,
  uint32_t new_right_x_off)
{
  int right_x_off_t = 0, right_y_off_t = 0;
  int temp_x_off = 0, temp_y_off = 0;
  int boundry_x_comp = 0, boundry_y_comp = 0;
  float zoom_scale_x = 1;
  uint32_t x_range = dim_t.pad_l_w - dim_t.orig_l_w;
  static struct msm_st_frame snap_frame;
  static struct stereo_dim_t snap_dim_t;

  if ((op_frame->L.stCropInfo.out_w != 0) &&
      (op_frame->L.stCropInfo.out_h != 0))
    zoom_scale_x = (float)op_frame->L.stCropInfo.in_w /
      (float)op_frame->L.stCropInfo.out_w;

  CDBG("%s: Before calc L shift %f R shift %f  scale %f zoom_scale_x %f\n",
    __func__, param_t.final_left_shift, param_t.final_right_shift,
    shift_scale_x, zoom_scale_x);

  if (param_t.st_conv_mode == ST_CONV_MANUAL) {
    CDBG("%s: ManualConvergence is enabled\n", __func__);
    right_x_off_t =
      (int)new_right_x_off - (ctrl->stereoCtrl.manualConvRange / 4);
  } else {
    right_x_off_t = x_range/2;
    right_x_off_t -=
      (float)(param_t.final_right_shift * shift_scale_x * zoom_scale_x);
  }
  temp_x_off = right_x_off_t;
  right_x_off_t += (int)param_t.comp_matrix_x_off;
  right_y_off_t = (int)((dim_t.pad_r_h - dim_t.orig_r_h)/2) -
    param_t.comp_matrix_y_off;
  temp_y_off = right_y_off_t;

  CDBG("%s: Before clamp y off = %d\n", __func__, temp_y_off);
  CDBG("%s: Before Boundry R x off = %d\n", __func__, right_x_off_t);

  /* Right Frame Boundary condition check*/
  right_x_off_t = MIN(right_x_off_t, (int)x_range - 1);
  right_x_off_t = MAX(right_x_off_t, MAX_VFE_CORRUPTED_COLUMNS_IN_MIDDLE);
  right_y_off_t = MIN(right_y_off_t, (int)(dim_t.pad_r_h - dim_t.orig_r_h) - 1);
  right_y_off_t = MAX(right_y_off_t, 0);

  CDBG("%s: After Boundry R x off = %d\n", __func__, right_x_off_t);

  op_frame->R.pix_x_off = right_x_off_t;
  op_frame->R.pix_y_off = right_y_off_t;

  if (temp_x_off != right_x_off_t) {
    /* If the x offset is clamped to 0, we are trying to move R frame on left
     * but hit the boundry. So in order to comensate we move L frame on right
     * and vice-versa.
     */
    boundry_x_comp = (right_x_off_t - temp_x_off) + param_t.comp_matrix_x_off;

    CDBG("%s: Before Clamp = %d, After Clamp = %d\n", __func__,
      temp_x_off, right_x_off_t);
    CDBG("%s: R Frame crop offset hits the boundry.\n", __func__);
    CDBG("%s: Compensate on Left. x_comp = %d\n.", __func__, boundry_x_comp);
  }

  op_frame->L.pix_x_off = x_range/2;
  CDBG("%s: Before Boundry & Comp L x off = %d\n", __func__, op_frame->L.pix_x_off);
  op_frame->L.pix_x_off +=
    (float)(param_t.final_left_shift * shift_scale_x * zoom_scale_x);
  op_frame->L.pix_x_off += boundry_x_comp;

  op_frame->L.pix_y_off = op_frame->R.pix_y_off;

  /* Left Frame Boundary condition check*/
  op_frame->L.pix_x_off = MIN(op_frame->L.pix_x_off, x_range - 1);
  op_frame->L.pix_x_off = MAX(op_frame->L.pix_x_off, 0);
  op_frame->L.pix_y_off = MIN(op_frame->L.pix_y_off,
    (dim_t.pad_l_h - dim_t.orig_l_h) - 1);
  op_frame->L.pix_y_off = MAX(op_frame->L.pix_y_off, 0);

  CDBG("%s: After Boundry & Comp L x off = %d\n", __func__, op_frame->L.pix_x_off);
  CDBG("%s: L y off = %d, R y off = %d\n", __func__, op_frame->L.pix_y_off,
    op_frame->R.pix_y_off);

  if (param_t.stereo_conv_state == S3D_STATE_2D) {
    op_frame->R.pix_x_off = op_frame->L.pix_x_off;
    op_frame->R.pix_y_off = op_frame->L.pix_y_off;
  }

  op_frame->R.stCropInfo = op_frame->L.stCropInfo;

  op_frame->buf_info.stcam_conv_value = op_frame->R.pix_x_off +
    (ctrl->stereoCtrl.manualConvRange / 4);
} /* stereocam_calculate_final_shift */

/*===========================================================================
 * FUNCTION    - prepare_left_frame -
 *
 * DESCRIPTION:  Prepare stereo left frame for VPE.
 *==========================================================================*/
static void prepare_left_frame(config_ctrl_t *ctrl,
  struct msm_st_frame *ip_frame, struct msm_st_frame *op_frame,
  stereo_frame_t *pStereoFrame)
{
  uint32_t stride = 0;
  float shift_scale_x;
  uint32_t mono_w_scale = 0, mono_h_scale = 0;

  CDBG("%s: Received Dual Frame from config Thread\n", __func__);
  uint8_t pad_2K_bool = (ctrl->vfeCtrl.vfeMode == VFE_MODE_SNAPSHOT) ?
    FALSE : TRUE;

  FIND_STEREO_SIZE_FACTOR(pStereoFrame->packing, FALSE,
    mono_w_scale, mono_h_scale);

  *op_frame = *ip_frame;
  op_frame->type = OUTPUT_TYPE_ST_L;

  op_frame->L.buf_y_off = 0;
  op_frame->L.buf_cbcr_off =
    PAD_2_2K((dim_t.pad_l_w * mono_w_scale) * (dim_t.pad_l_h * mono_h_scale),
      pad_2K_bool);

  op_frame->L.buf_y_stride = dim_t.pad_l_w * mono_w_scale;
  op_frame->L.buf_cbcr_stride = dim_t.pad_l_w * mono_w_scale;

  if (pStereoFrame->non_zoom_upscale)
    shift_scale_x = (float)dim_t.pad_l_w /
      ctrl->stereoCtrl.procFrame.left_pack_dim.orig_w;
  else
    shift_scale_x = (float)dim_t.pad_l_w /
      ctrl->stereoCtrl.procFrame.left_pack_dim.modified_w;

  stereocam_calculate_final_shift(ctrl, op_frame, shift_scale_x,
    ctrl->stereoCtrl.manualConvValue);

  CDBG("%s: L.pix_x_off:%d L.pix_y_off:%d", __func__,
    op_frame->L.pix_x_off, op_frame->L.pix_y_off);

#if DUMP_DUAL_FRAME
  static int s_count = 0, t_count = 0, v_count = 0;
  int dump_en = 0;
  char buf[128];
  char* filename1 = "/data/dual_video_%d.yuv";

  if (dbg_dispatch_mode == DBG_DISPATCH_SNAP_S) {
    filename1 = "/data/dual_snap_%d.yuv";
    s_count++;
    if (s_count >= 1 && s_count < 5) {
      sprintf(buf, filename1, s_count);
      dump_en = 1;
    }
  } else if (dbg_dispatch_mode == DBG_DISPATCH_SNAP_T) {
    filename1 = "/data/dual_thumb_%d.yuv";
    t_count++;
    if (t_count >= 1 && t_count < 5) {
      sprintf(buf, filename1, t_count);
      dump_en = 1;
    }
  } else {
    v_count++;
    if (v_count >= 1 && v_count < 6) {
      sprintf(buf, filename1, v_count);
      dump_en = 1;
    }
  }

  if (dump_en)
    cam_dump_yuv2((void *)op_frame->buf_info.buffer,
      (void *)(op_frame->buf_info.buffer +
      PAD_2_2K((dim_t.pad_l_w * mono_w_scale) *
      (dim_t.pad_l_h * mono_h_scale), pad_2K_bool)),
      (dim_t.pad_l_w * mono_w_scale) * (dim_t.pad_l_h * mono_h_scale), buf);
#endif
} /* prepare_left_frame */

/*===========================================================================
 * FUNCTION    - prepare_non_fusible_frame -
 *
 * DESCRIPTION: Currrent frame is not fusible so left frame should
 *              be copied into right frame buffer.
 *==========================================================================*/
static int prepare_non_fusible_frame(config_ctrl_t *ctrl,
  struct msm_st_frame *ip_frame, struct msm_st_frame *op_frame,
  stereo_frame_t *pStereoFrame)
{
  uint32_t stride = 0;
  uint32_t mono_w_scale = 0, mono_h_scale = 0;

  uint8_t pad_2K_bool = (ctrl->vfeCtrl.vfeMode == VFE_MODE_SNAPSHOT) ?
    FALSE : TRUE;

  FIND_STEREO_SIZE_FACTOR(pStereoFrame->packing, FALSE,
    mono_w_scale, mono_h_scale);

  CDBG("%s: R.pix_x_off:%d R.pix_y_off:%d", __func__,
    op_frame->R.pix_x_off, op_frame->R.pix_y_off);

  c2d_obj.src_addr.vAddr0 = ip_frame->buf_info.buffer;
  c2d_obj.src_addr.gAddr0 = find_gpu_addr_item(ip_frame->buf_info.fd,
    c2d_obj.src_addr.vAddr0);

  if (!c2d_obj.src_addr.gAddr0) {
    uint32_t buf_gAddr;
    buf_gAddr = get_gpu_addr(ip_frame->buf_info.fd,
      PAD_2_2K((dim_t.pad_r_w * mono_w_scale) * (dim_t.pad_l_h * mono_h_scale),
      pad_2K_bool) * 3/2, ip_frame->buf_info.phy_offset,
      ip_frame->buf_info.buffer);

    c2d_obj.src_addr.gAddr0 = buf_gAddr;
    add_gpu_addr_item(ip_frame->buf_info.fd, c2d_obj.src_addr.vAddr0,
      c2d_obj.src_addr.gAddr0, buf_gAddr);
  }

  c2d_obj.src_addr.vAddr1 = c2d_obj.src_addr.vAddr0 +
    PAD_2_2K((dim_t.pad_r_w * mono_w_scale) * (dim_t.pad_l_h * mono_h_scale),
      pad_2K_bool);
  c2d_obj.src_addr.gAddr1 = c2d_obj.src_addr.gAddr0 +
    PAD_2_2K((dim_t.pad_r_w * mono_w_scale) * (dim_t.pad_l_h * mono_h_scale),
      pad_2K_bool);

  stride = dim_t.pad_r_w * mono_w_scale;

  /* update C2D source surface. */
  update_C2D_surface(&(c2d_obj.srcSurfaceDef), dim_t.pad_r_w, dim_t.pad_r_h,
    c2d_obj.src_id, &(c2d_obj.src_addr), stride, C2D_SOURCE, c2d_format);

  if (!stereo_geo_correction(dim_t.pad_r_w, dim_t.pad_r_h, pStereoFrame)) {
    CDBG_HIGH("%s: stereo_geo_correction failed.\n", __func__);
    return FALSE;
  }
  /* Prepare C2D output buffer for VPE. */
  op_frame->R.buf_y_off = 0;
  op_frame->R.buf_cbcr_off =
    PAD_2_2K(dim_t.pad_l_w * dim_t.pad_l_h, pad_2K_bool);

  op_frame->R.buf_y_stride = dim_t.pad_l_w;
  op_frame->R.buf_cbcr_stride = dim_t.pad_l_w;

  return TRUE;
} /* prepare_non_fusible_frame */

/*===========================================================================
 * FUNCTION    - prepare_right_frame -
 *
 * DESCRIPTION: Send right frame to C2D and prepare it for VPE.
 *==========================================================================*/
static int prepare_right_frame(config_ctrl_t *ctrl,
  struct msm_st_frame *ip_frame, struct msm_st_frame *op_frame,
  stereo_frame_t *pStereoFrame)
{
  uint32_t stride = 0;
  static int s_count = 0, t_count = 0, v_count = 0;
  int dump_en = 0;
  char buf[128];
  char* filename1 = "/data/c2d_video_%d.yuv";

  CDBG("%s: Prepare right frame for C2D and VPE\n", __func__);
  uint8_t pad_2K_bool = (ctrl->vfeCtrl.vfeMode == VFE_MODE_SNAPSHOT) ?
    FALSE : TRUE;

  op_frame->type = OUTPUT_TYPE_ST_R;

  /* Prepare C2D output buffer for VPE. */
  op_frame->R.buf_y_off = 0;
  op_frame->R.buf_cbcr_off =
    PAD_2_2K(dim_t.pad_r_w * dim_t.pad_r_h, pad_2K_bool);

  op_frame->R.buf_y_stride = dim_t.pad_r_w;
  op_frame->R.buf_cbcr_stride = dim_t.pad_r_w;

  if (param_t.stereo_conv_state == S3D_STATE_2D) {
    CDBG("%s: current 3D frame is not fusible\n", __func__);
    if (!prepare_non_fusible_frame(ctrl, ip_frame, op_frame, pStereoFrame)) {
      CDBG_HIGH("%s: prepare_non_fusible_frame failed\n", __func__);
      return FALSE;
    }
    goto dump_and_return;
  }

  CDBG("%s: R.pix_x_off:%d R.pix_y_off:%d", __func__,
    op_frame->R.pix_x_off, op_frame->R.pix_y_off);

  CDBG("%s: current 3D frame is fusible\n", __func__);

  if ((pStereoFrame->packing == TOP_DOWN_FULL) ||
    (pStereoFrame->packing == TOP_DOWN_HALF)) {

    c2d_obj.src_addr.vAddr0 = ip_frame->buf_info.buffer +
      (dim_t.pad_l_w * dim_t.pad_l_h);
    c2d_obj.src_addr.gAddr0 = find_gpu_addr_item(ip_frame->buf_info.fd,
      c2d_obj.src_addr.vAddr0);

    if (!c2d_obj.src_addr.gAddr0) {
      uint32_t buf_gAddr;
      buf_gAddr = get_gpu_addr(ip_frame->buf_info.fd,
        PAD_2_2K(dim_t.pad_r_w * (dim_t.pad_l_h + dim_t.pad_r_h),
        pad_2K_bool) * 3/2, ip_frame->buf_info.phy_offset,
        ip_frame->buf_info.buffer);

      c2d_obj.src_addr.gAddr0 = buf_gAddr + (dim_t.pad_l_w * dim_t.pad_l_h);
      add_gpu_addr_item(ip_frame->buf_info.fd, c2d_obj.src_addr.vAddr0,
        c2d_obj.src_addr.gAddr0, buf_gAddr);
    }

    c2d_obj.src_addr.vAddr1 = c2d_obj.src_addr.vAddr0 +
      PAD_2_2K(dim_t.pad_r_w * dim_t.pad_r_h, pad_2K_bool) +
      ((dim_t.pad_l_w * dim_t.pad_l_h)/2);
    c2d_obj.src_addr.gAddr1 = c2d_obj.src_addr.gAddr0 +
      PAD_2_2K(dim_t.pad_r_w * dim_t.pad_r_h, pad_2K_bool) +
      ((dim_t.pad_l_w * dim_t.pad_l_h)/2);

    stride = dim_t.pad_r_w;

    /* update C2D source surface. */
    update_C2D_surface(&(c2d_obj.srcSurfaceDef), dim_t.pad_r_w, dim_t.pad_r_h,
      c2d_obj.src_id, &(c2d_obj.src_addr), stride, C2D_SOURCE, c2d_format);
  } else if ((pStereoFrame->packing == SIDE_BY_SIDE_FULL) ||
    (pStereoFrame->packing == SIDE_BY_SIDE_HALF)) {

    c2d_obj.src_addr.vAddr0 = ip_frame->buf_info.buffer + dim_t.pad_l_w;
    c2d_obj.src_addr.gAddr0 = find_gpu_addr_item(ip_frame->buf_info.fd,
      c2d_obj.src_addr.vAddr0);

    if (!c2d_obj.src_addr.gAddr0) {
     uint32_t buf_gAddr;
     buf_gAddr = get_gpu_addr(ip_frame->buf_info.fd,
       PAD_2_2K((dim_t.pad_l_w + dim_t.pad_r_w) * dim_t.pad_r_h,
       pad_2K_bool) * 3/2, ip_frame->buf_info.phy_offset,
       ip_frame->buf_info.buffer);

     c2d_obj.src_addr.gAddr0 = buf_gAddr + dim_t.pad_l_w;

     add_gpu_addr_item(ip_frame->buf_info.fd, c2d_obj.src_addr.vAddr0,
       c2d_obj.src_addr.gAddr0, buf_gAddr);
    }

    c2d_obj.src_addr.vAddr1 = c2d_obj.src_addr.vAddr0 +
     PAD_2_2K((dim_t.pad_l_w + dim_t.pad_r_w) * dim_t.pad_r_h, pad_2K_bool);
    c2d_obj.src_addr.gAddr1 = c2d_obj.src_addr.gAddr0 +
     PAD_2_2K((dim_t.pad_l_w + dim_t.pad_r_w) * dim_t.pad_r_h, pad_2K_bool);

    stride = dim_t.pad_l_w + dim_t.pad_r_w;

    /* update C2D source surface. */
    update_C2D_surface(&(c2d_obj.srcSurfaceDef), dim_t.pad_r_w, dim_t.pad_r_h,
     c2d_obj.src_id, &(c2d_obj.src_addr), stride, C2D_SOURCE, c2d_format);
  } else {
    CDBG_HIGH("%s: Invalid packing = %d\n", __func__, pStereoFrame->packing);
    return FALSE;
  }

  if (!stereo_geo_correction(dim_t.pad_r_w, dim_t.pad_r_h, pStereoFrame)) {
    CDBG_HIGH("%s: stereo_geo_correction failed.\n", __func__);
    return FALSE;
  }

dump_and_return:
#if DUMP_C2D_FRAME
  if (dbg_dispatch_mode == DBG_DISPATCH_SNAP_S) {
    filename1 = "/data/c2d_snap_%d.yuv";
    s_count++;
    if (s_count >= 1 && s_count < 5) {
      sprintf(buf, filename1, s_count);
      dump_en = 1;
    }
  } else if (dbg_dispatch_mode == DBG_DISPATCH_SNAP_T) {
    filename1 = "/data/c2d_thumb_%d.yuv";
    t_count++;
    if (t_count >= 1 && t_count < 5) {
      sprintf(buf, filename1, t_count);
      dump_en = 1;
    }
  } else {
    v_count++;
    if (v_count >= 1 && v_count < 6) {
      sprintf(buf, filename1, v_count);
      dump_en = 1;
    }
  }

  if (dump_en)
    cam_dump_yuv2((void *)op_frame->buf_info.buffer,
      (void *)(op_frame->buf_info.buffer +
      PAD_2_2K(dim_t.pad_r_w * dim_t.pad_r_h, pad_2K_bool)),
      dim_t.pad_r_w * dim_t.pad_r_h, buf);
#endif

  return TRUE;
} /* prepare_right_frame */

/*===========================================================================
 * FUNCTION    - stereocam_update_analysis_shift -
 *
 * DESCRIPTION: Update right shift value from stereo analysis.
 *==========================================================================*/
void stereocam_update_analysis_shift(config_ctrl_t *ctrl)
{
  static uint32_t frame_count = 0;
  vfe_cmd_t *vCmd = (vfe_cmd_t *)ctrl->vfeCtrl.vfeCmd;
  /* TODO: Find the best way to VFE_OP 1 skip pattern.
   *       vCmd->VFE_FrameSkipCfgCmd.output1YPeriod + 1;
   */
  uint32_t number_of_steps = 4;
  float shift_delta, index, step_shift = 0;

  if (VFE_MODE_SNAPSHOT == ctrl->vfeCtrl.vfeMode) {
    frame_count = 0;
    step_shift = 0;
    param_t.final_right_shift = ctrl->stereoCtrl.right_image_shift;
    return;
  }

  shift_delta = ctrl->stereoCtrl.right_image_shift -
    param_t.right_shift_old;

  if (shift_delta == 0)
    frame_count = 0;

  frame_count++;

  index = frame_count % number_of_steps;
  CDBG("%s: index = %f\n", __func__, index);
  if (index == 0) {
    CDBG("%s: 1st shift_delta = %f, f_count = %d\n", __func__, shift_delta,
      frame_count);
    frame_count = 0;
    param_t.final_right_shift = param_t.right_shift_old + shift_delta;
    param_t.right_shift_old = ctrl->stereoCtrl.right_image_shift;
  } else {
    step_shift = (shift_delta / number_of_steps) * index;
    CDBG("%s: divide = %f\n", __func__, (shift_delta / number_of_steps));
    CDBG("%s: 2st shift_delta = %f, f_count = %d step_shift = %f\n", __func__,
      shift_delta, frame_count, step_shift);
    param_t.final_right_shift = param_t.right_shift_old + step_shift;
  }
} /* stereocam_update_analysis_shift */

/*===========================================================================
 * FUNCTION    - update_geo_correction_matrix -
 *
 * DESCRIPTION: In 3D when VFE down-scales the frame, it uses M/N
 *              scaler which will introduce some incorrect columns at the
 *              join, or middle, of L/R frames. Now if correction matrix's
 *              x offset is negative than C2D will shift the frame by that
 *              x offset and fill the new columns with first column data.
 *              Since first few columns could be incorrect, new filling
 *              by C2D will be bad and could be seen in preview if VPE
 *              crop offset doesn't cut it. Following change will make
 *              sure that if matrix's x offset is less than 0 than VPE will
 *              crop out those bad columns and compansate them in Left.
 *==========================================================================*/
static void update_geo_correction_matrix(stereo_frame_t *pStereoFrame)
{
  int y_disp_t = 0;
  float corrected_matrix_y_rot_off = 0.0;

  memcpy(main_geo_correction, pStereoFrame->geo_corr_matrix,
    sizeof(main_geo_correction));

  param_t.comp_matrix_x_off = 0;
  CDBG("%s: Before update x_off_compensate = %d, matrix x off = %f\n",
    __func__, param_t.comp_matrix_x_off, main_geo_correction[2]);

  if (main_geo_correction[2] < 0) {
    if (main_geo_correction[1] < 0)
      corrected_matrix_y_rot_off =
        ceil(ST_ABS(main_geo_correction[1] * dim_t.pad_l_h));
    CDBG("%s: x offset is < 0. matrix_y_rot_off = %f\n", __func__,
      corrected_matrix_y_rot_off);
    param_t.comp_matrix_x_off =
      (uint32_t)(ceil(ST_ABS(main_geo_correction[2])) +
      corrected_matrix_y_rot_off);
    main_geo_correction[2] = 0.0 + corrected_matrix_y_rot_off;
  } else {
    if (main_geo_correction[1] < 0)
      corrected_matrix_y_rot_off =
        ceil(ST_ABS(main_geo_correction[1] * dim_t.pad_l_h));
    CDBG("%s: x offset is > 0. matrix_y_rot_off = %f\n", __func__,
      corrected_matrix_y_rot_off);
    main_geo_correction[2] += corrected_matrix_y_rot_off + 1;
  }
  CDBG("%s: After update x_off_compensate = %d, matrix x off = %f\n",
    __func__, param_t.comp_matrix_x_off, main_geo_correction[2]);

  param_t.comp_matrix_y_off = 0;
  CDBG("%s: Before update y_off_compensate = %d, matrix y off = %f\n",
    __func__, param_t.comp_matrix_y_off, main_geo_correction[5]);

  y_disp_t = (int)ceil(ST_ABS(main_geo_correction[5])) -
    (int)((dim_t.pad_l_h - dim_t.orig_l_h)/2);

  if ((y_disp_t > 0) && (main_geo_correction[5] > 0))
    param_t.comp_matrix_y_off = y_disp_t;
  else if ((y_disp_t > 0) && (main_geo_correction[5] < 0))
    param_t.comp_matrix_y_off = -y_disp_t;

  CDBG("%s: After update y_off_compensate = %d, matrix y off = %f\n",
    __func__, param_t.comp_matrix_y_off, main_geo_correction[5]);
} /* update_geo_correction_matrix */

/*===========================================================================
 * FUNCTION    - stereocam_dispatch -
 *
 * DESCRIPTION: stereocam dispatch thread which will talk to VPE & C2D
 *==========================================================================*/
static void *stereocam_dispatch(void *data)
{
  fd_set dispatch_fds;
  int dispatch_nfds = 0;
  int rc;
  config_ctrl_t *cctrl = (config_ctrl_t *)data;
  stereo_ctrl_t *stCtrl = (stereo_ctrl_t *)&cctrl->stereoCtrl;
  struct msm_cam_evt_msg camMsg;
  struct msm_st_frame input_frame, output_frame_L, output_frame_R;
  struct msm_frame c2d_op_frame;
  struct msm_pmem_info c2d_op_frame_info;
  int c2d_done = 0;
  int vpe_done = 0;
  stereo_frame_t *pstereoFrame = &cctrl->stereoCtrl.videoFrame;
  c2d_format = C2D_COLOR_FORMAT_420_NV12;
  stCtrl->manualConvValue = stCtrl->manualConvRange / 2;

  if (pipe(st_dispatch_terminate_fd)< 0) {
    CDBG_HIGH("%s: thread termination pipe creation failed\n", __func__);
    return NULL;
  }

  CDBG("%s: st_dispatch_terminate_fds %d %d\n", __func__,
    st_dispatch_terminate_fd[0], st_dispatch_terminate_fd[1]);

  if (!init_c2d_lib()) {
    CDBG_HIGH("%s: init_c2d_lib failed\n", __func__);
    return NULL;
  }

  memset(&param_t, 0x0, sizeof(param_t));

  stereocam_dispatch_ready_signal();
  do {
    struct timeval dispatch_timeout;
    dispatch_timeout.tv_usec = 0;
    dispatch_timeout.tv_sec = 6;

    FD_ZERO(&dispatch_fds);
    FD_SET(cctrl->child_fd_set[STEREO_DISPATCH][PIPE_IN][READ_END],
      &dispatch_fds);
    FD_SET(st_dispatch_terminate_fd[0], &dispatch_fds);

    dispatch_nfds =
      MAX(cctrl->child_fd_set[STEREO_DISPATCH][PIPE_IN][READ_END],
      st_dispatch_terminate_fd[0]);

    CDBG("%s: ... Duty Loop start ...\n", __func__);

    rc = select(dispatch_nfds + 1, &dispatch_fds, NULL, NULL,
      &dispatch_timeout);
    if (rc == 0) {
      CDBG_HIGH("...stereocam_dispatch select timeout...\n");
      continue;
    } else if (rc < 0) {
      CDBG_ERROR("%s: SELECT ERROR %s \n", __func__, strerror(errno));
      if (stereocam_dispatch_exit != 0) break;
      usleep(1000 * 100);
      continue;
    } else if (rc) {
      if (stereocam_dispatch_exit != 0) break;
      CDBG("%s: select woke up\n", __func__);

      if (FD_ISSET(cctrl->child_fd_set[STEREO_DISPATCH][PIPE_IN][READ_END],
        &dispatch_fds)) {
        if (read(cctrl->child_fd_set[STEREO_DISPATCH][PIPE_IN][READ_END],
          &camMsg, sizeof(camMsg)) < 0)
          CDBG_HIGH("%s: Cannot read from config thread\n", __func__);

        if (camMsg.data == NULL) {
          CDBG_ERROR("%s: Error...Msg Data pointer is NULL\n", __func__);
          break;
        } else {
          CDBG("%s: Msg Data pointer is Good.\n", __func__);
          input_frame = *(struct msm_st_frame *)camMsg.data;
        }

        if (input_frame.type == OUTPUT_TYPE_ST_D) {

          stereocam_update_analysis_shift(cctrl);

          /* If lib3D analysis is non-fusible, as per R&D specs we should copy
           * L frame onto R and continue. But this leads to sudden jumps in the
           * preview and video recording which degrades user experience compare
           * to applying 2D state. So igonre the lib3D convergence state and
           * always keep it as fusible or 3D. */
          param_t.stereo_conv_state = S3D_STATE_3D;

          param_t.st_conv_mode = stCtrl->convMode;

          CDBG("%s: new r_shift = %d\n", __func__,
            (uint32_t)param_t.final_right_shift);

          CDBG("%s: cropinfo in_w = %d, in_h = %d\n", __func__,
            input_frame.L.stCropInfo.in_w, input_frame.L.stCropInfo.in_h);
          CDBG("%s: cropinfo out_w = %d, out_h = %d\n", __func__,
            input_frame.L.stCropInfo.out_w, input_frame.L.stCropInfo.out_h);

          if (input_frame.buf_info.path == OUTPUT_TYPE_V) {
            dbg_dispatch_mode = DBG_DISPATCH_VIDEO;
            pstereoFrame = &stCtrl->videoFrame;
            c2d_format = C2D_COLOR_FORMAT_420_NV12;
          } else if (input_frame.buf_info.path == OUTPUT_TYPE_S) {
            dbg_dispatch_mode = DBG_DISPATCH_SNAP_S;
            pstereoFrame = &stCtrl->snapshotMainFrame;
            c2d_format = C2D_COLOR_FORMAT_420_NV21;

            if (!stereocam_get_correction_matrix(stCtrl, pstereoFrame)) {
              CDBG_HIGH("%s: stereocam_get_correction_matrix for "
                "snapshotMainFrame failed \n", __func__);
              stereocam_dispatch_exit = 1;
              continue;
            }
          } else if (input_frame.buf_info.path == OUTPUT_TYPE_T) {
            dbg_dispatch_mode = DBG_DISPATCH_SNAP_T;
            pstereoFrame = &stCtrl->snapshotThumbFrame;
            c2d_format = C2D_COLOR_FORMAT_420_NV21;

            if (!stereocam_get_correction_matrix(stCtrl, pstereoFrame)) {
              CDBG_HIGH("%s: stereocam_get_correction_matrix for "
                "snapshotThumbFrame failed \n", __func__);
              stereocam_dispatch_exit = 1;
              continue;
            }
          } else {
            CDBG_HIGH("%s: Invalid buffer path\n", __func__);
            break;
          }

          if (!init_c2d_buffer(cctrl, &c2d_op_frame_info, pstereoFrame,
            input_frame.buf_info.path))
            CDBG_HIGH("%s: init_c2d_buffer failed for bufPath = %d\n",
              __func__, input_frame.buf_info.path);

          vpe_done = 0;
          c2d_done = 0;

          get_local_dimension(pstereoFrame);
          update_geo_correction_matrix(pstereoFrame);

          prepare_left_frame(cctrl, &input_frame, &output_frame_L,
            pstereoFrame);

          if (write(cctrl->child_fd_set[STEREO_DISPATCH][PIPE_OUT][WRITE_END],
            &output_frame_L, sizeof(output_frame_L)) < 0)
            CDBG_HIGH("%s: Config thread wake up for L-Frame failed", __func__);
        } else if (input_frame.type == OUTPUT_TYPE_ST_L) {
          CDBG("%s: VPE is done processing L-Frame\n", __func__);
          vpe_done = 1;
        } else {
          CDBG_HIGH("%s: Invalid input_frame.type = %d\n", __func__,
            input_frame.type);
          break;
        }

        CDBG("%s: c2d_done %d vpe_done %d\n", __func__, c2d_done, vpe_done);
        if (!c2d_done) {
          output_frame_R = output_frame_L;
          output_frame_R.buf_info.y_off = c2d_op_frame_info.y_off;
          output_frame_R.buf_info.cbcr_off = c2d_op_frame_info.cbcr_off;
          output_frame_R.buf_info.fd = c2d_op_frame_info.fd;
          output_frame_R.buf_info.buffer =
            (unsigned long)c2d_op_frame_info.vaddr;
          output_frame_R.buf_info.stcam_quality_ind =
            stCtrl->lib3d.quality_indicator * 100;

          if (!prepare_right_frame(cctrl, &input_frame, &output_frame_R,
            pstereoFrame)) {
            CDBG_HIGH("%s: prepare_right_frame failed \n", __func__);
            stereocam_dispatch_exit = 1;
            continue;
          }

          c2d_done = 1;
        }

        if (vpe_done && c2d_done) {
          vpe_done = 0;
          c2d_done = 0;
          if (write(cctrl->child_fd_set[STEREO_DISPATCH][PIPE_OUT][WRITE_END],
            &output_frame_R, sizeof(output_frame_R)) < 0)
            CDBG_HIGH("%s: Config thread wake up failed", __func__);
        }
      }
    }
    CDBG("%s: ... Duty Loop End ...\n", __func__);
  } while (!stereocam_dispatch_exit);

  if (!release_c2d_buffer(cctrl, &c2d_op_frame_info))
    CDBG_HIGH("%s: release_c2d_buffer failed\n", __func__);

  dlclose(libc2d);

  if (st_dispatch_terminate_fd[0] >= 0)
    close(st_dispatch_terminate_fd[0]);
  if (st_dispatch_terminate_fd[1] >= 0)
    close(st_dispatch_terminate_fd[1]);

  CDBG("%s: EXIT\n", __func__);
  return NULL;
} /* stereocam_dispatch */
#endif
