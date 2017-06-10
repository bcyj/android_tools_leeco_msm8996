/*============================================================================
  @file surround_filters_interface.h

  Overlap add fft filters applicable to speaker array filtering.

        Copyright (c) 2012 Qualcomm Technologies, Inc.
        All Rights Reserved.
        Qualcomm Technologies Confidential and Proprietary
============================================================================*/
#ifndef __SURROUND_FILTERS_INTERFACE_H_
#define __SURROUND_FILTERS_INTERFACE_H_

#include "comdef.h"
#include "profile.h"

/*----------------------------------------------------------------------------
 * External Interface
 * -------------------------------------------------------------------------*/
typedef uint16_t Word16;

// channel_id_t - Channel identifier, used in channel ordering
typedef enum {
    CHANNEL_ID_LEFT           = 0,
    CHANNEL_ID_RIGHT,
    CHANNEL_ID_SURROUND_RIGHT,
    CHANNEL_ID_SURROUND_LEFT,
    CHANNEL_ID_CENTER,
    CHANNEL_ID_LFE,
    NUM_CHANNEL_ID
} channel_id_t;

// FUNCTION
//    surround_filters_init: allocate internal memory, prepare filter coeffs, and
//       return handle.
// PARAMETERS:
//    surroundObj: handle of the surround filters object
//    numOutputs: output channel number (speaker number in speaker array)
//    numInputs: number of input sources (each source maybe mono or stereo)
//    realCoeffs: array of [numInputs] int16* pointers to freq domain filter
//       coeff array, real part
//    imagCoeffs: array of [numInputs] int16* pointers to freq domain filter
//       coeff array, imag part
//    returns: 0 for success and -1 for fail
int surround_filters_init(void *surroundObj, int numOutputs, int numInputs, Word16 **realCoeffs,
					 Word16 **imagCoeffs, int subwoofer, int low_freq, int high_freq, Profiler *prof );

// FUNCTION
//    surround_filters_set_channel_map: set up channel mapping for output.
//    To be called after init.
// PARAMETERS:
//    handle: handle of the surround filters object
//    chanMap: array of int, indexed by channel_id_t, containing
//      one to one mapping of channels.  For y=chanMap[x], the signal for
//      channel x will be placed in position y of the output signal.
//      By default, channel mapping is in order of the channel_id_t enum.
//    returns: 0 for success and -1 for fail
int surround_filters_set_channel_map(void *handle, const int chanMap[NUM_CHANNEL_ID]);

// FUNCTION
//    surround_filters_release: shut down engine and release memory
// PARAMETERS:
//    surroundObj: handle of the surround filters object
void surround_filters_release(void *surroundObj);

// FUNCTION
//    surround_filters_process: processes one frame of input into the outputs
// PARAMETERS:
//    surroundObj: handle of the surround filters object
//    outBuf2d: pointer to output samples, which will be in the format of N
//       (N=8 now) consecutive frames of samples, from left side to right side
//       (in a speaker array)
//    inBufArray: array of [numInputs] int16* pointers. This pointer will point
//       to a frame of 16-bit PCM samples if the input is mono; another
//       consecutive frame follows if the input is stereo. 1 or 2 frames of
//       samples should be backing up accordingly. Sample rate of input samples
//       should be QC_STD_SAMPLERATE (48kHz)
//    inChannelArray: array of channel numbers for each input (1:mono; 2:stereo)
//    returns: none
void surround_filters_process(void *surroundObj, Word16 *outBuf2d, Word16 **inBufArray, int *inChannelArray);

// FUNCTION
//    surround_filters_intl_process: processes one frame of interleaved input
//                                   into interleaved output buffer
// PARAMETERS:
//    surroundObj: handle of the surround filters object
//    outBuf: pointer to interleaved output samples
//    inBuf: pointer to interleaved input samples
//    returns: none
void surround_filters_intl_process(void *surroundObj, Word16 *outBuf, Word16 *inBuf);

// FUNCTION
//    surround_filters_get_framesize: returns frame size of the surround filters
// PARAMETERS:
//    surroundObj: pointer passed back in init function
//    returns: number of samples in a frame to be used in each input / output block
int surround_filters_get_framesize(void *surroundObj);

// FUNCTION
//    surround_filters_get_last_error: returns last error, optionally fills out
//       error string
// PARAMETERS:
//    surroundObj: pointer passed back in init function
//    outString: pointer to memory allocated by client to be filled with error
//       description if numChars > 0
//    outStringLen: memory backing the outString; if 0, outString not accessed
//    returns: 0 if no error, otherwise some error-code
int surround_filters_get_last_error(void *surroundObj, char *outString, int outStringLen);

#endif  /* __SURROUND_FILTERS_INTERFACE_H_ */
