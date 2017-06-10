#ifndef __USF_TYPES_H__
#define __USF_TYPES_H__

/*============================================================================
                           usf_types.h

DESCRIPTION:  Ultrasound Framework types header file

Copyright (c) 2011 Qualcomm Technologies, Inc.  All Rights Reserved.  Qualcomm Technologies Proprietary
Export of this technology or software is regulated by the U.S. Government.
Diversion contrary to U.S. law prohibited.

==============================================================================*/

/*-----------------------------------------------------------------------------
  Include Files
-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
  Consts and macros
-----------------------------------------------------------------------------*/
/**
  Encoder (Tx), decoder (Rx) supported US data formats
*/
#define USF_POINT_EPOS_FORMAT   0
#define USF_RAW_FORMAT          1

/**
  Types of events, produced by the calculators
*/
#define USF_TSC_EVENT       1
#define USF_MOUSE_EVENT     2
#define USF_KEYBOARD_EVENT  4
#define USF_RAW_EVENT       8
#define USF_ALL_EVENTS      (USF_TSC_EVENT | \
                             USF_MOUSE_EVENT | \
                             USF_KEYBOARD_EVENT | \
                             USF_RAW_EVENT)

/**
    Max size of the client name
*/
#define USF_MAX_CLIENT_NAME_SIZE    20

/**
    Max number of the ports (mics/speakers)
*/
#define USF_MAX_PORT_NUM            4

/**
  Units of coordinates (pixels, mm)
*/
// Unit is pixel
#define  USF_PIX_COORDINATE  0
// Unit is 0.01 mm
#define  USF_CMM_COORDINATE  1

/**
  Mouse buttons supported by USF
*/
#define USF_BUTTON_LEFT_MASK    1
#define USF_BUTTON_MIDDLE_MASK  2
#define USF_BUTTON_RIGHT_MASK   4

/*-----------------------------------------------------------------------------
  Typedefs
-----------------------------------------------------------------------------*/

/**
  Info  structure common for Tx and Rx
*/
typedef struct
{
  /*
    Input:  General info
  */
  // Name of the client - event calculator
  const char* client_name;
  /* Selected device identification - the proper AFE port */
  unsigned int dev_id;
  // (USF_POINT_EPOS_FORMAT - for EPOS; USF_RAW_FORMAT - for echo)
  unsigned int stream_format;
  // Required sample rate in Hz
  unsigned int sample_rate;
  // Size of a buffer (bytes) for US data transfer between the module and USF
  unsigned int buf_size;
  // Number of the buffers for the US data transfer
  unsigned short buf_num;
  // Number of the microphones (TX) or speakers(RX)
  unsigned short port_cnt;
  // Microphones(TX) or speakers(RX) data offsets in aDSP memory
  unsigned char  port_id[USF_MAX_PORT_NUM];
  // Bits per sample 16 or 32
  unsigned short bits_per_sample;

  /*
    Input:  Transparent info for encoder in the LPASS
  */
  // Parameters data size in bytes
  unsigned short params_data_size;
  // Pointer to the parameters
  unsigned char *params_data;
} us_xx_info_type;

/**
  Input info type
*/
typedef struct {
  // Touch screen dimensions: min & max;for input module
  int tsc_x_dim[2];
  int tsc_y_dim[2];

  // Touch screen fuzz; for input module
  int tsc_x_fuzz;
  int tsc_y_fuzz;

  // Touch screen pressure limits: min & max; for input module
  int tsc_pressure[2];

  // TBD: mouse info
  // TBD: keyboard info

  // Bitmap of types of events (USF_X_EVENT), produced by calculator
  unsigned short event_types;
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
  // Pen coordinates (x, y) in units, defined by <coordinates_type>
  int coordinates[2];
  // {x;y}  in degrees [-90; 90]
  unsigned int inclinations[2];
  // [0-1023] (10bits); 0 - pen up
  unsigned int pressure;
  // 0 - mapped in the display pixel. 1 - raw in 0.01 mm (only for log)
  unsigned char coordinates_type;
} point_event_type;

/**
  Mouse event type
*/
typedef struct
{
  // The mouse relative movement (dX, dY, dZ)
  int rels[3];
  // Bitmap of mouse buttons states: 1 - down, 0 - up
  unsigned short buttons_states;
} mouse_event_type;

/**
  Keyboard event type
*/
typedef struct
{
  // Calculated MS key- see input.h.
  unsigned int key;
  // Keyboard's key state: 1 - down, 0 - up
  unsigned char key_state;
} key_event_type;

/**
  Raw event type
*/
typedef struct
{
  // Raw data size in bytes
  unsigned short raw_data_size;
 // Pointer to the raw data
  unsigned char *raw_data;
} raw_event_type;

/**
  USF event type
*/
typedef struct
{
  // Event sequence number
  unsigned int seq_num;
  // Event generation system time
  unsigned int timestamp;
  // Destination input event type (e.g. touch screen, mouse, key)
  unsigned short event_type;
  // Union of all event types
  union
  {
    point_event_type point_event;
    mouse_event_type mouse_event;
    key_event_type key_event;
    raw_event_type raw_event;
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
  unsigned short event_counter;
  // Calculated events or NULL
  usf_event_type * event;
  // Read index to the end of available region in the shared US data memory
  unsigned int free_region;

  /*
    Input:  Transparent data
  */
  // Parameters size
  unsigned short params_data_size;
  // Pointer to the parameters
  unsigned char *params_data;

  /*
    Output parameters
  */
  // Write index to the end of ready US data region in the shared memory
  unsigned int ready_region;
} us_tx_update_info_type;

typedef struct
{
  /*
    Input:  General info
  */
  // Write index to the end of ready US data region in the shared memory
  unsigned int ready_region;

  /*
    Input:  Transparent data
  */
  // Parameters size
  unsigned short params_data_size;
  // Pointer to the parameters
  unsigned char *params_data;

  /*
    Output parameters
  */
  // Read index to the end of available region in the shared US data memory
  unsigned int free_region;
} us_rx_update_info_type;

#endif // __USF_TYPES_H__

