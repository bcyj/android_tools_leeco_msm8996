/*
// $Id: //source/divx/drm/sdk1.6/main/latest/inc/DrmApiExt.h#1 $
// Copyright (c) 2005 DivX, Inc. http://www.divx.com/corporate
// All rights reserved.
//
// This software is the confidential and proprietary information of DivX
// Inc. ("Confidential Information").  You shall not disclose such Confidential
// Information and shall use it only in accordance with the terms of the license
// agreement you entered into with DivX, Inc.
*/

/*
These functions are the top level interface to the
Digital Rights Management (DRM) capabilities.
*/

/** @defgroup DRM DRM API
 *  Description of Digital Rights Management (DRM)
 *  @{
 */

#ifndef DRMAPI_EX_H_INCLUDE
#define DRMAPI_EX_H_INCLUDE

#include "DivXVersion.h"

/************************************************************************************
 * DRM FUNCTIONAL MODES:
 *
 *   drmContextRoleAuthentication = Identifies this DRM block as executing as
 *           the Authentication Side Block
 *   drmContextRoleDecryption = Identifies this DRM block as executing as the
 *           Decrypt Side Block
 *   drmContextRoleMonolithic = Identifies this DRM block as handling both
 *           authentication and decryption.
 *
 */
typedef enum drmContextRole
{
    drmContextRoleAuthentication,
    drmContextRoleDecryption,
    drmContextRoleMonolithic
} drmContextRole_t;

typedef enum drmStreamType
{
    /* Bit 0 = 1 for video stream type */
    drmStreamTypeVideoMPEG4ASP = 0x01L,  /* 0001b */
    drmStreamTypeVideoH264     = 0x03L,  /* 0011b */
    /* Bit 0 = 0 for audio stream type */
    drmStreamTypeAudioMP3      = 0x00L,  /* 0000b */
    drmStreamTypeAudioPCM      = 0x02L,  /* 0010b */
    drmStreamTypeAudioAC3      = 0x06L,  /* 0110b */
    drmStreamTypeAudioAAC      = 0x0EL,  /* 1110b */
} drmStreamType_t;

typedef void* drmContext_t;

#ifdef __cplusplus
extern "C" {
#endif

/** @brief @b drmInitSystemEx 

First call should have drmContext set to NULL.  The function will return the length of the drmContext required to be allocated
or passed into the function.

The second call will then subsequently take in the allocated drmContext and initialize it before playback.

This function must be called before any API functions are called and must be called for each playback.

@param[in] role - Must be one of the three roles configuring the DRM sub-system operation.
@param[in] drmContext - Pointer to the buffer containing the memory for the drmContext
@param[in] drmContextLength - Pointer to length of the drmContext memory

@retval DRM_SUCCESS - success.
@retval DRM_NOT_AUTHORIZED - failure.


*/
drmErrorCodes_t drmInitSystemEx( IN drmContextRole_t role,
                                 IN uint8_t* drmContext,
                                 INOUT uint32_t *drmContextLength );

/** @brief @b drmGetDDInfoWriteBufferEx 

Only used for the video stream since DDchunks are expected.

@param[in] drmContext - Handle to the block of memory returned by ::drmInitSystem.
@return uint8_t* is the buffer that the user should write the DDChunk as retrieved from the bitstream.

note: DO NOT DEALLOCATE THIS BUFFER.  The DRM context uses this to cache the DDINFO if needed for frame
decryption and bitstream modification if needed.

*/
uint8_t* drmGetDDInfoWriteBufferEx( IN uint8_t* drmContext );


/** @brief @b drmInsertBitstreamPayloadEx 

@param[in] drmContext - Handle to the block of memory returned by ::drmInitSystem.
@param[in] streamType - The type of stream you are attempting to retrive a chunk for
@param[in] buffer - The user allocated buffer for the chunk data.  The buffer must be at least
drmGetMaxBitstreamPayloadLengthEx() bytes given that this function may
inject data into the buffer.
@param[out]  bufferLength - Number of bytes written to the buffer.

@return drmErrorCodes_t 
@retval DRM_SUCCESS - success.
@retval DRM_GENERAL_ERROR - failure.
@retval DRM_NOT_AUTHORIZED - failure.

*/
drmErrorCodes_t drmInsertBitstreamPayloadEx( IN uint8_t* drmContext, 
                                             IN drmStreamType_t streamType,
                                             INOUT uint8_t* buffer,
                                             IN uint32_t* bufferLength );

/** @brief @b drmDecryptVideoEx 

@param[in] drmContext - Handle to the block of memory returned by ::drmInitSystem.
@param[in,out] frame - Pointer to memory buffer containing the encrypted frame data.
@param[in] frameSize - Size of the data in the @c frame buffer.

@return drmErrorCodes_t
@retval DRM_SUCCESS - success.
@retval DRM_NOT_AUTHORIZED - failure.
*/
drmErrorCodes_t drmDecryptVideoEx( IN uint8_t* drmContext,
                                   IN uint8_t *frame,
                                   IN uint32_t frameSize );

/** @brief @b drmGetMaxBitstreamPayloadLengthEx 

This function should be called when allocating an AVI chunk buffer to determine how
many bytes extra will be needed for the bitstream payload, if any.

@param[in] streamType - Type of stream

@return Length in bytes of the maximum bitstream payload
*/
uint32_t drmGetMaxBitstreamPayloadLengthEx( IN drmStreamType_t streamType );

/**  @}  */

#ifdef __cplusplus
};
#endif

#endif
/* DRMAPI_EX_H_INCLUDE */
