/******************************************************************************
  @file    qcril_qmi_coex.c
  @brief   qcril qmi - lte coex

  DESCRIPTION
    Handles RIL requests, Callbacks, indications for LTE COEX support.

  ---------------------------------------------------------------------------

  Copyright (c) 2012 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
******************************************************************************/


/*===========================================================================

                           INCLUDE FILES

===========================================================================*/

#include <fcntl.h>
#include <signal.h>
#include "qcril_qmi_coex.h"
#include "coexistence_manager_v01.h"
#include "qcril_data_client.h"
#include "qcril_qmi_nas.h"


#define COEX_LTE_RANGE_NUMBER                                   (10)
#define COEX_LTE_RANGE_DEL_COMMA                                ','
#define COEX_LTE_RANGE_DEL_END                                  '\0'
#define COEX_LTE_FREQUENCY_RANGE                                "persist.radio.coex_freq"
#define COEX_LTE_GSM_CHANNEL_RANGE                              "persist.radio.coex_channel"
#define COEX_LTE_GSM_CHANNEL_INVALID                            -1
#define COEX_LTE_GSM_CHANNEL_START                              1
#define COEX_LTE_GSM_CHANNEL_END                                14
#define COEX_LTE_QMI_HEADER_SIZE                                7

#define COEX_LTE_RRC_INVALID_FDD_FREQUENCY  -1
#define COEX_LTE_RRC_MIN_TDD_DL_EARFN       36000
#define COEX_LTE_RRC_MAX_TDD_DL_EARFN       45589
#define COEX_LTE_PORT_RETRY_INTERVAL_SEC    1
#define COEX_LTE_PORT_RETRY_MAX_ATTEMPTS    100
#define COEX_LTE_RIVA_MASK                  0x0028

#define COEX_GSM_MIN_ARFCN                  (0)
#define COEX_GSM_MAX_ARFCN                  (1023)
#define COEX_GSM_INVALID_FREQUENCY          (-1)

//coex riva dedicated thread handling
#define COEX_RIVA_LOCK()      pthread_mutex_lock(&coex_lte_gsm_range_info.coex_riva_mutex);
#define COEX_RIVA_UNLOCK()    pthread_mutex_unlock(&coex_lte_gsm_range_info.coex_riva_mutex);
#define COEX_RIVA_WAIT()      qcril_qmi_coex_riva_thread_condition_wait_helper();
#define COEX_RIVA_SIGNAL()    pthread_cond_signal(&coex_lte_gsm_range_info.coex_riva_cond_var);

/*===========================================================================

                    INTERNAL DEFINITIONS AND TYPES

===========================================================================*/

typedef enum
{
    QMI_RIL_RIVA_COEX_SIGNAL_COND_NONE = 0,
    QMI_RIL_RIVA_COEX_SIGNAL_COND_ABORT,
} coex_riva_signal_cond_e_type;

typedef struct
{
    int freq_ranges[COEX_LTE_RANGE_NUMBER];
    int lte_channel_ranges[COEX_LTE_RANGE_NUMBER];
    int good_channel_start;
    int good_channel_end;
    int gsm_bad_channel_start;
    int gsm_bad_channel_end;
    int gsm_good_channel_start;
    int gsm_good_channel_end;
    int lte_bad_channel_start;
    int lte_bad_channel_end;
    int lte_good_channel_start;
    int lte_good_channel_end;
    int smd_riva_fp;
    qmi_idl_service_object_type coex_manager_sv_object;
    int lte_downlink_freq;
    int lte_downlink_active_channel;
    int lte_uplink_freq;
    int lte_uplink_active_channel;
    int is_lte_active;
    int gsm_downlink_freq;
    int gsm_downlink_active_channel;
    int gsm_uplink_freq;
    pthread_t riva_thread_id;
    pthread_mutex_t                             coex_riva_mutex;
    pthread_mutexattr_t                         coex_riva_mutex_atr;
    pthread_cond_t                              coex_riva_cond_var;
    coex_riva_signal_cond_e_type                coex_riva_signal_cond;
}coex_lte_gsm_range_info_type;

static coex_lte_gsm_range_info_type coex_lte_gsm_range_info;

static void qcril_qmi_coex_util_convert_lte_active_channel_to_frequencies
(
    uint16_t active_channel,
    int     *downlink,
    int     *uplink,
    int     *uplink_active_channel
);

static void qcril_qmi_coex_util_convert_gsm_active_channel_to_frequencies
(
    uint16_t                 active_channel,
    nas_active_band_enum_v01 active_band,
    int                     *downlink,
    int                     *uplink
);

static void qcril_qmi_coex_util_evaluate_lte_coex_range(char * input_range_ptr,int * lte_coex_range_ptr, int lte_coex_range_length);
static void qcril_qmi_coex_util_fillup_default_lte_coex_frequencies(int * lte_coex_range_ptr);
static void qcril_qmi_coex_util_fillup_default_lte_coex_channels(int * lte_coex_range_ptr);
static int qcril_qmi_coex_evaluate_channel_set
(
    int  frequency,
    int *bad_channel_start,
    int *bad_channel_end,
    int *good_channel_start,
    int *good_channel_end
);

static void qcril_qmi_coex_softap_report_info_to_telephony(int current_channel);
static void qcril_qmi_coex_report_lte_info_to_riva();
static void qcril_qmi_coex_riva_port_open_initiate();
static void* qcril_qmi_coex_riva_port_open_handler(void * param);
static void qcril_qmi_coex_riva_encode_header(uint8* buf,uint8 msg_type,uint16 txn_id, uint16 msg_id, uint16 msg_len);
static void qcril_qmi_coex_evaluate_report_lte_info_to_riva(qcril_timed_callback_handler_params_type *param);
void qcril_qmi_nas_retrieve_rf_band_info();
static IxErrnoType qcril_qmi_coex_riva_thread_condition_wait_helper();
static void qcril_qmi_coex_evaluate_final_channel_set();

/*===========================================================================

                                FUNCTIONS

===========================================================================*/

//===========================================================================
// qcril_qmi_coex_init
//===========================================================================
void qcril_qmi_coex_init()
{
    int temp_len;
    int iter_i;
    char property_name[ PROPERTY_VALUE_MAX ];
    char property_value[ PROPERTY_VALUE_MAX ];

    QCRIL_LOG_FUNC_ENTRY();

    memset(&coex_lte_gsm_range_info, 0, sizeof(coex_lte_gsm_range_info));

    coex_lte_gsm_range_info.good_channel_start = COEX_LTE_GSM_CHANNEL_START;
    coex_lte_gsm_range_info.good_channel_end = COEX_LTE_GSM_CHANNEL_END;
    coex_lte_gsm_range_info.gsm_bad_channel_start = COEX_LTE_GSM_CHANNEL_INVALID;
    coex_lte_gsm_range_info.gsm_bad_channel_end = COEX_LTE_GSM_CHANNEL_INVALID;
    coex_lte_gsm_range_info.gsm_good_channel_start = COEX_LTE_GSM_CHANNEL_START;
    coex_lte_gsm_range_info.gsm_good_channel_end = COEX_LTE_GSM_CHANNEL_END;
    coex_lte_gsm_range_info.lte_bad_channel_start = COEX_LTE_GSM_CHANNEL_INVALID;
    coex_lte_gsm_range_info.lte_bad_channel_end = COEX_LTE_GSM_CHANNEL_INVALID;
    coex_lte_gsm_range_info.lte_good_channel_start = COEX_LTE_GSM_CHANNEL_START;
    coex_lte_gsm_range_info.lte_good_channel_end = COEX_LTE_GSM_CHANNEL_END;
    memset(property_value, 0, sizeof(property_value));
    snprintf( property_name, sizeof(property_name), "%s", COEX_LTE_FREQUENCY_RANGE);
    property_get( property_name, property_value, "" );
    temp_len = strlen( property_value );
    if(temp_len > 0)
    {
        QCRIL_LOG_INFO("lte coex frequency range %s", property_value);
        qcril_qmi_coex_util_evaluate_lte_coex_range(property_value, coex_lte_gsm_range_info.freq_ranges, COEX_LTE_RANGE_NUMBER);
    }
    else
    {
        qcril_qmi_coex_util_fillup_default_lte_coex_frequencies(coex_lte_gsm_range_info.freq_ranges);
    }

    memset(property_value, 0, sizeof(property_value));
    snprintf( property_name, sizeof(property_name), "%s", COEX_LTE_GSM_CHANNEL_RANGE);
    property_get( property_name, property_value, "" );
    temp_len = strlen( property_value );
    if(temp_len > 0)
    {
        QCRIL_LOG_INFO("lte coex channel range %s", property_value);
        qcril_qmi_coex_util_evaluate_lte_coex_range(property_value, coex_lte_gsm_range_info.lte_channel_ranges, COEX_LTE_RANGE_NUMBER);
    }
    else
    {
        qcril_qmi_coex_util_fillup_default_lte_coex_channels(coex_lte_gsm_range_info.lte_channel_ranges);
    }

    if( (qmi_ril_is_feature_supported(QMI_RIL_FEATURE_APQ) &&
        !qmi_ril_is_feature_supported(QMI_RIL_FEATURE_8084)) ||
        QCRIL_IS_DSDA_COEX_ENABLED() )
    {
        pthread_mutexattr_init( &coex_lte_gsm_range_info.coex_riva_mutex_atr );
        pthread_mutex_init(&coex_lte_gsm_range_info.coex_riva_mutex, &coex_lte_gsm_range_info.coex_riva_mutex_atr);
        pthread_cond_init (&coex_lte_gsm_range_info.coex_riva_cond_var, NULL);

        coex_lte_gsm_range_info.coex_manager_sv_object = cxm_get_service_object_v01();
        qcril_qmi_coex_riva_port_open_initiate();
    }

    QCRIL_LOG_FUNC_RETURN();
} //qcril_qmi_coex_init


//===========================================================================
// qcril_qmi_coex_riva_port_open_initiate
//===========================================================================
void qcril_qmi_coex_riva_port_open_initiate()
{
    RIL_Errno res;
    pthread_attr_t attr;
    int conf;

    QCRIL_LOG_FUNC_ENTRY();

    pthread_attr_init (&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    conf = pthread_create(&coex_lte_gsm_range_info.riva_thread_id, &attr, qcril_qmi_coex_riva_port_open_handler, NULL);
    pthread_attr_destroy(&attr);
    qmi_ril_set_thread_name(coex_lte_gsm_range_info.riva_thread_id, QMI_RIL_LTE_COEX_RIVA_THREAD_NAME);
    QCRIL_LOG_INFO( ".. conf, pid %d, %d", (int)conf, (int) coex_lte_gsm_range_info.riva_thread_id );

    QCRIL_LOG_FUNC_RETURN();
} //qcril_qmi_coex_riva_port_open_initiate

//===========================================================================
// qcril_qmi_coex_riva_port_open_handler
//===========================================================================
void* qcril_qmi_coex_riva_port_open_handler(void * param)
{
    int number_of_tries = 0;
    IxErrnoType res = E_SUCCESS;

    QCRIL_LOG_FUNC_ENTRY();
    QCRIL_NOTUSED(param);

    while( number_of_tries < COEX_LTE_PORT_RETRY_MAX_ATTEMPTS )
    {
        number_of_tries++;
        coex_lte_gsm_range_info.smd_riva_fp = open("/dev/smd_cxm_qmi", O_WRONLY);
        if( coex_lte_gsm_range_info.smd_riva_fp > 0 )
        {
            QCRIL_LOG_INFO("opened SMD RIVA port %d", coex_lte_gsm_range_info.smd_riva_fp);
            qcril_qmi_nas_retrieve_rf_band_info();
            break;
        }
        else
        {
            QCRIL_LOG_INFO("Unable to open SMD RIVA port %d, attempt #%d", coex_lte_gsm_range_info.smd_riva_fp,number_of_tries);
        }
        COEX_RIVA_LOCK();
        res = COEX_RIVA_WAIT();
        COEX_RIVA_UNLOCK();
        if( E_ABORTED == res )
        {
            break;
        }
    }
    qmi_ril_clear_thread_name(pthread_self());

    QCRIL_LOG_FUNC_RETURN();
    return NULL;
} //qcril_qmi_coex_riva_port_open_handler

//===========================================================================
// qcril_qmi_coex_release
//===========================================================================
void qcril_qmi_coex_release()
{
    QCRIL_LOG_FUNC_ENTRY();

    if(coex_lte_gsm_range_info.smd_riva_fp > 0)
    {
        close(coex_lte_gsm_range_info.smd_riva_fp);
    }
    if(( qmi_ril_is_feature_supported(QMI_RIL_FEATURE_APQ) &&
        !qmi_ril_is_feature_supported(QMI_RIL_FEATURE_8084)) ||
        QCRIL_IS_DSDA_COEX_ENABLED() )
    {
        pthread_mutex_destroy( &coex_lte_gsm_range_info.coex_riva_mutex );
        pthread_mutexattr_destroy( &coex_lte_gsm_range_info.coex_riva_mutex_atr );
        pthread_cond_destroy( &coex_lte_gsm_range_info.coex_riva_cond_var );
    }
    memset(&coex_lte_gsm_range_info, 0, sizeof(coex_lte_gsm_range_info));

    QCRIL_LOG_FUNC_RETURN();
} //qcril_qmi_coex_release

//===========================================================================
// qcril_qmi_coex_util_evaluate_lte_coex_range
//===========================================================================
void qcril_qmi_coex_util_evaluate_lte_coex_range(char * input_range_ptr,int * lte_coex_range_ptr, int lte_coex_range_length)
{
    int iter_i = 0;
    int range_len = 0;
    int element_start = 0;
    int element_number = 0;

    QCRIL_LOG_FUNC_ENTRY();

    if(input_range_ptr && lte_coex_range_ptr)
    {
        range_len = strlen(input_range_ptr);
        for(iter_i = 0; iter_i <= range_len; iter_i++)
        {
            if(COEX_LTE_RANGE_DEL_COMMA == input_range_ptr[iter_i] || COEX_LTE_RANGE_DEL_END == input_range_ptr[iter_i])
            {
                if(element_number < lte_coex_range_length)
                {
                    lte_coex_range_ptr[element_number] = qcril_other_ascii_to_int(&input_range_ptr[element_start],(iter_i-element_start));
                    QCRIL_LOG_INFO("lte coex element number %d, value %d", (element_number+1), lte_coex_range_ptr[element_number]);
                    element_number++;
                    element_start = iter_i + 1;
                }
                else
                {
                    QCRIL_LOG_FATAL(" Out of bounds");
                }
            }
        }
    }
    else
    {
        QCRIL_LOG_FATAL("Null Pointer");
    }

    QCRIL_LOG_FUNC_RETURN();
} //qcril_qmi_coex_util_evaluate_lte_coex_range

//===========================================================================
// qcril_qmi_coex_util_fillup_default_lte_coex_frequencies
//===========================================================================
void qcril_qmi_coex_util_fillup_default_lte_coex_frequencies(int * lte_coex_range_ptr)
{
    QCRIL_LOG_FUNC_ENTRY();

    if(lte_coex_range_ptr)
    {
        lte_coex_range_ptr[0]=2496;
        lte_coex_range_ptr[1]=2690;
        lte_coex_range_ptr[2]=2300;
        lte_coex_range_ptr[3]=2350;
        lte_coex_range_ptr[4]=2350;
        lte_coex_range_ptr[5]=2370;
        lte_coex_range_ptr[6]=2370;
        lte_coex_range_ptr[7]=2400;
        lte_coex_range_ptr[8]=824;
        lte_coex_range_ptr[9]=834;
    }
    else
    {
        QCRIL_LOG_FATAL("Null Pointer");
    }

    QCRIL_LOG_FUNC_RETURN();
} //qcril_qmi_coex_util_fillup_default_lte_coex_frequencies

//===========================================================================
// qcril_qmi_coex_util_fillup_default_lte_coex_channels
//===========================================================================
void qcril_qmi_coex_util_fillup_default_lte_coex_channels(int * lte_coex_range_ptr)
{
    QCRIL_LOG_FUNC_ENTRY();

    if(lte_coex_range_ptr)
    {
        lte_coex_range_ptr[0]=10;
        lte_coex_range_ptr[1]=14;
        lte_coex_range_ptr[2]=1;
        lte_coex_range_ptr[3]=4;
        lte_coex_range_ptr[4]=1;
        lte_coex_range_ptr[5]=6;
        lte_coex_range_ptr[6]=1;
        lte_coex_range_ptr[7]=9;
        lte_coex_range_ptr[8]=11;
        lte_coex_range_ptr[9]=14;
    }
    else
    {
        QCRIL_LOG_FATAL("Null Pointer");
    }

    QCRIL_LOG_FUNC_RETURN();
} //qcril_qmi_coex_util_fillup_default_lte_coex_channels


//===========================================================================
// qcril_qmi_coex_process_rf_band_info
//===========================================================================
void qcril_qmi_coex_process_rf_band_info
(
    qcril_coex_rf_band_info_type rf_band_info_arr[QCRIL_COEX_RD_BAND_INFO_LENGTH],
    int                          rf_band_info_len
)
{
    int                           current_channel      = COEX_LTE_GSM_CHANNEL_INVALID;
    int                           is_lte_set_updated   = FALSE;
    int                           is_gsm_set_updated   = FALSE;
    qcril_coex_rf_band_info_type *rf_band_info_ptr     = rf_band_info_arr;
    int                           rf_band_info_ptr_len = 0;
    nas_rf_band_info_type_v01    *rf_band_lte_info     = NULL;
    nas_rf_band_info_type_v01    *rf_band_gsm_info     = NULL;

    QCRIL_LOG_FUNC_ENTRY();

    do
    {
        if ((rf_band_info_len > QCRIL_COEX_RD_BAND_INFO_LENGTH) ||
            (rf_band_info_len <= 0) ||
            (!rf_band_info_arr))
        {
            QCRIL_LOG_ERROR("Invalid input %d", rf_band_info_len);
            break;
        }

        for (; rf_band_info_ptr_len < rf_band_info_len; rf_band_info_ptr_len++)
        {
            rf_band_info_ptr = &rf_band_info_arr[rf_band_info_ptr_len];
            if (rf_band_info_ptr)
            {
                if ((NAS_RADIO_IF_LTE_V01 == rf_band_info_ptr->rat) &&
                    rf_band_info_ptr->rf_band_info)
                {
                    rf_band_lte_info = rf_band_info_ptr->rf_band_info;
                    QCRIL_LOG_INFO("LTE Radio interface %d, Active band %d, Active channel %d",
                                    rf_band_lte_info->radio_if,
                                    rf_band_lte_info->active_band,
                                    rf_band_lte_info->active_channel);

                    coex_lte_gsm_range_info.lte_downlink_active_channel = rf_band_lte_info->active_channel;
                    qcril_qmi_coex_util_convert_lte_active_channel_to_frequencies(
                                            coex_lte_gsm_range_info.lte_downlink_active_channel,
                                            &coex_lte_gsm_range_info.lte_downlink_freq,
                                            &coex_lte_gsm_range_info.lte_uplink_freq,
                                            &coex_lte_gsm_range_info.lte_uplink_active_channel);

                    if (COEX_LTE_RRC_INVALID_FDD_FREQUENCY != coex_lte_gsm_range_info.lte_downlink_freq &&
                                 COEX_LTE_RRC_INVALID_FDD_FREQUENCY != coex_lte_gsm_range_info.lte_uplink_freq)
                    {
                        /* Check if channel set is updated */
                        if(TRUE == qcril_qmi_coex_evaluate_channel_set(coex_lte_gsm_range_info.lte_uplink_freq,
                                                                      &coex_lte_gsm_range_info.lte_bad_channel_start,
                                                                      &coex_lte_gsm_range_info.lte_bad_channel_end,
                                                                      &coex_lte_gsm_range_info.lte_good_channel_start,
                                                                      &coex_lte_gsm_range_info.lte_good_channel_end))
                        {
                            is_lte_set_updated = TRUE;
                        }
                    }

                    if(( qmi_ril_is_feature_supported(QMI_RIL_FEATURE_APQ) &&
                        !qmi_ril_is_feature_supported(QMI_RIL_FEATURE_8084)) ||
                        QCRIL_IS_DSDA_COEX_ENABLED() )
                    {
                        qcril_qmi_coex_initiate_report_lte_info_to_riva(QCRIL_QMI_COEX_INITIATE_FOR_RF_CHECK);
                    }
                }

                if ((NAS_RADIO_IF_GSM_V01 == rf_band_info_ptr->rat) &&
                     rf_band_info_ptr->rf_band_info)
                {
                    rf_band_gsm_info = rf_band_info_ptr->rf_band_info;
                    QCRIL_LOG_INFO("GSM Radio interface %d, Active band %d, Active channel %d",
                                    rf_band_gsm_info->radio_if,
                                    rf_band_gsm_info->active_band,
                                    rf_band_gsm_info->active_channel);

                    coex_lte_gsm_range_info.gsm_downlink_active_channel = rf_band_gsm_info->active_channel;
                    qcril_qmi_coex_util_convert_gsm_active_channel_to_frequencies(
                                            coex_lte_gsm_range_info.gsm_downlink_active_channel,
                                            rf_band_gsm_info->active_band,
                                            &coex_lte_gsm_range_info.gsm_downlink_freq,
                                            &coex_lte_gsm_range_info.gsm_uplink_freq);


                    if (COEX_GSM_INVALID_FREQUENCY != coex_lte_gsm_range_info.gsm_downlink_freq &&
                        COEX_GSM_INVALID_FREQUENCY != coex_lte_gsm_range_info.gsm_uplink_freq)
                    {
                        if(TRUE == qcril_qmi_coex_evaluate_channel_set(coex_lte_gsm_range_info.gsm_uplink_freq,
                                                                      &coex_lte_gsm_range_info.gsm_bad_channel_start,
                                                                      &coex_lte_gsm_range_info.gsm_bad_channel_end,
                                                                      &coex_lte_gsm_range_info.gsm_good_channel_start,
                                                                      &coex_lte_gsm_range_info.gsm_good_channel_end))
                        {
                            is_gsm_set_updated = TRUE;
                        }
                    }

                }
            }
            else
            {
                QCRIL_LOG_INFO("Null Pointer");
            }
        }

        if (is_gsm_set_updated || is_lte_set_updated)
        {
            if (is_gsm_set_updated && is_lte_set_updated)
            {
                qcril_qmi_coex_evaluate_final_channel_set();
            }
            else if (is_lte_set_updated)
            {
                coex_lte_gsm_range_info.good_channel_start = coex_lte_gsm_range_info.lte_good_channel_start;
                coex_lte_gsm_range_info.good_channel_end   = coex_lte_gsm_range_info.lte_good_channel_end;
            }
            else
            {
                coex_lte_gsm_range_info.good_channel_start = coex_lte_gsm_range_info.gsm_good_channel_start;
                coex_lte_gsm_range_info.good_channel_end   = coex_lte_gsm_range_info.gsm_good_channel_end;
            }
            qcril_qmi_coex_softap_report_info_to_telephony(current_channel);
        }
    }while(0);

    QCRIL_LOG_FUNC_RETURN();
} //qcril_qmi_coex_process_rf_band_info

/*=========================================================================
  FUNCTION: qcril_qmi_coex_evaluate_channel_set

===========================================================================*/
/*!
    @brief
    Evaluate LTE and GSM channel set to finalize on a optimized channel set.

    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_coex_evaluate_final_channel_set
(
    void
)
{
    int temp_final_set[COEX_LTE_GSM_CHANNEL_END - COEX_LTE_GSM_CHANNEL_START + 1] = {0};

    int good_channel_num            = 0;
    int optimum_good_channel_num    = 0;
    int set_length                  = COEX_LTE_GSM_CHANNEL_END - COEX_LTE_GSM_CHANNEL_START + 1;
    int is_channel_set              = FALSE;
    int i;

    for (i = 0; i < set_length; i++)
    {
        if (((i+1) >= coex_lte_gsm_range_info.lte_good_channel_start) &&
            ((i+1) <= coex_lte_gsm_range_info.lte_good_channel_end))
        {
            temp_final_set[i] = 1;
        }

        if (((i+1) >= coex_lte_gsm_range_info.gsm_good_channel_start) &&
            ((i+1) <= coex_lte_gsm_range_info.gsm_good_channel_end))
        {
            temp_final_set[i] &= 1;
        }
        else
        {
            temp_final_set[i] &= 0;
        }

        if (temp_final_set[i])
        {
            good_channel_num++;
        }
    }

    for (i = 0; i < set_length && good_channel_num; i++)
    {
        if (temp_final_set[i])
        {
            optimum_good_channel_num++;
        }

        if ((temp_final_set[i] == 0) && (optimum_good_channel_num))
        {
            if (optimum_good_channel_num >= ((good_channel_num/2) + (good_channel_num%2)))
            {
                coex_lte_gsm_range_info.good_channel_end   = i;
                coex_lte_gsm_range_info.good_channel_start = i - optimum_good_channel_num + 1;
                is_channel_set = TRUE;
            }
            else
            {
                optimum_good_channel_num = 0;
            }
        }
    }

    if ((!is_channel_set) && (good_channel_num == optimum_good_channel_num))
    {
        if (good_channel_num)
        {
            coex_lte_gsm_range_info.good_channel_end   = COEX_LTE_GSM_CHANNEL_END;
            coex_lte_gsm_range_info.good_channel_start = COEX_LTE_GSM_CHANNEL_END - good_channel_num + 1;
            is_channel_set = TRUE;
        }
        else
        {
            QCRIL_LOG_ERROR("Invalid channel set configuration, no good channels found");
        }
    }

}

/*=========================================================================
  FUNCTION:  qcril_qmi_coex_util_convert_gsm_active_channel_to_frequencies

===========================================================================*/
/*!
    @brief
    Convert GSM active channel to frequency.

    @return
    None.
*/
/*=========================================================================*/
void qcril_qmi_coex_util_convert_gsm_active_channel_to_frequencies
(
    uint16_t                 active_channel,
    nas_active_band_enum_v01 active_band,
    int                     *downlink,
    int                     *uplink
)
{
    boolean is_frequency_calculated = FALSE;
    QCRIL_LOG_FUNC_ENTRY();

    if( downlink && uplink )
    {
        *downlink = COEX_GSM_INVALID_FREQUENCY;
        *uplink = COEX_GSM_INVALID_FREQUENCY;
        if (active_channel <= COEX_GSM_MAX_ARFCN)
        {
            if (NAS_ACTIVE_BAND_GSM_450_V01 == active_band)
            {
                if ((active_channel >= 259) && (active_channel <= 293))
                {
                    *uplink   = 450.6 + (0.2 * (active_channel - 259));
                    *downlink = *uplink + 10;
                    is_frequency_calculated = TRUE;
                }
            }
            else if (NAS_ACTIVE_BAND_GSM_480_V01 == active_band)
            {
                if ((active_channel >= 306) && (active_channel <= 340))
                {
                    *uplink   = 479 + (0.2 * (active_channel - 306));
                    *downlink = *uplink + 10;
                    is_frequency_calculated = TRUE;
                }
            }
            else if (NAS_ACTIVE_BAND_GSM_850_V01 == active_band)
            {
                if ((active_channel >= 128) && (active_channel <= 251))
                {
                    *uplink   = 824.2 + (0.2 * (active_channel - 128));
                    *downlink = *uplink + 45;
                    is_frequency_calculated = TRUE;
                }
            }
            else if (NAS_ACTIVE_BAND_GSM_900_EXTENDED_V01 == active_band)
            {
                if (active_channel <= 124)
                {
                    *uplink   = 890 + (0.2 * active_channel);
                    *downlink = *uplink + 45;
                    is_frequency_calculated = TRUE;
                }
                else if ((active_channel >= 975) && (active_channel <= 1023))
                {
                    *uplink   = 890 + (0.2 * (active_channel - 1024));
                    *downlink = *uplink + 45;
                    is_frequency_calculated = TRUE;
                }
            }
            else if (NAS_ACTIVE_BAND_GSM_900_PRIMARY_V01 == active_band)
            {
                if ((active_channel >= 1) && (active_channel <= 124))
                {
                    *uplink   = 890 + (0.2 * active_channel);
                    *downlink = *uplink + 45;
                    is_frequency_calculated = TRUE;
                }
            }
            else if (NAS_ACTIVE_BAND_GSM_900_RAILWAYS_V01 == active_band)
            {
                if (active_channel <= 124)
                {
                    *uplink   = 890 + (0.2 * active_channel);
                    *downlink = *uplink + 45;
                    is_frequency_calculated = TRUE;
                }
                else if ((active_channel >= 975) && (active_channel <= 1023))
                {
                    *uplink   = 890 + (0.2 * (active_channel - 1024));
                    *downlink = *uplink + 45;
                    is_frequency_calculated = TRUE;
                }
            }
            else if (NAS_ACTIVE_BAND_GSM_1800_V01 == active_band)
            {
                if ((active_channel >= 512) && (active_channel <= 885))
                {
                    *uplink   = 1710.2 + (0.2 * (active_channel - 512));
                    *downlink = *uplink + 95;
                    is_frequency_calculated = TRUE;
                }
            }
            else if (NAS_ACTIVE_BAND_GSM_1900_V01 == active_band)
            {
                if ((active_channel >= 512) && (active_channel <= 810))
                {
                    *uplink   = 1850.2 + (0.2 * (active_channel - 512));
                    *downlink = *uplink + 80;
                    is_frequency_calculated = TRUE;
                }
            }
            else
            {
                QCRIL_LOG_FATAL("Invalid active band");
            }
        }

        if (!is_frequency_calculated)
        {
            QCRIL_LOG_FATAL("Unknown active band %d or active channel %d",
                             active_band, active_channel);
        }
    }
    else
    {
        QCRIL_LOG_FATAL("Null Pointer");
    }

    QCRIL_LOG_FUNC_RETURN();
}

//===========================================================================
// qcril_qmi_coex_util_convert_lte_active_channel_to_frequencies
//===========================================================================
void qcril_qmi_coex_util_convert_lte_active_channel_to_frequencies
(
    uint16_t active_channel,
    int *downlink,
    int *uplink,
    int *uplink_active_channel
)
{
    QCRIL_LOG_FUNC_ENTRY();

    if( downlink && uplink && uplink_active_channel )
    {
        *downlink = COEX_LTE_RRC_INVALID_FDD_FREQUENCY;
        *uplink = COEX_LTE_RRC_INVALID_FDD_FREQUENCY;
        if (active_channel <= COEX_LTE_RRC_MAX_TDD_DL_EARFN)
        {
            /* Generic formula for determining DL Carrier Freq from DL earfcn as given in
            Sec 5.7.3 of 36.101
              DL Carrier Freq F_DL = F_DL_LOW + 0.1 (N_DL_EARFCN - N_OFFSET_DL)
              UL Carrier Freq F_UL = F_UL_LOW + 0.1 (N_UL_EARFCN - N_OFFSET_UL)
              UL EARCN N_UL_EARFCN = 10 * (( F_DL - TX_RX_SEPARATION) - F_UL_LOW) + N_OFFSET_UL
            */
            if (active_channel <= 599)
            {
            *downlink = 2110 + (0.1 * active_channel);
            *uplink = *downlink - 190;
            *uplink_active_channel = 10 * (*uplink - 1920) + 18000;
            }
            else if (active_channel <= 1199)
            {
            *downlink = 1930 + (0.1 * (active_channel - 600));
            *uplink = *downlink - 80;
            *uplink_active_channel = 10 * (*uplink - 1850) + 18600;
            }
            else if (active_channel <= 1949)
            {
            *downlink = 1805 + (0.1 * (active_channel - 1200));
            *uplink = *downlink - 95;
            *uplink_active_channel = 10 * (*uplink - 1710) + 19200;
            }
            else if (active_channel <= 2399)
            {
            *downlink = 2110 + (0.1 * (active_channel - 1950));
            *uplink = *downlink - 400;
            *uplink_active_channel = 10 * (*uplink - 1710) + 19950;
            }
            else if (active_channel <= 2649)
            {
            *downlink = 869 + (0.1 * (active_channel - 2400));
            *uplink = *downlink - 45;
            *uplink_active_channel = 10 * (*uplink - 824) + 20400;
            }
            else if (active_channel <= 2749)
            {
            *downlink = 875 + (0.1 * (active_channel - 2650));
            *uplink = *downlink - 45;
            *uplink_active_channel = 10 * (*uplink - 830) + 20650;
            }
            else if (active_channel <= 3449)
            {
            *downlink = 2620 + (0.1 * (active_channel - 2750));
            *uplink = *downlink - 120;
            *uplink_active_channel = 10 * (*uplink - 2500) + 20750;
            }
            else if (active_channel <= 3799)
            {
            *downlink = 925 + (0.1 * (active_channel - 3450));
            *uplink = *downlink - 45;
            *uplink_active_channel = 10 * (*uplink - 880) + 21450;
            }
            else if (active_channel <= 4149)
            {
            *downlink = 1844.9 + (0.1 * (active_channel - 3800));
            *uplink = *downlink - 95;
            *uplink_active_channel = 10 * (*uplink - 1749.9) + 21800;
            }
            else if (active_channel <= 4749)
            {
            *downlink = 2110 + (0.1 * (active_channel - 4150));
            *uplink = *downlink - 400;
            *uplink_active_channel = 10 * (*uplink - 1710) + 22150;
            }
            else if (active_channel <= 4949)
            {
            *downlink = 1475.9 + (0.1 * (active_channel - 4750));
            *uplink = *downlink - 48;
            *uplink_active_channel = 10 * (*uplink - 1427.9) + 22750;
            }
            else if ((active_channel >= 5010) && (active_channel <= 5179))
            {
            *downlink = 729 + (0.1 * (active_channel - 5010));
            *uplink = *downlink - 30;
            *uplink_active_channel = 10 * (*uplink - 699) + 23010;
            }
            else if ((active_channel >= 5180) && (active_channel <= 5279))
            {
            *downlink = 746 + (0.1 * (active_channel - 5180));
            *uplink = *downlink + 31;
            *uplink_active_channel = 10 * (*uplink - 777) + 23180;
            }
            else if ((active_channel >= 5280) && (active_channel <= 5379))
            {
            *downlink = 758 + (0.1 * (active_channel - 5280));
            *uplink = *downlink + 30;
            *uplink_active_channel = 10 * (*uplink - 788) + 23280;
            }
            else if ((active_channel >= 5730) && (active_channel <= 5849))
            {
            *downlink = 734 + (0.1 * (active_channel - 5730));
            *uplink = *downlink - 30;
            *uplink_active_channel = 10 * (*uplink - 704) + 23730;
            }
            else if ((active_channel >= 5850) && (active_channel <= 5999))
            {
            *downlink = 860 + (0.1 * (active_channel - 5850));
            *uplink = *downlink - 45;
            *uplink_active_channel = 10 * (*uplink - 815) + 23850;
            }
            else if ((active_channel >= 6000) && (active_channel <= 6149))
            {
            *downlink = 875 + (0.1 * (active_channel - 6000));
            *uplink = *downlink - 45;
            *uplink_active_channel = 10 * (*uplink - 830) + 24000;
            }
            else if ((active_channel >= 6150) && (active_channel <= 6449))
            {
            *downlink = 791 + (0.1 * (active_channel - 6150));
            *uplink = *downlink + 41;
            *uplink_active_channel = 10 * (*uplink - 832) + 24150;
            }
            else if ((active_channel >= 6450) && (active_channel <= 6599))
            {
            *downlink = 1495.9 + (0.1 * (active_channel - 6450));
            *uplink = *downlink - 48;
            *uplink_active_channel = 10 * (*uplink - 1447.9) + 24450;
            }
            else if ((active_channel >= 6600) && (active_channel <= 7399))
            {
            *downlink = 3510 + (0.1 * (active_channel - 6600));
            *uplink = *downlink - 100;
            *uplink_active_channel = 10 * (*uplink - 3410) + 24600;
            }
            else if ((active_channel >= 7500) && (active_channel <= 7699))
            {
            *downlink = 2180 + (0.1 * (active_channel - 7500));
            *uplink = *downlink - 180;
            *uplink_active_channel = 10 * (*uplink - 2000) + 25500;
            }
            else if ((active_channel >= 7700) && (active_channel <= 8039))
            {
            *downlink = 1525 + (0.1 * (active_channel - 7700));
            *uplink = *downlink + 101.5;
            *uplink_active_channel = 10 * (*uplink - 1626.5) + 25700;
            }
            else if ((active_channel >= 8040) && (active_channel <= 8689))
            {
            *downlink = 1930 + (0.1 * (active_channel - 8040));
            *uplink = *downlink - 80;
            *uplink_active_channel = 10 * (*uplink - 1850) + 26040;
            }
            else if ((active_channel >= COEX_LTE_RRC_MIN_TDD_DL_EARFN) && (active_channel <= 36199))
            {
            *downlink = 1900 + (0.1 * (active_channel - 36000));
            *uplink = *downlink;
            *uplink_active_channel = 10 * (*uplink - 1900) + 36000;
            }
            else if ((active_channel >= 36200) && (active_channel <= 36349))
            {
            *downlink = 2010 + (0.1 * (active_channel - 36200));
            *uplink = *downlink;
            *uplink_active_channel = 10 * (*uplink - 2010) + 36200;
            }
            else if ((active_channel >= 36350) && (active_channel <= 36949))
            {
            *downlink = 1850 + (0.1 * (active_channel - 36350));
            *uplink = *downlink;
            *uplink_active_channel = 10 * (*uplink - 1850) + 36350;
            }
            else if ((active_channel >= 36950) && (active_channel <= 37549))
            {
            *downlink = 1930 + (0.1 * (active_channel - 36950));
            *uplink = *downlink;
            *uplink_active_channel = 10 * (*uplink - 1930) + 36950;
            }
            else if ((active_channel >= 37550) && (active_channel <= 37749))
            {
            *downlink = 1910 + (0.1 * (active_channel - 37550));
            *uplink = *downlink;
            *uplink_active_channel = 10 * (*uplink - 1910) + 37550;
            }
            else if ((active_channel >= 37750) && (active_channel <= 38249))
            {
            *downlink = 2570 + (0.1 * (active_channel - 37750));
            *uplink = *downlink;
            *uplink_active_channel = 10 * (*uplink - 2570) + 37750;
            }
            else if ((active_channel >= 38250) && (active_channel <= 38649))
            {
            *downlink = 1880 + (0.1 * (active_channel - 38250));
            *uplink = *downlink;
            *uplink_active_channel = 10 * (*uplink - 1880) + 38250;
            }
            else if ((active_channel >= 38650) && (active_channel <= 39649))
            {
            *downlink = 2300 + (0.1 * (active_channel - 38650));
            *uplink = *downlink;
            *uplink_active_channel = 10 * (*uplink - 2300) + 38650;
            }
            else if ((active_channel >= 39650) && (active_channel <= 41589))
            {
            *downlink = 2496 + (0.1 * (active_channel - 39650));
            *uplink = *downlink;
            *uplink_active_channel = 10 * (*uplink - 2496) + 39650;
            }
            else if ((active_channel >= 41590) && (active_channel <= 43589))
            {
            *downlink = 3400 + (0.1 * (active_channel - 41590));
            *uplink = *downlink;
            *uplink_active_channel = 10 * (*uplink - 3400) + 41590;
            }
            else if ((active_channel >= 43590) && (active_channel <= 45589))
            {
            *downlink = 3600 + (0.1 * (active_channel - 43590));
            *uplink = *downlink;
            *uplink_active_channel = 10 * (*uplink - 3600) + 43590;
            }
            else
            {
                QCRIL_LOG_FATAL("Invalid Case - No match found");
            }
        }
        else
        {
            QCRIL_LOG_FATAL("Invalid Case - Active channel out of range");
        }

        QCRIL_LOG_INFO("downlink %d downlink_active_channel %d uplink %d uplink_active_channel %d", *downlink, active_channel, *uplink, *uplink_active_channel);
    }
    else
    {
        QCRIL_LOG_FATAL("Null Pointer");
    }

    QCRIL_LOG_FUNC_RETURN();
} //qcril_qmi_coex_util_convert_active_channel_to_frequencies


//===========================================================================
// qcril_qmi_coex_evaluate_channel_set
//===========================================================================
int qcril_qmi_coex_evaluate_channel_set
(
    int frequency,
    int *bad_channel_start,
    int *bad_channel_end,
    int *good_channel_start,
    int *good_channel_end
)
{
    int temp_bad_channel_start;
    int temp_bad_channel_end;
    int is_channel_set_updated = FALSE;

    QCRIL_LOG_FUNC_ENTRY();

    if (!bad_channel_start || !bad_channel_end ||
        !good_channel_start || !good_channel_end)
    {
        return is_channel_set_updated;
    }

    QCRIL_LOG_INFO("frequency %d", frequency);
    QCRIL_LOG_INFO("before processing : bad channel range %d to %d, good channel range %d to %d",
                   *bad_channel_start, *bad_channel_end,
                   *good_channel_start, *good_channel_end);

    if( coex_lte_gsm_range_info.freq_ranges[0] <= frequency && frequency <= coex_lte_gsm_range_info.freq_ranges[1] )
    {
        temp_bad_channel_start = coex_lte_gsm_range_info.lte_channel_ranges[0];
        temp_bad_channel_end = coex_lte_gsm_range_info.lte_channel_ranges[1];
    }
    else if( coex_lte_gsm_range_info.freq_ranges[2] <= frequency && frequency < coex_lte_gsm_range_info.freq_ranges[3] )
    {
        temp_bad_channel_start = coex_lte_gsm_range_info.lte_channel_ranges[2];
        temp_bad_channel_end = coex_lte_gsm_range_info.lte_channel_ranges[3];
    }
    else if( coex_lte_gsm_range_info.freq_ranges[4] <= frequency && frequency < coex_lte_gsm_range_info.freq_ranges[5] )
    {
        temp_bad_channel_start = coex_lte_gsm_range_info.lte_channel_ranges[4];
        temp_bad_channel_end = coex_lte_gsm_range_info.lte_channel_ranges[5];
    }
    else if( coex_lte_gsm_range_info.freq_ranges[6] <= frequency && frequency <= coex_lte_gsm_range_info.freq_ranges[7] )
    {
        temp_bad_channel_start = coex_lte_gsm_range_info.lte_channel_ranges[6];
        temp_bad_channel_end = coex_lte_gsm_range_info.lte_channel_ranges[7];
    }
    else if( coex_lte_gsm_range_info.freq_ranges[8] <= frequency && frequency <= coex_lte_gsm_range_info.freq_ranges[9] )
    {
        temp_bad_channel_start = coex_lte_gsm_range_info.lte_channel_ranges[8];
        temp_bad_channel_end = coex_lte_gsm_range_info.lte_channel_ranges[9];
    }
    else
    {
        temp_bad_channel_start = COEX_LTE_GSM_CHANNEL_INVALID;
        temp_bad_channel_end = COEX_LTE_GSM_CHANNEL_INVALID;
    }

    if( temp_bad_channel_start != *bad_channel_start || temp_bad_channel_end != *bad_channel_end )
    {
        QCRIL_LOG_INFO("channel range changed");
        *bad_channel_start = temp_bad_channel_start;
        *bad_channel_end = temp_bad_channel_end;
        if( COEX_LTE_GSM_CHANNEL_START == temp_bad_channel_start )
        {
            *good_channel_start = temp_bad_channel_end + 1;
            *good_channel_end = COEX_LTE_GSM_CHANNEL_END;
        }
        else if( COEX_LTE_GSM_CHANNEL_END == temp_bad_channel_end )
        {
            *good_channel_start = COEX_LTE_GSM_CHANNEL_START;
            *good_channel_end = temp_bad_channel_start - 1;
        }
        else
        {
            *good_channel_start = COEX_LTE_GSM_CHANNEL_START;
            *good_channel_end = COEX_LTE_GSM_CHANNEL_END;
        }
        is_channel_set_updated = TRUE;
    }
    else
    {
        QCRIL_LOG_INFO("no change in channel range");
    }

    QCRIL_LOG_INFO("after processing : bad channel range %d to %d, good channel range %d to %d",
                   *bad_channel_start, *bad_channel_end,
                   *good_channel_start, *good_channel_end);


    QCRIL_LOG_FUNC_RETURN_WITH_RET(is_channel_set_updated);
    return is_channel_set_updated;
} //qcril_qmi_coex_evaluate_channel_set

//===========================================================================
// qcril_qmi_coex_softap_report_info_to_telephony
//===========================================================================
void qcril_qmi_coex_softap_report_info_to_telephony(int current_channel)
{
    char payload[16];

    QCRIL_LOG_FUNC_ENTRY();

    QCRIL_SNPRINTF( payload, sizeof( payload ), "%d,%d,%d",
                     current_channel,
                     coex_lte_gsm_range_info.good_channel_start,
                     coex_lte_gsm_range_info.good_channel_end);

    QCRIL_LOG_INFO("Sending %s to Telephony",payload);
    qcril_hook_unsol_response( QCRIL_DEFAULT_INSTANCE_ID, QCRIL_EVT_HOOK_UNSOL_LTE_COEX, payload, strlen(payload));

    QCRIL_LOG_FUNC_RETURN();
} //qcril_qmi_coex_softap_report_info_to_telephony

//===========================================================================
// qcril_qmi_coex_report_lte_info_to_riva
//===========================================================================
void qcril_qmi_coex_report_lte_info_to_riva()
{

    cxm_state_ind_msg_v01 cxm_state_ind_msg;
    char smd_riva_lte_data[512];
    uint32_t decoded_len = 0;
    uint32_t num_of_bytes_written = 0;


    QCRIL_LOG_FUNC_ENTRY();

    if(( qmi_ril_is_feature_supported(QMI_RIL_FEATURE_APQ) &&
        !qmi_ril_is_feature_supported(QMI_RIL_FEATURE_8084)) ||
        QCRIL_IS_DSDA_COEX_ENABLED() )
    {
        memset(&cxm_state_ind_msg, 0, sizeof(cxm_state_ind_msg));
        memset(smd_riva_lte_data, 0, sizeof(smd_riva_lte_data));

        cxm_state_ind_msg.lte_ml1_state.ul_bandwidth = coex_lte_gsm_range_info.lte_uplink_freq;
        cxm_state_ind_msg.lte_ml1_state.ul_earfcn = coex_lte_gsm_range_info.lte_uplink_active_channel;
        cxm_state_ind_msg.lte_ml1_state.mask = COEX_LTE_RIVA_MASK; //uplink freq, uplink earfcn
        cxm_state_ind_msg.lte_ml1_state.is_connected = coex_lte_gsm_range_info.is_lte_active;

        QCRIL_LOG_INFO("ul_bandwidth %d, ul_earfcn %d, mask %02x, is_connected %d", cxm_state_ind_msg.lte_ml1_state.ul_bandwidth,
                                                                                    cxm_state_ind_msg.lte_ml1_state.ul_earfcn,
                                                                                    cxm_state_ind_msg.lte_ml1_state.mask,
                                                                                    cxm_state_ind_msg.lte_ml1_state.is_connected);

        qmi_idl_message_encode(coex_lte_gsm_range_info.coex_manager_sv_object,
                               QMI_IDL_INDICATION,
                               QMI_CXM_STATE_IND_MSG_V01,
                               &cxm_state_ind_msg,
                               sizeof(cxm_state_ind_msg),
                               smd_riva_lte_data+COEX_LTE_QMI_HEADER_SIZE,
                               sizeof(smd_riva_lte_data)-COEX_LTE_QMI_HEADER_SIZE,
                               &decoded_len);

        qcril_qmi_coex_riva_encode_header((uint8*)smd_riva_lte_data, QMI_IDL_INDICATION, 0, QMI_CXM_STATE_IND_MSG_V01, decoded_len );

        decoded_len += COEX_LTE_QMI_HEADER_SIZE;

        if( coex_lte_gsm_range_info.smd_riva_fp > 0 )
        {
            num_of_bytes_written = write(coex_lte_gsm_range_info.smd_riva_fp, smd_riva_lte_data, decoded_len);
            QCRIL_LOG_INFO("decoded len %d, number of bytes written %d", decoded_len, num_of_bytes_written);
        }
        else
        {
            QCRIL_LOG_INFO("SMD RIVA port unintialized %d", coex_lte_gsm_range_info.smd_riva_fp);
        }
    }

    QCRIL_LOG_FUNC_RETURN();
} //qcril_qmi_coex_report_lte_info_to_riva

//===========================================================================
// qcril_qmi_coex_riva_encode_header
//===========================================================================
void qcril_qmi_coex_riva_encode_header(uint8* buf,uint8 msg_type,uint16 txn_id, uint16 msg_id, uint16 msg_len)
{
 memcpy(buf,   &msg_type, 1);
 memcpy(buf+1, &txn_id, 2);
 memcpy(buf+3, &msg_id, 2);
 memcpy(buf+5, &msg_len, 2);
} //qcril_qmi_coex_riva_encode_header


//===========================================================================
// qcril_qmi_coex_initiate_report_lte_info_to_riva
//===========================================================================
void qcril_qmi_coex_initiate_report_lte_info_to_riva(int reason)
{
    QCRIL_LOG_FUNC_ENTRY();

    qcril_setup_timed_callback_ex_params( QCRIL_DEFAULT_INSTANCE_ID,
                                          QCRIL_DEFAULT_MODEM_ID,
                                          qcril_qmi_coex_evaluate_report_lte_info_to_riva,
                                          (void*)(intptr_t) reason,
                                          NULL, // immediate
                                          NULL );
    QCRIL_LOG_FUNC_RETURN();
} //qcril_qmi_coex_initiate_report_lte_info_to_riva

//===========================================================================
// qcril_qmi_coex_evaluate_report_lte_info_to_riva
//===========================================================================
void qcril_qmi_coex_evaluate_report_lte_info_to_riva(qcril_timed_callback_handler_params_type *param)
{
    int reason = FALSE;
    int report = TRUE;
    qcril_data_active_call_info_t * data_call_list = NULL;
    int number_of_calls = QMI_RIL_ZERO;
    int iter_i = 0;
    int is_lte_active = FALSE;

    QCRIL_LOG_FUNC_ENTRY();

    reason = (intptr_t) param->custom_param;
    if(QCRIL_QMI_COEX_INITIATE_FOR_DATA_CHECK == reason)
    {
        data_call_list = qcril_malloc(sizeof( *data_call_list ) * QCRIL_DATA_MAX_CALL_RECORDS);
        if(data_call_list)
        {
            memset(data_call_list, 0, (sizeof( *data_call_list ) * QCRIL_DATA_MAX_CALL_RECORDS));
            number_of_calls = qcril_qmi_data_nas_control_get_current_calls_number(data_call_list);
            for( iter_i = 0; iter_i < number_of_calls; iter_i++ )
            {
                if( RADIO_TECH_LTE == data_call_list[iter_i].radioTech && CALL_ACTIVE_PHYSLINK_UP == data_call_list[iter_i].active )
                {
                    is_lte_active = TRUE;
                    break;
                }
            }
            if(coex_lte_gsm_range_info.is_lte_active != is_lte_active)
            {
                coex_lte_gsm_range_info.is_lte_active = is_lte_active;
                report = TRUE;
            }
            else
            {
                report = FALSE;
            }
            qcril_free( data_call_list );
        }
        else
        {
            report = FALSE;
            QCRIL_LOG_FATAL("Unable to allocate memory");
        }
    }

    QCRIL_LOG_INFO("reason %d report %d", reason, report);
    if(TRUE == report)
    {
        qcril_qmi_coex_report_lte_info_to_riva(); //reporting the info
    }

    QCRIL_LOG_FUNC_RETURN();
} //qcril_qmi_coex_evaluate_report_lte_info_to_riva

//===========================================================================
// qcril_qmi_coex_terminate_riva_thread
//===========================================================================
void qcril_qmi_coex_terminate_riva_thread()
{
    QCRIL_LOG_FUNC_ENTRY();

    COEX_RIVA_LOCK();
    coex_lte_gsm_range_info.coex_riva_signal_cond = QMI_RIL_RIVA_COEX_SIGNAL_COND_ABORT;
    COEX_RIVA_SIGNAL();
    COEX_RIVA_UNLOCK();

    QCRIL_LOG_FUNC_RETURN();
} //qcril_qmi_coex_terminate_riva_thread

//===========================================================================
//qcril_qmi_coex_riva_thread_condition_wait_helper
//===========================================================================
IxErrnoType qcril_qmi_coex_riva_thread_condition_wait_helper()
{
    IxErrnoType res = E_ABORTED;
    struct timeval tp;
    struct timespec ts;

    gettimeofday(&tp, NULL);
    ts.tv_sec = tp.tv_sec + 1;     //1 seconds
    ts.tv_nsec = tp.tv_usec * 1000;

    if( QMI_RIL_RIVA_COEX_SIGNAL_COND_NONE == coex_lte_gsm_range_info.coex_riva_signal_cond )
    {
        res = pthread_cond_timedwait(&coex_lte_gsm_range_info.coex_riva_cond_var, &coex_lte_gsm_range_info.coex_riva_mutex, &ts);
        if(ETIMEDOUT != res)
        {
            res = E_ABORTED;
        }
    }

    if( E_ABORTED ==  res)
    {
        QCRIL_LOG_INFO("coex riva process cancelled");
    }

    return res;
} //qcril_qmi_coex_riva_thread_condition_wait_helper

