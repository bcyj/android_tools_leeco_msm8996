/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
  Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
=============================================================================*/

package com.qualcomm.location.pvd;

import libs.com.qualcomm.location.pvd.PVDStatus;

/**
 * System-private API for talking to PVD service
 *
 * @hide
 */
interface IPVDResponseListener
{
    void onPipVenueDetectionStatus(in PVDStatus status, String json);
}
