/*===========================================================================
                           IDigitalPenDataCallback.aidl

Copyright (c) 2012, 2014 Qualcomm Technologies, Inc.  All Rights Reserved.

Qualcomm Technologies Proprietary and Confidential.
=============================================================================*/
package com.qti.snapdragon.digitalpen;

import com.qti.snapdragon.digitalpen.util.DigitalPenData;

/**
 * Note that this is a one-way interface so the server does not block waiting
 * for the client.
 *
 * @hide
 */
oneway interface IDigitalPenDataCallback {
    /**
     * Called when the service gets new data from the daemon.
     */
    void onDigitalPenPropData(in DigitalPenData data);
}
