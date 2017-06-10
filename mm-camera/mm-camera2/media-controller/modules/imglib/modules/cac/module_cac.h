/**********************************************************************
* Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved. *
* Qualcomm Technologies Proprietary and Confidential.                 *
**********************************************************************/

#ifndef __MODULE_CAC_H__
#define __MODULE_CAC_H__

#include "img_common.h"
#include "img_comp.h"
#include "img_comp_factory.h"
#include "cac.h"
#include "module_imglib_common.h"
#include "camera_dbg.h"
#include "modules.h"
#include "mct_pipeline.h"

#define IMGLIB_MOD_NAME "cac"
#define MAX_NUM_FRAMES 1
#define MAX_CAC_STATIC_PORTS 5

#define MODULE_CAC_DEBUG 1

#define MODULE_CAC_MIN_NUM_PP_BUFS 1
/** hysterisis_trend
 * @HYSTERISIS_TREND_UPWARD : values increased
 * @HYSTERISIS_TREND_DOWNWARD: value decresed
 * @HYSTERISIS_TREND_NONE: Trend not recorded yet
**/
typedef enum {
  HYSTERISIS_TREND_NONE,
  HYSTERISIS_TREND_UPWARD,
  HYSTERISIS_TREND_DOWNWARD,
} hysterisis_trend;

/** hysterisis_info_t
 * @prev_lux_value: Previous lux value
 * @lux_trend: Lux bases hysterisis trend
 * @prev_gain_value: Previous gain value
 * @gain_trend: Gain Based hysterisis trend
**/
typedef struct {
  float prev_lux_value;
  hysterisis_trend lux_trend;
  float prev_gain_value;
  hysterisis_trend gain_trend;
  uint8_t norml_hyst_enabled;
  uint8_t lowl_hyst_enabled;
  uint8_t prev_sampling_factor;
} hysterisis_info_t;

/** cac_config_t
 *   @r_gamma: R gamma table
 *   @g_gamma: G gamma table
 *   @b_gamma: B gamma table
 *   @chromatix_info: Chromatix Data
 *   @cac_3a_info_t: awb gain data
 */
typedef struct {
  img_gamma_t r_gamma;
  img_gamma_t g_gamma;
  img_gamma_t b_gamma;
  cac_chromatix_info_t chromatix_info;
  cac_3a_info_t cac_3a_data;
  cac_chroma_order chroma_order;
} cac_config_t;


/**cac2_config_t
 * @cac2_enable_flag: cac enable flag
 * @rnr_enable_flag: rnr enable flag
 * @cac_chromatix_info: cac related chromatix info
 * @chroma_order: chroma order cbcr/crcb
 * @rnr_chromatix_info: rnr related chromatix info
 * @rnr_hysterisis_info: rnr hysteris info
*/
typedef struct {
  uint8_t cac2_enable_flag;
  uint8_t rnr_enable_flag;
  cac_v2_chromatix_info_t cac_chromatix_info;
  cac_chroma_order chroma_order;
  rnr_chromatix_info_t rnr_chromatix_info;
  hysterisis_info_t rnr_hysterisis_info;
} cac2_config_t;


/** cac_client_t
 *   @mutex: client lock
 *   @cond: conditional variable for the client
 *   @comp: component ops structure
 *   @identity: MCT session/stream identity
 *   @frame: frame info from the module
 *   @state: state of face detection client
 *   @p_sinkport: sink port associated with the client
 *   @p_srcport: source port associated with the client
 *   @frame: array of image frames
 *   @parent: pointer to the parent module
 *   @stream_off: Flag to indicate whether streamoff is called
 *   @p_mod: pointer to the module
 *   @dump_input_frame: Flag to indicate whether input frame needs to be dumped
 *   @dump_output_frame: Flag to indicate whether out frame needs to be dumped
 *
 *   CAC client structure
 **/
typedef struct {
  pthread_mutex_t mutex;
  pthread_cond_t cond;
  img_component_ops_t comp;
  uint32_t identity;
  int state;
  cac_config_t cac_cfg_info;
  cac2_config_t cac2_cfg_info;
  mct_port_t *p_sinkport;
  mct_port_t *p_srcport;
  mct_stream_info_t *stream_info;
  isp_buf_divert_t *p_buf_divert_data;
  img_frame_t frame[MAX_NUM_FRAMES];
  mct_module_t *parent_mod;
  int8_t stream_off;
  void *p_mod;
  img_comp_mode_t mode;
  boolean dump_input_frame;
  boolean dump_output_frame;
} cac_client_t;


/** module_cac_t
 *   @cac_client_cnt: Variable to hold the number of CAC
 *              clients
 *   @mutex: client lock
 *   @cond: conditional variable for the client
 *   @comp: core operation structure
 *   @lib_ref_count: reference count for cac library access
 *   @cac_client: List of CAC clients
 *   @msg_thread: message thread
 *   @parent: pointer to the parent module
 *
 *   CAC module structure
 **/
typedef struct {
  int cac_client_cnt;
  pthread_mutex_t mutex;
  pthread_cond_t cond;
  img_core_ops_t core_ops;
  int lib_ref_count;
  mct_list_t *cac_client;
  mod_imglib_msg_th_t msg_thread;
  mct_module_t *parent_mod;
} module_cac_t;


/*CAC client APIs*/
int module_cac_client_create(mct_module_t *p_mct_mod, mct_port_t *p_port,
  uint32_t identity, mct_stream_info_t *stream_info);

void module_cac_client_destroy(cac_client_t *p_client);

int module_cac_client_stop(cac_client_t *p_client);

int module_cac_client_exec(cac_client_t *p_client);

void module_cac_client_divert_exec(void *userdata, void *data);

int module_cac_v1_config_client(cac_client_t *p_client);

int module_cac_v2_config_client(cac_client_t *p_client);
#endif
