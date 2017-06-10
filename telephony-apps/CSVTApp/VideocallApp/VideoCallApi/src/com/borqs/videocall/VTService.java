/* BORQS Software Solutions Pvt Ltd. CONFIDENTIAL
 * Copyright (c) 2012 All rights reserved.
 *
 * The source code contained or described herein and all documents
 * related to the source code ("Material") are owned by BORQS Software
 * Solutions Pvt Ltd. No part of the Material may be used,copied,
 * reproduced, modified, published, uploaded,posted, transmitted,
 * distributed, or disclosed in any way without BORQS Software
 * Solutions Pvt Ltd. prior written permission.
 *
 * No license under any patent, copyright, trade secret or other
 * intellectual property right is granted to or conferred upon you
 * by disclosure or delivery of the Materials, either expressly, by
 * implication, inducement, estoppel or otherwise. Any license
 * under such intellectual property rights must be express and
 * approved by BORQS Software Solutions Pvt Ltd. in writing.
 *
 */

package com.borqs.videocall;

import android.app.Notification;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.graphics.Bitmap;
import android.os.Binder;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.os.SystemProperties;
import android.util.Log;
import android.view.Surface;
import android.media.AudioManager;
import android.hardware.Camera;
import android.hardware.Camera.CameraInfo;
import android.hardware.Camera.Parameters;
import android.hardware.Camera.Size;
import android.graphics.SurfaceTexture;
import android.view.Surface;
import com.borqs.videocall.CameraHandler.CameraState;
import java.io.IOException;

public class VTService extends Service {

    private final static String TAG = "VideoTelePhonyService";

    public enum VTE_STATE {
        CONNECTING, CONNECTED, DISCONNECTED
    };

    // private final static int VT_SERVICE_ID = R.string.vt_service_name;
    // private NotificationManager mNM;

    // private VTNotificationManager mVTnm = null;
    private static Context mContext;
    private static VTManager mVTEngine;
    private static VideoCallManager mVideoCallManager;
    private static boolean mFirstTime = true;

    private AudioManager mAudioManager = null;

    public interface VTServiceListener {
        void onVTConnected(boolean bSuccessOrFail);

        void onVTReleased(boolean bComplete);
    }

    public interface VTServiceNotificationCallback {
        /**
         * Notify application to update the notification (In Call)
         */
        void notifyInVtCall(VTService ctx);

        /**
         * Notify application to close in call notification
         */
        void closeInCallNotification();

        /**
         * Notify application to cancel in call notification
         */

        void cancelNotification(VTService ctx);
    }

    private static VTServiceNotificationCallback mNmgCallback = null;
    private VTServiceListener mListener = null;

    private VTE_STATE mVTEState;

    static private class ACTIONS {
        static final long CONNECT = (1l << 0);
        static final long DISCONNECT = (1l << 1);
        static final long SET_LOCAL_SURFACE = (1l << 2);
        static final long SET_REMOTE_SURFACE = (1l << 3);
        static final long SET_DEVICE = (1l << 4);
        static final long CAPTURE_FRAME = (1l << 5);
        static final long START_RECORDING = (1l << 6);
        static final long STOP_RECORDING = (1l << 7);
        // TODO: to be extended
    };

    static final int VTM_NOTIFY_TYPE_INCOMING_DATA = 0;
    static final int VTM_NOTIFY_TYPE_INFO = 1;
    static final int VTM_NOTIFY_TYPE_ERROR = 2;
    static final int VTM_NOTIFY_TYPE_STATUS_UPDATE = 3;
    static final int VTM_NOTIFY_TYPE_CMD_COMPLETE = 4;
    static final int VTM_NOTIFY_REOPEN_CAMERA = 5;

    private long mPendingActionFlags = 0;

    public static final String VIDEPSTOREPATH = "/local/";
    public static String RECORDPATH = "";

    public static final int MAX_CAMERA_REOPEN_TIMES = 3;
    public static final long CAMERA_REOPREN_DELAY_TIME = 500;
    // Camera related
    private Parameters mParameters;
    private int mZoomMax;
    private int mZoomValue; // The current zoom value
    Size mPreviewSize;

    // Multiple cameras support
    private int mNumberOfCameras;
    private int mFrontCameraId;
    private int mBackCameraId;
    private int mCameraId;

    private SurfaceTexture mCameraSurface;
    private static int mLastvsource = 0;
    private static boolean mLastFrame = false;

    private CameraSetting mCameraSetting;
    private int mFails;
    /**
     * Set callback which will be called by service for notification related
     * action. This opration should be handled appropriately.
     *
     * @param - notification callbacks
     */
    public static void setNotificationCallback(VTServiceNotificationCallback listener) {
        mNmgCallback = listener;
    }

    private VTManager.OnCmdCompListener onVTCmdComplNotify = new VTManager.OnCmdCompListener() {
        public boolean onCmdComp(VTManager vtm, int what, int extra) {
            mHandler.sendMessage(Message
                    .obtain(mHandler, what, extra, VTM_NOTIFY_TYPE_CMD_COMPLETE));
            return true;
        }
    };

    private VTManager.OnInfoListener onVTInfoNotify = new VTManager.OnInfoListener() {
        public boolean onInfo(VTManager vtm, int what, int extra, Object obj) {
            mHandler.sendMessage(Message.obtain(mHandler, what, extra, VTM_NOTIFY_TYPE_INFO, obj));
            return true;
        }
    };

    private VTManager.OnErrorListener onVTErrorNotify = new VTManager.OnErrorListener() {
        public boolean onError(VTManager vtm, int what, int extra) {
            mHandler.sendMessage(Message.obtain(mHandler, what, extra, VTM_NOTIFY_TYPE_ERROR));
            return true;
        }
    };

    // TODO: wait for the completion of VTManager
    /*
     * private VTManager.OnIncomingDataListener onVTIncomingDataNotify = new
     * VTManager.OnIncomingDataListener(){ public void onIncommingData(){ } };
     *
     * private VTManager.OnStatusUpdateListener onVTStatusUpdateNotify = new
     * VTManager.OnStatusUpdateListener(){ public void onStatusUpdate(){ } };
     */

    private Handler mHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {

            final int what = msg.what;
            final int extra = msg.arg1;
            final int type = msg.arg2;
            final Object obj = msg.obj;

            // FIXME: VTManager won't raise any ERROR notification NOW, so no
            // action here according
            if (type == VTM_NOTIFY_TYPE_ERROR) {
                if (MyLog.DEBUG)
                    MyLog.d(TAG, "HandleMessage ERROR: " + msg.what);
                switch (what) {
                case VTManager.ErrorType.INVALID_STATE:
                    break;
                case VTManager.ErrorType.INVALID_DEVICE:
                    break;
                case VTManager.ErrorType.INVALID_SURFACE:
                    break;
                case VTManager.ErrorType.DEVICE_OPEN_FAIL:
                    break;
                case VTManager.ErrorType.CONNECTION_FAIL:
                    mNotifyHandler.obtainMessage(VTManager.VTMANAGER_ERROR_CONNECTION_FAIL)
                            .sendToTarget();
                    break;
                case VTManager.ErrorType.RECORDING_FAIL:
                    break;
                case VTManager.ErrorType.UNKNOWN:
                    break;

                default:
                    break;
                }
                ;
            }
            // FIXME: VTManager won't raise any info notification except
            // CONNECTED NOW, so no other action here according
            if (type == VTM_NOTIFY_TYPE_INFO) {

                if (MyLog.DEBUG)
                    MyLog.d(TAG, "HandleMessage INFORMATION: " + msg.what);

                switch (what) {
                case VTManager.InfoType.CONNECTED:
                    break;
                case VTManager.InfoType.DISCONNECT_REQ:
                    if (MyLog.DEBUG)
                        MyLog.d(TAG,
                                "receive disconnect req, do disconnection(), current connected?"
                                        + (mVTEState == VTE_STATE.CONNECTED));
                    VTService.this.disconnect();
                    break;
                case VTManager.InfoType.DISCONNECTED:
                    if (MyLog.DEBUG)
                        MyLog.d(TAG, "disconnected!!!");
                    break;
                case VTManager.InfoType.REMOTE_VIDEO_OPENED:
                    break;
                case VTManager.InfoType.REMOTE_VIDEO_CLOSED:
                    break;
                case VTManager.InfoType.RECORD_STARTED:
                    break;
                case VTManager.InfoType.RECORD_STOPPED:
                    break;
                case VTManager.InfoType.USER_INPUT_INCOMMING:
                    // for user input
                    if (MyLog.DEBUG)
                        MyLog.d(TAG, "User input received: " + (String) obj + "mScreenHandler: "
                                + mNotifyHandler);
                    mNotifyHandler.obtainMessage(what, extra, 0, obj).sendToTarget();
                    break;
                case VTManager.InfoType.SIGNAL_WEAK:
                    if (MyLog.DEBUG)
                        MyLog.d(TAG, "Received Signal weak notification.");
                    mNotifyHandler.obtainMessage(what, extra, 0, obj).sendToTarget();
                    break;
                case VTManager.InfoType.CAMERA_FRAME_START:
                    if (MyLog.DEBUG)
                        MyLog.d(TAG, "Received Camera Start Notification.");
                    mNotifyHandler.obtainMessage(what, extra, 0, obj).sendToTarget();
                    break;
                case VTManager.InfoType.UNKNOWN:
                default:
                    break;
                }
            }

            if (type == VTM_NOTIFY_TYPE_CMD_COMPLETE) {
                if (MyLog.DEBUG)
                    MyLog.d(TAG, "HandleMessage cmd complete: " + msg.what);
                switch (what) {
                case VTManager.CmdType.CONNECT:
                    if ((mPendingActionFlags & ACTIONS.CONNECT) != 0) {
                        if (MyLog.DEBUG)
                            MyLog.d(TAG, "connected!!!");
                        mVTEState = VTE_STATE.CONNECTED;
                        if (mListener != null) {
                            mListener.onVTConnected(true);
                        }
                        mVideoCallManager.setIsMediaReadyToReceivePreview(true);
                        // remote flag bit
                        mPendingActionFlags &= ~ACTIONS.CONNECT;
                    } else { // else ignore the fake notification
                        if (MyLog.DEBUG)
                            MyLog.v(TAG, "Fake connect infomation notification.");
                    }
                    break;
                case VTManager.CmdType.DISCONNECT:
                    if (MyLog.DEBUG)
                        MyLog.d(TAG, "VTManager.CmdType.DISCONNECT!!!");

                    break;
                default:
                    break;
                }
            }

            if (type == VTM_NOTIFY_REOPEN_CAMERA){
                    MyLog.d(TAG, "handle VTM_NOTIFY_REOPEN_CAMERA");
                    int vsource = extra;
                    initializeCamera(vsource, mCameraSetting);
            }
        }// end of handleMessage()
    };

    @Override
    public void onCreate() {

        super.onCreate();
        if (MyLog.DEBUG) {
            MyLog.d(TAG, "===service onCreate");
        }
        mFails = MAX_CAMERA_REOPEN_TIMES;
        mAudioManager = (AudioManager) this.getSystemService(Context.AUDIO_SERVICE);

        if (mVTEngine == null) {
            if (MyLog.DEBUG)
                MyLog.d(TAG,
                        "VT engine hasn't been create, this should be restored by system, so just stopSelf !!!!!!");
            stopSelf();
            /*
             * BALDEV final NotificationManager nmg =
             * (NotificationManager)this.getSystemService
             * (Context.NOTIFICATION_SERVICE); try{ if (MyLog.DEBUG)
             * MyLog.d(TAG, "try to clear notification indicator."); nmg.cancel(
             * VTNotificationManager.VT_IN_CALL_NOTIFICATION); }catch( Exception
             * e){ if (MyLog.DEBUG) MyLog.d(TAG,
             * "catch a exception while clear notification indicator"); }
             */
            Log.d(TAG, "before mNmgCallback=" + mNmgCallback);
            if (mNmgCallback != null) {
                mNmgCallback.cancelNotification(this);
            }
            // try to destroy application
            // VideoCallApp.getInstance().tryEndApplication();

            return;
        }

        mVTEState = VTE_STATE.DISCONNECTED;
        // TITANK:TODO
        mVTEngine.setOnErrorListener(onVTErrorNotify);
        mVTEngine.setOnInfoListener(onVTInfoNotify);
        mVTEngine.setOnCmdCompListener(onVTCmdComplNotify);
        // setOnIncomingDataListener(OnIncomingDataListener l)
        // setOnStatusUpdateListener(OnStatusUpdateListener l)
        if (MyLog.DEBUG)
            MyLog.d(TAG, " VTManager service create");
        /*
         * BALDEV mVTnm = VTNotificationManager.getDefault();
         *
         * if (mVTnm == null) { VTNotificationManager.init(this); mVTnm =
         * VTNotificationManager.getDefault(); } mVTnm.notifyInVTCall(this);
         */
        if (mNmgCallback != null) {
            mNmgCallback.notifyInVtCall(this);
        }
        mVideoCallManager = VideoCallManager.getInstance(mContext);
        mBackCameraId = mVideoCallManager.getBackCameraId();
        mFrontCameraId = mVideoCallManager.getFrontCameraId();
        Log.e(TAG,"Number of Camera = " + android.hardware.Camera.getNumberOfCameras());
        if (2 == android.hardware.Camera.getNumberOfCameras()){
            mCameraId = mFrontCameraId;
            if (SystemProperties.get("ro.board.platform","0").equals("msm8909")){
                RemoteServiceConnector.setProperty("persist.radio.rotate","2");
            } else {
                RemoteServiceConnector.setProperty("persist.radio.rotate","1");
            }
            if (MyLog.DEBUG)
                MyLog.d(TAG, "two cameras, the default camera is set to be the frontCamera");
        }
        else if (1 == android.hardware.Camera.getNumberOfCameras()){
            mCameraId = mBackCameraId;
            RemoteServiceConnector.setProperty("persist.radio.rotate","1");
            if (MyLog.DEBUG)
                MyLog.d(TAG, "only one camera, the default camera is set to be the backCamera");
        }
        else{
            mCameraId = mFrontCameraId;
            RemoteServiceConnector.setProperty("persist.radio.rotate","1");
        }
    }

    @Override
    public void onStart(Intent intent, int startId) {
        super.onStart(intent, startId);
        if (MyLog.DEBUG)
            MyLog.d(TAG, "===service start");
    }

    @Override
    public void onDestroy() {
        super.onDestroy();

        if (MyLog.DEBUG)
            MyLog.v(TAG, "=service destroy");

        Log.e(TAG, "service destroy: state=: " + mVTEState);

        // Notification
        if (MyLog.DEBUG)
            MyLog.d(TAG, "Close In Call Notification");
        /*
         * BALDEV if( mVTnm != null) mVTnm.closeInCallNotification();
         */
        if (mNmgCallback != null) {
            mNmgCallback.closeInCallNotification();
        }
        // closeNotification();
        if (mVideoCallManager != null) {
            mVideoCallManager.setIsMediaReadyToReceivePreview(false);
            stopPreview();
            closeCamera();
        }

        if (mVTEngine != null) {
            disconnect();
            mVTEngine.release();
            mVTEngine = null;
            if (mListener != null) {
                mListener.onVTReleased(true);
            }
        }

        if (mVideoCallManager != null) {
            mVideoCallManager.setVTManager(mVTEngine);
            mVideoCallManager = null;
        }

        if (mAudioManager != null) {
            if (MyLog.DEBUG)
                MyLog.d(TAG, "set audio parameter: videotelephony=off");
            mAudioManager.setParameters("videotelephony=off");
        }

        stopForeground(true);
        mFirstTime = true;
    }

    public VTService() {
        if (MyLog.DEBUG)
            MyLog.v(TAG, "VT Service Constructor.");
    }

    public static boolean startService(Context context) {
        if (MyLog.DEBUG)
            MyLog.v(TAG, "=========startService========== ");

        if (mVTEngine != null) {
            return true;
        }
        if (MyLog.DEBUG)
            MyLog.v(TAG, "===Do startService===");
        mContext = context;

        // TODO: test whether it's valid.
        try {
            mVTEngine = new VTManager();
            mVideoCallManager.setVTManager(mVTEngine);

        } catch (VTDisabledException e) {
            Log.e(TAG, "VT is disabled, " + e.getMessage());
            mVTEngine = null;
            return false;
        }

        // start the service
        context.startService(new Intent(context, VTService.class));

        return true;
    }

    public static void stopService(Context context) {
        if (mVTEngine == null) {
            return;
        }
        context.stopService(new Intent(context, VTService.class));
        return;
    }

    @Override
    public IBinder onBind(Intent intent) {
        if (MyLog.DEBUG)
            MyLog.v(TAG, "==service on bind every time ");
        return mBinder;
    }

    public void setVTServiceListener(VTServiceListener l) {
        mListener = l;
    }

    public void captureScreen(int where, String strDstPath) {
        if (MyLog.DEBUG)
            MyLog.v(TAG, "=========captureScreen========== ");
        Bitmap bm;

        // FIXME: turn on below codes when SDK support
        bm = captureScreen(where);

        if (MyLog.DEBUG)
            MyLog.d(TAG, "data of capture: " + bm);

        VTServiceCallUtils.storeImageToFile(mContext, bm, strDstPath);
    }

    private Bitmap captureScreen(int where) {

        // FIXME: Wait for the VTManager implementing a uniform programming mode
        // mPendingActionFlags |= ACTIONS.CAPTURE_FRAMES
        Bitmap ret = null;

        try {
            ret = mVTEngine.captureRemoteFrame();
        } catch (Exception e) {
            Log.e(TAG, "capture remote frame failure from VTManager");
        }

        return ret;
    }

    public void setLocalDisplay(SurfaceTexture surface) {

        // FIXME: Wait for the VTManager implementing a uniform programming mode
        // mPendingActionFlags |= ACTIONS.SET_LOCAL_SURFACE;

        if (MyLog.DEBUG)
            MyLog.d(TAG, "Set Local Display Surface.");
        mCameraSurface = surface;
        mVideoCallManager.setIsMediaReadyToReceivePreview(false);
        // if (mVTEngine != null && surface != null ) {
        // mCameraSurface = mVTEngine.setLocalDisplay(surface);
        // }

        if (mCameraSurface == null) {
            stopPreview();
            closeCamera();
        } else {
            if (mVideoCallManager.getCameraState() == CameraState.CAMERA_CLOSED) {
                initializeCamera(-1, null);
            } else {
                // Set preview display if the surface is being created and
                // preview
                // was already started. That means preview display was set to
                // null
                // and we need to set it now.
                mVideoCallManager.setDisplay(mCameraSurface);
            }
        }

    }

    /**
     * This method opens the camera and starts the camera preview
     */
    private void initializeCamera(int vsource, CameraSetting cameraSetting) {
        if (MyLog.DEBUG)
            MyLog.d(TAG, "Initializing camera");
        mCameraSetting = cameraSetting;
        // Open camera if not already open
        if (false == openCamera(mCameraId)) {
            MyLog.d(TAG, "Initializing camera opencamera return false mFails=" + mFails);
            if (--mFails >= 0) {
                mHandler.sendMessageDelayed(
                        Message.obtain(mHandler, 0, vsource, VTM_NOTIFY_REOPEN_CAMERA, null), CAMERA_REOPREN_DELAY_TIME);
            }
            return;
        }
        mFails = MAX_CAMERA_REOPEN_TIMES;

        if (vsource == VTManager.VideoSource.CAMERA_SECONDARY) {
            if (SystemProperties.get("ro.board.platform","0").equals("msm8909")){
                RemoteServiceConnector.setProperty("persist.radio.rotate","2");
            } else {
                RemoteServiceConnector.setProperty("persist.radio.rotate","1");
            }
            if (mCameraSetting != null) {
                mCameraSetting.updateCameraParams(this, CameraSetting.CONSTRAST_SETTING,
                        mCameraSetting.getCurrentValueIndex(CameraSetting.CONSTRAST_SETTING));
                mCameraSetting.updateCameraParams(this, CameraSetting.BRIGHT_SETTING,
                        mCameraSetting.getCurrentValueIndex(CameraSetting.BRIGHT_SETTING));
            }
        } else if (vsource == VTManager.VideoSource.CAMERA_MAIN) {
            RemoteServiceConnector.setProperty("persist.radio.rotate","1");
            if (mCameraSetting != null) {
                mCameraSetting.updateCameraParams(this, CameraSetting.CONSTRAST_SETTING,
                        mCameraSetting.getCurrentValueIndex(CameraSetting.CONSTRAST_SETTING));
                mCameraSetting.updateCameraParams(this, CameraSetting.BRIGHT_SETTING,
                        mCameraSetting.getCurrentValueIndex(CameraSetting.BRIGHT_SETTING));
            }
        }
        // initializeZoom();
        initializeCameraParams();
        // Start camera preview
        startPreview();

    }

    /**
     * This method crates the camera object if camera is not disabled
     *
     * @param cameraId
     *            ID of the front or the back camera
     * @return Camera instance on success, null otherwise
     */
    private boolean openCamera(int cameraId) {
        boolean result = false;

        try {
            return mVideoCallManager.openCamera(cameraId);
        } catch (Exception e) {
            Log.e(TAG, "Failed to open camera device, error " + e.toString());
            return result;
        }
    }

    /**
     * Initialize camera parameters based on negotiated height, width TODO: FPS
     * support
     */
    private void initializeCameraParams() {
        mParameters = mVideoCallManager.getCameraParameters();
        try {
            // Set the camera preview size
            /*
             * if (mIsMediaLoopback) { // In loopback mode the IMS is hard coded
             * to render the // camera frames of only the size 176x144 on the
             * far end surface mParameters.setPreviewSize(176, 144); } else
             */{
                if (MyLog.DEBUG)
                    MyLog.d(TAG,
                            "Supported Preview Sizes = " + mParameters.getSupportedPreviewSizes());
                if (MyLog.DEBUG)
                    MyLog.d(TAG, "Set Preview Size directly with negotiated Height = "
                            + mVideoCallManager.getNegotiatedHeight() + " negotiated width= "
                            + mVideoCallManager.getNegotiatedWidth());
                //mParameters.set("vt-crop", "enable");
                //mParameters.set("video-rotation", "90");
                mParameters.setPreviewSize(/*
                                            * mVideoCallManager.getNegotiatedWidth
                                            * ()
                                            */144,
                /* mVideoCallManager.getNegotiatedHeight() */176);
                mParameters.setPreviewFrameRate(30);
                mParameters.set("preview-format","nv12");
                mParameters.setCameraMode((1 << 30));
            }

            mVideoCallManager.setCameraParameters(mParameters);
        } catch (RuntimeException e) {
            Log.e(TAG, "Error setting Camera preview size exception=" + e);
            Log.e(TAG, "Supported Preview sizes = " + mParameters.getSupportedPreviewSizes());
        }
    }

    /**
     * This method starts the camera preview
     */
    private void startPreview() {
        try {
            // mCameraPreview.setVisibility(View.VISIBLE);
            mVideoCallManager.startCameraPreview(mCameraSurface);
        } catch (IOException ioe) {
            closeCamera();
            Log.e(TAG, "Exception while setting preview texture, " + ioe.toString());
        }
    }

    /**
     * This method disconnect and releases the camera
     */
    private void closeCamera() {
        mVideoCallManager.closeCamera();
    }

    /*
     * This method stops the camera preview
     */
    private void stopPreview() {
        // mCameraPreview.setVisibility(View.GONE);
        mVideoCallManager.stopCameraPreview();
    }

    public void setRemoteDisplay(SurfaceTexture surface) {
        if (MyLog.DEBUG)
            MyLog.d(TAG, "Set Remote Surface.");
        // FIXME: Wait for the VTManager implementing a uniform programming mode
        // mPendingActionFlags |= ACTIONS.SET_REMOTE_SURFACE;
        // if (mVTEngine != null) {
        // mVTEngine.getRemoteDisplay(surface);
        // mVideoCallManager.setFarEndSurface(mVTEngine.getRemoteDisplay(surface));
        // }
        mVideoCallManager.setFarEndSurface(surface);
    }

    public void setVTDevice(String dev) {
        if (MyLog.DEBUG)
            MyLog.v(TAG, "===engine setVTDevice" + dev);

        // FIXME: Wait for the VTManager implementing a uniform programming mode
        // mPendingActionFlags |= ACTIONS.SET_DEVICE;

        mVTEngine.setVTDevice(dev);
    }

    public void connect() {
        if (MyLog.DEBUG)
            MyLog.d(TAG, "===engine connect");
        // this is an asynchrony action
        if (mVTEState == VTE_STATE.DISCONNECTED && ((mPendingActionFlags & ACTIONS.CONNECT) == 0)) {

            // b444: Call mAudioManager.setParameters() before VT engine is
            // connected.
            if (mAudioManager != null) {
                if (MyLog.DEBUG)
                    MyLog.d(TAG, "set audio parameter: videotelephony=on");
                mAudioManager.setParameters("videotelephony=on");
            }

            if (MyLog.DEBUG)
                MyLog.d(TAG, "Init");
            // mVTEngine.init();
            // Initialize DPL
            mVideoCallManager.mediaInit();
            if (MyLog.DEBUG)
                MyLog.d(TAG, "connect");
            // Add pending flag
            mPendingActionFlags |= ACTIONS.CONNECT;
            // mVTEngine.connect();
        }
    }

    public void disconnect() {
        if (MyLog.DEBUG)
            MyLog.v(TAG, "===engine disconnect");
        // FIXME: Wait for the VTManager implementing a uniform programming mode
        mPendingActionFlags |= ACTIONS.DISCONNECT;
        if (mVTEState == VTE_STATE.CONNECTED || ((mPendingActionFlags & ACTIONS.CONNECT) != 0)) {
            if (MyLog.DEBUG)
                MyLog.v(TAG, "disconnect");
            stopPreview();
            closeCamera();
            mVideoCallManager.mediaDeInit();
            // mVTEngine.disconnect();
            mVTEState = VTE_STATE.DISCONNECTED;
            // clear any pending flags
            mPendingActionFlags &= ~ACTIONS.CONNECT;

            // b444: Call mAudioManager.setParameters() after VT engine is
            // disconnected.
            if (mAudioManager != null) {
                if (MyLog.DEBUG)
                    MyLog.d(TAG, "set audio parameter: videotelephony=off");
                mAudioManager.setParameters("videotelephony=off");
            }
        }
        mPendingActionFlags &= ~ACTIONS.DISCONNECT;
    }

    public boolean isConnected() {
        return (mVTEState == VTE_STATE.CONNECTED);
    }

    public boolean setMut(boolean muted) {

        boolean bRet = true;

        if (MyLog.DEBUG)
            MyLog.v(TAG, "setMute=" + muted);
        try {
            // FIXME: turn on below codes when SDK updated
            mVTEngine.setMut(muted);
        } catch (Exception e) {
            Log.e(TAG, "set mut failed");
            bRet = false;
        }

        return bRet;
    }

    /* modified by liulin */
    public void sendUserInput(String input) throws IllegalStateException {
        if (MyLog.DEBUG)
            MyLog.d(TAG, "sendUserInput: " + input);
        if (mVTEState == VTE_STATE.CONNECTED && (mPendingActionFlags & ACTIONS.DISCONNECT) == 0) {
            mVTEngine.sendUserInput(input);
            if (input.equals("close_:camera_")) {
                mLastFrame = true;
            } else if (input.equals("open_:camera_")) {
                mLastFrame = false;
            }
        } else {
            if (MyLog.DEBUG)
                MyLog.d(TAG, "VT engine begin or has disconnect,cancel sendUserInput " + input);
        }
    }

    public void setVideoFrame(int vsource, byte[] frames) {
        if (MyLog.DEBUG)
            MyLog.d(TAG, "setVideoFrame, vsource: " + vsource + "frames " + frames);
        if (frames != null) {
            mVideoCallManager.setIsMediaReadyToReceivePreview(false);
            if (vsource != mLastvsource && !mLastFrame) {
                mVTEngine.setCameraSource(vsource);
                mLastvsource = vsource;
                MyLog.d(TAG, "Setting rotation OFF ");
            }
            mVideoCallManager.sendVideoFrames(frames);
        } else {
            mVideoCallManager.setIsMediaReadyToReceivePreview(true);
            mVTEngine.setCameraSource(vsource);
            mLastvsource = vsource;
            mLastFrame = false;
        }
    }

    public void setVideoSource(int vsource, String path, CameraSetting mCameraSetting) {
        if (MyLog.DEBUG)
            MyLog.d(TAG, "setVideoSource, vsource: " + vsource + " path: " + path);

        if (mFirstTime /*
                        * && mVideoCallManager.getCameraState() ==
                        * CameraState.CAMERA_CLOSED
                        */) {
            MyLog.d(TAG, "iFirst time setVideosource ");
            mFirstTime = false;
            if (path == null) {
                path = "/data/data/com.borqs.videocall/files/VTImg.raw";
            }
            mVTEngine.setVideoSource(vsource, path);
            return;
        }

        switch (vsource) {
        case VTManager.VideoSource.CAMERA_NONE:
            switchCamera(-1, null, VTManager.VideoSource.CAMERA_NONE);
            // mVTEngine.setVideoSource(vsource, path);
            if (!mLastFrame) {
                mVTEngine.setCameraSource(vsource);
            } else {
                mVTEngine.setCameraSource(VTManager.VideoSource.CAMERA_STILL);
            }
            mLastvsource = vsource;
            break;
        case VTManager.VideoSource.CAMERA_SECONDARY:
            mVideoCallManager.setIsMediaReadyToReceivePreview(true);
            switchCamera(mFrontCameraId, mCameraSetting, VTManager.VideoSource.CAMERA_SECONDARY);
            mVTEngine.setCameraSource(vsource);
            mLastvsource = vsource;
            break;
        case VTManager.VideoSource.CAMERA_MAIN:
            mVideoCallManager.setIsMediaReadyToReceivePreview(true);
            switchCamera(mBackCameraId, mCameraSetting, VTManager.VideoSource.CAMERA_MAIN);
            mVTEngine.setCameraSource(vsource);
            mLastvsource = vsource;
            break;

        }

    }

    /**
     * This method is used to fix the bug crash because of adjusting the camera
     * contrast in VT call. Adjusting the camera contrast need
     * startPreviewInternal and restart preview, and in VT call that will cause
     * crash. This method stops/starts updating the preview surface without
     * close the camera device, so the switch will be quick and the parameter
     * adjusted can be saved.
     */
    public void pauseCameraPreview(boolean off, int vsource) {
        int cameraId = mBackCameraId;
        if (vsource == VTManager.VideoSource.CAMERA_MAIN) {
            cameraId = mBackCameraId;
        } else if (vsource == VTManager.VideoSource.CAMERA_SECONDARY) {
            cameraId = mFrontCameraId;
        } else {
            return;
        }
        if (off) {
            stopPreview();
            if (!mLastFrame) {
                mVTEngine.setCameraSource(VTManager.VideoSource.CAMERA_NONE);
            } else {
                mVTEngine.setCameraSource(VTManager.VideoSource.CAMERA_STILL);
            }
            mLastvsource = VTManager.VideoSource.CAMERA_NONE;
        } else {
            mVideoCallManager.setIsMediaReadyToReceivePreview(true);
            openCamera(cameraId);
            startPreview();
            mVTEngine.setCameraSource(vsource);
            mLastvsource = vsource;
        }
    }

    /**
     * This method switches the camera to front/back or off
     *
     * @param cameraId
     */
    private void switchCamera(int cameraId, CameraSetting mCameraSetting, int vsource) {
        // Change the camera Id
        mCameraId = cameraId;

        // Stop camera preview if already running
        if (mVideoCallManager.getCameraState() != CameraState.CAMERA_CLOSED) {
            if (cameraId != -1) {
                stopPreview();
                closeCamera();
            } else {
                stopPreview();
            }
        }

        // Restart camera if camera doesn't need to stay off
        if (cameraId != -1) {
            initializeCamera(vsource, mCameraSetting);
        }
        //only for 144x176
        else {
            RemoteServiceConnector.setProperty("persist.radio.rotate","3");
        }
    }

    public void setCameraParameter(String key, String value) {
        if (MyLog.DEBUG)
            MyLog.d(TAG, "setCameraParameter, key: " + key + " value: " + value);
        try {
            Parameters parameters = mVideoCallManager.getCameraParameters();
            parameters.set(key, value);
            Log.d(TAG, "setCameraParameter parameters=" + parameters.flatten());
            mVideoCallManager.setCameraParameters(parameters);
        } catch (Exception e) {
            Log.i(TAG, "Exception:" + e);
        }
        // mVTEngine.setCameraParameter(key, value);
    }

    /* modified end */

    // /**
    // * Show a notification while this service is running.
    // */
    // private void showNotification() {
    //
    // mNM = (NotificationManager) getSystemService(NOTIFICATION_SERVICE);
    //
    // // In this sample, we'll use the same text for the ticker and the
    // // expanded notification
    // CharSequence text = "local service start";
    //
    // // Set the icon, scrolling text and timestamp
    // Notification notification = new Notification(
    // R.drawable.vt_listen_status, text, System.currentTimeMillis());
    //
    // // The PendingIntent to launch our activity if the user selects this
    // // notification
    // Intent intent = VideoCallApp.getInstance().constuctRestoreIntent();
    //
    // PendingIntent contentIntent = PendingIntent.getActivity(this, 0,
    // intent, 0);
    //
    // // Set the info for the views that show in the notification panel.
    // notification.setLatestEventInfo(this,
    // getString(R.string.vt_service_title), text, contentIntent);
    //
    // // Send the notification.
    // // We use a layout id because it is a unique number. We use it later to
    // // cancel.
    // mNM.notify(VT_SERVICE_ID, notification);
    // }
    //
    // private void closeNotification() {
    // if( mNM != null){
    // if (MyLog.DEBUG) MyLog.d(TAG, "close Notification.");
    // mNM.cancel(VT_SERVICE_ID);
    // }
    // }

    public class LocalBinder extends Binder {
        VTService getService() {
            return VTService.this;
        }
    }

    // Use Local binder
    private final IBinder mBinder = new LocalBinder();

    private Handler mNotifyHandler;

    void setHandler(Handler h) {
        mNotifyHandler = h;
    }

};
