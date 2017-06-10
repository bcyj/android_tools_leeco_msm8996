#ifndef __USF_TYPES_H__
#define __USF_TYPES_H__

/*============================================================================
                           usf_types.h

DESCRIPTION:  Ultrasound Framework types header file

Copyright (c) 2011-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
==============================================================================*/

/*-----------------------------------------------------------------------------
  Include Files
-----------------------------------------------------------------------------*/
#include <linux/types.h>

/*-----------------------------------------------------------------------------
  Consts and macros
-----------------------------------------------------------------------------*/
/**
  Special timeout values
*/
#define USF_NO_WAIT_TIMEOUT   0x00000000
// Infinitive
#define USF_INFINITIVE_TIMEOUT   0xffffffff
// Default value, used by the driver
#define USF_DEFAULT_TIMEOUT   0xfffffffe

/**
  Encoder (Tx), decoder (Rx) supported US data formats
*/
#define USF_POINT_EPOS_FORMAT   0
#define USF_RAW_FORMAT          1

/**
  Indexes of event types, produced by the calculators
*/
#define USF_TSC_EVENT_IND      0
#define USF_TSC_PTR_EVENT_IND  1
#define USF_MOUSE_EVENT_IND    2
#define USF_KEYBOARD_EVENT_IND 3
#define USF_TSC_EXT_EVENT_IND  4
#define USF_MAX_EVENT_IND      5

/**
  Types of events, produced by the calculators
*/
#define USF_NO_EVENT 0
#define USF_TSC_EVENT      (1 << USF_TSC_EVENT_IND)
#define USF_TSC_PTR_EVENT  (1 << USF_TSC_PTR_EVENT_IND)
#define USF_MOUSE_EVENT    (1 << USF_MOUSE_EVENT_IND)
#define USF_KEYBOARD_EVENT (1 << USF_KEYBOARD_EVENT_IND)
#define USF_TSC_EXT_EVENT  (1 << USF_TSC_EXT_EVENT_IND)
#define USF_ALL_EVENTS         (USF_TSC_EVENT |\
                                USF_TSC_PTR_EVENT |\
                                USF_MOUSE_EVENT |\
                                USF_KEYBOARD_EVENT |\
                                USF_TSC_EXT_EVENT)

/**
    Max size of the client name
*/
#define USF_MAX_CLIENT_NAME_SIZE    20

/**
    Max number of the ports (mics/speakers)
*/
#define USF_MAX_PORT_NUM            8

/**
  Mouse buttons supported by USF
*/
#define USF_BUTTON_LEFT_MASK    1
#define USF_BUTTON_MIDDLE_MASK  2
#define USF_BUTTON_RIGHT_MASK   4

/**
  min, max array dimension
*/
#define MIN_MAX_DIM 2

/**
  coordinates (x,y,z) array dimension
*/
#define COORDINATES_DIM 3

/**
  tilts (x,y) array dimension
*/
#define TILTS_DIM 2

/**
    ID of the Sync Gesture library module.
    This module supports the following parameter IDs:
     - USM_PARAM_ID_SYNC_GESTURE_ALGO_CONFIG
     - USM_PARAM_ID_SYNC_GESTURE_DYNAMIC_CONFIG

    To configure the Sync Gesture, the client must set the
    module ID via the SET_PARAM command or
    GET_PARAM command, and fill the module ID
    with the respective parameter IDs as listed above.
*/
#define USM_SYNC_GESTURE_LIB_MODULE_ID                             0x00012733

/**
    ID of the parameter used by USM_SYNC_GESTURE_LIB_MODULE_ID
    to configure the Sync Gesture library. SET_PARAM command can
    use this parameter ID.
*/
#define USM_PARAM_ID_SYNC_GESTURE_ALGO_CONFIG                      0x00012734

/** ID of the parameter used by USM_MODULE_ID_SYNC_GESTURE_LIB
    to  set a dynamic configuration to the Sync Gesture library.
    USM_STREAM_CMD_SET_PARAM can use this parameter ID.
*/
#define USM_PARAM_ID_SYNC_GESTURE_DYNAMIC_CONFIG                   0x00012735

/**
    Payload of the USM_PARAM_ID_SYNC_GESTURE_DYNAMIC_CONFIG
    parameter ID used by USM_MODULE_ID_SYNC_GESTURE_LIB.
*/
typedef struct usm_param_id_sync_gesture_dynanic_cfg_t
{
    uint32_t                  sync_gesture_dynamic_cfg_version;
   /* Version used for tracking sync_gesture_dynamic configuration.
      current supported value:
      USM_API_VERSION_SYNC_GESTURE_DYNAMIC_CONFIG */
    uint32_t                  directions;
    /* Set of directions needed by the client. The set could be
       up-down or right-left and it could be all the four directions */
} usm_param_id_sync_gesture_dynanic_cfg_t;

#define USM_API_VERSION_SYNC_GESTURE_DYNAMIC_CONFIG                0x1

/*-----------------------------------------------------------------------------
  Typedefs
-----------------------------------------------------------------------------*/

/**
  Info structure common for Tx and Rx
*/
typedef struct
{
  /*
    Input:  General info
  */
  // Name of the client - event calculator
  const char* client_name;
  /* Selected device identification - the proper AFE port */
  uint32_t dev_id;
  // (USF_POINT_EPOS_FORMAT - for EPOS; USF_RAW_FORMAT - for echo)
  uint32_t stream_format;
  // Required sample rate in Hz
  uint32_t sample_rate;
  // Size of a buffer (bytes) for US data transfer between the module and USF
  uint32_t buf_size;
  // Number of the buffers for the US data transfer
  uint16_t buf_num;
  // Number of the microphones (TX) or speakers(RX)
  uint16_t port_cnt;
  // Microphones(TX) or speakers(RX) data offsets in aDSP memory
  uint8_t  port_id[USF_MAX_PORT_NUM];
  // Bits per sample 16 or 32
  uint16_t bits_per_sample;

  /*
    Input:  Transparent info for encoder in the LPASS
  */
  // Parameters data size in bytes
  uint16_t params_data_size;
  // Pointer to the parameters
  uint8_t *params_data;

  // Max size of buffer fo get and set parameter
  uint32_t max_get_set_param_buf_size;
} us_xx_info_type;

/**
  Input info type
*/
typedef struct {
  // Touch screen dimensions: min & max;for input module
  int tsc_x_dim[MIN_MAX_DIM];
  int tsc_y_dim[MIN_MAX_DIM];
  int tsc_z_dim[MIN_MAX_DIM];

  // Touch screen tilt dimensions: min & max;for input module
  int tsc_x_tilt[MIN_MAX_DIM];
  int tsc_y_tilt[MIN_MAX_DIM];

  // Touch screen pressure limits: min & max; for input module
  int tsc_pressure[MIN_MAX_DIM];

  // The requested buttons bitmap
  uint16_t req_buttons_bitmap;

  // TBD: mouse info
  // TBD: keyboard info

  // Bitmap of types of events (USF_X_EVENT), produced by calculator
  uint16_t event_types;

  // Bitmap of types of events from devs, conflicting with USF
  uint16_t conflicting_event_types;

} us_input_info_type;

/**
  Tx info type
*/
typedef struct
{
   // Common info
   us_xx_info_type us_xx_info;
   // Info specific for Tx
   us_input_info_type input_info;
} us_tx_info_type;

/**
  Rx info type
*/
typedef struct
{
  // Common info
  us_xx_info_type us_xx_info;
} us_rx_info_type;

/**
  Point (coordinate) event type
*/
typedef struct
{
  // Pen coordinates (x, y, z) in units, defined by <coordinates_type>
  int coordinates[COORDINATES_DIM];
  // {x;y}  in transparent units
  int inclinations[TILTS_DIM];
  // [0-1023] (10bits); 0 - pen up
  uint32_t pressure;
  // Bitmap for buttons state. 1 - down, 0 - up
  uint16_t buttons_state_bitmap;
} point_event_type;

/**
  Mouse event type
*/
typedef struct
{
  // The mouse relative movement (dX, dY, dZ)
  int rels[COORDINATES_DIM];
  // Bitmap of mouse buttons states: 1 - down, 0 - up
  uint16_t buttons_states;
} mouse_event_type;

/**
  Keyboard event type
*/
typedef struct
{
  // Calculated MS key- see input.h.
  uint32_t key;
  // Keyboard's key state: 1 - down, 0 - up
  uint8_t key_state;
} key_event_type;

/**
  USF event type
*/
typedef struct
{
  // Event sequence number
  uint32_t seq_num;
  // Event generation system time
  uint32_t timestamp;
  // Destination input event type index (e.g. touch screen, mouse, key)
  uint16_t event_type_ind;
  // Union of all event types
  union
  {
    point_event_type point_event;
    mouse_event_type mouse_event;
    key_event_type key_event;
  } event_data;
} usf_event_type;

/**
  Tx update info type
*/
typedef struct
{
  /*
    Input:  General info
  */
  // Number of calculated events
  uint16_t event_counter;
  // Calculated events or NULL
  usf_event_type *event;
  // Read index to the end of available region in the shared US data memory
  uint32_t free_region;
  // Time (sec) to wait for data or special values:
  // USF_NO_WAIT_TIMEOUT, USF_INFINITIVE_TIMEOUT, USF_DEFAULT_TIMEOUT
  uint32_t timeout;
  //Events (from conflicting devs) to be disabled/enabled
  uint16_t event_filters;

  /*
    Input:  Transparent data
  */
  // Parameters size
  uint16_t params_data_size;
  // Pointer to the parameters
  uint8_t *params_data;

  /*
    Output parameters
  */
  // Write index to the end of ready US data region in the shared memory
  uint32_t ready_region;
} us_tx_update_info_type;

typedef struct
{
  /*
    Input:  General info
  */
  // Write index to the end of ready US data region in the shared memory
  uint32_t ready_region;

  /*
    Input:  Transparent data
  */
  // Parameters size
  uint16_t params_data_size;
  // Pointer to the parameters
  uint8_t *params_data;

  /*
    Output parameters
  */
  // Read index to the end of available region in the shared US data memory
  uint32_t free_region;
} us_rx_update_info_type;

/**
  US detection place (HW|FW)
*/
typedef enum
{
  // US is detected in HW
  US_DETECT_HW,
  // US is detected in FW
  US_DETECT_FW
} us_detect_place_enum;

/**
  US detection mode
*/
typedef enum
{
  // US detection is disabled
  US_DETECT_DISABLED_MODE,
  // US detection is enabled in continue mode
  US_DETECT_CONTINUE_MODE,
  // US detection is enabled in one shot mode
  US_DETECT_SHOT_MODE
} us_detect_mode_enum;

/**
  US detection info type
*/
typedef struct
{
  // US detection place (HW|FW)
  // NA in the Active and OFF states
  us_detect_place_enum us_detector;
  //  US detection mode
  us_detect_mode_enum  us_detect_mode;
  // US data dropped during this time (msec)
  uint32_t       skip_time;
  // Transparent data size
  uint16_t       params_data_size;
  // Pointer to the transparent data
  uint8_t        *params_data;
  // Time (sec) to wait for US presence event
  uint32_t       detect_timeout;
  // Out parameter: US presence
  bool           is_us;
} us_detect_info_type;

/**
  Get USF version
*/
typedef struct
{
  // Size of memory for the version string
  uint16_t buf_size;
  // Pointer to the memory for the version string
  char *pbuf;
} us_version_info_type;

typedef struct
{
  // Id of module
  uint32_t module_id;
  // Id of parameter
  uint32_t param_id;
  // Size of memory of the parameter buffer
  uint32_t buf_size;
  // Pointer to the memory of the parameter buffer
  uint8_t *pbuf;
} us_stream_param_type;

#endif // __USF_TYPES_H__

