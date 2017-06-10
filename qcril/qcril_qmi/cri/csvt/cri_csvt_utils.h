/***************************************************************************************************
    @file
    cri_csvt_utils.h

    @brief
    Supports utility functions for facilitating QMI CSVT call object operations.

    Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
    Qualcomm Technologies Proprietary and Confidential.
***************************************************************************************************/

#ifndef CRI_CSVT_UTILS
#define CRI_CSVT_UTILS

#include "utils_common.h"
#include "cri_core.h"
#include "circuit_switched_video_telephony_v01.h"

#define CRI_CSVT_MAX_CALLS (5)
#define CRI_CSVT_CALL_OBJECT_INVALIDATE_TIMEOUT (3)

typedef struct cri_csvt_call_object_type
{
    int is_valid;
    int hlos_call_id;
    int is_mt;
    char remote_party_number[CSVT_MAX_DIAL_STRING_LEN_V01 + 1];
    int csvt_call_object_invalidate_timer_id;
    csvt_event_report_ind_msg_v01 csvt_info;
}cri_csvt_call_object_type;

typedef struct cri_csvt_utils_hlos_call_object_type
{
    int hlos_call_id;
    int is_mt;
    csvt_event_type_enum_v01 csvt_call_state;
    char remote_party_number[CSVT_MAX_DIAL_STRING_LEN_V01 + 1];
    uint32_t call_fail_cause;
}cri_csvt_utils_hlos_call_object_type;

extern int csvt_client_id;
extern cri_csvt_call_object_type csvt_calls[CRI_CSVT_MAX_CALLS];






/***************************************************************************************************
    @function
    cri_csvt_utils_init_client

    @brief
    Creates and initializes QMI CSVT service client.

    @param[in]
        subscription_id
           subscription on which the client needs to be created
        hlos_ind_cb
           callback function that would be called by the QMI framework for delivering
           QMI CSVT service specific notifications

    @param[out]
        none

    @retval
    QMI_ERR_NONE_V01 If CSVT client creation and initialization is successful, appropriate
    error code otherwise
***************************************************************************************************/
qmi_error_type_v01 cri_csvt_utils_init_client(cri_core_subscription_id_type subscription_id,
                                              hlos_ind_cb_type hlos_ind_cb);


/***************************************************************************************************
    @function
    cri_csvt_utils_reset_client

    @brief
    Resets the QMI CSVT service client.

    @param[in]
        none

    @param[out]
        none

    @retval
    QMI_ERR_NONE_V01 If CSVT client reset is successful, appropriate error code otherwise
***************************************************************************************************/
qmi_error_type_v01 cri_csvt_utils_reset_client();

/***************************************************************************************************
    @function
    cri_csvt_utils_reinit_client

    @brief
    Reinitializes QMI CSVT service client.

    @param[in]
        subscription_id
           subscription on which the client needs to be created

    @param[out]
        none

    @retval
    QMI_ERR_NONE_V01 If CSVT client reinitialization is successful, appropriate
    error code otherwise
***************************************************************************************************/
qmi_error_type_v01 cri_csvt_utils_reinit_client(cri_core_subscription_id_type subscription_id);




/***************************************************************************************************
    @function
    cri_csvt_utils_release_client

    @brief
    Releases QMI CSVT service client and invalidates QMI CSVT related cache.

    @param[in]
        qmi_service_client_id
           CRI client id for the corresponding QMI CSVT client

    @param[out]
        none

    @retval
    none
***************************************************************************************************/
void cri_csvt_utils_release_client(int qmi_service_client_id);



/***************************************************************************************************
    @function
    cri_csvt_utils_cleanup_calls

    @brief
    Set call state as END for all the ongoing call, inform HLOS and then clean up the calls

    @param[in]
        none

    @param[out]
        none

    @retval
    none
***************************************************************************************************/
void cri_csvt_utils_cleanup_calls();



/***************************************************************************************************
    @function
    cri_csvt_utils_log_csvt_call_objects

    @brief
    Logs all existing QMI CSVT call objects.

    @param[in]
        none

    @param[out]
        none

    @retval
    none
***************************************************************************************************/
void cri_csvt_utils_log_csvt_call_objects();





/***************************************************************************************************
    @function
    cri_csvt_utils_retrieve_number_of_ongoing_csvt_calls

    @brief
    Retrieves the number of active QMI CSVT call objects.

    @param[in]
        none

    @param[out]
        none

    @retval
    number of active (calls in SETUP and END state are not considered) QMI CSVT call objects
***************************************************************************************************/
int cri_csvt_utils_retrieve_number_of_ongoing_csvt_calls(boolean ignore_call_end);






/***************************************************************************************************
    @function
    cri_csvt_utils_retrieve_hlos_ongoing_call_objects

    @brief
    Retrieves the QMI CSVT call objects in HLOS format.

    @param[in]
        none

    @param[out]
        none

    @retval
    array of active (calls in SETUP and END state are not considered) QMI CSVT call objects
    in HLOS format
    array needs to be freed up by the caller
***************************************************************************************************/
cri_csvt_utils_hlos_call_object_type* cri_csvt_utils_retrieve_hlos_ongoing_call_objects(
        boolean include_call_end);





/***************************************************************************************************
    @function
    cri_csvt_utils_retrieve_last_call_failure_cause

    @brief
    Retrieves the call failure cause of the last ended call.

    @param[in]
        none

    @param[out]
        none

    @retval
    call failure cause of the last ended call
***************************************************************************************************/
uint32_t cri_csvt_utils_retrieve_last_call_failure_cause();





/***************************************************************************************************
    @function
    cri_csvt_utils_is_hlos_call_id_belongs_to_csvt_call

    @brief
    Checks if a particular HLOS call id belongs to an existing QMI CSVT call object.

    @param[in]
        hlos_call_id
            hlos call id that is being checked

    @param[out]
        none

    @retval
    TRUE if a particular HLOS call id belongs to an existing QMI CSVT call object, FALSE
    otherwise
***************************************************************************************************/
int cri_csvt_utils_is_hlos_call_id_belongs_to_csvt_call(int hlos_call_id);





/***************************************************************************************************
    @function
    cri_csvt_utils_is_csvt_calls_present

    @brief
    Checks if QMI CSVT call object(s) exist(s)

    @param[in]
        none

    @param[out]
        none

    @retval
    TRUE if there is a existing QMI CSVT call object, FALSE otherwise
***************************************************************************************************/
int cri_csvt_utils_is_csvt_calls_present();





/***************************************************************************************************
    @function
    cri_csvt_utils_find_csvt_call_object_id_for_confirm_event

    @brief
    Checks if there exists a QMI CSVT call object which is ready to be updated with CONFIRM
    event state.

    @param[in]
        qmi_id
            QMI call id that is being used for retrieval of call object ready to be updated with
            CONFIRM event state


    @param[out]
        none

    @retval
    TRUE if there exists a QMI CSVT call object which is ready to be updated with CONFIRM
    event state, FALSE otherwise
***************************************************************************************************/
int cri_csvt_utils_find_csvt_call_object_id_for_confirm_event(uint32_t qmi_id);






/***************************************************************************************************
    @function
    cri_csvt_utils_find_qmi_id_based_on_csvt_call_object_id

    @brief
    Retrieves the corresponding QMI call id for a given CSVT call object id.

    @param[in]
        csvt_call_object_id
            csvt call object id that is being used for retrieval of QMI call id

    @param[out]
        none

    @retval
    corresponding QMI call id for a given CSVT call object id If found, NIL otherwise
***************************************************************************************************/
uint32_t cri_csvt_utils_find_qmi_id_based_on_csvt_call_object_id(int csvt_call_object_id);





/***************************************************************************************************
    @function
    cri_csvt_utils_find_csvt_call_object_id_based_on_qmi_id

    @brief
    Retrieves the corresponding CSVT call object id for a given QMI call id.

    @param[in]
        qmi_id
            QMI call id that is being used for retrieval of csvt call object id

    @param[out]
        none

    @retval
    corresponding CSVT call object id for a given QMI call id If found, NIL otherwise
***************************************************************************************************/
int cri_csvt_utils_find_csvt_call_object_id_based_on_qmi_id(uint32_t qmi_id);





/***************************************************************************************************
    @function
    cri_csvt_utils_allocate_hlos_call_id

    @brief
    Allocates HLOS call id.

    @param[in]
        none

    @param[out]
        none

    @retval
    allocated HLOS call id if allocation was successful, NIL otherwise
***************************************************************************************************/
int cri_csvt_utils_allocate_hlos_call_id();





/***************************************************************************************************
    @function
    cri_csvt_utils_find_hlos_call_id_in_csvt_call_state

    @brief
    Retrieves the corresponding HLOS call id for a given QMI CSVT call state.

    @param[in]
        csvt_call_state
            CSVT call state that is being used for retrieval of HLOS call id

    @param[out]
        none

    @retval
    corresponding HLOS call id of a QMI CSVT call object in a given call state If found,
    NIL otherwise
***************************************************************************************************/
int cri_csvt_utils_find_hlos_call_id_in_csvt_call_state(csvt_event_type_enum_v01 csvt_call_state);





/***************************************************************************************************
    @function
    cri_csvt_utils_is_csvt_call_with_hlos_call_id_in_csvt_call_state

    @brief
    Checks if a QMI CSVT call object is present with the given HLOS call id and call state.

    @param[in]
        hlos_call_id
            hlos_call_id that is being used
        csvt_call_state
            CSVT call state that is being used

    @param[out]
        none

    @retval
    TRUE If a QMI CSVT call object was found with the given HLOS call id and call state,
    FALSE otherwise
***************************************************************************************************/
int cri_csvt_utils_is_csvt_call_with_hlos_call_id_in_csvt_call_state(
    int hlos_call_id,
    csvt_event_type_enum_v01 csvt_call_state
);






/***************************************************************************************************
    @function
    cri_csvt_utils_find_hlos_id_based_on_csvt_call_object_id

    @brief
    Retrieves the corresponding HLOS call id for a given CSVT call object id.

    @param[in]
        csvt_call_object_id
            CSVT call object id that is being used for retrieval of HLOS call id

    @param[out]
        none

    @retval
    corresponding HLOS call id for a given CSVT call object id If found, NIL otherwise
***************************************************************************************************/
int cri_csvt_utils_find_hlos_id_based_on_csvt_call_object_id(int csvt_call_object_id);




/***************************************************************************************************
    @function
    cri_csvt_utils_find_csvt_call_object_id_based_on_hlos_call_id

    @brief
    Retrieves the corresponding CSVT call object id for a given HLOS call id.

    @param[in]
        hlos_call_id
            HLOS call idthat is being used for retrieval of CSVT call object id

    @param[out]
        none

    @retval
    corresponding CSVT call object id for a given HLOS call id If found, NIL otherwise
***************************************************************************************************/
int cri_csvt_utils_find_csvt_call_object_id_based_on_hlos_call_id(int hlos_call_id);




/***************************************************************************************************
    @function
    cri_csvt_utils_update_csvt_call_object_with_csvt_info

    @brief
    Updates a QMI CSVT call object with the information from EVENT_REPORT_IND.

    @param[in]
        csvt_call_object_id
            QMI CSVT call object id of the QMI CSVT call object that is being updated
        csvt_info
            pointer to the information from EVENT_REPORT_IND that is being used for updating

    @param[out]
        none

    @retval
    none
***************************************************************************************************/
void cri_csvt_utils_update_csvt_call_object_with_csvt_info(
    int csvt_call_object_id,
    csvt_event_report_ind_msg_v01 *csvt_info
);



/***************************************************************************************************
    @function
    cri_csvt_utils_update_csvt_call_object_with_qmi_id

    @brief
    Updates a QMI CSVT call object with the QMI call id

    @param[in]
        csvt_call_object_id
            QMI CSVT call object id of the QMI CSVT call object that is being updated
        qmi_id
            QMI call id that is being used for updating the QMI CSVT call object

    @param[out]
        none

    @retval
    none
***************************************************************************************************/
void cri_csvt_utils_update_csvt_call_object_with_qmi_id(
    int csvt_call_object_id,
    uint32_t qmi_id
);





/***************************************************************************************************
    @function
    cri_csvt_utils_is_valid_csvt_call_object_id

    @brief
    Checks If there exists a QMI CSVT call object with the given CSVT call object id.

    @param[in]
        csvt_call_object_id
            QMI CSVT call object id that is being checked for validity

    @param[out]
        none

    @retval
    TRUE If there exists a QMI CSVT call object with the given CSVT call object id, FALSE
    otherwise
***************************************************************************************************/
int cri_csvt_utils_is_valid_csvt_call_object_id(int csvt_call_object_id);




/***************************************************************************************************
    @function
    cri_csvt_utils_confirm_call_based_on_qmi_id

    @brief
    Sends QMI_CSVT_CONFIRM_CALL_REQ_V01 for the call with the given QMI CSVT call id.

    @param[in]
        qmi_id
            QMI CSVT call id of the call that is being confirmed

    @param[out]
        none

    @retval
    none
***************************************************************************************************/
void cri_csvt_utils_confirm_call_based_on_qmi_id(uint32_t qmi_id);




/***************************************************************************************************
    @function
    cri_csvt_utils_setup_timer_to_invalidate_csvt_call_object

    @brief
    Setups a timer with CRI_CSVT_CALL_OBJECT_INVALIDATE_TIMEOUT seconds at the expiry of which
    the QMI CSVT call object is invalidated.

    @param[in]
        csvt_call_object_id
            CSVT call object id of the CSVT call object that is being considered for invalidation

    @param[out]
        none

    @retval
    none
***************************************************************************************************/
void cri_csvt_utils_setup_timer_to_invalidate_csvt_call_object(int csvt_call_object_id);




/***************************************************************************************************
    @function
    cri_csvt_utils_invalidate_csvt_call_object_timer_expiry_handler

    @brief
    Handles the expired timer which was setup to invalidates a QMI CSVT call object.

    @param[in]
        invalidate_csvt_call_object_cb_data
            expiry call back data
        invalidate_csvt_call_object_cb_data_len
            length of the expiry callback data

    @param[out]
        none

    @retval
    none
***************************************************************************************************/
void cri_csvt_utils_invalidate_csvt_call_object_timer_expiry_handler(
    void *invalidate_csvt_call_object_cb_data,
    size_t invalidate_csvt_call_object_cb_data_len
);





/***************************************************************************************************
    @function
    cri_csvt_utils_allocate_csvt_call_object

    @brief
    Allocates a QMI CSVT call object.

    @param[in]
        remote_party_number
            string containing the remote party number
        is_mt
            TRUE if the call is MT, FALSE otherwise

    @param[out]
        none

    @retval
    allocated CSVT call object id if allocation was successful, NIL otherwise
***************************************************************************************************/
int cri_csvt_utils_allocate_csvt_call_object(char *remote_party_number,
                                             int is_mt);





/***************************************************************************************************
    @function
    cri_csvt_utils_invalidate_csvt_call_object

    @brief
    Invalidates the QMI CSVT call object for the given CSVT call object id.

    @param[in]
        csvt_call_object_id
            CSVT call object id that is being used for invalidating the corresponding CSVT
            call object

    @param[out]
        none

    @retval
    none
***************************************************************************************************/
void cri_csvt_utils_invalidate_csvt_call_object(int csvt_call_object_id);


#endif

