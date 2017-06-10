/*************************************************************************
 Copyright (c)2012-2013 Qualcomm Technologies, Inc.  All Rights Reserved.
 Qualcomm Technologies Proprietary and Confidential.
*************************************************************************/

package com.qualcomm.qti.presenceapp;

import java.lang.reflect.Method;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.util.Log;
import android.content.SharedPreferences;
import com.qualcomm.qti.rcsimssettings.QrcsImsSettingsIntType;
import android.os.RemoteException;

public class DataBroadcastReceiver extends BroadcastReceiver {

    public static boolean previousDataEnabledState = false;
    public static boolean AppStart = true;

    @Override
    public void onReceive(Context context, Intent intent) {

        if (AppGlobalState.getOperatorMode() == AppGlobalState.VZW_MODE)
        {
            Log.d("PRESENCE_UI", "DataBroadcastReceiver onReceive on network connectivity change");

            boolean mobileDataEnabled = false;
            ConnectivityManager cm = (ConnectivityManager) context
                    .getSystemService(Context.CONNECTIVITY_SERVICE);
            try {
                Class cmClass = Class.forName(cm.getClass().getName());
                Method method = cmClass.getDeclaredMethod("getMobileDataEnabled");
                method.setAccessible(true); // Make the method callable
                // get the setting for "mobile data"
                mobileDataEnabled = (Boolean) method.invoke(cm);
                Log.d("PRESENCE_UI", "mobileDataEnabled from Settings " + mobileDataEnabled);
            } catch (Exception e) {
                e.printStackTrace();
            }
            Log.d("PRESENCE_UI", "previousDataEnabledState " + previousDataEnabledState
                    + " AppStart  " + AppStart);

            if (intent.getExtras() != null) {
                Log.d("PRESENCE_UI", "NOT  null");

                NetworkInfo ni = (NetworkInfo) intent.getExtras().get(
                        ConnectivityManager.EXTRA_NETWORK_INFO);
                if (ni != null && ni.getState() == NetworkInfo.State.CONNECTED) {
                    SavePreference(context, "DATA_KEY", true);

                    //String networkType = ni.getSubtypeName();
                    //SaveNetworkPreference(context, "NETWORK_TYPE_KEY", networkType);
                    Log.i("PRESENCE_UI",
                            "Network getSubtypeName" + ni.getSubtypeName());
                    Log.i("PRESENCE_UI", "Network getTypeName " + ni.getTypeName()
                            + " connected");
                    try
                    {
                        if (MainActivity.imsSettingService != null
                                && mobileDataEnabled
                                && !previousDataEnabledState)
                        {
                            Log.d("PRESENCE_UI", "Setting Object = "
                                    + MainActivity.imsSettingService);
                            Log.d("PRESENCE_UI", "Setting data enabled");
                            MainActivity.qipcall_config
                                    .setMobile_data_enabled(true);
                            Settings.isMobile_data_enabled = true;
                            MainActivity.qipcall_config
                                    .setMobile_data_enabled_valid(true);
                            Log.d("PRESENCE_UI",
                                    "Before QrcsImsSettings_SetQipcallConfig");
                            QrcsImsSettingsIntType client_handle = new QrcsImsSettingsIntType();
                            client_handle.setQrcsImsSettingsIntType(AppGlobalState
                                    .getQrcsImsSettingsclienthandle());
                            MainActivity.imsSettingService
                                    .QrcsImsSettings_SetQipcallConfig(
                                            client_handle,
                                            MainActivity.qipcall_config);
                            previousDataEnabledState = true;
                            Log.d("PRESENCE_UI", "previousDataEnabledState  changed to "
                                    + previousDataEnabledState);
                        }
                    } catch (RemoteException e) {
                        e.printStackTrace();
                    } catch (NoClassDefFoundError error) {
                        Log.d("PRESENCE_UI", "NoClassDefFoundError = " + error);
                    } catch (Exception exception) {
                        Log.d("PRESENCE_UI", "Exception = " + exception);
                    }

                }

            }
            if (intent.getExtras().getBoolean(
                    ConnectivityManager.EXTRA_NO_CONNECTIVITY, Boolean.FALSE)) {
                Log.d("PRESENCE_UI", "null");
                SavePreference(context, "DATA_KEY", false);

                try {
                    if (MainActivity.imsSettingService != null
                            && !mobileDataEnabled
                            && previousDataEnabledState)
                    {
                        Log.d("PRESENCE_UI", "Setting Object = "
                                + MainActivity.imsSettingService);
                        Log.d("PRESENCE_UI", "Setting data disabled");
                        MainActivity.qipcall_config.setMobile_data_enabled(false);
                        Settings.isMobile_data_enabled = false;
                        MainActivity.qipcall_config
                                .setMobile_data_enabled_valid(true);
                        Log.d("PRESENCE_UI",
                                "Before QrcsImsSettings_SetQipcallConfig");
                        QrcsImsSettingsIntType client_handle = new QrcsImsSettingsIntType();
                        client_handle.setQrcsImsSettingsIntType(AppGlobalState
                                .getQrcsImsSettingsclienthandle());
                        MainActivity.imsSettingService
                                .QrcsImsSettings_SetQipcallConfig(
                                        client_handle,
                                        MainActivity.qipcall_config);
                        previousDataEnabledState = false;
                        Log.d("PRESENCE_UI", "previousDataEnabledState  changed to "
                                + previousDataEnabledState);
                    }
                    else
                    {
                        Log.d("PRESENCE_UI", "Setting object is null");
                    }

                    Log.d("PRESENCE_UI", "There's no network connectivity");

                } catch (RemoteException e) {
                    e.printStackTrace();
                } catch (NoClassDefFoundError error) {
                    Log.d("PRESENCE_UI", "NoClassDefFoundError = " + error);
                } catch (Exception exception) {
                    Log.d("PRESENCE_UI", "Exception = " + exception);
                }
            }
        }

    }

    public void SavePreference(Context ctx, String key, boolean value) {

        SharedPreferences preferences = ctx.getSharedPreferences(
                "presencedata", Context.MODE_PRIVATE);

        SharedPreferences.Editor editor = preferences.edit();
        editor.putBoolean(key, value);
        Log.d("PRESENCE_UI", "Saving ....Data in preference " + value);
        editor.commit();
    }

    public void SaveNetworkPreference(Context ctx, String key, String value) {

        SharedPreferences preferences = ctx.getSharedPreferences(
                "presencedata", Context.MODE_PRIVATE);

        SharedPreferences.Editor editor = preferences.edit();
        editor.putString(key, value);
        Log.d("PRESENCE_UI", "Saving ....Data in preference " + value);
        editor.commit();
    }

    private void sendLogMesg(Context c, String string) {

        Utility.sendLogMesg(c, string);

    }

}
