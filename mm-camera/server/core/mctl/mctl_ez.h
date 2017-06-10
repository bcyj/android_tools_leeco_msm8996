/*============================================================================

   Copyright (c) 2011 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#ifndef __MCTL_EZ_H__
#define __MCTL_EZ_H__
#include "eztune_diagnostics.h"
#include "mctl.h"

typedef enum {
  EZ_MCTL_SOCKET_CMD,
  EZ_MCTL_VFE_CMD,
  EZ_MCTL_ISP3A_CMD,
  EZ_MCTL_SENSOR_CMD,
  EZ_MCTL_CONFIG_CMD,
  EZ_MCTL_PP_CMD,
  EZ_MCTL_PREV_SOCKET_CMD,
}mctl_ez_command;


void mctl_eztune_server_connect(m_ctrl_t *mctl, int fd);
void mctl_eztune_prev_server_connect(m_ctrl_t *mctl, int fd);
void mctl_eztune_read_and_proc_cmd(mctl_ez_command cmd);
void mctl_eztune_set_vfe(vfemodule_t module, optype_t optype, int32_t value);
void mctl_eztune_set_3A(isp_set_optype_t optype, int32_t value);
void mctl_eztune_set_aftuning(aftuning_optype_t optype, uint8_t value);
void mctl_eztune_get_misc_sensor(miscoptype_t optype);
void mctl_eztune_update_diagnostics(isp_diagtype_t optype);
#endif /* __MCTL_EZ_H__ */
