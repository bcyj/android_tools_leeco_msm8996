/*Copyright (c) 2013 Qualcomm Technologies, Inc.
 *All Rights Reserved.
 *Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

/* Copyright (c) 2012, Code Aurora Forum. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 *       copyright notice, this list of conditions and the following
 *       disclaimer in the documentation and/or other materials provided
 *       with the distribution.
 *     * Neither the name of Code Aurora Forum, Inc. nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

package com.borqs.videocall;

import java.io.IOException;

import android.content.Context;
import android.graphics.SurfaceTexture;
import android.hardware.Camera.Parameters;
import android.hardware.Camera.Size;
import android.util.Log;
import com.borqs.videocall.CameraHandler.CameraState;

import java.util.List;

/**
 * Provides an interface for the applications to interact with Camera for the
 * near end preview and sending the frames to the far end and also with Media
 * engine to render the far end video during a Video Call Session.
 */
public class VideoCallManager {
    private static final String TAG = "VideoCallManager";
    private static VideoCallManager sVideoCallManager; // Use a singleton
    private CameraHandler mCameraHandler;

    /** @hide */
    private VideoCallManager(Context context) {
        log("Instantiating VideoCallManager");
        mCameraHandler = CameraHandler.getInstance(context);
    }

    /**
     * This method returns the single instance of VideoCallManager object
     *
     * @param mContext
     */
    public static synchronized VideoCallManager getInstance(Context context) {
        if (sVideoCallManager == null) {
            sVideoCallManager = new VideoCallManager(context);
        }
        return sVideoCallManager;
    }

    /**
     * Initialize the Media
     */
    public void mediaInit() {
        MediaHandler.init();
    }

    /**
     * Deinitialize the Media
     */
    public void mediaDeInit() {
        MediaHandler.deInit();
    }

    /**
     * Send the SurfaceTexture to Media module
     *
     * @param st
     *            SurfaceTexture to be passed
     */
    public void setFarEndSurface(SurfaceTexture st) {
        MediaHandler.setSurface(st);
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
     * Get negotiated FPS
     */
    public int getNegotiatedFPS() {
        return MediaHandler.getNegotiatedFPS();
    }

    public static boolean isMediaReadyToReceivePreview() {
        return MediaHandler.canSendPreview();
    }

    public static void setIsMediaReadyToReceivePreview(boolean flag) {
        MediaHandler.setIsReadyToReceivePreview(flag);
    }

    public static void setVTManager(VTManager VTEngine) {
        MediaHandler.setVTManager(VTEngine);

    }

    public static void sendVideoFrames(byte[] frame) {
        MediaHandler.sendPreviewFrame(frame);

    }

    public byte[] getLastVideoFrames() {
        return mCameraHandler.getLastFrame();
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
    public void startCameraPreview(SurfaceTexture surfaceTexture) throws IOException {
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
     * Return the camera parameters that specifies the current settings of the
     * camera
     *
     * @return camera parameters
     */
    public Parameters getCameraParameters() {
        return mCameraHandler.getCameraParameters();
    }

    /**
     * Set the camera parameters
     *
     * @param parameters
     *            to be set
     */
    public void setCameraParameters(Parameters parameters) {
        mCameraHandler.setCameraParameters(parameters);
        //mCameraHandler.setCameraFMT();
    }

    /**
     * Get the camera ID for the back camera
     *
     * @return camera ID
     */
    public int getBackCameraId() {
        return mCameraHandler.getBackCameraId();
    }

    /**
     * Get the camera ID for the front camera
     *
     * @return camera ID
     */
    public int getFrontCameraId() {
        return mCameraHandler.getFrontCameraId();
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
     * Gets the camera preview size that matches the given width or height to
     * preserve the aspect ratio.
     *
     * @param size
     *            specifies height or width of the camera surface
     * @param isHeight
     *            specifies if the given size is height if true or width if
     *            false
     * @return width and height of camera preview as a Point point.x = width
     *         point.y = height
     */
    public Size getCameraPreviewSize(int targetSize, boolean isHeight) {
        double minDiff = Double.MAX_VALUE;
        Size optimalSize = null;

        List<Size> mSupportedPreviewSizes = mCameraHandler.getSupportedPreviewSizes();
        if (mSupportedPreviewSizes == null)
            return null; // Camera not yet open

        // Try to find a size that matches closely with the required height or
        // width
        for (Size size : mSupportedPreviewSizes) {
            int srcSize = 0;
            if (isHeight) {
                srcSize = size.height;
            } else {
                srcSize = size.width;
            }

            if (Math.abs(srcSize - targetSize) < minDiff) {
                optimalSize = size;
                minDiff = Math.abs(srcSize - targetSize);
            }
        }
        return optimalSize;
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

    private void log(String msg) {
        Log.d(TAG, msg);
    }
}
