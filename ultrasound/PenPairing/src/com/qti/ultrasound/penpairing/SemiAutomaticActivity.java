/******************************************************************************
 * ---------------------------------------------------------------------------
 *  Copyright (C) 2013-2014 Qualcomm Technologies, Inc.
 *  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
 *  ---------------------------------------------------------------------------
 *******************************************************************************/

package com.qti.ultrasound.penpairing;

import com.qti.snapdragon.digitalpen.DigitalPenGlobalControl;

import android.annotation.SuppressLint;
import android.app.Activity;
import android.app.AlertDialog;
import android.content.DialogInterface;
import android.content.DialogInterface.OnClickListener;
import android.graphics.Color;
import android.graphics.drawable.GradientDrawable;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.util.Log;
import android.view.View;
import android.widget.ProgressBar;
import android.widget.TextView;
import android.content.res.Configuration;
import android.view.Surface;
import android.content.pm.ActivityInfo;
import android.view.WindowManager;

public class SemiAutomaticActivity extends Activity {

    private static final String PAIRING_DAEMON_NAME = "usf_pairing";

    private static final String PAIRING_SOCKET_PATH = "/data/usf/pairing/pairing_com_soc";

    // These MUST be the same constants as in the pen pairing
    // daemon, and with the same values.
    private static final int STATUS_IN_PROGRESS = 1;

    private static final int STATUS_REPOSITION = 2;

    private static final int STATUS_BLOCKED = 3;

    private static final int STATUS_SUCCESS = 4;

    private static final int STATUS_FAIL = 5;

    private static final int STATUS_TIMEOUT = 6;

    private static final int MSG_IND_STATUS = 0;

    private static final int MSG_IND_PEN_ID = 1;

    // Messages delivered to handler.
    public static final int ALERT_FAIL_MESSAGE = 0;

    public static final int SOCKET_EVENT_MESSAGE = 1;

    public static final String TIMEOUT_MESSAGE = "Pen Pairing daemon failed to find a pen. Please try again";

    // Background threads use this Handler to post messages to
    // the main application thread
    @SuppressLint("HandlerLeak")
    private final Handler mPairingHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case ALERT_FAIL_MESSAGE:
                    String message = (String)msg.obj; // Extract the string
                    alertAndQuit("Pen Pairing Failed", message);
                    break;
                case SOCKET_EVENT_MESSAGE:
                    int[] res = (int[])msg.obj;
                    onPairingSocketEvent(res);
                    break;
                default:
                    Log.e(this.toString(), "Unexpected message type received by handler");
            }
        }
    };

    protected SocketListener socketListener = null;

    protected ControlThread controlThread = null;

    protected DaemonWrapper mPairingDaemon = null;

    private String mPenType = "";

    // Allows turning the pen feature off to enable the pairing process.
    private DigitalPenGlobalControl mDigitalPenGlobalControl = null;

    private boolean mIsPenEnabled;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_semiautomatic);
        mDigitalPenGlobalControl = new DigitalPenGlobalControl(this);
        mPairingDaemon = new DaemonWrapper(PAIRING_DAEMON_NAME);
        // Lock the activity orientation with the default device orientation
        setRequestedOrientation(getDeviceDefaultOrientation());

        Bundle b = getIntent().getExtras();
        mPenType = b.getString("penType");
    }

    protected int getDeviceDefaultOrientation() {
        WindowManager windowManager =  (WindowManager) getSystemService(WINDOW_SERVICE);
        Configuration config = getResources().getConfiguration();
        int rotation = windowManager.getDefaultDisplay().getRotation();

        if (((rotation == Surface.ROTATION_0 || rotation == Surface.ROTATION_180) &&
             config.orientation == Configuration.ORIENTATION_LANDSCAPE) ||
            ((rotation == Surface.ROTATION_90 || rotation == Surface.ROTATION_270) &&
             config.orientation == Configuration.ORIENTATION_PORTRAIT)) {
            return ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE;
        } else {
            return ActivityInfo.SCREEN_ORIENTATION_PORTRAIT;
        }
    }

    @Override
    protected void onStart() {
        super.onStart();

        // Turn off digital pen daemon if already active when trying to pair.
        try {
            mIsPenEnabled = mDigitalPenGlobalControl.isPenFeatureEnabled();
            if (mIsPenEnabled) {
                mDigitalPenGlobalControl.disablePenFeature();
            }
        } catch (android.os.RemoteException e) {
            Log.e(this.toString(),
                    "Got RemoteException from DigitalPenGlobalControl: " + e.toString());
        }

        if (!mPairingDaemon.start()) {
            alertAndQuit("Pen Pairing Failed",
                    "An error has occurred while starting the pairing daemon. click 'OK' to exit");
            return;
        }

        socketListener = new SocketListener(mPairingHandler, PAIRING_SOCKET_PATH, mPairingDaemon,
                2 * (Integer.SIZE / Byte.SIZE));

        Log.d(this.toString(), "Starting socket thread...");

        socketListener.start();

        controlThread = new ControlThread(socketListener, mPairingDaemon, mPairingHandler);
        controlThread.start();
    }

    @Override
    protected void onStop() {
        mPairingDaemon.stop();

        if (mIsPenEnabled) {
            try {
                mDigitalPenGlobalControl.enablePenFeature();
            } catch (android.os.RemoteException e) {
                Log.e(this.toString(),
                        "Got RemoteException from DigitalPenGlobalControl: " + e.toString());
            }
        }
        super.onStop();
    }

    private void onPairingSocketEvent(int[] res) {
        int status = res[MSG_IND_STATUS];

        if (status != STATUS_IN_PROGRESS) {
            mPairingDaemon.stop();
        }

        switch (status) {
            case STATUS_IN_PROGRESS:
                onPairingInProgress();
                break;
            case STATUS_REPOSITION:
                alertAndQuit("Pen Pairing Failed",
                        "Pen is not positioned correctly. Please try again");
                break;
            case STATUS_BLOCKED:
                alertAndQuit("Pen Pairing Failed",
                        "One of the microphones is blocked. Please try again");
                break;
            case STATUS_FAIL:
                alertAndQuit("Pen Pairing Failed", "Pen Pairing has failed.");
                break;
            case STATUS_TIMEOUT:
                alertAndQuit("Pen Pairing Failed", TIMEOUT_MESSAGE);
                break;
            case STATUS_SUCCESS:
                int penId = res[MSG_IND_PEN_ID];
                onPairingSuccessful(penId);
                break;
        }
    }

    protected void onPairingInProgress() {
        setOnScreenText("Series aquisition in progress", R.id.semiautomatic_text_view);
        setCircleColor(Color.YELLOW, findViewById(R.id.semiautomatic_circle_view));
        showProgressBar(R.id.semiautomatic_progress_bar);
    }

    protected void onPairingSuccessful(int penId) {
        setOnScreenText("Pen detected (series ID " + String.valueOf(penId) + ")",
                R.id.semiautomatic_text_view);
        setCircleColor(Color.GREEN, findViewById(R.id.semiautomatic_circle_view));
        hideProgressBar(R.id.semiautomatic_progress_bar);
        PairingDbHelper pairingDbHelper = new PairingDbHelper(this);
        pairingDbHelper.addPen(penId, mPenType);
        Log.d(this.toString(), "Added new pen with pen id " + penId);

    }

    protected void setOnScreenText(String message, int textViewId) {
        TextView tv = (TextView)findViewById(textViewId);
        tv.setText(message);
    }

    protected void setCircleColor(int circleColor, View cv) {
        GradientDrawable gd = (GradientDrawable)cv.getBackground().mutate();
        gd.setColor(circleColor);
        gd.invalidateSelf();
    }

    protected void showProgressBar(int progressBarId) {
        ProgressBar progress = (ProgressBar)findViewById(progressBarId);
        progress.setVisibility(View.VISIBLE);
    }

    protected void hideProgressBar(int progressBarId) {
        ProgressBar progress = (ProgressBar)findViewById(progressBarId);
        progress.setVisibility(View.INVISIBLE);
    }

    protected void alertAndQuit(String title, String message) {
        if (isFinishing()) {
            return;
        }
        AlertDialog.Builder alert = new AlertDialog.Builder(this);
        alert.setTitle(title).setMessage(message).setPositiveButton("Ok", new OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                finish();
            }
        });
        alert.setCancelable(false);
        alert.show();
    }
}
