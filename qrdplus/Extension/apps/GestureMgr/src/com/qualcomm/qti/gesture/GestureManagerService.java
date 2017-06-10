/**
 * Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.qti.gesture;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

import android.content.ActivityNotFoundException;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.gesture.Gesture;
import android.gesture.GestureLibraries;
import android.gesture.GestureLibrary;
import android.gesture.GesturePoint;
import android.gesture.GestureStroke;
import android.gesture.Prediction;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.os.Handler;
import android.os.Message;
import android.os.PowerManager;
import android.os.PowerManager.WakeLock;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.os.SystemClock;
import android.os.SystemService;
import android.os.SystemVibrator;
import android.os.Vibrator;
import android.text.TextUtils;
import android.util.Log;

public class GestureManagerService extends IGestureManager.Stub implements SensorEventListener {

    private static final int POCKET_SENSOR_TYPE = 0x10001;

    private static final String ACTION_CAMERA =
            "org.codeaurora.snapcam_com.android.camera.CameraGestureActivity";
    private static final String ACTION_FLASH =
            "qualcomm.android.LEDFlashlight_qualcomm.android.LEDFlashlight.LEDGestureActivity";
    private static final String ACTION_MUSIC =
            "com.android.music_com.android.music.MusticGesturePlayActivity";
    private static final String ACTION_MUSIC_PREV =
            "com.android.music_com.android.music.MusticGesturePrevActivity";
    private static final String ACTION_MUSIC_NEXT =
            "com.android.music_com.android.music.MusticGestureNextActivity";

    private static final boolean GESTURE_INIT_STATE = false;

    private static final String TAG = "GestureManagerService";

    private static GestureManagerService sInstance;
    private static final float MIN_PREDICTION_SCORE = 2.5f;

    public static GestureManagerService getDefault() {
        return sInstance;
    }

    public static void init(Context context) {
        sInstance = new GestureManagerService(context);
        try {
            ServiceManager.addService("service.gesture", sInstance);
        } catch (Exception e) {
            Log.d(TAG, "faild to publish gesture service");
        }
    }

    private static final String FEATURE_ENABLE_PATH = "/sys/class/gesture/gesture_ft5x06/enable";
    private static final String POCKET_ENABLE_PATH = "/sys/class/gesture/gesture_ft5x06/pocket";

    private final ArrayList<GesturePoint> mStrokeBuffer = new ArrayList<GesturePoint>();
    private final ArrayList<GestureStroke> mGestureStrokes = new ArrayList<GestureStroke>();

    private final GestureLibrary mGestureLibrary;
    private final Gesture mGesture;
    private final Context mContext;
    private final SensorManager mSensorManager;
    private final WakeLock mWakeLock;

    private boolean screenOn = true;
    private int mWakeLockCount;

    public static final long FADE_OFFSET_GESTURE = 800;

    private static final long TIMEOUT_WAKE_LOCK = 2000;

    private static final int EVENT_GESTURE_DONE = 1;
    private static final int EVENT_WAKE_LOCK_TIMEOUT = 2;

    private Handler mHandler = new Handler() {

        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case EVENT_GESTURE_DONE:
                    onGestureFinish();
                    break;
                case EVENT_WAKE_LOCK_TIMEOUT:
                    releaseWakeLock();
                    break;
            }
        }

    };

    private final BroadcastReceiver screenStateReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            if (Intent.ACTION_SCREEN_OFF.equals(intent.getAction())) {
                screenOn = false;
                onScreenStateChanged();
            } else if (Intent.ACTION_SCREEN_ON.equals(intent.getAction())) {
                screenOn = true;
                onScreenStateChanged();
            }
        }
    };

    private void onScreenStateChanged() {
        if (screenOn) {
            // stop gesture service when screen on and unregister pocket sensor
            SystemService.stop("gestureservice");
            unregisterPocketSensor();
            setNode(false, POCKET_ENABLE_PATH);
        } else {
            // start gesture service when screen off and register pocket sensor
            SystemService.start("gestureservice");
            registerPocketSensorIfNeed();
        }
    }

    private GestureManagerService(Context context) {
        mContext = context;
        mGesture = new Gesture();
        File gesturesCacheFile = mContext.getFileStreamPath("gestures");
        mGestureLibrary = GestureLibraries.fromFile(gesturesCacheFile);
        if (!gesturesCacheFile.exists()) {
            initGesture(ACTION_CAMERA);
            initGesture(ACTION_FLASH);
            initGesture(ACTION_MUSIC);
            initGesture(ACTION_MUSIC_PREV);
            initGesture(ACTION_MUSIC_NEXT);
        }
        mGestureLibrary.load();
        // enable the feature and set it in unpocket mode
        updateDetectState();
        setNode(false, POCKET_ENABLE_PATH);

        // initialize wake lock
        PowerManager pm = (PowerManager) mContext.getSystemService(Context.POWER_SERVICE);
        mWakeLock = pm.newWakeLock(PowerManager.PARTIAL_WAKE_LOCK, TAG);
        mWakeLock.setReferenceCounted(false);

        mSensorManager = (SensorManager) mContext.getSystemService(Context.SENSOR_SERVICE);

        // register listener for screen on/off
        IntentFilter filter = new IntentFilter();
        filter.addAction(Intent.ACTION_SCREEN_OFF);
        filter.addAction(Intent.ACTION_SCREEN_ON);
        context.registerReceiver(screenStateReceiver, filter);

    }

    private void acquireWakeLock() {
        synchronized (mWakeLock) {
            mWakeLock.acquire();
            mHandler.removeMessages(EVENT_WAKE_LOCK_TIMEOUT);
            Message msg = mHandler.obtainMessage(EVENT_WAKE_LOCK_TIMEOUT);
            mHandler.sendMessageDelayed(msg, TIMEOUT_WAKE_LOCK);
        }
    }

    private void releaseWakeLock() {
        synchronized (mWakeLock) {
            if (!mWakeLock.isHeld())
                return;
            mWakeLock.release();
            mHandler.removeMessages(EVENT_WAKE_LOCK_TIMEOUT);
        }
    }

    protected void registerPocketSensorIfNeed() {
        List<GestureAction> gestures = GestureAction.listEnabledActions(mContext);
        if (!gestures.isEmpty()) {
            // just register directly, sensor manager will check whether it is
            // present
            mSensorManager.registerListener(this,
                    mSensorManager.getDefaultSensor(POCKET_SENSOR_TYPE),
                    SensorManager.SENSOR_DELAY_NORMAL);
        }
    }

    protected void unregisterPocketSensor() {
        // just unregister directly, sensor manager will check whether it is
        // present
        mSensorManager.unregisterListener(this);
    }

    public boolean onTouch(int x, int y) throws RemoteException {
        GesturePoint point = new GesturePoint(x, y, System.currentTimeMillis());
        mStrokeBuffer.add(point);
        return true;
    }

    public boolean onTouchUp() throws RemoteException {
        mGestureStrokes.add(new GestureStroke(mStrokeBuffer));
        if (mContext.getResources().getBoolean(R.bool.flag_multiple_stroke)) {
            mHandler.sendMessageDelayed(
                    mHandler.obtainMessage(EVENT_GESTURE_DONE), FADE_OFFSET_GESTURE);
        } else {
            onGestureFinish();
        }
        return false;
    }

    public boolean onTouchDown() throws RemoteException {
        acquireWakeLock();
        mStrokeBuffer.clear();
        if (mHandler.hasMessages(EVENT_GESTURE_DONE)) {
            mHandler.removeMessages(EVENT_GESTURE_DONE);
            Log.d(TAG, "has pending gesture...");
        } else {
            mGestureStrokes.clear();
        }
        return true;
    }

    private void onGestureFinish() {
        Gesture gesture = new Gesture();
        for (GestureStroke strke : mGestureStrokes) {
            gesture.addStroke(strke);
        }
        String actionKey = detect(gesture);
        GestureAction action = null;
        if (actionKey != null && (action = GestureAction.queryAction(actionKey)) != null
                && action.isEnabled() && action.isValid(mContext)) {
            vibrateOnDetected();
            startAction(action.getPackageName(), action.getClassName());
        }
        releaseWakeLock();
    }

    private void startAction(String packageName, String className) {
        Intent intent = new Intent();
        intent.setClassName(packageName, className);
        intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_ACTIVITY_CLEAR_TASK
                | Intent.FLAG_ACTIVITY_EXCLUDE_FROM_RECENTS);
        try {
            mContext.startActivity(intent);
        } catch (ActivityNotFoundException e) {
        }
    }

    public String detect(Gesture gesture) {
        ArrayList<Prediction> predictions = mGestureLibrary.recognize(gesture);
        if (!predictions.isEmpty()) {
            Prediction bestPrediction = predictions.get(0);
            if (bestPrediction.score >= MIN_PREDICTION_SCORE) {
                return bestPrediction.name;
            }
        }
        return null;
    }

    private void vibrateOnDetected() {
        Vibrator mSystemVibrator = new SystemVibrator();
        int nVibratorLength = 100;
        mSystemVibrator.vibrate(nVibratorLength);
        SystemClock.sleep(nVibratorLength);
        mSystemVibrator.cancel();
    }

    public List<Gesture> getGesture(String action) {
        return mGestureLibrary.getGestures(action);
    }

    public boolean saveGesture(String action, Gesture gesture) {
        if (TextUtils.isEmpty(action)) {
            Log.d(TAG, "save gesture error! action is empty");
            return false;
        }
        mGestureLibrary.removeEntry(action);
        mGestureLibrary.addGesture(action, gesture);
        return mGestureLibrary.save();
    }

    public Gesture getDefault(String actionKey) {
        if (ACTION_CAMERA.equals(actionKey)) {
            return Utils.createHalfRoundGesture();
        } else if (ACTION_MUSIC.equals(actionKey)) {
            return Utils.createDoubleUpArrow();
        } else if (ACTION_FLASH.equals(actionKey)) {
            return Utils.createBottomArrow();
        } else if (ACTION_MUSIC_PREV.equals(actionKey)) {
            return Utils.createLeftArrow();
        } else if (ACTION_MUSIC_NEXT.equals(actionKey)) {
            return Utils.createRightArrow();
        }
        return null;
    }

    public boolean updateDetectState() {
        List<GestureAction> gestures = GestureAction.listEnabledActions(mContext);
        // enable the feature when there is one enabled gesture at least.
        return setNode(!gestures.isEmpty(), FEATURE_ENABLE_PATH);
    }

    private boolean setNode(boolean enable, String node) {
        String flag = enable ? "1" : "0";
        Log.d(TAG, "echo " + flag + " to " + node);
        try {
            if (isNodeEnabled(node) == enable) {
                Log.d(TAG, "no change, ignore the node set");
                return true;
            }
        } catch (IllegalStateException e) {
        }
        FileOutputStream fileOS = null;
        try {
            fileOS = new FileOutputStream(node);
            fileOS.write(flag.getBytes());
            return true;
        } catch (Exception e) {
            Log.w(TAG, "failed to set node: " + e);
            return false;
        } finally {
            if (fileOS != null) {
                try {
                    fileOS.close();
                } catch (IOException e) {
                }
            }
        }
    }

    private boolean isNodeEnabled(String node) throws IllegalStateException {
        FileInputStream fileIS = null;
        byte[] buffer = new byte[1024];
        try {
            fileIS = new FileInputStream(node);
            int len = fileIS.read(buffer);
            return new String(buffer, 0, len).trim().equals("1");
        } catch (Exception e) {
            throw new IllegalStateException("failed to get node state");
        } finally {
            if (fileIS != null) {
                try {
                    fileIS.close();
                } catch (IOException e) {
                }
            }
        }
    }

    public boolean resetGesture(String actionKey) {
        Gesture gesture = getDefault(actionKey);
        if (gesture != null) {
            return saveGesture(actionKey, gesture);
        }
        return false;
    }

    private boolean initGesture(String action) {
        String[] compoentNames = GestureAction.getComponentName(action);
        if (compoentNames != null) {
            return new GestureAction(compoentNames[0], compoentNames[1], GESTURE_INIT_STATE).save()
                    && resetGesture(action);
        }
        return false;
    }

    public void onSensorChanged(SensorEvent event) {
        float[] values = null;
        if (!screenOn && event != null && (values = event.values) != null && values.length > 0) {
            setNode(values[0] == 1.0f, POCKET_ENABLE_PATH);
        }
    }

    public void onAccuracyChanged(Sensor sensor, int accuracy) {
    }
}
