/******************************************************************************
  @file    qmi_test_console.c
  @brief   The QMI Test console for simple RIL

  DESCRIPTION
  The QMI Test console for simple RIL

  ---------------------------------------------------------------------------

  Copyright (c) 2010 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>

#include "qmi_test_console.h"
#include "qmi_simple_ril_core.h"

typedef struct qmi_test_console_info
{
    qmi_test_console_downlink_msg_collector downlink_collector;
    qmi_test_console_uplink_msg_distributor uplink_distributor;
    qmi_test_console_shutdown_handler shutdown_handler;
    pthread_t console_main_thread_handler;
    pthread_t console_collector_thread_handler;
    qmi_util_event_pipe_info ril_uplink_pipe;
    qmi_util_event_pipe_info* ril_downlink_pipe;

    qmi_simple_ril_info_set * active_input;

    int input_cleared;

} qmi_test_console_info;

typedef struct qmi_test_console_input_route
    {
        qmi_util_event_info event_header;
        
        char * input_str;
    } qmi_test_console_input_route;


#define TEST_CON_CATEGORY_INPUT                 (1)
#define TEST_CON_CATEGORY_FEEDBACK              (CORE_EVT_CATEGORY_FEEDBACK_POST)
#define TEST_CON_CATEGORY_READY_FOR_INPUT       (CORE_EVT_CATEGORY_INPUT_READY_IND)
#define TEST_CON_CATEGORY_CHECK_POST            (100)
#define TEST_CON_CATEGORY_TERMINATION           (CORE_EVT_CATEGORY_TERMINATION)



static qmi_test_console_info test_console_info;

static void* qmi_test_console_main_thread_func(void * param);
static void* qmi_test_console_collector_thread_func(void * param);

static qmi_test_console_input_route* qmi_test_console_create_input_route(char * input_str);
static void qmi_test_console_destroy_input_route(qmi_test_console_input_route* input_route);


void qmi_test_console_initialize(qmi_test_console_downlink_msg_collector downlink_collector, 
                                 qmi_test_console_uplink_msg_distributor uplink_distributor,
                                 qmi_test_console_shutdown_handler shutdown_handler)
    {
    memset(&test_console_info, 0, sizeof(qmi_test_console_info));
    test_console_info.downlink_collector = downlink_collector;
    test_console_info.uplink_distributor = uplink_distributor;
    test_console_info.shutdown_handler = shutdown_handler;

    test_console_info.active_input = qmi_simple_ril_util_alloc_info_set();

    qmi_util_event_pipe_init_obj(&test_console_info.ril_uplink_pipe);
    }

void qmi_test_console_run(int is_modal_run)
    {
    void * exit_val;

    qmi_util_logln1("qmi_test_console_run", is_modal_run);
    qmi_simple_ril_engine_initialize(&test_console_info.ril_uplink_pipe, &test_console_info.ril_downlink_pipe);

    qmi_util_logln0("qmi_test_console_run thread creation");
    pthread_create(&test_console_info.console_main_thread_handler, NULL, qmi_test_console_main_thread_func, NULL);
    pthread_create(&test_console_info.console_collector_thread_handler, NULL, qmi_test_console_collector_thread_func, NULL);
    if (is_modal_run) 
        {
        pthread_join(test_console_info.console_main_thread_handler, &exit_val);
        pthread_kill(test_console_info.console_collector_thread_handler, SIGQUIT );
        }
    qmi_util_logln0("qmi_test_console_run completing");
    }

void* qmi_test_console_main_thread_func(void * param)
    {
    int go_on = TRUE;
    qmi_util_event_info* event_info;
    qmi_simple_ril_performance_feedback_info* feedback_info;
    qmi_simple_ril_cmd_input_info* cmd_input;
    qmi_test_console_input_route* input;
    char out_buf[256];
    char *out_buf_ptr[2] = { out_buf, NULL };

    char * input_line;
    char attractor_buf[1024];
    char rest_buf[1024];
    int parse_res;

    char * commands;
    char * attractor;

    char in_buf[1024];

    qmi_util_event_info* check_post_event;

    sleep(1);
    do
    {
        qmi_util_logln0("qmi_test_console_main_thread_func awaiting cmd" );
        qmi_util_event_pipe_wait_for_event(&test_console_info.ril_uplink_pipe, &event_info);
        if(event_info)
        {
        qmi_util_logln1("qmi_test_console_main_thread_func got cmd", event_info->category);
        
        switch ( event_info->category )
        {
            case TEST_CON_CATEGORY_READY_FOR_INPUT:

                test_console_info.input_cleared = TRUE;

                check_post_event = qmi_util_create_generic_event( TEST_CON_CATEGORY_CHECK_POST );
                if ( NULL != check_post_event )
                {
                    qmi_util_event_pipe_post_event(&test_console_info.ril_uplink_pipe, check_post_event);
                }

                qmi_util_destroy_generic_event( event_info );
                break;

            case TEST_CON_CATEGORY_CHECK_POST:

                if ( test_console_info.active_input->nof_entries > 0 && test_console_info.input_cleared )
                {
                    input_line = test_console_info.active_input->entries[ 0 ]; // FIFO

                    parse_res = sscanf(input_line, "[%s]%s", attractor_buf, rest_buf );

                    qmi_util_logln1s(".. parsing input line", input_line);

                    if (2 == parse_res)
                    {
                        commands  = rest_buf;
                        attractor = attractor_buf;
                    }
                    else
                    {
                        commands  = input_line;
                        attractor = NULL;
                    }

                    cmd_input = qmi_simple_ril_allocate_cmd_with_parsing( commands );
                    if (cmd_input)
                    {
                        cmd_input->attractor = qmi_util_str_clone( attractor );
                        qmi_util_event_pipe_post_event(test_console_info.ril_downlink_pipe, &cmd_input->event_header);
                    }
                    qmi_simple_ril_util_remove_entry_from_info_set( test_console_info.active_input, 0 ); // first entry 

                    test_console_info.input_cleared = FALSE;
                }

                qmi_util_destroy_generic_event( event_info );

                break;

            case TEST_CON_CATEGORY_INPUT:
                input = (qmi_test_console_input_route*)event_info;
                if (input->input_str )
                {
                    qmi_simple_ril_util_add_entry_to_info_set( test_console_info.active_input, input->input_str );    
                }
                qmi_test_console_destroy_input_route( input );

                check_post_event = qmi_util_create_generic_event( TEST_CON_CATEGORY_CHECK_POST );
                if ( NULL != check_post_event )
                {
                    qmi_util_event_pipe_post_event(&test_console_info.ril_uplink_pipe, check_post_event);
                }
                break;

            case TEST_CON_CATEGORY_FEEDBACK:
                feedback_info = (qmi_simple_ril_performance_feedback_info*) event_info;
                qmi_util_logln0("qmi_test_console_main_thread_func dumping msg from uplink");

               if ( feedback_info->attractor )
                {
                   sprintf(out_buf, "[%s]", feedback_info->attractor);
                   test_console_info.uplink_distributor(1, out_buf_ptr);
                }

                if ( feedback_info->info_set && feedback_info->info_set->nof_entries > 0 )
                {  
                    test_console_info.uplink_distributor(feedback_info->info_set->nof_entries, feedback_info->info_set->entries);
                }
                 
                qmi_simple_ril_core_destroy_perfromance_feedback_info( feedback_info );                
                break;
                

            case TEST_CON_CATEGORY_TERMINATION:
                qmi_simple_ril_engine_destroy();
                qmi_util_destroy_generic_event( event_info );
                go_on = FALSE;
                sleep(1);
                break;

            default:
                qmi_util_logln0("qmi_test_console_main_thread_func unhandled category");
                break;
        }
      }
    } while (go_on);
    
    if (test_console_info.shutdown_handler)
    {
        test_console_info.shutdown_handler(0);
    }


    return NULL;
    }

qmi_test_console_input_route* qmi_test_console_create_input_route(char * input_str)
{
    qmi_test_console_input_route* res = NULL;

    res = malloc( sizeof( *res ) );
    if ( res )
    {
        memset( res, 0, sizeof( *res ) );
        res->event_header.category = TEST_CON_CATEGORY_INPUT;
        res->input_str = qmi_util_str_clone( input_str );
    }

    return res;
}

void qmi_test_console_destroy_input_route(qmi_test_console_input_route* input_route)
{
    if ( input_route )
    {
        qmi_util_str_destroy( input_route->input_str );
        free( input_route );
    }
}

void* qmi_test_console_collector_thread_func(void * param)
    {
    qmi_simple_ril_cmd_completion_info* cmd_comp;
    qmi_simple_ril_cmd_input_info* cmd_input;
    char in_buf[1024];
    int res_collect;

    qmi_test_console_input_route* input;

    do
    {
        res_collect = test_console_info.downlink_collector( in_buf );
        if (QMI_SIMPLE_RIL_ERR_NONE == res_collect)
        {
            input = qmi_test_console_create_input_route( in_buf );
            if ( input  )
            {
                qmi_util_event_pipe_post_event(&test_console_info.ril_uplink_pipe, &input->event_header);
            }
        }
        else
        { // stall
            qmi_util_logln0("qmi_test_console_collector_thread_func going dormant" );
            sleep( 0xFFFFFFFA );
        }

    } while (TRUE);
  

    return NULL;
    }

