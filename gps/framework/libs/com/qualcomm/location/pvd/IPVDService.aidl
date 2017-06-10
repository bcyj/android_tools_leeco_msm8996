/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
  Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
=============================================================================*/

package com.qualcomm.location.pvd;

import libs.com.qualcomm.location.pvd.IPVDResponseListener;

/**
 * System-private API for talking to PVD service
 *
 * @hide
 */
interface IPVDService
{
    /**
     * To be called by clients to Register / Unregister
     */
    boolean registerPVDListener(IPVDResponseListener listener, boolean valid_pos, double lat, double lon, float radius);
    boolean unregisterPVDListener(IPVDResponseListener listener);
}
