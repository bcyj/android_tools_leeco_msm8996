/* Copyright (c) 2014 Qualcomm Technologies, Inc.
 * All Rights Reserved. Qualcomm Technologies Confidential and Proprietary.
 */

package com.qualcomm.ims.csvt;

public interface CsvtIntents {

    /**
     * <p>Broadcast Action: A new ringing connection has received.
     * The intent will have the following
     * extra values:</p>
     * <ul>
     *   <li><em>connectionAddress</em> - Connection address (e.g. phone number)
     *    associated with connection.</li>
     * </ul>
     */
    public static final String ACTION_NEW_CSVT_RINGING_CONNECTION
            = "qualcomm.intent.action.NEW_CSVT_RINGING_CONNECTION";
}
