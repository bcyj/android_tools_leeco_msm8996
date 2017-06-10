/*===========================================================================
                           ual_util_frame_file.cpp

DESCRIPTION: Provide common functions implementation to frame file utilities.


INITIALIZATION AND SEQUENCING REQUIREMENTS: None


Copyright (c) 2012-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
=============================================================================*/


/*----------------------------------------------------------------------------
Include files
----------------------------------------------------------------------------*/
#include "usf_log.h"
#include <utils/Log.h>
#include <sys/stat.h>
#include "ual_util_frame_file.h"
#include "usf_validation.h"

/*----------------------------------------------------------------------------
  Defines
----------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------------
  Static Variable Definitions
-----------------------------------------------------------------------------*/
/**
 * The raw output format value
 */
static const uint32_t RAW_OUT_FORMAT  = 0;

/**
 * The sync gesture input format value
 */
static const uint32_t SYNC_GES_IN_FORMAT  = 3;

/**
 * The proximity input format value
 */
static const uint32_t PROX_IN_FORMAT  = 2;

/**
 * The raw input format value
 */
static const uint32_t RAW_IN_FORMAT  = 1;

/**
 * The epos input format value
 */
static const uint32_t EPOS_IN_FORMAT  = 0;

/**
 * The max possible number of channels.
 */
static const int MAX_NUM_OF_CHANNELS = 10;

/**
 * The header length in DWORD
 */
static const int EPOS_CHANNEL_HEADER = 4;

/**
 * Number of bytes in DWORD
 */
static const int DWORD = 4;

/**
 * Size of checksum in DWORD
 */
static const int CHECKSUM_SIZE = 1;

/**
 * The number of samples in epos format channel
 */
static const uint16_t EPOS_NUM_SAMPLES = 512;

/**
 * The size of each desription in wave header
 */
static const int WAVE_DESCRIPTION = 4;

/*-----------------------------------------------------------------------------
  Typedefs
-----------------------------------------------------------------------------*/
/**
 * Contains wave header file information
 */
typedef struct
{
  char chunkID[WAVE_DESCRIPTION];
  uint32_t chunkSize;
  char format[WAVE_DESCRIPTION];
  char subChunk1ID[WAVE_DESCRIPTION];
  uint32_t subchunk1Size;
  uint16_t audioFormat;
  uint16_t numChannels;
  uint32_t sampleRate;
  uint32_t byteRate;
  uint16_t blockAlign;
  uint16_t bitPerSample;
  char subChunk2ID[WAVE_DESCRIPTION];
  uint32_t subchunk2Size;
} wave_header_t;

/*------------------------------------------------------------------------------
  Function definitions
------------------------------------------------------------------------------*/
/*==============================================================================
  FUNCTION:  ual_util_extract_epos
==============================================================================*/
/**
 * This function would extract the given data to a linked list containing the
 * channels data according to the epos format.
 *
 * @param data_buf the data to extract
 * @param frame_file_data output parameter to contain the linked list
 * @param frame_file_data_size output parameter to contain the size of the linked list
 * @param num_samples the number of samples in the extracted data
 * @param paramsStruct struct that contains common daemon information
 *
 * @return uint16_t 0 - failure
 *                  else - returns the number of bytes extracted
 */
static uint32_t ual_util_extract_epos(uint8_t **data_buf,
                                      uint8_t** frame_file_data,
                                      uint16_t* frame_file_data_size,
                                      uint16_t *num_samples,
                                      us_all_info const *paramsStruct);

/*==============================================================================
  FUNCTION:  ual_util_extract_raw
==============================================================================*/
/**
 * This function would extract the given data to a linked list containing the
 * channels data according to the raw format.
 *
 * @param data_buf data to extract
 * @param frame_file_data output parameter to contain the linked list
 * @param frame_file_data_size output parameter to contain the size of the linked
 * list
 * @param num_samples the number of samples in the extracted data
 * @param paramsStruct struct that contains common daemon information
 *
 * @return uint16_t 0 - failure
 *                  else - returns the number of bytes extracted
 */
static uint32_t ual_util_extract_raw(uint8_t **data_buf,
                                     uint8_t** frame_file_data,
                                     uint16_t* frame_file_data_size,
                                     uint16_t *num_samples,
                                     us_all_info const *paramsStruct);

/*==============================================================================
  FUNCTION:  ual_util_frame_file_write_wave
==============================================================================*/
/**
 * This function would write the given data to the given file.
 *
 * @param paramsStruct
 * @param frame_file_data linked list containing data to be written
 * @param frame_file_channel_size the size of each element to be written
 * @param num_samples the number of samples in the extracted data
 * @param element_size the size of each element to be written
 * @param frameFile the file to write the data to.
 *
 * @return int  0 - success
 *              -1 - failure.
 */
static int ual_util_frame_file_write_wave(us_all_info const *paramsStruct,
                                          uint8_t** frame_file_data,
                                          uint16_t frame_file_channel_size,
                                          uint16_t num_samples,
                                          size_t element_size,
                                          FILE* frameFile);

/*==============================================================================
  FUNCTION:  ual_util_frame_file_open_wave
==============================================================================*/
/**
 * See function definition at header file.
 */
int ual_util_frame_file_add_wave_hdr(us_all_info const *paramsStruct,
                                     FILE *frameFileName)
{
  if (NULL == paramsStruct ||
      NULL == frameFileName)
  {
    LOGE("%s, Invalid parameters.",
         __FUNCTION__);
    return -1;
  }
  // Set number of samples according to format
  uint16_t num_samples = paramsStruct->usf_tx_data_format ?
                         paramsStruct->usf_tx_port_data_size :
                         EPOS_NUM_SAMPLES;
  int ret = 0; // Returned value
  // Set skipping factor
  uint16_t skip = 0, group = 0;
  uint32_t frame = 0;
  switch (paramsStruct->usf_tx_data_format)
  {
  case SYNC_GES_IN_FORMAT:
  case RAW_IN_FORMAT:
    ret =  usf_parse_transparent_data(paramsStruct->usf_tx_transparent_data_size,
                                      paramsStruct->usf_tx_transparent_data,
                                      &frame,
                                      &group,
                                      &skip);
    break;
  case EPOS_IN_FORMAT:
    // Epos/Tester_Epos
    ret = usf_parse_transparent_data(paramsStruct->usf_tx_transparent_data_size,
                                     paramsStruct->usf_tx_transparent_data,
                                     &skip,
                                     &group);
    break;
  case PROX_IN_FORMAT:
    // Proximity doesn't have skip/group in its transparent data
    // so we set it manually.
    skip = group = 1;
    break;
  default:
    LOGE("%s, Input format not supported.",
         __FUNCTION__);
    return -1;
  }

  if (0 != ret ||
      0 == skip)
  {
    LOGE("%s, Error while calculating skip.",
         __FUNCTION__);
    return -1;
  }

  wave_header_t wave_header;
  strncpy(wave_header.chunkID , "RIFF", WAVE_DESCRIPTION);
  strncpy(wave_header.format , "WAVE", WAVE_DESCRIPTION);
  strncpy(wave_header.subChunk1ID , "fmt ", WAVE_DESCRIPTION);
  wave_header.subchunk1Size = 16; // For PCM
  wave_header.audioFormat = 1; // PCM
  wave_header.numChannels = paramsStruct->usf_tx_port_count;
  wave_header.sampleRate = paramsStruct->usf_tx_sample_rate / skip;
  wave_header.byteRate = (((paramsStruct->usf_tx_sample_rate / skip) *
            paramsStruct->usf_tx_port_count *
            paramsStruct->usf_tx_sample_width) / 8);
  wave_header.blockAlign = ((paramsStruct->usf_tx_port_count *
            paramsStruct->usf_tx_sample_width) / 8);
  wave_header.bitPerSample = paramsStruct->usf_tx_sample_width;
  strncpy(wave_header.subChunk2ID , "data", WAVE_DESCRIPTION);
  wave_header.subchunk2Size = paramsStruct->usf_frame_count *
    paramsStruct->usf_tx_port_count *
    num_samples *
    paramsStruct->usf_tx_sample_width / 8;
  wave_header.chunkSize = 4 + (8 + wave_header.subchunk1Size) +
    (8 + wave_header.subchunk2Size);

  fwrite(&wave_header, 1, sizeof(wave_header_t), frameFileName);
  return 0;
}

/*==============================================================================
  FUNCTION:  ual_util_frame_file_write
==============================================================================*/
/**
 * See function definition at header file.
 */
int ual_util_frame_file_write (uint8_t *data_buf,
                               size_t element_size,
                               uint32_t bytesFromRegion,
                               us_all_info const *paramsStruct,
                               FILE *frameFile)
{ // Parameter check
  if (NULL == paramsStruct ||
      NULL == frameFile)
  {
    LOGE("%s, Invalid arguments.",
         __FUNCTION__);
    return -1;
  }
  // No data to record
  if (NULL == data_buf)
  {
    return 0;
  }
  // Check that num of ports is supported
  if (MAX_NUM_OF_CHANNELS < paramsStruct->usf_tx_port_count)
  {
    LOGE("%s, number of ports is not supported.",
         __FUNCTION__);
    return -1;
  }

  uint8_t* frame_file_data[MAX_NUM_OF_CHANNELS] = {0};
  uint32_t bytes_written = 0, ret = 0; // Returned value
  uint16_t frame_file_data_size = 0, num_samples = 0;
  // Output parameter is RAW
  if (RAW_OUT_FORMAT == paramsStruct->usf_frame_file_format)
  { // Write the whole data as is.
    fwrite(data_buf,
           element_size,
           bytesFromRegion,
           frameFile);
    return 0;
  }

  // Output parameter is not RAW
  while (bytesFromRegion != bytes_written)
  {
    switch (paramsStruct->usf_tx_data_format)
    {
    case SYNC_GES_IN_FORMAT:
    case PROX_IN_FORMAT:
    case RAW_IN_FORMAT:
      ret = ual_util_extract_raw(&data_buf,
                                 frame_file_data,
                                 &frame_file_data_size,
                                 &num_samples,
                                 paramsStruct);
      break;
    case EPOS_IN_FORMAT:
      // Epos/Tester_Epos
      ret = ual_util_extract_epos(&data_buf,
                                  frame_file_data,
                                  &frame_file_data_size,
                                  &num_samples,
                                  paramsStruct);
      break;
    default:
      LOGE("%s, Input format not supported.",
           __FUNCTION__);
      return -1;
    }

    if (0 == ret)
    { // Failure
      return -1;
    }
    if (0 != ual_util_frame_file_write_wave(paramsStruct,
                                            frame_file_data,
                                            frame_file_data_size,
                                            num_samples,
                                            element_size,
                                            frameFile))
    {
      return -1;
    }
    bytes_written += ret;
  }
  return 0;
}

/*==============================================================================
  FUNCTION:  ual_util_extract_epos
==============================================================================*/
/**
 * See function definition at header file.
 */
uint32_t ual_util_extract_epos(uint8_t **data_buf,
                               uint8_t** frame_file_data,
                               // The size of data in frame_file_data array
                               uint16_t* frame_file_data_size,
                               // The length of the frame_file_data array
                               uint16_t* num_samples,
                               us_all_info const *paramsStruct)
{
  if (NULL == data_buf ||
      NULL == frame_file_data ||
      NULL == frame_file_data_size ||
      NULL == num_samples ||
      NULL == paramsStruct)
  {
    LOGE("%s, Invalid parameters.",
         __FUNCTION__);
    return 0;
  }

  // Go over ports, for each extract the data
  for (int i = 0; i < paramsStruct->usf_tx_port_count; i++)
  {
    *data_buf += EPOS_CHANNEL_HEADER * DWORD;
    frame_file_data[i] = *data_buf;
    *data_buf += EPOS_NUM_SAMPLES * (paramsStruct->usf_tx_sample_width / 8) +
      CHECKSUM_SIZE * DWORD;
  }

  *frame_file_data_size = EPOS_NUM_SAMPLES * (paramsStruct->usf_tx_sample_width / 8);
  *num_samples = EPOS_NUM_SAMPLES;

  return (EPOS_CHANNEL_HEADER * DWORD + *frame_file_data_size + CHECKSUM_SIZE * DWORD) *
    (paramsStruct->usf_tx_port_count);
}

/*==============================================================================
  FUNCTION:  ual_util_extract_raw
==============================================================================*/
/**
 * See function definition at header file.
 */
uint32_t ual_util_extract_raw(uint8_t **data_buf,
                              uint8_t** frame_file_data,
                              uint16_t* frame_file_data_size,
                              uint16_t *num_samples,
                              us_all_info const *paramsStruct)
{
  if (NULL == data_buf ||
      NULL == frame_file_data ||
      NULL == frame_file_data_size ||
      NULL == num_samples ||
      NULL == paramsStruct)
  {
    LOGE("%s, Invalid parameters.",
         __FUNCTION__);
    return 0;
  }

  // Ignore header
  *data_buf += paramsStruct->usf_tx_frame_hdr_size;

  // Data size in bytes
  *frame_file_data_size = paramsStruct->usf_tx_port_data_size *
                          (paramsStruct->usf_tx_sample_width / 8);

  // Go over ports. For each store the beginning in array
  for (int i = 0; i < paramsStruct->usf_tx_port_count; i++)
  {
    frame_file_data[i] = *data_buf;
    *data_buf += paramsStruct->usf_tx_port_data_size *
                 (paramsStruct->usf_tx_sample_width / 8);
  }

  *num_samples = paramsStruct->usf_tx_port_data_size;

  return paramsStruct->usf_tx_frame_hdr_size + (*frame_file_data_size) *
      (paramsStruct->usf_tx_port_count);
}

/*==============================================================================
  FUNCTION:  ual_util_frame_file_write_wave
==============================================================================*/
/**
 * See function definition at header file.
 */
int ual_util_frame_file_write_wave(us_all_info const *paramsStruct,
                                   uint8_t** frame_file_data,
                                   uint16_t frame_file_data_size,
                                   uint16_t num_samples,
                                   size_t element_size,
                                   FILE* frameFile)
{
  if (NULL == paramsStruct ||
      NULL == frame_file_data ||
      NULL == frameFile)
  {
    LOGE("%s, Invalid parameters.",
         __FUNCTION__);
    return -1;
  }

  for (uint16_t j = 0; j < num_samples; j++)
  {
    for (uint16_t i = 0; i < paramsStruct->usf_tx_port_count; i++)
    {
      fwrite(frame_file_data[i],
             element_size,
             (paramsStruct->usf_tx_sample_width / 8),
             frameFile);
      // Increment to the next sample
      frame_file_data[i] += (paramsStruct->usf_tx_sample_width / 8);
    }
  }
  return 0;
}
