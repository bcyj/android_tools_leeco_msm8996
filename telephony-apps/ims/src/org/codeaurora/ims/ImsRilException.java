/*
 * Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package org.codeaurora.ims;

public class ImsRilException extends RuntimeException {
    private int mErrorCode;

    public int getErrorCode() {
        return mErrorCode;
    }

    public ImsRilException(int errorCode, String errorMsg) {
        super(errorMsg);
        mErrorCode = errorCode;
    }
}
