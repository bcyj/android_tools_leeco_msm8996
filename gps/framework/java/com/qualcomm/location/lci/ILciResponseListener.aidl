/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
  Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
=============================================================================*/

package com.qualcomm.location.lci;

/**
 * System-private API for talking to LCI service
 *
 * @hide
 */
interface ILciResponseListener
{
    void onLciCandidatesReceived(String json);
}
