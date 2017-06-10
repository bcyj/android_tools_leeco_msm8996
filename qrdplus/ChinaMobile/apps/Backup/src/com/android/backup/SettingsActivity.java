/*
 * Copyright (c) 2015 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 * Not a Contribution, Apache license notifications and license are retained
 * for attribution purposes only.
 */

package com.android.backup;

import java.io.File;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.ContentResolver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.os.Environment;
import android.os.storage.StorageEventListener;
import android.os.storage.StorageManager;
import android.preference.ListPreference;
import android.preference.Preference;
import android.preference.PreferenceActivity;
import android.preference.PreferenceManager;
import android.preference.PreferenceScreen;
import android.preference.Preference.OnPreferenceClickListener;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.Toast;

import static com.android.backup.HelpActivity.*;
import static com.android.backup.BackupUtils.*;

public class SettingsActivity extends PreferenceActivity implements
        Preference.OnPreferenceChangeListener,
        Preference.OnPreferenceClickListener {

    private static String TAG = "SettingsActivity";
    private static final String KEY_BACKUP = "backup_location";
    private static final String KEY_RESTORE = "restore_location";
    private static final String KEY_HELP = "settings_help";
    private static final String BACKUP_TAG_PATH = "/backup/App/backuptag";
    private static final String RESTORE_TAG_PATH = "/backup/App/restoretag";

    /** If there is no setting in the provider, use this. */
    private static final int FALLBACK_BACKUP = 0;
    private static final int FALLBACK_RESTORE = 0;
    private static int currentBackup = -1;
    private static int currentRestore = -1;

    private static final int PATH_INTERNAL = 0;
    private static final int PATH_EXTERNAL = 1;

    private ListPreference mBackupPreference;
    private ListPreference mRestorePreference;
    private Preference mHelpPreference;

    private StorageManager mStorageManager;
    private StorageEventListener mStorageListener;
    private SharedPreferences sharedPreferences;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        final ContentResolver resolver = this.getContentResolver();
        addPreferencesFromResource(R.xml.settings_preference);
        sharedPreferences = getSharedPreferences(BackupUtils.LOCATION_PREFERENCE_NAME,
              Context.MODE_PRIVATE);

        mBackupPreference = (ListPreference) findPreference(KEY_BACKUP);
        currentBackup = sharedPreferences.getInt(BackupUtils.KEY_BACKUP_LOCATION,
                FALLBACK_BACKUP);
        mBackupPreference.setSummary((currentBackup == FALLBACK_BACKUP) ?
                R.string.internal_location_summary : R.string.external_location_summary);

        mRestorePreference = (ListPreference) findPreference(KEY_RESTORE);
        currentRestore = sharedPreferences.getInt(BackupUtils.KEY_RESTORE_LOCATION,
                FALLBACK_RESTORE);
        mRestorePreference.setSummary((currentRestore == FALLBACK_RESTORE) ?
                R.string.internal_location_summary : R.string.external_location_summary);

        mHelpPreference = (Preference) findPreference(KEY_HELP);

        mBackupPreference.setOnPreferenceClickListener(this);
        mBackupPreference.setOnPreferenceChangeListener(this);
        mRestorePreference.setOnPreferenceClickListener(this);
        mRestorePreference.setOnPreferenceChangeListener(this);
        mHelpPreference.setOnPreferenceClickListener(this);
        mHelpPreference.setOnPreferenceChangeListener(this);

        if(!BackupUtils.isSDMounted()) {
            mBackupPreference.setSummary(R.string.internal_location_summary);
            mRestorePreference.setSummary(R.string.internal_location_summary);
            mBackupPreference.setEnabled(false);
            mRestorePreference.setEnabled(false);
            deleteBackupTagFile();
            deleteRestoreTagFile();
        }

        mStorageListener = new StorageEventListener() {
            @Override
            public void onStorageStateChanged(String path, String oldState, String newState) {
                //update backup and restore status for the pathes in settings
                Log.d(TAG, "onStorageStateChanged(), path: " + path +
                        ", oldState: " + oldState + ", newState: " + newState);
                if (!(path != null && path.equals(BackupUtils.getSDPath()))) {
                    return;
                }
                if (newState.equals(Environment.MEDIA_MOUNTED)) {
                    mBackupPreference.setEnabled(true);
                    mRestorePreference.setEnabled(true);
                    if (sharedPreferences.getInt(BackupUtils.KEY_BACKUP_LOCATION,
                            FALLBACK_BACKUP) == PATH_EXTERNAL) {
                        mBackupPreference.setSummary(R.string.external_location_summary);
                        createBackupTagFile();
                    } else {
                        deleteBackupTagFile();
                    }
                    if (sharedPreferences.getInt(BackupUtils.KEY_RESTORE_LOCATION,
                            FALLBACK_RESTORE) == PATH_EXTERNAL) {
                        mRestorePreference.setSummary(R.string.external_location_summary);
                        createRestoreTagFile();
                    } else {
                        deleteRestoreTagFile();
                    }
                } else {
                    mBackupPreference.setSummary(R.string.internal_location_summary);
                    mRestorePreference.setSummary(R.string.internal_location_summary);
                    mBackupPreference.setEnabled(false);
                    mRestorePreference.setEnabled(false);
                    deleteBackupTagFile();
                    deleteRestoreTagFile();
                }
            }
        };

        mStorageManager = StorageManager.from(SettingsActivity.this);
        mStorageManager.registerListener(mStorageListener);
    }

    @Override
    public void onDestroy() {
        if (mStorageManager != null && mStorageListener != null) {
            mStorageManager.unregisterListener(mStorageListener);
        }
        super.onDestroy();
    }

    @Override
    public boolean onPreferenceClick(Preference preference) {
        if (preference.getKey().equals(KEY_BACKUP)) {
            return true;
        } else if (preference.getKey().equals(KEY_RESTORE)) {
            return true;
        } else if (preference.getKey().equals(KEY_HELP)) {
            Intent intent = new Intent();
            intent.setClass(SettingsActivity.this, HelpActivity.class);
            intent.putExtra(HELP_TYPE_KEY, TYPE_SETTING);
            startActivity(intent);
            return true;
        }
        return false;
    }

    @Override
    public boolean onPreferenceChange(Preference preference, Object objValue) {
        if (preference.getKey().equals(KEY_BACKUP)) {
            int value = ((ListPreference)preference).findIndexOfValue((String) objValue);
            if (value != currentBackup) {
                Log.d(TAG, "update backup reference from "+ currentBackup + " to " + value);
                try {
                    SharedPreferences.Editor editor = sharedPreferences.edit();
                    editor.putInt(BackupUtils.KEY_BACKUP_LOCATION, value);
                    editor.commit();
                } catch (NumberFormatException e) {
                    Log.e(TAG, "could not persist backup reference setting", e);
                }
                currentBackup = value;
                mBackupPreference.setSummary(objValue.toString());
                Log.d(TAG, "onPreferenceChange backup:" +
                sharedPreferences.getInt(BackupUtils.KEY_BACKUP_LOCATION, -1));
                if (currentBackup == PATH_INTERNAL) {
                    deleteBackupTagFile();
                } else {
                    createBackupTagFile();
                }
            }
            return true;
        } else if (preference.getKey().equals(KEY_RESTORE)) {
            int value = ((ListPreference)preference).findIndexOfValue((String) objValue);
            if (value != currentRestore) {
                Log.d(TAG, "update restore reference from "+ currentRestore + " to " + value);
                try {
                    SharedPreferences.Editor editor = sharedPreferences.edit();
                    editor.putInt(BackupUtils.KEY_RESTORE_LOCATION, value);
                    editor.commit();
                } catch (NumberFormatException e) {
                    Log.e(TAG, "could not persist restore reference setting", e);
                }
                currentRestore = value;
                mRestorePreference.setSummary(objValue.toString());
                Log.d(TAG, "onPreferenceChange restore:" +
                sharedPreferences.getInt(BackupUtils.KEY_RESTORE_LOCATION, -1));
                if (currentRestore == PATH_INTERNAL) {
                    deleteRestoreTagFile();
                } else {
                    createRestoreTagFile();
                }
            }
            return true;
        } else if (preference.getKey().equals(KEY_HELP)) {
            return true;
        }
        return false;
    }

    public static void createBackupTagFile() {
        String dirString = System.getenv("SECONDARY_STORAGE") + BACKUP_TAG_PATH;
        File path = new File(dirString);
        if (!path.exists()) {
            try {
                path.createNewFile();
            } catch (Exception e) {
                // TODO: handle exception
                Log.d(TAG, e.getMessage());
            }
        }
    }

    public static void deleteBackupTagFile() {
        String dirString = System.getenv("SECONDARY_STORAGE") + BACKUP_TAG_PATH;
        File path = new File(dirString);
        if (path.exists()) {
           path.delete();
        }
    }

    public static void createRestoreTagFile() {
        String dirString = System.getenv("SECONDARY_STORAGE") + RESTORE_TAG_PATH;
        File path = new File(dirString);
        if (!path.exists()) {
            try {
                path.createNewFile();
            } catch (Exception e) {
                // TODO: handle exception
                Log.d(TAG, e.getMessage());
            }
        }
    }

    public static void deleteRestoreTagFile() {
        String dirString = System.getenv("SECONDARY_STORAGE") + RESTORE_TAG_PATH;
        File path = new File(dirString);
        if (path.exists()) {
           path.delete();
        }
    }

}
