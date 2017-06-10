/*
 * Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.qti.atfwd.tests;

import android.os.ServiceManager;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.os.IBinder;
import android.test.AndroidTestCase;

import com.qualcomm.atfwd.AtCmd;
import com.qualcomm.atfwd.AtCmdResponse;
import com.qualcomm.atfwd.AtCmdFwdService;

public class CommandHandler extends AndroidTestCase {

    private static final String TAG = CommandHandler.class.getName();
    private static final int WAIT_TIME = 3*60*1000;

    @Override
    protected void setUp() throws Exception {
        boolean serviceStarted = false;
        IBinder atCmdFwdIBinder = ServiceManager.checkService("AtCmdFwd");
        if(atCmdFwdIBinder == null) {
            long currentTimeInMillis = System.currentTimeMillis();
            while (System.currentTimeMillis() != currentTimeInMillis+WAIT_TIME) {
                if(serviceStarted == false) {
                    ComponentName comp = new ComponentName("com.qualcomm.atfwd",
                            "com.qualcomm.atfwd.AtFwdService");
                    ComponentName service = getContext().
                            startService(new Intent().setComponent(comp));
                    assertNotNull(service);
                    serviceStarted = true;
                }
                atCmdFwdIBinder = ServiceManager.checkService("AtCmdFwd");
                if(atCmdFwdIBinder != null) {
                    break;
                }
            }
        }
    }

    public AtCmdFwdService getAtCmdFwdIface() {
        AtCmdFwdService atCmdFwdIface = (AtCmdFwdService)ServiceManager.getService("AtCmdFwd");
        assertNotNull(atCmdFwdIface);
        return atCmdFwdIface;
    }

    public void assertErrorResponse(AtCmdResponse atCmdResponse, String response) {
        assertNotNull(atCmdResponse);
        assertEquals(AtCmdResponse.RESULT_ERROR, atCmdResponse.getResult());
        assertEquals(response, atCmdResponse.getResponse());
    }

    public void assertOkResult(AtCmdResponse atCmdResponse) {
        assertNotNull(atCmdResponse);
        assertEquals(AtCmdResponse.RESULT_OK, atCmdResponse.getResult());
    }

    public void assertOkResponse(AtCmdResponse atCmdResponse, String response) {
        assertNotNull(atCmdResponse);
        assertEquals(AtCmdResponse.RESULT_OK, atCmdResponse.getResult());
        assertEquals(response, atCmdResponse.getResponse());
    }

    public AtCmd buildOpcodeNaEqQuAtCmd(String name) {
        return new AtCmd(AtCmd.ATCMD_OPCODE_NA_EQ_QU, name, new String[0]);
    }

    public AtCmd buildOpcodeNaQuAtCmd(String name) {
        return new AtCmd(AtCmd.ATCMD_OPCODE_NA_QU, name, new String[0]);
    }

    public AtCmd buildOpcodeNaAtCmd(String name) {
        return new AtCmd(AtCmd.AT_OPCODE_NA, name, new String[0]);
    }

    public AtCmd buildOpcodeNaEqArAtCmd(String name, String[] tokens) {
        return new AtCmd(AtCmd.ATCMD_OPCODE_NA_EQ_AR, "+CMAR", tokens);
    }
}
