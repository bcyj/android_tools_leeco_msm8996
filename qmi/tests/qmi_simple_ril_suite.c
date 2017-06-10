/******************************************************************************
  @file    qmi_simple_ril_suite.c
  @brief   QMI simple RIL test suite, main file

  DESCRIPTION
  QMI simple RIL test suite, main
  Initialization and shutdown
  ---------------------------------------------------------------------------

  Copyright (c) 2010 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>
#ifndef FEATURE_QMI_ANDROID
#include <syslog.h>
#endif

#include "qmi_client_utils.h"
#include "qmi_test_console.h"
#include "qmi_simple_ril_core.h"

/* For QMI PBM TEST (begin) */
#include "phonebook_manager_service_v01.h"
#define PBM_MSG_DEFAULT_TIMEOUT 5000
#define MENU_EXIT_CHARACTER 'r'

#ifndef FEATURE_QMI_ANDROID
#define SYSLOG_PREFIX "SimpleRIL"
#endif

static int pbm_main();

/*qmi message library handle*/
static int qmi_handle = QMI_INVALID_CLIENT_HANDLE;
/* For QMI PBM TEST (end) */

static FILE* collector_input_handle;
static FILE* distributor_output_handle;
pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;

FILE* distributor_data_handle;
extern int is_sig_pipe;

static int qmi_simple_ril_suite_downlink_msg_collector (char* downlink_msg);
static void qmi_simple_ril_suite_uplink_msg_distributor(int nof_strings, char** uplinklink_msg);
static void qmi_simple_ril_suite_shutdown_handler (int cause);

#define MODEM_KEY "modem="
#define MODEM_PORT_KEY "modem_port="
#define INPUT_KEY "input="
#define OUTPUT_KEY "output="
#define DATA_KEY "data="

void catch_sig_pipe(int i)
{
    is_sig_pipe = 1;
}


int main(int argc, char *argv[])
    {
    int i;
    int pbm = 0;
    int modemset = 0;

    char * input = NULL;
    char * output = NULL;
    char * data = NULL;

#ifndef FEATURE_QMI_ANDROID
    /* If logging to syslog, initialize logging */
    openlog(SYSLOG_PREFIX, LOG_PID | LOG_NDELAY, LOG_USER);
#endif

    printf("*** Simple RIL / QMI test console v2.0 ***\n");

    for(i = 1; i < argc; i++) 
    {
        if(0 == strcmp(argv[i], "pbm")) 
        {
            pbm = TRUE;
        }
        else if (0 == strcmp(argv[i], "debug")) 
        {
            qmi_util_log_enable(1);
        }
        #ifdef FEATURE_QMI_ANDROID
        else if (0 == strncmp(argv[i], MODEM_KEY, strlen(MODEM_KEY))) 
        {
            if(0 == qmi_util_set_modem(argv[i] + strlen(MODEM_KEY))) 
                {
                modemset = 1;
                }
        }
        #endif
        else if (0 == strncmp(argv[i], MODEM_PORT_KEY, strlen(MODEM_PORT_KEY))) 
        {
            qmi_util_set_modem_port(argv[i] + strlen(MODEM_PORT_KEY));
            modemset = 1;
        }
        else if (0 == strncmp(argv[i], INPUT_KEY, strlen(INPUT_KEY))) 
        {
            input = argv[i] + strlen(INPUT_KEY);
        }
        else if (0 == strncmp(argv[i], OUTPUT_KEY, strlen(OUTPUT_KEY))) 
        {
            output = argv[i] + strlen(OUTPUT_KEY);
        }
        else if (0 == strncmp(argv[i], DATA_KEY, strlen(DATA_KEY)))
        {
            data = argv[i] + strlen(DATA_KEY);
        }
    }

    #ifdef FEATURE_QMI_ANDROID
    if(0 == modemset) 
    {
        qmi_util_set_modem("default");
    }
    #endif

    if(1 == pbm) 
    {
        return pbm_main();
    }

    qmi_util_logln2s("sril input output", input, output);

    signal(SIGPIPE, catch_sig_pipe);

    if ( NULL == input )
    {
        collector_input_handle = stdin;
    }
    else
    {
        collector_input_handle = fopen( input, "r" );
        if ( NULL == collector_input_handle )
        { // fallback
            qmi_util_logln0( "fallback input to stdin " );
            collector_input_handle = stdin;
        }
    }
    if ( NULL == output )
    {
        distributor_output_handle = stdout;
    }
    else
    {
        distributor_output_handle = fopen( output, "w" );
        if ( NULL == distributor_output_handle )
        {
            qmi_util_logln0( "fallback output to stdout " );
            distributor_output_handle = stdout;
        }
    }
    if ( NULL == data )
    {
        distributor_data_handle = stdout;
    }
    else
    {
        distributor_data_handle = fopen( data, "w" );
        if ( NULL == distributor_data_handle )
        {
            qmi_util_logln0( "fallback data to stdout " );
            distributor_data_handle = stdout;
        }
    }

    fprintf(distributor_output_handle, "*** starting console ***\n");
    qmi_test_console_initialize(qmi_simple_ril_suite_downlink_msg_collector,
                                qmi_simple_ril_suite_uplink_msg_distributor,
                                qmi_simple_ril_suite_shutdown_handler);
    fprintf(distributor_output_handle, "Type in simple RIL commands and observe feedback \n");
    fprintf(distributor_output_handle, "Use 'quit' command to terminate simple RIL \n");
    fflush(distributor_output_handle);
    qmi_test_console_run(TRUE);
    return 0;
    }

int qmi_simple_ril_suite_downlink_msg_collector (char* downlink_msg)
    {
    int c;
    int nof_collected = 0;
    char *cur = downlink_msg;
    int res;
    while ( (c = fgetc(collector_input_handle)) && (c != '\n') && (c!= EOF) )
    {
        *cur = c;
        cur++;
        nof_collected++;
    }
    *cur = 0;
    if ( 0 == nof_collected && EOF == c )
    {
        res = QMI_SIMPLE_RIL_NO_DATA;
    }
    else
    {
        res = QMI_SIMPLE_RIL_ERR_NONE;
    }
    return res;
    }

FILE * qmi_simple_ril_suite_output_handle()
{
    return distributor_output_handle;
}

void qmi_simple_ril_suite_uplink_msg_distributor(int nof_strings, char** uplinklink_msg)
{
    int idx;
    FILE * file = qmi_simple_ril_suite_output_handle();
    pthread_mutex_lock( &log_mutex );
    for (idx = 0; idx < nof_strings; idx++) 
    {
        // this is where writing to output file/stdout happens
        fprintf(file, "%s\n", uplinklink_msg[idx]);
        if(stdout != file)  {
            fprintf(stdout, "%s\n", uplinklink_msg[idx]);
        }
#ifndef FEATURE_QMI_ANDROID
        syslog(LOG_DEBUG,  "%s", uplinklink_msg[idx]);
#endif
    }
    fflush (file);
    pthread_mutex_unlock( &log_mutex );
}

void qmi_simple_ril_suite_shutdown_handler (int cause)
    {
    qmi_util_logln1("qmi_simple_ril_suite_shutdown_handler", cause);
    }





/* QMI PBM Test code begins */

/* #include "qmi.h"
#include "qmi_client.h"
*/

/* Function prototypes */
static qmi_client_error_type modem_pbm_init();
static char display_menu();
static void get_all_pb_info();
static qmi_client_error_type get_pb_info(pbm_get_pb_capabilities_req_msg_v01*,
                        pbm_get_pb_capabilities_resp_msg_v01*);
static void process_user_choice(char);
static void demand_user_input();
static void print_pb_info(pbm_get_pb_capabilities_resp_msg_v01);
static void read_adn_records();
static void write_adn_records();
static void handle_read_indication(void*, int);

/* Callbacks */
static void pbm_unsol_ind ( qmi_client_type,
                           unsigned long,
                           unsigned char*,
                           int,
                           void*
                          );
static void pbm_write_cb ( qmi_client_type,
                           unsigned long,
                           void*,
                           int,
                           void*,
                           qmi_client_error_type
                          );

/* Utilities */
static int getWideStringLength(char*);
static void convertToNarrowString(char*, char*);
static void convertToWideString(char*, char*);
static void stripLineFeed(char*);

/* Global variables */
static qmi_client_type pbm_user_handle;
static qmi_idl_service_object_type pbm_service_obj;
static pbm_write_record_resp_msg_v01 pbm_write_resp;

/* Main function for performing PBM Test */
static int pbm_main()
{
    qmi_client_error_type modem_init_result;
    char dummy[10];

    modem_init_result = modem_pbm_init();
    if (modem_init_result != QMI_NO_ERR) 
    {
        printf("Failed to initialize PBM modem subsystem, error code=%d \n", 
               modem_init_result);
        printf("Press any key to continue \n");
        fgets(dummy, sizeof(dummy), stdin);
        return -1;
    }
    printf("PBM modem subsystem successfully initialized \n");
    /* This function never returns unless the user presses the exit character at the menu */
    demand_user_input();

    /* Release the client */
    modem_init_result = qmi_client_release(pbm_user_handle);
    if (modem_init_result < 0 )
    {
        printf("Release not successful \n");
    }
    else
    {
        printf("QMI client release successful \n");
    }
    if (qmi_handle >= 0)
    {
      qmi_release(qmi_handle);
    }
    printf("QMI release done.........\n");

    return 0;
}

/* This function is responsible for initializing the QMI connection to QMI PBM service */
static qmi_client_error_type modem_pbm_init()
{
    qmi_client_error_type rc;

    /* Initialize the qmi datastructure(Once per process ) */
    qmi_handle = qmi_init(NULL,NULL);
    if (qmi_handle < 0)
    {

      printf("Error: Qmi message library not initialized\n");
      return qmi_handle;
    }
    /*Get the pbm service object */
    pbm_service_obj = pbm_get_service_object_v01();

    /* Initialize a connection to first QMI control port */
    rc = qmi_client_init("rmnet0",
                        pbm_service_obj,
                        pbm_unsol_ind,
                        NULL,
                        &pbm_user_handle
                        );
    if (rc != QMI_NO_ERR ) 
    {
         printf("Error: connection not Initialized...Error Code:%d\n",rc);
         rc = qmi_client_release(pbm_user_handle);
         if (rc < 0 ) 
         {
            printf("Release not successful \n");
         }
         else
         {
            printf("QMI client release successful \n");
         }
    }
    else
    {
        printf("Connection Initialized....User Handle:%x\n", pbm_user_handle);
    }
    return rc;
}

/* This function displays various menu options to the user and returns the user's choice */
static char display_menu()
{
    char user_choice[3];
    printf("\n\n***************SIM Phonebook Test Menu***************\n\n");
    printf("Press '1' for retrieving storage information for various phonebooks\n");
    printf("Press '2' for reading entries from ADN phonebook\n");
    printf("Press '3' for writing entries to ADN phonebook\n");
    printf("Press '%c' to exit this program\n", MENU_EXIT_CHARACTER);
    printf("\n\nEnter choice: ");
    fgets(user_choice, sizeof(user_choice), stdin);
    printf("\n");
    return user_choice[0];
}

/* This function retrieves information for ADN, FDN, BDN Phonebooks via QMI PB. Other PBs can be similarly added */
static void get_all_pb_info()
{
    pbm_get_pb_capabilities_req_msg_v01 pbm_info_req;
    pbm_get_pb_capabilities_resp_msg_v01 pbm_info_resp;

    /* memset the request response structure to zero */
    memset(&pbm_info_req,0,sizeof(pbm_info_req));
    memset(&pbm_info_resp,0,sizeof(pbm_info_resp));

    pbm_info_req.phonebook_info.session_type = PBM_SESSION_TYPE_GLOBAL_PB_SLOT1_V01;

    pbm_info_req.phonebook_info.pb_type = PBM_PB_TYPE_ADN_V01;
    if (get_pb_info(&pbm_info_req, &pbm_info_resp) == QMI_NO_ERR)
        print_pb_info(pbm_info_resp);

    pbm_info_req.phonebook_info.pb_type = PBM_PB_TYPE_FDN_V01;
    memset(&pbm_info_resp,0,sizeof(pbm_info_resp));
    if (get_pb_info(&pbm_info_req, &pbm_info_resp) == QMI_NO_ERR)
        print_pb_info(pbm_info_resp);

    pbm_info_req.phonebook_info.pb_type = PBM_PB_TYPE_BDN_V01;
    memset(&pbm_info_resp,0,sizeof(pbm_info_resp));
    if (get_pb_info(&pbm_info_req, &pbm_info_resp) == QMI_NO_ERR)
        print_pb_info(pbm_info_resp);
}

/* This function is a wrapper around actual call for retrieving Phonebook information via QMI PBM */
static qmi_client_error_type get_pb_info(pbm_get_pb_capabilities_req_msg_v01* p_pbm_info_req,
                        pbm_get_pb_capabilities_resp_msg_v01* p_pbm_info_resp)
{
    qmi_client_error_type rc;
    printf("Size of request is %d\n", sizeof(*p_pbm_info_req));
    printf("Size of response is %d\n", sizeof(*p_pbm_info_resp));
    printf("Information being sent is session type = %d, PB type = %d\n", p_pbm_info_req->phonebook_info.session_type, 
                                                                          p_pbm_info_req->phonebook_info.pb_type);
    rc = qmi_client_send_msg_sync(pbm_user_handle,
                                  QMI_PBM_GET_PB_CAPABILITIES_REQ_V01,
                                  p_pbm_info_req,
                                  sizeof(*p_pbm_info_req),
                                  p_pbm_info_resp,
                                  sizeof(*p_pbm_info_resp),
                                  PBM_MSG_DEFAULT_TIMEOUT);

    if (rc != QMI_NO_ERR) 
    {
        printf("Getting PB info for PB type %d returned error %d\n", (int)(p_pbm_info_req->phonebook_info.pb_type), (int)rc);
    }
    return rc;
}

/* This function calls the appropriate handler depending on user's choice to the menu options */
static void process_user_choice(char c)
{
    switch(c)
    {
        case '1': 
            get_all_pb_info();
            break;
        case '2':
            read_adn_records();
            break;
        case '3':
            write_adn_records();
            break;
        default: 
            printf("Invalid choice entered. Please try again\n");
            break;
    }   
}

/* This function shows the menu and processes the option from user. This function doesn't return until the user enters the exit character
   as a response to the menu options */
static void demand_user_input()
{
    while (1) 
    {
        char c = display_menu();
        if (c == MENU_EXIT_CHARACTER) 
        {
            break;
        }
        process_user_choice(c);
    }
}

/* This function prints information regarding a Phonebook whose response structure is passed. This is called with QMI PBM's response structure */
static void print_pb_info(pbm_get_pb_capabilities_resp_msg_v01 pbm_info_resp)
{
    printf("\nPB Info received from QMI PBM\n");
    printf("\n-----------------------------\n");
    if (pbm_info_resp.resp.result != QMI_RESULT_SUCCESS_V01)
    {
        printf("Response from QMI PBM returned error %d\n", pbm_info_resp.resp.error);
        return;
    }

    if (pbm_info_resp.capability_basic_info_valid) 
    {
        printf("Session Type : %d\n", (int)pbm_info_resp.capability_basic_info.session_type);
        printf("Phonebook Type: %d\n", (int) pbm_info_resp.capability_basic_info.pb_type);
        printf("Used Records: %d\n", (int) pbm_info_resp.capability_basic_info.used_records);
        printf("Max Records: %d\n", (int)pbm_info_resp.capability_basic_info.max_records);
        printf("Max Number Length: %d\n", (int) pbm_info_resp.capability_basic_info.max_num_len);
        printf("Max Name Length: %d\n", (int) pbm_info_resp.capability_basic_info.max_name_len);
    }

    if (pbm_info_resp.group_capability_valid) 
    {
        printf("Max Groups Possible: %d\n", pbm_info_resp.group_capability.max_grp);
        printf("Max Group Alpha String Length: %d\n", pbm_info_resp.group_capability.max_grp_tag_len);
    }

    if (pbm_info_resp.ad_num_capability_valid)
    {
        printf("Max additional numbers: %d\n", pbm_info_resp.ad_num_capability.max_ad_num);
        printf("Max additional number length: %d\n", pbm_info_resp.ad_num_capability.max_ad_num_len);
        printf("Max additional number alpha string length: %d\n", pbm_info_resp.ad_num_capability.max_ad_num_tag_len);
    }

    if (pbm_info_resp.email_capability_valid) 
    {
        printf("Max emails: %d\n", pbm_info_resp.email_capability.max_email);
        printf("Max email length: %d\n", pbm_info_resp.email_capability.max_email_len);
    }

    if (pbm_info_resp.max_second_name_len) 
    {
        printf("Max second name length: %d\n", pbm_info_resp.max_second_name_len);
    }
    
    if (pbm_info_resp.is_hidden_entry_supported_valid) 
    {
        printf("Hidden entry support: %d\n", pbm_info_resp.is_hidden_entry_supported);
    }
}

/* This function is responsible for reading **all** records from the ADN Phonebook via QMI PBM */
static void read_adn_records()
{
    pbm_read_records_req_msg_v01 pbm_read_req;
    pbm_read_records_resp_msg_v01 pbm_read_resp;
    pbm_get_pb_capabilities_req_msg_v01 pbm_info_req;
    pbm_get_pb_capabilities_resp_msg_v01 pbm_info_resp;

    qmi_client_error_type rc;
    
    /* memset the request response structure to zero */
    memset(&pbm_info_req,0,sizeof(pbm_info_req));
    memset(&pbm_info_resp,0,sizeof(pbm_info_resp));

    pbm_info_req.phonebook_info.session_type = PBM_SESSION_TYPE_GLOBAL_PB_SLOT1_V01;
    pbm_info_req.phonebook_info.pb_type = PBM_PB_TYPE_ADN_V01;

    if (get_pb_info(&pbm_info_req, &pbm_info_resp) == QMI_NO_ERR)
    {
        if (pbm_info_resp.capability_basic_info_valid)
        {
            /* memset the request response structure to zero */
            memset(&pbm_read_req,0,sizeof(pbm_read_req));
            memset(&pbm_read_resp,0,sizeof(pbm_read_resp));

            pbm_read_req.record_info.pb_type = PBM_PB_TYPE_ADN_V01;
            pbm_read_req.record_info.session_type = PBM_SESSION_TYPE_GLOBAL_PB_SLOT1_V01;
            pbm_read_req.record_info.record_start_id = 1;
            pbm_read_req.record_info.record_end_id = pbm_info_resp.capability_basic_info.max_records;

            rc = qmi_client_send_msg_sync(pbm_user_handle,
                                          QMI_PBM_READ_RECORDS_REQ_V01,
                                          &pbm_read_req,
                                          sizeof(pbm_read_req),
                                          &pbm_read_resp,
                                          sizeof(pbm_read_resp),
                                          PBM_MSG_DEFAULT_TIMEOUT);
            if (rc != QMI_NO_ERR)
            {
                printf("Read request for ADN failed with error code = %d\n", (int)rc);
                return;
            }
            else
            {
                if ((pbm_read_resp.resp.result == QMI_RESULT_SUCCESS_V01) && (pbm_read_resp.num_of_recs_valid))
                {
                    if (pbm_read_resp.num_of_recs == 0)
                    {
                        printf("There are no entries filled in ADN, nothing to read\n");
                    }
                    else
                    {
                        printf("There are %d filled entries in ADN\n", pbm_read_resp.num_of_recs);
                    }
                }                    
            }
        }
        else
        {
            printf(" Error: Information received for ADN did not return total number of records\n");
            return;
        }
    }
    else
    {
        return;
    }
}

/* This function asks the user for name and number to be written onto the ADN phonebook and writes a new contact in ADN with the
   specified name and number. The edit function can be written similary but instead of passing record_id as 0, the client (apps) side
   would pass the record_id (i.e. index in SIM Phonebook) of the record they would like to edit */
static void write_adn_records()
{
    qmi_client_error_type rc;
    char name[(QMI_PBM_NAME_MAX_LENGTH_V01/2) + 1];
    pbm_write_record_req_msg_v01 pbm_write_req;
    qmi_txn_handle txn_handle;
    int i;

    /* memset the request response structure to zero */
    memset(&pbm_write_req,0,sizeof(pbm_write_req));
    memset(&pbm_write_resp,0,sizeof(pbm_write_resp));
        
    printf("Enter the name of the new contact to be added (MAX length = %d):", QMI_PBM_NAME_MAX_LENGTH_V01/2);
    fgets(name, (QMI_PBM_NAME_MAX_LENGTH_V01/2) + 1, stdin);
    stripLineFeed(name);
    convertToWideString(name, pbm_write_req.record_information.name);
    pbm_write_req.record_information.name_len = 2 * getWideStringLength(pbm_write_req.record_information.name);
    printf("Enter the number of the new contact to be added (MAX length = %d):", QMI_PBM_NUMBER_MAX_LENGTH_V01);
    fgets(pbm_write_req.record_information.number, QMI_PBM_NUMBER_MAX_LENGTH_V01, stdin);
    stripLineFeed(pbm_write_req.record_information.number);
    pbm_write_req.record_information.number_len = strlen(pbm_write_req.record_information.number);
    pbm_write_req.record_information.phonebook_type = PBM_PB_TYPE_ADN_V01;
    pbm_write_req.record_information.record_id = 0;
    pbm_write_req.record_information.session_type = PBM_SESSION_TYPE_GLOBAL_PB_SLOT1_V01;

    rc =  qmi_client_send_msg_async(pbm_user_handle,
                                    QMI_PBM_WRITE_RECORD_REQ_V01,
                                    &pbm_write_req,
                                    sizeof(pbm_write_req),
                                    &pbm_write_resp,
                                    sizeof(pbm_write_resp),
                                    &pbm_write_cb,
                                    NULL,
                                    &txn_handle);
    if (rc != QMI_NO_ERR)
    {
        printf("Error: Sending asynchronous request for writing record failed with error code %d\n", (int)rc);
        return;
    }
    printf("Sending asynchronous request for writing record was successfull\n");
}

/* When a read request is issued to QMI PBM, the result comes as a read indication. This function handles such an indication and prints
   out the contents of the records present in the indication. In practice, such functions receiving indications should be kept as small/fast
   as possible. If necessary, a separate thread should be signalled which will do the hefty work leaving this function to exit promptly. Also, no request
   QMI can be issued from within the context of a callback or indication function */
static void handle_read_indication(void* payload, int payload_len)
{

    pbm_record_read_ind_msg_v01 pbm_read_ind;
    char* narrow_string = NULL;
    uint i = 0;
    int len;

    memset(&pbm_read_ind, 0, sizeof(pbm_read_ind));
    {
        memcpy(&pbm_read_ind, payload, payload_len);
        printf("Number of record instances in the read indication: %d\n", pbm_read_ind.basic_record_data.record_instances_len);
        for (i = 0; i < pbm_read_ind.basic_record_data.record_instances_len; i++)
        {
            printf("\n\nIndex of record in ADN: %d\n", pbm_read_ind.basic_record_data.record_instances[i].record_id);
            len = getWideStringLength(pbm_read_ind.basic_record_data.record_instances[i].name);
            if (len > 0)
            {
                narrow_string = malloc(len+1);
                if (narrow_string != NULL)
                {
                    convertToNarrowString(pbm_read_ind.basic_record_data.record_instances[i].name, narrow_string);
                    printf("Name in the record: %s\n", narrow_string);
                    free(narrow_string);
                    narrow_string = NULL;
                }
            }
            printf("Number in the record: %s\n", pbm_read_ind.basic_record_data.record_instances[i].number);
            printf("Number type in the record: %d\n", (int)pbm_read_ind.basic_record_data.record_instances[i].num_type);
            printf("Number plan in the record: %d\n", (int)pbm_read_ind.basic_record_data.record_instances[i].num_plan);
        }
    }
}

/* Callbacks */
/* This function handles unsolicited indications from QMI PBM. This function is specified during initialization with QMI PBM. In our case, this function receives
   indications and ignores all of them unless they are Read record indications in which case it calls the appropriate handler. In practice, such functions receiving indications should be kept as small/fast
   as possible. If necessary, a separate thread should be signalled which will do the hefty work leaving this function to exit promptly. Also, no request
   QMI can be issued from within the context of a callback or indication function */
static void pbm_unsol_ind ( qmi_client_type                user_handle,
                            unsigned long                  msg_id,
                            unsigned char                  *ind_buf,
                            int                            ind_buf_len,
                            void                           *ind_cb_data
                  )
{
    uint decoded_payload_len = 0;
    qmi_client_error_type rc;
    void* decoded_payload;


    qmi_idl_get_message_c_struct_len(pbm_service_obj,
                                QMI_IDL_INDICATION, 
                                msg_id,
                                &decoded_payload_len); 

    decoded_payload = malloc(decoded_payload_len);
    if (decoded_payload == NULL)
    {
        printf("Error: Unable to allocate memory\n");
    }
    else
    {
        rc = qmi_client_message_decode(pbm_user_handle,
                                  QMI_IDL_INDICATION,
                                  msg_id,
                                  ind_buf,
                                  ind_buf_len,
                                  decoded_payload,
                                  decoded_payload_len);
    
        if (rc != QMI_NO_ERR)
        {
            printf("Error: Decoding unsolicited indication with id = %ld, returned in error = %d\n", msg_id, (int)rc);
        }
        else
        {
            if (msg_id == QMI_PBM_RECORD_READ_IND_V01)
            {
                handle_read_indication(decoded_payload, decoded_payload_len);
            }
            else
            {
                /* ignore all other indications */
            }
        }
        free(decoded_payload);
    }
}

/* This function handles the callback that comes as a result of asynchronous request to QMI PBM for writing a new record in ADN phonebook.
   In practice, such functions receiving indications should be kept as small/fast as possible. If necessary, a separate thread should be
   signalled which will do the hefty work leaving this function to exit promptly. Also, no request QMI can be issued from within the
   context of a callback or indication function */
static void pbm_write_cb ( qmi_client_type              user_handle,
                           unsigned long                msg_id,
                           void                         *resp_c_struct,
                           int                          resp_c_struct_len,
                           void                         *resp_cb_data,
                           qmi_client_error_type        transp_err
                          )
{
    pbm_write_record_resp_msg_v01* p_pbm_write_resp;
    
    if (transp_err !=  QMI_NO_ERR)
    {
        printf("Error: QMI PBM request for writing failed with error code %d\n", (int) transp_err);
    }
    else
    {
        p_pbm_write_resp = (pbm_write_record_resp_msg_v01*) resp_c_struct;
        if (p_pbm_write_resp->resp.result == QMI_RESULT_SUCCESS_V01)
        {
            printf("Writing a record using PBM was successful\n");
            if (p_pbm_write_resp->record_id_valid)
            {
                printf("New record was added at index %d\n", p_pbm_write_resp->record_id);
            }
        }
        else
        {
            printf("Writing a record using PBM failed with error code %d\n", p_pbm_write_resp->resp.error);
        }        
    }
}

/*Utilities*/
/* This function returns the length of a string encoded in UCS-2 format. This function is required because the names in ADN phonebook are
   stored in UCS-2 format and QMI PBM expects those strings to be sent between the processors in that format as well. The terminating code point
   in UCS-2 is a 16 bit character will all bits set to 0 (i.e. 2 consecutive null octets). This function is equivalent to strlen for ASCII strings */
static int getWideStringLength(char* wide_string)
{
    int i = 0;
    char first, second;

    if (wide_string == NULL)
    {
        return 0;
    }

    for (i = 0; ((wide_string[i] != '\0') || (wide_string[i+1] != '\0')); i+=2)
    {}

    return i/2;
}

/* This function converts a string from UCS-2 format (wide_string) to ASCII format (narrow_string). The first parameter is the wide string (i.e source)
   and the second parameter is the narrow string (i.e. destination). It is assumed that enough memory has been allocated by the callee for destination
   before calling this function */
static void convertToNarrowString(char* wide_string, char* narrow_string)
{
    int i, len;

    if (wide_string == NULL)
    {
        narrow_string[0] = '\0';
        return;
    }

    len = getWideStringLength(wide_string);

    for (i = 0; i < len; i++)
    {
         // Copy over the first (MSB) octet of the code point
        narrow_string[i] = wide_string[2*i];
    }
    narrow_string[len] = '\0';
}

/* This function converts a string from ASCII format (narrow_string) to UCS-2 format (wide_string). The first parameter is the narrow string (i.e source)
   and the second parameter is the wide string (i.e. destination). It is assumed that enough memory has been allocated by the callee for destination
   before calling this function */
static void convertToWideString(char* narrow_string, char* wide_string)
{
    int i, len;

    if (narrow_string == NULL)
    {
        wide_string[0] = '\0';
        wide_string[1] = '\0';
        return;
    }
    
    len = strlen(narrow_string);
    for (i = 0; i < len; i++)
    {
        wide_string[2*i] = narrow_string[i];
        wide_string[2*i+1] = '\0';
    }
    wide_string[2*len] = '\0';
    wide_string[2*len + 1] = '\0';
}

/* This function strips off any stray linefeed at the end of the buffer containing user input
   because of line end inconsistencies between Unix like systems and Windows */
static void stripLineFeed(char* str)
{
    int i;

    for (i = 0; str[i] != '\0'; i++)
    {
        if (str[i] == '\n')
        {
            str[i] = '\0';
        }
    }
}


