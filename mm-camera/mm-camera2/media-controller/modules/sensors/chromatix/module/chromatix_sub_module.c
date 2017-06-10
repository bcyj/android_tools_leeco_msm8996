/* chromatix_sub_module.c
 *
 * Copyright (c) 2012-2015 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#include <dlfcn.h>
#include "chromatix_sub_module.h"
#include "sensor_common.h"

/*==========================================================
 * FUNCTION    - chromatix_open_library -
 *
 * DESCRIPTION:
 *==========================================================*/
static void *chromatix_load_library(void **chromatix_handle,
  void *chromatix_name)
{
  char *new_chromatix = (char *)chromatix_name;
  void *(*open_lib)(void);

  if (!chromatix_handle) {
    SERR("failed: invalid params chromatix_handle %p", chromatix_handle);
    goto ERROR;
  }

  /* Open new chromatix library */
  *chromatix_handle = dlopen((const char *)new_chromatix, RTLD_NOW);
  if (!(*chromatix_handle)) {
    SERR("Failed to dlopen %s: %s", (char *)new_chromatix, dlerror());
    goto ERROR;
  }

  /* Find function pointer to open */
  *(void **)&open_lib = dlsym(*chromatix_handle, "load_chromatix");
  if (!open_lib) {
    SERR("Failed to find symbol: %s: :%s", (char *)new_chromatix, dlerror());
    goto ERROR;
  }

  /* Call open on chromatix library and get chromatix pointer */
  return (open_lib());

ERROR:
  return NULL;
}
/*==========================================================
 * FUNCTION    - chromatix_open_library -
 *
 * DESCRIPTION:
 *==========================================================*/
static int32_t chromatix_open_library(void *chromatix_ctrl, void *data)
{
  int32_t rc = SENSOR_SUCCESS;
  sensor_chromatix_data_t *ctrl = (sensor_chromatix_data_t *)chromatix_ctrl;
  sensor_chromatix_name_t *chromatix_name = (sensor_chromatix_name_t *)data;
  char *new_chromatix = chromatix_name->chromatix;
  char *new_chromatix_common = chromatix_name->common_chromatix;

  if (!ctrl || !new_chromatix) {
    SERR("failed");
    rc = SENSOR_FAILURE;
    goto ERROR;
  }

  SLOW("new chromatix %s", new_chromatix);

  /* Load snapshot chromatix if its not loaded */
  if (!ctrl->snapshot_chromatix_ptr) {
    ctrl->snapshot_chromatix_ptr = chromatix_load_library(
       &ctrl->snapshot_chromatix_lib_handle,
       chromatix_name->snapshot_chromatix);

    if (ctrl->snapshot_chromatix_ptr)
      memcpy(&ctrl->snapshot_chromatixData, ctrl->snapshot_chromatix_ptr,
        sizeof(chromatix_parms_type));
  }

  /* Check if same chromatix is requested again */
  if (ctrl->cur_chromatix_name) {
    SLOW("old chromatix %s", ctrl->cur_chromatix_name);

    if (!strncmp((const char *)new_chromatix, ctrl->cur_chromatix_name,
      strlen(new_chromatix))) {
      SLOW("same chromatix, avoid reloading");

      ctrl->reloaded_chromatix = FALSE;
      goto LOAD_CHROMATIX_COMMON;
    }
  }

  ctrl->reloaded_chromatix = TRUE;

  /* close previously opened chromatix library */
  if (ctrl->chromatix_lib_handle) {
    dlclose(ctrl->chromatix_lib_handle);
    ctrl->chromatix_lib_handle = NULL;
    ctrl->chromatix_ptr = NULL;
  }

  ctrl->chromatix_ptr = chromatix_load_library(
     &ctrl->chromatix_lib_handle, new_chromatix);
  ctrl->cur_chromatix_name = (char *)new_chromatix;

  if (ctrl->chromatix_ptr) {
    memcpy(&ctrl->chromatixData, ctrl->chromatix_ptr,
      sizeof(chromatix_parms_type));
  SLOW("chromatix_version = %x", (int)ctrl->chromatix_ptr->chromatix_version);
  }

LOAD_CHROMATIX_COMMON:

  SLOW("new chromatix common %s", new_chromatix_common);

  if (ctrl->cur_chromatix_common_name) {
    SLOW("old chromatix common %s", ctrl->cur_chromatix_common_name);

    if (!strncmp((const char *)new_chromatix_common,
      ctrl->cur_chromatix_common_name, strlen(new_chromatix_common))) {
      SLOW("same chromatix common, avoid reloading");

      ctrl->reloaded_chromatix_common = FALSE;
      return SENSOR_SUCCESS;
    }
  }

  ctrl->reloaded_chromatix_common = TRUE;

  /* close previously opened chromatix library */
  if (ctrl->common_chromatix_lib_handle) {
    dlclose(ctrl->common_chromatix_lib_handle);
  }
  ctrl->common_chromatix_ptr = chromatix_load_library(
  &ctrl->common_chromatix_lib_handle, chromatix_name->common_chromatix);
  ctrl->cur_chromatix_common_name = (char *)new_chromatix_common;

  if (ctrl->common_chromatix_ptr) {
    memcpy(&ctrl->common_chromatixData, ctrl->common_chromatix_ptr,
      sizeof(chromatix_VFE_common_type));
  }

ERROR:
  return rc;
}

/** chromatix_open_liveshot_library: open liveshot chromatix
 *  library
 *
 *  @chromatix_ctrl: control structure for chromatix sub module
 *  @data: strcture that contains chromatix name to be opened
 *
 *  Return: TRUE for success and FALSE failure
 *
 *  This function loads liveshot chromatix and updates the
 *  pointer in control structure
 **/

static int32_t chromatix_open_liveshot_library(void *chromatix_ctrl,
  void *data)
{
  int32_t rc = SENSOR_SUCCESS;
  sensor_chromatix_data_t *ctrl = (sensor_chromatix_data_t *)chromatix_ctrl;
  sensor_chromatix_name_t *chromatix_name = (sensor_chromatix_name_t *)data;

  if (!ctrl || !chromatix_name) {
    SERR("failed");
    rc = SENSOR_FAILURE;
    goto ERROR;
  }

  SLOW("liveshot chromatix %s", chromatix_name->liveshot_chromatix);

  ctrl->liveshot_chromatix_ptr = chromatix_load_library(
    &ctrl->liveshot_chromatix_lib_handle, chromatix_name->liveshot_chromatix);
  if (!ctrl->liveshot_chromatix_ptr) {
     SERR("liveshot chromatix NULL");
     rc = SENSOR_FAILURE;
     goto ERROR;
  }

  SHIGH("liveshot handle %p", ctrl->liveshot_chromatix_lib_handle);
  memcpy(&ctrl->liveshot_chromatixData, ctrl->liveshot_chromatix_ptr,
    sizeof(chromatix_parms_type));

  SLOW("liveshot chromatix_version = %x",
    (int)ctrl->liveshot_chromatix_ptr->chromatix_version);

ERROR:
  return rc;
}


/** chromatix_close_liveshot_library: close liveshot chromatix
 *  library
 *
 *  @chromatix_ctrl: control structure for chromatix sub module
 *  @data: NA
 *
 *  Return: TRUE for success and FALSE failure
 *
 *  This function unloads of liveshot chromatix and updates the
 *  pointer in control structure
 **/

static int32_t chromatix_close_liveshot_library(void *chromatix_ctrl,
  void *data)
{
  int32_t rc = SENSOR_SUCCESS;
  sensor_chromatix_data_t *ctrl = (sensor_chromatix_data_t *)chromatix_ctrl;

  if (!ctrl) {
    SERR("failed");
    rc = SENSOR_FAILURE;
    goto ERROR;
  }

  SHIGH("liveshot handle %p", ctrl->liveshot_chromatix_lib_handle);
  /* close previously opened chromatix library */
  if (ctrl->liveshot_chromatix_lib_handle) {
    dlclose(ctrl->liveshot_chromatix_lib_handle);
    ctrl->liveshot_chromatix_ptr = NULL;
  }
ERROR:
  return rc;
}

/*==========================================================
 * FUNCTION    - chromatix_get_ptr -
 *
 * DESCRIPTION:
 *==========================================================*/
static int32_t chromatix_get_ptr(void *chromatix_ctrl, void *data)
{
  int32_t rc = SENSOR_SUCCESS;
  sensor_chromatix_data_t *ctrl = (sensor_chromatix_data_t *)chromatix_ctrl;
  sensor_chromatix_params_t *chromatix_params =
    (sensor_chromatix_params_t *)data;

  SLOW("Enter");
  if (!data) {
    SERR("failed: data NULL");
    rc = SENSOR_FAILURE;
    return rc;
  }

  if (!ctrl->chromatix_ptr || !ctrl->common_chromatix_ptr) {
    SERR("failed: chromatix ptr NULL");
    rc = SENSOR_FAILURE;
    return rc;
  }

  /* Check whether current stream has snapshot but not video */
  if ((chromatix_params->stream_mask & (1 << CAM_STREAM_TYPE_SNAPSHOT)) &&
      !(chromatix_params->stream_mask & (1 << CAM_STREAM_TYPE_VIDEO)) &&
      !(chromatix_params->stream_mask & (1 << CAM_STREAM_TYPE_PREVIEW)) &&
      (ctrl->snapshot_chromatix_lib_handle)) {
    /* Pass snapshot chromatix pointer */
    chromatix_params->chromatix_ptr = &ctrl->snapshot_chromatixData;
  } else {
    chromatix_params->chromatix_ptr = &(ctrl->chromatixData);
  }

  if (ctrl->snapshot_chromatix_lib_handle)
    chromatix_params->snapchromatix_ptr = &ctrl->snapshot_chromatixData;

  chromatix_params->common_chromatix_ptr = &(ctrl->common_chromatixData);
  if (ctrl->liveshot_chromatix_ptr) {
    chromatix_params->liveshot_chromatix_ptr = &(ctrl->liveshot_chromatixData);
  } else {
    chromatix_params->liveshot_chromatix_ptr = NULL;
  }

  chromatix_params->chromatix_reloaded = ctrl->reloaded_chromatix;
  chromatix_params->chromatix_common_reloaded = ctrl->reloaded_chromatix_common;

  SLOW("Exit rc %d", rc);
  return rc;
}

/*==========================================================
 * FUNCTION    - chromatix_open -
 *
 * DESCRIPTION:
 *==========================================================*/
static int32_t chromatix_open(void **chromatix_ctrl, const char *subdev_name)
{
  int32_t rc = SENSOR_SUCCESS;
  sensor_chromatix_data_t *ctrl = NULL;

  ctrl = malloc(sizeof(sensor_chromatix_data_t));
  if (!ctrl) {
    SERR("failed");
    return SENSOR_FAILURE;
  }
  memset(ctrl, 0, sizeof(sensor_chromatix_data_t));

  *chromatix_ctrl = (void *)ctrl;
  return rc;

ERROR:
  free(ctrl);
  return rc;
}

/*==========================================================
 * FUNCTION    - chromatix_process -
 *
 * DESCRIPTION:
 *==========================================================*/
static int32_t chromatix_process(void *chromatix_ctrl,
  sensor_submodule_event_type_t event, void *data)
{
  int32_t rc = SENSOR_SUCCESS;
  if (!chromatix_ctrl) {
    SERR("failed");
    return SENSOR_FAILURE;
  }
  switch (event) {
  case CHROMATIX_OPEN_LIBRARY:
    chromatix_open_library(chromatix_ctrl, data);
    break;
  case CHROMATIX_OPEN_LIVESHOT_LIBRARY:
    chromatix_open_liveshot_library(chromatix_ctrl, data);
    break;
  case CHROMATIX_CLOSE_LIVESHOT_LIBRARY:
    chromatix_close_liveshot_library(chromatix_ctrl, data);
    break;
  case CHROMATIX_GET_PTR:
    chromatix_get_ptr(chromatix_ctrl, data);
    break;
  default:
    SERR("invalid event %d", event);
    rc = SENSOR_FAILURE;
   break;
  }
  return SENSOR_SUCCESS;
}

/*==========================================================
 * FUNCTION    - chromatix_close -
 *
 * DESCRIPTION:
 *==========================================================*/
static int32_t chromatix_close(void *chromatix_ctrl)
{
  int32_t rc = SENSOR_SUCCESS;
  sensor_chromatix_data_t *ctrl = (sensor_chromatix_data_t *)chromatix_ctrl;

  /* close previously opened chromatix library */
  if (ctrl->chromatix_lib_handle) {
    dlclose(ctrl->chromatix_lib_handle);
    ctrl->chromatix_ptr = NULL;
  }
  /* close previously opened chromatix library */
  if (ctrl->common_chromatix_lib_handle) {
    dlclose(ctrl->common_chromatix_lib_handle);
    ctrl->common_chromatix_ptr = NULL;
  }
  /* close previously opened snapshot chromatix library */
  if (ctrl->snapshot_chromatix_lib_handle) {
    dlclose(ctrl->snapshot_chromatix_lib_handle);
    ctrl->common_chromatix_ptr = NULL;
  }
  free(ctrl);
  return rc;
}

/*==========================================================
 * FUNCTION    - chromatix_sub_module_init -
 *
 * DESCRIPTION:
 *==========================================================*/
int32_t chromatix_sub_module_init(sensor_func_tbl_t *func_tbl)
{
  if (!func_tbl) {
    SERR("failed");
    return SENSOR_FAILURE;
  }
  func_tbl->open = chromatix_open;
  func_tbl->process = chromatix_process;
  func_tbl->close = chromatix_close;
  return SENSOR_SUCCESS;
}
