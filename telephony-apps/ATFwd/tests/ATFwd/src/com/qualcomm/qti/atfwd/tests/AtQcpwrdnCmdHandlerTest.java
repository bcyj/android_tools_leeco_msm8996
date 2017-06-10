/*
 * Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.qti.atfwd.tests;

import android.test.AndroidTestCase;

import com.qualcomm.atfwd.AtCmd;
import com.qualcomm.atfwd.AtCmdResponse;
import com.qualcomm.atfwd.AtQcpwrdnCmdHandler;

public class AtQcpwrdnCmdHandlerTest  extends CommandHandler {

    public void testGetCommandName() throws Exception {
        AtQcpwrdnCmdHandler atQcpwrdnCmdHandler = new AtQcpwrdnCmdHandler(getContext());
        assertEquals("$QCPWRDN", atQcpwrdnCmdHandler.getCommandName());
    }

    public void testHandleCommandOpCodeTokensNotSupported() throws Exception {
        String[] tokens = new String[1];
        tokens[0] = "12";
        AtCmd atCmd = buildOpcodeNaEqArAtCmd("$QCPWRDN", tokens);
        AtCmdResponse atCmdResponse = getAtCmdFwdIface().processAtCmd(atCmd);
        assertErrorResponse(atCmdResponse, "+CME ERROR: 0");
    }

    public void testHandleCommandInitiateShutdown() throws Exception {
        AtCmd atCmd = new AtCmd(0, "$QCPWRDN", new String[0]);
        AtCmdResponse atCmdResponse = getAtCmdFwdIface().processAtCmd(atCmd);
        assertOkResult(atCmdResponse);
    }
}