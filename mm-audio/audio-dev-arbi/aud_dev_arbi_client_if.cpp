/*===========================================================================
          aud_dev_arbi_client_if.cpp

DESCRIPTION:
Implementation of the client side of the audio device arbitration mechanism.

INITIALIZATION AND SEQUENCING REQUIREMENTS:

Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
=============================================================================*/

#define LOG_TAG "aud_dev_arbi_client"

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
#include "aud_dev_arbi_client_if.h"

extern const char *AUD_DEV_ARBI_SOCKET_NAME;
extern const char *AUD_DEV_ARBI_SOCKET_FULL_PATH;

/*==============================================================================
   FUNCTION:  aud_dev_arbi_client_handle_t
==============================================================================*/
extern "C" int aud_dev_arbi_client_init
(
    aud_dev_arbi_client_handle_t *outHandle,
    aud_dev_arbi_event_cb_t evtCb
)
{
    if (NULL == outHandle) {
        LOGE("%s: null handle pointer", __FUNCTION__);
        return -EINVAL;
    }

    if (NULL == evtCb) {
        LOGE("%s: null event callback", __FUNCTION__);
        return -EINVAL;
    }

    ClientIpcEndpoint *clientEp = new ClientIpcEndpoint();
    if (clientEp->start(AUD_DEV_ARBI_SOCKET_FULL_PATH, evtCb) < 0) {
        LOGE("%s: error starting client endpoint",
           __FUNCTION__);

        delete clientEp;
        return -EINVAL;
    }

    *outHandle = (void*)clientEp;
    return 0;
}

/*==============================================================================
   FUNCTION:  aud_dev_arbi_client_deinit
==============================================================================*/
extern "C" int aud_dev_arbi_client_deinit
(
    aud_dev_arbi_client_handle_t handle
)
{
    if (NULL == handle) {
        LOGE("%s: null handle", __FUNCTION__);
        return -EINVAL;
    }

    ClientIpcEndpoint *clientEp = (ClientIpcEndpoint*)handle;

    clientEp->stop();
    delete clientEp;

    return 0;
}

/*==============================================================================
   FUNCTION:  aud_dev_arbi_client_register
==============================================================================*/
extern "C" int aud_dev_arbi_client_register
(
    aud_dev_arbi_client_handle_t handle,
    aud_dev_arbi_client_t client,
    audio_devices_t audDev
)
{
    int rc = -EINVAL;

    if (NULL == handle) {
        LOGE("%s: null handle", __FUNCTION__);
        return rc;
    }

    ClientIpcEndpoint *clientEp = (ClientIpcEndpoint*)handle;

    if (!clientEp->registerForDevice(client, audDev)) {
        LOGE("%s: error registering for device %d",
             __FUNCTION__, audDev);
        return rc;
    }

    rc = 0;
    return rc;
}

/*==============================================================================
   FUNCTION:  aud_dev_arbi_client_acquire
==============================================================================*/
extern "C" int aud_dev_arbi_client_acquire
(
    aud_dev_arbi_client_handle_t handle,
    audio_devices_t audDev
)
{
    int rc = -EINVAL;

    if (NULL == handle) {
        LOGE("%s: null handle", __FUNCTION__);
        return rc;
    }

    ClientIpcEndpoint *clientEp = (ClientIpcEndpoint*)handle;

    if (!clientEp->acquireDevice(audDev)) {
        LOGE("%s: error acquiring device %d",
             __FUNCTION__, audDev);
        return rc;
    }

    rc = 0;
    return rc;
}

/*==========================================================================
   FUNCTION:  aud_dev_arbi_client_release
===========================================================================*/
extern "C" int aud_dev_arbi_client_release
(
    aud_dev_arbi_client_handle_t handle,
    audio_devices_t audDev
)
{
    int rc = -EINVAL;

    if (NULL == handle) {
        LOGE("%s: null handle", __FUNCTION__);
        return rc;
    }

    ClientIpcEndpoint *clientEp = (ClientIpcEndpoint*)handle;

    if (!clientEp->releaseDevice(audDev)) {
        LOGE("%s: error releasing device %d",
             __FUNCTION__, audDev);
        return rc;
    }

    rc = 0;
    return rc;
}
