#ifndef RMP_H
#define RMP_H

/** @file RMP.h
 * @brief
 * This file contains the definitions of the constants, data structures
 * and interfaces to the RMP Module
 */

/*===========================================================================
  Copyright (c) 2012-2013 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
===========================================================================*/

/*===========================================================================

					EDIT HISTORY FOR FILE

when       who     what, where, why
--------   ---     ----------------------------------------------------------
11/20/12   mg      Initial Version.

===========================================================================*/

/*----------------------------------------------------------------------------
 *Type Declarations
 *--------------------------------------------------------------------------*/

#include "comdef.h"

/**Error types */
typedef enum {
	RMP_SUCCESS = 0,
	RMP_ERR_FAILURE = -100,
	RMP_ERR_SFS,
	RMP_ERR_CIPHER,
	RMP_ERR_INIT_FAILED,
	RMP_ERR_TERM_FAILED,
	RMP_ERR_INVALID_PTR,
	RMP_ERR_NOT_PROVISIONED,
	RMP_ERR_PROV_CHALLENGE_FAILED,
	RMP_ERR_INVALID_RMP_HANDLE,
	RMP_ERR_ALLOC_STN_FAILED,
	RMP_ERR_FREE_STN_FAILED,
	RMP_ERR_INVALID_STN_HANDLE,
	RMP_ERR_ALLOC_KL_FAILED,
	RMP_ERR_FREE_KL_FAILED,
	RMP_ERR_INVALID_KL_INDEX,
	RMP_ERR_INVALID_DEVICE_ID,
	RMP_ERR_INVALID_EMM,
	RMP_ERR_PROCESS_EMM_FAILED,
	RMP_ERR_INVALID_EMM_PROTOCOL_NUM,
	RMP_ERR_INVALID_EMM_UPDATE_NUM,
	RMP_ERR_INVALID_EMM_DESC_TAG,
	RMP_ERR_EMM_MANIP_CHECK_FAILED,
	RMP_ERR_EMM_INVALID_DEV_ID_ID,
	RMP_ERR_INVALID_ECM_PROTOCOL_NUM,
	RMP_ERR_WORK_KEY_NOT_CONFIGED,
	RMP_ERR_INVALID_ECM,
	RMP_ERR_PROCESS_ECM_FAILED,
	RMP_ERR_ENCRYPT_MSG_FAILED,
	RMP_ERR_DECRYPT_MSG_FAILED,
	RMP_ERR_INVALID_VERSION,
	RMP_ERR_SIZE = 0x7FFFFFFF
} rmp_status_t;

/**Device ID union */
typedef union rmp_device_id_u {
	uint64 device_id_u;
	uint8 device_id_a[8];
} rmp_device_id_t;

/**Station specific context identity */
typedef struct rmp_station_ctx_s {
	uint32 stn_handle;
	uint16 stn_id;
} rmp_station_ctx_t;

/**Provision data */
typedef struct rmp_provision_data_s {
	rmp_device_id_t orig_device_id;
	uint8 orig_device_key[16];
	uint8 emm_md_kmac[16];
	uint8 cbc_iv[4][16];
	uint8 multi2_system_key[32];
	uint8 multi2_cbc_iv[8];
	uint8 oem_key[16];
	uint8 oem_specific_param[11];
	uint8 padding[5];
} rmp_provision_data_t;

/**Provision keys */
typedef struct rmp_provision_keys_s {
	uint8 cek[16];
	uint8 ceiv[16];
} rmp_provision_keys_t;

/**EMM Descriptor type */
typedef enum {
	EMM_DEVICE_KEY_UPDATE,
	EMM_WORK_KEY_SETUP,
	EMM_DUMMY,
	EMM_INVALID = 0x7FFFFFFF
} rmp_emm_type_t;

/** Device key validation parameters */
typedef struct rmp_device_key_params_s {
  uint8 oem_key[16];
  uint8 oem_specific_param[11];
  uint8 dev_key_update_param[4];
  uint8 generation_num;
} rmp_device_key_params_t;

#define PLAYBACK_STN_ID 0xFFFF

/*----------------------------------------------------------------------------
 *Function Declarations and Documentation
 *--------------------------------------------------------------------------*/
#ifdef __cplusplus
extern "C" {
#endif
/**
  @brief Initialize the RMP Module.
  - The function loads the RMP module into TrustZone and initializes it with
    the provisioned TRMP data stored in SFS. The first time the function is
    called, it will create an SFS file for storing the station-specific
    contexts. If the station context file exists, its contents are loaded
    into the RMP module’s memory.

  @return
  0        - Success.
  non 0    - Failure.
  @dependencies
  Must be called prior to invoking any of the other RMP APIs.

  @sideeffects
  None.
*/
rmp_status_t rmp_initialize(void);

/**
  @brief Terminate the RMP Module
  - The function releases all resources associated with the RMP Module and
    terminates the operation of the RMP module in TrustZone.

  @return
  0        - Success.
  non 0    - Failure.

  @dependencies
  None.

  @sideeffects
  None.
*/
rmp_status_t rmp_terminate(void);

/**
  @Brief Get maximum number of station contexts
  - The function returns the maximum number of station contexts that can be
    allocated in the RMP module. This number is also the maximum number of
    station contexts that will be saved in the SFS file.
  @return
  maximum number of station contexts

  @dependencies
  None.

  @sideeffects
  None.
*/
uint8 rmp_get_max_num_station_ctx(void);

/**
  @brief Allocate Station Specific Context
  - The function allocates a context for a Broadcasting Station identified
    by its Station ID (Network ID or TS ID). For playback, a context should
    be created using a Station ID of PLAYBACK_STN_ID. Whenever the data in
    the station-specific context is updated, the contents of the context are
    saved in an SFS file. The contents of this file are loaded into the RMP
    module when the RMP module is initialized. If, when calling this function,
    a context is found with the same station ID as that provided by the
    function, the function returns a handle to the found context. Otherwise,
    the function returns a handle to the first free station-specific context
    found.

  @param[in]  stn_id     - The ID of the specific Broadcasting Station or
                           PLAYBACK_STN_ID for playback.
  @param[out] stn_handle - Returned value of the handle to the context.

  @return
  0        - Success.
  non 0    - Failure.

  @dependencies
  None.

  @sideeffects
  None.
*/
rmp_status_t rmp_allocate_station_ctx(uint16 stn_id, uint32 *stn_handle);

/**
  @brief Get number of existing station specific contexts
  - The function returns the number of station specific contexts that have
    been previously allocated and have not been deleted from the RMP module.

  @return
  Number of existing station specific contexts.
*/
uint8 rmp_get_num_station_ctx(void);

/**
  @brief Get Station Specific Contexts
  - The function returns the station handles and Station IDs of all the
    existing Station Specific contexts.

  @param[in,out] stn_ctx_array - A pre-allocated array of rmp_stn_ctx_t
                                 elements.
  @param[in]     num_stations  - The number of elements in the array.

  @return
  0        - Success.
  non 0    - Failure.

  @dependencies
  None.

  @sideeffects
  None.
*/
rmp_status_t rmp_get_station_ctx(rmp_station_ctx_t *stn_ctx_array,
		uint8 num_stations);

/**
  @brief Free Station Specific Context
  - The function clears the contents of the required station-specific
    contexts. If the stn_id value of an element in the array is not
    PLAYBACK_STN_ID, the context will be removed from the station-specific
    context file. If the stn_id value of an element in the array is not
    PLAYBACK_STN_ID and the stn_handle value of an element in the array is set
    to 0xFFFFFFFF, the function will search for the station context using
    its stn_id.

  @param[in] stn_ctx_array - An array containing the list of station
                             contexts to be cleared.
  @param[in] num_stations  - The number of elements in the array

  @return
  0        - Success.
  non 0    - Failure.

  @dependencies
  None.

  @sideeffects
  None.
*/
rmp_status_t rmp_free_station_ctx(rmp_station_ctx_t *stn_ctx_array,
		uint8 num_stations);

/**
  @brief Allocate Key Ladder Index
  - The function allocates a cipher key entry in the crypto engine used by
    the Secure Demux to decrypt the Transport Stream.

  @param[in]  stn_handle - Handle to the Station Specific context.
  @param[out] kl_index   - Returned value of the key ladder index.

  @return
  0        - Success.
  non 0    - Failure.

  @dependencies
  None.

  @sideeffects
  None.
*/
rmp_status_t rmp_allocate_kl_index(uint32 stn_handle, uint32 *kl_index);

/**
  @brief Delete the station specific data file
  - The function deletes the sfs file containing the persistent station
    data.
    NOTE: The function can only run when the RMP module is not initialized.

  @return
  0        - Success.
  non 0    - Failure.

  @dependencies
  None.

  @sideeffects
  None.
*/
rmp_status_t rmp_delete_stn_file(void);

/**
  @brief Free Key Ladder Index
  - The function releases the allocated cipher key entry.

  @param[in] kl_index   - Index to the Key Ladder entry.

  @return
  0        - Success.
  non 0    - Failure.

  @dependencies
  None.

  @sideeffects
  None.
*/
rmp_status_t rmp_free_kl_index(uint32 kl_index);

/**
  @brief Get Original Device Id
  - The function returns the original Device Id of the RMP Module.
    The device ID is returned in big-endian byte order; the most significant
    byte of the device ID will be returned in the first byte of the containing
    array, and the least significant byte will be returned in the last byte of
    the containing array. The rational behind returning the device ID as a
    union is to enable efficient comparison of device IDs.

  @param[out] device_id  - Returned value of the original Device Id.

  @return
  0        - Success.
  non 0    - Failure.

  @dependencies
  None.

  @sideeffects
  None.
*/
rmp_status_t rmp_get_original_device_id(rmp_device_id_t *device_id);

/**
  @brief Get Updated Device Id
  - The function returns the updated device ID of the specified broadcast
    station. The device ID is returned in big-endian byte order; the most
    significant byte of the device ID will be returned in the first byte of
    the containing array, and the least significant byte will be returned in
    the last byte of the containing array. The rational behind returning the
    device ID as a union is to enable efficient comparison of device IDs.


  @param[in]  stn_handle - The handle to the station context.
  @param[out] device_id  - Returned value of the updated Device Id.

  @return
  0        - Success.
  non 0    - Failure.

  @dependencies
  None.

  @sideeffects
  None.
*/
rmp_status_t rmp_get_updated_device_id(uint32 stn_handle,
		rmp_device_id_t *device_id);

/**
  @brief Process EMM Body
  - The function passes a single pre-filtered EMM body (containing a device ID
    matching either the original device ID or the updated device ID) to the
    RMP module for processing. The decrypted cipher data from the EMM
    descriptors is stored in secure memory. When receiving an F1 Work Setup
    descriptor, the station context and the station context file will be
    updated with the new Kw information. The function will return the value of
    the updated device ID after processing the EMM.


  @param[in]  stn_handle - The handle to the station context.
  @param[in]  emm_body   - Pointer to an EMM body.
  @param[out] emm_type   - Returned value of the EMM descriptor type.
  @param[out] device_id  - Returned value of the updated Device Id.

  @return
  0        - Success.
  non 0    - Failure.

  @dependencies
  None.

  @sideeffects
  None.
*/
rmp_status_t rmp_process_emm(uint32 stn_handle, void *emm_body,
		rmp_emm_type_t *emm_type, rmp_device_id_t *device_id);

/**
  @brief Set Device key.
  - The function delivers a new updated device key directly to a
    specified station context.

  @param[in] stn_handle     - The handle to the station context.
  @param[in] update_num     - Update number.
  @param[in] generation_num - Generation number.
  @param[in] device_key     - 16 byte Updated Device key.

  @return
  0        - Success.
  non 0    - Failure.

  @dependencies
  None.

  @sideeffects
  None.
*/
rmp_status_t rmp_set_device_key(uint32 stn_handle, uint16 update_num,
		uint8 generation_num, uint8 *device_key);

/**
  @brief Process ECM-F1 Body
  - The function passes a single ECM-F1 body to the RMP module for
    processing. The decrypted Ks keys are stored in the crypto engine key
    ladder entry at the supplied key ladder index.

  @param[in]  kl_index - Index to the Key Ladder entry.
  @param[in]  ecm_body - Pointer to an ECM-F1 body.

  @return
  0        - Success.
  non 0    - Failure.

  @dependencies
  None.

  @sideeffects
  None.
*/
rmp_status_t rmp_process_ecm(uint32 kl_index, void *ecm_body);

/**
  @brief Provision data in the clear
  - The function delivers the provision data to the RMP module in the clear.
    The data is stored in an SFS file in the /persist directory tree.

  @param[in] rmp_data - pointer to the provision data.

  @return
  0        - Success.
  non 0    - Failure.

  @dependencies
  None.

  @sideeffects
  None.
*/
rmp_status_t rmp_provision_unencrypted(rmp_provision_data_t *rmp_data);

/**
  @brief Provision content encryption key
  - The function delivers the CEK and the CEIV to the RMP module.
    The keys are stored in a temporary SFS file.

  @param[in] cek  - 16 byte array containing the CEK.
  @param[in] ceiv - 16 byte array containing the CEIV.

  @return
  0        - Success.
  non 0    - Failure.

  @dependencies
  None.

  @sideeffects
  None.
*/
rmp_status_t rmp_provision_cek(uint8 *cek, uint8 *ceiv);

/**
  @brief Provision encrypted data
  - The function delivers encrypted provision data to the RMP module.
    The data is stored in a temporary SFS file. The complete data structure
    should be encrypted using a secret encryption algorithm together with the
    CEK and the CEIV.

  @param[in] rmp_data - pointer to the encrypted provision data.

  @return
  0        - Success.
  non 0    - Failure.

  @dependencies
  None.

  @sideeffects
  None.
*/
rmp_status_t rmp_provision_encrypted(rmp_provision_data_t *rmp_data);

/**
  @brief Finalize provisioning of encrypted data
  - The function finalizes the provisioning process by decrypting the
    provision data using the secret encryption algorithm together with
    the supplied CEK and the CEIV. The provision data is stored in the SFS
    file in the /persist directory tree, and the temporary files are deleted.

  @return
  0        - Success.
  non 0    - Failure.

  @dependencies
  None.

  @sideeffects
  None.
*/
rmp_status_t rmp_provision_finalize(void);

/**
  @brief Provision data integrity check
  - The function receives a 16 byte MAC generated by applying a MAC algorithm
    on the clear text provision data (including the padding field).
    The RMP module generates its own version of the MAC using  the same
    algorithm and compares it to the received MAC.

  @param[in] prov_mac - 16 byte MAC.

  @return
  0        - Success.
  non 0    - Failure.

  @dependencies
  None.

  @sideeffects
  None.
*/
rmp_status_t rmp_provision_challenge(uint8 *prov_mac);

/**
  @brief Delete the provision data
  - The function deletes the provision data file from the /persist
    directory tree.

  @return
  0        - Success.
  non 0    - Failure.

  @dependencies
  None.

  @sideeffects
  None.
*/
rmp_status_t rmp_provision_delete(void);

/**
  @brief Validate generated device key
  - The function receives a device key generated by the rmpgen tool
    and sends the key, together with the values used to generate it,
    to be validated by the RMP module. The RMP module generates the
    device key and returns the result of the comparison.

  @param[in] device_key - 16 byte device key.
  @param[in] d_k_params - structure containing the parameters used
                          to generate the device key.

  @return
  0        - Success.
  non 0    - Failure.

  @dependencies
  None.

  @sideeffects
  None.
*/
rmp_status_t rmp_validate_device_key(uint8* device_key,
	rmp_device_key_params_t* d_k_params);

#ifdef __cplusplus
}
#endif

#endif /*RMP_H */
