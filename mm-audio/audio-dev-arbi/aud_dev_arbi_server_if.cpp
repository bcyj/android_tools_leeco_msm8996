/*===========================================================================
          aud_dev_arbi_server_if.cpp

DESCRIPTION:
Implementation of the server side of the audio device arbitration mechanism.

INITIALIZATION AND SEQUENCING REQUIREMENTS:

Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
=============================================================================*/

#define LOG_TAG "aud_dev_arbi_server"

/*----------------------------------------------------------------------------
   Include files
----------------------------------------------------------------------------*/
#include <utils/Log.h>
#include <errno.h>
#include <sys/file.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "aud_dev_arbi.h"
#include "common_log.h"

extern const char *AUD_DEV_ARBI_SOCKET_FULL_PATH;

static ServerIpcEndpoint *serverEp = NULL;

/*==============================================================================
   FUNCTION:  aud_dev_arbi_server_init
==============================================================================*/
extern "C" int aud_dev_arbi_server_init()
{
    if (serverEp != NULL)
        return 0;

    serverEp = new ServerIpcEndpoint();
    if (serverEp->start(AUD_DEV_ARBI_SOCKET_FULL_PATH) < 0) {
        LOGE("%s: error starting server endpoint",
           __FUNCTION__);

        delete serverEp;
        serverEp = NULL;

        return -EINVAL;
    }

    return 0;
}

/*==============================================================================
   FUNCTION:  aud_dev_arbi_server_deinit
==============================================================================*/
extern "C" int aud_dev_arbi_server_deinit()
{
    if (NULL == serverEp)
        return 0;

    serverEp->stop();

    delete serverEp;
    serverEp = NULL;

    return 0;
}

/*==============================================================================
   FUNCTION:  aud_dev_arbi_server_acquire
==============================================================================*/
extern "C" int aud_dev_arbi_server_acquire
(
    audio_devices_t audDev
)
{
    int rc = -EINVAL;

    if (NULL == serverEp) {
        LOGE("%s: server IF not initialized",
           __FUNCTION__);
        return rc;
    }

    if (!serverEp->acquireDevice(audDev)) {
        LOGE("%s: error acquiring device",
           __FUNCTION__);
        return rc;
    }

    rc = 0;
    return rc;
}

/*==========================================================================
   FUNCTION:  aud_dev_arbi_server_release
===========================================================================*/
extern "C" int aud_dev_arbi_server_release
(
    audio_devices_t audDev
)
{
    int rc = -EINVAL;

    if (NULL == serverEp) {
        LOGE("%s: server IF not initialized",
           __FUNCTION__);
        return rc;
    }

    if (!serverEp->releaseDevice(audDev)) {
        LOGE("%s: error releasing device",
           __FUNCTION__);
        return rc;
    }

    rc = 0;
    return rc;
}
