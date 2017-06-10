/* sensor.h
 *
 * Copyright (c) 2012-2015 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __SENSOR_H__
#define __SENSOR_H__

#include <stdio.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <pthread.h>
#include "sensor_common.h"
#include "sensor_thread.h"

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#define Q8  0x00000100
#define Q10 0x00000400

#define AE_BRACKET_MAX_ENTRIES 10

typedef struct {
  uint32_t width;
  uint32_t height;
  uint32_t first_pixel;
  uint32_t last_pixel;
  uint32_t first_line;
  uint32_t last_line;
  sensor_camif_inputformat_t format;
} sensor_camif_setting_t;

typedef struct {
  sensor_mode_t op_mode;
  uint32_t width;
  uint32_t height;
  uint32_t first_pixel;
  uint32_t last_pixel;
  uint32_t first_line;
  uint32_t last_line;
} sensor_dim_info_t;

typedef struct {
  float max_fps;
  float min_fps;
} sensor_fps_range_t;

typedef struct {
  cam_exp_bracketing_t ae_bracket_config;
  float real_gain[AE_BRACKET_MAX_ENTRIES];
  uint32_t linecount[AE_BRACKET_MAX_ENTRIES];
  int32_t valid_entries;
  int32_t apply_index;
  int32_t sof_counter;
  int32_t skip_frame;
  int32_t post_meta_bus;
  uint32_t prev_gain;
  uint32_t prev_linecount;
  uint8_t sync_last_gain_needed;
  uint8_t next_gain_needed;
} sensor_ae_bracket_info_t;

typedef struct {
  float real_gain;
  int32_t linecount;
  int32_t luma_hdr;
  int32_t fgain_hdr;
} sensor_exp_t;

typedef struct {
  pthread_mutex_t                mutex;
  int32_t                        fd;
  cam_stream_type_t              cur_stream_mask;
  uint32_t                       current_fps_div;
  uint32_t                       prev_gain;
  uint32_t                       prev_linecount;
  uint32_t                       stored_gain;
  uint32_t                       current_gain;
  uint32_t                       current_linecount;
  int32_t                        current_luma_hdr;
  uint32_t                       current_fgain_hdr;
  float                          sensor_real_gain;
  float                          sensor_digital_gain;
  float                          max_fps;
  float                          cur_fps;
  uint16_t                       cur_frame_length_lines;
  uint16_t                       cur_line_length_pclk;
  enum msm_sensor_resolution_t   prev_res;
  enum msm_sensor_resolution_t   cur_res;
  float                          digital_gain;
  sensor_camif_setting_t         camif_setting;
  struct msm_sensor_init_params *sensor_init_params;
  struct msm_sensor_info_t      *sensor_info;
  cam_hfr_mode_t                 hfr_mode;
  uint16_t                       wait_frame_count;
  sensor_ae_bracket_info_t       ae_bracket_info;
  uint32_t                       hdr_sof_counter;
  uint8_t                        hdr_zsl_mode;
  uint8_t                        manual_exposure_mode;
  uint8_t                        isp_frame_skip;/* Default is 0*/
  /*Sensor skip counter is delta of initial skip and sensor hardware delay
    * to determine in non zsl mode when to apply exposure values*/
  int16_t                        sensor_skip_counter;
  int32_t                        video_hdr_enable;
  int32_t                        snapshot_hdr_enable;
  int32_t                        dis_enable;
  /* Assume a sensor whose output large enough that it requires more than
     one VFE to process. when that sensor is streaming, assume another sensor
     is opened in simultaneous camera usecase. If VFE does not have enough
     resources to process second camera open, it posts upstream event providing
     op pixel clk to be used by first sensor. Dynamic restart happens where
     first sensor will be stopped and started again where it has to choose a
     resolution whose op pixel clk fits in the value sent by VFE */
  uint32_t                       isp_pixel_clk_max;
  cam_dimension_t                max_dim;
  uint32_t                       cur_exposure_time;
  uint8_t                        next_valid_frame;
  boolean                        apply_without_sync;
} sensor_data_t;

typedef struct {
  sensor_lib_params_t *lib_params;
  sensor_data_t *s_data;
  int readfd;
  int writefd;
  int session_count;
} sensor_ctrl_t;

typedef struct {
mct_module_t * module;
unsigned int session_id;
}sensor_info_t;

typedef struct {
  mct_module_t * module;
  unsigned int session_id;
  unsigned int frame_id;
} sensor_frame_t;

float sensor_get_hfr_mode_fps(cam_hfr_mode_t mode);
static int8_t sensor_set_exposure(void *sctrl, sensor_exp_t exposure);
static int32_t sensor_set_vfe_sof(void *sctrl, void *data);
static int32_t sensor_apply_exposure(void *sctrl);
#endif /* __SENSOR_H__ */
