/* module_sensor.h
 *
 * Copyright (c) 2012-2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __MODULE_SENSOR_H__
#define __MODULE_SENSOR_H__

#include "sensor_common.h"
#include "sensor.h"
#include "modules.h"
//#include "sensor_thread.h"
#include "mct_list.h"

typedef struct {
  cam_stream_type_t stream_type;
  uint32_t identity;
  uint32_t width;
  uint32_t height;
  int32_t bundle_id;
  uint32_t is_stream_on;
} module_sensor_port_stream_info_t;

typedef struct {
  cam_bundle_config_t bundle_config;
  uint32_t stream_mask;
  int32_t stream_on_count;
} module_sensor_port_bundle_info_t;

/* Sensor port private data */
typedef struct {
  mct_list_t *stream_list; /* module_sensor_port_stream_info_t * */
  mct_list_t *bundle_list; /* module_sensor_port_bundle_info_t * */
} module_sensor_port_data_t;

typedef struct _module_sensor_match_id_params_t {
  enum sensor_sub_module_t sub_module;
  uint8_t subdev_id;
  char *subdev_name;
} module_sensor_match_id_params_t;

boolean module_sensor_handle_pixel_clk_change(mct_module_t *module,
  uint32_t identity, void *data);
boolean module_sensor_update_af_bracket_entry(mct_module_t *module,
  void *s_bundle, mct_event_t *event);
boolean module_sensor_update_mtf_bracket_entry(mct_module_t *module,
  void *s_bundle, mct_event_t *event);
boolean module_sensor_update_flash_bracket_entry(mct_module_t *module,
  void *s_bundle, mct_event_t *event);

mct_module_t *module_sensor_init(const char *name);
void module_sensor_deinit(mct_module_t *module);
#endif /* __MODULE_SENSOR_H__ */
