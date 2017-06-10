/*
 * Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.android.nfc.echoserver;

import com.android.nfc.NfcService;

/**
 * Simulates the lower tester for DTA LLCP echo service testing
 */
public class LowerTesterSimulator extends EchoServer2 {

    public LowerTesterSimulator() {
        mService = NfcService.getInstance();
        mLtMode = true;
    }

}
