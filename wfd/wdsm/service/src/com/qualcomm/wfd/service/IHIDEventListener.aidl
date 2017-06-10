/* ==============================================================================
 * IHIDEventListener.aidl
 *
 * Copyright (c) 2013 Qualcomm Technologies, Inc.
 * All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
 ============================================================================== */

package com.qualcomm.wfd.service;

oneway interface IHIDEventListener {

    void onHIDReprtDescRcv(in byte[] HIDRD);

    void onHIDReprtRcv(in byte[] HIDRep);

}
