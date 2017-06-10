/*
 * Copyright (c) 2013, Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qti.fcctest;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.app.DialogFragment;
import android.app.Fragment;
import android.app.FragmentTransaction;
import android.content.Context;
import android.content.DialogInterface;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.ProgressBar;
import android.widget.TabHost;
import android.widget.TabHost.TabSpec;
import android.widget.TextView;

import com.qti.fcctest.TestContentFragment.OnSelectionChangedListener;
import com.qti.fcctest.Utils.SelectionsContent;
import com.qti.fcctest.Utils.ValueContent;

public class FccTestActivity extends Activity implements View.OnClickListener,
        OnSelectionChangedListener, DialogInterface.OnClickListener {
    private static final String TAG = "FccTestActivity";

    private static final int TIME_FOR_ENABLE_TEST_MODE = 15 * 1000;

    private boolean mTestModeEnabled = false;

    private Button mBtnEnableTest;
    private Button mBtnExistTest;
    private TextView mPrompt;
    private View mTabContent;

    private int mCurTest = Utils.ID_TX_TEST;
    private TabHost mTabHost;
    private TabManager mTabManager;
    private SelectionsContent mSelections;

    private static final int MSG_ENABLE_TEST_DONE = 1;
    private Handler mHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case MSG_ENABLE_TEST_DONE:
                    // Update the test mode value.
                    mTestModeEnabled = true;

                    // Dismiss the progress dialog.
                    MyDialog dialog = (MyDialog) getFragmentManager()
                            .findFragmentByTag("progress");
                    if (dialog != null) dialog.dismiss();

                    // Update the views visibility.
                    updateViewsVisibility();
                    break;
            }
            super.handleMessage(msg);
        }
    };

    static {
        try {
            System.loadLibrary("wifi_ftm_jni");
        } catch (UnsatisfiedLinkError ule) {
            System.err.println("WARNING: Could not load library libwifi_ftm_jni.so");
        }
    }

    @Override
    protected void onCreate(Bundle bundle) {
        super.onCreate(bundle);

        setContentView(R.layout.fcctest_activity);

        mBtnEnableTest = (Button) findViewById(R.id.btn_enable_or_start_test);
        mBtnEnableTest.setOnClickListener(this);
        mBtnExistTest = (Button) findViewById(R.id.btn_exist_test);
        mBtnExistTest.setOnClickListener(this);
        mPrompt = (TextView) findViewById(R.id.enable_test_prompt);
        mTabContent = findViewById(android.R.id.tabcontent);

        mTabHost = (TabHost) findViewById(android.R.id.tabhost);
        mTabHost.setup();
        mTabManager = new TabManager(this, mTabHost, android.R.id.tabcontent);

        // Add the Tx test tab
        String tx = getString(R.string.tx_test);
        mTabManager.addTab(mTabHost.newTabSpec(String.valueOf(Utils.ID_TX_TEST))
                .setIndicator(tx));

        // Add the Rx test tab
        String rx = getString(R.string.rx_test);
        mTabManager.addTab(mTabHost.newTabSpec(String.valueOf(Utils.ID_RX_TEST))
                .setIndicator(rx));

        // Add the SCW test tab
        String scw = getString(R.string.scw_test);
        mTabManager.addTab(mTabHost.newTabSpec(String.valueOf(Utils.ID_SCW_TEST))
                .setIndicator(scw));

        updateViewsVisibility();
    }

    @Override
    protected void onDestroy() {
        StopTest();

        super.onDestroy();
    }

    @Override
    public void onClick(View v) {
        switch (v.getId()) {
            case R.id.btn_enable_or_start_test:
                if (mTestModeEnabled) {
                    // We will start the test according to current selected test.
                    int titleResId = 0;
                    int btnResId = 0;
                    if (mCurTest == Utils.ID_TX_TEST) {
                        StartTx();
                        titleResId = R.string.tx_test;
                        btnResId = R.string.stop_tx_test_text;
                    } else if (mCurTest == Utils.ID_RX_TEST) {
                        StartRx();
                        titleResId = R.string.rx_test;
                        btnResId = R.string.stop_rx_test_text;
                    } else if (mCurTest == Utils.ID_SCW_TEST) {
                        StartSCW();
                        titleResId = R.string.scw_test;
                        btnResId = R.string.stop_scw_test_text;
                    }
                    MyDialog dialog = MyDialog.newInstance(
                            titleResId, R.string.testing_message, null, btnResId, true);
                    dialog.show(getFragmentManager(), "progress");
                } else {
                    // We will enable the test mode, and the enable process maybe take some time.
                    MyDialog dialog = MyDialog.newInstance(
                            R.string.enable_test_mode_title,
                            R.string.enable_test_mode_message,
                            null, // show the message by res id.
                            android.R.string.cancel,
                            true); // show the progress bar.
                    dialog.show(getFragmentManager(), "progress");

                    mHandler.sendEmptyMessageDelayed(MSG_ENABLE_TEST_DONE,
                            TIME_FOR_ENABLE_TEST_MODE);

                    // Start the test.
                    new Thread(new Runnable() {
                        @Override
                        public void run() {
                            Log.i(TAG, "Try to enable the test mode.");
                            StartTest();
                        }
                    }).start();
                }
                break;
            case R.id.btn_exist_test:
                finish();
                break;
        }
    }

    @Override
    public void onClick(DialogInterface dialog, int which) {
        switch (which) {
            case DialogInterface.BUTTON_POSITIVE:
                dialog.dismiss();
                break;
            case DialogInterface.BUTTON_NEGATIVE:
                if (mTestModeEnabled) {
                    // The progress dialog is showing for in test process.
                    // As the button is used to stop the test process, so stop the test process.
                    if (mCurTest == Utils.ID_TX_TEST) {
                        StopTx();
                    } else if (mCurTest == Utils.ID_RX_TEST) {
                        // Stop the rx test and display the report.
                        String report = ReportRx();
                        StopRx();

                        MyDialog reportDialog = MyDialog.newInstance(
                                R.string.rx_test_result_title,
                                -1,
                                report,
                                android.R.string.ok,
                                false); // show the progress bar.
                        reportDialog.show(getFragmentManager(), "report");
                    } else if (mCurTest == Utils.ID_SCW_TEST) {
                        StopSCW();
                    }
                } else {
                    // The progress dialog is showing for enable test mode.
                    // As the button is used to cancel the process, so finish the activity.
                    finish();
                }
                break;
        }
    }

    @Override
    public void onSelectionChanged(SelectionsContent allSelection, int changedItemId) {
        mSelections = allSelection;
        ValueContent changed = mSelections.getSelections().get(String.valueOf(changedItemId));
        Log.i(TAG, "Selection changed, change item: " + changed._label
                + " with value: " + changed._value);

        switch (changedItemId) {
            case Utils.ID_TEST_ITEM_CHANNEL:
                Log.d(TAG, "SetChannel: " + changed._value);
                SetChannel(Integer.valueOf(changed._value));
                break;
            case Utils.ID_TEST_ITEM_RATE:
                if (mCurTest == Utils.ID_TX_TEST) {
                    Log.d(TAG, "SetTxRate: " + changed._value);
                    SetTxRate(changed._value);
                }
                break;
            case Utils.ID_TEST_ITEM_POWER:
                if (mCurTest == Utils.ID_TX_TEST) {
                    Log.d(TAG, "SetTxPower: " + changed._value);
                    SetTxPower(Integer.valueOf(changed._value));
                }
                break;
            case Utils.ID_TEST_ITEM_POWER_MODE:
                if (mCurTest == Utils.ID_TX_TEST) {
                    Log.d(TAG, "SetTxPowerMode: " + changed._value);
                    SetPowerMode(Integer.valueOf(changed._value));
                }
                break;
            case Utils.ID_TEST_ITEM_ANTENNA:
                break;
            case Utils.ID_TEST_ITEM_TYPE:
                break;
            case Utils.ID_TEST_ITEM_PATTERN:
                break;
        }
    }

    private void updateViewsVisibility() {
        if (mTestModeEnabled) {
            mPrompt.setVisibility(View.GONE);
            mTabContent.setVisibility(View.VISIBLE);
            mBtnEnableTest.setText(R.string.start_test_text);
        } else {
            mPrompt.setVisibility(View.VISIBLE);
            mTabContent.setVisibility(View.GONE);
            mBtnEnableTest.setText(R.string.enable_test_mode);
        }
    }

    private static class MyDialog extends DialogFragment {
        private static final String KEY_TITLE = "title";
        private static final String KEY_MSG_RES = "msg_res";
        private static final String KEY_MSG_STR = "msg_str";
        private static final String KEY_BUTTON = "button";
        private static final String KEY_PROGRESS = "progress";

        public static MyDialog newInstance(int titleResId, int msgResId, String msg, int btnResId,
                boolean showProgress) {
            MyDialog dialog = new MyDialog();
            Bundle bundle = new Bundle();
            bundle.putInt(KEY_TITLE, titleResId);
            bundle.putInt(KEY_MSG_RES, msgResId);
            bundle.putString(KEY_MSG_STR, msg);
            bundle.putInt(KEY_BUTTON, btnResId);
            bundle.putBoolean(KEY_PROGRESS, showProgress);
            dialog.setArguments(bundle);
            dialog.setCancelable(false);
            return dialog;
        }

        @Override
        public Dialog onCreateDialog(Bundle savedInstanceState) {
            Bundle arg = getArguments();
            int titleResId = arg.getInt(KEY_TITLE);
            int msgResId = arg.getInt(KEY_MSG_RES);
            String msgStr = arg.getString(KEY_MSG_STR);
            int btnResId = arg.getInt(KEY_BUTTON);
            boolean showProgress = arg.getBoolean(KEY_PROGRESS);

            View v = getActivity().getLayoutInflater()
                    .inflate(R.layout.dialog_content, null);
            ProgressBar progress = (ProgressBar) v.findViewById(R.id.progress);
            progress.setVisibility(showProgress ? View.VISIBLE : View.GONE);

            TextView message = (TextView) v.findViewById(R.id.dialog_message);
            if (msgResId > 0) {
                message.setText(msgResId);
            } else {
                message.setText(msgStr);
            }

            AlertDialog.Builder builder = new AlertDialog.Builder(getActivity());
            builder.setIcon(android.R.drawable.ic_dialog_alert)
                    .setTitle(titleResId)
                    .setView(v)
                    .setCancelable(false);

            if (showProgress) {
                builder.setNegativeButton(btnResId, (FccTestActivity) getActivity());
            } else {
                builder.setPositiveButton(btnResId, (FccTestActivity) getActivity());
            }

            return builder.create();
        }
    }

    private static class TabManager implements TabHost.OnTabChangeListener {
        private FccTestActivity activity;
        private final TabHost tabHost;
        private final int containerResId;

        private TestContentFragment testFragment;

        public TabManager(FccTestActivity activity, TabHost tabHost, int containerResId) {
            this.activity = activity;
            this.tabHost = tabHost;
            this.containerResId = containerResId;

            // set the tab changed listener.
            activity.mTabHost.setOnTabChangedListener(this);
        }

        @Override
        public void onTabChanged(String tabTag) {
            Log.i(TAG, "onTabChanged, the tab: " + tabTag);
            int tabId = Integer.valueOf(tabTag);
            if (activity.mCurTest != tabId) {
                testFragment.onTestModeChanged(tabId);
                activity.mCurTest = tabId;
                activity.updateViewsVisibility();
            }
        }

        public void addTab(TabSpec tabSpec) {
            tabSpec.setContent(new MyTabFactory(activity));
            if (testFragment == null) {
                FragmentTransaction ft = activity.getFragmentManager().beginTransaction();
                testFragment = (TestContentFragment) Fragment.instantiate(
                        activity, TestContentFragment.class.getName(), null);
                testFragment.setOnSelectionChangedListener(activity);
                ft.add(containerResId, testFragment, tabSpec.getTag());
                ft.commit();
            }

            tabHost.addTab(tabSpec);
        }

        static class MyTabFactory implements TabHost.TabContentFactory {
            private final Context mContext;

            public MyTabFactory(Context context) {
                mContext = context;
            }

            @Override
            public View createTabContent(String tag) {
                View v = new View(mContext);
                v.setMinimumWidth(0);
                v.setMinimumHeight(0);
                return v;
            }
        }

    }

    public static native int StartTest();
    public static native int StopTest();
    public static native int SetChannel(int channel);
    public static native int SetTxPower(int power);
    public static native int SetTxRate(String rate);
    public static native int SetPowerMode(int mode);
    public static native int StartTx();
    public static native int StopTx();
    public static native int StartSCW();
    public static native int StopSCW();
    public static native int StartRx();
    public static native int StopRx();
    public static native String ReportRx();

}
