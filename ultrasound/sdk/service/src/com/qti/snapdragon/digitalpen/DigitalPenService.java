/*===========================================================================
                           DigitalPenService.java

Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.

Qualcomm Technologies Proprietary and Confidential.
==============================================================================*/

package com.qti.snapdragon.digitalpen;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.util.concurrent.CancellationException;
import java.util.concurrent.ScheduledFuture;
import java.util.concurrent.ScheduledThreadPoolExecutor;
import java.util.concurrent.TimeUnit;
import java.util.Scanner;
import android.graphics.PixelFormat;
import android.view.Gravity;
import android.view.View;
import android.view.WindowManager;
import android.content.res.Resources;

import com.qti.snapdragon.digitalpen.SmarterStandSensorListener.AccelerometerChangeCallback;
import com.qti.snapdragon.sdk.digitalpen.impl.EventInterface;

import android.util.Log;
import android.content.IntentFilter;
import android.content.BroadcastReceiver;
import android.hardware.SensorManager;
import com.qti.snapdragon.digitalpen.util.DigitalPenConfig;
import com.qti.snapdragon.digitalpen.util.DigitalPenData;
import com.qti.snapdragon.digitalpen.util.DigitalPenEvent;

import android.app.ActivityManager;
import android.app.ActivityManager.RunningAppProcessInfo;

import com.qti.snapdragon.digitalpen.ConfigManager.ConfigChangedListener;
import com.qti.snapdragon.digitalpen.ConfigManager.State;
import com.qti.snapdragon.digitalpen.DirectoryObserver.FileChangeListener;
import com.qti.snapdragon.digitalpen.IDigitalPenDataCallback;
import com.qti.snapdragon.digitalpen.IDigitalPenEventCallback;

import android.content.Context;
import android.os.RemoteCallbackList;
import android.content.Intent;
import android.os.Bundle;
import android.os.FileObserver;
import android.os.SystemProperties;
import android.os.SystemService;
import android.os.Message;
import android.os.RemoteException;
import android.os.Binder;
import android.os.UserHandle;
import android.os.Handler;
import android.os.Process;
import codeaurora.ultrasound.IDigitalPenDimensionsCallback;
import java.lang.Math;

public class DigitalPenService extends IDigitalPenService.Stub {

    private static final String EPOS_PATH = "/data/usf/epos/";
    private static final String COMMAND_FILENAME = "command.txt";
    private static final String SERVICE_SETTINGS_SYMLINK_FILENAME = "service_settings.xml";
    public static int mRefs;
    private static final String TAG = "DigitalPenService";
    private static final String LOAD_CONFIG_ACTION = "com.qti.snapdragon.digitalpen.LOAD_CONFIG";
    private static final String GENERATE_ON_SCREEN_DATA_ACTION = "com.qti.snapdragon.digitalpen.GenerateOnScreenData";

    // Capabilities & the meanings of their parameters, sent when event listener
    // is registered.
    private static final int VERSION_MAJOR = 135;
    private static final int VERSION_MINOR = 0;
    private static final int SIDE_CHANNEL_CAPABLE = 1; // true

    private static final DigitalPenEvent[] CAPABILITIES_TABLE = {

            // version: maj, min
            new DigitalPenEvent(
                    DigitalPenEvent.TYPE_VERSION,
                    new int[] {
                            VERSION_MAJOR, VERSION_MINOR
                    }),

            // side-channel supported: 0 or 1
            new DigitalPenEvent(
                    DigitalPenEvent.TYPE_SIDE_CHANNEL_CAPABLE,
                    new int[] {
                            SIDE_CHANNEL_CAPABLE
                    }),
    };

    // This flag is used by the messaging mechanism to report that the daemon
    // has stopped
    private static final int DAEMON_STOPPED = 0;
    // Stop daemon with Sigterm declaration
    private static final String pidDirectory = "/data/usf/";
    private static final String pidFileExtention = ".pid";

    private final String mDaemonName = "usf_epos";

    // daemon event caching, to be reported to new clients
    private int cachedCurrentPowerState = DigitalPenEvent.POWER_STATE_OFF;
    private int cachedPreviousPowerState = DigitalPenEvent.POWER_STATE_OFF;
    private int cachedMicsBlocked = 0;
    private int cachedPenBatteryState = DigitalPenEvent.BATTERY_OK;
    private int cachedPenBatteryLevel = DigitalPenEvent.BATTERY_LEVEL_UNAVAILABLE;

    private GlobalSettingsPersister mGlobalSettingsPersister;
    private State mCurrentState = State.LEGACY;
    private boolean mIsFilteringData;
    private ConfigChangedListener mConfigChangeListener = new ConfigChangedListener() {

        @Override
        public void onConfigChanged(DigitalPenConfig newConfig, State newState, boolean stateChanged) {
            synchronized (DigitalPenService.this) {
                if (stateChanged) {
                    handleNewState(newState);
                }
                changeConfig(newConfig);
            }
        }

    };

    // state machine for the service
    private void handleNewState(State newState) {
        State oldState = mCurrentState;
        mCurrentState = newState;

        // detect if a new application is now active
        int pid = getCallingPidOverridable();
        boolean isNewApp = false;
        if (pid != mAppPid) {
            logNewAppProcess(pid);
            removeOldAppDataCallbacks(mAppPid);
            mAppPid = pid;
            isNewApp = true;
        }

        // Respond do a new state
        switch (newState) {
            case LEGACY_WITH_SIDE_CHANNEL:
                // Non-pen-aware app is foreground, but background listener
                enableBackgroundSideChannelIndicators();
                mIsFilteringData = true;
                break;
            case LEGACY:
                // No pen-aware app or background listeners
                disableBackgroundSideChannelIndicators();
                mIsFilteringData = false;
                removeOldAppDataCallbacks(mAppPid);
                mAppPid = NO_APP_PID;
                break;
            case APP:
                // Pen-aware app has come to foreground
                disableBackgroundSideChannelIndicators();
                mIsFilteringData = false;
                ifBackgroundCanceledNotifyApp(oldState, isNewApp);
                sendCachedStateEvents();
                break;
            default:
                throw new RuntimeException("Unknown state");
        }
    }

    private void sendCachedStateEvents() {
        postToHandler(new Runnable() {
            @Override
            public void run() {
                sendEvent(DigitalPenEvent.TYPE_POWER_STATE_CHANGED, new int[] {
                        cachedCurrentPowerState, cachedPreviousPowerState
                });
                sendEvent(DigitalPenEvent.TYPE_MIC_BLOCKED, new int[] {
                        cachedMicsBlocked
                });
                sendEvent(DigitalPenEvent.TYPE_PEN_BATTERY_STATE, new int[] {
                        cachedPenBatteryState, cachedPenBatteryLevel
                });
            }
        });
    }

    // Check if we have come from the background listener state, notify that app
    // that it was canceled.
    // Don't send the event if the background app itself came foreground.
    private void ifBackgroundCanceledNotifyApp(State oldState, boolean isNewApp) {
        if (oldState == State.LEGACY_WITH_SIDE_CHANNEL && isNewApp) {
            postToHandler(new Runnable() {
                @Override
                public void run() {
                    sendEvent(EventInterface.EVENT_TYPE_BACKGROUND_SIDE_CHANNEL_CANCELED,
                            new int[] {
                                    0, 0
                            });

                }
            });
        }
    }

    private ConfigManager mConfigManager = new ConfigManager(new DigitalPenConfig(),
            mConfigChangeListener);

    private ActivityManager mActivityManager;
    private Context mContext;
    private View mStatusBarOverlayView;
    private WindowManager mWindowManager;
    private android.view.WindowManager.LayoutParams mWindowLayoutParams;

    private ServiceNotifications mServiceNotifications;
    /**
     * Those are a lists of callbacks that have been registered with the
     * service. Note that this is package scoped (instead of private) so that it
     * can be accessed more efficiently from inner classes.
     */
    private final RemoteCallbackList<IDigitalPenDataCallback> mDataCallback = new RemoteCallbackList<IDigitalPenDataCallback>();
    private final RemoteCallbackList<IDigitalPenEventCallback> mEventCallback = new RemoteCallbackList<IDigitalPenEventCallback>();

    /**
     * We differentiate between the feature state and the daemon state. The
     * feature state is enabled when pen input is requested by the system.
     * During the time the feature is enabled, the daemon may be either enabled
     * or disabled. For example, during times the screen is off or during aDSP
     * sub-system restart, the daemon is disabled.
     */
    private static boolean mFeatureEnabled;
    private static boolean mDaemonEnabled = false;
    private IDigitalPenDimensionsCallback mDimensionsCallback;

    /**
     * Turn the pen daemon off when the main screen is off and turn it on when
     * the screen is on. This feature can be enabled/disabled from OEM
     * interface.
     */
    private class ScreenState {
        private boolean mEnableStopPenOnScreenOff = false;
        private boolean mDaemonDisabledAfterScreenOffAction = false;

        /**
         * check if the report destination includes socket.
         */
        private boolean reportDestinationHasSocket(byte reportDestination) {
            if (DigitalPenConfig.DP_COORD_DESTINATION_SOCKET == reportDestination ||
                    DigitalPenConfig.DP_COORD_DESTINATION_BOTH == reportDestination) {
                return true;
            }
            return false;
        }

        /**
         * Enable/disable the feature.
         */
        public void setStopPenOnScreenOff(boolean enable) {
            mEnableStopPenOnScreenOff = enable;
            if (mEnableStopPenOnScreenOff) {
                Log.d(TAG, "Enabling stopping the pen when main display is off");
            } else {
                Log.d(TAG, "Disabling stopping the pen when main display is off");
            }
        }

        /**
         * Change the pen daemon state according to the screen state. This is
         * done only if this feature is enabled and no one is registered to
         * side-channel events.
         */
        public void changePenStateAccordingToScreenState(String screenState) {
            if (screenState == Intent.ACTION_SCREEN_ON) {
                // Turn the daemon on only if its last state is off due to
                // screen-off action
                if (mDaemonDisabledAfterScreenOffAction) {
                    enable();
                }
            }
            else if (screenState == Intent.ACTION_SCREEN_OFF) {
                DigitalPenConfig currentConfig = mConfigManager.getCurrentConfig();
                boolean sideChannelDestination =
                        (reportDestinationHasSocket(currentConfig
                                .getOnScreenCoordReportDestination())
                                ||
                                reportDestinationHasSocket(currentConfig
                                        .getOffScreenCoordReportDestination()) ||
                        currentConfig.getSendAllDataEventsToSideChannel());

                if (mEnableStopPenOnScreenOff && !sideChannelDestination) {
                    if (isEnabled()) {
                        disableDaemon();
                        // Mark that the last daemon state is off due to
                        // screen-off action
                        mDaemonDisabledAfterScreenOffAction = true;
                    }
                }
            } else {
                Log.e(TAG, "Invalid screen action " + screenState);
            }
        }

        /**
         * Mark that the last daemon state is not because of a screen-on/off
         * action.
         */
        public void resetPenState() {
            mDaemonDisabledAfterScreenOffAction = false;
        }
    }

    ScreenState mScreenState = new ScreenState();

    static {
        Log.d(TAG, "Loaded DigitalPenService library");
        /*
         * Static variable is updated to make sure the function gets into the
         * library
         */
        mRefs++;

    }

    private int getStatusBarHeight(Resources res, String key) {
        int result = 0;

        int resourceId = res.getIdentifier(key, "dimen", "android");
        if (resourceId > 0) {
            result = res.getDimensionPixelSize(resourceId);
        }
        return result;
    }

    protected void postToHandler(Runnable runnable) {
        mHandler.post(runnable);
    }

    protected void setupStatusBarOverlay() {
        final String STATUS_BAR_HEIGHT_RES_NAME = "status_bar_height";
        final int STATUS_BAR_OVERLAY_SIDE_CHANNEL_COLOR = 0x80FF0000;

        Resources res = mContext.getResources();
        int StatusBarOverlayHeight = getStatusBarHeight(res, STATUS_BAR_HEIGHT_RES_NAME);
        mStatusBarOverlayView = new View(mContext);
        mWindowLayoutParams = new WindowManager.LayoutParams(
                WindowManager.LayoutParams.MATCH_PARENT, StatusBarOverlayHeight,
                WindowManager.LayoutParams.TYPE_SYSTEM_OVERLAY,
                WindowManager.LayoutParams.FLAG_LAYOUT_INSET_DECOR
                        | WindowManager.LayoutParams.FLAG_LAYOUT_IN_SCREEN, PixelFormat.TRANSLUCENT);
        mWindowLayoutParams.gravity = Gravity.TOP;
        mStatusBarOverlayView.setLayoutParams(mWindowLayoutParams);

        mStatusBarOverlayView.setBackgroundColor(STATUS_BAR_OVERLAY_SIDE_CHANNEL_COLOR);
        mStatusBarOverlayView.setVisibility(View.GONE);

        mWindowManager = (WindowManager) mContext.getSystemService(Context.WINDOW_SERVICE);

        mWindowManager.addView(mStatusBarOverlayView, mWindowLayoutParams);
    }

    protected void setStatusBarOverlayVisibility(int visibility) {
        if (null == mStatusBarOverlayView) {
            setupStatusBarOverlay();
        }
        mStatusBarOverlayView.setVisibility(visibility);
        mWindowManager.updateViewLayout(mStatusBarOverlayView, mWindowLayoutParams);
    }

    public DigitalPenService(Context context) {
        init(context);
    }

    protected DigitalPenService() {
        // for test purposes only; must call init
    }

    protected void init(Context context) {
        mContext = context;
        if (mContext == null) {
            Log.d(TAG, "mContext is null");
        }

        mServiceNotifications = createServiceNotifications();

        createGlobalSettingsPersister();

        mActivityManager = (ActivityManager) context.getSystemService(Context.ACTIVITY_SERVICE);

        IntentFilter filter = new IntentFilter();
        filter.addAction(LOAD_CONFIG_ACTION);
        filter.addAction(GENERATE_ON_SCREEN_DATA_ACTION);
        filter.addAction(Intent.ACTION_SCREEN_ON);
        filter.addAction(Intent.ACTION_SCREEN_OFF);

        mContext.registerReceiver(
                new BroadcastReceiver() {

                    @Override
                    public void onReceive(Context context, Intent intent) {
                        onReceiveIntent(intent);
                    }
                },
                filter);

        mEposDirObserver = new DirectoryObserver(EPOS_PATH);
        startPostBootThread();
    }

    synchronized protected void onReceiveIntent(Intent intent) {
        Log.d(TAG, "Received intent: " + intent);

        if (intent.getAction() == LOAD_CONFIG_ACTION) {
            try {
                onSidebandLoadConfig(intent);
            } catch (IllegalArgumentException e) {
                e.printStackTrace();
            }

        } else if (intent.getAction() == GENERATE_ON_SCREEN_DATA_ACTION) {
            Scanner scanner = new Scanner(intent
                    .getStringExtra("GenerateOnScreenData"));
            scanner.skip("x=");
            int[] sideButtonsState = new int[] {
                    0, 0, 0
            };
            DigitalPenData data = new DigitalPenData(scanner.nextInt(), 0, 0,
                    0, 0,
                    0, 0, false, sideButtonsState,
                    DigitalPenData.REGION_ON_SCREEN);
            scanner.close();
            invokeDataCallbacks(data);

        } else if (intent.getAction() == Intent.ACTION_SCREEN_ON ||
                intent.getAction() == Intent.ACTION_SCREEN_OFF) {
            mScreenState.changePenStateAccordingToScreenState(intent
                    .getAction());
        }
    }

    protected ServiceNotifications createServiceNotifications() {
        return new ServiceNotifications(mContext);
    }

    private void createGlobalSettingsPersister() {
        final String settingsSymlinkPath = EPOS_PATH + SERVICE_SETTINGS_SYMLINK_FILENAME;
        if (isRunningInTestEnvironment()) {
            // substitute a version of the code that never serializes
            mGlobalSettingsPersister = new GlobalSettingsPersister(settingsSymlinkPath) {
                @Override
                public void serialize(DigitalPenConfig config) {
                    // do nothing
                }
            };
        } else {
            mGlobalSettingsPersister = new GlobalSettingsPersister(settingsSymlinkPath);
        }
    }

    // Use judiciously. Should really subclass to vary test behavior.
    // Only acceptable for bootstrapping in constructor.
    private boolean isRunningInTestEnvironment() {
        // Assumes if the app isn't in system/framework, it must be a test
        return !mContext.getApplicationInfo().publicSourceDir.startsWith("/system/framework");
    }

    /** Cache events as they are received from the daemon for later re-sending */
    private void cacheEvent(int eventType, int[] params) {
        switch (eventType) {
            case DigitalPenEvent.TYPE_POWER_STATE_CHANGED:
                cachedCurrentPowerState = params[0];
                cachedPreviousPowerState = params[1];
                break;
            case DigitalPenEvent.TYPE_MIC_BLOCKED:
                cachedMicsBlocked = params[0];
                break;
            case DigitalPenEvent.TYPE_PEN_BATTERY_STATE:
                cachedPenBatteryState = params[0];
                cachedPenBatteryLevel = params[1];
                break;
            default:
                break;

        }
    }

    /** Send an event to all registered users */
    protected void sendEvent(int eventType, int[] params) {

        // also broadcast an intent
        Intent eventIntent = EventInterface.makeEventIntent(eventType, params);
        mContext.sendBroadcastAsUser(eventIntent, UserHandle.ALL);

        DigitalPenEvent eventToSend = new DigitalPenEvent(eventType, params);

        // Invoke callback functions with the event received from socket
        int i = mEventCallback.beginBroadcast();
        while (i-- > 0) {
            try {
                mEventCallback.getBroadcastItem(i).onDigitalPenPropEvent(
                        eventToSend);
            } catch (RemoteException e) {
                // The RemoteCallbackList will take care of removing
                // the dead object for us.
            }
        }
        mEventCallback.finishBroadcast();

        mServiceNotifications.sendEvent(eventType, params);
    }

    private static final int MSG_ID_LOAD_CONFIG = 1;
    private static final int MSG_ID_ACCEL_UPDATE = 2;
    private static final int NO_APP_PID = 0;
    final int SHORT_SLEEP_TIME = 500; // 500 msec sleep time, for pollers
    final int LONG_SLEEP_TIME = 1500; // 1500 msec sleep time
    // msec, max time it will take the daemon to exit if failed during start
    // sequence
    final int DAEMON_START_FAILURE_MAX_TIME = 1500;

    // Main thread's handler
    private final Handler mHandler = new Handler() {

        // Delay between receiving a notification that the daemon has
        // stopped and restarting it, in msec
        private final int DAEMON_RESTART_DELAY = 1000;

        @Override
        public void handleMessage(Message msg) {
            switch (msg.arg1) {
                case DAEMON_STOPPED:
                    disableDaemon();

                    Log.w(TAG, "Scheduling daemon restart in "
                            + DAEMON_RESTART_DELAY + " ms");

                    postDelayed(new Runnable() {
                        public void run() {
                            Log.w(TAG, "Restarting daemon");

                            // Note that failure to restart the daemon will not
                            // result
                            // in re-entrancy as enableDaemon() makes sure that
                            // the
                            // daemon is enabled
                            if (!enableDaemon())
                            {
                                Log.e(TAG, "Daemon could not be restarted. Disabling feature.");
                                disable();
                            }
                        }

                    }, DAEMON_RESTART_DELAY);

                    break;
            }
        }
    };
    private DaemonPollingThread mDaemonPollingThread;
    private ScheduledThreadPoolExecutor mAppPidPoller = new ScheduledThreadPoolExecutor(1);
    private ScheduledFuture<?> mAppPidPollerTask;
    private SmarterStandSensorListener mSmarterStandSensorListener;

    private int mAppPid = NO_APP_PID;
    // data socket
    private static final String DATA_SOCKET_PATH = "/data/usf/epos/data_socket";
    private SocketThread mDataSocketThread;
    private final DataSocketReceiveWorker.OnDataListener mDataSocketListener = new DataSocketReceiveWorker.OnDataListener() {

        @Override
        public void onData(DigitalPenData data) {
            // Only off-screen data is allowed while filtering
            if (mIsFilteringData && data.getRegion() != DigitalPenData.REGION_OFF_SCREEN) {
                return;
            }
            invokeDataCallbacks(data);
        }
    };

    // control socket
    private static final String CONTROL_SOCKET_PATH = "/data/usf/epos/control_socket";
    private SocketThread mControlSocketThread;
    private ControlSocketReceiveWorker.OnEventListener mControlSocketListener = new ControlSocketReceiveWorker.OnEventListener() {

        @Override
        public void onEvent(int eventType, int[] params) {
            cacheEvent(eventType, params);
            sendEvent(eventType, params);
        }
    };
    private DirectoryObserver mEposDirObserver;

    class DaemonPollingThread extends Thread {
        private final Handler mServiceHandler;

        DaemonPollingThread(Handler serviceHandler) {
            mServiceHandler = serviceHandler;
        }

        @Override
        public void run() {
            try {
                Thread.sleep(LONG_SLEEP_TIME);
            } catch (InterruptedException e) {
                return;
            }
            while (true) {
                if ((SystemProperties.get("init.svc.usf_epos", "stopped")).
                        equals("stopped")) { // Daemon has stopped
                    Log.d(TAG, "Digital pen daemon has stopped");
                    // Send event reporting an internal daemon error
                    sendEvent(
                            DigitalPenEvent.TYPE_INTERNAL_DAEMON_ERROR,
                            new int[] {
                                0
                            }); // No error number supported for now,
                    // sending 0
                    // Inform service about Daemon's stopping
                    Message msg = new Message();
                    msg.arg1 = DAEMON_STOPPED;
                    mServiceHandler.sendMessage(msg);
                    break;
                }
                // Sleep for a short amount before trying again
                try {
                    Thread.sleep(SHORT_SLEEP_TIME);
                } catch (InterruptedException e) {
                    return;
                }
            }
        }
    }

    class CheckApplicationStillForeground implements Runnable {

        @Override
        public void run() {
            synchronized (DigitalPenService.this) {
                boolean expectedForeground;
                switch (mCurrentState) {
                    case LEGACY:
                        return; // nothing to check
                    case APP:
                        expectedForeground = true;
                        break;
                    case LEGACY_WITH_SIDE_CHANNEL:
                        expectedForeground = false;
                        break;

                    default:
                        throw new RuntimeException("Unknown state");
                }
                boolean processExists = false;
                boolean processIsForeground = false;
                for (RunningAppProcessInfo process : mActivityManager.getRunningAppProcesses()) {
                    if (process.pid == mAppPid) {
                        processExists = true;
                        processIsForeground = process.importance == RunningAppProcessInfo.IMPORTANCE_FOREGROUND;
                        break;
                    }
                }

                if (!processExists) {
                    Log.w(TAG, "Detected last app (pid " + mAppPid
                            + ") has crashed, releasing settings");
                    releaseActivity(); // sets mAppPid to NO_APP_PID
                }

                if (expectedForeground && !processIsForeground) {
                    Log.w(TAG, "App not in expected foreground state, releasing.");
                    releaseActivity();
                }
            }
        }
    }

    /**
     * getPid() function reads the daemon's pid file and returns its pid.
     *
     * @return int the current daemon's pid -1 in case of an error -2 in case
     *         non-integer is read from the pid file
     */
    int getPid() {
        String str = "";
        StringBuffer buf = new StringBuffer();
        int retPid;
        BufferedReader reader = null;
        try {
            // Try to read pid file located at (pidDirectory + mDaemonName +
            // pidFileExtention) path,
            // this file should include one integer, which is the daemon's pid
            FileInputStream fStream = new FileInputStream(pidDirectory +
                    mDaemonName +
                    pidFileExtention);
            reader = new BufferedReader(new InputStreamReader(fStream));
            while (null != (str = reader.readLine())) {
                buf.append(str);
            }
        } catch (IOException e) {
            return -1;
        } finally {
            if (null != reader) {
                try {
                    reader.close();
                } catch (IOException e) {
                    return -1;
                }
            }
        }

        try {
            retPid = Integer.parseInt(buf.toString());
        } catch (NumberFormatException e) {
            Log.e(toString(), "Daemon pid file does not contain an integer");
            return -2;
        }
        return retPid;
    }

    /**
     * This function tries to stop the daemon as appropriate by sending it a
     * SIGTERM signal, instead of just calling stop service which in turn sends
     * a SIGKILL. Thus, in exterme cases, where an unexpected error happens,
     * this function calls stop service. <b>This function has a retry mechansim
     * with a timeout</b>
     */
    private void stopDaemon() {
        int pid = -1;
        int numTries = 10;
        while (--numTries > 0) {
            pid = getPid();
            if (-2 == pid) { // Problem getting pid
                // Stop daemon using system service stop call
                SystemService.stop(mDaemonName);
            } else if (-1 != pid) { // No problems
                try {
                    // Stop daemon with SIGTERM
                    Runtime.getRuntime().exec("kill -15 " + pid);
                    // Stop smarter stand calculation when daemon is stopped.
                    if (mSmarterStandSensorListener != null) {
                        mSmarterStandSensorListener.stop();
                        mSmarterStandSensorListener = null;
                    }
                    return;
                } catch (IOException e) {
                    Log.e(toString(), e.getMessage());
                }
            }
            // Error occurred, Sleep for short amount before trying again
            try {
                Thread.sleep(SHORT_SLEEP_TIME);
            } catch (InterruptedException e) {
                return;
            }
        }
    }

    /**
     * Dynamically change the configuration of the running daemon.
     */
    protected boolean changeConfig(DigitalPenConfig config)
    {
        if (!isEnabled()) {
            Log.w(TAG, "Ignoring changeConfig when service not enabled");
            return false;
        }
        checkSmarterStandChange(config);
        mScreenState.setStopPenOnScreenOff(config.isStopPenOnScreenOffEnabled());
        return sendControlMessage(MSG_ID_LOAD_CONFIG, config.marshalForDaemon());
    }

    private void checkSmarterStandChange(DigitalPenConfig config) {
        if (config.isSmarterStandEnabled() && mSmarterStandSensorListener == null) {
            mSmarterStandSensorListener =
                    createSmarterStandSensorListener(new AccelerometerChangeCallback() {

                        @Override
                        public void onNewPosition(double accelerometerAngleReading) {
                            processAccelerometerData(accelerometerAngleReading);
                        }
                    }, config.getSmarterStandSupportedOrientation());
            mSmarterStandSensorListener.start();
        } else if (!config.isSmarterStandEnabled() && mSmarterStandSensorListener != null) {
            mSmarterStandSensorListener.stop();
            mSmarterStandSensorListener = null;
        }
    }

    protected SmarterStandSensorListener createSmarterStandSensorListener(
            AccelerometerChangeCallback callback, int supportedOrientation) {
        return new SmarterStandSensorListener(callback,
                (SensorManager) mContext.getSystemService(Context.SENSOR_SERVICE),
                supportedOrientation);
    }

    synchronized protected boolean sendControlMessage(int msgId, byte[] msgBuf) {
        if (!isControlSocketConnected()) {
            Log.w(TAG, "Control socket not connected");
            return false;
        }
        // form message: id + payloadLen + payload
        byte[] msg = ByteBuffer.allocate(2 * (Integer.SIZE / 8) + msgBuf.length).
                order(ByteOrder.LITTLE_ENDIAN).
                putInt(msgId).
                putInt(msgBuf.length).
                put(msgBuf).
                array();

        // send to daemon
        try {
            OutputStream out = getControlStream();
            out.write(msg, 0, msg.length);
        } catch (IOException e) {
            Log.w(TAG, "IOException while trying to write to daemon" +
                    " through socket: " + e.getMessage());
            return false;
        }
        return true;
    }

    protected OutputStream getControlStream() throws IOException {
        return mControlSocketThread.getSocket().getOutputStream();
    }

    protected boolean isControlSocketConnected() {
        if (null == mControlSocketThread)
            return false;

        // Check for connection, the timeout is 2 second
        int num_tries = 4;
        while (!mControlSocketThread.isConnected()) {
            try {
                Thread.sleep(SHORT_SLEEP_TIME);
            } catch (InterruptedException e) {
            }
            if (0 == --num_tries)
                return false;
        }
        return true;
    }

    public boolean onSidebandLoadConfig(Intent intent) {
        if (!isEnabled()) {
            enable();
        }
        SidebandConfigChanger changer = new SidebandConfigChanger(mConfigManager.getGlobalConfig());
        mConfigManager.setGlobalConfig(changer.processIntent(intent));
        return true;
    }

    protected void processAccelerometerData(double d) {
        ByteBuffer buffer = ByteBuffer.allocate(Double.SIZE / 8).
                order(ByteOrder.LITTLE_ENDIAN).
                putDouble(d);
        sendControlMessage(MSG_ID_ACCEL_UPDATE, buffer.array());
    }

    class PollingPostBootThread extends Thread {
        final int SHORT_SLEEP_TIME = 300;

        @Override
        public void run() {
            while (SystemProperties.get("init.svc.usf-post-boot", "running").equals("running")) {
                try {
                    Thread.sleep(SHORT_SLEEP_TIME);
                } catch (InterruptedException e) {
                    Log.e(TAG, "Interrupted while waiting for post boot " + e.getMessage());
                }
            }

            synchronized (DigitalPenService.this) {
                attachSettingsFileObserver();
                setGlobalConfigFromPersister(); // loads global config
                boolean enableDigitalPenOnBoot = mConfigManager.getGlobalConfig()
                        .isStartPenOnBootEnabled();
                if (enableDigitalPenOnBoot) {
                    Log.i(TAG,
                            "usf-post-boot is "
                                    + SystemProperties.get("init.svc.usf-post-boot", "running")
                                    + ", enabling daemon");
                    enable();
                }
                attachCommandFileObserver();
            }
        }

        private void attachCommandFileObserver() {
            // Observer will parse & delete COMMAND_FILE when written
            mEposDirObserver.registerObserver(COMMAND_FILENAME, FileObserver.CLOSE_WRITE,
                    new FileChangeListener() {
                        @Override
                        public void onEvent(int event, String file) {
                            parseAndDeleteCommandFile(true);
                        }
                    });

            // check now in case we missed it during boot
            parseAndDeleteCommandFile(false);
        }

        private void parseAndDeleteCommandFile(boolean mustBeFound) {
            try {
                final File commandFile = new File(EPOS_PATH + COMMAND_FILENAME);
                Scanner scan = new Scanner(commandFile);
                String line = scan.nextLine();
                scan.close();

                // Delete file before handling to avoid endless loop if
                // enable/disable fails.
                // This can happen when issuing the command before boot is
                // finished, or if
                // command is leftover from prior boot.
                commandFile.delete();

                // Handle the command
                Log.i(TAG, "Got command from " + commandFile.getAbsolutePath() + ": " + line);
                if (line.equalsIgnoreCase("enable")) {
                    synchronized (DigitalPenService.this) {
                        enable();
                    }
                } else if (line.equalsIgnoreCase("disable")) {
                    synchronized (DigitalPenService.this) {
                        disable();
                    }
                } else {
                    Log.w(TAG, "Unknown command");
                }
            } catch (FileNotFoundException e) {
                if (mustBeFound) {
                    e.printStackTrace();
                }
            }
        }

        private void attachSettingsFileObserver() {

            // The ultrasound settings app will delete and create a new symlink
            // when the settings change.
            mEposDirObserver.registerObserver(SERVICE_SETTINGS_SYMLINK_FILENAME,
                    FileObserver.CREATE, new FileChangeListener() {

                        @Override
                        public void onEvent(int event, String file) {
                            synchronized (DigitalPenService.this) {
                                if (!isEnabled()) {
                                    return;
                                }
                                Log.i(TAG, "Detected change to "
                                        + SERVICE_SETTINGS_SYMLINK_FILENAME +
                                        ", toggling service");
                                if (!disable()) {
                                    Log.e(TAG,
                                            "Couldn't disable service after global settings change");
                                }
                                if (!enable()) {
                                    Log.e(TAG,
                                            "Couldn't enable service after global settings change");
                                }
                            }
                        }
                    });
        }

    }

    private void setGlobalConfigFromPersister() {
        mConfigManager.setGlobalConfig(mGlobalSettingsPersister.deserialize());
        notifyOffScreenDimensionsChange();
    }

    private void notifyOffScreenDimensionsChange() {
        float[] origin = new float[3];
        float[] endX = new float[3];
        float[] endY = new float[3];
        final int X = 0;
        final int Y = 1;
        final int OFF_SCREEN_MAX_AXIS_RESOLUTION = 1800;

        mConfigManager.getGlobalConfig().getOffScreenPlane(origin, endX, endY);

        float height = Math.abs(origin[Y] - endY[Y]);
        float width = Math.abs(endX[X] - origin[X]);
        float resolutionProportion = width / height;

        if (height > width) {
            height = OFF_SCREEN_MAX_AXIS_RESOLUTION;
            width = OFF_SCREEN_MAX_AXIS_RESOLUTION * resolutionProportion;
        } else {
            width = OFF_SCREEN_MAX_AXIS_RESOLUTION;
            height = OFF_SCREEN_MAX_AXIS_RESOLUTION * (1 / resolutionProportion);
        }

        try {
            if (mDimensionsCallback != null) {
                mDimensionsCallback.onDimensionsChange(Math.round(width), Math.round(height));
            } else {
                Log.e(TAG, "Could not set off screen display dimensions since callback is null");
            }
        } catch (RemoteException e) {
            Log.e(TAG, "onDimensionsChange callback threw RemoteException");
            e.printStackTrace();
        }
    }

    /*
     * Starts daemon on boot
     */
    public void startPostBootThread() {
        logFunctionCalled();
        PollingPostBootThread pollingPostBootThread = new PollingPostBootThread();
        pollingPostBootThread.start();
    }

    protected boolean checkCallerAccess() {
        if (Process.SYSTEM_UID != Binder.getCallingUid()) {
            Log.e(toString(), "Access denied, caller UID=" + Binder.getCallingUid() +
                    ", required UID=" + Process.SYSTEM_UID);
            return false;
        }
        return true;
    }

    /**
     * INTERFACE METHODS
     */

    /*
     * Register to notifications on off-screen area dimension changes
     */
    @Override
    public void registerOffScreenDimensionsCallback(IDigitalPenDimensionsCallback callback) {
        logFunctionCalled();

        if (!checkCallerAccess()) {
            return;
        }

        mDimensionsCallback = callback;
    }

    /**
     * OEM INTERFACE METHODS
     */

    /*
     * Enable the daemon and make a connection with it through the sockets. Note
     * the difference between the daemon state and the feature state: While the
     * pen feature is enabled, there can be periods during which the daemon is
     * disabled, such as while the screen is off or during aDSP sub-system
     * restart.
     */
    private synchronized boolean enableDaemon() {
        logFunctionCalled();

        if (!(SystemProperties.get("init.svc.usf_epos", "stopped")).
                equals("running")) {
            // Start the daemon
            SystemService.start(mDaemonName);
            // Sleep for short amount until the daemon state property
            // is updated
            try {
                Thread.sleep(DAEMON_START_FAILURE_MAX_TIME);
            } catch (InterruptedException e) {
                return false;
            }
        }

        if (!(SystemProperties.get("init.svc.usf_epos", "stopped")).
                equals("running")) { // Couldn't start usf_epos because of a
                                     // system error
            // Nothing we can do to fix this, we return false in a hope that the
            // user will later try to enable it again.
            Log.e(toString(), "Couldn't start the daemon");
            return false;
        }

        mDaemonEnabled = true;

        // Create and start the threads

        if ((null == mControlSocketThread) || !mControlSocketThread.isAlive()) {
            mControlSocketThread = new SocketThread(CONTROL_SOCKET_PATH,
                    new ControlSocketReceiveWorker(mControlSocketListener));
            mControlSocketThread.start();
        }
        if ((null == mDataSocketThread) || !mDataSocketThread.isAlive()) {
            mDataSocketThread = new SocketThread(DATA_SOCKET_PATH,
                    new DataSocketReceiveWorker(mDataSocketListener));
            mDataSocketThread.start();
        }
        if ((null == mDaemonPollingThread) || !mDaemonPollingThread.isAlive()) {
            mDaemonPollingThread = new DaemonPollingThread(mHandler);
            mDaemonPollingThread.start();
        }
        if (mAppPidPollerTask == null) {
            mAppPidPollerTask = mAppPidPoller.scheduleWithFixedDelay(
                    new CheckApplicationStillForeground(),
                    0, SHORT_SLEEP_TIME, TimeUnit.MILLISECONDS);
        }

        return true;
    }

    /*
     * Disable the daemon
     */
    private synchronized boolean disableDaemon() {
        logFunctionCalled();

        if (!mDaemonEnabled) { // Already disabled
            return false;
        }
        // Stop the polling thread, to avoid disable() re-entrant
        if (null != mDaemonPollingThread) {
            mDaemonPollingThread.interrupt();
            try {
                // Wait for polling thread to die, it's dangerous to stop daemon
                // while this thread is running.
                mDaemonPollingThread.join();
            } catch (InterruptedException e) {
            }
            mDaemonPollingThread = null;
        }
        // Make sure daemon has stopped
        while ((SystemProperties.get("init.svc.usf_epos", "stopped")).
                equals("running")) {
            stopDaemon();
            // Sleep some time to give the daemon chance to close
            try {
                Thread.sleep(LONG_SLEEP_TIME);
            } catch (InterruptedException e) {
            }
        }

        // Stop the threads
        if (null != mControlSocketThread) {
            mControlSocketThread.interrupt();
            try {
                mControlSocketThread.join();
            } catch (InterruptedException e) {
            }
            mControlSocketThread = null;
        }
        if (null != mDataSocketThread) {
            mDataSocketThread.interrupt();
            try {
                mDataSocketThread.join();
            } catch (InterruptedException e) {
            }
            mDataSocketThread = null;
        }

        if (mAppPidPollerTask != null) {
            try {
                mAppPidPollerTask.cancel(false);
                mAppPidPollerTask.get();
            } catch (CancellationException e) {
                // expected; we just cancelled
            } catch (Exception e) {
                e.printStackTrace(); // unexpected
            }
            mAppPidPollerTask = null;
        }

        mDaemonEnabled = false;

        return true;
    }

    /**
     * OEM INTERFACE METHODS
     */

    /*
     * Enable the Digital Pen feature
     */
    @Override
    public synchronized boolean enable() {
        logFunctionCalled();

        if (!checkCallerAccess()) {
            return false;
        }

        if (!enableDaemon())
        {
            return false;
        }
        setGlobalConfigFromPersister();
        mScreenState.resetPenState();

        mFeatureEnabled = true;

        // set global config as current config
        changeConfig(mConfigManager.getGlobalConfig());
        mServiceNotifications.notifyPenEnabled();

        return true;
    }

    /*
     * Disable the Digital Pen feature
     */
    @Override
    public synchronized boolean disable() {
        logFunctionCalled();

        if (!checkCallerAccess()) {
            return false;
        }

        disableDaemon();

        mScreenState.resetPenState();
        // Send an event informing programs that the feature has been disabled
        sendEvent(DigitalPenEvent.TYPE_POWER_STATE_CHANGED,
                new int[] {
                        DigitalPenEvent.POWER_STATE_OFF,
                        DigitalPenEvent.POWER_STATE_ACTIVE
                });

        // Remove notifications
        mServiceNotifications.notifyPenDisabled();
        mHandler.post(new Runnable() {
            public void run() {
                setStatusBarOverlayVisibility(View.GONE);
            }
        });

        // Clear battery level cache
        cachedPenBatteryLevel = DigitalPenEvent.BATTERY_LEVEL_UNAVAILABLE;

        mFeatureEnabled = false;

        return true;
    }

    /*
     * Checks whether the Digital Pen feature is enabled or not, Note that the
     * feature may be enabled, but the daemon may be disabled, such as during
     * times the screen is off or aDSP sub-system restart is in progress.
     */
    @Override
    public boolean isEnabled() {
        logFunctionCalled();

        return mFeatureEnabled;
    }

    /*
     * Set the global configuration for the digital pen framework
     */
    @Override
    synchronized public boolean setGlobalConfig(DigitalPenConfig config) {
        logFunctionCalled();

        if (!checkCallerAccess()) {
            return false;
        }

        mGlobalSettingsPersister.serialize(config);
        mConfigManager.setGlobalConfig(config);
        return true;
    }

    private void logFunctionCalled() {
        Thread currentThread = Thread.currentThread();
        StackTraceElement[] stackTrace = currentThread.getStackTrace();
        // stackTrace[3] has calling function:
        // 0: getThreadStackTrace
        // 1: getStackTrace
        // 2: this function
        // 3: calling function
        Log.i(TAG, stackTrace[3].getMethodName() + " called");
    }

    /*
     * Get the configuration for the digital pen framework
     */
    @Override
    public DigitalPenConfig getConfig() {
        logFunctionCalled();
        return mConfigManager.getGlobalConfig();
    }

    /*
     * Registers a callback function for proprietary data
     */
    @Override
    synchronized public boolean registerDataCallback(IDigitalPenDataCallback cb) {
        logFunctionCalled();
        if (cb != null) {
            mDataCallback.register(cb, new Integer(getCallingPidOverridable()));
            return true;
        }
        return false;
    }

    /*
     * Registers a callback function for proprietary event
     */
    @Override
    public boolean registerEventCallback(IDigitalPenEventCallback cb) {
        logFunctionCalled();
        if (cb != null) {
            mEventCallback.register(cb);
            try {
                for (DigitalPenEvent e : CAPABILITIES_TABLE) {
                    cb.onDigitalPenPropEvent(e);
                }
                return true;
            } catch (RemoteException e) {
                e.printStackTrace();
                return false;
            }
        }
        return false;
    }

    /*
     * Unregisters a registered callback function for proprietary data
     */
    @Override
    synchronized public boolean unregisterDataCallback(IDigitalPenDataCallback cb) {
        logFunctionCalled();
        if (cb != null) {
            mDataCallback.unregister(cb);
            return true;
        }
        return false;
    }

    /*
     * Unregisters a registered callback function for proprietary event
     */
    @Override
    public boolean unregisterEventCallback(IDigitalPenEventCallback cb) {
        logFunctionCalled();
        if (cb != null) {
            mEventCallback.unregister(cb);
            return true;
        }
        return false;
    }

    /**
     * APPLICATION INTERFACE METHODS
     */

    @Override
    synchronized public boolean applyAppSettings(Bundle settings) {
        logFunctionCalled();
        mConfigManager.applyAppSettings(settings);
        return true;
    }

    // protected so it can be overridden in testing
    protected int getCallingPidOverridable() {
        return Binder.getCallingPid();
    }

    private void logNewAppProcess(int pid) {
        String processName = "<unknown>"; // should only happen in testing
        for (RunningAppProcessInfo process : mActivityManager.getRunningAppProcesses()) {
            if (process.pid == pid) {
                processName = process.processName;
                break;
            }
        }
        Log.d(TAG, "Detected new application: " + processName + "(pid: " + pid + ")");
    }

    @Override
    synchronized public boolean releaseActivity() {
        // This function should be synchronized since it can be called from
        // polling thread and application threads.
        logFunctionCalled();
        mConfigManager.releaseApp();
        return false;
    }

    private void enableBackgroundSideChannelIndicators() {
        mServiceNotifications.backgroundListenerEnabled(true);
        mHandler.post(new Runnable() {
            public void run() {
                setStatusBarOverlayVisibility(View.VISIBLE);
            }
        });
    }

    private void disableBackgroundSideChannelIndicators() {
        mServiceNotifications.backgroundListenerEnabled(false);
        // Inform service that the visibility of the status bar
        // should change
        mHandler.post(new Runnable() {
            public void run() {
                setStatusBarOverlayVisibility(View.GONE);
            }
        });
    }

    private void removeOldAppDataCallbacks(int pidToRemove) {
        if (pidToRemove == NO_APP_PID) {
            return;
        }
        int count = mDataCallback.beginBroadcast(); // lock the list for removal
        for (int i = 0; i < count; ++i) {
            Integer pid = (Integer) mDataCallback.getBroadcastCookie(i);
            if (pidToRemove == pid) {
                mDataCallback.unregister(mDataCallback.getBroadcastItem(i));
            }
        }
        mDataCallback.finishBroadcast();
    }

    synchronized protected void invokeDataCallbacks(DigitalPenData dataToSend) {
        // Invoke callback functions, with the data received from socket
        int i = mDataCallback.beginBroadcast();
        while (i-- > 0) {
            try {
                mDataCallback.getBroadcastItem(i).onDigitalPenPropData(dataToSend);
            } catch (RemoteException e) {
                // The RemoteCallbackList will take care of removing
                // the dead object for us.
            }
        }
        mDataCallback.finishBroadcast();
    }
}
