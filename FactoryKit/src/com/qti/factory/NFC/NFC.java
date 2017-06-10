/*
 * Copyright (c) 2014, Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
package com.qti.factory.NFC;

import android.app.Activity;
import android.app.PendingIntent;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.IntentFilter.MalformedMimeTypeException;
import android.nfc.NfcAdapter;
import android.nfc.tech.IsoDep;
import android.nfc.tech.MifareClassic;
import android.nfc.tech.MifareUltralight;
import android.nfc.tech.NdefFormatable;
import android.nfc.tech.NfcA;
import android.nfc.tech.NfcB;
import android.nfc.tech.NfcF;
import android.nfc.tech.NfcV;
import android.os.Bundle;
import android.os.CountDownTimer;
import android.os.SystemClock;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.Window;
import android.widget.TextView;
import android.widget.Toast;

import com.qti.factory.R;
import com.qti.factory.Utils;

public class NFC extends Activity {
    String TAG = "kunl";
    TextView mTextView;
    LayoutInflater mInflater = null;
    private Context mContext;
    private NfcAdapter mNfcAdapter = null;
    private final int ENABLE_RETRY = 4;

    PendingIntent mPendingIntent;
    IntentFilter[] mIntentFilters;
    IntentFilter mIntentFilter;
    String[][] mTechLists;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        mInflater = LayoutInflater.from(getApplicationContext());
        requestWindowFeature(Window.FEATURE_INDETERMINATE_PROGRESS);
        setContentView(R.layout.nfc);
        init();
    }

    private void init() {
        mContext = getApplicationContext();
        mTextView = (TextView) findViewById(R.id.nfc_hint);
        mNfcAdapter = NfcAdapter.getDefaultAdapter(mContext);

        if (mNfcAdapter == null) {
            toast(getString(R.string.nfc_not_available));
            fail(null);
            return;
        }
        if (!mNfcAdapter.isEnabled()) {
            logd("To enable NFC");
            mTextView.setText(getString(R.string.enabling));
            if (thread.getState() == Thread.State.NEW) {
                thread.start();
            }
        }

        mPendingIntent = PendingIntent.getActivity(this, 0,
                new Intent(this, getClass()).addFlags(Intent.FLAG_ACTIVITY_SINGLE_TOP), 0);

        IntentFilter intentFilter = new IntentFilter(NfcAdapter.ACTION_TECH_DISCOVERED);
        try {
            intentFilter.addDataType("*/*");
        } catch (MalformedMimeTypeException e) {
            e.printStackTrace();
        }
        mIntentFilters = new IntentFilter[] { intentFilter };
        mTechLists = new String[][] { new String[] { NfcF.class.getName() },
                new String[] { NfcA.class.getName() }, new String[] { NfcB.class.getName() },
                new String[] { NfcV.class.getName() }, new String[] { IsoDep.class.getName() },
                new String[] { IsoDep.class.getName() },
                new String[] { NdefFormatable.class.getName() },
                new String[] { MifareClassic.class.getName() },
                new String[] { MifareUltralight.class.getName() } };

        // broadcast
        mIntentFilter = new IntentFilter(NfcAdapter.ACTION_ADAPTER_STATE_CHANGED);
        mIntentFilter.addAction(NfcAdapter.ACTION_ADAPTER_STATE_CHANGED);
    }

    protected void onNewIntent(Intent intent) {

        String action = intent.getAction();
        logd(action);
        if ("android.nfc.action.TECH_DISCOVERED".equals(action)) {
            // [4, 59, -99, 106, 7, 41, -128]
            byte[] id = intent.getByteArrayExtra("android.nfc.extra.ID");
            String hexString = Utils.byteArrayToHexArray(id);
            logd(hexString);
            mTextView.setText("TAG: " + hexString);
            if (hexString.length() > 0)
                quitActionTimer.start();
        }

        setIntent(intent);
    };

    protected void onResume() {

        logd("");
        if (mNfcAdapter.isEnabled())
            setProgressBarIndeterminateVisibility(true);
        registerReceiver(mBroadcastReceiver, mIntentFilter);
        mNfcAdapter.enableForegroundDispatch(this, mPendingIntent, mIntentFilters, mTechLists);
        super.onResume();
    };

    @Override
    protected void onPause() {
        logd("");
        setProgressBarIndeterminateVisibility(false);
        unregisterReceiver(mBroadcastReceiver);
        mNfcAdapter.disableForegroundDispatch(this);
        super.onPause();
    }

    Runnable runnable = new Runnable() {

        @Override
        public void run() {
            int i = 0;
            while (i < ENABLE_RETRY) {

                logd("NfcEnabled=" + mNfcAdapter.isEnabled() + " try=" + i++);
                if (!mNfcAdapter.isEnabled())
                    mNfcAdapter.enable();
                else
                    break;
                SystemClock.sleep(2000);
            }
        }
    };

    Thread thread = new Thread(runnable);

    BroadcastReceiver mBroadcastReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            logd(action);
            mNfcAdapter = NfcAdapter.getDefaultAdapter(mContext);

            if (NfcAdapter.ACTION_ADAPTER_STATE_CHANGED.equals(action)) {

                int state = intent.getIntExtra(NfcAdapter.EXTRA_ADAPTER_STATE, 0);
                switch (state) {
                case NfcAdapter.STATE_ON:
                    mTextView.setText(getString(R.string.scanning));
                    setProgressBarIndeterminateVisibility(true);
                    break;
                case NfcAdapter.STATE_OFF:
                    mTextView.setText(getString(R.string.off));
                    setProgressBarIndeterminateVisibility(false);
                    break;
                case NfcAdapter.STATE_TURNING_OFF:
                    mTextView.setText(getString(R.string.turning_off));
                    setProgressBarIndeterminateVisibility(false);
                    break;
                case NfcAdapter.STATE_TURNING_ON:
                    mTextView.setText(getString(R.string.turning_on));
                    setProgressBarIndeterminateVisibility(false);
                    break;
                default:
                    break;
                }
            }
        }
    };

    private final int QUIT_DELAY_TIME = 1000;

    CountDownTimer quitActionTimer = new CountDownTimer(QUIT_DELAY_TIME, QUIT_DELAY_TIME) {

        @Override
        public void onTick(long arg0) {
        }

        @Override
        public void onFinish() {
            pass();
        }
    };

    void fail(Object msg) {

        loge(msg);
        setResult(RESULT_CANCELED);
        Utils.writeCurMessage(this, TAG, "Failed");
        finish();
    }

    void pass() {

        setResult(RESULT_OK);
        Utils.writeCurMessage(this, TAG, "Pass");
        finish();
    }

    @Override
    public void finish() {
        super.finish();
    }

    public void toast(Object s) {

        if (s == null)
            return;
        Toast.makeText(this, s + "", Toast.LENGTH_SHORT).show();
    }

    private void logd(Object s) {

        Thread mThread = Thread.currentThread();
        StackTraceElement[] mStackTrace = mThread.getStackTrace();
        String mMethodName = mStackTrace[3].getMethodName();

        s = "[" + mMethodName + "] " + s;
        Log.d(TAG, s + "");
    }

    private void loge(Object e) {

        if (e == null)
            return;
        Thread mThread = Thread.currentThread();
        StackTraceElement[] mStackTrace = mThread.getStackTrace();
        String mMethodName = mStackTrace[3].getMethodName();
        e = "[" + mMethodName + "] " + e;
        Log.e(TAG, e + "");
    }
}
