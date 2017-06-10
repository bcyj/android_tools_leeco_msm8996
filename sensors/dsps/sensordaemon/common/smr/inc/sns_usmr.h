#ifndef SNS_USMR_H
#define SNS_USMR_H

/*============================================================================
  @file sns_usmr.h

  @brief

  Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ============================================================================*/

/*===========================================================================
                   Include Files
  ===========================================================================*/
#include "sns_common.h"
#include "qmi_idl_lib.h"
#include "qmi_csi.h"
#include "qmi_client.h"

/*============================================================================
                        Type Declarations
  ============================================================================*/

/* Priority of messages  */
typedef enum
{
  SNS_SMR_MSG_PRI_LOW = 0,
  SNS_SMR_MSG_PRI_MED = 10,
  SNS_SMR_MSG_PRI_HIGH = 20,
} sns_smr_msg_pri_e;

/*
 * Services during their QMI registration may choose to set an "Instance ID".
 * This ID is used by Sensors clients as "priority" field; in case multiple
 * services exist, the one with the largest instance ID will be used.  In most
 * cases, services registered on the ADSP are done so with higher priority.
 */
typedef enum
{
  SNS_SMR_SVC_PRI_LOW = 0,
  SNS_SMR_SVC_PRI_MED = 50,
  SNS_SMR_SVC_PRI_HIGH = 100,
} sns_smr_svc_pri_e;

struct smr_client_hndl_s;
struct smr_req_handle_s;
struct smr_service_hndl_s;

typedef struct smr_service_hndl_s *smr_service_hndl;
typedef qmi_req_handle smr_req_handle;
typedef struct smr_client_hndl_s *smr_client_hndl;
typedef qmi_client_handle smr_qmi_client_handle;

typedef void *smr_txn_handle;
typedef unsigned int qmi_service_instance;

/*===========================================================================
                            MACROS
  ===========================================================================*/

/** Magic instance ID for .. to indicate that no preference on instance ID */
#define SMR_CLIENT_INSTANCE_ANY 0xFFFF

typedef enum
{
  SMR_NO_ERR = 0,           /* 0 */
  SMR_TIMEOUT_ERR,          /* 1 */
  SMR_OUT_OF_MEMORY,        /* 2 */
  SMR_INVALID_PARAMS,       /* 3 */
  SMR_OS_ERR,               /* 4 */
  SMR_XPORT_ERR /* An error indicated by underlying transport, QMI
                 * This shall be stored in a static variable and can
                 * be queried. This saves some bytes in the translation of
                 * QMI errors to a native format
                 */
} smr_err;

typedef enum
{
  SMR_CB_NO_ERR = 0,
  SMR_CB_CONN_REFUSED,
  SMR_CB_NO_MEM,
  SMR_CB_INTERNAL_ERR,
  SMR_CB_UNSUPPORTED_ERR,
  SMR_CB_REQ_HANDLED,
} smr_cb_err;

/*=============================================================================
  CALLBACK FUNCTIONS - CLIENT
  =============================================================================*/

/**
 * @brief
 * This callback function is called by the SMR infrastructure when a connection
 * is fully released.
 *
 * @param[i] release_cb_data cookie provided in smr_client_release()
 */
typedef void (*smr_client_release_cb) (void *release_cb_data);

/**
 * @brief
 * This callback function is called by the SMR infrastructure when a response
 * is received after a request is sent using smr_client_send_req()
 *
 * @param[i] client_handle Handle used by the infrastructure to identify different clients.
 * @param[i] msg_id Message ID
 * @param[i] resp_c_struct Pointer to the response
 * @param[i] resp_c_struct_len Length of the response
 * @param[i] resp_cb_data User-data
 * @param[i] transp_err Error code
 */
typedef void (*smr_client_resp_cb) (smr_client_hndl client_handle,
                                    unsigned int msg_id,
                                    void *resp_c_struct,
                                    unsigned int resp_c_struct_len,
                                    void *resp_cb_data,
                                    smr_err transp_err);

/**
 * @brief
 * This callback function is called by the SMR infrastructure when an
 * indication is received. This callback is registered at initialization.
 *
 * @param[i] client_handle Handle used by the infrastructure to identify different clients.
 * @param[i] msg_id Message ID
 * @param[i] ind_buf Pointer to the raw/undecoded indication
 * @param[i] ind_buf_len Length of the indication
 * @param[i] resp_cb_data User-data
 */
typedef void (*smr_client_ind_cb) (smr_client_hndl client_handle,
                                   unsigned int msg_id,
                                   void *ind_c_struct,
                                   unsigned int ind_struct_len,
                                   void *ind_cb_data);

/**
 * @brief
 * This callback function is called by the SMR infrastructure when the service
 * terminates or deregisters
 *
 * @param[i] client_handle Handle used by the infrastructure to identify different clients.
 * @param[i] error Error code
 * @param[i] err_cb_data User-data
 */
typedef void (*smr_client_error_cb) (smr_client_hndl client_handle,
                                     smr_err error,
                                     void *err_cb_data);

/**
 * @brief
 * This callback function is called by the SMR infrastructure
 * when a service becomes available - or a timout occurs
 *
 * @param[i] client_handle Handle used by the infrastructure to identify different clients.
 * @param[i] instanceID - if available, the service Id of the
 *       service requested
 * @param[i] timeOutExpired - true if the timer expired, false
 *       otherwise
 */
typedef void (*smr_client_init_cb)(qmi_idl_service_object_type serviceObj,
                                   qmi_service_instance instanceID,
                                   bool timeoutExpired);

/*=============================================================================
  CALLBACK FUNCTIONS - SERVICE
  =============================================================================*/

/**
 * @brief
 * This callback function is called by the SMR infrastructure when
 * infrastructure receives a request from each client(user of the service).
 *
 * @param[i] smr_service_hndl Handle used by the infrastructure to identify different clients.
 * @param[i] service_cookie Service specific data. Service cookie is registered
 *                          with the infrastructure during service registration.
 * @param[o] connection_handle Token to represent this client connection to the service.
 *
 * @return SMR_CB_NO_ERR or error code.
 */
typedef smr_cb_err (*smr_service_connect)(smr_qmi_client_handle smr_client_hndl,
                                          void *service_cookie,
                                          void **connection_handle);

/**
 * @brief
 * This callback function is called by the SMR infrastructure when the each
 * client(user of service) deregisters with the  SMR infrastructure.
 *
 * @param[i] connection_handle Token to represent this client connection to the service.
 * @param[i] service_cookie Service specific data. Service cookie is registered
 *                          with the infrastructure during service registration.
 *
 * @return SMR_CB_NO_ERR or error code.
 */
typedef void (*smr_service_disconnect)(void *connection_handle,
                                       void *service_cookie);

/**
 * @brief
 * This callback is invoked when the infrastructure receives an incoming message.
 * The infrastructure decodes the data and gives it to the services.
 *
 * @param[i] connection_handle Token to represent this client connection to the service.
 * @param[i] req_handle Handle provided by the infrastructure to specify this
 *                      particular transaction and message.
 * @param[i] msg_id Message ID pertaining to this particular message.
 * @param[i] req_c_struct C struct with the decoded data.
 * @param[i] req_c_struct_len Length of the c struct.
 * @param[i] service_cookie Service specific data. Service cookie is registered
 *                          with the infrastructure during service registration.
 *
 * @return SMR_CB_NO_ERR or error code.
 */
/*=========================================================================*/
typedef smr_cb_err (*smr_process_req)(void *connection_handle,
                                      smr_req_handle req_handle,
                                      unsigned int msg_id,
                                      void *req_c_struct,
                                      unsigned int req_c_struct_len,
                                      void *service_cookie);

/**
 * @brief
 * This callback function (if provided) is called by the framework
 * when a previously busy client becomes available for more indications.
 *
 * @param[i] smr_service_hndl Handle used by the infrastructure to identify different services.
 * @param[i] connection_handle Token to represent this client connection to the service.
 * @param[i] service_cookie Service specific data. Service cookie is registered
 *                          with the infrastructure during service registration.
 */
typedef void (*smr_resume_ind)(smr_service_hndl smr_service_hndl,
                               void *connection_handle,
                               void *service_cookie);

/*===========================================================================
  PUBLIC FUNCTIONS - CLIENT
  ===========================================================================*/

/**
 * @brief
 * Helper blocking function to lookup and initialize a connection with a
 * service with a specific instance ID.
 *
 * @param[i] service_obj Service object
 * @param[i] instance_id Instance ID of the service. Use SMR_CLIENT_INSTANCE_AN
 *                       if there is no preference on the instance ID.
 * @param[i] ind_cb Indication callback function
 * @param[i] ind_cb_data Indication callback user-data
 * @param[i] timeout Time-out in milliseconds. 0 = no timeout
 * @param[i] err_cb Pointer to callback function
 * @param[i] err_cb_data User-data
 * @param[o] client_handle Handle used by the infrastructure to identify different clients.
 *
 * @retval SMR_NO_ERR Success
 *         SMR_INTERNAL_ERR Invalid input parameters
 *         SMR_TIMEOUT_ERR No service of the required instance_id was
 *                         found in the time provided by timeout.
 */
smr_err smr_client_init(qmi_idl_service_object_type service_obj,
                        qmi_service_instance instance_id,
                        smr_client_ind_cb ind_cb,
                        void *ind_cb_data,
                        uint32_t timeout,
                        smr_client_error_cb err_cb,
                        void *err_cb_data,
                        smr_client_hndl *client_handle);

/**
 * @brief
 * Releases the connection to a service and return immediately without blocking.
 * the provided callback is called when the connection is fully released and
 * it is safe for the caller to release any associated resources.
 *
 * @param[i] client_handle Handle used by the infrastructure.
 * @param[i] release_cb Callback to call when the connection is fully released.
 * @param[i] release_cb_data Cookie to be provided with the callback.
 *
 * @return SMR_NO_ERR if function is successful, error code otherwise.
 */
smr_err smr_client_release(smr_client_hndl client_handle,
                           smr_client_release_cb release_cb,
                           void *release_cb_data);

/**
 * @brief
 * Sends an asynchronous message on the specified connection.
 * The callback function is expected to decode the message before sending it
 * through this function.
 *
 * @param[i] client_handle Handle used by the infrastructure.
 * @param[i] msg_id Message ID
 * @param[i] req_c_struct Pointer to the request
 * @param[i] req_c_struct_len Length of the request
 * @param[i] resp_c_struct Pointer to where the response will be stored
 * @param[i] resp_c_struct_len Length of the response buffer
 * @param[i] resp_cb Callback function to handle the response
 * @param[i] resp_cb_data Cablback user-data
 * @param[o] txn_handle Handle used to identify the transaction
 *
 * @return SMR_NO_ERR and sets the transaction handle if function is successful,
 *                    error code otherwise.
 *         SMR_SERVICE_ERR if server cannot be reached
 */
smr_err smr_client_send_req(smr_client_hndl client_handle,
                            unsigned int msg_id,
                            void *req_c_struct,
                            unsigned int req_c_struct_len,
                            void *resp_c_struct,
                            unsigned int resp_c_struct_len,
                            smr_client_resp_cb resp_cb,
                            void *resp_cb_data,
                            smr_txn_handle *txn_handle);

/*===========================================================================
  PUBLIC FUNCTIONS - SERVICE
  ===========================================================================*/

/**
 * @brief
 * Register a service with the SMR infrastructure.
 *
 * @param[i] service_obj Object containing meta information to encode/decode the messages.
 * @param[i] instance_id instance id to be used to register the service
 * @param[i] service_connect Callback to register each client with the service.
 * @param[i] service_disconnect Callback to unregister each client from service.
 * @param[i] service_process_req Callback that handles the incoming requests.
 * @param[i] service_cookie User data to be returned in callback functions.
 * @param[o] service_provider Handle to represent this service connection.
 *
 * @return SMR_NO_ERR if function is successful, error code otherwise.
 */
smr_err smr_service_register(qmi_idl_service_object_type service_obj,
                             qmi_service_instance instance_id,
                             smr_service_connect service_connect,
                             smr_service_disconnect service_disconnect,
                             smr_process_req service_process_req,
                             void *service_cookie,
                             smr_service_hndl *service_provider);

/**
 * @brief
 * Unregisters a service.
 *
 * @param[i] service_provider Handle given in the qmi_csi_register by the service.
 *
 * @return SMR_NO_ERR if function is successful, error code otherwise.
 */
smr_err smr_service_unregister(smr_service_hndl service_provider);

/**
 * @brief
 * Sends a response to the client.  Response must always be sent, as the
 * client will likely not free it's pre-allocated response buffer until
 * it receives one.
 *
 * @param[i] req_handle Handle used by QCSI infrastructure to identify the
 *                      transaction and the destination client.
 * @param[i] msg_id Message ID for this particular message.
 * @param[i] resp_c_struct C data structure for this response.
 * @param[i] resp_c_struct_len Size of the response c struct.
 *
 * @return SMR_NO_ERR if function is successful, error code otherwise.
 */
smr_err smr_service_send_resp(smr_req_handle req_handle,
                              unsigned int msg_id,
                              void *resp_c_struct,
                              unsigned int resp_c_struct_len);

/**
 * @brief
 * Sends an indication to the client.
 *
 * @param[i] service_hndl Opaque handle to identify the destination client.
 * @param[i] msg_id Message ID for this particular message.
 * @param[i] ind_c_struct C data strcuture for this indication.
 * @param[i] ind_c_struct_len Size of the indication c struct
 *
 * @return SMR_NO_ERR if function is successful, error code otherwise.
 */
smr_err smr_service_send_ind(smr_qmi_client_handle client_handle,
                             unsigned int msg_id,
                             void *ind_c_struct,
                             unsigned int ind_c_struct_len);

/**
 * @brief
 * Sends a broadcast indication to all registered clients.
 *
 * @param[i] service_provider Handle used by SMR infrastructure to identify
 *           the service that intends to send a  broadcast message.
 * @param[i] msg_id Message ID for this particular message.
 * @param[i] ind_c_struct C data structure for this broadcast indication.
 * @param[i] ind_c_struct_len Size of the broadcast indication.
 *
 * @return SMR_NO_ERR if function is successful, error code otherwise.
 */
smr_err smr_service_broadcast_ind(smr_service_hndl *service_provider,
                                  unsigned int msg_id,
                                  void *ind_c_struct,
                                  unsigned int ind_c_struct_len);

/**
 * @brief
 * Returns the qmi error
 *
 * @return Last QMI error, if the last call was successful,
 *         QMI_NO_ERROR
 */
int smr_get_qmi_err(void);

/**
* @brief
* This callback function is called by the SMR infrastructure when a service
* becomes available, or if the client-specified timeout passes.
*
* @param[i] serviceObj QMI service object
* @param[i] instanceID Instance ID of the service found
* @param[i] timeoutExpired Whether the timeout expired
*/

smr_err  smr_client_check(qmi_idl_service_object_type serviceObj,
                          qmi_service_instance instanceID,
                          unsigned int timeout,
                          smr_client_init_cb cb_func);


#endif /* SNS_USMR_H */
