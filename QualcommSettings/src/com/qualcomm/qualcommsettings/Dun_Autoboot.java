/*******************************************************************************
@file    Dun_Autoboot.java
@brief   Dun Auto Boot

DESCRIPTION
Used for booting DunService once kernel boots up

---------------------------------------------------------------------------
Copyright (C) 2009-2011 Qualcomm Technologies, Inc.
All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
---------------------------------------------------------------------------
******************************************************************************/

package com.qualcomm.qualcommsettings;

import com.qualcomm.qcnvitems.IQcNvItems;
import com.qualcomm.qcnvitems.QcNvItems;
import com.qualcomm.qcnvitems.QmiNvItemTypes.AutoAnswer;
import android.os.Bundle;
import android.os.SystemProperties;
import android.app.Service;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.telephony.PhoneStateListener;
import android.telephony.ServiceState;
import android.telephony.TelephonyManager;
import android.util.Log;
import android.provider.Settings;
import android.provider.Settings.SettingNotFoundException;
import android.content.ContentResolver;
import java.io.IOException;
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;
import android.content.SharedPreferences.OnSharedPreferenceChangeListener;
import android.preference.Preference;
import android.preference.PreferenceActivity;
import android.preference.PreferenceCategory;

public class Dun_Autoboot extends BroadcastReceiver {

    /**
     * Variables
     */
    private static final String TAG = "Dun Auto Boot";
    private Context mContext;

    @Override
    public void onReceive(Context context, Intent intent) {
        mContext = context;

        if ("android.intent.action.BOOT_COMPLETED".equals(intent.getAction())) {
            ComponentName comp = new ComponentName(context.getPackageName(), DunService.class
                    .getName());
            if (comp != null) {
                ComponentName service = context.startService(new Intent().setComponent(comp));
                if (service == null) {
                    Log.e(TAG, "Could Not Start Service " + comp.toString());
                } else {
                    Log.e(TAG, "Dun Auto Boot Started Successfully");
                }
            } else {
                Log.e(TAG, "Dun Auto Boot Not Started Successfully");
            }
        } else {
            Log.e(TAG, "Received Unexpected Intent " + intent.toString());
        }

    }

}
