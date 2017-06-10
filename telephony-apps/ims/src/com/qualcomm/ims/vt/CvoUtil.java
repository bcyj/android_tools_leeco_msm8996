/* Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.ims.vt;

import android.util.Log;

/**
 * Provides utilities for Coordinated Video Orientation (CVO).
 */
public class CvoUtil {
    private static final String TAG = "VideoCall_CvoUtil";
    private static final boolean DBG = true;

    public static final int ORIENTATION_UNKNOWN = -1;
    public static final int ORIENTATION_ANGLE_0 = 0;
    public static final int ORIENTATION_ANGLE_90 = 3;
    public static final int ORIENTATION_ANGLE_180 = 2;
    public static final int ORIENTATION_ANGLE_270 = 1;
    public static final int ORIENTATION_THRESHOLD = 45;

    private CvoUtil() {
    }

    /**
     * Converts device rotation to CVO orientation.
     *
     * @param angle Rotation angle to be converted.
     * @return CVO orientation.
     */
    public static int toOrientation(int angle) {
        if ( isInRange2(angle, 90, ORIENTATION_THRESHOLD)) {
            return ORIENTATION_ANGLE_90;
        } else if ( isInRange2(angle, 180, ORIENTATION_THRESHOLD)) {
            return ORIENTATION_ANGLE_180;
        } else if ( isInRange2(angle, 270, ORIENTATION_THRESHOLD)) {
            return ORIENTATION_ANGLE_270;
        } else {
            return ORIENTATION_ANGLE_0;
        }
    }

    /**
     * @return true if {@code v} is within [ {@code left}, {@code right} ), false otherwise.
     */
    /* package */
    static boolean isInRange(int v, int left, int right) {
        return (v >= left) && (v < right);
    }

    /**
     * @return true if {@code v} is within [ {@code p - radius} , {@code p + radius} ),
     *  false otherwise.
     */
    /* package */
    static boolean isInRange2(int v, int p, int radius) {
        return isInRange(v, p - radius, p + radius);
    }

    private static void log(String msg) {
        Log.d(TAG, msg);
    }

    private static void loge(String msg) {
        Log.e(TAG, msg);
    }

}
