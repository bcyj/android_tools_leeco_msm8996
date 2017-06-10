/*
 * Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.qti.atfwd.tests;

import android.test.AndroidTestCase;

import com.qualcomm.atfwd.AtCmd;
import com.qualcomm.atfwd.AtCmdResponse;
import com.qualcomm.atfwd.AtCkpdCmdHandler;

public class AtCkpdCmdHandlerTest extends CommandHandler {

    public void testGetCommandName() throws Exception {
        AtCkpdCmdHandler atCkpdCmdHandler = new AtCkpdCmdHandler(getContext());
        assertEquals("+CKPD", atCkpdCmdHandler.getCommandName());
    }

    public void testHandleInvalidTokens() throws Exception {
        String[] tokens = new String[3];
        tokens[0] = "=";
        tokens[1] = "2";
        tokens[2] = "=";
        AtCmd atCmd = buildOpcodeNaEqArAtCmd("+CKPD", tokens);
        AtCmdResponse atCmdResponse = getAtCmdFwdIface().processAtCmd(atCmd);
        assertErrorResponse(atCmdResponse, "+CME ERROR: 0");
    }
}
