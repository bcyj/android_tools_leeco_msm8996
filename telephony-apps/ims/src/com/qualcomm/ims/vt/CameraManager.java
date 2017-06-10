/* Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.ims.vt;

import android.app.admin.DevicePolicyManager;
import android.content.Context;
import android.util.Log;
import android.hardware.camera2.CameraAccessException;

/** CameraManager selects ImsCamera or Camera2
 */
/* package */
class CameraManager {
    private static final String TAG = "VideoCall_CameraManager";
    public static final int CAMERA_TYPE_IMS_CAMERA = 0;
    public static final int CAMERA_TYPE_CAMERA2 = 1;

    private final int sCameraType;
    private final Context mContext;
    private DevicePolicyManager mDpm;
    private static CameraManager sCameraManager;

    private CameraManager(Context context) {
        mContext = context;
        sCameraType = CAMERA_TYPE_IMS_CAMERA; // TODO Update based on a property.
        mDpm = (DevicePolicyManager) mContext.getSystemService(Context.DEVICE_POLICY_SERVICE);
    }

    /* package */
    static synchronized void init(Context context) {
        if (sCameraManager == null) {
            sCameraManager = new CameraManager(context);
        }
    }

    /* package */
    static Camera open(String id, Camera.Listener listener) throws CameraAccessException {
        log("open cameraid= " + id + " listener= " + listener);
        // TODO Add more error check...
        if (sCameraManager.mDpm == null) {
            loge("Error: sCameraManager.mDpm is null");
            throw new RuntimeException("DevicePolicyManager not available");
        }

        if (listener == null) {
            final String msg = "Error: Listener is null";
            loge(msg);
            throw new IllegalArgumentException(msg);
        }

        if (sCameraManager.mDpm.getCameraDisabled(null)) {
            throw new CameraAccessException(CameraAccessException.CAMERA_ERROR,
                    "Camera is disabled.");
        }

        Camera camera = null;
        switch (sCameraManager.sCameraType) {
            case CAMERA_TYPE_IMS_CAMERA:
                camera = new ImsCamera(sCameraManager.mContext, id, listener);
                break;
            case CAMERA_TYPE_CAMERA2:
                camera = new Camera2(sCameraManager.mContext, id, listener);
                break;
        }

        if (camera == null) {
            loge("Error: Undefined camera type.");
            throw new RuntimeException("Undefined camera type.");
        }

        camera.open();
        return camera;
    }


    private static void log(String msg) {
        Log.d(TAG, msg);
    }

    private static void loge(String msg) {
        Log.e(TAG, msg);
    }
}
