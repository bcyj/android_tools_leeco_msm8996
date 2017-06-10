/*===========================================================================
                           ual_util.cpp

DESCRIPTION: Provide common functions implementation to all usf daemons.


INITIALIZATION AND SEQUENCING REQUIREMENTS: None

Copyright (c) 2011-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
=============================================================================*/
#define LOG_TAG "ual_util"

/*----------------------------------------------------------------------------
Include files
----------------------------------------------------------------------------*/
#include "usf_log.h"
#include <sys/stat.h>
#include <cutils/properties.h>
#include <stdlib.h>
#include "ual_util.h"
#include <gui/SurfaceComposerClient.h>
#include <gui/ISurfaceComposer.h>
#include <ui/DisplayInfo.h>
#include "usf_validation.h"
#include "ual_util_frame_file.h"

/*----------------------------------------------------------------------------
  Defines
----------------------------------------------------------------------------*/
#define FORM_FILE "/data/usf/form_factor.cfg"
#define BUFFER_SIZE 500
#define MIN_DIM_SIZE 0
#define MAX_LEN_STR_BYTE 5
#define OPEN_RETRIES 10
#define UAL_OPEN_SLEEP_TIME   0.1
/*-----------------------------------------------------------------------------
  Typedefs
-----------------------------------------------------------------------------*/
/**
  This struct containe info anout one param from the cfg file.
*/
typedef struct
{
  char paramName[FILE_PATH_MAX_LEN];
  char paramValue[FILE_PATH_MAX_LEN];
} paramStruct;

/**
  EchoTxTransparentData holds the tx transparent data that is
  going to be inserted manually, since we now read it from a
  file and the gesture, hovering, and p2p
  tx_transparent_data_file will be empty.
 */
typedef struct
{
  uint16_t skip;
  uint16_t group;
  uint32_t frame_size;
} EchoTxTransparentData;

/**
  EchoRxTransparentData holds the rx transparent data that is
  going to be inserted manually, since we now read it from a
  file and the gesture, hovering, and p2p
  rx_transparent_data_file will be empty.
 */
typedef struct
{
  uint16_t frame_size;
  uint16_t group;
} EchoRxTransparentData;

/**
   EposTxTransparentData holds the tx transparent data that is
   going to be inserted manually, since we now read it from a
   file and the epos tx_transparent_data_file will be empty.
 */
typedef struct
{
    uint16_t skip;
    uint16_t group;
} EposTxTransparentData;

/*-----------------------------------------------------------------------------
  Static Variable Definitions
-----------------------------------------------------------------------------*/
/**
  Holds info about speakers and mics.
*/
static FormStruct formStruct;

/**
  Number of the required speakers
*/
static uint16_t s_req_speakers_num = 0;
/**
  The required speakers identification
*/
static uint8_t s_req_speakers_id[US_FORM_FACTOR_CONFIG_MAX_SPEAKERS];

/**
  Number of the required mics
*/
static uint16_t s_req_mics_num = 0;
/**
  The required mics identification
*/
static uint8_t s_req_mics_id[US_FORM_FACTOR_CONFIG_MAX_MICS];

/**
 * The path to store files containing the pid of the daemon
 */
static const char *PID_DIRECTORY = "/data/usf";

/**
 * Value for wav frame file format
 */
static const uint32_t WAVE_FORMAT = 1;

/**
 * The default prefix for the is supported property name
 */
static const char *IS_SUPPORTED_PROP_PREFIX = "ro.qc.sdk.us.";

/*------------------------------------------------------------------------------
  Function definitions
------------------------------------------------------------------------------*/
static int ual_util_trim (char* value,
                          int size);

/*==============================================================================
  FUNCTION:  ual_util_fill_mic_info
==============================================================================*/
/**
  This function receives mic num and string of this mic info and
  scanf the string into the formStruct. Return value match to the
  ret value needed in the function ual_util_parse_cfg_file.
*/
int ual_util_fill_mic_info (int micIndex,
                            char* micParamInfo)
{
  if (US_FORM_FACTOR_CONFIG_MAX_MICS <= micIndex ||
      0 > micIndex ||
      NULL == micParamInfo)
  {
    LOGW("%s: Bad params.",
         __FUNCTION__);
    return -1;
  }
  int ret = sscanf(micParamInfo,
                   "%d ,%f ,%f",
                   &formStruct.mics_id[micIndex],
                   &formStruct.mics_info[micIndex][X_IND],
                   &formStruct.mics_info[micIndex][Y_IND]);
  if (3 == ret)
  {
    LOGD("%s: mic_info %d is: %d, %f, %f",
         __FUNCTION__,
         micIndex,
         formStruct.mics_id[micIndex],
         formStruct.mics_info[micIndex][X_IND],
         formStruct.mics_info[micIndex][Y_IND]);

    return 1;
  }
  else
  {
    return 0;
  }
}

/*==============================================================================
  FUNCTION:  ual_util_fill_speaker_info
==============================================================================*/
/**
  This function receives speaker num and string of this speaker info and
  scanf the string into the formStruct. Return value match to the
  ret value needed in the function ual_util_parse_cfg_file.
*/
int ual_util_fill_speaker_info (int speakerIndex,
                                char* speakerParamInfo)
{
  if (US_FORM_FACTOR_CONFIG_MAX_SPEAKERS <= speakerIndex ||
      0 > speakerIndex ||
      NULL == speakerParamInfo)
  {
    LOGW("%s: Bad params.",
         __FUNCTION__);
    return -1;
  }
  int ret = sscanf(speakerParamInfo,
                   "%d ,%f ,%f",
                   &formStruct.speakers_id[speakerIndex],
                   &formStruct.speakers_info[speakerIndex][X_IND],
                   &formStruct.speakers_info[speakerIndex][Y_IND]);
  if (3 == ret)
  {
    LOGD("%s: speaker_info %d is: %d, %f, %f",
         __FUNCTION__,
         speakerIndex,
         formStruct.speakers_id[speakerIndex],
         formStruct.speakers_info[speakerIndex][X_IND],
         formStruct.speakers_info[speakerIndex][Y_IND]);

    return 1;
  }
  else
  {
    return 0;
  }
}

/*==============================================================================
  FUNCTION:  ual_util_parse_line
==============================================================================*/
/**
  This function is for internal use only of the function parseCfgCile().
  The function receive line and parse it - if the line is comment or
  empty line it returns -1, if failure it returns -2, otherwise it returns in
  paramInfo the param name and the param value and return value is 0.
*/
int ual_util_parse_line (char *line,
                         paramStruct *paramInfo)
{
  uint i = 0, j = 0, lineLen;

  if (NULL == line ||
      NULL == paramInfo)
  {
    return -2;

  }
  if (line[0] == '#' ||
      line[0] == '\n' ||
      line[0] == ' ')
  {
    return -1;
  }

  lineLen = strlen(line);
  for (i = 0; i < lineLen; i++)
  {
    if (line[i] == ' ' ||
        line[i] == '\t' ||
        line[i] == '\n')
    {
      break;
    }
    paramInfo->paramName[i] = line[i];
  }

  paramInfo->paramName[i] = '\0';
  while ( ('[' != line[i]) &&
          (i < lineLen) )
  {
    i++;
  }
  i++;

  // Ignoring spaces at the beginning
  while ( (' ' == line[i]) &&
          (i < lineLen) )
  {
    i++;
  }

  while ( (']' != line[i]) &&
          (i < lineLen) )
  {
    paramInfo->paramValue[j] = line[i];
    i++;
    j++;
  }

  return ual_util_trim(paramInfo->paramValue, j);
}

/*==============================================================================
  FUNCTION:  ual_util_trim
==============================================================================*/
/**
 * This function cuts off the last spaces in a given value
 * @param value the value to trim
 * @param length the length of the value
 *
 * @return int 0 in case of success
 *             -2 in case of error
 */
static int ual_util_trim (char* value,
                          int length)
{
  if (NULL == value ||
      0 > length)
  {
    return -2;
  }

  int j = length;

  // Trim
  while ((0 <= --j) &&
         (' ' == value[j]));

  value[j+1] = '\0';

  return 0;
}

/*==============================================================================
  FUNCTION:  ual_util_get_transparent_data
==============================================================================*/
/**
  This function gets the transparent data from a file. if the file path is
  empty than the transparent data will be built manualy later (all daemons except
  tester and proximity)

  @param ps_transparent_data_string - value read from cfg file (in params struct)
  @param ps_transparent_data - pointer to transparent data byte array in the params struct
  @param ps_transparent_data_size - pointer to transparent data size field in the params struct
  @param type - string containing "rx" or "tx"
  @return 0 if all goes well. -1 on error.
*/

static int ual_util_get_transparent_data(const char *ps_transparent_data_string,
                                         char *ps_transparent_data,
                                         uint32_t *ps_transparent_data_size,
                                         const char *type)
{
  char *transparent_data;
  uint32_t transparent_data_size;

  // Empty transparent data file means the transparent data is built manually later.
  // File exists only in proximity and tester daemons.
  if (0 == strcmp(ps_transparent_data_string,""))
  {
    *ps_transparent_data_size = 0;
    LOGD("%s: usf_%s_transparent_data_size is: %d",
         __FUNCTION__,
         type,
         *ps_transparent_data_size);
  }
  else
  {
    // Transparent data file exists. Read it and copy to the params struct.
    transparent_data = (char*) ual_util_malloc_read(ps_transparent_data_string,
                                                    transparent_data_size);
    if (NULL == transparent_data)
    {
      LOGE("%s: NULL transparent_data",
           __FUNCTION__);
      return -1;
    }

    if (MAX_TRANSPARENT_DATA_SIZE < transparent_data_size)
    {
      free(transparent_data);
      LOGE("%s: usf_%s_transparent_data_size is too big: %d, exceed %d",
           __FUNCTION__,
           type,
           transparent_data_size,
           MAX_TRANSPARENT_DATA_SIZE);
      return -1;
    }
    memcpy(ps_transparent_data,
           transparent_data,
           transparent_data_size);
    *ps_transparent_data_size = transparent_data_size;
    LOGD("%s: usf_%s_transparent_data_size is: %u",
         __FUNCTION__,
         type,
         *ps_transparent_data_size);

    char transparent_data_string[4*MAX_TRANSPARENT_DATA_SIZE] = {0}; // Max size is 4* since each byte is at most "255,"
    for (unsigned int i = 0; i < *ps_transparent_data_size; ++i)
    {
        char byte[MAX_LEN_STR_BYTE];
        snprintf(byte, MAX_LEN_STR_BYTE, "%d,", transparent_data[i]);
        strncat(transparent_data_string, byte, MAX_LEN_STR_BYTE);
    }
    transparent_data_string[strlen(transparent_data_string)-1] = '\0'; // Remove last ','
    LOGD("%s: usf_%s_transparent_data is: %s",
         __FUNCTION__,
         type,
         transparent_data_string);
    free(transparent_data);
  }
  return 0;
}

/*==============================================================================
  FUNCTION:  ual_util_parse_cfg_file
==============================================================================*/
/**
  See function description at header file under the function declaration.
*/
int ual_util_parse_cfg_file (FILE *cfgFile,
                             us_all_info *paramsStruct)
{

  int gotten, i, numOfFieldsInCfgFile = -1, ret = 1, rc;
  char *buffer, *bufferTemp, *save_pointer;
  char *paramName = NULL, *temp = NULL, *line = NULL;
  paramStruct paramInfo;

  if (NULL == cfgFile ||
      NULL == paramsStruct)
  {
    LOGW("%s: Null parameter.",
         __FUNCTION__);
    return -1;
  }
  // Set all params to 0 as default.
  memset(paramsStruct,
         0,
         sizeof(us_all_info));

  // Read from config file
  fseek (cfgFile,
         0,
         SEEK_END);
  int nConfigLen = ftell (cfgFile);
  fseek (cfgFile,
         0,
         SEEK_SET);
  bufferTemp = (char *)malloc (nConfigLen + 1);
  if (NULL == bufferTemp)
  {
    LOGW("%s: Could not alloc %d bytes for EPOS calib. Abort.",
         __FUNCTION__,
         nConfigLen + 1);
    return -1;
  }
  buffer = bufferTemp;

  gotten = fread(buffer,
                 sizeof(char),
                 nConfigLen,
                 cfgFile);
  if (0 >= gotten)
  {
    LOGW("%s: fread failed",
         __FUNCTION__);
  }
  else
  {
    buffer[gotten] = '\0';
    line = strtok_r(buffer,
                    "\n",
                    &save_pointer);
    numOfFieldsInCfgFile = 0;
  }

  // If fread failed then line is NULL and numOfFieldsInCfgFile is -1
  while (NULL != line)
  {
    int rc = ual_util_parse_line(line,
                                 &paramInfo);
    if (-1 == rc)
    {
      line = strtok_r(NULL,
                      "\n",
                      &save_pointer);
      continue;
    }
    paramName = paramInfo.paramName;
    temp = paramInfo.paramValue;
    LOGD("%s: paramName: %s, paramValue: %s",
         __FUNCTION__,
         paramName,
         temp);
    line = strtok_r(NULL,
                    "\n",
                    &save_pointer);

    if (strcmp(paramName,"usf_device_id") == 0)
    {
      ret = sscanf(temp,
                   "%d",
                   &paramsStruct->usf_device_id);
      LOGD("%s: usf_device_id: %d",
           __FUNCTION__,
           paramsStruct->usf_device_id);
    }

    // Tx perams
    else if (strcmp(paramName,"usf_tx_data_format") == 0)
    {
      ret = sscanf(temp,
                   "%d",
                   &paramsStruct->usf_tx_data_format);
      LOGD("%s: usf_tx_data_format is: %d",
           __FUNCTION__,
           paramsStruct->usf_tx_data_format);
    }
    else if (strcmp(paramName,"usf_tx_sample_rate") == 0)
    {
      ret = sscanf(temp,
                   "%d",
                   &paramsStruct->usf_tx_sample_rate);
      LOGD("%s: usf_tx_sample_rate is: %d",
           __FUNCTION__,
           paramsStruct->usf_tx_sample_rate);
      paramsStruct->use_tx = true;
    }
    else if (strcmp(paramName,"usf_tx_sample_width") == 0)
    {
      ret = sscanf(temp,
                   "%hu",
                   &paramsStruct->usf_tx_sample_width);
      LOGD("%s: usf_tx_sample_width is: %d",
           __FUNCTION__,
           paramsStruct->usf_tx_sample_width);
    }
    else if (strcmp(paramName,"usf_tx_port_count") == 0)
    {
      ret = sscanf(temp,
                   "%hu",
                   &paramsStruct->usf_tx_port_count);

      LOGD("%s: usf_tx_port_count is: %d",
           __FUNCTION__,
           paramsStruct->usf_tx_port_count);
    }
    else if (strcmp(paramName,"usf_tx_ports") == 0)
    {
      strlcpy(paramsStruct->usf_tx_ports,
              temp,
              FILE_PATH_MAX_LEN);
      LOGD("%s: usf_tx_ports is: %s",
           __FUNCTION__,
           paramsStruct->usf_tx_ports);
    }
    else if (strcmp(paramName,"usf_tx_frame_hdr_size") == 0)
    {
      ret = sscanf(temp,
                   "%hu",
                   &paramsStruct->usf_tx_frame_hdr_size);
      LOGD("%s: usf_tx_frame_hdr_size is: %d",
           __FUNCTION__,
           paramsStruct->usf_tx_frame_hdr_size);
    }
    else if (strcmp(paramName,"usf_tx_port_data_size") == 0)
    {
      ret = sscanf(temp,
                   "%hu",
                   &paramsStruct->usf_tx_port_data_size);
      LOGD("%s: usf_tx_port_data_size is: %d",
           __FUNCTION__,
           paramsStruct->usf_tx_port_data_size);
    }
    else if (strcmp(paramName,"usf_tx_queue_capacity") == 0)
    {
      ret = sscanf(temp,
                   "%hu",
                   &paramsStruct->usf_tx_queue_capacity);
      LOGD("%s: usf_tx_queue_capacity is: %d",
           __FUNCTION__,
           paramsStruct->usf_tx_queue_capacity);
    }
    else if (strcmp(paramName,"usf_tx_transparent_data_file") == 0)
    {
      ual_util_get_transparent_data(temp,paramsStruct->usf_tx_transparent_data,&paramsStruct->usf_tx_transparent_data_size,"tx");
    }
    else if (strcmp(paramName,"usf_tx_buf_size") == 0)
    {
      ret = sscanf(temp,
                   "%d",
                   &paramsStruct->usf_tx_buf_size);
      LOGD("%s: usf_tx_buf_size is: %d",
           __FUNCTION__,
           paramsStruct->usf_tx_buf_size);
    }
    else if (strcmp(paramName,"usf_tx_max_get_set_param_buf_size") == 0)
    {
      ret = sscanf(temp,
                   "%d",
                   &paramsStruct->usf_tx_max_get_set_param_buf_size);
      LOGD("%s: usf_tx_max_get_set_param_buf_size is: %d",
           __FUNCTION__,
           paramsStruct->usf_tx_max_get_set_param_buf_size);
    }

    // Rx params
    else if (strcmp(paramName,"usf_rx_pattern") == 0)
    {
      strlcpy(paramsStruct->usf_rx_pattern,
              temp,
              FILE_PATH_MAX_LEN);
      LOGD("%s: usf_rx_pattern is: %s",
           __FUNCTION__,
           paramsStruct->usf_rx_pattern);
    }
    else if (strcmp(paramName, "usf_rx_data_format") == 0)
    {
      ret = sscanf(temp,
                   "%d",
                   &paramsStruct->usf_rx_data_format);
      LOGD("%s: usf_rx_data_format is: %d",
           __FUNCTION__,
           paramsStruct->usf_rx_data_format);
    }
    else if (strcmp(paramName,"usf_rx_sample_rate") == 0)
    {
      ret = sscanf(temp,
                   "%d",
                   &paramsStruct->usf_rx_sample_rate);
      LOGD("%s: usf_rx_sample_rate is: %d",
           __FUNCTION__,
           paramsStruct->usf_rx_sample_rate);
      paramsStruct->use_rx = true;
    }
    else if (strcmp(paramName,"usf_rx_sample_width") == 0)
    {
      ret = sscanf(temp,
                   "%hu",
                   &paramsStruct->usf_rx_sample_width);
      LOGD("%s: usf_rx_sample_width is: %d",
           __FUNCTION__,
           paramsStruct->usf_rx_sample_width);
    }
    else if (strcmp(paramName,"usf_rx_port_count") == 0)
    {
      ret = sscanf(temp,
                   "%hu",
                   &paramsStruct->usf_rx_port_count);
      LOGD("%s: usf_rx_port_count is: %d",
           __FUNCTION__,
           paramsStruct->usf_rx_port_count);
    }
    else if (strcmp(paramName,"usf_rx_ports") == 0)
    {
      strlcpy(paramsStruct->usf_rx_ports,
              temp,
              FILE_PATH_MAX_LEN);
      LOGD("%s: usf_rx_ports is: %s",
           __FUNCTION__,
           paramsStruct->usf_rx_ports);
    }
    else if (strcmp(paramName, "usf_rx_frame_hdr_size") == 0)
    {
      ret = sscanf(temp,
                   "%hu",
                   &paramsStruct->usf_rx_frame_hdr_size);
      LOGD("%s: usf_rx_frame_hdr_size is: %d",
           __FUNCTION__,
           paramsStruct->usf_rx_frame_hdr_size);
    }
    else if (strcmp(paramName, "usf_rx_port_data_size") == 0)
    {
      ret = sscanf(temp,
                   "%hu",
                   &paramsStruct->usf_rx_port_data_size);
      LOGD("%s: usf_rx_port_data_size is: %d",
           __FUNCTION__,
           paramsStruct->usf_rx_port_data_size);
    }
    else if (strcmp(paramName, "usf_rx_queue_capacity") == 0)
    {
      ret = sscanf(temp,
                   "%hu",
                   &paramsStruct->usf_rx_queue_capacity);
      LOGD("%s: usf_rx_queue_capacity is: %d",
           __FUNCTION__,
           paramsStruct->usf_rx_queue_capacity);
    }
    else if (strcmp(paramName, "usf_rx_pattern_size") == 0)
    {
      ret = sscanf(temp,
                   "%d",
                   &paramsStruct->usf_rx_pattern_size);
      LOGD("%s: usf_rx_pattern_size is: %d",
           __FUNCTION__,
           paramsStruct->usf_rx_pattern_size);
    }
    else if (strcmp(paramName, "usf_rx_transparent_data_file") == 0)
    {
       ual_util_get_transparent_data(temp,paramsStruct->usf_rx_transparent_data,&paramsStruct->usf_rx_transparent_data_size,"rx");
    }
    else if (strcmp(paramName, "usf_rx_buf_size") == 0)
    {
      ret = sscanf(temp,
                   "%d",
                   &paramsStruct->usf_rx_buf_size);
      LOGD("%s: usf_rx_buf_size is: %d",
           __FUNCTION__,
           paramsStruct->usf_rx_buf_size);
    }
    else if (strcmp(paramName,"usf_rx_max_get_set_param_buf_size") == 0)
    {
      ret = sscanf(temp,
                   "%d",
                   &paramsStruct->usf_rx_max_get_set_param_buf_size);
      LOGD("%s: usf_rx_max_get_set_param_buf_size is: %d",
           __FUNCTION__,
           paramsStruct->usf_rx_max_get_set_param_buf_size);
    }

    // Common params
    else if (strcmp(paramName, "usf_frame_file") == 0)
    {
      strlcpy(paramsStruct->usf_frame_file,
              temp,
              FILE_PATH_MAX_LEN);
      LOGD("%s: usf_frame_file is: %s",
           __FUNCTION__,
           paramsStruct->usf_frame_file);
    }
    else if (strcmp(paramName, "usf_frame_count") == 0)
    {
      ret = sscanf(temp,
                   "%d",
                   &paramsStruct->usf_frame_count);
      LOGD("%s: usf_frame_count is: %d",
           __FUNCTION__,
           paramsStruct->usf_frame_count);
    }
    else if (strcmp(paramName, "usf_frame_file_format") == 0)
    {
      ret = sscanf(temp,
                   "%d",
                   &paramsStruct->usf_frame_file_format);
      LOGD("%s: usf_frame_file_format is: %d",
           __FUNCTION__,
           paramsStruct->usf_frame_file_format);
    }

    // Epos params
    else if (strcmp(paramName, "usf_epos_on_screen_event_dest") == 0)
    {
      ret = sscanf(temp,
                   "%d",
                   &paramsStruct->usf_epos_on_screen_event_dest);
      LOGD("%s: usf_epos_on_screen_event_dest is: %d",
           __FUNCTION__,
           paramsStruct->usf_epos_on_screen_event_dest);
    }
    else if (strcmp(paramName, "usf_epos_off_screen_event_dest") == 0)
    {
      ret = sscanf(temp,
                   "%d",
                   &paramsStruct->usf_epos_off_screen_event_dest);
      LOGD("%s: usf_epos_off_screen_event_dest is: %d",
           __FUNCTION__,
           paramsStruct->usf_epos_off_screen_event_dest);
    }
    else if (strcmp(paramName, "usf_epos_coord_type_on_disp") == 0)
    {
      ret = sscanf(temp,
                   "%d",
                   &paramsStruct->usf_epos_coord_type_on_disp);
      LOGD("%s: usf_epos_coord_type_on_disp is: %d",
           __FUNCTION__,
           paramsStruct->usf_epos_coord_type_on_disp);
    }
    else if (strcmp(paramName, "usf_epos_coord_type_off_disp") == 0)
    {
      ret = sscanf(temp,
                   "%d",
                   &paramsStruct->usf_epos_coord_type_off_disp);
      LOGD("%s: usf_epos_coord_type_off_disp is: %d",
           __FUNCTION__,
           paramsStruct->usf_epos_coord_type_off_disp);
    }
    else if (strcmp(paramName, "req_buttons_bitmap") == 0)
    {
      ret = sscanf(temp,
                   "%hu",
                   &paramsStruct->req_buttons_bitmap);
      LOGD("%s: req_buttons_bitmap is: %hu",
           __FUNCTION__,
           paramsStruct->req_buttons_bitmap);
    }
    else if (strcmp(paramName, "eraser_button_mode") == 0)
    {
      ret = sscanf(temp,
                   "%hu",
                   &paramsStruct->eraser_button_mode);
      LOGD("%s: eraser_button_mode is: %hu",
           __FUNCTION__,
           paramsStruct->eraser_button_mode);
    }
    else if (strcmp(paramName, "eraser_button_index") == 0)
    {
      ret = sscanf(temp,
                   "%hu",
                   &paramsStruct->eraser_button_index);
      LOGD("%s: eraser_button_mode is: %hu",
           __FUNCTION__,
           paramsStruct->eraser_button_index);
    }
    // On screen transformation matrix points
    else if (strcmp(paramName, "usf_on_screen_transform_origin") == 0)
    {
      strlcpy(paramsStruct->usf_on_screen_transform_origin,
              temp,
              FILE_PATH_MAX_LEN);
      LOGD("%s: usf_on_screen_transform_origin is: %s",
           __FUNCTION__,
           paramsStruct->usf_on_screen_transform_origin);
    }
    else if (strcmp(paramName, "usf_on_screen_transform_end_X") == 0)
    {
      strlcpy(paramsStruct->usf_on_screen_transform_end_X,
              temp,
              FILE_PATH_MAX_LEN);
      LOGD("%s: usf_on_screen_transform_end_X is: %s",
           __FUNCTION__,
           paramsStruct->usf_on_screen_transform_end_X);
    }
    else if (strcmp(paramName, "usf_on_screen_transform_end_Y") == 0)
    {
      strlcpy(paramsStruct->usf_on_screen_transform_end_Y,
              temp,
              FILE_PATH_MAX_LEN);
      LOGD("%s: usf_on_screen_transform_end_Y is: %s",
           __FUNCTION__,
           paramsStruct->usf_on_screen_transform_end_Y);
    }
    else if (strcmp(paramName, "usf_epos_on_screen_hover_max_range") == 0)
    {
      ret = sscanf(temp,
                   "%d",
                   &paramsStruct->usf_epos_on_screen_hover_max_range);
      LOGD("%s: usf_epos_on_screen_hover_max_range is:  %d",
           __FUNCTION__,
           paramsStruct->usf_epos_on_screen_hover_max_range);
    }
    // Off screen transformation matrix points
    else if (strcmp(paramName, "usf_epos_off_screen_transform_origin") == 0)
    {
      strlcpy(paramsStruct->usf_epos_off_screen_transform_origin,
              temp,
              FILE_PATH_MAX_LEN);
      LOGD("%s: usf_epos_off_screen_transform_origin is: %s",
           __FUNCTION__,
           paramsStruct->usf_epos_off_screen_transform_origin);
    }
    else if (strcmp(paramName, "usf_epos_off_screen_transform_end_X") == 0)
    {
      strlcpy(paramsStruct->usf_epos_off_screen_transform_end_X,
              temp,
              FILE_PATH_MAX_LEN);
      LOGD("%s: usf_epos_off_screen_transform_end_X is: %s",
           __FUNCTION__,
           paramsStruct->usf_epos_off_screen_transform_end_X);
    }
    else if (strcmp(paramName, "usf_epos_off_screen_transform_end_Y") == 0)
    {
      strlcpy(paramsStruct->usf_epos_off_screen_transform_end_Y,
              temp,
              FILE_PATH_MAX_LEN);
      LOGD("%s: usf_epos_off_screen_transform_end_Y is: %s",
           __FUNCTION__,
           paramsStruct->usf_epos_off_screen_transform_end_Y);
    }
    else if (strcmp(paramName, "usf_epos_off_screen_hover_max_range") == 0)
    {
      ret = sscanf(temp,
                   "%d",
                   &paramsStruct->usf_epos_off_screen_hover_max_range);
      LOGD("%s: usf_epos_off_screen_hover_max_range is:  %d",
           __FUNCTION__,
           paramsStruct->usf_epos_off_screen_hover_max_range);
    }
    else if (strcmp(paramName, "epos_lib_max_trace_level") == 0)
    {
      ret = sscanf(temp,
                   "%hu",
                   &paramsStruct->epos_lib_max_trace_level);
      LOGD("%s: epos_lib_max_trace_level is: %hu",
           __FUNCTION__,
           paramsStruct->epos_lib_max_trace_level);
    }
    else if (strcmp(paramName, "usf_epos_on_screen_act_zone_border") == 0)
    {
      strlcpy(paramsStruct->usf_epos_on_screen_act_zone_border,
              temp,
              FILE_PATH_MAX_LEN);
      LOGD("%s: usf_epos_on_screen_act_zone_border is: %s",
           __FUNCTION__,
           paramsStruct->usf_epos_on_screen_act_zone_border);
    }
    else if (strcmp(paramName, "usf_epos_off_screen_act_zone_border") == 0)
    {
      strlcpy(paramsStruct->usf_epos_off_screen_act_zone_border,
              temp,
              FILE_PATH_MAX_LEN);
      LOGD("%s: usf_epos_off_screen_act_zone_border is: %s",
           __FUNCTION__,
           paramsStruct->usf_epos_off_screen_act_zone_border);
    }
    else if (strcmp(paramName, "usf_fuzz_params") == 0)
    {
      strlcpy(paramsStruct->usf_fuzz_params,
              temp,
              FILE_PATH_MAX_LEN);
      LOGD("%s: usf_fuzz_params is: %s",
           __FUNCTION__,
           paramsStruct->usf_fuzz_params);
    }
    else if (strcmp(paramName, "usf_tx_skip") == 0)
    {
      ret = sscanf(temp,
                   "%hu",
                   &paramsStruct->usf_tx_skip);
      LOGD("%s: usf_tx_skip is: %hu",
           __FUNCTION__,
           paramsStruct->usf_tx_skip);
    }
    else if (strcmp(paramName, "usf_tx_group") == 0)
    {
      ret = sscanf(temp,
                   "%hu",
                   &paramsStruct->usf_tx_group);
      LOGD("%s: usf_tx_group is: %hu",
           __FUNCTION__,
           paramsStruct->usf_tx_group);
    }
    else if (strcmp(paramName, "usf_rx_group") == 0)
    {
      ret = sscanf(temp,
                   "%hu",
                   &paramsStruct->usf_rx_group);
      LOGD("%s: usf_rx_group is: %hu",
           __FUNCTION__,
           paramsStruct->usf_rx_group);
    }
    else if (strcmp(paramName, "usf_epos_cfg_point_downscale") == 0)
    {
      ret = sscanf(temp,
                   "%d",
                   &paramsStruct->usf_epos_cfg_point_downscale);
      LOGD("%s: usf_epos_cfg_point_downscale is: %d",
           __FUNCTION__,
           paramsStruct->usf_epos_cfg_point_downscale);
    }
    else if (strcmp(paramName, "usf_epos_product_packet") == 0)
    {
      strlcpy(paramsStruct->usf_epos_calib_files[EPOS_PRODUCT_FILE],
              temp,
              FILE_PATH_MAX_LEN);
      LOGD("%s: usf_epos_product_packet is: %s",
           __FUNCTION__,
           paramsStruct->usf_epos_calib_files[EPOS_PRODUCT_FILE]);
    }
    else if (strcmp(paramName, "usf_epos_unit_packet") == 0)
    {
      strlcpy(paramsStruct->usf_epos_calib_files[EPOS_UNIT_FILE],
              temp,
              FILE_PATH_MAX_LEN);
      LOGD("%s: usf_epos_unit_packet is: %s",
           __FUNCTION__,
           paramsStruct->usf_epos_calib_files[EPOS_UNIT_FILE]);
    }
    else if (strcmp(paramName, "usf_epos_persistent_packet") == 0)
    {
      strlcpy(paramsStruct->usf_epos_calib_files[EPOS_PERSISTENT_FILE],
              temp,
              FILE_PATH_MAX_LEN);
      LOGD("%s: usf_epos_persistent_file is: %s",
           __FUNCTION__,
           paramsStruct->usf_epos_calib_files[EPOS_PERSISTENT_FILE]);
    }
    else if (strcmp(paramName, "usf_epos_series_packet") == 0)
    {
      strlcpy(paramsStruct->usf_epos_calib_files[EPOS_PEN_SERIES_FILE],
              temp,
              FILE_PATH_MAX_LEN);
      LOGD("%s: usf_epos_series_packet is: %s",
           __FUNCTION__,
           paramsStruct->usf_epos_calib_files[EPOS_PEN_SERIES_FILE]);
    }
    else if (strcmp(paramName, "usf_epos_coord_file") == 0)
    {
      strlcpy(paramsStruct->usf_epos_coord_file,
              temp,
              FILE_PATH_MAX_LEN);
      LOGD("%s: usf_epos_coord_file is: %s",
           __FUNCTION__,
           paramsStruct->usf_epos_coord_file);
    }
    else if (strcmp(paramName, "usf_epos_coord_count") == 0)
    {
      ret = sscanf(temp,
                   "%d",
                   &paramsStruct->usf_epos_coord_count);
      LOGD("%s: usf_epos_coord_count is: %d",
           __FUNCTION__,
           paramsStruct->usf_epos_coord_count);
    }
    else if (strcmp(paramName, "usf_epos_timeout_to_coord_rec") == 0)
    {
      ret = sscanf(temp,
                   "%d",
                   &paramsStruct->usf_epos_timeout_to_coord_rec);
      LOGD("%s: usf_epos_timeout_to_coord_rec is: %d",
           __FUNCTION__,
           paramsStruct->usf_epos_timeout_to_coord_rec);
    }
    else if (strcmp(paramName, "usf_epos_off_screen_mode") == 0)
    {
      ret = sscanf(temp,
                   "%d",
                   (int *)&paramsStruct->usf_epos_off_screen_mode);
      LOGD("%s: usf_epos_off_screen_mode is: %d",
           __FUNCTION__,
           paramsStruct->usf_epos_off_screen_mode);
    }
    else if (strcmp(paramName, "usf_epos_debug_print_interval") == 0)
    {
      ret = sscanf(temp,
                   "%d",
                   &paramsStruct->usf_epos_debug_print_interval);
      LOGD("%s: usf_epos_debug_print_interval is: %d",
           __FUNCTION__,
           paramsStruct->usf_epos_debug_print_interval);
    }
    else if (strcmp(paramName, "no_act_zone_sleep_duration") == 0)
    {
      ret = sscanf(temp,
                   "%hu",
                   &paramsStruct->no_act_zone_sleep_duration);
      LOGD("%s: no_act_zone_sleep_duration is: %hu",
           __FUNCTION__,
           paramsStruct->no_act_zone_sleep_duration);
    }
    else if (strcmp(paramName, "no_act_zone_probe_duration") == 0)
    {
      ret = sscanf(temp,
                   "%hu",
                   &paramsStruct->no_act_zone_probe_duration);
      LOGD("%s: no_act_zone_probe_duration is: %hu",
           __FUNCTION__,
           paramsStruct->no_act_zone_probe_duration);
    }
    else if (strcmp(paramName, "no_act_zone_empty_frames_count") == 0)
    {
      ret = sscanf(temp,
                   "%d",
                   &paramsStruct->no_act_zone_empty_frames_count);
      LOGD("%s: no_act_zone_empty_frames_count is: %d",
           __FUNCTION__,
           paramsStruct->no_act_zone_empty_frames_count);
    }
    else if (strcmp(paramName, "usf_epos_lib_path") == 0)
    {
      strlcpy(paramsStruct->usf_epos_lib_path,
              temp,
              FILE_PATH_MAX_LEN);
      LOGD("%s: usf_epos_lib_path is: %s",
           __FUNCTION__,
           paramsStruct->usf_epos_lib_path);
    }
    else if (strcmp(paramName, "usf_epos_smarter_stand_angle") == 0)
    {
      ret = sscanf(temp,
                   "%lf",
                   &paramsStruct->usf_epos_smarter_stand_angle);
      LOGD("%s: usf_epos_smarter_stand_angle is: %lf",
           __FUNCTION__,
           paramsStruct->usf_epos_smarter_stand_angle);
    }
    else if (strcmp(paramName, "usf_epos_zero_angle_thres") == 0)
    {
      ret = sscanf(temp,
                   "%lf",
                   &paramsStruct->usf_epos_zero_angle_thres);
      LOGD("%s: usf_epos_zero_angle_thres is: %lf",
           __FUNCTION__,
           paramsStruct->usf_epos_zero_angle_thres);
    }
    else if (strcmp(paramName, "usf_epos_rotation_axis_origin") == 0)
    {
      strlcpy(paramsStruct->usf_epos_rotation_axis_origin,
              temp,
              FILE_PATH_MAX_LEN);
      LOGD("%s: usf_epos_rotation_axis_origin is: %s",
           __FUNCTION__,
           paramsStruct->usf_epos_rotation_axis_origin);
    }
    else if (strcmp(paramName, "usf_epos_rotation_axis_direction") == 0)
    {
      strlcpy(paramsStruct->usf_epos_rotation_axis_direction,
              temp,
              FILE_PATH_MAX_LEN);
      LOGD("%s: usf_epos_rotation_axis_direction is: %s",
           __FUNCTION__,
           paramsStruct->usf_epos_rotation_axis_direction);
    }
    else if (strcmp(paramName, "usf_epos_durations_file") == 0)
    {
      strlcpy(paramsStruct->usf_epos_durations_file,
              temp,
              FILE_PATH_MAX_LEN);
      LOGD("%s: usf_epos_durations_file is: %s",
           __FUNCTION__,
           paramsStruct->usf_epos_durations_file);
    }
    else if (strcmp(paramName, "usf_epos_on_screen_hover_icon_mode") == 0)
    {
        ret = sscanf(temp,
                     "%d",
                     (int*)&paramsStruct->usf_epos_on_screen_hover_icon_mode);
        LOGD("%s: usf_epos_on_screen_hover_icon_mode is: %d",
             __FUNCTION__,
             paramsStruct->usf_epos_on_screen_hover_icon_mode);
    }
    else if (strcmp(paramName, "usf_epos_off_screen_hover_icon_mode") == 0)
    {
        ret = sscanf(temp,
                     "%d",
                     (int*)&paramsStruct->usf_epos_off_screen_hover_icon_mode);
        LOGD("%s: usf_epos_off_screen_hover_icon_mode is: %d",
             __FUNCTION__,
             paramsStruct->usf_epos_off_screen_hover_icon_mode);
    }
    else if (strcmp(paramName, "usf_epos_touch_disable_threshold") == 0)
    {
        ret = sscanf(temp,
                     "%d",
                     &paramsStruct->usf_epos_touch_disable_threshold);
        LOGD("%s: usf_epos_touch_disable_threshold is: %d",
             __FUNCTION__,
             paramsStruct->usf_epos_touch_disable_threshold);
    }
    else if (strcmp(paramName, "usf_epos_battery_low_level_threshold") == 0)
    {
        ret = sscanf(temp,
                     "%d",
                     &paramsStruct->usf_epos_battery_low_level_threshold);
        LOGD("%s: usf_epos_battery_low_level_threshold is: %d",
             __FUNCTION__,
             paramsStruct->usf_epos_battery_low_level_threshold);
    }

    // P2P params
    else if (strcmp(paramName, "usf_p2p_device_uid") == 0)
    {
      ret = sscanf(temp,
                   "%d",
                   &paramsStruct->usf_p2p_device_uid);
      LOGD("%s: usf_p2p_device_uid is: %d",
           __FUNCTION__,
           paramsStruct->usf_p2p_device_uid);
    }
    else if (strcmp(paramName, "usf_p2p_event_dest") == 0)
    {
      ret = sscanf(temp,
                   "%d",
                   &paramsStruct->usf_p2p_event_dest);
      LOGD("%s: usf_p2p_event_dest is: %d",
           __FUNCTION__,
           paramsStruct->usf_p2p_event_dest);
    }
    else if (strcmp(paramName, "usf_p2p_pattern_type") == 0)
    {
      ret = sscanf(temp,
                   "%d",
                   &paramsStruct->usf_p2p_pattern_type);
      LOGD("%s: usf_p2p_pattern_type is: %d",
           __FUNCTION__,
           paramsStruct->usf_p2p_pattern_type);
    }
    else if (strcmp(paramName, "usf_p2p_event_out_port") == 0)
    {
      ret = sscanf(temp,
                   "%d",
                   &paramsStruct->usf_p2p_event_out_port);
      LOGD("%s: usf_p2p_event_out_port is: %d",
           __FUNCTION__,
           paramsStruct->usf_p2p_event_out_port);
    }
    else if (strcmp(paramName, "usf_p2p_app_type") == 0)
    {
        ret = sscanf(temp,
                     "%hu",
                     &paramsStruct->usf_p2p_app_type);
        LOGD("%s: usf_p2p_app_type is: %hu",
             __FUNCTION__,
             paramsStruct->usf_p2p_app_type);
    }
    else if (strcmp(paramName, "usf_p2p_pos_alg_type") == 0)
    {
        ret = sscanf(temp,
                     "%hu",
                     &paramsStruct->usf_p2p_pos_alg_type);
        LOGD("%s: usf_p2p_pos_alg_type is: %hu",
             __FUNCTION__,
             paramsStruct->usf_p2p_pos_alg_type);
    }
    else if (strcmp(paramName, "usf_p2p_data_alg_type") == 0)
    {
        ret = sscanf(temp,
                     "%hu",
                     &paramsStruct->usf_p2p_data_alg_type);
        LOGD("%s: usf_p2p_data_alg_type is: %hu",
             __FUNCTION__,
             paramsStruct->usf_p2p_data_alg_type);
    }
    else if (strcmp(paramName, "usf_p2p_num_users") == 0)
    {
        ret = sscanf(temp,
                     "%hu",
                     &paramsStruct->usf_p2p_num_users);
        LOGD("%s: usf_p2p_num_users is: %hu",
             __FUNCTION__,
             paramsStruct->usf_p2p_num_users);
    }
    else if (strcmp(paramName, "usf_p2p_user_index") == 0)
    {
        ret = sscanf(temp,
                     "%hu",
                     &paramsStruct->usf_p2p_user_index);
        LOGD("%s: usf_p2p_user_index is: %hu",
             __FUNCTION__,
             paramsStruct->usf_p2p_user_index);
    }
    else if (strcmp(paramName, "usf_p2p_samples_per_frame") == 0)
    {
        ret = sscanf(temp,
                     "%u",
                     &paramsStruct->usf_p2p_samples_per_frame);
        LOGD("%s: usf_p2p_samples_per_frame is: %u",
             __FUNCTION__,
             paramsStruct->usf_p2p_samples_per_frame);
    }
    else if (strcmp(paramName, "usf_p2p_sequence_index") == 0)
    {
        uint32_t num_elements = 0;
        ret = ual_util_cfg_parse_uint16_array(temp,
                     &paramsStruct->usf_p2p_sequence_index[0],
                     P2P_MAX_USERS,
                     &num_elements);
        if (ret == 0)
        {
            if (num_elements != paramsStruct->usf_p2p_num_users)
            {
                LOGE("%s: usf_p2p_sequence_index must have usf_p2p_num_users elements. Got %u elements",
                     __FUNCTION__, num_elements);
                ret = -1;
            }
        }
        LOGD("%s: usf_p2p_sequence_index has %u entries",
             __FUNCTION__,
             num_elements);
        for (i=0; i<(int)num_elements; i++)
        {
            LOGD("%s: usf_p2p_sequence_index[%d]=%hu",
                 __FUNCTION__, i, paramsStruct->usf_p2p_sequence_index[i]);
        }
    }
    else if (strcmp(paramName, "usf_p2p_first_bin") == 0)
    {
        ret = sscanf(temp,
                     "%hu",
                     &paramsStruct->usf_p2p_first_bin);
        LOGD("%s: usf_p2p_first_bin is: %hu",
             __FUNCTION__,
             paramsStruct->usf_p2p_first_bin);
    }
    else if (strcmp(paramName, "usf_p2p_last_bin") == 0)
    {
        ret = sscanf(temp,
                     "%hu",
                     &paramsStruct->usf_p2p_last_bin);
        LOGD("%s: usf_p2p_last_bin is: %hu",
             __FUNCTION__,
             paramsStruct->usf_p2p_last_bin);
    }
    else if (strcmp(paramName, "usf_p2p_resampler_freq_ppm") == 0)
    {
        ret = sscanf(temp,
                     "%u",
                     &paramsStruct->usf_p2p_resampler_freq_ppm);
        LOGD("%s: usf_p2p_resampler_freq_ppm is: %u",
             __FUNCTION__,
             paramsStruct->usf_p2p_resampler_freq_ppm);
    }
    else if (strcmp(paramName, "usf_p2p_fft_size") == 0)
    {
        ret = sscanf(temp,
                     "%hu",
                     &paramsStruct->usf_p2p_fft_size);
        LOGD("%s: usf_p2p_fft_size is: %hu",
             __FUNCTION__,
             paramsStruct->usf_p2p_fft_size);
    }
    else if (strcmp(paramName, "usf_p2p_rx_pattern_mode") == 0)
    {
        ret = sscanf(temp,
                     "%hu",
                     &paramsStruct->usf_p2p_rx_pattern_mode);
        LOGD("%s: usf_p2p_rx_pattern_mode is: %hu",
             __FUNCTION__,
             paramsStruct->usf_p2p_rx_pattern_mode);
    }
    else if (strcmp(paramName, "usf_p2p_p2a_threshold") == 0)
    {
        ret = sscanf(temp,
                     "%hu",
                     &paramsStruct->usf_p2p_p2a_threshold);
        LOGD("%s: usf_p2p_p2a_threshold is: %hu",
             __FUNCTION__,
             paramsStruct->usf_p2p_p2a_threshold);
    }
    else if (strcmp(paramName, "usf_p2p_los_window_len_ratio") == 0)
    {
        ret = sscanf(temp,
                     "%hu",
                     &paramsStruct->usf_p2p_los_window_len_ratio);
        LOGD("%s: usf_p2p_los_window_len_ratio is: %hu",
             __FUNCTION__,
             paramsStruct->usf_p2p_los_window_len_ratio);
    }
    else if (strcmp(paramName, "usf_p2p_los_peak_threashold_ratio") == 0)
    {
        ret = sscanf(temp,
                     "%hu",
                     &paramsStruct->usf_p2p_los_peak_threashold_ratio);
        LOGD("%s: usf_p2p_los_peak_threashold_ratio is: %hu",
             __FUNCTION__,
             paramsStruct->usf_p2p_los_peak_threashold_ratio);
    }
    else if (strcmp(paramName, "usf_p2p_library_mode") == 0)
    {
        ret = sscanf(temp,
                     "%hu",
                     &paramsStruct->usf_p2p_library_mode);
        LOGD("%s: usf_p2p_library_mode is: %hu",
             __FUNCTION__,
             paramsStruct->usf_p2p_library_mode);
    }

    // Hovering params
    else if (strcmp(paramName, "usf_hovering_event_dest") == 0)
    {
      ret = sscanf(temp,
                   "%d",
                   &paramsStruct->usf_hovering_event_dest);
      LOGD("%s: usf_hovering_event_dest is: %d",
           __FUNCTION__,
           paramsStruct->usf_hovering_event_dest);
    }
    else if (strcmp(paramName, "usf_hovering_event_out_port") == 0)
    {
      ret = sscanf(temp,
                   "%d",
                   &paramsStruct->usf_hovering_event_out_port);
      LOGD("%s: usf_hovering_event_out_port is: %d",
           __FUNCTION__,
           paramsStruct->usf_hovering_event_out_port);
    }

    // Gesture params
    else if (strcmp(paramName, "usf_gesture_event_dest") == 0)
    {
      ret = sscanf(temp,
                   "%d",
                   &paramsStruct->usf_gesture_event_dest);
      LOGD("%s: usf_gesture_event_dest is: %d",
           __FUNCTION__,
           paramsStruct->usf_gesture_event_dest);
    }
    else if (strcmp(paramName, "usf_gesture_event_out_port") == 0)
    {
      ret = sscanf(temp,
                   "%d",
                   &paramsStruct->usf_gesture_event_out_port);
      LOGD("%s: usf_gesture_event_out_port is: %d",
           __FUNCTION__,
           paramsStruct->usf_gesture_event_out_port);
    }
    else if (strcmp(paramName, "usf_gesture_keys") == 0)
    {
      strlcpy(paramsStruct->usf_gesture_keys,
              temp,
              FILE_PATH_MAX_LEN);
      LOGD("%s: usf_gesture_keys is: %s",
           __FUNCTION__,
           paramsStruct->usf_gesture_keys);
    }
    else if (strcmp(paramName, "usf_adapter_lib") == 0)
    {
      strlcpy(paramsStruct->usf_adapter_lib,
              temp,
              FILE_PATH_MAX_LEN);
      LOGD("%s: usf_adapter_lib is: %s",
           __FUNCTION__,
           paramsStruct->usf_adapter_lib);
    }
    else if (strcmp(paramName, "usf_gesture_app_lib_bypass") == 0)
    {
      ret = sscanf(temp,
                   "%d",
                   &paramsStruct->usf_gesture_app_lib_bypass);
      LOGD("%s: usf_gesture_app_lib_bypass is: %d",
           __FUNCTION__,
           paramsStruct->usf_gesture_app_lib_bypass);
    }
    else if (strcmp(paramName,"usf_algo_transparent_data_file") == 0)
    {
      ual_util_get_transparent_data(temp,
                                    paramsStruct->usf_algo_transparent_data,
                                    &paramsStruct->usf_algo_transparent_data_size,
                                    "algo");
    }

    // DSP calculated events params
    else if (strcmp(paramName, "usf_output_type") == 0)
    {
      ret = sscanf(temp,
                   "%d",
                   &paramsStruct->usf_output_type);
      LOGD("%s: usf_output_type is: %d",
           __FUNCTION__,
           paramsStruct->usf_output_type);
    }

    // SW Calib params
    else if (strcmp(paramName, "usf_sw_calib_timeout_msec") == 0)
    {
      ret = sscanf(temp,
                   "%d",
                   &paramsStruct->usf_sw_calib_timeout_msec);
      LOGD("%s: usf_sw_calib_timeout_msec is: %d",
           __FUNCTION__,
           paramsStruct->usf_sw_calib_timeout_msec);
    }
    else if (strcmp(paramName, "usf_sw_calib_product_packet") == 0)
    {
      strlcpy(paramsStruct->usf_sw_calib_calibration_file[SW_CALIB_PRODUCT_FILE].path,
              temp,
              FILE_PATH_MAX_LEN);
      LOGD("%s: usf_sw_calib_product_packet is: %s",
           __FUNCTION__,
           paramsStruct->usf_sw_calib_calibration_file[SW_CALIB_PRODUCT_FILE].path);
      paramsStruct->usf_sw_calib_calibration_file[SW_CALIB_PRODUCT_FILE].mandatory = true;
    }
    else if (strcmp(paramName, "usf_sw_calib_unit_packet") == 0)
    {
      strlcpy(paramsStruct->usf_sw_calib_calibration_file[SW_CALIB_UNIT_FILE].path,
              temp,
              FILE_PATH_MAX_LEN);
      LOGD("%s: usf_sw_calib_unit_packet is: %s",
           __FUNCTION__,
           paramsStruct->usf_sw_calib_calibration_file[SW_CALIB_UNIT_FILE].path);
      paramsStruct->usf_sw_calib_calibration_file[SW_CALIB_UNIT_FILE].mandatory = true;
    }
    else if (strcmp(paramName, "usf_sw_calib_persistent_packet") == 0)
    {
      strlcpy(paramsStruct->usf_sw_calib_calibration_file[SW_CALIB_PERSISTENT_FILE].path,
              temp,
              FILE_PATH_MAX_LEN);
      LOGD("%s: usf_sw_calib_persistent_file is: %s",
           __FUNCTION__,
           paramsStruct->usf_sw_calib_calibration_file[SW_CALIB_PERSISTENT_FILE].path);
      paramsStruct->usf_sw_calib_calibration_file[SW_CALIB_PERSISTENT_FILE].mandatory = false;
    }
    else if (strcmp(paramName, "usf_sw_calib_series_packet") == 0)
    {
      strlcpy(paramsStruct->usf_sw_calib_calibration_file[SW_CALIB_PEN_SERIES_FILE].path,
              temp,
              FILE_PATH_MAX_LEN);
      LOGD("%s: usf_sw_calib_series_packet is: %s",
           __FUNCTION__,
           paramsStruct->usf_sw_calib_calibration_file[SW_CALIB_PEN_SERIES_FILE].path);
      paramsStruct->usf_sw_calib_calibration_file[SW_CALIB_PEN_SERIES_FILE].mandatory = true;
    }
    else if (strcmp(paramName, "usf_sw_calib_packet") == 0)
    {
      strlcpy(paramsStruct->usf_sw_calib_calibration_file[SW_CALIB_CALIB_FILE].path,
              temp,
              FILE_PATH_MAX_LEN);
      LOGD("%s: usf_sw_calib_packet is: %s",
           __FUNCTION__,
           paramsStruct->usf_sw_calib_calibration_file[SW_CALIB_CALIB_FILE].path);
      paramsStruct->usf_sw_calib_calibration_file[SW_CALIB_CALIB_FILE].mandatory = true;
    }
    else if (strcmp(paramName,"usf_tester_power_min_scales") == 0)
    {
      strlcpy(paramsStruct->usf_tester_power_min_scales,
              temp,
              FILE_PATH_MAX_LEN);
      LOGD("%s: usf_tester_power_min_scales is: %s",
           __FUNCTION__,
           paramsStruct->usf_tester_power_min_scales);
      paramsStruct->usf_sw_calib_is_tester_mode = true;
    }
    else if (strcmp(paramName,"usf_tester_power_max_scales") == 0)
    {
      strlcpy(paramsStruct->usf_tester_power_max_scales,
              temp,
              FILE_PATH_MAX_LEN);
      LOGD("%s: usf_tester_power_max_scales is: %s",
           __FUNCTION__,
           paramsStruct->usf_tester_power_max_scales);
    }
    else if (strcmp(paramName,"usf_tester_power_thresholds") == 0)
    {
      strlcpy(paramsStruct->usf_tester_power_thresholds,
              temp,
              FILE_PATH_MAX_LEN);
      LOGD("%s: usf_tester_power_thresholds is: %s",
           __FUNCTION__,
           paramsStruct->usf_tester_power_thresholds);
    }
    else if (strcmp(paramName,"usf_tester_quality_min_scales") == 0)
    {
      strlcpy(paramsStruct->usf_tester_quality_min_scales,
              temp,
              FILE_PATH_MAX_LEN);
      LOGD("%s: usf_tester_quality_min_scales is: %s",
           __FUNCTION__,
           paramsStruct->usf_tester_quality_min_scales);
    }
    else if (strcmp(paramName,"usf_tester_quality_max_scales") == 0)
    {
      strlcpy(paramsStruct->usf_tester_quality_max_scales,
              temp,
              FILE_PATH_MAX_LEN);
      LOGD("%s: usf_tester_quality_max_scales is: %s",
           __FUNCTION__,
           paramsStruct->usf_tester_quality_max_scales);
    }
    else if (strcmp(paramName,"usf_tester_quality_thresholds") == 0)
    {
      strlcpy(paramsStruct->usf_tester_quality_thresholds,
              temp,
              FILE_PATH_MAX_LEN);
      LOGD("%s: usf_tester_quality_thresholds is: %s",
           __FUNCTION__,
           paramsStruct->usf_tester_quality_thresholds);
    }
    // Pairing params
    else if (strcmp(paramName, "usf_pairing_circle_x") == 0)
    {
      ret = sscanf(temp,
                   "%d",
                   &paramsStruct->usf_pairing_circle_x);
      LOGD("%s: usf_pairing_circle_x is: %d",
           __FUNCTION__,
           paramsStruct->usf_pairing_circle_x);
    }
    else if (strcmp(paramName, "usf_pairing_circle_y") == 0)
    {
      ret = sscanf(temp,
                   "%d",
                   &paramsStruct->usf_pairing_circle_y);
      LOGD("%s: usf_pairing_circle_y is: %d",
           __FUNCTION__,
           paramsStruct->usf_pairing_circle_y);
    }
    else if (strcmp(paramName, "usf_pairing_circle_r") == 0)
    {
      ret = sscanf(temp,
                   "%d",
                   &paramsStruct->usf_pairing_circle_r);
      LOGD("%s: usf_pairing_circle_r is: %d",
           __FUNCTION__,
           paramsStruct->usf_pairing_circle_r);
    }
    else if (strcmp(paramName, "usf_socket_path") == 0)
    {
      strlcpy(paramsStruct->usf_socket_path,
              temp,
              FILE_PATH_MAX_LEN);
      LOGD("%s: usf_socket_path is: %s",
           __FUNCTION__,
           paramsStruct->usf_socket_path);
    }
    else if (strcmp(paramName, "usf_pairing_max_aquisition_time") == 0)
    {
      ret = sscanf(temp,
                   "%d",
                   &paramsStruct->usf_pairing_max_aquisition_time);
      LOGD("%s: usf_pairing_max_aquisition_time is: %d",
           __FUNCTION__,
           paramsStruct->usf_pairing_max_aquisition_time);
    }
    else if (strcmp(paramName, "usf_pairing_product_packet") == 0)
    {
      strlcpy(paramsStruct->usf_pairing_default_calib_files[PAIRING_PRODUCT_FILE],
              temp,
              FILE_PATH_MAX_LEN);
      LOGD("%s: usf_pairing_product_packet is: %s",
           __FUNCTION__,
           paramsStruct->usf_pairing_default_calib_files[PAIRING_PRODUCT_FILE]);
    }
    else if (strcmp(paramName, "usf_pairing_unit_packet") == 0)
    {
      strlcpy(paramsStruct->usf_pairing_default_calib_files[PAIRING_UNIT_FILE],
              temp,
              FILE_PATH_MAX_LEN);
      LOGD("%s: usf_pairing_unit_packet is: %s",
           __FUNCTION__,
           paramsStruct->usf_pairing_default_calib_files[PAIRING_UNIT_FILE]);
    }
    else if (strcmp(paramName, "usf_pairing_calib_files_path_prefix") == 0)
    {
      strlcpy(paramsStruct->usf_pairing_calib_files_path_prefix,
              temp,
              FILE_PATH_MAX_LEN);
      LOGD("%s: usf_pairing_calib_files_path_prefix is: %s",
           __FUNCTION__,
           paramsStruct->usf_pairing_calib_files_path_prefix);
    }
    // Form file params
    else if (strcmp(paramName, "num_of_mics") == 0)
    {
      ret = sscanf(temp,
                   "%d",
                   &formStruct.num_of_mics);
      LOGD("%s: num_of_mics is: %d",
           __FUNCTION__,
           formStruct.num_of_mics);
      if (US_FORM_FACTOR_CONFIG_MAX_MICS < formStruct.num_of_mics)
      {
        LOGW("%s: num_of_mics(%d) is replaced by supported number(%d)",
             __FUNCTION__,
             formStruct.num_of_mics,
             US_FORM_FACTOR_CONFIG_MAX_MICS);
        formStruct.num_of_mics = US_FORM_FACTOR_CONFIG_MAX_MICS;
      }
    }
    else if (strcmp(paramName, "num_of_speakers") == 0)
    {
      ret = sscanf(temp,
                   "%d",
                   &formStruct.num_of_speakers);
      LOGD("%s: num_of_speakers is: %d",
           __FUNCTION__,
           formStruct.num_of_speakers);
      if (US_FORM_FACTOR_CONFIG_MAX_SPEAKERS < formStruct.num_of_speakers)
      {
        LOGW("%s: num_of_speakers(%d) is replaced by supported number(%d)",
             __FUNCTION__,
             formStruct.num_of_speakers,
             US_FORM_FACTOR_CONFIG_MAX_SPEAKERS);
        formStruct.num_of_speakers = US_FORM_FACTOR_CONFIG_MAX_SPEAKERS;
      }
    }
    else if (strcmp(paramName, "mic_info1") == 0)
    {
      ret = ual_util_fill_mic_info(0,
                                   temp);
    }
    else if (strcmp(paramName, "mic_info2") == 0)
    {
      ret = ual_util_fill_mic_info(1,
                                   temp);
    }
    else if (strcmp(paramName, "mic_info3") == 0)
    {
      ret = ual_util_fill_mic_info(2,
                                   temp);
    }
    else if (strcmp(paramName, "mic_info4") == 0)
    {
      ret = ual_util_fill_mic_info(3,
                                   temp);
    }
    else if (strcmp(paramName, "mic_info5") == 0)
    {
      ret = ual_util_fill_mic_info(4,
                                   temp);
    }
    else if (strcmp(paramName, "mic_info6") == 0)
    {
      ret = ual_util_fill_mic_info(5,
                                   temp);
    }
    else if (strcmp(paramName, "mic_info7") == 0)
    {
      ret = ual_util_fill_mic_info(6,
                                   temp);
    }
    else if (strcmp(paramName, "mic_info8") == 0)
    {
      ret = ual_util_fill_mic_info(7,
                                   temp);
    }
    else if (strcmp(paramName, "mic_info9") == 0)
    {
      ret = ual_util_fill_mic_info(8,
                                   temp);
    }
    else if (strcmp(paramName, "mic_info10") == 0)
    {
      ret = ual_util_fill_mic_info(9,
                                   temp);
    }
    else if (strcmp(paramName, "speaker_info1") == 0)
    {
      ret = ual_util_fill_speaker_info (0,
                                        temp);
    }
    else if (strcmp(paramName, "speaker_info2") == 0)
    {
      ret = ual_util_fill_speaker_info (1,
                                        temp);
    }
    else if (strcmp(paramName, "speaker_info3") == 0)
    {
      ret = ual_util_fill_speaker_info (2,
                                        temp);
    }
    else if (strcmp(paramName, "speaker_info4") == 0)
    {
      ret = ual_util_fill_speaker_info (3,
                                        temp);
    }

    // power save params
    else if (strcmp(paramName, "ps_act_state_params") == 0)
    {
      strlcpy(paramsStruct->ps_act_state_params,
              temp,
              PS_INFO_SIZE);
      LOGD("%s: ps_act_state_params is: [%s]",
           __FUNCTION__,
           paramsStruct->ps_act_state_params);
    }
    else if (strcmp(paramName, "ps_standby_state_params") == 0)
    {
      strlcpy(paramsStruct->ps_standby_state_params,
              temp,
              PS_INFO_SIZE);
      LOGD("%s: ps_standby_state_params is: [%s]",
           __FUNCTION__,
           paramsStruct->ps_standby_state_params);
    }
    else if (strcmp(paramName, "ps_idle_state_params") == 0)
    {
      strlcpy(paramsStruct->ps_idle_state_params,
              temp,
              PS_INFO_SIZE);
      LOGD("%s: ps_idle_state_params is: [%s]",
           __FUNCTION__,
           paramsStruct->ps_idle_state_params);
    }
    else if (strcmp(paramName, "ps_standby_detect_info") == 0)
    {
      strlcpy(paramsStruct->ps_standby_detect_info,
              temp,
              PS_INFO_SIZE);
      LOGD("%s: ps_standby_detect_info is: [%s]",
           __FUNCTION__,
           paramsStruct->ps_standby_detect_info);
    }
    else if (strcmp(paramName, "ps_idle_detect_info") == 0)
    {
      strlcpy(paramsStruct->ps_idle_detect_info,
              temp,
              PS_INFO_SIZE);
      LOGD("%s: ps_idle_detect_info is: [%s]",
           __FUNCTION__,
           paramsStruct->ps_idle_detect_info);
    }
    else if (strcmp(paramName, "ps_standby_detect_calibration") == 0)
    {
      strlcpy(paramsStruct->ps_standby_detect_calibration,
              temp,
              FILE_PATH_MAX_LEN);
      LOGD("%s: ps_standby_detect_calibration is: [%s]",
           __FUNCTION__,
           paramsStruct->ps_standby_detect_calibration);
    }
    else if (strcmp(paramName, "ps_idle_detect_calibration") == 0)
    {
      strlcpy(paramsStruct->ps_idle_detect_calibration,
              temp,
              FILE_PATH_MAX_LEN);
      LOGD("%s: ps_idle_state_params is: [%s]",
           __FUNCTION__,
           paramsStruct->ps_idle_detect_calibration);
    }
    else if (strcmp(paramName, "ps_idle_detect_port") == 0)
    {
      ret = sscanf(temp,
                   "%d",
                   &paramsStruct->ps_idle_detect_port);
      LOGD("%s: ps_idle_detect_port is: %d",
           __FUNCTION__,
           paramsStruct->ps_idle_detect_port);
    }
    else if (strcmp(paramName, "ps_idle_detect_period") == 0)
    {
      ret = sscanf(temp,
                   "%d",
                   &paramsStruct->ps_idle_detect_period);
      LOGD("%s: ps_idle_detect_period is: %d",
           __FUNCTION__,
           paramsStruct->ps_idle_detect_period);
    }
    else if (strcmp(paramName, "usf_event_type") == 0)
    {
      ret = sscanf(temp,
                   "%u",
                   &paramsStruct->usf_event_type);
      LOGD("%s: usf_event_type is: %d",
           __FUNCTION__,
           paramsStruct->usf_event_type);
    }
    else if (strcmp(paramName, "ual_work_mode") == 0)
    {
      ret = sscanf(temp,
                   "%d",
                   &paramsStruct->ual_work_mode);
      LOGD("%s: ual_work_mode is: %d",
           __FUNCTION__,
           paramsStruct->ual_work_mode);
    }
    else if (strcmp(paramName, "usf_append_timestamp") == 0)
    {
      ret = sscanf(temp,
                   "%d",
                   &paramsStruct->usf_append_timestamp);
      LOGD("%s: usf_append_timestamp is: %d",
           __FUNCTION__,
           paramsStruct->usf_append_timestamp);
    }
    else
    {
      LOGW("%s: Unidentified parameter: %s",
           __FUNCTION__,
           paramName);
      numOfFieldsInCfgFile--;
    }

    numOfFieldsInCfgFile++;

    if (1 != ret)
    {
      LOGW("%s: sscanf return %d, Should return 1.",
           __FUNCTION__,
           ret);
      numOfFieldsInCfgFile = -1;
    }

  } // End while

  free(bufferTemp);
  LOGI("%s: End parsing",
       __FUNCTION__);
  return numOfFieldsInCfgFile;
}

/*==============================================================================
  FUNCTION:  ual_util_read_pattern
==============================================================================*/
/**
  See function description at header file under the function declaration.
*/
int ual_util_read_pattern (uint8_t *pBuf,
                           us_all_info *paramsStruct,
                           char* patternDirPath)
{
  int rc = -1, patternNameLen;
  FILE *fpPattern = NULL;
  const char *cBinExt = ".bin";
  uint16_t extSize = strlen(cBinExt);
  uint16_t nameSize;
  char patternFileName[FILE_PATH_MAX_LEN] = {0};

  // Make sure paramsStruct is not null and only if
  // paramsStruct->usf_rx_pattern_size > 0 make sure pBuf
  // and patternDirPath are not null
  if (NULL == paramsStruct ||
       (paramsStruct->usf_rx_pattern_size &&
         (NULL == pBuf ||
          NULL == patternDirPath) ) )
  {
    LOGW("%s: Null parameter.",
         __FUNCTION__);
    return -1;
  }

  if (0 == paramsStruct->usf_rx_pattern_size)
  {
    return 0;
  }

  // Checks for empty file name
  if ( (NULL == paramsStruct->usf_rx_pattern) ||
       (0 == *paramsStruct->usf_rx_pattern) )
  {
    LOGW("%s: Emtpty pattern file name",
         __FUNCTION__);
    return -1;
  }

  patternNameLen = strlen(paramsStruct->usf_rx_pattern);

  if (strlen(patternDirPath) + patternNameLen + 1 >
      FILE_PATH_MAX_LEN)
  {
    LOGW("%s: patternFileName is too long - %d instead of %d",
         __FUNCTION__,
         strlen(patternDirPath) + patternNameLen + 1,
         FILE_PATH_MAX_LEN);
    return -1;
  }

  strlcpy(patternFileName,
          patternDirPath,
          FILE_PATH_MAX_LEN);
  strlcat(patternFileName,
          paramsStruct->usf_rx_pattern,
          FILE_PATH_MAX_LEN);

  LOGD("%s: patternFileName is: %s",
       __FUNCTION__,
       patternFileName);

  nameSize = strlen(patternFileName);
  if ( (nameSize > extSize ) &&
       (!strcmp(cBinExt, (patternFileName + nameSize - extSize)) ) )
  { // BIN file
    LOGD("%s: read_pattern: BIN",
         __FUNCTION__);

    fpPattern = fopen (patternFileName,
                       "rb");
    if (NULL == fpPattern)
    {
      LOGW("%s: Could not open pattern bin file %s.",
           __FUNCTION__,
           patternFileName);
      return -1;
    }

    rc = 0;
    if (fread (pBuf,
               paramsStruct->usf_rx_pattern_size *
               (paramsStruct->usf_rx_sample_width / 8),
               1,
               fpPattern) != 1)
    {
      LOGW("%s: Could not read %d bytes from %s",
           __FUNCTION__,
           paramsStruct->usf_rx_pattern_size *
           (paramsStruct->usf_rx_sample_width / 8),
           patternFileName);
      rc = -1;
    }
  } // BIN file
  else
  { // ASCII file
    LOGD("%s: read_pattern: ASCII",
         __FUNCTION__);

    fpPattern = fopen(patternFileName,
                      "rt");
    if (NULL == fpPattern)
    {
      LOGW("%s: Could not open pattern ascii file %s.",
           __FUNCTION__,
           patternFileName);
      return -1;
    }

    short *pData = (short *)pBuf;
    rc = 0;
    for (uint32_t ind = 0; ind < paramsStruct->usf_rx_pattern_size; ++ind)
    {
      int cnt = fscanf(fpPattern,
                       "%hd",
                       (pData + ind));
      if (cnt != 1)
      {
        LOGW("%s: Wrong read(%d); ind(%d) from file %s.",
             __FUNCTION__,
             cnt,
             ind,
             patternFileName);
        rc = -1;
        break;
      }
    } // End for
  } // ASCII file

  fclose (fpPattern);
  fpPattern = NULL;

  LOGD("%s: read_pattern result(%d); (%d) byte from (%s)",
       __FUNCTION__,
       rc,
       paramsStruct->usf_rx_pattern_size *
       (paramsStruct->usf_rx_sample_width / 8),
       patternFileName);

  short *pData = (short *)pBuf;
  LOGD("%s: read_pattern data[0](%d); data[%d](%d)",
       __FUNCTION__,
       *pData, paramsStruct->usf_rx_pattern_size - 1,
       *(pData+paramsStruct->usf_rx_pattern_size - 1));

  return rc;
}

/*==============================================================================
  FUNCTION:  ual_util_parse_string2array
==============================================================================*/
/**
  See function description at header file under the function declaration.
*/
bool ual_util_parse_string2array(uint16_t data_size,
                                 char* data_string,
                                 uint8_t* data)
{
  char *buf = NULL, *save_pointer;
  uint16_t j = 0;
  bool rc = true;
  char *temp_str = data_string;

  if ((NULL == data_string) ||
      (NULL == data))
  {
    LOGE("%s: wrong data_string=0x%p; data=0x%p",
         __FUNCTION__,
         data_string,
         data);
    return false;
  }

  for (j = 0; j < data_size; ++j)
  {
    int temp = 0;
    buf = strtok_r(temp_str,
                   ",",
                   &save_pointer);
    if (NULL == buf) {
      LOGE("%s: wrong data format; data_string=[%s]",
           __FUNCTION__,
           data_string);
      break;
    }
    int ret = sscanf(buf,
                 "%u%*[^ ]", // Trim spaces
                 &temp);
    if (1 != ret)
    {
      LOGE("%s: sscanf return %d, Should return 1.",
           __FUNCTION__,
           ret);
      break;
    }
    data[j] = temp;

    temp_str = NULL; // for the next strtok_r()
  }

  if (j < data_size)
  {
    rc = false;
    LOGE("%s: data_string=[%s]; j=%d; data_size=%d",
         __FUNCTION__,
         data_string,
         j,
         data_size);
  }

  return rc;
}

/*==============================================================================
  FUNCTION:  ual_util_tx_config
==============================================================================*/
/**
  See function description at header file under the function declaration.
*/
int ual_util_tx_config (us_all_info* paramsStruct,
                        char* clientName)
{
  int ret;
  uint32_t temp_port0, temp_port1, temp_port2, temp_port3;
  uint8_t transparentTxData[TRANSPARENT_DATA_MAX_SIZE] = {0};
  bool rc = false;

  if (NULL == paramsStruct ||
      NULL == clientName)
  {
    LOGW("%s: Null parameter.",
         __FUNCTION__);
    return -1;
  }

  if (!paramsStruct->use_tx)
  {
    LOGW("%s: paramsStruct.use_tx is false.",
         __FUNCTION__);
    return 1;
  }

  us_tx_info_type tx_info;

  memset(&tx_info, 0, sizeof(us_tx_info_type));

  tx_info.us_xx_info.client_name = clientName;
  tx_info.us_xx_info.dev_id = paramsStruct->usf_device_id;
  tx_info.us_xx_info.stream_format = paramsStruct->usf_tx_data_format;
  tx_info.us_xx_info.sample_rate = paramsStruct->usf_tx_sample_rate;
  tx_info.us_xx_info.buf_num = paramsStruct->usf_tx_queue_capacity;
  tx_info.us_xx_info.port_cnt = paramsStruct->usf_tx_port_count;
  tx_info.us_xx_info.max_get_set_param_buf_size =
    paramsStruct->usf_tx_max_get_set_param_buf_size;

  char *str = paramsStruct->usf_tx_ports;
  char *endptr = NULL;
  int port_ind = 0;

  for (port_ind = 0; port_ind < tx_info.us_xx_info.port_cnt; ++port_ind)
  {
    int val = strtol(str, &endptr, 10);
    if ((ERANGE == val) ||
        (endptr == str))
    {
      LOGW("%s: Wrong ports_string [%s]",
           __FUNCTION__,
           str);
      return -1;
    }
    tx_info.us_xx_info.port_id[port_ind] = val;
    if ('\0' == *endptr)
    {
      ++port_ind; // number of the ports
      break;
    }
    else
    {
      str = endptr+1;
    }
  }

  if (port_ind != tx_info.us_xx_info.port_cnt)
  {
    LOGW("%s: Port count does not match to number of "
         "ports given in usf_tx_ports."
         "port_cnt=%d, ret from scnf of usf_tx_ports=%d",
         __FUNCTION__,
         tx_info.us_xx_info.port_cnt,
         port_ind);
    return -1;
  }

  // port_cnt was checked during the parsing
  // port_cnt <= tx_info.us_xx_info.port_cnt <= US_FORM_FACTOR_CONFIG_MAX_MICS
  s_req_mics_num = tx_info.us_xx_info.port_cnt;
  if (s_req_mics_num > USF_MAX_PORT_NUM)
  {
    LOGE("%s:  requested ports number(%d) is bigger than the max supported(%d)",
         __FUNCTION__,
         s_req_mics_num,
         USF_MAX_PORT_NUM);
    return false;
  }

  for (int ind = 0; ind < s_req_mics_num; ++ind)
  {
    s_req_mics_id[ind] = tx_info.us_xx_info.port_id[ind];
  }

  tx_info.us_xx_info.bits_per_sample = paramsStruct->usf_tx_sample_width;

  // TX transparent data
  tx_info.us_xx_info.params_data_size =
  paramsStruct->usf_tx_transparent_data_size;

  // Ptr to static 64 bytes - fill from cfg file
  tx_info.us_xx_info.params_data =
  (uint8_t *) paramsStruct->usf_tx_transparent_data;
  tx_info.us_xx_info.buf_size = paramsStruct->usf_tx_buf_size; // Group size

  tx_info.input_info.event_types = paramsStruct->usf_event_type;

  tx_info.input_info.tsc_x_dim[MIN_IND] = MIN_DIM_SIZE;
  tx_info.input_info.tsc_x_dim[MAX_IND] = TSC_LOGICAL_MAX_X;
  tx_info.input_info.tsc_y_dim[MIN_IND] = MIN_DIM_SIZE;
  tx_info.input_info.tsc_y_dim[MAX_IND] = TSC_LOGICAL_MAX_Y;
  tx_info.input_info.tsc_z_dim[MIN_IND] = MIN_DIM_SIZE;
  tx_info.input_info.tsc_z_dim[MAX_IND] =
    fmax(paramsStruct->usf_epos_on_screen_hover_max_range,
         paramsStruct->usf_epos_off_screen_hover_max_range);
  tx_info.input_info.req_buttons_bitmap = paramsStruct->req_buttons_bitmap;

  tx_info.input_info.conflicting_event_types =
    paramsStruct->conflicting_event_types;

  memcpy(tx_info.input_info.tsc_x_tilt,
         paramsStruct->usf_x_tilt,
         sizeof(tx_info.input_info.tsc_x_tilt));
  memcpy(tx_info.input_info.tsc_y_tilt,
         paramsStruct->usf_y_tilt,
         sizeof(tx_info.input_info.tsc_y_tilt));

  memcpy(tx_info.input_info.tsc_pressure,
         paramsStruct->usf_tsc_pressure,
         sizeof(tx_info.input_info.tsc_pressure));

  rc = ual_configure_TX(&tx_info);
  if (rc != 1 &&
      UAL_MODE_IDLE_ALL_DATA_PATH != paramsStruct->ual_work_mode)
  {
    LOGW("%s: ual_configure_TX failed: rc=%d;",
         __FUNCTION__,
         rc);
    return -1;
  }

  rc = ual_start_TX();
  if (rc != 1 &&
      UAL_MODE_IDLE_ALL_DATA_PATH != paramsStruct->ual_work_mode)
  {
    LOGW("%s: ual_start_TX failed: rc=%d;",
         __FUNCTION__,
         rc);
    return -1;
  }

  return 0;
}


/*==============================================================================
  FUNCTION:  ual_util_rx_config
==============================================================================*/
/**
  See function description at header file under the function declaration.
*/
int ual_util_rx_config (us_all_info *paramsStruct,
                        char* clientName)
{
  int ret;
  uint32_t temp_port0, temp_port1, temp_port2, temp_port3;
  uint8_t transparentRxData[TRANSPARENT_DATA_MAX_SIZE] = {0};
  bool rc = false;

  if (NULL == paramsStruct ||
      NULL == clientName)
  {
    LOGW("%s: Null parameter.",
         __FUNCTION__);
    return -1;
  }

  if (!paramsStruct->use_rx)
  {
    LOGW("%s: paramsStruct.use_rx is false.",
         __FUNCTION__);
    return 1;
  }

  us_rx_info_type rx_info;
  rx_info.us_xx_info.client_name = clientName;
  rx_info.us_xx_info.dev_id = paramsStruct->usf_device_id;
  rx_info.us_xx_info.stream_format = paramsStruct->usf_rx_data_format;
  rx_info.us_xx_info.sample_rate = paramsStruct->usf_rx_sample_rate;
  rx_info.us_xx_info.buf_num = paramsStruct->usf_rx_queue_capacity;
  rx_info.us_xx_info.port_cnt = paramsStruct->usf_rx_port_count;
  rx_info.us_xx_info.max_get_set_param_buf_size =
    paramsStruct->usf_rx_max_get_set_param_buf_size;

  ret = sscanf(paramsStruct->usf_rx_ports,
               "%u ,%u ,%u ,%u",
               &temp_port0,
               &temp_port1,
               &temp_port2,
               &temp_port3);
  if (ret != rx_info.us_xx_info.port_cnt)
  {
    LOGW("%s: Port count does not match to number of "
         "ports given in usf_rx_ports."
         "rx_port_cnt=%d, ret from scnf of usf_rx_ports=%d",
         __FUNCTION__,
         rx_info.us_xx_info.port_cnt,
         ret);
    return -1;
  }

  rx_info.us_xx_info.port_id[0] = temp_port0;
  rx_info.us_xx_info.port_id[1] = temp_port1;
  rx_info.us_xx_info.port_id[2] = temp_port2;
  rx_info.us_xx_info.port_id[3] = temp_port3;

  // port_cnt was checked during the parsing
  // port_cnt <= rx_info.us_xx_info.port_cnt <= US_FORM_FACTOR_CONFIG_MAX_SPEAKERS
  s_req_speakers_num = rx_info.us_xx_info.port_cnt;
  for (int ind=0; ind<s_req_speakers_num; ++ind)
  {
    s_req_speakers_id[ind] = rx_info.us_xx_info.port_id[ind];
  }

  rx_info.us_xx_info.bits_per_sample = paramsStruct->usf_rx_sample_width;

  // RX Transparent data
  rx_info.us_xx_info.params_data_size =
  paramsStruct->usf_rx_transparent_data_size;

  // Ptr to static 64 bytes - fill from cfg file
  rx_info.us_xx_info.params_data =
  (uint8_t *) paramsStruct->usf_rx_transparent_data;
  rx_info.us_xx_info.buf_size = paramsStruct->usf_rx_buf_size; // Group size

  rc = ual_configure_RX(&rx_info);
  if (1 != rc &&
      UAL_MODE_IDLE_ALL_DATA_PATH != paramsStruct->ual_work_mode)
  {
    LOGW("%s: ual_configure_RX failed: rc=%d;",
         __FUNCTION__,
         rc);
    return -1;
  }

  rc = ual_start_RX();
  if (1 != rc &&
      UAL_MODE_IDLE_ALL_DATA_PATH != paramsStruct->ual_work_mode)
  {
    LOGW("%s: ual_start_RX failed: rc=%d;",
         __FUNCTION__,
         rc);
    return -1;
  }

  return 0;
}

/*==============================================================================
  FUNCTION:  ual_util_parse_form_factor
==============================================================================*/
/**
  Parses the form factor file and updates formStruct.
  Returns -1 for failure, 0 for success.
*/
int ual_util_parse_form_factor ()
{
  FILE *ffFile;
  us_all_info paramsStruct;

  memset(&formStruct,
         0,
         sizeof(FormStruct));

  ffFile = fopen(FORM_FILE,
                 "r");
  if (NULL == ffFile)
  {
    LOGE("%s: Open form factor file failure",
         __FUNCTION__);
    return -1;
  }

  int rc = ual_util_parse_cfg_file(ffFile,
                                   &paramsStruct);
  fclose(ffFile);
  if (-1 == rc)
  {
    LOGE("%s: ual_util_parse_cfg_file failed.",
         __FUNCTION__);
  }
  else
  {
    rc = 0;
  }

  return rc;
}

/*==============================================================================
  FUNCTION:  ual_util_daemon_init
==============================================================================*/
/**
  See function description at header file under the function declaration.
*/
int ual_util_daemon_init (us_all_info *paramsStruct,
                          char *cfgLinkFileLocation,
                          FILE *cfgFile,
                          char *daemonName)
{
  int ret;
  char cfgFileName[BUFFER_SIZE], *save_pointer;

  if (NULL == paramsStruct ||
      NULL == cfgLinkFileLocation)
  {
    LOGW("%s: Null parameter.",
         __FUNCTION__);
    return -1;
  }

  umask(0);

  // Get abilities/parameters of device
  int rc = ual_util_parse_form_factor();
  if (-1 == rc)
  {
    LOGE("%s: ual_util_parse_form_factor failed.",
         __FUNCTION__);
    return -1;
  }
  int cfg_file_name_len = readlink(cfgLinkFileLocation,
                                   cfgFileName,
                                   // readlink does not append \0
                                   sizeof(cfgFileName) - 1);
  if (0 < cfg_file_name_len)
  {
    cfgFileName[cfg_file_name_len] = '\0';
    LOGD("%s, link cfg detected: %s -> %s",
         __FUNCTION__,
         cfgLinkFileLocation,
         cfgFileName);
  }
  else
  {
    LOGE("%s, link cfg: %s file is not a symbolic link",
         __FUNCTION__,
         cfgLinkFileLocation);
    return -1;
  }

  // Open daemon configuration file
  cfgFile = fopen(cfgLinkFileLocation,
                  "r+");
  if (NULL == cfgFile)
  {
    LOGW("%s: Opening cfgFile(%s) failure",
         __FUNCTION__,
         cfgFileName);
    return -1;
  }

  // Parse config file
  ret = ual_util_parse_cfg_file(cfgFile,
                                paramsStruct);
  fclose(cfgFile);
  cfgFile = NULL;
  if (-1 == ret)
  {
    LOGW("%s: ual_util_parse_cfg_file failure",
         __FUNCTION__);
    return -1;
  }

  ret = usf_validation_cfg_file(cfgFileName,
                                daemonName,
                                paramsStruct,
                                &formStruct);
  if (ret)
  {
    LOGE("%s, Config file validation FAILED, returned %d",
         __FUNCTION__,
         ret);
    return -1;
  }

  return 0;
}

/*==============================================================================
  FUNCTION:  ual_util_get_mic_config
==============================================================================*/
/**
  See function description at header file under the function declaration.
*/
int ual_util_get_mic_config (int micIndex,
                             float *micCfg)
{
  if (NULL == micCfg)
  {
    LOGW("%s: NULL parameter.",
         __FUNCTION__);
    return -1;
  }

  if (s_req_mics_num <= micIndex ||
      0 > micIndex)
  {
    LOGW("%s: Bad mic num parameter, s_req_mics_num = %d, micIndex = %d",
         __FUNCTION__, s_req_mics_num, micIndex);
    return -1;
  }

  // Find required mic & provide its configuration
  uint8_t req_port_id = s_req_mics_id[micIndex];
  int rc = -1;

  for (int req_port_index=0; req_port_index<formStruct.num_of_mics; ++req_port_index)
  {
    if (req_port_id == formStruct.mics_id[req_port_index])
    {
      for (int i = 0; i < COORDINATES_DIM; i++)
      {
        micCfg[i] = formStruct.mics_info[req_port_index][i];
      }
      rc = 0;
      LOGW("%s: micIndex=%d; req_port_id=%d; req_port_index=%d",
           __FUNCTION__,
           micIndex,
           req_port_id,
           req_port_index);
      break;
    }
  }

  return rc;
}

/*==============================================================================
  FUNCTION:  ual_util_get_speaker_config
==============================================================================*/
/**
  See function description at header file under the function declaration.
*/
int ual_util_get_speaker_config (int speakerIndex,
                                 float *speakerCfg)
{
  if (NULL == speakerCfg)
  {
    LOGW("%s: NULL parameter.",
         __FUNCTION__);
    return -1;
  }

  if (s_req_speakers_num <= speakerIndex ||
      0 > speakerIndex)
  {
    LOGW("%s: Bad speaker num parameter.",
         __FUNCTION__);
    return -1;
  }

  // Find required speaker & provide its configuration
  uint8_t req_port_id = s_req_speakers_id[speakerIndex];
  int rc = -1;
  for (int req_port_index=0; req_port_index<formStruct.num_of_speakers; ++req_port_index)
  {
    if (req_port_id == formStruct.speakers_id[req_port_index])
    {
      for (int i = 0; i < COORDINATES_DIM; i++)
      {
        speakerCfg[i] = formStruct.speakers_info[req_port_index][i];
      }
      rc = 0;
      LOGW("%s: speakerIndex=%d; req_port_id=%d; req_port_index=%d",
           __FUNCTION__,
           speakerIndex,
           req_port_id,
           req_port_index);
      break;
    }
  }

  return rc;
}

/*==============================================================================
  FUNCTION:  ual_util_get_file
==============================================================================*/
/**
  See function description at header file under the function declaration.
*/
FILE * ual_util_get_file (us_all_info const *paramsStruct,
                          char *defaultFrameDirName,
                          bool isNotBinary)
{
  int frameFileNameLen;
  char fullFrameFileName[MAX_FILE_NAME_LEN] = {0};
  FILE* frameFile = NULL;

  if (NULL == paramsStruct)
  {
    LOGE("%s: paramsStruct is NULL",
         __FUNCTION__);
    return NULL;
  }
  char const *frameFileName = isNotBinary ? paramsStruct->usf_epos_coord_file :
                                            paramsStruct->usf_frame_file;

  // Checks for empty file name
  if ( (NULL == frameFileName) ||
       (0 == *frameFileName) )
  {
    LOGW("%s: Emtpty frame file name",
         __FUNCTION__);
    return frameFile;
  }

  frameFileNameLen = strlen(frameFileName);

  // If no '/' sign in the frame file name then the file
  // will be saved in a default dir for records.
  if (NULL == strchr(frameFileName,
                     '/'))
  {
    // Only if the frameFileName doesn't contains '/'
    // we use defaultFrameDirName.
    if (NULL == defaultFrameDirName)
    {
      LOGW("%s: NULL parameter.",
           __FUNCTION__);
      return frameFile;
    }
    if (strlen(defaultFrameDirName) + frameFileNameLen + 1 >
        MAX_FILE_NAME_LEN)
    {
      LOGW("%s: frameFileName is too long - %d instead of %d",
           __FUNCTION__,
           strlen(defaultFrameDirName) + frameFileNameLen + 1,
           MAX_FILE_NAME_LEN);
      return frameFile;
    }

    strlcpy(fullFrameFileName,
            defaultFrameDirName,
            MAX_FILE_NAME_LEN);
    strlcat(fullFrameFileName,
            frameFileName,
            MAX_FILE_NAME_LEN);
  }
  // If '/' sign appears in the frame file name then
  // the name is absolute path to the file location.
  else
  {
    strlcpy(fullFrameFileName,
            frameFileName,
            MAX_FILE_NAME_LEN);
  }

  if (1 == paramsStruct->usf_append_timestamp &&
      // Append timestamp to frame recording files only
      false == isNotBinary)
  {
    LOGD("%s, Appending timestamp",
         __FUNCTION__);
    // Get current time
    timespec realtime;
    clock_gettime(CLOCK_REALTIME,
                  &realtime);
    char now[MAX_FILE_NAME_LEN];
    snprintf(now,
             MAX_FILE_NAME_LEN,
             "_%lld",
             (1000000LL * realtime.tv_sec + realtime.tv_nsec / 1000LL));
    if (strlen(now) + strlen(fullFrameFileName) + 1 > MAX_FILE_NAME_LEN)
    {
      LOGW("%s: Not appending timestamp since filename is too long - %d instead of %d",
           __FUNCTION__,
           strlen(now) + strlen(fullFrameFileName) + 1,
           MAX_FILE_NAME_LEN);
    }
    else
    {
      strlcat(fullFrameFileName,
              now,
              MAX_FILE_NAME_LEN);
    }
  }

  LOGD("%s: frameFileName is: %s",
       __FUNCTION__,
       fullFrameFileName);

  if (isNotBinary)
  { // Coordinates
    frameFile = fopen(fullFrameFileName,
                      "w+");
  }
  else
  { //Frame File
    frameFile = fopen(fullFrameFileName,
                      "wb+");
    if (WAVE_FORMAT == paramsStruct->usf_frame_file_format)
    {
      LOGD("%s, adding wave header file",
           __FUNCTION__);
      if (0 != ual_util_frame_file_add_wave_hdr(paramsStruct, frameFile))
      {
        if (NULL != frameFile)
        {
          fclose(frameFile);
          frameFile = NULL;
        }
        LOGW("%s: Adding wave header file failure",
             __FUNCTION__);
        return NULL;
      }
    }
  }

  if (NULL == fullFrameFileName)
  {
    LOGW("%s: Open frameFile failure",
         __FUNCTION__);
  }

  return frameFile;
}

/*==============================================================================
  FUNCTION:  ual_util_get_frame_file
==============================================================================*/
/**
  See function description at header file under the function declaration.
*/
FILE * ual_util_get_frame_file (us_all_info const *paramsStruct,
                                char *defaultFrameDirName)
{
  return ual_util_get_file(paramsStruct,
                           defaultFrameDirName,
                           false);
}

/*==============================================================================
  FUNCTION:  ual_util_malloc_read
==============================================================================*/
/**
  See function description at header file under the function declaration.
*/
void*  ual_util_malloc_read(const char* file_name, uint32_t& data_size)
{
  if (NULL == file_name)
  {
    LOGE("%s: File name is NULL",
         __FUNCTION__);
    return NULL;
  }

  FILE* fd = fopen(file_name, "rb");

  if (NULL == fd)
  {
    LOGE("%s: Could not open file %s",
         __FUNCTION__,
         file_name);
    return NULL;
  }

   struct stat sb;
   int ret = stat(file_name, &sb);
   if (ret == -1)
   {
     LOGE("%s: stat of file %s failed; errno=%d",
          __FUNCTION__,
          file_name,
          errno);
     fclose(fd);
     return NULL;
   }

  data_size = sb.st_size;
  void* pData = malloc(data_size);
  if (NULL == pData)
  {
    LOGE("%s: Could not alloc %d bytes for file %s",
         __FUNCTION__,
         data_size,
         file_name);
    fclose(fd);
    return NULL;
  }

  // Reading the required file into the allocated memory.
  ret = fread(pData, data_size, 1, fd);
  if (1 != ret)
  {
    LOGE("%s: Could not read %d bytes from %s; errno=%d",
         __FUNCTION__,
         data_size,
         file_name,
         errno);
    free(pData);
    pData = NULL;
  }
  fclose(fd);

  return pData;
} // ual_util_malloc_read

/*==============================================================================
  FUNCTION:  ual_util_write_file
==============================================================================*/
/**
  Writes the given data to the file path given. Note that 'data_size' is the amount
  of elements in 'data', with each one containing 'element_size' bytes.
*/
int ual_util_write_file(const char *file_path,
                         void *data,
                         uint32_t data_size,
                         int element_size)
{
  if (NULL == file_path || NULL == data || 0 >= data_size)
  {
    LOGE("%s: Invalid input arguments",
         __FUNCTION__);
    return -1;
  }
  FILE* fd = fopen(file_path, "wb");
  if (NULL == fd)
  {
    LOGE("%s: Could not open file %s",
         __FUNCTION__,
         file_path);
    return -1;
  }
  if (fwrite(data,
             element_size,
             data_size,
             fd) < data_size)
  {
    LOGE("%s: Error encountered while writing to file %s",
         __FUNCTION__,
         file_path);
    return -1;
  }
  fclose(fd);
  return 0;
}

/*==============================================================================
  FUNCTION:  ual_util_print_US_version
==============================================================================*/
/**
  See function description at header file under the function declaration.
*/
void ual_util_print_US_version(const char* calculator_name,
                               const char* calculator_version)
{
  us_versions_type us_versions = {NULL, NULL};
  ual_get_version(us_versions);
  if ((NULL == us_versions.p_usf_version) ||
      (NULL == us_versions.p_ual_version))
  {
    LOGE("%s: ual_get_version failure: usf_ver=0x%p, ual_ver=0x%p",
         __FUNCTION__,
         us_versions.p_usf_version,
         us_versions.p_ual_version);
  }
  else
  {
    const char* p_calc_version = calculator_version;
    if (NULL == calculator_version)
    {
      p_calc_version = "-";
    }
    const char* p_calc_name = calculator_name;
    if (NULL == calculator_name)
    {
      p_calc_name = "-";
    }
    LOGI("%s: usf version[%s], ual version[%s], calc[%s] version[%s]",
         __FUNCTION__,
         us_versions.p_usf_version,
         us_versions.p_ual_version,
         p_calc_name,
         p_calc_version);
  }
} // ual_util_print_US_version

/*==============================================================================
  FUNCTION:  ual_util_declare_pid
==============================================================================*/
/**
  See function description at header file under the function declaration.
*/
int ual_util_declare_pid(int pid,
                         const char* pid_filename)
{
  if (NULL == pid_filename || pid < 0)
  {
    LOGE("%s: Problem with parameters",
         __FUNCTION__);
    return -1;
  }
  char full_filename[MAX_FILE_NAME_LEN];
  snprintf(full_filename,
           MAX_FILE_NAME_LEN,
           "%s/%s",
           PID_DIRECTORY,
           pid_filename);
  FILE* file = fopen(full_filename,
                     "w+");
  if (NULL == file) {
    LOGE("%s: Problem opening %s for writing.",
         __FUNCTION__,
         full_filename);
    return -1;
  }
  fprintf(file,
          "%d",
          pid);
  fclose(file);
  LOGD("%s: Wrote to pid file successfully",
       __FUNCTION__);
  return 0;
} // ual_util_declare_pid

/*==============================================================================
  FUNCTION:  ual_util_remove_declare_pid
==============================================================================*/
/**
  See function description at header file under the function declaration.
*/
int ual_util_remove_declare_pid(const char* pid_filename)
{
  if (NULL == pid_filename)
  {
    LOGE("%s: Problem with parameters",
         __FUNCTION__);
    return -1;
  }
  char full_filename[MAX_FILE_NAME_LEN];
  snprintf(full_filename,
           MAX_FILE_NAME_LEN,
           "%s/%s",
           PID_DIRECTORY,
           pid_filename);

  if (0 != remove(full_filename))
  {
    return -1;
  }

  LOGD("%s: Removed pid file successfully",
       __FUNCTION__);
  return 0;
} // ual_util_remove_declare_pid


/*==============================================================================
  FUNCTION: ual_util_prefill_ports_num_and_id
==============================================================================*/
/**
  See function description at header file under the function declaration.
*/
int ual_util_prefill_ports_num_and_id(us_all_info *paramsStruct)
{
  int ret;
  uint32_t temp_port[US_FORM_FACTOR_CONFIG_MAX_MICS];

  // TX
  s_req_mics_num = paramsStruct->usf_tx_port_count;
  if (s_req_mics_num > US_FORM_FACTOR_CONFIG_MAX_MICS)
  {
    LOGW("%s: usf_tx_port_count > US_FORM_FACTOR_CONFIG_MAX_MICS",__FUNCTION__);
    return -1;
  }
  ret = sscanf(paramsStruct->usf_tx_ports,
               "%u ,%u ,%u ,%u, %u ,%u ,%u ,%u",
               temp_port,
               temp_port+1,
               temp_port+2,
               temp_port+3,
               temp_port+4,
               temp_port+5,
               temp_port+6,
               temp_port+7);
  if (ret != s_req_mics_num)
  {
    LOGW("%s: Port count does not match to number of "
         "ports given in usf_tx_ports."
         "port_cnt=%d, ret from scnf of usf_tx_ports=%d",
         __FUNCTION__,
         s_req_mics_num,
         ret);
    return -1;
  }
  for (int ind=0; ind<s_req_mics_num; ++ind)
  {
    s_req_mics_id[ind] = temp_port[ind];
  }

  // RX
  s_req_speakers_num = paramsStruct->usf_rx_port_count;
  if (s_req_speakers_num > US_FORM_FACTOR_CONFIG_MAX_SPEAKERS)
  {
    LOGW("%s: usf_rx_port_count > US_FORM_FACTOR_CONFIG_MAX_SPEAKERS",__FUNCTION__);
    return -1;
  }
  ret = sscanf(paramsStruct->usf_rx_ports,
               "%u ,%u ,%u ,%u, %u ,%u ,%u ,%u",
               temp_port,
               temp_port+1,
               temp_port+2,
               temp_port+3,
               temp_port+4,
               temp_port+5,
               temp_port+6,
               temp_port+7);
  if (ret != s_req_speakers_num)
  {
    LOGW("%s: Port count does not match to number of "
         "ports given in usf_rx_ports."
         "rx_port_cnt=%d, ret from scnf of usf_rx_ports=%d",
         __FUNCTION__,
         s_req_speakers_num,
         ret);
    return -1;
  }
  for (int ind=0; ind<s_req_speakers_num; ++ind)
  {
    s_req_speakers_id[ind] = temp_port[ind];
  }

  return 0;
}

/*==============================================================================
  FUNCTION: ual_util_is_supported
==============================================================================*/
/**
  See function description at header file under the function declaration.
*/
bool ual_util_is_supported(const char *daemon_name)
{
  if (NULL == daemon_name)
  {
    LOGE("%s: Null parameters",
         __FUNCTION__);
    return false;
  }

  char prop_name[PROPERTY_KEY_MAX];
  char prop_val[PROPERTY_VALUE_MAX];

  snprintf(prop_name,
           PROPERTY_KEY_MAX,
           "%s%s",
           IS_SUPPORTED_PROP_PREFIX,
           daemon_name);

  // Gets property value with "0" as default
  property_get(prop_name,
               prop_val,
               "0");

  if (!strcmp(prop_val,
              "0"))
  {
   return false;
  }
  return true;
}

/*==============================================================================
  FUNCTION:  ual_util_alarm_handler
==============================================================================*/
/**
 * See function definition in header file
 */
void ual_util_alarm_handler(int sig)
{
  LOGD("%s: Received SIGALRM",
       __FUNCTION__);
  // alarm is used for exit request repeat (to wake-up blocked functions)
  alarm(1);
}

/*==============================================================================
  FUNCTION:  ual_util_set_buf_size
==============================================================================*/
/**
 * See function definition in header file
 */
void ual_util_set_buf_size(uint32_t *usf_buf_size,
                           uint16_t usf_port_data_size,
                           uint16_t usf_sample_width,
                           uint16_t usf_port_count,
                           uint16_t usf_frame_hdr_size,
                           uint16_t usf_group,
                           const char *type,
                           bool has_event,
                           bool has_raw,
                           uint32_t event_size)
{
  *usf_buf_size = 0;
  if (has_raw)
  {

    *usf_buf_size = usf_port_data_size *
                    (usf_sample_width/8) *
                    usf_port_count +
                    usf_frame_hdr_size;
  }
  if (has_event)
  {
    *usf_buf_size += event_size;
  }

  *usf_buf_size = ((((*usf_buf_size) * usf_group) + ((1<<5)-1)) >> 5) << 5;
  LOGD("%s: usf_%s_buf_size = %d",
       __FUNCTION__,
       type,
       *usf_buf_size);
}

/*==============================================================================
  FUNCTION:  ual_util_close_and_sync_file
==============================================================================*/
/**
 * See function definition in header file
 */
void ual_util_close_and_sync_file(FILE *file)
{
  if (NULL != file)
  {
    if (0 != fsync(fileno(file)))
    {
      LOGW("%s: Flushing file to disk failed. Reason: %s",
           __FUNCTION__,
           strerror(errno));
    }
    fclose(file);
  }
}

/*==============================================================================
  FUNCTION:  ual_util_set_tx_buf_size
==============================================================================*/
/**
 * See function definition in header file
 */
void ual_util_set_tx_buf_size(us_all_info *paramsStruct)
{
  ual_util_set_buf_size(&paramsStruct->usf_tx_buf_size,
                        paramsStruct->usf_tx_port_data_size,
                        paramsStruct->usf_tx_sample_width,
                        paramsStruct->usf_tx_port_count,
                        paramsStruct->usf_tx_frame_hdr_size,
                        paramsStruct->usf_tx_group,
                        "tx",
                        false,
                        true,
                        0);
}

/*==============================================================================
  FUNCTION:  ual_util_set_rx_buf_size
==============================================================================*/
/**
 * See function definition in header file
 */
void ual_util_set_rx_buf_size(us_all_info *paramsStruct)
{
  ual_util_set_buf_size(&paramsStruct->usf_rx_buf_size,
                        paramsStruct->usf_rx_port_data_size,
                        paramsStruct->usf_rx_sample_width,
                        paramsStruct->usf_rx_port_count,
                        paramsStruct->usf_rx_frame_hdr_size,
                        paramsStruct->usf_rx_group,
                        "rx",
                        false,
                        true,
                        0);
}

/*==============================================================================
  FUNCTION:  ual_util_set_epos_tx_transparent_data
==============================================================================*/
/**
 * See function definition in header file
 */
void ual_util_set_epos_tx_transparent_data(us_all_info *paramsStruct)
{
  EposTxTransparentData tx_transparent_data;

  tx_transparent_data.skip = paramsStruct->usf_tx_skip;
  tx_transparent_data.group = paramsStruct->usf_tx_group;
  paramsStruct->usf_tx_transparent_data_size = sizeof(tx_transparent_data);
  memcpy(paramsStruct->usf_tx_transparent_data,
         &tx_transparent_data,
         sizeof(tx_transparent_data));
}

/*==============================================================================
  FUNCTION:  ual_util_set_echo_tx_transparent_data
==============================================================================*/
/**
 * See function definition in header file
 */
void ual_util_set_echo_tx_transparent_data(us_all_info *paramsStruct)
{
  EchoTxTransparentData tx_transparent_data;

  tx_transparent_data.skip = paramsStruct->usf_tx_skip;
  tx_transparent_data.group = paramsStruct->usf_tx_group;
  tx_transparent_data.frame_size = paramsStruct->usf_tx_port_data_size;
  paramsStruct->usf_tx_transparent_data_size = sizeof(tx_transparent_data);
  memcpy(paramsStruct->usf_tx_transparent_data,
         &tx_transparent_data,
         sizeof(tx_transparent_data));
}

/*==============================================================================
  FUNCTION:  ual_util_set_echo_rx_transparent_data
==============================================================================*/
/**
 * See function definition in header file
 */
void ual_util_set_echo_rx_transparent_data(us_all_info *paramsStruct)
{
  EchoRxTransparentData rx_transparent_data;

  rx_transparent_data.group = paramsStruct->usf_rx_group;
  rx_transparent_data.frame_size = paramsStruct->usf_rx_port_data_size;
  paramsStruct->usf_rx_transparent_data_size = sizeof(rx_transparent_data);
  memcpy(paramsStruct->usf_rx_transparent_data,
         &rx_transparent_data,
         sizeof(rx_transparent_data));
}

/*==============================================================================
  FUNCTION:  ual_util_inject_to_trans_data
==============================================================================*/
/**
 * See function definition in header file
 */
int ual_util_inject_to_trans_data(char      *trans_data,
                                  uint32_t  *size,
                                  size_t     max_size,
                                  int        value,
                                  int        num_bytes)
{
  if (NULL == trans_data ||
      NULL == size)
  {
    LOGE("%s: Error, bad arguments",
         __FUNCTION__);
    return 1;
  }
  if(*size + (unsigned)num_bytes >= max_size) { // Max length is not enough
      LOGE("%s: Not enough size for transparent data to inject new value",
           __FUNCTION__);
      // Fatal error, exiting with faliure
      return 1;
  }

  for (int i = 0; i < num_bytes; i++) {
    // Insert byte at index i to transparent data
    unsigned int mask = 0xFF << (i * 8);
    char ith_byte = (char) ((value & mask) >> (i * 8));
    trans_data[*size + i] = ith_byte;
  }
  // Update to new size
  *size = *size + num_bytes;
  return 0;
}

/*==============================================================================
  FUNCTION:  ual_util_ual_open_retries
==============================================================================*/
/**
 * See function definition in header file
 */
int ual_util_ual_open_retries(us_all_info const *paramsStruct)
{
  ual_cfg_type cfg;
  cfg.usf_dev_id = 1;
  cfg.ual_mode = static_cast<ual_work_mode_type>(paramsStruct->ual_work_mode);
  bool rc = false;

  for (int i = 0; i < OPEN_RETRIES; i++)
  {
    rc = ual_open(&cfg);

    if (rc)
    {
      return 0;
    }

    LOGW("%s: Trying to open ual, %d...",
         __FUNCTION__,
         i + 1);
    sleep(UAL_OPEN_SLEEP_TIME);
  }
  LOGE("%s: ual_open failed.",
       __FUNCTION__);
  return 1;
}

/*==============================================================================
  FUNCTION:  ual_util_cfg_parse_uint16_array
==============================================================================*/
/**
 * See function definition in header file
 */
int ual_util_cfg_parse_uint16_array(char *value, uint16_t *out, uint32_t maxSize, uint32_t *actualSize)
{
  if (value == NULL || actualSize == NULL)
  {
    LOGE("%s: NULL value or actualSize params", __FUNCTION__);
    return 1;
  }

  bool has_values = true;
  uint32_t num_values = 0;
  char *str = value, *endptr;

  while (has_values)
  {
    int val = strtol(str, &endptr, 10);
    if ((ERANGE == val) ||
        (endptr == str))
    {
      LOGW("%s: Wrong values string [%s]",
           __FUNCTION__,
           str);
      return 1;
    }

    if (num_values >= maxSize)
    {
      break; // too many values
    }
    out[num_values] = (uint16_t)val;
    num_values++;
    if ('\0' == *endptr)
    {
      has_values = false;
    } else
    {
      str = endptr + 1;
    }
  }

  if (has_values)
  {
    LOGW("%s: too many values (max %u)",
         __FUNCTION__, maxSize);
    return -1;
  }

  *actualSize = num_values;
  return 0;
}

