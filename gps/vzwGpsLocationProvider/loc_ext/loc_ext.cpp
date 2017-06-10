/******************************************************************************
  @file:  loc_ext.cpp
  @brief:

  DESCRIPTION
    This file defines the implemenation for GPS hardware abstraction layer.

  INITIALIZATION AND SEQUENCING REQUIREMENTS

  -----------------------------------------------------------------------------
  Copyright (c) 2009,2013,2014 Qualcomm Technologies, Inc.
  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
  -----------------------------------------------------------------------------
/*=====================================================================
                        EDIT HISTORY FOR MODULE

  This section contains comments describing changes made to the module.
  Notice that changes are listed in reverse chronological order.

when       who      what, where, why
--------   ---      ---------------------------------------------------
03/24/10   dx       Add VzW requirement implementation
10/09/09   dx       Add function entrance check to avoid panics when GPS is turned off
10/09/09   dx       Process configuration file gps.conf
10/01/09   dx       Intermediate position support, GPS status, XTRA Inj fix
04/01/09   wc       Initial version

======================================================================*/

#define LOG_NDDEBUG 0
#define LOG_TAG "LocSvc_vzw"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <math.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <netinet/in.h>         /* struct sockaddr_in */
#include <sys/socket.h>
#include <netdb.h>
#include <time.h>

#include <hardware/gps.h>
#include "loc_vzw.h"
#ifndef USE_GLIB
#include <android_runtime/AndroidRuntime.h>
#include <utils/SystemClock.h>
#endif /* USE_GLIB */
#include <cutils/properties.h>
#include <string.h>
#include "platform_lib_includes.h"
#include "loc.h"
#include <loc_eng.h>

#ifdef USE_RPC
#include <loc_api_rpcgen_common_rpc.h>
#include <loc_api_fixup.h>
#elif USE_QMI
#include <location_service_v02.h>
#endif

static void* parsePositionExt(void* data);
static void* parseSvExt(void* data);
static void local_sv_status_callback(GpsSvStatus* sv_status, void* sv_dop);
static void local_location_callback(UlpLocation* location, void* ext);

// Global data structure for location engine
loc_eng_data_s_type loc_ext_data;
static void do_nothing() {}
// Cannot use loc_ext_data since it is intialized to 0 in loc_eng
ContextBase* context = NULL;
/*===========================================================================
FUNCTION    createPthread

DESCRIPTION
   Creates a pthread and returns the thread id

DEPENDENCIES
   None

RETURN VALUE
   thread id

SIDE EFFECTS
   N/A

===========================================================================*/

static pthread_t createPthread(const char* name, void (*start)(void *), void* arg)
{
    pthread_t threadId = 0;
    pthread_create(&threadId, NULL, (void *(*)(void*))start, arg);
    return threadId;
}




/*===========================================================================
FUNCTION    loc_ext_init

DESCRIPTION
   Initialize the location engine, this include setting up global datas
   and registers location engien with loc api service.

DEPENDENCIES
   None

RETURN VALUE
   0: success

SIDE EFFECTS
   N/A

===========================================================================*/
int loc_ext_init()
{
    ENTRY_LOG();
    LOC_API_ADAPTER_EVENT_MASK_T event =
        LOC_API_ADAPTER_BIT_PARSED_POSITION_REPORT |
        LOC_API_ADAPTER_BIT_SATELLITE_REPORT |
        LOC_API_ADAPTER_BIT_IOCTL_REPORT |
        LOC_API_ADAPTER_BIT_STATUS_REPORT |
        LOC_API_ADAPTER_BIT_NMEA_1HZ_REPORT;
    LocCallbacks clientCallbacks = {local_location_callback, /* location_cb */
                                    status_callback, /* status_cb */
                                    local_sv_status_callback, /* sv_status_cb */
                                    nmea_callback, /* nmea_cb */
                                    NULL, /* set_capabilities_cb */
                                    do_nothing, /* acquire_wakelock_cb */
                                    do_nothing, /* release_wakelock_cb */
                                    (pthread_t (*)(const char*, void (*)(void*), void*))LOC_EXT_CREATE_THREAD_CB_PLATFORM_LIB_ABSTRACTION, /* create_thread_cb */
                                    parsePositionExt, /* location_ext_parser */
                                    parseSvExt,  /* sv_ext_parser */
                                    NULL /* request_utc_time_cb */};

    if (NULL == context) {
        context = new ContextBase(
                        new MsgTask((MsgTask::tCreate)clientCallbacks.create_thread_cb,
                                    "loc_ext"),
                        LocDualContext::mFgExclMask,
                        LocDualContext::mLBSLibName);
    }

    int ret_val = loc_eng_init(loc_ext_data, &clientCallbacks, event, context);

    // override the setting.  VZW does not want intermediate fixes.
    loc_ext_data.intermediateFix = 0;

    EXIT_LOG(%d, ret_val);
    return ret_val;
}

/*===========================================================================
FUNCTION    loc_ext_cleanup

DESCRIPTION
   Cleans location engine. The location client handle will be released.

DEPENDENCIES
   None

RETURN VALUE
   None

SIDE EFFECTS
   N/A

===========================================================================*/
void loc_ext_cleanup()
{
    ENTRY_LOG();
    loc_eng_cleanup(loc_ext_data);
    EXIT_LOG(%s, VOID_RET);
}


/*===========================================================================
FUNCTION    loc_ext_start

DESCRIPTION
   Starts the tracking session

DEPENDENCIES
   None

RETURN VALUE
   0: success

SIDE EFFECTS
   N/A

===========================================================================*/
int loc_ext_start()
{
    ENTRY_LOG();
    int ret_val = loc_eng_start(loc_ext_data);

    EXIT_LOG(%d, ret_val);
    return ret_val;
}

/*===========================================================================
FUNCTION    loc_eng_stop

DESCRIPTION
   Stops the tracking session

DEPENDENCIES
   None

RETURN VALUE
   0: success

SIDE EFFECTS
   N/A

===========================================================================*/
int loc_ext_stop()
{
    ENTRY_LOG();
    int ret_val = loc_eng_stop(loc_ext_data);

    EXIT_LOG(%d, ret_val);
    return ret_val;
}

/*===========================================================================
FUNCTION    loc_ext_set_position_mode

DESCRIPTION
   Sets the mode and fix frequnecy (in seconds) for the tracking session.

DEPENDENCIES
   None

RETURN VALUE
   0: success

SIDE EFFECTS
   N/A

===========================================================================*/
int loc_ext_set_position_mode(VzwPositionMode mode, bool isSingleShot, int fix_frequency, int timeOut_sec, const char* credentials)
{
    ENTRY_LOG();
    GpsPositionRecurrence recurrence = isSingleShot ?
                                       GPS_POSITION_RECURRENCE_SINGLE :
                                       GPS_POSITION_RECURRENCE_PERIODIC;
    LocPositionMode locMode;
    switch (mode)
    {
    case VZW_GPS_POSITION_MODE_MS_BASED:
        locMode = LOC_POSITION_MODE_MS_BASED;
        break;
    case VZW_GPS_POSITION_MODE_MS_ASSISTED:
        locMode = LOC_POSITION_MODE_MS_ASSISTED;
        break;
    case VZW_GPS_POSITION_MODE_OPTIMAL_SPEED:
        locMode = LOC_POSITION_MODE_RESERVED_1;
        break;
    case VZW_GPS_POSITION_MODE_OPTIMAL_ACCURACY:
        locMode = LOC_POSITION_MODE_RESERVED_2;
        break;
    case VZW_GPS_POSITION_MODE_OPTIMAL_DATA:
        locMode = LOC_POSITION_MODE_RESERVED_3;
        break;
    case VZW_GPS_POSITION_MODE_CID:
    case VZW_GPS_POSITION_MODE_ECID:
        locMode = LOC_POSITION_MODE_RESERVED_4;
        break;
    case VZW_GPS_POSITION_MODE_AFLT:
        locMode = LOC_POSITION_MODE_RESERVED_5;
        break;
    default:
        locMode = LOC_POSITION_MODE_STANDALONE;
    }

    LocPosMode params(locMode, recurrence, fix_frequency * 1000,
                      100, timeOut_sec * 1000, credentials, "VERIZON");
    int ret_val = loc_eng_set_position_mode(loc_ext_data,params);

    EXIT_LOG(%d, ret_val);
    return ret_val;
}

/*===========================================================================
FUNCTION    loc_ext_set_server

DESCRIPTION
   This is used to set the default AGPS server. Server address is obtained
   from gps.conf.

DEPENDENCIES
   NONE

RETURN VALUE
   0

SIDE EFFECTS
   N/A

===========================================================================*/
int loc_ext_set_server(VzwAGpsType type, const char* hostname, int port)
{
    ENTRY_LOG();
    LocServerType serverType;
    int ret_val;

    switch (type) {
    case VZW_AGPS_TYPE_SUPL:
        serverType = LOC_AGPS_SUPL_SERVER;
        break;
    case VZW_AGPS_TYPE_C2K:
        serverType = LOC_AGPS_CUSTOM_PDE_SERVER;
        break;
    case VZW_AGPS_TYPE_MPC:
        serverType = LOC_AGPS_MPC_SERVER;
        break;
    default:
        LOC_LOGE("Bad parameter VzwAGpsType=%u in %s 0\n", type, __FUNCTION__);
        ret_val = -1;
        goto exit;
    }
    ret_val = loc_eng_set_server_proxy(loc_ext_data, serverType, hostname, port);

exit:
    EXIT_LOG(%d, ret_val);
    return ret_val;
}

void loc_ext_delete_aiding_data(GpsAidingData f)
{
    ENTRY_LOG();
    loc_eng_delete_aiding_data(loc_ext_data, f);

    EXIT_LOG(%s, VOID_RET);
}

static void* parsePositionExt(void* data)
{
    VzwGpsLocationExt* locationExt = NULL;

    if (NULL != data) {
      locationExt = (VzwGpsLocationExt*)malloc(sizeof(*locationExt));

      if (locationExt == NULL) {
          LOC_LOGE("malloc VzwGpsLocationExt failed: out of memory.\n");
      } else {
        memset(locationExt, 0, sizeof(*locationExt));
#ifdef USE_RPC
        rpc_loc_parsed_position_s_type *location_report_ptr =
            (rpc_loc_parsed_position_s_type*)data;

        // Time stamp UTC)
        if (location_report_ptr->valid_mask & RPC_LOC_POS_VALID_TIMESTAMP_UTC)
        {
            locationExt->flags |= VZW_GPS_LOCATION_HAS_TIMESTAMP;
        }

        // Altitude (sea level)
        if (location_report_ptr->valid_mask &  RPC_LOC_POS_VALID_ALTITUDE_WRT_MEAN_SEA_LEVEL )
        {
            locationExt->flags    |= VZW_GPS_LOCATION_HAS_ALTITUDE_SEA_LEVEL;
            locationExt->altitude_sea_level = location_report_ptr->altitude_wrt_mean_sea_level;
        }

        // Uncertainty (elliptical)
        unsigned long long mask_elliptical = RPC_LOC_POS_VALID_HOR_UNC_ELLI_SEMI_MAJ | RPC_LOC_POS_VALID_HOR_UNC_ELLI_SEMI_MIN |
            RPC_LOC_POS_VALID_HOR_UNC_ELLI_ORIENT_AZIMUTH;
        if ( (location_report_ptr->valid_mask & mask_elliptical) == mask_elliptical)
        {
            locationExt->flags    |= VZW_GPS_LOCATION_HAS_ELLI_ACCURACY;
            locationExt->accuracy_hor_ellipse_semi_major = location_report_ptr->hor_unc_ellipse_semi_major;
            locationExt->accuracy_hor_ellipse_semi_minor = location_report_ptr->hor_unc_ellipse_semi_minor;
            locationExt->accuracy_hor_ellipse_angle = location_report_ptr->hor_unc_ellipse_orient_azimuth;
        }

        // Uncertainty (vertical)
        if ( (location_report_ptr->valid_mask & RPC_LOC_POS_VALID_VERTICAL_UNC) )
        {
            locationExt->flags    |= VZW_GPS_LOCATION_HAS_VERT_ACCURACY;
            locationExt->accuracy_vertical = location_report_ptr->vert_unc;
        }

        // Confidence level
        if ( (location_report_ptr->valid_mask & RPC_LOC_POS_VALID_CONFIDENCE_HORIZONTAL) )
        {
            locationExt->flags    |= VZW_GPS_LOCATION_HAS_CONFIDENCE;
            locationExt->confidence_horizontal = location_report_ptr->confidence_horizontal;
        }

        if (location_report_ptr->valid_mask &  RPC_LOC_POS_VALID_MAGNETIC_VARIATION )
        {
            locationExt->flags    |= VZW_GPS_LOCATION_HAS_MAGNETIC_VARIATION;
            locationExt->magnetic_variation = location_report_ptr->magnetic_deviation;
        }
#elif USE_QMI
        qmiLocEventPositionReportIndMsgT_v02 *location_report_ptr =
            (qmiLocEventPositionReportIndMsgT_v02*)data;

        // Time stamp (UTC)
        if(location_report_ptr->timestampUtc_valid == 1)
        {
            locationExt->flags |= VZW_GPS_LOCATION_HAS_TIMESTAMP;
        }

        // Altitude (sea level)
        if (location_report_ptr->altitudeWrtMeanSeaLevel_valid == 1 )
        {
            locationExt->flags    |= VZW_GPS_LOCATION_HAS_ALTITUDE_SEA_LEVEL;
            locationExt->altitude_sea_level = location_report_ptr->altitudeWrtMeanSeaLevel;
        }

        if ( (location_report_ptr->horUncEllipseSemiMinor_valid ) ||
             (location_report_ptr->horUncEllipseSemiMajor_valid ) ||
             (location_report_ptr->horUncEllipseOrientAzimuth_valid )) \
        {
            locationExt->flags    |= VZW_GPS_LOCATION_HAS_ELLI_ACCURACY;
            locationExt->accuracy_hor_ellipse_semi_major = location_report_ptr->horUncEllipseSemiMajor;
            locationExt->accuracy_hor_ellipse_semi_minor = location_report_ptr->horUncEllipseSemiMinor;
            locationExt->accuracy_hor_ellipse_angle = location_report_ptr->horUncEllipseOrientAzimuth;
        }

        // Uncertainty (vertical)
        if ( (location_report_ptr->vertUnc_valid) )
        {
            locationExt->flags    |= VZW_GPS_LOCATION_HAS_VERT_ACCURACY;
            locationExt->accuracy_vertical = location_report_ptr->vertUnc;
        }

        // Confidence level
        if ( (location_report_ptr->horConfidence_valid ) )
        {
            locationExt->flags    |= VZW_GPS_LOCATION_HAS_CONFIDENCE;
            locationExt->confidence_horizontal = location_report_ptr->horConfidence;
        }

        if (location_report_ptr->magneticDeviation_valid )
        {
            locationExt->flags    |= VZW_GPS_LOCATION_HAS_MAGNETIC_VARIATION;
            locationExt->magnetic_variation = location_report_ptr->magneticDeviation;
        }

        if (location_report_ptr->DOP_valid)
        {
            locationExt->flags    |= VZW_GPS_LOCATION_HAS_DOP;
            locationExt->pdop = location_report_ptr->DOP.PDOP;
            locationExt->hdop = location_report_ptr->DOP.HDOP;
            locationExt->vdop = location_report_ptr->DOP.VDOP;
        }
#endif
      }
    }

    return locationExt;
}

static void* parseSvExt(void* data)
{
    GpsDop* SvDop = NULL;
    if (NULL != data) {
        SvDop = (GpsDop*)malloc(sizeof(*SvDop));

      if (SvDop == NULL) {
          LOC_LOGE("malloc SvDop failed: out of memory.\n");
      } else {
        memset(SvDop, 0, sizeof(*SvDop));

#ifdef USE_RPC
        rpc_loc_gnss_info_s_type *gnss_report_ptr =
            (rpc_loc_gnss_info_s_type*) data;

        if (gnss_report_ptr->valid_mask & RPC_LOC_GNSS_INFO_VALID_POS_DOP)
        {
            SvDop->valid_mask = GPS_SV_STATUS_VALID_PDOP;
            SvDop->PDOP = gnss_report_ptr->position_dop;
        }

        if (gnss_report_ptr->valid_mask & RPC_LOC_GNSS_INFO_VALID_HOR_DOP)
        {
            SvDop->valid_mask = GPS_SV_STATUS_VALID_HDOP;
            SvDop->HDOP = gnss_report_ptr->horizontal_dop;
        }
        if (gnss_report_ptr->valid_mask & RPC_LOC_GNSS_INFO_VALID_VERT_DOP)
        {
            SvDop->valid_mask = GPS_SV_STATUS_VALID_VDOP;
            SvDop->VDOP = gnss_report_ptr->vertical_dop;
        }
#elif USE_QMI
        // For QMI, the DOPs comes in the position report
#endif
      }
    }

    return SvDop;
}

static void local_location_callback(UlpLocation* location, void* ext)
{
    ENTRY_LOG();
    if (NULL != location && NULL != ext) {
        location_callback(&location->gpsLocation, (VzwGpsLocationExt*)ext);
    } else {
        // VzwGpsEventValue vzw_event = VZW_GPS_EVENT_GENERAL_FAILURE;
        // vzw_event_callback(&vzw_event);

        GpsStatus gs ;
        gs.size = sizeof(gs);
        gs.status = GPS_STATUS_SESSION_END;
        CALLBACK_LOG_CALLFLOW("status_cb", %s, loc_get_gps_status_name(gs.status));
        status_callback(&gs);

        LOC_LOGW("loc_ext_report_position: unexpected session failure");
    }

    if (NULL != ext)
        free(ext);
    EXIT_LOG(%s, VOID_RET);
}

static void local_sv_status_callback(GpsSvStatus* sv_status, void* sv_dop)
{
    ENTRY_LOG();
    CALLBACK_LOG_CALLFLOW("sv_status_cb -", %d, sv_status->num_svs);
    sv_status_callback(sv_status, (GpsDop*) sv_dop);
    free(sv_dop);
    EXIT_LOG(%s, VOID_RET);
}
