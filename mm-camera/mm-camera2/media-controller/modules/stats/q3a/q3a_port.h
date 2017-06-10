/* q3a_port.h
 *
 * Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __Q3A_PORT_H__
#define __Q3A_PORT_H__

#include "mct_port.h"
#include "q3a_thread.h"

#define DUAL_LED_CALIBRATION       0
#define DUAL_LED_CALIBRATION_FRAME 15

#define Q3A_PORT_STATE_CREATED        0x1
#define Q3A_PORT_STATE_RESERVED       0x2
#define Q3A_PORT_STATE_UNRESERVED     0x3
#define Q3A_PORT_STATE_LINKED         0x4
#define Q3A_PORT_STATE_UNLINKED       0x5


#define NUMBER_OF_SKIP_FRAMES_PRE_FLASH_START (4)
#define NUMBER_OF_SKIP_FRAMES_PRE_FLASH_END   (4)

typedef enum {
  AF_WITH_LED_STATE_IDLE,
  AF_WITH_LED_STATE_AEC_RUNNING,
  AF_WITH_LED_STATE_AF_RUNNING,
  AF_WITH_LED_STATE_AF_DONE,
  AF_WITH_LED_STATE_MAX
} af_with_led_state_t;

typedef enum{
  AF_WITH_LED_NONE,
  AF_WITH_LED_TOUCH,
  AF_WITH_LED_SNAPSHOT,
  AF_WITH_LED_MAX
} af_with_led_type_t;

/** q3a_port_af_led_t
 *    @state:                  current state of the feature
 *    @state_lock:              mutex to protect the access to the state
 *    @led_status:              LED status - on or off
 *    @aec_update_data:         used to store the estimated AEC values
 *    @aec_no_led_data:         used to store the AEC values before the LED is turned ON
 *                              turned ON
 *    @aec_output_data:         used to store the data that need to send out
 *                              q3a port
 *    @led_needed:              a flag to track the AEC update and store the
 *                              condition if the LED is needed or not
 *    @preview_fps:             current preview fps to help the timer
 *    @led_wait_count:          counter to track if the LED estimation is valid
 *    @send_stored_update_data: flag to tell if we need to send stored AEC vals
 *                              before the LED was turned ON
 *    @skip_update_frame:       how many frames to skip if we requested sending
 *                              the stored AEC update
 **/
typedef struct _q3a_port_af_led_t {
  af_with_led_state_t state;
  pthread_mutex_t     state_lock;
  pthread_mutex_t     timer_lock;
  int                 led_status;
  stats_update_t      aec_update_data;
  stats_update_t      aec_no_led_data;
  stats_update_t      aec_output_data;
  boolean             led_af_needed;
  boolean             flash_needed;
  int                 preview_fps;
  int                 led_wait_count;
  boolean             send_stored_update_data;
  boolean             send_stored_no_led_data;
  int                 skip_update_frame;
  boolean             af_focus_mode_block;
  boolean             af_scene_mode_block;
  boolean             prepare_snapshot_trigger;
} q3a_port_af_led_t;

/** q3a_port_private_t
 *    @reserved_id:       The reserved ID
 *    @stream_type:       The type of the stream
 *    @state:             The state of the port - created, reserved, linked,
 *                        etc.
 *    @preview_stream_id: The ID of the preview stream
 *    @aec_port:          The pointer to the AEC port
 *    @awb_port:          The pointer to the AWB port
 *    @af_port:           The pointer to the AF port
 *    @aecawb_data:       The thread data of the aecawb thread
 *    @af_data:           The thread data of the af thread
 *    @af_led_data:       The data tracking the AF+LED feature
 *    @cur_sof_id:        The ID of the current frame
 *    @stream_on:         If the preview stream is on or off for non-ZSL case
 *
 *  This is the private data of the Q3A port.
 **/
typedef struct _q3a_port_private {
  unsigned int             reserved_id;
  unsigned int             stream_type;
  unsigned int             state;
  uint32_t                 preview_stream_id;
  mct_port_t               *aec_port;
  mct_port_t               *awb_port;
  mct_port_t               *af_port;
  q3a_thread_aecawb_data_t *aecawb_data;
  q3a_thread_af_data_t     *af_data;
  q3a_port_af_led_t        af_led_data;
  unsigned int             cur_sof_id;
  boolean                  stream_on;
  boolean                  aec_roi_enable;
  boolean                  aec_settled;
  uint32_t                 aec_ocsillate_cnt;
  boolean                  af_supported;
} q3a_port_private_t;

void    q3a_port_deinit(mct_port_t *port);
boolean q3a_port_find_identity(mct_port_t *port, unsigned int identity);
boolean q3a_port_init(mct_port_t *port, mct_port_t *aec_port,
  mct_port_t *awb_port, mct_port_t *af_port, unsigned int identity);

#endif /* __Q3A_PORT_H__ */
