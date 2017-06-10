/* Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package org.codeaurora.ims;

public interface ICallListListener {
    void onCallSessionAdded(ImsCallSessionImpl callSession);

    void onCallSessionRemoved(ImsCallSessionImpl callSession);
}
