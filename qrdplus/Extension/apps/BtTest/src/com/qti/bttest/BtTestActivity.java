/*
 * Copyright (c) 2013-2014, Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qti.bttest;

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
import android.widget.TabWidget;
import android.widget.TabHost.TabSpec;
import android.widget.TextView;

import com.qti.bttest.TestContentFragment.OnSelectionChangedListener;
import com.qti.bttest.Utils.SelectionsContent;
import com.qti.bttest.Utils.TestItem;
import com.qti.bttest.Utils.ValueContent;

public class BtTestActivity extends Activity implements View.OnClickListener,
        OnSelectionChangedListener, DialogInterface.OnClickListener {
    private static final String TAG = "BtTestActivity";

    // Wait 15s for enter into the test mode.
    private static final int TIME_FOR_ENABLE_TEST_MODE = 15 * 1000;
    // Wait 5s for enter into the cmd test mode.
    private static final int TIME_FOR_ENTER_CMD_TEST_MODE = 5 * 1000;

    private Button mBtnEnableTest;
    private Button mBtnExistTest;
    private TabWidget mTabWidget;
    private TextView mPrompt;
    private View mTabContent;

    private int mCurTestMode = Utils.TEST_MODE_NONE;
    private int mCurTest = Utils.ID_ONLY_BURST;
    private TabHost mTabHost;
    private TabManager mTabManager;
    private SelectionsContent mSelections;

    private static final int MSG_ENABLE_TEST_DONE = 1;
    private static final int MSG_ENABLE_CMD_TEST_MODE_DONE = 2;

    private Handler mHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            // If the activity is finishing or destroyed, we needn't handle the message.
            if (isFinishing() || isDestroyed()) {
                return;
            }

            switch (msg.what) {
                case MSG_ENABLE_TEST_DONE:
                    // Update the test mode.
                    mCurTestMode = Utils.TEST_MODE_NO_CMD;

                    // Dismiss the progress dialog.
                    MyDialog testProgress = (MyDialog) getFragmentManager()
                            .findFragmentByTag("progress");
                    if (testProgress != null) testProgress.dismiss();

                    // Prompt the dialog to notify the user if enable the cmd test mode.
                    MyDialog alert = MyDialog.newInstance(
                            R.string.enable_cmd_test_mode_title,
                            R.string.enable_cmd_test_mode_message,
                            null, // show the message by res id.
                            R.string.enable_cmd_test_mode_button,
                            false); // show the progress bar.
                    alert.show(getFragmentManager(), "alert");

                    break;
                case MSG_ENABLE_CMD_TEST_MODE_DONE:
                    // Update the test mode value.
                    mCurTestMode = Utils.TEST_MODE_CMD;

                    // Dismiss the progress dialog.
                    MyDialog cmdProgress = (MyDialog) getFragmentManager()
                            .findFragmentByTag("progress");
                    if (cmdProgress != null) cmdProgress.dismiss();

                    // Update the views visibility.
                    updateViewsVisibility();
                    break;
            }
            super.handleMessage(msg);
        }
    };

    static {
        try {
            System.loadLibrary("bt_jni");
        } catch (UnsatisfiedLinkError ule) {
            System.err.println("WARNING: Could not load library libbt_jni.so");
        }
    }

    @Override
    protected void onCreate(Bundle bundle) {
        super.onCreate(bundle);

        setContentView(R.layout.bttest_activity);

        mBtnEnableTest = (Button) findViewById(R.id.btn_enable_or_start_test);
        mBtnEnableTest.setOnClickListener(this);
        mBtnExistTest = (Button) findViewById(R.id.btn_exist_test);
        mBtnExistTest.setOnClickListener(this);
        mTabWidget = (TabWidget) findViewById(android.R.id.tabs);
        mPrompt = (TextView) findViewById(R.id.enable_test_prompt);
        mTabContent = findViewById(android.R.id.tabcontent);

        mTabHost = (TabHost) findViewById(android.R.id.tabhost);
        mTabHost.setup();
        mTabManager = new TabManager(this, mTabHost, android.R.id.tabcontent);

        // Add the "Only Burst" tab
        String only_burst = getString(R.string.tx_only_burst);
        mTabManager.addTab(mTabHost.newTabSpec(String.valueOf(Utils.ID_ONLY_BURST))
                .setIndicator(only_burst));

        // Add the "Continuous" tab
        String continuous = getString(R.string.tx_continuous);
        mTabManager.addTab(mTabHost.newTabSpec(String.valueOf(Utils.ID_CONTINUOUS))
                .setIndicator(continuous));

        // Add the "Low energy" tab
        String low_energy = getString(R.string.low_energy);
        mTabManager.addTab(mTabHost.newTabSpec(String.valueOf(Utils.ID_LOW_ENERGY))
                .setIndicator(low_energy));

        updateViewsVisibility();
    }

    @Override
    protected void onDestroy() {
        if (mCurTestMode == Utils.TEST_MODE_CMD) {
            StopCMDMode();
        }
        StopTest();

        // As there may be some delay messages send to handler, we need ensure there isn't
        // any message need the handler to deal with before destroy this activity.
        if (mHandler != null) {
            mHandler.removeMessages(MSG_ENABLE_TEST_DONE);
            mHandler.removeMessages(MSG_ENABLE_CMD_TEST_MODE_DONE);
        }

        super.onDestroy();
    }

    @Override
    public void onClick(View v) {
        switch (v.getId()) {
            case R.id.btn_enable_or_start_test:
                if (mCurTestMode == Utils.TEST_MODE_CMD) {
                    // We will start the test according to current selected test.
                    int titleResId = 0;
                    if (mCurTest == Utils.ID_ONLY_BURST) {
                        StartOnlyBurstTest();
                        titleResId = R.string.tx_only_burst;
                    } else if (mCurTest == Utils.ID_CONTINUOUS) {
                        StartContinuousTest();
                        titleResId = R.string.tx_continuous;
                    } else if (mCurTest == Utils.ID_LOW_ENERGY) {
                        StartLETest();
                        titleResId = R.string.low_energy;
                    }
                    MyDialog dialog = MyDialog.newInstance(titleResId, R.string.testing_message,
                            null, R.string.stop_test_text, true);
                    dialog.show(getFragmentManager(), "progress");
                } else {
                    // We will enable the test mode, and the enable process maybe take some time.
                    MyDialog dialog = MyDialog.newInstance(
                            R.string.enable_test_mode_title,
                            R.string.please_wait_message,
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
                            Log.i(TAG, "Try to enable the test mode, enter HCI mode and reset it.");
                            StartTest();
                            EnterHCIMode();
                            ResetHCI();
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
                // It will only be clicked when alert the user if enter the cmd mode.
                // Dismiss the dialog.
                dialog.dismiss();

                // The start cmd process need take some time, and show the progress dialog.
                MyDialog progress = MyDialog.newInstance(
                        R.string.enable_cmd_test_mode_title,
                        R.string.please_wait_message,
                        null, // show the message by res id.
                        0, // do not show any button.
                        true); // show the progress bar.
                progress.show(getFragmentManager(), "progress");

                mHandler.sendEmptyMessageDelayed(MSG_ENABLE_CMD_TEST_MODE_DONE,
                        TIME_FOR_ENTER_CMD_TEST_MODE);

                // Start the cmd mode.
                new Thread(new Runnable() {
                    @Override
                    public void run() {
                        Log.i(TAG, "Try to start the command mode.");
                        StartCMDMode();
                    }
                }).start();
                break;
            case DialogInterface.BUTTON_NEGATIVE:
                if (mCurTestMode == Utils.TEST_MODE_CMD) {
                    // The progress dialog is showing for in test process.
                    // As the button is used to stop the test process, so stop the test process.
                    if (mCurTest == Utils.ID_ONLY_BURST) {
                        StopOnlyBurstTest();
                    } else if (mCurTest == Utils.ID_CONTINUOUS) {
                        StopContinuousTest();
                    } else if (mCurTest == Utils.ID_LOW_ENERGY) {
                        StopLETest();
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
    public void onSelectionChanged(SelectionsContent allSelection, TestItem changedItem) {
        mSelections = allSelection;
        ValueContent changed = mSelections.getCurValue(changedItem);
        Log.i(TAG, "Selection changed, change item: " + changedItem._label
                + " with new value: " + changed._label + "(" + changed._value + ")");

        String nativeFunctionName = null;
        switch (changedItem._id) {
            case Utils.ID_TEST_ITEM_HOPPING:
                boolean hopping = Boolean.valueOf(changed._value);
                SetHopping(hopping);
                nativeFunctionName = "SetHopping";
                break;
            case Utils.ID_TEST_ITEM_WHITENING:
                boolean whitening = Boolean.valueOf(changed._value);
                SetWhitening(whitening);
                nativeFunctionName = "SetWhitening";
                break;
            case Utils.ID_TEST_ITEM_CHANNEL:
                String[] values = Utils.getValueArray(changed._value);
                if (values.length == 1) {
                    SetChannel(Integer.valueOf(values[0]));
                    nativeFunctionName = "SetChannel";
                } else if (values.length == 5) {
                    SetChannelArray(Integer.valueOf(values[0]),
                            Integer.valueOf(values[1]),
                            Integer.valueOf(values[2]),
                            Integer.valueOf(values[3]),
                            Integer.valueOf(values[4]));
                    nativeFunctionName = "SetChannelArray";
                } else {
                    Log.e(TAG, "Shouldn't be here, set channel, values length: " + values.length);
                }
                break;
            case Utils.ID_TEST_ITEM_POWER:
                int power = Integer.valueOf(changed._value);
                SetPower(power);
                nativeFunctionName = "SetPower";
                break;
            case Utils.ID_TEST_ITEM_PACKET_TYPE:
                int packetType = Integer.valueOf(changed._value);
                SetPacketType(packetType);
                nativeFunctionName = "SetPacketType";
                break;
            case Utils.ID_TEST_ITEM_TRANS_PATTERN:
                int pattern = Integer.valueOf(changed._value);
                SetTransPattern(pattern);
                nativeFunctionName = "SetTransPattern";
                break;
            case Utils.ID_TEST_ITEM_PAYLOAD_LENGTH:
                int payloadlength = Integer.valueOf(changed._value);
                SetLength(payloadlength);
                nativeFunctionName = "SetLength";
                break;
            case Utils.ID_TEST_ITEM_TRANS_CHANNEL:
                int transChannel = Integer.valueOf(changed._value);
                SetChannel(transChannel);
                nativeFunctionName = "SetChannel";
                break;
            case Utils.ID_TEST_ITEM_TEST_TYPE:
                int testType = Integer.valueOf(changed._value);
                SetTestType(testType);
                nativeFunctionName = "SetTestType";
                break;
            case Utils.ID_TEST_ITEM_PATTERN_LENGTH:
                int patternLength = Integer.valueOf(changed._value);
                SetLength(patternLength);
                nativeFunctionName = "SetLength";
                break;
            case Utils.ID_TEST_ITEM_TEST_PAYLOAD:
                int testPayload = Integer.valueOf(changed._value);
                SetTestPayload(testPayload);
                nativeFunctionName = "SetTestPayload";
                break;
        }
        if (nativeFunctionName != null) {
            Log.d(TAG, nativeFunctionName + ": " + changed._value);
        }
    }

    private void updateViewsVisibility() {
        if (mCurTestMode == Utils.TEST_MODE_NONE) {
            mTabWidget.setVisibility(View.GONE);
            mPrompt.setVisibility(View.VISIBLE);
            mTabContent.setVisibility(View.GONE);
            mBtnEnableTest.setText(R.string.enable_test_mode);
        } else if (mCurTestMode == Utils.TEST_MODE_CMD) {
            mTabWidget.setVisibility(View.VISIBLE);
            mPrompt.setVisibility(View.GONE);
            mTabContent.setVisibility(View.VISIBLE);
            mBtnEnableTest.setText(R.string.start_test_text);
        }
    }

    public static class MyDialog extends DialogFragment {
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

            if (btnResId > 0) {
                if (showProgress) {
                    builder.setNegativeButton(btnResId, (BtTestActivity) getActivity());
                } else {
                    builder.setPositiveButton(btnResId, (BtTestActivity) getActivity());
                }
            }

            return builder.create();
        }
    }

    private static class TabManager implements TabHost.OnTabChangeListener {
        private BtTestActivity activity;
        private final TabHost tabHost;
        private final int containerResId;

        private TestContentFragment testFragment;

        public TabManager(BtTestActivity activity, TabHost tabHost, int containerResId) {
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
    public static native int ResetHCI();
    public static native int EnterHCIMode();
    public static native int StartCMDMode();
    public static native int StopCMDMode();
    public static native int StartOnlyBurstTest();
    public static native int StopOnlyBurstTest();
    public static native int SetHopping(boolean hopping);
    public static native int SetWhitening(boolean whitening);
    public static native int SetChannel(int channel);
    public static native int SetChannelArray(int ch1, int ch2, int ch3, int ch4, int ch5);
    public static native int SetPower(int power);
    public static native int SetPacketType(int packetType);
    public static native int SetTransPattern(int pattern);
    public static native int SetLength(int length);
    public static native int StartContinuousTest();
    public static native int StopContinuousTest();
    public static native int SetTestType(int testType);
    public static native int StartLETest();
    public static native int StopLETest();
    public static native int SetTestPayload(int payload);
}
