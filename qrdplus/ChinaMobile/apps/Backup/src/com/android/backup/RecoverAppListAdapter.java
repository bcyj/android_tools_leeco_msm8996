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

import java.io.File;
import java.io.FilenameFilter;
import java.util.regex.Pattern;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Iterator;
import java.util.Comparator;
import java.util.Date;
import android.util.SparseBooleanArray;
import java.text.SimpleDateFormat;

import android.database.Cursor;
import android.graphics.drawable.Drawable;
import android.content.Context;
import android.view.Menu;
import android.view.MenuItem;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.view.KeyEvent;
import android.widget.BaseAdapter;
import android.widget.ImageView;
import android.widget.TextView;
import android.widget.ResourceCursorAdapter;
import android.widget.CheckedTextView;
import android.widget.Toast;

// multi_delete
import android.widget.CheckBox;
import android.app.ListActivity;
import android.app.Activity;
import android.app.AlertDialog;

import android.content.DialogInterface;
import android.content.DialogInterface.OnClickListener;

import android.util.Log;
import java.lang.Thread;
import android.os.Handler;
import android.os.Message;
import android.provider.Settings;

import java.io.File;
import java.io.FileWriter;
import android.os.FileUtils;
import java.io.IOException;
import java.io.FileOutputStream;
import java.io.FileInputStream;
import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.OutputStream;
import java.io.FileNotFoundException;
import java.io.InputStream;
import java.io.OutputStream;
import android.content.Intent;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.os.SystemProperties;
import android.os.SystemService;

public class RecoverAppListAdapter extends BaseAdapter
{
    private static final String TAG = "RecoverAppListAdapter";
    private static final String BACKUP_APP = "/backup/App";
//    private static final String DEFAULT_BACKUP_ROOT_DIR = BackupUtils.strSDCardPath
//            + "/backup/App";
    private String mRecoverPath;
    private String mBackupRootDir;
    private Context mContext;
    private LayoutInflater mInflater;
    private ArrayList<String> mFileNameList;
    private ArrayList<String> mApkNameList;
    private ArrayList<Boolean> mFileCheckedList;
    private File mCurrentDir;
    private Handler mHandler;
    private static final TheComparator mAcsOrder = new TheComparator();

    public static final int SORT_ASCEND = 0;
    public static final int SORT_TIME = 1;
    public static final int SORT_SIZE = 2;
    public static final int SORT_TYPE = 3;

    private static final int MENU_MULTI_DELETE = 1;
    private static final int MENU_MULTISEL_OK = 2;
    private static final int MENU_MULTISEL_ALL = 3;
    private static final int MENU_MULTISEL_CANCEL = 4;

    public static final int DEFAULT_MODE = 0;
    public static final int PICKER_DELETE_MODE = 1;

    public static final int MULTI_FLAG_DEFAULT = 0;
    public static final int MULTI_FLAG_MULTI_DELETE = 1;

    private static final int EVENTRE_STORE_APPDATA = 103;
    private static final int EVENTRE_CANCEL_PROGRESSDIALOG = 104;
    private static final int EVENTRE_STORE_APP_PERCENT = 106;
    private static final int EVENTRE_STORE_APP_DELETE = 107;
    private static final int EVENTRE_STORE_APP_DELETE_PER = 108;
    private static final int EVENTRE_STORE_APP_DELETE_FIN = 109;
    private int mMultiFlag = MULTI_FLAG_DEFAULT;
    private ListActivity mListActivity;
    public int mMode = 0;
    public boolean mCancel = false;
    private static final String RECOVER_APP_FILE = "/backup/recoverAppData.txt";
    private String mRecoverAppDirFile;
//    private static final String DEFAULT_RECOVER_APP_DIR_FILE = BackupUtils.strSDCardPath
//            + "/backup/recoverAppData.txt";

    public static class ViewHolder {
        TextView filename;
        // for multi-select
        CheckedTextView check;
    }

    /**
     * This Comparator sorts strings into increasing order.
     */
    public static class TheComparator implements Comparator<String> {
        public int compare(String str1, String str2) {
            int len1 = str1.length();
            int len2 = str2.length();
            int len = len1 <= len2 ? len1 : len2;
            for (int i = 0; i < len; i++) {
                int value1 = str1.codePointAt(i);
                int value2 = str2.codePointAt(i);

                // 'A' -> 'a'
                if (value1 > 64 && value1 < 91)
                    value1 = value1 + 32;
                if (value2 > 64 && value2 < 91)
                    value2 = value2 + 32;

                if (value1 == value2)
                    continue;

                if (value1 > 256 && value2 > 256) {
                    return value1 > value2 ? -1 : 1;
                }
            }

            if (len1 == len2) {
                return 0;
            } else {
                return len1 > len2 ? -1 : 1;
            }
        }
    }

    // Constructor 2
    public RecoverAppListAdapter(Context context, Handler handler, String path) {
        mContext = context;
        mListActivity = (ListActivity) context;
        mInflater = LayoutInflater.from(mContext);
        mHandler = handler;
        mRecoverPath = path;
        mBackupRootDir = mRecoverPath + BACKUP_APP;
        mRecoverAppDirFile = mRecoverPath + RECOVER_APP_FILE;
    }

    public void sortImpl() {
        mCurrentDir = new File(mBackupRootDir);

        String[] mFileNameArray;

        if (mCurrentDir != null && mCurrentDir.exists()) {
            mFileNameArray = mCurrentDir.list();
            Log.d(TAG, "FileListAdapter:mFileNameArray: " + mFileNameArray);
            mFileNameList = getSortedFileNameArray(mCurrentDir, mFileNameArray);
        } else {
            mFileNameList = new ArrayList<String>();
            mCurrentDir = null;
        }
        mFileCheckedList = new ArrayList<Boolean>();
        mApkNameList = new ArrayList<String>();
        for (String p : mFileNameList) {
            try {
                PackageManager pm = mContext.getPackageManager();
                PackageInfo pakinfo = pm.getPackageArchiveInfo(mCurrentDir.getPath() + "/" + p,
                        PackageManager.GET_ACTIVITIES);
                if (pakinfo != null) {
                    ApplicationInfo appinfo = pakinfo.applicationInfo;
                    appinfo.sourceDir = mCurrentDir.getPath() + "/" + p;
                    appinfo.publicSourceDir = mCurrentDir.getPath() + "/" + p;
                    mApkNameList.add((String) pm.getApplicationLabel(appinfo));
                } else {
                    mApkNameList.add(p);
                }
            } catch (Exception ex) {
                Log.e(TAG, "package:" + p + " error for " + ex);
                ex.printStackTrace();
                mApkNameList.add(p);
            }
            mFileCheckedList.add(Boolean.FALSE);
        }
    }

    public int getShowMode() {
        return mMode;
    }

    public boolean handleBackKeyAction(int keyCode, KeyEvent event) {
        switch (keyCode) {
            case KeyEvent.KEYCODE_BACK:
                if (mMode == PICKER_DELETE_MODE) {
                    mMode = DEFAULT_MODE;
                    mListActivity.getListView().clearChoices();
                    notifyDataSetChanged();
                    return true;
                }
                break;

            default:
                break;
        }
        return false;
    }

    public int getCount() {
        if (mFileNameList != null) {
            return mFileNameList.size();
        } else {
            return 0;
        }
    }

    public Object getItem(int position) {
        if (mFileNameList != null) {
            return mFileNameList.get(position);
        } else {
            return null;
        }
    }

    public long getItemId(int position) {
        return position;
    }

    public View getView(int position, View convertView, ViewGroup parent) {
        // multi_delete
        boolean multiDel = false;
        if (mContext instanceof RecoverActivity) {
            multiDel = ((RecoverActivity) mContext).getMultiDelFlag();
        }

        if (mCurrentDir == null) {
            return null;
        }

        ViewHolder holder;
        if (convertView == null) {
            convertView = mInflater.inflate(R.layout.list_item1, null);
            holder = new ViewHolder();
            holder.filename = (TextView) convertView.findViewById(R.id.file_name);
            holder.check = (CheckedTextView) convertView.findViewById(R.id.check);
            convertView.setTag(holder);
        } else {
            holder = (ViewHolder) convertView.getTag();
        }

        // 1. Bind file name
        String filename = mApkNameList.get(position);
        holder.filename.setText(filename);

        String mFullDir = mCurrentDir.toString() + "/" + filename;
        File mFile = new File(mFullDir);
        holder.check.setChecked(mFileCheckedList.get(position).booleanValue());

        return convertView;
    }

    private ArrayList<String> getSortedFileNameArray(final File theFile, String[] filenames) {
        ArrayList<String> fileFolderArray = new ArrayList<String>();

        if (filenames != null) {
            for (String s : filenames) {
                Log.d(TAG, "add to filelist " + s + " addable = " + (!(s.equals("path"))));
                if (s.endsWith(".apk"))
                    fileFolderArray.add(s);
            }
        }
        return fileFolderArray;
    }

    public boolean onPrepareOptionsMenu(Activity activity, Menu menu) {

        menu.clear();

        // multi select ok
        menu.add(0, MENU_MULTISEL_OK, 0, R.string.menu_selectok)
                .setIcon(R.drawable.ic_menu_ok);

        // multi select all
        menu.add(0, MENU_MULTISEL_ALL, 0, R.string.menu_selectall)
                .setIcon(R.drawable.ic_menu_selectall);

        // multi select cancel
        menu.add(0, MENU_MULTISEL_CANCEL, 0, R.string.menu_clearall)
                .setIcon(R.drawable.ic_menu_clearall);

        return true;
    }

    public boolean onOptionsItemSelected(final RecoverAppActivity activity, MenuItem item) {
        final SparseBooleanArray array = activity.getListView().getCheckedItemPositions();
        int size = array.size();
        Log.d(TAG, "onOptionsItemSelected item" + item);

        switch (item.getItemId()) {
            case R.id.delete: {
                Log.d(TAG, "onOptionsItemSelected MENU_MULTI_DELETE");
                final int iSel = getSelectCnt();
                if (iSel == 0) {
                    Toast.makeText(activity, activity.getString(R.string.stop_no_item_selected),
                            Toast.LENGTH_SHORT).show();
                } else {
                    new AlertDialog.Builder(activity,
                            android.R.style.Theme_DeviceDefault_Dialog_NoActionBar)
                            .setTitle(R.string.delete_confirm_alert_title)
                            .setMessage(
                                    iSel == 1 ? R.string.delete_confirm_alert_msg1
                                            : R.string.delete_confirm_alert_msg)
                            .setNegativeButton(R.string.button_cancel, null)
                            .setPositiveButton(R.string.button_ok,
                                    new DialogInterface.OnClickListener() {
                                        public void onClick(DialogInterface dialog,
                                                int whichButton) {
                                            mHandler.sendMessage(mHandler.obtainMessage(
                                                    EVENTRE_STORE_APP_DELETE, iSel, 0));
                                        }
                                    }).show();
                }
                return true;
            }

            case MENU_MULTISEL_OK:
                Log.d(TAG, "onOptionsItemSelected MENU_MULTISEL_OK!!");

                if (size == 0) {
                    Log.d(TAG, "onOptionsItemSelected MENU_MULTISEL_OK  NO item(s) checked!");
                    Toast.makeText(mListActivity, R.string.stop_no_item_selected, 2).show();
                } else {
                    Log.d(TAG, "onOptionsItemSelected MENU_MULTISEL_OK item(s) checked!" + size);
                    mHandler.sendMessage(mHandler.obtainMessage(EVENTRE_STORE_APPDATA));
                }
                return true;

            case MENU_MULTISEL_ALL:
                allSelect(activity);
                return true;

            case MENU_MULTISEL_CANCEL:
                clearSelect(activity);
                return true;

            default:
                return false;

        }
    }

    private void allSelect(RecoverAppActivity activity) {
        Log.d(TAG, "allSelect");
        int count = getCount();
        Log.d(TAG, "allSelect count" + count);
        for (int i = 0; i < count; i++) {
            activity.getListView().setItemChecked(i, true);
        }
        notifyDataSetChanged();
    }

    private void clearSelect(RecoverAppActivity activity) {
        Log.d(TAG, "clearSelect");
        activity.getListView().clearChoices();
        notifyDataSetChanged();
    }

    public void operateMultiDelete(RecoverAppActivity activity) {
        Log.d(TAG, "operateMultiDelete start");
        int size = mFileNameList.size();
        int fin = 0;
        int failCnt = 0;
        for (int i = 0; i < size; i++)
        {
            boolean selected = mFileCheckedList.get(i).booleanValue();
            Log.d(TAG, "operateMultiDelete selected: " + selected);
            if (!selected) {
                continue;
            }
            String filename = mFileNameList.get(i);
            Log.d(TAG, "operateMultiDelete filename: " + filename);
            if (filename != null) {
                File del_dir = new File(mBackupRootDir + "/" + filename);
                deleteDir(del_dir);
                if (del_dir.exists())
                    failCnt += 1;
                del_dir = new File(mBackupRootDir + "/"
                        + filename.substring(0, filename.length() - 4) + ".tar");
                if (del_dir.exists())
                    failCnt += 1;
                deleteDir(del_dir);
            }
            mHandler.sendMessage(mHandler.obtainMessage(EVENTRE_STORE_APP_DELETE_PER, ++fin, 0));
            try {
                Thread.sleep(1000);
            } catch (Exception ex) {

            }
        }
        mListActivity.getListView().clearChoices();
        sortImpl();
        mHandler.sendMessage(mHandler.obtainMessage(EVENTRE_STORE_APP_DELETE_FIN, failCnt, 0));
    }

    public void operateMultiRestore(RecoverAppActivity activity) {
        Log.d(TAG, "operateMultiRestore start");
        // final SparseBooleanArray array =
        // activity.getListView().getCheckedItemPositions();
        // int size = array.size();
        int size = mFileNameList.size();
        int percent = 0;
        // open the adb service
        int adbEnable = Settings.Secure.getInt(activity.getContentResolver(),
                Settings.Secure.ADB_ENABLED, 0);
        if (adbEnable == 0) {
            Settings.Secure.putInt(activity.getContentResolver(), Settings.Secure.ADB_ENABLED, 1);
        }
        for (int i = 0; i < size; i++) {
            Log.d(TAG, "mCancel=" + activity.mCancel);
            if (activity.mCancel) {
                break;
            }
            boolean selected = mFileCheckedList.get(i).booleanValue();
            Log.d(TAG, "operateMultiDelete selected: " + selected);
            if (!selected) {
                continue;
            }
            String filename = mFileNameList.get(i);
            Log.d(TAG, "operateMultiDelete filename: " + filename);
            if (filename != null) {
                try {
                    Log.d(TAG, "install start");
                    String[] command = new String[] {
                            "pm", "install", mBackupRootDir + "/" + filename
                    };
                    Process process = Runtime.getRuntime().exec(command);
                    process.waitFor();
                    Log.d(TAG, "install end");
                    String[] pckName = filename.split(".apk");
                    String mPckName = pckName[0];

                    FileOutputStream fout = null;
                    try {
                        File file = new File(mRecoverAppDirFile);
                        fout = new FileOutputStream(file, true);
                        Log.d(TAG, "packageName = " + mPckName);
                        fout.write(mPckName.getBytes());
                        fout.write('\n');
                    } catch (IOException e) {
                        Log.d(TAG, "backupEmail IOException" + e);
                        return;
                    } finally {
                        if (null != fout) {
                            try {
                                fout.flush();
                                fout.close();
                            } catch (IOException e) {
                                return;
                            }
                        }
                    }
                } catch (Exception e) {
                    Log.d(TAG, "MultiRestore Error");
                }
            }
            try {
                if (activity.mCancel)
                    break;
                Thread.sleep(1500);
            } catch (Exception ex) {

            }
            mHandler.sendMessage(mHandler.obtainMessage(EVENTRE_STORE_APP_PERCENT, ++percent, 0));
        }
        Log.d(TAG, "sendbroadcast:BACKUPAPPDATA");
        if (!activity.mCancel)
        {
            activity.sendBroadcast(new Intent("android.intent.action.RECOVERAPPDATA"));
            Log.d(TAG, "sendbroadcast:BACKUPAPPDATA end");
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

            // wether the adb open or not
            if (adbEnable == 0) {
                Settings.Secure.putInt(activity.getContentResolver(), Settings.Secure.ADB_ENABLED,
                        0);
            }
        }
        mHandler.sendMessage(mHandler.obtainMessage(EVENTRE_CANCEL_PROGRESSDIALOG));
    }

    private void deleteDir(File dir) {
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
                deleteDir(files);
            }
        }
        dir.delete();
    }

    // File size
    private static String getFileSize(final File file)
    {
        String ret;
        long size = file.length();
        float fSize;
        if (size < 1024 * 1024) // KB
        {
            fSize = (float) size / (1024);
            ret = String.format("%.2f KB", fSize);
        }
        else if (size < 1024 * 1024 * 1024) // MB
        {
            fSize = (float) size / (1024 * 1024);
            ret = String.format("%.2f MB", fSize);
        }
        else // GB
        {
            fSize = (float) size / (1024 * 1024 * 1024);
            ret = String.format("%.2f GB", fSize);
        }
        return ret;
    }

    public void selectAll(boolean checked) {
        for (int i = 0; i < mFileCheckedList.size(); ++i) {
            mFileCheckedList.set(i, Boolean.valueOf(checked));
        }
    }

    public void selectItem(int position) {
        mFileCheckedList.set(position,
                Boolean.valueOf(!mFileCheckedList.get(position).booleanValue()));
    }

    public boolean isAllSelect() {
        for (int i = 0; i < mFileCheckedList.size(); ++i) {
            if (!mFileCheckedList.get(i).booleanValue())
                return false;
        }
        return true;
    }

    public int getSelectCnt() {
        int total = 0;
        for (int i = 0; i < mFileCheckedList.size(); ++i) {
            if (mFileCheckedList.get(i).booleanValue())
                total += 1;
        }
        return total;
    }

}
