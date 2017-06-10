/******************************************************************************
 * ---------------------------------------------------------------------------
 *  Copyright (C) 2013-2014 Qualcomm Technologies, Inc.
 *  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
 *  ---------------------------------------------------------------------------
 *******************************************************************************/

package com.qti.ultrasound.penpairing;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.util.Log;

public class PairingBroadcastReceiver extends BroadcastReceiver {
    private static final String PEN_ID_INTENT_KEY = "penId";

    private static final String PEN_TYPE_INTENT_KEY = "penType";

    private static final String PEN_NAME_INTENT_KEY = "penName";

    private static final int RESULT_FAIL = -1;

    private static final int RESULT_SUCCESS = 0;

    private static final String PEN_ID_INTENT_FAIL = "'penId' key wasn't found in intent";

    private static final String PEN_TYPE_INTENT_FAIL = "'penType' key wasn't found in intent";

    private static final String PEN_NAME_INTENT_FAIL = "'penName' key wasn't found in intent";

    private static final String PEN_NAME_EXISTS = "penName given already exists in database";

    @Override
    public void onReceive(Context context, Intent intent) {
        Log.d(this.toString(), "Received broadcast intent: " + intent.toString());

        int penId;
        String penType;
        String penName;

        if (-1 == (penId = intent.getIntExtra(PEN_ID_INTENT_KEY, -1))) {
            setResultCode(RESULT_FAIL);
            setResultData(PEN_ID_INTENT_FAIL);
            return;
        }

        if (null == (penType = intent.getStringExtra(PEN_TYPE_INTENT_KEY))) {
            setResultCode(RESULT_FAIL);
            setResultData(PEN_TYPE_INTENT_FAIL);
            return;
        }

        if (null == (penName = intent.getStringExtra(PEN_NAME_INTENT_KEY))) {
            setResultCode(RESULT_FAIL);
            setResultData(PEN_NAME_INTENT_FAIL);
            return;
        }

        PairingDbHelper db = new PairingDbHelper(context);
        if (-1 == db.addNamedPen(penId, penType, penName)) {
            setResultCode(RESULT_FAIL);
            setResultData(PEN_NAME_EXISTS);
            return;
        }

        SharedPreferences.Editor editor = context.getSharedPreferences(
                PairedPensActivity.NAME_SHARED_PREFERENCE, 0).edit();
        PairedPensActivity.changeCurrent(penName, db, editor);
        setResultCode(RESULT_SUCCESS);
        setResultData("Inserted pen named '" + penName + "' with series " + penId + "with type " + penType + " to database");
    }

}
