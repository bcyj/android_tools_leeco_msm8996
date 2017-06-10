/*
 * Copyright (c) 2011-2012, Qualcomm Technologies Incorporated.
 * All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 * Developed by QRD Engineering team.
 */
package com.android.profile;

import android.content.Context;
import android.content.SharedPreferences;
import android.content.res.Resources;
import android.preference.PreferenceManager;
import android.widget.Toast;
import static com.android.profile.ProfileConst.PROFILE_GENERAL;
import static com.android.profile.ProfileConst.PROFILE_MEETING;
import static com.android.profile.ProfileConst.PROFILE_OUTING;
import static com.android.profile.ProfileConst.PROFILE_SILENT;
import static com.android.profile.ProfileConst.PROFILE_POWERSAVE;
import static com.android.profile.ProfileConst.PROFILE_NEW;

public class ProfileUtils {

    public final static String KEY_SELECTED_ID = "idSelected";
    public final static String KEY_DEFAUT_RINGVOLUME = "def_ring_volume";
    public final static String KEY_DEFAUT_BRIGHTNESS = "def_brightness";
    public final static String KEY_DEFAUT_RINGTONE1 = "def_ringtone1";
    public final static String KEY_DEFAUT_RINGTONE2 = "def_ringtone2";

    public static String getStringValueSaved(Context mContext, String key, String def) {

        SharedPreferences mSharedPreferences = PreferenceManager
                .getDefaultSharedPreferences(mContext);
        return mSharedPreferences.getString(key, def);
    }

    public static long getLongValueSaved(Context mContext, String key, long def) {

        SharedPreferences mSharedPreferences = PreferenceManager
                .getDefaultSharedPreferences(mContext);
        return mSharedPreferences.getLong(key, def);
    }

    public static int getIntValueSaved(Context mContext, String key, int def) {

        SharedPreferences mSharedPreferences = PreferenceManager
                .getDefaultSharedPreferences(mContext);
        return mSharedPreferences.getInt(key, def);

    }

    public static boolean getBooleanValueSaved(Context mContext, String key, boolean def) {

        SharedPreferences mSharedPreferences = PreferenceManager
                .getDefaultSharedPreferences(mContext);
        return mSharedPreferences.getBoolean(key, def);

    }

    public static void saveIntValue(Context mContext, String key, int value) {

        SharedPreferences mSharedPreferences = PreferenceManager
                .getDefaultSharedPreferences(mContext);
        SharedPreferences.Editor editor = mSharedPreferences.edit();
        editor.putInt(key, value);
        editor.commit();
    }

    public static void saveStringValue(Context mContext, String key, String value) {

        SharedPreferences mSharedPreferences = PreferenceManager
                .getDefaultSharedPreferences(mContext);
        SharedPreferences.Editor editor = mSharedPreferences.edit();
        editor.putString(key, value);
        editor.commit();
    }

    public static String getDBProfileName(Context mContext, String profile_name){
        Resources resources = mContext.getResources();
        if(profile_name.equals(resources.getText(R.string.general))){
            return PROFILE_GENERAL;
        }else if(profile_name.equals(resources.getText(R.string.meeting))){
            return PROFILE_MEETING;
        }else if(profile_name.equals(resources.getText(R.string.outing))){
            return PROFILE_OUTING;
        }else if(profile_name.equals(resources.getText(R.string.silent))){
            return PROFILE_SILENT;
        }else if(profile_name.equals(resources.getText(R.string.powersave))){
            return PROFILE_POWERSAVE;
        } else if (profile_name.equals(resources.getText(R.string.default_name))) {
            return PROFILE_NEW;
        }
        return profile_name;
    }

    public static String getShowProfileName(Context mContext, String profile_name){
        Resources resources = mContext.getResources();
        if(profile_name.equals(PROFILE_GENERAL)){
            return resources.getText(R.string.general).toString();
        }else if(profile_name.equals(PROFILE_MEETING)){
            return resources.getText(R.string.meeting).toString();
        }else if(profile_name.equals(PROFILE_OUTING)){
            return resources.getText(R.string.outing).toString();
        }else if(profile_name.equals(PROFILE_SILENT)){
            return resources.getText(R.string.silent).toString();
        }else if(profile_name.equals(PROFILE_POWERSAVE)){
            return resources.getText(R.string.powersave).toString();
        } else if (profile_name.equals(PROFILE_NEW)) {
            // If the profile name is new profile we should get the string
            // from resource.
            return resources.getText(R.string.default_name).toString();
        }
        return profile_name;
    }
}
