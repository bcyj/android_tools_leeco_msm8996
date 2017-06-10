/******************************************************************************
  @file    qmi_simple_ril_core.h
  @brief   Sample simple RIL core

  DESCRIPTION
  Sample Radio Interface Layer (telephony adaptation layer) core

  ---------------------------------------------------------------------------

  Copyright (c) 2010 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.  ---------------------------------------------------------------------------
******************************************************************************/

#include "qmi.h"
#include "qmi_client_utils.h"

#ifndef QMI_SIMPLE_RIL_CORE_H
#define QMI_SIMPLE_RIL_CORE_H

#define SIMPLE_RIL_NO_TIMEOUT (0)

typedef struct qmi_simple_ril_info_set
    {
    int nof_entries;
    int nof_allocated;
    char ** entries;
    } qmi_simple_ril_info_set;

extern void qmi_simple_ril_util_free_info_set(qmi_simple_ril_info_set* info_set);
extern qmi_simple_ril_info_set* qmi_simple_ril_util_alloc_info_set(void);
extern void qmi_simple_ril_util_add_entry_to_info_set(qmi_simple_ril_info_set* info_set, char* new_enry);
extern qmi_simple_ril_info_set* qmi_simple_ril_util_clone_info_set(qmi_simple_ril_info_set* info_set_ref);
extern void qmi_simple_ril_util_merge_info_set(qmi_simple_ril_info_set* info_set_ref_to, qmi_simple_ril_info_set* info_set_ref_from);
extern void qmi_simple_ril_util_remove_entry_from_info_set(qmi_simple_ril_info_set* info_set, int entry_idx);

struct qmi_simple_ril_core_pending_command_info;
typedef struct qmi_simple_ril_cmd_input_info
    {
        qmi_util_event_info event_header;

        char * command;
        qmi_simple_ril_info_set * info_set;

        qmi_simple_ril_info_set * instructions;
        char * attractor;

        struct qmi_simple_ril_core_pending_command_info* pending_command_info_owner;
    } qmi_simple_ril_cmd_input_info;

extern qmi_simple_ril_cmd_input_info* qmi_simple_ril_allocate_cmd(char * cmd_str);
extern qmi_simple_ril_cmd_input_info* qmi_simple_ril_allocate_cmd_with_parsing(char * cmd_str);
extern void qmi_simple_ril_destroy_cmd(qmi_simple_ril_cmd_input_info* cmd);

typedef struct qmi_simple_ril_cmd_ack_info
    {
        int error_code;
        qmi_util_request_id request_id;
    } qmi_simple_ril_cmd_ack_info;


typedef int (* qmi_simple_ril_command_handler) (qmi_simple_ril_cmd_input_info* cmd_params, qmi_simple_ril_cmd_ack_info* ack_info);


typedef struct qmi_simple_ril_cmd_completion_info
    {
        qmi_util_event_info event_header;
        
        char * command;
        qmi_util_request_id request_id;
        int error_code;
        qmi_simple_ril_info_set* info_set;
        int is_overriden;               

        int must_report_feedback;
        int must_exit;
        int must_purge;
        int must_drop_condition;
        int must_be_silent;
        int must_end_batch;
    } qmi_simple_ril_cmd_completion_info;


typedef struct qmi_simple_ril_performance_feedback_info
    {
        qmi_util_event_info event_header;
        
        int     error_code;
        char    * attractor;
        int     is_unsolicited;
        qmi_simple_ril_info_set* info_set;
    } qmi_simple_ril_performance_feedback_info;


extern qmi_simple_ril_cmd_completion_info* qmi_simple_ril_completion_info_allocate();
extern qmi_simple_ril_cmd_completion_info* qmi_simple_ril_completion_info_allocate_ex(qmi_simple_ril_cmd_input_info* cmd);
extern void qmi_simple_ril_e_completion_info_free(qmi_simple_ril_cmd_completion_info* comp_info);

typedef int (* qmi_simple_ril_command_completion_handler) (qmi_simple_ril_cmd_completion_info* ack_info);

extern void qmi_simple_ril_engine_initialize(qmi_util_event_pipe_info* ril_uplink_event_pipe, qmi_util_event_pipe_info** ril_downlink_event_pipe);
extern void qmi_simple_ril_engine_destroy();

extern void qmi_simple_ril_complete_request(qmi_simple_ril_cmd_completion_info* completion_info);
extern void qmi_simple_ril_complete_request_from_cmd_and_err(qmi_simple_ril_cmd_input_info* cmd_params, int error_code);



// ----------------- operations

typedef struct qmi_simple_ril_core_run_params
{
        int    is_break_on_any_failure;
        int    is_feedback_for_every_step;
        int    is_elaborate_failure_details;
        int    is_auto_terminate_when_none_pending;
        int    is_auto_purge;

} qmi_simple_ril_core_run_params;

struct qmi_simple_ril_core_operation_info;
typedef struct qmi_simple_ril_core_pending_command_info
{
        qmi_util_event_info event_header;

        qmi_simple_ril_cmd_input_info * original_cmd_input;

        struct qmi_simple_ril_core_operation_info * owner_operation;

        qmi_util_request_id pending_request_id;
        struct qmi_simple_ril_core_pending_command_info * next;

        char * command;

        int is_in_handler;

        int is_purge_command;

        int is_blocking_command;

        int is_end_batch_command;

        int is_must_report_feedback;

        int is_complete;
        int is_commenced;
        int result_error_code;
        int has_been_reported;

        qmi_simple_ril_info_set* result_info_set;
} qmi_simple_ril_core_pending_command_info;


typedef struct qmi_simple_ril_core_operation_info
{
        char * attractor;
        int    is_break_on_any_failure;
        int    is_feedback_for_every_step;
        int    is_elaborate_failure_details;
        int    is_auto_terminate_when_none_pending;
        int    is_auto_purge;

        int    is_purging;
        int    is_completing_batch;

        char* condition_being_awaited;
        char* condition_value_expected;

        qmi_simple_ril_core_pending_command_info  * pending_commands;
        struct qmi_simple_ril_core_operation_info * next;
} qmi_simple_ril_core_operation_info;

typedef struct qmi_simple_ril_core_operation_list_info
{
        qmi_simple_ril_core_run_params common_parameters;

        qmi_simple_ril_core_operation_info * operations;

        pthread_mutex_t         operation_list_guard;
        pthread_mutexattr_t     mtx_atr;
} qmi_simple_ril_core_operation_list_info;


void qmi_simple_ril_core_delete_pending_command_info_direct(qmi_simple_ril_core_operation_list_info* op_list, qmi_simple_ril_core_pending_command_info * pending_command_info);


qmi_simple_ril_core_operation_info * qmi_simple_ril_core_create_operation_info(qmi_simple_ril_core_operation_list_info* op_list, char* attractor_param);
qmi_simple_ril_core_operation_info * qmi_simple_ril_core_find_operation_by_attractor(qmi_simple_ril_core_operation_list_info* op_list, char * attractor);
void qmi_simple_ril_core_delete_operation_info_direct(qmi_simple_ril_core_operation_list_info* op_list, qmi_simple_ril_core_operation_info * op_info);

qmi_simple_ril_core_pending_command_info * qmi_simple_ril_core_find_pending_command_by_request_id(qmi_simple_ril_core_operation_list_info* op_list, qmi_util_request_id request_id);
qmi_simple_ril_core_pending_command_info * qmi_simple_ril_core_add_command_from_input_params_to_operation( qmi_simple_ril_core_operation_list_info* op_list, qmi_simple_ril_cmd_input_info * cmd_input  );

qmi_simple_ril_core_operation_list_info* qmi_simple_ril_core_create_operation_list_info(qmi_simple_ril_core_run_params* common_parameters);
void qmi_simple_ril_core_destroy_operation_list_info(qmi_simple_ril_core_operation_list_info* op_list);

qmi_simple_ril_performance_feedback_info* qmi_simple_ril_core_create_perfromance_feedback_info( qmi_simple_ril_info_set* info_set_reference_if_any, char * attractor_if_any );
void qmi_simple_ril_core_destroy_perfromance_feedback_info( qmi_simple_ril_performance_feedback_info* feedback_info );

// -- error codes
#define QMI_SIMPLE_RIL_ERR_NONE                         0
#define QMI_SIMPLE_RIL_ERR_ARG                          1
#define QMI_SIMPLE_RIL_ERR_INVALID_CONTEXT              2
#define QMI_SIMPLE_RIL_ERR_NO_RESOURCES                 3
#define QMI_SIMPLE_RIL_ERR_PURGE                        4
#define QMI_SIMPLE_RIL_NO_DATA                          5
#define QMI_SIMPLE_RIL_FALSE                            6

char* qmi_simple_ril_core_conv_err_cause_to_str(int cause);

#define CORE_EVT_CATEGORY_INCOMING_CMD_INFO             (0)
#define CORE_EVT_CATEGORY_COMPLETION_INFO               (1)
#define CORE_EVT_CATEGORY_FEEDBACK_POST                 (2)
#define CORE_EVT_CATEGORY_EXEC_PENDING_CMD              (3)
#define CORE_EVT_CATEGORY_INPUT_READY_IND               (4)
#define CORE_EVT_CATEGORY_TERMINATION                   (5)
#define CORE_EVT_CATEGORY_TERMINATION_TRANSLATE         (6)
#define CORE_EVT_CATEGORY_COND_VAR_CHANGED              (7)


#define CORE_COMMAND_INSTRUCTION_ENFORCE_RESPONSE       "enforce_response"


typedef struct qmi_simple_ril_core_cond_var_entry
{
        qmi_util_event_info event_header;

        struct qmi_simple_ril_core_cond_var_entry* next;

        char * cond_var_name;
        char * cond_var_value;

        int is_on_watch;
        char * watch_filter;

} qmi_simple_ril_core_cond_var_entry;

typedef struct qmi_simple_ril_core_cond_var_list
{
        qmi_simple_ril_core_cond_var_entry * root;

        pthread_mutex_t         cond_var_list_guard;
        pthread_mutexattr_t     mtx_atr;
} qmi_simple_ril_core_cond_var_list;

qmi_simple_ril_core_cond_var_entry* qmi_simple_ril_core_find_cond_var_entry_by_name( qmi_simple_ril_core_cond_var_list* cond_var_list, char* cond_var_name );
qmi_simple_ril_core_cond_var_entry* qmi_simple_ril_core_get_cond_var_entry_by_name( qmi_simple_ril_core_cond_var_list* cond_var_list, char* cond_var_name );
qmi_simple_ril_core_cond_var_entry* qmi_simple_ril_core_create_cond_var_entry( qmi_simple_ril_core_cond_var_list* cond_var_list, char* cond_var_name );
qmi_simple_ril_core_cond_var_list* qmi_simple_ril_core_create_cond_var_list( void );
void qmi_simple_ril_core_destroy_cond_var_list( qmi_simple_ril_core_cond_var_list* cond_var_list );

void qmi_simple_ril_core_set_cond_var( char* cond_var_name, char* cond_var_value );
#endif // QMI_SIMPLE_RIL_CORE_H
