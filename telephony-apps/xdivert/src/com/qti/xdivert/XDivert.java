/**
 * Copyright (c) 2014, Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qti.xdivert;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.os.UserHandle;
import android.telephony.TelephonyManager;
import android.util.Log;

import com.android.internal.telephony.TelephonyIntents;

public class XDivert extends BroadcastReceiver {

    private static final boolean DBG = true;
    private static final String LOG_TAG = "XDivert";
    private Context mContext;
    private XDivertUtility mXDivertUtility;

    @Override
    public void onReceive(Context context, Intent intent) {
        // Return if not a primary user
        if (UserHandle.myUserId() != 0) {
            return;
        }

        Log.v(LOG_TAG,"Action intent recieved:"+intent);
        if (TelephonyManager.getDefault().getMultiSimConfiguration() ==
                TelephonyManager.MultiSimVariants.DSDS) {
            if (Intent.ACTION_BOOT_COMPLETED.equals(intent.getAction()) ) {
                XDivertUtility.init(context);
            } else {
                mXDivertUtility = XDivertUtility.getInstance();
                Log.v(LOG_TAG," mXDivertUtility:" + mXDivertUtility);
                if (mXDivertUtility == null) {
                    XDivertUtility.init(context);
                    mXDivertUtility = XDivertUtility.getInstance();
                }
                mXDivertUtility.onReceive(context, intent);
            }
        }
    }

}
