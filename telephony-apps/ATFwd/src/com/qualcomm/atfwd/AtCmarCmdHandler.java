/*******************************************************************************
    Copyright (c) 2011 Qualcomm Technologies, Inc.  All Rights Reserved.
    Qualcomm Technologies Proprietary and Confidential.
*******************************************************************************/

package com.qualcomm.atfwd;

import com.qualcomm.atfwd.AtCmdHandler.AtCmdHandlerInstantiationException;
import com.android.internal.os.storage.ExternalStorageFormatter;
import com.android.internal.widget.LockPatternUtils;

import android.app.admin.DevicePolicyManager;
import android.content.Context;
import android.content.ContextWrapper;
import android.content.Intent;
import android.os.Environment;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.os.HandlerThread;
import android.util.Log;

public class AtCmarCmdHandler extends AtCmdBaseHandler implements AtCmdHandler {

    private static final String TAG = "AtCmarCmdHandler";

    private Context mContext;

    private LockPatternUtils mLockPatternUtils;

    private boolean mSdReset = false;

    public AtCmarCmdHandler(Context c) throws AtCmdHandlerInstantiationException {
        super(c);
        mContext = c;
        mLockPatternUtils = new LockPatternUtils(mContext);
    }

    @Override
    public String getCommandName() {
        return "+CMAR";
    }

    @Override
    public AtCmdResponse handleCommand(AtCmd cmd) {

        String tokens[] = cmd.getTokens();
        int opCode = cmd.getOpcode();

        if ((opCode != AtCmd.ATCMD_OPCODE_NA_EQ_AR) && (opCode != AtCmd.ATCMD_OPCODE_NA_EQ_QU)) {
            // CMAR supports only control command and test command
            Log.d(TAG, "CMAR opcode eror");
            return new AtCmdResponse(AtCmdResponse.RESULT_ERROR, cmd
                    .getAtCmdErrStr(AtCmd.AT_ERR_OP_NOT_SUPP));
        }

        if (opCode == AtCmd.ATCMD_OPCODE_NA_EQ_QU) {
            Log.d(TAG, "+CMAR=? test command, RESULT OK ");
            return new AtCmdResponse(AtCmdResponse.RESULT_OK, null);
        }

        if (tokens.length == 0) {
            Log.d(TAG, "CMAR mandatory parameter pin lock code missing ");
            return new AtCmdResponse(AtCmdResponse.RESULT_ERROR, cmd
                    .getAtCmdErrStr(AtCmd.AT_ERR_INCORRECT_PARAMS));
        }

        if (tokens.length > 1 && tokens[1].equals("1")) {
            mSdReset = true;
            Log.d(TAG, " Option enabled to erase SD card, if present ");
        }

        // control command: +CMAR=<phone lock code>

        /*
         * CMAR command will exit with phone failure error for below scenarios.
         * Reset will not be performed
         *
         * 1.If phone uses pattern lock code set. In future we can consider
         * comparing the serialized string of the pattern lock code but in that
         * case TE issuing CMAR has to be aware of the serialized string value.
         * Currently CMAR param does not support pattern lock code.
         *
         *  2. If phone lock code is not set in phone.
         *  PIN verfication is a must as per GSM 27.007 Sec 8.36
         */


        if (mLockPatternUtils.getKeyguardStoredPasswordQuality() ==
            DevicePolicyManager.PASSWORD_QUALITY_SOMETHING) {
            Log.d(TAG, "Pattern Lock enabled/No password set , CMAR is unsupported ");
            return new AtCmdResponse(AtCmdResponse.RESULT_ERROR,
                    cmd.getAtCmdErrStr(AtCmd.AT_ERR_PHONE_FAILURE));

        }

        /* Phone has pin/password lock code - perform PIN verification before reset*/
        if (mLockPatternUtils.checkPassword(tokens[0])) {

            processResetCommand();
            return new AtCmdResponse(AtCmdResponse.RESULT_OK, null);

        } else {

            Log.d(TAG, "+CMAR=<pin lock code > Verification failed");
            return new AtCmdResponse(AtCmdResponse.RESULT_ERROR,
                    cmd.getAtCmdErrStr(AtCmd.AT_ERR_INCORRECT_PASSWORD));

        }

    }

    private void processResetCommand() {

        String status = Environment.getExternalStorageState();
        if (Environment.MEDIA_BAD_REMOVAL.equals(status)
                || Environment.MEDIA_REMOVED.equals(status)
                || !mSdReset) {
            /*
             * SD card not present/SD card is present but sdReset is disabled
             * Wipe internal storage data & reboot
             */
            Log.d(TAG, " Phone Storage MASTER RESET triggered");
            mContext.sendBroadcast(new Intent("android.intent.action.MASTER_CLEAR"));

        } else {
            /*
             * SD card is present and sdReset is enabled. User data & SD card
             * will be erased. As per standards CMAR command does not have an
             * option to say whether or not to erase external storage. sdReset
             * option is implementation specific.
             */
            Log.d(TAG, " External Storage MASTER RESET triggered");
            Intent intent = new Intent(ExternalStorageFormatter.FORMAT_AND_FACTORY_RESET);
            intent.setComponent(ExternalStorageFormatter.COMPONENT_NAME);
            mContext.startService(intent);
        }

    }

}
