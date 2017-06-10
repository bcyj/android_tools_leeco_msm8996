/**********************************************************************
  Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
**********************************************************************/
#include <string.h>
#include "afd.h"
#include "frameproc.h"
static afd_t *afdCtrl[MAX_INSTANCES];

/*==========================================================================
 * FUNCTION    - afd_init -
 *
 * DESCRIPTION:
 *=========================================================================*/
int afd_init(frame_proc_t *frameCtrl)
{

  uint32_t index = frameCtrl->handle & 0xFF;
  if (index >= MAX_INSTANCES)
    return -1;
  afdCtrl[index] = malloc(sizeof(afd_t));
  if (!afdCtrl[index])
    return -1;
  memset(afdCtrl[index], 0, sizeof(afd_t));
  frameCtrl->output.afd_d.afd_state =  AFD_STATE_OFF;
  return 0;
} /* afd_init */


/*===========================================================================
 * FUNCTION    - afd_set_params -
 *
 * DESCRIPTION:
 *==========================================================================*/
int afd_set_params(frame_proc_t *frameCtrl, frame_proc_set_afd_data_t *data)
{
  int rc = 0;
  uint32_t index = frameCtrl->handle & 0xFF;
  afd_t *afd = afdCtrl[index];
  switch(data->type) {
  case FRAME_PROC_AFD_ENABLE:
    frameCtrl->output.afd_d.afd_enable = 1;
    frameCtrl->output.afd_d.afd_state =  AFD_STATE_ON;
    break;
  case FRAME_PROC_AFD_RESET:
    afd_reset((void *)frameCtrl, afd);
    break;
  default:
    return -1;
  }
  return rc;
} /* afd_set_params */

/*===========================================================================
 * FUNCTION    - frame_proc_afd_execute -
 *
 * DESCRIPTION:
 *==========================================================================*/
int frame_proc_afd_execute(frame_proc_t *frameCtrl)
{
  int rc = -1;
  uint32_t index = frameCtrl->handle & 0xFF;
  afd_t *afd = afdCtrl[index];
  CDBG_HIGH("%s: E", __func__);
  rc = afd_execute((void *)frameCtrl, afd);
  if (frameCtrl->output.afd_d.afd_state == AFD_STATE_DONE) {
    frameCtrl->output.afd_d.afd_enable = 0;
	frameCtrl->output.afd_d.afd_state = AFD_STATE_OFF;
  }

  return rc;
} /* aec_process */

/*===========================================================================
 * FUNCTION    - afd_exit -
 *
 * DESCRIPTION:
 *==========================================================================*/
int afd_exit(frame_proc_t *frameCtrl)
{
  uint32_t index = frameCtrl->handle & 0xFF;
  if (index >= MAX_INSTANCES)
    return -1;
  if (afdCtrl[index]) {
    free(afdCtrl[index]);
	afdCtrl[index] = NULL;
  }
	return 0;
} /* afd_exit */
