#ifndef __UAL_H__
#define __UAL_H__

/*============================================================================
                           ual.h

DESCRIPTION:  Ultrasound Abstraction Layer header file

Copyright (c) 2011-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
==============================================================================*/

/*-----------------------------------------------------------------------------
  Include Files
-----------------------------------------------------------------------------*/
#include <linux/types.h>
#include "usf_types.h"

/*-----------------------------------------------------------------------------
  Consts and macros
-----------------------------------------------------------------------------*/
// Maximum number of ports
const uint16_t UAL_MAX_PORT_NUM = USF_MAX_PORT_NUM;

// TX status ON - TX path is configured
const uint32_t UAL_TX_STATUS_ON = 1;
// RX status ON - RX path is configured
const uint32_t UAL_RX_STATUS_ON = 2;

#define ARRAY_SIZE(a) (sizeof(a)/sizeof(a[0]))

/*-----------------------------------------------------------------------------
  Typedefs
-----------------------------------------------------------------------------*/
/**
  Data transfer direction (Tx/Rx)
*/
typedef enum
{
  UAL_TX_DIRECTION,
  UAL_RX_DIRECTION
} ual_direction_type;

/**
  Supported mode of the USF work activity
  It is mainly used for USF power consumption analysis
*/
typedef enum
{
  // Get US data calculate events and inject them into the input module
  UAL_MODE_STANDARD = 0,
  // Get US data and drop it without event calculation
  UAL_MODE_NO_CALC_IN_EVENTS,
  // Calculate input events upon US data, but not inject into the input module
  UAL_MODE_NO_INJECT_IN_EVENTS,
  // Config data path, but don't read US data
  UAL_MODE_IDLE_USF_DATA_PATH,
  // Perform only device switch
  UAL_MODE_IDLE_ALL_DATA_PATH
} ual_work_mode_type;

/**
  UAL configuration type
*/
typedef  struct
{
  // <N> number in /dev/usf<N>
  uint32_t    usf_dev_id;
  // Supported mode of the USF work activity
  ual_work_mode_type  ual_mode;
} ual_cfg_type;

/**
  Data region description
*/
typedef struct
{
  // The received US data size (bytes;  0 - no data)
  uint32_t    data_buf_size;
  // The received US data (NULL - no data)
  uint8_t    *data_buf;
} ual_data_region_type;

/**
  Data type cyclic buffer
*/
typedef struct
{
  // Start data is in region[0]
  ual_data_region_type region[2];
} ual_data_type;

/**
  Versions type
*/
typedef struct
{
  // USF driver version
  const char* p_usf_version;
  // UAL version
  const char* p_ual_version;
} us_versions_type;

/*------------------------------------------------------------------------------
  Function declarations
------------------------------------------------------------------------------*/

/*==============================================================================
  FUNCTION:  ual_open
==============================================================================*/
/**
  Open session with the UAL
*/
bool ual_open
(
  // The session configuration
  ual_cfg_type *cfg
);

/*==============================================================================
  FUNCTION:  ual_configure_TX
==============================================================================*/
/**
  Configure US TX device (& stream)
*/
bool ual_configure_TX
(
  // US TX device (& stream) configuration
  us_tx_info_type *tx_info
);

/*==============================================================================
  FUNCTION:  ual_configure_RX
==============================================================================*/
/**
  Configure US RX device (& stream)
*/
bool ual_configure_RX
(
  // US RX device (& stream) configuration
  us_rx_info_type *rx_info
);

/*==============================================================================
  FUNCTION:  ual_start_TX
==============================================================================*/
/**
  Start TX data transfer
*/
bool ual_start_TX
(
  void
);

/*==============================================================================
  FUNCTION:  ual_start_RX
==============================================================================*/
/**
  Start RX data transfer
*/
bool ual_start_RX
(
  void
);

/*==============================================================================
  FUNCTION:  ual_stop_TX
==============================================================================*/
/**
  Stop TX data transfer
*/
bool ual_stop_TX
(
  void
);

/*==============================================================================
  FUNCTION:  ual_stop_RX
==============================================================================*/
/**
  Stop RX data transfer
*/
bool ual_stop_RX
(
  void
);

/*==============================================================================
  FUNCTION:  ual_close
==============================================================================*/
/**
  Stop all US data transfers, release the US devices. An argument to this
  function specifies whether this function is called because of an error. If so,
  the close sequence will be longer as we will first wait for the sound card
  state to be ONLINE.
*/
bool ual_close
(
  // Whether this function was called because of an error
  bool error_state
);

/*==============================================================================
  FUNCTION:  ual_write
==============================================================================*/
/**
  Send US RX data
*/
bool ual_write
(
  // US RX data to send
  uint8_t *data,
  // Size of US RX data to send
  uint32_t data_size
);

/*==============================================================================
  FUNCTION:  ual_read
==============================================================================*/
/**
  Read US TX data into ual_data_type and sends
  calculated events (usf_event_type) to the UAL.
  Note:
  1) ual_read() call means,
     the region from the previous call may be released.
  2) timeout event is notified by return code = true and
     data absence
*/
bool ual_read
(
  // Place for received TX US data
  ual_data_type *data,
  // The calculated events
  usf_event_type *events,
  // The number of calculated events
  uint16_t event_counter,
  // Time (sec) to wait for data or special values:
  // USF_NO_WAIT_TIMEOUT, USF_INFINITIVE_TIMEOUT, USF_DEFAULT_TIMEOUT
  uint32_t timeout = USF_DEFAULT_TIMEOUT
);

/*==============================================================================
  FUNCTION:  ual_start_us_detection
==============================================================================*/
/**
  Start "continue" US detection and
  wait (some time) for US presence info.
*/
bool ual_start_us_detection
(
  // US detection info
  us_detect_info_type& detect_info
);

/*==============================================================================
  FUNCTION:  ual_stop_us_detection
==============================================================================*/
/**
  Stop "continue" US detection before
  US presence answer.
*/
bool ual_stop_us_detection
(
  // US detection info
  us_detect_info_type& detect_info
);

/*==============================================================================
  FUNCTION:  ual_get_version
==============================================================================*/
/**
  Get USF driver and UAL versions
*/
void ual_get_version
(
  // USF, UAL versions
  us_versions_type& us_versions
);

/*==============================================================================
  FUNCTION:  ual_set_event_filters
==============================================================================*/
/**
  Set filters of events from devices, conflicting with USF
*/
void ual_set_event_filters
(
  // Events (from conflicting devs) to be disabled/enabled
  uint16_t event_filters
);

/*==============================================================================
  FUNCTION:  ual_get_status
==============================================================================*/
/**
  Get UAL status: TX(1), RX(2)
*/
uint32_t ual_get_status
(
  void
);

/*==============================================================================
  FUNCTION:  ual_set_TX_param
==============================================================================*/
/**
  Set TX stream param
*/
bool ual_set_TX_param
(
  // US TX stream configuration
  us_stream_param_type& stream_param
);

/*==============================================================================
  FUNCTION:  ual_get_TX_param
==============================================================================*/
/**
  Get TX stream param
*/
bool ual_get_TX_param
(
  // US TX stream configuration
  us_stream_param_type& stream_param
);

/*==============================================================================
  FUNCTION:  ual_set_RX_param
==============================================================================*/
/**
  Set RX stream param
*/
bool ual_set_RX_param
(
  // US TX stream configuration
  us_stream_param_type& stream_param
);

/*==============================================================================
  FUNCTION:  ual_get_RX_param
==============================================================================*/
/**
  Get RX stream param
*/
bool ual_get_RX_param
(
  // US TX stream configuration
  us_stream_param_type& stream_param
);

#endif // __UAL_H__
