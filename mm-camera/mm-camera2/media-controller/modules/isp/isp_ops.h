/*============================================================================
Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
============================================================================*/

#ifndef __ISP_OPS_H__
#define __ISP_OPS_H__


typedef struct {
  void *parent;      /* parent pointer */
  uint32_t handle;   /* used to support multiple isp hws */
  int (*notify) (void *parent, uint32_t handle, uint32_t type,
                 void *notify_data, uint32_t notify_data_size);
} isp_notify_ops_t;

typedef struct {
  void * ctrl;
  /* initialize the module object */
  int (*init) (void *ctrl, void *in_params, isp_notify_ops_t *notify_ops);
  /* destroy the module object */
  int (*destroy) (void *ctrl);
  /* set parameter */
  int (*set_params) (void *ctrl, uint32_t params_id,
                     void *in_params, uint32_t in_params_size);
  /* get parameter */
  int (*get_params) (void *ctrl, uint32_t params_id,
                     void *in_params, uint32_t in_params_size,
                     void *out_params, uint32_t out_params_size);
  int (*action) (void *ctrl, uint32_t action_code,
                 void *action_data, uint32_t action_data_size);
} isp_ops_t;

#endif /* __ISP_OPS_H__ */

