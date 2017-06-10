/*
 * Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.qti.atfwd.tests;

import android.test.AndroidTestCase;

import com.qualcomm.atfwd.AtCmd;
import com.qualcomm.atfwd.AtCmdResponse;
import com.qualcomm.atfwd.AtCrslCmdHandler;

public class AtCrslCmdHandlerTest extends CommandHandler {

    public void testGetCommandName() throws Exception {
        AtCrslCmdHandler atCrslCmdHandler = new AtCrslCmdHandler(getContext());
        assertEquals("+CRSL", atCrslCmdHandler.getCommandName());
    }

    public void testHandleCommandOpCodeNaEqQu() throws Exception {
        AtCmd atCmd = buildOpcodeNaEqQuAtCmd("+CRSL");
        AtCmdResponse atCmdResponse = getAtCmdFwdIface().processAtCmd(atCmd);
        assertOkResult(atCmdResponse);
    }

    public void testHandleCommandOpCodeNaQu() throws Exception {
        AtCmd atCmd = buildOpcodeNaQuAtCmd("+CRSL");
        AtCmdResponse atCmdResponse = getAtCmdFwdIface().processAtCmd(atCmd);
        assertOkResult(atCmdResponse);
    }

    public void testHandleCommandOpCodeNaQuInvalidTokens() throws Exception {
        AtCmd atCmd = buildOpcodeNaEqArAtCmd("+CRSL", new String[0]);
        AtCmdResponse atCmdResponse = getAtCmdFwdIface().processAtCmd(atCmd);
        assertErrorResponse(atCmdResponse, "+CME ERROR: 50");
    }
}