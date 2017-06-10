/* Copyright (c) 2013, Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#include <hardware/gps.h>
#include <log_util.h>
#include <loc_vzw.h>
#include "utils/Log.h"
#include "fake_logd_write.h"

void nmea_callback (GpsUtcTime timestamp, const char *nmea, int length)
{
    LOC_LOGI ("test_gps_nmea_cb:");
}

void sv_status_callback(GpsSvStatus * sv_info, GpsDop* dop)
{
    LOC_LOGI ("test_gps_sv_status_cb:");
}

void status_callback(GpsStatus * status)
{
    LOC_LOGI ("test_gps_status_cb: %d", status->status);
}

void location_callback (GpsLocation * location, VzwGpsLocationExt* locExt)
{
    LOC_LOGI ("test_gps_location_cb: lat: %f, lon %f, acc %f, time %llu",
                 location->latitude, location->longitude, location->accuracy,
                 (long long) location->timestamp);
}
