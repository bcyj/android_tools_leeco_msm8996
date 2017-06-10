/* Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.ims.vt;

import android.content.Context;
import android.hardware.camera2.CameraAccessException;
import android.util.Size;
import android.util.Log;
import android.view.Surface;

/**
 * Base class used to support different camera implementations.
 * The class is used to hide implementation details of ImsCamera which is based on {@code camera1}
 * APIs and {@code Camera2} which is based on Camera2 APIs.
 */
public abstract class Camera {
    private static final String TAG = "VideoCall_Camera";
    Listener mListener;

    protected Context mContext;
    protected String mCameraId;

    public Camera(Context context, String id, Listener listener) {
        mContext = context;
        mCameraId = id;
        mListener = listener;
    }

    public interface Listener {
        /**
         * Notifies about camera failures.
         * @param camera Camera where the failure detected.
         * @param error Error code of the failures.
         */
        void onError(Camera camera, int error);
    }

    /*package*/
    static class ConfigIms {
        public ConfigIms(int w, int h, int f) {
            width = w;
            height = h;
            fps = f;
        }

        int width;
        int height;
        int fps;

        @Override
        public String toString() {
            return "Camera.ConfigIms(width=" + width + ", height=" + height+ ", fps=" + fps + ")";
        }
    }

    static public class CameraException extends CameraAccessException {
        public static final int CAMERA_ERROR = CameraAccessException.CAMERA_ERROR;

        public CameraException(int problem) {
            super(problem);
        }

        public CameraException(int problem, String message) {
            super(problem, message);
        }

        public CameraException(String message) {
            this(CAMERA_ERROR, null);
        }
    }

    /**
     * Retrieves the preview size. The preview size is used to configure the preview surface.
     * @throws CameraAccessException
     */
    public abstract Size getPreviewSize() throws CameraAccessException;

    /**
     * Opens the camera.
     * @throws CameraAccessException
     */
    public abstract void open() throws CameraAccessException;

    /**
     * Closes the camera.
     */
    public abstract void close();

    /**
     * Sets camera zoom.
     * @param v Camera zoom value.
     * @throws CameraAccessException
     */
    public abstract void setZoom(float v) throws CameraAccessException;

    /**
     * Reconfigures camera with negotiated with peer configuration.
     * Note: Preview and Recording must be stopped prior to reconfiguring the camera.
     * @param cfg Negotated configuration.
     * @throws CameraAccessException
     */
    public abstract void reconfigure(ConfigIms cfg) throws CameraAccessException;

    /**
     * Checks if zoom supported.
     * @return true if zoom supported, false otherwise
     * @throws CameraAccessException
     */
    public abstract boolean isZoomSupported() throws CameraAccessException;

    /**
     * Gets maximum supported zoom.
     * @return Maximum supported zoom.
     * @throws CameraAccessException
     */
    public abstract float getMaxZoom() throws CameraAccessException;

    /**
     * Starts camera preview.
     * @param surface Preview surface.
     * @throws CameraAccessException
     */
    public abstract void startPreview(Surface surface) throws CameraAccessException;

    /**
     * Stops camera preview and recording.
     * @throws CameraAccessException
     */
    public abstract void stopPreview()  throws CameraAccessException;

    /**
     * Starts camera recording and preview.
     * @param previewSurface Preview surface.
     * @param recordingSurface Recording surface.
     * @throws CameraAccessException
     */
    public abstract void startRecording(Surface previewSurface, Surface recordingSurface)
            throws CameraAccessException;

    /**
     * Stops camera recording.
     * @throws CameraAccessException
     */
    public abstract void stopRecording() throws CameraAccessException;

    /**
     * @return true if the camera is open, false otherwise.
     */
    public abstract boolean isOpen();

    /**
     * @return true if preview started, false otherwise.
     */
    public abstract boolean isPreviewStarted();

    /**
     * @return true if recording started, false otherwise.
     */
    public abstract boolean isRecordingStarted();

    /**
     * @return ID of the camera.
     */
    public String getId() {
        return mCameraId;
    }

    @Override
    public String toString() {
        return super.toString() + " - CameraId=" + getId();
    }

    /**
     * Notify listeners about an error.
     * @param error
     */
    protected void notifyOnError(int error) {
        if (mListener!=null) {
            mListener.onError(this, error);
        }
    }

    private static void log(String msg) {
        Log.d(TAG, msg);
    }

    private static void loge(String msg) {
        Log.e(TAG, msg);
    }

    protected static void logNotSupported(String fn) {
        Log.d(TAG, fn + ": Not supported");
    }
}
