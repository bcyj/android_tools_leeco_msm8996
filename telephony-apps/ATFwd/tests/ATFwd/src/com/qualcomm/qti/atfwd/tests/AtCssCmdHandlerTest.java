/*
 * Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.qti.atfwd.tests;

import android.test.AndroidTestCase;

import com.qualcomm.atfwd.AtCmd;
import com.qualcomm.atfwd.AtCmdResponse;
import com.qualcomm.atfwd.AtCssCmdHandler;

public class AtCssCmdHandlerTest extends CommandHandler {

    public void testGetCommandName() throws Exception {
        AtCssCmdHandler atCssCmdHandler = new AtCssCmdHandler(getContext());
        assertEquals("+CSS", atCssCmdHandler.getCommandName());
    }

    public void testHandleCommandOpCodeNaEqQu() throws Exception {
        AtCmd atCmd = buildOpcodeNaAtCmd("+CSS");
        AtCmdResponse atCmdResponse = getAtCmdFwdIface().processAtCmd(atCmd);
        assertOkResult(atCmdResponse);
    }
}