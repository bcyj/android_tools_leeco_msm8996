/*===========================================================================
                           usf_validation.cpp

DESCRIPTION: This file contains config file validations.


INITIALIZATION AND SEQUENCING REQUIREMENTS: None


Copyright (c) 2012-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
=============================================================================*/


/*----------------------------------------------------------------------------
Include files
----------------------------------------------------------------------------*/
#include "usf_log.h"
#include <sys/stat.h>
#include <stdlib.h>
#include <cutils/properties.h>
#include "usf_validation.h"
#include "usf_geometry.h"
#include "eposexports.h"
#include "usf_coord_transformer.h"

/*----------------------------------------------------------------------------
  Defines
----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
  Static Variable Definitions
-----------------------------------------------------------------------------*/
static const uint32_t USF_DEVICE_ID  = 1;
static const int MAX_VALUE_LEN = 100;
static const int MAX_VALID_ORIGIN_MM = 500;
static const int USF_TX_SAMPLE_WIDTH = 16;
static const uint32_t USF_RX_SAMPLE_RATE1 = 96000;
static const uint32_t USF_RX_SAMPLE_RATE2 = 192000;
static const uint32_t USF_TX_SAMPLE_RATE_ECHO = 96000;
static const uint32_t USF_TX_SAMPLE_RATE_ECHO_GESTURE = 192000;
static const uint32_t USF_RX_SAMPLE_RATE_ECHO_EPOS = 192000;
static const int USF_RX_FRAME_HDR_SIZE = 0;
static const uint32_t USF_RX_DATA_FORMAT = 1;
static const uint32_t USF_RX_PROX_DATA_FORMAT = 2;
static const uint32_t USF_RX_SYNC_GESTURE_DATA_FORMAT = 3;
static const uint32_t USF_RX_SYNC_DATA_FORMAT = 4;
static const int USF_RX_SAMPLE_WIDTH = 16;
static const uint32_t USF_RX_TRANSPARENT_DATA_SIZE = 4;
static const uint32_t USF_RX_TRANSPARENT_PROX_DATA_SIZE = 20;
static const uint32_t USF_TX_TRANSPARENT_DATA_SIZE_ECHO = 8;
static const uint32_t USF_TX_TRANSPARENT_DATA_SIZE_EPOS = 4;
static const uint32_t USF_TX_TRANSPARENT_DATA_SIZE_PROX = 76;
static const uint32_t USF_TX_TRANSPARENT_DATA_SIZE_SYNC_GESTURE = 12;
static const int TRANSPARENT_BASE = 16;
static const int MILLI_SECOND = 1000;
// TANSPARENT BYTES
static const int LOWER_BYTE_FRAME_EPOS = 0;
static const int LOWER_BYTE_SKIP = 0;
static const int UPPER_BYTE_FRAME_EPOS = 1;
static const int UPPER_BYTE_SKIP = 1;
static const int LOWER_BYTE_GROUP = 2;
static const int UPPER_BYTE_GROUP = 3;
static const int FIRST_BYTE_FRAME_ECHO = 4;
static const int SECOND_BYTE_FRAME_ECHO = 5;
static const int THIRD_BYTE_FRAME_ECHO = 6;
static const int FORTH_BYTE_FRAME_ECHO = 7;
// VALID_SKIP
static const int MIN_SKIP = 0;
static const int MAX_SKIP = 5;
// VALID GROUP
static const int MIN_GROUP = 0;
static const int MAX_GROUP = 10;

static const int MAX_ERASER_BUTTON_MODE = 2;
static const int MIN_ERASER_BUTTON_INDEX = 1;
static const int MAX_ERASER_BUTTON_INDEX = 2;


static const int FUZZ_PARAM_SIZE = 3;
static const uint16_t USF_TX_PORT_DATA_SIZE = 522;
static const uint16_t USF_TX_FRAME_HDR_SIZE_ECHO = 12;
static const uint16_t USF_TX_FRAME_HDR_SIZE_EPOS = 0;

static const uint32_t MIN_VALID_PORT_NUM = 1024;
static const uint32_t MAX_VALID_PORT_NUM = 49151;

static const uint32_t MIN_PROX_OUTPUT_VAL = OUTPUT_TYPE_RAW_MASK;
static const uint32_t MAX_PROX_OUTPUT_VAL = 7;

static const int EVENT_DEST_OUT = 0x2;
static const int EVENT_DEST_UAL = 0x1;

static const int EPOS_LIB_MAX_TRACE_LVL = TRACE_DEBUG;
/*-----------------------------------------------------------------------------
  Typedefs
-----------------------------------------------------------------------------*/

enum event_type
{
  NO_EVENT = 0,
  TOUCH_EVENT,
  MOUSE_EVENT
};

enum p2p_device_uid
{
  P2P_UID0 = 0,
  P2P_UID1,
  P2P_UID2
};

enum p2p_pattern_type
{
  P2P_PATTERN_TYPE0 = 0,
  P2P_PATTERN_TYPE1,
  P2P_PATTERN_TYPE2
};

enum tx_data_format
{
  TX_DATA_FORMAT0 = 0,  // For EPOS
  TX_DATA_FORMAT1,      // For Echo
  TX_DATA_FORMAT2,       // For Proximity
  TX_DATA_FORMAT3,       // For Synced Gesture
  TX_DATA_FORMAT4       // For Synced Echo
};

/*------------------------------------------------------------------------------
  Function definitions
------------------------------------------------------------------------------*/
static int usf_cfg_parameter_check(char* cfgFileName,
                                   us_all_info* paramsStruct,
                                   FormStruct* formStruct);

static int usf_cfg_dependencies_check(char* cfgFileName,
                                      char* daemonName,
                                      us_all_info* paramsStruct);

static int usf_parameter_check_general(char* cfgFileName,
                                       us_all_info* paramsStruct);

static int usf_parameter_check_tx_params(char* cfgFileName,
                                         us_all_info* paramsStruct,
                                         FormStruct* formStruct);

static int usf_parameter_check_rx_params(char* cfgFileName,
                                         us_all_info* paramsStruct,
                                         FormStruct* formStruct);

static int usf_parameter_check_common_parameter(char* cfgFileName,
                                                us_all_info* paramsStruct);

static int usf_parameter_check_power_save(char* cfgFileName,
                                          us_all_info* paramsStruct);

static int usf_dependencies_check_epos(char* cfgFileName,
                                       char* daemonName,
                                       us_all_info* paramsStruct);

static int usf_dependencies_check_transformation (char *transform_origin,
                                                  char *transform_end_x,
                                                  char *transform_end_y,
                                                  const char *param_name,
                                                  char *cfg_file_name);

static int usf_dependencies_check_echo(char* cfgFileName,
                                       char* daemonName,
                                       us_all_info* paramsStruct);

static int usf_dependencies_check_p2p(char* cfgFileName,
                                      us_all_info* paramsStruct,
                                      uint16_t skip);

static int usf_dependencies_check_proximity(char* cfgFileName,
                                            us_all_info* paramsStruct);

static int usf_dependencies_check_sync_gesture(char* cfgFileName,
                                               us_all_info* paramsStruct);

static int usf_dependencies_check_gesture(char* cfgFileName,
                                          us_all_info* paramsStruct,
                                          uint16_t skip);

static int usf_dependencies_check_hovering(char* cfgFileName,
                                           us_all_info* paramsStruct,
                                           uint16_t skip);

static int usf_parameter_check_ports(char* cfgFileName,
                                     char* usf_ports,
                                     int portsNum,
                                     int* portId,
                                     const char* parameterName);

static void print_error_message(char* cfgFileName,
                                const char* param_name,
                                const char* expected,
                                char* actual);

static void print_error_message_i(char* cfgFileName,
                                  const char* param_name,
                                  const char* expected,
                                  int actual);

/*==============================================================================
  FUNCTION:  usf_validation_cfg_file()
==============================================================================*/
/**
  See description in header file.
*/
int usf_validation_cfg_file (char* cfgFileName,
                             char* daemonName,
                             us_all_info* paramsStruct,
                             FormStruct* formStruct)
{
  if (!cfgFileName ||
      !paramsStruct ||
      !daemonName ||
      !formStruct)
  {
    return -1;
  }

  if (0 != usf_cfg_parameter_check(cfgFileName,
                                   paramsStruct,
                                   formStruct) ||
      0 != usf_cfg_dependencies_check(cfgFileName,
                                      daemonName,
                                      paramsStruct))
  {
    return 1;
  }

  LOGD("%s, Config file validation PASSED",
       __FUNCTION__);
  return 0;
}

/*==============================================================================
  FUNCTION:  usf_cfg_parameter_check()
==============================================================================*/
/**
 * This function checks that the cfg parameters are valid. The validation is similar for
 * all daemons.
 *
 * @param cfgFileName The path to the cfg file currently used.
 * @param paramsStruct A struct containing the cfg file parameters to be validated
 * @param formStruct A struct containing the form factor parameters
 *
 * @return 0 - Success
 *         1 - Error, meaning one of the parameters checked is invalid.
 *         -1 - Null parameters
 */
static int usf_cfg_parameter_check (char* cfgFileName,
                                    us_all_info* paramsStruct,
                                    FormStruct* formStruct)
{
  if (!cfgFileName ||
      !paramsStruct ||
      !formStruct)
  {
    return -1;
  }

  LOGD("%s, rx_use: %d, tx_use: %d",
       __FUNCTION__,
       paramsStruct->use_rx,
       paramsStruct->use_tx);

  if (0 != usf_parameter_check_general(cfgFileName,
                                       paramsStruct) ||
      0 != usf_parameter_check_tx_params(cfgFileName,
                                         paramsStruct,
                                         formStruct) ||
      0 != usf_parameter_check_rx_params(cfgFileName,
                                         paramsStruct,
                                         formStruct) ||
      0 != usf_parameter_check_common_parameter(cfgFileName,
                                                paramsStruct))
  {
    return 1;
  }


  return 0;
}

/*==============================================================================
  FUNCTION:  usf_cfg_dependencies_check()
==============================================================================*/
/**
 * This function checks that the cfg parameters are valid by checking all the
 * dependencies between them. The validations may vary between different daemons
 * and different parameter values.
 *
 * @param cfgFileName The path to the cfg file currently used.
 * @param paramsStruct A struct containing the cfg file parameters to be validated
 * @param daemonName The name of the daemon running the code
 *
 * @return 0 - Success
 *         1 - Error, meaning one of the parameters checked is invalid.
 *         -1 - Null parameters
 */
static int usf_cfg_dependencies_check (char* cfgFileName,
                                       char* daemonName,
                                       us_all_info* paramsStruct)
{
  if (!cfgFileName ||
      !daemonName ||
      !paramsStruct)
  {
    return -1;
  }
  switch (paramsStruct->usf_tx_data_format)
  {
  case TX_DATA_FORMAT0:
    return usf_dependencies_check_epos(cfgFileName,
                                       daemonName,
                                       paramsStruct);
  case TX_DATA_FORMAT1:
  case TX_DATA_FORMAT2:
  case TX_DATA_FORMAT3:
  case TX_DATA_FORMAT4:
    return usf_dependencies_check_echo(cfgFileName,
                                       daemonName,
                                       paramsStruct);
  }
  // Unreachable code, we already check that usf_tx_data_format is one
  // of the above values
  LOGE("PANIC: unreachable code was reached");
  return 1;
}

/*==============================================================================
  FUNCTION:  usf_parameter_check_general()
==============================================================================*/
/**
 * This function validates parameters for the general cfg values.
 *
 * @param cfgFileName The path to the cfg file currently used.
 * @param paramsStruct A struct containing the cfg file parameters to be validated
 *
 * @return 0 - Success
 *         1 - Error, meaning one of the parameters checked is invalid.
 *         -1 - Null parameters
 */
static int usf_parameter_check_general (char* cfgFileName,
                                        us_all_info* paramsStruct)
{
  if (!paramsStruct ||
      !paramsStruct)
  {
    return -1;
  }

  if (USF_DEVICE_ID != paramsStruct->usf_device_id)
  {
    char expected[MAX_VALUE_LEN];
    snprintf(expected,
             MAX_VALUE_LEN,
             "%d",
             USF_DEVICE_ID);
    print_error_message_i(cfgFileName,
                          "usf_device_id",
                          expected,
                          paramsStruct->usf_device_id);
    return 1;
  }
  return 0;
}

/*==============================================================================
  FUNCTION:  usf_parameter_check_tx_params()
==============================================================================*/
/**
 * This function validates the tx parameters are valid
 *
 * @param cfgFileName The path to the cfg file currently used.
 * @param paramsStruct A struct containing the cfg file parameters to be validated
 * @param formStruct A struct containing the form factor parameters
 *
 * @return 0 - Success
 *         1 - Error, meaning one of the parameters checked is invalid.
 *         -1 - Null parameters
 */
static int usf_parameter_check_tx_params (char* cfgFileName,
                                          us_all_info* paramsStruct,
                                          FormStruct* formStruct)
{
  if (!cfgFileName ||
      !paramsStruct ||
      !formStruct)
  {
    return -1;
  }

  if (false == paramsStruct->use_tx) // TX is not used
  {
    LOGD("%s, tx not used",
         __FUNCTION__);
    return 0;
  }

  // usf_tx_data_format
  char expected[MAX_VALUE_LEN];
  if ((TX_DATA_FORMAT0 != paramsStruct->usf_tx_data_format) &&
      (TX_DATA_FORMAT1 != paramsStruct->usf_tx_data_format) &&
      (TX_DATA_FORMAT2 != paramsStruct->usf_tx_data_format) &&
      (TX_DATA_FORMAT3 != paramsStruct->usf_tx_data_format) &&
      (TX_DATA_FORMAT4 != paramsStruct->usf_tx_data_format))
  {
    snprintf(expected,
             MAX_VALUE_LEN,
             "%d or %d or %d or %d or %d",
             TX_DATA_FORMAT0,
             TX_DATA_FORMAT1,
             TX_DATA_FORMAT2,
             TX_DATA_FORMAT3,
             TX_DATA_FORMAT4);
    print_error_message_i(cfgFileName,
                          "usf_tx_data_format",
                          expected,
                          paramsStruct->usf_tx_data_format);
    return 1;
  }

  // usf_tx_sample_width
  if (USF_TX_SAMPLE_WIDTH != paramsStruct->usf_tx_sample_width)
  {
    snprintf(expected,
             MAX_VALUE_LEN,
             "%d",
             USF_TX_SAMPLE_WIDTH);
    print_error_message_i(cfgFileName,
                          "usf_tx_sample_width",
                          expected,
                          paramsStruct->usf_tx_sample_width);
    return 1;
  }

  // usf_tx_port_count
  if (0 >= paramsStruct->usf_tx_port_count)
  {
    print_error_message_i(cfgFileName,
                          "usf_tx_port_count",
                          "positive",
                          paramsStruct->usf_tx_port_count);
    return 1;
  }

  if (formStruct->num_of_mics < paramsStruct->usf_tx_port_count)
  {
    char expected[MAX_VALUE_LEN];
    snprintf(expected,
             MAX_VALUE_LEN,
             "max supported mics: %d",
             formStruct->num_of_mics);
    print_error_message_i(cfgFileName,
                          "usf_tx_port_count",
                          expected,
                          paramsStruct->usf_tx_port_count);
    return 1;
  }

  // usf_tx_ports
  if (0 != usf_parameter_check_ports(cfgFileName,
                                     paramsStruct->usf_tx_ports,
                                     formStruct->num_of_mics,
                                     formStruct->mics_id,
                                     "usf_tx_ports"))
  {
    return 1;
  }

  return 0;
}

/*==============================================================================
  FUNCTION:  usf_parameter_check_rx_params()
==============================================================================*/
/**
 * This function validates the rx params.
 *
 * @param cfgFileName The path to the cfg file currently used.
 * @param paramsStruct A struct containing the cfg file parameters to be validated
 * @param formStruct A struct containing the form factor parameters
 *
 * @return 0 - Success
 *         1 - Error, meaning one of the parameters checked is invalid.
 *         -1 - Null parameters
 */
static int usf_parameter_check_rx_params (char* cfgFileName,
                                          us_all_info* paramsStruct,
                                          FormStruct* formStruct)
{
  if (!cfgFileName ||
      !paramsStruct ||
      !formStruct)
  {
    return -1;
  }

  if (false == paramsStruct->use_rx) // RX is not used
  {
    LOGD("%s, rx not used",
         __FUNCTION__);
    return 0;
  }

  // usf_rx_data_format
  char expected[MAX_VALUE_LEN];
  if (USF_RX_DATA_FORMAT      != paramsStruct->usf_rx_data_format &&
      USF_RX_PROX_DATA_FORMAT != paramsStruct->usf_rx_data_format &&
      USF_RX_SYNC_DATA_FORMAT != paramsStruct->usf_rx_data_format &&
      USF_RX_SYNC_GESTURE_DATA_FORMAT != paramsStruct->usf_rx_data_format)
  {
    snprintf(expected,
             MAX_VALUE_LEN,
             "%d or %d or %d or %d",
             USF_RX_DATA_FORMAT,
             USF_RX_PROX_DATA_FORMAT,
             USF_RX_SYNC_GESTURE_DATA_FORMAT,
             USF_RX_SYNC_DATA_FORMAT);
    print_error_message_i(cfgFileName,
                          "usf_rx_data_format",
                          expected,
                          paramsStruct->usf_rx_data_format);
    return 1;
  }

  // usf_rx_sample_rate
  if (USF_RX_SAMPLE_RATE1 != paramsStruct->usf_rx_sample_rate &&
      USF_RX_SAMPLE_RATE2 != paramsStruct->usf_rx_sample_rate)
  {
    snprintf(expected,
             MAX_VALUE_LEN,
             "%d or %d",
             USF_RX_SAMPLE_RATE1,
             USF_RX_SAMPLE_RATE2);
    print_error_message_i(cfgFileName,
                          "usf_rx_sample_rate",
                          expected,
                          paramsStruct->usf_rx_sample_rate);
    return 1;
  }

  // usf_rx_frame_hdr_size
  if (USF_RX_FRAME_HDR_SIZE != paramsStruct->usf_rx_frame_hdr_size)
  {
    snprintf(expected,
             MAX_VALUE_LEN,
             "%d",
             USF_RX_FRAME_HDR_SIZE);
    print_error_message_i(cfgFileName,
                          "usf_rx_frame_hdr_size",
                          expected,
                          paramsStruct->usf_rx_frame_hdr_size);
    return 1;
  }

  // usf_rx_sample_width
  if (USF_RX_SAMPLE_WIDTH != paramsStruct->usf_rx_sample_width)
  {
    snprintf(expected,
             MAX_VALUE_LEN,
             "%d",
             USF_RX_SAMPLE_WIDTH);
    print_error_message_i(cfgFileName,
                          "usf_rx_sample_width",
                          expected,
                          paramsStruct->usf_rx_sample_width);
    return 1;
  }

  // usf_rx_port_count
  if (0 >= paramsStruct->usf_rx_port_count)
  {
    print_error_message_i(cfgFileName,
                          "usf_rx_port_count",
                          "positive",
                          paramsStruct->usf_rx_port_count);
    return 1;
  }
  if (formStruct->num_of_speakers < paramsStruct->usf_rx_port_count)
  {
    snprintf(expected,
             MAX_VALUE_LEN,
             "max supported speakers: %d",
             formStruct->num_of_speakers);
    print_error_message_i(cfgFileName,
                          "usf_rx_port_count",
                          expected,
                          paramsStruct->usf_rx_port_count);
    return 1;
  }

  // usf_rx_ports
  if (0 != usf_parameter_check_ports(cfgFileName,
                                     paramsStruct->usf_rx_ports,
                                     formStruct->num_of_speakers,
                                     formStruct->speakers_id,
                                     "usf_rx_ports"))
  {
    return 1;
  }

  return 0;
}

/*==============================================================================
  FUNCTION:  usf_parameter_check_ports()
==============================================================================*/
/**
 * This function parses the ports given and checks that they all conform to the form
 * factor. If not, it prints an error
 *
 * @param cfgFileName The path to the cfg file currently used.
 * @param usf_ports The ports to be checked
 * @param portsNum The number of ports to be checked
 * @param portId The ports in the form factor to check with
 * @param parameterName The name of the parameter checked.
 *
 * @return 0 - Success
 *         1 - Error, meaning one of the parameters checked is invalid.
 *         -1 - Null parameters
 */
static int usf_parameter_check_ports (char* cfgFileName,
                                      char* usf_ports,
                                      int portsNum,
                                      int* portId,
                                      const char* parameterName)
{
  bool validIdFlag = false; // Indicates whether the curr port is valid
  char *save_pointer,*portNum;
  char startPort[FILE_PATH_MAX_LEN];

  strlcpy(startPort,
          usf_ports,
          FILE_PATH_MAX_LEN);

  int port = 0;

  portNum = strtok_r(startPort,
                     ",",
                     &save_pointer);
  while (portNum != NULL)
  {
    port = atoi(portNum);
    LOGD("%s: portNum as string: %s int: %d",
         __FUNCTION__,
         portNum,
         port);
    validIdFlag = false;
    // Find if port matches a port number in form factor
    for (int j = 0; j < portsNum; j++)
    {
      if (port == portId[j])
      {
        validIdFlag = true;
        break;
      }
    }
    // If no port matches
    if (!validIdFlag)
    {
      print_error_message_i(cfgFileName,
                            parameterName,
                            "match form factor ids",
                            port);
      return 1;
    }
    portNum = strtok_r(NULL,
                       ",",
                       &save_pointer);
  }
  return 0;
}

/*==============================================================================
  FUNCTION:  usf_parameter_check_common_parameter()
==============================================================================*/
/**
 * This function checks that the common cfg parameters are valid.
 *
 * @param cfgFileName The path to the cfg file currently used.
 * @param paramsStruct A struct containing the cfg file parameters to be validated
 *
 * @return 0 - Success
 *         1 - Error, meaning one of the parameters checked is invalid.
 *         -1 - Null parameters
 */
static int usf_parameter_check_common_parameter (char* cfgFileName,
                                                 us_all_info* paramsStruct)
{
  if (!cfgFileName ||
      !paramsStruct)
  {
    return -1;
  }

  // usf_append_timestamp
  if (0 != paramsStruct->usf_append_timestamp &&
      1 != paramsStruct->usf_append_timestamp)
  {
    print_error_message_i(cfgFileName,
                          "usf_append_timestamp",
                          "0 or 1",
                          paramsStruct->usf_append_timestamp);
    return 1;
  }

  // usf_frame_count
  if (paramsStruct->usf_frame_count > 0 &&
      (0 == strlen(paramsStruct->usf_frame_file)))
  {
    print_error_message_i(cfgFileName,
                          "usf_frame_count",
                          "0 since usf_frame_file is null",
                          paramsStruct->usf_frame_count);
    return 1;
  }

  // usf_frame_count
  if (0 == paramsStruct->usf_frame_count &&
      1 == paramsStruct->usf_append_timestamp)
  {
    print_error_message_i(cfgFileName,
                          "usf_append_timestamp",
                          "0 since usf_frame_count is 0",
                          paramsStruct->usf_frame_count);
    return 1;
  }

  // usf_fuzz_params
  int x, y, z;
  int ret = sscanf(paramsStruct->usf_fuzz_params,
                   "%d,%d,%d",
                   &x,
                   &y,
                   &z);
  if (ret == 3) // Then the parameter is used
  {
    LOGD("%s, FUZZ PARAMS USED: %d, %d, %d",
         __FUNCTION__,
         x,
         y,
         z);
    if ((x < 0) ||
        (y < 0) ||
        (z < 0))
    {
      print_error_message(cfgFileName,
                          "usf_fuzz_params",
                          "be positive or 0",
                          paramsStruct->usf_fuzz_params);
      return 1;
    }
  }

  // usf_frame_file_format
  if (0 != paramsStruct->usf_frame_file_format &&
      1 != paramsStruct->usf_frame_file_format)
  {
    print_error_message_i(cfgFileName,
                          "usf_frame_file_format",
                          "0 or 1",
                          paramsStruct->usf_frame_file_format);
    return 1;
  }


  // ual_work_mode
  char expected[MAX_VALUE_LEN];
  if (UAL_MODE_STANDARD != paramsStruct->ual_work_mode &&
      UAL_MODE_NO_CALC_IN_EVENTS != paramsStruct->ual_work_mode &&
      UAL_MODE_NO_INJECT_IN_EVENTS != paramsStruct->ual_work_mode &&
      UAL_MODE_IDLE_USF_DATA_PATH != paramsStruct->ual_work_mode &&
      UAL_MODE_IDLE_ALL_DATA_PATH != paramsStruct->ual_work_mode)
  {
    snprintf(expected,
             MAX_VALUE_LEN,
             "%d or %d or %d or %d or %d",
             UAL_MODE_STANDARD,
             UAL_MODE_NO_CALC_IN_EVENTS,
             UAL_MODE_NO_INJECT_IN_EVENTS,
             UAL_MODE_IDLE_USF_DATA_PATH,
             UAL_MODE_IDLE_ALL_DATA_PATH);
    print_error_message_i(cfgFileName,
                          "ual_work_mode",
                          expected,
                          paramsStruct->ual_work_mode);
    return 1;
  }

  // req_buttons_bitmap
  if (paramsStruct->req_buttons_bitmap > (BTN_STYLUS_MASK |
                                          BTN_STYLUS2_MASK |
                                          BTN_TOOL_PEN_MASK |
                                          BTN_TOOL_RUBBER_MASK |
                                          BTN_TOOL_FINGER_MASK |
                                          BTN_USF_HOVERING_ICON_MASK))
  {
    snprintf(expected,
             MAX_VALUE_LEN,
             "less (or equal) %d",
             (BTN_STYLUS_MASK |
              BTN_STYLUS2_MASK |
              BTN_TOOL_PEN_MASK |
              BTN_TOOL_RUBBER_MASK |
              BTN_TOOL_FINGER_MASK |
              BTN_USF_HOVERING_ICON_MASK));
    print_error_message_i(cfgFileName,
                          "req_buttons_bitmap",
                          expected,
                          paramsStruct->req_buttons_bitmap);
    return 1;
  }

  return 0;
}

/*==============================================================================
  FUNCTION:  usf_parameter_check_power_save()
==============================================================================*/
/**
 * This function checks that the power save parameters are valid.
 *
 * @param cfgFileName The path to the cfg file currently used.
 * @param paramsStruct A struct containing the cfg file parameters to be validated
 *
 * @return 0 - Success
 *         1 - Error, meaning one of the parameters checked is invalid.
 *         -1 - Null parameters
 */
static int usf_parameter_check_power_save(char* cfgFileName,
                                          us_all_info* paramsStruct)
{
  if (!cfgFileName ||
      !paramsStruct)
  {
    return -1;
  }

  // ps_idle_detect_port
  char tx_ports_cpy[FILE_PATH_MAX_LEN];
  memcpy(tx_ports_cpy,
         paramsStruct->usf_tx_ports,
         FILE_PATH_MAX_LEN);
  uint8_t *parsedTxPorts = (uint8_t *)malloc(sizeof(uint8_t) * paramsStruct->usf_tx_port_count);
  if (NULL == parsedTxPorts)
  {
    LOGE("%s: Failed to allocate.",
         __FUNCTION__);
    return 1;
  }
  // Separating tx ports
  if (false == ual_util_parse_string2array(paramsStruct->usf_tx_port_count,
                                           tx_ports_cpy,
                                           (uint8_t *)parsedTxPorts))
  {
    free(parsedTxPorts);
    return 1;
  }

  bool validDetectIdlePort = false;
  for (int i = 0; i < paramsStruct->usf_tx_port_count; i++)
  {
    LOGD("%s, tx port: %d, ps idle detect port: %d",
         __FUNCTION__,
         parsedTxPorts[i],
         paramsStruct->ps_idle_detect_port);
    if (parsedTxPorts[i] == paramsStruct->ps_idle_detect_port)
    {
      validDetectIdlePort = true;
    }
  }
  free(parsedTxPorts);

  // None of the tx ports match ps_idle_detect_port
  if (true != validDetectIdlePort)
  {
    print_error_message_i(cfgFileName,
                          "ps_idle_detect_port",
                          "match tx ports",
                          paramsStruct->ps_idle_detect_port);
    return 1;
  }

  return 0;
}

/*==============================================================================
  FUNCTION:  usf_dependencies_check_rx_epos_echo()
==============================================================================*/
/**
 * This function checks that rx parameters are ok for epos/echo.
 *
 * @param cfgFileName The path to the cfg file currently used.
 * @param daemonName the name of the daemon.
 * @param paramsStruct A struct containing the cfg file parameters to be validated
 *
 * @return 0 - Success
 *         1 - Error, meaning one of the parameters checked is invalid.
 *         -1 - Null parameters
 */
static int usf_dependencies_check_rx_epos_echo(char *cfgFileName,
                                               char* daemonName,
                                               us_all_info *paramsStruct)
{
  char expected[MAX_VALUE_LEN];
  if (!cfgFileName ||
      !paramsStruct)
  {
    return -1;
  }

  if (!paramsStruct->use_rx) { // Epos and tester are allowed not to use RX
    return 0;
  }

  //usf_rx_group
  uint16_t group, frame;
  // If in tester, transparent data file exists so parse group from it.
  if (0 == strcmp(daemonName,"tester") && 0 != paramsStruct->usf_rx_transparent_data_size) {
    int result = usf_parse_transparent_data(paramsStruct->usf_rx_transparent_data_size,
                                            paramsStruct->usf_rx_transparent_data,
                                            &frame,
                                            &group);
    if (0 != result) {
      return result;
    }
  }
  else
  {
    group = paramsStruct->usf_rx_group;
  }

  if ((MIN_GROUP >= group) ||
      (MAX_GROUP < group))
  {
      snprintf(expected,
               MAX_VALUE_LEN,
               "between %d and %d",
               (MIN_GROUP+1),
               MAX_GROUP);
    print_error_message_i(cfgFileName,
                          "usf_rx_group",
                          expected,
                          group);
    return 1;
  }

  // usf_rx_pattern_size
  uint16_t patternSizeRes = (paramsStruct->usf_rx_port_data_size) *
                            (paramsStruct->usf_rx_port_count) *
                            group;
  snprintf(expected,
           MAX_VALUE_LEN, "%d", patternSizeRes);
  if (patternSizeRes != paramsStruct->usf_rx_pattern_size &&
      // Since Sync gesture library decides pattern size itself
      USF_RX_SYNC_GESTURE_DATA_FORMAT != paramsStruct->usf_rx_data_format)
  {
    print_error_message_i(cfgFileName,
                          "usf_rx_pattern_size",
                          expected,
                          paramsStruct->usf_rx_pattern_size);
    return 1;
  }

  return 0;
}

/*==============================================================================
  FUNCTION:  usf_check_parallel_on_off_planes()
==============================================================================*/
/**
 * Verifies that the on screen and off screen planes are parallel to one
 * another.
 *
 * @param paramsStruct A struct containing the cfg file parameters to be validated.
 * @param cfg_file_name The path to the cfg file currently used.
 *
 * @return 0 - Success
 *         1 - Error, meaning one of the parameters checked is invalid.
 *         -1 - Null parameters
 */
static int usf_check_parallel_on_off_planes(us_all_info *paramsStruct,
                                            char *cfg_file_name)
{
  if (!paramsStruct)
  {
    return -1;
  }

  Vector on_origin, on_end_x, on_end_y, off_origin, off_end_x, off_end_y;
  on_origin = paramsStruct->usf_on_screen_transform_origin;
  on_end_x = paramsStruct->usf_on_screen_transform_end_X;
  on_end_y = paramsStruct->usf_on_screen_transform_end_Y;
  off_origin = paramsStruct->usf_epos_off_screen_transform_origin;
  off_end_x = paramsStruct->usf_epos_off_screen_transform_end_X;
  off_end_y = paramsStruct->usf_epos_off_screen_transform_end_Y;

  if (false == are_two_planes_parallel(on_origin, on_end_x, on_end_y,
                                       off_origin, off_end_x, off_end_y))
  {
    print_error_message(cfg_file_name,
                        "on and off screen planes",
                        "expected to be parallel",
                        (char *)"not parallel");
    return 1;
  }

  return 0;
}

/*==============================================================================
  FUNCTION:  usf_dependencies_check_epos()
==============================================================================*/
/**
 * This function validates the dependencies in epos daemon cfg parameters.
 *
 * @param cfgFileName The path to the cfg file currently used.
 * @param daemonName the name of the daemon.
 * @param paramsStruct A struct containing the cfg file parameters to be validated.
 *
 * @return 0 - Success
 *         1 - Error, meaning one of the parameters checked is invalid.
 *         -1 - Null parameters
 */
static int usf_dependencies_check_epos (char* cfgFileName,
                                        char* daemonName,
                                        us_all_info* paramsStruct)
{
  if (!paramsStruct ||
      !paramsStruct)
  {
    return -1;
  }

  char expected[MAX_VALUE_LEN];
  // usf_epos_rotation_axis_direction
  Vector x_axis(1.0,0.0,0.0), y_axis(0.0,1.0,0.0), z_axis(0.0,0.0,1.0);
  Vector normal_x, normal_y, normal_z, axis_direction;
  axis_direction = paramsStruct->usf_epos_rotation_axis_direction;
  normal_x.calc_normal(axis_direction, x_axis);
  normal_y.calc_normal(axis_direction, y_axis);
  normal_z.calc_normal(axis_direction, z_axis);
  // Two vectors are parallel if their cross product is 0
  if (0 == strcmp(daemonName,"digitalpen") &&
      0 != normal_x.get_length() &&
      0 != normal_y.get_length() &&
      0 != normal_z.get_length())
  {
    print_error_message(cfgFileName,
                        "usf_epos_rotation_axis_direction",
                        "parallel to x, y or z axis",
                        paramsStruct->usf_epos_rotation_axis_direction);
    return 1;
  }

   if (0 == strcmp(daemonName,"digitalpen") &&
       1 != axis_direction.get_length())
   {
     print_error_message(cfgFileName,
                         "usf_epos_rotation_axis_direction",
                         "normalized",
                         paramsStruct->usf_epos_rotation_axis_direction);
     return 1;
   }

  // eraser_button
  if (paramsStruct->eraser_button_mode >= ERASER_BUTTON_NUM_MODES)
  {
    snprintf(expected,
             MAX_VALUE_LEN,
             "less than %d",
             ERASER_BUTTON_NUM_MODES);
    print_error_message_i(cfgFileName,
                          "eraser_button_mode",
                          expected,
                          paramsStruct->eraser_button_mode);
    return 1;
  }

  if (paramsStruct->eraser_button_index >= ERASER_BUTTON_INDEX_NUM)
  {
    snprintf(expected,
             MAX_VALUE_LEN,
             "less than %d",
             ERASER_BUTTON_INDEX_NUM);
    print_error_message_i(cfgFileName,
                          "eraser_button_index",
                          expected,
                          paramsStruct->eraser_button_index);
    return 1;
  }

  // Active zone and recording
  if (0 < paramsStruct->no_act_zone_sleep_duration &&
      paramsStruct->usf_frame_count > 0)
  {
    print_error_message_i(cfgFileName,
                          "usf_frame_count",
                          "0 when using active zone",
                          paramsStruct->usf_frame_count);
    return 1;
  }

  // TX
  // usf_tx_sample_rate
  if (paramsStruct->usf_tx_sample_rate != USF_RX_SAMPLE_RATE_ECHO_EPOS)
  {
    snprintf(expected,
             MAX_VALUE_LEN,
             "%d",
             USF_RX_SAMPLE_RATE_ECHO_EPOS);
    print_error_message_i(cfgFileName,
                          "usf_tx_sample_rate",
                          expected,
                          paramsStruct->usf_tx_sample_rate);
    return 1;
  }

  // usf_tx_port_data_size
  if (paramsStruct->usf_tx_port_data_size != USF_TX_PORT_DATA_SIZE)
  {
    snprintf(expected,
             MAX_VALUE_LEN,
             "%d",
             USF_TX_PORT_DATA_SIZE);
    print_error_message_i(cfgFileName,
                          "usf_tx_port_data_size",
                          expected,
                          paramsStruct->usf_tx_port_data_size);
    return 1;
  }

  // usf_tx_frame_hdr_size
  if (paramsStruct->usf_tx_frame_hdr_size != USF_TX_FRAME_HDR_SIZE_EPOS)
  {
    snprintf(expected,
             MAX_VALUE_LEN,
             "%d",
             USF_TX_FRAME_HDR_SIZE_EPOS);
    print_error_message_i(cfgFileName,
                          "usf_tx_frame_hdr_size",
                          expected,
                          paramsStruct->usf_tx_frame_hdr_size);
    return 1;
  }

  // usf_epos_off_screen_mode
  if (0 > paramsStruct->usf_epos_off_screen_mode ||
      OFF_SCREEN_NUM_MODES <= paramsStruct->usf_epos_off_screen_mode)
  {
    snprintf(expected,
             MAX_VALUE_LEN,
             "above (including) 0 and %d (not including)",
             OFF_SCREEN_NUM_MODES);
    print_error_message_i(cfgFileName,
                          "usf_epos_off_screen_mode",
                          expected,
                          paramsStruct->usf_epos_off_screen_mode);
    return 1;
  }

  //usf_tx_group, usf_tx_skip
  uint16_t group, skip;
  // If in tester, transparent data file exists so parse group and skip from it.
  if (0 == strcmp(daemonName,"tester") && 0 != paramsStruct->usf_tx_transparent_data_size) {
    int result = usf_parse_transparent_data(paramsStruct->usf_tx_transparent_data_size,
                                            paramsStruct->usf_tx_transparent_data,
                                            &group,
                                            &skip);
    if (0 != result)
    {
      return result;
    }
  }
  else
  {
    group = paramsStruct->usf_tx_group;
    skip = paramsStruct->usf_tx_skip;
  }

  if ((group <= MIN_GROUP) ||
      (MAX_GROUP < group))
  {
    snprintf(expected,
             MAX_VALUE_LEN,
             "between %d and %d",
             (MIN_GROUP+1),
             MAX_GROUP);
    print_error_message_i(cfgFileName,
                          "usf_tx_group",
                          expected,
                          group);
    return 1;
  }

  if ((MIN_SKIP >= skip) ||
      (MAX_SKIP < skip))
  {
    snprintf(expected,
             MAX_VALUE_LEN,
             "between %d and %d",
             (MIN_SKIP+1),
             MAX_SKIP);
    print_error_message_i(cfgFileName,
                          "usf_tx_skip",
                          expected,
                          skip);
    return 1;
  }

  // Transformation check
  if (0 == strcmp(daemonName,"digitalpen") &&
      (0 != usf_dependencies_check_transformation(paramsStruct->usf_on_screen_transform_origin,
                                                  paramsStruct->usf_on_screen_transform_end_X,
                                                  paramsStruct->usf_on_screen_transform_end_Y,
                                                  "usf_on_screen_transform",
                                                  cfgFileName) ||
      0 != usf_dependencies_check_transformation(paramsStruct->usf_epos_off_screen_transform_origin,
                                                  paramsStruct->usf_epos_off_screen_transform_end_X,
                                                  paramsStruct->usf_epos_off_screen_transform_end_Y,
                                                  "usf_epos_off_screen_transform",
                                                  cfgFileName) ||
      0 != usf_check_parallel_on_off_planes(paramsStruct,
                                            cfgFileName)))
  {
    return 1;
  }

  // usf_event_type
  if (NO_EVENT != paramsStruct->usf_event_type &&
      TOUCH_EVENT != paramsStruct->usf_event_type &&
      MOUSE_EVENT != paramsStruct->usf_event_type)
  {
    snprintf(expected,
             MAX_VALUE_LEN,
             "%d or %d or %d",
             NO_EVENT,
             TOUCH_EVENT,
             MOUSE_EVENT);
    print_error_message_i(cfgFileName,
                          "usf_event_type",
                          expected,
                          paramsStruct->usf_event_type);
    return 1;
  }

  // epos_lib_max_trace_level
  if (paramsStruct->epos_lib_max_trace_level > EPOS_LIB_MAX_TRACE_LVL)
  {
      snprintf(expected,
               MAX_VALUE_LEN,
               "lower or equal to %d",
               EPOS_LIB_MAX_TRACE_LVL);
      print_error_message_i(cfgFileName,
                            "epos_lib_max_trace_level",
                            expected,
                            paramsStruct->epos_lib_max_trace_level);
      return 1;
  }

  // Only EPOS daemon has the power save parameters.
  if (!strcmp(daemonName,"digitalpen"))
  {
    // ps_idle_detect_port
    if (0 != usf_parameter_check_power_save(cfgFileName,
                                            paramsStruct))
    {
      return 1;
    }
  }

  // usf_epos_on_screen_hover_icon_mode
  if (0 > paramsStruct->usf_epos_on_screen_hover_icon_mode ||
      HOVER_ICON_NUM_MODES <= paramsStruct->usf_epos_on_screen_hover_icon_mode)
  {
    snprintf(expected,
             MAX_VALUE_LEN,
             "above (including) 0 and %d (not including)",
             HOVER_ICON_NUM_MODES);
    print_error_message_i(cfgFileName,
                          "usf_epos_on_screen_hover_icon_mode",
                          expected,
                          paramsStruct->usf_epos_on_screen_hover_icon_mode);
    return 1;
  }

  // usf_epos_off_screen_hover_icon_mode
  if (0 > paramsStruct->usf_epos_off_screen_hover_icon_mode ||
      HOVER_ICON_NUM_MODES <= paramsStruct->usf_epos_off_screen_hover_icon_mode)
  {
    snprintf(expected,
             MAX_VALUE_LEN,
             "above (including) 0 and %d (not including)",
             HOVER_ICON_NUM_MODES);
    print_error_message_i(cfgFileName,
                          "usf_epos_off_screen_hover_icon_mode",
                          expected,
                          paramsStruct->usf_epos_off_screen_hover_icon_mode);
    return 1;
  }

  // if hover icon is active, req_buttons_bitmap must have the hover icon
  // button bit set
  if ((paramsStruct->usf_epos_on_screen_hover_icon_mode > 0) ||
       (paramsStruct->usf_epos_off_screen_hover_icon_mode > 0))
  {
      if (!(paramsStruct->req_buttons_bitmap & BTN_USF_HOVERING_ICON_MASK))
      {
          snprintf(expected,
                   MAX_VALUE_LEN,
                   "bit %d must be set since hovering icon is enabled"
                   "for either on-screen or off-screen",
                   BTN_USF_HOVERING_ICON_SHIFT);
          print_error_message_i(cfgFileName,
                                "req_buttons_bitmap",
                                expected,
                                paramsStruct->req_buttons_bitmap);
          return 1;
      }
  }

  // Only EPOS daemon has usf_epos_touch_disable_threshold parameters.
  if (!strcmp(daemonName,"digitalpen"))
  {
    // usf_epos_touch_disable_threshold
    if (0 >= paramsStruct->usf_epos_touch_disable_threshold)
    {
      snprintf(expected,
               MAX_VALUE_LEN,
               "should be greater than 0");
      print_error_message_i(cfgFileName,
                            "usf_epos_touch_disable_threshold",
                            expected,
                            paramsStruct->usf_epos_touch_disable_threshold);
      return 1;
    }
  }

  return usf_dependencies_check_rx_epos_echo(cfgFileName,
                                             daemonName,
                                             paramsStruct);
}

/*==============================================================================
  FUNCTION:  usf_dependencies_check_transformation()
==============================================================================*/
/**
 * This function checks that the transformation parameters are valid.
 *
 * @param transform_origin
 * @param transform_end_x
 * @param transform_end_y
 * @param param_name
 * @param cfg_file_name The path to the cfg file currently used.
 *
 * @return 0 - Success
 *         1 - Error, meaning one of the parameters checked is invalid.
 *         -1 - Null parameters
 */
static int usf_dependencies_check_transformation (char *transform_origin,
                                                  char *transform_end_x,
                                                  char *transform_end_y,
                                                  const char *param_name,
                                                  char *cfg_file_name)
{
  if (NULL == transform_origin ||
      NULL == transform_end_x ||
      NULL == transform_end_y ||
      NULL == param_name ||
      NULL == cfg_file_name)
  {
    return -1;
  }

  Vector origin, end_x, end_y;
  origin = transform_origin;
  end_x = transform_end_x;
  end_y = transform_end_y;
  end_x.vector_substract(origin);
  end_y.vector_substract(origin);

  char expected[MAX_VALUE_LEN];
  char full_param_name[MAX_VALUE_LEN];
  char value[MAX_VALUE_LEN];

  // The two x,y vectors are perpendicular
  if (0 != end_x.dot_product(end_y))
  {
    snprintf(value,
             MAX_VALUE_LEN,
             "dot product is: %lf",
             end_x.dot_product(end_y));
    print_error_message(cfg_file_name,
                        full_param_name,
                        "to create two orthogonal vectors",
                        value);
    return 1;
  }

  // Origin distance from EPOS origin is reasonable
  if (MAX_VALID_ORIGIN_MM < origin.get_element(X) ||
      MAX_VALID_ORIGIN_MM < origin.get_element(Y) ||
      MAX_VALID_ORIGIN_MM < origin.get_element(Z))
  {
    snprintf(full_param_name,
             MAX_VALUE_LEN,
             "%s_origin",
             param_name);
    snprintf(expected,
             MAX_VALUE_LEN,
             "below %d",
             MAX_VALID_ORIGIN_MM);
    print_error_message(cfg_file_name,
                        full_param_name,
                        expected,
                        transform_origin);
    return 1;
  }

  // Screen size is reasonable
  if (MAX_VALID_ORIGIN_MM < end_x.get_length() ||
      MAX_VALID_ORIGIN_MM < end_y.get_length())
  {
    snprintf(expected,
             MAX_VALUE_LEN,
             "x,y screen size below %d",
             MAX_VALID_ORIGIN_MM);
    snprintf(value,
             MAX_VALUE_LEN,
             "width: %lf, height: %lf",
             end_x.get_length(),
             end_y.get_length());
    print_error_message(cfg_file_name,
                        param_name,
                        expected,
                        value);
    return 1;
  }

  return 0;
}

/*==============================================================================
  FUNCTION:  usf_dependencies_check_echo()
==============================================================================*/
/**
 * This function validates the dependencies in echo parameters.
 *
 * @param cfgFileName The path to the cfg file currently used.
 * @param paramsStruct A struct containing the cfg file parameters to be validated
 * @param daemonName The name of the daemon running the code
 *
 * @return 0 - Success
 *         1 - Error, meaning one of the parameters checked is invalid.
 *         -1 - Null parameters
 */
static int usf_dependencies_check_echo (char* cfgFileName,
                                        char* daemonName,
                                        us_all_info* paramsStruct)
{
  if (!cfgFileName ||
      !daemonName ||
      !paramsStruct)
  {
    return -1;
  }

  // TX
  // usf_tx_sample_rate
  char expected[MAX_VALUE_LEN];
  if (paramsStruct->usf_tx_sample_rate != USF_RX_SAMPLE_RATE_ECHO_EPOS &&
      paramsStruct->usf_tx_sample_rate != USF_TX_SAMPLE_RATE_ECHO)
  {
    snprintf(expected,
             MAX_VALUE_LEN,
             "%d or %d",
             USF_RX_SAMPLE_RATE_ECHO_EPOS,
             USF_TX_SAMPLE_RATE_ECHO);
    print_error_message_i(cfgFileName,
                          "usf_tx_sample_rate",
                          expected,
                          paramsStruct->usf_tx_sample_rate);
    return 1;
  }

  // usf_tx_port_data_size
  if (TX_DATA_FORMAT3 != paramsStruct->usf_tx_data_format && // sync gesture format
      TX_DATA_FORMAT4 != paramsStruct->usf_tx_data_format && // sync free form format
      ((paramsStruct->usf_tx_port_data_size)%(paramsStruct->usf_tx_sample_rate/MILLI_SECOND)) != 0)
  {
    print_error_message_i(cfgFileName,
                          "usf_tx_port_data_size",
                          "to be devided without a remainder by usf_tx_sample_rate",
                          paramsStruct->usf_tx_port_data_size);
    return 1;
  }

  // usf_tx_frame_hdr_size
  if (paramsStruct->usf_tx_frame_hdr_size != USF_TX_FRAME_HDR_SIZE_ECHO)
  {
    snprintf(expected,
             MAX_VALUE_LEN,
             "%d",
             USF_TX_FRAME_HDR_SIZE_ECHO);
    print_error_message_i(cfgFileName,
                          "usf_tx_frame_hdr_size",
                          expected,
                          paramsStruct->usf_tx_frame_hdr_size);
    return 1;
  }

  // usf_tx_transparent_data
  uint16_t skip, group;
  // If in tester, transparent data file exists so parse group and skip from it.
  if (0 == strcmp(daemonName,"tester") && 0 != paramsStruct->usf_tx_transparent_data_size)
  {
    int result;
    {
       uint32_t frame;
       result = usf_parse_transparent_data(paramsStruct->usf_tx_transparent_data_size,
                                           paramsStruct->usf_tx_transparent_data,
                                           &frame,
                                           &group,
                                           &skip);
    }
    if (0 != result)
    {
      return result;
    }
  }
  else
  {
    group = paramsStruct->usf_tx_group;
    skip = paramsStruct->usf_tx_skip;
  }

  if (0 == strcmp(daemonName, "proximity"))
  {
    // usf_tx_transparent_data_size
    if (paramsStruct->usf_tx_transparent_data_size !=
              USF_TX_TRANSPARENT_DATA_SIZE_PROX)
    {
      snprintf(expected,
               MAX_VALUE_LEN,
               "%d",
               USF_TX_TRANSPARENT_DATA_SIZE_PROX);
      print_error_message_i(cfgFileName,
                            "usf_tx_transparent_data_size",
                            expected,
                            paramsStruct->usf_tx_transparent_data_size);
      return 1;
    }
  }
  else if (0 == strcmp(daemonName, "sync_gesture"))
  {
    // usf_tx_transparent_data_size
    if (paramsStruct->usf_tx_transparent_data_size !=
              USF_TX_TRANSPARENT_DATA_SIZE_SYNC_GESTURE)
    {
      snprintf(expected,
               MAX_VALUE_LEN,
               "%d",
               USF_TX_TRANSPARENT_DATA_SIZE_SYNC_GESTURE);
      print_error_message_i(cfgFileName,
                            "usf_tx_transparent_data_size",
                            expected,
                            paramsStruct->usf_tx_transparent_data_size);
      return 1;
    }
  }
  else
  {
    if (MIN_SKIP >= skip ||
        MAX_SKIP < skip)
    {
      snprintf(expected,
               MAX_VALUE_LEN,
               "between %d and %d",
               (MIN_SKIP+1),
               MAX_SKIP);
      print_error_message_i(cfgFileName,
                            "usf_tx_skip",
                            expected,
                            skip);
      return 1;
    }

    if (MIN_GROUP >= group ||
        MAX_GROUP < group)
    {
      snprintf(expected,
               MAX_VALUE_LEN,
               "between %d and %d",
               (MIN_GROUP+1),
               MAX_GROUP);
      print_error_message_i(cfgFileName,
                            "usf_tx_group",
                            expected,
                            group);
      return 1;
    }
  }

  int rc = 0;
  if (strcmp(daemonName, "gesture") == 0 ||
      strcmp(daemonName, "sync_gesture") == 0)
  {
    if ((rc = usf_dependencies_check_gesture(cfgFileName,
                                             paramsStruct,
                                             skip)))
    {
      return rc;
    }
  }
  if (strcmp(daemonName, "sync_gesture") == 0)
  {
    if ((rc = usf_dependencies_check_sync_gesture(cfgFileName,
                                                  paramsStruct)))
    {
      return rc;
    }
  }
  else if (strcmp(daemonName, "hovering") == 0)
  {
    if ((rc = usf_dependencies_check_hovering(cfgFileName,
                                              paramsStruct,
                                              skip)))
    {
      return rc;
    }
  }
  else if (strcmp(daemonName, "p2p") == 0)
  {
    if ((rc = usf_dependencies_check_p2p(cfgFileName,
                                         paramsStruct,
                                         skip)))
    {
      return rc;
    }
  }
  else if (strcmp(daemonName, "proximity") == 0)
  {
    // In case of proximity we have a different rx check, that's why we return
    // here, and we don't fall to the rx check like the rest of the daemons.
    return usf_dependencies_check_proximity(cfgFileName,
                                            paramsStruct);
  }

  return usf_dependencies_check_rx_epos_echo(cfgFileName,
                                             daemonName,
                                             paramsStruct);
}

/*==============================================================================
  FUNCTION:  usf_parse_transparent_data()
==============================================================================*/
/**
  See description in header file.
*/
int usf_parse_transparent_data (int transparent_data_size,
                                char const* transparent_data,
                                uint32_t* frame,
                                uint16_t* group,
                                uint16_t* skip)
{
  if (!frame ||
      !group ||
      !skip)
  {
    return -1;
  }

  // Separating the transparent data
  uint8_t parsedTransparent[USF_TX_TRANSPARENT_DATA_SIZE_ECHO];
  memcpy(parsedTransparent,
         transparent_data,
         USF_TX_TRANSPARENT_DATA_SIZE_ECHO);

  // Switching bits and adding lower bytes
  *skip = (parsedTransparent[UPPER_BYTE_SKIP]<<8) + parsedTransparent[LOWER_BYTE_SKIP];
  *group = (parsedTransparent[UPPER_BYTE_GROUP]<<8) + parsedTransparent[LOWER_BYTE_GROUP];
  *frame = (parsedTransparent[FORTH_BYTE_FRAME_ECHO]<<24) +
           (parsedTransparent[THIRD_BYTE_FRAME_ECHO]<<16) +
           (parsedTransparent[SECOND_BYTE_FRAME_ECHO]<<8) +
           parsedTransparent[FIRST_BYTE_FRAME_ECHO];

  LOGD("%s: transparent skip is %d",
       __FUNCTION__,
       *skip);
  LOGD("%s: transparent group is %d",
       __FUNCTION__,
       *group);
  LOGD("%s: transparent frame: %d",
       __FUNCTION__,
       *frame);

  return 0;
}

/*==============================================================================
  FUNCTION:  usf_parse_transparent_data()
==============================================================================*/
/**
  See description in header file.
*/
int usf_parse_transparent_data (int transparent_data_size,
                                char const* transparent_data,
                                uint16_t* frame,
                                uint16_t* group)
{
  // Separating the transparent data
  uint8_t parsedTransparent[USF_TX_TRANSPARENT_DATA_SIZE_EPOS];
  memcpy(parsedTransparent,
         transparent_data,
         USF_TX_TRANSPARENT_DATA_SIZE_EPOS);

  // Switching bits
  *frame = parsedTransparent[UPPER_BYTE_FRAME_EPOS]<<8;
  *group = parsedTransparent[UPPER_BYTE_GROUP]<<8;

  *frame += parsedTransparent[LOWER_BYTE_FRAME_EPOS];
  *group += parsedTransparent[LOWER_BYTE_GROUP];

  LOGD("%s: frame/skip is %d",
       __FUNCTION__,
       *frame);
  LOGD("%s: group is %d",
       __FUNCTION__,
       *group);

  return 0;
}

/*==============================================================================
  FUNCTION:  usf_dependencies_check_p2p()
==============================================================================*/
/**
 * This function validates the dependencies in p2p daemon's parameters.
 *
 * @param cfgFileName The path to the cfg file currently used.
 * @param paramsStruct A struct containing the cfg file parameters to be validated
 * @param skip The skip value of the current daemon's transparent data
 *
 * @return 0 - Success
 *         1 - Error, meaning one of the parameters checked is invalid.
 *         -1 - Null parameters
 */
static int usf_dependencies_check_p2p (char* cfgFileName,
                                       us_all_info* paramsStruct,
                                       uint16_t skip)
{
  if (!cfgFileName ||
      !paramsStruct)
  {
    return -1;
  }

  // usf_p2p_event_out_port
  char expected[MAX_VALUE_LEN];
  if ((0 != (paramsStruct->usf_p2p_event_dest & EVENT_DEST_OUT)) // Port must be used
      && (paramsStruct->usf_p2p_event_out_port < MIN_VALID_PORT_NUM ||
          paramsStruct->usf_p2p_event_out_port > MAX_VALID_PORT_NUM)) // Invalid port num
  {
    snprintf(expected,
             MAX_VALUE_LEN,
             "between %d and %d",
             MIN_VALID_PORT_NUM,
             MAX_VALID_PORT_NUM);
    print_error_message_i(cfgFileName,
                          "usf_p2p_event_out_port",
                          expected,
                          paramsStruct->usf_p2p_event_out_port);
    return 1;
  }

  // usf_p2p_device_uid
  if (P2P_UID0 != paramsStruct->usf_p2p_device_uid &&
      P2P_UID1 != paramsStruct->usf_p2p_device_uid &&
      P2P_UID2 != paramsStruct->usf_p2p_device_uid)
  {
    snprintf(expected,
             MAX_VALUE_LEN,
             "%d or %d or %d",
             P2P_UID0,
             P2P_UID1,
             P2P_UID2);
    print_error_message_i(cfgFileName,
                          "usf_p2p_device_uid",
                          expected,
                          paramsStruct->usf_p2p_device_uid);
    return 1;
  }

  // usf_p2p_pattern_type
  if (P2P_PATTERN_TYPE0 != paramsStruct->usf_p2p_pattern_type &&
      P2P_PATTERN_TYPE1 != paramsStruct->usf_p2p_pattern_type &&
      P2P_PATTERN_TYPE2 != paramsStruct->usf_p2p_pattern_type)
  {
    snprintf(expected,
             MAX_VALUE_LEN,
             "%d or %d or %d",
             P2P_PATTERN_TYPE0,
             P2P_PATTERN_TYPE1,
             P2P_PATTERN_TYPE2);
    print_error_message_i(cfgFileName,
                          "usf_p2p_pattern_type",
                          expected,
                          paramsStruct->usf_p2p_pattern_type);
    return 1;
  }

  return 0;
}

/*==============================================================================
  FUNCTION:  usf_dependencies_check_proximity()
==============================================================================*/
/**
 * This function validates the dependencies in proximity daemon's parameters.
 *
 * @param cfgFileName The path to the cfg file currently used.
 * @param paramsStruct A struct containing the cfg file parameters to be validated
 *
 * @return 0 - Success
 *         1 - Error, meaning one of the parameters checked is invalid.
 *         -1 - Null parameters
 */
static int usf_dependencies_check_proximity(char* cfgFileName,
                                            us_all_info* paramsStruct)
{
  if (!cfgFileName ||
      !paramsStruct)
  {
    return -1;
  }

  char expected[MAX_VALUE_LEN];
  // RX check for proximity
  if (USF_RX_TRANSPARENT_PROX_DATA_SIZE !=
            paramsStruct->usf_rx_transparent_data_size)
  {
    snprintf(expected,
             MAX_VALUE_LEN,
             "%d",
             USF_RX_TRANSPARENT_PROX_DATA_SIZE);
    print_error_message_i(cfgFileName,
                          "usf_rx_transparent_data_size",
                          expected,
                          paramsStruct->usf_rx_transparent_data_size);
    return 1;
  }

  if (MIN_PROX_OUTPUT_VAL > paramsStruct->usf_output_type ||
      MAX_PROX_OUTPUT_VAL < paramsStruct->usf_output_type)
  {
    snprintf(expected,
             MAX_VALUE_LEN,
             "between %d and %d",
             MIN_PROX_OUTPUT_VAL,
             MAX_PROX_OUTPUT_VAL);
    print_error_message_i(cfgFileName,
                          "usf_output_type",
                          expected,
                          paramsStruct->usf_output_type);
    return 1;
  }

  if (!(OUTPUT_TYPE_RAW_MASK & paramsStruct->usf_output_type) &&
      0 < paramsStruct->usf_frame_count)
  {
    LOGE("%s: Cannot use recording in proximity daemon when output type does "
         "not include raw data",
         __FUNCTION__);
    return 1;
  }
  return 0;
}

/*==============================================================================
  FUNCTION:  usf_dependencies_check_sync_gesture()
==============================================================================*/
/**
 * This function validates the dependencies in sync gesture daemon's parameters.
 *
 * @param cfgFileName The path to the cfg file currently used.
 * @param paramsStruct A struct containing the cfg file parameters to be validated
 *
 * @return 0 - Success
 *         1 - Error, meaning one of the parameters checked is invalid.
 *         -1 - Null parameters
 */
static int usf_dependencies_check_sync_gesture (char* cfgFileName,
                                                us_all_info* paramsStruct)
{
  if (!cfgFileName ||
      !paramsStruct)
  {
    return -1;
  }

  char expected[MAX_VALUE_LEN];

  // usf_output_type
  if (OUTPUT_TYPE_RAW_MASK != paramsStruct->usf_output_type &&
      OUTPUT_TYPE_GESTURE_EVENT_MASK != paramsStruct->usf_output_type &&
      ((OUTPUT_TYPE_GESTURE_EVENT_MASK | OUTPUT_TYPE_RAW_MASK) != paramsStruct->usf_output_type))
  {
    snprintf(expected,
             MAX_VALUE_LEN,
             "%d or %d or %d",
             OUTPUT_TYPE_RAW_MASK,
             OUTPUT_TYPE_GESTURE_EVENT_MASK,
             (OUTPUT_TYPE_GESTURE_EVENT_MASK |
              OUTPUT_TYPE_RAW_MASK));
    print_error_message_i(cfgFileName,
                          "usf_output_type",
                          expected,
                          paramsStruct->usf_output_type);
    return 1;
  }

  if (!(OUTPUT_TYPE_RAW_MASK & paramsStruct->usf_output_type) &&
      0 < paramsStruct->usf_frame_count)
  {
    LOGE("%s: Cannot use recording in sync gesture daemon when output type does not include raw data",
         __FUNCTION__);
    return 1;
  }

  return 0;
}

/*==============================================================================
  FUNCTION:  usf_dependencies_check_gesture()
==============================================================================*/
/**
 * This function validates the dependencies in gesture daemon's parameters.
 *
 * @param cfgFileName The path to the cfg file currently used.
 * @param paramsStruct A struct containing the cfg file parameters to be validated
 * @param skip The skip value of the current daemon's transparent data
 *
 * @return 0 - Success
 *         1 - Error, meaning one of the parameters checked is invalid.
 *         -1 - Null parameters
 */
static int usf_dependencies_check_gesture (char* cfgFileName,
                                           us_all_info* paramsStruct,
                                           uint16_t skip)
{
  if (!cfgFileName ||
      !paramsStruct)
  {
    return -1;
  }

  char expected[MAX_VALUE_LEN];

  // usf_tx_sample_rate
  if (USF_TX_SAMPLE_RATE_ECHO_GESTURE != paramsStruct->usf_tx_sample_rate)
  {
    snprintf(expected,
             MAX_VALUE_LEN,
             "%d",
             USF_TX_SAMPLE_RATE_ECHO_GESTURE);
    print_error_message_i(cfgFileName,
                          "usf_tx_sample_rate",
                          expected,
                          paramsStruct->usf_tx_sample_rate);
    return 1;
  }

  // usf_gesture_event_dest
  // User asked for only UAL, but supplied a path in usf_adapter_lib.
  if (0 != strcmp(paramsStruct->usf_adapter_lib,
                  "") &&
      0 == (paramsStruct->usf_gesture_event_dest & EVENT_DEST_OUT) &&
      0 != (paramsStruct->usf_gesture_event_dest & EVENT_DEST_UAL))
  {
    snprintf(expected,
             MAX_VALUE_LEN,
             "%d or %d when usf_adapter_lib not empty",
             EVENT_DEST_OUT,
             EVENT_DEST_OUT | EVENT_DEST_UAL);
    print_error_message_i(cfgFileName,
                          "usf_gesture_event_dest",
                          expected,
                          paramsStruct->usf_gesture_event_dest);
    return 1;
  }
  else if (0 == (paramsStruct->usf_gesture_event_dest & EVENT_DEST_OUT) &&
           0 == (paramsStruct->usf_gesture_event_dest & EVENT_DEST_UAL))
  {
    snprintf(expected,
             MAX_VALUE_LEN,
             "%d or %d or %d",
             EVENT_DEST_OUT,
             EVENT_DEST_UAL,
             EVENT_DEST_UAL | EVENT_DEST_OUT);
    print_error_message_i(cfgFileName,
                          "usf_gesture_event_dest",
                          expected,
                          paramsStruct->usf_gesture_event_dest);
    return 1;
  }
  else if (0 == strcmp(paramsStruct->usf_adapter_lib,
                       "") &&
           0 != (paramsStruct->usf_gesture_event_dest & EVENT_DEST_OUT))
  {
    snprintf(expected,
             MAX_VALUE_LEN,
             "adapter_lib not empty");
    print_error_message_i(cfgFileName,
                          "usf_gesture_event_dest",
                          expected,
                          paramsStruct->usf_gesture_event_dest);
    return 1;
  }

  return 0;
}

/*==============================================================================
  FUNCTION:  usf_dependencies_check_hovering()
==============================================================================*/
/**
 * This function validates the dependencies in hovering daemon's parameters.
 *
 * @param cfgFileName The path to the cfg file currently used.
 * @param paramsStruct A struct containing the cfg file parameters to be validated
 * @param skip The skip value of the current daemon's transparent data
 *
 * @return 0 - Success
 *         1 - Error, meaning one of the parameters checked is invalid.
 *         -1 - Null parameters
 */
static int usf_dependencies_check_hovering (char* cfgFileName,
                                            us_all_info* paramsStruct,
                                            uint16_t skip)
{
  if (!cfgFileName ||
      !paramsStruct)
  {
    return -1;
  }

  // usf_hovering_event_out_port
  char expected[MAX_VALUE_LEN];
  if ((0 != (paramsStruct->usf_hovering_event_dest & EVENT_DEST_OUT)) // Port must be used
      && (paramsStruct->usf_hovering_event_out_port < MIN_VALID_PORT_NUM ||
          paramsStruct->usf_hovering_event_out_port > MAX_VALID_PORT_NUM)) // Invalid port num
  {
    snprintf(expected,
             MAX_VALUE_LEN,
             "between %d and %d",
             MIN_VALID_PORT_NUM,
             MAX_VALID_PORT_NUM);
    print_error_message_i(cfgFileName,
                          "usf_hovering_event_out_port",
                          expected,
                          paramsStruct->usf_hovering_event_out_port);
    return 1;
  }

  return 0;
}


/*==============================================================================
  FUNCTION:  print_error_message()
==============================================================================*/
/**
 * This function prints an error to the screen containing the invalid cfg parameter,
 * and the actual and expected values.
 *
 * @param cfgFileName The path to the cfg file currently used.
 * @param param_name The name of the invalid parameter
 * @param expected The expected value of the parameter
 * @param actual The actual value of the parameter
 */
static void print_error_message(char* cfgFileName,
                                const char* param_name,
                                const char* expected,
                                char* actual)
{
  if (!cfgFileName ||
      !param_name ||
      !expected ||
      !actual)
  {
    LOGE("%s, null parameters",
         __FUNCTION__);
    return;
  }

  LOGW("Cfg file: %s for parameter %s expected: %s, actual: %s",
       cfgFileName,
       param_name,
       expected,
       actual);
}

/*==============================================================================
  FUNCTION:  print_error_message_i()
==============================================================================*/
/**
 * This function prints an error to the screen for integer values.
 *
 * @param cfgFileName The path to the cfg file currently used.
 * @param param_name The name of the invalid parameter
 * @param expected The expected value of the parameter
 * @param actual The actual value of the parameter
 */
static void print_error_message_i(char* cfgFileName,
                                  const char* param_name,
                                  const char* expected,
                                  int actual)
{
  char tmp[MAX_VALUE_LEN];
  snprintf(tmp,
           MAX_VALUE_LEN,
           "%d",
           actual);
  print_error_message(cfgFileName,
                      param_name,
                      expected,
                      tmp);
}
