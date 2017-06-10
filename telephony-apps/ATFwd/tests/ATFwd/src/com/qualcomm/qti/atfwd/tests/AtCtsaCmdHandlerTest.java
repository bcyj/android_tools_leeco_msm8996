/*
 * Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.qti.atfwd.tests;

import android.test.AndroidTestCase;

import com.qualcomm.atfwd.AtCmd;
import com.qualcomm.atfwd.AtCmdResponse;
import com.qualcomm.atfwd.AtCtsaCmdHandler;

public class AtCtsaCmdHandlerTest  extends CommandHandler {

    public void testGetCommandName() throws Exception {
        AtCtsaCmdHandler atCtsaCmdHandler = new AtCtsaCmdHandler(getContext());
        assertEquals("+CTSA", atCtsaCmdHandler.getCommandName());
    }

    public void testHandleCommandOpCodeNaEqQu() throws Exception {
        AtCmd atCmd = buildOpcodeNaEqQuAtCmd("+CTSA");
        AtCmdResponse atCmdResponse = getAtCmdFwdIface().processAtCmd(atCmd);
        assertOkResponse(atCmdResponse, "+CTSA: (0-4)");
    }

    public void testOpCodeNaEqArInvalidTokenLength() throws Exception {
        String[] tokens = new String[2];
        tokens[0] = "0";
        tokens[1] = "15";
        AtCmd atCmd = buildOpcodeNaEqArAtCmd("+CTSA", tokens);
        AtCmdResponse atCmdResponse = getAtCmdFwdIface().processAtCmd(atCmd);
        assertErrorResponse(atCmdResponse, "+CME ERROR: 0");
    }

    public void testOpCodeNaQuNotSupported() throws Exception {
        AtCmd atCmd = buildOpcodeNaQuAtCmd("+CTSA");
        AtCmdResponse atCmdResponse = getAtCmdFwdIface().processAtCmd(atCmd);
        assertErrorResponse(atCmdResponse, "+CME ERROR: 4");
    }
}