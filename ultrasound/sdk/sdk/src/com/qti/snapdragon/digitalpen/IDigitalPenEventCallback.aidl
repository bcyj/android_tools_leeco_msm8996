/*===========================================================================
                           IDigitalPenEventCallback.aidl

Copyright (c) 2012, 2014 Qualcomm Technologies, Inc.  All Rights Reserved.

Qualcomm Technologies Proprietary and Confidential.
=============================================================================*/
package com.qti.snapdragon.digitalpen;

import com.qti.snapdragon.digitalpen.util.DigitalPenEvent;

/**
 * Note that this is a one-way interface so the server does not get blocked
 * waiting for the client.
 *
 * @hide
 */
oneway interface IDigitalPenEventCallback {
    /**
     * Called when the service gets new event from the daemon.
     */
    void onDigitalPenPropEvent(in DigitalPenEvent event);
}
