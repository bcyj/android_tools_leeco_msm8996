/*============================================================================
Copyright (c) 2013-2014 Qualcomm Technologies, Inc. All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#ifndef __ISP_RESOURCE_MGR_H__
#define __ISP_RESOURCE_MGR_H__

#define ISP_SESSION_BIT(is_isp, idx)  (1 << (16*is_isp + idx))
#define ISP_MAX_OP_PIX_CLK 320000000
#define ISP_RM_NUM_RDI (3)

typedef struct {
  uint32_t camif_mask;
  uint32_t camif_cid;
  uint32_t camif_streams;
  uint32_t rdi_mask[ISP_RM_NUM_RDI];
  uint32_t rdi_cid[ISP_RM_NUM_RDI];
  uint32_t rdi_streams[ISP_RM_NUM_RDI];
} isp_interface_session_mask_t;

typedef struct {
  pthread_mutex_t mutex;
  uint8_t num_isps;
  isp_info_t vfe_info[2];
  isp_interface_session_mask_t used_mask[2];
  void *isp_ptr;
  int isp_session_cnt;
  boolean avoid_turbo;
} isp_resources_t;

void increase_isp_session();
int isp_get_number_of_active_sessions();
void decrease_isp_session_cnt();
int isp_resource_mgr_init(uint32_t version, void *isp);
void isp_resouirce_mgr_destroy();
void choose_isp_interface(
  sensor_out_info_t *sensor_info,
  sensor_src_port_cap_t *sensor_cap,
  mct_stream_info_t *stream_info,
  uint8_t *use_pix);
int reserve_isp_resource(boolean use_pix,
               boolean is_ispif,
               sensor_src_port_cap_t *sensor_cap,
               mct_stream_info_t *stream_info,
               sensor_dim_output_t *dim_output,
               uint32_t cid,
               uint32_t op_pix_clk,
               uint32_t session_idx,
               uint32_t *isp_interface_mask,
               uint32_t *isp_id_mask);
int release_isp_resource(boolean is_ispif,
               uint32_t session_idx,
               uint32_t isp_interface_mask,
               uint32_t isp_id_mask);
int isp_interface_mask_to_interface_num(uint32_t isp_interface_mask,
               uint32_t isp_id_mask);

int isp_get_info(isp_info_t *info);
void isp_set_info(int num_isps, isp_info_t *info);
boolean has_isp_pix_interface(void);
boolean get_dual_vfe_session_id(int *session_idx);

#endif /* __ISPIF_ISP_RESOURCE_MGR_H__ */

