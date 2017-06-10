#ifndef PR_CLNT_H
#define PR_CLNT_H
/** @file pr_clnt.h
 * @brief
 * This file contains the definitions of the constants, data structures and
 * and interfaces for PlayReady DRM.
 */
/*===========================================================================
  Copyright (c) 2011 - 2014 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Qualcomm Technologies Confidential and Proprietary
===========================================================================*/

/*===========================================================================

                      EDIT HISTORY FOR FILE

when       who     what, where, why
--------   ---     ----------------------------------------------------------
07/09/14    am     Merge as is: kr: Use copy API for clear content playback through secure path
05/13/14    dm     Added support to toggle the IV constraint check at runtime
04/03/14    tp     Formatted by running "atyle --style=allman" command.
11/14/13    wt     Added Media DRM API
11/01/13    cz     Added a new playready_duplicate_ctx api
03/15/13    kr     Update playready_reinitialize() with more parameters
03/07/13    dm     Added support to store AES and keyfile for internal testing purpose.
11/08/12    rk     Added reprovisioning API
09/25/12    rp     Added multiple context support for cpcheck toggle api
09/18/12   chm     Added support for licence deletion with header & KID BASE64 encoded
09/13/12    dm     Added toggling support to enable/disable parsing the non-silent URL
08/21/12    dm     Added additional api to support PR2.0
08/29/12    rk     Moved generic provisioning API declaration to drmprov folder
07/18/12    rp     Added toggling support to PR key hardcoding and multithread support for shared ion buffers
05/16/12    dm     Added support for drm reinitialize and modified get property API.
04/16/12    kr     Additional PlayReady API support for WMDRM
04/11/12    rp     Added the support for turning on/off content protection check on TZ
03/26/12    dm     Added a new parameter in process license response()
03/21/12    kr     Add generic key provisioning APIs support
01/31/12    cz     Changed video and audio decryption functions to support fd and virtual address
01/27/12    cz     Changed the name of data_prt to data_ion_fd in audio and video decryption calls
01/19/12    dm     Added audio decryption and tamepoering check.
12/14/11    dm     Added support for bind,commit,MTP,domaind and envelope.
10/14/11    cz     Cleaned up v1 functions and replace v1 funcation names with v2 names
10/12/11    cz     Added playready v2 functions and disabled v1 functions
09/21/11   chm     Initial Version.

===========================================================================*/

/*===========================================================================

                     INCLUDE FILES FOR MODULE

===========================================================================*/
/** @addtogroup pr_clnt
  @} */
#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

#include <sys/types.h>
#include <QSEEComAPI.h>
#include "playready_entry.h"


    /********************************************************************************
      content protection check enable or disable on TZ.

      @return
      E_SUCCESS   - Success.
      E_FAILED    - Failure.

      @param[in]	 bContentProtection - 0/1- off/on the check.

      @dependencies
      None

      @sideeffects
      None
    ********************************************************************************/

    long playready_cpcheck_toggle
    (
        uint8             bContentProtection,
        uint32            app_ctx_id
    );

    /********************************************************************************
      Provision PlayReady keys using sfs.

      @return
      E_SUCCESS   - Success.
      E_FAILED    - Failure.

      @param[in]	  key        Type of key to provision.
      @param[in]	 *data       Data of the key.
      @param[in]	  size       Size of the key.

      @dependencies
      None

      @sideeffects
      None
    ********************************************************************************/
    long playready_save_keys
    (
        tz_pr_keys_type key,
        uint8 * data,
        uint32 size
    );


    /********************************************************************************
      Verify PlayReady keys.

      @return
      E_SUCCESS   - Success.
      E_FAILED    - Failure.

      @dependencies
      None

      @sideeffects
      None
    ********************************************************************************/
    long playready_verify_keys(void);


    /********************************************************************************
      Delete PlayReady keys.

      @return
      E_SUCCESS   - Success.
      E_FAILED    - Failure.

      @dependencies
      None

      @sideeffects
      None
    ********************************************************************************/
    long playready_delete_keys(void);


    /********************************************************************************
      Initializes PlayReady DRM.

      @return
      Refer to Microsoft Playready Porting Kit for the error codes.

      @param[out]    *app_ctx_id        Pointer to the Application context

      @dependencies
      Needs to be called before any other DRM functions are called.

      @sideeffects
      None
    ********************************************************************************/
    long playready_initialize
    (
        uint32         *app_ctx_id
    );


    /********************************************************************************
      UnInitializes PlayReady DRM.

      @return
      none

      @param[in]     app_ctx_id           Application context
      @param[in]     dec_ctx_id           Decrypt context
      @param[in]     dom_ctx_id           Domain context

      @dependencies
      Needs to be called to free up resources.

      @sideeffects
      None
    ********************************************************************************/
    void playready_uninitialize
    (
        uint32         app_ctx_id,
        uint32         dec_ctx_id,
        uint32         dom_ctx_id
    );


    /********************************************************************************
      Get's a license challenge from the DRM Agent.

      @return
      Refer to Microsoft Playready Porting Kit for the error codes.

      @param[in]       app_ctx_id       Application context
      @param[in]     **strRights        Array of pointers to the rights array.
      @param[in]       cRights          Number of elements in rights array.
      @param[in]      *domainID         Pointer to the domain ID.
      @param[in]      *pchCustomData    Pointer to the custom data.
      @param[in]       pcchCustomData   Size of the custom data.
      @param[out]     *pchSilentUrl     Contains the silent license acquisition URL.
      @param[in,out]  *pcchSilentUrl    Contains the size of the silent license acquisition URL.
      @param[out]     *pchNonSilentUrl  Contains the nonsilent license acquisition URL.
      @param[in,out]  *pcchNonSilentUrl Contains the size of the nonsilent license acquisition URL.
      @param[out]     *pbChallenge      Contains the resulting license challenge.
      @param[in,out]  *pcbChallenge     Contains the size of the resulting license challenge.

      @dependencies
      Need to have called playready_initialize() before calling this function.

      @sideeffects
      None
    ********************************************************************************/
    long playready_licacq_generate_challenge
    (
        uint32             app_ctx_id,
        tzconststring_t  **strRights,
        uint32             cRights,
        tzdrmdomainid_t   *domainID,
        uint8             *pchCustomData,
        uint32             pcchCustomData,
        uint8             *pchSilentUrl,
        uint32            *pcchSilentUrl,
        uint8             *pchNonSilentUrl,
        uint32            *pcchNonSilentUrl,
        uint8             *pbChallenge,
        uint32            *pcbChallenge
    );


    /********************************************************************************
      Store a license in the license database.

      @return
      Refer to Microsoft Playready Porting Kit for the error codes.

      @param[in]     app_ctx_id              Application context
      @param[in]    *pbResponse              Pointer to license response.
      @param[in]     cbResponse              Size of license response.
      @param[out]   *poLicenseResponse       Pointer to license response
      @dependencies
      Need to have called playready_set_header() before calling this function.

      @sideeffects
      None
    ********************************************************************************/
    unsigned long playready_licacq_process_response
    (
        uint32             app_ctx_id,
        uint8             *pbResponse,
        uint32             cbResponse,
        tzdrmlicensersp_t *poLicenseResponse
    );


    /********************************************************************************
      Delete's a license from the store.

      @return
      Refer to Microsoft Playready Porting Kit for the error codes.

      @param[in]    app_ctx_id   Application context
      @param[in]    KID          Pointer to the Key ID (not BASE64 encoded)

      @dependencies
      Need to have called playready_initialize before calling this function.

      @sideeffects
      None
    ********************************************************************************/
    long playready_delete_licenses
    (
        uint32            app_ctx_id,
        char             *KID
    );

    /********************************************************************************
      Delete's a license from the store

      @return
      Refer to Microsoft Playready Porting Kit for the error codes.

      @param[in]    app_ctx_id   Application context
      @param[in]    KID          Pointer to the Key ID (BASE64 encoded)
      @param[in]    KIDLen       Length of KID, normally 48 bytes.
      @param[out]   nLicDeleted  Pointer to the number of licensed that got deleted.

      @dependencies
      Need to have called playready_initialize before calling this function.

      @sideeffects
      None
    ********************************************************************************/
    long playready_storemgmt_deletelicenses
    (
        uint32         app_ctx_id,
        uint8         *pKID,
        uint32         pKIDLen,
        uint32          *nLicDeleted
    );

    /********************************************************************************
      Decrypt PlayReady DRM Content Data.

      @return
      Refer to Microsoft Playready Porting Kit for the error codes.

      @param[in]       dec_ctx_id                  Decrypt context
      @param[in]      *encryptionMetaData          IV, Offset, Block , Byte for decryption.
      @param[in]       numEncryptedUnits           How many entries in the encryption Metadata.
      @param[in]      *in_vir_addr                 virtual address of the input buffer
      @param[in]       out_ion_fd                  Pointer to decrypted buffer.

      @dependencies
      Need to have called playready_set_header() before calling this function.

      @sideeffects
      None
    ********************************************************************************/
    long playready_reader_decrypt
    (
        uint32                          dec_ctx_id,
        tzEncryptionMetaData_t         *encryptionMetaData,
        uint8                           numEncryptedUnits,
        uint8                          *in_vir_addr,
        uint32                          out_ion_fd
    );

    /********************************************************************************
      Set DRM Header.

      @return
      Refer to Microsoft Playready Porting Kit for the error codes.

      @param[in]      app_ctx_id       Application context
      @param[in]     *pbHeader         Pointer to DRM Header
      @param[in]      cbHeader         Size of DRM Header.

      @dependencies
      Need to have called playready_initialize before calling this function.

      @sideeffects
      None
    ********************************************************************************/
    long playready_set_header
    (
        uint32            app_ctx_id,
        uint8            *pbHeader,
        uint32            cbHeader
    );


    /********************************************************************************
      Gets the license state for the protected file.

      @return
      Refer to Microsoft Playready Porting Kit for the error codes.

      @param[in]      app_ctx_id             Application context
      @param[in]     *licenseStateData       Pointer to the license state information

      @dependencies
      Need to have called playready_set_header before calling this function.

      @sideeffects
      None
    ********************************************************************************/
    long playready_get_license_rights
    (
        uint32                           app_ctx_id,
        tzlicensestatedata_t            *licenseStateData
    );

    /**
      Binds to a valid license from the data store to render the content.

      @return
      Refer to Microsoft Playready Porting Kit for the error codes.

      @param[in]       app_ctx_id       Application context
      @param[in]     **strRights        Array of pointers to the rights array.
      @param[in]       cRights          Number of elements in rights array.
      @param[in]       dec_ctx          Decrypt context

      @dependencies
      Need to have called playready_set_header() before calling this function.

      @sideeffects
      None
    */
    long playready_reader_bind
    (
        uint32             app_ctx_id,
        tzconststring_t  **strRights,
        uint32             cRights,
        uint32            *dec_ctx_id
    );

    /********************************************************************************
      Decrypt PlayReady Audio Data with virtual address

      @return
      Refer to Microsoft Playready Porting Kit for the error codes.

      @param[in]       dec_ctx_id                  Decrypt context
      @param[in]      *encryptionMetaData          IV, Offset, Block , Byte for decryption.
      @param[in]       numEncryptedUnits           How many entries in the encryption Metadata.
      @param[in]      *in_vir_addr                 virtual address of the input and output buffer
      @param[in]       input_length                the length of input buffer

      @dependencies
      Need to have called playready_set_header() before calling this function.

      @sideeffects
      None
    ********************************************************************************/
    long playready_reader_decrypt_audio
    (
        uint32                          dec_ctx,
        tzEncryptionMetaData_t         *encryptionMetaData,
        uint8                           numEncryptedUnits,
        uint8                          *in_vir_addr
    );

    /**
      Commits all state data to the data store.

      @return
      Refer to Microsoft Playready Porting Kit for the error codes.

      @param[in]       app_ctx_id          Application context

      @dependencies
      Need to have called playready_reader_bind() before calling this function.

      @sideeffects
      None
    */
    long playready_reader_commit
    (
        uint32             app_ctx_id
    );

    /**
      Returns the synchronization list from PC.

      @return
      Refer to Microsoft Playready Porting Kit for the error codes.

      @param[in]      app_ctx_id           Application context
      @param[in]      cMaxCount            Maximum counts remaining
      @param[in]      cMaxHoursSize        Maximum hours remaining
      @param[in]      iStart               Index of the first item to process in the sync list
      @param[in]      cItemsToProcess      Total number of items to process in the sync list
      @param[out]    *iNextStart           Pointer to the next item in the sync list
      @param[out]    *cProcessed           Contains the number of items processed
      @param[out]    *pbChallenge          Contains the resulting synchronization challenge data
      @param[in,out] *cbChallenge          Contains the size of the resulting synchronization challenge data

      @dependencies
      Need to have called playready_initialize() before calling this function.

      @sideeffects
      None
    */
    long playready_sync_generate_challenge
    (
        uint32         app_ctx_id,
        uint32         cMaxCount,
        uint32         cMaxHours,
        uint32         iStart,
        uint32         cItemsToProcess,
        uint32        *iNextStart,
        uint32        *cProcessed,
        uint8         *pbChallenge,
        uint32        *cbChallenge
    );

    /**
      Gets the metering data for a specific metering ID.

      @return
      Refer to Microsoft Playready Porting Kit for the error codes.

      @param[in]      app_ctx_id         Application context
      @param[in]     *pbMeterCert        Pointer to the metering certificate
      @param[in]      cbMeterCert        Size of the metering certificate
      @param[out]    *pbChallenge        Contains the resulting metering challenge
      @param[in,out] *cbChallenge        Contains the size of the metering challenge

      @dependencies
      Need to have called playready_initialize() before calling this function.

      @sideeffects
      None
    */
    long playready_metering_generate_challenge
    (
        uint32          app_ctx_id,
        uint8          *pbMeterCert,
        uint32          cbMeterCert,
        uint8          *pbChallenge,
        uint32         *cbChallenge
    );


    /**
      Sets the meter response on the device.

      @return
      Refer to Microsoft Playready Porting Kit for the error codes.

      @param[in]    app_ctx_id          Application context
      @param[in]   *pbResponse          Pointer to the metering response
      @param[in]    cbResponse          Size of the metering response
      @param[out]  *fFlagsOut           Pointer to the MeterResponseFlags

      @dependencies
      Need to have called playready_initialize() before calling this function.

      @sideeffects
      None
    */
    unsigned long playready_metering_process_response
    (
        uint32          app_ctx_id,
        uint8          *pbResponse,
        uint32          cbResponse,
        uint32         *fFlagsOut
    );

    /**
      Cleans the device data store.

      @return
      Refer to Microsoft Playready Porting Kit for the error codes.

      @param[in]       app_ctx_id          Application context
      @param[in]       flags               Flags to indicate what type of clean operations to perform.  Valid values are:
                                           - DRM_STORE_CLEANUP_ALL
                                           - DRM_STORE_CLEANUP_DELETE_EXPIRED_LICENSES
                                           - DRM_STORE_CLEANUP_DELETE_REMOVAL_DATE_LICENSES
                                           - DRM_STORE_CLEANUP_RESET
      @dependencies
      Need to have called playready_initialize() before calling this function.
      When using DRM_STORE_CLEANUP_RESET you need to make sure you don't have any additional playready sessions open
      as this deletes the keys and

      @sideeffects
      None
    */
    unsigned long playready_cleanup_data_store
    (
        uint32          app_ctx_id,
        uint32            flags
    );


    /**
      Resets PlayReady DRM.

      @return

      @param[in]       flags               Flags to indicate what type of reset operation to perform. Not used currently.

      @dependencies
      No playready session should be running when calling this.

      @sideeffects
      None
    */
    long playready_reset
    (
        uint32                  flags
    );

    /**
      Gets the license state of the media content.

      @return
      Refer to Microsoft Playready Porting Kit for the error codes.

      @param[in]    app_ctx_id        Application context
      @param[in]   *pwszString        Pointer to the license state query data
      @param[in]    cchString         Size of the license state query data
      @param[out]  *lsCategory        Contains the number of play count/hours remaining

      @dependencies
      Need to have called playready_initialize() before calling this function.

      @sideeffects
      None
    */
    unsigned long playready_get_license_state
    (
        uint32                       app_ctx_id,
        const unsigned short        *pwszString,
        unsigned long                cchString,
        uint32                      *lsCategory
    );

    /**
      Gets the device certificate stored in SFS

      @return
      Refer to Microsoft Playready Porting Kit for the error codes.

      @param[out]    *pbDevCert           Pointer to the device certificate
      @param[in,out] *cbDevCert           Size of the device certificate

      @dependencies
      None

      @sideeffects
      None
    */
    long playready_get_device_certificate
    (
        uint8           *pbDevCert,
        uint32          *cbDevCert
    );

    /**
      Gets the status of the command sent for processing.

      @return
      Refer to Microsoft Playready Porting Kit for the error codes.

      @param[in]    app_ctx_id          Application context
      @param[in]    opcode              Operation code for the command to process
      @param[in]    requestarg1         Request argument
      @param[in]    requestarg2         Request argument
      @param[in]    requestarg3         Request argument
      @param[in]    requestarg4         Request argument
      @param[in]   *requestdata         Pointer to the requestdata
      @param[in]    requestdatalen      Size of the requestdata
      @param[out]  *response1           Contains the response parameter
      @param[out]  *response2           Contains the response parameter
      @param[out]  *response3           Contains the response parameter
      @param[out]  *response4           Contains the response parameter

      @dependencies
      Need to have called playready_initialize() before calling this function.

      @sideeffects
      None
    */
    long playready_process_command
    (
        uint32         app_ctx_id,
        uint32         opcode,
        uint32         requestarg1,
        uint32         requestarg2,
        uint32         requestarg3,
        uint32         requestarg4,
        uint8         *requestdata,
        uint32         requestdatalen,
        uint32        *response1,
        uint32        *response2,
        uint32        *response3,
        uint32        *response4
    );

    /**
      Gets the status of the data request being sent.

      @return
      DRM_E_NOTIMPL - This function is not implemented.

      @param[in]    app_ctx_id          Application context
      @param[in]    opcode              Operation code for the command to process
      @param[in]    requestarg1         Request argument
      @param[in]    requestarg2         Request argument
      @param[in]    requestarg3         Request argument
      @param[in]    requestarg4         Request argument
      @param[out]  *responsedata        Pointer to the responsedata
      @param[out]  *responsedatalen     Size of the responsedata
      @param[out]  *response1           Contains the response parameter
      @param[out]  *response2           Contains the response parameter
      @param[out]  *response3           Contains the response parameter
      @param[out]  *response4           Contains the response parameter

      @dependencies
      Need to have called playready_initialize() before calling this function.

      @sideeffects
      Not implemented in Playready Porting Kit.
    */
    long playready_process_request
    (
        uint32         app_ctx_id,
        uint32         opcode,
        uint32         requestarg1,
        uint32         requestarg2,
        uint32         requestarg3,
        uint32         requestarg4,
        uint8         *responsedata,
        uint32        *responsedatalen,
        uint32        *response1,
        uint32        *response2,
        uint32        *response3,
        uint32        *response4
    );

    /**
      Opens the enveloped file.

      @return
      Refer to Microsoft Playready Porting Kit for the error codes.

      @param[in]   app_ctx_id          Application context
      @param[in]  *filename            Pointer to the filename
      @param[out] *env_ctx_id          Pointer to the Envelope context

      @dependencies
      Need to have called playready_initialize() before calling this function.

      @sideeffects
      None
    */
    long playready_envelope_open
    (
        uint32           app_ctx_id,
        unsigned short  *filename,
        uint32          *env_ctx_id
    );

    /**
      Closes an enveloped file.

      @return
      Refer to Microsoft Playready Porting Kit for the error codes.

      @param[in]   env_ctx_id             Envelope context

      @dependencies
      Need to have called playready_envelope_open() before calling this function.

      @sideeffects
      None
    */
    long playready_envelope_close
    (
        uint32          env_ctx_id
    );

    /**
      Associates the decrypt context with the enveloped file.

      @return
      Refer to Microsoft Playready Porting Kit for the error codes.

      @param[in]   env_ctx_id             Envelope context
      @param[in]   dec_ctx_id             Decrypt context

      @dependencies
      Need to have called playready_envelope_open() and playready_reader_bind() before calling this function.

      @sideeffects
      None
    */
    long playready_envelope_initialize_read
    (
        uint32       env_ctx_id,
        uint32       dec_ctx_id
    );

    /**
      Reads clear data from the enveloped file.

      @return
      Refer to Microsoft Playready Porting Kit for the error codes.

      @param[in]     env_ctx_id          Envelope context
      @param[out]   *pbBuffer            Pointer to the buffer that receives the clear data
      @param[in]     cbToRead            Number of bytes to read
      @param[out]   *pcbBytesRead        Pointer to the number of bytes read

      @dependencies
      Need to have called playready_envelope_initialize_read() before calling this function.

      @sideeffects
      None
    */
    long playready_envelope_read
    (
        uint32           env_ctx_id,
        uint8           *pbBuffer,
        uint32           cbToRead,
        uint32          *pcbBytesRead
    );

    /**
      Seeks within an enveloped file.

      @return
      Refer to Microsoft Playready Porting Kit for the error codes.

      @param[in]     env_ctx_id          Envelope context
      @param[in]     lDistanceToMove     Distance by which to move the file pointer
      @param[in]     dwMoveMethod        Reference point by which to move the file pointer
      @param[out]   *pdwNewFilePointer   Pointer to the new position in the file

      @dependencies
      Need to have called playready_envelope_initialize_read() before calling this function.

      @sideeffects
      None
    */
    long playready_envelope_seek
    (
        uint32           env_ctx_id,
        long             lDistanceToMove,
        uint32           dwMoveMethod,
        uint32          *pdwNewFilePointer
    );

    /**
      Duplicates an envelope file handle.

      @return
      Refer to Microsoft Playready Porting Kit for the error codes.

      @param[in]     env_ctx_id          Envelope context
      @param[in]    *pwszFilename        Filename to re-open
      @param[out]    env_ctx_new         Pointer to the duplicate envelope context

      @dependencies
      Need to have called playready_envelope_open() before calling this function.

      @sideeffects
      None
    */
    long playready_envelope_duplicate_filecontext
    (
        uint32                  env_ctx_id,
        const unsigned short   *pwszFilename,
        uint32                 *env_ctx_new
    );

    /**
      Gets the original filename for the enveloped file.

      @return
      Refer to Microsoft Playready Porting Kit for the error codes.

      @param[in]       env_ctx_id              Envelope context
      @param[out]     *pwszOriginalFilename    Filename to re-open
      @param[in,out]  *pcchOriginalFilename    Size of the filename

      @dependencies
      Need to have called playready_envelope_open() before calling this function.

      @sideeffects
      None
    */
    long playready_envelope_get_originalfilename
    (
        uint32                  env_ctx_id,
        unsigned short         *pwszOriginalFilename,
        uint32                 *pcchOriginalFilename
    );

    /**
      Gets the size of the enveloped file.

      @return
      Refer to Microsoft Playready Porting Kit for the error codes.

      @param[in]       env_ctx_id              Envelope context
      @param[out]     *pcbFileSize             Size of the enveloped file

      @dependencies
      Need to have called playready_envelope_open() before calling this function.

      @sideeffects
      None
    */
    long playready_envelope_get_size
    (
        uint32                  env_ctx_id,
        uint32                 *pcbFileSize
    );

    /**
      Updates the embedded store of the Playready object with the license from the license store.

      @return
      Refer to Microsoft Playready Porting Kit for the error codes.

      @param[in]   app_ctx_id             Application context

      @dependencies
      Need to have called playready_licacq_process_response() before calling this function.

      @sideeffects
      None
    */
    long playready_update_embeddedstore
    (
        uint32               app_ctx_id
    );

    /**
      Writes the Playready object to the enveloped file.

      @return
      Refer to Microsoft Playready Porting Kit for the error codes.

      @param[in]       app_ctx_id              Application context
      @param[in]      *pwszFilename            Pointer to the filename where the object is to be written
      @param[in,out]  *env_ctx_id              Pointer to the envelope context

      @dependencies
      Need to have called playready_get_property() before calling this function.

      @sideeffects
      None
    */
    long playready_envelope_write_playreadyobject
    (
        uint32                  app_ctx_id,
        unsigned short         *pwszFilename,
        uint32                 *env_ctx_id
    );

    /**
      Commits the transactions to the embedded store.

      @return
      Refer to Microsoft Playready Porting Kit for the error codes.

      @param[in]   app_ctx_id             Application context

      @dependencies
      Need to have called playready_envelope_write_playreadyobject() before calling this function.

      @sideeffects
      None
    */
    long playready_update_embeddedstore_commit
    (
        uint32               app_ctx_id
    );

    /**
      Get DRM header property.

      @return
      Refer to Microsoft Playready Porting Kit for the error codes.

      @param[in]          app_ctx_id             Application context
      @param[in]          eproperty              Header property type to retreive
      @param[out]        *pbPropertyData         Pointer to DRM Property Data.
      @param[in,out]     *pcbPropertyData        Size of DRM Property Data.

      @dependencies
      Need to have called playready_set_header() before calling this function.

      @sideeffects
      None
    */
    long playready_get_property
    (
        uint32               app_ctx_id,
        tzdrmgetproptype     eproperty,
        uint8               *pbPropertyData,
        uint32              *pcbPropertyData
    );

    /**
      Set Domain ID.

      @return
      Refer to Microsoft Playready Porting Kit for the error codes.

      @param[in]     *pServiceID       Pointer to the service ID
      @param[in]      pcServiceID      Size of the service ID
      @param[in]     *pAccountID       Pointer to the account ID
      @param[in]      pcAccountID      Size of the account ID
      @param[in]      pRevision        Pointer to the revision ID
      @param[in]     *pDomainID        Pointer to the domain ID

      @dependencies
      Need to have called playready_initialize() before calling this function.

      @sideeffects
      None
    */
    long playready_set_domainid
    (
        uint8                 *pServiceID,
        uint32                 pcServiceID,
        uint8                 *pAccountID,
        uint32                 pcAccountID,
        uint32                 pRevision,
        tzdrmdomainid_t       *pDomainID
    );

    /**
      Gets the join domain challenge from the DRM Agent.

      @return
      Refer to Microsoft Playready Porting Kit for the error codes.

      @param[in]       app_ctx_id       Application context
      @param[in]       dwFlags          Flag to indicate the type of custom data
      @param[in]      *pDomainID        Pointer to the domain ID
      @param[in]      *pchFriendlyName  Pointer to the friendlyname
      @param[in]       cchFriendlyName  Size of the friendlyname
      @param[in]      *pchData          Pointer to the custom data
      @param[in]       cchData          Size of the custom data
      @param[out]     *pbChallenge      Contains the resulting join domain challenge
      @param[in,out]  *pcbChallenge     Contains the size of the resulting join domain challenge

      @dependencies
      Need to have called playready_set_domainid() before calling this function.

      @sideeffects
      None
    */
    long playready_joindomain_generate_challenge
    (
        uint32                 app_ctx_id,
        uint32                 dwFlags,
        tzdrmdomainid_t       *pDomainID,
        uint8                 *pchFriendlyName,
        uint32                 cchFriendlyName,
        uint8                 *pchData,
        uint32                 cchData,
        uint8                 *pbChallenge,
        uint32                *pcbChallenge
    );

    /**
      Sets the join domain response on the device.

      @return
      Refer to Microsoft Playready Porting Kit for the error codes.

      @param[in]     app_ctx_id              Application context
      @param[in]    *pbResponse              Pointer to join domain response
      @param[in]     cbResponse              Size of join domain response
      @param[out]   *pResult                 Pointer to the status code
      @param[out]   *pDomainID               Pointer to the domain ID

      @dependencies
      Need to have called playready_joindomain_generate_challenge() before calling this function.

      @sideeffects
      None
    */
    long playready_joindomain_process_response
    (
        uint32                app_ctx_id,
        uint8                *pbResponse,
        uint32                cbResponse,
        long                 *pResult,
        tzdrmdomainid_t      *poDomainID
    );

    /**
      Gets the leave domain challenge from the DRM Agent.

      @return
      Refer to Microsoft Playready Porting Kit for the error codes.

      @param[in]       app_ctx_id       Application context
      @param[in]       dwFlags          Flag to indicate the type of custom data
      @param[in]      *pDomainID        Pointer to the domain ID
      @param[in]      *pchData          Pointer to the custom data
      @param[in]       cchData          Size of the custom data
      @param[out]     *pbChallenge      Contains the resulting leave domain challenge
      @param[in,out]  *pcbChallenge     Contains the size of the resulting leave domain challenge

      @dependencies
      Need to have called playready_set_domainid() before calling this function.

      @sideeffects
      None
    */
    long playready_leavedomain_generate_challenge
    (
        uint32                 app_ctx_id,
        uint32                 dwFlags,
        tzdrmdomainid_t       *pDomainID,
        uint8                 *pchData,
        uint32                 cchData,
        uint8                 *pbChallenge,
        uint32                *pcbChallenge
    );

    /**
      Sets the leave domain response on the device.

      @return
      Refer to Microsoft Playready Porting Kit for the error codes.

      @param[in]     app_ctx_id              Application context
      @param[in]    *pbResponse              Pointer to leave domain response
      @param[in]     cbResponse              Size of leave domain response
      @param[out]   *pResult                 Pointer to the status code

      @dependencies
      Need to have called playready_leavedomain_generate_challenge() before calling this function.

      @sideeffects
      None
    */
    long playready_leavedomain_process_response
    (
        uint32                app_ctx_id,
        uint8                *pbResponse,
        uint32                cbResponse,
        long                 *pResult
    );

    /**
      Finds the domain certificate from the domain store.

      @return
      Refer to Microsoft Playready Porting Kit for the error codes.

      @param[in]       app_ctx_id       Application context
      @param[in]      *pDomainID        Pointer to the domain ID
      @param[out]     *pbDomainCert     Contains the resulting domain certificate
      @param[in,out]  *pcbDomainCert    Contains the size of the resulting domain certificate

      @dependencies
      Need to have called playready_set_domainid() before calling this function.

      @sideeffects
      None
    */
    long playready_domaincert_find
    (
        uint32                  app_ctx_id,
        tzdrmdomainid_t        *poDomainID,
        uint8                  *pbDomainCert,
        uint32                 *pcbDomainCert
    );

    /**
      Initializes the domain certificate enumeration context.

      @return
      Refer to Microsoft Playready Porting Kit for the error codes.

      @param[in]       app_ctx_id          Application context
      @param[out]      dom_ctx_id          Pointer to the Domain context

      @dependencies
      Need to have called playready_initialize() before calling this function.

      @sideeffects
      None
    */
    long playready_domaincert_initenum
    (
        uint32          app_ctx_id,
        uint32         *dom_ctx_id
    );

    /**
      Retrieves the next element in a domain certificate enumeration context.

      @return
      Refer to Microsoft Playready Porting Kit for the error codes.

      @param[in]         dom_ctx_id           Domain context
      @param[out]       *pcchDomainCert       Contains the size of the enumerated domain certificate
      @param[out]       *poDomainCertInfo     Pointer to the domain certificate info

      @dependencies
      Need to have called playready_domaincert_initenum() before calling this function.

      @sideeffects
      None
    */
    long playready_domaincert_enumnext
    (
        uint32                   dom_ctx_id,
        uint32                  *pcchDomainCert,
        tzdrmdomaincertinfo_t   *poDomainCertInfo
    );

    /**
      Initialize the state data for PlayReady cocktail DRM Stream Decryption.

      @return
      Refer to Microsoft Playready Porting Kit for the error codes.

      @param[in]       dec_ctx                     Decrypt context
      @param[in]       in_addr                     Array to the last 15 bytes of the encrypted payload.
      @param[in]       in_len                      Byte count of the playload to be decrypted

      @dependencies
      Need to have called playready_set_header() before calling this function.
      Need to call this function before calling playready_reader_decrypt()/playready_reader_decrypt_audio() for cocktail decryption

      @sideeffects
      None
    */
    long playready_reader_initdecrypt
    (
        uint32                          dec_ctx,
        uint8                           in_addr[15],
        uint32                          in_len
    );

    /**
      Removes the header information stored in Application context structure.

      @return
      Refer to Microsoft Playready Porting Kit for the error codes.

      @param[in]     app_ctx_id           Application context
      @param[in]     dec_ctx_id           Decrypt context
      @param[in]     dom_ctx_id           Domain context

      @dependencies
      Need to have called playready_initialize() before calling this function.

      @sideeffects
      None
    */
    long playready_reinitialize
    (
        uint32               app_ctx_id,
        uint32               dec_ctx_id,
        uint32               dom_ctx_id
    );

    /**
      Retrieves the data element from the server response.

      @return
      Refer to Microsoft Playready Porting Kit for the error codes.

      @param[in]     app_ctx_id              Application context
      @param[in]    *pbResponse              Pointer to server response.
      @param[in]     cbResponse              Size of server response.
      @param[in]     dwDataType              Data element type.
      @param[out]  *pchDataString          Pointer to the data string
      @param[in,out]  *pcchDataString          Size of the data string

      @dependencies
      Need to have called playready_licacq_generate_challenge() before calling this function.

      @sideeffects
      None
    */
    long playready_getadditional_responsedata
    (
        uint32               app_ctx_id,
        uint8               *pbResponse,
        uint32               cbResponse,
        uint32               dwDataType,
        uint8               *pchDataString,
        uint32              *pcchDataString
    );

    /********************************************************************************
      Non-silent URL parsing enable or disable on TZ.

      @return
      E_SUCCESS   - Success.
      E_FAILED    - Failure.

      @param[in]    bParsenonsilenturl - 0/1- off/on the parsing.

      @dependencies
      To be used only for envelope testing using the demo apk

      @sideeffects
      None
    ********************************************************************************/

    long playready_nonsilenturl_parsing_toggle
    (
        uint8             bParsenonsilenturl
    );

    /********************************************************************************
      Reprovision PlayReady keys using sfs.

      @return
      DRM_SUCCESS  - Success.
      DRM_S_FALSE  - Failure.

      @param[in]      key        Type of key to provision.
      @param[in]     *data       Data of the key.
      @param[in]      size       Size of the key.

      @dependencies
      None

      @sideeffects
      None
    ********************************************************************************/
    long playready_reprov_keys
    (
        tz_pr_keys_type key,
        uint8 * data,
        uint32 size
    );

    /********************************************************************************
      Provision PlayReady keys using sfs.

      @return
      E_SUCCESS   - Success.
      E_FAILED    - Failure.

      @param[in]	  key        Type of key to provision.
      @param[in]	 *data       Data of the key.
      @param[in]	  size       Size of the key.

      @dependencies
      None

      @sideeffects
      None
    ********************************************************************************/
    long playready_save_aes_keyfile
    (
        tz_pr_keys_type key,
        uint8 * data,
        uint32 size
    );

    /********************************************************************************
      playready_duplicate_ctx

      @return
      E_SUCCESS   - Success.
      E_FAILED    - Failure.

      @param[in]      ctx_id       app context id gotten for drm server
      @param[in]     *new_ctx_id   created a duplicated app context id

      @dependencies
      None

      @sideeffects
      None
    ********************************************************************************/
    long playready_duplicate_ctx
    (
        uint32  ctx_id,
        uint32 *new_ctx_id
    );

    /********************************************************************************
      playready_content_setproperty

      @return
      E_SUCCESS   - Success.
      E_FAILED    - Failure.

      @param[in]      app_ctx_id         DRM_APP_CONTEXT structure with ID
      @param[in]      set_property_type  DRM_CONTENT_SET_PROPERTY type, the property to set
      @param[in]    * data               Property specific data
      @param[in]      size               Size of data

      @dependencies
      Need to have called playready_initialize() before calling this function.

      @sideeffects
      None
    ********************************************************************************/
    long playready_content_setproperty
    (
        uint32        app_ctx_id,
        uint32        set_property_type,
        const uint8 * data,
        uint32        size
    );

    /********************************************************************************
      IV constraint check enable or disable on TZ.

      @return
      E_SUCCESS   - Success.
      E_FAILED    - Failure.

      @param[in]     app_ctx_id           Application context
      @param[in]   bIVconstraint - 0/1- off/on the check.

      @dependencies
      To be used only for internal playready unit testing.

      @sideeffects
      None
    ********************************************************************************/
    long playready_IVcheck_toggle
    (
        uint32            app_ctx_id,
        uint8             bIVconstraint
    );

/**
  This API allocates a non-secure ION buffer.

  @return
  PLAYREADY_COPY_SUCCESS                 - Success.
  PLAYREADY_COPY_ERROR_ION_MALLOC_FAILED - Failure.

  @param[out]      ion_vir_addr pointer to ion virtual address
  @param[out]      ion_fd       pre-alloc ion memory return value
  @param[in]       ion_fd_data  ion data
  @param[in]       memsize      size of space to malloc

  @dependencies
  None.

  @sideeffects
  None.
*/
PlayreadyCopyResult PlayreadyCopy_ION_Malloc
(
  uint8**             ion_vir_addr,
  int*                ion_fd,
  struct ion_fd_data* ion_fd_data,
  uint32              memsize
);

/**
  This API frees a ION buffer.

  @return
  PLAYREADY_COPY_SUCCESS                 - Success.
  PLAYREADY_COPY_ERROR_ION_FREE_FAILED   - Failure.

  @param[in]       ion_vir_addr pointer to memory to free
  @param[in]       ion_fd       pre-alloc ion memory return value
  @param[in]       ion_fd_data  ion data
  @param[in]       memsize      size of space to free

  @dependencies
  None.

  @sideeffects
  None.
*/
PlayreadyCopyResult PlayreadyCopy_ION_Free
(
  uint8**             ion_vir_addr,
  int*                ion_fd,
  struct ion_fd_data* ion_fd_data,
  uint32              memsize
);

/**
  This API copies (AES-128-CBC)the content in the non-secure buffer, referenced by
  the *nonSecBuffer parameter, into the secure buffer, referenced
  by the secBufferHandle.

  @return
  PLAYREADY_COPY_SUCCESS                  - Success.
  PLAYREADY_COPY_ERROR_COPY_FAILED    - Failure.

  @param[in]       app_ctx_id             app context id
  @param[in/out]   nonSecBuffer           non-secure buffer
  @param[in]       nonSecBufferLength     number of bytes in the non-secure buffer
  @param[in]       secBufferHandle        reference to the secure buffer
  @param[in]       secBufferOffset        offset from the beginning of the secure buffer which the
                                          data will be written
  @param[out]      secBufferLength        number of bytes written into the secure buffer
  @param[in]       CopyDir                Specifies whether to copy from secure to non-secure buffer
                                          (or) from non-secure to secure buffer. Only non-secure to
                                          secure direction is supported here.

  @dependencies
  This function must only be invoked after using playready_initialize() and should not terminate
  until playready_uninitialize() has been called.

  @sideeffects
  Should be used only for internal testing purpose.
*/
PlayreadyCopyResult Content_Protection_Copy
(
  uint32                    app_ctx_id,
  uint8*                    nonSecBuffer,
  const uint32              nonSecBufferLength,
  uint32                    secBufferHandle,
  uint32                    secBufferOffset,
  uint32*                   secBufferLength,
  PlayreadyCopyDir          CopyDir
);

#ifdef __cplusplus
}
#endif // __cplusplus
#endif // PR_CLNT_H
