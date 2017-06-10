/* Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.qti.imstestrunner;

import android.util.Log;

public class ResponseGeneratorFactory {

    private ResponseGeneratorFactory() {
    }

    public static ResponseGenerator getInstance(int testOption) {
        ResponseGenerator ret = null;
        switch (testOption) {
            case (ImsSocketAgent.TEST_BASIC_MO_MT_SUCC):
                ret = new BasicMoMtSuccessResponse();
                break;
                // TODO instantiate ResponseGenerator implementations for each
                // test case
            default:
                Log.d("ResponseGeneratorFactory", "Unknown testOption");
        }
        return ret;
    }
}
