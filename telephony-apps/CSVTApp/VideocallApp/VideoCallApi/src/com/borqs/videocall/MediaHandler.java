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

import android.graphics.SurfaceTexture;
import android.os.Handler;
import android.util.Log;

/**
 * Provides an interface to handle the media part of the video telephony call
 */
public class MediaHandler extends Handler {

    private static final String TAG = "VideoCall_MediaHandler";

    private static SurfaceTexture sSurface;

    // private static native void nativeInit();
    // private static native void nativeDeInit();
    // private static native void nativeHandleRawFrame(byte[] frame);
    // private static native int nativeSetSurface(SurfaceTexture st);
    // private static native short nativeGetNegotiatedFPS();
    // private static native int nativeGetNegotiatedHeight();
    // private static native int nativeGetNegotiatedWidth();
    // private static native void nativeRegisterForMediaEvents();

    public static final int PARAM_READY_EVT = 1;
    public static final int START_READY_EVT = 2;

    /*
     * Initializing default negotiated parameters to a working set of valuesso
     * that the application does not crash in case we do not get the Param ready
     * event
     */
    private static int mNegotiatedHeight = 224;// 240;
    private static int mNegotiatedWidth = 183;// 320;
    private static short mNegotiatedFPS = 20;

    private static boolean isReadyToReceivePreview = false;
    private static VTManager mVTEngine;
    private static int drop = 0;
    private static int mod = 3;

    // static {
    // System.loadLibrary("csvt_jni");
    // }

    /*
     * Initialize Media
     */
    public static void init() {
        Log.d(TAG, "init called");
        // nativeInit();
        mVTEngine.init();
        mVTEngine.connect();
        // registerForMediaEvents();
    }

    /*
     * Deinitialize Media
     */
    public static void deInit() {
        Log.d(TAG, "deInit called");
        // nativeDeInit();
        mVTEngine.disconnect();
    }

    /**
     * Send the camera preview frames to the media module to be sent to the far
     * end party
     *
     * @param frame
     *            raw frames from the camera
     */
    public static void sendPreviewFrame(byte[] frame) {
        Log.d(TAG, "sendPreviewFrame - called nativeHandleFrame");
        if (mVTEngine == null)
            return;
        if (drop == 0) {
            // mod = 5 - mod;
            mVTEngine.nativeHandleRawFrame(frame);
        }
        ++drop;
        drop = drop % 2;
        // drop = drop %mod;

    }

    /**
     * Send the SurfaceTexture to media module
     *
     * @param st
     */
    public static void setSurface(SurfaceTexture st) {
        Log.d(TAG, "setSurface(" + st + ")");
        sSurface = st;
        // nativeSetSurface(st);
        mVTEngine.setRemoteDisplay(st);
    }

    /**
     * Send the SurfaceTexture to media module. This should be called only for
     * re-sending an already created surface
     */
    private static void setSurface() {
        Log.d(TAG, "setSurface()");
        if (sSurface == null) {
            Log.e(TAG, "sSurface is null. So not passing it down");
            return;
        }
        // nativeSetSurface(sSurface);
        mVTEngine.setRemoteDisplay(sSurface);
    }

    /**
     * Get Negotiated FPS
     */
    public static short getNegotiatedFPS() {
        Log.d(TAG, "Negotiated FPS = " + mNegotiatedFPS);
        return mNegotiatedFPS;
    }

    /**
     * Get Negotiated Height
     */
    public static int getNegotiatedHeight() {
        Log.d(TAG, "Negotiated Height = " + mNegotiatedHeight);
        return mNegotiatedHeight;
    }

    /**
     * Get Negotiated Width
     */
    public static int getNegotiatedWidth() {
        Log.d(TAG, "Negotiated Width = " + mNegotiatedWidth);
        return mNegotiatedWidth;
    }

    public static synchronized boolean canSendPreview() {
        return MediaHandler.isReadyToReceivePreview;
    }

    public static synchronized void setIsReadyToReceivePreview(boolean flag) {
        Log.d(TAG, "setIsReadyToReceivePreview = " + flag);
        MediaHandler.isReadyToReceivePreview = flag;
    }

    /**
     * Register for event that will invoke
     * {@link MediaHandler#onMediaEvent(int)}
     */
    private static void registerForMediaEvents() {
        Log.d(TAG, "Registering for Media Callback Events");
        // nativeRegisterForMediaEvents();
    }

    /**
     * Callback method that is invoked when Media events occur
     */
    public static void onMediaEvent(int eventId) {
        Log.d(TAG, "onMediaEvent eventId = " + eventId);
        switch (eventId) {
        case PARAM_READY_EVT:
            Log.d(TAG, "Received PARAM_READY_EVT. Updating negotiated values");
            // mNegotiatedHeight = nativeGetNegotiatedHeight();
            // mNegotiatedWidth = nativeGetNegotiatedWidth();
            // mNegotiatedFPS = nativeGetNegotiatedFPS();

            /*
             * Re-send the surface that was created before. Assumption is that
             * this event happens only after the initial surface texture is
             * created
             */
            setSurface();
            break;
        case START_READY_EVT:
            Log.d(TAG, "Received START_READY_EVT. Camera frames can be sent now");
            // setIsReadyToReceivePreview(true);
            break;
        }

    }

    public static void setVTManager(VTManager VTEngine) {
        mVTEngine = VTEngine;
    }

    public static VTManager getVTManager() {
        return mVTEngine;
    }
}
