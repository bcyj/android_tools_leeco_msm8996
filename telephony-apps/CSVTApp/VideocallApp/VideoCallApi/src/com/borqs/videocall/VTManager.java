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

import android.content.ContentResolver;
import android.content.Context;
import android.content.res.AssetFileDescriptor;
import android.net.Uri;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.os.ParcelFileDescriptor;
import android.os.PowerManager;
import android.util.Log;
import android.view.Surface;
import android.view.TextureView;
import android.view.SurfaceHolder;
import android.graphics.Bitmap;
import android.media.AudioManager;
import android.database.Cursor;
import android.provider.MediaStore;
import android.graphics.SurfaceTexture;

import java.io.FileDescriptor;
import java.io.IOException;

import java.lang.ref.WeakReference;

/**
 * The VTManager class is used for operating video telephony engine video
 * telephony is a 3G specific feature, the APIs only runable for 3G compatilbe
 * terminals.
 *
 */
public class VTManager {
    private final static String TAG = "VTManager";
    private static final int USER_INPUT_LENGTH = 100;
    public final static int VTMANAGER_ERROR_BASE = 3000;
    public static final int VTMANAGER_ERROR_UNKNOWN = VTMANAGER_ERROR_BASE;
    public static final int VTMANAGER_ERROR_INVALID_STATE = VTMANAGER_ERROR_BASE + 1;
    public static final int VTMANAGER_ERROR_INVALID_DEVICE = VTMANAGER_ERROR_BASE + 2;
    public static final int VTMANAGER_ERROR_INVALID_SURFACE = VTMANAGER_ERROR_BASE + 3;
    public static final int VTMANAGER_ERROR_DEVICE_OPEN_FAIL = VTMANAGER_ERROR_BASE + 4;
    public static final int VTMANAGER_ERROR_CONNECTION_FAIL = VTMANAGER_ERROR_BASE + 5;
    public static final int VTMANAGER_ERROR_RECORDING_FAIL = VTMANAGER_ERROR_BASE + 6;

    /**
     * Defines Local config const
     */

    /**
     * Defines the video source. These constants are used with
     * {@link VTManager#setVideoSource(int)}.
     */

    public final class VideoSource {
        /*
         * Do not change these values without updating their counterparts
         */
        private VideoSource() {
        }

        public static final int DEFAULT = 0;
        /** Camera video source */
        public static final int CAMERA_MAIN = 1;
        public static final int CAMERA_SECONDARY = 2;
        public static final int CAMERA_NONE = 3;
        public static final int CAMERA_FILE = 4;
        public static final int CAMERA_STILL = 5;
        public static final int UNKNOWN = 6;
        /*
         * public static final int LANDSCAPE = 6; public static final int
         * PORTRAIT = 7;
         */

    }

    /**
     * Defines incomming data type.
     */
    public final class IncommingDataType {
        public static final int DEFAULT = 0;
        public static final int PICTURE = 1;
        public static final int MESSAGE = 2;
        public static final int RAWDATA = 3;
        public static final int USERINPUT = 4;
    }

    /**
     * Defines video telephony msg type.
     */
    public final class EventType {
        public static final int EVENT_UNKNOWN = 0;
        public static final int EVENT_INFO = 1;
        public static final int EVENT_ERROR = 2;
        public static final int EVENT_CMDCOMP = 3;
    }

    /**
     * Defines video telephony error type. App must handle this error message.
     */
    public final class ErrorType {
        public static final int UNKNOWN = 0;
        public static final int INVALID_STATE = 1;
        public static final int INVALID_DEVICE = 2;
        public static final int INVALID_SURFACE = 3;
        public static final int DEVICE_OPEN_FAIL = 4;
        public static final int CONNECTION_FAIL = 5;
        public static final int RECORDING_FAIL = 6;

    }

    /**
     * Defines video telephony information message type. App can ignore this.
     */
    public final class InfoType {
        public static final int UNKNOWN = 0;
        public static final int CONNECTED = 1;
        public static final int DISCONNECTED = 2;
        public static final int REMOTE_VIDEO_OPENED = 3;
        public static final int REMOTE_VIDEO_CLOSED = 4;
        public static final int RECORD_STARTED = 5;
        public static final int RECORD_STOPPED = 6;
        public static final int USER_INPUT_INCOMMING = 7;
        public static final int DISCONNECT_REQ = 8;
        public static final int SIGNAL_WEAK = 9;
        public static final int CAMERA_FRAME_START = 10;
    }

    /**
     * Defines video telephony information message type. App can ignore this.
     */
    public final class CmdType {
        public static final int SET_LOCAL_DISPLAY = 0;
        public static final int SET_REMOTE_DISPLAY = 1;
        public static final int SET_LOCAL_SURFACE = 2;
        public static final int SET_REMOTE_SURFACE = 3;
        public static final int SET_VTDEVICE = 4;
        public static final int CONNECT = 5;
        public static final int DISCONNECT = 6;
        public static final int RELEASE = 7;
        public static final int SET_VOLUME = 8;
        public static final int START_RECORD = 9;
        public static final int STOP_RECORD = 10;
        public static final int SET_VIDEO_SOURCE = 11;
        public static final int CAPTURE_FRAME = 12;
        public static final int GET_INCOMING_DATA = 13;
        public static final int REFUSE_INCOMING_DATA = 14;

    }

    private int mNativeContext; // accessed by native methods
    private int mListenerContext; // accessed by native methods
    private SurfaceTexture mLocalSurface; // accessed by native methods
    // /private SurfaceTexture mRemoteSurface; // accessed by native methods
    private EventHandler mEventHandler;
    // b413 for android 4.0
    private int mNativeLocalSurfaceTexture; // accessed by native methods
    private int mNativeRemoteSurfaceTexture; // accessed by native methods
    private SurfaceTexture mIRemoteSurface; // accessed by native methods

    // /private SurfaceTexture mtexture;

    /**
     * Default constructor. When done with the VTManager, you should call
     * {@link #release()}, to free the resources. If not released, next
     * MediaPlayer instances may result in an exception.
     */

    public VTManager() throws VTDisabledException {
        try {
            System.loadLibrary("csvt_jni");
            System.loadLibrary("camerahandler_jni");
        } catch (UnsatisfiedLinkError ule) {
            Log.e(TAG, "WARNING: Could not load vt_jni natives "+ule);
            throw new VTDisabledException();
        }

        mOnInfoListener = null;
        mOnErrorListener = null;
        mLocalSurface = null;
        // /mRemoteSurface=null;
        mIRemoteSurface = null;
        // /mtexture=null;

        Looper looper;
        if ((looper = Looper.myLooper()) != null) {
            mEventHandler = new EventHandler(this, looper);
        } else if ((looper = Looper.getMainLooper()) != null) {
            mEventHandler = new EventHandler(this, looper);
        } else {
            mEventHandler = null;
        }

        // weak reference
        native_setup(new WeakReference<VTManager>(this));
    }

    /**
     * Sets the Surface to use for displaying the video portion of remote. call
     * this before connect() . Set a null as surface will result in no display
     *
     * @param surface
     *            the Surface to use for video display
     *
     */
    public void setLocalDisplay(SurfaceTexture surface) {
        mLocalSurface = surface;
        setVTLocalSurface();
        if (MyLog.DEBUG)
            MyLog.v(TAG, "setLocalDisplay Succeed. ");
    }

    /**
     * Sets the Surface to use for displaying the video portion of local. call
     * this before connect() or connectAsync(). Set a null as surface will
     * result in no display
     *
     * @param surface
     *            the Surface to use for video display
     *
     */

    public void setRemoteDisplay(SurfaceTexture surface) {
        mIRemoteSurface = surface;
        setVTRemoteSurface();
        if (MyLog.DEBUG)
            MyLog.v(TAG, "setRemoteDisplay Succeed.");
    }

    private native void setVTLocalSurface();

    private native void setVTRemoteSurface();

    public native void init();

    /**
     * Sets the telephony device. Call this before any other method (including
     * setVTManagerDevice()) that might throw IllegalStateException in this
     * class.
     *
     * @param context
     *            the Context to use
     * @param teledevice
     *            the telephony device
     * @throws IllegalStateException
     *             if it is called in an order other than the one specified
     *             above
     */
    public native void setVTDevice(String teledevice) throws IllegalStateException;

    /**
     * connect remote side, asynchronously. Call this after
     * setVideoTelephonyDevice() or disconnect(), and before any other method
     * that might throw IllegalStateException in this class.
     *
     * After setting the telephony device,auido source ,video source and display
     * surface, you need to call connect(). connect() will blocks until video
     * telephony connection is ready. if local surface is set,view finder will
     * be started.
     *
     * @throws IllegalStateException
     *             if it is called in an order other than the one specified
     *             above
     */
    public void connect() {
        _connect();
    }

    private native void _connect() throws IllegalStateException;

    /**
     * disconnect thelephony call.
     *
     * @throws IllegalStateException
     *             if it is called in an order other than the one specified
     *             above
     */

    public native void disconnect() throws IllegalStateException;

    /**
     * Releases resources associated with this VTManager object. It is
     * considered good practice to call this method when you're done using the
     * VTManager.
     */
    public native void release();

    /**
     * Sets the volume of remote voice. This API is recommended for balancing
     * the output of remote audio streams within an application.
     *
     * @param Volume
     *            volume scalar (0-100)
     */
    public native void setVolume(float Volume);

    /**
     * Mut the Local voice. This API is recommended for mut the input audio
     * within an application. calling this method will cause the far end can not
     * hear your voice
     *
     * @param Mut
     *            true for mut and false for unmut
     */
    public native void setMut(boolean Mut);

    /**
     * Begins capturing remote video/audio data to the file specified with
     * setRecordFile(). Call this after start().
     *
     * @param path
     *            recording file path
     * @throws IllegalStateException
     *             if it is called before start().
     */
    public void startRecord(String path) throws IOException, IllegalArgumentException,
            IllegalStateException {
        // todo
        return;
    }

    /**
     * Stops recording. Call this after startRecord().
     *
     * @throws IllegalStateException
     *             if it is called before start()
     */

    public void stopRecord() throws IllegalStateException {
        return;
    }

    /**
     * Sets the video source to be used for VT. If this method is not called,
     * default video source is used. this method should be called after
     * setLocalDisplay()
     *
     * @see VideoSource
     * @throws IllegalStateException
     *             if it is called after start
     */

    public native void setVideoSource(int vsource, String path);

    /**
     * Sets the camera parameter for VT.
     *
     * @param key
     *            key name for the parameter
     *
     * @param value
     *            the string value of the parameter
     *
     * @throws IllegalStateException
     *             if it is called after start
     */

    public native void setCameraParameter(String key, String value);

    /**
     * Returns a bitmap of remote side video
     *
     * @return Bitmap for specified side,or null it no video captured
     * @throws IllegalStateException
     *             if it is called before start
     */

    public native Bitmap captureRemoteFrame() throws IllegalStateException;

    /**
     * Returns a bitmap of local side video
     *
     * @return Bitmap for specified side,or null it no video captured
     * @throws IllegalStateException
     *             if it is called before start
     */

    public native Bitmap captureLocalFrame() throws IllegalStateException;

    /**
     * Returns void
     *
     * @param cameraId
     *            which camera is in use (front or back)
     * @throws IllegalStateException
     *             if it is called before start
     */
    public native void setCameraSource(int cameraId) throws IllegalStateException;

    /**
     * Returns a object if there is a incomming data from remote.
     *
     * @param data_id
     *            data_id from onIncomingData listener
     * @return object from remote,null if there is no incomming data with
     *         data_id. you should according type convert it to picture,message
     *         or others.
     * @throws IllegalStateException
     *             if it is called before start
     */

    public Object getIncommingData(int data_id) throws IllegalStateException {
        return null;
    }

    /**
     * Refuse a incomming data.
     *
     * @param data_id
     *            data_id from onIncomingData listener
     * @throws IllegalStateException
     *             if it is called before start
     */
    public void refuseIncommingData(int data_id) throws IllegalStateException {
        return;
    }

    /**
     * Send user input keyboard to the peer terminal.
     *
     * @param user_input
     *            user input string, which will be wrapped by H.245
     *            UserInputIndication message and be sent to the peer. Notes:
     *            the length of user_input should not more than 100. otherwise,
     *            the strings that after 100 will be dicarded.
     * @throws IllegalStateException
     *             if it is called before start
     */
    public void sendUserInput(String user_input) throws IllegalStateException {
        // if UserInput is NULL or length is equal 0, return directly.
        if (user_input == null || "".equals(user_input.trim())) {
            return;
        }
        // Insure the UserInput max length is not more than 100.
        if (user_input.length() > USER_INPUT_LENGTH - 1) {
            String subStr = user_input.substring(0, USER_INPUT_LENGTH - 1);
            _sendUserInput(subStr);
        } else {
            _sendUserInput(user_input);
        }
        return;
    }

    private native void _sendUserInput(String user_input);

    /**
     * Interface definition for a callback to be invoked when vt engine report
     * an error.
     */
    public interface OnErrorListener {
        boolean onError(VTManager vtm, int what, int extra);
    }

    private OnErrorListener mOnErrorListener;

    /**
     * Register OnErrorListener callback
     *
     * @param l
     *            the callback that will be run
     */
    public void setOnErrorListener(OnErrorListener l) {
        mOnErrorListener = l;

    }

    /**
     * Interface definition for a callback to be invoked when vt engine report
     * an info event.
     */
    public interface OnInfoListener {
        boolean onInfo(VTManager vtm, int what, int extra, Object obj);
    }

    private OnInfoListener mOnInfoListener;

    /**
     * Register OnInfoListener callback
     *
     * @param l
     *            the callback that will be run
     */
    public void setOnInfoListener(OnInfoListener l) {
        mOnInfoListener = l;

    }

    /**
     * Interface definition for a callback to be invoked when vt engine command
     * completed.
     */
    public interface OnCmdCompListener {
        boolean onCmdComp(VTManager vtm, int what, int extra);
    }

    private OnCmdCompListener mOnCmdCompListener;

    /**
     * Register OnCmdCompListener callback
     *
     * @param l
     *            the callback that will be run
     */
    public void setOnCmdCompListener(OnCmdCompListener l) {
        mOnCmdCompListener = l;

    }

    /**
     * Called from native code when an interesting event happens. This method
     * just uses the EventHandler system to post the event back to the main app
     * thread. We use a weak reference to the original VTManager object so that
     * the native code is safe from the Java object disappearing from underneath
     * it. (This is the cookie passed to native_setup().)
     */
    private static void postEventFromNative(Object vtmanager_ref, int what, int arg1, int arg2,
            Object obj) {
        VTManager vtm = (VTManager) ((WeakReference) vtmanager_ref).get();
        if (vtm == null) {
            return;
        }

        if (vtm.mEventHandler != null) {
            Message m = vtm.mEventHandler.obtainMessage(what, arg1, arg2, obj);
            vtm.mEventHandler.sendMessage(m);
        }
    }

    public static void sendEventToVTService(int what, int arg1, int arg2, Object obj) {

        if (MediaHandler.getVTManager().mEventHandler != null) {
            Log.e(TAG, "Sending infor mess to vtservice");
            Message m = MediaHandler.getVTManager().mEventHandler.obtainMessage(what, arg1, arg2,
                    obj);
            MediaHandler.getVTManager().mEventHandler.sendMessage(m);
        }

    }

    private class EventHandler extends Handler {
        private VTManager mVTM;

        public EventHandler(VTManager vtm, Looper looper) {
            super(looper);
            mVTM = vtm;
        }

        @Override
        public void handleMessage(Message msg) {
            if (mVTM.mNativeContext == 0) {
                Log.w(TAG, "VTManager went away with unhandled events");
                return;
            }

            boolean error_was_handled = false;
            switch (msg.what) {
            case EventType.EVENT_INFO:
                if (mOnInfoListener != null)
                    mOnInfoListener.onInfo(mVTM, msg.arg1, msg.arg2, msg.obj);
                return;

            case EventType.EVENT_ERROR:
                Log.e(TAG, "Error (" + msg.arg1 + "," + msg.arg2 + ")");
                if (mOnErrorListener != null) {
                    error_was_handled = mOnErrorListener.onError(mVTM, msg.arg1, msg.arg2);
                }
                return;

            case EventType.EVENT_CMDCOMP:
                Log.e(TAG, "cmd complet (" + msg.arg1 + "," + msg.arg2 + ")");
                if (mOnCmdCompListener != null) {
                    error_was_handled = mOnCmdCompListener.onCmdComp(mVTM, msg.arg1, msg.arg2);
                }
                return;
            default:
                Log.e(TAG, "Unknown message type " + msg.what);
                return;
            }
        }
    }

    private native final void native_setup(Object vtmanager_this) throws IllegalStateException;

    private native final void native_finalize();

    public native void nativeHandleRawFrame(byte[] frame);

    /*
     * public SurfaceTexture getRemoteDisplay(Surface surface){ mIRemoteSurface
     * = getVTRemoteSurface(); if (MyLog.DEBUG)
     * MyLog.v(TAG,"getRemoteDisplay Succeed." ); return mIRemoteSurface; }
     */

    // public native SurfaceTexture getVTRemoteSurface();
}
