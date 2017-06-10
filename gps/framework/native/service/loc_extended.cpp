/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*

       loc extended

GENERAL DESCRIPTION
  loc extended module

  Copyright (c) 2013-2014 Qualcomm Atheros, Inc.
  All Rights Reserved.
  Qualcomm Atheros Confidential and Proprietary.

  Copyright (c) 2009 Qualcomm Technologies, Inc.
  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
=============================================================================*/

#define LOG_NDDEBUG 0
#define LOG_TAG "LocSvc_ext"

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
#include <android_runtime/AndroidRuntime.h>
#include <cutils/properties.h>
#include <utils/SystemClock.h>
#include <string.h>

#include <loc.h>
#include <loc_eng.h>
#include "loc_extended.h"

// Global data structure for location engine
loc_eng_data_s_type loc_prop_data;

int loc_extended_init(GpsExtCallbacks* callbacks)
{
    int retVal = -1;
    ENTRY_LOG();
    if(callbacks == NULL) {
        LOC_LOGE("loc_init failed. cb = NULL\n");
        EXIT_LOG(%d, retVal);
        return retVal;
    }
    LOC_API_ADAPTER_EVENT_MASK_T event =
        LOC_API_ADAPTER_BIT_LOCATION_SERVER_REQUEST |
        LOC_API_ADAPTER_BIT_ASSISTANCE_DATA_REQUEST |
        LOC_API_ADAPTER_BIT_NI_NOTIFY_VERIFY_REQUEST;
    LocCallbacks clientCallbacks = {NULL, /* location_cb */
                                    NULL, /* status_cb */
                                    NULL, /* sv_status_cb */
                                    NULL, /* nmea_cb */
                                    callbacks->set_capabilities_cb, /* set_capabilities_cb */
                                    callbacks->acquire_wakelock_cb, /* acquire_wakelock_cb */
                                    callbacks->release_wakelock_cb, /* release_wakelock_cb */
                                    callbacks->create_thread_cb, /* create_thread_cb */
                                    NULL, /* location_ext_parser */
                                    NULL, /* sv_ext_parser */
                                    callbacks->request_utc_time_cb /* request_utc_time_cb */};

    retVal = loc_eng_init(loc_prop_data, &clientCallbacks, event, NULL);
    loc_prop_data.adapter->mSupportsAgpsRequests = true;
    loc_prop_data.adapter->mSupportsPositionInjection = true;
    loc_prop_data.adapter->mSupportsTimeInjection = true;

    EXIT_LOG(%d, retVal);
    return retVal;
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
void loc_extended_cleanup()
{
    ENTRY_LOG();
    loc_eng_cleanup(loc_prop_data);
    EXIT_LOG(%s, VOID_RET);
}

/*===========================================================================
FUNCTION    loc_agps_init

DESCRIPTION
   Initialize the AGps interface.

DEPENDENCIES
   NONE

RETURN VALUE
   0

SIDE EFFECTS
   N/A

===========================================================================*/
void loc_extended_agps_init(AGpsExtCallbacks* callbacks)
{
    ENTRY_LOG();
    loc_eng_agps_init(loc_prop_data, callbacks);
    EXIT_LOG(%s, VOID_RET);
}

/*===========================================================================
FUNCTION    loc_agps_open

DESCRIPTION
   This function is called when on-demand data connection opening is successful.
It should inform ARM 9 about the data open result.

DEPENDENCIES
   NONE

RETURN VALUE
   0

SIDE EFFECTS
   N/A

===========================================================================*/
int loc_extended_agps_open(AGpsExtType agpsType,
                         const char* apn, AGpsBearerType bearerType)
{
    ENTRY_LOG();
    int ret_val = loc_eng_agps_open(loc_prop_data, agpsType, apn, bearerType);

    EXIT_LOG(%d, ret_val);
    return ret_val;
}

/*===========================================================================
FUNCTION    loc_agps_closed

DESCRIPTION
   This function is called when on-demand data connection closing is done.
It should inform ARM 9 about the data close result.

DEPENDENCIES
   NONE

RETURN VALUE
   0

SIDE EFFECTS
   N/A

===========================================================================*/
int loc_extended_agps_closed(AGpsExtType agpsType)
{
    ENTRY_LOG();
    int ret_val = loc_eng_agps_closed(loc_prop_data, agpsType);

    EXIT_LOG(%d, ret_val);
    return ret_val;
}


/*===========================================================================
FUNCTION    loc_agps_open_failed

DESCRIPTION
   This function is called when on-demand data connection opening has failed.
It should inform ARM 9 about the data open result.

DEPENDENCIES
   NONE

RETURN VALUE
   0

SIDE EFFECTS
   N/A

===========================================================================*/
int loc_extended_agps_open_failed(AGpsExtType agpsType)
{
    ENTRY_LOG();
    int ret_val = loc_eng_agps_open_failed(loc_prop_data, agpsType);

    EXIT_LOG(%d, ret_val);
    return ret_val;
}

/*===========================================================================
FUNCTION    loc_agps_set_server

DESCRIPTION
   If loc_eng_set_server is called before loc_eng_init, it doesn't work. This
   proxy buffers server settings and calls loc_eng_set_server when the client is
   open.

DEPENDENCIES
   NONE

RETURN VALUE
   0

SIDE EFFECTS
   N/A

===========================================================================*/
int loc_extended_agps_set_server(AGpsExtType type, const char* hostname, int port)
{
    ENTRY_LOG();
    LocServerType serverType;
    switch (type) {
    case AGPS_TYPE_SUPL:
        serverType = LOC_AGPS_SUPL_SERVER;
        break;
    case AGPS_TYPE_C2K:
        serverType = LOC_AGPS_CDMA_PDE_SERVER;
        break;
    }
    int ret_val = loc_eng_set_server_proxy(loc_prop_data, serverType, hostname, port);

    EXIT_LOG(%d, ret_val);
    return ret_val;
}

/*===========================================================================
FUNCTION    loc_xtra_init

DESCRIPTION
   Initialize XTRA module.

DEPENDENCIES
   None

RETURN VALUE
   0: success

SIDE EFFECTS
   N/A

===========================================================================*/
int loc_extended_xtra_init(GpsXtraExtCallbacks* callbacks)
{
    ENTRY_LOG();
    int ret_val = loc_eng_xtra_init(loc_prop_data, callbacks);

    EXIT_LOG(%d, ret_val);
    return ret_val;
}


/*===========================================================================
FUNCTION    loc_xtra_inject_data

DESCRIPTION
   Initialize XTRA module.

DEPENDENCIES
   None

RETURN VALUE
   0: success

SIDE EFFECTS
   N/A

===========================================================================*/
int loc_extended_xtra_inject_data(char* data, int length)
{
    ENTRY_LOG();
    int ret_val = loc_eng_xtra_inject_data(loc_prop_data, data, length);

    EXIT_LOG(%d, ret_val);
    return ret_val;
}

/*===========================================================================
FUNCTION    loc_extended_xtra_request_server

DESCRIPTION
   Request the XTRA server.

DEPENDENCIES
   None

RETURN VALUE
   0: success

SIDE EFFECTS
   N/A

===========================================================================*/
int loc_extended_xtra_request_server()
{
    ENTRY_LOG();
    int ret_val = loc_eng_xtra_request_server(loc_prop_data);

    EXIT_LOG(%d, ret_val);
    return ret_val;
}

/*===========================================================================
FUNCTION    loc_ni_init

DESCRIPTION
   This function initializes the NI interface

DEPENDENCIES
   NONE

RETURN VALUE
   None

SIDE EFFECTS
   N/A

===========================================================================*/
void loc_extended_ni_init(GpsNiExtCallbacks *callbacks)
{
    ENTRY_LOG();
    loc_eng_ni_init(loc_prop_data, callbacks);
    EXIT_LOG(%s, VOID_RET);
}

/*===========================================================================
FUNCTION    loc_ni_respond

DESCRIPTION
   This function sends an NI respond to the modem processor

DEPENDENCIES
   NONE

RETURN VALUE
   None

SIDE EFFECTS
   N/A

===========================================================================*/
void loc_extended_ni_respond(int notif_id, GpsUserResponseType user_response)
{
    ENTRY_LOG();
    loc_eng_ni_respond(loc_prop_data, notif_id, user_response);
    EXIT_LOG(%s, VOID_RET);
}

/*===========================================================================
FUNCTION    loc_extended_inject_time

DESCRIPTION
   This is used by Java native function to do time injection.

DEPENDENCIES
   None

RETURN VALUE
   0

SIDE EFFECTS
   N/A

===========================================================================*/
int loc_extended_inject_time(GpsUtcTime time, int64_t timeReference, int uncertainty)
{
    ENTRY_LOG();
    int ret_val = loc_eng_inject_time(loc_prop_data, time, timeReference, uncertainty);
    EXIT_LOG(%d, ret_val);
    return ret_val;
}


