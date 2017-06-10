/*============================================================================
   Copyright (c) 2010-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#ifndef __CONFIG_PROC_H__
#define __CONFIG_PROC_H__

/* ToDo: Don't know where to put this code. */
int config_proc_send_zoom_done_event(void *parm1, void *parm2); /*config_proc.c*/

/* === interface for processing ISP events and stats === */
int config_proc_event_message_1(void *parm1,  void *parm2); /*config_proc.c*/

/* === interface for processing user commands === */
int config_proc_set_ctrl_cmd(void *parm1, void *parm2, int *cmdPending);     /*config_proc_ctlcmd.c*/
int config_proc_native_ctrl_cmd(void *parm1, void *parm2, int *cmdPending);  /*config_proc_ctlcmd.c*/
int config_proc_private_ctrl_cmd(void *parm1, void *parm2, int *cmdPending);     /*config_proc_ctlcmd.c*/

int8_t config_plane_info(void *ctrl, void *plane_info); /* config.c */
int config_proc_v4l2fmt_to_camfmt(uint32_t v4l2_pixfmt);

/* === interface for sending commands to hardware */
int isp_sendcmd(int fd, int type,
  void *pCmdData, unsigned int messageSize, int cmd_id);   /*config.c*/
int config_proc_write_sensor_gain(void *cctrl);            /*config_proc.c*/

int config_send_crop_to_mctl_pp(void *ctrl,
  void *pvideo_ctrl, void *crop);
int config_pp_send_stream_on(void *parm1, void *ctrlCmd);
int config_pp_send_stream_off(void *parm1, void *ctrlCmd);
int config_pp_end_pp_topology(void *parm1, int op_mode);
int config_pp_setup_pp_topology(void *parm1, int op_mode, void *ctrlCmd);
int config_pp_acquire_hw(void *parm1, void *p_ctrlCmd);
int config_pp_release_hw(void *parm1, void *p_ctrlCmd);
int config_pp_need_low_power_mode(void *p_ctrl, int op_mode, int *need_lp);
int config_pp_acquire_mctl_node(void *cctrl);
void config_pp_release_mctl_node(void *cctrl);

void config_init_ops(void *cctrl);
void config_shutdown_pp(void *cctrl);
int config_proc_face_detection_cmd (void *parm1, int is_fd_on);
int config_proc_zoom(void *parm1, int32_t zoom_val);
int config_proc_INTF_UPDATE_ACK(void *parm,
  int channel_interface);
uint32_t config_proc_get_stream_bit(uint32_t image_mode);

void cal_video_buf_size(uint16_t width, uint16_t height,
     uint32_t *luma_size, uint32_t *chroma_size, uint32_t *frame_size);

#endif /* __CONFIG_PROC_H__ */
