/*******************************************************************************
    Copyright (c) 2011,2012 Qualcomm Technologies, Inc.  All Rights Reserved.
    Qualcomm Technologies Proprietary and Confidential.
*******************************************************************************/

package com.qualcomm.atfwd;

import android.content.Context;
import android.os.HandlerThread;
import android.os.ServiceManager;
import android.view.WindowManager;
import android.util.Log;

public class AtCssCmdHandler extends AtCmdBaseHandler implements AtCmdHandler {

    private static final String TAG = "AtCssCmdHandler";

    public AtCssCmdHandler(Context c) throws AtCmdHandlerInstantiationException
    {
        super(c);
    }

    private WindowManager getWindowManager() {
        WindowManager wm = (WindowManager) mContext.getSystemService(Context.WINDOW_SERVICE);
        if (wm == null) {
            Log.e(TAG, "Unable to find WindowManager interface.");
        }
        return wm;
    }

    @Override
    public String getCommandName() {
        return "+CSS";
    }

    @Override
    public AtCmdResponse handleCommand(AtCmd cmd) {
        String tokens[] = cmd.getTokens();
        String result = null;
        boolean isAtCmdRespOK = false;

        Log.d(TAG, "OpCode  " + cmd.getOpcode());
        switch (cmd.getOpcode()) {
            case  AtCmd.AT_OPCODE_NA:
                // AT+CSS
                try {
                    WindowManager wm = getWindowManager();
                    if(null == wm) {
                        Log.e(TAG, "Unable to find WindowManager interface.");
                        result = cmd.getAtCmdErrStr(AtCmd.AT_ERR_PHONE_FAILURE);
                        break;
                    }
                    int screenHeight = wm.getDefaultDisplay().getHeight();
                    int screenWidth = wm.getDefaultDisplay().getWidth();
                    result = getCommandName() +": " + screenWidth + "," + screenHeight;
                    Log.d(TAG," At Result :" + result);
                    isAtCmdRespOK = true;
                }
                catch (SecurityException e) {
                    Log.e(TAG, "SecurityException: " + e);
                    result = cmd.getAtCmdErrStr(AtCmd.AT_ERR_OP_NOT_ALLOW);
                }
                break;
        }

        return isAtCmdRespOK ? new AtCmdResponse(AtCmdResponse.RESULT_OK, result) :
            new AtCmdResponse(AtCmdResponse.RESULT_ERROR, result);
    }
}

