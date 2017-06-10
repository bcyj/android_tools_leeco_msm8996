/**********************************************************************
  Copyright (c) 2011-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
**********************************************************************/
#include <string.h>
#include <stdlib.h>
#include "stats_proc.h"
#include "asd.h"

static asd_t *asdCtrl[MAX_INSTANCES];

/*==========================================================================
 * FUNCTION    - asd_export_eztune_data -
 *
 * DESCRIPTION:
 *=========================================================================*/
static void asd_export_eztune_data(stats_proc_t *sproc, asd_t *asd)
{
  sproc->share.asd_ext.eztune.mixed_light              = asd->mixed_light;
  sproc->share.asd_ext.eztune.histo_backlight_detected =
    asd->histo_backlight_detected;
} /* asd_export_eztune_data */

/*==========================================================================
 * FUNCTION    - asd_init -
 *
 * DESCRIPTION:
 *=========================================================================*/
int asd_init(stats_proc_t *sproc)
{
  uint32_t index = sproc->handle & 0xFF;
  if (index >= MAX_INSTANCES)
    return -1;
  asdCtrl[index] = malloc(sizeof(asd_t));
  if (!asdCtrl[index])
    return -1;
  asd_init_data(sproc, asdCtrl[index]);
  return 0;
} /* asd_init */

/*===========================================================================
 * FUNCTION    - asd_get_params -
 *
 * DESCRIPTION:
 *==========================================================================*/
int asd_get_params(stats_proc_t *sproc, stats_proc_get_asd_data_t *data)
{
  uint32_t index = sproc->handle & 0xFF;
  asd_t *asd = asdCtrl[index];
  switch (data->type) {
    case ASD_PARMS:
      data->d.asd_params.asd_soft_focus_dgr =
        sproc->share.asd_ext.soft_focus_dgr;
      data->d.asd_params.backlight_detected =
        sproc->share.asd_ext.backlight_detected;
      data->d.asd_params.backlight_scene_severity =
        sproc->share.asd_ext.backlight_scene_severity;
      data->d.asd_params.landscape_severity =
        sproc->share.asd_ext.landscape_severity;
      data->d.asd_params.portrait_severity =
        sproc->share.asd_ext.portrait_severity;
      break;
    default:
      CDBG_ERROR("Invalid ASD Get Params Type %d", data->type);
      return -1;
  }
  return 0;
} /* asd_get_params */

/*===========================================================================
 * FUNCTION    - asd_set_params -
 *
 * DESCRIPTION:
 *==========================================================================*/
int asd_set_params(stats_proc_t *sproc, stats_proc_set_asd_data_t *data)
{
  uint32_t index = sproc->handle & 0xFF;
  asd_t *asd = asdCtrl[index];

  switch (data->type) {
    case ASD_ENABLE:
      asd->asd_enable = data->d.asd_enable;
      CDBG_HIGH("%s: Scene detection enable %d", __func__, asd->asd_enable);

      if (!asd->asd_enable) {
        sproc->share.asd_ext.backlight_detected       = FALSE;
        asd->histo_backlight_detected            = FALSE;
        sproc->share.asd_ext.backlight_scene_severity = 0;

        sproc->share.asd_ext.snow_or_cloudy_scene_detected     = FALSE;
        sproc->share.snow_or_cloudy_luma_target_offset = 0;

        sproc->share.asd_ext.landscape_severity = 0;
        sproc->share.asd_ext.portrait_severity = 0;
      }
      asd->no_backlight_cnt = 0;
      break;
    case ASD_BESTSHOT:
      /* NO ASD settings for BEST SHOT MODE */
      break;
    default:
      CDBG_ERROR("Invalid ASD Set Params Type");
      return -1;
  }
  return 0;
} /* asd_set_params */

/*===========================================================================
 * FUNCTION    - asd_process -
 *
 * DESCRIPTION:
 *==========================================================================*/
int asd_process(stats_proc_t *sproc)
{
  uint32_t index = sproc->handle & 0xFF;
  asd_t *asd = asdCtrl[index];
  chromatix_parms_type *cptr = sproc->input.chromatix;

  if (asd->asd_enable) {
    if (sproc->share.awb_asd_sync_flag) {
      sproc->share.awb_asd_sync_flag = FALSE;
      if (cptr->backlit_scene_detect.backlight_detection_enable)
        asd_histogram_backlight_detect(sproc, asd);
      if (cptr->snow_scene_detect.snow_scene_detection_enable)
        asd_backlight_and_snowscene_detect(sproc, asd);
    }
    if (cptr->landscape_scene_detect.landscape_detection_enable)
      asd_landscape_detect(sproc, asd);
    if (cptr->portrait_scene_detect.portrait_detection_enable)
      asd_portrait_detect(sproc, asd);
  }
  if (sproc->share.eztune_enable)
    asd_export_eztune_data(sproc, asd);
  return 0;
} /* asd_process */

/*===========================================================================
 * FUNCTION    - asd_deinit -
 *
 * DESCRIPTION:
 *==========================================================================*/
void asd_deinit(stats_proc_t *sproc)
{
  uint32_t index = sproc->handle & 0xFF;
  if (index < MAX_INSTANCES) {
    if (asdCtrl[index])
      free(asdCtrl[index]);
      asdCtrl[index] = NULL;
  }
} /* asd_deinit */

