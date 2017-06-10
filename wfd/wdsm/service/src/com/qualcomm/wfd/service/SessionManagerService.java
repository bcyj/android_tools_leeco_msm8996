/* ==============================================================================
 * SessionManagerService.java
 *
 * Session Manager service implementation
 *
 * Copyright (c) 2012 - 2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 ============================================================================== */

package com.qualcomm.wfd.service;

import com.qualcomm.wfd.UIBCManager;
import com.qualcomm.wfd.WFDNative;
import com.qualcomm.wfd.WfdEnums;
import com.qualcomm.wfd.WfdEnums.*;
import com.qualcomm.wfd.WfdStatus;
import com.qualcomm.wfd.WfdDevice;

import java.util.Arrays;
import java.util.List;

import android.app.Notification;
import android.app.NotificationManager;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Bundle;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.RemoteCallbackList;
import android.os.RemoteException;
import android.os.Binder;
import android.os.IBinder;
import android.os.Process;
import android.os.SystemProperties;

import android.provider.Settings;
import android.util.Log;
import android.view.WindowManagerPolicy;
import android.view.InputEvent;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.Surface;
import android.view.Display;
import android.view.WindowManager;

import android.util.DisplayMetrics;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Point;
import android.content.res.Configuration;
import android.content.res.Resources;
import android.media.AudioSystem;
import android.hardware.display.DisplayManager;
import android.hardware.display.VirtualDisplay;

import java.io.File;
import java.io.IOException;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.ParserConfigurationException;

import org.w3c.dom.Document;
import org.w3c.dom.Node;
import org.xml.sax.SAXException;

public class SessionManagerService extends ISessionManagerService.Stub implements
 WFDNative.WfdActionListener, UIBCManager.HIDCEventListener {

    private static final String TAG = "SessionManagerService";

    private static final Object StaticLock = new Object();

    private static final int OPERATION_TIMEOUT = 5000; // 5 sec

    private static final int NOTI_MSG_ID = 42;

    private boolean mIsHDMIConnected = false;

    private boolean mIsHDMIConnectionAllowed = false;

    private static boolean sIsHDMI_WFD_ConcurrencyAllowed = false;

    private static final String sPlatform = "ro.board.platform";

    static {
       if (SystemProperties.get(sPlatform,"not found").contains("apq8084")) {
          sIsHDMI_WFD_ConcurrencyAllowed = true;
       }
    }

    private static boolean IS_HDCP_ENABLED = false;

    private boolean IS_RTP_TRANSPORT_NEGOTIATED = false;

    private boolean IS_PLAYBACK_CONTROL_COMPLETE = false;

    private static boolean IS_WFD_AUDIO_ENABLED = false;

    private static AVPlaybackMode PLAYBACK_MODE = AVPlaybackMode.AUDIO_VIDEO;

    private static RemoteCallbackList<IWfdActionListener> ActionListeners = null;

    private static UIBCManager UIBCMgr = null;

    private static boolean uibcEnabled = false;

    private static SessionState State = SessionState.INVALID;

    private static SessionState BeforeStandbyState = null;

    private static SessionState BeforeSecureState = null;

    private static int SessionId = -1;

    private int mCurrTransport = 0;

    private static WFDDeviceType DeviceType = WFDDeviceType.SOURCE;

    private WfdDevice mConnectedDevice = null;

    private Context mContext;

    private BroadcastReceiver mReceiver;

    private BroadcastReceiver mProtectedReceiver;

    public static final String configFileLocation = "/system/etc/wfdconfig.xml";

    private DocumentBuilder sDocumentBuilder = null;

    private Document sDocument = null;

    private File sConfigFile = null;

    private static Surface sSurface;

    private int[] mConfigItems = null;

    private boolean mServiceStarted = false;

    private boolean mSysInvoked = false; // Flag to indicate whether WFD Service
    // has been invoked through system setting app or through any other client
    // (at this point how to distinguish if the client too runs as system ?)

    private DisplayManager mDisplayManager = null;

    private VirtualDisplay mVirtualDisplay = null;

    private static final String broadcastPermission = "com.qualcomm.permission.wfd.QC_WFD";

    private static RemoteCallbackList<IHIDEventListener> HIDEventListeners = null;

    private int mUIBCrotation = -1;

    private boolean mRequestProxyRouting = true;

    private NotificationManager mNotiMgr = null;

    private boolean mDebug = false;

    private static final String sDebugKey = "persist.debug.wfd.wfdsvc";

    private HandlerThread mCmdHdlrThread = null;

    private Handler mCmdHdlr = null;

    private Runnable mTeardownRunnable = null;

    private static final String sWFDEnableKey = "sys.wfdservice";

    private int mVirtualDisplayDPI;

    private static final long wfdScanInterval = 300000; // ms i.e. 5min

    // This value should be in sync with value of mDisconnectedScanPeriodMs in
    // WiFiStateMachine.java
    private static final long defScanInterval = 10000; // ms

    public SessionManagerService(Context context) {
        Log.d(TAG, "SessionManagerService ctor");
        SystemProperties.set(sWFDEnableKey, "enable");
        mContext = context;
        setupXML();
        synchronized (StaticLock) {
            if (ActionListeners == null) {
                Log.d(TAG, "Reinitializing callback list");
                ActionListeners = new RemoteCallbackList<IWfdActionListener>();
            } else {
                Log.d(TAG, "Callback list in not null");
            }
            if(HIDEventListeners ==null) {
                Log.d(TAG, "Reinitializing HID callback list");
                HIDEventListeners = new RemoteCallbackList<IHIDEventListener>();
            }
        }
        mConfigItems = new int[ConfigKeys.TOTAL_CFG_KEYS.ordinal()];
        mDisplayManager = (DisplayManager) mContext
                .getSystemService(Context.DISPLAY_SERVICE);
        mCmdHdlrThread = new HandlerThread("WFD_CmdHdlr");
        mCmdHdlrThread.start();
        mCmdHdlr = new Handler(mCmdHdlrThread.getLooper());
        mTeardownRunnable = new Runnable() {

                        @Override
                        public void run() {
                               teardown();
                        }
        };
        Log.d(TAG, mCmdHdlrThread.getName() + " setup successfully");
        int ret = teardown();
        Log.d(TAG, "Cleanup any existing sessions. ret: " + ret);
        mReceiver = new BroadcastReceiver() {
            public void onReceive(Context context, Intent intent) {
                Log.d(TAG, "Received intent: " + intent.toUri(0));
                String action = intent.getAction();
                synchronized (StaticLock) {
                    if (action.equals(WindowManagerPolicy.ACTION_HDMI_PLUGGED)) {
                        mIsHDMIConnected = intent.getBooleanExtra(
                                WindowManagerPolicy.EXTRA_HDMI_PLUGGED_STATE, false);
                        if (!mIsHDMIConnectionAllowed && mIsHDMIConnected &&
                            DeviceType == WFDDeviceType.SOURCE) {
                            stopWfdSession();
                            stopUibcSession();
                        }
                    } else if (action.equals(Intent.ACTION_SCREEN_OFF)) {
                        BeforeStandbyState = State;
                        if (BeforeStandbyState == SessionState.PLAYING) {
                            try {
                                Log.e(TAG, "Waiting to play");
                                StaticLock.wait(OPERATION_TIMEOUT);
                            } catch (InterruptedException e) {
                                Log.e(TAG, "Wait for PLAY state interrupted");
                            }
                            if (State != SessionState.PLAY) {
                                Log.e(TAG, "Session failed to move to play state");
                            }
                        }
                        if (AudioSystem.isStreamActive(AudioSystem.STREAM_MUSIC, 0)) {
                           if ((getValue("AudioStreamInSuspend")).contains("1")) {
                               Log.e(TAG,"Audio Stream is on so not calling standby");
                           } else {
                               Log.e(TAG,"Calling Standby even if Audio Stream is on");
                               standby();
                           }
                        } else {
                           Log.e(TAG,"Calling Standby as no Audio Stream is playing");
                           standby();
                        }
                    } else if (action.equals(Intent.ACTION_SCREEN_ON)) {
                        if (BeforeStandbyState != null
                                && (BeforeStandbyState == SessionState.PLAY || BeforeStandbyState == SessionState.PLAYING)) {
                            if (State == SessionState.STANDING_BY) {
                                try {
                                    Log.e(TAG, "Waiting to standby");
                                    StaticLock.wait(OPERATION_TIMEOUT);
                                } catch (InterruptedException e) {
                                    Log.e(TAG, "Wait for standby state interrupted");
                                }
                                if (State != SessionState.STANDBY) {
                                    Log.e(TAG, "Session failed to move to standby state");
                                }
                            }
                            Log.d(TAG, "Resume session calling play()");
                            BeforeStandbyState = null;
                            play();
                        }
                    } else if(action.equals(Intent.ACTION_SHUTDOWN)) {
                        teardown();
                    } else if(action.equals(Intent.ACTION_CONFIGURATION_CHANGED)){
                        if (DeviceType == WFDDeviceType.SOURCE) {
                            setSurfaceProp(-1, -1, -1);
                        }
                    }
                }
            }
        };
        IntentFilter filter = new IntentFilter(WindowManagerPolicy.ACTION_HDMI_PLUGGED);
        filter.addAction(Intent.ACTION_SCREEN_ON);
        filter.addAction(Intent.ACTION_SCREEN_OFF);
        filter.addAction(Intent.ACTION_SHUTDOWN);
        filter.addAction(Intent.ACTION_CONFIGURATION_CHANGED);
        mContext.registerReceiver(mReceiver, filter);

        /**
         * Protect QUALCOMM speific proprietary intents from being broadcast
         * by any third-party app. Only system apps or those signed with the
         * platform signature can broadcast to this protected intent receiver
         * after acquiring the permission com.qualcomm.permission.wfd.QC_WFD
         */

        mProtectedReceiver = new BroadcastReceiver() {
            @Override
            public void onReceive(Context context, Intent intent) {
                Log.d(TAG, "Received intent: " + intent.toUri(0));
                String action = intent.getAction();
                synchronized (StaticLock) {
                    if (action.equals(WfdEnums.ACTION_WIFI_DISPLAY_RESOLUTION)) {
                        int formatType = intent.getIntExtra("format",
                                CapabilityType.WFD_CEA_RESOLUTIONS_BITMAP
                                        .ordinal());
                        int value = intent.getIntExtra("value", 0);
                        Log.d(TAG, "Calculated formatType: " + formatType
                                + " value: 0x" + Integer.toHexString(value));
                        int ret = setResolution(formatType, value);
                        Log.d(TAG, "setResoltuion returned: " + ret);
                    } else if (action
                            .equals(WfdEnums.ACTION_WIFI_DISPLAY_BITRATE)) {
                        int bitrate = intent.getIntExtra("value", 0);
                        Log.d(TAG, "Calculated bitrate: " + bitrate);
                        int ret = setBitrate(bitrate);
                        Log.d(TAG, "setBitrate returned: " + ret);
                    } else if (action
                            .equals(WfdEnums.ACTION_WIFI_DISPLAY_RTP_TRANSPORT)) {
                        int transportType = intent.getIntExtra("type", -1);
                        int bufferLengthMs = intent.getIntExtra("bufferLength",
                                0);
                        int port = intent.getIntExtra("port", -1);
                        Log.d(TAG, "Calculted type: " + transportType
                                + ", bufferLengthMs: " + bufferLengthMs
                                + ", port: " + port);
                        int ret = setRtpTransport(transportType,
                                bufferLengthMs, port);
                        Log.d(TAG, "setRtpTransport returned: " + ret);
                    } else if (action
                            .equals(WfdEnums.ACTION_WIFI_DISPLAY_TCP_PLAYBACK_CONTROL)) {
                        int cmdType = intent.getIntExtra("type", -1);
                        Log.d(TAG, "Calculated type:" + cmdType);
                        int ret = tcpPlaybackControl(cmdType, 0);
                        Log.d(TAG, "tcpPlaybackControl returned: " + ret);
                    } else if (action
                            .equals(WfdEnums.ACTION_WIFI_DISPLAY_PLAYBACK_MODE)) {
                        int mode = intent.getIntExtra("mode", -1);
                        Log.d(TAG, "Calculated mode:" + mode);
                        int ret = setAvPlaybackMode(mode);
                        Log.d(TAG, "setAvPlaybackMode returned: " + ret);
                    }
                }
            }
        };
        IntentFilter protectedfilter = new IntentFilter(
                WfdEnums.ACTION_WIFI_DISPLAY_RESOLUTION);
        protectedfilter.addAction(WfdEnums.ACTION_WIFI_DISPLAY_BITRATE);
        protectedfilter.addAction(WfdEnums.ACTION_WIFI_DISPLAY_RTP_TRANSPORT);
        protectedfilter
                .addAction(WfdEnums.ACTION_WIFI_DISPLAY_TCP_PLAYBACK_CONTROL);
        protectedfilter.addAction(WfdEnums.ACTION_WIFI_DISPLAY_PLAYBACK_MODE);
        mContext.registerReceiver(mProtectedReceiver, protectedfilter,
                broadcastPermission, null);
        WFDNative.listener = this;
        UIBCManager.HIDCListener = this;
    }

    protected void destroyService() {
        Log.d(TAG, "destroyService()");
        synchronized (StaticLock) {
            Log.d(TAG, "unregister/kill remote callbacks");
            SystemProperties.set(sWFDEnableKey, "disable");
            ActionListeners.kill();
            ActionListeners = null;
            HIDEventListeners.kill();
            HIDEventListeners = null;
            mContext.unregisterReceiver(mReceiver);
            mReceiver = null;
            mContext.unregisterReceiver(mProtectedReceiver);
            mProtectedReceiver = null;
            mContext = null;
            WFDNative.listener = null;
            mCmdHdlr.removeCallbacks(mTeardownRunnable);
            mCmdHdlrThread.quitSafely();
            try {
                 mCmdHdlrThread.join();
            } catch (InterruptedException e) {
                 e.printStackTrace();
            }
            mTeardownRunnable = null;
            mCmdHdlr = null;
            mCmdHdlrThread = null;
            releaseFileReferences();
            mDisplayManager = null;
        }
    }

    private void setupXML() {
        try {
            sDocumentBuilder = DocumentBuilderFactory.newInstance().newDocumentBuilder();
        } catch (ParserConfigurationException e) {
            e.printStackTrace();
            releaseFileReferences();
            return;
        }
        sConfigFile = new File(configFileLocation);
        if (sConfigFile.isFile()) {
            try {
                 sDocument = sDocumentBuilder.parse(sConfigFile);
            } catch (SAXException e) {
                e.printStackTrace();
                releaseFileReferences();
                return;
            } catch (IOException e) {
                e.printStackTrace();
                releaseFileReferences();
                return;
            }
        } else {
            Log.e(TAG, sConfigFile + " is not on the filesystem");
            releaseFileReferences();
        }
    }

    private void releaseFileReferences() {
        Log.d(TAG, "releaseFileReferences()");
        sDocumentBuilder = null;
        sDocument = null;
        sConfigFile = null;
    }

    private Node findNodeByName(Node node, String name) {
        if (node != null && name != null) {
            if (name.equalsIgnoreCase(node.getNodeName())) {
               return node;
            } else {
                for (Node n = node.getFirstChild(); n != null;n = n
                        .getNextSibling()) {
                     Node found = findNodeByName(n, name);
                     if (found != null) {
                         return found;
                     }
                }
            }
        }
        return null;
    }

        public String getValue(String key) {
               Node value = findNodeByName(sDocument, key);
                if(value == null) {
                   //may be null if invalid key is provided
                   Log.e(TAG,key + "not found in config file");
                   return "Not Found";
                }
                return value.getTextContent();
        }

    @Override
    public void updateState(SessionState state, int sessionId) {
        Log.d(TAG, "updateState()");
        synchronized (StaticLock) {
            if (state == State && sessionId == SessionId) {
                Log.d(TAG, "Nothing has changed. Ignoring updateState");
                return;
            }
            if (State == SessionState.STANDING_BY
                    && state == SessionState.PAUSE) {
                State = SessionState.STANDBY;
            } else {
                State = state;
            }
            if (sessionId != SessionId) {
                Log.d(TAG, "Session id changed from " + SessionId + " to " + sessionId
                        + " with state " + state);
            } else {
                Log.d(TAG, "Session id " + sessionId + " with state " + state);
            }
            SessionId = sessionId;
            StaticLock.notifyAll();

            if (State == SessionState.PAUSE) {
                  Log.d(TAG, "PAUSE done");
            } else if (State == SessionState.STANDBY) {
                Log.d(TAG, "STANDBY Done");
            } else if (State == SessionState.PLAY) {
                Log.d(TAG, "PLAY done");
            } else if (State == SessionState.TEARDOWN) {
                Log.d(TAG, "TEARDOWN done");
                clearVars();
                if (DeviceType != WFDDeviceType.SOURCE) {
                  if(sSurface != null) {
                    sSurface.release();
                    if(sSurface.isValid()) {
                      Log.e(TAG, "Something really bad happened!");
                    }
                    else {
                      sSurface = null;
                      Log.e(TAG,"Released Surface during TEARDOWN Done!");
                    }
                  }
                }
                WFDNative.stopWfdSession(SessionId);
                State = SessionState.INITIALIZED;
                if (SessionId < 0) {
                    // teardown called in some transient SM state
                    // Update it to a +ve value to give callback with a +ve
                    // session Id so that client handling reamins intact
                    SessionId = 1;
                }
            }

            // Broadcast status update to listeners
            if (ActionListeners != null) {
                Log.d(TAG, "Sending onStateUpdate() to listeners");
                final int N = ActionListeners.beginBroadcast();
                for (int i = 0; i < N; i++) {
                    try {
                        ActionListeners.getBroadcastItem(i).onStateUpdate(State.ordinal(),
                                SessionId);
                    } catch (RemoteException e) {
                        Log.e(TAG, "Error sending status update to client, removing listener");
                    }
                }
                ActionListeners.finishBroadcast();
            } else {
                Log.d(TAG, "updateState: Remote callback list is null");
            }
        }
    }

    @Override
    public void notifyEvent(WfdEvent event, int sessionId) {
        Log.d(TAG, "notifyEvent()");
        synchronized (StaticLock) {
            if (event == WfdEvent.UIBC_ENABLED) {
                Log.d(TAG, "UIBC called from "+DeviceType);
                try {
                if (UIBCMgr == null) {
                   UIBCMgr = new UIBCManager(DeviceType.getCode());
                }
                } catch (InstantiationException e) {
                     Log.e(TAG, "InstantiationException: " + e);
                  }
                  catch (IllegalAccessException e) {
                     Log.e(TAG, "IllegalAccessException: " + e);
                  }
                  catch (ClassNotFoundException e) {
                     Log.e(TAG, "ClassNotFoundException: " + e);
                  }
                if (DeviceType == WFDDeviceType.SOURCE) {
                    setSurfaceProp(-1, -1, -1);
                }
                UIBCMgr.start();
                uibcEnabled = true;
                broadcastUIBCRotation(mUIBCrotation);
            } else if (event == WfdEvent.UIBC_DISABLED) {
               try{
                UIBCMgr.stop();
               } catch (NullPointerException e){
                     Log.e(TAG, "NullPointerException: " + e);
                  }
               uibcEnabled = false;
               broadcastUIBCRotation(-1);
            } else if (event == WfdEvent.PAUSE_START) {
                Log.d(TAG, "PAUSE start");
                State = SessionState.PAUSING;
                broadcastWifiDisplayAudioIntent(false);
                broadcastWifiDisplayVideoEnabled(false);
            } else if (event == WfdEvent.STANDBY_START) {
                State = SessionState.STANDING_BY;
                Log.d(TAG, "STANDBY_START");
                //Still do whatever PAUSE does
                broadcastWifiDisplayAudioIntent(false);
                broadcastWifiDisplayVideoEnabled(false);
            } else if (event == WfdEvent.PLAY_START) {
                Log.d(TAG, "PLAY start");
                State = SessionState.PLAYING;
                broadcastWifiDisplayVideoEnabled(true);
            } else if (event == WfdEvent.TEARDOWN_START) {
                Log.d(TAG, "TEARDOWN start");
                mRequestProxyRouting = false;
                broadcastWifiDisplayAudioIntent(false);
                broadcastWifiDisplayVideoEnabled(false);
                State = SessionState.TEARING_DOWN;
                // At this point we are sure that RTSP teardown sequence has
                // been completed successfully, so release the virtual display
                // in case it hasn't been done already. In case sink is abruptly
                // shutdown, then closecallback should map here as well, which
                // will again take care of releasing virtual display, for all
                // other runtime failures, teardown is called which will release
                // the surface
                releaseVDS();
            } else if (event == WfdEvent.HDCP_CONNECT_SUCCESS) {
                Log.d(TAG, "HDCP Connect Success");
                IS_HDCP_ENABLED = true;
            } else if (event == WfdEvent.HDCP_CONNECT_FAIL) {
                Log.d(TAG, "HDCP Connect Fail");
                IS_HDCP_ENABLED = false;
            } else if (event == WfdEvent.HDCP_ENFORCE_FAIL) {
                Log.d(TAG, "HDCP Unsupported by Peer. Fail");
                IS_HDCP_ENABLED = false;
                WFDNative.teardown(SessionId, true);
            } else if(event == WfdEvent.VIDEO_RUNTIME_ERROR ||
                    event == WfdEvent.AUDIO_RUNTIME_ERROR ||
                    event == WfdEvent.HDCP_RUNTIME_ERROR ||
                    event == WfdEvent.VIDEO_CONFIGURE_FAILURE ||
                    event == WfdEvent.AUDIO_CONFIGURE_FAILURE ||
                    event == WfdEvent.NETWORK_RUNTIME_ERROR ||
                    event == WfdEvent.NETWORK_CONFIGURE_FAILURE ||
                    event == WfdEvent.START_SESSION_FAIL) {
                Log.d(TAG, "Error event received:" + event);
                mCmdHdlr.post(mTeardownRunnable);
            } else if (event == WfdEvent.RTP_TRANSPORT_NEGOTIATED) {
                Log.d(TAG, "RTP transport is changed successfully");
                IS_RTP_TRANSPORT_NEGOTIATED = true;
            }else if (event == WfdEvent.AUDIOPROXY_CLOSED ) {
                broadcastWifiDisplayAudioIntent(false);
            }else if (event == WfdEvent.AUDIOPROXY_OPENED ) {
                broadcastWifiDisplayAudioIntent(true);
            }else if (event == WfdEvent.TCP_PLAYBACK_CONTROL) {
                IS_PLAYBACK_CONTROL_COMPLETE = true;
            }

            // Broadcast event to the listeners
            if (ActionListeners != null) {
                Log.d(TAG, "Sending notifyEvent() to listeners");
                final int N = ActionListeners.beginBroadcast();
                for (int i = 0; i < N; i++) {
                    try {
                        ActionListeners.getBroadcastItem(i).notifyEvent(event.ordinal(), SessionId);
                    } catch (RemoteException e) {
                        Log.e(TAG, "Error sending notification to client, removing listener");
                    }
                }
                ActionListeners.finishBroadcast();
            } else {
                Log.d(TAG, "notifyEvent: Remote callback list is null");
            }
        }
    }

    @Override
    public void notify(Bundle b, int sessionId) {
        synchronized(StaticLock) {
            Log.d(TAG, "notify()");
            if (b != null) {
                String event = b.getString("event", "none");
                if (!event.equalsIgnoreCase("none")) {
                    if ("UIBCRotateEvent".equalsIgnoreCase(event)) {
                        int angle = b.getInt("rot_angle", -1);
                        switch (angle) {
                            case 0:
                                mUIBCrotation = Surface.ROTATION_0;
                            break;
                            case 90:
                                mUIBCrotation = Surface.ROTATION_90;
                            break;
                            case 180:
                                mUIBCrotation = Surface.ROTATION_180;
                            break;
                            case 270:
                                mUIBCrotation = Surface.ROTATION_270;
                            break;
                            default:
                                mUIBCrotation = -1;
                        }
                        Log.d(TAG, "Value of mUIBCrotation is " + mUIBCrotation);
                        broadcastUIBCRotation(mUIBCrotation);
                    } else if ("MMStreamStarted".equalsIgnoreCase(event)) {
                        // Only in case the WFD session is established through
                        // client app do we need to create a virtual display,
                        // else a WifiDisplay is created anyway which will be
                        // taking care of notifying the surface to Display
                        // framework
                        int width = b.getInt("width");
                        int height = b.getInt("height");
                        int secure = b.getInt("hdcp");
                        if (!mSysInvoked) {
                            Log.d(TAG,
                                    "MM Stream Started Width, Height and secure: "
                                            + " " + width + " " + height + " "
                                            + secure);
                            Surface surface = (Surface) (b
                                    .getParcelable("surface"));
                            if (surface != null) {
                                Log.d(TAG, "Surface supplied by source modules");
                                int flags = 0;
                                flags |= DisplayManager.VIRTUAL_DISPLAY_FLAG_PUBLIC
                                        | DisplayManager.VIRTUAL_DISPLAY_FLAG_PRESENTATION;
                                if (secure == 1) {
                                    flags |= DisplayManager.VIRTUAL_DISPLAY_FLAG_SECURE
                                          | DisplayManager.VIRTUAL_DISPLAY_FLAG_SUPPORTS_PROTECTED_BUFFERS;
                                }
                                mVirtualDisplayDPI = Math.min(width, height)
                                        * DisplayMetrics.DENSITY_XHIGH / 1080;
                                if (mDisplayManager != null) {
                                    mVirtualDisplay = mDisplayManager
                                            .createVirtualDisplay(
                                                    mConnectedDevice.deviceName,
                                                    width,
                                                    height,
                                                    mVirtualDisplayDPI,
                                                    surface, flags);
                                }
                            }
                        }

                        if (mDebug) {
                            createNotification(width, height, secure);
                        }
                    }
                }
            }
            //Broadcast event to the listeners
            final int N = ActionListeners.beginBroadcast();
            for (int i = 0; i < N; i++) {
                try {
                    ActionListeners.getBroadcastItem(i).notify(b, SessionId);
                } catch (RemoteException e) {
                    Log.e(TAG, "Error sending notification to client, removing listener");
                }
            }
            ActionListeners.finishBroadcast();
        }
    }

    /**
     * @param width
     *            Width of WFD session resolution
     * @param height
     *            Height of WFD session resolution
     * @param secure
     *            Whether WFD session is secure or not This is a helper method
     *            to create a notification in the notification drawer
     */
    private void createNotification(int width, int height, int secure) {
        final int iconWidth = 200;
        final int iconHeight = 200;
        mNotiMgr = (NotificationManager) mContext
                .getSystemService(Context.NOTIFICATION_SERVICE);
        Notification.Builder notiBldr = new Notification.Builder(mContext)
                .setContentTitle("Miracast")
                .setContentText(
                        "Session: " + width + " X " + height + " Secure: "
                                + secure).setUsesChronometer(true);
        Bitmap notiIcon = null, fwIcon = null;
        if (secure == 1) {
            notiBldr.setSmallIcon(android.R.drawable.ic_secure);
            try {
                fwIcon = BitmapFactory.decodeResource(Resources.getSystem(),
                        android.R.drawable.ic_lock_lock);
                notiIcon = Bitmap.createScaledBitmap(fwIcon, iconWidth,
                        iconHeight, false);
            } catch (Exception e) {
                // Ignore
            }
        } else {
            notiBldr.setSmallIcon(android.R.drawable.ic_menu_view);
            try {
                fwIcon = BitmapFactory.decodeResource(Resources.getSystem(),
                        android.R.drawable.ic_menu_compass);
                notiIcon = Bitmap.createScaledBitmap(fwIcon, iconWidth,
                        iconHeight, false);
            } catch (Exception e) {
                // Ignore
            }
        }
        if (fwIcon != null) {
            fwIcon.recycle();
            fwIcon = null;
        }
        if (notiIcon != null) {
            notiBldr.setLargeIcon(notiIcon);
        }
        Notification noti = notiBldr.build();
        if (mNotiMgr != null) {
            mNotiMgr.notify(NOTI_MSG_ID, noti);
        }
    }

    /**
     * INVALID => INITIALIZED
     */
    @Override
    public int initSys(IWfdActionListener listener, WfdDevice thisDevice) {
        if (Binder.getCallingUid() != Process.SYSTEM_UID) {
           Log.e(TAG,"Can't call initSys from non system-server context");
           return ErrorType.UNKNOWN.getCode();
        }
        mSysInvoked = true;
        return init(listener,thisDevice);
    }

    /**
     * INVALID => INITIALIZED
     */
    @Override
    public int init(IWfdActionListener listener, WfdDevice thisDevice) {
        Log.d(TAG, "init()");
        synchronized (StaticLock) {

           if(true == sIsHDMI_WFD_ConcurrencyAllowed){
              if (Binder.getCallingUid() != Process.SYSTEM_UID) {
                  Log.e(TAG, "Not allowing HDMIConnection for non System user");
                  mIsHDMIConnectionAllowed = false;
              } else {
                  Log.e(TAG, "System user UID is allowed HDMI Connection");
                  mIsHDMIConnectionAllowed = true;
              }
           }

            if (!mIsHDMIConnectionAllowed && mIsHDMIConnected &&
                 DeviceType == WFDDeviceType.SOURCE) {
                return ErrorType.HDMI_CABLE_CONNECTED.getCode();
            }
            if (listener != null) {
                ActionListeners.register(listener);
            }
            if (State == SessionState.INVALID) {
                if (thisDevice == null) {
                    Log.e(TAG, "WfdDevice arg can not be null");
                    return ErrorType.INVALID_ARG.getCode();
                }

                if (mSysInvoked) {
                    Log.d(TAG, "WFD invoked from sytem settings");
                } else {
                    Log.d(TAG, "WFD invoked in non-system context");
                }

                mConnectedDevice = null;
                boolean ret = WFDNative.enableWfd(thisDevice);
                updateState(SessionState.INITIALIZED, -1);
                SessionId = 1;// Move to a +ve session ID in order to handle
                // cases of teardown from transient SM states like
                // INITIALIZED, IDLE, ESTABLISHED
                Intent serviceIntent = new Intent(mContext, WfdService.class);
                if (!mServiceStarted) {
                    // Start WFD service to be foreground service
                    mContext.startService(serviceIntent);
                    mServiceStarted = true;
                    Log.d(TAG, "WfdService has started");
                }
                // Broadcast the WIFI_DISPLAY_ENABLED intent for legacy purposes
                // as well as secure application apps that take a decision based
                // on this whether to display secure content or not
                Intent intent = new Intent(WfdEnums.ACTION_WIFI_DISPLAY_ENABLED);
                Log.d(TAG, "Broadcasting WFD intent: " + intent);
                mContext.sendBroadcast(intent, broadcastPermission);
                setupWLANParams(true);
                return ret ? 0 : -1;
            } else if (State == SessionState.INITIALIZED){
                return ErrorType.ALREADY_INITIALIZED.getCode();
            } else {
                return ErrorType.INCORRECT_STATE_FOR_OPERATION.getCode();
            }
        }
    }

    @Override
    public int deinit() {
        Log.d(TAG, "deinit()");
        synchronized (StaticLock) {
            boolean ret = false;
            if (State != SessionState.INVALID) {
                broadcastWifiDisplayAudioIntent(false);
                broadcastWifiDisplayVideoEnabled(false);
                WFDNative.teardown(SessionId, false);
                ret = WFDNative.disableWfd();
                updateState(SessionState.INVALID, -1);
                mConnectedDevice = null;
                Intent serviceIntent = new Intent(mContext, WfdService.class);
                if (mServiceStarted) {
                    mContext.stopService(serviceIntent);
                    mServiceStarted = false;
                    Log.d(TAG, "WfdService has stopped");
                }
                // Broadcast the WIFI_DISPLAY_DISABLED intent for legacy
                // purposes as well as secure application apps that take a
                // decision based on this whether to display secure content or
                // not
                Intent intent = new Intent(
                        WfdEnums.ACTION_WIFI_DISPLAY_DISABLED);
                Log.d(TAG, "Broadcasting WFD intent: " + intent);
                mContext.sendBroadcast(intent, broadcastPermission);
                setupWLANParams(false);
            }
            return ret ? 0 : -1;
        }
    }

    @Override
    public int startWfdSession(WfdDevice device) {
        Log.d(TAG, "startWfdSession()");
        synchronized (StaticLock) {
            if (!mIsHDMIConnectionAllowed && mIsHDMIConnected &&
                DeviceType == WFDDeviceType.SOURCE) {
                return ErrorType.HDMI_CABLE_CONNECTED.getCode();
            } else if (State == SessionState.INVALID) {
                return ErrorType.NOT_INITIALIZED.getCode();
            } else if (State != SessionState.INITIALIZED) {
                return ErrorType.SESSION_IN_PROGRESS.getCode();
            } else if (device == null) {
                Log.e(TAG, "Peer device is null");
                return ErrorType.INVALID_ARG.getCode();
            }
            WFDNative.startWfdSession(device);
            //The session ID maintained by SM need not be changed since it
            //does not reflect any valid session ID per se
            updateState(SessionState.IDLE, SessionId);

            mConnectedDevice = device;
            mDebug = SystemProperties.getBoolean(sDebugKey, false);
        }
        return 0;
    }

    @Override
    public int stopWfdSession() {
        Log.d(TAG, "stopWfdSession()");
        return teardown();
    }

    @Override
    public int startUibcSession() {
        Log.d(TAG, "startUibcSession()");
        synchronized (StaticLock) {
            if (!mIsHDMIConnectionAllowed && mIsHDMIConnected &&
                DeviceType == WFDDeviceType.SOURCE) {
                return ErrorType.HDMI_CABLE_CONNECTED.getCode();
            }
            if (State == SessionState.INVALID) {
                return ErrorType.NOT_INITIALIZED.getCode();
            }
            if (uibcEnabled) {
                return ErrorType.UIBC_ALREADY_ENABLED.getCode();
            }
             if (UIBCMgr == null) {
                try {
                    UIBCMgr = new UIBCManager(DeviceType.getCode());
                } catch (Exception e) {
                    Log.e(TAG, "Error creating UIBC manager", e);
                    return ErrorType.UNKNOWN.getCode();
                }
            }
            if (DeviceType == WFDDeviceType.SOURCE) {
                setSurfaceProp(-1,-1,-1);
            }
            UIBCMgr.enable(SessionId);
            return 0;
        }
    }

    @Override
    public int stopUibcSession() {
        Log.d(TAG, "stopUibcSession");
        synchronized (StaticLock) {
            if (UIBCMgr != null && uibcEnabled) {
                UIBCMgr.disable(SessionId);
                uibcEnabled = false;
                return 0;
            } else {
                return ErrorType.UIBC_NOT_ENABLED.getCode();
            }
        }
    }

    @Override
    public int setUIBC() {
        synchronized (StaticLock) {
            Log.e(TAG, "setUibc");
            if (WFDNative.setUIBC(SessionId)) {
                return 0;
            } else {
                Log.e(TAG,"Unable to set UIBC parameters");
                return ErrorType.UNKNOWN.getCode();
            }
        }
    }

    @Override
    public boolean getUIBCStatus() {
        return uibcEnabled;
    }

    @Override
    public int setDeviceType(int type) {
        Log.d(TAG, "setDeviceType(): type - " + type);
        synchronized (StaticLock) {
            if (State == SessionState.INVALID || State == SessionState.INITIALIZED) {
                for (WFDDeviceType each : WFDDeviceType.values()) {
                    if (each.getCode() == type) {
                        DeviceType = each;
                        return 0;
                    }
                }
                return ErrorType.INVALID_ARG.getCode();
            } else {
                Log.e(TAG, "Session state is not INVALID or INITIALIZED");
                return ErrorType.INCORRECT_STATE_FOR_OPERATION.getCode();
            }
        }
    }

    @Override
    public int play() {
        Log.d(TAG, "play()");
        if (DeviceType != WFDDeviceType.SOURCE && sSurface == null) {
            Log.d(TAG, "Not calling play when surface is null");
            return 0;
        }
        return internalPlay(false);
    }

    private int internalPlay(boolean secureFlag) {
        Log.d(TAG, "internalPlay(): secureFlag - " + secureFlag);
        synchronized (StaticLock) {
            if (!mIsHDMIConnectionAllowed && mIsHDMIConnected &&
                DeviceType == WFDDeviceType.SOURCE) {
                return ErrorType.HDMI_CABLE_CONNECTED.getCode();
            }
            if (State == SessionState.PLAYING) {
                Log.d(TAG, "Already in the middle of PLAYING");
                return 0;
            }
            if (State == SessionState.ESTABLISHED || State == SessionState.PAUSE ||
                State == SessionState.STANDBY) {
                WFDNative.play(SessionId, secureFlag);
                State = SessionState.PLAYING;
                return 0;
            } else {
                Log.e(TAG, "Session state is not ESTABLISHED or PAUSE or STANDBY");
                return ErrorType.INCORRECT_STATE_FOR_OPERATION.getCode();
            }
        }
    }

    @Override
    public int pause() {
        Log.d(TAG, "pause()");
        return internalPause(false);
    }

    private int internalPause(boolean secureFlag) {
        Log.d(TAG, "internalPause(): secureFlag - " + secureFlag);
        synchronized (StaticLock) {
            if (!mIsHDMIConnectionAllowed && mIsHDMIConnected &&
                DeviceType == WFDDeviceType.SOURCE) {
                return ErrorType.HDMI_CABLE_CONNECTED.getCode();
            }
            if (State == SessionState.PAUSING) {
                Log.d(TAG, "Already in the middle of PAUSING");
                return 0;
            }
            if (State == SessionState.PLAY) {
                WFDNative.pause(SessionId, secureFlag);
                State = SessionState.PAUSING;
                return 0;
            } else {
                Log.e(TAG, "Session state is not PLAY");
                return ErrorType.INCORRECT_STATE_FOR_OPERATION.getCode();
            }
        }
    }

    @Override
    public int teardown() {
        Log.d(TAG, "teardown()");
        synchronized (StaticLock) {
            //Intent broadcast should always happen first.
            //DO NOT add anything before this line, unless
            //you know what you are doing.
            Log.d(TAG, "Teardown session, broadcast intents first");
            broadcastWifiDisplayAudioIntent(false);
            broadcastWifiDisplayVideoEnabled(false);
            if (State == SessionState.TEARDOWN || State == SessionState.TEARING_DOWN) {
                Log.d(TAG, "Already in the middle of teardown. State: "
                        + State);
                return 0;
            }

            if (State == SessionState.INVALID || State == SessionState.INITIALIZED) {
                Log.d(TAG, "No session in progress");
                return 0;
            }

            mRequestProxyRouting = false;
            // Now since we are going to proceed with teardown, release the
            // Virtual Display
            releaseVDS();
            if (State == SessionState.PLAY || State == SessionState.PLAYING
                    || State == SessionState.PAUSE || State == SessionState.PAUSING
                    || State == SessionState.STANDBY || State == SessionState.STANDING_BY
                    || State == SessionState.ESTABLISHED) {
                Log.d(TAG, "Perform triggered TEARDOWN in " + State + " state");
                WFDNative.teardown(SessionId, true);
                State = SessionState.TEARING_DOWN;
                return 0;
            } else {
                Log.e(TAG, "Session state is neither PLAY nor PAUSE");
                Log.d(TAG, "Perform local TEARDOWN without RTSP");
                WFDNative.teardown(SessionId, false);
                State = SessionState.TEARING_DOWN;
                return 0;
            }
        }
    }

    @Override
    public int standby() {
        Log.d(TAG, "standby()");
        synchronized (StaticLock) {
            if (!mIsHDMIConnectionAllowed && mIsHDMIConnected &&
                DeviceType == WFDDeviceType.SOURCE) {
                return ErrorType.HDMI_CABLE_CONNECTED.getCode();
            }
            if (State == SessionState.STANDING_BY) {
                Log.d(TAG, "Already in the middle of STANDING_BY");
                return 0;
            }
            if (State == SessionState.PAUSE || State == SessionState.PLAY) {
                if (WFDNative.standby(SessionId)) {
                    State = SessionState.STANDING_BY;
                } else {
                    Log.e(TAG, "Calling standby failed.");
                    return ErrorType.UNKNOWN.getCode();
                }
                return 0;
            } else {
                Log.e(TAG, "Session state is not PAUSE or PLAY");
                return ErrorType.INCORRECT_STATE_FOR_OPERATION.getCode();
            }
        }
    }

    @Override
    public int getCommonCapabilities(Bundle capabilities) {
        Log.d(TAG, "getCommonCapabilities()");
        if (!mIsHDMIConnectionAllowed && mIsHDMIConnected &&
            DeviceType == WFDDeviceType.SOURCE) {
            return ErrorType.HDMI_CABLE_CONNECTED.getCode();
        }
        // TODO Auto-generated method stub
        return 0;
    }

    @Override
    public int setNegotiatedCapabilities(Bundle capabilities) {
        Log.d(TAG, "setNegotiatedCapabilities()");
        if (!mIsHDMIConnectionAllowed && mIsHDMIConnected &&
            DeviceType == WFDDeviceType.SOURCE) {
            return ErrorType.HDMI_CABLE_CONNECTED.getCode();
        }
        // TODO Auto-generated method stub
        return 0;
    }

    @Override
    public int getSupportedTypes(int[] types) {
        Log.d(TAG, "getSupportedTypes()");
        WFDDeviceType[] deviceTypes = {
                WFDDeviceType.SOURCE
        }; // Only Source is supported
        types = new int[deviceTypes.length];
        for (int i = 0; i < deviceTypes.length; i++) {
            types[i] = deviceTypes[i].getCode();
        }
        return 0;
    }

    @Override
    public int getConfigItems(Bundle configItems) {
        synchronized (StaticLock) {
            Log.d(TAG, "getConfigItems()");
            WFDNative.getConfigItems(mConfigItems);
            configItems.putIntArray(WfdEnums.CONFIG_BUNDLE_KEY, mConfigItems);
            return 0;
        }
    }

    @Override
    public int uibcRotate(int angle) {
        synchronized (StaticLock) {
            Log.d(TAG, "uibcRotate");
            if(DeviceType == WFDDeviceType.SOURCE || DeviceType == WFDDeviceType.SOURCE_PRIMARY_SINK) {
                Log.e(TAG,"Source device can't send UIBC rotation event");
                return ErrorType.NOT_SINK_DEVICE.getCode();
            }
            if(angle!=0 && angle!=90 && angle!=180 && angle!=270) {
                //Right now WFD source supports only 0,90,180,270 degrees rotations
                Log.e(TAG, "Unsupported angle value!");
                return ErrorType.INVALID_ARG.getCode();
            }
            if (!uibcEnabled) {
                Log.e(TAG, "UIBC not enabled");
                return ErrorType.INCORRECT_STATE_FOR_OPERATION.getCode();
            }
            if (WFDNative.sendUIBCRotateEvent(angle)) {
                return 0;
            } else {
                return ErrorType.UNKNOWN.getCode();
            }
        }
    }

    private void broadcastWifiDisplayAudioIntent(boolean flag) {
        Log.d(TAG, "broadcastWifiDisplayAudioIntent() - flag: " + flag);
        if(DeviceType != WFDDeviceType.SOURCE) {
            Log.e(TAG, "Abort broadcast as device type is not source");
            return;
        }
        if (PLAYBACK_MODE == AVPlaybackMode.VIDEO_ONLY) {
            Log.d(TAG, "Playback mode is VIDEO_ONLY, ignore audio intent broadcast");
            return;
        }
        if (flag) {
            if (mRequestProxyRouting) {
                Log.e(TAG, "Calling Audio System Proxy enable");
                AudioSystem.setDeviceConnectionState(
                        AudioSystem.DEVICE_OUT_PROXY,
                        AudioSystem.DEVICE_STATE_AVAILABLE, "");
            }
        } else {
            Log.e(TAG, "Calling Audio System Proxy disable");
            AudioSystem.setDeviceConnectionState(AudioSystem.DEVICE_OUT_PROXY,
                            AudioSystem.DEVICE_STATE_UNAVAILABLE,"");
        }

    }
    private void broadcastWifiDisplayVideoEnabled(boolean flag) {
        Log.d(TAG, "broadcastWifiDisplayVideoEnabled() - flag: " + flag);
        if(DeviceType != WFDDeviceType.SOURCE) {
            Log.d(TAG, "Abort broadcast as device type is not source");
            return;
        }
        if (PLAYBACK_MODE == AVPlaybackMode.AUDIO_ONLY) {
            Log.d(TAG, "Playback mode is AUDIO_ONLY, ignore video intent broadcast");
            return;
        }
        Intent intent = new Intent(WfdEnums.ACTION_WIFI_DISPLAY_VIDEO);
        if (flag) {
            intent.putExtra("state", 1);
            int extra = uibcEnabled?mUIBCrotation:-1;
            intent.putExtra("wfd_UIBC_rot",extra);
        } else {
            intent.putExtra("state", 0);
            intent.putExtra("wfd_UIBC_rot", -1);
        }
        Log.d(TAG, "Broadcasting WFD video intent: " + intent);
        mContext.sendBroadcast(intent);
    }

    private void broadcastUIBCRotation(int value) {
        if(DeviceType != WFDDeviceType.SOURCE) {
            Log.d(TAG, "Abort broadcast as device type is not source");
            return;
        }
        if(State != SessionState.PLAY) {
          Log.e(TAG, "Aborting UIBC rotation in invalid WFD state");
          return;
        }
        Log.d(TAG, "broadcastUIBCRotation called with value " + value);
        Intent intent = new Intent(WfdEnums.ACTION_WIFI_DISPLAY_VIDEO);
        //WFD has to be in PLAY state to support UIBC rotation
        //hence the state should be 1
        intent.putExtra("state", 1);
        intent.putExtra("wfd_UIBC_rot",value);
        mContext.sendBroadcast(intent);
    }

    @Override
    public WfdStatus getStatus() {
        Log.d(TAG, "getStatus()");
        synchronized (StaticLock) {
            WfdStatus ret = new WfdStatus();
            ret.sessionId = SessionId;
            ret.state = State.ordinal();
            ret.connectedDevice = mConnectedDevice;
            return ret;
        }
    }

    @Override
    public int unregisterListener(IWfdActionListener listener) {
        Log.d(TAG, "unregisterListener: " + listener);
        synchronized (StaticLock) {
            if (listener != null) {
                return ActionListeners.unregister(listener) ? 0 : ErrorType.UNKNOWN.getCode();
            } else {
                Log.e(TAG, "Listener to unregister is null");
                return ErrorType.INVALID_ARG.getCode();
            }
        }
    }

    public int setResolution(int formatType, int value) {
        Log.d(TAG, "setResolution()");
        synchronized (StaticLock) {
            int ret = 0;
            if(formatType!= WfdEnums.CapabilityType.WFD_CEA_RESOLUTIONS_BITMAP.ordinal() &&
               formatType!= WfdEnums.CapabilityType.WFD_VESA_RESOLUTIONS_BITMAP.ordinal() &&
               formatType!= WfdEnums.CapabilityType.WFD_HH_RESOLUTIONS_BITMAP.ordinal()) {
               Log.e(TAG, "Invalid format type for resolution change");
               return ErrorType.INVALID_ARG.getCode();
            }
            CapabilityType resolutionType = CapabilityType.values()[formatType];
            switch (resolutionType) {
                case WFD_CEA_RESOLUTIONS_BITMAP:
                    if (!WfdEnums.isCeaResolution(value)) {
                        Log.e(TAG,"Invalid resolution type for resolution change");
                        return ErrorType.INVALID_ARG.getCode();
                    }
                    break;
                case WFD_HH_RESOLUTIONS_BITMAP:
                    if (!WfdEnums.isHhResolution(value)) {
                        Log.e(TAG,"Invalid resolution type for resolution change");
                        return ErrorType.INVALID_ARG.getCode();
                    }
                    break;
                case WFD_VESA_RESOLUTIONS_BITMAP:
                    if (!WfdEnums.isVesaResolution(value)) {
                        Log.e(TAG, "Invalid resolution type for resolution change");
                        return ErrorType.INVALID_ARG.getCode();
                    }
                    break;
                default:
                    return ErrorType.INVALID_ARG.getCode();
            }
            SessionState beforeResChangeState = State;
            if (beforeResChangeState == SessionState.PLAY) {
                broadcastWifiDisplayAudioIntent(false);
                ret = internalPause(true);
                if (State != SessionState.PAUSE) {
                    try {
                        StaticLock.wait(OPERATION_TIMEOUT);
                    } catch (InterruptedException e) {
                      Log.e(TAG, "Wait for PAUSE interrupted", e);
                      return ErrorType.UNKNOWN.getCode();
                    }
                }
                Log.d(TAG, "internalPause ret = " + ret + ", continuing setResolution");
            }
            int[] resParams = WfdEnums.getResParams();

            if (!WFDNative.setResolution(formatType, value, resParams)) {
                Log.e(TAG, "Setting new resolution failed!");
            }
            if (beforeResChangeState == SessionState.PLAY) {
                ret = internalPlay(true);
                Log.d(TAG, "internalPlay ret = " + ret);
            }
            return 0;
        }
    }

    @Override
    public int setBitrate(int bitrate) {
        Log.d(TAG, "setBitrate()");
        WFDNative.setBitrate(bitrate);
        return 0;
    }

    @Override
    public int queryTCPTransportSupport() {
        Log.d(TAG, "queryTCPTransportSupport");
        WFDNative.queryTCPTransportSupport();
        return 0;
    }

    @Override
    public int setRtpTransport(int transportType, int bufferLengthMs, int port) {
        synchronized(StaticLock) {
            boolean valid = false;
            for (RtpTransportType e: RtpTransportType.values()) {
                if (transportType == e.ordinal()) {
                    valid = true;
                    break;
                }
            }
            if (!valid) {
                Log.e(TAG, "Invalid transport type:" + transportType);
                return ErrorType.INVALID_ARG.getCode();
            }

            int ret = 0;

            /* For tcp, pause before negotiation*/
            SessionState beforeState = State;
            if (beforeState == SessionState.PLAY && mCurrTransport == RtpTransportType.TCP.ordinal()) {
                broadcastWifiDisplayAudioIntent(false);
                ret = internalPause(true);
                Log.d(TAG, "internalPause ret = " + ret + ", continuing setRtpTransport");
                if (State != SessionState.PAUSE) {
                    try {
                          StaticLock.wait(OPERATION_TIMEOUT);
                    } catch (InterruptedException e) {
                          Log.e(TAG, "Wait for PAUSE interrupted", e);
                          return ErrorType.UNKNOWN.getCode();
                    }
                }
                if (State != SessionState.PAUSE ) {
                           Log.e(TAG, "Session state is INVALID ");
                           return ErrorType.INCORRECT_STATE_FOR_OPERATION.getCode();
                }
            }

            IS_RTP_TRANSPORT_NEGOTIATED = false;
            WFDNative.negotiateRtpTransport(transportType, bufferLengthMs, port);

            while(!IS_RTP_TRANSPORT_NEGOTIATED && (State == SessionState.PAUSE ||
                      ( State == SessionState.PLAY &&
                        mCurrTransport == RtpTransportType.UDP.ordinal()))) {
                  try {
                        Log.d(TAG, "Wait for RTSP negotiation for new RTP transport");
                        StaticLock.wait(5);
                  } catch (InterruptedException e) {
                        Log.e(TAG, "setRtpTransport interrupted", e);
                        return ErrorType.UNKNOWN.getCode();
                  }
            }
            if (!IS_RTP_TRANSPORT_NEGOTIATED) {
                return ErrorType.UNKNOWN.getCode();
            }
            IS_RTP_TRANSPORT_NEGOTIATED = false;

            if (beforeState == SessionState.PLAY && mCurrTransport == RtpTransportType.UDP.ordinal()) {
                broadcastWifiDisplayAudioIntent(false);
                ret = internalPause(true);
                Log.d(TAG, "internalPause ret = " + ret + ", continuing setRtpTransport");
                if (State != SessionState.PAUSE) {
                    try {
                          StaticLock.wait(OPERATION_TIMEOUT);
                    } catch (InterruptedException e) {
                         Log.e(TAG, "Wait for PAUSE interrupted", e);
                         return ErrorType.UNKNOWN.getCode();
                    }
                }
                if (State != SessionState.PAUSE ) {
                       Log.e(TAG, "Session state is INVALID ");
                       return ErrorType.INCORRECT_STATE_FOR_OPERATION.getCode();
                }
            }

            WFDNative.setRtpTransport(transportType);

            if (beforeState == SessionState.PLAY) {
                ret = internalPlay(true);
                Log.d(TAG, "internalPlay ret = " + ret);
            }

            mCurrTransport = transportType;
            return 0;
        }
    }

    @Override
    public int tcpPlaybackControl(int cmdType, int cmdVal) {
        synchronized(StaticLock) {
            boolean valid = false;
            for (ControlCmdType e: ControlCmdType.values()) {
                if (cmdType == e.ordinal()) {
                    valid = true;
                    break;
                }
            }
            if (!valid) {
                Log.e(TAG, "Invalid control cmd type:" + cmdType);
                return ErrorType.INVALID_ARG.getCode();
            }
            int ret = 0;
            SessionState beforeState = State;
            if(cmdType == WfdEnums.ControlCmdType.FLUSH.ordinal()) {
                /* For flush, pause before negotiation*/
                if (beforeState == SessionState.PLAY) {
                    broadcastWifiDisplayAudioIntent(false);
                    ret = internalPause(true);
                    Log.d(TAG, "internalPause ret = " + ret + ", continuing flush");
                    if (State != SessionState.PAUSE) {
                        try {
                              StaticLock.wait(OPERATION_TIMEOUT);
                        } catch (InterruptedException e) {
                              Log.e(TAG, "Wait for PAUSE interrupted", e);
                              return ErrorType.UNKNOWN.getCode();
                        }
                    }
                    if (State != SessionState.PAUSE ) {
                           Log.e(TAG, "Session state is INVALID ");
                           return ErrorType.INCORRECT_STATE_FOR_OPERATION.getCode();
                    }
                }
            }

            IS_PLAYBACK_CONTROL_COMPLETE = false;
            WFDNative.tcpPlaybackControl(cmdType,cmdVal);

            if(cmdType == WfdEnums.ControlCmdType.FLUSH.ordinal()) {
                while(!IS_PLAYBACK_CONTROL_COMPLETE && State == SessionState.PAUSE) {
                    try {
                        Log.d(TAG, "Wait for RTSP negotiation for new RTP transport");
                        StaticLock.wait(5);
                    } catch (InterruptedException e) {
                        Log.e(TAG, "setRtpTransport interrupted", e);
                        return ErrorType.UNKNOWN.getCode();
                    }
                }
                if (!IS_PLAYBACK_CONTROL_COMPLETE) {
                    return ErrorType.UNKNOWN.getCode();
                }
                IS_PLAYBACK_CONTROL_COMPLETE = false;

                if (beforeState == SessionState.PLAY) {
                    ret = internalPlay(true);
                    Log.d(TAG, "internalPlay ret = " + ret);
                }
            }
            return 0;
        }
    }

    @Override
    public int setDecoderLatency(int latency) {
            WFDNative.setDecoderLatency(latency);
            Log.d(TAG, "setDecoderLatency done: ");
            return 0;
    }

    @Override
    public int setAvPlaybackMode(int mode) {
        Log.d(TAG, "setAvPlaybackMode mode: " + mode);
        synchronized (StaticLock) {
            if (State == SessionState.INVALID || State == SessionState.INITIALIZED) {
                boolean valid = false;
                AVPlaybackMode m = AVPlaybackMode.AUDIO_VIDEO;
                for (AVPlaybackMode e : AVPlaybackMode.values()) {
                    if (mode == e.ordinal()) {
                        m = e;
                        valid = true;
                        break;
                    }
                }
                if (!valid) {
                    Log.e(TAG, "Invalid AV playback mode:" + mode);
                    return ErrorType.INVALID_ARG.getCode();
                }
                if (WFDNative.setAvPlaybackMode(mode)) {
                    PLAYBACK_MODE = m;
                    return 0;
                } else {
                    return ErrorType.UNKNOWN.getCode();
                }
            } else {
                Log.e(TAG, "Session state is not INVALID or INITIALIZED");
                return ErrorType.INCORRECT_STATE_FOR_OPERATION.getCode();
            }
        }
    }

    @Override
    public int setSurface(Surface surface) {
        synchronized(StaticLock) {
            if (DeviceType == WFDDeviceType.SOURCE) {
                Log.d(TAG, "Device type is source. Ignoring setSurface");
                return ErrorType.NOT_SINK_DEVICE.getCode();
            }
            if(sSurface != null) {
               sSurface.release();
               if(sSurface.isValid()) {
                 Log.e(TAG, "Something really bad happened!");
               }
            }
            sSurface = surface;
            WFDNative.setVideoSurface(surface);
            return 0;
        }
    }

    @Override
    public int sendEvent(InputEvent event) {
       if(uibcEnabled == true )
       {
        synchronized(StaticLock) {
            if (DeviceType == WFDDeviceType.SOURCE) {
                Log.d(TAG, "Device type is source. Ignoring sendEvent");
                return ErrorType.NOT_SINK_DEVICE.getCode();
            }
        }
        if (event instanceof KeyEvent ||event instanceof MotionEvent) {
            UIBCMgr.addUIBCEvent(event);
        } else {
            return ErrorType.INVALID_ARG.getCode();
        }
       } else {
          Log.d(TAG, "UIBC connection is not established yet. Ignoring sendEvents");
       }
       return 0;
    }
    public int setSurfaceProp(int width,int height,int orientation) {
        synchronized(StaticLock) {
            if (DeviceType != WFDDeviceType.SOURCE) {
                WFDNative.setSurfaceProp(width, height, orientation);
            } else {
                Display display = ((WindowManager) mContext
                        .getSystemService(mContext.WINDOW_SERVICE))
                        .getDefaultDisplay();
                Point size = new Point();
                display.getRealSize(size);
                Configuration config = mContext.getResources()
                        .getConfiguration();
                Log.e(TAG, "X = " + size.x + " Y = " + size.y+ " Orientation = " + config.orientation);
                WFDNative.setSurfaceProp(size.x, size.y, config.orientation);
            }
            return 0;
       }
    }

    @Override
    public int getNegotiatedResolution(Bundle negRes) {
        synchronized (StaticLock) {
            Log.d(TAG, "getNegotiatedResolution()");
            if (State == SessionState.INVALID
                    || State == SessionState.INITIALIZED
                    || State == SessionState.IDLE
                    || State == SessionState.ESTABLISHED) {
                return ErrorType.INCORRECT_STATE_FOR_OPERATION.getCode();
            }
            int[] result = new int[1];
            int[] ret = WFDNative.getNegotiatedRes(result);
            if (result[0] == -1) {
                Log.e(TAG, "Failed to retrieve negotiated resolution");
                return ErrorType.UNKNOWN.getCode();
            }

            // Sanity Check
            if (ret == null || ret.length != 4) {
                Log.e(TAG, "Something is terribly wrong!");
                return ErrorType.UNKNOWN.getCode();
            }
            negRes.putIntArray("negRes", ret);
        }
        return 0;
    }

    @Override
    public int getCommonResolution(Bundle comRes) {
        synchronized (StaticLock) {
            Log.d(TAG, "getCommonResolution()");
            if (State == SessionState.INVALID
                    || State == SessionState.INITIALIZED
                    || State == SessionState.IDLE
                    || State == SessionState.ESTABLISHED) {
                return ErrorType.INCORRECT_STATE_FOR_OPERATION.getCode();
            }
            int[] result = new int[1];
            int[] ret = WFDNative.getCommonRes(result);
            if (result[0] == -1) {
                Log.e(TAG, "Failed to retrieve negotiated resolution");
                return ErrorType.UNKNOWN.getCode();
            }
            // The array will be containing values for a particular H264 profile
            // at [0],[1],[2] and [3] i.e. the array size should always be a
            // multiple of 4. Hence the number of profiles can be directly
            // calculated using the array length

            // Sanity Check
            if (ret == null || ret.length % 4 != 0) {
                Log.e(TAG, "Something is terribly wrong!");
                return ErrorType.UNKNOWN.getCode();
            }
            comRes.putInt("numProf", ret.length / 4);
            comRes.putIntArray("comRes", ret);
        }
        return 0;
    }

    @Override
    public int enableDynamicBitrateAdaptation(boolean flag) {
        synchronized (StaticLock) {
            if (DeviceType != WFDDeviceType.SOURCE) {
                Log.e(TAG, "Operation not permitted for a sink device");
                return ErrorType.NOT_SOURCE_DEVICE.getCode();
            }
            if (State == SessionState.INVALID
                    || State == SessionState.INITIALIZED
                    || State == SessionState.IDLE
                    || State == SessionState.ESTABLISHED) {
                return ErrorType.INCORRECT_STATE_FOR_OPERATION.getCode();
            }
            Log.d(TAG, "enableDynamicBitrateAdaptation() called with " + flag);
            if (flag) {
                if (!WFDNative
                        .executeRuntimeCommand(RuntimecmdType.ENABLE_BITRATE_ADAPT
                        .getCmdType())) {
                    return ErrorType.UNKNOWN.getCode();
                }
            } else {
                if (!WFDNative
                        .executeRuntimeCommand(RuntimecmdType.DISABLE_BITRATE_ADAPT
                                .getCmdType())) {
                    return ErrorType.UNKNOWN.getCode();
                }
            }
        }
        return 0;
    }

    /**
     * This is a helper method to release the instance of Virtual Display
     * created
     */
    private void releaseVDS() {
        if (mVirtualDisplay != null) {
            Log.e(TAG, "Releasing Virtual Display");
            mVirtualDisplay.release();
            mVirtualDisplay = null;
        }
    }

    @Override
    public int registerHIDEventListener(IHIDEventListener listener) {
        synchronized (StaticLock) {
            Log.d(TAG, "registerHIDEventListener()");
            if (listener != null) {
                return HIDEventListeners.register(listener) ? 0
                        : ErrorType.UNKNOWN.getCode();
            }
            Log.e(TAG, "HIDEventListener cannot be null!");
            return ErrorType.INVALID_ARG.getCode();
        }
    }

    @Override
    public int unregisterHIDEventListener(IHIDEventListener listener) {
        synchronized (StaticLock) {
            Log.d(TAG, "unregisterHIDEventListener()");
            if (listener != null) {
                return HIDEventListeners.unregister(listener) ? 0
                        : ErrorType.UNKNOWN.getCode();
            }
            Log.e(TAG, "HIDEventListener cannot be null!");
            return ErrorType.INVALID_ARG.getCode();
        }
    }

    @Override
    public void HIDReportDescRecvd(byte[] hidRepDesc) {
        synchronized (StaticLock) {
            if (hidRepDesc != null) {
                Log.e(TAG,
                        "HID Report Descriptor Received: "
                                + Arrays.toString(hidRepDesc));
            }
            if (HIDEventListeners != null) {
                Log.d(TAG, "Sending HIDReportDescRecvd() to listeners");
                final int N = HIDEventListeners.beginBroadcast();
                for (int i = 0; i < N; i++) {
                    try {
                        HIDEventListeners.getBroadcastItem(i)
                                .onHIDReprtDescRcv(hidRepDesc);
                    } catch (RemoteException e) {
                        Log.e(TAG,
                                "Error sending HID Report descriptor to client");
                    }
                }
                HIDEventListeners.finishBroadcast();
            } else {
                Log.d(TAG, "HIDReportDescRecvd Remote callback list is null");
            }
        }
    };

    @Override
    public void HIDReportRecvd(byte[] hidRep) {
        synchronized (StaticLock) {
            if (hidRep != null) {
                Log.e(TAG,
                        "HID Report Received: "
                                + Arrays.toString(hidRep));
            }
            if (HIDEventListeners != null) {
                Log.d(TAG, "Sending HIDReportRecvd() to listeners");
                final int N = HIDEventListeners.beginBroadcast();
                for (int i = 0; i < N; i++) {
                    try {
                        HIDEventListeners.getBroadcastItem(i).onHIDReprtRcv(
                                hidRep);
                    } catch (RemoteException e) {
                        Log.e(TAG, "Error sending HID Report to client");
                    }
                }
                HIDEventListeners.finishBroadcast();
            } else {
                Log.d(TAG, "HIDReportRecvd Remote callback list is null");
            }
        }
    };

     /**
      * This function is used to set default values to all variables that
      * might have been altered during the session but might not have got
      * a chance to clear up due to abrupt teardown. This will enable the
      * session to start in a clean slate state
      */
    public void clearVars() {
        UIBCMgr = null;
        uibcEnabled = false;
        mUIBCrotation = -1;
        mConnectedDevice = null;
        mSysInvoked = false;
        releaseVDS();
        mRequestProxyRouting = true;
        if (mNotiMgr != null) {
            mNotiMgr.cancel(NOTI_MSG_ID);
            mNotiMgr = null;
        }
        mDebug = false;
    }

    /**
     * @description This method is used to change the default scan interval to
     *              the P2P framework so that when WFD is operating over the P2P
     *              channel, the frequency of framework initiated scans can be
     *              reduced which would otherwise impact WFD throughput
     *
     * @param set
     *            This parameter controls whether the value needs to be set or
     *            reset
     */
    private void setupWLANParams(boolean set) {
        long period;
        if (Settings.Global.putLong(mContext.getContentResolver(),
                Settings.Global.WIFI_SCAN_INTERVAL_WHEN_P2P_CONNECTED_MS,
                set ? wfdScanInterval : defScanInterval)) {
            // A sanity check to see if the value is actually set
            period = Settings.Global
                    .getLong(
                            mContext.getContentResolver(),
                            Settings.Global.WIFI_SCAN_INTERVAL_WHEN_P2P_CONNECTED_MS,
                            0);
            Log.d(TAG, "WIFI_SCAN_INTERVAL_WHEN_P2P_CONNECTED_MS set to "
                    + period + " ms");
        } else {
            Log.d(TAG,
                    "Failed to set WIFI_SCAN_INTERVAL_WHEN_P2P_CONNECTED_MS value!");
        }
    }
}
