/*
 * Copyright (c) 2014 pci-suntektech Technologies, Inc.  All Rights Reserved.
 * pci-suntektech Technologies Proprietary and Confidential.
 */

package com.suntek.mway.rcs.nativeui.ui;

import com.suntek.mway.rcs.nativeui.R;
//import com.suntek.mway.rcs.api.SuntekSDKApi;
import com.suntek.mway.rcs.client.aidl.plugin.callback.IMContactSyncListener;
import com.suntek.mway.rcs.client.aidl.plugin.entity.mcontact.Auth;
import com.suntek.mway.rcs.client.aidl.plugin.entity.mcontact.ContactAction;
import com.suntek.mway.rcs.client.aidl.plugin.entity.mcontact.IntervalAction;
import com.suntek.mway.rcs.client.aidl.plugin.entity.mcontact.SyncAction;
import com.suntek.mway.rcs.client.api.util.ServiceDisconnectedException;
import com.suntek.mway.rcs.client.api.util.log.LogHelper;
import com.suntek.mway.rcs.nativeui.RcsApiManager;
import com.suntek.mway.rcs.nativeui.utils.RcsContactUtils;

import android.app.ActionBar;
import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.SharedPreferences;
import android.graphics.drawable.Drawable;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.net.wifi.WifiManager;
import android.os.Bundle;
import android.os.AsyncTask;
import android.os.Handler;
import android.os.RemoteException;
import android.preference.PreferenceManager;
import android.provider.Settings;
import android.text.TextUtils;
import android.util.Log;
import android.view.MenuItem;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.CompoundButton.OnCheckedChangeListener;
import android.widget.ImageView;
import android.widget.RelativeLayout;
import android.widget.TextView;
import android.widget.Toast;

import java.text.SimpleDateFormat;
import java.util.Date;

public class ContactsBackupRestoreActivity extends Activity implements OnClickListener {

    private TextView mBackupFrequency;
    private ImageView mIcon;
    private TextView mStatus;
    private CheckBox mOnlyWifiBackupRestoreCheckBox;
    //private TextView mOnlyWifiBackupTextView;
    private TextView mBackupOnceChangedText;
    private CheckBox mBackupOnceChangedCheckBox;
    private CheckBox mAutoBackup;
    private RelativeLayout mAutoBackupTime;
    private TextView mAutoBackupFreq;
    private IntervalAction mAutoBackupAction =IntervalAction.INTERVAL_SYNC_ONE_DAY;
    private Handler mHandler = new Handler();
    private static final String KEY_LAST_OPTION_TIME = "key_last_option_time";
    private static final String KEY_LAST_OPTION_TYPE = "key_last_option_type";
    private static final String KEY_AUTO_BACKUP_FREQ = "key_auto_backup_freq";
    private static final String KEY_BACKUP_ONCE_CHANGED = "key_backup_once_changed";
    private static final String KEY_AUTO_BACKUP = "key_auto_backup";
    private static final String PREF_BACKUP_RESTORE_NAME = "pref_backup_restore_name";
    //private static final String PREF_BACKUP_ONCE_CHANGED_NAME = "pref_backup_once_changed_name";
    private static final String KEY_ONLY_WIFI_BACKUP_RESOTORE = "key_only_wifi_backup_restore";
    //private static final String PREF_ONLY_WIFI_BACKUP_RESOTORE_NAME = "pref_only_wifi_backup_restore_name";
    private volatile int mLocalCount = -1;
    private static final int BACKUP = 0;
    private static final int RESTORE = 1;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_mcontact);
        ActionBar actionBar = getActionBar();
        actionBar.setDisplayUseLogoEnabled(false);
        actionBar.setDisplayHomeAsUpEnabled(true);
        actionBar.setDisplayShowHomeEnabled(false);
        Drawable myDrawable = getResources().getDrawable(
                R.color.public_account_bar_background_color);
        actionBar.setBackgroundDrawable(myDrawable);
        Thread t = new Thread() {
            @Override
            public void run() {
                if (!RcsContactUtils.isRcsConnection()) {
                    RcsContactUtils.sleep(500);
                    RcsContactUtils.setRcsConnectionState(true);
                }
                mHandler.post(new Runnable() {

                    @Override
                    public void run() {
                        // TODO Auto-generated method stub
                        try {
                            initView();
                        } catch (ServiceDisconnectedException e) {
                            e.printStackTrace();
                            Toast.makeText(
                                    ContactsBackupRestoreActivity.this,
                                    ContactsBackupRestoreActivity.this.getResources().getString(
                                            R.string.rcs_service_is_not_available),
                                    Toast.LENGTH_LONG).show();
                            finish();
                        }
                    }

                });
            }
        };
        t.start();
    }

    private void initView() throws ServiceDisconnectedException {
        mBackupFrequency = (TextView) findViewById(R.id.backup_frequency);
        mAutoBackupTime = (RelativeLayout) findViewById(R.id.auto_backup_time);
        mAutoBackupFreq = (TextView) findViewById(R.id.auto_backup_freq);
        mIcon = (ImageView) findViewById(R.id.icon);
        mStatus = (TextView) findViewById(R.id.status);
        //loadContactCount();
        SharedPreferences prefs = PreferenceManager
                .getDefaultSharedPreferences(this);
        String optionTime = prefs.getString(KEY_LAST_OPTION_TIME, "");
        if (!TextUtils.isEmpty(optionTime)) {
            if (prefs.getInt(KEY_LAST_OPTION_TYPE, 0) == BACKUP) {
                StringBuffer tempDate = new StringBuffer(getResources().getString(
                        R.string.last_backup_time));
                tempDate.append(optionTime);
                mStatus.setText(tempDate.toString());
            } else if (prefs.getInt(KEY_LAST_OPTION_TYPE, 0) == RESTORE) {
                StringBuffer tempDate = new StringBuffer(getResources().getString(
                        R.string.last_restore_time));
                tempDate.append(optionTime);
                mStatus.setText(tempDate.toString());
            }
        }
        mOnlyWifiBackupRestoreCheckBox = (CheckBox) findViewById(R.id.only_wifi_backup_checkbox);
        //mOnlyWifiBackupTextView = (TextView) findViewById(R.id.only_wifi_backup_textview);
        mBackupOnceChangedText = (TextView) findViewById(R.id.backup_once_changed_text);
        mBackupOnceChangedCheckBox = (CheckBox) findViewById(R.id.backup_once_changed_checkbox);
        mAutoBackup = (CheckBox) findViewById(R.id.auto_backup);
        final SharedPreferences backupRestorePrefs = getSharedPreferences(
                PREF_BACKUP_RESTORE_NAME, Activity.MODE_WORLD_READABLE | Activity.MODE_MULTI_PROCESS);
        //SharedPreferences onlyWifiBackupRestorePrefs = getSharedPreferences(
                //PREF_ONLY_WIFI_BACKUP_RESOTORE_NAME, Activity.MODE_WORLD_READABLE | Activity.MODE_MULTI_PROCESS);
        mOnlyWifiBackupRestoreCheckBox.setChecked(backupRestorePrefs.getBoolean(
                KEY_ONLY_WIFI_BACKUP_RESOTORE, false));
        //mOnlyWifiBackupRestoreCheckBox.setChecked(RcsApiManager.getMcontactApi()
                //.getOnlySyncEnableViaWifi());
        mOnlyWifiBackupRestoreCheckBox.setOnCheckedChangeListener(new OnCheckedChangeListener() {

            @Override
            public void onCheckedChanged(CompoundButton arg0, boolean check) {
                //SharedPreferences prefs = getSharedPreferences(
                        //PREF_ONLY_WIFI_BACKUP_RESOTORE_NAME, Activity.MODE_WORLD_READABLE | Activity.MODE_MULTI_PROCESS);
                SharedPreferences.Editor editor = backupRestorePrefs.edit();
                editor.putBoolean(KEY_ONLY_WIFI_BACKUP_RESOTORE, check);
                editor.apply();
                try {
                    RcsApiManager.getMcontactApi().setOnlySyncEnableViaWifi(check);
                } catch (ServiceDisconnectedException e) {
                    e.printStackTrace();
                    Toast.makeText(
                            ContactsBackupRestoreActivity.this,
                            ContactsBackupRestoreActivity.this.getResources().getString(
                                    R.string.rcs_service_is_not_available), Toast.LENGTH_LONG)
                            .show();
                }
            }
        });

        mBackupOnceChangedCheckBox.setOnCheckedChangeListener(new OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton arg0, boolean check) {
                @SuppressWarnings("deprecation")
                //SharedPreferences prefs = ContactsBackupRestoreActivity.this.getSharedPreferences(
                        //PREF_BACKUP_ONCE_CHANGED_NAME, Activity.MODE_WORLD_READABLE
                                //| MODE_MULTI_PROCESS);
                SharedPreferences.Editor editor = backupRestorePrefs.edit();
                editor.putBoolean(KEY_BACKUP_ONCE_CHANGED, check);
                editor.apply();
            }
        });
        //boolean isAutoBackup = RcsApiManager.getMcontactApi().getEnableAutoSync();
        /*mOnlyWifiBackupCheckBox.setClickable(isAutoBackup);
        int color = isAutoBackup ? ContactsBackupRestoreActivity.this.getResources().getColor(
                R.color.black) : ContactsBackupRestoreActivity.this.getResources().getColor(
                R.color.text_shadow_color_light_bold);
        mOnlyWifiBackupTextView.setTextColor(color);*/
        boolean isAutoBackup = backupRestorePrefs.getBoolean(KEY_AUTO_BACKUP, false);
        mAutoBackup.setChecked(isAutoBackup);

        int color = isAutoBackup ? ContactsBackupRestoreActivity.this
                .getResources().getColor(R.color.black) : ContactsBackupRestoreActivity.this
                .getResources().getColor(R.color.text_shadow_color_light_bold);
        //SharedPreferences backupOnceChangedPrefs = getSharedPreferences(
                //PREF_BACKUP_ONCE_CHANGED_NAME, Activity.MODE_WORLD_READABLE | Activity.MODE_MULTI_PROCESS);
        mBackupOnceChangedCheckBox.setChecked(backupRestorePrefs.getBoolean(
                KEY_BACKUP_ONCE_CHANGED, false));
        mBackupOnceChangedText.setTextColor(color);
        mBackupOnceChangedCheckBox.setClickable(isAutoBackup);

        mAutoBackupTime.setClickable(isAutoBackup);
        mAutoBackupFreq.setTextColor(isAutoBackup ? ContactsBackupRestoreActivity.this
                .getResources().getColor(R.color.black) : ContactsBackupRestoreActivity.this
                .getResources().getColor(R.color.text_shadow_color_light_bold));
        int autoBacupFrequence = prefs.getInt(KEY_AUTO_BACKUP_FREQ, 1);
        switch (autoBacupFrequence) {
            case 0:
                mAutoBackupAction = IntervalAction.INTERVAL_SYNC_ONE_DAY;
                break;
            case 1:
                mAutoBackupAction = IntervalAction.INTERVAL_SYNC_ONE_WEEK;
                break;
            case 2:
                mAutoBackupAction = IntervalAction.INTERVAL_SYNC_ONE_MONTH;
                break;
            default:
                break;
        }
        String[] items = getResources().getStringArray(R.array.contact_auto_backup_opera);
        mBackupFrequency.setText(items[autoBacupFrequence]);
        mAutoBackup.setClickable(true);
        mAutoBackup.setOnCheckedChangeListener(new OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton arg0, boolean check) {
                int color = check ? ContactsBackupRestoreActivity.this.getResources().getColor(
                        R.color.black) : ContactsBackupRestoreActivity.this.getResources()
                        .getColor(R.color.text_shadow_color_light_bold);
                color = check ? ContactsBackupRestoreActivity.this
                        .getResources().getColor(R.color.black)
                        : ContactsBackupRestoreActivity.this.getResources().getColor(
                                R.color.text_shadow_color_light_bold);
                mBackupOnceChangedText.setTextColor(color);
                mBackupOnceChangedCheckBox.setClickable(check);
                mAutoBackupTime.setClickable(check);
                mAutoBackupFreq.setTextColor(check ? ContactsBackupRestoreActivity.this
                        .getResources().getColor(R.color.black)
                        : ContactsBackupRestoreActivity.this.getResources().getColor(
                                R.color.text_shadow_color_light_bold));
                SharedPreferences.Editor editor = backupRestorePrefs.edit();
                editor.putBoolean(KEY_AUTO_BACKUP, check);
                editor.apply();
                try {
                    RcsApiManager.getMcontactApi().setEnableAutoSync(check,
                            SyncAction.CONTACT_UPLOAD);
                    RcsApiManager.getMcontactApi().cancelIntervalSync();
                } catch (ServiceDisconnectedException e) {
                    e.printStackTrace();
                    Toast.makeText(
                            ContactsBackupRestoreActivity.this,
                            ContactsBackupRestoreActivity.this.getResources().getString(
                                    R.string.rcs_service_is_not_available), Toast.LENGTH_LONG)
                            .show();
                }
                final boolean onlyWifiBackupChecked = mOnlyWifiBackupRestoreCheckBox.isChecked();
                if (check) {
                    Thread t = new Thread() {
                        @Override
                        public void run() {
                            try {
                                RcsApiManager.getMcontactApi().startIntervalSync(
                                        SyncAction.CONTACT_UPLOAD, mAutoBackupAction, 0);
                                RcsApiManager.getMcontactApi().setOnlySyncEnableViaWifi(
                                        onlyWifiBackupChecked);
                            } catch (ServiceDisconnectedException e) {
                                e.printStackTrace();
                                mHandler.post(new Runnable() {
                                    @Override
                                    public void run() {
                                        Toast.makeText(
                                                ContactsBackupRestoreActivity.this,
                                                ContactsBackupRestoreActivity.this
                                                        .getResources()
                                                        .getString(
                                                                R.string.rcs_service_is_not_available),
                                                Toast.LENGTH_LONG).show();
                                    }
                                });
                            }
                        }
                    };
                    t.start();
                }
            }
        });
    }

    private void loadContactCount() {
        try {
            mLocalCount = RcsApiManager.getMcontactApi().getLocalContactCounts();
            final int cloudCount = RcsApiManager.getMcontactApi().getRemoteContactCounts();
            runOnUiThread(new Runnable() {

                @Override
                public void run() {
                    mStatus.setText(getString(R.string.coutact_local_and_cloud_count, mLocalCount,
                            cloudCount));

                }
            });
        } catch (ServiceDisconnectedException e) {
            e.printStackTrace();
            mStatus.setText("");
        }
    }

    private boolean createOpenWifiDialog(final int workMode) {
        Log.d("RCS_UI", "Calling createOpenWifiDialog");
        Log.d("RCS_UI", "mOnlyWifiBackupRestoreCheckBox state: " + mOnlyWifiBackupRestoreCheckBox.isChecked());
        if (mOnlyWifiBackupRestoreCheckBox.isChecked()) {
            WifiManager wifiManager = (WifiManager)getSystemService(Context.WIFI_SERVICE);
            if (wifiManager.getWifiState() != WifiManager.WIFI_STATE_ENABLED) {
                String[] items;
                if (workMode == R.id.recover) {
                    items = new String[] {
                            getResources().getString(R.string.open_wlan),
                            getResources().getString(R.string.restore_via_data)
                    };
                } else {
                    items = new String[] {
                            getResources().getString(R.string.open_wlan),
                            getResources().getString(R.string.backup_via_data)
                    };
                }

                final boolean[] selectedItems = new boolean[] {
                        true, false
                };
                Dialog dialog = new AlertDialog.Builder(this).setTitle(getResources()
                        .getString(R.string.wlan_closed))
                        .setSingleChoiceItems(items, 0,
                                new android.content.DialogInterface.OnClickListener() {

                                    @Override
                                    public void onClick(DialogInterface dialog, int which) {
                                        if (which >= 0 && which < selectedItems.length) {
                                            for (int i = 0; i < selectedItems.length; i++) {
                                                selectedItems[i] = (which == i) ? true : false;
                                            }
                                        }
                                    }
                                })
                        .setNegativeButton(android.R.string.cancel, null)
                        .setPositiveButton(android.R.string.ok,
                                new DialogInterface.OnClickListener() {
                                    @Override
                                    public void onClick(DialogInterface dialog, int whichButton) {
                                        if (selectedItems.length == 2) {
                                            if (selectedItems[0]) {
                                                Intent intent = new Intent(
                                                        Settings.ACTION_WIFI_SETTINGS);
                                                startActivity(intent);
                                            } else {
                                                try {
                                                    if (R.id.backup == workMode) {
                                                        startSync(SyncAction.CONTACT_UPLOAD);
                                                    } else {
                                                        startSync(SyncAction.CONTACT_DOWNLOAD_APPEND);
                                                    }
                                                } catch (ServiceDisconnectedException e) {
                                                    e.printStackTrace();
                                                    toast(R.string.contact_service_error);
                                                }
                                            }
                                        }
                                        dialog.dismiss();
                                    }
                                }).create();
                dialog.show();
                return true;
            }
        }
        return false;
    }

    @Override
    public void onClick(View view) {
        switch (view.getId()) {
            case R.id.backup:
                if (createOpenWifiDialog(R.id.backup)) break;
                try {
                    startSync(SyncAction.CONTACT_UPLOAD);
                } catch (ServiceDisconnectedException e) {
                    toast(R.string.contact_service_error);
                    e.printStackTrace();
                }
                break;
            case R.id.recover:
                if (createOpenWifiDialog(R.id.recover)) break;
                try {
                    startSync(SyncAction.CONTACT_DOWNLOAD_APPEND);
                } catch (ServiceDisconnectedException e) {
                    e.printStackTrace();
                    toast(R.string.contact_service_error);
                }
                break;
            case R.id.auto_backup_time:
                showBackupTimeDialog(getCheckItem());
                break;
            default:
                break;
        }

    }

    @Override  
    public boolean onOptionsItemSelected(MenuItem item) {  
        switch (item.getItemId()) {
        case android.R.id.home:
            finish();
            return true;
        }
        return false;
    }

    private int getCheckItem() {

        if (IntervalAction.INTERVAL_SYNC_ONE_DAY.equals(mAutoBackupAction))
            return 0;
        else if (IntervalAction.INTERVAL_SYNC_ONE_WEEK.equals(mAutoBackupAction))
            return 1;
        else if (IntervalAction.INTERVAL_SYNC_ONE_MONTH.equals(mAutoBackupAction))
            return 2;
        else
            return 0;
    }

    private void showBackupTimeDialog(int checkedItem) {
        final String[] items = getResources().getStringArray(R.array.contact_auto_backup_opera);
        new AlertDialog.Builder(this)
                .setTitle(R.string.back_each)
                .setSingleChoiceItems(items, checkedItem, new DialogInterface.OnClickListener() {

                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        switch (which) {
                            case 0:
                                mAutoBackupAction = IntervalAction.INTERVAL_SYNC_ONE_DAY;
                                break;
                            case 1:
                                mAutoBackupAction = IntervalAction.INTERVAL_SYNC_ONE_WEEK;
                                break;
                            case 2:
                                mAutoBackupAction = IntervalAction.INTERVAL_SYNC_ONE_MONTH;
                                break;
                            default:
                                break;
                        }
                        if (mAutoBackup.isChecked()) {
                            try {
                                RcsApiManager.getMcontactApi()
                                        .startIntervalSync(SyncAction.CONTACT_UPLOAD,
                                                mAutoBackupAction, 0);
                            } catch (ServiceDisconnectedException e) {
                                e.printStackTrace();
                            }
                        }
                        mBackupFrequency.setText(items[which]);
                        SharedPreferences prefs = PreferenceManager
                                .getDefaultSharedPreferences(ContactsBackupRestoreActivity.this);
                        SharedPreferences.Editor editor = prefs.edit();
                        editor.putInt(KEY_AUTO_BACKUP_FREQ, getCheckItem());
                        editor.apply();
                        dialog.dismiss();
                    }
                }).show();
    }

    private void toast(int strId) {
        Toast.makeText(ContactsBackupRestoreActivity.this,
                getString(strId), Toast.LENGTH_LONG)
                .show();
    }

    private void startSync(SyncAction action) throws ServiceDisconnectedException {
        if (action == null)
            return;
        if (action.equals(SyncAction.CONTACT_DOWNLOAD)
                || action.equals(SyncAction.CONTACT_DOWNLOAD_APPEND)) {
            mIcon.setImageResource(R.drawable.recovering);
        } else if (action.equals(SyncAction.CONTACT_UPLOAD)
                || action.equals(SyncAction.CONTACT_UPLOAD_APPEND)) {
            mIcon.setImageResource(R.drawable.backuping);
        }
        RcsApiManager.getMcontactApi()
                .doSync(action, new IMContactSyncListener.Stub() {

            @Override
            public void onSync(Auth auto, final int action, final boolean isSuccess)
                    throws RemoteException {
                LogHelper.trace("onSync");
                loadContactCount();
                mHandler.post(new Runnable() {

                    @Override
                    public void run() {
                        if (isSuccess) {
                            mIcon.setImageResource(R.drawable.finish);
                            if (SyncAction.valueOf(action) == SyncAction.CONTACT_UPLOAD) {
                                toast(R.string.contact_backup_success);
                                updateLastOption(BACKUP);
                            } else {
                                toast(R.string.contact_recover_success);
                                updateLastOption(RESTORE);
                            }
                        } else {
                            mIcon.setImageResource(R.drawable.backup_idle);
                            if (SyncAction.valueOf(action) == SyncAction.CONTACT_UPLOAD) {
                                if (0 == mLocalCount) {
                                    toast(R.string.no_contact);
                                } else {
                                    toast(R.string.contact_backup_fail);
                                }
                            } else {
                                toast(R.string.contact_recover_fail);
                            }
                        }
                    }
                });
            }

            @Override
            public void onRunning() throws RemoteException {
                LogHelper.trace("onRunning");
                mHandler.post(new Runnable() {

                    @Override
                    public void run() {
                        toast(R.string.contact_sync_running);
                        mIcon.setImageResource(R.drawable.backup_idle);
                    }
                });
            }

            @Override
            public void onProgress(Auth arg0, final int ordinal, final int value, final int max)
                    throws RemoteException {
                LogHelper.trace("onProgress");
                mHandler.post(new Runnable() {
                    //
                    public void run() {
                        float percent = 0;
                        LogHelper.trace("onProgress");
                        ContactAction contactAction = ContactAction.valueOf(ordinal);
                        if (contactAction == ContactAction.CONTACT_ACTION_ADD) {
                            percent = (50 + ((float)value / max) * 30);
                            // updateDialog(percent, "濮濓絽婀崘娆忓弳閼辨梻閮存禍锟�;
                            mStatus.setText(getString(R.string.contact_write));
                        } else if (contactAction == ContactAction.CONTACT_ACTION_UPDATE) {
                            percent = (80 + ((float)value / max) * 15);
                            mStatus.setText(getString(R.string.contact_update));
                        } else if (contactAction == ContactAction.CONTACT_ACTION_DELETE) {
                            percent = (95 + ((float)value / max) * 5);
                            // updateDialog(percent, "濮濓絽婀崚鐘绘珟閼辨梻閮存禍锟�;
                            mStatus.setText(getString(R.string.contact_update));
                        } else if (contactAction == ContactAction.CONTACT_ACTION_READ) {
                            percent = 10 + ((float)value / max) * 40;
                            // updateDialog(percent, "濮濓絽婀拠璇插絿閼辨梻閮存禍锟�;
                            mStatus.setText(getString(R.string.contact_read));
                        }

                    }
                });

            }

            @Override
            public void onPreExecuteAuthSession(Auth arg0) throws RemoteException {
                LogHelper.trace("onPreExecuteAuthSession");
                mHandler.post(new Runnable() {
                    public void run() {
                        mStatus.setText(R.string.get_tokening);
                    }
                });
            }

            @Override
            public void onHttpResponeText(String message, String arg1) throws RemoteException {
                LogHelper.trace("onHttpResponeText: " + message);
            }

            @Override
            public void onExecuting(Auth arg0, int arg1) throws RemoteException {
                LogHelper.trace("onExecuting");
                mHandler.post(new Runnable() {
                    public void run() {
                        mStatus.setText(R.string.contact_commint_tip);
                    }
                });
            }

            @Override
            public void onAuthSession(final Auth auth, boolean arg1) throws RemoteException {
                LogHelper.trace("onAuthSession: auth = " + auth.toString());
                mHandler.post(new Runnable() {
                    public void run() {
                        if (auth.getResult_code() == 0) {
                            mStatus.setText(R.string.get_token_fail);
                            mIcon.setImageResource(R.drawable.backup_idle);
                        }
                    }
                });
            }

            @Override
            public void onThrowException(Auth autn, int syncAction, String exceptionMessage) {
                LogHelper.trace("Exception: " + exceptionMessage);
                if (SyncAction.valueOf(syncAction) == SyncAction.CONTACT_UPLOAD) {
                    mHandler.post(new Runnable() {
                        @Override
                        public void run() {
                            toast(R.string.contact_backup_fail);
                        }
                    });
                } else {
                    mHandler.post(new Runnable() {
                        @Override
                        public void run() {
                            toast(R.string.contact_recover_fail);
                        }
                    });
                }
            }

        });
    }

    private void updateLastOption(int type) {
        SimpleDateFormat formatter = new SimpleDateFormat(getResources().getString(
                R.string.date_and_time_format));
        Date curDate = new Date(System.currentTimeMillis());
        String str = formatter.format(curDate);
        if (type != BACKUP && type != RESTORE) {
            return;
        }
        SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(this);
        SharedPreferences.Editor editor = prefs.edit();
        editor.putString(KEY_LAST_OPTION_TIME, str);
        editor.putInt(KEY_LAST_OPTION_TYPE, type);
        editor.apply();
    }
}
