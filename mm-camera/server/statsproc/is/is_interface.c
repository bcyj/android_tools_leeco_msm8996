/*==============================================================================
   Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
 *============================================================================*/

#include <stdlib.h>
#include <sys/ioctl.h>
#include <math.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "tgtcommon.h"
#include "stats_proc_interface.h"
#include "stats_proc.h"
#include "camera_dbg.h"

/*==============================================================================
 * FUNCTION    - is_if_init -
 *
 * DESCRIPTION:
 *============================================================================*/
int is_if_init(stats_proc_t *sproc)
{
  int rc = 0;

  rc = dis_if_init(sproc);
  if (rc)
    goto END;

  rc = eis_if_init(sproc);

END:
  return rc;
}

/*==============================================================================
 * FUNCTION    - is_if_deinit -
 *
 * DESCRIPTION:
 *============================================================================*/
void is_if_deinit(stats_proc_t *sproc)
{
  dis_if_deinit(sproc);
  eis_if_deinit(sproc);
}

/*==============================================================================
 * FUNCTION    - is_process -
 *
 * DESCRIPTION:
 *============================================================================*/
int is_process(stats_proc_t *sproc, uint32_t frame_id)
{
  int rc = 0;

  if (frame_id == 0)
    return 0;

  if ((sproc->share.dis_ext.has_output == 0)
      || (sproc->input.gyro_info.q16_ready == 0)) {
    sproc->share.dis_ext.eis_output_valid = 0;
    goto DIS_PROC;
  }

  rc = eis_process(sproc);

DIS_PROC:
  rc = dis_process(sproc);

END:
  return rc;
}
