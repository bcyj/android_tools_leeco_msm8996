/******************************************************************************
 * ---------------------------------------------------------------------------
 *  Copyright (C) 2014 Qualcomm Technologies, Inc.
 *  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
 *  ---------------------------------------------------------------------------
 *******************************************************************************/

package com.qti.ultrasound.penpairing;

import android.annotation.SuppressLint;
import android.graphics.Color;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.util.Log;
import android.view.View;
import android.view.ViewGroup.LayoutParams;
import android.widget.RelativeLayout;

import java.io.IOException;

public class SWCalibrationActivity extends SemiAutomaticActivity {

    private static final String SW_CALIB_DAEMON_NAME = "usf_sw_calib";

    protected static final String SW_CALIB_SOCKET_PATH = "/data/usf/sw_calib/data_socket";

    private static final int MSG_IND_STATUS = 0;

    private static final int MSG_IND_X = 1;

    private static final int MSG_IND_Y = 2;

    // These MUST be the same constants as in the sw_calib
    // daemon, and with the same values.
    private static final int DPCSTATUS_ERROR = -1; /* Daemon error. */

    private static final int DPCSTATUS_OK = 0; /* All Ok. */

    private static final int DPCSTATUS_SPUR_ACOUSTIC = 1; /* Spurious acoustic tone. */

    private static final int DPCSTATUS_SPUR_EM = 2; /* Spurious electromagnetic tone. */

    private static final int DPCSTATUS_NO_PEN = 3; /* No pen detected. */

    private static final int DPCSTATUS_NOT_PEN_DOWN = 4; /* Pen down switch is not pressed. */

    private static final int DPCSTATUS_LOW_SIGNAL = 5; /* Low signal power. */

    private static final int DPCSTATUS_LOW_QUALITY = 6; /* Low signal quality. */

    private static final int DPCSTATUS_MULTIPATH = 7; /* Multipath detected. */

    private static final int DPCSTATUS_NOT_ON_POINT = 8; /* Pen is too far from expected point. */

    private static final String[] DPCSTATUS_NOTIFICATIONS = {
        "Calibration progressing normally", "Environmental issues",
        "Electomagnetic interference detected", "No pen detected",
        "Pen is not pressed on the circle", "Signal strength is low", "Signal quality is low",
        "Multipath detected", "Pen is not inside the circle"
    };

    private static final int CALIB_CIRCLE_DIAMETER_DP = 60; /* Diameter if the calibration circles */

    private static final int NUM_TOTAL_CALIB_CIRCLES = 4;

    private static final float MAX_X = 1000f; /* Maximum value of X coordinate of the calibration
                                              circle received from socket */

    private static final float MAX_Y = 1000f; /* Maximum value of Y coordinate of the calibration
                                              circle received from socket */

    private static final int CONTROL_THREAD_TIMEOUT_MSEC = 120000; // 2 minutes.

    private int mActivityHeight;

    private int mActivityWidth;

    private SocketListener mSwCalibSocketListener = null;

    private ControlThread mSwCalibControlThread = null;

    private DaemonWrapper mSwCalibDaemon = null;

    private int mPenId;

    private int mCurX = -1; /*the X value of the center of current circle. from
                            0 to 1000, 1000 meaning MAX_X.*/

    private int mCurY = -1; /* the Y value of the center of current circle.
                            from 0 to 1000, 1000 meaning MAX_Y.*/

    private View mCurrentCircle = null; /* The currently active calibration circle view.*/

    private int mNumCurrentCircle = 0;

    private String mPenType = "";

    // Background threads use this Handler to post messages to
    // the main application thread
    @SuppressLint("HandlerLeak")
    private final Handler mSwCalibHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case ALERT_FAIL_MESSAGE:
                    String message = (String)msg.obj; // Extract the string
                    alertAndQuit("Software Calibration Failed", message);
                    break;
                case SOCKET_EVENT_MESSAGE:
                    int[] res = (int[])msg.obj;
                    onSwCalibSocketEvent(res);
                    break;
                default:
                    Log.e(this.toString(), "Unexpected message type received by handler");
            }
        }
    };

    @Override
    public void onWindowFocusChanged(boolean hasFocus) {
        super.onWindowFocusChanged(hasFocus);
        if (hasFocus) {
            getWindow().getDecorView().setSystemUiVisibility(
                    View.SYSTEM_UI_FLAG_LAYOUT_STABLE | View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
                    | View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
                    | View.SYSTEM_UI_FLAG_HIDE_NAVIGATION | View.SYSTEM_UI_FLAG_FULLSCREEN
                    | View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY);
        }
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_sw_calibration);
        mSwCalibDaemon = new DaemonWrapper(SW_CALIB_DAEMON_NAME);
        // Lock the activity orientation with the default device orientation
        setRequestedOrientation(getDeviceDefaultOrientation());
        Bundle b = getIntent().getExtras();
        mPenType = b.getString("penType");
    }

    @Override
    protected void onStart() {
        super.onStart();
        mSwCalibSocketListener = new SocketListener(mSwCalibHandler, SW_CALIB_SOCKET_PATH,
                mSwCalibDaemon, 3 * (Integer.SIZE / Byte.SIZE));
        mSwCalibControlThread = new ControlThread(mSwCalibSocketListener, mSwCalibDaemon,
                mSwCalibHandler, getTimeoutForControlThread());
    }

    protected int getTimeoutForControlThread() {
        return CONTROL_THREAD_TIMEOUT_MSEC;
    }

    @Override
    protected void onStop() {
        mSwCalibDaemon.stop();
        super.onStop();
    }

    @Override
    protected void onPairingInProgress() {
        setOnScreenText("Series aquisition in progress", R.id.sw_calib_text_view);
        setCircleColor(Color.YELLOW, findViewById(R.id.sw_calib_circle_view));
        showProgressBar(R.id.sw_calib_progress_bar);
    }

    @Override
    protected void onPairingSuccessful(int penId) {
        mPenId = penId;
        mPairingDaemon.stop();

        setOnScreenText("Pen detected (series ID " + String.valueOf(mPenId) + ")\n"
                + "Please hold the digital pen pressed over the calibration positions.",
                R.id.sw_calib_text_view);
        setCircleColor(Color.GREEN, findViewById(R.id.sw_calib_circle_view));
        removeLinkToSeriesCalibFile();
        setLinkToSeriesCalibFile();

        if (!mSwCalibDaemon.start()) {
            Log.e(this.toString(), "Can't start sw_calib daemon");
            alertAndQuit("Software Calibration Failed", "Calibration process failed");
        }
        mSwCalibSocketListener.start();
        mSwCalibControlThread.start();
        updateActivityDimensions();
    }

    private void updateActivityDimensions() {
        View root = getWindow().getDecorView().getRootView();
        mActivityHeight = root.getHeight();
        mActivityWidth = root.getWidth();
    }

    private void setLinkToSeriesCalibFile() {
        try {
            Runtime.getRuntime()
            .exec(String
                    .format("ln -s /persist/usf/pen_pairing/%s/series_calib%d_%s.dat /data/usf/sw_calib/series_calib.dat",
                            mPenType, mPenId, mPenType));
        } catch (IOException e) {
            Log.e(this.toString(), e.toString());
            alertAndQuit("Software Calibration Failed", "Calibration process failed");
        }
    }

    private void removeLinkToSeriesCalibFile() {
        try {
            Runtime.getRuntime().exec("rm /data/usf/sw_calib/series_calib.dat");
        } catch (IOException e) {
            Log.e(this.toString(), e.toString());
        }
    }

    protected void onSwCalibSocketEvent(int[] res) {
        int status = res[MSG_IND_STATUS];
        int x = res[MSG_IND_X];
        int y = res[MSG_IND_Y];

        if (x == -1) { // x,y = -1 means no more calibration circles
            setCircleColor(Color.GREEN, mCurrentCircle);
            removeLinkToSeriesCalibFile();
            setOnScreenText("Calibration completed successfuly!", R.id.sw_calib_text_view);
            hideProgressBar(R.id.sw_calib_progress_bar);
            alertAndQuit("Software Calibration Successful",
                    "Calibration process completed successfuly");
            return;
        }

        if (status == DPCSTATUS_ERROR) {
            alertAndQuit("Software Calibration Failed",
                    "Software calibration failed, please try again");
            return;
        }

        if (x != mCurX || y != mCurY) {
            mCurX = x;
            mCurY = y;
            advanceToNextCircle();
        }

        if (status == DPCSTATUS_OK) {
            setOnScreenText(DPCSTATUS_NOTIFICATIONS[status] + ". Position " + mNumCurrentCircle
                    + "/" + NUM_TOTAL_CALIB_CIRCLES, R.id.sw_calib_notifications);
            setCircleColor(Color.YELLOW, mCurrentCircle);
        } else {
            setOnScreenText(DPCSTATUS_NOTIFICATIONS[status], R.id.sw_calib_notifications);
            setCircleColor(Color.RED, mCurrentCircle);
        }
    }

    private void advanceToNextCircle() {
        if (mCurrentCircle != null) {
            setCircleColor(Color.GREEN, mCurrentCircle);
        }

        mNumCurrentCircle++;
        mCurrentCircle = new View(this);
        mCurrentCircle.setBackgroundResource(R.drawable.circle);
        setCircleColor(Color.RED, mCurrentCircle);
        LayoutParams lp = new LayoutParams(CALIB_CIRCLE_DIAMETER_DP, CALIB_CIRCLE_DIAMETER_DP);
        lp.width = lp.height = CALIB_CIRCLE_DIAMETER_DP;
        mCurrentCircle.setLayoutParams(lp);

        RelativeLayout root = (RelativeLayout)findViewById(R.id.sw_root_layout);
        root.addView(mCurrentCircle);

        mCurrentCircle.setX((mCurX * mActivityWidth) / MAX_X - CALIB_CIRCLE_DIAMETER_DP / 2f);
        mCurrentCircle.setY((mCurY * mActivityHeight) / MAX_Y - CALIB_CIRCLE_DIAMETER_DP / 2f);
        root.invalidate();
    }
}
