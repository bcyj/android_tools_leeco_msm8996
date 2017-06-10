/**********************************************************************
  Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
**********************************************************************/
#include <string.h>
#include <stdlib.h>
#include "stats_proc.h"
#include "afd.h"

static afd_t *afdCtrl[MAX_INSTANCES];

/*==========================================================================
 * FUNCTION    - afd_export_eztune_data -
 *
 * DESCRIPTION:
 *=========================================================================*/
static void afd_export_eztune_data(stats_proc_t *sproc, afd_t *afd)
{
  sproc->share.afd_ext.eztune.status             = afd->state;
  sproc->share.afd_ext.eztune.std_width          = afd->std_width;
  sproc->share.afd_ext.eztune.flicker_detect     = afd->flicker;
  sproc->share.afd_ext.eztune.multiple_peak_algo = afd->num_peaks;
  sproc->share.afd_ext.eztune.actual_peaks       = afd->actual_peaks;
  sproc->share.afd_ext.eztune.flicker_freq       = afd->flicker_freq;
} /* afd_export_eztune_data */

/*==========================================================================
 * FUNCTION    - afd_init -
 *
 * DESCRIPTION:
 *=========================================================================*/
int afd_init(stats_proc_t *sproc)
{
  uint32_t index = sproc->handle & 0xFF;

  if (index >= MAX_INSTANCES)
    return -1;
  afdCtrl[index] = malloc(sizeof(afd_t));
  if (!afdCtrl[index])
    return -1;
  memset(afdCtrl[index], 0, sizeof(afd_t));
  afdCtrl[index]->state = AFD_STATE_OFF;
  sproc->share.afd_status   = AFD_OFF;
  afdCtrl[index]->conti_afd_delay = CAFD_DELAY;
  sproc->share.afd_atb = CAMERA_ANTIBANDING_OFF;
  return 0;
} /* afd_init */

/*===========================================================================
 * FUNCTION    - afd_get_params -
 *
 * DESCRIPTION:
 *==========================================================================*/
int afd_get_params(stats_proc_t *sproc, stats_proc_get_afd_data_t *data)
{
  uint32_t index = sproc->handle & 0xFF;
  afd_t *afd = afdCtrl[index];
  switch (data->type) {
    default:
      CDBG_ERROR("Invalid AFD Get Params Type");
      return -1;
  }
  return 0;
} /* afd_get_params */

/*===========================================================================
 * FUNCTION    - afd_set_params -
 *
 * DESCRIPTION:
 *==========================================================================*/
int afd_set_params(stats_proc_t *sproc, stats_proc_set_afd_data_t *data)
{
  int rc = 0;
  uint32_t index = sproc->handle & 0xFF;
  afd_t *afd = afdCtrl[index];
  CDBG_AFD("%s: set param %d",__func__,data->type);

  switch (data->type) {
    case AFD_ENABLE:
      if ( sproc->share.afd_enable && (afd->afd_mode ==  data->afd_mode))
        break;
      afd->conti.enable       = 0;
      sproc->share.afd_enable = data->afd_enable;
      afd->state              = AFD_STATE_INIT;
      afd->conti.trigger      = FALSE;
      afd->afd_mode               = data->afd_mode;
      sproc->share.afd_exec_once = 0;
      afd->frame_index = -3;
      switch(data->afd_mode){
        case CAMERA_ANTIBANDING_AUTO_50HZ:
          sproc->share.afd_status = AFD_50HZ_EXPOSURE_TABLE;
          sproc->share.afd_atb = CAMERA_ANTIBANDING_50HZ;
          afd->flicker_freq = 50;
          break;
        case CAMERA_ANTIBANDING_AUTO_60HZ:
          sproc->share.afd_status = AFD_60HZ_EXPOSURE_TABLE;
          sproc->share.afd_atb = CAMERA_ANTIBANDING_60HZ;
          afd->flicker_freq = 60;
          break;
        default:
          sproc->share.afd_status = AFD_REGULAR_EXPOSURE_TABLE;
          sproc->share.afd_atb = CAMERA_ANTIBANDING_OFF;
          afd->flicker_freq = 0;
        break;
      }
      break;
    case AFD_RESET:
      if (sproc->share.afd_enable){
        afd_reset(sproc, afd);
      }
      sproc->share.afd_enable = 0;
      afd->conti.enable = 0;
      break;
    default:
      return -1;
  }
  return rc;
} /* afd_set_params */

/*===========================================================================
 * FUNCTION    - afd_process -
 *
 * DESCRIPTION:
 *==========================================================================*/
int afd_process(stats_proc_t *sproc)
{
  int rc = 0;
  uint32_t index = sproc->handle & 0xFF;
  afd_t *afd = afdCtrl[index];
  if (afd->conti.enable) {
    if (afd->conti.trigger) { /* Continuous AFD trigger check */
      afd->conti.frame_skip_cnt++;
      if (afd->conti.frame_skip_cnt > afd->conti_afd_delay) {
        afd_reset(sproc, afd);
        afd->state              = AFD_STATE_ON;
        afd->conti.trigger      = FALSE;
      } else
        return 0;
    }
   }
    if (sproc->share.afd_enable) {
      rc = afd_algo_run(sproc, afd);
      if (afd->state == AFD_STATE_DONE) {
        afd->conti.frame_skip_cnt = 0;
        afd->state                = AFD_STATE_OFF;
        afd->conti.trigger        = TRUE;
      }
    }
    if (sproc->share.eztune_enable)
      afd_export_eztune_data(sproc, afd);
  return rc;
} /* afd_process */

/*===========================================================================
 * FUNCTION    - afd_deinit -
 *
 * DESCRIPTION:
 *==========================================================================*/
void afd_deinit(stats_proc_t *sproc)
{
  uint32_t index = sproc->handle & 0xFF;
  if (index >= MAX_INSTANCES)
    return;
  if (afdCtrl[index]) {
    free(afdCtrl[index]);
    afdCtrl[index] = NULL;
  }
} /* afd_deinit */
