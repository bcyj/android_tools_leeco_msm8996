/*******************************************************************************
    Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
    Qualcomm Technologies Proprietary and Confidential.
*******************************************************************************/

package com.qualcomm.atfwd;

import android.content.Context;
import android.content.Intent;
import android.os.Handler;
import android.os.UserHandle;
import android.util.Log;

public class AtQcpwrdnCmdHandler extends AtCmdBaseHandler implements AtCmdHandler {

    private static final String TAG = "AtQcpwrdnCmdHandler";
    private Context mContext;
    final Handler mHandler = new Handler(true /* async */);

    public AtQcpwrdnCmdHandler(Context c) throws AtCmdHandlerInstantiationException {
        super(c);
        mContext = c;
    }

    @Override
    public String getCommandName() {
        return "$QCPWRDN";
    }

    @Override
    public AtCmdResponse handleCommand(AtCmd cmd) {
        AtCmdResponse ret = null;
        String tokens[] = cmd.getTokens();

        Log.d(TAG, "$QCPWRDN command processing entry");

        if (tokens.length > 0) {
            /* We currently support $QCPWRDN only. */
            ret = new AtCmdResponse(AtCmdResponse.RESULT_ERROR, "+CME ERROR: 1");
        } else {
            Log.d(TAG, "Initiating Shutdown process");
            mHandler.post(new Runnable() {
                @Override
                public void run() {
                    Intent intent = new Intent(Intent.ACTION_REQUEST_SHUTDOWN);
                    intent.putExtra(Intent.EXTRA_KEY_CONFIRM, false);
                    intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                    mContext.startActivityAsUser(intent, UserHandle.CURRENT);
                    Log.d(TAG, "ShutdownActivity Started");
                }
            });
            Log.d(TAG, "Initiated Shutdown process");
            ret = new AtCmdResponse(AtCmdResponse.RESULT_OK, null);
        }

        return ret;
    }

}
