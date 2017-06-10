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

import android.app.ListActivity;
import android.widget.AbsListView;
import android.widget.AdapterView;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.LinearLayout;
import android.widget.ListView;
import android.widget.SpinnerAdapter;
import android.widget.TextView;
import android.app.ActionBar;
import android.app.Activity;
import android.app.AlertDialog;
import android.content.Intent;
import android.os.Bundle;
import android.view.Gravity;
import android.view.MenuInflater;
import android.view.ViewGroup;
import android.view.Window;
import android.view.View;
import android.view.ContextMenu;
import android.view.KeyEvent;
import android.view.Menu;
import android.view.MenuItem;
import android.view.ContextMenu.ContextMenuInfo;
import android.view.ViewGroup.LayoutParams;

import java.io.File;
import java.io.InputStream;

import com.android.backup.RecoverOperateActivity.SelectAdapter;

import android.content.DialogInterface;
import android.content.DialogInterface.OnClickListener;
import android.content.SharedPreferences;

import android.util.Log;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.app.ProgressDialog;
import android.content.Context;
import android.database.DataSetObserver;
import android.app.Dialog;
import android.view.LayoutInflater;

import android.widget.Toast;

import static com.android.backup.HelpActivity.HELP_TYPE_KEY;
import static com.android.backup.HelpActivity.TYPE_SETTING;

import static com.android.backup.HelpActivity.*;

public class RecoverAppActivity extends ListActivity implements View.OnClickListener {

    private static final String TAG = "RecoverAppActivity";
    private static final boolean LOCAL_DEBUG = true;

    public static final String CURRENT_DIR_STR = "android.com.backup.directory";
    private static final String BACKUP_APP = "/backup/App/";
//    private static final String DEFAULT_BACKUP_ROOT_DIR = BackupUtils.strSDCardPath
//            + "/backup/App/";

    public static final int DEFAULT_MODE = 0;
    public static final int PICKER_DELETE_MODE = 1;

    private static final int CONTEXT_MENU_ID_DELETE = Menu.FIRST/* +1 */;

    private static final int MENU_ID_SELECT_DEL = Menu.FIRST + 5;
    private static final int MENU_ID_SELECT_DEL_CONFIRM = Menu.FIRST + 6;
    private static final int MENU_ID_SELECT_DEL_SELECT_ALL = Menu.FIRST + 7;
    private static final int MENU_ID_SELECT_DEL_CANCEL_SELECT = Menu.FIRST + 8;

    private static final int EVENTRE_STORE_APPDATA = 103;
    private static final int EVENTRE_CANCEL_PROGRESSDIALOG = 104;
    private static final int EVENTRE_PREPARE_DIALOG = 105;
    private static final int EVENTRE_STORE_APP_PERCENT = 106;
    private static final int EVENTRE_STORE_APP_DELETE = 107;
    private static final int EVENTRE_STORE_APP_DELETE_PER = 108;
    private static final int EVENTRE_STORE_APP_DELETE_FIN = 109;

    private static final int FALLBACK_RESTORE = 0;
    private static final int PATH_INTERNAL = 0;
    private static final int PATH_EXTERNAL = 1;

    private static final String PREFERENCE_RESTORE_APP_ACTIVITY = "Preference_Restore_App_Activity";
    private static final String PREFERENCE_RESTORE_APP_SHOW_DIALOG_KEY = "Show_Dialog_Key";

    private String mRecoverPath;
    private String mBackupRootDir;
    private CheckBox mCheckBox;
    private ListView mListView;
    private Button mRestoreBtn;
    private SelectAdapter mAdapter;
    private RecoverAppListAdapter mOnlyAdapter;
    private ProgressDialog mSendProgressDialog = null;
    private boolean mMultiDelete = false;
    public boolean mCancel = false;
    public Handler mHandler = new restoreHandler();

    private class restoreHandler extends Handler {
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case EVENTRE_STORE_APPDATA: {
                    if (true) {
                        if (mSendProgressDialog != null) {
                            mSendProgressDialog.dismiss();
                        }
                        mSendProgressDialog = new Showconnectprogress(RecoverAppActivity.this);
                        mSendProgressDialog.setTitle(R.string.restore_title);
                        mSendProgressDialog.setProgressStyle(ProgressDialog.STYLE_HORIZONTAL);
                        mSendProgressDialog.setCanceledOnTouchOutside(false);
                        mSendProgressDialog.setMax(mOnlyAdapter.getSelectCnt());
                        mSendProgressDialog.setProgress(0);
                        mSendProgressDialog.setCancelable(true);
                        mSendProgressDialog.setButton(ProgressDialog.BUTTON_NEGATIVE,
                                getString(R.string.button_stop),
                                new DialogInterface.OnClickListener() {

                                    @Override
                                    public void onClick(DialogInterface dialog, int which) {
                                        ((Showconnectprogress) mSendProgressDialog).LockedView
                                            = true;
                                        new AlertDialog.Builder(RecoverAppActivity.this,
                                                android.R.style.Theme_DeviceDefault_Dialog_NoActionBar)
                                                .setMessage(R.string.confirm_stop_restore)
                                                .setNegativeButton(R.string.button_cancel, null)
                                                .setPositiveButton(R.string.button_ok,
                                                        new DialogInterface.OnClickListener() {
                                                            public void onClick(
                                                                    DialogInterface dialog,
                                                                    int whichButton) {
                                                                mCancel = true;
                                                                if (mSendProgressDialog != null) {
                                                                    mSendProgressDialog.dismiss();
                                                                    mSendProgressDialog = null;
                                                                }
                                                            }
                                                        }).show();
                                    }
                                });
                        mSendProgressDialog.show();
                    }
                    Thread t = new Thread() {
                        public void run() {
                            mOnlyAdapter.operateMultiRestore(RecoverAppActivity.this);
                        }
                    };
                    t.start();
                    break;
                }
                case EVENTRE_CANCEL_PROGRESSDIALOG: {
                    if (mSendProgressDialog != null) {
                        mSendProgressDialog.dismiss();
                    }
                    mAdapter.updateTitle();
                    if (mCancel)
                        mCancel = false;
                    else
                        Toast.makeText(RecoverAppActivity.this, getString(R.string.restore_ok,
                                mRecoverPath.equals(BackupUtils.strExternalPath)?
                                        getString(R.string.external_app_content) :
                                        getString(R.string.internal_app_content)),
                                Toast.LENGTH_SHORT)
                                .show();
                    break;
                }
                case EVENTRE_STORE_APP_PERCENT: {
                    if (mSendProgressDialog != null) {
                        mSendProgressDialog.setProgress(msg.arg1);
                    }
                    break;
                }
                case EVENTRE_PREPARE_DIALOG: {
                    if (mSendProgressDialog != null) {
                        mSendProgressDialog.dismiss();
                    }
                    mSendProgressDialog = new Showconnectprogress(RecoverAppActivity.this);
                    mSendProgressDialog.setTitle(R.string.prepare_restore_app);
                    mSendProgressDialog.setProgressStyle(ProgressDialog.STYLE_SPINNER);
                    mSendProgressDialog.setCanceledOnTouchOutside(false);
                    mSendProgressDialog.show();
                    break;
                }
                case EVENTRE_STORE_APP_DELETE: {
                    mSendProgressDialog = new ProgressDialog(RecoverAppActivity.this,
                            com.android.internal.R.style.Theme_Holo_Dialog_Alert);
                    mSendProgressDialog.setTitle(R.string.delete_title);
                    mSendProgressDialog.setProgressStyle(ProgressDialog.STYLE_HORIZONTAL);
                    mSendProgressDialog.setCanceledOnTouchOutside(false);
                    mSendProgressDialog.setMax(msg.arg1);
                    mSendProgressDialog.setProgress(0);
                    mSendProgressDialog.show();
                    new Thread() {
                        public void run() {
                            Looper.prepare();
                            RecoverAppActivity.this.mOnlyAdapter
                                    .operateMultiDelete(RecoverAppActivity.this);
                            Looper.loop();
                        }
                    }.start();
                    break;
                }
                case EVENTRE_STORE_APP_DELETE_PER: {
                    if (mSendProgressDialog != null) {
                        mSendProgressDialog.setProgress(msg.arg1);
                    }
                    break;
                }
                case EVENTRE_STORE_APP_DELETE_FIN: {
                    if (mSendProgressDialog != null) {
                        mSendProgressDialog.dismiss();
                        mSendProgressDialog = null;
                    }
                    RecoverAppActivity.this.mOnlyAdapter.notifyDataSetChanged();
                    if (msg.arg1 > 0)
                        Toast.makeText(RecoverAppActivity.this,
                                RecoverAppActivity.this.getString(R.string.delete_ok),
                                Toast.LENGTH_SHORT).show();
                    else
                        Toast.makeText(RecoverAppActivity.this,
                                RecoverAppActivity.this.getString(R.string.delete_failed),
                                Toast.LENGTH_SHORT).show();
                    Log.d(TAG, "mOnlyAdapter.getSelectCnt() = " + mOnlyAdapter.getSelectCnt());
                    mAdapter.updateTitle();
                    if (mOnlyAdapter.getSelectCnt() > 0)
                        mRestoreBtn.setEnabled(true);
                    else {
                        mRestoreBtn.setEnabled(false);
                        getWindow().invalidatePanelMenu(Window.FEATURE_OPTIONS_PANEL);
                    }
                    break;
                }
                default:
                    break;
            }
        }
    }

    @Override
    protected void onCreate(Bundle icicle) {
        super.onCreate(icicle);
        setContentView(R.layout.recovery_main);
        getListView().setChoiceMode(ListView.CHOICE_MODE_MULTIPLE);

        mRecoverPath = getRestorePath();
        mBackupRootDir = mRecoverPath + BACKUP_APP;

        mOnlyAdapter = new RecoverAppListAdapter(this, mHandler, mRecoverPath);
        mOnlyAdapter.sortImpl();
        mOnlyAdapter.notifyDataSetChanged();
        setListAdapter(mOnlyAdapter);
        mOnlyAdapter.selectAll(true);
        getListView().setOnCreateContextMenuListener(this);

        ActionBar actionBar = getActionBar();
        actionBar.setDisplayHomeAsUpEnabled(true);
        actionBar.setNavigationMode(ActionBar.NAVIGATION_MODE_LIST);
        mAdapter = new SelectAdapter();
        actionBar.setListNavigationCallbacks(mAdapter, mAdapter);
        setTitle("");
        mRestoreBtn = (Button) findViewById(R.id.recovery_btn);
        mRestoreBtn.setOnClickListener(this);
        //mRestoreBtn.setEnabled(false);
        mListView = getListView();

        if(mListView.getCount() == 0) {
            mRestoreBtn.setEnabled(false);
            final SharedPreferences sharedPreferences =
                    getSharedPreferences(PREFERENCE_RESTORE_APP_ACTIVITY, Context.MODE_PRIVATE);
            if (sharedPreferences.getBoolean(PREFERENCE_RESTORE_APP_SHOW_DIALOG_KEY, true)) {
                LayoutInflater layoutInflater = LayoutInflater.from(this);
                View dialogView = layoutInflater.inflate(R.layout.dialog_style, null);
                mCheckBox = (CheckBox) dialogView.findViewById(R.id.checkbox);
                new AlertDialog.Builder(RecoverAppActivity.this,
                        android.R.style.Theme_DeviceDefault_Dialog_NoActionBar)
                        .setMessage(BackupUtils.isSDSupported()?
                                R.string.no_sdcard_restore_abstract :
                                R.string.not_support_sdcard_restore_abstract)
                        .setView(dialogView)
                        .setNegativeButton(R.string.button_cancel, new DialogInterface.OnClickListener() {

                            @Override
                            public void onClick(DialogInterface dialog, int which) {

                                if (mCheckBox.isChecked()) {
                                    SharedPreferences.Editor editor = sharedPreferences.edit();
                                    editor.putBoolean(PREFERENCE_RESTORE_APP_SHOW_DIALOG_KEY, false);
                                    editor.commit();
                                }
                            }
                        })
                        .setPositiveButton(R.string.button_help, new DialogInterface.OnClickListener() {

                            @Override
                            public void onClick(DialogInterface dialog, int which) {

                                Intent intent = new Intent();
                                intent.setClass(RecoverAppActivity.this, HelpActivity.class);
                                intent.putExtra(HELP_TYPE_KEY, TYPE_RESTORE);
                                startActivity(intent);
                            }
                        }).create().show();
            }
        }
    }

    private String getRestorePath() {
        SharedPreferences sharedPreferences = getSharedPreferences(
                BackupUtils.LOCATION_PREFERENCE_NAME, Context.MODE_PRIVATE);
        String restorePath;
        if (BackupUtils.isSDSupported()) {
            if (BackupUtils.isSDMounted()) {
                if (sharedPreferences.getInt(BackupUtils.KEY_RESTORE_LOCATION, FALLBACK_RESTORE)
                        == PATH_EXTERNAL)
                    restorePath = BackupUtils.strExternalPath;
                else
                    restorePath = BackupUtils.strInternalPath;
            } else {
                restorePath = BackupUtils.strInternalPath;
            }
        } else {
            restorePath = BackupUtils.strInternalPath;
        }
        return restorePath;
    }

    class SelectAdapter implements SpinnerAdapter, ActionBar.OnNavigationListener {

        private static final int INIT_DATA = -1;
        private static final int SELECTED_DATA = 0;
        private static final int DEFAULT_DATA = 2;

        private int mPreState = INIT_DATA;

        private TextView mTitleTextView;
        private View mTitleView;
        private View mDropView;
        private TextView mTextView;

        public void updateTitle() {
            PrepareObj();
            mTitleTextView.setText(getTitle());
            mTitleTextView.invalidate();
        }

        @Override
        public boolean onNavigationItemSelected(int itemPosition, long itemId) {
            if (mPreState == INIT_DATA) {
                mPreState = itemPosition;
            } else if (mPreState == DEFAULT_DATA && itemPosition == SELECTED_DATA) {
                RecoverAppActivity.this.getActionBar().setSelectedNavigationItem(SELECTED_DATA);
                mPreState = INIT_DATA;
            }
            return true;
        }

        @Override
        public int getCount() {
            return 1;
        }

        @Override
        public Object getItem(int position) {
            return getTitle();
        }

        private int getSelectCnt()
        {
            return mOnlyAdapter.getSelectCnt();
        }

        private String getTitle()
        {
            String title = RecoverAppActivity.this.getString(R.string.menu_delete_sel);
            return "     " + getSelectCnt() + " " + title;
        }

        private void PrepareObj()
        {
            if (mTitleView == null) {
                mTitleView = new LinearLayout(RecoverAppActivity.this);
                ((LinearLayout) mTitleView).setOrientation(LinearLayout.VERTICAL);
                mTitleTextView = new TextView(RecoverAppActivity.this);
                LinearLayout.LayoutParams params = new LinearLayout.LayoutParams(
                        LayoutParams.FILL_PARENT, LayoutParams.FILL_PARENT);
                //params.topMargin = 20;
                mTitleTextView.setLayoutParams(params);
                ((LinearLayout) mTitleView).addView(mTitleTextView);
                mTitleView.setLayoutParams(new LinearLayout.LayoutParams(LayoutParams.FILL_PARENT,
                        LayoutParams.FILL_PARENT));
            }
        }

        @Override
        public long getItemId(int position) {
            return (long) position;
        }

        @Override
        public int getItemViewType(int position) {
            return 0;
        }

        @Override
        public View getView(int position, View convertView, ViewGroup parent) {
            PrepareObj();
            mTitleTextView.setText(getTitle());
            return mTitleView;
        }

        @Override
        public int getViewTypeCount() {
            return 1;
        }

        @Override
        public boolean hasStableIds() {
            return false;
        }

        @Override
        public boolean isEmpty() {
            return false;
        }

        @Override
        public void registerDataSetObserver(DataSetObserver observer) {
            // Set the default selected navigation item in list, 0 for the first item
            RecoverAppActivity.this.getActionBar().setSelectedNavigationItem(DEFAULT_DATA);
        }

        @Override
        public void unregisterDataSetObserver(DataSetObserver observer) {
            int itemIndex = RecoverAppActivity.this.getActionBar().getSelectedNavigationIndex();
            if (mPreState == DEFAULT_DATA && mPreState == itemIndex) {
                RecoverAppActivity.this.getActionBar().setSelectedNavigationItem(SELECTED_DATA);
            } else if (itemIndex == SELECTED_DATA) {
                if (mOnlyAdapter.isAllSelect()) {
                    mOnlyAdapter.selectAll(false);
                } else {
                    mOnlyAdapter.selectAll(true);
                }
                mOnlyAdapter.notifyDataSetChanged();
                if (mOnlyAdapter.getSelectCnt() > 0)
                    mRestoreBtn.setEnabled(true);
                else
                    mRestoreBtn.setEnabled(false);
            }
        }

        @Override
        public View getDropDownView(int position, View convertView, ViewGroup parent) {
            if (mDropView == null) {
                mDropView = new LinearLayout(RecoverAppActivity.this);
                ((LinearLayout) mDropView).setOrientation(LinearLayout.HORIZONTAL);
                mTextView = new TextView(RecoverAppActivity.this);
                LinearLayout.LayoutParams params = new LinearLayout.LayoutParams(
                        LayoutParams.FILL_PARENT, LayoutParams.WRAP_CONTENT);
                params.setMargins(20, 0, 0, 0);
                params.gravity = Gravity.CENTER_VERTICAL;
                mTextView.setLayoutParams(params);
                ((LinearLayout) mDropView).addView(mTextView);
                mDropView.setLayoutParams(new LinearLayout.LayoutParams(LayoutParams.MATCH_PARENT,
                        LayoutParams.MATCH_PARENT));
            }
            if (mTitleView != null)
                mDropView.setLayoutParams(new AbsListView.LayoutParams(
                        mTitleView.getWidth() * 5 / 4,
                        mTitleView.getHeight() * 5 / 4));
            if (mOnlyAdapter.isAllSelect())
                mTextView.setText(R.string.menu_deselect_all);
            else
                mTextView.setText(R.string.menu_select_all);
            return mDropView;
        }
    }

    @Override
    protected void onResume() {
        super.onResume();
    }

    /**
     * When context menu item is clicked, the function is called
     */
    public boolean onContextItemSelected(MenuItem item)
    {
        AdapterView.AdapterContextMenuInfo info;
        try
        {
            info = (AdapterView.AdapterContextMenuInfo) item.getMenuInfo();
        } catch (ClassCastException e)
        {
            Log.d(TAG, "onContextItemSelected bad menuInfo", e);
            return false;
        }

        // Get clicked item's content
        final String fileName = (String) mOnlyAdapter.getItem(info.position);
        Log.d(TAG, "onContextItemSelected fileName: " + fileName);

        switch (item.getItemId())
        {
            case CONTEXT_MENU_ID_DELETE: {
                Log.d(TAG, "onContextItemSelected CONTEXT_MENU_ID_DELETE");
                final File tmpdir = new File(mBackupRootDir + fileName);
                String[] fileNamePrefix = fileName.split(".apk");
                final File tmpTarFile = new File(mBackupRootDir + fileNamePrefix[0]
                        + ".tar");
                AlertDialog alertdiag = new AlertDialog.Builder(this)
                        .setIcon(android.R.drawable.ic_dialog_alert)
                        .setTitle(R.string.delete_confirm_alert_title)
                        .setMessage(R.string.delete_confirm_alert_msg)
                        .setNegativeButton(R.string.button_cancel, null)
                        .setPositiveButton(R.string.button_ok, new OnClickListener() {
                            public void onClick(DialogInterface dlg, int which)
                            {
                                deleteDir(tmpdir);
                                deleteDir(tmpTarFile);
                                mOnlyAdapter.sortImpl();
                                mOnlyAdapter.notifyDataSetChanged();
                            }
                        }).create();
                alertdiag.show();
                return true;
            }

        }

        return false;
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

    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {
        Log.d(TAG, "keyCode=" + keyCode);
        return super.onKeyDown(keyCode, event);
    }

    @Override
    protected void onListItemClick(ListView l, View v, int position, long id)
    {
        Log.d(TAG, "onListItemClick:" + position);
        super.onListItemClick(l, v, position, id);
        mOnlyAdapter.selectItem(position);
        mOnlyAdapter.notifyDataSetChanged();
        mAdapter.updateTitle();
        if (mOnlyAdapter.getSelectCnt() > 0)
            mRestoreBtn.setEnabled(true);
        else
            mRestoreBtn.setEnabled(false);
    }

    /**
     * Handle multi delete button click event View.OnClickListener.onClick()
     */
    public void onClick(View v) {
        if (v instanceof Button) {
            if (!BackupUtils.isSDMounted()) {
                try {
                    Log.d(TAG, "get location start");
                    Process setLocationProcess = Runtime.getRuntime().exec("pm get-install-location");
                    setLocationProcess.waitFor();
                    InputStream result = setLocationProcess.getInputStream();
                    byte[] buffer = new byte[20];
                    result.read(buffer);
                    if (buffer[0] == '2') {
                        Toast.makeText(getApplicationContext(), R.string.location_error, 2).show();
                        return;
                    }
                } catch (Exception e) {
                   Log.d(TAG, "set location error");
                }
                Log.d(TAG, "get location end");
            }
            if (mRecoverPath.equals(BackupUtils.strExternalPath) && !BackupUtils.isSDMounted()) {
                Log.d(TAG, "on recover path changed");
                Toast.makeText(getApplicationContext(), R.string.recover_path_change_content, 2)
                        .show();
                return;
            }
            if (this.mOnlyAdapter.getSelectCnt() > 0) {
                new AlertDialog.Builder(RecoverAppActivity.this,
                        android.R.style.Theme_DeviceDefault_Dialog_NoActionBar)
                        .setMessage(R.string.restore_app_confirm)
                        .setNegativeButton(R.string.button_cancel, null)
                        .setPositiveButton(R.string.button_ok,
                                new DialogInterface.OnClickListener() {
                                    public void onClick(DialogInterface dialog,
                                            int whichButton) {
                                        mHandler.sendMessage(mHandler
                                                .obtainMessage(EVENTRE_STORE_APPDATA));
                                    }
                                }).show();
            } else {
                Log.d(TAG, "on MENU_MULTISEL_OK no Item Checked");
                Toast.makeText(getApplicationContext(), R.string.stop_no_item_selected, 2)
                        .show();
            }
        }
    }

    // Return if multi-delete state
    public boolean getMultiDelFlag() {
        return mMultiDelete;
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu)
    {
        MenuInflater inflater = getMenuInflater();
        if (mOnlyAdapter.getCount() > 0)
            inflater.inflate(R.menu.list_single_select, menu);
        return super.onCreateOptionsMenu(menu);
    }

    public boolean onOptionsItemSelected(MenuItem item)
    {
        Log.d(TAG, "onOptionsItemSelected item" + item);

        if (mOnlyAdapter.onOptionsItemSelected(this, item))
            return true;
        else
            return super.onOptionsItemSelected(item);
    }

    private class Showconnectprogress extends ProgressDialog {
        private Context mContext;
        private boolean LockedView = false;

        public Showconnectprogress(Context context) {
            this(context, com.android.internal.R.style.Theme_Holo_Dialog_Alert);
            mContext = context;
        }

        public Showconnectprogress(Context context, int theme) {
            super(context, theme);
            mContext = context;
        }

        public void dismiss() {
            if (!LockedView)
                super.dismiss();
            else
                LockedView = false;
        }

        @Override
        public boolean onKeyDown(int keyCode, KeyEvent event) {

            if (LOCAL_DEBUG)
                Log.v(TAG, "Showconnectprogress keyCode " + keyCode);
            if ((mSendProgressDialog != null) && (keyCode == KeyEvent.KEYCODE_BACK)) {

                new AlertDialog.Builder(RecoverAppActivity.this,
                        android.R.style.Theme_DeviceDefault_Dialog_NoActionBar)
                        .setMessage(R.string.confirm_stop_restore)
                        .setNegativeButton(R.string.button_cancel, null)
                        .setPositiveButton(R.string.button_ok,
                                new DialogInterface.OnClickListener() {
                                    public void onClick(DialogInterface dialog, int whichButton)
                                    {
                                        mCancel = true;
                                    }
                                })
                        .show();
            }
            if ((mSendProgressDialog != null) && (keyCode == KeyEvent.KEYCODE_SEARCH)) {
                return true;
            }
            return super.onKeyDown(keyCode, event);
        }
    }

}
