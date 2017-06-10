/*
 * Copyright (c) 2014 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package com.qualcomm.qti.ims.internal.media;

import android.graphics.SurfaceTexture;
import android.util.Log;

/**
 * The class is used to hold an {@code android.hardware.Camera} instance.
 * <p>
 * The {@code open()} and {@code release()} calls are similar to the ones in
 * {@code android.hardware.Camera}.
 */

public class ImsCamera {
    private static final String TAG = "VideoCallImsCamera";
    private static final boolean DBG = true;
    private static final short IMS_CAMERA_OPERATION_SUCCESS = 0;

    static {
        System.loadLibrary("sdkimscamera_jni");
    }

    public static native short native_open(int cameraId);

    public static native short native_open(int cameraId, String packageName);

    public native short native_release();

    public native short native_startPreview();

    public native short native_stopPreview();

    public native short native_startRecording();

    public native short native_stopRecording();

    public native short native_setPreviewTexture(SurfaceTexture st);

    public native short native_setDisplayOrientation(int rotation);

    public native boolean native_isZoomSupported();

    public native int native_getMaxZoom();

    public native void native_setZoom(int zoomValue);

    public native short native_setPreviewSize(int width, int height);

    public native short native_setPreviewFpsRange(short fps);

    public static ImsCamera open(int cameraId) throws Exception {
        Log.d(TAG, "open cameraId=" + cameraId);
        return openImpl(cameraId, null);
    }

    public static ImsCamera open(int cameraId, String packageName)
            throws Exception {
        Log.d(TAG, "open cameraId=" + cameraId);
        if (packageName == null)
            throw new IllegalArgumentException();
        return openImpl(cameraId, packageName);
    }

    private static ImsCamera openImpl(int cameraId, String packageName)
            throws Exception {
        final short error = packageName == null ? native_open(cameraId)
                : native_open(cameraId, packageName);
        if (error == IMS_CAMERA_OPERATION_SUCCESS) {
            return new ImsCamera();
        }
        Log.e(TAG, "open cameraId=" + cameraId + " packageName=" + packageName
                + " failed with error=" + error);
        throw new Exception("Failed to open ImsCamera");
    }

    public short release() {
        if (DBG)
            log("release");
        short error = native_release();
        logIfError("release", error);
        return error;
    }

    public short startPreview() {
        if (DBG)
            log("startPreview");
        short error = native_startPreview();
        logIfError("startPreview", error);
        return error;
    }

    public short stopPreview() {
        if (DBG)
            log("stopPreview");
        short error = native_stopPreview();
        logIfError("stopPreview", error);
        return error;
    }

    public short startRecording() {
        if (DBG)
            log("startRecording");
        short error = native_startRecording();
        logIfError("startRecording", error);
        return error;
    }

    public short stopRecording() {
        if (DBG)
            log("stopRecording");
        short error = native_stopRecording();
        logIfError("stopRecording", error);
        return error;
    }

    public short setPreviewTexture(SurfaceTexture st) {
        if (DBG)
            log("setPreviewTexture");
        short error = native_setPreviewTexture(st);
        logIfError("setPreviewTexture", error);
        return error;
    }

    public short setDisplayOrientation(int rotation) {
        if (DBG)
            log("setDisplayOrientation rotation=" + rotation);
        short error = native_setDisplayOrientation(rotation);
        logIfError("setDisplayOrientation", error);
        return error;
    }

    public boolean isZoomSupported() {
        boolean result = native_isZoomSupported();
        if (DBG)
            log("isZoomSupported result=" + result);
        return result;
    }

    public int getMaxZoom() {
        int result = native_getMaxZoom();
        if (DBG)
            log("getMaxZoom result = " + result);
        return result;
    }

    public void setZoom(int zoomValue) {
        if (DBG)
            log("setZoom " + zoomValue);
        native_setZoom(zoomValue);
    }

    public short setPreviewSize(int width, int height) {
        if (DBG)
            log("setPreviewSize");
        short error = native_setPreviewSize(width, height);
        logIfError("setPreviewSize", error);
        return error;
    }

    public short setPreviewFpsRange(short fps) {
        if (DBG)
            log("setPreviewFpsRange");
        short error = native_setPreviewFpsRange(fps);
        logIfError("setPreviewFpsRange", error);
        return error;
    }

    private void log(String msg) {
        Log.d(TAG, msg);
    }

    private void loge(String msg) {
        Log.e(TAG, msg);
    }

    private void logIfError(String methodName, short error) {
        if (error != IMS_CAMERA_OPERATION_SUCCESS) {
            loge(methodName + " failed with error=" + error);
        }
    }
}
