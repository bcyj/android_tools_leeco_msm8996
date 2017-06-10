/***************************************************************************************************
    @file
    cri_core.h

    @brief
    Supports functions for performing operations on/using qmi clients
    Primary use would be to create, release qmi clients and send, recv messages using the same.

    Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
    Qualcomm Technologies Proprietary and Confidential.
***************************************************************************************************/

#ifndef CRI_CORE
#define CRI_CORE

#include "utils_common.h"
#include "qmi.h"
#include "qmi_i.h"
#include "common_v01.h"
#include "qmi_client.h"

struct cri_rule_handler_user_rule_info_type;
typedef uint32_t cri_core_hlos_token_id_type;
typedef uint16_t cri_core_token_id_type;
typedef uint64_t cri_core_context_type;

typedef enum {
  CRI_ERROR_TYPE_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  CRI_ERR_NONE_V01 = 0x0000,
  CRI_ERR_MALFORMED_MSG_V01 = 0x0001,
  CRI_ERR_NO_MEMORY_V01 = 0x0002,
  CRI_ERR_INTERNAL_V01 = 0x0003,
  CRI_ERR_ABORTED_V01 = 0x0004,
  CRI_ERR_CLIENT_IDS_EXHAUSTED_V01 = 0x0005,
  CRI_ERR_UNABORTABLE_TRANSACTION_V01 = 0x0006,
  CRI_ERR_INVALID_CLIENT_ID_V01 = 0x0007,
  CRI_ERR_NO_THRESHOLDS_V01 = 0x0008,
  CRI_ERR_INVALID_HANDLE_V01 = 0x0009,
  CRI_ERR_INVALID_PROFILE_V01 = 0x000A,
  CRI_ERR_INVALID_PINID_V01 = 0x000B,
  CRI_ERR_INCORRECT_PIN_V01 = 0x000C,
  CRI_ERR_NO_NETWORK_FOUND_V01 = 0x000D,
  CRI_ERR_CALL_FAILED_V01 = 0x000E,
  CRI_ERR_OUT_OF_CALL_V01 = 0x000F,
  CRI_ERR_NOT_PROVISIONED_V01 = 0x0010,
  CRI_ERR_MISSING_ARG_V01 = 0x0011,
  CRI_ERR_ARG_TOO_LONG_V01 = 0x0013,
  CRI_ERR_INVALID_TX_ID_V01 = 0x0016,
  CRI_ERR_DEVICE_IN_USE_V01 = 0x0017,
  CRI_ERR_OP_NETWORK_UNSUPPORTED_V01 = 0x0018,
  CRI_ERR_OP_DEVICE_UNSUPPORTED_V01 = 0x0019,
  CRI_ERR_NO_EFFECT_V01 = 0x001A,
  CRI_ERR_NO_FREE_PROFILE_V01 = 0x001B,
  CRI_ERR_INVALID_PDP_TYPE_V01 = 0x001C,
  CRI_ERR_INVALID_TECH_PREF_V01 = 0x001D,
  CRI_ERR_INVALID_PROFILE_TYPE_V01 = 0x001E,
  CRI_ERR_INVALID_SERVICE_TYPE_V01 = 0x001F,
  CRI_ERR_INVALID_REGISTER_ACTION_V01 = 0x0020,
  CRI_ERR_INVALID_PS_ATTACH_ACTION_V01 = 0x0021,
  CRI_ERR_AUTHENTICATION_FAILED_V01 = 0x0022,
  CRI_ERR_PIN_BLOCKED_V01 = 0x0023,
  CRI_ERR_PIN_PERM_BLOCKED_V01 = 0x0024,
  CRI_ERR_SIM_NOT_INITIALIZED_V01 = 0x0025,
  CRI_ERR_MAX_QOS_REQUESTS_IN_USE_V01 = 0x0026,
  CRI_ERR_INCORRECT_FLOW_FILTER_V01 = 0x0027,
  CRI_ERR_NETWORK_QOS_UNAWARE_V01 = 0x0028,
  CRI_ERR_INVALID_ID_V01 = 0x0029,
  CRI_ERR_INVALID_QOS_ID_V01 = 0x0029,
  CRI_ERR_REQUESTED_NUM_UNSUPPORTED_V01 = 0x002A,
  CRI_ERR_INTERFACE_NOT_FOUND_V01 = 0x002B,
  CRI_ERR_FLOW_SUSPENDED_V01 = 0x002C,
  CRI_ERR_INVALID_DATA_FORMAT_V01 = 0x002D,
  CRI_ERR_GENERAL_V01 = 0x002E,
  CRI_ERR_UNKNOWN_V01 = 0x002F,
  CRI_ERR_INVALID_ARG_V01 = 0x0030,
  CRI_ERR_INVALID_INDEX_V01 = 0x0031,
  CRI_ERR_NO_ENTRY_V01 = 0x0032,
  CRI_ERR_DEVICE_STORAGE_FULL_V01 = 0x0033,
  CRI_ERR_DEVICE_NOT_READY_V01 = 0x0034,
  CRI_ERR_NETWORK_NOT_READY_V01 = 0x0035,
  CRI_ERR_CAUSE_CODE_V01 = 0x0036,
  CRI_ERR_MESSAGE_NOT_SENT_V01 = 0x0037,
  CRI_ERR_MESSAGE_DELIVERY_FAILURE_V01 = 0x0038,
  CRI_ERR_INVALID_MESSAGE_ID_V01 = 0x0039,
  CRI_ERR_ENCODING_V01 = 0x003A,
  CRI_ERR_AUTHENTICATION_LOCK_V01 = 0x003B,
  CRI_ERR_INVALID_TRANSITION_V01 = 0x003C,
  CRI_ERR_NOT_A_MCAST_IFACE_V01 = 0x003D,
  CRI_ERR_MAX_MCAST_REQUESTS_IN_USE_V01 = 0x003E,
  CRI_ERR_INVALID_MCAST_HANDLE_V01 = 0x003F,
  CRI_ERR_INVALID_IP_FAMILY_PREF_V01 = 0x0040,
  CRI_ERR_SESSION_INACTIVE_V01 = 0x0041,
  CRI_ERR_SESSION_INVALID_V01 = 0x0042,
  CRI_ERR_SESSION_OWNERSHIP_V01 = 0x0043,
  CRI_ERR_INSUFFICIENT_RESOURCES_V01 = 0x0044,
  CRI_ERR_DISABLED_V01 = 0x0045,
  CRI_ERR_INVALID_OPERATION_V01 = 0x0046,
  CRI_ERR_INVALID_QMI_CMD_V01 = 0x0047,
  CRI_ERR_TPDU_TYPE_V01 = 0x0048,
  CRI_ERR_SMSC_ADDR_V01 = 0x0049,
  CRI_ERR_INFO_UNAVAILABLE_V01 = 0x004A,
  CRI_ERR_SEGMENT_TOO_LONG_V01 = 0x004B,
  CRI_ERR_SEGMENT_ORDER_V01 = 0x004C,
  CRI_ERR_BUNDLING_NOT_SUPPORTED_V01 = 0x004D,
  CRI_ERR_OP_PARTIAL_FAILURE_V01 = 0x004E,
  CRI_ERR_POLICY_MISMATCH_V01 = 0x004F,
  CRI_ERR_SIM_FILE_NOT_FOUND_V01 = 0x0050,
  CRI_ERR_EXTENDED_INTERNAL_V01 = 0x0051,
  CRI_ERR_ACCESS_DENIED_V01 = 0x0052,
  CRI_ERR_HARDWARE_RESTRICTED_V01 = 0x0053,
  CRI_ERR_ACK_NOT_SENT_V01 = 0x0054,
  CRI_ERR_INJECT_TIMEOUT_V01 = 0x0055,
  CRI_ERR_INCOMPATIBLE_STATE_V01 = 0x005A,
  CRI_ERR_FDN_RESTRICT_V01 = 0x005B,
  CRI_ERR_SUPS_FAILURE_CAUSE_V01 = 0x005C,
  CRI_ERR_NO_RADIO_V01 = 0x005D,
  CRI_ERR_NOT_SUPPORTED_V01 = 0x005E,
  CRI_ERR_NO_SUBSCRIPTION_V01 = 0x005F,
  CRI_ERR_CARD_CALL_CONTROL_FAILED_V01 = 0x0060,
  CRI_ERR_NETWORK_ABORTED_V01 = 0x0061,
  CRI_ERR_MSG_BLOCKED_V01 = 0x0062,
  CRI_ERR_INVALID_SESSION_TYPE_V01 = 0x0064,
  CRI_ERR_INVALID_PB_TYPE_V01 = 0x0065,
  CRI_ERR_NO_SIM_V01 = 0x0066,
  CRI_ERR_PB_NOT_READY_V01 = 0x0067,
  CRI_ERR_PIN_RESTRICTION_V01 = 0x0068,
  CRI_ERR_PIN2_RESTRICTION_V01 = 0x0069,
  CRI_ERR_PUK_RESTRICTION_V01 = 0x006A,
  CRI_ERR_PUK2_RESTRICTION_V01 = 0x006B,
  CRI_ERR_PB_ACCESS_RESTRICTED_V01 = 0x006C,
  CRI_ERR_PB_DELETE_IN_PROG_V01 = 0x006D,
  CRI_ERR_PB_TEXT_TOO_LONG_V01 = 0x006E,
  CRI_ERR_PB_NUMBER_TOO_LONG_V01 = 0x006F,
  CRI_ERR_PB_HIDDEN_KEY_RESTRICTION_V01 = 0x0070,
  CRI_ERR_DIAL_MODIFIED_TO_DIAL = 0x0100,
  CRI_ERR_DIAL_MODIFIED_TO_SS = 0x0101,
  CRI_ERR_DIAL_MODIFIED_TO_USSD = 0x0102,
  CRI_ERR_DIAL_FDN_CHECK_FAILURE = 0x0103,
  CRI_ERROR_TYPE_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
} cri_core_error_type;

typedef void (*hlos_ind_cb_type)(unsigned long message_id,
                                 void *ind_data,
                                 int ind_data_len);
typedef void (*hlos_resp_cb_type)(cri_core_context_type context,
                                  cri_core_error_type cri_core_error,
                                  void *hlos_cb_data,
                                  void *cri_resp_data);

typedef enum {
    CRI_CORE_PRIMARY_CRI_SUBSCRIPTION_ID = 0x1,
    CRI_CORE_SECONDARY_CRI_SUBSCRIPTION_ID = 0x2,
    CRI_CORE_TERTIARY_CRI_SUBSCRIPTION_ID = 0x3,
    CRI_CORE_MAX_CRI_SUBSCRIPTION_ID = 0xF
}cri_core_subscription_id_type;

#define CRI_CORE_MAX_CLIENTS (QMI_MAX_SERVICES)

#define CRI_CORE_MINIMAL_TIMEOUT (5)
#define CRI_CORE_STANDARD_TIMEOUT (30)
#define CRI_CORE_MAX_TIMEOUT (60)

/* Enum for services.  */
typedef enum
{
  QMI_CRI_FIRST_SERVICE                       = 0x01,
  QMI_CRI_WDS_SERVICE                         = QMI_FIRST_SERVICE,
  QMI_CRI_DMS_SERVICE                         = 0x02,
  QMI_CRI_NAS_SERVICE                         = 0x03,
  QMI_CRI_QOS_SERVICE                         = 0x04,
  QMI_CRI_WMS_SERVICE                         = 0x05,
  QMI_CRI_SERVICE_NOT_YET_IMPLEMENTED_1       = 0x06,
  QMI_CRI_EAP_SERVICE                         = 0x07,
  QMI_CRI_ATCOP_SERVICE                       = 0x08,
  QMI_CRI_VOICE_SERVICE                       = 0x09,
  QMI_CRI_CAT_SERVICE                         = 0x0A,
  QMI_CRI_SIM_SERVICE                         = 0x0B,
  QMI_CRI_PBM_SERVICE                         = 0x0C,
  QMI_CRI_SERVICE_NOT_YET_IMPLEMENTED_2       = 0x0D,
  QMI_CRI_SERVICE_NOT_YET_IMPLEMENTED_3       = 0x0E,
  QMI_CRI_SERVICE_NOT_YET_IMPLEMENTED_4       = 0x0F,
  QMI_CRI_SERVICE_NOT_YET_IMPLEMENTED_5       = 0x10,
  QMI_CRI_SAR_SERVICE                         = 0x11,
  QMI_CRI_IMS_VIDEO_SERVICE                   = 0x12,
  QMI_CRI_SERVICE_NOT_YET_IMPLEMENTED_6       = 0x13,
  QMI_CRI_CSD_SERVICE                         = 0x14,
  QMI_CRI_SERVICE_NOT_YET_IMPLEMENTED_7       = 0x15,
  QMI_CRI_SERVICE_NOT_YET_IMPLEMENTED_8       = 0x16,
  QMI_CRI_SERVICE_NOT_YET_IMPLEMENTED_9       = 0x17,
  QMI_CRI_SERVICE_NOT_YET_IMPLEMENTED_10      = 0x18,
  QMI_CRI_SERVICE_NOT_YET_IMPLEMENTED_11      = 0x19,
  QMI_CRI_SERVICE_NOT_YET_IMPLEMENTED_12      = 0x1A,
  QMI_CRI_SERVICE_NOT_YET_IMPLEMENTED_13      = 0x1B,
  QMI_CRI_SERVICE_NOT_YET_IMPLEMENTED_14      = 0x1C,
  QMI_CRI_CSVT_SERVICE                        = 0x1D,
  QMI_CRI_IMS_VT_SERVICE,
  QMI_CRI_IMS_PRESENCE_SERVICE                = 0x1F,
  QMI_CRI_RFPE_SERVICE                        = 0x29,
  QMI_CRI_DSD_SERVICE                         = 0x2A,
  QMI_CRI_SERVICE_NOT_YET_IMPLEMENTED_15      = 0x2B,
  QMI_CRI_FIRST_VS_SERVICE,
  QMI_CRI_RF_SAR_SERVICE                      = QMI_FIRST_VS_SERVICE,
  QMI_CRI_MAX_SERVICES
} qmi_cri_service_id_type;

typedef enum cri_core_message_category_type
{
    RESP = 1,
    IND,
    QMUXD
}cri_core_message_category_type;

typedef struct cri_core_cri_message_data_type
{
    cri_core_message_category_type cri_message_category;
    unsigned long event_id;
    qmi_client_type user_handle;
    void *data;
    size_t data_len;
    void *cb_data;
    qmi_client_error_type transport_error;
}cri_core_cri_message_data_type;

typedef struct cri_core_cri_client_init_service_info_type
{
    qmi_service_id_type cri_service_id;
    hlos_ind_cb_type hlos_ind_cb;
}cri_core_cri_client_init_service_info_type;

typedef struct cri_core_cri_client_init_info_type
{
    cri_core_subscription_id_type subscription_id;
    int number_of_cri_services_to_be_initialized;
    cri_core_cri_client_init_service_info_type service_info[CRI_CORE_MAX_CLIENTS];
}cri_core_cri_client_init_info_type;





/***************************************************************************************************
    @function
    cri_core_start

    @brief
    Initializes the CRI framework. Needs to be called before initializing any services via
    cri_core_cri_client_init.

    @param[in]
        none

    @param[out]
        none

    @retval
    ESUCCESS if CRI framework has been successfully initialized, appropriate error code otherwise
***************************************************************************************************/
int cri_core_start();






/***************************************************************************************************
    @function
    cri_core_cri_client_init

    @brief
    Initializes all the QMI services requested by the caller. Needs to be called before attempting
    to send or receive messages to/from modem through CRI.

    @param[in]
        client_init_info
            contains information about all the services that need to be initialized.

    @param[out]
        none

    @retval
    QMI_ERR_NONE_V01 if CRI framework has been successfully initialized, appropriate error code
    otherwise
***************************************************************************************************/
qmi_error_type_v01 cri_core_cri_client_init(cri_core_cri_client_init_info_type *client_init_info);

/***************************************************************************************************
    @function
    cri_core_cri_client_reset

    @brief
    Reset the QMI services to clear the subscription binding

    @param[in]
        client_init_info
            contains information about all the services that need to be reset.

    @param[out]
        none

    @retval
    QMI_ERR_NONE_V01 if CRI framework has been successfully reset, appropriate error code
    otherwise
***************************************************************************************************/
qmi_error_type_v01 cri_core_cri_client_reset(cri_core_cri_client_init_info_type *client_init_info);

/***************************************************************************************************
    @function
    cri_core_cri_client_reinit

    @brief
    Re-initialize the QMI services to update the subscription binding

    @param[in]
        client_init_info
            contains information about all the services that need to be initialized.

    @param[out]
        none

    @retval
    QMI_ERR_NONE_V01 if CRI framework has been successfully re-initialized, appropriate error code
    otherwise
***************************************************************************************************/
qmi_error_type_v01 cri_core_cri_client_reinit(cri_core_cri_client_init_info_type *client_init_info);





/***************************************************************************************************
    @function
    cri_core_cri_client_release

    @brief
    Releases the QMI services created using cri_core_cri_client_init().

    @param[in]
        none

    @param[out]
        none

    @retval
    none
***************************************************************************************************/
void cri_core_cri_client_release();





/***************************************************************************************************
    @function
    cri_core_generate_context_using_subscription_id__hlos_token_id

    @brief
    Generates the context using subscription id and HLOS token id.

    @param[in]
        subscription_id
            subscription id for which the context is being requested for
        hlos_token_id
            HLOS token id for which the context is being requested for

    @param[out]
        none

    @retval
    context calculated using subscription id and HLOS token id
***************************************************************************************************/
cri_core_context_type cri_core_generate_context_using_subscription_id__hlos_token_id(
                            cri_core_subscription_id_type subscription_id,
                            cri_core_hlos_token_id_type hlos_token_id);





/***************************************************************************************************
    @function
    cri_core_generate_context_using_cri_token_id

    @brief
    Generates the context using CRI token id.

    @param[in]
        cri_core_context
           context to which CRI token id needs to be attached
        cri_core_token_id
            CRI token id for which the context is being requested for

    @param[out]
        none

    @retval
    context which would have HLOS token id, subscription id and CRI token id
***************************************************************************************************/
cri_core_context_type cri_core_generate_context_using_cri_token_id(
                            cri_core_context_type cri_core_context,
                            cri_core_token_id_type cri_core_token_id);





/***************************************************************************************************
    @function
    cri_core_retrieve_cri_token_id_from_context

    @brief
    Retrieves the CRI token id from the context.

    @param[in]
        cri_core_context
           context from which CRI token id needs to be retrieved

    @param[out]
        cri_core_token_id
            pointer to CRI token id which has been retrieved from the provided context

    @retval
    none
***************************************************************************************************/
void cri_core_retrieve_cri_token_id_from_context(cri_core_context_type cri_core_context,
                                                 cri_core_token_id_type *cri_core_token_id);





/***************************************************************************************************
    @function
    cri_core_retrieve_subscription_id__hlos_token_id_from_context

    @brief
    Retrieves the CRI token id from the context.

    @param[in]
        cri_core_context
           context from which subscription id and HLOS token id need to be retrieved

    @param[out]
        subscription_id
            pointer to subscription id which has been retrieved from the provided context
        hlos_token_id
            pointer to HLOS token id which has been retrieved from the provided context

    @retval
    none
***************************************************************************************************/
void cri_core_retrieve_subscription_id__hlos_token_id_from_context(
           cri_core_context_type cri_core_context,
           cri_core_subscription_id_type *subscription_id,
           cri_core_hlos_token_id_type *hlos_token_id);





/***************************************************************************************************
    @function
    cri_core_create_loggable_context

    @brief
    Calculates the string that can be used for logging the context.
    Should only be called from the core handler thread - Is NOT thread safe

    @param[in]
        cri_core_context
           context which needs to be logged.

    @param[out]
        none

    @retval
    string that contains the loggable format of the context
    format of the string is:
    context <context> (hlos token id <hlos token id>, sub id <sub id>, cri token id <cri token id>)
***************************************************************************************************/
char* cri_core_create_loggable_context(cri_core_context_type cri_core_context);





/***************************************************************************************************
    @function
    cri_core_retrieve_err_code

    @brief
    Calculates the error code from a qmi response.

    @param[in]
        transport_error
           error code returned by the qmi framework when a message is sent using sync/async
           mechanism
        resp_err
           QMI response TLV that is part of every QMI response message
           most of the QMI response messages have the response TLV as the first TLV of the message

    @param[out]
        none

    @retval
    CRI_ERR_NONE_V01 if message has been sent succesfully and response is also successful,
    appropriate error code otherwise
***************************************************************************************************/
cri_core_error_type cri_core_retrieve_err_code(qmi_error_type_v01 transport_error,
                                               qmi_response_type_v01* resp_err);





/***************************************************************************************************
    @function
    cri_core_create_qmi_client

    @brief
    Initializes QMI framework that would be used by CRI.

    @param[in]
        sys_event_rx_hdlr
           callback function that would be called by the QMI framework for delivering
           framework specific notifications

    @param[out]
        none

    @retval
    QMI framework handle if QMI framework has been initialized succesfully,
    appropriate negative error code otherwise
***************************************************************************************************/
int cri_core_create_qmi_client(qmi_sys_event_rx_hdlr sys_event_rx_hdlr);





/***************************************************************************************************
    @function
    cri_core_create_qmi_service_client

    @brief
    Initializes QMI service client.

    @param[in]
        service_id
           service id of the service that needs to be initialized
        hlos_ind_cb
           callback function that would be called by the QMI framework for delivering
           service specific notifications

    @param[out]
        none

    @retval
    CRI client ID if QMI service has been initialized succesfully, QMI_INTERNAL_ERR otherwise
***************************************************************************************************/
int cri_core_create_qmi_service_client(qmi_service_id_type service_id,
                                       hlos_ind_cb_type hlos_ind_cb);






/***************************************************************************************************
    @function
    cri_core_release_qmi_client

    @brief
    Releases QMI framework.

    @param[in]
        qmi_client_handle
           QMI client handle returned by cri_core_create_qmi_client

    @param[out]
        none

    @retval
    QMI_NO_ERR if QMI framework has been released succesfully, appropriate error code otherwise
***************************************************************************************************/
int cri_core_release_qmi_client(int qmi_client_handle);






/***************************************************************************************************
    @function
    cri_core_release_qmi_service_client

    @brief
    Releases QMI service client.

    @param[in]
        qmi_service_client_id
           CRI client ID returned by cri_core_create_qmi_service_client

    @param[out]
        none

    @retval
    QMI_NO_ERR if QMI service client has been released succesfully, appropriate error code otherwise
***************************************************************************************************/
int cri_core_release_qmi_service_client(int qmi_service_client_id);





/***************************************************************************************************
    @function
    cri_core_qmi_send_msg_sync

    @brief
    Sends QMI message using sync mechanism to the QMI service server on modem.

    @param[in]
        qmi_service_client_id
           CRI client ID returned by cri_core_create_qmi_service_client
        message_id
           message id of the message that needs to be sent
        req_message
           pointer to the request message that needs to be sent
        req_message_len
           length of the request message that needs to be sent
        resp_message
           pointer to the response message that would be received in response to the sent request
        resp_message_len
           length of the response message that would be received
        timeout_secs
           number of seconds to wait for the response message from the QMI service server

    @param[out]
        none

    @retval
    QMI_ERR_NONE_V01 if QMI message has been sent successfully and a response has been received,
    appropriate error code otherwise
***************************************************************************************************/
qmi_error_type_v01 cri_core_qmi_send_msg_sync(int qmi_service_client_id,
                                              unsigned long message_id,
                                              void *req_message,
                                              int req_message_len,
                                              void *resp_message,
                                              int resp_message_len,
                                              int timeout_secs);





/***************************************************************************************************
    @function
    cri_core_qmi_send_msg_async

    @brief
    Sends QMI message using async mechanism to the QMI service server on modem.
    Creates a rule (condition on which the callback for this async request should be called)
    with a timeout.
    If caller of this function does not have any specific conditions on which the callback needs
    to be called, then the callback would be called as soon as the response arrives or the timer
    expires.

    @param[in]
        cri_core_context
           context comprising of HLOS token id and subscription id
        qmi_service_client_id
           CRI client ID returned by cri_core_create_qmi_service_client
        message_id
           message id of the message that needs to be sent
        req_message
           pointer to the request message that needs to be sent
        req_message_len
           length of the request message that needs to be sent
        resp_message_len
           length of the response message that would be received
        hlos_cb_data
           user callback data that needs to be passed when calling the callback
        hlos_resp_cb
           user callback that needs to be called when the rule has been met
        timeout_secs
           number of seconds to wait for the rule to be met
        user_rule_info
           pointer to user specific rule data that needs to be used for checking If corresponding
           rule has been met

    @param[out]
        none

    @retval
    QMI_ERR_NONE_V01 if QMI message has been sent successfully and a rule has been created
    successfully, appropriate error code otherwise
***************************************************************************************************/
qmi_error_type_v01 cri_core_qmi_send_msg_async(cri_core_context_type cri_core_context,
                                               int qmi_service_client_id,
                                               unsigned long message_id,
                                               void *req_message,
                                               int req_message_len,
                                               int resp_message_len,
                                               const void *hlos_cb_data,
                                               hlos_resp_cb_type hlos_resp_cb,
                                               int timeout_secs,
                                               struct cri_rule_handler_user_rule_info_type
                                                                            *user_rule_info
                                               );




/***************************************************************************************************
    @function
    cri_core_async_resp_handler

    @brief
    Handles the QMI async response in context of the core thread (as opposed to the QMI framework
    thread).
    QMI async response would have been queued up in the core queue by the QMI framework thread
    as soon as the response arrived.

    @param[in]
        event_data
           pointer to data that comprises of the QMI async response related information

    @param[out]
        none

    @retval
    none
***************************************************************************************************/
void cri_core_async_resp_handler(void *event_data);





/***************************************************************************************************
    @function
    cri_core_unsol_ind_handler

    @brief
    Handles the QMI indication in context of the core thread (as opposed to the QMI framework
    thread).
    QMI indication would have been queued up in the core queue by the QMI framework thread
    as soon as the indication arrived.

    @param[in]
        event_data
           pointer to data that comprises of the QMI indication related information

    @param[out]
        none

    @retval
    none
***************************************************************************************************/
void cri_core_unsol_ind_handler(void *event_data);






/***************************************************************************************************
    @function
    cri_core_retrieve_hlos_ind_cb

    @brief
    Retrieves the HLOS indication callback provided through cri_core_cri_client_init while
    requesting for initialization of a QMI service.

    @param[in]
        qmi_service_client_id
           CRI client ID for which HLOS indication callback needs to be retrieved

    @param[out]
        none

    @retval
    pointer to HLOS indication callback function
***************************************************************************************************/
hlos_ind_cb_type cri_core_retrieve_hlos_ind_cb(int qmi_service_client_id);


#endif
