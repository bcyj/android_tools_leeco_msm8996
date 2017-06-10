/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*

       loc extended

GENERAL DESCRIPTION
  loc extended module

  Copyright (c) 2013 Qualcomm Atheros, Inc.
  All Rights Reserved.
  Qualcomm Atheros Confidential and Proprietary.

  Copyright (c) 2009 Qualcomm Technologies, Inc.
  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
=============================================================================*/

#ifndef LOC_EXTENDED_H
#define LOC_EXTENDED_H

#include "gps_extended.h"

int loc_extended_init(GpsExtCallbacks* callbacks);
void loc_extended_cleanup();
void loc_extended_agps_init(AGpsExtCallbacks* callbacks);
int loc_extended_agps_open(AGpsExtType agpsType, const char* apn, AGpsBearerType bearerType);
int loc_extended_agps_closed(AGpsExtType agpsType);
int loc_extended_agps_open_failed(AGpsExtType agpsType);
int loc_extended_agps_set_server(AGpsExtType type, const char* hostname, int port);
int loc_extended_xtra_init(GpsXtraExtCallbacks* callbacks);
int loc_extended_xtra_inject_data(char* data, int length);
int loc_extended_xtra_request_server();
void loc_extended_ni_init(GpsNiExtCallbacks *callbacks);
void loc_extended_ni_respond(int notif_id, GpsUserResponseType user_response);
int loc_extended_inject_time(GpsUtcTime time, int64_t timeReference, int uncertainty);

#endif // LOC_EXTENDED_H
