/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*

			HLOS QSEEComAPI Library Functions

GENERAL DESCRIPTION
  Contains the library functions for accessing QSEECom driver.

EXTERNALIZED FUNCTIONS
  None

INITIALIZATION AND SEQUENCING REQUIREMENTS

Copyright (c) 2010-2014 Qualcomm Technologies, Inc.
All Rights Reserved.
Qualcomm Technologies Confidential and Proprietary

*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/
#ifndef __QSEECOMAPI_H_
#define __QSEECOMAPI_H_


/*----------------------------------------------------------------------------
 * Include Files
* -------------------------------------------------------------------------*/
#include <stdint.h>
#include <stdbool.h>

#define QSEECOM_ALIGN_SIZE	0x40
#define QSEECOM_ALIGN_MASK	(QSEECOM_ALIGN_SIZE - 1)
#define QSEECOM_ALIGN(x)	\
	((x + QSEECOM_ALIGN_SIZE) & (~QSEECOM_ALIGN_MASK))

#ifdef __cplusplus
extern "C" {
#endif

/*----------------------------------------------------------------------------
 * Preprocessor Definitions and Constants
 * -------------------------------------------------------------------------*/
/** The memory is locked and non-pageable */
#define MEM_LOCKED         0x00000001
/** The memory is marked non-cacheable */
#define MEM_NON_CACHED     0x00000002

#define QSEECOM_APP_CLEAR_KEY_FAILED		-11
#define QSEECOM_APP_KMS_MAX_FAILURE		-10
#define QSEECOM_APP_UPDATE_KEY_USERINFO_FAILED	-9
#define QSEECOM_APP_WIPE_KEY_FAILED		-8
#define QSEECOM_APP_CREATE_KEY_FAILED		-7
#define QSEECOM_APP_QUERY_FAILED		-6
#define QSEECOM_APP_NOT_LOADED			-5
#define QSEECOM_APP_ALREADY_LOADED		-4
#define QSEECOM_LISTENER_UNREGISTERED		-3
#define QSEECOM_LISTENER_ALREADY_REGISTERED	-2
#define QSEECOM_LISTENER_REGISTER_FAIL		-1
/* Required for Non Contiguous Ion Memory support */
#define QSEECOM_MAX_SG_ENTRY 512

/*----------------------------------------------------------------------------
 * Type Declarations
 * -------------------------------------------------------------------------*/
struct QSEECom_handle {
	unsigned char *ion_sbuffer;
};

struct QSEECom_ion_fd_data {
	int32_t fd;
	uint32_t cmd_buf_offset;
};

struct QSEECom_ion_fd_info {
	struct QSEECom_ion_fd_data data[4];
};

struct QSEECom_scattered_buf {
	uint32_t phys_addr;
	uint32_t len;
};

struct qseecom_rpmb_provision_key {
	uint32_t key_type;
};

enum QSEECom_command_id	{
  QSEECOM_RPMB_RESERVED_COMMAND = 0x0E,
  QSEECOM_RPMB_PROVISION_KEY_COMMAND = 0x0F,/* mapping exists in qseecomi.h, do not change*/
  QSEECOM_RPMB_ERASE_COMMAND = 0x10,/* mapping exists in qseecomi.h, do not change*/
  QSEECOM_FSM_LTE_INIT_DB = 0x100,
  QSEECOM_FSM_LTE_STORE_KENB = 0x101,
  QSEECOM_FSM_LTE_GEN_KEYS = 0x102,
  QSEECOM_FSM_LTE_GET_KEY_OFFSETS = 0x103,
  QSEECOM_FSM_LTE_GEN_KENB_STARS = 0x104,
  QSEECOM_FSM_LTE_GET_KENB_STARS = 0x105,
  QSEECOM_FSM_LTE_STORE_NH = 0x106,
  QSEECOM_FSM_LTE_DELETE_NH = 0x107,
  QSEECOM_FSM_LTE_DELETE_KEYS = 0x108,
  QSEECOM_FSM_IKE_CMD_SIGN = 0x200,
  QSEECOM_FSM_IKE_CMD_PROV_KEY = 0x201,
  QSEECOM_FSM_IKE_CMD_ENCRYPT_PRIVATE_KEY = 0x202,
  QSEECOM_FSM_OEM_FUSE_WRITE_ROW = 0x301,
  QSEECOM_FSM_OEM_FUSE_READ_ROW = 0x302,
  QSEECOM_RPMB_MAX_COMMAND = 0xEFFFFFFF
};

enum QSEECom_key_management_usage_type {
		QSEECOM_KM_USAGE_DISK_ENCRYPTION = 0x01,
		QSEECOM_KM_USAGE_FILE_ENCRYPTION = 0x02,
		QSEECOM_KM_USAGE_MAX
};

enum QSEECom_bandwidth_request_mode {
        QSEECOM_CE_INACTIVE = 0,
        QSEECOM_CE_LOW,
        QSEECOM_CE_MEDIUM,
        QSEECOM_CE_HIGH,
};

struct QSEECom_scattered_entries {
	struct QSEECom_scattered_buf  sg_list[QSEECOM_MAX_SG_ENTRY];
};
/*----------------------------------------------------------------------------
 * Function Declarations and Documentation
 * -------------------------------------------------------------------------*/
/**
 * @brief Open a handle to the  QSEECom device.
 *
 * - Load a secure application. The application will be verified that it is
 *    secure by digital signature verification.
 * Allocate memory for sending requests to the QSAPP
 *
 * Note/Comments:
 * There is a one-to-one relation for a HLOS client and a QSAPP;
 * meaning that only one app can communicate to a QSAPP at a time.
 *
 * Please note that there is difference between an application and a listener
 * service. A QSAPP must be loaded at the request of the HLOS,
 * and all requests are orginated by the HLOS client.
 * A listener service on the otherhand is started during start-up by a
 * daemon, qseecomd.
 *
 * A HLOS application may create mutiple handles to the QSAPP
 *
 * @param[in/out] handle The device handle
 * @param[in] fname The directory and filename to load.
 * @param[in] sb_size Size of the shared buffer memory  for sending requests.
 * @return Zero on success, negative on failure. errno will be set on
 *  error.
 */
int QSEECom_start_app(struct QSEECom_handle **clnt_handle, const char *path,
			const char *fname, uint32_t sb_size);

/**
 * @brief Close the application associated with the handle.
 *
 * - Unload a secure application. The driver will verify if there exists
 *   any other applications that are communicating with the QSAPP to which
 *   the "handle" is tied.
 * - De-allocate memory for sending requests to QSAPP.
 *
 * @param[in] handle The device handle
 * @return Zero on success, negative on failure. errno will be set on
 *  error.
 */
int QSEECom_shutdown_app(struct QSEECom_handle **handle);

/**
 * @brief Open a handle to the  QSEECom device.
 *
 * - Load an external elf. The elf will be verified that it is
 *    secure by digital signature verification.
 *
 * A HLOS application may create mutiple opens (only one is permitted for the
 * app, but each listener service can open a unique device in the same HLOS app
 * /executable.
 * @param[in/out] handle The device handle
 * @param[in] fname The directory and filename to load.
 * @return Zero on success, negative on failure. errno will be set on
 *  error.
 */
int QSEECom_load_external_elf(struct QSEECom_handle **clnt_handle, const char *path,
			const char *fname);

/**
 * @brief Close the external elf
 *
 * - Unload an external elf.
 *
 * @param[in] handle The device handle
 *
 * @return Zero on success, negative on failure. errno will be set on
 *  error.
 */
int QSEECom_unload_external_elf(struct QSEECom_handle **handle);

/**
 * @brief Register an HLOS listener service. This allows messages from QSAPP
 * to be received.
 *
 * @param[in] handle The device handle
 * @param[in] lstnr_id The listener service identifier. This ID must be uniquely
 * assigned to avoid any collisions.
 * @param[in] sb_length Shared memory buffer between OS and QSE.
 * @param[in] flags Provide the shared memory flags attributes.
 *
 * @return Zero on success, negative on failure. errno will be set on
 *  error.
 *
 */
int QSEECom_register_listener(struct QSEECom_handle **handle,
			uint32_t lstnr_id, uint32_t sb_length, uint32_t flags);

/**
 * @brief Unregister a listener service.
 *
 * @param[in] handle The device handle
 *
 * @return Zero on success, negative on failure. errno will be set on
 *  error.
 */
int QSEECom_unregister_listener(struct QSEECom_handle *handle);


/**
 * @brief Send QSAPP a "user" defined buffer (may contain some message/
 * command request) and receives a response from QSAPP in receive buffer.
 * The HLOS client writes to the send_buf, where QSAPP writes to the rcv_buf.
 * This is a blocking call.
 *
 * @param[in] handle    The device handle
 * @param[in] send_buf  The buffer to be sent.
 *                      If using ion_sbuffer, ensure this
 *                      QSEECOM_BUFFER_ALIGN'ed.
 * @param[in] sbuf_len  The send buffer length
 *                      If using ion_sbuffer, ensure length is
 *                      multiple of QSEECOM_BUFFER_ALIGN.
 * @param[in] rcv_buf   The QSEOS returned buffer.
 *                      If using ion_sbuffer, ensure this is
 *                      QSEECOM_BUFFER_ALIGN'ed.
 * @param[in] rbuf_len  The returned buffer length.
 *                      If using ion_sbuffer, ensure length is
 *                      multiple of QSEECOM_BUFFER_ALIGN.
 * @param[in] rbuf_len  The returned buffer length.
 *
 * @return Zero on success, negative on failure. errno will be set on
 *  error.
 */
int QSEECom_send_cmd(struct QSEECom_handle *handle, void *send_buf,
			uint32_t sbuf_len, void *rcv_buf, uint32_t rbuf_len);


/**
 * @brief Send QSAPP a "user" defined buffer (may contain some message/
 * command request) and receives a response from QSAPP in receive buffer.
 * This API is same as send_cmd except it takes in addition parameter,
 * "ifd_data".  This "ifd_data" holds information (ion fd handle and
 * cmd_buf_offset) used for modifying data in the message in send_buf
 * at an offset.  Essentailly, it has the ion fd handle information to
 * retrieve physical address and modify the message in send_buf at the
 * mentioned offset.
 *
 * The HLOS client writes to the send_buf, where QSAPP writes to the rcv_buf.
 * This is a blocking call.
 *
 * @param[in] handle    The device handle
 * @param[in] send_buf  The buffer to be sent.
 *                      If using ion_sbuffer, ensure this
 *                      QSEECOM_BUFFER_ALIGN'ed.
 * @param[in] sbuf_len  The send buffer length
 *                      If using ion_sbuffer, ensure length is
 *                      multiple of QSEECOM_BUFFER_ALIGN.
 * @param[in] rcv_buf   The QSEOS returned buffer.
 *                      If using ion_sbuffer, ensure this is
 *                      QSEECOM_BUFFER_ALIGN'ed.
 * @param[in] rbuf_len  The returned buffer length.
 *                      If using ion_sbuffer, ensure length is
 *                      multiple of QSEECOM_BUFFER_ALIGN.
 * @param[in] QSEECom_ion_fd_info  data related to memory allocated by ion.
 *
 * @return Zero on success, negative on failure. errno will be set on
 *  error.
 */
int QSEECom_send_modified_cmd(struct QSEECom_handle *handle, void *send_buf,
			uint32_t sbuf_len, void *resp_buf, uint32_t rbuf_len,
			struct QSEECom_ion_fd_info  *ifd_data);

/**
 * @brief Receive a service defined buffer.
 *
 * @param[in] handle    The device handle
 * @param[out] buf      The buffer that is received
 * @param[in] len       The receive buffer length
 *
 * @return Zero on success, negative on failure. errno will be set on
 *  error.
 */
int QSEECom_receive_req(struct QSEECom_handle *handle,
			void *buf, uint32_t len);

/**
 * @brief Send a response based on the previous QSEECom_receive_req.
 *
 * This allows a listener service to receive a command (e.g. read file abc).
 * The service can then handle the request from QSEECom_receive_req, and provide
 * that information back to QSAPP.
 *
 * This allows the HLOS to act as the server and QSAPP to behave as the client.
 *
 * @param[in] handle    The device handle
 * @param[out] send_buf  The buffer to be returned back to QSAPP
 * @param[in] len       The send buffer length
 *
 * @return Zero on success, negative on failure. errno will be set on
 *  error.
 */
int QSEECom_send_resp(struct QSEECom_handle *handle,
			void *send_buf, uint32_t len);

/**
 * @brief Set the bandwidth for QSEE.
 *
 * This API resulst in improving the performance on the Crypto hardware
 * in QSEE. It should be called before issuing send_cmd/send_modified_cmd
 * for commands that requires using the crypto hardware on the QSEE.
 * Typically this API should be called before issuing the send request to
 * enable high performance mode and after completion of the send_cmd to
 * resume to low performance and hence to low power mode.
 *
 * This allows the clients of QSEECom to set the QSEE cyptpo HW bus
 * bandwidth to high/low.
 *
 * @param[in] high    Set to 1 to enable bandwidth.
 *
 * @return Zero on success, negative on failure. errno will be set on
 *  error.
 */
int QSEECom_set_bandwidth(struct QSEECom_handle *handle, bool high);

/**
 * @brief Query QSEE to check if app is loaded.
 *
 * This API queries QSEE to see if the app is loaded or not.
 *
 * @param[in] app_name  Name of the app.
 *
 * @return QSEECOM_APP_QUERY_FAILED/QSEECOM_APP_NOT_LOADED/QSEECOM_APP_LOADED.
 */
int QSEECom_app_load_query(struct QSEECom_handle *handle, char *app_name);

/*
* @brief Send a "user" defined buffer (may contain some message/
* command request) and receives a response in resp buffer.
* The HLOS client writes to the send_buf, whereas rsp_buf is filled by TZ.
* This is a blocking call.
*
* @param[in] send_buf : Pointer to the command buffer (populated by client) to be sent.
*                       NOTE: This buffer contains the service related command structure.
*                       For RPMB:
*                       - struct type 'qseecom_rpmb_provision_key' is send_buf.
* @param[in] sbuf_len : Sizeof 'send_buf' (sent above).
* @param[in] resp_buf : Pointer to the reponse buffer (to be populated by TZ).
*                       Note: This buffer contains service related command structures.
*                       For RPMB
*                        - Does not need any separate response, so this value is NULL.
* @param[in] rbuf_len : sizeof 'resp_buf' (sent above).
* @param[in] cmd_id   : This is service specific command id of type 'enum QSEECom_command_id'
*                       Can contain the following:
*                        - QSEECOM_RPMB_PROVISION_KEY_COMMAND
*                        - QSEECOM_RPMB_ERASE_COMMAND
* @return Zero on success, negative on failure. errno will be set on error.
*/

int QSEECom_send_service_cmd(void *send_buf, uint32_t sbuf_len, void *resp_buf,
		uint32_t rbuf_len, enum QSEECom_command_id cmd_id);

/**
 * @brief create key.
 *
 * This API create the key.
 *
 * @param[in] usage, to specify which user application will use this key management function.
 *
 * @param[in] a 32bit hash as password (optional).
 *
 * @return Zero on success, negative on failure. errno will be set on error.
 *
 **/

int QSEECom_create_key(enum QSEECom_key_management_usage_type usage, unsigned char *hash32);

/**
 * @brief wipe key.
 *
 * This API wipe the key.
 *
 * @param[in] usage, to specify which user application will use this key management function.
 *
 * @return Zero on success, negative on failure. errno will be set on error.
 *
 **/

int QSEECom_wipe_key(enum QSEECom_key_management_usage_type usage);

/**
 * @brief clear key.
 *
 * This API clear the key from PIPE.
 *
 * @param[in] usage, to specify which user application will use this key management function.
 *
 * @return Zero on success, negative on failure. errno will be set on error.
 *
 **/

int QSEECom_clear_key(enum QSEECom_key_management_usage_type usage);

/**
 * @brief update key user info.
 *
 * This API update the key user info.
 *
 * @param[in] usage, to specify which user application will use this key management function.
 *
 * @param[in] current_hash32, to specify the current password hash
 *
 * @param[in] new_hash32, to specify the new password hash.

 * @return Zero on success, negative on failure. errno will be set on error.
 *
 **/

int QSEECom_update_key_user_info(enum QSEECom_key_management_usage_type usage,
				unsigned char *current_hash32, unsigned char *new_hash32);

/**
 * @brief qseecom_send_modified_resp
 *
 * A "user" defined API that contains the Ion fd allocated by the
 * listener service, to be communicated with TZ. This API is same as
 * "QSEECom_send_modified_cmd" but servicing the listener instead of app
 * This "ifd_data" holds information (ion fd handle and cmd_buf_offset)
 * used for modifying data in the message in send_buf at an offset.
 * Essentailly, it has the ion fd handle information to retrieve physical
 * address and modify the message in send_buf at the mentioned offset.
 *
 * @param[in] handle,   The device handle
 * @param[in] send_buf, The buffer to be sent.
 *                      If using ion_sbuffer, ensure this
 *                      QSEECOM_BUFFER_ALIGN'ed.
 * @param[in] sbuf_len, The send buffer length
 *                      If using ion_sbuffer, ensure length is
 *                      multiple of QSEECOM_BUFFER_ALIGN.
 * @param[in] QSEECom_ion_fd_info, data related to memory allocated by ion.
 *
 * @return Zero on success, negative on failure. errno will be set on
 *  error.
 */
int QSEECom_send_modified_resp(struct QSEECom_handle *handle, void *send_buf,
                                uint32_t sbuf_len, struct QSEECom_ion_fd_info *ifd);

/**
 * @brief scale bus bandwidth.
 *
 * @param[in] handle    The device handle
 * @param[in] mode      INACVTIVE, LOW, MEDIUM or HIGH
 *
 * @return Zero on success, negative on failure. errno will be set on
 *  error.
 */
int QSEECom_scale_bus_bandwidth(struct QSEECom_handle *handle, int mode);

#ifdef __cplusplus
}
#endif

#endif
