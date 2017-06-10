/*
 * Copyright (c) 2015 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
package com.qapp.secprotect.encryptdata;

import static com.qapp.secprotect.Configs.MODE_DECRYPT;
import static com.qapp.secprotect.Configs.MODE_ENCRYPT;
import static com.qapp.secprotect.Configs.STORAGE_ROOT;
import static com.qapp.secprotect.utils.UtilsLog.logd;

import java.lang.reflect.Field;
import java.util.HashMap;
import java.util.Map;

import android.app.AlertDialog;
import android.app.Dialog;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.DialogInterface.OnCancelListener;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Bundle;
import android.support.v4.app.FragmentActivity;
import android.support.v4.widget.SlidingPaneLayout;
import android.support.v4.widget.SlidingPaneLayout.PanelSlideListener;
import android.view.KeyEvent;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuItem;
import android.view.SubMenu;
import android.view.View;
import android.view.WindowManager;
import android.widget.EditText;

import com.qapp.secprotect.Configs;
import com.qapp.secprotect.R;
import com.qapp.secprotect.explorer.FileExplorerFragment;
import com.qapp.secprotect.framework.MainApp;
import com.qapp.secprotect.utils.UtilsLog;
import com.qapp.secprotect.utils.UtilsSystem;

public class EncryptDataActivity extends FragmentActivity implements
        EncryptOpsFragment.OnOperationClickListener {

    private Context mContext;
    FileExplorerFragment mFileExplorerFragment;
    EncryptOpsFragment mOperationFragment;
    SlidingPaneLayout mSlidingPaneLayout;

    Map<String, ?> mMap = new HashMap<String, Integer>();

    void init() {
        mContext = getApplicationContext();
        getActionBar().setDisplayShowHomeEnabled(false);
        FileExplorerFragment.init(MODE_ENCRYPT, STORAGE_ROOT);

        setContentView(R.layout.encryptdata);
        mSlidingPaneLayout = (SlidingPaneLayout) findViewById(R.id.encryptdata);
        mSlidingPaneLayout.setPanelSlideListener(mPanelSlideListener);
        mSlidingPaneLayout.openPane();

        mSlidingPaneLayout.setSliderFadeColor(0x77ffffff);// ARGB

        mFileExplorerFragment = (FileExplorerFragment) getSupportFragmentManager()
                .findFragmentById(R.id.fileexplorer_fragment);
        mOperationFragment = (EncryptOpsFragment) getFragmentManager()
                .findFragmentById(R.id.operation_fragment);

        MainApp.getInstance().scheduledExecutorService.execute(loadKeyRunnable);
    }

    Runnable loadKeyRunnable = new Runnable() {

        @Override
        public void run() {
            if (MainApp.getInstance().mInternalKey == null) {
                MainApp.getInstance().mInternalKey = UtilsPassword
                        .loadKeyFromFile(UtilsPassword.INTERNAL_INDEX);
                logd("mInternalKey: " + MainApp.getInstance().mInternalKey);
            }
            if (MainApp.getInstance().mSdcardKey == null) {
                MainApp.getInstance().mSdcardKey = UtilsPassword
                        .loadKeyFromFile(UtilsPassword.SDCARD_INDEX);
            }
        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        if (UtilsSystem.isMonkeyRunning()) {
            return;
        }
        init();
    }

    @Override
    protected void onResume() {
        logd("");
        refreshUI();
        IntentFilter intentFilter = new IntentFilter();
        intentFilter.addAction("");
        registerReceiver(mBroadcastReceiver, intentFilter);
        super.onResume();
    }

    @Override
    protected void onPause() {
        logd("");
        unregisterReceiver(mBroadcastReceiver);
        super.onPause();
    }

    @Override
    protected void onDestroy() {
        logd("");
        MainApp.getInstance().clearPasswordCache();
        super.onDestroy();
    };

    private void refreshUI() {
        logd("");
    }

    BroadcastReceiver mBroadcastReceiver = new BroadcastReceiver() {
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            logd(action);
        };
    };

    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {

        boolean isRootPath = false;

        if (mFileExplorerFragment != null) {
            // FileExplorer is on root directory
            isRootPath = mFileExplorerFragment.onKeyDown(keyCode, event);
            if (!isRootPath) {
                return true;
            }
        }
        if (isRootPath && !mSlidingPaneLayout.isOpen()) {
            mSlidingPaneLayout.smoothSlideOpen();
            logd("Go back to OperationFragment");
            return true;
        }
        return super.onKeyDown(keyCode, event);

    }

    @Override
    public void onOperationClick(int mode) {

        logd(mode);
        switch (mode) {
        case MODE_ENCRYPT:
            setTitle(R.string.encrypt);
            FileExplorerFragment.init(MODE_ENCRYPT, STORAGE_ROOT);
            mSlidingPaneLayout.smoothSlideClosed();
            break;
        case MODE_DECRYPT:
            if (MainApp.getInstance().mInternalKey == null) {
                UtilsLog.toast(this, getString(R.string.need_password_toast));

                Intent intent = new Intent(this, PasswordActivity.class);
                intent.putExtra("mode", Configs.INTENT_CREATE_PASSWORD);
                startActivity(intent);
                return;
            }
            if (MainApp.getInstance().mInternalPassword == null) {
                showDialog(DIALOG_LOGIN);
                return;
            }
            setTitle(R.string.decrypt);
            FileExplorerFragment.init(MODE_DECRYPT, STORAGE_ROOT);
            mSlidingPaneLayout.smoothSlideClosed();
            break;

        default:
            break;
        }

        mFileExplorerFragment.refreshExplorer();
    }

    PanelSlideListener mPanelSlideListener = new PanelSlideListener() {

        @Override
        public void onPanelSlide(View arg0, float arg1) {
        }

        @Override
        public void onPanelOpened(View arg0) {
            setTitle(R.string.encryptdata);
            logd("");
        }

        @Override
        public void onPanelClosed(View arg0) {
            logd("");
            switch (FileExplorerFragment.getFileExplorerMode()) {

            case MODE_ENCRYPT:
                setTitle(R.string.encrypt);
                break;

            case MODE_DECRYPT:
                setTitle(R.string.decrypt);
                break;

            default:
                break;
            }

        }
    };

    private static final int DIALOG_LOGIN = 0;

    @Override
    protected Dialog onCreateDialog(int id, Bundle args) {
        logd(id);
        switch (id) {
        case DIALOG_LOGIN:
            final View loginDialogView = LayoutInflater.from(
                    EncryptDataActivity.this).inflate(R.layout.dialog_login,
                    null);

            AlertDialog loginDialog = new AlertDialog.Builder(this)
                    .setTitle(R.string.login)
                    .setView(loginDialogView)
                    .setOnCancelListener(new OnCancelListener() {

                        @Override
                        public void onCancel(DialogInterface dialog) {
                            finish();
                        }
                    })
                    .setPositiveButton(R.string.ok,
                            new DialogInterface.OnClickListener() {

                                public void onClick(DialogInterface dialog,
                                        int whichButton) {
                                    boolean dismissDialog = true;

                                    String password = ((EditText) loginDialogView
                                            .findViewById(R.id.login_password))
                                            .getText().toString();

                                    if (!password.equals(UtilsPassword
                                            .loadPassword(UtilsPassword.INTERNAL_INDEX))) {
                                        UtilsLog.toast(
                                                mContext,
                                                getString(R.string.password_wrong));
                                        dismissDialog = false;
                                    } else {
                                        // password right
                                        dismissDialog = true;
                                        setTitle(R.string.decrypt);
                                        FileExplorerFragment.init(MODE_DECRYPT,
                                                STORAGE_ROOT);
                                        mSlidingPaneLayout.smoothSlideClosed();
                                    }

                                    try {
                                        Field field = dialog.getClass()
                                                .getSuperclass()
                                                .getDeclaredField("mShowing");
                                        field.setAccessible(true);
                                        field.set(dialog, dismissDialog);
                                    } catch (Exception e) {
                                        UtilsLog.loge(e);
                                    }
                                }
                            })
                    .setNegativeButton(R.string.cancel,
                            new DialogInterface.OnClickListener() {

                                public void onClick(DialogInterface dialog,
                                        int whichButton) {
                                    try {
                                        Field field = dialog.getClass()
                                                .getSuperclass()
                                                .getDeclaredField("mShowing");
                                        field.setAccessible(true);
                                        field.set(dialog, true);
                                    } catch (Exception e) {
                                        UtilsLog.loge(e);
                                    }
                                }
                            }).create();
            loginDialog
                    .getWindow()
                    .setSoftInputMode(
                            WindowManager.LayoutParams.SOFT_INPUT_ADJUST_RESIZE
                                    | WindowManager.LayoutParams.SOFT_INPUT_STATE_HIDDEN);
            return loginDialog;
        default:
            break;
        }
        return null;
    }

    private static final int MENU_CHANGE_PASSWORD = Menu.FIRST;

    public boolean onCreateOptionsMenu(Menu menu) {
        logd("");
        SubMenu addMenu = menu.addSubMenu(0, MENU_CHANGE_PASSWORD, 0,
                R.string.advanced);
        addMenu.setIcon(android.R.drawable.ic_menu_add);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
        case MENU_CHANGE_PASSWORD:
            break;
        default:
            break;
        }
        return super.onOptionsItemSelected(item);
    }

}
