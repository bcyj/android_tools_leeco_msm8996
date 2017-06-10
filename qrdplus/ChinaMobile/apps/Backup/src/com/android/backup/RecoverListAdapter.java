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
import android.os.Handler;
import android.os.Message;

// multi_delete
import android.widget.CheckBox;
import android.app.ListActivity;
import android.app.Activity;
import android.app.AlertDialog;

import android.content.DialogInterface;
import android.content.DialogInterface.OnClickListener;

import android.util.Log;

public class RecoverListAdapter extends BaseAdapter
{
    private static final String TAG = "RecoverListAdapter";
    private static final String BACKUP_DATA_DIR = "/backup/Data";
    private static final String OLD_BACKUP_DATA_DIR = "/.backup/";
//    private static final String DEFAULT_BACKUP_ROOT_DIR = BackupUtils.strSDCardPath
//            + "/backup/Data";
//    private static final String OLD_DEFAULT_BACKUP_ROOT_DIR = BackupUtils.strSDCardPath
//            + "/.backup/";
    private String mRecoverPath;
    private String mBackupRootDir;
    private String mOldBackupRootDir;
    private Context mContext;
    private LayoutInflater mInflater;
    private ArrayList<String> mFileNameList;
    private ArrayList<Boolean> mFileSelectList;
    private File mCurrentDir;
    private File oldCurrentDir;

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

    private static final int EVENT_DELETE_START = 100;
    private static final int EVENT_DELETE_END = 101;

    private Handler mDelHandler;
    public int mMode;
    private int mMultiFlag = MULTI_FLAG_DEFAULT;
    private ListActivity mListActivity;

    public static class ViewHolder
    {
        TextView filename;
        TextView subFileNum;
        // for multi-select
        CheckedTextView check;
    }

    /**
     * This Comparator sorts strings into increasing order.
     */
    public static class TheComparator implements Comparator<String>
    {
        public int compare(String str1, String str2)
        {
            return str2.compareTo(str1);
        }
    }

    // Constructor 2
    public RecoverListAdapter(Context context, Handler delHandler, String path)
    {
        mContext = context;
        mDelHandler = delHandler;
        mListActivity = (ListActivity) context;
        mInflater = LayoutInflater.from(mContext);
        mMode = DEFAULT_MODE;
        mRecoverPath = path;
        mBackupRootDir = mRecoverPath + BACKUP_DATA_DIR;
        mOldBackupRootDir = mRecoverPath + OLD_BACKUP_DATA_DIR;
    }

    public void selectItems(int position, boolean select)
    {
        mFileSelectList.set(position, Boolean.valueOf(select));
        notifyDataSetChanged();
    }

    public boolean isCheckItems(int position)
    {
        return mFileSelectList.get(position).booleanValue();
    }

    public void sortImpl()
    {
        mCurrentDir = new File(mBackupRootDir);
        oldCurrentDir = new File(mOldBackupRootDir);
        String[] mFileNameArray;
        File[] oldFileArray = oldCurrentDir.listFiles(FILENAME_FILTER);
        if (oldFileArray != null && oldFileArray.length != 0) {
            Log.d(TAG, "  oldFileArray!=null&&oldFileArray.length!=0");
            String[] oldFileArrayName = new String[oldFileArray.length];
            int i = 0;
            for (File f : oldFileArray) {
                String[] mTemp = f.toString().split("/");
                oldFileArrayName[i] = mTemp[mTemp.length - 1];
                Log.d(TAG, "oldFileArrayName[i]=" + oldFileArrayName[i]);
                i++;
            }

            if (mCurrentDir != null && mCurrentDir.exists()) {
                mFileNameArray = mCurrentDir.list();
                String[] myFileNameArray = new String[mFileNameArray.length
                        + oldFileArrayName.length];
                int mNum = 0;
                for (String str : mFileNameArray) {
                    myFileNameArray[mNum] = str;
                    mNum++;
                }
                for (String str : oldFileArrayName) {
                    myFileNameArray[mNum] = str;
                    mNum++;
                }
                Log.d(TAG, "FileListAdapter:myFileNameArray: " + myFileNameArray);
                mFileNameList = getSortedFileNameArray(mCurrentDir, myFileNameArray);
            }
            else
            {
                mFileNameList = getSortedFileNameArray(mCurrentDir, oldFileArrayName);
            }

        } else {

            if (mCurrentDir != null && mCurrentDir.exists()) {
                mFileNameArray = mCurrentDir.list();
                Log.d(TAG, "FileListAdapter:mFileNameArray: " + mFileNameArray);
                mFileNameList = getSortedFileNameArray(mCurrentDir, mFileNameArray);
            }
            else
            {
                mFileNameList = new ArrayList<String>();
                mCurrentDir = null;
            }

        }
        mFileSelectList = new ArrayList<Boolean>();
        for (String str : mFileNameList)
        {
            mFileSelectList.add(Boolean.FALSE);
        }
        notifyDataSetChanged();
    }

    private static final FilenameFilter FILENAME_FILTER = new FilenameFilter() {

        public boolean accept(File dir, String name) {
            if (name.toLowerCase().endsWith(".zip")) {
                Log.d(TAG, "  endsWith(.zip)");
                return true;
            }
            return false;
        }
    };

    public void setShowMode(int mode)
    {
        mMode = mode;

    }

    public int getShowMode()
    {
        return mMode;
    }

    public boolean handleBackKeyAction(int keyCode, KeyEvent event)
    {
        switch (keyCode)
        {
            case KeyEvent.KEYCODE_BACK:
                if (mMode == PICKER_DELETE_MODE)
                {
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

    public int getCount()
    {
        if (mFileNameList != null) {
            return mFileNameList.size();
        } else {
            return 0;
        }
    }

    public Object getItem(int position)
    {
        if (mFileNameList != null) {
            return mFileNameList.get(position);
        } else {
            return null;
        }
    }

    public long getItemId(int position)
    {
        return position;
    }

    public View getView(int position, View convertView, ViewGroup parent)
    {
        // multi_delete
        boolean multiDel = false;
        if (mContext instanceof RecoverActivity) {
            multiDel = ((RecoverActivity) mContext).getMultiDelFlag();
        }

        if (mCurrentDir == null)
        {
            return null;
        }

        ViewHolder holder;
        if (convertView == null)
        {
            convertView = mInflater.inflate(R.layout.list_item, null);
            holder = new ViewHolder();

            holder.filename = (TextView) convertView.findViewById(R.id.file_name);
            holder.subFileNum = (TextView) convertView.findViewById(R.id.sub_file_num);
            holder.check = (CheckedTextView) convertView.findViewById(R.id.check);
            convertView.setTag(holder);
        }
        else
        {
            holder = (ViewHolder) convertView.getTag();
        }

        // 1. Bind file name
        String filename = mFileNameList.get(position);
        if (position == 0)
            filename = filename + "(" + mContext.getString(R.string.latest) + ")";
        holder.filename.setText(filename);

        String mFullDir = mCurrentDir.toString() + "/" + filename;
        File mFile = new File(mFullDir);
        if (mMode == PICKER_DELETE_MODE) {
            holder.check.setVisibility(View.VISIBLE);
            holder.check
                    .setChecked(((ListActivity) mContext).getListView().isItemChecked(position));
        }
        else {
            holder.check.setVisibility(View.GONE);
        }

        if (mFileSelectList.get(position).booleanValue())
        {
            convertView.setBackgroundColor(
                    mContext.getResources().getColor(android.R.color.holo_blue_dark));
        }
        else
        {
            convertView.setBackgroundColor(
                    mContext.getResources().getColor(android.R.color.transparent));
        }
        return convertView;
    }

    private ArrayList<String> getSortedFileNameArray(final File theFile, String[] filenames)
    {
        ArrayList<String> fileFolderArray = new ArrayList<String>();

        if (filenames != null) {
            for (String s : filenames) {
                Log.d(TAG, "add to filelist " + s + " addable = " + (!(s.equals("path"))));
                if ((!(s.equals("path"))) && (!(s.equals("restorepath")))
                        && (!(s.equals("apppath"))) && (!(s.contains(".txt")))
                        && (!(s.contains(".crypt"))))
                    fileFolderArray.add(s);
            }

            try {
                Collections.sort(fileFolderArray, mAcsOrder);
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
        return fileFolderArray;
    }

    public boolean onPrepareOptionsMenu(Activity activity, Menu menu)
    {

        menu.clear();

        if (DEFAULT_MODE == mMode) {
            if (getCount() > 0) {
                menu.add(0, MENU_MULTI_DELETE, 0, R.string.multi_delete)
                        .setIcon(android.R.drawable.ic_menu_delete);
            }
        } else if (PICKER_DELETE_MODE == mMode) {
            // multi select ok
            menu.add(0, MENU_MULTISEL_OK, 0, R.string.menu_selectok)
                    .setIcon(R.drawable.ic_menu_ok);

            // multi select all
            menu.add(0, MENU_MULTISEL_ALL, 0, R.string.menu_selectall)
                    .setIcon(R.drawable.ic_menu_selectall);

            // multi select cancel
            menu.add(0, MENU_MULTISEL_CANCEL, 0, R.string.menu_clearall)
                    .setIcon(R.drawable.ic_menu_clearall);
        }

        return true;
    }

    public boolean onOptionsItemSelected(final RecoverActivity activity, MenuItem item)
    {
        final SparseBooleanArray array = activity.getListView().getCheckedItemPositions();
        int size = array.size();
        Log.d(TAG, "onOptionsItemSelected item" + item);

        switch (item.getItemId())
        {
            case MENU_MULTI_DELETE: {
                Log.d(TAG, "onOptionsItemSelected MENU_MULTI_DELETE");

                // mMultiFlag = MULTI_FLAG_MULTI_DELETE;
                mMode = PICKER_DELETE_MODE;
                // mMultiselMaxitems = 0xFFFF;

                notifyDataSetChanged();
                mListActivity.getListView().clearChoices();
                return true;
            }

            case MENU_MULTISEL_OK:
                Log.d(TAG, "onOptionsItemSelected MENU_MULTISEL_OK!!");

                if (size == 0) {
                    Log.d(TAG, "onOptionsItemSelected MENU_MULTISEL_OK  NO item(s) checked!");
                    Toast.makeText(mListActivity, R.string.stop_no_item_selected, 2).show();
                }
                else {
                    Log.d(TAG, "onOptionsItemSelected MENU_MULTISEL_OK item(s) checked!" + size);
                    AlertDialog alertdiag = new AlertDialog.Builder(activity)
                            .setIcon(android.R.drawable.ic_dialog_alert)
                            .setTitle(R.string.delete_confirm_alert_title)
                            .setMessage(R.string.delete_confirm_alert_msg)
                            .setNegativeButton(R.string.button_cancel, null)
                            .setPositiveButton(R.string.button_ok, new OnClickListener() {
                                public void onClick(DialogInterface dlg, int which)
                                {
                                    if (mDelHandler == null) {
                                        Log.d(TAG, "mDelHandler == null");
                                    }
                                    Message message = mDelHandler.obtainMessage(EVENT_DELETE_START);
                                    mDelHandler.sendMessage(message);
                                    new Thread() {
                                        public void run() {
                                            Log.d(TAG, "run");
                                            operateMultiDelete(activity);
                                            try {
                                                this.sleep(300);
                                            } catch (Exception e) {
                                                Log.e(TAG, " " + e);
                                            } finally {
                                                Message message = mDelHandler
                                                        .obtainMessage(EVENT_DELETE_END);
                                                mDelHandler.sendMessage(message);
                                            }
                                        }
                                    }.start();
                                }
                            }).create();
                    alertdiag.show();
                }
                return true;

            case MENU_MULTISEL_ALL:
                allSelect(activity);
                return true;

            case MENU_MULTISEL_CANCEL:
                clearSelect(activity);
                return true;

            default:
                return true;

        }
    }

    private void allSelect(RecoverActivity activity) {
        Log.d(TAG, "allSelect");
        int count = getCount();
        Log.d(TAG, "allSelect count" + count);
        for (int i = 0; i < count; i++) {
            activity.getListView().setItemChecked(i, true);
        }
        notifyDataSetChanged();
    }

    private void clearSelect(RecoverActivity activity) {
        Log.d(TAG, "clearSelect");
        activity.getListView().clearChoices();
        notifyDataSetChanged();
    }

    private void operateMultiDelete(RecoverActivity activity) {
        Log.d(TAG, "operateMultiDelete start");
        final SparseBooleanArray array = activity.getListView().getCheckedItemPositions();
        int size = array.size();
        for (int i = 0; i < size; i++)
        {
            boolean selected = array.valueAt(i);
            Log.d(TAG, "operateMultiDelete selected: " + selected);
            if (!selected) {
                continue;
            }
            int position = array.keyAt(i);
            String filename = (String) getItem(position);
            Log.d(TAG, "operateMultiDelete filename: " + filename);
            if (filename != null) {
                // sxm add 20121016
                if (filename.endsWith(".zip")) {
                    File old_del_dir = new File(mOldBackupRootDir + "/" + filename);
                    deleteDir(old_del_dir);
                } else {
                    // sxm add 20121016 End
                    File del_dir = new File(mBackupRootDir + "/" + filename);
                    deleteDir(del_dir);
                }
            }
        }
        mListActivity.getListView().clearChoices();
        // sortImpl();
    }

    private void deleteDir(File dir) {
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

}
