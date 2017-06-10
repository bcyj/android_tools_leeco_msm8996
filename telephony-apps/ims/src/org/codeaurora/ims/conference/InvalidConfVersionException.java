/* Copyright (c) 2013 Qualcomm Technologies, Inc
 * All Rights Reserved. Qualcomm Technologies Confidential and Proprietary.
 */

package org.codeaurora.ims.conference;

import android.os.Message;

public class InvalidConfVersionException extends Exception {
    String mMessage;

    public InvalidConfVersionException(String message) {
        super(message);
        this.mMessage = message;
    }

    /* we can further classify Exceptions based error message we get from parser */
    public String toString() {
        return "InvalidConfVersionException" + mMessage;
    }
}
