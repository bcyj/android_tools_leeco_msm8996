/* Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.ims.vt;

import android.os.Handler;
import android.os.Message;
import android.os.RegistrantList;
import android.os.SystemProperties;
import android.util.Log;
import android.view.Surface;

import com.android.internal.os.SomeArgs;

/**
 * Provides an interface to handle the media part of the video telephony call
 */
class ImsMedia extends Handler {
    private static final String TAG = "VideoCall_ImsMedia";

    // Use QVGA as default resolution
    private static final int DEFAULT_WIDTH = 240;
    private static final int DEFAULT_HEIGHT = 320;
    private static final int DEFAULT_FPS = 20;

    // DPL library's error codes.
    public static final int DPL_INIT_SUCCESSFUL = 0;
    public static final int DPL_INIT_FAILURE = -1;
    public static final int DPL_INIT_MULTIPLE = -2;

    public static final int PLAYER_STATE_STARTED = 0;
    public static final int PLAYER_STATE_STOPPED = 1;

    private static final int LOOPBACK_MODE_HEIGHT = 144;
    private static final int LOOPBACK_MODE_WIDTH = 176;
    private static final int LOOPBACK_MODE_FPS = 20;

    private static boolean mInitCalledFlag = false;

    // Native functions.
    private static native int nativeInit();
    private static native void nativeDeInit();
    private static native void nativeHandleRawFrame(byte[] frame);
    private static native int nativeSetSurface(Surface st);
    private static native void nativeSetDeviceOrientation(int orientation);
    private static native short nativeGetNegotiatedFPS();
    private static native int nativeGetNegotiatedHeight();
    private static native int nativeGetNegotiatedWidth();
    private static native int nativeGetUIOrientationMode();
    private static native int nativeGetPeerHeight();
    private static native int nativeGetPeerWidth();
    private static native int nativeGetVideoQualityIndication();
    private static native int nativeRequestRtpDataUsage(int mediaId);
    private static native void nativeRegisterForMediaEvents(ImsMedia instance);
    private static native Surface nativeGetRecordingSurface();


    public static final int MEDIA_EVENT = 0;

    // Following values are from the IMS VT API documentation
    public static final int CAMERA_PARAM_READY_EVT = 1;
    public static final int START_READY_EVT = 2;
    public static final int PLAYER_START_EVENT = 3;
    public static final int PLAYER_STOP_EVENT = 4;
    public static final int DISPLAY_MODE_EVT = 5;
    public static final int PEER_RESOLUTION_CHANGE_EVT = 6;
    public static final int VIDEO_QUALITY_EVT = 7;
    public static final int DATA_USAGE_EVT = 8;
    public static final int STOP_READY_EVT = 9;

    // UI Orientation Modes
    private static final int LANDSCAPE_MODE = 1;
    private static final int PORTRAIT_MODE = 2;
    private static final int CVO_MODE = 3;

    // Following values 0, 1, and 2 are from the IMS VT API documentation
    public static final int VIDEO_QUALITY_UNKNOWN = -1;
    public static final int VIDEO_QUALITY_LOW = 0;
    public static final int VIDEO_QUALITY_MEDIUM = 1;
    public static final int VIDEO_QUALITY_HIGH = 2;

    /*
     * Initializing default negotiated parameters to a working set of values so that the application
     * does not crash in case we do not get the Param ready event
     */
    private int mNegotiatedHeight = DEFAULT_WIDTH;
    private int mNegotiatedWidth = DEFAULT_HEIGHT;
    private int mUIOrientationMode = PORTRAIT_MODE;
    private short mNegotiatedFps = DEFAULT_FPS;

    private int mPeerHeight = DEFAULT_HEIGHT;
    private int mPeerWidth = DEFAULT_WIDTH;
    private int mVideoQualityLevel = VIDEO_QUALITY_UNKNOWN;

    private boolean mIsMediaLoopback = false;
    private Surface mSurface = null;
    private Surface mRecordingSurface = null;

    private IMediaListener mMediaListener;
    private CameraListener mCameraListener;

    // Use a singleton
    private static ImsMedia mInstance;

    /**
     * This method returns the single instance of ImsMedia object *
     */
    public static synchronized ImsMedia getInstance() {
        if (mInstance == null) {
            mInstance = new ImsMedia();
        }
        return mInstance;
    }

    /**
     * Private constructor for ImsMedia
     */
    private ImsMedia() {
        initializemIsMediaLoopback();
    }

    // TODO create CameraParams class and pass it as an argument to onParamReady
    public interface IMediaListener {
        void onDisplayModeEvent();

        void onPeerResolutionChanged(int width, int height);

        void onPlayerStateChanged(int state);

        void onVideoQualityEvent(int videoQuality);

        void onDataUsageChanged(int mediaId, long uplink, long downlink);
    };

    public interface CameraListener {
        void onCameraConfigChanged(int width, int height, short fps, Surface surface);
        void onRecordingEnabled();
        void onRecordingDisabled();
        void onMediaDeinitialized();
    };

    /*
     * public abstract CameraRelatedEventListener implements ICameraRelatedEventListener { void
     * onParamReadyEvent() {} void onStartReadyEvent(){} void onStopReadyEvent(){} };
     */
    static {
        System.loadLibrary("imsmedia_jni");
    }

    /*
     * Initialize Media
     * @return DPL_INIT_SUCCESSFUL 0 initialization is successful. DPL_INIT_FAILURE -1 error in
     * initialization of QMI or other components. DPL_INIT_MULTIPLE -2 trying to initialize an
     * already initialized library.
     */
    /* package */int init() {
        if (!mInitCalledFlag) {
            int status = nativeInit();
            log("init called error = " + status);
            switch (status) {
                case DPL_INIT_SUCCESSFUL:
                    mInitCalledFlag = true;
                    registerForMediaEvents(this);
                    break;
                case DPL_INIT_FAILURE:
                    mInitCalledFlag = false;
                    break;
                case DPL_INIT_MULTIPLE:
                    mInitCalledFlag = true;
                    loge("Dpl init is called multiple times");
                    status = DPL_INIT_SUCCESSFUL;
                    break;
            }
            return status;
        }

        // Dpl is already initialized. So return success
        return DPL_INIT_SUCCESSFUL;
    }

    /*
     * Deinitialize Media
     */
    /* package */void deInit() {
        log("deInit called");
        notifyOnMediaDeinitialized();
        nativeDeInit();
        mInitCalledFlag = false;
    }

    private void notifyOnMediaDeinitialized() {
        if (mCameraListener == null) {
            log("notifyOnMediaDeinitialized: Listener is not set.");
            return;
        }
        try {
            mCameraListener.onMediaDeinitialized();
        } catch (Exception e) {
            loge("notifyOnMediaDeinitialized: Error=" + e);
        }
    }

    private void initializemIsMediaLoopback() {
        // Check the Media loopback property
        int property = SystemProperties.getInt("net.lte.VT_LOOPBACK_ENABLE", 0);
        mIsMediaLoopback = (property == 1) ? true : false;
    }

    public void sendCvoInfo(int orientation) {
        log("sendCvoInfo orientation=" + orientation);
        nativeSetDeviceOrientation(orientation);
    }

    /**
     * Send the Surface to media module
     *
     * @param st
     */
    public void setSurface(Surface st) {
        log("setSurface(Surface: " + st + ")");
        mSurface = st;
        nativeSetSurface(st);
    }

    /**
     * Send the Surface to media module. This should be called only for re-sending an already
     * created surface
     */
    public void setSurface() {
        log("setSurface()");
        if (mSurface == null) {
            loge("sSurface is null. So not passing it down");
            return;
        }
        nativeSetSurface(mSurface);
    }

    /**
     * Get Negotiated Height
     */
    public int getNegotiatedHeight() {
        log("Negotiated Height = " + mNegotiatedHeight);
        return mNegotiatedHeight;
    }

    /**
     * Get Negotiated Width
     */
    public int getNegotiatedWidth() {
        log("Negotiated Width = " + mNegotiatedWidth);
        return mNegotiatedWidth;
    }

    public short getNegotiatedFps() {
        log("Negotiated Fps = " + mNegotiatedFps);
        return mNegotiatedFps;
    }

    /**
     * Get recording surface
     */
    public Surface getRecordingSurface() {
        log("RecordingSurface= " + mRecordingSurface);
        return mRecordingSurface;
    }

    /**
     * Get Negotiated Width
     */
    public int getUIOrientationMode() {
        log("UI Orientation Mode = " + mUIOrientationMode);
        return mUIOrientationMode;
    }

    /**
     * Get Peer Height
     */
    public int getPeerHeight() {
        log("Peer Height = " + mPeerHeight);
        return mPeerHeight;
    }

    /**
     * Get Peer Width
     */
    public int getPeerWidth() {
        log("Peer Width = " + mPeerWidth);
        return mPeerWidth;
    }

    /**
     * Get Video Quality level
     */
    public int getVideoQualityLevel() {
        log("Video Quality Level = " + mVideoQualityLevel);
        return mVideoQualityLevel;
    }

    /**
     * Request Call Data Usage
     */
    public void requestCallDataUsage(int mediaId) {
        log("requestCallDataUsage");
        int status = nativeRequestRtpDataUsage(mediaId);
        log("requestCallDataUsage: status = " + status);
    }

    /**
     * Register for event that will invoke {@link ImsMedia#onMediaEvent(int)}
     */
    private void registerForMediaEvents(ImsMedia instance) {
        log("Registering for Media Callback Events");
        nativeRegisterForMediaEvents(instance);
    }

    public void setMediaListener(IMediaListener listener) {
        log("Registering for Media Listener");
        mMediaListener = listener;
    }

    public void setCameraListener(CameraListener listener) {
        mCameraListener = listener;
    }

    private void doOnMediaEvent(int eventId) {
        switch (eventId) {
            case CAMERA_PARAM_READY_EVT:
                log("Received PARAM_READY_EVT. Updating negotiated values");
                if (updatePreviewParams() && mCameraListener != null) {
                    log("Negotiated Camera values mNegotiatedWidth " + mNegotiatedWidth
                        + " mNegotiatedHeight= " + mNegotiatedHeight);
                    mCameraListener.onCameraConfigChanged(mNegotiatedWidth,
                            mNegotiatedHeight, mNegotiatedFps, mRecordingSurface);
                }
                break;
            case PEER_RESOLUTION_CHANGE_EVT:
                mPeerHeight = nativeGetPeerHeight();
                mPeerWidth = nativeGetPeerWidth();
                log("Received PEER_RESOLUTION_CHANGE_EVENT. Updating peer values"
                        + " mPeerHeight=" + mPeerHeight + " mPeerWidth=" + mPeerWidth);
                if (mMediaListener != null) {
                    mMediaListener.onPeerResolutionChanged(mPeerWidth, mPeerHeight);
                }
                break;
            case START_READY_EVT:
                log("Received START_READY_EVT. Camera recording can be started");
                if (mCameraListener != null) {
                    mCameraListener.onRecordingEnabled();
                }
                break;

            case STOP_READY_EVT:
                log("Received STOP_READY_EVT");
                if (mCameraListener != null) {
                    mCameraListener.onRecordingDisabled();
                }
                break;
            case DISPLAY_MODE_EVT:
                mUIOrientationMode = nativeGetUIOrientationMode();
                //Add later with CVO
                //processUIOrientationMode();
                if (mMediaListener != null) {
                    mMediaListener.onDisplayModeEvent();
                }
                break;
            case PLAYER_START_EVENT:
                if (mMediaListener != null) {
                    mMediaListener.onPlayerStateChanged(PLAYER_STATE_STARTED);

                }
                break;
            case PLAYER_STOP_EVENT:
                if (mMediaListener != null) {
                    mMediaListener.onPlayerStateChanged(PLAYER_STATE_STOPPED);
                }
                break;
            case VIDEO_QUALITY_EVT:
                mVideoQualityLevel = nativeGetVideoQualityIndication();
                log("Received VIDEO_QUALITY_EVT" + mVideoQualityLevel);
                if (mMediaListener != null) {
                    mMediaListener.onVideoQualityEvent(mVideoQualityLevel);
                }
                break;
            default:
                loge("Received unknown event id=" + eventId);
        }
    }

    /**
     * Callback method that is invoked when Media events occur
     */
    public void onMediaEvent(int eventId) {
        log("onMediaEvent eventId = " + eventId);
        final Message msg = obtainMessage(MEDIA_EVENT, eventId, 0);
        sendMessage(msg);
    }

    /**
     * Callback method that is invoked when Data Usage event occurs
     */
    private void onDataUsageEvent(int mediaId, long uplink, long downlink) {
        log("onDataUsageEvent mediaId = " + mediaId + " uplink = " + uplink + " downlink = "
                + downlink);
        SomeArgs args = SomeArgs.obtain();
        args.arg1 = mediaId;
        args.arg2 = uplink;
        args.arg3 = downlink;
        Message msg = obtainMessage(DATA_USAGE_EVT, args);
        sendMessage(msg);
    }

    public void handleMessage(Message msg) {
        switch (msg.what) {
            case MEDIA_EVENT:
                doOnMediaEvent(msg.arg1);
                break;
            case DATA_USAGE_EVT:
                SomeArgs args = (SomeArgs) msg.obj;
                try {
                    int mediaId = (int) args.arg1;
                    long uplink = (long) args.arg2;
                    long downlink = (long) args.arg3;
                    if (mMediaListener != null) {
                        mMediaListener.onDataUsageChanged(mediaId, uplink, downlink);
                    }
                } finally {
                    args.recycle();
                }
                break;
            default:
                loge("Received unknown msg id = " + msg.what);
        }
    }

    private static boolean areSurfacesSame(Surface a, Surface b) {
        if (a == null && b == null) {
            return true;
        } else if (a == null || b == null) {
            return false;
        }
        return a.equals(b);
    }

    private synchronized boolean updatePreviewParams() {
        if (mIsMediaLoopback) {
            mNegotiatedHeight = LOOPBACK_MODE_HEIGHT;
            mNegotiatedWidth = LOOPBACK_MODE_WIDTH;
            mNegotiatedFps = LOOPBACK_MODE_FPS;
            return true;
        } else {
            int h = nativeGetNegotiatedHeight();
            int w = nativeGetNegotiatedWidth();
            short fps = nativeGetNegotiatedFPS();
            // TODO Check if IMS-VT library will return null for camera1 case.
            // TODO Check if it is OK to update all camera params if new surface is received.
            Surface surface = nativeGetRecordingSurface();
            if (mNegotiatedHeight != h
                    || mNegotiatedWidth != w
                    || mNegotiatedFps != fps
                    || !areSurfacesSame(mRecordingSurface, surface)) {
                mNegotiatedHeight = h;
                mNegotiatedWidth = w;
                mNegotiatedFps = fps;
                mRecordingSurface = surface;
                return true;
            }
            return false;
        }
    }

    private static void log(String msg) {
        Log.d(TAG, msg);
    }

    private static void loge(String msg) {
        Log.e(TAG, msg);
    }
}
