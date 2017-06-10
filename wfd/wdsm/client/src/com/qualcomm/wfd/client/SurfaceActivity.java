/*
 * Copyright (c) 2012 - 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.wfd.client;

import com.qualcomm.wfd.client.ServiceUtil.ServiceFailedToBindException;
import com.qualcomm.wfd.client.WfdOperationUtil.WfdOperation;
import com.qualcomm.wfd.client.WfdOperationUtil.WfdOperationTask;

import com.qualcomm.wfd.WfdEnums;
import com.qualcomm.wfd.WfdStatus;
import com.qualcomm.wfd.service.IWfdActionListener;
import static com.qualcomm.wfd.client.WfdOperationUtil.*;

import android.app.Activity;
import android.app.ProgressDialog;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.res.Configuration;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.os.RemoteException;
import android.util.Log;
import android.view.KeyEvent;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.widget.FrameLayout;
import android.widget.Toast;

public class SurfaceActivity extends Activity{

    public static final String TAG = "Client.SurfaceActivity";
    private WfdSurface surface;
    private FrameLayout surfaceFrame;
    private Boolean modePlay = true;
    private Boolean modeStandby = false;
    private Boolean modeUibc = false;
    private SurfaceEventHandler eventHandler;
    private MenuItem playPauseMenuItem;
    private MenuItem standbyResumeMenuItem;
    private MenuItem startStopUibcMenuItem;
    public static ProgressDialog startSessionProgressDialog = null;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        Log.d(TAG, "surface onCreate");
        super.onCreate(savedInstanceState);
        setContentView(R.layout.surface_layout);

        surface = (WfdSurface)findViewById(R.id.wfdSurface);
        Log.d(TAG, "surface:" + surface);
        surfaceFrame = (FrameLayout) findViewById(R.id.surfaceFrame);
        Log.d(TAG, "surfaceFrame:" + surfaceFrame);
        surfaceFrame.invalidate();
        Log.d(TAG, "surface instantiated");

        eventHandler = new SurfaceEventHandler();

        if (!ServiceUtil.getmServiceAlreadyBound()) {
            try {
                ServiceUtil.bindService(getApplicationContext(), eventHandler);
            } catch (ServiceFailedToBindException e) {
                Log.e(TAG, "ServiceFailedToBindException received");
            }
        } else {
            serviceUtilInitWrapper();
        }

        startSessionProgressDialog = ProgressDialog.show(this,
                "Starting Session", "Press back to cancel", true, true,
                new  DialogInterface.OnCancelListener() {
                    @Override
                    public void onCancel(DialogInterface dialog) {
                        Log.d(TAG, "startSessionProgressDialog onCancel called");
                    }
        });
    }

    /** To prevent dynamic configuration changes from destroying activity */
    @Override
    public void onConfigurationChanged (Configuration newConfig) {
        Log.e(TAG, "onConfigurationChanged called due to"+ newConfig.toString());
        super.onConfigurationChanged(newConfig);
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        try {
            if(ServiceUtil.getmServiceAlreadyBound()) {
              unbindWfdService();
            }
        } catch (IllegalArgumentException e) {
            Log.e(TAG,"Illegal Argument Exception ",e);
        }

        if(startSessionProgressDialog != null) {
          startSessionProgressDialog.dismiss();
        }

        Log.d(TAG, "onDestroy() called");
    }

    protected void unbindWfdService() {
        Log.d(TAG, "unbindWfdService() called");
        ServiceUtil.unbindService(getApplicationContext());
    }

    private void serviceUtilInitWrapper() {
        IWfdActionListener mActionListener = WfdOperationUtil.createmActionListener(eventHandler);

        try {
            int initReturn = ServiceUtil.getInstance().init(mActionListener, null);
            Log.d(TAG, "onCreate: init returned- " + initReturn);
            if (!(initReturn == 0 || initReturn == WfdEnums.ErrorType.ALREADY_INITIALIZED.getCode())) {
                Log.e(TAG, "onCreate: init failed with error- " + initReturn);
            }
        } catch (RemoteException e) {
            Log.e(TAG, "Remote exception", e);
        }

        Toast.makeText(getApplicationContext(), getResources().getString(
                R.string.wfd_service_connected), Toast.LENGTH_SHORT).show();
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        MenuInflater inflater = getMenuInflater();
        inflater.inflate(R.menu.surface_menu, menu);
        return true;
    }

    @Override
    public boolean onPrepareOptionsMenu(Menu menu) {
        super.onPrepareOptionsMenu(menu);
        playPauseMenuItem = menu.findItem(R.id.menu_play_pause);
        standbyResumeMenuItem = menu.findItem(R.id.menu_standby_resume);
        startStopUibcMenuItem = menu.findItem(R.id.menu_start_stop_UIBC);

        try {
              if(ServiceUtil.getmServiceAlreadyBound()) {
                  modeUibc = ServiceUtil.getInstance().getUIBCStatus();
              }
              if(modeUibc) {
                  startStopUibcMenuItem.setTitle("Stop UIBC");
                  surface.startUIBCEventCapture();
                  Log.d(TAG, "onPrepareOptionsMenu: UIBC Enabled");
              } else {
                  startStopUibcMenuItem.setTitle("Start UIBC");
                  surface.stopUIBCEventCapture();
                  Log.d(TAG, "onPrepareOptionsMenu: UIBC Disabled");
              }
        } catch (RemoteException e) {
            Log.e(TAG, "Remote exception", e);
        }

        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        // Handle item selection
        switch (item.getItemId()) {
        /*case R.id.menu_advanced_settings:
            startActivity(new Intent(this, AdvancedPreferences.class));
            break;*/
        case R.id.menu_preferences:
            Log.d(TAG, "prefences clicked");
            startActivity(new Intent(this, Preferences.class));
            break;
        default:
            Log.d(TAG, "No menu item selected");
        }
        return super.onOptionsItemSelected(item);
    }

    public void playPauseSurfaceActivity(MenuItem i) {
        Log.d(TAG, "playPauseSurfaceActivity() called");
        Log.d(TAG, "Play/Pause button clicked");
        if (modePlay) {
            startWfdOperationTask(this, WfdOperation.PAUSE);
        } else {
            startWfdOperationTask(this, WfdOperation.PLAY);
        }
    }

    public void standbyResumeSurfaceActivity(MenuItem i) {
        Log.d(TAG, "standbyResumeSurfaceActivity() called");
        Log.d(TAG, "Standby/Resume button clicked");
        if (!modeStandby) {
            startWfdOperationTask(this, WfdOperation.STANDBY);
        } else {
            startWfdOperationTask(this, WfdOperation.RESUME);
        }
    }


    public void startStopUibcSurfaceActivity(MenuItem i) {
        Log.d(TAG, "startStopUibcSurfaceActivity() called");
        Log.d(TAG, "Start/Stop UIBC button clicked");
        if (!modeUibc) {
            startWfdOperationTask(this, WfdOperation.START_UIBC);
        } else {
            startWfdOperationTask(this, WfdOperation.STOP_UIBC);
        }
    }

    public void onUIBCRotate(MenuItem i) {
        if(!ServiceUtil.getmServiceAlreadyBound()) {
            // To take care of devious people !
            Toast.makeText(getApplicationContext(), "Not in a WFD session!",
                    Toast.LENGTH_SHORT).show();
            return;
        }
        int angle = -1;
        switch (i.getItemId()) {
            case R.id.UIBC_rotate_0:
                angle = 0;
            break;
            case R.id.UIBC_rotate_90:
                angle = 90;
            break;
            case R.id.UIBC_rotate_180:
                angle = 180;
            break;
            case R.id.UIBC_rotate_270:
                angle = 270;
            break;
            default:// Never possible though
                return;
        }
        Bundle cfgItem = new Bundle();
        try {
            ServiceUtil.getInstance().getConfigItems(cfgItem);
        } catch (RemoteException e) {
            e.printStackTrace();
        }
        // Rotation should be triggered only if UIBC is enabled
        int[] configArr = new int[WfdEnums.ConfigKeys.TOTAL_CFG_KEYS.ordinal()];
        configArr = cfgItem.getIntArray(WfdEnums.CONFIG_BUNDLE_KEY);
        if (configArr != null
                && 1 == configArr[WfdEnums.ConfigKeys.UIBC_VALID.ordinal()]) {
            int ret = -1;
            try {
                ret = ServiceUtil.getInstance().uibcRotate(angle);
            } catch (RemoteException e) {
                e.printStackTrace();
            }
            String res = "failed!";
            if (ret == 0) {
                res = "Successful!";
            }
            Toast.makeText(getApplicationContext(),
                    "UIBC rotation with " + Integer.toString(angle) + " degrees " + res,
                    Toast.LENGTH_SHORT).show();
        } else {
            Toast.makeText(getApplicationContext(), "UIBC not enabled!",
                    Toast.LENGTH_SHORT).show();
            return;
        }
    }


    public void onSetDecoderLatency(MenuItem i) {
        if(!ServiceUtil.getmServiceAlreadyBound()) {
            // To take care of devious people !
            Toast.makeText(getApplicationContext(), "Not in a WFD session!",
                    Toast.LENGTH_SHORT).show();
            return;
        }
        int latency = -1;
        switch (i.getItemId()) {
            case R.id.Decoder_Latency_0:
                latency = 0;
            break;
            case R.id.Decoder_Latency_50:
                latency = 50;
            break;
            case R.id.Decoder_Latency_100:
                latency = 100;
            break;
            case R.id.Decoder_Latency_150:
                latency = 150;
            break;
            case R.id.Decoder_Latency_200:
                latency = 200;
            break;
            case R.id.Decoder_Latency_250:
                latency = 250;
            break;
            default:// Never possible though
                return;
        }
            int ret = -1;
            try {
                ret = ServiceUtil.getInstance().setDecoderLatency(latency);
            } catch (RemoteException e) {
                e.printStackTrace();
            }
            String res = "failed!";
            if (ret == 0) {
                res = "Successful!";
            }
            Toast.makeText(getApplicationContext(),
                    "Decoder latency value " + Integer.toString(latency) + " msec " + res,
                    Toast.LENGTH_SHORT).show();
    }


    public void teardownSurfaceActivity(MenuItem i) {
        Log.d(TAG, "teardownSurfaceActivity() called");
        Log.d(TAG, "Teardown button clicked");
        WfdOperationTask task = new WfdOperationTask(this, "Tearing down session", WfdOperation.TEARDOWN);
        task.execute();
    }

    /**
     * This function is to start/stop UIBC event capture
     * depending upon the state of WFD session and whether
     * UIBC is already enabled/disabled
     */
    public void checkAndSetUIBC(boolean captureEvents) {
        if (startStopUibcMenuItem.getTitle() == "Stop UIBC") {
           if (captureEvents) {
            surface.startUIBCEventCapture();
            }
            else {
                surface.stopUIBCEventCapture();
            }
        }
    }


    @Override
        public boolean onKeyDown(int keyCode, KeyEvent event) {
        if (keyCode == KeyEvent.KEYCODE_BACK) {
            Log.e(TAG, "Back button pressed in Surface Activity");
            if(startSessionProgressDialog != null) {
               startSessionProgressDialog.dismiss();
            }
            WfdStatus status = null;
            try {
                status = ServiceUtil.getInstance().getStatus();
            } catch (RemoteException e) {
                Log.e(TAG,"Remote Exception when retrieving status");
            }
            if (status.state == WfdEnums.SessionState.INVALID.ordinal()
             || status.state == WfdEnums.SessionState.INITIALIZED.ordinal()
             || status.state == WfdEnums.SessionState.IDLE.ordinal()) {
                Log.d(TAG, "No session in progress , call finish on Surface activity");
                setResult(RESULT_CANCELED);
                finish();
            } else {
                WfdOperationTask task = new WfdOperationTask(this, "Tearing down session", WfdOperation.TEARDOWN);
                task.execute();
            }
        }
        return false;
    }

    /**
     * Class for internal event handling in SurfaceActivity. Must run on UI thread.
     */
    class SurfaceEventHandler extends Handler {

        @Override
        public void handleMessage(Message msg) {
            Log.d(TAG, "Event handler received: " + msg.what);

            if (WfdOperationUtil.wfdOperationProgressDialog != null) {
                Log.d(TAG, "SurfaceEventHandler: wfdOperationProgressDialog != null");
                if (WfdOperationUtil.wfdOperationProgressDialog.isShowing()) {
                    Log.d(TAG, "SurfaceEventHandler: wfdOperationProgressDialog isShowing");
                    WfdOperationUtil.wfdOperationProgressDialog.dismiss();
                    Log.d(TAG, "SurfaceEventHandler: wfdOperationProgressDialog dismissed");
                } else {
                    Log.d(TAG, "SurfaceEventHandler: wfdOperationProgressDialog not isShowing");
                }
            } else {
                Log.d(TAG, "SurfaceEventHandler: wfdOperationProgressDialog == null");
            }

            switch (msg.what) {

                case PLAY_CALLBACK: {
                    Log.d(TAG, "SurfaceEventHandler: PLAY_CALLBACK- modePlay: " + modePlay);
                    modePlay = true;
                    modeStandby = false;
                    playPauseMenuItem.setTitle("Pause");
                    standbyResumeMenuItem.setTitle("Standby");
                    checkAndSetUIBC(true);
                    if (startSessionProgressDialog != null) {
                        Log.d(TAG, "SurfaceEventHandler: playCallback- startSessionProgressDialog != null");
                        startSessionProgressDialog.dismiss();
                        Log.d(TAG, "SurfaceEventHandler: playCallback- wfdOperationProgressDialog dismissed");
                    } else {
                        Log.d(TAG, "SurfaceEventHandler: playCallback- startSessionProgressDialog == null");
                    }
                }
                break;
                case PAUSE_CALLBACK: {
                    Log.d(TAG, "SurfaceEventHandler: PAUSE_CALLBACK- modePlay: " + modePlay);
                    modePlay = false;
                    playPauseMenuItem.setTitle("Play");
                    standbyResumeMenuItem.setTitle("Standby");
                    checkAndSetUIBC(false);
                }
                break;
                case STANDBY_CALLBACK: {
                    Log.d(TAG, "SurfaceEventHandler: STANDBY_CALLBACK");
                    modeStandby = true;
                    Log.d(TAG, "SurfaceEventHandler: STANDBY_CALLBACK- modePlay: " + modePlay);
                    standbyResumeMenuItem.setTitle("Resume");
                    checkAndSetUIBC(false);
                    if (WfdOperationUtil.wfdOperationProgressDialog != null && WfdOperationUtil.wfdOperationProgressDialog.isShowing()) {
                        WfdOperationUtil.wfdOperationProgressDialog.dismiss();
                        Log.d(TAG, "clientProgressDialog dismissed");
                    }
                }
                break;
                case UIBC_ACTION_COMPLETED: {
                    Log.d(TAG, "SurfaceEventHandler: UIBC_ACTION_COMPLETED- modeUibc: " + modeUibc);

                    if (modeUibc) {
                        if (WfdOperationUtil.UIBC_DISABLED == msg.arg1) {
                            if(startStopUibcMenuItem != null) {
                               startStopUibcMenuItem.setTitle("Start UIBC");
                            }
                            Log.d(TAG, "UIBC_ACTION_COMPLETED : UIBC_DISABLED");
                            surface.stopUIBCEventCapture();
                            modeUibc = false;
                        }
                    } else {
                        if(WfdOperationUtil.UIBC_ENABLED == msg.arg1) {
                             if(startStopUibcMenuItem != null) {
                               startStopUibcMenuItem.setTitle("Stop UIBC");
                             }
                             Log.d(TAG, "UIBC_ACTION_COMPLETED : UIBC_ENABLED");
                             surface.startUIBCEventCapture();
                             modeUibc = true;
                            }
                    }

                }
                break;
                case CLEAR_UI:
                    Log.d(TAG, "SurfaceEventHandler: CLEAR_UI");
                //fall through to TEARDOWN_CALLBACK case for normal teardown cleanup
                case TEARDOWN_CALLBACK: {
                    Log.d(TAG, "SurfaceEventHandler: TEARDOWN_CALLBACK");
                    try {
                        if(ServiceUtil.getmServiceAlreadyBound() != false) {
                            ServiceUtil.getInstance().deinit();
                        }
                    } catch (RemoteException e) {
                        Log.e(TAG, "EventHandler: teardownCallback- Remote exception when calling deinit()", e);
                    }
                    setResult(RESULT_OK);
                    finish();
                }
                break;
                case SERVICE_BOUND: {
                    serviceUtilInitWrapper();
                }
                break;
                case AUDIO_ONLY_NOTIFICATION: {
                    if(surface != null) {
                        surface.disablePauseOnSurfaceDestroyed();
                    }
                }
                break;
                default:
                    Log.e(TAG, "SurfaceEventHandler: Unknown event received: " + msg.what);
            }
        }
    }
}
