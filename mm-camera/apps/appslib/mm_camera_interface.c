/*============================================================================

   Copyright (c) 2010-2011 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#include "camera_dbg.h"
#include "mm_camera_interface.h"
#include <errno.h>
#include "camera.h"
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include "snapshot.h"
#include <time.h>

#define SET_PARM_BIT32(parm, parm_arr) \
  (parm_arr[parm/32] |= (1<<(parm%32)))

#define GET_PARM_BIT32(parm, parm_arr) \
  ((parm_arr[parm/32]>>(parm%32))& 0x1)

typedef struct {
  int16_t *p_zoom_ratio;
  int32_t max_zoom;
}mm_camera_zoom_table_t;

typedef enum {
  MM_CAMERA_IDLE,
  MM_CAMERA_DEINIT,
  MM_CAMERA_INIT,
  MM_CAMERA_EXEC,
}mm_camera_state;

typedef struct {
  mm_camera_notify *notifyIntf;
  mm_camera_stream_mode_t mode;
  mm_camera_config *configIntf;
  mm_camera_ops* opsIntf;
  int controlFd;
  mm_camera_zoom_table_t zoom_ratio_table;
  cam_prop_t properties;
  struct msm_camera_info cameraInfo;
  mm_camera_state state;
  camera_info_t* p_cam_info;
  camera_mode_t current_mode;
  uint8_t cameraID;
  uint8_t dyn_device_query;
  void* snapshot_handle;
  void (*preview_frame_cb) (struct msm_frame *);
}mm_camera_intf_t;

static mm_camera_intf_t g_mm_camera_intf_obj;

/* Remove the table once all sensors support the get snapshot size function */
static struct camera_size_type default_picture_sizes[] = {
  { 4000, 3000}, // 12MP
  { 3200, 2400}, // 8MP
  { 2592, 1944}, // 5MP
  { 2048, 1536}, // 3MP QXGA
  { 1920, 1080}, //HD1080
  { 1600, 1200}, // 2MP UXGA
  { 1280, 768}, //WXGA
  { 1280, 720}, //HD720
  { 1024, 768}, // 1MP XGA
  { 800, 600}, //SVGA
  { 800, 480}, // WVGA
  { 640, 480}, // VGA
  { 352, 288}, //CIF
  { 320, 240}, // QVGA
  { 176, 144} // QCIF
};

static struct camera_size_type default_preview_sizes[] = {
  { 1920, 1080}, //1080p
  { 1280, 720}, // 720P, reserved
  { 800, 480}, // WVGA
  { 768, 432},
  { 720, 480},
  { 640, 480}, // VGA
  { 576, 432},
  { 480, 320}, // HVGA
  { 384, 288},
  { 352, 288}, // CIF
  { 320, 240}, // QVGA
  { 240, 160}, // SQVGA
  { 176, 144}, // QCIF
};

/* For 7x27 we dont use all the resolutions mentioned in the default
   preview size table even if the hardware and sensor supports that
   Hence adding temporary solution to use the following table */
static struct camera_size_type default_preview_sizes2[] = {
  { 864, 480}, // FWVGA for 7x27A
  { 800, 480}, // WVGA
  { 768, 432},
  { 720, 480},
  { 640, 480}, // VGA
  { 576, 432},
  { 480, 320}, // HVGA
  { 432, 240}, // WQVGA for 7x27A
  { 384, 288},
  { 352, 288}, // CIF
  { 320, 240}, // QVGA
  { 240, 160}, // SQVGA
  { 176, 144}, // QCIF
};

static struct camera_size_type default_hfr_sizes[] = {
  { 800, 480}, // WVGA
  { 640, 480} // VGA
};
static struct camera_size_type default_hfr_sizes2[] = {
  { 432, 240}, //WQVGA
  { 320, 240}  //QVGA
};

static struct camera_size_type default_3D_preview_sizes[] = {
  { 1280, 720} // 720P
};

/*===========================================================================
FUNCTION      get_device_id

DESCRIPTION
===========================================================================*/
uint8_t get_device_id()
{
  uint8_t camera_id = 0;
  if ((MM_CAMERA_IDLE != g_mm_camera_intf_obj.state) &&
    (MSM_MAX_CAMERA_SENSORS > g_mm_camera_intf_obj.cameraID)) {
    camera_id = g_mm_camera_intf_obj.cameraID;
  }
  CDBG("%s: device id = %d", __func__, camera_id);
  return camera_id;
}

/*===========================================================================
FUNCTION      get_notify_obj

DESCRIPTION
===========================================================================*/
mm_camera_notify* get_notify_obj()
{
  return g_mm_camera_intf_obj.notifyIntf;
}

/*===========================================================================
FUNCTION      camera_issue_ctrl_cmd

DESCRIPTION
===========================================================================*/
static mm_camera_status_t camera_issue_ctrl_cmd(cam_ctrl_type type,
  uint16_t length, void *value, uint32_t timeout_ms)
{
  struct msm_ctrl_cmd ctrlCmd;
  mm_camera_status_t status = MM_CAMERA_SUCCESS;
  int32_t ioctl_cmd = MSM_CAM_IOCTL_CTRL_COMMAND;
  int resp_fd = g_mm_camera_intf_obj.controlFd;
  int8_t is_blocking_cmd = TRUE;

  switch (type) {
    case CAMERA_AUTO_FOCUS_CANCEL:
    case CAMERA_STOP_SNAPSHOT:
      ioctl_cmd = MSM_CAM_IOCTL_CTRL_COMMAND_2;
      resp_fd = -1;
      is_blocking_cmd = FALSE;
      break;
    default:
      /* take default value */
      break;
  }

  ctrlCmd.timeout_ms = timeout_ms;
  ctrlCmd.type       = (uint16_t)type;
  ctrlCmd.length     = length;
  ctrlCmd.resp_fd    = resp_fd;
  ctrlCmd.value = value;

  if (ioctl(g_mm_camera_intf_obj.controlFd, ioctl_cmd,
    &ctrlCmd) < 0) {
    CDBG_HIGH("%s: error (%s): type %d, length %d, status %d",
      __FUNCTION__, strerror(errno), type, length, ctrlCmd.status);
    return MM_CAMERA_ERR_GENERAL;
  }
  CDBG("%s: succeeded type %d ctrl_status %d,", __func__, type,
    ctrlCmd.status);

  /* For non-blocking commands we dont expect kernel to fill the data,
   * hence checking the ioctl should be sufficient
   */
  if (!is_blocking_cmd) {
    return MM_CAMERA_SUCCESS;
  }

  switch (type) {
    case CAMERA_SET_PARM_AUTO_FOCUS:
      status = (ctrlCmd.status == CAMERA_EXIT_CB_DONE) ? MM_CAMERA_SUCCESS:
        MM_CAMERA_ERR_GENERAL;
      break;
    default: {
      if (CAM_CTRL_SUCCESS == ctrlCmd.status)
        status = MM_CAMERA_SUCCESS;
      else if (CAM_CTRL_INVALID_PARM == ctrlCmd.status)
        status = MM_CAMERA_ERR_INVALID_OPERATION;
      else if (CAM_CTRL_NOT_SUPPORTED == ctrlCmd.status)
        status = MM_CAMERA_ERR_NOT_SUPPORTED;
      else
        status = MM_CAMERA_ERR_GENERAL;
      break;
    }
  }
  return status;
}

/*===========================================================================
FUNCTION      camera_issue_command

DESCRIPTION
===========================================================================*/
static mm_camera_status_t camera_issue_command (int32_t ioctl_cmd,
  uint16_t length, void *value, uint32_t timeout_ms, char* name)
{
  struct msm_ctrl_cmd ctrlCmd;
  mm_camera_status_t status = MM_CAMERA_SUCCESS;

  ctrlCmd.timeout_ms = timeout_ms;
  ctrlCmd.length     = length;
  ctrlCmd.value = value;

  if (ioctl(g_mm_camera_intf_obj.controlFd, ioctl_cmd,
    &ctrlCmd) < 0) {
    CDBG_HIGH("%s: %s error (%s): length %d, status %d",
      __func__, name, strerror(errno), length, ctrlCmd.status);
    return MM_CAMERA_ERR_GENERAL;
  }
  return status;
}

/*===========================================================================
FUNCTION      camera_issue_command2

DESCRIPTION
===========================================================================*/
static mm_camera_status_t camera_issue_command2 (int32_t ioctl_cmd,
  void* value, char* name)
{
  mm_camera_status_t status = MM_CAMERA_SUCCESS;

  if (ioctl(g_mm_camera_intf_obj.controlFd, ioctl_cmd,
    value) < 0) {
    CDBG_HIGH("%s: %s error (%s)", __func__, name, strerror(errno));
    return MM_CAMERA_ERR_GENERAL;
  }
  return status;
}

/*===========================================================================
FUNCTION      handle_set_hist_params

DESCRIPTION
===========================================================================*/
static mm_camera_status_t handle_set_hist_params(int8_t histEnable)
{
  mm_camera_status_t status = MM_CAMERA_SUCCESS;
  CDBG("camera_set_parms CAMERA_PARM_HISTOGRAM %d", histEnable);
  if (histEnable) {
    if (!is_camstats_thread_running()) {
      if (FALSE == launch_camstats_thread()) {
        CDBG("launch_camstats_thread failed");
        return MM_CAMERA_ERR_INVALID_OPERATION;
      }
      status = camera_issue_ctrl_cmd(CAMERA_SET_PARM_HISTOGRAM,
        sizeof(int8_t), &histEnable, 5000);
      if (MM_CAMERA_SUCCESS != status) {
        CDBG("CAMERA_SET_PARM_HISTOGRAM failed");
        return MM_CAMERA_ERR_GENERAL;
      }
    }
  } else {
    status = camera_issue_ctrl_cmd(CAMERA_SET_PARM_HISTOGRAM,
      sizeof(int8_t), &histEnable, 5000);
    if (MM_CAMERA_SUCCESS != status) {
      CDBG("CAMERA_SET_PARM_HISTOGRAM failed");
      return MM_CAMERA_ERR_GENERAL;
    }
    release_camstats_thread();
  }
  return status;
}

/*===========================================================================
FUNCTION      mm_camera_get_camera_info

DESCRIPTION
===========================================================================*/
mm_camera_status_t mm_camera_get_camera_info(camera_info_t* p_cam_info,
  int* p_num_cameras)
{
  int i = 0;
  int controlfd = -1;
  char device[MAX_DEV_NAME_LEN];

  CDBG("%s: [S]\n", __func__);
  mm_camera_status_t status = MM_CAMERA_SUCCESS;
  struct msm_camera_info cameraInfo;

  if (NULL == p_cam_info) {
    status = MM_CAMERA_ERR_INVALID_INPUT;
    goto ERROR;
  }

  snprintf(device, MAX_DEV_NAME_LEN, MSM_CAMERA_CONTROL, 0);
  controlfd = open(device, O_RDWR);
  if (controlfd < 0) {
    CDBG_HIGH("%s: controlfd is invalid %s", __func__, strerror(errno));
    status = MM_CAMERA_ERR_GENERAL;
    goto ERROR;
  }

  if (ioctl(controlfd, MSM_CAM_IOCTL_GET_CAMERA_INFO, &cameraInfo) < 0) {
    CDBG_HIGH("%s: error (%s)", __func__, strerror(errno));
    status = MM_CAMERA_ERR_GENERAL;
    goto ERROR;
  }

  CDBG("%s: num %d\n", __func__, cameraInfo.num_cameras);
  for (i=0; i < cameraInfo.num_cameras; i++) {
    p_cam_info[i].camera_id = i;
    p_cam_info[i].modes_supported =
      (!cameraInfo.has_3d_support[i]) ? CAMERA_MODE_2D :
      CAMERA_MODE_2D | CAMERA_MODE_3D;
    p_cam_info[i].position =
      (cameraInfo.is_internal_cam[i]) ? FRONT_CAMERA : BACK_CAMERA;
    p_cam_info[i].sensor_mount_angle = cameraInfo.s_mount_angle[i];
  }
  *p_num_cameras = cameraInfo.num_cameras;
  ERROR:
  if (controlfd > 0) {
    close(controlfd);
  }
  CDBG("%s: %d [E]", __func__, status);
  return status;
}

/*===========================================================================
FUNCTION      camera_query_parms

DESCRIPTION
===========================================================================*/
mm_camera_status_t camera_query_parms(camera_parm_type_t parm_type,
  void** pp_value, uint32_t* p_count)
{
  CDBG("%s: %d", __func__, parm_type);
  mm_camera_status_t status = MM_CAMERA_SUCCESS;
  if (MM_CAMERA_EXEC != g_mm_camera_intf_obj.state) {
    if (CAMERA_PARM_CAMERA_INFO != parm_type) {
      CDBG("%s: failed", __func__);
      return MM_CAMERA_ERR_INVALID_OPERATION;
    }
  }
  switch (parm_type) {
    case CAMERA_PARM_PICT_SIZE: {
        struct camera_size_type* pict_size_table = default_picture_sizes;
        int pict_table_size = sizeof(default_picture_sizes)/
          sizeof(default_picture_sizes[0]);
        int i = 0;
        CDBG("%s: CAMERA_PARM_GET_PICT_SIZE_TABLE max_width %d max_height %d",
          __func__, g_mm_camera_intf_obj.properties.max_pict_width,
          g_mm_camera_intf_obj.properties.max_pict_height);
        for (i = 0; i < pict_table_size; i++) {
          if ((pict_size_table->width <=
            (int32_t)g_mm_camera_intf_obj.properties.max_pict_width) &&
            (pict_size_table->height <=
            (int32_t)g_mm_camera_intf_obj.properties.max_pict_height)) {
            CDBG("%s: CAMERA_PARM_GET_PICT_SIZE_TABLE width %d height %d",
              __func__, pict_size_table->width, pict_size_table->height);
            break;
          }
          pict_size_table++;
        }
        if (i >= pict_table_size) {
          CDBG("%s: CAMERA_PARM_GET_PICT_SIZE_TABLE failed", __func__);
          status = MM_CAMERA_ERR_GENERAL;
          break;
        }
        *pp_value = (void *)pict_size_table;
        *p_count = pict_table_size - i;
        CDBG("CAMERA_PARM_GET_PICT_SIZE_TABLE  send table %p %p count %d",
          (void *)default_picture_sizes, *pp_value, *p_count);
        break;
      }
    case CAMERA_PARM_ZOOM_RATIO: {
        if (NULL != g_mm_camera_intf_obj.zoom_ratio_table.p_zoom_ratio) {
          *pp_value = (void *)g_mm_camera_intf_obj.zoom_ratio_table.p_zoom_ratio;
          *p_count = g_mm_camera_intf_obj.zoom_ratio_table.max_zoom + 1;
          break;
        }
        status = camera_issue_ctrl_cmd(CAMERA_GET_PARM_MAXZOOM, sizeof(int32_t),
          &(g_mm_camera_intf_obj.zoom_ratio_table.max_zoom), 5000);
        CDBG("%s: CAMERA_PARM_ZOOM_RATIO max_zoom %d", __func__,
          g_mm_camera_intf_obj.zoom_ratio_table.max_zoom);
        if (MM_CAMERA_SUCCESS == status) {
          if (g_mm_camera_intf_obj.zoom_ratio_table.max_zoom <= 0) {
            status = MM_CAMERA_ERR_NOT_SUPPORTED;
            break;
          }
          g_mm_camera_intf_obj.zoom_ratio_table.p_zoom_ratio =
            (int16_t *)malloc(sizeof(int16_t) *
            (g_mm_camera_intf_obj.zoom_ratio_table.max_zoom + 1));
          if (!g_mm_camera_intf_obj.zoom_ratio_table.p_zoom_ratio) {
            CDBG("%s: CAMERA_PARM_ZOOM_RATIO failed", __func__);
            status = MM_CAMERA_ERR_NO_MEMORY;
            break;
          }
          status = camera_issue_ctrl_cmd(CAMERA_GET_PARM_ZOOMRATIOS,
            sizeof(int16_t) * (g_mm_camera_intf_obj.zoom_ratio_table.max_zoom
            + 1), g_mm_camera_intf_obj.zoom_ratio_table.p_zoom_ratio, 5000);
        } else
          break;
        if (MM_CAMERA_SUCCESS == status) {
          *pp_value = (void *)g_mm_camera_intf_obj.zoom_ratio_table.p_zoom_ratio;
          *p_count = g_mm_camera_intf_obj.zoom_ratio_table.max_zoom + 1;
          CDBG("%s: CAMERA_PARM_ZOOM_RATIO SUCCESS", __func__);
        }
        break;
      }
    case CAMERA_PARM_CAMERA_INFO: {
        if (!(g_mm_camera_intf_obj.dyn_device_query)) {
          CDBG("%s: CAMERA_PARM_CAMERA_INFO failed", __func__);
          return MM_CAMERA_ERR_INVALID_OPERATION;
        }
        if (NULL != g_mm_camera_intf_obj.p_cam_info) {
          *pp_value = (void *)g_mm_camera_intf_obj.p_cam_info;
          *p_count = g_mm_camera_intf_obj.cameraInfo.num_cameras;
          break;
        }
        CDBG("%s: CAMERA_PARM_CAMERA_INFO num %d", __func__,
          g_mm_camera_intf_obj.cameraInfo.num_cameras);
        g_mm_camera_intf_obj.p_cam_info = (camera_info_t *)malloc(
          g_mm_camera_intf_obj.cameraInfo.num_cameras * sizeof(camera_info_t));
        if (!g_mm_camera_intf_obj.p_cam_info) {
          CDBG("%s: CAMERA_PARM_CAMERA_INFO failed", __func__);
          status = MM_CAMERA_ERR_NO_MEMORY;
          break;
        }
        int i = 0;
        for (i=0; i<g_mm_camera_intf_obj.cameraInfo.num_cameras; i++) {
          g_mm_camera_intf_obj.p_cam_info[i].camera_id = i;
          g_mm_camera_intf_obj.p_cam_info[i].modes_supported =
            (!g_mm_camera_intf_obj.cameraInfo.has_3d_support[i]) ? CAMERA_MODE_2D :
            CAMERA_MODE_2D | CAMERA_MODE_3D;
          g_mm_camera_intf_obj.p_cam_info[i].position =
            (g_mm_camera_intf_obj.cameraInfo.is_internal_cam[i]) ? FRONT_CAMERA :
            BACK_CAMERA;
        }
        *pp_value = (void *)g_mm_camera_intf_obj.p_cam_info;
        *p_count = g_mm_camera_intf_obj.cameraInfo.num_cameras;
        status = MM_CAMERA_SUCCESS;
        break;
      }
    case CAMERA_PARM_PREVIEW_SIZE: {
#ifndef USE_PREVIEW_TABLE2
        struct camera_size_type* preview_size_table = default_preview_sizes;
        int preview_table_size = sizeof(default_preview_sizes)/
          sizeof(default_preview_sizes[0]);
        if (CAMERA_MODE_3D == g_mm_camera_intf_obj.current_mode) {
            preview_size_table = default_3D_preview_sizes;
            preview_table_size = sizeof(default_3D_preview_sizes)/
              sizeof(default_3D_preview_sizes[0]);
        }
#else
        struct camera_size_type* preview_size_table = default_preview_sizes2;
        int preview_table_size = sizeof(default_preview_sizes2)/
          sizeof(default_preview_sizes2[0]);
#endif
        int i = 0;
        CDBG("%s: CAMERA_PARM_PREVIEW_SIZE_TABLE max_width %d max_height %d",
          __func__, g_mm_camera_intf_obj.properties.max_preview_width,
          g_mm_camera_intf_obj.properties.max_preview_height);
        for (i = 0; i < preview_table_size; i++) {
          if ((preview_size_table->width <=
            (int32_t)g_mm_camera_intf_obj.properties.max_preview_width) &&
            (preview_size_table->height <=
            (int32_t)g_mm_camera_intf_obj.properties.max_preview_height)) {
            CDBG("%s: CAMERA_PARM_PREVIEW_SIZE_TABLE width %d height %d",
              __func__, preview_size_table->width, preview_size_table->height);
            break;
          }
          preview_size_table++;
        }
        *pp_value = (void *)preview_size_table;
        *p_count = preview_table_size - i;
        CDBG("CAMERA_PARM_PREVIEW_SIZE_TABLE  send table %p %p count %d",
          (void *)preview_size_table, *pp_value, *p_count);
        break;
      }
    case CAMERA_PARM_HFR_SIZE: {
#ifndef USE_HFR_TABLE2
        struct camera_size_type* hfr_size_table = default_hfr_sizes;
        int hfr_table_size = sizeof(default_hfr_sizes)/
          sizeof(default_hfr_sizes[0]);
#else
        struct camera_size_type* hfr_size_table = default_hfr_sizes2;
        int hfr_table_size = sizeof(default_hfr_sizes)/
          sizeof(default_hfr_sizes[0]);
#endif
        int i = 0;
        CDBG("%s: CAMERA_PARM_HFR_SIZE_TABLE max_width %d max_height %d",
          __func__, g_mm_camera_intf_obj.properties.max_preview_width,
          g_mm_camera_intf_obj.properties.max_preview_height);
        for (i = 0; i < hfr_table_size; i++) {
          if ((hfr_size_table->width <=
            (int32_t)g_mm_camera_intf_obj.properties.max_preview_width) &&
            (hfr_size_table->height <=
            (int32_t)g_mm_camera_intf_obj.properties.max_preview_height)) {
            CDBG("%s: CAMERA_PARM_HFR_SIZE_TABLE width %d height %d",
              __func__, hfr_size_table->width, hfr_size_table->height);
            break;
          }
          hfr_size_table++;
        }
        *pp_value = (void *)hfr_size_table;
        *p_count = hfr_table_size - i;
        CDBG("CAMERA_PARM_HFR_SIZE_TABLE  send table %p %p count %d",
          (void *)hfr_size_table, *pp_value, *p_count);
         break;
    }
    default:
      status = MM_CAMERA_ERR_NOT_SUPPORTED;
      break;
  }
  return status;
}

/*===========================================================================
FUNCTION      camera_set_parms

DESCRIPTION
===========================================================================*/
mm_camera_status_t camera_set_parms(camera_parm_type_t parm_type,
  void* p_value)
{
  CDBG("%s: %d", __func__, parm_type);
  mm_camera_status_t status = MM_CAMERA_SUCCESS;

  switch (parm_type) {
    case CAMERA_PARM_HISTOGRAM: {
        status = handle_set_hist_params(*((int *)p_value));
      }
      break;
    case CAMERA_PARM_DIMENSION: {
        status = camera_issue_ctrl_cmd(CAMERA_SET_PARM_DIMENSION,
          sizeof(cam_ctrl_dimension_t), p_value, 5000);
        break;
      }
    case CAMERA_PARM_FPS: {
        status = camera_issue_ctrl_cmd(CAMERA_SET_PARM_FPS,
          sizeof(uint16_t), p_value, 5000);
        break;
      }
    case CAMERA_PARM_FPS_MODE: {
        status = camera_issue_ctrl_cmd(CAMERA_SET_FPS_MODE,
          sizeof(int32_t), p_value, 5000);
        break;
      }
    case CAMERA_PARM_EFFECT: {
        status = camera_issue_ctrl_cmd(CAMERA_SET_PARM_EFFECT,
          sizeof(int32_t), p_value, 5000);
        break;
      }
    case CAMERA_PARM_EXPOSURE_COMPENSATION: {
        status = camera_issue_ctrl_cmd(CAMERA_SET_PARM_EXPOSURE_COMPENSATION,
          sizeof(int32_t), p_value, 5000);
        break;
      }
    case CAMERA_PARM_EXPOSURE: {
        status = camera_issue_ctrl_cmd(CAMERA_SET_PARM_EXPOSURE,
          sizeof(int32_t), p_value, 5000);
        break;
      }
    case CAMERA_PARM_SHARPNESS: {
        status = camera_issue_ctrl_cmd(CAMERA_SET_PARM_SHARPNESS,
          sizeof(int32_t), p_value, 5000);
        break;
      }
    case CAMERA_PARM_CONTRAST: {
        status = camera_issue_ctrl_cmd(CAMERA_SET_PARM_CONTRAST,
          sizeof(int32_t), p_value, 5000);
        break;
      }
    case CAMERA_PARM_SATURATION: {
        status = camera_issue_ctrl_cmd(CAMERA_SET_PARM_SATURATION,
          sizeof(int32_t), p_value, 5000);
        break;
      }
    case CAMERA_PARM_BRIGHTNESS: {
        status = camera_issue_ctrl_cmd(CAMERA_SET_PARM_BRIGHTNESS,
          sizeof(int32_t), p_value, 5000);
        break;
      }
    case CAMERA_PARM_WHITE_BALANCE: {
        status = camera_issue_ctrl_cmd(CAMERA_SET_PARM_WB,
          sizeof(int32_t), p_value, 5000);
        break;
      }
    case CAMERA_PARM_LED_MODE: {
        status = camera_issue_ctrl_cmd(CAMERA_SET_PARM_LED_MODE,
          sizeof(int32_t), p_value, 5000);
        break;
      }
    case CAMERA_PARM_ANTIBANDING: {
        camera_antibanding_type ab_type;
        ab_type = *((camera_antibanding_type *)p_value);
        CDBG("%s: antibanding %d", __func__, ab_type);
        if (CAMERA_ANTIBANDING_AUTO == ab_type)
          status = camera_issue_ctrl_cmd(CAMERA_ENABLE_AFD,
            0, NULL, 5000);
        else
          status = camera_issue_ctrl_cmd(CAMERA_SET_PARM_ANTIBANDING,
            sizeof(int32_t), p_value, 5000);
        break;
      }
    case CAMERA_PARM_ROLLOFF: {
        status = camera_issue_ctrl_cmd(CAMERA_SET_PARM_ROLLOFF,
          sizeof(int32_t), p_value, 5000);
        break;
      }
    case CAMERA_PARM_CONTINUOUS_AF: {
        status = camera_issue_ctrl_cmd(CAMERA_SET_CAF,
          sizeof(int32_t), p_value, 5000);
        break;
      }
    case CAMERA_PARM_FOCUS_RECT: {
        status = camera_issue_ctrl_cmd(CAMERA_SET_PARM_FOCUS_RECT,
          sizeof(int32_t), p_value, 5000);
        break;
      }
    case CAMERA_PARM_AEC_MTR_AREA: {
      status = camera_issue_ctrl_cmd(CAMERA_SET_AEC_MTR_AREA,
        sizeof(cam_set_aec_roi_t), p_value, 5000);
      break;
    }
    case CAMERA_PARM_AEC_ROI: {
        status = camera_issue_ctrl_cmd(CAMERA_SET_PARM_AEC_ROI,
          sizeof(cam_set_aec_roi_t), p_value, 5000);
        break;
      }
    case CAMERA_PARM_AF_ROI: {
        status = camera_issue_ctrl_cmd(CAMERA_SET_PARM_AF_ROI,
          sizeof(roi_info_t), p_value, 5000);
        break;
      }
    case CAMERA_PARM_HJR: {
        status = camera_issue_ctrl_cmd(CAMERA_SET_PARM_HJR,
          sizeof(int8_t), p_value, 5000);
        break;
      }
    case CAMERA_PARM_ISO: {
        status = camera_issue_ctrl_cmd(CAMERA_SET_PARM_ISO,
          sizeof(camera_iso_mode_type), p_value, 5000);
        break;
      }
    case CAMERA_PARM_BL_DETECTION: {
        status = camera_issue_ctrl_cmd(CAMERA_SET_PARM_BL_DETECTION_ENABLE,
          sizeof(int32_t), p_value, 5000);
        break;
      }
    case CAMERA_PARM_SNOW_DETECTION: {
        status = camera_issue_ctrl_cmd(CAMERA_SET_PARM_SNOW_DETECTION_ENABLE,
          sizeof(int32_t), p_value, 5000);
        break;
      }
    case CAMERA_PARM_BESTSHOT_MODE: {
        status = camera_issue_ctrl_cmd(CAMERA_SET_PARM_BESTSHOT_MODE,
          sizeof(int32_t), p_value, 5000);
        break;
      }
    case CAMERA_PARM_ZOOM: {
        status = camera_issue_ctrl_cmd(CAMERA_SET_PARM_ZOOM,
          sizeof(int32_t), p_value, 5000);
        break;
      }
    case CAMERA_PARM_VIDEO_DIS: {
        status = camera_issue_ctrl_cmd(CAMERA_SET_VIDEO_DIS_PARAMS,
          sizeof(video_dis_param_ctrl_t), p_value, 5000);
        break;
      }
    case CAMERA_PARM_VIDEO_ROT: {
        status = camera_issue_ctrl_cmd(CAMERA_SET_VIDEO_ROT_PARAMS,
          sizeof(video_rotation_param_ctrl_t), p_value, 5000);
        break;
      }
    case CAMERA_PARM_SCE_FACTOR: {
        status = camera_issue_ctrl_cmd(CAMERA_SET_SCE_FACTOR,
          sizeof(int32_t), p_value, 5000);
        break;
      }
    case CAMERA_PARM_WAVELET_DENOISE: {
        status = camera_issue_ctrl_cmd(CAMERA_SET_PARM_WAVELET_DENOISE,
          sizeof(int32_t), p_value, 5000);
        break;
      }
    case CAMERA_PARM_MCE: {
        status = camera_issue_ctrl_cmd(CAMERA_SET_PARM_MCE,
          sizeof(int32_t), p_value, 5000);
        break;
      }
    case CAMERA_PARM_RESET_LENS_TO_INFINITY: {
        status = camera_issue_ctrl_cmd(
                CAMERA_SET_PARM_RESET_LENS_TO_INFINITY,
                0, NULL, 5000);
        break;
      }
    case CAMERA_PARM_HFR: {
        status = camera_issue_ctrl_cmd(CAMERA_SET_PARM_HFR,
          sizeof(int32_t), p_value, 5000);
        break;
      }
    case CAMERA_PARM_HDR: {
        status = camera_issue_ctrl_cmd(CAMERA_SET_PARM_HDR,
          sizeof(exp_bracketing_t), p_value, 5000);
        break;
      }
    case CAMERA_PARM_FD: {
        status = camera_issue_ctrl_cmd(CAMERA_SET_PARM_FD,
          sizeof(int32_t), p_value, 5000);
        break;
      }
    case CAMERA_PARM_MODE: {
        g_mm_camera_intf_obj.current_mode = *((camera_mode_t *)p_value);
        status = MM_CAMERA_SUCCESS;
        break;
      }
    case CAMERA_PARM_CAMERA_ID: {
      g_mm_camera_intf_obj.cameraID = *((uint8_t *)p_value);
      status = MM_CAMERA_SUCCESS;
      break;
    }
    case CAMERA_PARM_JPEG_ROTATION: {
#ifndef DISABLE_JPEG_ENCODING
      jpeg_encoder_setRotation(*((int *)p_value));
#endif /* DISABLE_JPEG_ENCODING */
      status = MM_CAMERA_SUCCESS;
      break;
    }
    case CAMERA_PARM_JPEG_MAINIMG_QUALITY: {
#ifndef DISABLE_JPEG_ENCODING
      jpeg_encoder_setMainImageQuality(*((uint32_t *)p_value));
#endif /* DISABLE_JPEG_ENCODING */
      status = MM_CAMERA_SUCCESS;
      break;
    }
    case CAMERA_PARM_JPEG_THUMB_QUALITY: {
#ifndef DISABLE_JPEG_ENCODING
      jpeg_encoder_setThumbnailQuality(*((uint32_t *)p_value));
#endif /* DISABLE_JPEG_ENCODING */
      status = MM_CAMERA_SUCCESS;
      break;
    }
    case CAMERA_PARM_ZSL_ENABLE: {
      status = camera_issue_ctrl_cmd(CAMERA_ENABLE_ZSL,
        sizeof(uint8_t), p_value, 5000);
      break;
    }
    case CAMERA_PARM_REDEYE_REDUCTION: {
      status = camera_issue_ctrl_cmd(CAMERA_SET_REDEYE_REDUCTION,
        sizeof(int32_t), p_value, 5000);
      break;
    }
    case CAMERA_PARM_3D_DISPLAY_DISTANCE: {
      status = camera_issue_ctrl_cmd(CAMERA_SET_PARM_3D_DISPLAY_DISTANCE,
        sizeof(float), p_value, 5000);
      break;
    }
    case CAMERA_PARM_3D_VIEW_ANGLE: {
      status = camera_issue_ctrl_cmd(CAMERA_SET_PARM_3D_VIEW_ANGLE,
        sizeof(uint32_t), p_value, 5000);
      break;
    }
    case CAMERA_PARM_3D_EFFECT: {
      status = camera_issue_ctrl_cmd(CAMERA_SET_PARM_3D_EFFECT,
        sizeof(uint32_t), p_value, 5000);
      break;
    }
    case CAMERA_PARM_PREVIEW_FORMAT: {
      status = camera_issue_ctrl_cmd(CAMERA_SET_PARM_PREVIEW_FORMAT,
        sizeof(cam_format_t), p_value, 5000);
      break;
    }
    case CAMERA_PARM_3D_MANUAL_CONV_VALUE: {
      status = camera_issue_ctrl_cmd(CAMERA_SET_PARM_3D_MANUAL_CONV_VALUE,
        sizeof(uint32_t), p_value, 5000);
      break;
    }
    case CAMERA_PARM_ENABLE_3D_MANUAL_CONVERGENCE: {
      status = camera_issue_ctrl_cmd(CAMERA_ENABLE_3D_MANUAL_CONVERGENCE,
        sizeof(uint32_t), p_value, 5000);
      break;
    }
    case CAMERA_PARM_AEC_LOCK: {
      status = camera_issue_ctrl_cmd(CAMERA_SET_AEC_LOCK,
        sizeof(uint32_t), p_value, 5000);
      break;
    }
    case CAMERA_PARM_AWB_LOCK: {
      status = camera_issue_ctrl_cmd(CAMERA_SET_AWB_LOCK,
        sizeof(uint32_t), p_value, 5000);
      break;
    }
    case CAMERA_PARM_RECORDING_HINT: {
      status = camera_issue_ctrl_cmd(CAMERA_SET_RECORDING_HINT,
        sizeof(uint32_t), p_value, 5000);
      break;
    }
    default:
      status = MM_CAMERA_ERR_NOT_SUPPORTED;
      break;

  }
  CDBG("%s: Exit", __func__);
  return status;
}

/*===========================================================================
FUNCTION      camera_is_parm_supported

DESCRIPTION
===========================================================================*/
int8_t camera_is_parm_supported (camera_parm_type_t parm_type)
{
  mm_camera_status_t status = MM_CAMERA_SUCCESS;
  int index = parm_type/32; /* 32 bits*/
  int is_parm_supported = GET_PARM_BIT32(parm_type,
    g_mm_camera_intf_obj.properties.parm);
  CDBG("%s: parm_type %d index %d is_supported %d", __func__, parm_type,
    index, is_parm_supported);
  return is_parm_supported;
}

/*===========================================================================
FUNCTION      camera_is_subparm_supported

DESCRIPTION
===========================================================================*/
int8_t camera_is_subparm_supported(camera_parm_type_t parm_type,
  void* sub_parm)
{
  mm_camera_status_t status = MM_CAMERA_SUCCESS;
  int is_subparm_supported = 0;
  switch (parm_type) {
    case CAMERA_PARM_EFFECT: {
        uint32_t value = *((uint32_t *)sub_parm);
        is_subparm_supported = ((g_mm_camera_intf_obj.properties.effect &
          (1<<value)) != 0);
        CDBG("%s: value %d", __func__, value);
        break;
      }
    default:;
  }
  CDBG("%s: parm_type %d is_supported %d", __func__, parm_type,
    is_subparm_supported);
  return is_subparm_supported;
}

/*===========================================================================
FUNCTION      camera_is_ops_supported

DESCRIPTION
===========================================================================*/
int8_t camera_is_ops_supported (mm_camera_ops_type_t ops_type)
{
  mm_camera_status_t status = MM_CAMERA_SUCCESS;
  int index = ops_type/32; /* 32 bits*/
  int is_ops_supported = ((g_mm_camera_intf_obj.properties.ops[index] &
    (1<<ops_type)) != 0);
  CDBG("%s: ops_type %d index %d is_supported %d", __func__, ops_type,
    index, is_ops_supported);
  return is_ops_supported;
}

/*===========================================================================
FUNCTION      camera_get_parms

DESCRIPTION
===========================================================================*/
mm_camera_status_t camera_get_parms(camera_parm_type_t parm_type,
  void* p_value)
{
  mm_camera_status_t status = MM_CAMERA_SUCCESS;
  CDBG("%s: %d", __func__, parm_type);
  switch (parm_type) {
    case CAMERA_PARM_QUERY_FALSH4SNAP: {
        status = camera_issue_ctrl_cmd(CAMERA_QUERY_FLASH_FOR_SNAPSHOT,
          sizeof(int), p_value, 1000);
        break;
      }

    case CAMERA_PARM_FOCUS_DISTANCES: {
        status = camera_issue_ctrl_cmd(CAMERA_GET_PARM_FOCUS_DISTANCES,
          sizeof(focus_distances_info_t), p_value, 5000);
        break;
      }

    case CAMERA_PARM_FOCAL_LENGTH: {
        status = camera_issue_ctrl_cmd(CAMERA_GET_PARM_FOCAL_LENGTH,
          sizeof(focus_distances_info_t), p_value, 5000);
        break;
      }

    case CAMERA_PARM_HORIZONTAL_VIEW_ANGLE: {
        status = camera_issue_ctrl_cmd(CAMERA_GET_PARM_HORIZONTAL_VIEW_ANGLE,
          sizeof(focus_distances_info_t), p_value, 5000);
        break;
      }

    case CAMERA_PARM_VERTICAL_VIEW_ANGLE: {
        status = camera_issue_ctrl_cmd(CAMERA_GET_PARM_VERTICAL_VIEW_ANGLE,
          sizeof(focus_distances_info_t), p_value, 5000);
        break;
      }

    case CAMERA_PARM_BUFFER_INFO: {
      status = MM_CAMERA_SUCCESS;
      cam_buf_info_t* buf_info = (cam_buf_info_t *)p_value;
#ifndef DISABLE_JPEG_ENCODING
      jpeg_encoder_get_buffer_offset(buf_info->resolution.width,
        buf_info->resolution.height,
        &buf_info->yoffset, &buf_info->cbcr_offset,
        &buf_info->size);
#endif /* DISABLE_JPEG_ENCODING */
      status = MM_CAMERA_SUCCESS;
      break;
    }

    case CAMERA_PARM_SNAPSHOTDATA: {
        status = camera_issue_ctrl_cmd(CAMERA_GET_PARM_SNAPSHOTDATA,
          sizeof(snapshotData_info_t), p_value, 5000);
        break;
      }

    case CAMERA_PARM_3D_FRAME_FORMAT: {
        status = camera_issue_ctrl_cmd(CAMERA_GET_PARM_3D_FRAME_FORMAT,
          sizeof(camera_3d_frame_t), p_value, 5000);
        break;
      }

    case CAMERA_PARM_3D_DISPLAY_DISTANCE: {
      status = camera_issue_ctrl_cmd(CAMERA_GET_PARM_3D_DISPLAY_DISTANCE,
        sizeof(uint32_t), p_value, 5000);
      break;
    }

    case CAMERA_PARM_3D_VIEW_ANGLE: {
      status = camera_issue_ctrl_cmd(CAMERA_GET_PARM_3D_VIEW_ANGLE,
        sizeof(uint32_t), p_value, 5000);
      break;
    }

    case CAMERA_PARM_3D_EFFECT: {
      status = camera_issue_ctrl_cmd(CAMERA_GET_PARM_3D_EFFECT,
        sizeof(uint32_t), p_value, 5000);
      break;
    }

    case CAMERA_PARM_3D_MANUAL_CONV_RANGE: {
      status = camera_issue_ctrl_cmd(CAMERA_GET_PARM_3D_MANUAL_CONV_RANGE,
        sizeof(uint32_t), p_value, 5000);
      break;
    }

    default:
      status = MM_CAMERA_ERR_NOT_SUPPORTED;
      break;

  }
  return status;
}

/*===========================================================================
FUNCTION      camera_ops_start

DESCRIPTION
===========================================================================*/
mm_camera_status_t camera_ops_start (mm_camera_ops_type_t ops_type,
  void* parm1, void* parm2)
{

  mm_camera_status_t status = MM_CAMERA_ERR_NOT_SUPPORTED;
  CDBG("%s %d enter", __func__, ops_type);
  switch (ops_type) {
    case CAMERA_OPS_STREAMING_PREVIEW: {
      status = camera_issue_ctrl_cmd(CAMERA_START_PREVIEW, 0, NULL, 5000);
      if (MM_CAMERA_SUCCESS == status) {
        g_mm_camera_intf_obj.mode = CAMERA_MODE_STREAM_VIDEO;
      }
    }
    break;
    case CAMERA_OPS_STREAMING_VIDEO: {
      status = camera_issue_ctrl_cmd(CAMERA_START_VIDEO, 0, NULL, 5000);
      if ((MM_CAMERA_SUCCESS == status) &&
        (CAMERA_MODE_3D == g_mm_camera_intf_obj.current_mode)) {
        status = camera_issue_ctrl_cmd(CAMERA_START_RECORDING,
          0, NULL, 1000);
      }
      if (MM_CAMERA_SUCCESS == status) {
        g_mm_camera_intf_obj.mode = CAMERA_MODE_STREAM_VIDEO;
      }
    }
    break;
    case CAMERA_OPS_STREAMING_ZSL: {
      status = camera_issue_ctrl_cmd(CAMERA_START_ZSL, 0, NULL, 5000);
      CDBG("%s, status = %d\n", __func__, status);
      if(MM_CAMERA_SUCCESS == status) {
          g_mm_camera_intf_obj.mode = CAMERA_MODE_STREAM_ZSL;
      }
    }
    break;
    case CAMERA_OPS_FOCUS: {
        mmcamera_util_profile("Camera_interface AF ");
        status = camera_issue_ctrl_cmd(CAMERA_SET_PARM_AUTO_FOCUS,
          sizeof(isp3a_af_mode_t), parm1, 5000);
        mmcamera_util_profile("Camera_interface post AF ");
        break;
      }
    case CAMERA_OPS_GET_PICTURE: {
        status = camera_issue_command(MSM_CAM_IOCTL_GET_PICTURE,
          sizeof(common_crop_t), parm1, 5000, "get_picture");
        break;
      }
    case CAMERA_OPS_PREPARE_SNAPSHOT: {
      mmcamera_util_profile("Camera_interface pre_prepare snap ");
        if (camera_is_ops_supported(CAMERA_OPS_PREPARE_SNAPSHOT)) {
          status = camera_issue_ctrl_cmd(CAMERA_PREPARE_SNAPSHOT,
            0, NULL, 1000);
        } else {
          /*return success if not supported*/
          status = MM_CAMERA_SUCCESS;
        }
        mmcamera_util_profile("Camera_interface post prepare_snap");
        g_mm_camera_intf_obj.mode = CAMERA_MODE_STREAM_VIDEO;
        break;
      }
    case CAMERA_OPS_SNAPSHOT: {
      mmcamera_util_profile("Camera_interface post snap ");
        status = camera_issue_ctrl_cmd(CAMERA_START_SNAPSHOT,
          0, NULL, 5000);
        if (MM_CAMERA_SUCCESS == status) {
          g_mm_camera_intf_obj.mode = CAMERA_MODE_STREAM_VIDEO;
        }
        mmcamera_util_profile("Camera_interface post snap ");
        break;
      }
    case CAMERA_OPS_LIVESHOT: {
        status = camera_issue_ctrl_cmd(CAMERA_START_LIVESHOT,
          0, NULL, 5000);
        break;
      }
    case CAMERA_OPS_RAW_SNAPSHOT: {
        status = camera_issue_ctrl_cmd(CAMERA_START_RAW_SNAPSHOT,
          0, NULL, 5000);
        break;
      }
    case CAMERA_OPS_VIDEO_RECORDING: {
        status = camera_issue_ctrl_cmd(CAMERA_START_RECORDING,
          0, NULL, 1000);
        break;
      }
    case CAMERA_OPS_REGISTER_BUFFER: {
      struct msm_pmem_info* pmemBuf = (struct msm_pmem_info*)parm1;
      if (CAMERA_MODE_3D == g_mm_camera_intf_obj.current_mode) {
        CDBG("%s 3d mode register pmemtype %d", __func__, pmemBuf->type);
        switch(pmemBuf->type) {
          case MSM_PMEM_MAINIMG:
            pmemBuf->type = MSM_PMEM_MAINIMG_VPE;
            break;
          case MSM_PMEM_THUMBNAIL:
            pmemBuf->type = MSM_PMEM_THUMBNAIL_VPE;
            break;
        }
      }
      if (g_mm_camera_intf_obj.snapshot_handle) {
        if ( (MSM_PMEM_MAINIMG == pmemBuf->type)
          || (MSM_PMEM_RAW_MAINIMG == pmemBuf->type)
          || (MSM_PMEM_THUMBNAIL == pmemBuf->type) ) {
          status = snapshot_add_buffers(g_mm_camera_intf_obj.snapshot_handle,
            pmemBuf);
          if (status)
            break;
        }
      }
      status = camera_issue_command2(MSM_CAM_IOCTL_REGISTER_PMEM,
        pmemBuf, "register_buffers");
      break;
    }
    case CAMERA_OPS_UNREGISTER_BUFFER: {
      if (CAMERA_MODE_3D == g_mm_camera_intf_obj.current_mode) {
        struct msm_pmem_info* p_pmemBuf = (struct msm_pmem_info*)parm1;
        CDBG("%s 3d mode unregister pmemtype %d", __func__, p_pmemBuf->type);
        switch(p_pmemBuf->type) {
          case MSM_PMEM_MAINIMG:
            p_pmemBuf->type = MSM_PMEM_MAINIMG_VPE;
          break;
          case MSM_PMEM_THUMBNAIL:
            p_pmemBuf->type = MSM_PMEM_THUMBNAIL_VPE;
          break;
        }
      }
      status = camera_issue_command2(MSM_CAM_IOCTL_UNREGISTER_PMEM,
        parm1, "unregister_buffers");
      break;
    }
    case CAMERA_OPS_RAW_CAPTURE: {
      raw_capture_params_t* p_raw_capture_params = (raw_capture_params_t*)parm1;
      status = snapshot_set_raw_capture_parms(
        g_mm_camera_intf_obj.snapshot_handle,
        p_raw_capture_params);
      if( status ) break;
      status = snapshot_start(g_mm_camera_intf_obj.snapshot_handle);
      break;
    }
    case CAMERA_OPS_CAPTURE_AND_ENCODE: {
#ifndef DISABLE_JPEG_ENCODING
      encode_params_t* p_encode_params = (encode_params_t *)parm2;
#endif /* DISABLE_JPEG_ENCODING */
      CDBG_HIGH("%s, CAMERA_OPS_CAPTURE_AND_ENCODE mode %d\n", __func__,
        g_mm_camera_intf_obj.mode);
      if (g_mm_camera_intf_obj.mode == CAMERA_MODE_STREAM_VIDEO ) {
        capture_params_t* p_capture_params = (capture_params_t*)parm1;
        status = snapshot_set_capture_parms(
          g_mm_camera_intf_obj.snapshot_handle,
          p_capture_params);
        if( status ) break;
      } else {
        zsl_capture_params_t* p_zsl_capture_params =
          (zsl_capture_params_t*)parm1;
        status = snapshot_set_zsl_capture_parms(
          g_mm_camera_intf_obj.snapshot_handle,
          p_zsl_capture_params, SNAPSHOT_TYPE_CAPTURE_AND_ENCODE);
        if( status ) break;
      }
#ifndef DISABLE_JPEG_ENCODING
      status = snapshot_set_encode_parms(g_mm_camera_intf_obj.snapshot_handle,
        p_encode_params);
      if( status ) break;
#endif /* DISABLE_JPEG_ENCODING */
      status = snapshot_start(g_mm_camera_intf_obj.snapshot_handle);
      break;
    }
    case CAMERA_OPS_CAPTURE: {
      if (g_mm_camera_intf_obj.mode == CAMERA_MODE_STREAM_VIDEO ) {
        capture_params_t* p_capture_params = (capture_params_t*)parm1;
        status = snapshot_set_capture_parms(
          g_mm_camera_intf_obj.snapshot_handle,
          p_capture_params);
        if( status ) break;
      } else {
        zsl_capture_params_t* p_zsl_capture_params =
          (zsl_capture_params_t*)parm1;
        status = snapshot_set_zsl_capture_parms(
          g_mm_camera_intf_obj.snapshot_handle,
          p_zsl_capture_params, SNAPSHOT_TYPE_CAPTURE_ONLY);
      }
      status = snapshot_start(g_mm_camera_intf_obj.snapshot_handle);
      break;
    }
    case CAMERA_OPS_ENCODE: {
#ifndef DISABLE_JPEG_ENCODING
      encode_params_t* p_encode_params = (encode_params_t *)parm1;
      status = snapshot_set_encode_parms(g_mm_camera_intf_obj.snapshot_handle,
        p_encode_params);
      if( status ) break;
      status = snapshot_start_encode(g_mm_camera_intf_obj.snapshot_handle);
#endif /* DISABLE_JPEG_ENCODING */
     break;
    }
    case CAMERA_OPS_ZSL_STREAMING_CB: {
      if (g_mm_camera_intf_obj.mode == CAMERA_MODE_STREAM_ZSL) {
        g_mm_camera_intf_obj.notifyIntf->preview_frame_cb =
          g_mm_camera_intf_obj.preview_frame_cb;
        status = MM_CAMERA_SUCCESS;
      }
      break;
     }
    default:
      break;
  }
  CDBG("%s %d exit", __func__, ops_type);
  return status;
}

/*===========================================================================
FUNCTION      camera_ops_stop

DESCRIPTION
===========================================================================*/
mm_camera_status_t camera_ops_stop (mm_camera_ops_type_t ops_type,
  void* parm1, void* parm2)
{
  mm_camera_status_t status = MM_CAMERA_ERR_NOT_SUPPORTED;
  CDBG("%s %d enter", __func__, ops_type);
  switch (ops_type) {
    case CAMERA_OPS_STREAMING_PREVIEW: {
      status = camera_issue_ctrl_cmd(CAMERA_STOP_PREVIEW, 0, NULL, 1000);
    }
    break;
    case CAMERA_OPS_STREAMING_VIDEO: {
      if (CAMERA_MODE_3D == g_mm_camera_intf_obj.current_mode) {
        status = camera_issue_ctrl_cmd(CAMERA_STOP_RECORDING, 0, NULL, 1000);

        if (MM_CAMERA_SUCCESS != status) {
          CDBG_ERROR("%s 3D CAMERA_STOP_RECORDING failed", __func__);
          return status;
        }
      }
      status = camera_issue_ctrl_cmd(CAMERA_STOP_VIDEO, 0, NULL, 1000);
    }
    break;
    case CAMERA_OPS_STREAMING_ZSL: {
      status = camera_issue_ctrl_cmd(CAMERA_STOP_ZSL, 0, NULL, 5000);
    }
    break;
    case CAMERA_OPS_FOCUS: {
      status = camera_issue_ctrl_cmd(CAMERA_AUTO_FOCUS_CANCEL,
        0, NULL, 0);
      break;
    }
    case CAMERA_OPS_SNAPSHOT: {
      status = camera_issue_ctrl_cmd(CAMERA_STOP_SNAPSHOT,
        0, NULL, 0);
      break;
    }
    case CAMERA_OPS_VIDEO_RECORDING: {
      status = camera_issue_ctrl_cmd(CAMERA_STOP_RECORDING,
        0, NULL, 1000);
      break;
    }
    case CAMERA_OPS_CAPTURE:
    case CAMERA_OPS_ENCODE:
    case CAMERA_OPS_RAW_CAPTURE:
    case CAMERA_OPS_CAPTURE_AND_ENCODE: {
      status = snapshot_cancel (g_mm_camera_intf_obj.snapshot_handle);
      break;
    }
    case CAMERA_OPS_ZSL_STREAMING_CB: {
      if (g_mm_camera_intf_obj.mode == CAMERA_MODE_STREAM_ZSL) {
        g_mm_camera_intf_obj.notifyIntf->preview_frame_cb = NULL;
        status = MM_CAMERA_SUCCESS;
      }
    }
    default: {
      break;
    }
  }
  CDBG("%s %d exit", __func__, ops_type);
  return status;
}


/*===========================================================================
FUNCTION      camera_ops_init

DESCRIPTION
===========================================================================*/
mm_camera_status_t camera_ops_init (mm_camera_ops_type_t ops_type,
  void* parm1, void* parm2)
{
  mm_camera_status_t status = MM_CAMERA_ERR_NOT_SUPPORTED;
  CDBG("%s %d enter", __func__, ops_type);
  switch(ops_type) {
    case CAMERA_OPS_RAW_CAPTURE:
    case CAMERA_OPS_CAPTURE_AND_ENCODE:
    case CAMERA_OPS_CAPTURE:
    case CAMERA_OPS_STREAMING_ZSL: {
      snapshot_type_t type = SNAPSHOT_TYPE_CAPTURE_ONLY;
      if (CAMERA_OPS_CAPTURE_AND_ENCODE == ops_type)
        type = SNAPSHOT_TYPE_CAPTURE_AND_ENCODE;
      else if(CAMERA_OPS_RAW_CAPTURE == ops_type)
        type = SNAPSHOT_TYPE_RAW_CAPTURE;
      else if(CAMERA_OPS_STREAMING_ZSL == ops_type){
        type = SNAPSHOT_TYPE_ZSL;
      }

      status = snapshot_create(&g_mm_camera_intf_obj.snapshot_handle,
        g_mm_camera_intf_obj.notifyIntf, g_mm_camera_intf_obj.controlFd);
      if (MM_CAMERA_SUCCESS != status) {
        CDBG("%s snapshot_create failed %d", __func__, status);
        break;
      }
      if(CAMERA_OPS_STREAMING_ZSL == ops_type){
        zsl_params_t* p_zsl_parms = (zsl_params_t* )parm1;
        status = snapshot_set_zsl_streaming_parms(
          g_mm_camera_intf_obj.snapshot_handle,
          p_zsl_parms);
        if (MM_CAMERA_SUCCESS != status) {
          CDBG("%s snapshot_set_zsl_streaming_parms failed %d",
          __func__, status);
          break;
        }
      }
      status = snapshot_init(g_mm_camera_intf_obj.snapshot_handle, type);
      if (MM_CAMERA_SUCCESS != status) {
        CDBG("%s snapshot_init failed %d", __func__, status);
        break;
      }
      break;
    }
    default: {
      break;
    }
  }
  CDBG("%s %d exit", __func__, ops_type);
  return status;
}

/*===========================================================================
FUNCTION      camera_ops_deinit

DESCRIPTION
===========================================================================*/
mm_camera_status_t camera_ops_deinit (mm_camera_ops_type_t ops_type,
  void* parm1, void* parm2)
{
  mm_camera_status_t status = MM_CAMERA_ERR_NOT_SUPPORTED;
  CDBG("%s %d enter", __func__, ops_type);
  switch(ops_type) {
    case CAMERA_OPS_RAW_CAPTURE:
    case CAMERA_OPS_CAPTURE_AND_ENCODE:
  case CAMERA_OPS_CAPTURE:
  case CAMERA_OPS_STREAMING_ZSL: {
      status = snapshot_delete(g_mm_camera_intf_obj.snapshot_handle);
      if (!status) {
        g_mm_camera_intf_obj.snapshot_handle = NULL;
      }
    }
    break;
    default: {
        break;
      }
  }
  CDBG("%s %d exit", __func__, ops_type);
  return status;
}

/*===========================================================================
FUNCTION      mm_camera_init

DESCRIPTION
===========================================================================*/
mm_camera_status_t mm_camera_init(mm_camera_config * configIntf,
  mm_camera_notify* notifyIntf, mm_camera_ops* opsIntf,
  uint8_t dyn_device_query)
{
  mm_camera_status_t status = MM_CAMERA_SUCCESS;
  CDBG("%s: [S]", __func__);
  if ((NULL == configIntf) || (NULL == notifyIntf) || (NULL == opsIntf)) {
    CDBG_HIGH("%s: interface is NULL", __func__);
    return MM_CAMERA_ERR_INVALID_INPUT;
  }

  if (dyn_device_query) {
    char device[MAX_DEV_NAME_LEN];
    snprintf(device, MAX_DEV_NAME_LEN, MSM_CAMERA_CONTROL, 0);
    g_mm_camera_intf_obj.controlFd = open(device, O_RDWR);
    if (g_mm_camera_intf_obj.controlFd < 0) {
      CDBG_HIGH("%s: controlFd is invalid %s", __func__, strerror(errno));
      return MM_CAMERA_ERR_GENERAL;
    }

    status = camera_issue_command2(MSM_CAM_IOCTL_GET_CAMERA_INFO,
      &g_mm_camera_intf_obj.cameraInfo, "camera_info");
    if (MM_CAMERA_SUCCESS != status) {
      CDBG_HIGH("%s: GET_CAMERA_INFO failed", __func__);
      close(g_mm_camera_intf_obj.controlFd);
      g_mm_camera_intf_obj.controlFd = -1;
      return MM_CAMERA_ERR_GENERAL;
    }
  }

  CDBG("%s: GET_CAMERA_INFO success num_cam %d", __func__,
    g_mm_camera_intf_obj.cameraInfo.num_cameras);
  g_mm_camera_intf_obj.current_mode = CAMERA_MODE_2D;
  g_mm_camera_intf_obj.dyn_device_query = dyn_device_query;
  g_mm_camera_intf_obj.cameraID = 0;
  g_mm_camera_intf_obj.notifyIntf = notifyIntf;
  g_mm_camera_intf_obj.preview_frame_cb =
    g_mm_camera_intf_obj.notifyIntf->preview_frame_cb;
  g_mm_camera_intf_obj.configIntf = configIntf;
  g_mm_camera_intf_obj.opsIntf = opsIntf;
  configIntf->mm_camera_get_parm = camera_get_parms;
  configIntf->mm_camera_set_parm = camera_set_parms;
  configIntf->mm_camera_query_parms = camera_query_parms;
  configIntf->mm_camera_is_supported = camera_is_parm_supported;
  configIntf->mm_camera_is_parm_supported = camera_is_subparm_supported;
  opsIntf->mm_camera_start = camera_ops_start;
  opsIntf->mm_camera_stop = camera_ops_stop;
  opsIntf->mm_camera_is_supported = camera_is_ops_supported;
  opsIntf->mm_camera_init = camera_ops_init;
  opsIntf->mm_camera_deinit = camera_ops_deinit;
  SET_PARM_BIT32(CAMERA_PARM_MODE,
    g_mm_camera_intf_obj.properties.parm);
  SET_PARM_BIT32(CAMERA_PARM_HFR,
    g_mm_camera_intf_obj.properties.parm);
  SET_PARM_BIT32(CAMERA_PARM_CAMERA_ID,
    g_mm_camera_intf_obj.properties.parm);
  SET_PARM_BIT32(CAMERA_PARM_CAMERA_INFO,
    g_mm_camera_intf_obj.properties.parm);
  SET_PARM_BIT32(CAMERA_PARM_PICT_SIZE,
    g_mm_camera_intf_obj.properties.parm);
  SET_PARM_BIT32(CAMERA_PARM_PREVIEW_SIZE,
    g_mm_camera_intf_obj.properties.parm);
  SET_PARM_BIT32(CAMERA_PARM_BUFFER_INFO,
    g_mm_camera_intf_obj.properties.parm);
  SET_PARM_BIT32(CAMERA_PARM_JPEG_ROTATION,
    g_mm_camera_intf_obj.properties.parm);
  SET_PARM_BIT32(CAMERA_PARM_JPEG_MAINIMG_QUALITY,
    g_mm_camera_intf_obj.properties.parm);
  SET_PARM_BIT32(CAMERA_PARM_JPEG_THUMB_QUALITY,
    g_mm_camera_intf_obj.properties.parm);
#ifdef FEATURE_ZSL_SUPPORTED
  SET_PARM_BIT32(CAMERA_PARM_ZSL_ENABLE,
    g_mm_camera_intf_obj.properties.parm);
#endif
  SET_PARM_BIT32(CAMERA_PARM_FOCAL_LENGTH,
    g_mm_camera_intf_obj.properties.parm);
  SET_PARM_BIT32(CAMERA_PARM_HORIZONTAL_VIEW_ANGLE,
    g_mm_camera_intf_obj.properties.parm);
  SET_PARM_BIT32(CAMERA_PARM_VERTICAL_VIEW_ANGLE,
    g_mm_camera_intf_obj.properties.parm);
  SET_PARM_BIT32(CAMERA_PARM_RESET_LENS_TO_INFINITY,
    g_mm_camera_intf_obj.properties.parm);

  /* Ops supported by all targets */
  SET_PARM_BIT32(CAMERA_OPS_CAPTURE_AND_ENCODE,
    g_mm_camera_intf_obj.properties.ops);
  SET_PARM_BIT32(CAMERA_OPS_RAW_CAPTURE,
    g_mm_camera_intf_obj.properties.ops);
  SET_PARM_BIT32(CAMERA_OPS_ENCODE,
    g_mm_camera_intf_obj.properties.ops);
#ifdef FEATURE_ZSL_SUPPORTED
  SET_PARM_BIT32(CAMERA_OPS_ZSL_STREAMING_CB,
   g_mm_camera_intf_obj.properties.ops);
  SET_PARM_BIT32(CAMERA_OPS_STREAMING_ZSL,
   g_mm_camera_intf_obj.properties.ops);
#endif

  if (dyn_device_query && (0 <= g_mm_camera_intf_obj.controlFd)) {
    close(g_mm_camera_intf_obj.controlFd);
    g_mm_camera_intf_obj.controlFd = -1;
  }
  g_mm_camera_intf_obj.state = MM_CAMERA_INIT;
  CDBG("%s: config success [E]", __func__);
  return MM_CAMERA_SUCCESS;
}

/*===========================================================================
FUNCTION      mm_camera_exec

DESCRIPTION
===========================================================================*/
mm_camera_status_t mm_camera_exec()
{
  mm_camera_status_t status = MM_CAMERA_SUCCESS;
  config_params_t config_params;
  CDBG("%s: [S]", __func__);
  if(MM_CAMERA_IDLE == g_mm_camera_intf_obj.state) {
    CDBG("%s: invalid state %d", __func__, g_mm_camera_intf_obj.state);
    return MM_CAMERA_ERR_INVALID_OPERATION;
  }

  char device[MAX_DEV_NAME_LEN];
  CDBG("%s: state %d fd %d", __func__, g_mm_camera_intf_obj.state,
    g_mm_camera_intf_obj.controlFd);
  if (g_mm_camera_intf_obj.controlFd <= 0) {
    snprintf(device, MAX_DEV_NAME_LEN, MSM_CAMERA_CONTROL, get_device_id());
    g_mm_camera_intf_obj.controlFd = open(device, O_RDWR);
    if (g_mm_camera_intf_obj.controlFd < 0) {
      CDBG("%s: controlFd is invalid %s", __func__, strerror(errno));
      return MM_CAMERA_ERR_GENERAL;
    }
  }
  else
    return MM_CAMERA_ERR_GENERAL; // mm_camera_exec is already done
  /* Launch config thread*/
  config_params.camera_id = get_device_id();
  config_params.mode = g_mm_camera_intf_obj.current_mode;
  set_config_start_params(&config_params);
  launch_cam_conf_thread();
  CDBG("%s: after launching config thread", __func__);
  if (wait_cam_conf_ready()) {
    CDBG_ERROR("%s: config thread launch failed", __func__);
    return MM_CAMERA_ERR_GENERAL;
  }

  status = camera_issue_ctrl_cmd(CAMERA_GET_CAPABILITIES,
    sizeof(cam_prop_t), &(g_mm_camera_intf_obj.properties), 1000);
  if (MM_CAMERA_SUCCESS != status) {
    CDBG("%s: CAMERA_GET_CAPABILITIES failed", __func__);
    return MM_CAMERA_ERR_GENERAL;
  }
  CDBG("%s: effect %d", __func__, g_mm_camera_intf_obj.properties.effect);

  if (CAMERA_MODE_3D == g_mm_camera_intf_obj.current_mode) {
    uint32_t enable = 1;
    status = camera_issue_ctrl_cmd(CAMERA_ENABLE_STEREO_CAM,
      sizeof(uint32_t), &enable, 1000);
    if (MM_CAMERA_SUCCESS != status) {
      CDBG_ERROR("%s: CAMERA_ENABLE_STEREO_CAM failed", __func__);
      return MM_CAMERA_ERR_GENERAL;
    }
  }
  g_mm_camera_intf_obj.state = MM_CAMERA_EXEC;

  CDBG("%s: config success [E]", __func__);
  return MM_CAMERA_SUCCESS;
}

/*===========================================================================
FUNCTION      mm_camera_deinit

DESCRIPTION
===========================================================================*/
mm_camera_status_t mm_camera_deinit()
{
  mm_camera_status_t status = MM_CAMERA_SUCCESS;
  CDBG("mm_camera_deinit [S]");

  if (MM_CAMERA_DEINIT == g_mm_camera_intf_obj.state) {
    CDBG_ERROR("%s: invalid state %d", __func__, g_mm_camera_intf_obj.state);
    return MM_CAMERA_ERR_INVALID_OPERATION;
  }

  if (g_mm_camera_intf_obj.zoom_ratio_table.p_zoom_ratio) {
    free(g_mm_camera_intf_obj.zoom_ratio_table.p_zoom_ratio);
    g_mm_camera_intf_obj.zoom_ratio_table.p_zoom_ratio = NULL;
  }

  if (g_mm_camera_intf_obj.p_cam_info) {
    free(g_mm_camera_intf_obj.p_cam_info);
    g_mm_camera_intf_obj.p_cam_info = NULL;
  }

  if (CAMERA_MODE_3D == g_mm_camera_intf_obj.current_mode) {
    uint32_t enable = 0;
    status = camera_issue_ctrl_cmd(CAMERA_ENABLE_STEREO_CAM,
      sizeof(uint32_t), &enable, 1000);
  }
  camera_issue_ctrl_cmd(CAMERA_EXIT, 0, NULL, 1000);
  /* Release config thread*/
  release_cam_conf_thread();
  /* Release cam stats thread */
  release_camstats_thread();
  g_mm_camera_intf_obj.state = MM_CAMERA_DEINIT;
  CDBG("mm_camera_deinit [E]");
  return MM_CAMERA_SUCCESS;
}

/*===========================================================================
FUNCTION      mm_camera_destroy

DESCRIPTION
===========================================================================*/
mm_camera_status_t mm_camera_destroy()
{
  CDBG("%s: [S]", __func__);
  if (MM_CAMERA_DEINIT != g_mm_camera_intf_obj.state) {
    CDBG("%s: invalid state %d", __func__, g_mm_camera_intf_obj.state);
    return MM_CAMERA_ERR_INVALID_OPERATION;
  }
  if (0 <= g_mm_camera_intf_obj.controlFd) {
    close(g_mm_camera_intf_obj.controlFd);
  }
  memset(&g_mm_camera_intf_obj, 0, sizeof(mm_camera_intf_t));
  g_mm_camera_intf_obj.state = MM_CAMERA_IDLE;
  CDBG("%s: [E]", __func__);
  return MM_CAMERA_SUCCESS;
}
