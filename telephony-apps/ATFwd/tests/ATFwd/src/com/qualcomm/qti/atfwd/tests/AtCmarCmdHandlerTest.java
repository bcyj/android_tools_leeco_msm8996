/*
 * Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.qti.atfwd.tests;

import android.test.AndroidTestCase;

import com.qualcomm.atfwd.AtCmd;
import com.qualcomm.atfwd.AtCmdResponse;
import com.qualcomm.atfwd.AtCmarCmdHandler;

public class AtCmarCmdHandlerTest extends CommandHandler {

    public void testGetCommandName() throws Exception {
        AtCmarCmdHandler atCmarCmdHandler = new AtCmarCmdHandler(getContext());
        assertEquals("+CMAR", atCmarCmdHandler.getCommandName());
    }

    public void testHandleOpCodeNaQuNotSupported() throws Exception {
        AtCmd atCmd = buildOpcodeNaQuAtCmd("+CMAR");
        AtCmdResponse atCmdResponse = getAtCmdFwdIface().processAtCmd(atCmd);
        assertErrorResponse(atCmdResponse, "+CME ERROR: 4");
    }

    public void testHandleOpCodeNaEqQu() throws Exception {
        AtCmd atCmd = buildOpcodeNaEqQuAtCmd("+CMAR");
        AtCmdResponse atCmdResponse = getAtCmdFwdIface().processAtCmd(atCmd);
        assertOkResult(atCmdResponse);
    }

    public void testHandleOpCodeNaEqArInvalidTokens() throws Exception {
        AtCmd atCmd = buildOpcodeNaEqArAtCmd("+CMAR", new String[0]);
        AtCmdResponse atCmdResponse = getAtCmdFwdIface().processAtCmd(atCmd);
        assertErrorResponse(atCmdResponse, "+CME ERROR: 50");
    }
}
