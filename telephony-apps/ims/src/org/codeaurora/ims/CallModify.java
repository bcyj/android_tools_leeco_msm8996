/* Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package org.codeaurora.ims;

public class CallModify {
    // Keep this error codes in sync with error codes defined in
    // imsIF.proto file.
    public static int E_SUCCESS = 0;
    public static int E_CANCELLED = 7;
    public static int E_UNUSED = 16;

    public int call_index;

    public CallDetails call_details;

    public int error;

    public CallModify() {
        this(new CallDetails(), 0);
    }

    public CallModify(CallDetails callDetails, int callIndex) {
        this(callDetails, callIndex, E_SUCCESS);
    }

    public CallModify(CallDetails callDetails, int callIndex, int err) {
        call_details = callDetails;
        call_index = callIndex;
        error = err;
    }

    public void setCallDetails(CallDetails calldetails) {
        call_details = new CallDetails(calldetails);
    }

    /**
     * @return true if the message is sent to notify about the error.
     */
    public boolean error() {
        return this.error != E_UNUSED && this.error != E_SUCCESS;
    }

    /**
     * @return string representation.
     */
    @Override
    public String toString() {
        return (" " + call_index
                + " " + call_details
                + " " + error);
    }
}
