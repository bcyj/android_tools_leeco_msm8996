/* sensor_util.h
 *
 * Copyright (c) 2012-2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __SENSOR_UTIL_H__
#define __SENSOR_UTIL_H__

#include <linux/media.h>
#include "mct_stream.h"
#include "module_sensor.h"

#define SENSOR_SESSIONID(id) (((id) & 0xFFFF0000) >> 16)
#define SENSOR_STREAMID(id) ((id) & 0x0000FFFF)

boolean sensor_util_unpack_identity(unsigned int identity,
  uint32_t *session_id, uint32_t *stream_id);
boolean sensor_util_find_bundle(void *data1, void *data2);
boolean sensor_util_find_identity(void *data1, void *data2);
mct_port_t *sensor_util_find_src_port_with_identity(mct_module_t *module,
  uint32_t identity);
int32_t sensor_util_assign_bundle_id(mct_module_t* module,
  uint32_t identity, cam_bundle_config_t* bundle);
void sensor_util_dump_bundle_and_stream_lists(mct_port_t* port,
  const char *func, int line);
boolean sensor_util_post_event_on_src_port(mct_module_t *module,
  mct_event_t *event);
int32_t sensor_util_find_bundle_id_for_stream(mct_port_t* port,
  uint32_t identity);
uint32_t sensor_util_find_stream_identity_by_stream_type_and_session(
  mct_module_t *module, cam_stream_type_t stream_type, uint16_t session_id);
module_sensor_port_bundle_info_t* sensor_util_find_bundle_by_id(mct_port_t* port,
  int32_t bundle_id);
void sensor_util_remove_list_entries_by_identity(mct_port_t *port,
  uint32_t identity);
boolean sensor_util_get_sbundle(mct_module_t *s_module,
  uint32_t identity, sensor_bundle_info_t *bundle_info);
int32_t sensor_util_set_digital_gain_to_isp(mct_module_t* module,
  module_sensor_bundle_info_t* s_bundle, uint32_t identity);
int32_t sensor_util_set_frame_skip_to_isp(mct_module_t* module,
  uint32_t identity, enum msm_vfe_frame_skip_pattern frame_skip_pattern);
int32_t sensor_util_is_previous_frame_sent(mct_module_t* module,
  mct_event_t *event);
boolean sensor_util_check_format(sensor_src_port_cap_t *caps,
  mct_stream_info_t *stream_info);
boolean sensor_util_post_bus_sensor_params(mct_module_t *s_module,
  module_sensor_bundle_info_t *s_bundle, uint32_t identity);
boolean sensor_util_load_liveshot_chromatix(mct_module_t *module,
  mct_port_t *port, mct_event_t *event, module_sensor_bundle_info_t *s_bundle);
boolean sensor_util_find_is_any_bundle_started(mct_port_t *port);
boolean sensor_util_post_led_state_msg(mct_module_t *s_module,
  module_sensor_bundle_info_t *s_bundle, uint32_t identity);
boolean sensor_util_post_downstream_event(mct_module_t *s_module,
  uint32_t identity, mct_event_module_type_t type, void *data);

#endif /* __SENSOR_UTIL_H__ */
