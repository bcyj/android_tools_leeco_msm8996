/* Copyright (c) 2011-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include "hardware/gps.h"
#include <gps_extended.h>
#include "log_util.h"
#include <stdarg.h>
#include <pthread.h>
#include <sys/time.h>
#include <android/log.h>
#include <private/android_filesystem_config.h>
#ifdef USE_GLIB
#include <utils/Log_garden.h>
#endif /* USE_GLIB */
#include "loc_vzw.h"
#include "android_runtime/AndroidRuntime.h"
#include "loc_cfg.h"
#include "test_android_gps.h"
#include "xtra_system_interface.h"
#include "ulp_service.h"
#include "loc_extended.h"
#include "loc_target.h"
//#include <android/log.h>
#include <time.h>
#include <linux/android_alarm.h>
#include "qmi_client_instance_defs.h"

#define TRUE     1
#define FALSE    0
#define LOG_TAG  "afw-simu"

#ifdef USE_GLIB
#include <glib.h>
#define strlcpy g_strlcpy
#endif /* USE_GLIB */

typedef enum
{
    ACTION_NONE,
    ACTION_QUIT,
    ACTION_OPEN_ATL,
    ACTION_CLOSE_ATL,
    ACTION_FAIL_ATL,
    ACTION_NI_NOTIFY,
    ACTION_XTRA_DATA,
    ACTION_XTRA_TIME,
    ACTION_NLP_RESPONSE,
    ACTION_PHONE_CONTEXT_UPDATE
} test_thread_action_e_type;

pthread_mutex_t test_thread_mutex;
pthread_cond_t test_thread_cond;

pthread_mutex_t location_conn_mutex;
pthread_cond_t location_conn_cond;


pthread_mutex_t networkpos_req_mutex;
pthread_cond_t networkpos_req_cond;

pthread_mutex_t phone_context_mutex;
pthread_cond_t phone_context_cond;

pthread_mutex_t ulp_location_mutex;
pthread_cond_t ulp_location_cond;

pthread_mutex_t session_status_mutex;
pthread_cond_t session_status_cond;

pthread_mutex_t wait_count_mutex;
pthread_cond_t wait_count_cond;

pthread_mutex_t wait_atlcb_mutex;
pthread_cond_t wait_atlcb_cond;

pthread_mutex_t wait_xtratime_mutex;
pthread_cond_t wait_xtratime_cond;

pthread_mutex_t wait_xtradata_mutex;
pthread_cond_t wait_xtradata_cond;

test_thread_action_e_type test_thread_action;

const GpsInterface *pGpsInterface = NULL;
AGpsRilInterface *pAgpsRilInterface = NULL;
GpsNiNotification sNotification;
int LEGACY;
#ifdef TEST_ULP
const UlpEngineInterface* pUlpEngineInterface = NULL;
const UlpPhoneContextInterface *pUlpPhoneContextInterface = NULL;
const UlpNetworkInterface* pUlpNetworkInterface = NULL;
#endif

#define XTRA_DATA_BUF_LEN 200000
#define XTRA_DATA_FILE_NAME "localxtra.bin"
#define XTRA_DEFAULT_SERVER1 "http://xtra1.gpsonextra.net/xtra2.bin"
#define XTRA_DEFAULT_SERVER2 "http://xtra2.gpsonextra.net/xtra2.bin"
#define XTRA_DEFAULT_SERVER3 "http://xtra3.gpsonextra.net/xtra2.bin"
#define XTRA_USER_AGENT_STRING "LE/1.2.3/OEM/Model/Board/Carrier" // Test user agent string
#define XTRA_NTP_DEFAULT_SERVER1 "time.gpsonextra.net"

struct timeval TimeAtStartNav = {0, 0};
struct timeval TimeAtFirstFix = {0, 0};

int g_checkForEngineOff = 0;
int g_checkForXtraDataCallBack = 1;
int g_exitStatus = 0; // Test result unknown

static int g_xtra_data_len = 0;
char g_xtra_data_buf[XTRA_DATA_BUF_LEN];

static time_t  startTime, firstFixTime;

// Xtra data file
//std::ofstream g_xtra_data_file;
FILE * g_xtra_data_file;

typedef struct time_s {
    GpsUtcTime time;
    int64_t timeReference;
    int uncertainty;
} XtraTime;

// Global Xtra time struct
XtraTime g_xtra_time;

//Global Phone context stettings
// global phone context setting
bool    is_gps_enabled = TRUE;
/** is network positioning enabled */
bool    is_network_position_available = TRUE;
/** is wifi turned on */
bool    is_wifi_setting_enabled = TRUE;
/** is battery being currently charged */
bool    is_battery_charging = TRUE;
/** is agps enabled */
bool    is_agps_enabled = TRUE;
/** run tracking session if true*/
static bool    g_run_active_client = FALSE;


/** Agps Command Line options **/
typedef struct agps_command_line_options {
    AGpsType agpsType; // Agps type
    const char * apn; // apn
    AGpsBearerType agpsBearerType; // Agps bearer type.
/* values for struct members from here down are initialized by reading gps.conf
   file using -c options to garden */
    unsigned long suplVer;
    char suplHost[256];
    int suplPort;
    char c2kHost[256];
    int c2kPort;
} AgpsCommandLineOptionsType;

// Default Agps command line values
static AgpsCommandLineOptionsType sAgpsCommandLineOptions = {
    AGPS_TYPE_SUPL,
    "myapn.myapn.com",
    AGPS_APN_BEARER_IPV4,
    0x10000, // SUPL version
    "supl.host.com",
    1234,
    "c2k.pde.com",
    1234
};

// Default values for Xtra configuration.
// struct members can also be initialized from
// gps.conf file through -c option
XtraClientConfigType g_xtra_client_config = {
    XTRA_NTP_DEFAULT_SERVER1,
    XTRA_DEFAULT_SERVER1,
    XTRA_DEFAULT_SERVER2,
    XTRA_DEFAULT_SERVER3,
    XTRA_USER_AGENT_STRING
};

/** Structure that holds the command line options given to main **/
typedef struct command_line_options {
    GpsPositionRecurrence r; // recurrence type
    int l; // Number of sessions to loop through.
    int t; // time to stop fix in seconds
    int s; // Stacks to test.
    int b; // Legacy TRUE OR FALSE.
    int ulpTestCaseNumber; // Run specified ULP test case number
    int deleteAidingDataMask; // Specifies Hexadecimal mask for deleting aiding data.
    int positionMode; // Specifies Position mode.
    int interval; // Time in milliseconds between fixes.
    int accuracy; // Accuracy in meters
    int responseTime; // Requested time to first fix in milliseconds
    GpsLocation location; // Only latitude, longiture and accuracy are used in this structure.
    int networkInitiatedResponse[256]; // To store the response pattern
    int niCount; // Number of back to back Ni tests
    int niResPatCount; // Number of elements in the response pattern
    AgpsCommandLineOptionsType agpsData; // Agps Data
    int isSuccess; // Success, Failure, ...
    int rilInterface; // Ril Interface
    XtraClientConfigType xtraClientConfig; // Xtra System configuration
    char gpsConfPath[256]; // Path to config file
    int disableAutomaticTimeInjection; // Flag to indicate whether to disable or enable automatic time injection
    int niSuplFlag; // Flag to indicate that tests being conducted is ni supl
    int printNmea; // Print nmea string
    int satelliteDetails; // Print detailed info on satellites in view.
    int fixThresholdInSecs; // User specified time to first fix threshold in seconds
    int zppTestCaseNumber; // Run specified ULP test case number
    int enableXtra; // Flag to enable/disable Xtra
    int stopOnMinSvs; // part of conditions that cause test to stop. Minimum number of SVs
    float stopOnMinSnr; // part of conditions that casue test to stop. Minimum number of SNR
    int tracking;    // start tracking session
} CommandLineOptionsType;


/** Default values for position/location */
static GpsLocation sPositionDefaultValues = {
    sizeof(GpsLocation),
    0,
    32.90285,
    -117.202185,
    0,
    0,
    0,
    10000,
    (GpsUtcTime)0,
};


// Default values
static CommandLineOptionsType sOptions = {
    GPS_POSITION_RECURRENCE_SINGLE,
    1,
    60, // Time to stop fix.
    1, // Android frame work (AFW)AGpsBearerType
    TRUE, // Yes to Legacy
    1, // ULP test case number
    0, // By default don't delete aiding data.
    GPS_POSITION_MODE_STANDALONE, // Standalone mode.
    1000, // 1000 millis between fixes
    0, // Accuracy
    0, // 1 millisecond?
    sPositionDefaultValues,
    {0}, // Invalid for NI response
    0, // Defaults to 0 back to back NI test
    0, // Number of elements in NI response pattern
    sAgpsCommandLineOptions, // default Agps values
    1, // Agps Success
    0, // Test Ril Interface
    g_xtra_client_config,
    "/etc/gps.conf", // Default path to config file
    0, // By default do not disable automatic time injection
    0, // NI SUPL flag defaults to zero
    0, // Print NMEA info. By default does not get printed
    0, // SV info. Does not print the detaul by default
    20, // Default value of 20 seconds for time to first fix
    4, // ZPP test case number
    1, // Xtra Enabled By default
    0, // Minimum number of SVs option off by default
    0, // Minimum number of SNR option off by default
    0  //Tracking Session status off by default
};


static loc_param_s_type cfg_parameter_table[] =
{
  {"SUPL_VER",     &sOptions.agpsData.suplVer,                         NULL, 'n'},
  {"SUPL_HOST",    &sOptions.agpsData.suplHost,                        NULL, 's'},
  {"SUPL_PORT",    &sOptions.agpsData.suplPort,                        NULL, 'n'},
  {"C2K_HOST",     &sOptions.agpsData.c2kHost,                         NULL, 's'},
  {"C2K_PORT",     &sOptions.agpsData.c2kPort,                         NULL, 'n'},
  {"NTP_SERVER",   &sOptions.xtraClientConfig.xtra_sntp_server_url[0], NULL, 's'},
  {"XTRA_SERVER_1",&sOptions.xtraClientConfig.xtra_server_url[0],      NULL, 's'},
  {"XTRA_SERVER_2",&sOptions.xtraClientConfig.xtra_server_url[1],      NULL, 's'},
  {"XTRA_SERVER_3",&sOptions.xtraClientConfig.xtra_server_url[2],      NULL, 's'}
};

#ifdef TEST_ULP
//prototypes
void test_ulp(int ulptestCase);
#endif /* TEST_ULP */
void test_zpp(int zpptestCase);
void run_tracking_session(void);

static uint64_t getUpTimeSec()
{
    struct timespec ts;

    ts.tv_sec = ts.tv_nsec = 0;
    clock_gettime(CLOCK_BOOTTIME, &ts);
    return ts.tv_sec + (ts.tv_nsec / 1000000000LL);
}

void mutex_init()
{
    pthread_mutex_init (&test_thread_mutex, NULL);
    pthread_cond_init (&test_thread_cond, NULL);

    pthread_mutex_init (&location_conn_mutex, NULL);
    pthread_cond_init (&location_conn_cond, NULL);

    pthread_mutex_init (&networkpos_req_mutex, NULL);
    pthread_cond_init (&networkpos_req_cond, NULL);

    pthread_mutex_init (&phone_context_mutex, NULL);
    pthread_cond_init (&phone_context_cond, NULL);

    pthread_mutex_init (&session_status_mutex, NULL);
    pthread_cond_init (&session_status_cond, NULL);

    pthread_mutex_init (&wait_count_mutex,NULL);
    pthread_cond_init (&wait_count_cond,NULL);

    pthread_mutex_init (&wait_atlcb_mutex,NULL);
    pthread_cond_init (&wait_atlcb_cond,NULL);

    pthread_mutex_init (&wait_xtratime_mutex,NULL);
    pthread_cond_init (&wait_xtratime_cond,NULL);

    pthread_mutex_init (&wait_xtradata_mutex,NULL);
    pthread_cond_init (&wait_xtradata_cond,NULL);

    pthread_mutex_init (&ulp_location_mutex, NULL);
    pthread_cond_init (&ulp_location_cond, NULL);

}

void mutex_destroy()
{
    pthread_mutex_destroy (&test_thread_mutex);
    pthread_cond_destroy (&test_thread_cond);

    pthread_mutex_destroy (&location_conn_mutex);
    pthread_cond_destroy (&location_conn_cond);

    pthread_mutex_destroy (&networkpos_req_mutex);
    pthread_cond_destroy (&networkpos_req_cond);

    pthread_mutex_destroy (&phone_context_mutex );
    pthread_cond_destroy (&phone_context_cond);

    pthread_mutex_destroy (&session_status_mutex);
    pthread_cond_destroy (&session_status_cond);

    pthread_mutex_destroy (&wait_count_mutex);
    pthread_cond_destroy (&wait_count_cond);

    pthread_mutex_destroy (&wait_atlcb_mutex);
    pthread_cond_destroy (&wait_atlcb_cond);

    pthread_mutex_destroy (&wait_xtratime_mutex);
    pthread_cond_destroy (&wait_xtratime_cond);

    pthread_mutex_destroy (&wait_xtradata_mutex);
    pthread_cond_destroy (&wait_xtradata_cond);

    pthread_mutex_destroy (&ulp_location_mutex );
    pthread_cond_destroy (&ulp_location_cond);

}

void garden_print(const char *fmt, ...)
{
    va_list ap;
    char buf[1024];
    va_start(ap, fmt);
    vsnprintf(buf, 1024, fmt, ap);
    va_end(ap);
    fprintf(stderr,"GARDEN: %s\n",buf);
    LOC_LOGV("%s", buf);
}

int timeval_difference (struct timeval *result, struct timeval *x, struct timeval *y)
{
       /* Perform the carry for the later subtraction by updating y. */
       if (x->tv_usec < y->tv_usec) {
         int nsec = (y->tv_usec - x->tv_usec) / 1000000 + 1;
         y->tv_usec -= 1000000 * nsec;
         y->tv_sec += nsec;
       }
       if (x->tv_usec - y->tv_usec > 1000000) {
         int nsec = (x->tv_usec - y->tv_usec) / 1000000;
         y->tv_usec += 1000000 * nsec;
         y->tv_sec -= nsec;
       }

       /* Compute the time remaining to wait.
          tv_usec is certainly positive. */
       result->tv_sec = x->tv_sec - y->tv_sec;
       result->tv_usec = x->tv_usec - y->tv_usec;

       /* Return 1 if result is negative. */
       return x->tv_sec < y->tv_sec;
}

/*  to get over the fact tha Pthread needs a function returning void * */
/*  but Android gps.h declares a fn which returns just a void. */

typedef void (*ThreadStart) (void *);
struct tcreatorData {
    ThreadStart pfnThreadStart;
    void* arg;
};

void *my_thread_fn (void *tcd)
{
    tcreatorData* local_tcd = (tcreatorData*)tcd;
    if (NULL != local_tcd) {
        local_tcd->pfnThreadStart (local_tcd->arg);
        free(local_tcd);
    }

    return NULL;
}

void action_open_atl() {

    garden_print ("got GPS_REQUEST_AGPS_DATA_CONN");
    pthread_mutex_lock (&test_thread_mutex);
    test_thread_action = ACTION_OPEN_ATL;
    pthread_cond_signal (&test_thread_cond);
    pthread_mutex_unlock (&test_thread_mutex);
}

void action_close_atl() {

    garden_print ("got GPS_RELEASE_AGPS_DATA_CONN");
    pthread_mutex_lock (&test_thread_mutex);
    test_thread_action = ACTION_CLOSE_ATL;
    pthread_cond_signal (&test_thread_cond);
    pthread_mutex_unlock (&test_thread_mutex);

}

void action_fail_atl() {

    pthread_mutex_lock (&test_thread_mutex);
    test_thread_action = ACTION_FAIL_ATL;
    pthread_cond_signal (&test_thread_cond);
    pthread_mutex_unlock (&test_thread_mutex);
}

void notify_main_thread_of_atlcb() {
    // Notify main thread everytime callback is invoked
    pthread_mutex_lock (&wait_atlcb_mutex);
    pthread_cond_signal (&wait_atlcb_cond);
    pthread_mutex_unlock (&wait_atlcb_mutex);
}

// location-xtra library calls this function back when data is ready
void test_xtra_system_data_callback(char * data,int length) {

    pthread_mutex_lock (&test_thread_mutex);

    g_xtra_data_file = fopen(XTRA_DATA_FILE_NAME,"w");

    if(g_xtra_data_file != NULL)
    {
        fwrite(data,1,length,g_xtra_data_file);
        // close file
        fclose(g_xtra_data_file);
    }

    // clear the buffer
    memset(g_xtra_data_buf,0,XTRA_DATA_BUF_LEN);

    // Set length of xtra data
    g_xtra_data_len = length;
    //copy the data to global buffer
    memcpy(g_xtra_data_buf, data, g_xtra_data_len);

    test_thread_action = ACTION_XTRA_DATA;

    // Signal data to test_thread that data is ready
    pthread_cond_signal (&test_thread_cond);

    pthread_mutex_unlock (&test_thread_mutex);
}

// location-xtra library calls this function back when time is ready
void test_xtra_system_time_callback(int64_t utcTime, int64_t timeReference, int uncertainty) {

    pthread_mutex_lock (&test_thread_mutex);

    g_xtra_time.time = utcTime;
    g_xtra_time.timeReference = timeReference;
    g_xtra_time.uncertainty = uncertainty;

    test_thread_action = ACTION_XTRA_TIME;

    // Signal data to test_thread that data is ready
    pthread_cond_signal (&test_thread_cond);

    pthread_mutex_unlock (&test_thread_mutex);
}

// Required by Xtra Client init function
XtraClientDataCallbacksType myXtraClientDataCallback = {
    test_xtra_system_data_callback
};

// Required by Xtra Client function
XtraClientTimeCallbacksType myXtraClientTimeCallback = {
    test_xtra_system_time_callback
};

/*
 * Helper thread for emulating the Android Framework.
 */
void *test_thread (void *args)
{
    garden_print ("Test Thread Enter");
    int rc = 0;
    pthread_mutex_lock (&test_thread_mutex);
    do {
        pthread_cond_wait (&test_thread_cond, &test_thread_mutex);
        /*
         * got a condition
         */
        garden_print ("test thread unblocked, action = %d",
                     test_thread_action);
        if (test_thread_action == ACTION_QUIT) {
            garden_print ("ACTION_QUIT");
            break;
        }
        switch (test_thread_action) {

        case ACTION_OPEN_ATL:
            garden_print ("ACTION_OPEN_ATL");
            /*
             * sleep(3);
             */
            loc_extended_agps_open(sOptions.agpsData.agpsType, sOptions.agpsData.apn,
                                   sOptions.agpsData.agpsBearerType);
            break;

        case ACTION_CLOSE_ATL:
            garden_print ("ACTION_CLOSE_ATL");
            loc_extended_agps_closed(sOptions.agpsData.agpsType);
            break;

        case ACTION_FAIL_ATL:
            garden_print ("ACTION_FAIL_ATL");
            loc_extended_agps_open_failed(sOptions.agpsData.agpsType);
            break;

        case ACTION_NI_NOTIFY:
            static int pc = 0;
            pc = pc % sOptions.niResPatCount; // Cycle through pattern
            if(sOptions.networkInitiatedResponse[pc] != 3) // Don't send response if "No response" is in pattern
            {
                loc_extended_ni_respond(sNotification.notification_id,
                                        sOptions.networkInitiatedResponse[pc]);
            }
            pc++;
            break;

        case ACTION_XTRA_DATA:
            garden_print ("ACTION_XTRA_DATA");
            /*size_t len = (10 * 1024);
            char *buf = (char *) malloc (len); */
            rc = loc_extended_xtra_inject_data(g_xtra_data_buf, g_xtra_data_len);
            garden_print ("inject_xtra_data returned %d", rc);
            break;
        case ACTION_XTRA_TIME:
            garden_print ("ACTION_XTRA_TIME");
            rc = loc_extended_inject_time(g_xtra_time.time, g_xtra_time.timeReference,
                                          g_xtra_time.uncertainty);
            garden_print ("inject_time returned %d", rc);
            break;

#ifdef TEST_ULP
        case ACTION_NLP_RESPONSE:
            garden_print("ACTION_NLP_RESPONSE \n Simulating AFW injection of network location after 5 secs\n");
            sleep(5);
            pthread_mutex_lock (&networkpos_req_mutex);
            if((pUlpNetworkInterface != NULL) && (pUlpNetworkInterface->ulp_send_network_position != NULL)){
              UlpNetworkPositionReport position_report;
              position_report.valid_flag = ULP_NETWORK_POSITION_REPORT_HAS_POSITION;
              position_report.position.latitude = 32.7;
              position_report.position.longitude = -119;
              position_report.position.HEPE = 1000;
              position_report.position.pos_source = ULP_NETWORK_POSITION_SRC_UNKNOWN;
              pUlpNetworkInterface->ulp_send_network_position(&position_report);
            }

            pthread_cond_signal (&networkpos_req_cond);
            pthread_mutex_unlock (&networkpos_req_mutex);
            break;

        case ACTION_PHONE_CONTEXT_UPDATE:
            pthread_mutex_lock (&phone_context_mutex);
            garden_print("ACTION_PHONE_CONTEXT_UPDATE \n Simulating AFW injection of phone context info\n");
            if(pUlpPhoneContextInterface != NULL) {
              UlpPhoneContextSettings settings;
              settings.context_type = ULP_PHONE_CONTEXT_GPS_SETTING |
                                      ULP_PHONE_CONTEXT_NETWORK_POSITION_SETTING |
                                      ULP_PHONE_CONTEXT_WIFI_SETTING |
                                      ULP_PHONE_CONTEXT_AGPS_SETTING;
              settings.is_gps_enabled = is_gps_enabled;
              settings.is_agps_enabled = is_agps_enabled;
              settings.is_network_position_available = is_network_position_available;
              settings.is_wifi_setting_enabled = is_wifi_setting_enabled;
              pUlpPhoneContextInterface->ulp_phone_context_settings_update(&settings);

            }
            pthread_cond_signal (&phone_context_cond);
            pthread_mutex_unlock (&phone_context_mutex);
            break;
#endif
        default:
            break;
        }
        test_thread_action = ACTION_NONE;

    } while (1);
    pthread_mutex_unlock (&test_thread_mutex);

    garden_print ("Test Thread Exit");
    return NULL;
}

void test_gps_location_cb (GpsLocation * location)
{
    static int callCount = 1;
    static int ttff_meas = 0;
    struct timeval result = {0,0};
    int64_t ttffSecs = 0;
    if(callCount) {
        gettimeofday(&TimeAtFirstFix,NULL);
    }
    garden_print ("## gps_location_callback ##:");
    garden_print("LAT: %f, LON: %f, ACC: %f, TIME: %llu",
                 location->latitude, location->longitude, location->accuracy,
                 (long long) location->timestamp);

    if(!ttff_meas)
    {
        firstFixTime = getUpTimeSec();
        garden_print("TTFF in Secs:: %ld, StartTime: %ld, FirstFixTime: %ld",
                     (firstFixTime - startTime), startTime, firstFixTime );
        ttff_meas = 1;
    }

    if(callCount) {
        timeval_difference(&result,&TimeAtFirstFix, &TimeAtStartNav);
        ttffSecs = (int64_t)result.tv_sec + (int64_t)result.tv_usec/(int64_t)1000000;
        garden_print("Time to First Fix in Secs:: %ld",ttffSecs);
    }
    //check if time to first fix is within threshold value
    if(ttffSecs <= sOptions.fixThresholdInSecs) {
        g_exitStatus = 1;
    }
    else {
        g_exitStatus = -1;
    }
    pthread_mutex_lock (&location_conn_mutex);
    pthread_cond_signal (&location_conn_cond);
    pthread_mutex_unlock (&location_conn_mutex);
}

void test_ulp_location_cb (UlpLocation * location)
{
    garden_print("======================================================");
    garden_print ("ulp_location_cb :");
    garden_print("LAT: %f, LON: %f, ACC: %f, TIME: %llu",
                 location->gpsLocation.latitude, location->gpsLocation.longitude,
                 location->gpsLocation.accuracy,(long long) location->gpsLocation.timestamp);
    garden_print("======================================================");
    pthread_mutex_lock (&ulp_location_mutex);
    pthread_cond_signal (&ulp_location_cond);
    pthread_mutex_unlock (&ulp_location_mutex);
}

void test_gps_status_cb (GpsStatus * status)
{

    garden_print("## gps_status_callback ##:: GPS Status: %d",status->status);

    if(status->status == GPS_STATUS_ENGINE_ON)
    {
        g_checkForEngineOff = 1;
    }

    if(status->status == GPS_STATUS_ENGINE_OFF)
    {
        pthread_mutex_lock (&session_status_mutex);
        pthread_cond_signal (&session_status_cond);
        pthread_mutex_unlock (&session_status_mutex);
    }
}

void test_gps_sv_status_cb (GpsSvStatus * sv_info)
{

    garden_print ("## gps_sv_status_callback ##:: Number of SVs: %d",sv_info->num_svs);
    if(sOptions.satelliteDetails) {
        for(int i=0;i<sv_info->num_svs; ++i)
        {
            garden_print("%02d : PRN: %04d, SNR: %09.4f, ELE: %09.4f, AZI: %09.4f",
                          i+1,sv_info->sv_list[i].prn,sv_info->sv_list[i].snr, sv_info->sv_list[i].elevation,
                          sv_info->sv_list[i].azimuth);
        }
        garden_print("Ephemeris Mask : 0x%X",sv_info->ephemeris_mask);
        garden_print("Almanac Mask: 0x%X",sv_info->almanac_mask);
        garden_print("Used in Fix Mask: 0x%X:",sv_info->used_in_fix_mask);
    }

    // if the minimum number of SVs with minimum number of SNR
    // has been satisfied then stop session
    if (sOptions.stopOnMinSvs && sOptions.stopOnMinSnr) {
        if (sOptions.stopOnMinSvs <= sv_info->num_svs) {
            int minCnSvCount = 0;
            // count number of SVs that meet the min SNR
            for(int i=0; i<sv_info->num_svs; ++i) {
                if (sv_info->sv_list[i].snr >= sOptions.stopOnMinSnr) {
                    minCnSvCount++;
                }
            }
            if (minCnSvCount >= sOptions.stopOnMinSvs){
                garden_print("Stop test, as %d SVs are seen with at least a SNR of %f",
                              sOptions.stopOnMinSvs, sOptions.stopOnMinSnr);
                pthread_mutex_lock (&location_conn_mutex);
                pthread_cond_signal (&location_conn_cond);
                pthread_mutex_unlock (&location_conn_mutex);
            }
        }
    }

}

void test_gps_nmea_cb (GpsUtcTime timestamp, const char *nmea, int length)
{
    if(sOptions.printNmea) {

        garden_print ("## gps_nmea_callback ##:: Timestamp: %d String:%s Length:%d",
                                           timestamp,nmea,length);

    }
}

void test_gps_set_capabilities_cb (uint32_t capabilities)
{

    garden_print("## gps_set_capabilities ##:");
    garden_print("Capabilities: 0x%x",capabilities);

}

void test_gps_acquire_wakelock_cb ()
{

    garden_print ("## gps_acquire_wakelock ##:");

}

void test_gps_release_wakelock_cb ()
{

    garden_print ("## gps_release_wakelock ##:");

}

pthread_t test_gps_create_thread_cb (const char *name, void (*start) (void *),
                                void *arg)
{
    garden_print("## gps_create_thread ##:");
    pthread_t thread_id = -1;
    garden_print ("%s", name);

    tcreatorData* tcd = (tcreatorData*)malloc(sizeof(*tcd));

    if (NULL != tcd) {
        tcd->pfnThreadStart = start;
        tcd->arg = arg;

        if (0 > pthread_create (&thread_id, NULL, my_thread_fn, (void*)tcd)) {
            garden_print ("error creating thread");
            free(tcd);
        } else {
            garden_print ("created thread");
        }
    }


    return thread_id;
}

void test_gps_xtra_download_req_cb ()
{


    garden_print ("## gps_xtra_download_request ##:");
    XtraClientInterfaceType const * pXtraClientInterface = get_xtra_client_interface();

    // Pass on the data request to Xtra Client
    pXtraClientInterface->onDataRequest();

    pthread_mutex_lock (&wait_xtradata_mutex);
    pthread_cond_signal (&wait_xtradata_cond);
    pthread_mutex_unlock (&wait_xtradata_mutex);

    // No need for test_main to wait as this method has been called
    g_checkForXtraDataCallBack = 0;
}

void test_gps_xtra_time_req_cb ()
{

    garden_print ("## gps_request_utc_time ##:");
    XtraClientInterfaceType const * pXtraClientInterface = get_xtra_client_interface();

    // Pass on the time request to Xtra Client
    pXtraClientInterface->onTimeRequest();


    pthread_mutex_lock (&wait_xtratime_mutex);
    pthread_cond_signal (&wait_xtratime_cond);
    pthread_mutex_unlock (&wait_xtratime_mutex);
}

void test_report_xtra_server_callback(const char* server1, const char* server2, const char* server3)
{
    garden_print ("## report_xtra_server_callback ##:");

}


static void test_ulp_request_phone_context_cb(UlpPhoneContextRequest *req)
{
 sleep (2);
 garden_print ("test_ulp_request_phone_context with context_type: %x,request_type: %d ",
           req->context_type, req->request_type );

 pthread_mutex_lock (&test_thread_mutex);
 test_thread_action = ACTION_PHONE_CONTEXT_UPDATE;
 pthread_cond_signal (&test_thread_cond);
 pthread_mutex_unlock (&test_thread_mutex);
}

static void test_ulp_network_location_request_cb(UlpNetworkRequestPos* req)
{
    garden_print ("test_ulp_network_location_request_cb with request_type: %d,interval %d, desired_position_source: %d ",
           req->request_type , req->interval_ms , req->desired_position_source );

    if (req->request_type == ULP_NETWORK_POS_STOP_REQUEST )
    {
       garden_print ("received network provider stop request\n");
    }
    else
    {
       sleep (1);
       pthread_mutex_lock (&test_thread_mutex);
       test_thread_action = ACTION_NLP_RESPONSE;
       pthread_cond_signal (&test_thread_cond);
       pthread_mutex_unlock (&test_thread_mutex);
    }
}
void test_agps_status_cb (AGpsExtStatus * status)
{

    garden_print ("## test_agps_status_cb ##");
    if(sOptions.niSuplFlag) {
        static int niCount = 1;
        if(niCount <= sOptions.niCount) {
            if(sOptions.isSuccess)
            {
                if (status->status == GPS_REQUEST_AGPS_DATA_CONN) {
                    action_open_atl();
                }
                else if (status->status == GPS_RELEASE_AGPS_DATA_CONN) {
                    action_close_atl();
                }
            }
            else
            {
                garden_print ("got status %d, sending failure ", status->status);
                action_fail_atl();
            }
        }
        notify_main_thread_of_atlcb();
        niCount++;
    }
    else
    {
        if(sOptions.isSuccess)
        {
            if (status->status == GPS_REQUEST_AGPS_DATA_CONN) {
                action_open_atl();
            }
            else if (status->status == GPS_RELEASE_AGPS_DATA_CONN) {
                action_close_atl();
            }
        }
        else
        {
            garden_print ("got status %d, sending failure ", status->status);
            action_fail_atl();
        }
    }

}

static void test_gps_ni_notify_cb(GpsNiNotification *notification)
{

    static int niCount = 1;
    garden_print ("## gps_ni_notify_callback ##:%d",niCount);

    sNotification = *notification;
    garden_print ("ACTION_NI_NOTIFY: notification_id %d, ni_type %d, notify_flags %d, timeout %d, default_response %d, requestor_id %s, text %s, requestor_id_encoding %d, text_encoding %d, extras %s\n",
        sNotification.notification_id,
        (int) sNotification.ni_type,
        (int) sNotification.notify_flags,
        sNotification.timeout,
        sNotification.default_response,
        sNotification.requestor_id,
        sNotification.text,
        sNotification.requestor_id_encoding,
        sNotification.text_encoding,
        sNotification.extras);


    // Do not notify test thread if callback count exceeds command line option
    // for number of back to back tests
    if(niCount <= sOptions.niCount)
    {
        pthread_mutex_lock (&test_thread_mutex);
        test_thread_action = ACTION_NI_NOTIFY;
        pthread_cond_signal (&test_thread_cond);
        pthread_mutex_unlock (&test_thread_mutex);
    }
    // Notify main thread everytime callback is invoked.
    pthread_mutex_lock (&wait_count_mutex);
    pthread_cond_signal (&wait_count_cond);
    pthread_mutex_unlock (&wait_count_mutex);
    niCount++;
}


GpsCallbacks myCallbacks = {
    sizeof (GpsCallbacks),
    test_gps_location_cb,
    test_gps_status_cb,
    test_gps_sv_status_cb,
    test_gps_nmea_cb,
    test_gps_set_capabilities_cb,
    test_gps_acquire_wakelock_cb,
    test_gps_release_wakelock_cb,
    test_gps_create_thread_cb,
    test_gps_xtra_time_req_cb
};

GpsXtraExtCallbacks myGpsXtraExtCallbacks = {
    test_gps_xtra_download_req_cb,
    test_gps_create_thread_cb,
    test_report_xtra_server_callback,
};

#ifdef TEST_ULP
UlpEngineCallbacks sUlpEngineCallbacks = {
    sizeof(UlpEngineCallbacks),
    test_ulp_location_cb,
    test_gps_create_thread_cb
};

UlpPhoneContextCallbacks pUlpPhoneContextCallbacks = {
    test_ulp_request_phone_context_cb,
};

UlpNetworkLocationCallbacks pUlpNetworkLocationCallbacks = {
    test_ulp_network_location_request_cb,
};
#endif

AGpsExtCallbacks myAGpsExtCallbacks = {
    test_agps_status_cb,
    test_gps_create_thread_cb
};

GpsExtCallbacks myGpsExtCallbacks = {
    sizeof(GpsExtCallbacks),
    test_gps_set_capabilities_cb,
    test_gps_acquire_wakelock_cb,
    test_gps_release_wakelock_cb,
    test_gps_create_thread_cb,
    test_gps_xtra_time_req_cb
};

GpsNiExtCallbacks myGpsNiExtCallbacks = {
    test_gps_ni_notify_cb,
    test_gps_create_thread_cb
};

extern const GpsInterface *gps_get_hardware_interface ();

/*=============================================================================
  FUNCTION test_main

    Android Framework simulation entry point.

    Includes individual tests for specific features.

    Search the makefile for "TEST_ANDROID_GPS_FLAGS" and uncomment the
    flags that correspond to desired tests.

=============================================================================*/
int test_main ()
{
    int rc = 0;
    uint32_t port = 10001;
    uint32_t targetType;
    pthread_t tid;
    int r;
    test_thread_action = ACTION_NONE;

    mutex_init();

    pthread_create (&tid, NULL, test_thread, NULL);

    startTime = firstFixTime = 0;
    startTime = getUpTimeSec();

#ifdef _ANDROID_
    int err;
    hw_module_t* module;
    err = hw_get_module(GPS_HARDWARE_MODULE_ID, (hw_module_t const**)&module);
    if (err == 0) {
        hw_device_t* device;
        err = module->methods->open(module, GPS_HARDWARE_MODULE_ID, &device);
        if (err == 0) {
            gps_device_t* gps_device = (gps_device_t *)device;
            pGpsInterface = gps_device->get_gps_interface(gps_device);
        }
    }
#else
    pGpsInterface = (GpsInterface *) gps_get_hardware_interface ();
#endif

    if (NULL == pGpsInterface) {
        mutex_destroy();
        garden_print(" Could not get a handle to GpsInterface, Cannot proceed ");
        return -1;
    }

#ifdef _ANDROID_
    targetType = getTargetGnssType(loc_get_target());
    if((sOptions.tracking == 1) && (GNSS_AUTO == targetType))
    {
      /*
      ** decouple from qmuxd,  or else QMI-LOC Channel would block until
      ** qmuxd is available. QMI-LOC channel would not use qmuxd,
      ** qmi_cci_qmux_xport_unregister() call is process specific, hence
      ** decoupling from process context would be better
      */
      garden_print("decouple qmuxd -- qmi_cci_qmux_xport_unregister");
      qmi_cci_qmux_xport_unregister(QMI_CLIENT_QMUX_RMNET_INSTANCE_0);
      qmi_cci_qmux_xport_unregister(QMI_CLIENT_QMUX_RMNET_SDIO_INSTANCE_0);
      qmi_cci_qmux_xport_unregister(QMI_CLIENT_QMUX_RMNET_USB_INSTANCE_0);
      qmi_cci_qmux_xport_unregister(QMI_CLIENT_QMUX_RMNET_SMUX_INSTANCE_0);
    }
#endif

    rc = pGpsInterface->init(&myCallbacks);

    garden_print ("initialize GPS interface returned %d", rc);

    if (sOptions.s & 2) {
        loc_ext_init();
    }

    loc_extended_init(&myGpsExtCallbacks);
    loc_extended_agps_init(&myAGpsExtCallbacks);
    loc_extended_ni_init(&myGpsNiExtCallbacks);

    pAgpsRilInterface = (AGpsRilInterface *) pGpsInterface->get_extension (AGPS_RIL_INTERFACE);
    if (NULL == pAgpsRilInterface) {
        //mutex_destroy();
        garden_print("Could not get a handle to AgpsRilInterface...but proceeding");
        //return -1;
    }

    // Initialize XTRA client
    XtraClientInterfaceType const* pXtraClientInterface = get_xtra_client_interface();
    pXtraClientInterface->init(&myXtraClientDataCallback, &myXtraClientTimeCallback, &(sOptions.xtraClientConfig));

    if(!sOptions.disableAutomaticTimeInjection)
    {
        /* Force time injection from xtra client */
        test_gps_xtra_time_req_cb();
    }

    loc_extended_xtra_init(&myGpsXtraExtCallbacks);

    // Initialize AGPS server settings
    if(sOptions.agpsData.agpsType == AGPS_TYPE_SUPL)
    {
        rc = loc_extended_agps_set_server(sOptions.agpsData.agpsType,
                                          sOptions.agpsData.suplHost,
                                          sOptions.agpsData.suplPort );

    }
    else if(sOptions.agpsData.agpsType == AGPS_TYPE_C2K)
    {
       rc = loc_extended_agps_set_server(sOptions.agpsData.agpsType,
                                         sOptions.agpsData.c2kHost,
                                         sOptions.agpsData.c2kPort );
    }

    if(sOptions.deleteAidingDataMask != 0){
        pGpsInterface->delete_aiding_data (sOptions.deleteAidingDataMask);
    }

    if(sOptions.location.flags != 0 ) {
        pGpsInterface->inject_location(sOptions.location.latitude, sOptions.location.longitude, sOptions.location.accuracy);
    }

    ulp_init(&sUlpEngineCallbacks, &pUlpNetworkLocationCallbacks, &pUlpPhoneContextCallbacks);
    pUlpEngineInterface =
        (const UlpEngineInterface*)ulp_get_extension(ULP_ENGINE_INTERFACE);
    pUlpNetworkInterface =
        (UlpNetworkInterface*)ulp_get_extension(ULP_NETWORK_INTERFACE);
    pUlpPhoneContextInterface =
           (const UlpPhoneContextInterface*)ulp_get_extension(ULP_PHONE_CONTEXT_INTERFACE);
    if((NULL == pUlpEngineInterface)||
       (NULL == pUlpNetworkInterface)||
       (NULL == pUlpPhoneContextInterface)) {
        LOC_LOGE("Error in classInit.ulp_get_extension is null ");
    }


#ifdef TEST_ULP
    if((sOptions.ulpTestCaseNumber > 0) && (sOptions.ulpTestCaseNumber < 13)) {
        pthread_mutex_lock (&phone_context_mutex);
        garden_print ("Waiting for Ulp Test to finish...");
        garden_print ("Running ULP test case number: %d",sOptions.ulpTestCaseNumber);
        test_ulp(sOptions.ulpTestCaseNumber);
        pthread_cond_wait (&phone_context_cond, &phone_context_mutex);
        pthread_mutex_unlock (&phone_context_mutex);
        sleep(3);
        garden_print ("Ulp Test - 1 session complted!");
    }

#endif
   if((sOptions.zppTestCaseNumber > 0) && (sOptions.zppTestCaseNumber < 5)) {
        garden_print ("Running ZPP test case number: %d",sOptions.zppTestCaseNumber);
        test_zpp(sOptions.zppTestCaseNumber);
    } else
    {
        garden_print ("Invalid ZPP test case number: %d",sOptions.zppTestCaseNumber);
    }


    for(int k=0;k<sOptions.l; ++k)
    {
        garden_print("Session %d:",k);
        if (NULL != pGpsInterface) {
           rc = pGpsInterface->set_position_mode (sOptions.positionMode,
                                               sOptions.r , sOptions.interval,
                                               sOptions.accuracy,sOptions.responseTime);
            garden_print ("set_position_mode returned %d", rc);

            gettimeofday(&TimeAtStartNav,NULL);
            rc = pGpsInterface->start ();
            garden_print ("start GPS interface returned %d", rc);
        }

        if (sOptions.s & 2) {
            rc = loc_ext_set_position_mode(sOptions.positionMode,
                                           sOptions.r == GPS_POSITION_RECURRENCE_SINGLE,
                                           sOptions.interval, sOptions.responseTime, NULL);
            garden_print ("vzw_set_position_mode returned %d", rc);

            rc = loc_ext_start();
            garden_print ("vzw start GPS interface returned %d", rc);
        }

        if (sOptions.r == GPS_POSITION_RECURRENCE_SINGLE) {
            struct timespec ts;

            garden_print ("Waiting for location callback...");

            pthread_mutex_lock (&location_conn_mutex);
            clock_gettime(CLOCK_REALTIME, &ts);
            ts.tv_sec += sOptions.t;

            pthread_cond_timedwait(&location_conn_cond, &location_conn_mutex, &ts);
            pthread_mutex_unlock (&location_conn_mutex);
        } else {
            garden_print ("sleep for %d seconds", sOptions.t);
            sleep(sOptions.t);
        }

        if( (pAgpsRilInterface != NULL) && (sOptions.rilInterface) ) {
            pAgpsRilInterface->update_network_availability(88, "MY_APN_FOR_RIL_TEST");
            garden_print ("Ril Interface test Done...");
        }


        rc = pGpsInterface->stop ();
        garden_print ("stop GPS interface returned %d", rc);

        if (sOptions.s & 2) {
            rc = loc_ext_stop();
            garden_print ("vzw stop GPS interface returned %d", rc);
        }
    } // End of for loop for sessions

    int niCount = 1;
    for(;niCount<= sOptions.niCount;++niCount)
    {
        //wait for NI back to back test to finish
        garden_print("Waiting for NI Back to back tests to finish...");
        pthread_mutex_lock (&wait_count_mutex);
        pthread_cond_wait(&wait_count_cond, &wait_count_mutex);
        pthread_mutex_unlock (&wait_count_mutex);

        // IF NI SUPL back to back tests are being conducted
        // wait for atl also
        if(sOptions.niSuplFlag) {
            // If NI SUPL tests are being conducted wait for ATL callback
            garden_print("Waiting for ATL Callback...");
            pthread_mutex_lock (&wait_atlcb_mutex);
            pthread_cond_wait(&wait_atlcb_cond, &wait_atlcb_mutex);
            pthread_mutex_unlock (&wait_atlcb_mutex);
        }

    }

    if(g_checkForEngineOff) {
        garden_print ("Waiting for Engine off...");
        pthread_mutex_lock (&session_status_mutex);
        pthread_cond_wait(&session_status_cond, &session_status_mutex);
        pthread_mutex_unlock (&session_status_mutex);
    }

    /*
    // Wait for xtra time call back
    garden_print("Waiting for XTRA time call back ...");
    pthread_mutex_lock (&wait_xtratime_mutex);
    pthread_cond_wait (&wait_xtratime_cond,&wait_xtratime_mutex);
    pthread_mutex_unlock (&wait_xtratime_mutex); */

    // Wait for xtra data call back
    if(sOptions.enableXtra && g_checkForXtraDataCallBack) {
        garden_print("Waiting for XTRA data call back ...");
        pthread_mutex_lock (&wait_xtradata_mutex);
        pthread_cond_wait (&wait_xtradata_cond,&wait_xtradata_mutex);
        pthread_mutex_unlock (&wait_xtradata_mutex);
    }

    /*********
    *
    *   ## IMPORTANT ##
    *   This tracking option will start a tracking session in continous loop
    *   Alwasys keep this as a last test case and add new test cases
    *   before this option
    *
    ******* */
    if(sOptions.tracking == 1)
    {
       g_run_active_client = TRUE;
       run_tracking_session();
    }

    pGpsInterface->cleanup ();

    // Stop Xtra Client and wait for it to exit
    garden_print("Shutting down Xtra Client ....");
    pXtraClientInterface->stop();
    garden_print("Xtra Client shutdown complete");

    if (sOptions.s & 2)
        loc_ext_cleanup();

    garden_print ("cleanup GPS interface returned ");


    pthread_mutex_lock (&test_thread_mutex);
    test_thread_action = ACTION_QUIT;
    pthread_cond_signal (&test_thread_cond);
    pthread_mutex_unlock (&test_thread_mutex);

    garden_print ("wait for pthread_join");

    void *ignored;
    pthread_join (tid, &ignored);
    garden_print ("pthread_join done");

    mutex_destroy();

/* #ifndef UDP_XPORT */
/*   ipc_router_core_deinit(); */
/* #endif */
    garden_print ("GARDEn Tests Finished!");

    return g_exitStatus;
}

void printHelp(char **arg)
{
     garden_print("usage: %s -r <1|0> -t <xxx> -u <1-12>", arg[0]);
     garden_print("    -r:  RECURRENCE 1:SINGLE; 0:PERIODIC; Defaults: %d", sOptions.r);
     garden_print("    -l:  Number of sessions to loop through. Takes an argument. An argument of 0 means no sessions. Defaults:%d", sOptions.l);
     garden_print("    -t:  User defined length of time to issue stop navigation. Takes an argument. Time in seconds Defaults: %d", sOptions.t);
     garden_print("    -b:  run backwards compatibility tests (legacy mode). Takes and argument 1 for true and 0 for false. Defaults: %d ",(int)sOptions.b);
     garden_print("    -u:  run specified ULPLite test case Defaults: %d", sOptions.ulpTestCaseNumber);
     garden_print("    -s:  stacks to test: 1:afw; 2:vzw; 3:(afw | vzw, default) Defaults: %d",sOptions.s);
     garden_print("    -d:  Delete aiding data: Takes a hexadecimal mask as an argument as given in gps.h Defaults: 0x%X ",sOptions.deleteAidingDataMask);
     garden_print("    -m:  Position Mode. Takes an argument 0:GPS_POSITION_MODE_STANDALONE, 1:GPS_POSITION_MODE_MS_BASED, 2:GPS_POSITION_MODE_MS_ASSISTED Defaults: %d ", sOptions.positionMode);
     garden_print("    -i:  Interval. Takes an argument. Time in milliseconds between fixes Defaults: %d", sOptions.interval);
     garden_print("    -a:  Accuracy. Takes an argument. Accuracy in meters Defaults: %d ", sOptions.accuracy);
     garden_print("    -x:  Response Time. Takes an argument. Requested time to first fix in milliseconds Defaults: %d" , sOptions.responseTime);
     garden_print("    -P:  Inject Position. Takes 3 arguments seperated by a COMMA. Latitude, Longitude, and accuracy Defaults: %f,%f,%d ",sOptions.location.latitude,sOptions.location.longitude,(int)sOptions.location.accuracy);
     garden_print("    -N:  Network Initiated. Takes 2 arguments separated by COMMA. First being the number of back to back NI tests and second being a COMMA separated pattern of  1:Accept, 2:Deny,or 3:No Response Defaults: %d:%d",sOptions.niCount,0);
     for(int i = 0; i<sOptions.niResPatCount; ++i) { garden_print("%12d",sOptions.networkInitiatedResponse[i]); }
     garden_print("    -S:  ATL Success. Takes 3 arguments seperated by a COMMA. AgpsType, Apn, AgpsBearerType Defaults: %d,%s,%d,0x%X,%s,%d,%s,%d ", (int)sOptions.agpsData.agpsType, sOptions.agpsData.apn, (int)sOptions.agpsData.agpsBearerType,(int)sOptions.agpsData.suplVer,sOptions.agpsData.suplHost,sOptions.agpsData.suplPort,sOptions.agpsData.c2kHost,sOptions.agpsData.c2kPort);
     garden_print("    -f:  ATL Failure. Takes 1 argument. AgpsType Defaults: %d ", sOptions.agpsData.agpsType);
     garden_print("    -g:  Test Ril Interface. Takes an argument 0 or 1. Defaults: %d",sOptions.rilInterface);
     garden_print("    -D:  Xtra Data. Takes 3 arguments separated by a COMMA. The 3 arguments being 3 valid xtra data server urls. Defaults: \n%56s\n%56s\n%56s",sOptions.xtraClientConfig.xtra_server_url[0], sOptions.xtraClientConfig.xtra_server_url[1],sOptions.xtraClientConfig.xtra_server_url[2]);
     garden_print("    -w:  Xtra Time. Takes an argument. The argument must be a valid sntp server url. Defaults: \n%38s", sOptions.xtraClientConfig.xtra_sntp_server_url[0]);
     garden_print("    -e:  Xtra User Agent string. Takes an argument. The argument is a string. Defaults: %s",sOptions.xtraClientConfig.user_agent_string);
     garden_print("    -c:  gps.conf file. Takes an argument. The argument is the path to gps.conf file. Defaults: %s",sOptions.gpsConfPath);
     garden_print("    -j:  Using this option will disable garden from doing an automatic time injection. Takes an argument 0: Enable, 1: Disable. Defaults: %d",sOptions.disableAutomaticTimeInjection);
     garden_print("    -k:  used in conjunction with -N option to indicate that the test being conducted is an NI SUPL test. Takes no arguments.");
     garden_print("    -n:  Use this option to print nmea string, timestamp and length. Takes no arguments. Defaults:%d",sOptions.printNmea);
     garden_print("    -y:  Use this option to print detailed info on satellites in view. Defaults:%d",sOptions.satelliteDetails);
     garden_print("    -o:  This option, when used, will enforce a check to determine if time to first fix is within a given threshold value. Takes one argument, the threshold value in seconds. Defaults: %d",sOptions.fixThresholdInSecs);
     garden_print("    -q:  This option is used to enable/disable XTRA. Takes one argument. 0:disable, 1:enable. Defaults to %d", sOptions.enableXtra);
     garden_print("    -A:  Minimum number of SVs seen in combination with -B option to determine when to stop the test without actually getting a position report to save test time");
     garden_print("    -B:  Minimum SNR for each SV seen in -A option to determine when to stop the test  without actually getting a position report to save test time");
     garden_print("    -T:  Start a tracking session *WARNING* this tracking session will run until process is alive Ctrl-C to exit");
     garden_print("    -Z:  ZPP Test case number 1 to 5 - Default: 4");
     garden_print("    -h:  print this help");
}


int main (int argc, char *argv[])
{
    int result = 0;
    int opt;
    extern char *optarg;
    char *argPtr;
    char *tokenPtr;
    LEGACY = FALSE;

    while ((opt = getopt (argc, argv, "r:l:t:u:h:b:s:d:m:i:a:x:P:N:S:f:g:D:w:c:j:knye:o:z:q:A:B:T::Z:")) != -1) {
        switch (opt) {
        case 'r':
            sOptions.r = atoi(optarg);
            garden_print("Recurrence:%d",sOptions.r);
            break;
        case 'l':
            sOptions.l = atoi(optarg);
            garden_print("Number of Sessions to loop through:%d",sOptions.l);
            break;
        case 't':
            sOptions.t = atoi(optarg);
            garden_print("User defined length of time to issue stop navigation:%d",sOptions.t);
            break;
        case 'b':
            sOptions.b = atoi(optarg);
            garden_print("Run backward compatibility tests:%d",sOptions.b);
            break;
        case 'u':
            sOptions.ulpTestCaseNumber = atoi(optarg);
            garden_print("ulptestCase number: %d \n",sOptions.ulpTestCaseNumber);
            break;
        case 's':
            sOptions.s = atoi(optarg);
            garden_print("Stacks to test:%d",sOptions.s);
            break;
        case 'd':
            sOptions.deleteAidingDataMask = strtoll(optarg,NULL,16);
            garden_print("Delete Aiding Mask:%x",sOptions.deleteAidingDataMask);
            break;
        case 'm':
            sOptions.positionMode = atoi(optarg);
            garden_print("Position Mode:%d",sOptions.positionMode);
            break;
        case 'i':
            sOptions.interval = atoi(optarg);
            garden_print("Interval:%d",sOptions.interval);
            break;
        case 'a':
            sOptions.accuracy = atoi(optarg);
            garden_print("Accuracy:%d",sOptions.accuracy);
            break;
        case 'x':
            sOptions.responseTime = atoi(optarg);
            garden_print("Response Time:%d",sOptions.responseTime);
            break;
        case 'P':
            sOptions.location.flags = 0x0011;
            tokenPtr = strtok_r(optarg, ",", &argPtr);
            if(tokenPtr != NULL) {
                sOptions.location.latitude = atof(tokenPtr);
                tokenPtr = strtok_r(NULL, ",", &argPtr);
                if(tokenPtr != NULL) {
                    sOptions.location.longitude = atof(tokenPtr);
                    tokenPtr = strtok_r(NULL, ",", &argPtr);
                    if(tokenPtr != NULL) {
                        sOptions.location.accuracy = atoi(tokenPtr);
                    }
                }
            }
            garden_print("Inject Position:: flags:%x, lat:%f, lon:%f, acc:%d",sOptions.location.flags,
                    sOptions.location.latitude, sOptions.location.longitude,sOptions.location.accuracy);
            break;
        case 'N':
            // Number of back to back tests
            tokenPtr = strtok_r(optarg, ",", &argPtr);
            if(tokenPtr != NULL) {
                sOptions.niCount = atoi(tokenPtr);
                if(sOptions.niCount > 0)
                {
                   char *ret;
                   while((ret = strtok_r(NULL, ",", &argPtr)) != NULL)
                   {
                      sOptions.networkInitiatedResponse[sOptions.niResPatCount++] = atoi(ret);
                   }
                }
            }
            garden_print("Number of back to back NI tests : %d",sOptions.niCount);
            break;
        case 'S':
            tokenPtr = strtok_r(optarg, ",", &argPtr);
            if(tokenPtr != NULL) {
                sOptions.agpsData.agpsType = (AGpsType)atoi(tokenPtr);
                tokenPtr = strtok_r(NULL, ",", &argPtr);
                if(tokenPtr != NULL) {
                    sOptions.agpsData.apn = tokenPtr;
                    tokenPtr = strtok_r(NULL, ",", &argPtr);
                    if(tokenPtr != NULL) {
                        sOptions.agpsData.agpsBearerType = (AGpsBearerType)atoi(tokenPtr);
                    }
                }
            }
            sOptions.isSuccess = 1;
            garden_print("Success:: Agps Type:%d, apn:%s, Agps Bearer Type:%d, success:%d",
            sOptions.agpsData.agpsType, sOptions.agpsData.apn, sOptions.agpsData.agpsBearerType, sOptions.isSuccess);
            break;
        case 'f':
            sOptions.agpsData.agpsType = (AGpsType)atoi(optarg);
            sOptions.isSuccess = 0; // Failure
            garden_print("Failure:: Agps type:%d, success:%d",
            sOptions.agpsData.agpsType, sOptions.isSuccess);
            break;
        case 'g':
            sOptions.rilInterface = atoi(optarg);
            garden_print("Test Ril Interface:%d",sOptions.rilInterface);
            break;
        case 'D':
            tokenPtr = strtok_r(optarg,",",&argPtr);
            if(tokenPtr != NULL) {
                strlcpy(sOptions.xtraClientConfig.xtra_server_url[0],tokenPtr, MAX_URL_LEN);
                tokenPtr = strtok_r(NULL,",",&argPtr);
                if(tokenPtr != NULL) {
                    strlcpy(sOptions.xtraClientConfig.xtra_server_url[1],tokenPtr, MAX_URL_LEN);
                    tokenPtr = strtok_r(NULL,",",&argPtr);
                    if(tokenPtr != NULL) {
                        strlcpy(sOptions.xtraClientConfig.xtra_server_url[2],tokenPtr, MAX_URL_LEN);
                    }
                }
            }
            garden_print("Xtra data server URLs:%s , %s , %s",
            sOptions.xtraClientConfig.xtra_server_url[0],sOptions.xtraClientConfig.xtra_server_url[1],
            sOptions.xtraClientConfig.xtra_server_url[2]);
            break;
        case 'w':
            strlcpy(sOptions.xtraClientConfig.xtra_sntp_server_url[0],optarg,MAX_URL_LEN);
            garden_print("Xtra SNTP server URL: %s",sOptions.xtraClientConfig.xtra_sntp_server_url[0]);
            break;
        case 'e':
            strlcpy(sOptions.xtraClientConfig.user_agent_string,optarg,256);
            garden_print("Xtra User Agent String: %s",sOptions.xtraClientConfig.user_agent_string);
            break;
        case 'c':
            strlcpy(sOptions.gpsConfPath,optarg,256);
            // Initialize by reading the gps.conf file
            UTIL_READ_CONF(sOptions.gpsConfPath, cfg_parameter_table);
            garden_print("Parameters read from the config file :");
            garden_print("**************************************");
            garden_print("SUPL_VER      : 0x%X",sOptions.agpsData.suplVer);
            garden_print("SUPL_HOST     : %s",sOptions.agpsData.suplHost);
            garden_print("SUPL_PORT     : %ld",sOptions.agpsData.suplPort);
            garden_print("C2K_HOST      : %s",sOptions.agpsData.c2kHost);
            garden_print("C2K_PORT      : %ld",sOptions.agpsData.c2kPort);
            garden_print("NTP_SERVER    : %s",sOptions.xtraClientConfig.xtra_sntp_server_url[0]);
            garden_print("XTRA_SERVER_1 : %s",sOptions.xtraClientConfig.xtra_server_url[0]);
            garden_print("XTRA_SERVER_2 : %s",sOptions.xtraClientConfig.xtra_server_url[1]);
            garden_print("XTRA_SERVER_3 : %s",sOptions.xtraClientConfig.xtra_server_url[2]);
            garden_print("**************************************");
            break;
        case 'j':
            sOptions.disableAutomaticTimeInjection = 1;
            garden_print("Automatic time injections is disabled");
            break;
        case 'k':
            sOptions.niSuplFlag = 1;
            garden_print("NI SUPL flag:%d",sOptions.niSuplFlag);
            break;
        case 'n':
            sOptions.printNmea = 1;
            garden_print("Print NMEA info:%d",sOptions.printNmea);
            break;
        case 'y':
            sOptions.satelliteDetails = 1;
            garden_print("Print Details Satellites in View info:%d",sOptions.satelliteDetails);
            break;
        case 'o':
            sOptions.fixThresholdInSecs = atoi(optarg);
            garden_print("Time to first fix threshold value in seconds : %d", sOptions.fixThresholdInSecs);
            break;
        case 'z':
            sOptions.zppTestCaseNumber = atoi(optarg);
            garden_print("ZPP testCase: %d \n",sOptions.zppTestCaseNumber);
            break;
        case 'q':
            sOptions.enableXtra = atoi(optarg);
            garden_print("Xtra Enabled: %d",sOptions.enableXtra);
            break;
        case 'A':
            sOptions.stopOnMinSvs = atoi(optarg);
            garden_print("Stop on Minimum Svs: %d",sOptions.stopOnMinSvs);
            break;
        case 'B':
            sOptions.stopOnMinSnr = atof(optarg);
            garden_print("Stop on Minimum SNR: %f",sOptions.stopOnMinSnr);
            break;
        case 'T':
            sOptions.tracking = 1;
            garden_print("Start Tracking Session -- continuous untill processs is alive press Ctrl-C to exit");
            break;
        case 'Z':
            sOptions.zppTestCaseNumber =  atoi(optarg);
            garden_print("ZPP Test number: %d",sOptions.zppTestCaseNumber);
            break;
        case 'h':
        default:
            printHelp(argv);
            return 0;
        }
    }

    garden_print("Starting GARDEn");
    result = test_main();

    garden_print("Exiting GARDEn");
    return result;
}

#ifdef TEST_ULP
void test_ulp(int ulptestCase)
{
   garden_print ("Starting Ulp test cases\n");
   int rc = 0;

   switch (ulptestCase)
   {
    // Run test scenario 1
  // GPS tracking session when GPS is enabled
  // Remove GPS tracking session
    case 1:
     {

      UlpLocationCriteria criteria;

      // set up global phone context
      is_gps_enabled                = TRUE;
      is_network_position_available = TRUE;
      is_wifi_setting_enabled       = TRUE;
      is_battery_charging           = TRUE;
      criteria.valid_mask = 0;
      criteria.valid_mask |= ULP_CRITERIA_HAS_ACTION;
      criteria.action = ULP_ADD_CRITERIA;
      criteria.valid_mask |= ULP_CRITERIA_HAS_PROVIDER_SOURCE;
      criteria.provider_source = ULP_PROVIDER_SOURCE_GNSS;
      criteria.valid_mask |= ULP_CRITERIA_HAS_RECURRENCE_TYPE;
      criteria.recurrence_type = ULP_LOC_RECURRENCE_PERIODIC;
      criteria.valid_mask |= ULP_CRITERIA_HAS_MIN_INTERVAL;
      // assume in seconds
      criteria.min_interval = 5;
      pUlpEngineInterface->update_criteria(criteria);
      sleep (1);
      rc = pGpsInterface->start ();
      garden_print ("start GPS interface for ulp test returned %d", rc);
      sleep (5);
      criteria.action = ULP_REMOVE_CRITERIA;
      pUlpEngineInterface->update_criteria(criteria);
      rc = pGpsInterface->stop ();
      garden_print ("stop GPS interface for ulp test returned %d", rc);
      sleep (3);
     }
     break;
     // Network tracking session when network is enabled
     // Remove network tracking session
    case 2:
     {
      // set up global phone context
      is_gps_enabled                = TRUE;
      is_network_position_available = TRUE;
      is_wifi_setting_enabled       = TRUE;
      is_battery_charging           = TRUE;

      UlpLocationCriteria criteria;
      criteria.valid_mask = 0;
      criteria.valid_mask |= ULP_CRITERIA_HAS_ACTION;
      criteria.action = ULP_ADD_CRITERIA;
      criteria.valid_mask |= ULP_CRITERIA_HAS_PROVIDER_SOURCE;
      criteria.provider_source = ULP_PROVIDER_SOURCE_HYBRID;
      criteria.valid_mask |= ULP_CRITERIA_HAS_RECURRENCE_TYPE;
      criteria.recurrence_type = ULP_LOC_RECURRENCE_PERIODIC;
      criteria.valid_mask |= ULP_CRITERIA_HAS_MIN_INTERVAL;

      // assume in seconds
      criteria.min_interval = 5;
      pUlpEngineInterface->update_criteria(criteria);
      sleep (1);
      rc = pGpsInterface->start ();
      sleep (10);

      criteria.action = ULP_REMOVE_CRITERIA;
      pUlpEngineInterface->update_criteria(criteria);
      rc = pGpsInterface->stop ();
      sleep (10);
     }
     break;
     // Run test scenario 3
     // GPS tracking session when GPS is disabled
     // Remove GPS tracking session
     //
     // Result: No GPS session should be started
    case 3:
     {
      // set up global phone context
      is_gps_enabled                = FALSE;
      is_network_position_available = TRUE;
      is_wifi_setting_enabled       = TRUE;
      is_battery_charging           = TRUE;

      UlpLocationCriteria criteria;
      criteria.valid_mask = 0;
      criteria.valid_mask |= ULP_CRITERIA_HAS_ACTION;
      criteria.action = ULP_ADD_CRITERIA;
      criteria.valid_mask |= ULP_CRITERIA_HAS_PROVIDER_SOURCE;
      criteria.provider_source = ULP_PROVIDER_SOURCE_GNSS;
      criteria.valid_mask |= ULP_CRITERIA_HAS_RECURRENCE_TYPE;
      criteria.recurrence_type = ULP_LOC_RECURRENCE_PERIODIC;
      criteria.valid_mask |= ULP_CRITERIA_HAS_MIN_INTERVAL;
      // assume in seconds
      criteria.min_interval = 5;
      pUlpEngineInterface->update_criteria(criteria);
      sleep (1);
      rc = pGpsInterface->start ();
      sleep (5);
      criteria.action = ULP_REMOVE_CRITERIA;
      pUlpEngineInterface->update_criteria(criteria);
      rc = pGpsInterface->stop ();
      sleep (3);
     }
     break;
     // Run test scenario 4
     // GPS tracking session when GPS is enabled
     // While tracking session is on progress, GPS is disabled
     // Remove GPS tracking session
     //
     // Result: GPS tracking session is started, and then stopped
     // when GPS is disabled
    case 4:
     {
      // set up global phone context
      is_gps_enabled                = TRUE;
      is_network_position_available = TRUE;
      is_wifi_setting_enabled       = TRUE;
      is_battery_charging           = TRUE;

      UlpLocationCriteria criteria;
      criteria.valid_mask = 0;
      criteria.valid_mask |= ULP_CRITERIA_HAS_ACTION;
      criteria.action = ULP_ADD_CRITERIA;
      criteria.valid_mask |= ULP_CRITERIA_HAS_PROVIDER_SOURCE;
      criteria.provider_source = ULP_PROVIDER_SOURCE_GNSS;
      criteria.valid_mask |= ULP_CRITERIA_HAS_RECURRENCE_TYPE;
      criteria.recurrence_type = ULP_LOC_RECURRENCE_PERIODIC;
      criteria.valid_mask |= ULP_CRITERIA_HAS_MIN_INTERVAL;
      // assume in seconds
      criteria.min_interval = 5;
      pUlpEngineInterface->update_criteria(criteria);
      sleep (1);
      rc = pGpsInterface->start();
      sleep (4);

      // set up global phone context
      is_gps_enabled                = FALSE;
      is_network_position_available = TRUE;
      is_wifi_setting_enabled       = TRUE;
      is_battery_charging           = TRUE;
      //Simulate phone context update from AFW
      pthread_mutex_lock (&test_thread_mutex);
      test_thread_action = ACTION_PHONE_CONTEXT_UPDATE;
      pthread_cond_signal (&test_thread_cond);
      pthread_mutex_unlock (&test_thread_mutex);

      sleep (1);
      criteria.action = ULP_REMOVE_CRITERIA;
      pUlpEngineInterface->update_criteria(criteria);
      rc = pGpsInterface->stop ();
      sleep (3);
     }
     break;
     // Run test scenario 5
     // GPS tracking session when GPS is disabled
     // later on, user enables GPS setting
     // later on, GPS tracking session is removed
     //
     // Result: GPS tracking session is only started when gps setting is enabled,
     // and then stopped when the request is removed
    case 5:
     {
      // set up global phone context
      is_gps_enabled                = FALSE;
      is_network_position_available = FALSE;
      is_wifi_setting_enabled       = TRUE;
      is_battery_charging           = TRUE;

      UlpLocationCriteria criteria;
      criteria.valid_mask = 0;
      criteria.valid_mask |= ULP_CRITERIA_HAS_ACTION;
      criteria.action = ULP_ADD_CRITERIA;
      criteria.valid_mask |= ULP_CRITERIA_HAS_PROVIDER_SOURCE;
      criteria.provider_source = ULP_PROVIDER_SOURCE_GNSS;
      criteria.valid_mask |= ULP_CRITERIA_HAS_RECURRENCE_TYPE;
      criteria.recurrence_type = ULP_LOC_RECURRENCE_PERIODIC;
      criteria.valid_mask |= ULP_CRITERIA_HAS_MIN_INTERVAL;
      // assume in seconds
      criteria.min_interval = 5;
      pUlpEngineInterface->update_criteria(criteria);
      sleep (1);
      rc = pGpsInterface->start();
      sleep (4);

      garden_print("+++++++++++Ulp TC 5 +++++++++\n");
      // set up global phone context
      is_gps_enabled                = TRUE;
      is_network_position_available = FALSE;
      is_wifi_setting_enabled       = TRUE;
      is_battery_charging           = TRUE;
      pthread_mutex_lock (&test_thread_mutex);
      test_thread_action = ACTION_PHONE_CONTEXT_UPDATE;
      pthread_cond_signal (&test_thread_cond);
      pthread_mutex_unlock (&test_thread_mutex);
      sleep (1);
      criteria.action = ULP_REMOVE_CRITERIA;
      pUlpEngineInterface->update_criteria(criteria);
      rc = pGpsInterface->stop();
      sleep (3);
     }
     break;
     // Run test scenario 6
     // GPS single shot session when GPS is ensabled
     // later on, high accuracy tracking (5 second interval) session come in
     // later on, single shot fix request removed (potentially due to position has been satisfied)
     // later on, GPS session (1hz) come in
     // later on, 1hz tracking criteria removed
     // later on, 5hz tracking criteria removed
     // later on, stop request received
     //
     // ulp_brain_choose_providers:|position mode|GPS start| GPS engine
    case 6:
     {
      // set up global phone context
      is_gps_enabled                = TRUE;
      is_network_position_available = TRUE;
      is_wifi_setting_enabled       = TRUE;
      is_battery_charging           = TRUE;
      is_agps_enabled               = TRUE;

      // Single shot fix request comes in
      UlpLocationCriteria criteria;
      criteria.valid_mask = 0;
      criteria.valid_mask |= ULP_CRITERIA_HAS_ACTION;
      criteria.action = ULP_ADD_CRITERIA;
      criteria.valid_mask |= ULP_CRITERIA_HAS_PROVIDER_SOURCE;
      criteria.provider_source = ULP_PROVIDER_SOURCE_GNSS;
      criteria.valid_mask |= ULP_CRITERIA_HAS_RECURRENCE_TYPE;
      criteria.recurrence_type = ULP_LOC_RECURRENCE_SINGLE;
      criteria.valid_mask |= ULP_CRITERIA_HAS_MIN_INTERVAL;
      pUlpEngineInterface->update_criteria(criteria);
      sleep (1);

      // ULP start received
      rc = pGpsInterface->start();
      sleep (4);

      // Periodic fix request comes in (5 seconds interval)
      criteria.valid_mask = 0;
      criteria.valid_mask |= ULP_CRITERIA_HAS_ACTION;
      criteria.action = ULP_ADD_CRITERIA;
      criteria.valid_mask |= ULP_CRITERIA_HAS_PREFERRED_HORIZONTAL_ACCURACY;
      criteria.preferred_horizontal_accuracy = ULP_HORZ_ACCURACY_HIGH;
      criteria.valid_mask |= ULP_CRITERIA_HAS_RECURRENCE_TYPE;
      criteria.recurrence_type = ULP_LOC_RECURRENCE_PERIODIC;
      criteria.valid_mask |= ULP_CRITERIA_HAS_MIN_INTERVAL;
      criteria.min_interval = 5;
      pUlpEngineInterface->update_criteria(criteria);
      sleep (2);

      // Single shot request gets removed (due to receive position report)
      criteria.valid_mask = 0;
      criteria.valid_mask |= ULP_CRITERIA_HAS_ACTION;
      criteria.action = ULP_REMOVE_CRITERIA;
      criteria.valid_mask |= ULP_CRITERIA_HAS_PROVIDER_SOURCE;
      criteria.provider_source = ULP_PROVIDER_SOURCE_GNSS;
      criteria.valid_mask |= ULP_CRITERIA_HAS_RECURRENCE_TYPE;
      criteria.recurrence_type = ULP_LOC_RECURRENCE_SINGLE;
      criteria.valid_mask |= ULP_CRITERIA_HAS_MIN_INTERVAL;
      pUlpEngineInterface->update_criteria(criteria);


      // Periodic fix request comes in (1 second interval)
      criteria.valid_mask = 0;
      criteria.valid_mask |= ULP_CRITERIA_HAS_ACTION;
      criteria.action = ULP_ADD_CRITERIA;
      criteria.valid_mask |= ULP_CRITERIA_HAS_PREFERRED_HORIZONTAL_ACCURACY;
      criteria.preferred_horizontal_accuracy = ULP_HORZ_ACCURACY_HIGH;
      criteria.valid_mask |= ULP_CRITERIA_HAS_RECURRENCE_TYPE;
      criteria.recurrence_type = ULP_LOC_RECURRENCE_PERIODIC;
      criteria.valid_mask |= ULP_CRITERIA_HAS_MIN_INTERVAL;
      criteria.min_interval = 1;
      pUlpEngineInterface->update_criteria(criteria);

      sleep (5);
      // Periodic fix request removed (1 second interval)
      criteria.valid_mask = 0;
      criteria.valid_mask |= ULP_CRITERIA_HAS_ACTION;
      criteria.action = ULP_REMOVE_CRITERIA;
      criteria.valid_mask |= ULP_CRITERIA_HAS_PREFERRED_HORIZONTAL_ACCURACY;
      criteria.preferred_horizontal_accuracy = ULP_HORZ_ACCURACY_HIGH;
      criteria.valid_mask |= ULP_CRITERIA_HAS_RECURRENCE_TYPE;
      criteria.recurrence_type = ULP_LOC_RECURRENCE_PERIODIC;
      criteria.valid_mask |= ULP_CRITERIA_HAS_MIN_INTERVAL;
      criteria.min_interval = 1;
      pUlpEngineInterface->update_criteria(criteria);

      sleep (3);
      // Periodic fix request removed (5 second interval)
      criteria.valid_mask = 0;
      criteria.valid_mask |= ULP_CRITERIA_HAS_ACTION;
      criteria.action = ULP_REMOVE_CRITERIA;
      criteria.valid_mask |= ULP_CRITERIA_HAS_PREFERRED_HORIZONTAL_ACCURACY;
      criteria.preferred_horizontal_accuracy = ULP_HORZ_ACCURACY_HIGH;
      criteria.valid_mask |= ULP_CRITERIA_HAS_RECURRENCE_TYPE;
      criteria.recurrence_type = ULP_LOC_RECURRENCE_PERIODIC;
      criteria.valid_mask |= ULP_CRITERIA_HAS_MIN_INTERVAL;
      criteria.min_interval = 5;
      pUlpEngineInterface->update_criteria(criteria);

      // Stop ulp
      rc = pGpsInterface->stop();
      sleep (3);
     }
     break;
     // Run test scenario 7
     // Network provider is enabled
     // First comes network single shot
     // Secondly comes network tracking session (30 seconds)
     // Secondly comes network tracking session (10 seconds)
     // Single shot removed
     // 30 seconds network tracking session removed
     // 10 seconds network tracking session removed
     // ulp stop received
     //
     // Search "ulp_brain_choose_providers:|request type: " for output
    case 7:
     {

      is_gps_enabled                = TRUE;
      is_network_position_available = TRUE;
      is_wifi_setting_enabled       = TRUE;
      is_battery_charging           = TRUE;

      UlpLocationCriteria criteria;
      criteria.valid_mask = 0;
      criteria.valid_mask |= ULP_CRITERIA_HAS_ACTION;
      criteria.action = ULP_ADD_CRITERIA;
      criteria.valid_mask |= ULP_CRITERIA_HAS_PROVIDER_SOURCE;
      criteria.provider_source = ULP_PROVIDER_SOURCE_HYBRID;
      criteria.valid_mask |= ULP_CRITERIA_HAS_RECURRENCE_TYPE;
      criteria.recurrence_type = ULP_LOC_RECURRENCE_SINGLE;
      pUlpEngineInterface->update_criteria(criteria);
      sleep (1);
      rc = pGpsInterface->start();
      sleep (4);

      criteria.valid_mask = 0;
      criteria.valid_mask |= ULP_CRITERIA_HAS_ACTION;
      criteria.action = ULP_ADD_CRITERIA;
      criteria.valid_mask |= ULP_CRITERIA_HAS_PREFERRED_HORIZONTAL_ACCURACY;
      criteria.preferred_horizontal_accuracy = ULP_HORZ_ACCURACY_LOW;
      criteria.valid_mask |= ULP_CRITERIA_HAS_RECURRENCE_TYPE;
      criteria.recurrence_type = ULP_LOC_RECURRENCE_PERIODIC;
      criteria.valid_mask |= ULP_CRITERIA_HAS_MIN_INTERVAL;
      // assume in seconds
      criteria.min_interval = 30;
      pUlpEngineInterface->update_criteria(criteria);
      sleep (4);
      criteria.valid_mask = 0;
      criteria.valid_mask |= ULP_CRITERIA_HAS_ACTION;
      criteria.action = ULP_REMOVE_CRITERIA;
      criteria.valid_mask |= ULP_CRITERIA_HAS_PROVIDER_SOURCE;
      criteria.provider_source = ULP_PROVIDER_SOURCE_HYBRID;
      criteria.valid_mask |= ULP_CRITERIA_HAS_RECURRENCE_TYPE;
      criteria.recurrence_type = ULP_LOC_RECURRENCE_SINGLE;
      pUlpEngineInterface->update_criteria(criteria);
      sleep (4);
      criteria.valid_mask = 0;
      criteria.valid_mask |= ULP_CRITERIA_HAS_ACTION;
      criteria.action = ULP_REMOVE_CRITERIA;
      criteria.valid_mask |= ULP_CRITERIA_HAS_PREFERRED_HORIZONTAL_ACCURACY;
      criteria.preferred_horizontal_accuracy = ULP_HORZ_ACCURACY_LOW;
      criteria.valid_mask |= ULP_CRITERIA_HAS_RECURRENCE_TYPE;
      criteria.recurrence_type = ULP_LOC_RECURRENCE_PERIODIC;
      criteria.valid_mask |= ULP_CRITERIA_HAS_MIN_INTERVAL;
      // assume in seconds
      criteria.min_interval = 30;
      pUlpEngineInterface->update_criteria(criteria);
      sleep (4);
      // Stop ulp
      rc = pGpsInterface->stop();
      sleep (4);
     }
     break;
     // Run test scenario 8
     // GPS and Network provider is enabled
     // First comes network single shot
     // Secondly comes network tracking session (30 seconds)
     // Then comes GPS tracking session (10 seconds)
     // Single shot network is removed
     // GPS tracking session (10 seconds) is removed
     // Network tracking session (30 seconds) is removed
     // ulp stop received
     // Search log for "ulp_brain_choose_providers:|position mode|GPS engine|request type: "
    case 8:
     {
      // set up global phone context
      is_gps_enabled                = TRUE;
      is_network_position_available = TRUE;
      is_wifi_setting_enabled       = TRUE;
      is_battery_charging           = TRUE;

      UlpLocationCriteria criteria;
      criteria.valid_mask = 0;
      criteria.valid_mask |= ULP_CRITERIA_HAS_ACTION;
      criteria.action = ULP_ADD_CRITERIA;
      criteria.valid_mask |= ULP_CRITERIA_HAS_PROVIDER_SOURCE;
      criteria.provider_source = ULP_PROVIDER_SOURCE_HYBRID;
      criteria.valid_mask |= ULP_CRITERIA_HAS_RECURRENCE_TYPE;
      criteria.recurrence_type = ULP_LOC_RECURRENCE_SINGLE;
      pUlpEngineInterface->update_criteria(criteria);
      sleep (1);
      rc = pGpsInterface->start();
      sleep (4);
      criteria.valid_mask = 0;
      criteria.valid_mask |= ULP_CRITERIA_HAS_ACTION;
      criteria.action = ULP_ADD_CRITERIA;
      criteria.valid_mask |= ULP_CRITERIA_HAS_PREFERRED_HORIZONTAL_ACCURACY;
      criteria.preferred_horizontal_accuracy = ULP_HORZ_ACCURACY_LOW;
      criteria.valid_mask |= ULP_CRITERIA_HAS_RECURRENCE_TYPE;
      criteria.recurrence_type = ULP_LOC_RECURRENCE_PERIODIC;
      criteria.valid_mask |= ULP_CRITERIA_HAS_MIN_INTERVAL;
      // assume in seconds
      criteria.min_interval = 30;
      pUlpEngineInterface->update_criteria(criteria);
      sleep (4);
      // Periodic GPS fix request comes in (10 seconds interval)
      criteria.valid_mask = 0;
      criteria.valid_mask |= ULP_CRITERIA_HAS_ACTION;
      criteria.action = ULP_ADD_CRITERIA;
      criteria.valid_mask |= ULP_CRITERIA_HAS_PREFERRED_HORIZONTAL_ACCURACY;
      criteria.preferred_horizontal_accuracy = ULP_HORZ_ACCURACY_HIGH;
      criteria.valid_mask |= ULP_CRITERIA_HAS_RECURRENCE_TYPE;
      criteria.recurrence_type = ULP_LOC_RECURRENCE_PERIODIC;
      criteria.valid_mask |= ULP_CRITERIA_HAS_MIN_INTERVAL;
      criteria.min_interval = 10;
      pUlpEngineInterface->update_criteria(criteria);
      sleep (2);

      // Single shot network session removed (request satisified)
      criteria.valid_mask = 0;
      criteria.valid_mask |= ULP_CRITERIA_HAS_ACTION;
      criteria.action = ULP_REMOVE_CRITERIA;
      criteria.valid_mask |= ULP_CRITERIA_HAS_PROVIDER_SOURCE;
      criteria.provider_source = ULP_PROVIDER_SOURCE_HYBRID;
      criteria.valid_mask |= ULP_CRITERIA_HAS_RECURRENCE_TYPE;
      criteria.recurrence_type = ULP_LOC_RECURRENCE_SINGLE;
      pUlpEngineInterface->update_criteria(criteria);

      sleep (4);

      // Periodic GPS fix request goes away (10 seconds interval)
      criteria.valid_mask = 0;
      criteria.valid_mask |= ULP_CRITERIA_HAS_ACTION;
      criteria.action = ULP_REMOVE_CRITERIA;
      criteria.valid_mask |= ULP_CRITERIA_HAS_PREFERRED_HORIZONTAL_ACCURACY;
      criteria.preferred_horizontal_accuracy = ULP_HORZ_ACCURACY_HIGH;
      criteria.valid_mask |= ULP_CRITERIA_HAS_RECURRENCE_TYPE;
      criteria.recurrence_type = ULP_LOC_RECURRENCE_PERIODIC;
      criteria.valid_mask |= ULP_CRITERIA_HAS_MIN_INTERVAL;
      criteria.min_interval = 10;
      pUlpEngineInterface->update_criteria(criteria);
      sleep (2);

      // Periodic network tracking session goes away (30 seconds interval)
      criteria.valid_mask = 0;
      criteria.valid_mask |= ULP_CRITERIA_HAS_ACTION;
      criteria.action = ULP_REMOVE_CRITERIA;
      criteria.valid_mask |= ULP_CRITERIA_HAS_PREFERRED_HORIZONTAL_ACCURACY;
      criteria.preferred_horizontal_accuracy = ULP_HORZ_ACCURACY_LOW;
      criteria.valid_mask |= ULP_CRITERIA_HAS_RECURRENCE_TYPE;
      criteria.recurrence_type = ULP_LOC_RECURRENCE_PERIODIC;
      criteria.valid_mask |= ULP_CRITERIA_HAS_MIN_INTERVAL;
      // assume in seconds
      criteria.min_interval = 30;
      pUlpEngineInterface->update_criteria(criteria);

      sleep (4);

      // Stop ulp
      rc = pGpsInterface->stop();
      sleep (4);
     }
     break;
     // Run test scenario 9
     // GPS and Network provider is enabled
     // First comes network single shot
     // Secondly comes network tracking session (30 seconds)
     // Then comes GPS tracking session (10 seconds)
     // Single shot network is removed
     // GPS disabled
     // GPS tracking session (10 seconds) is removed
     // Network tracking session (30 seconds) is removed
     // ulp stop received
     //
     // Search log for "ulp_brain_choose_providers:|position mode|GPS engine|request type: "
    case 9:
     {
      // set up global phone context
      is_gps_enabled                = TRUE;
      is_network_position_available = TRUE;
      is_wifi_setting_enabled       = TRUE;
      is_battery_charging           = TRUE;

      UlpLocationCriteria criteria;
      criteria.valid_mask = 0;
      criteria.valid_mask |= ULP_CRITERIA_HAS_ACTION;
      criteria.action = ULP_ADD_CRITERIA;
      criteria.valid_mask |= ULP_CRITERIA_HAS_PROVIDER_SOURCE;
      criteria.provider_source = ULP_PROVIDER_SOURCE_HYBRID;
      criteria.valid_mask |= ULP_CRITERIA_HAS_RECURRENCE_TYPE;
      criteria.recurrence_type = ULP_LOC_RECURRENCE_SINGLE;
      pUlpEngineInterface->update_criteria(criteria);
      sleep (1);
      rc = pGpsInterface->start();
      sleep (4);
      criteria.valid_mask = 0;
      criteria.valid_mask |= ULP_CRITERIA_HAS_ACTION;
      criteria.action = ULP_ADD_CRITERIA;
      criteria.valid_mask |= ULP_CRITERIA_HAS_PREFERRED_HORIZONTAL_ACCURACY;
      criteria.preferred_horizontal_accuracy = ULP_HORZ_ACCURACY_LOW;
      criteria.valid_mask |= ULP_CRITERIA_HAS_RECURRENCE_TYPE;
      criteria.recurrence_type = ULP_LOC_RECURRENCE_PERIODIC;
      criteria.valid_mask |= ULP_CRITERIA_HAS_MIN_INTERVAL;
      // assume in seconds
      criteria.min_interval = 30;
      pUlpEngineInterface->update_criteria(criteria);
      sleep (4);
      // Periodic GPS fix request comes in (10 seconds interval)
      criteria.valid_mask = 0;
      criteria.valid_mask |= ULP_CRITERIA_HAS_ACTION;
      criteria.action = ULP_ADD_CRITERIA;
      criteria.valid_mask |= ULP_CRITERIA_HAS_PREFERRED_HORIZONTAL_ACCURACY;
      criteria.preferred_horizontal_accuracy = ULP_HORZ_ACCURACY_HIGH;
      criteria.valid_mask |= ULP_CRITERIA_HAS_RECURRENCE_TYPE;
      criteria.recurrence_type = ULP_LOC_RECURRENCE_PERIODIC;
      criteria.valid_mask |= ULP_CRITERIA_HAS_MIN_INTERVAL;
      criteria.min_interval = 10;
      pUlpEngineInterface->update_criteria(criteria);
      sleep (2);

      // Single shot network session removed (request satisified)
      criteria.valid_mask = 0;
      criteria.valid_mask |= ULP_CRITERIA_HAS_ACTION;
      criteria.action = ULP_REMOVE_CRITERIA;
      criteria.valid_mask |= ULP_CRITERIA_HAS_PROVIDER_SOURCE;
      criteria.provider_source = ULP_PROVIDER_SOURCE_HYBRID;
      criteria.valid_mask |= ULP_CRITERIA_HAS_RECURRENCE_TYPE;
      criteria.recurrence_type = ULP_LOC_RECURRENCE_SINGLE;
      pUlpEngineInterface->update_criteria(criteria);

      sleep (4);
      // set up global phone context
      is_gps_enabled                = FALSE;
      is_network_position_available = TRUE;
      is_wifi_setting_enabled       = TRUE;
      is_battery_charging           = TRUE;
      //Simulate phone context update from AFW
      pthread_mutex_lock (&test_thread_mutex);
      test_thread_action = ACTION_PHONE_CONTEXT_UPDATE;
      pthread_cond_signal (&test_thread_cond);
      pthread_mutex_unlock (&test_thread_mutex);
      sleep (2);

      // Periodic GPS fix request goes away (10 seconds interval)
      criteria.valid_mask = 0;
      criteria.valid_mask |= ULP_CRITERIA_HAS_ACTION;
      criteria.action = ULP_REMOVE_CRITERIA;
      criteria.valid_mask |= ULP_CRITERIA_HAS_PREFERRED_HORIZONTAL_ACCURACY;
      criteria.preferred_horizontal_accuracy = ULP_HORZ_ACCURACY_HIGH;
      criteria.valid_mask |= ULP_CRITERIA_HAS_RECURRENCE_TYPE;
      criteria.recurrence_type = ULP_LOC_RECURRENCE_PERIODIC;
      criteria.valid_mask |= ULP_CRITERIA_HAS_MIN_INTERVAL;
      criteria.min_interval = 10;
      pUlpEngineInterface->update_criteria(criteria);
      sleep (2);

      // Periodic network tracking session goes away (30 seconds interval)
      criteria.valid_mask = 0;
      criteria.valid_mask |= ULP_CRITERIA_HAS_ACTION;
      criteria.action = ULP_REMOVE_CRITERIA;
      criteria.valid_mask |= ULP_CRITERIA_HAS_PREFERRED_HORIZONTAL_ACCURACY;
      criteria.preferred_horizontal_accuracy = ULP_HORZ_ACCURACY_LOW;
      criteria.valid_mask |= ULP_CRITERIA_HAS_RECURRENCE_TYPE;
      criteria.recurrence_type = ULP_LOC_RECURRENCE_PERIODIC;
      criteria.valid_mask |= ULP_CRITERIA_HAS_MIN_INTERVAL;
      // assume in seconds
      criteria.min_interval = 30;
      pUlpEngineInterface->update_criteria(criteria);

      sleep (4);

      // Stop ulp
      rc = pGpsInterface->stop();
      sleep (4);
     }
     break;
    // Run test scenario 10
    // GPS single shot when GPS is enabled, GPS_CAPABLITY support MSA
    // Remove single shot
    // Output: single shot will be set to tracking with long interval, ,MSA
    case 10:
    {
       // set up global phone context
       is_gps_enabled                = TRUE;
       is_network_position_available = TRUE;
       is_wifi_setting_enabled       = TRUE;
       is_battery_charging           = TRUE;

       UlpLocationCriteria criteria;
       criteria.valid_mask = 0;
       criteria.valid_mask |= ULP_CRITERIA_HAS_ACTION;
       criteria.action = ULP_ADD_CRITERIA;
       criteria.valid_mask |= ULP_CRITERIA_HAS_PROVIDER_SOURCE;
       criteria.provider_source = ULP_PROVIDER_SOURCE_GNSS;
       criteria.valid_mask |= ULP_CRITERIA_HAS_RECURRENCE_TYPE;
       criteria.recurrence_type = ULP_LOC_RECURRENCE_SINGLE;
       pUlpEngineInterface->update_criteria(criteria);
       sleep (1);
       rc = pGpsInterface->start();
       sleep (5);
       criteria.action = ULP_REMOVE_CRITERIA;
       pUlpEngineInterface->update_criteria(criteria);
       rc = pGpsInterface->stop();
       sleep (3);
    }
    break;
    // Run test scenario 11
    // high accuracy tracking (30 second interval) session come in
    // 10 seconds later, another high accuracy tracking (30 second interval) session come in
    // later on, one 30 second tracking criteria removed
    // later on, the other 30 second tracking criteria removed
    // later on, stop request received
    //
    // ulp_brain_choose_providers:|position mode|GPS start| GPS engine
   case 11:
       {
         // set up global phone context
         is_gps_enabled                = TRUE;
         is_network_position_available = TRUE;
         is_wifi_setting_enabled       = TRUE;
         is_battery_charging           = TRUE;

         UlpLocationCriteria criteria;

         // Periodic GPS fix request comes in (30 seconds interval)
         criteria.valid_mask = 0;
         criteria.valid_mask |= ULP_CRITERIA_HAS_ACTION;
         criteria.action = ULP_ADD_CRITERIA;
         criteria.valid_mask |= ULP_CRITERIA_HAS_PREFERRED_HORIZONTAL_ACCURACY;
         criteria.preferred_horizontal_accuracy = ULP_HORZ_ACCURACY_HIGH;
         criteria.valid_mask |= ULP_CRITERIA_HAS_RECURRENCE_TYPE;
         criteria.recurrence_type = ULP_LOC_RECURRENCE_PERIODIC;
         criteria.valid_mask |= ULP_CRITERIA_HAS_MIN_INTERVAL;
         criteria.min_interval = 30;
         pUlpEngineInterface->update_criteria(criteria);
         rc = pGpsInterface->start();
         sleep (10);

         pUlpEngineInterface->update_criteria(criteria);
         sleep (10);

         // Remove first tracking session
         criteria.action = ULP_REMOVE_CRITERIA;
         pUlpEngineInterface->update_criteria(criteria);
         sleep (2);
         // Remove second tracking session
         pUlpEngineInterface->update_criteria(criteria);
         sleep (2);
         rc = pGpsInterface->stop();
         sleep (3);
       }
       break;
   case 12:
       {
       // Run test scenario 12
       // First, hybrid provider tracking (30 second interval) session come in
       // 10 seconds later, another low accuracy (30 second interval) session come in
       // later on, the 30 second hybrid provider tracking criteria removed
       // later on, the other 30 second low accuracy tracking criteria removed
       // later on, stop request received
       //
       // Search log for "ulp_brain_choose_providers:|position mode|GPS engine|request type: "
       // set up global phone context
       is_gps_enabled                = TRUE;
       is_network_position_available = TRUE;
       is_wifi_setting_enabled       = TRUE;
       is_battery_charging           = TRUE;

       UlpLocationCriteria criteria1;

       // Periodic GPS fix request comes in (30 seconds interval)
       criteria1.valid_mask = 0;
       criteria1.valid_mask |= ULP_CRITERIA_HAS_ACTION;
       criteria1.action = ULP_ADD_CRITERIA;
       criteria1.valid_mask |= ULP_CRITERIA_HAS_PROVIDER_SOURCE;
       criteria1.provider_source = ULP_PROVIDER_SOURCE_HYBRID;
       criteria1.valid_mask |= ULP_CRITERIA_HAS_RECURRENCE_TYPE;
       criteria1.recurrence_type = ULP_LOC_RECURRENCE_PERIODIC;
       criteria1.valid_mask |= ULP_CRITERIA_HAS_MIN_INTERVAL;
       criteria1.min_interval = 30;
       pUlpEngineInterface->update_criteria(criteria1);
       rc = pGpsInterface->start();
       sleep (10);
       UlpLocationCriteria criteria2;

       // Periodic GPS fix request comes in (30 seconds interval)
       criteria2.valid_mask = 0;
       criteria2.valid_mask |= ULP_CRITERIA_HAS_ACTION;
       criteria2.action = ULP_ADD_CRITERIA;
       criteria2.valid_mask |= ULP_CRITERIA_HAS_PREFERRED_HORIZONTAL_ACCURACY;
       criteria2.preferred_horizontal_accuracy = ULP_HORZ_ACCURACY_LOW;
       criteria2.valid_mask |= ULP_CRITERIA_HAS_RECURRENCE_TYPE;
       criteria2.recurrence_type = ULP_LOC_RECURRENCE_PERIODIC;
       criteria2.valid_mask |= ULP_CRITERIA_HAS_MIN_INTERVAL;
       criteria2.min_interval = 30;
       pUlpEngineInterface->update_criteria(criteria2);
       sleep (10);

       // Remove first tracking session
       criteria1.action = ULP_REMOVE_CRITERIA;
       pUlpEngineInterface->update_criteria(criteria1);
       sleep (2);

       // Remove second tracking session
       criteria2.action = ULP_REMOVE_CRITERIA;
       pUlpEngineInterface->update_criteria(criteria2);
       sleep (2);
       pGpsInterface->stop();
       sleep (3);
       }
       break;
   default:
       garden_print(" Unknown ulp test case\n");
   }
}
#endif /* TEST_ULP */

void test_zpp(int zpptestCase)
{
   garden_print ("Starting ZPP test cases\n");
   int rc = 0;

   if (NULL == pUlpEngineInterface) {
      garden_print ("In test_zpp. Error.pUlpEngineInterface null\n");
      return;
   }

   switch (zpptestCase)
   {
    // Run test scenario 1
  // System event screen on
   case 1:
     {
       pthread_mutex_lock (&ulp_location_mutex);
       pUlpEngineInterface->system_update(ULP_LOC_SCREEN_ON);
       pthread_cond_wait (&ulp_location_cond, &ulp_location_mutex);
       pthread_mutex_unlock (&ulp_location_mutex);
       garden_print ("ZPP TestCase: System event screen on passed\n");

     }
    break;
    // System event time zone change
    case 2:
     {
         pthread_mutex_lock (&ulp_location_mutex);
         pUlpEngineInterface->system_update(ULP_LOC_TIMEZONE_CHANGE);
         pthread_cond_wait (&ulp_location_cond, &ulp_location_mutex);
         pthread_mutex_unlock (&ulp_location_mutex);
         garden_print ("ZPP TestCase:System event time zone change passed\n");
     }
    break;
   // Power connected 30s then power disconnected
   case 3:
    {
        pthread_mutex_lock (&ulp_location_mutex);
        pUlpEngineInterface->system_update(ULP_LOC_POWER_CONNECTED);
        pthread_cond_wait (&ulp_location_cond, &ulp_location_mutex);
        garden_print ("ZPP TestCase:System event Power connected. got 1st loc report");
        pthread_cond_wait (&ulp_location_cond, &ulp_location_mutex);
        pthread_mutex_unlock (&ulp_location_mutex);
        pUlpEngineInterface->system_update(ULP_LOC_POWER_DISCONNECTED);
        garden_print ("ZPP TestCase:System event Power connected passed\n");
    }
   break;

   // verify ZPP throtting thru threshold in gps.conf
   case 4:
    {
        struct timeval present_time;
        struct timespec expire_time;
        pthread_mutex_lock (&ulp_location_mutex);
        pUlpEngineInterface->system_update(ULP_LOC_SCREEN_ON);
        sleep(1);
        pUlpEngineInterface->system_update(ULP_LOC_TIMEZONE_CHANGE);
        /* Calculate absolute expire time */
        gettimeofday(&present_time, NULL);
        expire_time.tv_sec  = present_time.tv_sec + 10;
        expire_time.tv_nsec = present_time.tv_usec * 1000;
        pthread_cond_timedwait (&ulp_location_cond, &ulp_location_mutex, &expire_time);
        pthread_mutex_unlock (&ulp_location_mutex);
        pUlpEngineInterface->system_update(ULP_LOC_POWER_DISCONNECTED);
        garden_print ("ZPP TestCase:verify ZPP throtting passed\n");
    }
   break;

    default:
       garden_print(" Unknown zpp test case\n");
   }
}

void run_tracking_session(void)
{
   int  pipefd[2] = {0,0};
   char buf;
   int rc;

   if (pipe(pipefd) == -1) {
      LOC_LOGE("pipe error");
      return;
   }

   // set up global phone context
   is_gps_enabled                = TRUE;
   is_network_position_available = TRUE;
   is_wifi_setting_enabled       = TRUE;
   is_battery_charging           = TRUE;

   pGpsInterface->set_position_mode (GPS_POSITION_MODE_MS_BASED,
                                     GPS_POSITION_RECURRENCE_PERIODIC,
                                     1,
                                     ULP_HORZ_ACCURACY_HIGH,
                                     1000);

   rc = pGpsInterface->start ();
   garden_print ("Tracking Session Start -- return code = %d", rc);

   //run the tracking session until the process stops
   while(g_run_active_client)
   {
      read(pipefd[0], &buf, 1);
   }

}
