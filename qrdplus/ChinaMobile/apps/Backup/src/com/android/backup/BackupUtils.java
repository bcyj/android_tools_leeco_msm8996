/*
 * Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 * Not a Contribution, Apache license notifications and license are retained
 * for attribution purposes only.
 */

/*
 * Copyright (C) 2008 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.android.backup;

import android.content.Context;
import android.content.SharedPreferences;
import android.os.StatFs;
import android.preference.PreferenceManager;
import android.provider.Settings;
import android.telephony.TelephonyManager;
import android.os.storage.IMountService;
import android.os.storage.StorageVolume;
import android.os.ServiceManager;
import android.os.Environment;

import com.android.internal.telephony.PhoneConstants;

public class BackupUtils {

    public static String MULTI_SIM_NAME = "perferred_name_sub";
    public static final int SLOT_ALL = 2;
    public static final int SLOT_1 = 0;
    public static final int SLOT_2 = 1;
    public static final int EVENT_STOP_BACKUP = 100;
    public static final int EVENT_FILE_CREATE_ERR = 101;
    public static final int EVENT_INIT_PROGRESS_TITLE = 102;
    public static final int EVENT_SET_PROGRESS_VALUE = 103;
    public static final int EVENT_SDCARD_NOT_MOUNTED = 104;
    public static final int EVENT_SDCARD_NO_SPACE = 105;
    public static final int EVENT_RESUME_BACKUP_THREAD = 106;
    public static final int EVENT_BACKUP_RESULT = 107;
    public static final int EVENT_RESTORE_RESULT = 108;
    public static final int EVENT_SDCARD_FULL = 109;

    public static final int BACKUP_TYPE_CONTACTS = 1;
    public static final int BACKUP_TYPE_MMS = 2;
    public static final int BACKUP_TYPE_SMS = 3;
    public static final int BACKUP_TYPE_EMAIL = 4;
    public static final int BACKUP_TYPE_CALENDAR = 5;
    public static final int BACKUP_TYPE_ALL = 6;
    public static final int BACKUP_TYPE_PICTURE = 7;
    public static final int BACKUP_TYPE_APP = 8;

    public static final String strExternalPath = System.getenv("SECONDARY_STORAGE");
    public static final String strInternalPath = "/storage/sdcard0";
    public static final String strSDCardPath = System.getenv("SECONDARY_STORAGE");

    public static final String LOCATION_PREFERENCE_NAME = "Location_Preference";
    public static final String KEY_BACKUP_LOCATION = "Key_Backup_Location";
    public static final String KEY_RESTORE_LOCATION = "Key_Restore_Location";

    public static boolean isTfCardExist()
    {
        StatFs stat = new StatFs(strSDCardPath);
        if (stat.getBlockCount() > 0)
            return true;
        else
            return false;
    }

    public static boolean isTfCardFull() throws IllegalArgumentException
    {
        long available = getTfAvailableSpace();
        if (available < 1 * 1024 * 1024) {
            return true;
        }
        return false;
    }

    // Get available space of TF Card.
    private static long getTfAvailableSpace() throws IllegalArgumentException
    {
        StatFs stat = new StatFs(strSDCardPath);
        long remaining = (long) stat.getAvailableBlocks() * (long) stat.getBlockSize();
        return remaining;
    }

    /**
     * Return the sim name of subscription.
     */
    public static String getMultiSimName(Context context, int subscription) {
        if (subscription >= TelephonyManager.getDefault().getPhoneCount() || subscription < 0) {
            return null;
        }
        String multiSimName = Settings.System.getString(context.getContentResolver(),
                MULTI_SIM_NAME + (subscription + 1));
        if (multiSimName == null) {
            if (subscription == PhoneConstants.SUB1) {
                return context.getString(R.string.slot1);
            } else if (subscription == PhoneConstants.SUB2) {
                return context.getString(R.string.slot2);
            }
        }
        return multiSimName;
    }

    /**
     * Decide whether the current product  is DSDS in MMS
     */
    public static boolean isMultiSimEnabled() {
        return TelephonyManager.getDefault().isMultiSimEnabled();
    }

    public static int getBackUpMmsSmsSlot(Context context, String key) {
        SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(context);
        return prefs.getInt(key, SLOT_ALL);
    }

    public static void setBackUpMmsSmsSlot(Context context, String key, int slot) {
        SharedPreferences.Editor editPrefs =
            PreferenceManager.getDefaultSharedPreferences(context).edit();
        editPrefs.putInt(key, slot);
        editPrefs.apply();
    }

    public static String getSDPath() {
        IMountService ms;
        StorageVolume[] volumes;
        try {
            ms = IMountService.Stub.asInterface(ServiceManager.getService("mount"));
            volumes = ms.getVolumeList();
        } catch (Exception e) {
            return null;
        }
        for (int i = 0; i < volumes.length; i++) {
            if (volumes[i].isRemovable() && volumes[i].allowMassStorage()
                    && volumes[i].getPath().toUpperCase().contains("SDCARD")) {
                return volumes[i].getPath();
            }
        }
        return null;
    }

    public static boolean isInternalFull() throws IllegalArgumentException
    {
        long available = getInternalAvailableSpace();
        if (available < 1 * 1024 * 1024) {
            return true;
        }
        return false;
    }

    private static long getInternalAvailableSpace() throws IllegalArgumentException
    {
        StatFs stat = new StatFs(Environment.getExternalStorageDirectory().getAbsolutePath());
        long remaining = (long) stat.getAvailableBlocks() * (long) stat.getBlockSize();
        return remaining;
    }

    public static boolean isSDSupported() {
        return getSDPath() != null ? true : false;
    }

    public static boolean isSDMounted() {
        if (!isSDSupported()) return false;
        try {
            IMountService ms = IMountService.Stub.asInterface(ServiceManager.getService("mount"));
            return ms.getVolumeState(getSDPath()).equals(Environment.MEDIA_MOUNTED);
        } catch (Exception e) {
            return false;
        }
    }

}
