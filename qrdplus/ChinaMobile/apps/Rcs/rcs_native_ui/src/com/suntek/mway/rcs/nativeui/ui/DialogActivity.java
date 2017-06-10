/*
 * Copyright (c) 2014 pci-suntektech Technologies, Inc.  All Rights Reserved.
 * pci-suntektech Technologies Proprietary and Confidential.
 */

package com.suntek.mway.rcs.nativeui.ui;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;

import com.suntek.mway.rcs.client.aidl.constant.APIConstant;
import com.suntek.mway.rcs.client.api.autoconfig.RcsAccountApi;
import com.suntek.mway.rcs.client.api.util.ServiceDisconnectedException;
import com.suntek.mway.rcs.nativeui.R;
import com.suntek.mway.rcs.nativeui.RcsApiManager;
import com.suntek.mway.rcs.nativeui.RcsNativeUIApp;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.DialogInterface.OnClickListener;
import android.content.Intent;
import android.net.ConnectivityManager;
import android.net.wifi.WifiManager;
import android.os.Bundle;
import android.telephony.TelephonyManager;
import android.view.KeyEvent;
import android.widget.Toast;

public class DialogActivity extends Activity {

    private static final int OPEN_ACCOUNT = 1;
    private static final int OPEN_PS = 2;
    private static final int RE_GET_DMS_CONFIG = 3;

    private static final String TYPE = "type";
    private static final String TITLE = "title";
    private static final String MESSAGE = "message";
    private static final String ACCEPT_BTN = "Accept_btn";
    private static final String REJECTBTN = "rejectBtn";

    public static void startOpenAccountDialog(Context context, String title, String message,
            int accept_btn, int rejectBtn) {
        Intent intent = new Intent(context, DialogActivity.class);
        intent.putExtra(TYPE, OPEN_ACCOUNT);
        intent.putExtra(TITLE, title);
        intent.putExtra(MESSAGE, message);
        intent.putExtra(ACCEPT_BTN, accept_btn);
        intent.putExtra(REJECTBTN, rejectBtn);
        intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        context.startActivity(intent);
    }

    public static void startOpenPSDialog(Context context) {
        Intent intent = new Intent(context, DialogActivity.class);
        intent.putExtra(TYPE, OPEN_PS);
        intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        context.startActivity(intent);
    }

    public static void startReGetDmsConfig(Context context){
        Intent intent = new Intent(context, DialogActivity.class);
        intent.putExtra(TYPE, RE_GET_DMS_CONFIG);
        intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        context.startActivity(intent);
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        this.setFinishOnTouchOutside(false);
        int type = getIntent().getIntExtra(TYPE, 0);
        switch (type) {
            case OPEN_ACCOUNT:
                showOpenAccountDialog();
                break;
            case OPEN_PS:
                showOpenPSDialog();
                break;
            case RE_GET_DMS_CONFIG:
                showRegetDmsDialog();
                break;
            default:
                break;
        }
    }

    private void showRegetDmsDialog() {
        String title = this.getString(R.string.re_get_dms_config_title);
        String message = this.getString(R.string.re_get_dms_config_message);
        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        builder.setTitle(title);
        builder.setMessage(message);
        builder.setPositiveButton(getString(R.string.btn_confirm), new OnClickListener() {
            @Override
            public void onClick(DialogInterface arg0, int arg1) {
                try {
                    RcsApiManager.getAccountApi().start();
                } catch (ServiceDisconnectedException e) {
                    e.printStackTrace();
                }
                arg0.dismiss();
                finish();
            }
        });
        builder.setNegativeButton(getString(R.string.btn_cancel), new OnClickListener() {
            @Override
            public void onClick(DialogInterface arg0, int arg1) {
                arg0.dismiss();
                finish();
            }
        });
        AlertDialog alertDialog = builder.create();
        alertDialog.setCanceledOnTouchOutside(false);
        alertDialog.setCancelable(false);
        alertDialog.show();
    }

    private void showOpenPSDialog() {
        String title = this.getString(R.string.please_open_ps);
        String message = this.getString(R.string.please_open_ps_message);
        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        builder.setTitle(title);
        builder.setMessage(message);
        builder.setPositiveButton(getString(R.string.btn_confirm), new OnClickListener() {
            @Override
            public void onClick(DialogInterface arg0, int arg1) {
                closeWifi(DialogActivity.this);
                setMobileData(DialogActivity.this, true);
                try {
                    RcsApiManager.getAccountApi().start();
                } catch (ServiceDisconnectedException e) {
                    e.printStackTrace();
                }
                arg0.dismiss();
                finish();
            }
        });
        builder.setNegativeButton(getString(R.string.btn_cancel), new OnClickListener() {
            @Override
            public void onClick(DialogInterface arg0, int arg1) {
                arg0.dismiss();
                finish();
            }
        });
        AlertDialog alertDialog = builder.create();
        alertDialog.setCanceledOnTouchOutside(false);
        alertDialog.setCancelable(false);
        alertDialog.show();
    }

    private void showOpenAccountDialog() {
        String title = getIntent().getStringExtra(TITLE);
        String message = getIntent().getStringExtra(MESSAGE);
        int accept_Btn = getIntent().getIntExtra(ACCEPT_BTN, APIConstant.BUTTON_DISPLAY);
        int reject_Btn = getIntent().getIntExtra(REJECTBTN, APIConstant.BUTTON_DISPLAY);
        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        builder.setTitle(title);
        builder.setMessage(message);
        if (accept_Btn == APIConstant.BUTTON_DISPLAY) {
            builder.setPositiveButton(getString(R.string.open_account_accpet),
                    new OnClickListener() {
                        @Override
                        public void onClick(DialogInterface arg0, int arg1) {
                            try {
                                RcsApiManager.getAccountApi().acceptAutoReg();
                            } catch (ServiceDisconnectedException e) {
                                e.printStackTrace();
                            }
                            arg0.dismiss();
                            finish();
                        }
                    });
        }
        if (reject_Btn == APIConstant.BUTTON_DISPLAY) {
            builder.setNegativeButton(getString(R.string.open_account_reject),
                    new OnClickListener() {
                        @Override
                        public void onClick(DialogInterface arg0, int arg1) {
                            arg0.dismiss();
                            finish();
                        }
                    });
        }
        AlertDialog alertDialog = builder.create();
        alertDialog.setCanceledOnTouchOutside(false);
        alertDialog.setCancelable(false);
        alertDialog.show();
    }

    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {
        switch (event.getKeyCode()) {
            case KeyEvent.KEYCODE_BACK:
                finish();
                return true;

            default:
                break;
        }
        return super.onKeyDown(keyCode, event);
    }

    private void setMobileData(boolean pBoolean) {
        try {
            ConnectivityManager mConnectivityManager = (ConnectivityManager) DialogActivity.this
                    .getSystemService(Context.CONNECTIVITY_SERVICE);
            Class<? extends ConnectivityManager> ownerClass = mConnectivityManager.getClass();
            @SuppressWarnings("rawtypes")
            Class[] argsClass = new Class[1];
            argsClass[0] = boolean.class;
            Method method = ownerClass.getMethod("setMobileDataEnabled", argsClass);
            method.invoke(mConnectivityManager, pBoolean);
        } catch (Exception e) {
            TelephonyManager telephonyManager = (TelephonyManager) DialogActivity.this
                    .getSystemService(Context.TELEPHONY_SERVICE);
            Class<? extends TelephonyManager> ownerClass = telephonyManager.getClass();
            Class[] argsClass = new Class[1];
            argsClass[0] = boolean.class;
            try {
                Method method = ownerClass.getMethod("setDataEnabled", argsClass);
                method.invoke(telephonyManager, pBoolean);
            } catch (NoSuchMethodException e1) {
                Toast.makeText(DialogActivity.this, R.string.open_mobile_data_fail,
                        Toast.LENGTH_SHORT).show();
                e1.printStackTrace();
            } catch (IllegalAccessException e1) {
                Toast.makeText(DialogActivity.this, R.string.open_mobile_data_fail,
                        Toast.LENGTH_SHORT).show();
                e1.printStackTrace();
            } catch (IllegalArgumentException e1) {
                Toast.makeText(DialogActivity.this, R.string.open_mobile_data_fail,
                        Toast.LENGTH_SHORT).show();
                e1.printStackTrace();
            } catch (InvocationTargetException e1) {
                Toast.makeText(DialogActivity.this, R.string.open_mobile_data_fail,
                        Toast.LENGTH_SHORT).show();
                e1.printStackTrace();
            }
            e.printStackTrace();
        }
    }

    private void closeWifi(Context context){
        WifiManager mWifiManager = (WifiManager) context
                .getSystemService(Context.WIFI_SERVICE);
        if (mWifiManager.isWifiEnabled()){
            RcsNativeUIApp.setNeedOpenWifi(true);
            mWifiManager.setWifiEnabled(false);
        }
    }

    private void openWifi(Context context){
        WifiManager mWifiManager = (WifiManager) context
                .getSystemService(Context.WIFI_SERVICE);
        mWifiManager.setWifiEnabled(true);
    }

    private void setMobileData(Context context, boolean pBoolean) {
        try {
            ConnectivityManager mConnectivityManager = (ConnectivityManager) context
                    .getSystemService(Context.CONNECTIVITY_SERVICE);
            Class<? extends ConnectivityManager> ownerClass = mConnectivityManager.getClass();
            @SuppressWarnings("rawtypes")
            Class[] argsClass = new Class[1];
            argsClass[0] = boolean.class;
            Method method = ownerClass.getMethod("setMobileDataEnabled", argsClass);
            method.invoke(mConnectivityManager, pBoolean);
        } catch (Exception e) {
            TelephonyManager telephonyManager = (TelephonyManager) context
                    .getSystemService(Context.TELEPHONY_SERVICE);
            Class<? extends TelephonyManager> ownerClass = telephonyManager.getClass();
            Class[] argsClass = new Class[1];
            argsClass[0] = boolean.class;
            try {
                Method method = ownerClass.getMethod("setDataEnabled", argsClass);
                method.invoke(telephonyManager, pBoolean);
            } catch (NoSuchMethodException e1) {
                e1.printStackTrace();
            } catch (IllegalAccessException e1) {
                e1.printStackTrace();
            } catch (IllegalArgumentException e1) {
                e1.printStackTrace();
            } catch (InvocationTargetException e1) {
                e1.printStackTrace();
            }
            e.printStackTrace();
        }
    }

}
