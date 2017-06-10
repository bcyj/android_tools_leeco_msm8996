/*
 * Copyright (c) 2014 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package com.qualcomm.qti.ims.internal.media;

import android.content.Context;
import android.hardware.SensorManager;
import android.os.AsyncResult;
import android.os.Handler;
import android.os.Registrant;
import android.os.RegistrantList;
import android.util.Log;
import android.view.OrientationEventListener;
import android.view.WindowManager;

/**
 * Provides an interface to handle the CVO - Coordinated Video Orientation part
 * of the video telephony call
 */
public class CvoHandler extends Handler {

    private static final String TAG = "VideoCall_CvoHandler";
    private static final boolean DBG = true;

    private static final int ORIENTATION_ANGLE_0 = 0;
    private static final int ORIENTATION_ANGLE_90 = 1;
    private static final int ORIENTATION_ANGLE_180 = 2;
    private static final int ORIENTATION_ANGLE_270 = 3;
    private static final int ORIENTATION_MODE_THRESHOLD = 45;

    /**
     * Phone orientation angle which can take one of the 4 values
     * ORIENTATION_ANGLE_0, ORIENTATION_ANGLE_90, ORIENTATION_ANGLE_180,
     * ORIENTATION_ANGLE_270
     */
    private int mCurrentOrientation = OrientationEventListener.ORIENTATION_UNKNOWN;
    private RegistrantList mCvoRegistrants = new RegistrantList();

    private Context mContext;
    private WindowManager mWindowManager; // Used to get display rotation.

    private OrientationEventListener mOrientationEventListener;

    public CvoHandler(Context context) {
        mContext = context;

        mOrientationEventListener = createOrientationListener();
        startOrientationListener(false);

        mWindowManager = (WindowManager) mContext
                .getSystemService(Context.WINDOW_SERVICE);
        log("CvoHandler created");
    }

    public interface CvoEventListener {
        /**
         * This callback method will be invoked when the device orientation
         * changes.
         */
        void onDeviceOrientationChanged(int rotation);
    }

    /**
     * Register for CVO device orientation changed event
     */
    public void registerForCvoInfoChange(Handler h, int what, Object obj) {
        log("registerForCvoInfoChange handler= " + h + " what= " + what
                + " obj= " + obj);
        Registrant r = new Registrant(h, what, obj);
        mCvoRegistrants.add(r);
    }

    public void unregisterForCvoInfoChange(Handler h) {
        log("unregisterForCvoInfoChange handler= " + h);
        mCvoRegistrants.remove(h);
    }

    /**
     * Enable sensor to listen for device orientation changes
     */
    public void startOrientationListener(boolean start) {
        log("startOrientationListener " + start);
        if (start) {
            if (mOrientationEventListener.canDetectOrientation()) {
                notifyInitialOrientation();
                mOrientationEventListener.enable();
            } else {
                log("Cannot detect orientation");
            }
        } else {
            mOrientationEventListener.disable();
            mCurrentOrientation = OrientationEventListener.ORIENTATION_UNKNOWN;
        }
    }

    /* Protected to facilitate unittesting. */
    protected void doOnOrientationChanged(int angle) {
        final int newOrientation = calculateDeviceOrientation(angle);
        if (hasDeviceOrientationChanged(newOrientation)) {
            notifyCvoClient(newOrientation);
        }
    }

    /**
     * For CVO mode handling, phone is expected to have only 4 orientations The
     * orientation sensor gives every degree change angle. This needs to be
     * categorized to one of the 4 angles. This method does this calculation.
     *
     * @param angle
     * @return one of the 4 orientation angles ORIENTATION_ANGLE_0,
     *         ORIENTATION_ANGLE_90, ORIENTATION_ANGLE_180,
     *         ORIENTATION_ANGLE_270
     */
    protected static int calculateDeviceOrientation(int angle) {
        int newOrientation = ORIENTATION_ANGLE_0;
        if ((angle >= 0 && angle < 0 + ORIENTATION_MODE_THRESHOLD)
                || (angle >= 360 - ORIENTATION_MODE_THRESHOLD && angle < 360)) {
            newOrientation = ORIENTATION_ANGLE_0;
        } else if (angle >= 90 - ORIENTATION_MODE_THRESHOLD
                && angle < 90 + ORIENTATION_MODE_THRESHOLD) {
            newOrientation = ORIENTATION_ANGLE_90;
        } else if (angle >= 180 - ORIENTATION_MODE_THRESHOLD
                && angle < 180 + ORIENTATION_MODE_THRESHOLD) {
            newOrientation = ORIENTATION_ANGLE_180;
        } else if (angle >= 270 - ORIENTATION_MODE_THRESHOLD
                && angle < 270 + ORIENTATION_MODE_THRESHOLD) {
            newOrientation = ORIENTATION_ANGLE_270;
        }
        return newOrientation;
    }

    /**
     * Detect change in device orientation
     */
    private boolean hasDeviceOrientationChanged(int newOrientation) {
        if (DBG) {
            log("hasDeviceOrientationChanged mCurrentOrientation= "
                    + mCurrentOrientation + " newOrientation= "
                    + newOrientation);
        }
        if (newOrientation != mCurrentOrientation) {
            mCurrentOrientation = newOrientation;
            return true;
        }
        return false;
    }

    /**
     * Send newOrientation to client
     */
    private void notifyCvoClient(int newOrientation) {
        AsyncResult ar = new AsyncResult(null, mCurrentOrientation, null);
        mCvoRegistrants.notifyRegistrants(ar);
    }

    static public int convertMediaOrientationToActualAngle(int newOrientation) {
        int angle = 0;
        switch (newOrientation) {
        case ORIENTATION_ANGLE_0:
            angle = 0;
            break;
        case ORIENTATION_ANGLE_90:
            angle = 90;
            break;
        case ORIENTATION_ANGLE_180:
            angle = 180;
            break;
        case ORIENTATION_ANGLE_270:
            angle = 270;
            break;
        default:
            loge("getAngleFromOrientation: Undefined orientation");
        }
        return angle;
    }

    private void notifyInitialOrientation() {
        final int angle = getCurrentOrientation();
        log("Current orientation is: " + angle);
        if (angle != OrientationEventListener.ORIENTATION_UNKNOWN) {
            doOnOrientationChanged(convertMediaOrientationToActualAngle(angle));
        } else {
            log("Initial orientation is ORIENTATION_UNKNOWN");
        }
    }

    private int getCurrentOrientation() {
        if (mWindowManager != null) {
            return mWindowManager.getDefaultDisplay().getRotation();
        } else {
            loge("WindowManager not available.");
            return OrientationEventListener.ORIENTATION_UNKNOWN;
        }
    }

    private OrientationEventListener createOrientationListener() {
        return new OrientationEventListener(mContext,
                SensorManager.SENSOR_DELAY_NORMAL) {
            @Override
            public void onOrientationChanged(int angle) {
                doOnOrientationChanged(angle);
            }
        };
    }

    private static void log(String msg) {
        Log.d(TAG, msg);
    }

    private static void loge(String msg) {
        Log.e(TAG, msg);
    }

}
