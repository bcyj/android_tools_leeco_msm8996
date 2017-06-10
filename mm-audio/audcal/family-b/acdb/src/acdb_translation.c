/*===========================================================================
    FILE:           acdb_translation.c

    OVERVIEW:       This file contains the implementaion of the translation between
                    data structure versioning changes.
    DEPENDENCIES:   None

                    Copyright (c) 2010-2014 Qualcomm Technologies, Inc.
                    All Rights Reserved.
                    Qualcomm Technologies Proprietary and Confidential.
========================================================================== */

/*===========================================================================
    EDIT HISTORY FOR MODULE

    This section contains comments describing changes made to the module.
    Notice that changes are listed in reverse chronological order. Please
    use ISO format for dates.

    $Header: //source/qcom/qct/multimedia2/Audio/audcal4/acdb_sw/main/latest/acdb/src/acdb_translation.c#2 $

    when        who     what, where, why
    ----------  ---     -----------------------------------------------------
    2010-09-20  ernanl  Initial implementation of the APIs and
                        associated helper methods.
========================================================================== */

/* ---------------------------------------------------------------------------
 * Include Files
 *--------------------------------------------------------------------------- */
#include "acdb_translation.h"
#include "acdb.h"

/* ---------------------------------------------------------------------------
 * Preprocessor Definitions and Constants
 *--------------------------------------------------------------------------- */

/* ---------------------------------------------------------------------------
 * Type Declarations
 *--------------------------------------------------------------------------- */

/* ---------------------------------------------------------------------------
 * Global Data Definitions
 *--------------------------------------------------------------------------- */

/* ---------------------------------------------------------------------------
 * Static Variable Definitions
 *--------------------------------------------------------------------------- */

/* ---------------------------------------------------------------------------
 * Static Function Declarations and Definitions
 *--------------------------------------------------------------------------- */

//Translate sample rate bit flag mask to actual sample rate value
int32_t acdb_translate_sample_rate (const uint32_t nInput,
                                    uint32_t* pOutput
                                    )
{
	switch (nInput)
	{
	 //translate the Sample rate ID
	 case ACDB_SAMPLERATE_8000Hz:
		*pOutput = 8000;
		break;
	 case ACDB_SAMPLERATE_11025Hz:
		*pOutput = 11025;
		break;
	 case ACDB_SAMPLERATE_12000Hz:
		*pOutput = 12000;
		break;
	 case ACDB_SAMPLERATE_16000Hz:
		*pOutput = 16000;
		break;
	 case ACDB_SAMPLERATE_22050Hz:
		*pOutput = 22050;
		break;
	 case ACDB_SAMPLERATE_24000Hz:
		*pOutput = 24000;
		break;
	 case ACDB_SAMPLERATE_32000Hz:
		*pOutput = 32000;
		break;
	 case ACDB_SAMPLERATE_44100Hz:
		*pOutput = 44100;
		break;
	 case ACDB_SAMPLERATE_48000Hz:
		*pOutput = 48000;
		break;
	 case ACDB_SAMPLERATE_96000Hz:
		*pOutput = 96000;
		break;
	 case ACDB_SAMPLERATE_192000Hz:
		*pOutput = 192000;
		break;
	}

    return ACDB_SUCCESS;
}

int32_t acdb_devinfo_getSampleMaskOffset (uint32_t nDeviceType)
{
   int32_t offset= 0; //0 bytes copied

   switch (nDeviceType)
   {
   case ACDB_DEVICE_TYPE_I2S_PARAM:
	  offset = 9*sizeof(uint32_t);
      break;
   case ACDB_DEVICE_TYPE_PCM_PARAM:
	  offset = 9*sizeof(uint32_t);
      break;
   case ACDB_DEVICE_TYPE_SLIMBUS_PARAM:
      offset = 4*sizeof(uint32_t);
      break;
   case ACDB_DEVICE_TYPE_DIGITAL_MIC_PARAM:
	   offset = 6*sizeof(uint32_t);
      break;
   case ACDB_DEVICE_TYPE_HDMI_PARAM:
      offset = 6*sizeof(uint32_t);
      break;
   case ACDB_DEVICE_TYPE_VIRTUAL_PARAM:
      offset = 5*sizeof(uint32_t);
      break;
   case ACDB_DEVICE_TYPE_INT_BTFM_PARAM:
      offset = 4*sizeof(uint32_t);
      break;
   case ACDB_DEVICE_TYPE_RT_PROXY_PARAM:
      offset = sizeof(uint32_t);
      break;
   case ACDB_DEVICE_TYPE_SIMPLE_PARAM:
      offset = sizeof(uint32_t);
      break;
   default:
      offset = 0;
      break;
   }
   return offset;
}

int32_t acdb_devinfo_getBytesPerSampleMaskOffset (uint32_t nDeviceType)
{
   int32_t offset= 0; //0 bytes copied

   switch (nDeviceType)
   {
   case ACDB_DEVICE_TYPE_I2S_PARAM:
	   offset = 6*sizeof(uint32_t);
      break;
   case ACDB_DEVICE_TYPE_PCM_PARAM:
	   offset = 7*sizeof(uint32_t);
      break;
   case ACDB_DEVICE_TYPE_SLIMBUS_PARAM:
      offset = 2*sizeof(uint32_t);
      break;
   case ACDB_DEVICE_TYPE_DIGITAL_MIC_PARAM:
      offset = 3*sizeof(uint32_t);
      break;
   case ACDB_DEVICE_TYPE_HDMI_PARAM:
      offset = 4*sizeof(uint32_t);
      break;
   case ACDB_DEVICE_TYPE_VIRTUAL_PARAM:
      offset = 2*sizeof(uint32_t);
      break;
   case ACDB_DEVICE_TYPE_INT_BTFM_PARAM:
      offset = 2*sizeof(uint32_t);
      break;
   case ACDB_DEVICE_TYPE_RT_PROXY_PARAM:
      offset = 5*sizeof(uint32_t);
      break;
   case ACDB_DEVICE_TYPE_SIMPLE_PARAM:
      offset = 4*sizeof(uint32_t);
      break;
   default:
      offset = 0;
      break;
   }
   return offset;
}
