/*
 * Copyright (c) 2015, Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
package com.qualcomm.qti.telephony.extcarrierpack;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.util.Log;

public class CarrierNewOutgoingCallReceiver extends BroadcastReceiver{

    private static boolean DBG = Log.isLoggable(CarrierpackApplication.TAG, Log.DEBUG);

    @Override
    public void onReceive(Context context, Intent intent) {
        log("onreceive : "+intent.getAction());
        if (intent != null
                && intent.getAction().equals(Intent.ACTION_NEW_OUTGOING_CALL)
                && intent.getStringExtra(Intent.EXTRA_PHONE_NUMBER)
                .equals(context.getString(R.string.np_lock_code))) {

            abortBroadcast();
            this.setResultData("");
            clearAbortBroadcast();

            if (CarrierpackApplication.getInstance().isPersoLocked()
                    && !CarrierpackApplication.getInstance().isDePersonalizationEnabled()) {
                Intent npLockIntent = new Intent("org.codeaurora.carrier.ACTION_DEPERSO_PANEL");
                npLockIntent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                context.startActivity(npLockIntent);
            } else {
                log("phone is not personalized or depersonalization is not disabled in phone app");
            }
        }
    }

    private void log(String logMsg) {
        if (DBG) Log.d(CarrierpackApplication.TAG, logMsg);
    }

}
