/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
  Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
=============================================================================*/

package com.qualcomm.location.lci;

import java.com.qualcomm.location.lci.ILciResponseListener;

/**
 * System-private API for talking to LCI service
 *
 * @hide
 */
interface ILciService
{
    int addRef();
    int release();
    int requestLciCandidates(ILciResponseListener cb);
    int sendUserLciSelection(String lci);
    int unregisterListener(ILciResponseListener cb);
}
