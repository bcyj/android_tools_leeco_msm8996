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

import com.android.backup.R;
import android.preference.CheckBoxPreference;
import android.preference.Preference;
import android.preference.PreferenceActivity;
import android.preference.PreferenceScreen;
import android.app.Activity;
import android.app.ActivityManager;
import android.app.TabActivity;
import android.os.Bundle;
import android.view.Window;
import android.app.AlertDialog;
import android.content.Context;
import android.widget.TabHost;
import android.widget.Toast;
import android.view.Menu;
import android.view.MenuItem;
import android.app.ProgressDialog;
import android.os.Handler;
import android.os.Message;
import android.os.SystemService;
//import android.os.Process;
import android.net.Uri;
import android.content.ContentUris;
import android.text.format.DateFormat;
import java.util.List;
import java.util.ArrayList;
import java.util.Arrays;
import java.io.File;
import java.io.FileWriter;
import android.os.FileUtils;
import java.io.IOException;
import java.io.FileOutputStream;
import java.io.OutputStream;
import java.io.InputStream;
import java.io.FileNotFoundException;
import java.io.FileInputStream;
import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.BufferedReader;
import java.io.FileReader;
import java.io.ByteArrayInputStream;

import android.view.KeyEvent;
import android.content.Intent;
import android.content.DialogInterface;
import android.content.DialogInterface.OnCancelListener;
import android.content.DialogInterface.OnClickListener;

import android.content.ContentResolver;
import android.database.Cursor;

import com.android.backup.vcalendar.VcalendarImporter;
import com.android.backup.vcalendar.VcalendarImporter.OnVcsInsertListener;

import static com.android.backup.BackupUtils.BACKUP_TYPE_ALL;
import static com.android.backup.BackupUtils.BACKUP_TYPE_APP;
import static com.android.backup.BackupUtils.BACKUP_TYPE_CALENDAR;
import static com.android.backup.BackupUtils.BACKUP_TYPE_CONTACTS;
import static com.android.backup.BackupUtils.BACKUP_TYPE_EMAIL;
import static com.android.backup.BackupUtils.BACKUP_TYPE_MMS;
import static com.android.backup.BackupUtils.BACKUP_TYPE_PICTURE;
import static com.android.backup.BackupUtils.BACKUP_TYPE_SMS;
import static com.android.backup.BackupUtils.EVENT_FILE_CREATE_ERR;
import static com.android.backup.BackupUtils.EVENT_INIT_PROGRESS_TITLE;
import static com.android.backup.BackupUtils.EVENT_RESTORE_RESULT;
import static com.android.backup.BackupUtils.EVENT_SDCARD_NOT_MOUNTED;
import static com.android.backup.BackupUtils.EVENT_SET_PROGRESS_VALUE;
import static com.android.backup.BackupUtils.EVENT_STOP_BACKUP;
import static com.android.backup.vcalendar.VcalendarImporter.CALENDAR_MEMO_FULL;
import static com.android.backup.vcalendar.VcalendarImporter.CALENDAR_APPOINTMENT_FULL;
import static com.android.backup.vcalendar.VcalendarImporter.CALENDAR_ANNIVERSARY_FULL;
import static com.android.backup.vcalendar.VcalendarImporter.CALENDAR_TASK_FULL;

//import android.pim.vcard.VCardConfig;
//import android.pim.vcard.VCardEntryCommitter;
//import android.pim.vcard.VCardEntryConstructor;
//import android.pim.vcard.VCardEntryCounter;
//import android.pim.vcard.VCardInterpreter;
//import android.pim.vcard.VCardInterpreterCollection;
//import android.pim.vcard.VCardParser_V21;
//import android.pim.vcard.VCardParser_V30;
//import android.pim.vcard.VCardSourceDetector;
//import android.pim.vcard.exception.VCardException;
//import android.pim.vcard.exception.VCardNestedException;
//import android.pim.vcard.exception.VCardNotSupportedException;
//import android.pim.vcard.exception.VCardVersionException;
//import android.accounts.Account;

import com.android.backup.vcard.VCardConfig;
import com.android.backup.vcard.VCardEntryCommitter;
import com.android.backup.vcard.VCardEntryConstructor;
import com.android.backup.vcard.VCardEntryCounter;
import com.android.backup.vcard.VCardInterpreter;
import com.android.backup.vcard.VCardInterpreterCollection;
import com.android.backup.vcard.VCardParser;
import com.android.backup.vcard.VCardParser_V21;
import com.android.backup.vcard.VCardParser_V30;
//import android.pim.vcard.VCardParserImpl_V21;
//import android.pim.vcard.VCardParserImpl_V30;
import com.android.backup.vcard.VCardSourceDetector;
import com.android.backup.vcard.exception.VCardException;
import com.android.backup.vcard.exception.VCardNestedException;
import com.android.backup.vcard.exception.VCardNotSupportedException;
import com.android.backup.vcard.exception.VCardVersionException;
import android.accounts.Account;

import android.provider.BaseColumns;
import android.os.Looper;

import android.content.pm.ApplicationInfo;
import android.content.pm.PackageInfo;
import java.util.ArrayList;
import java.util.List;
import android.util.Log;
import com.android.backup.ContactsContract;
import android.os.SystemProperties;
import android.os.PowerManager;
//zip
import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.InputStream;
import java.util.Enumeration;
import java.util.zip.ZipEntry;
import java.util.zip.ZipFile;
import java.util.zip.ZipOutputStream;
import org.xmlpull.v1.XmlPullParser;
import org.xmlpull.v1.XmlPullParserException;
import android.util.Xml;
//zip END
import android.provider.Settings;
import java.lang.Process;

public class OldRecoverOperateActivity extends PreferenceActivity implements OnVcsInsertListener {

    private static final String TAG = "OldRecoverOperateActivity";
    private static final boolean LOCAL_DEBUG = true;
    public static final int VCARD_TYPE = VCardConfig.VCARD_TYPE_DEFAULT;
    public static final String CURRENT_DIR_STR = "android.com.backup.directory";

    private static final String DEFAULT_BACKUP_ROOT_DIR = BackupUtils.strSDCardPath
            + "/.backup/";
    private static final String DEFAULT_BACKUP_CONTACTS_DIR = "/contacts/";
    private static final String DEFAULT_BACKUP_MMS_SMS_DIR = "/mms/";
    private static final String DEFAULT_BACKUP_EMAIL_DIR = "/email/";
    private static final String DEFAULT_BACKUP_CALENDAR_DIR = "/calendar/";
    private static final String DEFAULT_BACKUP_EMAIL_DIR_FILE = BackupUtils.strSDCardPath
            + "/.backup/restorepath";
    private static final String DEFAULT_BACKUP_APP_DIR_FILE = BackupUtils.strSDCardPath
            + "/.backup/restoreapppath.txt";
    private static final String DEFAULT_BACKUP_PICTURE_DIR = "/image/";
    private static final String DEFAULT_BACKUP_APP_DIR = "/apps";
    private String mCurrentDir;
    private String myCurrentDir;

    private static final String KEY_BACKUP_CONTACTS = "bak_contact";
    private static final String KEY_BACKUP_SMS = "bak_sms";
    private static final String KEY_BACKUP_MMS = "bak_mms";
    private static final String KEY_BACKUP_EMAIL = "bak_email";
    private static final String KEY_BACKUP_CALENDAR = "bak_calendar";
    private static final String KEY_BACKUP_PICTURE = "bak_picture";
    private static final String KEY_BACKUP_APPLICATION = "bak_application";

    private static final String ITEM_RECORD = "record";
    private static final String ITEM_CATEGORY = "category";
    private static final String PACKAGENAME = "packagename";
    private static final String APPNAME = "label";
    private static final String APKNAME = "name";

    private static final int MENU_MULTISEL_OK = 1;
    private static final int MENU_MULTISEL_ALL = 2;
    private static final int MENU_MULTISEL_CANCEL = 3;

    private boolean mCancelBackupFlag = false;

    private static Uri MMS_URI = Uri.parse("content://mms-sms/mailbox/");
    private static final int OUT_PUT_BUFFER_SIZE = 1024 * 4;

    private CheckBoxPreference[] mBackupSource = new CheckBoxPreference[4];

    private Showconnectprogress mSendProgressDialog = null;
    private OperateThread mOperateThread = null;
    private Handler mHandler;
    private ContentResolver mResolver;
    private VCardParser mVCardParser;

    private VcalendarImporter mVcalImporter = null;

    private BackupMmsOperation mBackupSmsOper = null;
    private BackupMmsOperation mBackupMmsOper = null;
    // zip
    private static byte[] buf = new byte[1024];
    // zip END
    // private native static int restore_native(byte[] dir);
    private ApplicationCategory mAppList = null;

    private boolean[] isBackup = new boolean[6];
    private static PowerManager.WakeLock sWakeLock;
    private static final Object mWakeLockSync = new Object();

    private class BackupHandler extends Handler {

        public void handleMessage(Message msg) {
            switch (msg.what) {
                case EVENT_STOP_BACKUP: {
                    Log.d(TAG, "EVENT_STOP_BACKUP");
                    break;
                }
                case EVENT_FILE_CREATE_ERR: {
                    Log.d(TAG, "EVENT_FILE_CREATE_ERR");
                    break;
                }

                case EVENT_SDCARD_NOT_MOUNTED: {
                    Log.d(TAG, "EVENT_SDCARD_NOT_MOUNTED");
                    if (mSendProgressDialog != null) {
                        mSendProgressDialog.dismiss();
                        mSendProgressDialog = null;
                    }
                    Toast.makeText(OldRecoverOperateActivity.this, R.string.sdcard_unmounted,
                            Toast.LENGTH_SHORT).show();
                    finish();
                    break;
                }
                case EVENT_INIT_PROGRESS_TITLE: {
                    Log.d(TAG, "EVENT_INIT_PROGRESS_TITLE");
                    int type = msg.arg1;
                    String title = null;
                    switch (type) {
                        case BACKUP_TYPE_CONTACTS:
                            title = getString(R.string.restore_contact_title);
                            break;
                        case BACKUP_TYPE_MMS:
                            title = getString(R.string.restore_mms_title);
                            break;
                        case BACKUP_TYPE_SMS:
                            title = getString(R.string.restore_sms_title);
                            break;
                        case BACKUP_TYPE_EMAIL:
                            title = getString(R.string.restore_email_title);
                            break;
                        case BACKUP_TYPE_CALENDAR:
                            title = getString(R.string.restore_calendar_title);
                            break;
                        case BACKUP_TYPE_PICTURE:
                            title = getString(R.string.restore_picture_title);
                            break;
                        case BACKUP_TYPE_APP:
                            title = getString(R.string.restore_app_title);
                            break;
                    }
                    updateProgressDialog(title, msg.arg2);
                    break;
                }
                case EVENT_SET_PROGRESS_VALUE: {
                    Log.d(TAG, "EVENT_SET_PROGRESS_VALUE msg.arg1:" + msg.arg1);
                    updateProgressValue(msg.arg1);
                    break;
                }
                case EVENT_RESTORE_RESULT: {
                    Log.d(TAG, "EVENT_RESTORE_RESULT msg.arg1:" + msg.arg1 + " msg.arg: "
                            + msg.arg2);
                    int result = msg.arg2;
                    String title = null;
                    String strResult = getString(result == 1 ? R.string.result_ok
                            : R.string.result_failed);
                    int type = msg.arg1;
                    switch (type) {
                        case BACKUP_TYPE_CONTACTS:
                            title = getString(R.string.show_restore_result_info,
                                    getString(R.string.backup_contact), strResult);
                            break;
                        case BACKUP_TYPE_MMS:
                            title = getString(R.string.show_restore_result_info,
                                    getString(R.string.backup_mms), strResult);
                            break;
                        case BACKUP_TYPE_SMS:
                            title = getString(R.string.show_restore_result_info,
                                    getString(R.string.backup_sms), strResult);
                            break;
                        case BACKUP_TYPE_EMAIL:
                            title = getString(R.string.show_restore_result_info,
                                    getString(R.string.backup_email), strResult);
                            break;
                        case BACKUP_TYPE_CALENDAR:
                            title = getString(R.string.show_restore_result_info,
                                    getString(R.string.backup_calendar), strResult);
                            Log.d(TAG, "R.string.backup_calendar = " + R.string.backup_calendar);
                            break;
                        case BACKUP_TYPE_PICTURE:
                            title = getString(R.string.show_restore_result_info,
                                    getString(R.string.backup_picture), strResult);
                            break;
                        case BACKUP_TYPE_ALL:
                            title = getString(R.string.result_ok);
                            break;
                    }
                    Log.d(TAG, "EVENT_RESTORE_RESULT title: " + title);
                    Toast.makeText(OldRecoverOperateActivity.this, title, Toast.LENGTH_SHORT)
                            .show();
                    break;
                }
                default:
                    Log.i(TAG, "unknow message: " + msg.what);
                    return;

            }
        }
    };

    private class OperateThread extends Thread {

        public OperateThread() {
            super("OperateThread");
        }

        public void run() {
            Looper.prepare();
            String[] myCurrentDirArr = mCurrentDir.split(".zip");
            myCurrentDir = myCurrentDirArr[0];
            if (!unZip(("/mnt" + DEFAULT_BACKUP_ROOT_DIR + mCurrentDir),
                    (DEFAULT_BACKUP_ROOT_DIR + myCurrentDir))) {
                Log.e(TAG, "error when unZip");
            }
            Log.d(TAG, " run:mSendProgressDialog--1"
                    + ((mSendProgressDialog != null) ? "not null" : "null"));
            // restore contacts
            if (mBackupSource[0].isChecked() && !mCancelBackupFlag) {
                String contact_dir_file = DEFAULT_BACKUP_ROOT_DIR + myCurrentDir
                        + DEFAULT_BACKUP_CONTACTS_DIR;
                Log.d(TAG, "contact_dir_file = " + contact_dir_file);
                File contacts_dir = new File(contact_dir_file);
                if (contacts_dir.exists()) {
                    File[] fileLists = contacts_dir.listFiles();
                    int num = fileLists.length;
                    for (int i = 0; i < num && (!mCancelBackupFlag); i++) {
                        if (fileLists[i].exists()) {
                            restoreContacts(fileLists[i].toString());
                        } else {
                            Log.d(TAG,
                                    "OperateThread: contacts_dir is not exist: "
                                            + contacts_dir.toString());
                            continue;
                        }
                    }
                } else {
                    Log.d(TAG, "contacts_dir dont exist");
                    mHandler.sendMessage(mHandler.obtainMessage(EVENT_RESTORE_RESULT,
                            BACKUP_TYPE_CONTACTS, 0));
                }
            }
            Log.d(TAG, " run:mSendProgressDialog--2"
                    + ((mSendProgressDialog != null) ? "not null" : "null"));

            // restore MMS
            if (mBackupSource[1].isChecked() && !mCancelBackupFlag) {
                String filename = DEFAULT_BACKUP_ROOT_DIR + myCurrentDir
                        + DEFAULT_BACKUP_MMS_SMS_DIR;
                File mms_dir = new File(filename);
                File[] mmsLists = mms_dir.listFiles();// mms-resume-judge
                if (mms_dir.exists() && (!(mmsLists.length == 1 && mmsLists[0].isDirectory()))) {
                    restoreMMS(filename);
                } else {
                    Log.d(TAG, "OperateThread: mms_dir is not exist: " + mms_dir);
                    mHandler.sendMessage(mHandler.obtainMessage(EVENT_RESTORE_RESULT,
                            BACKUP_TYPE_MMS, 0));
                }
            }

            // restore SMS
            if (mBackupSource[2].isChecked() && !mCancelBackupFlag) {
                String filename = DEFAULT_BACKUP_ROOT_DIR + myCurrentDir + "/sms/";
                File sms_dir = new File(filename);
                if (sms_dir.exists()) {
                    File[] sms_file = sms_dir.listFiles();
                    if (sms_dir.exists() && sms_file.length != 0) {
                        restoreSMS(filename);
                    } else {
                        Log.d(TAG, "OperateThread: sms_dir is not exist:" + sms_dir);
                        mHandler.sendMessage(mHandler.obtainMessage(EVENT_RESTORE_RESULT,
                                BACKUP_TYPE_SMS, 0));
                    }
                } else {
                    Log.d(TAG, "sms_dir not exists");
                    mHandler.sendMessage(mHandler.obtainMessage(EVENT_RESTORE_RESULT,
                            BACKUP_TYPE_SMS, 0));
                }
            }

            // restore calendar
            if (mBackupSource[3].isChecked() && !mCancelBackupFlag) {
                File calendar_dir = new File(DEFAULT_BACKUP_ROOT_DIR + myCurrentDir
                        + DEFAULT_BACKUP_CALENDAR_DIR);
                if (calendar_dir.exists()) {
                    restoreCalendar(calendar_dir);
                } else {
                    Log.d(TAG, "OperateThread: calendar_dir is not exist:" + calendar_dir);
                    mHandler.sendMessage(mHandler.obtainMessage(EVENT_RESTORE_RESULT,
                            BACKUP_TYPE_CALENDAR, 0));
                }
            }

            // restore app
            String app_filename = DEFAULT_BACKUP_ROOT_DIR + myCurrentDir + DEFAULT_BACKUP_APP_DIR;
            boolean isRecoverApp = false;
            int firstrestoreflag = 0;
            for (int i = 0; i < mAppList.size(); i++) {
                ApplicationPreference appPreference = mAppList.get(i);
                if (appPreference.isChecked() && !mCancelBackupFlag) {
                    mHandler.sendMessage(mHandler.obtainMessage(EVENT_INIT_PROGRESS_TITLE,
                            BACKUP_TYPE_APP, 0));
                    Log.d(TAG, "Backup App start: i: " + i);
                    AppInfo appin = appPreference.getCachedAppInfo();
                    appin.print();
                    Log.d(TAG, "appin.sourcedir = " + appin.sourcedir);
                    String[] temp = appin.sourcedir.split("/");
                    String apkName = temp[temp.length - 1];
                    try {
                        // tar -xf com.sina.weibo.tar -C /data/data data/data
                        Log.d(TAG, "install start");
                        String[] command = new String[] {
                                "pm",
                                "install",
                                "/mnt" + DEFAULT_BACKUP_ROOT_DIR + "/" + myCurrentDir
                                        + DEFAULT_BACKUP_APP_DIR + "/" + apkName
                        };
                        for (String str : command) {
                            Log.d(TAG, "cm =" + str);
                        }
                        Process process = Runtime.getRuntime().exec(command);
                        process.waitFor();
                    } catch (Exception e) {
                        Log.d(TAG, "exception = " + e);
                    }

                    isRecoverApp = true;
                    firstrestoreflag++;

                }
            }
            if (mSendProgressDialog != null) {
                Log.d(TAG, " dialog dismiss");
                mSendProgressDialog.dismiss();
                mSendProgressDialog = null;
            }

            mHandler.sendMessage(mHandler.obtainMessage(EVENT_RESTORE_RESULT, BACKUP_TYPE_ALL, 1));
            Looper.loop();
        }
    }

    // zip
    private boolean unZip(String unZipfileName, String rootDir) {
        int c;
        FileOutputStream fileOut;
        File file;
        InputStream inputStream;
        ZipFile zipFile;
        Log.d(TAG, " unZipfileName = " + unZipfileName);

        try {
            zipFile = new ZipFile(unZipfileName);

            //
            for (Enumeration<? extends ZipEntry> entries = zipFile.entries(); entries
                    .hasMoreElements();) {
                ZipEntry entry = (ZipEntry) entries.nextElement();
                Log.d(TAG, "entry.getName() = " + entry.getName());
                file = new File(rootDir + "/" + entry.getName());//

                if (entry.isDirectory()) {
                    file.mkdirs();
                } else {
                    // makeDir
                    File parent = file.getParentFile();
                    if (!parent.exists()) {
                        parent.mkdirs();
                    }

                    inputStream = zipFile.getInputStream(entry);

                    fileOut = new FileOutputStream(file);
                    while ((c = inputStream.read(buf)) != -1) {
                        fileOut.write(buf, 0, c);
                    }
                    fileOut.close();

                    inputStream.close();
                }
            }
            zipFile.close();
        } catch (IOException ioe) {
            ioe.printStackTrace();
            return false;
        }
        return true;
    }

    private boolean unZipSpecifiedFile(String unZipfileName, String filename, String rootDir) {
        int c;
        FileOutputStream fileOut;
        File file;
        InputStream inputStream;
        ZipFile zipFile;
        Log.d(TAG, " unZipfileName = " + unZipfileName);

        try {
            zipFile = new ZipFile(unZipfileName);

            //
            for (Enumeration<? extends ZipEntry> entries = zipFile.entries(); entries
                    .hasMoreElements();) {
                ZipEntry entry = (ZipEntry) entries.nextElement();
                Log.d(TAG, "entry.getName() = " + entry.getName());
                // String tmpName =
                // entry.getName().subString(entry.getName().lastIndexOf("/"));
                if (entry.getName().endsWith(filename)) {
                    file = new File(rootDir + "/" + entry.getName());//

                    if (entry.isDirectory()) {
                        file.mkdirs();
                    } else {
                        // makeDir
                        File parent = file.getParentFile();
                        if (!parent.exists()) {
                            parent.mkdirs();
                        }

                        inputStream = zipFile.getInputStream(entry);

                        fileOut = new FileOutputStream(file);
                        while ((c = inputStream.read(buf)) != -1) {
                            fileOut.write(buf, 0, c);
                        }
                        fileOut.close();

                        inputStream.close();
                    }
                    break;
                }
            }
            zipFile.close();
        } catch (IOException ioe) {
            ioe.printStackTrace();
            return false;
        }
        return true;
    }

    //
    private boolean unZipFileDir(String unZipfileName) {
        int c;
        FileOutputStream fileOut;
        File file;
        InputStream inputStream;
        ZipFile zipFile;
        Log.d(TAG, " unZipfileName = " + unZipfileName);

        try {
            zipFile = new ZipFile(unZipfileName);

            //
            for (Enumeration<? extends ZipEntry> entries = zipFile.entries(); entries
                    .hasMoreElements();) {
                ZipEntry entry = (ZipEntry) entries.nextElement();
                Log.d(TAG, "entry.getName() = " + entry.getName());
                if (entry.getName().contains("contacts")) {
                    isBackup[0] = true;
                }
                if (entry.getName().contains("mms")) {
                    isBackup[1] = true;
                }
                if (entry.getName().contains("sms")) {
                    isBackup[2] = true;
                }
                // if (entry.getName().contains("email")){
                // isBackup[3]=true;
                // }
                if (entry.getName().contains("calendar")) {
                    isBackup[3] = true;
                }
                if (entry.getName().contains("image")) {
                    isBackup[4] = true;
                }
                if (entry.getName().contains("apps")) {
                    isBackup[5] = true;
                }

            }
            zipFile.close();
        } catch (IOException ioe) {
            ioe.printStackTrace();
            finish();
            return false;
        } finally {

        }
        return true;
    }

    //
    private void deleteDir(String str) {
        File dir;
        try {
            dir = new File(str);
        } catch (Exception e) {
            return;
        }
        // if (dir == null || !dir.exists() || !dir.isDirectory()) {
        if (dir == null || !dir.exists()) {
            return;
        }
        if (!dir.isDirectory()) {
            dir.delete();
            return;
        }
        for (File files : dir.listFiles()) {
            if (files.isFile()) {
                files.delete();
            } else if (files.isDirectory()) {
                deleteDir(files.toString());
            }
        }
        dir.delete();
        Log.d(TAG, "dirToBeDeleted" + dir.getAbsolutePath());
    }

    // zip END
    private boolean restoreContacts(String filename) {
        if (true)
            Log.d(TAG, " restoreContacts:mSendProgressDialog"
                    + ((mSendProgressDialog != null) ? "not null" : "null"));
        final Uri uri = Uri.parse("file://" + filename);
        Log.v(TAG, "restoreContacts path = " + uri);

        // Update the progress bar title
        mHandler.sendMessage(mHandler.obtainMessage(EVENT_INIT_PROGRESS_TITLE,
                BACKUP_TYPE_CONTACTS, 0));

        mResolver = OldRecoverOperateActivity.this.getContentResolver();

        VCardEntryCounter counter = new VCardEntryCounter();
        VCardSourceDetector detector = new VCardSourceDetector();
        VCardInterpreterCollection builderCollection = new VCardInterpreterCollection(
                Arrays.asList(counter, detector));
        boolean result;
        try {
            result = readOneVCardFile(uri,
                    VCardConfig.DEFAULT_CHARSET, builderCollection, null, true, null);
        } catch (VCardNestedException e) {
            try {
                // Assume that VCardSourceDetector was able to detect the
                // source.
                // Try again with the detector.
                result = readOneVCardFile(uri,
                        VCardConfig.DEFAULT_CHARSET, counter, detector, false, null);
            } catch (VCardNestedException e2) {
                result = false;
                Log.e(TAG, "Must not reach here. " + e2);
            }
        }
        Log.d(TAG, "counter = " + counter.getCount());
        String charset = detector.getEstimatedCharset();
        doActuallyReadOneVCard(uri, null,
                charset, false, detector, null);
        return result;
    }

    private Uri doActuallyReadOneVCard(Uri uri, Account account,
            String charset, boolean showEntryParseProgress,
            VCardSourceDetector detector, List<String> errorFileNameList) {
        if (true)
            Log.d(TAG, " doActuallyReadOneVCard:mSendProgressDialog"
                    + ((mSendProgressDialog != null) ? "not null" : "null"));
        final Context context = OldRecoverOperateActivity.this;
        mResolver = context.getContentResolver();
        VCardEntryConstructor builder;

        int vcardType = VCARD_TYPE;
        if (charset != null) {
            builder = new VCardEntryConstructor(charset, charset, false, vcardType, null);
        } else {
            charset = VCardConfig.DEFAULT_CHARSET;
            builder = new VCardEntryConstructor(null, null, false, vcardType, null);
        }

        VCardEntryCommitter committer = new VCardEntryCommitter(mResolver);
        builder.addEntryHandler(committer);

        try {
            if (!readOneVCardFile(uri, charset, builder, detector, false, null)) {
                return null;
            }
        } catch (VCardNestedException e) {
            Log.e(TAG, "Never reach here.");
        }
        final ArrayList<Uri> createdUris = committer.getCreatedUris();
        return (createdUris == null || createdUris.size() != 1) ? null : createdUris.get(0);
    }

    private boolean readOneVCardFile(Uri uri, String charset,
            VCardInterpreter builder, VCardSourceDetector detector,
            boolean throwNestedException, List<String> errorFileNameList)
            throws VCardNestedException {
        if (true)
            Log.d(TAG, " readOneVCardFile:mSendProgressDialog"
                    + ((mSendProgressDialog != null) ? "not null" : "null"));
        InputStream is;
        try {
            is = mResolver.openInputStream(uri);
            mVCardParser = new VCardParser_V21(detector);

            try {
                ContactsContract.enableTransactionLock(getPackageName(), getContentResolver());
                Log.d(TAG, "readOneVCardFile cancel flag: " + mCancelBackupFlag);
                mVCardParser.parse(is, charset, builder, mCancelBackupFlag);
            } catch (VCardVersionException e1) {
                try {
                    is.close();
                } catch (IOException e) {
                }
                if (builder instanceof VCardEntryConstructor) {
                    // Let the object clean up internal temporal objects,
                    ((VCardEntryConstructor) builder).clear();
                }
                is = mResolver.openInputStream(uri);

                try {
                    mVCardParser = new VCardParser_V30();
                    mVCardParser.parse(is, charset, builder, mCancelBackupFlag);
                } catch (VCardVersionException e2) {
                    throw new VCardException("vCard with unspported version.");
                }
            } finally {
                ContactsContract.disableTransactionLock(getPackageName(), getContentResolver());
                if (is != null) {
                    try {
                        is.close();
                    } catch (IOException e) {
                    }
                }
            }
        } catch (IOException e) {
            Log.e(TAG, "IOException was emitted: " + e.getMessage());

            // mProgressDialogForReadVCard.dismiss();
            //
            // if (errorFileNameList != null) {
            // errorFileNameList.add(uri.toString());
            // } else {
            // runOnUIThread(new DialogDisplayer(
            // getString(R.string.fail_reason_io_error) +
            // ": " + e.getLocalizedMessage()));
            // }
            return false;
        } catch (VCardNotSupportedException e) {
            Log.e(TAG, "VCardNotSupportedException was emitted: " + e.getMessage());
            // if ((e instanceof VCardNestedException) && throwNestedException)
            // {
            // throw (VCardNestedException)e;
            // }
            // if (errorFileNameList != null) {
            // errorFileNameList.add(uri.toString());
            // } else {
            // runOnUIThread(new DialogDisplayer(
            // getString(R.string.fail_reason_vcard_not_supported_error) +
            // " (" + e.getMessage() + ")"));
            // }
            return false;
        } catch (VCardException e) {
            Log.e(TAG, "VCardException was emitted: " + e.getMessage());
            // if (errorFileNameList != null) {
            // errorFileNameList.add(uri.toString());
            // } else {
            // runOnUIThread(new DialogDisplayer(
            // getString(R.string.fail_reason_vcard_parse_error) +
            // " (" + e.getMessage() + ")"));
            // }
            return false;
        }
        return true;
    }

    private boolean restoreApp(String appdir, boolean flag) {
        Log.d(TAG, "appdir = " + appdir);
        FileOutputStream fout = null;
        try {
            File file = new File(DEFAULT_BACKUP_APP_DIR_FILE);
            if (flag) {
                if (file.exists()) {
                    Log.d(TAG, "restoreApp delete the last path file first");
                    file.delete();
                }
            }
            fout = new FileOutputStream(file, true);
            fout.write(appdir.getBytes());
            fout.write('\n');
        } catch (IOException e) {
            Log.d(TAG, "restoreApp IOException" + e);
            return false;
        } finally {
            if (null != fout) {
                try {
                    fout.flush();
                    fout.close();
                } catch (IOException e) {
                    return false;
                }
            }
        }
        return true;
    }

    private boolean runRestoreAppSh() {
        SystemProperties.set("persist.sys.shflag", "0");
        try {
            Thread.sleep(200);
        } catch (InterruptedException e) {
            Log.e(TAG, "InterruptedException :" + e);
        }
        boolean isAdbStart = false;
        isAdbStart = (Settings.Secure.getInt(getContentResolver(), Settings.Secure.ADB_ENABLED, 0) != 0);
        if (!isAdbStart) {
            SystemService.start("adbd");
        }
        try {
            Thread.sleep(500);
        } catch (InterruptedException e) {
            Log.e(TAG, "InterruptedException :" + e);
        }
        Log.d(TAG, "persist.sys.shflag: " + SystemProperties.get("persist.sys.shflag", "0"));
        SystemService.start("restoreAppData");
        boolean whileFlag = true;
        int num = 1;
        boolean isRestart = false;
        boolean isStop = true;
        while (whileFlag) {
            if (num >= 65 && isStop) {
                SystemService.stop("restoreAppData");
                isRestart = true;
                isStop = false;
            }
            try {
                Thread.sleep(500);
            } catch (InterruptedException e) {
                Log.e(TAG, "InterruptedException :" + e);
            }
            if (isRestart) {
                SystemService.start("restoreAppData");
                isRestart = false;
            }
            if ("1".equals(SystemProperties.get("persist.sys.shflag", "0"))) {
                whileFlag = false;
            }
            num++;
        }
        if (isAdbStart) {
            // do nothing
        } else {
            SystemService.stop("adbd");
        }
        Log.d(TAG, "persist.sys.shflag: " + SystemProperties.get("persist.sys.shflag", "0"));
        return true;
    }

    private boolean restoreApp(String appdir, String pid, String packagename) {
        Log.d(TAG, "restoreApp pid:" + pid);
        Log.d(TAG, "appdir = " + appdir + " packagename = " + packagename);
        FileOutputStream fout = null;
        try {
            File file = new File(DEFAULT_BACKUP_APP_DIR_FILE);
            if (file.exists()) {
                Log.d(TAG, "restoreApp delete the last path file first");
                file.delete();
            }
            fout = new FileOutputStream(file);
            fout.write(appdir.getBytes());
            fout.write('\n');
            fout.write(packagename.getBytes());
            fout.write('\n');
            fout.write(pid.getBytes());
            fout.write('\n');
        } catch (IOException e) {
            Log.d(TAG, "restoreApp IOException" + e);
            return false;
        } finally {
            if (null != fout) {
                try {
                    fout.flush();
                    fout.close();
                } catch (IOException e) {
                    return false;
                }
            }
        }
        SystemProperties.set("persist.sys.shflag", "0");
        try {
            Thread.sleep(200);
        } catch (InterruptedException e) {
            Log.e(TAG, "InterruptedException :" + e);
        }
        Log.d(TAG, "persist.sys.shflag: " + SystemProperties.get("persist.sys.shflag", "0"));
        SystemService.start("restoreAppData");
        boolean whileFlag = true;
        while (whileFlag) {
            try {
                Thread.sleep(500);
            } catch (InterruptedException e) {
                Log.e(TAG, "InterruptedException :" + e);
            }
            if ("1".equals(SystemProperties.get("persist.sys.shflag", "0"))) {
                whileFlag = false;
            }
        }
        Log.d(TAG, "persist.sys.shflag: " + SystemProperties.get("persist.sys.shflag", "0"));
        return true;
    }

    private int getAppProgressPid(String appinfo) {
        int pid = -1;

        ActivityManager _ActivityManager = (ActivityManager) this
                .getSystemService(Context.ACTIVITY_SERVICE);
        List<ActivityManager.RunningAppProcessInfo> list = _ActivityManager
                .getRunningAppProcesses();
        int size = list.size();
        Log.d(TAG, "killEmailProgress size: " + size);
        for (int i = 0; i < size; i++) {
            Log.d(TAG, "killEmailProgress appname: " + list.get(i).processName);
            Log.d(TAG, "killEmailProgress pid: " + list.get(i).pid);
            if (list.get(i).processName.equalsIgnoreCase(appinfo)) {
                Log.d(TAG, "kill Email progress");
                // Process.killProcess(list.get(i).pid);
                pid = list.get(i).pid;
                break;
            }
        }

        return pid;
    }

    private boolean restoreMMS(String dir) {
        Log.d(TAG, "restoreMMS() dir " + dir);
        mBackupSmsOper = new BackupMmsOperation(this, mHandler, dir);
        mHandler.sendMessage(mHandler.obtainMessage(EVENT_INIT_PROGRESS_TITLE, BACKUP_TYPE_MMS, 0));
        mBackupSmsOper.recoverMms(dir);
        return true;
    }

    private boolean restoreSMS(String dir) {
        mBackupMmsOper = new BackupMmsOperation(this, mHandler, null);
        mHandler.sendMessage(mHandler.obtainMessage(EVENT_INIT_PROGRESS_TITLE, BACKUP_TYPE_SMS, 0));
        mBackupMmsOper.recoverSmsMessagesWithXML(dir);
        return true;
    }

    private int getEmailProgressPid() {
        int pid = -1;

        ActivityManager _ActivityManager = (ActivityManager) this
                .getSystemService(Context.ACTIVITY_SERVICE);
        List<ActivityManager.RunningAppProcessInfo> list = _ActivityManager
                .getRunningAppProcesses();
        int size = list.size();
        Log.d(TAG, "killEmailProgress size: " + size);
        for (int i = 0; i < size; i++) {
            Log.d(TAG, "killEmailProgress appname: " + list.get(i).processName);
            Log.d(TAG, "killEmailProgress pid: " + list.get(i).pid);
            if (list.get(i).processName.equalsIgnoreCase("com.android.email")) {
                Log.d(TAG, "kill Email progress");
                // Process.killProcess(list.get(i).pid);
                pid = list.get(i).pid;
                break;
            }
        }

        return pid;
    }

    public void importProgressSet(int progress) {
        Log.d(TAG, "importProgressSet() progress " + progress);
    }

    private boolean restoreCalendar(File vcsfile) {
        mHandler.sendMessage(mHandler.obtainMessage(EVENT_INIT_PROGRESS_TITLE,
                BACKUP_TYPE_CALENDAR, 0));

        String myFileDir = vcsfile.toString();
        File[] vcsList = vcsfile.listFiles();
        int length = vcsList.length;
        int success = 1;
        for (int i = 0; i < length; i++) {
            mVcalImporter = new VcalendarImporter(OldRecoverOperateActivity.this,
                    OldRecoverOperateActivity.this);
            // [2] insert operation
            if (!vcsList[i].toString().endsWith("vcs")) {
                continue;
            }
            Log.d(TAG, "vcsList = " + vcsList[i]);
            success = mVcalImporter.insertFromFile(vcsList[i]);
            Log.d(TAG, "restoreCalendar _insertFromFile() return " + success);
        }

        if (success == 1) {
        }
        else if (success < 0) {
        }
        else if (success == 0) {
        } else if (success > 1) {
        }
        // [3] release
        mVcalImporter = null;
        return true;
    }

    private class Showconnectprogress extends ProgressDialog {
        private Context mContext;

        public Showconnectprogress(Context context) {
            this(context, com.android.internal.R.style.Theme_Dialog_Alert);
            mContext = context;
        }

        public Showconnectprogress(Context context, int theme) {
            super(context, theme);
            mContext = context;
        }

        @Override
        public boolean onKeyDown(int keyCode, KeyEvent event) {
            if (LOCAL_DEBUG)
                Log.v(TAG, "Showconnectprogress keyCode " + keyCode);
            if ((mSendProgressDialog != null) && (keyCode == KeyEvent.KEYCODE_BACK)) {

                // show dialog to confirm stop backup
                new AlertDialog.Builder(OldRecoverOperateActivity.this)
                        .setIcon(android.R.drawable.ic_dialog_alert)
                        .setTitle(R.string.confirm_stop_restore_title)
                        .setMessage(R.string.confirm_stop_restore)
                        .setNegativeButton(R.string.button_cancel, null)
                        .setPositiveButton(R.string.button_ok,
                                new DialogInterface.OnClickListener() {
                                    public void onClick(DialogInterface dialog, int whichButton)
                                    {
                                        mCancelBackupFlag = true;
                                        SystemService.stop("restoreAppData");
                                        // Stop restore vcalendar
                                        if (mVcalImporter != null) {
                                            mVcalImporter.cancel();
                                        }
                                        if (mVCardParser != null) {
                                            mVCardParser.cancel();
                                        }
                                        if (mSendProgressDialog != null) {
                                            mSendProgressDialog.dismiss();
                                            mSendProgressDialog = null;
                                        }
                                        Toast.makeText(OldRecoverOperateActivity.this,
                                                R.string.stop_restore_info, Toast.LENGTH_SHORT)
                                                .show();
                                        finish();
                                    }
                                })
                        .show();
            }
            if ((mSendProgressDialog != null) && (keyCode == KeyEvent.KEYCODE_SEARCH)) {
                return true;
            }
            return false;
        }
    }

    @Override
    protected void onCreate(Bundle icicle) {
        super.onCreate(icicle);

        addPreferencesFromResource(R.xml.system_software2);

        for (int i = 0; i < isBackup.length; i++) {
            isBackup[i] = false;
        }
        Intent intent = getIntent();
        mCurrentDir = intent.getStringExtra(CURRENT_DIR_STR);
        unZipFileDir("/mnt" + DEFAULT_BACKUP_ROOT_DIR + mCurrentDir);

        mBackupSource[0] = (CheckBoxPreference) findPreference(KEY_BACKUP_CONTACTS);
        mBackupSource[1] = (CheckBoxPreference) findPreference(KEY_BACKUP_MMS);
        mBackupSource[2] = (CheckBoxPreference) findPreference(KEY_BACKUP_SMS);
        mBackupSource[3] = (CheckBoxPreference) findPreference(KEY_BACKUP_CALENDAR);
        for (int i = 0; i < 4; i++) {
            mBackupSource[i].setChecked(isBackup[i]);
        }
        mAppList = (ApplicationCategory) findPreference(KEY_BACKUP_APPLICATION);
        Log.d(TAG, "onCreate mAppList size is: " + mAppList.size());
        createAppPreference();
        mHandler = new BackupHandler();
        mCancelBackupFlag = false;
        // remove the preference
        if (false) {
            getPreferenceScreen().removePreference(mAppList);
        }
    }

    protected void onResume() {
        super.onResume();
        synchronized (mWakeLockSync) {
            PowerManager pm = (PowerManager) getSystemService(Context.POWER_SERVICE);
            sWakeLock = pm.newWakeLock(PowerManager.FULL_WAKE_LOCK |
                    PowerManager.ACQUIRE_CAUSES_WAKEUP |
                    PowerManager.ON_AFTER_RELEASE,
                    TAG);
            sWakeLock.acquire();
        }
        // end
    }

    protected void onPause() {
        super.onPause();
        // mAppList.clearAppList();
        synchronized (mWakeLockSync) {
            if (sWakeLock != null) {
                sWakeLock.release();
            }
        }
    }

    @Override
    protected void onStop() {
        super.onStop();
        Log.d(TAG, "onStop: ");
    }

    protected void onDestroy() {
        super.onDestroy();
        Log.d(TAG, "onDestroy");
        Thread t = new Thread() {
            public void run() {
                Log.d(TAG, "deleteDir");
                deleteDir(DEFAULT_BACKUP_ROOT_DIR + myCurrentDir + "/");
            }
        };
        t.start();
    }

    public boolean onPrepareOptionsMenu(Menu menu)
    {

        menu.clear();

        // multi select ok
        menu.add(0, MENU_MULTISEL_OK, 0, R.string.menu_selectok)
                .setIcon(R.drawable.ic_menu_ok);

        return true;
    }

    public boolean onOptionsItemSelected(MenuItem item)
    {
        Log.d(TAG, "onOptionsItemSelected item" + item);

        switch (item.getItemId())
        {
            case MENU_MULTISEL_OK:
                Log.d(TAG, "onOptionsItemSelected MENU_MULTISEL_OK" + mBackupSource.length);
                if (isChecked()) {
                    // remind reboot
                    /*
                     * new AlertDialog.Builder(RecoverOperateActivity.this)
                     * .setIcon(android.R.drawable.ic_dialog_alert)
                     * .setTitle(R.string.confirm_stop_restore_title)
                     * .setMessage(R.string.recoverOrNot)
                     * .setNegativeButton(R.string.button_cancel, null)
                     * .setPositiveButton(R.string.button_ok, new
                     * DialogInterface.OnClickListener() { public void
                     * onClick(DialogInterface dialog, int whichButton) {
                     * operateRestore(); } }) .show();
                     */
                    operateRestore();// delete
                }
                else {
                    Log.d(TAG, "on MENU_MULTISEL_OK no Item Checked");
                    Toast.makeText(getApplicationContext(), R.string.stop_no_item_selected, 2)
                            .show();
                }
                return true;

                /*
                 * case MENU_MULTISEL_ALL: allSelect(true); return true; case
                 * MENU_MULTISEL_CANCEL: allSelect(false); return true;
                 */
            default:
                return true;

        }
    }

    @Override
    public boolean onPreferenceTreeClick(PreferenceScreen preferenceScreen, Preference preference) {
        Log.d(TAG, "onPreferenceTreeClick ");
        if (preference instanceof ApplicationPreference) {
            ApplicationPreference appPreference = (ApplicationPreference) preference;
            AppInfo app = appPreference.getCachedAppInfo();
            app.print();
            appPreference.setChecked();
            return true;
        }

        return super.onPreferenceTreeClick(preferenceScreen, preference);
    }

    private void allSelect(boolean enable) {
        Log.d(TAG, "allSelect enable" + enable);
        for (int i = 0; i < mBackupSource.length; i++) {
            mBackupSource[i].setChecked(enable);
        }
    }

    private void operateRestore() {
        Log.d(TAG, "operateRestore start");

        if (!BackupUtils.isTfCardExist()) {
            Message message = mHandler.obtainMessage(EVENT_SDCARD_NOT_MOUNTED);
            mHandler.sendMessage(message);
            return;
        }

        mSendProgressDialog = new Showconnectprogress(this);
        mSendProgressDialog.setTitle(R.string.prepare_restore);
        // mSendProgressDialog.setProgressStyle(ProgressDialog.STYLE_HORIZONTAL);
        mSendProgressDialog.setProgressStyle(ProgressDialog.STYLE_SPINNER);
        mSendProgressDialog.setIndeterminate(true);
        mSendProgressDialog.setCanceledOnTouchOutside(false);
        mSendProgressDialog.show();
        if (LOCAL_DEBUG)
            Log.v(TAG, "Show the progress bar");
        mOperateThread = new OperateThread();
        Thread thread = new Thread(mOperateThread);
        thread.start();
    }

    private void updateProgressDialog(String title, int max) {
        if (mSendProgressDialog != null) {
            mSendProgressDialog.setTitle(title);
            // if (max > 0) {
            // mSendProgressDialog.setMax(max);
            // }
            // mSendProgressDialog.setProgress(0);
        }
    }

    private void updateProgressValue(int val) {
        if (mSendProgressDialog != null) {
            mSendProgressDialog.setProgress(val);
        } else {
            Log.d(TAG, "updateProgressValue mSendProgressDialog NULL");
        }
    }

    public boolean getCancelRestoreFlag() {
        return mCancelBackupFlag;
    }

    public void setCancelRestoreFlag(boolean flag) {
        mCancelBackupFlag = flag;
    }

    public boolean isChecked() {
        int i;
        boolean flag = false;
        for (i = 0; i < mBackupSource.length; i++) {
            if (mBackupSource[i].isChecked()) {
                flag = true;
            }
            if (i == mBackupSource.length) {
                flag = false;
            }
        }

        if (flag == false) {
            Log.d(TAG, "isChecked() mAppList.size(): " + mAppList.size());
            for (int j = 0; j < mAppList.size(); j++) {
                ApplicationPreference appPreference = mAppList.get(j);
                if (appPreference.isChecked()) {
                    flag = true;
                    break;
                }
            }
        }

        return flag;
    }

    private void createAppPreference() {
        ArrayList<AppInfo> appListInfo = getBackupAppInfo();
        for (AppInfo app : appListInfo) {
            app.print();
            ApplicationPreference preference = new ApplicationPreference(this, app);
            mAppList.addPreference(preference);
        }
    }

    private ArrayList<AppInfo> getBackupAppInfo() {
        ArrayList<AppInfo> appList = new ArrayList<AppInfo>();

        int index = mCurrentDir.indexOf(".zip");
        String dir = (index == -1 ? mCurrentDir : (mCurrentDir.substring(0, index)));
        Log.d(TAG, "dir = " + dir + " mCurrentDir = " + mCurrentDir);
        unZipSpecifiedFile(("/mnt" + DEFAULT_BACKUP_ROOT_DIR + mCurrentDir), "apk.xml",
                DEFAULT_BACKUP_ROOT_DIR);
        // String backupinfofile = DEFAULT_BACKUP_ROOT_DIR + dir + ".txt";

        InputStream inputStream = null;
        XmlPullParser xmlParser = Xml.newPullParser();

        String backupinfofile = DEFAULT_BACKUP_ROOT_DIR + DEFAULT_BACKUP_APP_DIR + "/" + "apk.xml";

        Log.d(TAG, "getBackupAppInfo backupinfofile: " + backupinfofile);
        File file = new File(backupinfofile);
        if (!file.exists()) {
            Log.d(TAG, "getBackupAppInfo backup info file is  not exist");
            return appList;
        }

        try
        {
            xmlParser.setInput(new FileReader(backupinfofile));
            int evtType = xmlParser.getEventType();
            while (evtType != XmlPullParser.END_DOCUMENT) {
                switch (evtType)
                {
                    case XmlPullParser.START_TAG: {
                        String tag = xmlParser.getName();
                        if (tag.equalsIgnoreCase(ITEM_RECORD)) {
                            String packagename = xmlParser.getAttributeValue(null, PACKAGENAME);
                            String name = xmlParser.getAttributeValue(null, APPNAME);
                            String apkname = xmlParser.getAttributeValue(null, APKNAME);
                            AppInfo tmpInfo = new AppInfo();
                            tmpInfo.appName = name;
                            tmpInfo.packageName = packagename;
                            tmpInfo.sourcedir = apkname;
                            tmpInfo.print();
                            appList.add(tmpInfo);
                        }
                        break;
                    }
                    case XmlPullParser.END_TAG:
                        break;
                    default:
                        break;
                }
                evtType = xmlParser.next();
            }
        } catch (Exception e) {
            Log.d(TAG, "error = " + e);
        }

        deleteDir(DEFAULT_BACKUP_ROOT_DIR + DEFAULT_BACKUP_APP_DIR);

        return appList;
    }

    private ArrayList<AppInfo> getInstalledAppInfo() {
        ArrayList<AppInfo> appList = new ArrayList<AppInfo>();
        List<PackageInfo> packages = getPackageManager().getInstalledPackages(0);
        for (int i = 0; i < packages.size(); i++) {
            PackageInfo packageInfo = packages.get(i);
            AppInfo tmpInfo = new AppInfo();
            tmpInfo.appName = packageInfo.applicationInfo.loadLabel(getPackageManager()).toString();
            tmpInfo.packageName = packageInfo.packageName;
            tmpInfo.versionName = packageInfo.versionName;
            tmpInfo.versionCode = packageInfo.versionCode;
            // tmpInfo.appInfo = packageInfo.applicationInfo;
            // tmpInfo.appIcon =
            // packageInfo.applicationInfo.loadIcon(getPackageManager());
            // Only display the non-system app info
            if ((packageInfo.applicationInfo.flags & ApplicationInfo.FLAG_SYSTEM) == 0)
            {
                Log.d(TAG, "This is a installed app" + tmpInfo.appName);
                appList.add(tmpInfo);
            }
            else
            {
                Log.d(TAG, "This is a system app" + tmpInfo.appName);
            }
        }
        for (int i = 0; i < appList.size(); i++)
        {
            appList.get(i).print();
        }

        return appList;
    }

    private boolean isBackupAppReistalled(AppInfo appinfo) {
        boolean ret = false;
        ArrayList<AppInfo> appList = getInstalledAppInfo();
        for (int i = 0; i < appList.size(); i++) {
            if (appinfo.appName.equals(appList.get(i).appName)) {
                ret = true;
                break;
            } else {
                continue;
            }
        }

        return ret;
    }

}
