/******************************************************************************
  @file:  loc_vzw.h
  @brief:

  DESCRIPTION
    This file defines the global data structure used by this module.

  INITIALIZATION AND SEQUENCING REQUIREMENTS

  Copyright (c) 2010 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

  This file contains proprietary and confidential information of Qualcomm Technologies, Inc.
  Use, modification or distribution is prohibited without a pre-existing license from
  Qualcomm Technologies, Inc. THE MODIFICATIONS ARE NOT MADE AVAILABLE UNDER THE APACHE LICENSE.

  This file has been modified by Qualcomm Technologies, Inc. This file contains code which is derived from
  code subject to the following license:

  Copyright (C) 2008 The Android Open Source Project

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.


******************************************************************************/

/*=====================================================================
                        EDIT HISTORY FOR MODULE

  This section contains comments describing changes made to the module.
  Notice that changes are listed in reverse chronological order.

when       who      what, where, why
--------   ---      ---------------------------------------------------

======================================================================*/

#ifndef LOC_VZW_H
#define LOC_VZW_H


/** Vzw AGPS server type */
typedef uint16_t VzwAGpsType;
#define VZW_AGPS_TYPE_SUPL          1
#define VZW_AGPS_TYPE_C2K           2
#define VZW_AGPS_TYPE_MPC           3

/** Represents SV status specifically for VZW */
#define GPS_SV_STATUS_VALID_SV_LIST     0x00000001
#define GPS_SV_STATUS_VALID_EPH_MASK    0x00000002
#define GPS_SV_STATUS_VALID_ALM_MASK    0x00000004
#define GPS_SV_STATUS_VALID_USE_MASK    0x00000008
#define GPS_SV_STATUS_VALID_PDOP        0x00000010
#define GPS_SV_STATUS_VALID_HDOP        0x00000020
#define GPS_SV_STATUS_VALID_VDOP        0x00000040

typedef struct {
    int valid_mask;
    float PDOP;
    float HDOP;
    float VDOP;
} GpsDop;

/** Represents a location. */
typedef struct {
    /** Contains GpsLocationFlags bits. */
    uint16_t        flags;
    /** Represents altitude in meters above the sea level . */
    double          altitude_sea_level;
    /** Represents expected semi major of the elliptical model of horizontal accuracy in meters. */
    float           accuracy_hor_ellipse_semi_major;
    /** Represents expected semi minor of the elliptical model of horizontal accuracy in meters. */
    float           accuracy_hor_ellipse_semi_minor;
    /** Represents expected orientation angle of the elliptical model of horizontal accuracy in meters. */
    float           accuracy_hor_ellipse_angle;
    /** Represents expected vertical accuracy in meters. */
    float           accuracy_vertical;
    /** Confidence level of the ellipse */
    float           confidence_horizontal;
    /** magnetic variation in degrees */
    float           magnetic_variation;
    /** position dillusion of precision */
    float          pdop;
    /** horizontal dillusion of precision */
    float          hdop;
    /** vertical dillusion of precision */
    float          vdop;
} VzwGpsLocationExt;


// IMPORTANT: Note that the following values must match
// constants in VzwGpsLocationProvider.java.
/** GPS MSA mode. */
#define VZW_GPS_POSITION_MODE_MS_ASSISTED       0
/** GPS MSB mode. */
#define VZW_GPS_POSITION_MODE_MS_BASED          1
/** Mode for running GPS standalone (no assistance). */
#define VZW_GPS_POSITION_MODE_STANDALONE        2
/** GPS AFLT mode. */
#define VZW_GPS_POSITION_MODE_AFLT              3
/** GPS SPEED OPTIMAL MODE. */
#define VZW_GPS_POSITION_MODE_OPTIMAL_SPEED     4
/** GPS ACCURACY OPTIMAL MODE. */
#define VZW_GPS_POSITION_MODE_OPTIMAL_ACCURACY  5
/** GPS DATA OPTIMAL MODE. */
#define VZW_GPS_POSITION_MODE_OPTIMAL_DATA      6
#define VZW_GPS_POSITION_MODE_CID               7
#define VZW_GPS_POSITION_MODE_ECID              8

typedef uint32_t VzwPositionMode;


/** GPS event values. */
typedef uint32_t VzwGpsEventValue;
// IMPORTANT: Note that the following values must match
// constants in IVzwHalGpsLocationProvider.java.
#define VZW_GPS_EVENT_ESTABLISH_CONNECTION_FAILED      0x00000001
#define VZW_GPS_EVENT_AGPS_AUTH_PASS                   0x00000002
#define VZW_GPS_EVENT_AGPS_AUTH_FAIL                   0x00000003
#define VZW_GPS_EVENT_AGPS_AUTH_PDE_NOT_REACHABLE      0x00000004
#define VZW_GPS_EVENT_AGPS_AUTH_EXPIRED                0x00000005
#define VZW_GPS_EVENT_AGPS_AUTH_DNS_FAIL               0x00000006

#define VZW_GPS_EVENT_INIT_FAIL                        0x00000007
#define VZW_GPS_EVENT_INIT_PASS                        0x00000008

#define VZW_GPS_EVENT_LOCATION_AVAILABLE               0x00000009
#define VZW_GPS_EVENT_FIRST_FIX                        0x0000000A
#define VZW_GPS_EVENT_FIX_REQ_FAIL                     0x0000000B
#define VZW_GPS_EVENT_FIX_REQUESTED                    0x0000000C

#define VZW_GPS_EVENT_GENERAL_AGPS_FAILURE             0x0000000D
#define VZW_GPS_EVENT_GENERAL_FAILURE                  0x0000000E

#define VZW_GPS_EVENT_INIT_CONFIG_NOT_PROVIDED         0x0000000F
#define VZW_GPS_EVENT_LOCATION_REQUEST_TIMEOUT         0x00000010

#define VZW_GPS_EVENT_INIT_SATELLITE_STATUS            0x00000011
#define VZW_GPS_EVENT_SECURITY_FAILED                  0x00000012
#define VZW_GPS_EVENT_SET_FIX_MODE_FAIL                0x00000013
#define VZW_GPS_EVENT_SET_FIX_RATE_FAIL                0x00000014
#define VZW_GPS_EVENT_SET_GPS_PERFORMANCE_FAIL         0x00000015
#define VZW_GPS_EVENT_SET_PDE_FAIL                     0x00000016

#define VZW_GPS_EVENT_STARTED                          0x00000017
#define VZW_GPS_EVENT_STOPPED                          0x00000018


// IMPORTANT: Note that the following values must match
// constants in NativeMethods.java.

/** VzwGpsLocation has valid altitude wrt to sea level. */
#define VZW_GPS_LOCATION_HAS_ALTITUDE_SEA_LEVEL     0x00000100
/** VzwGpsLocation has valid elliptical accuracy */
#define VZW_GPS_LOCATION_HAS_ELLI_ACCURACY          0x00000200
/** VzwGpsLocation has vertical accuracy */
#define VZW_GPS_LOCATION_HAS_VERT_ACCURACY          0x00000400
/** VzwGpsLocation has valid confidence */
#define VZW_GPS_LOCATION_HAS_CONFIDENCE             0x00000800
/** VzwGpsLocation has valid timestamp */
#define VZW_GPS_LOCATION_HAS_TIMESTAMP              0x00001000
/** VzwGpsLocation has valid magnetic variation */
#define VZW_GPS_LOCATION_HAS_MAGNETIC_VARIATION     0x00002000
/** VzwGpsLocation has valid dop */
#define VZW_GPS_LOCATION_HAS_DOP                    0x00004000

int  loc_ext_init();
int  loc_ext_start();
int  loc_ext_stop();
void loc_ext_cleanup();
void loc_ext_set_fix_frequency(int fix_frequency);
int  loc_ext_set_position_mode(VzwPositionMode mode, bool isSingleShot,
                               int fix_frequency, int timeOut_sec,
                               const char* credentials);

int loc_ext_set_server(VzwAGpsType type, const char *hostname, int port);

void loc_ext_delete_aiding_data(GpsAidingData f);

void location_callback(GpsLocation* location, VzwGpsLocationExt* ext);
void status_callback(GpsStatus* status);
void sv_status_callback(GpsSvStatus* sv_status, GpsDop* sv_dop );
void nmea_callback(GpsUtcTime timestamp, const char* nmea, int length);
void vzw_event_callback(VzwGpsEventValue * pEvent);









#endif // LOC_VZW_H
