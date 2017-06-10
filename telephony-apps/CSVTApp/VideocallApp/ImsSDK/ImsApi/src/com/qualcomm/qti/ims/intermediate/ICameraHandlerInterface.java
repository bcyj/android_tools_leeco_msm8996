/*
 *Copyright (c) 2014 Qualcomm Technologies, Inc.
 *All Rights Reserved.
 *Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package com.qualcomm.qti.ims.intermediate;

import java.io.IOException;
import android.content.Context;
import android.graphics.SurfaceTexture;
import android.hardware.Camera.Parameters;
import android.view.TextureView;

public interface ICameraHandlerInterface {

    /**
     * Enum that defines the various camera states
     */
    public enum CameraState {
        CAMERA_CLOSED, // Camera is not yet opened or is closed
        PREVIEW_STOPPED, // Camera is open and preview not started
        PREVIEW_STARTED, // Preview is active
    };

    void close();

    int getFrontCameraId();

    int getBackCameraId();

    int getCameraDirection();

    void getCameraParameters();

    CameraState getCameraState();

    void getLastFrame();

    int getNumberOfCameras();

    boolean open(int cameraId) throws Exception;

    void setCameraFMT();

    void setCameraParameters(Parameters param);

    void setDisplay(SurfaceTexture sf);

    void setDisplay(TextureView tv);

    void setDisplayOrientation();

    void startCameraRecording();

    void startPreview(SurfaceTexture st);

    void stopCameraRecording();

    void stopPreview();

}
