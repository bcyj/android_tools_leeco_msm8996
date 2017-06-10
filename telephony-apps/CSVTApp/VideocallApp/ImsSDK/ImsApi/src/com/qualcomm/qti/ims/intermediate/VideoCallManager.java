/*
 * Copyright (c) 2014 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package com.qualcomm.qti.ims.intermediate;

import android.content.Context;
import android.graphics.SurfaceTexture;
import android.os.AsyncResult;
import android.os.Handler;
import android.os.Message;
import android.util.Log;
import android.view.View;

import java.io.IOException;

import com.qualcomm.qti.ims.intermediate.ICameraHandlerInterface.CameraState;
import com.qualcomm.qti.ims.internal.media.CvoHandler;
import com.qualcomm.qti.ims.internal.media.ImsCamera;
import com.qualcomm.qti.ims.internal.media.ImsCameraHandler;
import com.qualcomm.qti.ims.internal.media.MediaHandler;
import com.qualcomm.qti.ims.internal.media.CvoHandler.CvoEventListener;
import com.qualcomm.qti.ims.internal.media.MediaHandler.MediaEventListener;

/**
 * Provides an interface for the applications to interact with Camera for the
 * near end preview and sending the frames to the far end and also with Media
 * engine to render the far end video during a Video Call Session.
 */
public class VideoCallManager {
    private static final String TAG = "VideoCallManager";
    private static VideoCallManager mInstance; // Use a singleton
    private ImsCameraHandler mCameraHandler;
    private MediaHandler mMediaHandler;
    private CvoHandler mCvoHandler;

    private static final int LOOPBACK_MODE_HEIGHT = 144;
    private static final int LOOPBACK_MODE_WIDTH = 176;

    private int mNumberOfCameras;
    private int mBackCameraId;
    private int mFrontCameraId;
    private SurfaceTexture mSurfaceTexture;
    private static final int CAMERA_UNKNOWN = -1;

    // Property used to indicate that the Media running in loopback mode
    private boolean mIsMediaLoopback = false;
    private Handler mHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            AsyncResult ar;
            switch (msg.what) {
            case CVO_MODE_REQUEST_CHANGED:
                ar = (AsyncResult) msg.obj;
                if (ar != null && ar.result != null && ar.exception == null) {
                    boolean start = (Boolean) ar.result;
                    mCvoHandler.startOrientationListener(start);
                }
                break;
            case CVO_INFO_CHANGED:
                ar = (AsyncResult) msg.obj;
                if (ar != null && ar.result != null && ar.exception == null) {
                    int orientation = (Integer) ar.result;
                    mMediaHandler.sendCvoInfo(orientation);
                    notifyCvoClient(orientation);
                }
                break;
            }
        }
    };

    private CvoEventListener mCvoEventListener;
    private int mIsMediaInitialized = 0; //0 not initialized, 1 initializing, 2 initialized

    public static final int MEDIA_INIT_SUCCESS = 0;
    private static final int CVO_MODE_REQUEST_CHANGED = 0;
    private static final int CVO_INFO_CHANGED = 2;

    /** @hide */
    private VideoCallManager(Context context) {
        log("Instantiating VideoCallManager");
        mCameraHandler = ImsCameraHandler.getInstance(context);
        mBackCameraId = mCameraHandler.getBackCameraId();
        mFrontCameraId = mCameraHandler.getFrontCameraId();

        mMediaHandler = MediaHandler.getInstance();
        mCvoHandler = new CvoHandler(context);
        mMediaHandler.registerForCvoModeRequestChanged(mHandler,
                CVO_MODE_REQUEST_CHANGED, null);
        mCvoHandler.registerForCvoInfoChange(mHandler, CVO_INFO_CHANGED, null);
    }

    private void notifyCvoClient(int orientation) {
        int angle = mCvoHandler
                .convertMediaOrientationToActualAngle(orientation);
        log("handleMessage Device orientation angle=" + angle);
        if (mCvoEventListener != null) {
            mCvoEventListener.onDeviceOrientationChanged(angle);
        }
    }

    /**
     * This method returns the single instance of VideoCallManager object
     *
     * @param mContext
     */
    public static synchronized VideoCallManager getInstance(Context context) {
        if (mInstance == null) {
            mInstance = new VideoCallManager(context);
        }
        return mInstance;
    }

    /**
     * Initialize the Media
     */
    public int mediaInit() {
        return mMediaHandler.init();
    }

    public void earlyMediaInit() {
        if (mIsMediaInitialized == 0) {
            mIsMediaInitialized = 1;
            if (VideoCallManager.MEDIA_INIT_SUCCESS == mediaInit()) {
                mIsMediaInitialized = 2;
                if (mSurfaceTexture != null){
                    MediaHandler.setSurface(mSurfaceTexture);
                    mSurfaceTexture = null;
                }
            } else {
                Log.e(TAG, "mediaInit failed");
                mIsMediaInitialized = 0;
            }
        }
    }

    /**
     * Deinitialize the Media
     */
    public void mediaDeInit() {
        if (mIsMediaInitialized != 0){
            MediaHandler.deInit();
            mSurfaceTexture = null;
            mIsMediaInitialized = 0;
        }
    }

    /**
     * Send the SurfaceTexture to Media module
     *
     * @param st
     *            SurfaceTexture to be passed
     */
    public void setFarEndSurface(SurfaceTexture st) {
        if (mIsMediaInitialized != 2){
            mSurfaceTexture = st;
        }else{
            MediaHandler.setSurface(st);
        }
    }

    /**
     * Send the SurfaceTexture to Media module
     */
    public void setFarEndSurface() {
        MediaHandler.setSurface();
    }

    /**
     * Get negotiated height
     */
    public int getNegotiatedHeight() {
        return MediaHandler.getNegotiatedHeight();
    }

    /**
     * Get negotiated width
     */
    public int getNegotiatedWidth() {
        return MediaHandler.getNegotiatedWidth();
    }

    /**
     * Get negotiated width
     */
    public int getUIOrientationMode() {
        return mMediaHandler.getUIOrientationMode();
    }

    /**
     * Get negotiated FPS
     */
    public short getNegotiatedFps() {
        return MediaHandler.getNegotiatedFps();
    }

    public boolean isCvoModeEnabled() {
        return mMediaHandler.isCvoModeEnabled();
    }

    /**
     * Return the number of cameras supported by the device
     *
     * @return number of cameras
     */
    public int getNumberOfCameras() {
        return mCameraHandler.getNumberOfCameras();
    }

    /**
     * Open the camera hardware
     *
     * @param cameraId
     *            front or the back camera to open
     * @return true if the camera was opened successfully
     * @throws Exception
     */
    public synchronized boolean openCamera(int cameraId) throws Exception {
        return mCameraHandler.open(cameraId);
    }

    /**
     * Start the camera preview if camera was opened previously
     *
     * @param mSurfaceTexture
     *            Surface on which to draw the camera preview
     * @throws IOException
     */
    public void startCameraPreview(SurfaceTexture surfaceTexture)
            throws IOException {
        mCameraHandler.startPreview(surfaceTexture);
    }

    /**
     * Close the camera hardware if the camera was opened previously
     */
    public void closeCamera() {
        mCameraHandler.close();
    }

    /**
     * Stop the camera preview if the camera is open and the preview is not
     * already started
     */
    public void stopCameraPreview() {
        mCameraHandler.stopPreview();
    }

    /**
     * Get the camera ID for the back camera
     *
     * @return camera ID
     */
    public int getBackCameraId() {
        return mBackCameraId;
    }

    /**
     * Get the camera ID for the front camera
     *
     * @return camera ID
     */
    public int getFrontCameraId() {
        return mFrontCameraId;
    }

    /**
     * Return the current camera state
     *
     * @return current state of the camera state machine
     */
    public CameraState getCameraState() {
        return mCameraHandler.getCameraState();
    }

    /**
     * Set the display texture for the camera
     *
     * @param surfaceTexture
     */
    public void setDisplay(SurfaceTexture surfaceTexture) {
        mCameraHandler.setDisplay(surfaceTexture);
    }

    /**
     * Returns the direction of the currently open camera
     *
     * @return one of the following possible values -
     *         CameraInfo.CAMERA_FACING_FRONT - CameraInfo.CAMERA_FACING_BACK -
     *         -1 - No Camera active
     */
    public int getCameraDirection() {
        return mCameraHandler.getCameraDirection();
    }

    /**
     * Set the camera display orientation based on the screen rotation and the
     * camera direction
     */
    void setCameraDisplayOrientation() {
        mCameraHandler.setDisplayOrientation();
    }

    public ImsCamera getImsCameraInstance() {
        return mCameraHandler.getImsCameraInstance();
    }

    public void startCameraRecording() {
        mCameraHandler.startCameraRecording();
    }

    public void stopCameraRecording() {
        mCameraHandler.stopCameraRecording();
    }

    public void setOnParamReadyListener(MediaEventListener listener) {
        mMediaHandler.setMediaEventListener(listener);
    }

    /*
     * Setup a CVO Event listener for triggering UI callbacks like
     * onDeviceOrientationChanged to be invoked directly
     */
    public void setCvoEventListener(CvoEventListener listener) {
        log("setCvoEventListener");
        // TODO: Create a list of listeners or do not allow over-write
        mCvoEventListener = listener;
    }

    public void startOrientationListener(boolean start) {
        if (isCvoModeEnabled()) {
            mCvoHandler.startOrientationListener(start);
        }
    }

    private void log(String msg) {
        Log.d(TAG, msg);
    }

    public void initializeCameraParams() {
        try {
            // Get the parameter to make sure we have the up-to-date value.
            ImsCamera imsCamera = getImsCameraInstance();
            // Set the camera preview size
            if (mIsMediaLoopback) {
                // In loopback mode the IMS is hard coded to render the
                // camera frames of only the size 176x144 on the far end surface
                imsCamera.setPreviewSize(LOOPBACK_MODE_WIDTH,
                        LOOPBACK_MODE_HEIGHT);
            } else {
                log("Set Preview Size directly with negotiated Height = "
                        + getNegotiatedHeight() + " negotiated width= "
                        + getNegotiatedWidth());
                imsCamera.setPreviewSize(getNegotiatedWidth(),
                        getNegotiatedHeight());
                imsCamera.setPreviewFpsRange(getNegotiatedFps());
            }
        } catch (RuntimeException e) {
            Log.e(TAG, "Error setting Camera preview size/fps exception=" + e);
        }
    }

    public void startPreviewAndRecording(SurfaceTexture surfaceTexture) {
        try {
            startCameraPreview(surfaceTexture);
            startCameraRecording();
        } catch (IOException ioe) {
            closeCamera();
            Log.e(TAG, "Exception startPreviewAndRecording, " + ioe.toString());
        }
    }

    /**
     * This method stops the camera recording and preview
     */
    public void stopRecordingAndPreview() {
        stopCameraRecording();
        stopCameraPreview();
    }

    public void initializeCamera(SurfaceTexture surfaceTexture, int cameraId) {
        Log.d(TAG, "Initializing camera id=" + cameraId);

        if (cameraId == CAMERA_UNKNOWN) {
            Log.e(TAG,
                    "initializeCamera: Not initializing camera as mCameraId is unknown");
            return;
        }

        // Open camera if not already open
        try {
            if (false == openCamera(cameraId)) {
                return;
            }
        } catch (Exception e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }
        initializeCameraParams();
        startPreviewAndRecording(surfaceTexture);
    }
}
