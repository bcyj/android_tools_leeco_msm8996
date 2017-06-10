/*
// $Id: //source/divx/drm/sdk1.6/main/latest/inc/DrmApi.h#2 $
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

#ifndef DRMAPI_H_INCLUDE
#define DRMAPI_H_INCLUDE

#include "DivXVersion.h"

// Return values.
typedef enum drmErrorCodes 
{
    DRM_SUCCESS = 0,
    DRM_NOT_AUTHORIZED,
    DRM_NOT_REGISTERED,
    DRM_RENTAL_EXPIRED,
    DRM_GENERAL_ERROR,
    DRM_NEVER_REGISTERED,
} drmErrorCodes_t;


#define IN
#define OUT
#define INOUT

#ifdef __cplusplus
extern "C" {
#endif

/** @brief @b drmGetDrmContextSize returns number of bytes of context.

@return uint32_t
@retval Number of bytes of the context.

@b Note: This should be same as DRM_CONTEXT_SIZE_BYTES.  This function can provide a check during
development that the implementation of the context matches the expected size.
*/
uint32_t drmGetDrmContextSize( );

/** @brief @b drmGetStrdSize returns number of bytes of strd chunk in the AVI this version of API can handle.

@return uint32_t
@retval Number of bytes of the strd.

@b Note: This should be same as DRM_STRD_SIZE_BYTES.  If the actual size of the strd chunk is less than this
returned value, the calling application should error out.  It is ok if the strd is larger because it may represent
a backward compatible chunk of larger size.
*/
uint32_t drmGetStrdSize( );

/* Random number functions. */
/** @brief @b drmSetRandomSample creates the set of random samples used within the DRM routines.

@return drmErrorCodes_t
@retval DRM_SUCCESS - success.
@retval DRM_NOT_AUTHORIZED - failure.

@b Note: Call this at least 3 times from user input before call to drmGetRegistrationCode or drmInitPlayback.
DrmAdpLocal.c should be modified with the platform specific version of the function ::localClock().
*/
drmErrorCodes_t drmSetRandomSample( IN uint8_t* drmContext ); 

/** @brief @b drmGetRandomSampleCounter is used to determine the number of random numbers available.

@return uint32_t
@retval Number of random samples available.
*/ 
uint32_t drmGetRandomSampleCounter( IN uint8_t* drmContext ); /* Can be useful for making sure you have enough random samples. */

/* 
   Owner Registration Function. 
*/
/** @brief @b drmGetRegistrationCodeString is used to obtain the product registration code in an encrypted, human
readable form.

@param[in] drmContext - Pointer to the buffer containing the memory for the drmContext
@param[out] registrationCodeString - Registration code

@return drmErrorCodes_t
@retval DRM_SUCCESS - success.
@retval DRM_NOT_AUTHORIZED - failure.

@b Note: DrmAdpLocal.c should be modified with the platform specific version of the function ::localLoadDrmMemory.
*/
drmErrorCodes_t drmGetRegistrationCodeString( IN uint8_t* drmContext,
                                              OUT char *registrationCodeString );

/* 
   Owner Registration Function. 
*/
/** @brief @b drmGetDeactivationCodeString is used to obtain the product registration code in an encrypted, human
readable form.

@param[in] drmContext - Pointer to the buffer containing the memory for the drmContext
@param[out] deactivationCodeString - Null terminated string representing the registration code.

@return drmErrorCodes_t
@retval DRM_SUCCESS - success.
@retval DRM_NOT_AUTHORIZED - failure.

@b Note: DrmAdpLocal.c should be modified with the platform specific version of the function ::localLoadDrmMemory.
*/
drmErrorCodes_t drmGetDeactivationCodeString( IN uint8_t* drmContext,
                                              OUT char *deactivationCodeString );

/* 
   Owner Registration Function. 
*/
/** @brief @b drmGetActivationMessage is used to obtain the product registration code in an encrypted, human
readable form.

@param[in] drmContext - Pointer to the buffer containing the memory for the drmContext
@param[out] activationMessage - Null terminated string representing the registration code.
@param[in] activationMessageLength - length of the buffer that is receiving the message.

@return drmErrorCodes_t
@retval DRM_SUCCESS - success.
@retval DRM_NOT_AUTHORIZED - failure.

@b Note: DrmAdpLocal.c should be modified with the platform specific version of the function ::localLoadDrmMemory.
*/
drmErrorCodes_t drmGetActivationMessage( IN uint8_t* drmContext,
                                         OUT divxint8_t* activationMessage,
                                         IN uint32_t* activationMessageLength );

/* 
   Owner Registration Function. 
*/
/** @brief @b drmGetDeactivationMessage is used to obtain the product registration code in an encrypted, human
readable form.

@param[in] drmContext - Pointer to the buffer containing the memory for the drmContext
@param[out] messageString - Null terminated string representing the registration code.
@param[in] messageStringLength - length of the buffer that is receiving the message.

@return drmErrorCodes_t
@retval DRM_SUCCESS - success.
@retval DRM_NOT_AUTHORIZED - failure.

*/
drmErrorCodes_t drmGetDeactivationMessage( IN uint8_t* drmContext,
                                           OUT divxint8_t* messageString,
                                           IN uint32_t* messageStringLength  );

/* 
   Queries activation status.
*/
/** @brief @b drmGetActivationStatus is used to query the activation status so callers can determine if and who the
device is registered to.

@param[out] userId - User Id if this system has been properly activated.
@param[out] userIdLength - User Id length of the buffer being provided. Should be > 5 bytes otherwise an error will be returned.
This parameter will be modified to the actual bytes returned.

@return drmErrorCodes_t
@retval DRM_SUCCESS - the device is registed to a valid user
@retval DRM_NOT_REGISTERED - the device is not registed to a valid user, it has been deactivated.
@retval DRM_NEVER_REGISTERED - the device has never been registered to a valid user
@retval DRM_GENERAL_ERROR - invalid parameters
*/
drmErrorCodes_t drmGetActivationStatus( OUT uint8_t* userId,
                                        OUT uint32_t* userIdLength );

/* 
   Queries activation status.
*/
/** @brief @b drmIsDeviceActivated is used to query the activation status, will return whether or not the device is currently activated.

@return divxint8_t
@retval DRM_DEVICE_IS_ACTIVATED  - the device is currently activated with a valid user.
@retval DRM_DEVICE_NOT_ACTIVATED - the device is currnetly NOT activated with a valid user.

*/
divxint8_t drmIsDeviceActivated( );

/** @brief @b drmInitDrmMemory 

This function must the first time the device is loaded up it's firmware from the factory.  
It sets the initial DrmMemory values, as expected by the rest of the system.

@retval DRM_SUCCESS - success.
@retval DRM_NOT_AUTHORIZED - failure, could not save DrmMemory.

*/
drmErrorCodes_t drmInitDrmMemory( );

/** @brief @b drmInitSystem 

First call should have drmContext set to NULL.  The function will return the length of the drmContext required to be allocated
or passed into the function.

The second call will then subsequently take in the allocated drmContext and initialize it before playback.

This function must be called before any API functions are called and must be called for each playback.

@param[in] drmContext - Pointer to the buffer containing the memory for the drmContext
@param[in] drmContextLength - Pointer to length of the drmContext memory

@retval DRM_SUCCESS - success.
@retval DRM_NOT_AUTHORIZED - failure.


*/
drmErrorCodes_t drmInitSystem( INOUT uint8_t *drmContext,
                               INOUT uint32_t *drmContextLength );

/* 
   Playback functions. 
*/

/** @brief @b drmInitPlayback 

@param[in] drmContext - Pointer to the buffer containing the memory for the drmContext
@param[in] strdData - The strd data in the AVI file to be played.

@retval DRM_SUCCESS - success.
@retval DRM_NOT_AUTHORIZED - failure.
*/
drmErrorCodes_t drmInitPlayback( IN uint8_t *drmContext,
                                 IN uint8_t *strdData );


/** @brief @b drmQueryRentalStatus is used to determine the rental status of the AVI file used to create the @c drmContext.

@param[in] drmContext - Pointer to the block of memory returned by ::drmInitSystem.
@param[out] rentalMessageFlag - Returned TRUE or FALSE if the rental number of views left screen needs to be displayed.
@param[out] useLimit - Returned the number of playbacks allowed for the file.
@param[out] useCount - Returned the number of time the file has been played back.

@return drmErrorCodes_t
@retval DRM_SUCCESS - success, check rentalFlag, useLimit, and useCount.
@retval DRM_RENTAL_EXPIRED - failure, rental expired.
@retval DRM_NOT_AUTHORIZED - failure.
*/
drmErrorCodes_t drmQueryRentalStatus( IN uint8_t *drmContext,
                                      OUT uint8_t *rentalMessageFlag,
                                      OUT uint8_t *useLimit,
                                      OUT uint8_t *useCount );

/** @brief @b drmCommitPlayback 

@param[in] drmContext - Pointer to the block of memory returned by ::drmInitSystem.

@return drmErrorCodes_t
@retval DRM_SUCCESS - success.
@retval DRM_NOT_AUTHORIZED - failure.

*/
drmErrorCodes_t drmCommitPlayback( IN uint8_t *drmContext );


/** @brief @b drmFinalizePlayback

@param[in] drmContext - Pointer to the block of memory returned by ::drmInitSystem.

@return drmErrorCodes_t
@retval DRM_SUCCESS - success.
@retval DRM_NOT_AUTHORIZED - failure.

*/
drmErrorCodes_t drmFinalizePlayback( IN uint8_t *drmContext );

/** @brief @b drmDecryptVideo 

@param[in] drmContext - Pointer to the block of memory returned by ::drmInitSystem.
@param[in,out] frame - Pointer to memory buffer containing the encrypted frame data.
@param[in] frameSize - Size of the data in the @c frame buffer.
@param[in] drmFrameInfo - Pointer to the memory buffer containing the DRM info for the frame .

@return drmErrorCodes_t
@retval DRM_SUCCESS - success.
@retval DRM_NOT_AUTHORIZED - failure.
*/
drmErrorCodes_t drmDecryptVideo( IN uint8_t *drmContext,
                                 INOUT uint8_t *frame,
                                 IN uint32_t frameSize, 
                                 IN uint8_t *drmFrameInfo );

/** @brief @b drmDecryptAudio 

@param[in] drmContext - Pointer to the block of memory returned by ::drmInitSystem.
@param[in,out] frame - Pointer to memory buffer containing the encrypted frame data.
@param[in] frameSize - Size of the data in the @c frame buffer.

@return drmErrorCodes_t
@retval DRM_SUCCESS - success.
@retval DRM_NOT_AUTHORIZED - failure.
*/
drmErrorCodes_t drmDecryptAudio( IN uint8_t *drmContext,
                                 IN uint8_t *frame,
                                 IN uint32_t frameSize );

/** @brief @b drmQueryCgmsa - Query Copy Generation Management System - Analog

@param[in] drmContext - Pointer to the block of memory returned by ::drmInitSystem.
@param[out] cgmsaSignal - 0, 1, 2, or 3 based on standard CGMSA signaling.

@retval DRM_SUCCESS - success.
@retval DRM_NOT_AUTHORIZED - failure.
*/
drmErrorCodes_t drmQueryCgmsa( IN uint8_t *drmContext,
                               OUT uint8_t *cgmsaSignal );

/** @brief @b drmQueryAcptb - Query Analog Copy Protection Trigger Bits

@param[in] drmContext - Pointer to the block of memory returned by ::drmInitSystem.
@param[out] acptbSignal - 0, 1, 2, or 3 based on standard trigger bit signaling.
acptb values:
0 = Off.
1 = Auto gain control / pseudo sync pulse.
2 = Two line color burst.
3 = Four line color burst.

@retval DRM_SUCCESS - success.
@retval DRM_NOT_AUTHORIZED - failure.
*/
drmErrorCodes_t drmQueryAcptb( IN uint8_t *drmContext,
                               OUT uint8_t *acptbSignal );

/** @brief @b drmQueryDigitalProtection - Query if digital output protection should be turned on.

@param[in] drmContext - pointer to the block of memory returned by ::drmInitSystem
@param[out] digitalProtectionSignal - 0=off, 1=on.

@retval DRM_SUCCESS - success.
@retval DRM_NOT_AUTHORIZED - failure.
*/
drmErrorCodes_t drmQueryDigitalProtection( IN uint8_t *drmContext,
                                           OUT uint8_t *digitalProtectionSignal );


/** @brief @b drmQueryIct - Query if Image Constraint token flag is turned on in the file.

@param[in] drmContext - pointer to the block of memory returned by ::drmInitSystem
@param[out] ict - 0=off, 1=on.

@retval DRM_SUCCESS - success.
@retval DRM_NOT_AUTHORIZED - failure.
*/
drmErrorCodes_t drmQueryIct( IN uint8_t *drmContext,
                             OUT uint8_t *ict );

/** @brief @b drmAdpGetLastError - Useful for debugging and getting more detailed error messages.

@retval error code defined in DrmAdpApi.c (used for debugging).
*/
drmErrorCodes_t drmGetLastError( IN uint8_t* drmContext );

/*!
 Function prototype for the API function called at any time to return
 the version of the implementing library.
  
  @note the char * members of the struct do not need
        memory allocated; the pointed to location will be updated.

  @param pVersion (IN/OUT) Pointer to a library structure into which values will be copied.
  @return DIVXVERSION_OK for success, otherwise returns a DIVXVERSION_ error code.

 */
int drmGetVersion( OUT DivXVersion *pVersion );

/**  @}  */

#ifdef __cplusplus
};
#endif

#endif
/* DRMAPI_H_INCLUDE */
