/*
 * Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.qti.atfwd.tests;

import android.test.AndroidTestCase;

import com.qualcomm.atfwd.AtCmd;
import com.qualcomm.atfwd.AtCmdResponse;
import com.qualcomm.atfwd.AtCfunCmdHandler;

public class AtCfunCmdHandlerTest extends CommandHandler {

    public void testGetCommandName() throws Exception {
        AtCfunCmdHandler atCfunCmdHandler = new AtCfunCmdHandler(getContext());
        assertEquals("+CFUN", atCfunCmdHandler.getCommandName());
    }

    public void testOpCodeNaEqNotSupported() throws Exception {
        AtCmd atCmd = buildOpcodeNaEqQuAtCmd("+CFUN");
        AtCmdResponse atCmdResponse = getAtCmdFwdIface().processAtCmd(atCmd);
        assertErrorResponse(atCmdResponse, "+CME ERROR: 1");
    }
}
