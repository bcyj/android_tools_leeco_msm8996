/* ExtendedRemoteDisplay.java
 * Implements RemoteDisplay APIs to use WFDService from Android framework
 *
 *
 * Copyright (c) 2013 - 2015 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 * Not a Contribution.
 * Apache license notifications and license are retained
 * for attribution purposes only.
 */
/*
 * Copyright (C) 2012 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.qualcomm.wfd;
import android.content.Intent;

import dalvik.system.CloseGuard;

import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.RemoteException;
import android.os.IBinder;
import android.os.Bundle;
import android.os.Message;
import android.util.Log;

import android.os.SystemProperties;
import android.view.Surface;
import com.qualcomm.wfd.WfdDevice;
import com.qualcomm.wfd.service.IWfdActionListener;
import com.qualcomm.wfd.service.ISessionManagerService;
import com.qualcomm.wfd.WfdEnums;
import com.qualcomm.wfd.WfdStatus;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.media.RemoteDisplay;
import android.view.Display;
import android.hardware.display.DisplayManager;
import android.view.WindowManager;
import android.graphics.Point;



enum WfdOperation {
    PLAY,
    PAUSE,
    STANDBY,
    RESUME,
    START_UIBC,
    STOP_UIBC,
    TEARDOWN
};

enum WfdMode {
    PLAYING,
    PAUSED,
    STANDING_BY,
    INVALID
};

enum WFDState {
    DEINIT,
    BINDING,
    BOUND,
    INITIALIZING,
    INITIALIZED,
    ESTABLISHING,
    ESTABLISHED,
    PLAY,
    PLAYING,
    TEARINGDOWN,
    TEARDOWN
}

class WFDCallback {
    public static final int PLAY_CALLBACK = 0;
    public static final int PAUSE_CALLBACK = 1;
    public static final int STANDBY_CALLBACK = 2;
    public static final int UIBC_ACTION_COMPLETED = 3;
    public static final int TEARDOWN_CALLBACK = 4;
    public static final int INVALID_WFD_DEVICE = 5;
    public static final int SET_WFD_FINISHED = 6;
    public static final int SERVICE_BOUND = 7;
    public static final int START_SESSION_ON_UI = 8;
    public static final int INIT_CALLBACK = 9;
    public static final int MM_STREAM_START = 10;
    public static final int INVALID_STATE = 11;
    // All error events to follow
    public static final int NOTIFY_DISPLAY_CONNECTED = -1;
    public static final int NOTIFY_DISPLAY_DISCONNECTED = -2;
    public static final int NOTIFY_DISPLAY_ERROR = -3;
};

class WFDCmds {
    public static final int START = 0;
    public static final int END   = 1;
};


class ServiceUtil {

    private static final String TAG = "ExtendedRemoteDisplay.ServiceUtil";
    private static ISessionManagerService uniqueInstance = null;
    private static boolean mServiceAlreadyBound = false;
    private static Handler eventHandler = null;

    protected static boolean getmServiceAlreadyBound() {
        return mServiceAlreadyBound;
    }

    public static void bindService(Context context, Handler inEventHandler)
        throws ServiceUtil.ServiceFailedToBindException {
            if (!mServiceAlreadyBound  || uniqueInstance == null) {
                Log.d(TAG,
                      "bindService- !AlreadyBound||uniqueInstance=null");

                Intent serviceIntent =
                    new Intent("com.qualcomm.wfd.service.WfdService");
                serviceIntent.setPackage("com.qualcomm.wfd.service");
                eventHandler = inEventHandler;
                if (!context.bindService(serviceIntent,
                                         mConnection, Context.BIND_AUTO_CREATE)) {
                    Log.e(TAG,"Failed to connect to Provider service");
                    throw new ServiceUtil.ServiceFailedToBindException(
                                         "Failed to connect to Provider service");
                }
            }
        }

    public static void unbindService(Context context) {
        if (mServiceAlreadyBound) {
            try {
                context.unbindService(mConnection);
            } catch (IllegalArgumentException e) {
                Log.e(TAG,"IllegalArgumentException: " + e);
            }
            mServiceAlreadyBound = false;
            uniqueInstance = null;
        }
    }

    public synchronized static ISessionManagerService getInstance() {
        while (uniqueInstance == null) {
            Log.d(TAG, "Waiting for service to bind ...");
            try {
                ServiceUtil.class.wait();
            } catch (InterruptedException e) {
                Log.e(TAG, "InterruptedException: " + e);
            }
        }
        return uniqueInstance;
    }

    public static class ServiceFailedToBindException extends Exception {
        public static final long serialVersionUID = 1L;

        private ServiceFailedToBindException(String inString)
        {
            super(inString);
        }
    }

    protected static ServiceConnection mConnection = new ServiceConnection() {
        public void onServiceConnected
                                  (ComponentName className, IBinder service) {
            Log.d(TAG, "Connection object created");
            mServiceAlreadyBound = true;
            uniqueInstance = ISessionManagerService.Stub.asInterface(service);
            synchronized(ServiceUtil.class) {
                ServiceUtil.class.notifyAll();
            }
            Message messageBound =
                        eventHandler.obtainMessage(WFDCallback.SERVICE_BOUND);
            eventHandler.sendMessage(messageBound);
        }

        public void onServiceDisconnected(ComponentName className) {
            Log.d(TAG, "Remote service disconnected");
            mServiceAlreadyBound = false;
        }
    };

}


class WfdActionListenerImpl extends IWfdActionListener.Stub {
    Handler mHandler;
    private static final String TAG = "ExtendedRemoteDisplay.WfdActionListenerImpl";
    WfdActionListenerImpl(Handler handler) {
        super();
        mHandler = handler;
    }

    @Override
    public void onStateUpdate(int newState, int sessionId)
                                                   throws RemoteException {
        WfdEnums.SessionState state = WfdEnums.SessionState.values()[newState];
        switch (state) {
            case INITIALIZED:
                Log.d(TAG, "WfdEnums.SessionState==INITIALIZED");
                if (sessionId > 0) {
                    Log.d(TAG,
                         "WfdEnums.SessionState==INITIALIZED, sessionId > 0");
                    Message messageTeardown =
                         mHandler.obtainMessage(WFDCallback.TEARDOWN_CALLBACK);
                    mHandler.sendMessage(messageTeardown);
                } else {
                    Log.d(TAG,
                         "WfdEnums.SessionState==INITIALIZED, Init callback");
                    Message messageInit =
                          mHandler.obtainMessage(WFDCallback.INIT_CALLBACK);
                    mHandler.sendMessage(messageInit);
                }
                break;

            case INVALID:
                Log.d(TAG, "WfdEnums.SessionState==INVALID");
                Message messageInvalid = mHandler
                        .obtainMessage(WFDCallback.INVALID_STATE);
                mHandler.sendMessage(messageInvalid);
                break;

            case IDLE:
                Log.d(TAG, "WfdEnums.SessionState==IDLE");
                break;

            case PLAY:
                Log.d(TAG, "WfdEnums.SessionState==PLAY");
                Message messagePlay =
                             mHandler.obtainMessage(WFDCallback.PLAY_CALLBACK);
                mHandler.sendMessage(messagePlay);
                break;

            case PAUSE:
                Log.d(TAG, "WfdEnums.SessionState==PAUSE");
                Message messagePause =
                            mHandler.obtainMessage(WFDCallback.PAUSE_CALLBACK);
                mHandler.sendMessage(messagePause);
                break;

            case STANDING_BY:
                Log.d(TAG, "WfdEnums.SessionState = STANDING_BY");
                Message messageStandby =
                          mHandler.obtainMessage(WFDCallback.STANDBY_CALLBACK);
                mHandler.sendMessage(messageStandby);
                break;

            case ESTABLISHED:
                Log.d(TAG, "WfdEnums.SessionState==ESTABLISHED");
                Message messageEstablishedCallback =
                       mHandler.obtainMessage(WFDCallback.START_SESSION_ON_UI);
                mHandler.sendMessage(messageEstablishedCallback);
                break;

            case TEARDOWN:
                Log.d(TAG, "WfdEnums.SessionState==TEARDOWN");
                Message messageTeardown =
                         mHandler.obtainMessage(WFDCallback.TEARDOWN_CALLBACK);
                mHandler.sendMessage(messageTeardown);
                break;
        }
    }

    @Override
    public void notifyEvent(int event, int sessionId) throws RemoteException {
        if (event == WfdEnums.WfdEvent.TEARDOWN_START.ordinal()) {
            Message msgNotDispDisconnec = mHandler
                    .obtainMessage(WFDCallback.NOTIFY_DISPLAY_DISCONNECTED);
            mHandler.sendMessage(msgNotDispDisconnec);
        }
    }

    @Override
    public void notify(Bundle b, int sessionId) throws RemoteException {
        if(b != null) {
            Log.d(TAG, "Notify from WFDService");
            String event = b.getString("event");
            if("MMStreamStarted".equalsIgnoreCase(event)) {
                Message messageEvent =
                     mHandler.obtainMessage(WFDCallback.MM_STREAM_START);
                messageEvent.setData(b);
                mHandler.sendMessage(messageEvent);
            }
        }
    }
}



/**
 * Listens for Wifi remote display connections managed by the media server.
 *
 * @hide
 */
public final class ExtendedRemoteDisplay{

    public static int mRefs;
    private static final String TAG = "ExtendedRemoteDisplay";
    private static final String sDownscaleKey = "sys.hwc.mdp_downscale_enabled";
    private static boolean sIsDownscaleSupported = false;

    static {
        System.loadLibrary("extendedremotedisplay");
        Log.d(TAG, "Loaded extendedremotedisplay.so");
        /*Static variable is updated to make sure the function gets into the
          library */
        try {
            sIsDownscaleSupported = SystemProperties.getBoolean(sDownscaleKey,false);
            if (sIsDownscaleSupported) {
               Log.e(TAG, "Downscaling property available");
            }
        } catch (Exception e) {
            Log.e(TAG, "Exception while getting system Property");
            e.printStackTrace();
        }
        mRefs++;
    }

    /* these constants must be kept in sync with IRemoteDisplayClient.h */

    public static final int DISPLAY_FLAG_SECURE = 1 << 0;

    public static final int DISPLAY_ERROR_UNKOWN = 1;
    public static final int DISPLAY_ERROR_CONNECTION_DROPPED = 2;

    private static final int WFD_SOURCE = 0;

    private static final int WFD_PRIMARY_SINK = 1;

    private static final int OP_TIMEOUT = 10000; // 10 seconds

    private final CloseGuard mGuard = CloseGuard.get();
    private RemoteDisplay.Listener mListener;
    private Handler  mHandler;
    public  ExtendedRemoteDisplayCmdHandler  mCmdHdlr;
    private HandlerThread mHThread;
    private Looper   mLooper;

    private Context mContext;

    private long mPtr;
    private Surface surface;
    private String mIface;

    private boolean mConnected;

    private WfdDevice mLocalWfdDevice;
    private WfdDevice mPeerWfdDevice;
    private ExtendedRemoteDisplayEventHandler mEventHandler;
    private IWfdActionListener mActionListener;
    public boolean mInvalid = true;

    public static WFDState sState = WFDState.DEINIT;
    public static Object sERDLock = new Object();

    private native long getNativeObject();
    private native Surface getSurface(long obj, int width, int height);
    private native int destroySurface(long obj, Surface surface);
    private native void destroyNativeObject(long obj);

    public ExtendedRemoteDisplay(RemoteDisplay.Listener listener,
                           Handler handler,
                           Context context) {
        mListener = listener;
        mHandler = handler;
        mContext = context;
        mEventHandler = new ExtendedRemoteDisplayEventHandler();

        mHThread = new HandlerThread(TAG);
        mHThread.start();
        mLooper = mHThread.getLooper();
        mCmdHdlr = new ExtendedRemoteDisplayCmdHandler(mLooper);
    }

    @Override
    protected void finalize() throws Throwable {
        Log.i(TAG, "finalize called on " + this.toString());
        try {
            if (!mInvalid) {
                Log.e(TAG, "!!!Finalized without being invalidated!!!");
                // Plod on to teardown. Ideally this should be never
                // be hit if state machine works fine but something is pretty
                // messed up here
            dispose(true);
            }
            if(mEventHandler != null) {
               mEventHandler = null;
            }
        } finally {
            super.finalize();
        }
    }

    /**
     * Starts listening for displays to be connected on the specified interface.
     *
     * @param iface The interface address and port in the form "x.x.x.x:y".
     * @param listener The listener to invoke
     *         when displays are connected or disconnected.
     * @param handler The handler on which to invoke the listener.
     * @param context The current service context
     *  */
    public static ExtendedRemoteDisplay listen(String iface, RemoteDisplay.Listener listener,
                                         Handler handler, Context context) {
        if (iface == null) {
            throw new IllegalArgumentException("iface must not be null");
        }
        if (listener == null) {
            throw new IllegalArgumentException("listener must not be null");
        }
        if (handler == null) {
            throw new IllegalArgumentException("handler must not be null");
        }

        ExtendedRemoteDisplay display =
            new ExtendedRemoteDisplay(listener, handler, context);

        if(display.mCmdHdlr == null) {
            return null;
        }

        display.startListening(iface);

        return display;
    }

    /**
     * Disconnects the remote display and stops listening for new connections.
     */
    public void dispose() {
        dispose(false);
    }

    private void dispose(boolean finalized) {

        if (mGuard != null) {
            if (finalized) {
                mGuard.warnIfOpen();
            } else {
                mGuard.close();
            }
        }
        if (mCmdHdlr != null) {
            /*
             * The cmdHdlr is nullified once the current instance is
             * invalidated. That happens ONLY when ERD moves to DEINIT state by
             * which time we are sure that : 1. Display has been notified of
             * disconnection 2. ERD has unbound from WFD service 3. deinit() has
             * been called upon WFD service This is basically what was supposed
             * to be done by posting the message on the cmdHdlr, so we are still
             * in good shape not posting it
             */
            Message messageEnd = mCmdHdlr.obtainMessage(WFDCmds.END);
            mCmdHdlr.sendMessage(messageEnd);
        }
    }

    /**
     * Starts WFDService and waits for incoming RTSP connections
     */
    public void startListening(String iface) {
        mIface = iface;
        Log.i(TAG, "New ERD instance" + this.toString());
        SystemProperties.set("persist.sys.wfd.virtual","1");

        Message messageStart =
                           mCmdHdlr.obtainMessage(WFDCmds.START);
        mCmdHdlr.sendMessage(messageStart);

        mGuard.open("dispose");
    }



    /**Creats Local WFDDevice Object from iface in the format
     * ip.ad.dr.ess:port
     * */
    private void createLocalWFDDevice(String  iface) {

        mLocalWfdDevice = new WfdDevice();
        mLocalWfdDevice.deviceType = WFD_SOURCE;
        mLocalWfdDevice.macAddress = null;

        if(iface != null) {
            Log.e(TAG, "Create WFDDevice from" + iface);
            int index = iface.indexOf(':', 0);
            if(index > 0) {
                mLocalWfdDevice.ipAddress = iface.substring(0, index);
                mLocalWfdDevice.rtspPort  =
                  Integer.parseInt(iface.substring(index + 1, iface.length()));

                if(mLocalWfdDevice.ipAddress == null ||
                    mLocalWfdDevice.rtspPort < 1 ||
                       mLocalWfdDevice.rtspPort > 65535) {
                    Log.e(TAG, "Invalid RTSP port received or no valid IP");
                    mLocalWfdDevice = null;
                }
            }
        }
    }

    /**Creats Peer WFDDevice Object from iface in the format
     * ip.ad.dr.ess:port
     * */
    private void createPeerWFDDevice(String  iface) {
        mPeerWfdDevice = new WfdDevice();
        mPeerWfdDevice.deviceType = WFD_PRIMARY_SINK;
        mPeerWfdDevice.macAddress = null;
        mPeerWfdDevice.ipAddress = null;
        mPeerWfdDevice.rtspPort = 0;

    }

    private void notifyDisplayConnected(final Surface surface,
            final int width, final int height, final int flags) {
        mHandler.post(new Runnable() {
            @Override
            public void run() {
                mConnected = true;
                    mListener.onDisplayConnected(surface, width, height, flags,0);
            }
        });
    }

    private void notifyDisplayDisconnected() {
        mHandler.post(new Runnable() {
            @Override
             public void run() {
                if(mConnected == true) {
                    mListener.onDisplayDisconnected();
                    mConnected = false;
                }
            }
        });
    }

    private void notifyDisplayError(final int error) {
        mHandler.post(new Runnable() {
            @Override
            public void run() {
                mListener.onDisplayError(error);
            }
        });
    }


    /**
     * Class for internal event handling.
     */
    class ExtendedRemoteDisplayEventHandler extends Handler {

        @Override
        public void handleMessage(Message msg) {
            Log.d(TAG, "Event handler received: " + msg.what);

            switch (msg.what) {

                case WFDCallback.PLAY_CALLBACK: {
                    sState = WFDState.PLAY;
                    Log.d(TAG, "WFDService in PLAY state");
                }
                break;

                case WFDCallback.PAUSE_CALLBACK: {
                    Log.d(TAG, "WFDService in PAUSE state");
                }
                break;

                case WFDCallback.STANDBY_CALLBACK: {
                    Log.d(TAG, "WFDService in STANDBY state");
                }
                break;

                case WFDCallback.SERVICE_BOUND: {
                    ExtendedRemoteDisplay.this.mActionListener =
                                      new WfdActionListenerImpl(mEventHandler);
                    sState = WFDState.BOUND;
                    SystemProperties.set("persist.sys.wfd.virtual","1");
                    try {
                        //set local device type
                        int setDeviceTypeRet =
                            ServiceUtil.getInstance().setDeviceType(WFD_SOURCE);
                        if (setDeviceTypeRet == 0) {
                            Log.d(TAG, "mWfdService.setDeviceType called.");
                        } else {
                            Log.d(TAG,
                                 "mWfdService.setDeviceType failed error code: "
                                  + setDeviceTypeRet);
                        }

                        //verify session state
                        WfdStatus wfdStatus =
                                     ServiceUtil.getInstance().getStatus();

                        Log.d(TAG, "wfdStatus.state= " + wfdStatus.state);

                        if (wfdStatus.state ==
                              WfdEnums.SessionState.INVALID.ordinal() ||
                            wfdStatus.state ==
                              WfdEnums.SessionState.INITIALIZED.ordinal()) {
                            Log.d(TAG,
                                 "wfdStatus.state is INVALID or INITIALIZED");
                        }

                        if(mLocalWfdDevice != null) {
                            ServiceUtil.getInstance().
                                 initSys(ExtendedRemoteDisplay.this.mActionListener,
                                 ExtendedRemoteDisplay.this.mLocalWfdDevice);
                            sState = WFDState.INITIALIZING;
                        }


                    } catch (RemoteException e) {
                        Log.e(TAG, "WfdService init() failed", e);
                        return;
                    }
                }
                break;
                case WFDCallback.TEARDOWN_CALLBACK: {
                    try {
                        sState = WFDState.TEARDOWN;
                        notifyDisplayDisconnected();
                        ServiceUtil.getInstance().deinit();
                        if (mPtr != 0) {
                            if (mPtr != -1) {
                                /*
                                 * This instance is being deleted. Get rid of
                                 * the native object and the surface only in
                                 * case the source modules haven't supplied the
                                 * surface else don't invoke the ~s of native
                                 * ERD, since its instances weren't created in
                                 * the first place itself
                                 */
                                destroySurface(mPtr, surface);
                                destroyNativeObject(mPtr);
                            }
                            if (surface != null) {
                                if (surface.isValid()) {
                                    surface.release();
                                    Log.e(TAG,
                                            "Released surface successfully");
                                } else {
                                    Log.e(TAG, "surface not valid");
                                }
                            } else {
                                Log.e(TAG, "Why on earth is surface null??");
                            }
                            mPtr = 0;
                        }
                        SystemProperties.set("persist.sys.wfd.virtual","0");
                    } catch (RemoteException e) {
                        Log.e(TAG,
                        "EventHandler:Remote exception when calling deinit()",
                        e);
                    }

                }
                break;

                case WFDCallback.MM_STREAM_START: {
                    sState = WFDState.PLAYING;
                    Bundle b = msg.getData();
                    int width = b.getInt("width");
                    int height = b.getInt("height");

                    /* Note that secure here doesn't Indicate the session is
                       always secure. It just means that link is protected and
                       WFDService can switch to secure if a secure content
                       is played
                    */
                    int secure = b.getInt("hdcp");

                    surface = (Surface) (b.getParcelable("surface"));
                    if (mPtr == 0) {
                        if (surface == null) {
                            // Use native ERD to get surface
                            Log.d(TAG, "Create Native object");
                            mPtr = getNativeObject();
                            if (mPtr == 0) {
                                Log.e(TAG,
                                        "ExtendedRemoteDisplay Failed to get Native Object");
                            } else {
                                surface = getSurface(mPtr, width, height);
                            }
                        } else {
                            Log.e(TAG, "Surface is supplied by source module");
                            // Use surface supplied by source modules
                            mPtr = -1; // mPtr is now reduced to just a dummy
                            // flag to prevent re-notification
                            // to Display Manager, setting it to a
                            // negative value to avoid using any more
                            // extra flags than necessary
                        }

                        if (sIsDownscaleSupported && mPtr != -1) {
                            // Go through the entire rigmarole of reporting
                            // higher primary resolution only if downscale
                            // support is available on target, else report
                            // external resolution. Also this is applicable only
                            // for legacy V4L2 solution, in case of VDS based
                            // approach, true WFD resolution is to be reported
                            DisplayManager displayManager = (DisplayManager) mContext
                                .getSystemService(Context.DISPLAY_SERVICE);
                            Display[] displays = displayManager.getDisplays();
                            for (int i = 0; i < displays.length; i++) {
                                if (displays[i].getType() == Display.TYPE_BUILT_IN) {
                                    Point point = new Point();
                                    displays[i].getRealSize(point);
                                    Log.d(TAG, "Primary Height and width: "
                                        + point.x + " " + point.y);
                                    int xres = 0;
                                    int yres = 0;
                                    if (point.x < 2048 && point.y < 2048) {
                                        if (point.x > point.y) {
                                            xres = point.x;
                                            yres = point.y;
                                        } else {
                                            xres = point.y;
                                            yres = point.x;
                                        }
                                        if (xres > width || yres > height) {
                                            width = xres;
                                            height = yres;
                                        }
                                    }
                                    break;//Found Built-in display
                                }
                            }
                        }
                        Log.d(TAG,
                                "MM Stream Started Width, Height and Secure: "
                                        + " " + width + " " + height + " "
                                        + secure);

                        if(secure != 0) {
                            notifyDisplayConnected(surface, width, height,
                                               DISPLAY_FLAG_SECURE);
                        } else {
                            notifyDisplayConnected(surface, width, height, 0);
                        }
                    } else {
                        Log.e(TAG,
                          "ExtendedRemoteDisplay Not honor stream start");
                    }
                }
                break;

                case WFDCallback.INIT_CALLBACK: {
                    try{
                        sState = WFDState.ESTABLISHING;
                        int ret = ServiceUtil.getInstance().
                                               startWfdSession(mPeerWfdDevice);
                        Log.d(TAG, "EventHandler:startSession-initiated" + ret);
                        mInvalid = false;
                    }catch (RemoteException e){
                        Log.e(TAG, "EventHandler: startSession- failed");
                        notifyDisplayError(DISPLAY_ERROR_UNKOWN);
                    }
                }
                break;

                case WFDCallback.START_SESSION_ON_UI: {
                    sState = WFDState.ESTABLISHED;
                    Log.d(TAG, "EventHandler: startSession- completed");
                }
                break;

                case WFDCallback.UIBC_ACTION_COMPLETED: {
                    Log.d(TAG, "EventHandler: uibcActionCompleted- completed");
                }
                break;

                case WFDCallback.INVALID_STATE: {
                    synchronized (ExtendedRemoteDisplay.sERDLock) {
                        notifyDisplayDisconnected();
                        sState = WFDState.DEINIT;
                        mInvalid = true;
                        if (ServiceUtil.getmServiceAlreadyBound()) {
                            try {
                                ServiceUtil.getInstance().unregisterListener(
                                        ExtendedRemoteDisplay.this.mActionListener);
                            } catch (RemoteException e) {
                                Log.e(TAG,
                                        "RemoteException in unregistering listener"
                                                + e);
                            }
                        }

                        if(mLooper != null) {
                           mLooper.quitSafely();
                           mLooper = null;
                        }

                        if(mHThread != null) {
                           mHThread.quitSafely();
                           try {
                               mHThread.join();
                           } catch (InterruptedException e) {
                               Log.e(TAG,"EventHandler: Failed to join for mHThread");
                           }
                        }

                        if(mCmdHdlr != null) {
                           mCmdHdlr = null;
                        }

                        Log.d(TAG, "Unbind the WFD service");
                        ServiceUtil.unbindService(mContext);

                        ExtendedRemoteDisplay.sERDLock.notifyAll();
                    }
                    Log.d(TAG, "ERD instance invalidated");
                }
                break;
                case WFDCallback.NOTIFY_DISPLAY_DISCONNECTED: {
                    notifyDisplayDisconnected();
                }
                break;
                default:
                    Log.e(TAG, "Unknown event received: " + msg.what);
            }
        }
    }


    /**
     * Class for Command handling. Must run on UI thread.
     */
    class ExtendedRemoteDisplayCmdHandler extends Handler {

        public ExtendedRemoteDisplayCmdHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            Log.d(TAG, "Cmd handler received: " + msg.what);

            switch (msg.what) {

                case WFDCmds.START: {
                    if (sState != WFDState.DEINIT) {
                        Log.d(TAG, "Waiting to move to DEINIT");
                        synchronized (ExtendedRemoteDisplay.sERDLock) {
                            try {
                                ExtendedRemoteDisplay.sERDLock
                                        .wait(ExtendedRemoteDisplay.OP_TIMEOUT);
                            } catch (InterruptedException e) {
                                Log.e(TAG,
                                        "InterruptedException while waiting to move to DEINIT");
                                e.printStackTrace();
                            }
                        }
                    }
                    if(sState!= WFDState.DEINIT) {
                        Log.e(TAG, "Something's gone kaput!");
                        notifyDisplayError(DISPLAY_ERROR_UNKOWN);
                        break;
                    }
                    Log.d(TAG, "Started......");

                    createLocalWFDDevice(mIface);
                    createPeerWFDDevice(mIface);

                    if (mLocalWfdDevice == null || mPeerWfdDevice == null) {
                        throw new IllegalStateException(
                                  "Could not start listening for "
                          + "remote display connection on \"" + mIface + "\"");
                    }

                    try {
                        sState = WFDState.BINDING;
                        ServiceUtil.bindService(mContext, mEventHandler);
                    } catch (ServiceUtil.ServiceFailedToBindException e) {
                        Log.e(TAG, "ServiceFailedToBindException received");
                    }

                }
                break;

                case WFDCmds.END: {

                    Log.d(TAG, "Ending.........");
                    Log.d(TAG, "Notify Display Disconnected");
                    notifyDisplayDisconnected();
                    if (sState == WFDState.DEINIT) {
                        SystemProperties.set("persist.sys.wfd.virtual","0");
                        return;
                    }
                    if(sState == WFDState.INITIALIZED ||
                                           sState == WFDState.INITIALIZING) {
                        try {
                            ServiceUtil.getInstance().deinit();
                            Log.d(TAG, "Unbind the WFD service");
                            ServiceUtil.unbindService(mContext);
                            SystemProperties.set("persist.sys.wfd.virtual","0");
                        } catch (RemoteException e) {
                            Log.e(TAG, "RemoteException in deInit");
                        }
                        return;
                    }

                    try {
                        if(ServiceUtil.getmServiceAlreadyBound()) {
                          Log.d(TAG, "Teardown WFD Session");
                          ServiceUtil.getInstance().teardown();
                        }
                    } catch (RemoteException e) {
                        Log.e(TAG, "RemoteException in teardown");
                    }
                }
                break;

                default:
                    Log.e(TAG, "Unknown cmd received: " + msg.what);
            }
        }
    }


}

