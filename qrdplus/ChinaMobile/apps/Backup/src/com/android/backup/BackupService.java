/*
 * Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.android.backup;

import static com.android.backup.BackupUtils.BACKUP_TYPE_ALL;
import static com.android.backup.BackupUtils.BACKUP_TYPE_APP;
import static com.android.backup.BackupUtils.BACKUP_TYPE_CALENDAR;
import static com.android.backup.BackupUtils.BACKUP_TYPE_CONTACTS;
import static com.android.backup.BackupUtils.BACKUP_TYPE_MMS;
import static com.android.backup.BackupUtils.BACKUP_TYPE_SMS;
import static com.android.backup.BackupUtils.EVENT_BACKUP_RESULT;
import static com.android.backup.BackupUtils.EVENT_FILE_CREATE_ERR;
import static com.android.backup.BackupUtils.EVENT_INIT_PROGRESS_TITLE;
import static com.android.backup.BackupUtils.EVENT_SET_PROGRESS_VALUE;

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.FileWriter;
import java.io.IOException;
import java.io.OutputStream;
import java.util.ArrayList;
import java.util.List;
import java.util.Map;

import com.android.backup.ContactsContract.RawContacts;
import com.android.backup.vcalendar.VCalComposer;
import com.android.backup.vcalendar.VCalendarManager;
import com.android.backup.vcard.VCardComposer;
import com.android.backup.vcard.VCardComposer.HandlerForOutputStream;
import com.android.backup.vcard.VCardConfig;

import android.app.ProgressDialog;
import android.app.Service;
import android.content.ContentResolver;
import android.content.ContentUris;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.database.Cursor;
import android.net.Uri;
import android.os.FileUtils;
import android.os.IBinder;
import android.os.RemoteCallbackList;
import android.os.RemoteException;
import android.os.SystemProperties;
import android.provider.CalendarContract.Events;
import android.util.Log;

public class BackupService extends Service {

    private static final String TAG = "BackupService";
    private static boolean DBG = true;
//    private static final String SDCardPath = BackupUtils.strSDCardPath;
//    private static final String DEFAULT_BACKUP_ROOT_DIR = SDCardPath + "/backup/Data";
//    private static final String DEFAULT_BACKUP_ROOT_DIR1 = SDCardPath + "/backup";
//    private static final String DEFAULT_BACKUP_ROOT_DIR2 = SDCardPath + "/backup/App";
    private static final String BACKUP_DATA = "/backup/Data";
    private static final String BACKUP_FOLDER = "/backup";
    private static final String BACKUP_APP = "/backup/App";
    private static final String DEFAULT_BACKUP_CONTACTS_DIR = "/Contact";
    private static final String DEFAULT_BACKUP_MMS_SMS_DIR = "/Mms";
    private static final String DEFAULT_BACKUP_SMS_DIR = "/Sms";
    private static final String DEFAULT_BACKUP_CALENDAR_DIR = "/Calendar";
//    private static final String DEFAULT_BACKUP_APP_DIR_FILE = SDCardPath
//            + "/backup/backupAppData.txt";
    private static final String BACKUP_APP_TXT = "/backup/backupAppData.txt";

    private static final String BACKUP_EXTRA_DIR = "backup_dir";
    private static final String BACKUP_EXTRA_DATA_CHK = "data_checked";
    private static final String BACKUP_EXTRA_DATA_CNT = "data_cnt";
    private static final String BACKUP_EXTRA_MSG_SLOT = "msg_slot";
    private static final String BACKUP_EXTRA_APP_SIZE = "app_size";
    private static final String BACKUP_EXTRA_APP_CHK = "app_checked";
    private static final String BACKUP_EXTRA_APP_INFO = "app_info";
    private static final String BACKUP_EXTRA_PATH = "backup_path";

    private static final String[] EVENT_PROJECTION = new String[] {
        Events._ID,
    };
    private static final int EVENT_INDEX_ID = 0;

    private static final int BACKUP = 0;
//    private static final int CALENDAR = 1;
    private static final String VCALENDAR = "/other/vcalendar/";
//    private static final String VCalendarPath = BackupUtils.strSDCardPath
//            + "/other/vcalendar/";
    private static final int VCARD_TYPE = VCardConfig.VCARD_TYPE_DEFAULT;
    private static final int OUT_PUT_BUFFER_SIZE = 1024 * 4;

    private static final int BACKUP_RESULT_FAIL = 0;
    private static final int BACKUP_RESULT_OK = 1;

    private String mBackupPath;
    private String mBackupDataDir;
    private String mBackupRootDir;
    private String mBackupAppDir;
    private String mBackupAppFile;
    private String mVCalendarPath;

    private RemoteCallbackList<IBackupCallback> mCallbackList;
    private IBackupCallback mBackupCallback;

    private boolean mCancelBackupFlag = false;
    private String mCurrentDir = null;
    private int mStartId = 0;
    private BackupMmsOperation mBackupMessageOper = null;

    private IRemoteBackupService.Stub mRemoteBinder = new IRemoteBackupService.Stub() {

        @Override
        public void unregisterCallback(String packageName, IBackupCallback cb)
                throws RemoteException {
            Log.v(TAG, "BackupService unregisterCallback");
            if (cb != null) {
                mBackupCallback = null;
                if (mCallbackList != null) {
                    mCallbackList.unregister(cb);
                }
            }
        }

        @Override
        public void registerCallback(String packageName, IBackupCallback cb)
                throws RemoteException {
            Log.v(TAG, "BackupService registerCallback");
            if (cb != null) {
                mBackupCallback = cb;
                if (mCallbackList != null) {
                    mCallbackList.register(cb);
                }
            }
        }

        @Override
        public void init(String packageName) throws RemoteException {
            Log.v(TAG, "BackupService init: " + packageName);
        }

        @Override
        public void setCancelBackupFlag(boolean flag) throws RemoteException {
            mCancelBackupFlag = flag;
            if (flag && mBackupMessageOper != null) {
                 mBackupMessageOper.cancel();
            }
        }
    };

    public void sendBackupMessage(int what, int type, int result) {
        Log.v(TAG, "BackupService sendCallbackMessage what: " + what + " type: " + type
                + " result: " + result);
        if (mBackupCallback == null) {
            return;
        }
        mCallbackList.beginBroadcast();
        try {
            mBackupCallback.handleBackupMsg(what, type, result);
        } catch (RemoteException e) {
            e.printStackTrace();
        }
        mCallbackList.finishBroadcast();
    }

    @Override
    public IBinder onBind(Intent intent) {
        Log.v(TAG, "BackupService: onBind");
        return mRemoteBinder;
    }

    @Override
    public void onCreate() {
        super.onCreate();
        Log.v(TAG, "BackupService: onCreate");
        mCallbackList = new RemoteCallbackList<IBackupCallback>();
        mCancelBackupFlag = false;
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        Log.v(TAG, "BackupService: onDestroy");
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        Log.v(TAG, "BackupService: onStartCommand: " + startId);
        mStartId = startId;
        mCancelBackupFlag = false;

        ArrayList<AppInfo> appList = intent.getParcelableArrayListExtra(BACKUP_EXTRA_APP_INFO);
        for (AppInfo data : appList) {
            data.print();
        }

        mCurrentDir = intent.getStringExtra(BACKUP_EXTRA_DIR);
        boolean[] appChecked = intent.getBooleanArrayExtra(BACKUP_EXTRA_APP_CHK);
        int appSize = intent.getIntExtra(BACKUP_EXTRA_APP_SIZE, 0);
        boolean[] dataChecked = intent.getBooleanArrayExtra(BACKUP_EXTRA_DATA_CHK);
        int dataCount = intent.getIntExtra(BACKUP_EXTRA_DATA_CNT, 0);
        int[] msgSlot = intent.getIntArrayExtra(BACKUP_EXTRA_MSG_SLOT);
        mBackupPath = intent.getStringExtra(BACKUP_EXTRA_PATH);
        mBackupDataDir = mBackupPath + BACKUP_DATA;
        mBackupRootDir = mBackupPath + BACKUP_FOLDER;
        mBackupAppDir = mBackupPath + BACKUP_APP;
        mBackupAppFile = mBackupPath + BACKUP_APP_TXT;
        mVCalendarPath = mBackupPath + VCALENDAR;

        BackupServiceThread backupThread = new BackupServiceThread(
                appList, appChecked, appSize, dataChecked, dataCount, msgSlot);
        backupThread.start();

        return 0;
    }

    @Override
    public boolean onUnbind(Intent intent) {
        return super.onUnbind(intent);
    }

    public boolean getCancelBackupFlag() {
        return mCancelBackupFlag;
    }

    public void setCancelBackupFlag(boolean flag) {
        mCancelBackupFlag = flag;
    }

    private boolean copyFile(String srcdir, String disdir) {
        FileInputStream fileInputStream;
        FileOutputStream fileOutputStream;
        BufferedInputStream bis;
        BufferedOutputStream bos;
        boolean sIsAborted = false;
        int position = 0;
        int readLength = 0;
        if (DBG)
            Log.d(TAG, " copyFile srcdir: " + srcdir + " disdir: " + disdir);
        File disfileinfo = null;
        File fileinfo = new File(srcdir);
        if (!fileinfo.exists()) {
            Log.e(TAG, "copyPictureFile file not exist");
            return false;
        }

        byte[] buffer = new byte[OUT_PUT_BUFFER_SIZE];

        try {
            fileInputStream = new FileInputStream(fileinfo);
        } catch (IOException e) {
            Log.e(TAG, "copyPictureFile open stream " + e.toString());
            return false;
        }
        bis = new BufferedInputStream(fileInputStream, 0x4000);

        try {
            disfileinfo = new File(disdir);
            if (!disfileinfo.exists()) {
                disfileinfo.createNewFile();
            }
        } catch (IOException e) {
            Log.e(TAG, "copyPictureFile 1open stream " + e.toString());
            return false;
        }
        try {
            fileOutputStream = new FileOutputStream(disfileinfo);
        } catch (IOException e) {
            Log.e(TAG, "copyPictureFile open stream " + e.toString());
            return false;
        }
        bos = new BufferedOutputStream(fileOutputStream, 0x10000);

        try {
            while ((position != fileinfo.length())) {
                if (mCancelBackupFlag) {
                    sIsAborted = true;
                    break;
                }
                if (position != fileinfo.length()) {
                    readLength = bis.read(buffer, 0, OUT_PUT_BUFFER_SIZE);
                }
                if (DBG)
                    Log.d(TAG, "Read File");
                bos.write(buffer, 0, readLength);
                position += readLength;
            }
        } catch (IOException e) {
            Log.e(TAG, "Write aborted " + e.toString());
            if (DBG)
                Log.d(TAG, "Write Abort Received");
            return false;
        }

        if (bis != null) {
            try {
                bis.close();
            } catch (IOException e) {
                Log.e(TAG, "input stream close" + e.toString());
                if (DBG)
                    Log.d(TAG, "Error when closing stream after send");
                return false;
            }
        }

        if (bos != null) {
            try {
                bos.close();
            } catch (IOException e) {
                Log.e(TAG, "onPut close stream " + e.toString());
                if (DBG)
                    Log.d(TAG, "Error when closing stream after send");
                return false;
            }
        }
        if (DBG)
            Log.d(TAG, "sendFile - position = " + position);
        if (position == fileinfo.length() && sIsAborted == false) {
            if (DBG)
                Log.d(TAG, "sendFile - return ok ");
            return true;
        } else {
            if (DBG)
                Log.d(TAG, "sendFile - return false ");
            return false;
        }
    }

    private boolean backupApp(AppInfo appinfo, String dirpath, boolean needdir) {
        Log.d(TAG, "backupApp start: path:" + dirpath);
        appinfo.print();

        String appName = appinfo.appName;
        String sub = appinfo.sourcedir.substring(appinfo.sourcedir
                .lastIndexOf("/"));
        Log.d(TAG, "backupApp sub:" + sub + " appName : " + appName);
        copyFile(appinfo.sourcedir, dirpath + "/" + appinfo.packageName
                + ".apk");

        FileOutputStream fout = null;
        try {
            File file = new File(mBackupAppFile);
            fout = new FileOutputStream(file, true);
            if (needdir == true) {
                dirpath += "/";
                Log.d(TAG, "Dirpath = " + dirpath);
                fout.write(dirpath.getBytes());
                fout.write('\n');
            }
            Log.d(TAG, "PackageName = " + appinfo.packageName);
            fout.write(appinfo.packageName.getBytes());
            fout.write('\n');
        } catch (IOException e) {
            Log.d(TAG, "backupEmail IOException" + e);
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

    private void outputRawContactsAsVCardFile(Uri uri, String filename,
            String selection, String[] selectionArgs) {

        OutputStream outputStream = null;
        try {
            outputStream = new FileOutputStream(filename);
        } catch (FileNotFoundException e) {
            return;
        }

        final VCardComposer composer = new VCardComposer(BackupService.this,
                VCARD_TYPE, false);
        composer.addHandler(composer.new HandlerForOutputStream(outputStream));

        // No extra checks since composer always uses restricted views
        if (!composer.init(uri, selection, selectionArgs, null))
            return;

        // Update the progressbar info
        int max = composer.getCount();
        Log.d(TAG, "outputRawContactsAsVCardFile max: " + max);
        sendBackupMessage(EVENT_INIT_PROGRESS_TITLE, BACKUP_TYPE_CONTACTS, max);

        int i = 1;
        while (!composer.isAfterLast()) {
            Log.d(TAG, "OutputRawContactsAsVCardFile : i =" + i);
            if (mCancelBackupFlag) {
                return;
            }
            try {
                if (mBackupPath.equals(BackupUtils.strExternalPath)) {
                    if (BackupUtils.isTfCardFull()) {
                        mCancelBackupFlag = true;
                        return;
                    }
                } else {
                    if (BackupUtils.isInternalFull()) {
                        mCancelBackupFlag = true;
                        return;
                    }
                }
            } catch (IllegalArgumentException e) {
                Log.v(TAG, "Backup contacts: " + e.getMessage());
                return;
            }
            if (!composer.createOneEntry()) {
                Log.w(TAG, "Failed to output a contact.");
            }

            if (i % 50 == 0) {
                sendBackupMessage(EVENT_SET_PROGRESS_VALUE, i, 0);
            }
            i++;
        }
        composer.terminate();
    }

    public boolean backupContacts() {
        String filename = mBackupDataDir + mCurrentDir
                + DEFAULT_BACKUP_CONTACTS_DIR + "/contact.vcf";
        String selection = "(deleted=0) and (account_type='com.android.localphone')";
        outputRawContactsAsVCardFile(RawContacts.CONTENT_URI, filename,
                selection, null);
        return new File(filename).exists();

    }

    private boolean backupMMS(String dir, int slot) {
        mBackupMessageOper = new BackupMmsOperation(this, null, dir, slot, mBackupPath);
        mBackupMessageOper.backupMmsMessages();
        mBackupMessageOper = null;
        return new File(dir).exists();
    }

    private boolean backupSMS(String dir, int slot) {
        mBackupMessageOper = new BackupMmsOperation(this, null, dir, slot, mBackupPath);
        mBackupMessageOper.backupSmsMessages();
        mBackupMessageOper = null;
        return new File(dir).exists();
    }

    private boolean backupCalendar(int type) {
        ContentResolver cr = getContentResolver();
        Uri uri = Events.CONTENT_URI;
        Cursor cursor = cr.query(uri, EVENT_PROJECTION, null, null, null);
        if (cursor == null) {
            return false;
        }
        if (!cursor.moveToFirst()) {
            cursor.close();
            return false;
        }
        if (DBG) {
            Log.d(TAG, "backupCalendar cursor.getCount(): " + cursor.getCount());
        }
        String filename = "";
        if (BACKUP == type) {
            filename = mBackupDataDir + mCurrentDir + DEFAULT_BACKUP_CALENDAR_DIR
                    + "/calendar.vcs";
        } else {
            filename = mVCalendarPath + "vcalendar.vcs";
        }

        File newfile = new File(filename);
        Log.d(TAG, "Parent=" + (newfile.getParent()));
        File parent = new File(newfile.getParent());
        parent.mkdirs();

        if (newfile.exists()) {
            newfile.delete();
        }

        try {
            if (!newfile.createNewFile()) {
                return false;
            }

            FileWriter fw = new FileWriter(newfile);
            VCalendarManager vm;
            String vCalString;
            int size = cursor.getCount();

            sendBackupMessage(EVENT_INIT_PROGRESS_TITLE, BACKUP_TYPE_CALENDAR, size);
            for (int i = 0; i < size; i++) {
                if (mCancelBackupFlag) {
                    return false;
                }

                try {
                    if (mBackupPath.equals(BackupUtils.strExternalPath)) {
                        if (BackupUtils.isTfCardFull()) {
                            mCancelBackupFlag = true;
                            return true;
                        }
                    } else {
                        if (BackupUtils.isInternalFull()) {
                            mCancelBackupFlag = true;
                            return true;
                        }
                    }
                } catch (IllegalArgumentException e) {
                    Log.v(TAG, "Backup calendar: " + e.getMessage());
                    return false;
                }

                long eventId = cursor.getLong(EVENT_INDEX_ID);
                Uri uri_item = ContentUris.withAppendedId(Events.CONTENT_URI, eventId);
                vm = new VCalendarManager(this, uri_item);
                vm.setVcalVersion(VCalComposer.VERSION_VCAL20_INT);
                vm.setUseUtc(false);
                Log.d(TAG, "size = " + size + "  i  =" + i);
                if (size == 1) {
                    vCalString = vm.getVeventData(vm.FLAG_ALL_DATA);
                } else {
                    if (i == 0) {
                        vCalString = vm.getVeventData(vm.FLAG_FIRST_DATA);
                    } else if (i == (size - 1)) {
                        vCalString = vm.getVeventData(vm.FLAG_TAIL_DATA);
                    } else {
                        vCalString = vm.getVeventData(vm.FLAG_VCAL_DATA);
                    }
                }

                fw.write(vCalString);
                fw.flush();

                cursor.moveToNext();
                sendBackupMessage(EVENT_SET_PROGRESS_VALUE, i, 0);
            }
        } catch (IOException e) {
            e.printStackTrace();
            return false;
        }

        return true;
    }

    private class BackupServiceThread extends Thread {

        private List<List<Map<String, String>>> mChildData;
        private ArrayList<AppInfo> mAppInfoList;
        private boolean[] mAppChecked;
        private int mAppSize;
        private boolean[] mDataChecked;
        private int mDataCount;
        private int[] mMsgSlot;

        public BackupServiceThread(ArrayList<AppInfo> appList, boolean[] appChecked, int appSize,
                boolean[] dataChecked, int dataCount, int[] msgSlot) {
            super("OperateThread1");
            mAppInfoList = appList;
            mAppChecked = appChecked;
            mAppSize = appSize;
            mDataChecked = dataChecked;
            mDataCount = dataCount;
            mMsgSlot = msgSlot;
        }

        public void run() {
            Log.v(TAG, "BackupService: BackupServiceThread");
            Log.d(TAG, "BackupServiceThread directory: " + mCurrentDir);

            boolean backupapp = false;
            int num = 1;
            int firstFlag = 0;
            int appFailCnt = 0;

            // Backup app
            String app_filename = mBackupAppDir;
            if (DBG) {
                Log.d(TAG, "App_filename = " + app_filename);
            }
            File app_dir = new File(app_filename);
            if (mAppSize > 0) {
                for (int i = 0; i < mAppInfoList.size(); i++) {
                    if (0 == firstFlag) {
                        sendBackupMessage(EVENT_INIT_PROGRESS_TITLE, BACKUP_TYPE_APP,
                                mAppSize + mDataCount);
                    }
                    if (!mCancelBackupFlag && mAppChecked[i]) {
                        firstFlag++;
                        AppInfo appin = mAppInfoList.get(i);
                        appin.print();
                        backupapp = true;
                        if (prepareBackupDir(app_dir)) {
                            checkBackupSdcardTag();
                            sendBackupMessage(EVENT_SET_PROGRESS_VALUE, num, 0);
                            num++;
                            if (1 == firstFlag) {
                                if (DBG)
                                    Log.d(TAG, "prepareBackupDir true");
                                backupApp(appin, app_filename, false);
                            } else {
                                if (DBG)
                                    Log.d(TAG, "prepareBackupDir false");
                                backupApp(appin, app_filename, false);
                            }
                            Log.d(TAG, "sendbroadcast:BACKUPAPPDATA");
                            BackupService.this.sendBroadcast(new Intent(
                                    "android.intent.action.BACKUPAPPDATA"));
                            Log.d(TAG, "sendbroadcast:BACKUPAPPDATA end    ");
                            boolean whileFlag = true;
                            while (whileFlag) {
                                try {
                                    Thread.sleep(1500);
                                } catch (InterruptedException e) {
                                    Log.e(TAG, "InterruptedException :" + e);
                                }
                                if ("1".equals(SystemProperties.get("persist.sys.shflag", "0"))) {
                                    whileFlag = false;
                                }
                            }
                            if (!(new File(app_filename + "/" + appin.packageName + ".apk")).exists()
                                    || !(new File(app_filename + "/" + appin.packageName
                                            + ".tar").exists())) {
                                appFailCnt++;
                            }
                        } else {
                            appFailCnt++;
                        }
                    }
                }
            } else {
                sendBackupMessage(EVENT_INIT_PROGRESS_TITLE, BACKUP_TYPE_APP, mDataCount);
            }

            // Backup contacts
            boolean contactFail = false;
            if (!mCancelBackupFlag && mDataChecked[0]) {
                File contacts_dir = new File(mBackupDataDir
                        + mCurrentDir + DEFAULT_BACKUP_CONTACTS_DIR);
                if (prepareBackupDir(contacts_dir)) {
                    backupContacts();
                } else {
                    contactFail = true;
                    Log.d(TAG, "OperateThread: contacts error");
                }
                sendBackupMessage(EVENT_SET_PROGRESS_VALUE, num, 0);
                num++;
            }

            // Backup MMS
            boolean mmsFail = false;
            if (!mCancelBackupFlag && mDataChecked[1]) {
                String mms_file = mBackupDataDir + mCurrentDir
                        + DEFAULT_BACKUP_MMS_SMS_DIR;
                File mms_dir = new File(mms_file);
                if (prepareBackupDir(mms_dir)) {
                    mmsFail = !backupMMS(mms_file, mMsgSlot[0]);
                } else {
                    mmsFail = true;
                    Log.d(TAG, "OperateThread:mms error");
                }
                if (!mmsFail && BackupUtils.isMultiSimEnabled()) {
                    BackupUtils.setBackUpMmsSmsSlot(BackupService.this,
                            mCurrentDir + "_Mms", mMsgSlot[0]);
                }
                sendBackupMessage(EVENT_SET_PROGRESS_VALUE, num, 0);
                num++;
            }

            // Backup SMS
            boolean smsFail = false;
            if (!mCancelBackupFlag && mDataChecked[2]) {
                String mms_sms_file = mBackupDataDir + mCurrentDir
                        + DEFAULT_BACKUP_SMS_DIR;
                File mms_sms_dir = new File(mms_sms_file);

                if (prepareBackupDir(mms_sms_dir)) {
                    smsFail = !backupSMS(mms_sms_file, mMsgSlot[1]);// wrong
                } else {
                    smsFail = true;
                    Log.d(TAG, "OperateThread:sms error");
                }
                if (!smsFail && BackupUtils.isMultiSimEnabled()) {
                    BackupUtils.setBackUpMmsSmsSlot(BackupService.this,
                            mCurrentDir + "_Sms", mMsgSlot[1]);
                }
                sendBackupMessage(EVENT_SET_PROGRESS_VALUE, num, 0);
                num++;
            }

            // Backup calendar
            boolean calendarFail = false;
            if (!mCancelBackupFlag && mDataChecked[3]) {
                File calendar_dir = new File(mBackupDataDir
                        + mCurrentDir + DEFAULT_BACKUP_CALENDAR_DIR);
                if (prepareBackupDir(calendar_dir)) {
                    backupCalendar(BACKUP);
                } else {
                    calendarFail = true;
                    Log.d(TAG, "OperateThread:calender error");
                    sendBackupMessage(EVENT_BACKUP_RESULT, BACKUP_TYPE_CALENDAR, 0);
                }
                sendBackupMessage(EVENT_SET_PROGRESS_VALUE, num, 0);
                num++;
            }

            if (appFailCnt == 0 && !contactFail && !mmsFail && !smsFail && !calendarFail) {
                if (backupapp && (mDataChecked[0] || mDataChecked[1] || mDataChecked[2] || mDataChecked[3]))
                    sendBackupMessage(EVENT_BACKUP_RESULT, BACKUP_TYPE_ALL, BACKUP_RESULT_OK);
                else if (backupapp)
                    sendBackupMessage(EVENT_BACKUP_RESULT, BACKUP_TYPE_APP, BACKUP_RESULT_OK);
                else if (mDataChecked[0])
                    sendBackupMessage(EVENT_BACKUP_RESULT, BACKUP_TYPE_CONTACTS, BACKUP_RESULT_OK);
                else if (mDataChecked[1])
                    sendBackupMessage(EVENT_BACKUP_RESULT, BACKUP_TYPE_MMS, BACKUP_RESULT_OK);
                else if (mDataChecked[2])
                    sendBackupMessage(EVENT_BACKUP_RESULT, BACKUP_TYPE_SMS, BACKUP_RESULT_OK);
                else if (mDataChecked[3])
                    sendBackupMessage(EVENT_BACKUP_RESULT, BACKUP_TYPE_CALENDAR, BACKUP_RESULT_OK);

                Intent scanIntent =
                        new Intent("com.android.fileexplorer.action.MEDIA_SCANNER_SCAN_ALL");
                scanIntent.setData(Uri.fromFile(new File(BackupUtils.strExternalPath)));
                sendBroadcast(scanIntent);
            } else {
                sendBackupMessage(EVENT_BACKUP_RESULT, BACKUP_TYPE_ALL, BACKUP_RESULT_FAIL);
            }
            BackupService.this.stopSelf(mStartId);
        }

        public boolean isChildChecked(int groupPosition, int childPosition) {
            if (groupPosition < mChildData.size()) {
                List<Map<String, String>> list = mChildData.get(groupPosition);
                if (list != null && childPosition < list.size()) {
                    Map<String, String> item = list.get(childPosition);
                    if (item != null) {
                        Object key = item.get("Checked");
                        if (key != null && (key instanceof String)
                                && ((String) key).compareTo("true") == 0)
                            return true;
                    }

                }
            }
            return false;
        }
    }

    private boolean prepareBackupDir(File dir) {
        if (!dir.exists()) {
            Log.d(TAG, "prepareBackupDir backup dir is not exist");
            if (!dir.mkdir()) {
                if (DBG)
                    Log.d(TAG,
                            "prepareBackupDir backup stop - can't create base directory "
                                    + dir.getPath());
                sendBackupMessage(EVENT_FILE_CREATE_ERR, 0, 0);
                return false;
            } else {
                Log.d(TAG, "prepareBackupDir change file permission mode");
                FileUtils.setPermissions(mBackupRootDir,
                        FileUtils.S_IRWXO | FileUtils.S_IRWXU
                                | FileUtils.S_IRWXG, -1, -1);
                return true;
            }
        }
        return true;
    }

    private void checkBackupSdcardTag() {
        final int path_fake = -1;
        SharedPreferences sharedPreferences = getSharedPreferences(
                BackupUtils.LOCATION_PREFERENCE_NAME, Context.MODE_PRIVATE);

        if (path_fake == sharedPreferences.getInt(BackupUtils.KEY_BACKUP_LOCATION, path_fake)) {
            SettingsActivity.deleteBackupTagFile();
        } else {
            SettingsActivity.createBackupTagFile();
        }

        if (path_fake == sharedPreferences.getInt(BackupUtils.KEY_RESTORE_LOCATION, path_fake)) {
            SettingsActivity.deleteRestoreTagFile();
        } else {
            SettingsActivity.createRestoreTagFile();
        }
    }
}
