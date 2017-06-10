/*
 * Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qti.backupagent;

import android.app.backup.BackupAgentHelper;
import android.app.backup.BackupDataInput;
import android.app.backup.BackupDataOutput;
import android.app.backup.FileBackupHelper;
import android.app.backup.FullBackupDataOutput;
import android.content.SharedPreferences;
import android.os.ParcelFileDescriptor;
import android.util.Log;

import java.io.File;
import java.io.IOException;

public class QtiBackupAgent extends BackupAgentHelper {
    private final boolean DEBUG = false;
    static final private String TAG = "QtiBackupAgent";
    SharedPreferences settings;
    SharedPreferences.Editor editor;

    private void doBackup() {

        //backup contacts
        if (DEBUG) Log.d(TAG, "Starting contacts backup.");
        QtiBackupContacts contactsBackup = new QtiBackupContacts(this, QtiBackupActivity.CONTACTS_FILE);
        contactsBackup.backupContacts();
    }

    private void doRestore(File file) {
        //Restore contacts
        if (DEBUG) Log.d(TAG, "Starting contacts restore.");
        String contactsFile = QtiBackupActivity.CONTACTS_FILE;
        if(contactsFile.equals(file.getName())) {
            QtiBackupContacts contactsBackup = new QtiBackupContacts(this, file.getName());
            contactsBackup.restoreContacts(file);
        }
    }

    @Override
        public void onFullBackup(FullBackupDataOutput data) throws IOException {
            doBackup();
            super.onFullBackup(data);
        }

    @Override
        public void onRestoreFile(ParcelFileDescriptor data, long size,
                File destination, int type, long mode, long mtime)
        throws IOException {
            //Skip restoring user settings for backup app.
            String settingsFile = QtiBackupActivity.PREFS_NAME+".xml";
            if(settingsFile.equals(destination.getName()))
                return;

            super.onRestoreFile(data, size, destination, type, mode, mtime);
            doRestore(destination);
        }
}
