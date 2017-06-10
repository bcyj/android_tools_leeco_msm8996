/*
 * ©2012-2014 Borqs Software Solutions Pvt Ltd. All rights reserved.
 */

package com.borqs.videocall;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Calendar;
import java.util.Timer;
import java.util.TimerTask;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;

import android.app.AlertDialog;
import android.app.Application;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.ContentResolver;
import android.content.ContentValues;
import android.content.ContentUris;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.content.pm.PackageManager.NameNotFoundException;
import android.content.res.Configuration;
import android.database.Cursor;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.os.SystemProperties;
import android.media.AudioManager;
import android.media.MediaRecorder;
import android.media.MediaScanner;
import android.media.ToneGenerator;
import android.net.Uri;
import android.os.Environment;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.os.PowerManager;
import android.os.SystemClock;
import android.os.Vibrator;
import android.text.format.DateFormat;
import android.text.format.DateUtils;
import android.provider.CallLog;
import android.app.NotificationManager;
import android.provider.ContactsContract.PhoneLookup;
//import android.provider.Checkin;
import android.provider.Contacts;
import android.provider.ContactsContract;
import android.provider.Settings;
import android.provider.CallLog.Calls;
import android.util.Config;
import android.util.Log;
import android.text.TextUtils;
import android.view.KeyEvent;
import android.view.View;
import android.view.Surface;
import android.widget.FrameLayout;
import android.widget.ImageButton;
import android.widget.LinearLayout;
import android.widget.TextView;
import android.widget.Toast;
import android.provider.MediaStore;
import com.android.internal.telephony.CallerInfo;
import com.android.internal.telephony.CallerInfoAsyncQuery;
import com.android.internal.telephony.Connection;
import com.android.internal.telephony.PhoneConstants;
//import com.android.internal.telephony.Connection.DisconnectCause;
import android.telecom.PhoneAccountHandle;
import android.telephony.DisconnectCause;
import android.telephony.PhoneNumberUtils;
import android.view.IWindowManager;
import android.os.ServiceManager;
import com.borqs.videocall.VTManager.VideoSource;
import com.borqs.videocall.RemoteServiceConnector;
import android.bluetooth.BluetoothHeadset;

import android.bluetooth.BluetoothClass;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothHeadset;
import android.bluetooth.BluetoothProfile;
import android.bluetooth.BluetoothUuid;
import android.os.ParcelUuid;
import java.util.List;
import android.graphics.SurfaceTexture;
import android.os.AsyncTask;
import java.nio.ByteBuffer;
import java.nio.IntBuffer;

public class VideoCallApp extends Application {

    static final String TAG = "VT/VideoCallApp";
    static VideoCallApp sMe = null;
    public static final String VIDEO_CALL_SERVICE = "videophone";

    public static final String INTENT_ACTION_LAUNCH_VIDEOCALLSCREEN = "com.borqs.videocall.action.LaunchVideoCallScreen";
    public static final String TMP_INTENT_ACTION_LAUNCH_VIDEOCALLSCREEN = "art.videocall.action.LaunchVideoCallScreen";

    public static final String INTENT_EXTRA_CALL_OR_ANSWER = "IsCallOrAnswer";
    public static final String INTENT_EXTRA_LAUNCH_MODE = "LaunchMode";
    public static final String INTENT_EXTRA_PHONE_URL = "call_number_key";
    public static final String INTENT_EXTRA_IS_LOCKED_MODE = "IsLockedMode";

    public static final String INTENT_ACTION_STOP_VTCALL = "com.borqs.videocall.action.StopVTCall";
    public static final String INTENT_ACTION_ANSWER_VTCALL = "com.borqs.videocall.action.answerCall";
    public static final String INTENT_ACTION_STARTRING_VTCALL = "com.borqs.videocall.action.startring";
    public static final String INTENT_ACTION_SILENCERING_VTCALL = "org.codeaurora.PowerKeyDown.Mute";
    public static final String INTENT_ACTION_CLEAR_MISSED_VTCALL =
            "com.borqs.videocall.action.clearMissedVTCall";
    public static final int CALL_FROM_BEGIN = 0;
    public static final int CALL_FROM_IP = 0;
    public static final int CALL_FROM_TTY = 1;
    public static final int CALL_FROM_LOOPBACK = 2;
    public static final int CALL_FROM_END = 3;

    public static final int APP_STATUS_IDLE = 0;
    public static final int APP_STATUS_FDN_CHECKING = 1;
    public static final int APP_STATUS_RINGING = 2;
    public static final int APP_STATUS_CONNECTED = 3;

    public static final int ENGINE_STATE_NOT_CONNECT = 0;
    public static final int ENGINE_STATE_CONNECTING = 1;
    public static final int ENGINE_STATE_CONNECTED = 2;

    static final int MSG_BASE = 5000;
    static final int MSG_FALLBACK_RESULT = MSG_BASE + 2;
    static final int MSG_FALLBACK_OK = MSG_BASE + 3;
    static final int MSG_FALLBACK_CANCEL = MSG_BASE + 4;
    static final int MSG_END_CALL = MSG_BASE + 5;
    static final int MSG_ANSWER_CALL = MSG_BASE + 6;
    static final int MSG_MUTE = MSG_BASE + 7;
    static final int MSG_HEADSET_SETHOOK = MSG_BASE + 8;
    static final int MSG_HEADSET_SETHOOK_LONGPRESS = MSG_BASE + 9;
    static final int MSG_LOCAL_SURFACE_CREATED = MSG_BASE + 10;
    static final int MSG_LOCAL_SURFACE_DESTROYED = MSG_BASE + 11;
    static final int MSG_REMOTE_SURFACE_CREATED = MSG_BASE + 12;
    static final int MSG_REMOTE_SURFACE_DESTROYED = MSG_BASE + 13;
    static final int MSG_BLUETOOTH_PLUG = MSG_BASE + 14;
    static final int MSG_WIRED_HEADSET_PLUG = MSG_BASE + 15;
    static final int MSG_FLIP_CHANGE = MSG_BASE + 16;
    static final int MSG_SILENCERING_VTCALL = MSG_BASE + 17;

    static final int MSG_FDN_ALLOWED = MSG_BASE + 20;
    static final int MSG_FDN_FORBIDDEN = MSG_BASE + 21;
    static final int MSG_ACTION_MEDIA_UNMOUNTED = MSG_BASE + 26;
    static final int MSG_CLEAR_MISSED_VTCALL = MSG_BASE + 27;

    static final int MSG_STOP_RECORD = MSG_BASE + 30;

    private static final int EVENT_WAKE_LOCK_TIMEOUT = MSG_BASE + 35;
    private static final int MSG_NOTIFITY_INCALL = MSG_BASE + 40;
    private static final int WAKE_LOCK_TIMEOUT = 5000;

    static final String EXTRA_FALLBACK_MESSAGE = "FallBackMessage";

    // User Input Related
    static final String MSG_CLOSE_CAMERA = "close_:camera_";
    static final String MSG_OPEN_CAMERA = "open_:camera_";
    static final String MSG_SWITCH_TO_MAIN_CAMERA = "<swi_main>";
    static final String MSG_SWITCH_TO_SECONDARY_CAMERA = "<swi_sec>";
    static final String MSG_RECORD_AUDIO = "<rec_aud>";
    static final String MSG_STOP_RECORD_AUDIO = "<stop_rec_aud>";

    private static final int MAX_MSG_LEN = 20; // 20 chars as default msg string
                                               // length
    // capture picture
    private static final String MSG_CAPTURE_PICTURE = "<cap_pic>";
    private static final String MSG_MULTI_RINGTONE_START = "CC";
    private static final String MSG_MULTI_RINGTONE_END = "CD";
    private static final String MSG_MULTI_RINGTONE_END_OLD = "DC";

    private StringBuffer mMsgBuf = new StringBuffer(MAX_MSG_LEN);
    private String mCallerName = null; // it can be null!
    StringBuffer mUserInputBuffer = new StringBuffer(); // it can be null!

    private Bitmap mCallerPic = null;
    private VTServiceInterface mVTServiceInterface = null;

    // ScreenBrightness setting
    int mScreenBrightness;

    // variables for call log
    private int mCallLogType = CallLog.Calls.MISSED_TYPE;
    private int mCallLogCSVTType = VTCallUtils.MISSED_CSVT_TYPE;
    private int mMissedCause = DisconnectCause.INCOMING_MISSED;
    public static final int VT_CALLLOG_CALLTYPE = 1;
    private long mStartTime = SystemClock.uptimeMillis();
    int mDuration = -1;
    String mCallTimeElapsed = "00:00";
    String mRecordTimeElapsed = "00:00";
    int fallbackFlag = 0;

    class QueryTimeStamp {
        long mTimeStamp = -1;
    };

    enum AUDIO_RECORD_STATE {
        IDLE, STARTING, STARTED
    }

    public AUDIO_RECORD_STATE mAudioRecordStatus = AUDIO_RECORD_STATE.IDLE;
    QueryTimeStamp mStartQueryTimeStamp = new QueryTimeStamp();
    QueryTimeStamp mCompleteQueryTimeStamp = new QueryTimeStamp();
    // private boolean mIsReset;

    VTCallUtils mUtils = null;
    AlertDialog flipAlertDialog = null;

    public static final String STR_CALLURL_PREFIX[] = { "socket://", // CALL_FROM_IP
            "modem://", // CALL_FROM_TTY
            "localloopback", // loopback mode for debug only
    };

    // Important control object instances
    // vt call screen
    VideoCallScreen mVideoCallScreen = null;
    // vt service
    VTService mVTService = null;
    boolean mIsStartingService = false;
    // incoming call connection
    IVTConnection mConnection = null;
    // dial out call proxy
    VTCallProxy mCallClient = null;

    CameraSetting mCameraSetting = null;

    // Sound recording related
    MediaRecorder mSoundRecorder;
    private String mTempRecAudFullPath = null;
    private String mRecAudFullPath = null;

    static String AUDIO_RECORDER_PATH = "/sdcard/Audios/VideoCall";
    static String recorderPrefix = "VideoCall_";

    boolean mIsHardwareH324MStack = false;

    boolean mIsScreenOn = false;
    int mNotificationVol = 0;
    boolean mIsMusicPaused = false;

    // wake lock
    PowerManager.WakeLock mWakeLock;

    /*****************************************
     * Application state variables----Start
     *****************************************/
    private int mAppStatus = APP_STATUS_IDLE;
    private Context mCtx;

    private Context phoneCtx;

    /* Package */boolean mIsServerBound = false;
    /* Package */boolean mIsMMRingtoneStart = false;
    /* Package */int mEngineState = ENGINE_STATE_NOT_CONNECT;
    // remember if the remote surface has been set.
    // It's important to set remote surface before connecting VTService
    /* Package */boolean mHasRemoteSurfaceSet = false;
    // To indicate if we are launching callscreen. The behavior, launching
    // callscrenn,
    // is taken only when the VT channel is created, BUT the callscreen is go
    // background at the same time.
    // It's an async way to launch a activity, so we need below var to avoid
    // repeated launch.
    // if it's launching, just wait here
    /* Package */boolean mIsLaunchingCallScreen = false;

    /* Package */boolean mIsAnswering = false;

    private boolean mIsLocalSurfaceOpenPending = false;
    private boolean mIsRemoteSurfaceOpenPending = false;
    private SurfaceTexture mLocalSurface = null;
    private SurfaceTexture mRemoteSurface = null;

    // mute state
    /* Package */boolean mIsMute = false;
    /* Package */boolean mIsLandscape = false;

    // Var to protect multiple fallback message notification which is possible
    // by wrong modem state
    boolean mIsPhoneConnected = false; // indicate if phone has been connected
    boolean mIsVTConnected = false;

    private boolean mIsCallLogAdded = false;

    // camera related
    /* Package */int mCurrentVideoSource = VTManager.VideoSource.CAMERA_SECONDARY;
    /* Package */int mLastVideoSource = VTManager.VideoSource.CAMERA_NONE;
    String mStrReplaceImagePath = null;

    // UI state related
    /* Package */public static int sPanelMode = VTPanel.VTPANEL_MODE_DAILOUT_CONNECTING;
    /* Package */static int sPendingPanelMode = VTPanel.VTPANEL_MODE_DAILOUT_CONNECTING;
    /* Package */boolean mIsSmsRfsChecked = false;
    static boolean isLockPanel = false;

    private int mFallbackSetting = 0;
    public boolean mOpenFlipToAnswer = false;
    // call related
    public String mInputPhoneURL;
    int mLaunchMode = CALL_FROM_TTY;
    boolean mDialOrAnswer = true;
    String mDeviceName;

    static final int VTQUERY_STATE_IDLE = 0;
    static final int VTQUERY_STATE_RUNNING = 1;

    /* locks for synchronize */
    private Object mQueryCompleteGuard = new Object();
    private Object mVTInfoQueryStateGuard = new Object();
    private boolean mIsQueryCompleted = false;
    private boolean mAddCallLogPending = false;
    private int mVTInfoQueryState = VTQUERY_STATE_IDLE;;

    private Ringer mRinger = null;
    private CallerInfo mCallerInfo = null;

    // true for dial, false for answer
    private boolean mIsDialOrAnswer;

    private static final String PHONE_UI_EVENT_MULTIPLE_QUERY = "multiple incoming call queries attempted";

    // values used to track the query state
    private static final int VTINFO_QUERY_READY = 0;
    private static final int VTINFO_QUERYING = -1;

    private Timer mScheduleTimer = null;
    AudioManager mAudioManager = null;
    private InCallTonePlayer mInCallRingbackTonePlayer = null;

    public boolean bBtHeadsetPlugged = false;
    private BluetoothAdapter adapter;
    private byte[] imgbuf = null;
    private SendtoVTServiceThread t = null;

    /********************************************
     * Application state variables----End
     *********************************************/
    private static Runnable endAppRunnable = new Runnable() {
        public void run() {
            VTCallUtils.exitApplication();
        }
    };

    void tryEndApplication() {
        /*
         * mHandler.sendMessage( Message.obtain( mHandler,
         * VideoCallApp.MSG_DESTROY_APP));
         */

        synchronized (mQueryCompleteGuard) {
            while (mVTInfoQueryState == VTQUERY_STATE_RUNNING)
                try {
                    Thread.sleep(100);
                } catch (Exception e) {
                    if (MyLog.DEBUG)
                        MyLog.d(TAG, e.getMessage());
                }
        }
        ;

        if (MyLog.DEBUG)
            MyLog.d(TAG, "To post exit application runnable");
        mHandler.postDelayed(endAppRunnable, 20);
        return;
    }

    // /////////////////////////////////////////////////

    void ResetAll() {

        if (MyLog.DEBUG)
            MyLog.d(TAG, "reset all...");
        mIsSmsRfsChecked = false;
        // sPanelMode = sPendingPanelMode =
        // VTPanel.VTPANEL_MODE_DAILOUT_CONNECTING;

        mIsAnswering = false;

        mIsMMRingtoneStart = false;
        mEngineState = ENGINE_STATE_NOT_CONNECT;
        mIsMute = false;

        mHasRemoteSurfaceSet = false;
        mIsLaunchingCallScreen = false;
        mIsLocalSurfaceOpenPending = false;
        mIsRemoteSurfaceOpenPending = false;
        mIsCallLogAdded = false;
        mAudioRecordStatus = AUDIO_RECORD_STATE.IDLE;
        Context ctx = this.getApplicationContext();
        if (MyLog.DEBUG)
            Log.e(TAG, "Calling switchSpeaker - false");
        switchSpeaker(false);

        playInCallRingbackTone(false);

        mAudioManager = (AudioManager) mCtx.getSystemService(Context.AUDIO_SERVICE);
        // mAudioManager.setStreamVolume(AudioManager.STREAM_NOTIFICATION,mNotificationVol,
        // 0);

        /* make the Bt sco channel off as part of reset */
        if (mAudioManager != null)
            switchBtSco(false);
        // bBtHeadsetPlugged = false;

        VTServiceInterface.setAudioMode(ctx, AudioManager.MODE_NORMAL);

        if (mIsMusicPaused) {
            Intent i = new Intent("com.android.music.musicservicecommand");

            i.putExtra("command", "togglepause");
            mCtx.sendBroadcast(i);
            mIsMusicPaused = false;
        }
        // b444: move mAudioManager.setParameters() to VTService, to make sure
        // it called after VT engine is disconnected.
        /*
         * if(mAudioManager!=null) {
         * mAudioManager.setParameters("videotelephony=off"); }
         */
        VTNotificationManager nm = VTNotificationManager.getDefault();
        if (nm == null) {
            VTNotificationManager.init(this);
            nm = VTNotificationManager.getDefault();
        }
        if (MyLog.DEBUG)
            MyLog.d(TAG, "set notification alerts to true...");
        nm.getStatusBarMgr().enableNotificationAlerts(true);

        if (mAppStatus >= APP_STATUS_RINGING)
            unregisterVTAppReceiver();

        mAppStatus = APP_STATUS_IDLE;
        setRepImageSelectable(true);

        synchronized (mQueryCompleteGuard) {
            mStartQueryTimeStamp.mTimeStamp = 1;
            mCompleteQueryTimeStamp.mTimeStamp = -1;
            mUserInputBuffer.delete(0, mUserInputBuffer.length());
            mCallerName = null;
            mCallerInfo = null;
        }

        // Timer
        if (MyLog.DEBUG)
            MyLog.d(TAG, "Cancel Timer!, in onDestroy");
        mCallTime.cancelTimer();

        // Release service, which should be finished before CALL clean up
        releaseVTService();

        // call clear
        if (mDialOrAnswer == false) { // if it's a answer
            if (mConnection != null) {
                mConnection.clear();
                mConnection = null;
            }
            mDialOrAnswer = true;
        } else {
            if (mCallClient != null)
                mCallClient.endCall();
        }

        if (mWakeLock != null) {
            if (mWakeLock.isHeld()) {
                mWakeLock.release();
            }
        }

        // Start call detail intent
        if (mIsVTConnected && !mIsMMRingtoneStart) {
            String str = getCallDetailString();
            if (str != null) {
                Toast.makeText(VideoCallApp.this, str, 5000).show();
            }
        }
        mIsVTConnected = false;
        mIsPhoneConnected = false;
        mCallTimeElapsed = "00:00";
    }

    int GetFlipState() {
        /*
         * int flipstatus1 = -1; int flipstatus2 = -1; try { IWindowManager mWm
         * =
         * IWindowManager.Stub.asInterface(ServiceManager.getService("window"));
         * flipstatus1 = mWm.getSwitchState(8); flipstatus2 =
         * mWm.getSwitchState(9); } catch (Exception ex){ if (MyLog.DEBUG)
         * Log.e(TAG,"[getSwitchState]: " + ex.toString()); } if (MyLog.DEBUG)
         * MyLog.v(TAG,"flipstatus1="+flipstatus1); if (MyLog.DEBUG)
         * MyLog.v(TAG,"flipstatus2="+flipstatus2);
         *
         * // 0:flip open, 1: flip close, 2: rotate flip open, 3: rotate flip
         * close if(flipstatus1 == 0 && flipstatus2 == 1) { return 0; }
         * if(flipstatus1 == 0 && flipstatus2 == 0) { return 1; } if(flipstatus1
         * == 1 && flipstatus2 == 0) { return 2; } if(flipstatus1 == 1 &&
         * flipstatus2 == 1) { return 3; }
         */
        return 0;
    }

    void releaseVTService() {

        if (MyLog.DEBUG)
            MyLog.d(TAG, "release VT Service. ");

        // TODO: following action might be redundant because we should make sure
        // service is disconnected
        // before the telephony signal channel is torn down.
        if (mEngineState == ENGINE_STATE_CONNECTED || mEngineState == ENGINE_STATE_CONNECTING) {
            mVTService.disconnect();
            mEngineState = ENGINE_STATE_NOT_CONNECT;
        }

        if (MyLog.DEBUG)
            MyLog.d(TAG, "stopVTService..., is bound=" + mIsServerBound);
        // if (mIsServerBound) {
        try {
            unbindService(mConn);
            VTService.stopService(this);
        } catch (Exception e) {
            if (MyLog.DEBUG)
                MyLog.d(TAG, "meet exception while unbindService," + e.getMessage());
        }
        mIsServerBound = false;
        mIsStartingService = false;
        mVTService = null;
        // }
    }

    void openCloseCamera() {
        // NOTE, camera device must be opened!!! before call this function.
        if (MyLog.DEBUG)
            MyLog.d(TAG, "update Camera Source: current video source" + mCurrentVideoSource);
        if (mVTService == null) {
            if (MyLog.DEBUG)
                MyLog.d(TAG,
                        "vtservice still null Or local surface not inited yet, can't update camera source, just return");
            return;
        }

        if (mCurrentVideoSource == VTManager.VideoSource.CAMERA_NONE) {
            if (MyLog.DEBUG)
                MyLog.d(TAG, "update camera source, open, now current source:" + mLastVideoSource);
            try {
                int currentSource = mLastVideoSource;
                mLastVideoSource = VTManager.VideoSource.CAMERA_NONE;
                mCurrentVideoSource = currentSource;
                if (MyLog.DEBUG)
                    MyLog.d(TAG, "status" + t.getStatus());
                mVTService.sendUserInput(MSG_OPEN_CAMERA);
                if (mStrReplaceImagePath == null) {
                    // mVTService.setLocalDisplay(this.mLocalSurface);
                    String orientationMode = mIsLandscape ? CameraSetting.UIOrientationMode_Landscape
                            : CameraSetting.UIOrientationMode_Portrait;
                    mVTService.setVideoSource(currentSource, mStrReplaceImagePath, null);
                    getVTServiceInterface().setCameraParameter(CameraSetting.UIOrientationMode,
                            orientationMode, mVTService);
                    // mVTService.sendUserInput( MSG_OPEN_CAMERA);
                } else {
                    // borqs b089
                    // if currentsource is none, surface is destroyed, so just
                    // need to creat surface again,
                    // camera will be opened in surface created
                    mVTService.setVideoSource(currentSource, mStrReplaceImagePath, null);
                    // mVideoCallScreen.mScreenHandler.obtainMessage(VideoCallScreen.MSG_CLOSE_REP_IMAGE).sendToTarget();
                }

            } catch (Exception e) {
                if (MyLog.DEBUG)
                    Log.e(TAG, "Exception while open camera, " + e.getMessage());
            }
        } else {
            mStrReplaceImagePath = VTImageReplaceSetting
                    .getCurrentReplaceImagePath(VideoCallApp.this);
            if (MyLog.DEBUG)
                MyLog.d(TAG, "update camera source, close. replace image path: "
                        + mStrReplaceImagePath);
            try {
                // mVTService.setVideoSource(VTManager.VideoSource.CAMERA_NONE,
                // mStrReplaceImagePath);
                imgbuf = sendRepImagebuf();
                mLastVideoSource = mCurrentVideoSource;
                mCurrentVideoSource = VTManager.VideoSource.CAMERA_NONE;
                if (mStrReplaceImagePath == null) {
                    mVTService.sendUserInput(MSG_CLOSE_CAMERA);
                    mVTService.setVideoSource(VTManager.VideoSource.CAMERA_NONE,
                            mStrReplaceImagePath, null);
                    // mVTService.setLocalDisplay(null);
                    imgbuf = VideoCallManager.getInstance(getApplicationContext())
                            .getLastVideoFrames();
                    if (t.getStatus().equals(AsyncTask.Status.RUNNING)) {
                        t.cancel(true);
                    }
                    t = new SendtoVTServiceThread();
                    t.execute();
                } else {
                    mVTService.sendUserInput(MSG_CLOSE_CAMERA);
                    mVTService.setVideoSource(VTManager.VideoSource.CAMERA_NONE,
                            mStrReplaceImagePath, null);
                    if (t.getStatus().equals(AsyncTask.Status.RUNNING)) {
                        t.cancel(true);
                    }
                    // mVTService.setCamera(0);
                    t = new SendtoVTServiceThread();
                    t.execute();
                    mVideoCallScreen.mScreenHandler.obtainMessage(
                            VideoCallScreen.MSG_SHOW_REP_IMAGE).sendToTarget();
                }
            } catch (Exception e) {
                if (MyLog.DEBUG)
                    Log.e(TAG, "Exception while close and set replace camera, " + e.getMessage());
            }
        }

        if (mVideoCallScreen != null) {
            mVideoCallScreen.mScreenHandler.obtainMessage(VideoCallScreen.MSG_CAMERA_STATE_UPDATE)
                    .sendToTarget();
        }
        return;
    }

    byte[] sendRepImagebuf() {
        if (MyLog.DEBUG)
            MyLog.v(TAG, "showRepImage");
        Bitmap bm = null;
        byte[] buffer = null;
        int[] buf = new int[176 * 144 * 3 / 2];
        IntBuffer ib = null;
        try {
            int imagesize = 176 * 144 * 3 / 2;
            buffer = new byte[imagesize];
            FileInputStream input;

            if (MyLog.DEBUG)
                MyLog.d(TAG,
                        " Image path"
                                + getApplicationContext().getFileStreamPath(
                                        VTImageReplaceSetting.mStrFN1));
            File f = getApplicationContext().getFileStreamPath(VTImageReplaceSetting.mStrFN1);
            if (!f.exists()) {
                if (MyLog.DEBUG)
                    MyLog.d(TAG, "VTImg.row does not exist, create new");
                VTImageReplaceSetting.setDefaultImage(getApplicationContext());
            }
            input = this.openFileInput(VTImageReplaceSetting.mStrFN1);

            int ns = input.read(buffer);
            if (MyLog.DEBUG)
                MyLog.v(TAG, "Read bytes " + ns);
            if (ns != imagesize) {
                if (MyLog.DEBUG)
                    MyLog.d(TAG, "exit checkItemImage because the wrong file size.");
                return null;
            }
            return buffer;
        } catch (FileNotFoundException e) {
            if (MyLog.DEBUG)
                Log.e(TAG, "File not found, " + e.getMessage());
        } catch (IOException eio) {
            if (MyLog.DEBUG)
                Log.e(TAG, "Failed to read, " + eio.getMessage());
        }
        return buffer;
    }

    void setRepImageSelectable(boolean selectable) {
        Intent intent = new Intent(VTImageReplaceSetting.SELECTABLE_ACTION);
        intent.putExtra("selectable", selectable);
        mCtx.sendBroadcast(intent);
    }

    public class SendtoVTServiceThread extends AsyncTask<Void, Void, Void> {

        protected Void doInBackground(Void... arg0) {
            while (!t.isCancelled()) {
                try {
                    if (mVTService != null) {
                        mVTService.setVideoFrame(VTManager.VideoSource.CAMERA_NONE, imgbuf);
                        Thread.sleep(60);
                        if (MyLog.DEBUG)
                            Log.e(TAG, "Thread running");
                    }
                } catch (InterruptedException ex) {
                }
            }
            if (t.isCancelled()) {
                // mVTService.setVideoSource(mCurrentVideoSource,
                // mStrReplaceImagePath);
                mVTService.setVideoFrame(mCurrentVideoSource, null);

            }
            return null;
        }
    }

    void switchSpeaker(boolean bEnable) {
        boolean isSpeakerOn = VTServiceInterface.isSpeakerOn(mCtx);
        if (MyLog.DEBUG)
            Log.e(TAG, "switchSpeaker, isSpeakerOn = " + isSpeakerOn + ", bEnable = " + bEnable);
        if (isSpeakerOn != bEnable) {
            VTServiceInterface.turnOnSpeaker(mCtx, bEnable);
            for (int i = 0; i < 10; i++) {
                SystemClock.sleep(100);
                isSpeakerOn = VTServiceInterface.isSpeakerOn(mCtx);
                Log.e(TAG, "after turnOnSpeaker, isSpeakerOn = " + isSpeakerOn + ", bEnable = "
                        + bEnable + "i =" + i);
                if (isSpeakerOn != bEnable) {
                    VTServiceInterface.turnOnSpeaker(mCtx, bEnable);
                } else {
                    break;
                }
            }

            if (MyLog.DEBUG)
                Log.e(TAG, "switchSpeaker after turn on Speaker, isSpeakerOn = " + isSpeakerOn
                        + ", bEnable = " + bEnable);
        }

        if (mVideoCallScreen != null) {
            mVideoCallScreen.mScreenHandler
                    .obtainMessage(VideoCallScreen.MSG_SPEAKER_STATUS_UPDATE).sendToTarget();
        }

    }

    public void switchBtSco(boolean bEnable) {
        boolean isBtScoOn = mAudioManager.isBluetoothScoOn();
        if (MyLog.DEBUG)
            Log.e(TAG, "switchBtSco, isBtScoOn = " + isBtScoOn + ", bEnable = " + bEnable);
        if (isBtScoOn != bEnable) {
            if (mLaunchMode == CALL_FROM_LOOPBACK) {
                mAudioManager.setMode(AudioManager.MODE_IN_COMMUNICATION);
                mAudioManager.setStreamVolume(AudioManager.STREAM_NOTIFICATION, 1, 0);

                if (bEnable) {
                    mAudioManager.startBluetoothScoVirtualCall();
                } else {
                    mAudioManager.stopBluetoothSco();
                }
            } else {
                //mAudioManager.setBluetoothScoOn(bEnable);
                if (bEnable) {
                    //mAudioManager.startBluetoothSco();
                    mAudioManager.startBluetoothScoVirtualCall();
                } else {
                    mAudioManager.stopBluetoothSco();
                }
            }
            // wait for audio manager to set Bluetooth sco on properly
            while (isBtScoOn != bEnable) {
                SystemClock.sleep(100);
                isBtScoOn = mAudioManager.isBluetoothScoOn();
            }
            if (mVideoCallScreen != null) {
                mVideoCallScreen.mScreenHandler.obtainMessage(
                        VideoCallScreen.MSG_SPEAKER_STATUS_UPDATE).sendToTarget();
            }

        }
    }

    // Need handler for callbacks to the UI thread
    Handler mHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            if (MyLog.DEBUG)
                MyLog.d(TAG, "HandleMessage : " + msg.what);
            if (VideoCallApp.this.mAppStatus == VideoCallApp.APP_STATUS_IDLE
                    && (VideoCallApp.this.mConnection == null)) {
                if (MyLog.DEBUG)
                    MyLog.d(TAG, "App still in idle state, handle message: " + msg.what);
                switch (msg.what) {
                case EVENT_WAKE_LOCK_TIMEOUT: {
                    if (MyLog.DEBUG)
                        MyLog.d(TAG, "wake lock timeout, release it");
                    if (mWakeLock != null && mWakeLock.isHeld()) {
                        mWakeLock.release();
                    }
                    break;
                }
                case MSG_CLEAR_MISSED_VTCALL:
                    if (MyLog.DEBUG)
                        MyLog.d(TAG, "handle MSG_CLEAR_MISSED_VTCALL");
                    VTNotificationManager nm = VTNotificationManager.getDefault();
                    if (nm == null) {
                        VTNotificationManager.init(VideoCallApp.this);
                        nm = VTNotificationManager.getDefault();
                    }
                    if (nm != null) {
                        VTNotificationManager.getDefault()
                            .closeMissedCallNotification((Boolean) msg.obj);
                    }
                    break;
                default: {

                    if (MyLog.DEBUG)
                        MyLog.d(TAG, "Error Scenario Exit");
                    // onVTReleased(true);
                    if (mVideoCallScreen != null) {
                        mVideoCallScreen.mScreenHandler.obtainMessage(
                                VideoCallScreen.MSG_SERVICE_DISCONNECTED).sendToTarget();
                    }
                    mVTService = null;
                    closeCallScreen();

                    break;
                }
                }
                return;
            }

            switch (msg.what) {
            case IVTConnection.VTCALL_RESULT_CONNECTED:
                if (MyLog.DEBUG)
                    MyLog.d(TAG, "Call connected.");

                if (isBtHeadsetPlugged() == true) {
                    switchSpeaker(!isBtHeadsetPlugged());
                    switchBtSco(isBtHeadsetPlugged());
                }

            case IVTConnection.VTCALL_STATE_ACTIVE:
                if (MyLog.DEBUG)
                    MyLog.d(TAG, "Call State Active.");
                doCallConnect();
                break;
            // Fallback related
            case IVTConnection.VTCALL_RESULT_FALLBACK_47:
                if (MyLog.DEBUG)
                    MyLog.d(TAG, "Call fallback #47.");
                doFallback(VideoCallApp.this.getText(R.string.call_result_failed_fallback_47));
                break;
            case IVTConnection.VTCALL_RESULT_FALLBACK_57:
                if (MyLog.DEBUG)
                    MyLog.d(TAG, "Call fallback #57.");
                doFallback(VideoCallApp.this.getText(R.string.call_result_failed_fallback_57));
                break;
            case IVTConnection.VTCALL_RESULT_FALLBACK_58:
                if (MyLog.DEBUG)
                    MyLog.d(TAG, "Call fallback #58.");
                doFallback(VideoCallApp.this.getText(R.string.call_result_failed_fallback_58));
                break;
            case IVTConnection.VTCALL_RESULT_FALLBACK_88:
                if (MyLog.DEBUG)
                    MyLog.d(TAG, "Call fallback #88.");
                doFallback(VideoCallApp.this.getText(R.string.call_result_failed_fallback_88));
                break;
            // disconnect related
            case IVTConnection.VTCALL_RESULT_DISCONNECTED:
                if (MyLog.DEBUG)
                    MyLog.d(TAG, "Call disconnected");
                doCallDisconnect(null);
                break;
            case IVTConnection.VTCALL_RESULT_DISCONNECTED_LOST_SIGNAL:
                if (MyLog.DEBUG)
                    MyLog.d(TAG, "Call disconnected lost signal");
                doCallDisconnect(VideoCallApp.this
                        .getText(R.string.call_result_disconnect_signal_lost));
                break;
            case IVTConnection.VTCALL_RESULT_DISCONNECTED_INCOMING_MISSED:
                if (MyLog.DEBUG)
                    MyLog.d(TAG, "incoming call missed");
                doCallDisconnect(null);
                break;
            case IVTConnection.VTCALL_RESULT_DISCONNECTED_NO_ANSWER:
                if (MyLog.DEBUG)
                    MyLog.d(TAG, "Call disconnected no answer");
                doCallDisconnect(VideoCallApp.this
                        .getText(R.string.call_result_disconnect_no_answer));
                break;
            case IVTConnection.VTCALL_RESULT_DISCONNECTED_BUSY:
                if (MyLog.DEBUG)
                    MyLog.d(TAG, "Call disconnected busy ");
                doCallDisconnect(VideoCallApp.this.getText(R.string.call_result_disconnect_busy));
                break;
            case IVTConnection.VTCALL_RESULT_DISCONNECTED_INVALID_NUMBER:
                if (MyLog.DEBUG)
                    MyLog.d(TAG, "Call disconnected invalid number");
                doCallDisconnect(VideoCallApp.this
                        .getText(R.string.call_result_disconnect_invalid_number));
                break;
            case IVTConnection.VTCALL_RESULT_DISCONNECTED_INCOMING_REJECTED:
                if (MyLog.DEBUG)
                    MyLog.d(TAG, "Call disconnected incoming reject");
                doCallDisconnect(VideoCallApp.this
                        .getText(R.string.call_result_disconnect_incoming_rejected));
                break;
            case IVTConnection.VTCALL_RESULT_DISCONNECTED_POWER_OFF:
                if (MyLog.DEBUG)
                    MyLog.d(TAG, "Call disconnected power off");
                doCallDisconnect(VideoCallApp.this
                        .getText(R.string.call_result_disconnect_power_off));
                break;
            case IVTConnection.VTCALL_RESULT_DISCONNECTED_OUT_OF_SERVICE:
                if (MyLog.DEBUG)
                    MyLog.d(TAG, "Call disconnected out of service");
                if (mIsPhoneConnected) {
                    // phone has been connected
                    doCallDisconnect(VideoCallApp.this.getText(R.string.out_of_3G_network));
                } else {
                    // before phone is connected
                    doCallDisconnect(VideoCallApp.this
                            .getText(R.string.call_result_disconnect_out_of_service));
                }
                break;
            case IVTConnection.VTCALL_RESULT_DISCONNECTED_UNASSIGNED_NUMBER:
                if (MyLog.DEBUG)
                    MyLog.d(TAG, "Call disconnected unassinged number");
                doCallDisconnect(VideoCallApp.this
                        .getText(R.string.call_result_disconnect_unassigned_number));
                break;
            case IVTConnection.VTCALL_RESULT_DISCONNECTED_NUMBER_CHANGED:
                if (MyLog.DEBUG)
                    MyLog.d(TAG, "Call disconnected number changed");
                doCallDisconnect(VideoCallApp.this
                        .getText(R.string.call_result_disconnect_number_changed));
                break;

            // new result info
            case IVTConnection.VTCALL_RESULT_NORMAL_UNSPECIFIED:
                doCallDisconnect(VideoCallApp.this.getText(R.string.call_failed_normal));
                break;
            case IVTConnection.VTCALL_RESULT_NETWORK_CONGESTION:
                doCallDisconnect(VideoCallApp.this.getText(R.string.call_failed_normal));
                break;
            case IVTConnection.VTCALL_RESULT_DISCONNECTED_LOCAL_OUT_OF_3G_SERVICE:
                doCallDisconnect(VideoCallApp.this.getText(R.string.local_out_of_3G_network));
                break;

            case IVTConnection.VTCALL_RESULT_PROTOCOL_ERROR_UNSPECIFIED:
                if (mIsPhoneConnected) {
                    // phone has been connected
                    doCallDisconnect(VideoCallApp.this.getText(R.string.out_of_3G_network));
                } else {
                    doCallDisconnect(VideoCallApp.this.getText(R.string.network_busy));
                }
                break;
            case IVTConnection.VTCALL_RESULT_NO_USER_RESPONDING:
                doCallDisconnect(VideoCallApp.this.getText(R.string.no_user_responding));
                break;
            case IVTConnection.VTCALL_RESULT_BEARER_NOT_SUPPORTED_65:
                doCallDisconnect(VideoCallApp.this.getText(R.string.bearer_not_supported));
                break;
            case IVTConnection.VTCALL_RESULT_BEARER_NOT_SUPPORTED_79:
                doCallDisconnect(VideoCallApp.this.getText(R.string.bearer_not_supported));
                break;

            // exception from VT telephony
            case IVTConnection.VTCALL_RESULT_CALL_EXCEPTION:
                doCallDisconnect(VideoCallApp.this.getText(R.string.call_exception_failed));
                break;
            case IVTConnection.VTCALL_RESULT_ANSWERCALL_EXCEPTION:
                Log.w(TAG, "Answer call exception, do nothing ...");
                // doCallDisconnect(VideoCallApp.this.getText(R.string.answer_call_exception_failed));
                break;
            case IVTConnection.MESSAGE_RINGBACK_TONE:
                if (MyLog.DEBUG)
                    MyLog.d(TAG, "Call ringback tone.");
                playInCallRingbackTone((Boolean) msg.obj);
                break;

            /* info notification from vtmanager */
            case VTManager.InfoType.USER_INPUT_INCOMMING:
                if (MyLog.DEBUG)
                    MyLog.d(TAG, "handle data_incomming infor: input: " + (String) (msg.obj));
                int arg1 = msg.arg1;
                String input = (String) (msg.obj);
                dealWithUserInput(input);
                break;
            case VTManager.InfoType.SIGNAL_WEAK:
                if (MyLog.DEBUG)
                    MyLog.d(TAG, "recived signal weak message.");
                Toast.makeText(VideoCallApp.this, getText(R.string.call_state_notify_signal_weak),
                        Toast.LENGTH_SHORT).show();
                break;

            case VTManager.InfoType.CAMERA_FRAME_START:
                if (MyLog.DEBUG)
                    MyLog.d(TAG, "recived Camera frame start.");

                if (mCurrentVideoSource != VTManager.VideoSource.CAMERA_NONE) {
                    if (mStrReplaceImagePath != null) {
                        mVideoCallScreen.mScreenHandler.obtainMessage(
                                VideoCallScreen.MSG_CLOSE_REP_IMAGE).sendToTarget();

                    }
                    if (t.getStatus().equals(AsyncTask.Status.RUNNING)) {
                        if (MyLog.DEBUG)
                            MyLog.d(TAG, "stop image ");
                        t.cancel(true);
                    }
                }

                break;
            /* error type from vtmanager */
            case VTManager.VTMANAGER_ERROR_CONNECTION_FAIL:
                if (MyLog.DEBUG)
                    MyLog.d(TAG, "handle connection fail.");
                doEndCall();
                Toast.makeText(VideoCallApp.this,
                        VideoCallApp.this.getText(R.string.call_result_connect_fail),
                        Toast.LENGTH_LONG).show();
                break;

            case MSG_NOTIFITY_INCALL:
                if (MyLog.DEBUG)
                    Log.d(TAG, "handle MSG_NOTIFITY_INCALL");
                VTNotificationManager nm = VTNotificationManager.getDefault();
                if (nm != null && mVTService != null) {
                    nm.notifyInVTCall(mVTService);
                }
                break;
            case MSG_END_CALL:
                if ((mAppStatus == APP_STATUS_RINGING) && (mIsAnswering == false)
                        && (mDialOrAnswer == false)) {
                    if (mVideoCallScreen != null) {
                        mVideoCallScreen.SendRejectSms();
                    }
                }
                doEndCall();
                break;
            case MSG_ANSWER_CALL:
                if ((mAppStatus == APP_STATUS_RINGING) && (mIsAnswering == false)
                        && (mDialOrAnswer == false))
                    mVideoCallScreen.onAccept();
                break;
            case MSG_SILENCERING_VTCALL:
                if ((mAppStatus == APP_STATUS_RINGING) && !mDialOrAnswer) {
                    stopRing();
                    // if(mVideoCallScreen != null &&
                    // mVideoCallScreen.mRingingMuteView!=null)
                    // {
                    // mVideoCallScreen.mRingingMuteView.setImageResource(R.drawable.cmcc_videocall_dialer_btn_icon_mute);
                    // }
                }
                break;
            case MSG_HEADSET_SETHOOK:
                if (mIsVTConnected == false) {
                    if ((mAppStatus == APP_STATUS_RINGING) && (mIsAnswering == false)
                            && (mDialOrAnswer == false)) {
                        if (MyLog.DEBUG)
                            MyLog.d(TAG, "connecting: long press head set accept");
                        answerCall();
                    }
                } else { // mute while the phone is connected
                    if (MyLog.DEBUG)
                        MyLog.d(TAG, "connected: headset mute");
                    if (mVideoCallScreen != null)
                        mVideoCallScreen.onMute();
                    else {
                        if (true == getVTServiceInterface().setMute(!isMute(), mVTService))
                            mIsMute = !isMute();
                    }
                }
                break;
            case MSG_HEADSET_SETHOOK_LONGPRESS:
                doEndCall();
                break;

            case MSG_WIRED_HEADSET_PLUG: {
                boolean bHeadsetPlugged = VTCallUtils.isHeadSetIn(msg.arg1);
                if (mIsHardwareH324MStack) {
                    if (!mAudioManager.isBluetoothScoOn()) {
                        if (MyLog.DEBUG)
                            MyLog.d(TAG, "headset plugged, to switch speaker: " + bHeadsetPlugged);
                        switchSpeaker(!bHeadsetPlugged);
                    }
                } else {

                    if (MyLog.DEBUG)
                        MyLog.d(TAG, "headset plugged, to switch speaker: " + bHeadsetPlugged);
                    switchSpeaker(!bHeadsetPlugged);
                }

                int flipstate = GetFlipState();
                if (mAppStatus != APP_STATUS_IDLE && (flipstate == 1 || flipstate == 2)) {
                    if (bHeadsetPlugged) {
                        if (flipAlertDialog != null) {
                            try {
                                flipAlertDialog.dismiss();
                            } catch (Exception ex) {
                            }
                            flipAlertDialog = null;
                        }
                        if (isMute()) {
                            if (mVideoCallScreen != null) {
                                mVideoCallScreen.onMute();
                            } else {
                                getVTServiceInterface().setMute(!isMute(), mVTService);
                                mIsMute = false;
                            }
                        }
                    } else {
                        if (flipAlertDialog == null || !flipAlertDialog.isShowing())
                            if (mVideoCallScreen != null) {
                                flipAlertDialog = VTCallUtils.Alerter.doAlert(mVideoCallScreen,
                                        (String) getText(R.string.incall_error_swivel_callin));
                            }
                        if (!isMute()) {
                            if (mVideoCallScreen != null) {
                                mVideoCallScreen.onMute();
                            } else {
                                getVTServiceInterface().setMute(!isMute(), mVTService);
                                mIsMute = true;
                            }
                        }
                    }
                }
            }
                break; // end of case bluetooth or header plug event
            case MSG_BLUETOOTH_PLUG:

                // Since the presence of a wired headset or bluetooth affects
                // the
                // speakerphone, update the "speaker" state. We ONLY want to do
                // this on the wired headset connect / disconnect events for now
                // though, so we're only triggering on EVENT_WIRED_HEADSET_PLUG.
                bBtHeadsetPlugged = VTCallUtils.isHeadSetIn(msg.arg1);
                if (mIsHardwareH324MStack) {
                    switchSpeaker(!isBtHeadsetPlugged());
                } else {
                    MyLog.d(TAG, " BT headset plugged, to switch speaker: " + isBtHeadsetPlugged());
                    if (!isBtHeadsetPlugged() && mAudioManager.isWiredHeadsetOn()) {
                        mHandler.sendMessage(mHandler.obtainMessage(MSG_WIRED_HEADSET_PLUG,1, 0));
                    } else {
                        switchSpeaker(!isBtHeadsetPlugged());
                        switchBtSco(isBtHeadsetPlugged());
                    }
                }
                break; // end of case bluetooth or header plug event
            case MSG_LOCAL_SURFACE_CREATED: {
                if (sPanelMode != VTPanel.VTPANEL_MODE_REMOTE_ONLY) {
                    // Work-around for new CameraHal, we have to set local
                    // surface visible when VideoCallScreen.onStop.
                    mLocalSurface = (SurfaceTexture) msg.obj;

                    if (mLocalSurface == null) {
                        if (MyLog.DEBUG)
                            MyLog.d(TAG, "mLocalSurface is null.");
                        return;
                    }

                    if (mVTService == null) {
                        if (MyLog.DEBUG)
                            MyLog.d(TAG,
                                    "VT server not bound yet, just set localsurface pending flag.");
                        mIsLocalSurfaceOpenPending = true;
                        return;
                    }

                    // this blanks the screen if the surface changed, no-op
                    // otherwise
                    getVTServiceInterface().setLocalDisplay(mLocalSurface, mVTService);

                    if (MyLog.DEBUG)
                        MyLog.d(TAG, " set camera device");
                    String orientationMode = mIsLandscape ? CameraSetting.UIOrientationMode_Landscape
                            : CameraSetting.UIOrientationMode_Portrait;
                    getVTServiceInterface().setCameraParameter(CameraSetting.UIOrientationMode,
                            orientationMode, mVTService);
                    mVTService.setVideoSource(mCurrentVideoSource, mStrReplaceImagePath, null);
                    if (mStrReplaceImagePath == null) {
                        if (VideoCallScreen.mChangeConfig) {
                            VideoCallScreen.mChangeConfig = false;
                            return;
                        } else {
                            if (mCurrentVideoSource != VTManager.VideoSource.CAMERA_NONE
                                    && !getVTServiceInterface().isUseVTCamera())
                                mVTService.sendUserInput(MSG_OPEN_CAMERA);
                        }
                    } else {
                        // borqs b089
                        // in case: repimagepath is not null and videosource is
                        // none means
                        // running in background with rep image showing
                        if (mCurrentVideoSource == VTManager.VideoSource.CAMERA_NONE)
                            mVideoCallScreen.mScreenHandler.obtainMessage(
                                    VideoCallScreen.MSG_SHOW_REP_IMAGE).sendToTarget();
                    }
                }
            }
                break; // end of case LOCAL_SURFACE_CREATED:

            case MSG_LOCAL_SURFACE_DESTROYED:
                mLocalSurface = null;
                if (mVTService != null) {
                    getVTServiceInterface().setLocalDisplay(null, mVTService);
                }
                mIsLocalSurfaceOpenPending = false;
                break; // end of case LOCAL_SURFACE_DESTROYED:

            case MSG_REMOTE_SURFACE_CREATED: {
                mRemoteSurface = (SurfaceTexture) msg.obj;
                if (mVTService == null) {
                    if (MyLog.DEBUG)
                        MyLog.d(TAG, "VT server not bound yet, just set localsurface pending flag.");
                    mIsRemoteSurfaceOpenPending = true;
                    return;
                }

                getVTServiceInterface().setRemoteDisplay(mRemoteSurface, mVTService);

                if (mHasRemoteSurfaceSet == false) {
                    // try to connectVTService
                    mHasRemoteSurfaceSet = true;
                    connectVTService();
                }
            }
                break; // end of case REMOTE_SURFACE_CREATED:

            case MSG_REMOTE_SURFACE_DESTROYED:
                mRemoteSurface = null;
                if (mVTService != null)
                    getVTServiceInterface().setRemoteDisplay(null, mVTService);
                mIsRemoteSurfaceOpenPending = false;
                break; // end of case REMOTE_SURFACE_DESTROYED
            case MSG_FDN_ALLOWED:
                if (MyLog.DEBUG)
                    MyLog.d(TAG, "receive EVENT_FDN_ALLOWED");
                mAppStatus = APP_STATUS_IDLE;
                // continue to dial out
                if (mVideoCallScreen != null) {
                    // to update the UI
                    mVideoCallScreen.mScreenHandler
                            .obtainMessage(VideoCallScreen.MSG_FDN_POST_INIT).sendToTarget();
                }/*
                  * else { //do nothing, ignore this dial-out }
                  */
                break;
            case MSG_FDN_FORBIDDEN:
                if (MyLog.DEBUG)
                    MyLog.d(TAG, "receive EVENT_FDN_FORBIDDEN");
                {
                    Intent i = new Intent(mVideoCallScreen, FlyingModeAlertDialog.class);
                    i.putExtra(FlyingModeAlertDialog.PROMPT_RES_ID, R.string.error_fdn_forbidden);
                    mVideoCallScreen.startActivity(i);
                }
                mAppStatus = APP_STATUS_IDLE;
                closeCallScreen();
                break;
            case MSG_STOP_RECORD: {
                VTCallUtils.showStorageToast(VTCallUtils.SDCARD_FULL, VideoCallApp.this);
                stopAudioRecord();
                if (mVideoCallScreen != null) {
                    mVideoCallScreen.mScreenHandler.obtainMessage(VideoCallScreen.MSG_STOP_RECORD)
                            .sendToTarget();
                }
                break;
            }
            case MSG_FLIP_CHANGE: {
                int flipstate = msg.arg1;
                if (MyLog.DEBUG)
                    Log.e(TAG, "MSG_FLIP_CHANGE:flipstate=" + flipstate);
                switch (flipstate) {
                case 0:// flip open
                {
                    if (mOpenFlipToAnswer && !mDialOrAnswer
                            && (getStatus() == VideoCallApp.APP_STATUS_RINGING) && !mIsAnswering) {
                        if (mVideoCallScreen != null)
                            mVideoCallScreen.onAccept();
                    }
                    // close flipAlertDialog
                    if (flipAlertDialog != null) {
                        try {
                            flipAlertDialog.dismiss();
                        } catch (Exception ex) {
                        }
                        flipAlertDialog = null;
                    }
                    if (isMute()) {
                        if (mVideoCallScreen != null) {
                            mVideoCallScreen.onMute();
                        } else {
                            getVTServiceInterface().setMute(!isMute(), mVTService);
                            mIsMute = false;
                        }
                    }
                    break;
                }
                case 1:// flip close
                       // MT
                    if (!mDialOrAnswer && (getStatus() == APP_STATUS_RINGING)) {
                        if (mVideoCallScreen != null) {
                            mVideoCallScreen.onRefuseClicked();
                        }
                    }
                    // MO
                    else if (mVideoCallScreen != null) {
                        mVideoCallScreen.onEndCall();
                    }
                    break;
                case 2:// rotate flip open
                case 3:// rotate flip close
                    if (getStatus() != APP_STATUS_IDLE) {
                        if ((mIsHardwareH324MStack && !VTCallUtils.checkHeadsetStatus() && !mAudioManager
                                .isBluetoothScoOn())
                                || (!mIsHardwareH324MStack && !VTCallUtils.checkHeadsetStatus())) {
                            if (MyLog.DEBUG)
                                MyLog.d(TAG, "MSG_FLIP_CHANGE!VTCallUtils.checkHeadsetStatus()");

                            if (flipAlertDialog == null || !flipAlertDialog.isShowing()) {
                                if (mVideoCallScreen != null) {
                                    flipAlertDialog = VTCallUtils.Alerter.doAlert(mVideoCallScreen,
                                            (String) getText(R.string.incall_error_into_swivel));
                                }
                            }
                            if (getStatus() == APP_STATUS_CONNECTED) {
                                if (!isMute()) {
                                    if (mVideoCallScreen != null) {
                                        mVideoCallScreen.onMute();
                                    } else {
                                        getVTServiceInterface().setMute(true, mVTService);
                                        mIsMute = true;
                                    }
                                }
                            }
                        }
                    }
                    break;
                default:
                    break;
                }
            }
            }
        }
    };

    /*
     * public void onConfigurationChanged(Configuration newConfig) {
     * super.onConfigurationChanged(newConfig); mIsLandscape =
     * (newConfig.orientation == Configuration.ORIENTATION_LANDSCAPE);
     * Log.e(TAG," into onConfigurationChanged landscape=" + mIsLandscape);
     * if(mVTService!= null && mCurrentVideoSource !=
     * VTManager.VideoSource.CAMERA_NONE && mEngineState ==
     * ENGINE_STATE_CONNECTED || mEngineState == ENGINE_STATE_CONNECTING) { if(
     * newConfig.orientation == Configuration.ORIENTATION_LANDSCAPE) { //
     * mVTService
     * .setCameraParameter(CameraSetting.UIOrientationMode,CameraSetting
     * .UIOrientationMode_Landscape);
     * mVTService.setVideoSource(VideoSource.LANDSCAPE, null); } else {
     * //mVTService
     * .setCameraParameter(CameraSetting.UIOrientationMode,CameraSetting
     * .UIOrientationMode_Portrait);
     * mVTService.setVideoSource(VideoSource.PORTRAIT, null); } return; }
     *
     * }
     */

    private final class HeadsetServiceListener implements BluetoothProfile.ServiceListener {

        public void onServiceConnected(int profile, BluetoothProfile proxy) {
            bBtHeadsetPlugged = false;
            BluetoothHeadset mService = (BluetoothHeadset) proxy;
            int[] states = new int[1];
            states[0] = BluetoothProfile.STATE_CONNECTED;

            List<BluetoothDevice> deviceList = mService.getDevicesMatchingConnectionStates(states);
            if (!deviceList.isEmpty()) {
                if (MyLog.DEBUG)
                    MyLog.d(TAG, "onServiceConnected  set bBtHeadsetPlugged to 1  ");
                bBtHeadsetPlugged = true;
                return;
            }
        }

        public void onServiceDisconnected(int profile) {
            bBtHeadsetPlugged = false;
        }
    }

    private ServiceConnection mConn = new ServiceConnection() {

        public void onServiceConnected(ComponentName className, IBinder service) {
            if (MyLog.DEBUG)
                MyLog.d(TAG, "onServiceConnected");
            mVTService = ((VTService.LocalBinder) service).getService();
            mVTService.setVTServiceListener(VideoCallApp.this.mVTServiceListener);

            mVTService.setHandler(mHandler);
            mIsServerBound = true;
            mIsStartingService = false;

            if (mVideoCallScreen != null) {
                mVideoCallScreen.mScreenHandler
                        .obtainMessage(VideoCallScreen.MSG_SERVICE_CONNECTED).sendToTarget();
            }

            // set pending surface before connecting vt engine
            if (mIsLocalSurfaceOpenPending) {
                getVTServiceInterface().setLocalDisplay(mLocalSurface, mVTService);
                String orientationMode = mIsLandscape ? CameraSetting.UIOrientationMode_Landscape
                        : CameraSetting.UIOrientationMode_Portrait;
                getVTServiceInterface().setCameraParameter(CameraSetting.UIOrientationMode,
                        orientationMode, mVTService);
                mVTService.setVideoSource(mCurrentVideoSource, mStrReplaceImagePath, null);
                mIsLocalSurfaceOpenPending = false;
            }
            if (mIsRemoteSurfaceOpenPending) {
                getVTServiceInterface().setRemoteDisplay(mRemoteSurface, mVTService);
                mIsRemoteSurfaceOpenPending = false;
            }

            if (MyLog.DEBUG)
                MyLog.d(TAG, "to complete pending connectVTService");
            connectVTService();
        }

        public void onServiceDisconnected(ComponentName className) {
            if (MyLog.DEBUG)
                MyLog.d(TAG, "onServiceDisconnected");
            if (mVideoCallScreen != null) {
                mVideoCallScreen.mScreenHandler.obtainMessage(
                        VideoCallScreen.MSG_SERVICE_DISCONNECTED).sendToTarget();
            }
            mVTService = null;
        }
    };

    private final CallTime.OnTickListener mTickListener = new CallTime.OnTickListener() {
        /* Implement CallTime::OnTickListener interface onTickForCallTimeElapsed */
        public void onTickForCallTimeElapsed(long timeElapsed) {

            // if (MyLog.DEBUG) MyLog.v(TAG,
            // "onTickForCallTimeElapsed, elapsed:" + timeElapsed);

            mCallTimeElapsed = DateUtils.formatElapsedTime(timeElapsed);
            if (mVideoCallScreen != null) {
                mVideoCallScreen.mScreenHandler.obtainMessage(VideoCallScreen.MSG_TICK_UPDATE)
                        .sendToTarget();
            }
        }
    };
    public CallTime mCallTime = new CallTime(mTickListener);

    private final CallTime.OnTickListener mRecordTickListener = new CallTime.OnTickListener() {
        /*
         * Implement RecordTime::OnTickListener interface
         * onTickForCallTimeElapsed
         */
        public void onTickForCallTimeElapsed(long timeElapsed) {

            // if (MyLog.DEBUG) MyLog.v(TAG,
            // "onTickForCallTimeElapsed, elapsed:" + timeElapsed);
            mRecordTimeElapsed = DateUtils.formatElapsedTime(timeElapsed);
            if (mVideoCallScreen != null) {
                mVideoCallScreen.updateRecordTime(mRecordTimeElapsed);
            }

        }
    };
    private CallTime mRecordTime = new CallTime(mRecordTickListener);

    private final VTService.VTServiceListener mVTServiceListener = new VTService.VTServiceListener() {
        // implement VTSerivce.VTServiceListener
        // VTManager declare the VT engine gotten connected here after the
        // asynchronous method VTService.connect called
        public void onVTConnected(boolean bSuccessOrFail) {
            if (MyLog.DEBUG)
                MyLog.d(TAG, "onVTConnected: " + bSuccessOrFail);
            if (mEngineState == ENGINE_STATE_CONNECTING && bSuccessOrFail) {
                if (MyLog.DEBUG)
                    MyLog.d(TAG, "onVTConnected reset calltime");
                mStartTime = System.currentTimeMillis();
                mIsVTConnected = true;
                if (!mIsMMRingtoneStart) {
                    mCallTime.startTimer();
                    // update notification.
                    mHandler.sendMessageDelayed(mHandler.obtainMessage(MSG_NOTIFITY_INCALL), 3000);

                }
                // vibrate for 1 sec when the MO call was connected
                if (mDialOrAnswer)
                    vibrateIfNecessary();

                mEngineState = ENGINE_STATE_CONNECTED;

                if (mIsHardwareH324MStack) {
                    // if no headset pluged, turn on speaker
                    if (!VTCallUtils.checkHeadsetStatus() && !mAudioManager.isBluetoothScoOn()) {
                        switchSpeaker(true);
                    }
                } else {
                    if (!VTCallUtils.checkHeadsetStatus() && !mAudioManager.isBluetoothScoOn()) {
                        switchSpeaker(true);
                    }
                }
                if (mVideoCallScreen != null && mVideoCallScreen.mPanelManager != null) {
                    mVideoCallScreen.mPanelManager.updateSpeakerStatus();
                }
            } else {
                if (MyLog.DEBUG)
                    MyLog.d(TAG, "connect vt engine failed, end the call actively.");
                doEndCall();

            }
        }

        public void onVTReleased(boolean bComplete) {
            if (MyLog.DEBUG)
                MyLog.d(TAG, "onVTReleased: " + bComplete);
            if (mVideoCallScreen != null) {
                mVideoCallScreen.mScreenHandler.obtainMessage(
                        VideoCallScreen.MSG_SERVICE_DISCONNECTED).sendToTarget();
            }
            mVTService = null;
            closeCallScreen();
        }
    };

    private VTService.VTServiceNotificationCallback mVtServiceNfCallbacks = new VTService.VTServiceNotificationCallback() {
        public void notifyInVtCall(VTService ctx) {
            if (MyLog.DEBUG)
                MyLog.d(TAG, "notifyInVTCall 0");
            // VTNotificationManager nm = VTNotificationManager.getDefault();
            VTNotificationManager mVTnm = VTNotificationManager.getDefault();

            if (mVTnm == null) {
                VTNotificationManager.init(ctx);
                mVTnm = VTNotificationManager.getDefault();
            }
            if (mVTnm != null) {
                if (MyLog.DEBUG)
                    MyLog.d(TAG, "notifyInVTCall");
                mVTnm.notifyInVTCall(ctx);
            }
        }

        public void closeInCallNotification() {
            if (MyLog.DEBUG)
                MyLog.d(TAG, "closeInCallNotification 0");
            VTNotificationManager nm = VTNotificationManager.getDefault();
            if (nm != null)
                if (MyLog.DEBUG)
                    MyLog.d(TAG, "closeInCallNotification");
            nm.closeInCallNotification();
        }

        public void cancelNotification(VTService ctx) {
            if (MyLog.DEBUG)
                MyLog.d(TAG, "cancelNotification 0");
            NotificationManager nmg = (NotificationManager) ctx
                    .getSystemService(Context.NOTIFICATION_SERVICE);
            try {
                if (MyLog.DEBUG)
                    MyLog.d(TAG, "cancelNotification");
                nmg.cancel(VTNotificationManager.VT_IN_CALL_NOTIFICATION);
            } catch (Exception e) {
                if (MyLog.DEBUG)
                    MyLog.d(TAG, "catch a exception while clear notification indicator");
            }
        }
    };

    void doCallConnect() {
        if (MyLog.DEBUG)
            MyLog.d(TAG, "onCallConnected...");

        // b444: move mAudioManager.setParameters() to VTService, to make sure
        // it called before VT engine is connected.
        // mAudioManager.setParameters("videotelephony=on");
        // update UI if it's dial-out
        if (mVideoCallScreen != null) {
            if (mVideoCallScreen.hasStop) {
                Intent intent = constuctRestoreIntent();
                intent.addFlags(Intent.FLAG_ACTIVITY_REORDER_TO_FRONT);
                this.startActivity(intent);
            }
        }

        if (mIsDialOrAnswer == true || mVideoCallScreen == null) {
            if (mVideoCallScreen != null) {
                // to update the UI
                mVideoCallScreen.mScreenHandler.obtainMessage(VideoCallScreen.MSG_VT_CONNECTED)
                        .sendToTarget();
            } else {
                sPanelMode = VTPanel.VTPANEL_MODE_REMOTE;
                sPendingPanelMode = VTPanel.VTPANEL_MODE_REMOTE;
                //
                if (MyLog.DEBUG)
                    MyLog.d(TAG, "VideoCallScreen not available while connected, wake it up ");
                Intent intent = constuctRestoreIntent();
                this.startActivity(intent);
            }
        }

        mAppStatus = APP_STATUS_CONNECTED;
        setRepImageSelectable(false);
        mIsAnswering = false;
        mIsPhoneConnected = true;

        connectVTService();
    }

    // vibrate for 1 sec when the MO call was connected
    private void vibrateIfNecessary() {
        if (MyLog.DEBUG)
            MyLog.d(TAG, "vibrateIfNecessary()...");
        // b413,ISC did not have VIBRATING_MO_CALL_CONNECTED setting, so
        // vibrating every time for tamplately
        boolean shouldVibrateWhenMOCallConnected = true;
        // Settings.System.getInt(this.getContentResolver(),
        // Settings.System.VIBRATING_MO_CALL_CONNECTED, 0) > 0;
        if (MyLog.DEBUG)
            MyLog.d(TAG, "shouldVibrateWhenMOCallConnected: " + shouldVibrateWhenMOCallConnected);
        if (shouldVibrateWhenMOCallConnected) {
            vibrate();
        }
    }

    // vibrate for 1 second when MO call connected
    private void vibrate() {
        VibratorThread thread = new VibratorThread();
        thread.start();
    }

    private class VibratorThread extends Thread {
        public void run() {
            if (MyLog.DEBUG)
                MyLog.d(TAG, "Start vibration for 0.3 second!");
            // Vibrator v = new Vibrator();
            Vibrator v = (Vibrator) getSystemService(VIBRATOR_SERVICE);
            v.vibrate(300);
        }
    }

    private class QueryCallerInfoThread extends Thread {
        private VideoCallApp mApp;
        private String phoneNumber;

        public QueryCallerInfoThread(VideoCallApp app, String num) {
            mApp = app;
            phoneNumber = num;
        }

        public void run() {
            if (MyLog.DEBUG)
                MyLog.d(TAG, "Start querying callerinfo from contacts db");
            CallerInfo ci = new CallerInfo();
            mCallerPic = null;
            phoneNumber = PhoneNumberUtils.convertKeypadLettersToDigits(phoneNumber);
            phoneNumber = PhoneNumberUtils.stripSeparators(phoneNumber);
            phoneNumber = PhoneNumberUtils.formatNumber(phoneNumber);
            ci.phoneNumber = phoneNumber;
            Cursor c = mApp.getContentResolver().query(
                    Uri.withAppendedPath(PhoneLookup.CONTENT_FILTER_URI, phoneNumber),
                    new String[] { PhoneLookup.DISPLAY_NAME, PhoneLookup.CUSTOM_RINGTONE,
                            PhoneLookup.PHOTO_ID }, null, null, null);
            if (c != null) {
                if (c.moveToFirst()) {
                    ci.name = c.getString(0);
                    String ringtone = c.getString(1);
                    if (ringtone != null) {
                        ci.contactRingtoneUri = Uri.parse(ringtone);
                    }
                    long photoid = c.getLong(2);
                    if (MyLog.DEBUG)
                        Log.d(TAG, "contact photoid:" + photoid);
                    mCallerPic = getPhoto(getApplicationContext(), photoid);

                } else {
                    if (MyLog.DEBUG)
                        Log.d(TAG, " CallerInfo c.moveToFirst = null");
                }
                c.close();
            } else {
                if (MyLog.DEBUG)
                    Log.d(TAG, " CallerInfo ci = null");
                ci = null;
            }

            // notify query complete
            mApp.onQueryCallerInfoComplete(ci);
        }
    }

    public Bitmap getPhoto(Context ct, long photo_id) {
        Bitmap bmp = BitmapFactory.decodeResource(ct.getResources(), R.drawable.picture_unknown);
        if (photo_id != 0) {
            //DATA15 contains blob data of photo
            String[] projection = new String[] { ContactsContract.Data.DATA15 };
            String selection = ContactsContract.CommonDataKinds.Photo.PHOTO_ID + " = " + photo_id + " AND " +
                               ContactsContract.CommonDataKinds.Photo.MIMETYPE + " = " +
                                                "'" + ContactsContract.CommonDataKinds.Photo.CONTENT_ITEM_TYPE + "'";
            Cursor cur = ct.getContentResolver().query(ContactsContract.Data.CONTENT_URI,
                    projection, selection, null, null);
            if (cur != null && cur.moveToNext()) {
                //getting value of DATA15
                if(Cursor.FIELD_TYPE_BLOB == cur.getType(0)){
                    byte[] contactIcon = null;
                    //getting value of DATA15
                    contactIcon = cur.getBlob(0);
                    if (contactIcon != null) {
                        bmp = BitmapFactory.decodeByteArray(contactIcon, 0, contactIcon.length);
                    }
                }
                cur.close();
            }
        }
        return bmp;
    }

    private void processCallLog(boolean bIsRefuseOrMissed) {
        if (MyLog.DEBUG)
            MyLog.d(TAG, "processCallLog: bIsRefuseOrMissed" + bIsRefuseOrMissed);
        // add call log
        if (mDialOrAnswer) {
            // MO
            if (bIsRefuseOrMissed || (mIsVTConnected && !mIsMMRingtoneStart)) {
                // remote end after normal talking
                mCallLogType = CallLog.Calls.OUTGOING_TYPE;
                mCallLogCSVTType = VTCallUtils.OUTGOING_CSVT_TYPE;
                if (!mIsVTConnected)
                    mStartTime = System.currentTimeMillis();
            } else {
                // failed to connect
                // reset start time
                mStartTime = System.currentTimeMillis();
                mCallLogType = CallLog.Calls.OUTGOING_TYPE;
                mCallLogCSVTType = VTCallUtils.OUTGOING_CSVT_TYPE;
            }
            recordLastMO();
        } else {

            stopRing();
            if (mIsVTConnected && mIsMMRingtoneStart == false) {
                // connected
                mCallLogType = CallLog.Calls.INCOMING_TYPE;
                mCallLogCSVTType = VTCallUtils.INCOMING_CSVT_TYPE;
            } else {
                // failed to connect
                // reset start time
                mStartTime = System.currentTimeMillis();
                mCallLogType = CallLog.Calls.MISSED_TYPE;
                mCallLogCSVTType = VTCallUtils.MISSED_CSVT_TYPE;
                // missed caused default value: DisconnectCause.INCOMING_MISSED
                mMissedCause = (bIsRefuseOrMissed) ? DisconnectCause.INCOMING_REJECTED
                        : DisconnectCause.INCOMING_MISSED;
                if (mMissedCause == DisconnectCause.INCOMING_REJECTED)
                    mCallLogCSVTType = VTCallUtils.INCOMING_CSVT_TYPE;

            }
        }
        if (mStartTime < 0) {
            mStartTime = System.currentTimeMillis();
        }
        mDuration = (int) mCallTime.getCallDuration();

        synchronized (mQueryCompleteGuard) {
            if (MyLog.DEBUG)
                MyLog.d(TAG, "isQueryCompleted:? " + mIsQueryCompleted);
            if (mIsQueryCompleted) {
                addCallLog();
            } else {
                mAddCallLogPending = true;
            }
        }
    }

    private void recordLastMO() {
        //filepath: /data/data/com.borqs.videocall/files/lastMOVideoCall
        File file = new File(mCtx.getFilesDir(), "lastMOVideoCall");
        FileWriter fw = null;
        try {
            fw = new FileWriter(file);
            fw.write(mInputPhoneURL, 0, mInputPhoneURL.length());
            fw.close();
        } catch (IOException e) {
            if (MyLog.DEBUG)
                Log.e(TAG, "Can not open file writer");
        }
    }

    // NOTES: the difference between doCallDisconnect and doEndCall is that
    // doEndCall shall call IVideoTelephony.endcall to end the call actively
    void doCallDisconnect(CharSequence strReason) {

        if (MyLog.DEBUG)
            MyLog.d(TAG, "onCallDisconnected...: " + strReason);
        if (VTCallUtils.isVTActive()) {
            if (MyLog.DEBUG)
                MyLog.d(TAG, "VT call is active so dont do anything");
            return;
        }
        stopAudioRecord();
        if (strReason != null && !mIsVTConnected) {
            //We will hit this condition in below scenarios
            //MO a CSVT call and disconnect either MO/MT side and viceversa
            //MO a CSVT call and no response on either ends.
            Toast.makeText(this, strReason, Toast.LENGTH_LONG).show();
        } else if (mEngineState != ENGINE_STATE_CONNECTED && mDialOrAnswer) {
            Toast.makeText(this, getText(R.string.call_failed_normal), Toast.LENGTH_LONG).show();
        }

        if (!((String) getText(R.string.call_exception_failed)).equals((String) strReason)) {
            processCallLog(false);
        }
        if (mCurrentVideoSource == VTManager.VideoSource.CAMERA_NONE) {
            if (t.getStatus().equals(AsyncTask.Status.RUNNING)) {
                if (MyLog.DEBUG)
                    MyLog.d(TAG, "stop image ");
                t.cancel(true);
            }
        }
        // openCloseCamera();
        /*
         * if(t.getStatus().equals(AsyncTask.Status.RUNNING)){ if (MyLog.DEBUG)
         * MyLog.d(TAG, "stop image " ); t.cancel(true); }
         */
        ResetAll();

    }

    /* Package */
    void doEndCall() {

        if (MyLog.DEBUG)
            MyLog.d(TAG, "do End Call");
        stopAudioRecord();

        if (mCurrentVideoSource == VTManager.VideoSource.CAMERA_NONE) {
            if (t.getStatus().equals(AsyncTask.Status.RUNNING)) {
                if (MyLog.DEBUG)
                    MyLog.d(TAG, "stop image ");
                t.cancel(true);
            }
            // openCloseCamera();
        }

        if (mEngineState == ENGINE_STATE_CONNECTED || mEngineState == ENGINE_STATE_CONNECTING) {
            // VT engine connected, disconnect it
            // mVTService.disconnect();
            getVTServiceInterface().end(mVTService);
            mEngineState = ENGINE_STATE_NOT_CONNECT;
        }

        if (MyLog.DEBUG)
            MyLog.v(TAG, "End Call DialOrAnswer" + mDialOrAnswer);
        if (mDialOrAnswer)
            mCallClient.endCall();
        else
            mConnection.endSession();

        processCallLog(true);

        ResetAll();
    }

    /* Package */
    void doRejectCall() {

        if (MyLog.DEBUG)
            MyLog.d(TAG, "do Reject Call");
        stopAudioRecord();

        if (MyLog.DEBUG)
            MyLog.v(TAG, "Reject Call DialOrAnswer" + mDialOrAnswer);
        if (mDialOrAnswer)
            mCallClient.endCall();
        else {
            mConnection.rejectSession();
            if (VTCallUtils.isVTActive()) {
                if (MyLog.DEBUG)
                    MyLog.v(TAG, "Active VT call is going, no need to disconnect VT Engine");
                processCallLog(true);

                ResetAll();
                return;
            }
        }

        if (mEngineState == ENGINE_STATE_CONNECTED || mEngineState == ENGINE_STATE_CONNECTING) {
            // VT engine connected, disconnect it
            mVTService.disconnect();
            mEngineState = ENGINE_STATE_NOT_CONNECT;
        }

        processCallLog(true);

        ResetAll();
    }

    /* Package */public void doFallback(CharSequence strReason) {
        if (MyLog.DEBUG)
            MyLog.d(TAG, "onFallback...reason: " + strReason);
        mFallbackSetting = Settings.System.getInt(this.getContentResolver(),
                FallBackSetting.VIDEOCALL_FALL_BACK_SETTING, FallBackSetting.FALL_BACK_MANUAL);
        // ONLY for Loopback mode to test calllog
        if (mLaunchMode == VideoCallApp.CALL_FROM_LOOPBACK) {
            doCallDisconnect(getText(R.string.call_result_disconnect_incoming_rejected));
            return;
        }

        if (!mDialOrAnswer) {
            // Do fallback
            stopRing();
            if (MyLog.DEBUG)
                MyLog.v(TAG, "fallback, dial_or_answer:" + mDialOrAnswer);
            getVTServiceInterface().fallBack(mConnection);
        }
        ResetAll();

        if (mDialOrAnswer) { // Dial out, here wait a fallback notification and
                             // to make a decision
            if (MyLog.DEBUG)
                MyLog.d(TAG, "Dial FallBack, setting: " + mFallbackSetting);
            switch (mFallbackSetting) {
            case FallBackSetting.FALL_BACK_MANUAL: {
                // popup the yes-or-no dialog
                if (MyLog.DEBUG)
                    MyLog.d(TAG, "manual fallback");
                Intent intent = new Intent(this, FallBackAlertDialog.class);
                intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK
                        | Intent.FLAG_ACTIVITY_EXCLUDE_FROM_RECENTS);
                intent.putExtra(EXTRA_FALLBACK_MESSAGE,
                        strReason + " " + this.getText(R.string.fallback_manual));
                intent.putExtra(INTENT_EXTRA_PHONE_URL, mInputPhoneURL);
                startActivity(intent);
            }
                return;
            case FallBackSetting.FALL_BACK_AUTO_VOICE:
                Toast.makeText(this, strReason + " " + this.getText(R.string.fallback_auto_voice),
                        5000).show();
                VTCallUtils.launch2GCall(this.mInputPhoneURL, this);
                break;
            case FallBackSetting.FALL_BACK_AUTO_REJECT:
                Toast.makeText(this, strReason + " " + this.getText(R.string.fallback_auto_close),
                        Toast.LENGTH_LONG).show();
                break;
            default:
                break;
            }
            ;
        } else { // it's answering a call
            /* show waiting dialog for 15 seconds */
            fallbackFlag = 1;
            Intent intent = new Intent(this, WaitingFallBackDialog.class);
            intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK
                    | Intent.FLAG_ACTIVITY_EXCLUDE_FROM_RECENTS);
            startActivity(intent);
        }

        return;
    }

    boolean startAudioRecord() {
        if ((mAudioRecordStatus == AUDIO_RECORD_STATE.STARTED)
                || (mEngineState < VideoCallApp.ENGINE_STATE_CONNECTED)) {
            if (MyLog.DEBUG)
                MyLog.d(TAG,
                        "Audio is recording already or engine not connected yet, just return false");
            return false;
        }
        int sdcardStatus = VTCallUtils.checkStorage();
        if (VTCallUtils.SDCARD_OK != sdcardStatus) {
            VTCallUtils.showStorageToast(sdcardStatus, this);
            mAudioRecordStatus = AUDIO_RECORD_STATE.IDLE;
            if (MyLog.DEBUG)
                MyLog.d(TAG,
                        "VTCallUtils.SDCARD_OK != sdcardStatus : mAudioRecordStatus = AUDIO_RECORD_STATE.IDLE");
            return false;
        }
        AUDIO_RECORDER_PATH = Environment.getExternalStorageDirectory().getAbsolutePath()
                + VideoCallApp.getInstance().getApplicationContext().getResources()
                        .getString(R.string.recorder_folder);
        recorderPrefix = VideoCallApp.getInstance().getApplicationContext().getResources()
                .getString(R.string.recorder_prefix);

        File directory = new File(AUDIO_RECORDER_PATH);
        if (!directory.isDirectory()) {
            if (!directory.mkdirs()) {
                mAudioRecordStatus = AUDIO_RECORD_STATE.IDLE;
                if (MyLog.DEBUG)
                    MyLog.d(TAG,
                            "!directory.mkdirs() : mAudioRecordStatus = AUDIO_RECORD_STATE.IDLE");
                return false;
            }
        }

        long dateTaken = System.currentTimeMillis();
        // Add the dot prefix to hide the file when recording
        mTempRecAudFullPath = AUDIO_RECORDER_PATH + "/." + recorderPrefix
                + DateFormat.format("yyyy-MM-dd_kk-mm-ss", dateTaken).toString() + ".3ga";
        mRecAudFullPath = AUDIO_RECORDER_PATH + "/" + recorderPrefix
                + DateFormat.format("yyyy-MM-dd_kk-mm-ss", dateTaken).toString() + ".3ga";

        if (MyLog.DEBUG)
            MyLog.d(TAG, "start record audio, path: " + mTempRecAudFullPath);
        mSoundRecorder = new MediaRecorder();
        mSoundRecorder.setAudioSource(MediaRecorder.AudioSource.VOICE_CALL);
        mSoundRecorder.setOutputFormat(MediaRecorder.OutputFormat.THREE_GPP);
        mSoundRecorder.setAudioEncoder(MediaRecorder.AudioEncoder.AMR_NB);
        mSoundRecorder.setOutputFile(mTempRecAudFullPath);

        long nowTime = System.currentTimeMillis();
        if (nowTime - mStartTime < 4000) {
            try {
                Thread.sleep(4000 - (nowTime - mStartTime));
            } catch (InterruptedException ie) {
                if (MyLog.DEBUG)
                    Log.e(TAG, "Exception,Interrupted to sleep audio Starting");
            }
        }
        // Handle IOException
        try {
            mSoundRecorder.prepare();
        } catch (IOException exception) {
            if (MyLog.DEBUG)
                Log.e(TAG, "exception when sound recorder prepare");
            mSoundRecorder.reset();
            mSoundRecorder.release();
            mSoundRecorder = null;
            mAudioRecordStatus = AUDIO_RECORD_STATE.IDLE;
            if (MyLog.DEBUG)
                MyLog.d(TAG,
                        "exception when sound recorder prepare: mAudioRecordStatus = AUDIO_RECORD_STATE.IDLE");
            return false;
        }

        mSoundRecorder.start();
        if (MyLog.DEBUG)
            MyLog.d(TAG, "mSoundRecorder.start():mAudioRecordStatus = AUDIO_RECORD_STATE.STARTED");
        if (mRecordTime != null) {
            if (MyLog.DEBUG)
                MyLog.d(TAG, "start timer.");
            mRecordTime.startTimer();
        }
        mScheduleTimer = new Timer();
        mScheduleTimer.scheduleAtFixedRate(new CheckStorageTask(), 1000, 1000);
        mAudioRecordStatus = AUDIO_RECORD_STATE.STARTED;
        if (MyLog.DEBUG)
            MyLog.d(TAG, "exit start Audio recording!");
        return true;
    }

    class CheckStorageTask extends TimerTask {
        public void run() {
            if (MyLog.DEBUG)
                MyLog.d(TAG, "Checking storage when recording...");
            if (mAudioRecordStatus == AUDIO_RECORD_STATE.STARTED) {
                int sdcardStatus = VTCallUtils.checkStorage();
                if (MyLog.DEBUG)
                    MyLog.d(TAG, "CheckStorageTask sdcardStatus:" + sdcardStatus);
                if (VTCallUtils.SDCARD_OK != sdcardStatus) {
                    mHandler.obtainMessage(MSG_STOP_RECORD).sendToTarget();
                }
            }
        }
    };

    void stopAudioRecord() {

        if (MyLog.DEBUG)
            MyLog.d(TAG, "Stop Audio Record..");
        if (mSoundRecorder == null) {
            if (MyLog.DEBUG)
                MyLog.d(TAG, "sound recorder is null, just return");
            return;
        }
        try {
            if (mAudioRecordStatus != AUDIO_RECORD_STATE.IDLE) {
                int counter = 4;
                while (mAudioRecordStatus != AUDIO_RECORD_STATE.STARTED && counter > 0) {
                    counter--;
                    try {
                        Thread.sleep(1000);
                    } catch (InterruptedException ie) {
                        if (MyLog.DEBUG)
                            Log.e(TAG, "Exception,Interrupted to sleep audio stoping");
                    }

                }
                mSoundRecorder.stop();
                mAudioRecordStatus = AUDIO_RECORD_STATE.IDLE;
                if (MyLog.DEBUG)
                    MyLog.d(TAG,
                            "mSoundRecorder.stop():mAudioRecordStatus = EAudioRecordStatus.idle");
            }
        } catch (Exception ex) {
            Log.e(TAG, "Exception when stop recording: " + ex);
        }
        if (mRecordTime != null) {
            mRecordTime.cancelTimer();
            if (mVideoCallScreen != null) {
                mVideoCallScreen.updateRecordTime("");
                if (MyLog.DEBUG)
                    MyLog.d(TAG, "mVideoCallScreen.updateRecordTime to null ");
            }
        }
        if (mScheduleTimer != null) {
            if (MyLog.DEBUG)
                MyLog.d(TAG, "cancel timer.");
            mScheduleTimer.cancel();
            mScheduleTimer = null;
        }
        mSoundRecorder.release();
        mSoundRecorder = null;

        File tmpFile = new File(mTempRecAudFullPath);
        File finalFile = new File(mRecAudFullPath);
        boolean success = tmpFile.renameTo(finalFile);
        if (!success) {
            Log.e(TAG, "Failed to rename file:" + tmpFile);
            return;
        }
        try {
            final Uri contentUri = Uri.fromFile(finalFile);
            sendBroadcast(new Intent(Intent.ACTION_MEDIA_SCANNER_SCAN_FILE, contentUri));
            ;
        } catch (Exception ex) {
            Log.e(TAG, "Failed to scan recorded video: " + ex.toString());
        }
        Toast.makeText(this,
                this.getString(R.string.finish_record_mixed_audio) + AUDIO_RECORDER_PATH,
                Toast.LENGTH_SHORT).show();
        if (MyLog.DEBUG)
            MyLog.d(TAG, "exit stop Audio recording!");
    }

    /**/
    // VTService
    private void connectVTService() {
        if (MyLog.DEBUG)
            MyLog.d(TAG, "connectVTService");

        if (mEngineState == ENGINE_STATE_CONNECTED || mEngineState == ENGINE_STATE_CONNECTING) {
            if (MyLog.DEBUG)
                MyLog.d(TAG, "VT connected already.");
            return;
        }
        if ((mIsServerBound == false) || (mHasRemoteSurfaceSet == false)
                || (mIsPhoneConnected == false)) {
            if (MyLog.DEBUG)
                MyLog.d(TAG,
                        "service not binded or remote surface not set or phone not connected yet, just return.");
            return;
        }

        if (MyLog.DEBUG)
            MyLog.d(TAG, "connected and setVTDevice " + mDeviceName);
        if ("1".equals(SystemProperties.get("ro.kernel.qemu"))) {
            Toast.makeText(this.mCtx, this.getString(R.string.emulatordialout), Toast.LENGTH_LONG)
                    .show();
            return;
        }
        mVTService.setVTDevice(mDeviceName);
        mVTService.connect();
        mEngineState = ENGINE_STATE_CONNECTING;

        if (MyLog.DEBUG)
            MyLog.v(TAG, "connect Service is ok!!!!!!!!!!!!!!!!");
    }

    /**
     * Returns the singleton instance of the PhoneApp.
     */
    static VideoCallApp getInstance() {
        return sMe;
    }

    private String getCallDetailString() {
        if (MyLog.DEBUG)
            MyLog.d(TAG, "getCallDetailString...");

        // str1: phone number
        // str2: start time
        // str3: duration

        String str1, str2, str3;

        switch (mCallLogType) {
        case CallLog.Calls.OUTGOING_TYPE:
            if (mIsMMRingtoneStart == true) {
                return null;
            }
        case CallLog.Calls.INCOMING_TYPE:
            str1 = this.getString(R.string.call_number) + mInputPhoneURL;
            break;
        default:
            // should be types as follow
            // case CallLog.Calls.OUTGOING_FAILED_TYPE:
            // case CallLog.Calls.MISSED_TYPE:
            return null;
        }

        Calendar calendar = Calendar.getInstance();
        calendar.setTimeInMillis(mStartTime);
        String date = DateFormat.getDateFormat(this).format(calendar.getTime());
        String time = DateFormat.getTimeFormat(this).format(calendar.getTime());
        str2 = this.getString(R.string.call_start_time) + date + " " + time;

        str3 = this.getString(R.string.call_duration) + mCallTimeElapsed;

        String detail = new String("\n" + str1 + "\n\n" + str2 + "\n\n" + str3 + "\n");
        return detail;
    }

    static String getCallScreenClassName() {
        return VideoCallScreen.class.getName();
    }

    /**
     * Starts the InCallScreen Activity.
     */
    void displayCallScreen() {
        Intent intent = new Intent(INTENT_ACTION_LAUNCH_VIDEOCALLSCREEN);
        intent.addCategory(Intent.CATEGORY_DEFAULT);
        intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_ACTIVITY_EXCLUDE_FROM_RECENTS);
        startActivity(intent);
    }

    boolean isShowingCallScreen() {
        return mVideoCallScreen != null;
    }

    VideoCallScreen getCallScreen() {
        return mVideoCallScreen;
    }

    // VideoCallScreen might be NULL indicate the CallScreen are destroying
    void setCallScreen(VideoCallScreen s) {
        if (MyLog.DEBUG)
            MyLog.d(TAG, "setCallSrceen: " + s);
        mVideoCallScreen = s;
    }

    // FIXME: synchronization issues
    // The entry to launch VideoCallScreen
    // Special Note: Caller
    // @Param bDialOrAnswer, Launch for Dial or Answer
    // @Param nWhere, indicate where this launch happens , refer "CALL_FROM_XXX"
    // @Param starCallURL, specify the target address TO or FROM
    // void launchVTScreen( Context ctx, boolean bDialOrAnswer, boolean
    // bLockMode, String strAddr, int nWhere, IVTConnection iConnection){
    void launchVTScreen(boolean bDialOrAnswer, boolean bLockMode, String strAddr, int nWhere,
            IVTConnection iConnection) {

        if (MyLog.DEBUG)
            MyLog.d(TAG, "launchCallScreen()...");

        // It's late to set VideoCallApp.mDialOrAnswer value at VideoCallScreen
        // initializing.
        // VideoCallApp.mDialOrAnswer should be set properly when doEndCall() is
        // called but VideoCallScreen hasn't been created yet.
        // bDialOrAnswer here is always false.
        mDialOrAnswer = bDialOrAnswer;
        mConnection = iConnection;
        Intent intent = new Intent(INTENT_ACTION_LAUNCH_VIDEOCALLSCREEN);
        intent.addCategory(Intent.CATEGORY_DEFAULT);
        intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_ACTIVITY_EXCLUDE_FROM_RECENTS);
        intent.putExtra(INTENT_EXTRA_CALL_OR_ANSWER, bDialOrAnswer);
        intent.putExtra(INTENT_EXTRA_LAUNCH_MODE, nWhere);
        intent.putExtra(INTENT_EXTRA_PHONE_URL, strAddr);
        intent.putExtra(INTENT_EXTRA_IS_LOCKED_MODE, bLockMode);
        this.startActivity(intent);

        // if( bDialOrAnswer == false){ //MUST BE in ANSWERING
        mConnection.setHandler(mHandler);
        // }

        return;
    }

    Intent constuctRestoreIntent() {

        Intent intent = new Intent(INTENT_ACTION_LAUNCH_VIDEOCALLSCREEN);
        intent.addCategory(Intent.CATEGORY_DEFAULT);
        intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_ACTIVITY_EXCLUDE_FROM_RECENTS);
        intent.putExtra(INTENT_EXTRA_CALL_OR_ANSWER, true);
        intent.putExtra(INTENT_EXTRA_LAUNCH_MODE, mLaunchMode);
        intent.putExtra(INTENT_EXTRA_PHONE_URL, mInputPhoneURL);

        return intent;
    }

    public IVTConnection getVTConnection() {
        return mConnection;
    }

    /**
     * Dismisses the InCallScreen Activity, if it's currently running.
     */
    void closeCallScreen() {
        if (MyLog.DEBUG)
            MyLog.d(TAG, "closeCallScreen()...");
        if (isShowingCallScreen()) {
            View middleView = mVideoCallScreen.findViewById(R.id.vtpanel_middle_buttons);
            if (middleView != null)
                middleView.setVisibility(View.GONE);
            mVideoCallScreen.finish();
            // mVideoCallScreenInstance will be nulled out when
            // the VideoCallScreen actually stops.
        }
    }

    // FIXME: context verification
    boolean dialOut(int nLaunchMode) {
        if (MyLog.DEBUG)
            MyLog.d(TAG, "dialOut");

        mCallClient = new VTCallProxy(mInputPhoneURL, mHandler, this); // end of
                                                                       // new
                                                                       // VTIpConnectService.CallClient

        if (mIsHardwareH324MStack) {
            VTServiceInterface.setAudioMode(mCtx, AudioManager.MODE_IN_COMMUNICATION);
            //mAudioManager.setStreamVolume(AudioManager.STREAM_NOTIFICATION, 1, 0);
            if (!mAudioManager.isBluetoothScoOn()) {
                boolean bHeadsetPlugged = mAudioManager.isWiredHeadsetOn();
                if (MyLog.DEBUG)
                    MyLog.d(TAG, "headset plugged?: " + bHeadsetPlugged);
                switchSpeaker(!bHeadsetPlugged);
            }
        } else {
            VTServiceInterface.setAudioMode(mCtx, AudioManager.MODE_IN_COMMUNICATION);
            //mAudioManager.setStreamVolume(AudioManager.STREAM_NOTIFICATION, 1, 0);
            if (isBtHeadsetPlugged() == true) {
                if (MyLog.DEBUG)
                    MyLog.d(TAG, "dialOut BT Headet is connected");
                switchSpeaker(!isBtHeadsetPlugged());
                switchBtSco(isBtHeadsetPlugged());
            } else {
                boolean bHeadsetPlugged = mAudioManager.isWiredHeadsetOn();
                if (MyLog.DEBUG)
                    MyLog.d(TAG, "Wired headset plugged? : " + bHeadsetPlugged);
                switchSpeaker(!isBtHeadsetPlugged());
            }

        }
        return mCallClient.startCall(nLaunchMode);
    }

    void answerCall() {
        // ASSERT it's must be in answer mode
        if (MyLog.DEBUG)
            MyLog.d(TAG, "mConnection.acceptCall...");
        mIsAnswering = true;
        // stop ring at first
        stopRing();

        // if(mIsHardwareH324MStack){
        VTServiceInterface.setAudioMode(mCtx, AudioManager.MODE_IN_COMMUNICATION);
        //mAudioManager.setStreamVolume(AudioManager.STREAM_NOTIFICATION, 1, 0);
        // }

        // Special Note: wait for IVideoTelephony giving the CONNECTED
        // notification,
        // herein the onCallConnected is triggered, and the connectVTService
        // will be called.
        // connectVTService();

        // test for loopback
        if (mLaunchMode == CALL_FROM_LOOPBACK) {

            if (isBtHeadsetPlugged() == true) {
                switchSpeaker(!isBtHeadsetPlugged());
                switchBtSco(isBtHeadsetPlugged());
            }

            doCallConnect();
        } else {
            getVTServiceInterface().answer(mConnection);
        }
        //
        if (mVideoCallScreen != null) {
			mVideoCallScreen.mPanelManager.cameraImg.setVisibility(View.GONE);
            mVideoCallScreen.mScreenHandler.obtainMessage(VideoCallScreen.MSG_VT_CONNECTED)
                    .sendToTarget();
        }
    }

    private void initDevice() {

        if (MyLog.DEBUG)
            MyLog.d(TAG, "init Device");

        // if( mCameraSetting == null)
        mCameraSetting = new CameraSetting();
        VTNotificationManager.init(this);

        mFallbackSetting = Settings.System.getInt(this.getContentResolver(),
                FallBackSetting.VIDEOCALL_FALL_BACK_SETTING, FallBackSetting.FALL_BACK_MANUAL);
        // b413 ISC not support open flip to answer call
        // mOpenFlipToAnswer = Settings.System.getInt(this.getContentResolver(),
        // Settings.System.OPEN_FLIP_ANSWER_CALL, 0)>0;

        if (mWakeLock == null) {
            PowerManager pm = (PowerManager) getSystemService(Context.POWER_SERVICE);
            mWakeLock = pm.newWakeLock(PowerManager.FULL_WAKE_LOCK
                    | PowerManager.ACQUIRE_CAUSES_WAKEUP | PowerManager.ON_AFTER_RELEASE, TAG);
        }

    }

    boolean initApp() {

        if (MyLog.DEBUG)
            MyLog.d(TAG, "InitApp called and screen should just be created.");
        mDialOrAnswer = mVideoCallScreen.mDialOrAnswer;
        mLaunchMode = mVideoCallScreen.mLaunchMode;
        mInputPhoneURL = mVideoCallScreen.mInputPhoneURL;
        mDeviceName = mVideoCallScreen.mDeviceName;
        if(2 == android.hardware.Camera.getNumberOfCameras()){
            mCurrentVideoSource = VTManager.VideoSource.CAMERA_SECONDARY;
        }
        else if(1 == android.hardware.Camera.getNumberOfCameras()){
            mCurrentVideoSource = VTManager.VideoSource.CAMERA_MAIN;
        }
        mLastVideoSource = VTManager.VideoSource.CAMERA_NONE;
        mStrReplaceImagePath = VTImageReplaceSetting.getCurrentReplaceImagePath(VideoCallApp.this);
        mAudioManager = (AudioManager) mCtx.getSystemService(Context.AUDIO_SERVICE);
        mNotificationVol = mAudioManager.getStreamVolume(AudioManager.STREAM_NOTIFICATION);

        if (mAudioManager.isMusicActive()) {
            Intent i = new Intent("com.android.music.musicservicecommand");

            i.putExtra("command", "pause");
            mCtx.sendBroadcast(i);
            mIsMusicPaused = true;
        }

        // Initialize Device Corresponding
        initDevice();

        VTNotificationManager nm = VTNotificationManager.getDefault();
        if (nm == null) {
            VTNotificationManager.init(this);
            nm = VTNotificationManager.getDefault();
        }
        if (MyLog.DEBUG)
            MyLog.d(TAG, "set notification alerts to false 1...");
        nm.getStatusBarMgr().enableNotificationAlerts(false);

        // bind VTService
        // If it is an incoming call, we will start VTService firstly in the
        // VTCallReceiver.
        if (mVTService == null) {
            startVTService();
        }
        adapter = BluetoothAdapter.getDefaultAdapter();
        adapter.getProfileProxy(mCtx, new HeadsetServiceListener(), BluetoothProfile.HEADSET);

        registerVTAppReceiver();
        mAppStatus = APP_STATUS_RINGING;
        return true;
    }

    boolean startVTService() {
        if (MyLog.DEBUG)
            MyLog.d(TAG, "start VTService...");

        // set the notification callbacks
        VTService.setNotificationCallback(VideoCallApp.this.mVtServiceNfCallbacks);

        if (mIsStartingService) {
            if (MyLog.DEBUG)
                MyLog.d(TAG, "the VTService is starting, just return");
            return false;
        }

        mIsStartingService = true;
        if (false == VTService.startService(this)) {
            if (MyLog.DEBUG)
                MyLog.d(TAG, "failed to start VTService, just finish this call session.");
            return false;
        }

        if (false == bindService(new Intent(this, VTService.class), mConn, 0)) {
            if (MyLog.DEBUG)
                MyLog.v(TAG, "bindService is false!!!!!!!!!!!!!!!!");
            return false;
        }
        if (MyLog.DEBUG)
            MyLog.d(TAG, "start VTService");
        return true;
    }

    int getStatus() {
        return mAppStatus;
    }

    void setStatus(int ns) {
        if (ns >= APP_STATUS_IDLE && ns <= APP_STATUS_CONNECTED)
            mAppStatus = ns;
    }

    public VTServiceInterface getVTServiceInterface() {
        if (mVTServiceInterface == null) {
            mVTServiceInterface = new VTServiceInterface();
        }

        return mVTServiceInterface;
    }

    @Override
    public void onCreate() {
        // keep this
        sMe = this;
        mCtx = getApplicationContext();
        new RemoteServiceConnector(mCtx);
        VTServiceInterface.setAppContext(mCtx);
        try {
            this.phoneCtx = this.createPackageContext("com.android.phone", 0);
        } catch (NameNotFoundException ex) {
            Log.e("failed to get phone package:", ex.getMessage());
        }
        mStartQueryTimeStamp.mTimeStamp = 1;
        mCompleteQueryTimeStamp.mTimeStamp = -1;
        mIsHardwareH324MStack = "true".equals(SystemProperties.get("hw.vt.324stackinmodem"));
        mIsLandscape = getResources().getConfiguration().orientation == Configuration.ORIENTATION_LANDSCAPE;
        getVTServiceInterface().SetisUseVTCamera(
                !("false".equals(SystemProperties.get("hw.vt.usevtcamera"))));
        VTCallUtils.createCsvtService(mCtx);
        t = new SendtoVTServiceThread();

        if (MyLog.DEBUG)
            MyLog.v(TAG, "!!VideoCallApp onCreate()...mIsLandscape=" + mIsLandscape);
    }

    @Override
    public void onTerminate() {

        if (MyLog.DEBUG)
            MyLog.v(TAG, "!!VideoCallApp onTerminate()...");
        super.onTerminate();
    }

    /* set input phone url from videocall screen */
    void setInputPhoneUrl(String Url) {
        mInputPhoneURL = Url;
    }

    /*
     * query custom ringtone time spent
     */
    void startIncomingCallQuery(String callerURL, boolean dialOrAnswer) {

        // FIXME for loopback test
        if (dialOrAnswer == false && mRinger == null) {
            mRinger = new Ringer(this);
        }

        if (MyLog.DEBUG)
            MyLog.d(TAG, "startIncomingCallQuery: " + callerURL);
        mIsDialOrAnswer = dialOrAnswer;
        /* query a ringtone uri */
        if (callerURL == null || TextUtils.isEmpty(callerURL)
                || callerURL.equals(this.getString(R.string.unknown_number))) {
            // no need to query
            if (MyLog.DEBUG)
                MyLog.d(TAG, "callerUrl is null");
            mIsQueryCompleted = true;

            if (mRinger != null) { // answer
            // Uri profileRingTone = Settings.Profile.getRingTone(this
            // .getContentResolver(), Settings.Profile.L1RINGTONE);
                mRinger.setCustomRingtoneUri(Settings.System.DEFAULT_RINGTONE_URI);

                startRing();
            }

            return;
        }

        if (MyLog.DEBUG)
            MyLog.d(TAG, "start query ringtone");
        // use the setting profile
        // Uri profileRingTone = Settings.Profile.getRingTone(this
        // .getContentResolver(), Settings.Profile.L1RINGTONE);
        if (mRinger != null) {
            mRinger.setCustomRingtoneUri(Settings.System.DEFAULT_RINGTONE_URI);
        }

        // query the callerinfo to try to get the ringer.

        synchronized (mQueryCompleteGuard) {
            mStartQueryTimeStamp.mTimeStamp = System.currentTimeMillis();
            mCompleteQueryTimeStamp.mTimeStamp = mStartQueryTimeStamp.mTimeStamp;
        }

        if (MyLog.DEBUG)
            MyLog.d(TAG, "startGetCallerInfo, time stamp: " + mStartQueryTimeStamp.mTimeStamp);

        if (callerURL != null && !TextUtils.isEmpty(callerURL)
                && !callerURL.equals(this.getString(R.string.unknown_number))) {
            mVTInfoQueryState = VTQUERY_STATE_RUNNING;
            QueryCallerInfoThread thread = new QueryCallerInfoThread(this, callerURL);
            thread.start();
        }

    }

    public void onQueryCallerInfoComplete(CallerInfo ci) {
        synchronized (mQueryCompleteGuard) {
            if (ci == null) {
                if (MyLog.DEBUG)
                    MyLog.d(TAG, "query callerinfo failed!");
                return;
            }
            if (MyLog.DEBUG)
                MyLog.d(TAG, "check unit: name: " + ci.name + " number: " + ci.phoneNumber);

            // add pending call log
            mIsQueryCompleted = true;
            if (MyLog.DEBUG)
                MyLog.d(TAG, "set mIsQueryCompleted true");
            if (mAddCallLogPending) {
                if (MyLog.DEBUG)
                    MyLog.d(TAG, "mAddCallLogPending = true, and addCalllog");
                addCallLog();
            }

            if (ci.name != null) {
                mCallerName = ci.name;
                // update name display
                if (mVideoCallScreen != null) {
                    // send a message to screen
                    mVideoCallScreen.mScreenHandler.obtainMessage(
                            VideoCallScreen.MSG_UPDATE_CALLERINFO).sendToTarget();
                }
            }

            if (mIsDialOrAnswer == false) {
                // ring for answer condition
                if (ci.contactRingtoneUri != null) {
                    // set name and block information
                    if (mRinger != null) {
                        mRinger.setCustomRingtoneUri(ci.contactRingtoneUri);
                    }
                } else {
                    if (mRinger != null) {
                        Uri profileRingTone = Settings.System.DEFAULT_RINGTONE_URI;
                        Log.e(TAG, "default ringtone from profile setting:" + profileRingTone);
                        if (this.phoneCtx != null) {
                            if (VTCallUtils.getRingtoneMode(this.phoneCtx)) {
                                Uri customUri = VTCallUtils.getRingtoneUri(this.phoneCtx);
                                if (customUri != null) {
                                    String[] projection = { MediaStore.Images.ImageColumns.DISPLAY_NAME };
                                    Cursor mCursor = this.getContentResolver().query(customUri,
                                            projection, null, null, null);

                                    if (mCursor != null && mCursor.moveToFirst()) {
                                        mCursor.close();
                                        profileRingTone = customUri;
                                    }
                                }
                            }
                        }
                        mRinger.setCustomRingtoneUri(profileRingTone);
                    }
                }
                // ring
                if (MyLog.DEBUG)
                    MyLog.d(TAG, "ring");
                startRing();
            }
            mVTInfoQueryState = VTQUERY_STATE_IDLE;
            mCallerInfo = ci;
        }// end of sych

    }

    void postWakeLockTimeoutMsg() {
        Message msg = mHandler.obtainMessage(EVENT_WAKE_LOCK_TIMEOUT);
        mHandler.sendMessageDelayed(msg, WAKE_LOCK_TIMEOUT);
    }

    public boolean isMute() {
        return mIsMute;
    }

    public boolean isBtHeadsetPlugged() {
        return bBtHeadsetPlugged;
    }

    /* interface to stop ring */
    void stopRing() {
        if (mRinger != null) {
            mRinger.stopRing();
            mRinger = null;
            VTServiceInterface.setAudioMode(mCtx, AudioManager.MODE_NORMAL);
        }
    }

    void startRing() {
        if (mRinger != null) {
            VTServiceInterface.setAudioMode(mCtx, AudioManager.MODE_RINGTONE);
            try {
                mRinger.ring();
            } catch (Exception ex) {
                Log.e(TAG, "StartRing error :" + ex.toString());
                VTServiceInterface.setAudioMode(mCtx, AudioManager.MODE_NORMAL);
            }
        }
    }

    boolean isQueryCompleted() {
        synchronized (mQueryCompleteGuard) {
            return mIsQueryCompleted;
        }
    }

    /* interfaces for get caller info */
    CallerInfo getCallerInfo() {
        synchronized (mQueryCompleteGuard) {
            if (mIsQueryCompleted)
                return mCallerInfo;
            else
                return null;
        }
    }

    /* interfaces to get callerinfo */
    String getCallerName() {
        synchronized (mQueryCompleteGuard) {
            return mCallerName;
        }
    }

    Bitmap getCallerPicture() {
        synchronized (mQueryCompleteGuard) {
            return mCallerPic;
        }
    }

    String getCallerNumber() {
        return mInputPhoneURL;
    }

    String getUserInput() {
        return mUserInputBuffer.toString();
    }

    // Special back-door for Loopback mode
    void setLoopBackConnection() {
        mConnection = getVTServiceInterface().loopback(this);
    }

    private void addCallLog() {
        if (MyLog.DEBUG)
            MyLog.d(TAG, "addCallLog..");

        if (mIsCallLogAdded) {
            if (MyLog.DEBUG)
                MyLog.d(TAG, "call log is added");
            return;
        }

        mIsCallLogAdded = true;

        CallerInfo info = this.getCallerInfo();

        if (info == null) {
            if (MyLog.DEBUG)
                MyLog.d(TAG, "Info is null.");
            info = new CallerInfo();
            info.phoneNumber = mInputPhoneURL;
        }

        boolean isPrivateNumber = false; // TODO: need API for isPrivate()
        int duration = 0;
        if (MyLog.DEBUG)
            MyLog.d(TAG, "mCallLogType =" + mCallLogType);
        switch (mCallLogType) {
            case CallLog.Calls.MISSED_TYPE:
                duration = 0;
                break;
            case CallLog.Calls.INCOMING_TYPE:
            case CallLog.Calls.OUTGOING_TYPE:
                duration = mDuration;
                break;
            default:
                duration = 0;
                break;
        }

        final String _number = mInputPhoneURL;
        final int _presentation;

        if (TextUtils.isEmpty(_number)) {
            _presentation = PhoneConstants.PRESENTATION_UNKNOWN;

        } else if (_number.equals(this.getString(R.string.unknown_number))) {
            _presentation = PhoneConstants.PRESENTATION_RESTRICTED;
        } else {
            _presentation = PhoneConstants.PRESENTATION_ALLOWED;
        }

        final int _type = /* mCallLogType */mCallLogCSVTType;
        final long _start = mStartTime;
        final int _duration = (int) duration;// in seconds
        final int features = 1; //video
        final Long dataUsage = null;
        final PhoneAccountHandle accountHandle = null ;//call.getAccountHandle();

        if (MyLog.DEBUG)
            MyLog.d(TAG, "mStartTime: " + mStartTime);

        final boolean isNew;
        if (mCallLogType == CallLog.Calls.MISSED_TYPE)
               {// && mMissedCause == Connection.DisconnectCause.INCOMING_MISSED) {
            // for rejected missed call,we clear it's new field,thus do
            // not notify server
            isNew = true;
        } else {
            isNew = false;
        }
        Thread logThread = new Thread() {
            public void run() {
                if (MyLog.DEBUG)
                    MyLog.d(TAG, "create a new thread to add csvt Calllog");

                CallerInfo info = getCallerInfo();
                Uri uri = Calls.addCall( info,mCtx, _number, _presentation, _type,
                        features,accountHandle,_start,_duration,dataUsage);

                if (MyLog.DEBUG)
                    MyLog.d(TAG, "new call log uri: " + uri);
                if (!isNew) {
                    final ContentResolver resolver = sMe.getContentResolver();
                    ContentValues values = new ContentValues();
                    values.put("new", Integer.valueOf(0));
                   // resolver.update(uri, values, null, null);
                }

                // if (NOTIFY_SL) {
                // addToSmartLearner(info.currentInfo, record.start,
                // record.duration);
                // }
            }
        };
        logThread.start();

        if (MyLog.DEBUG)
            MyLog.d(TAG, "add miss call notification");
        if (mCallLogType == CallLog.Calls.MISSED_TYPE && isNew) {
            VTNotificationManager nm = VTNotificationManager.getDefault();
            nm.prepareToNotifyVTMissedCall(info.name, mInputPhoneURL, info.numberLabel, _start);
        }

    }

    private void dealWithUserInput(String input) {
        if (MyLog.DEBUG)
            MyLog.d(TAG, "deal with user input: " + input);
        int i = 0;
        int len = input.length();
        char ch = 0;

        if (input.equals(MSG_MULTI_RINGTONE_START)) {
            // multimedia ringtone start
            if (MyLog.DEBUG)
                MyLog.d(TAG, "start multimedia ringtone");
            mIsMMRingtoneStart = true;
            mCallTime.cancelTimer();
            mCallTimeElapsed = "00:00";
            if (mVideoCallScreen != null) {
                mVideoCallScreen.mScreenHandler.obtainMessage(VideoCallScreen.MSG_TICK_UPDATE)
                        .sendToTarget();
            }

        } else if ((mIsHardwareH324MStack && input.equals(MSG_MULTI_RINGTONE_END))
                || (!mIsHardwareH324MStack && input.equals(MSG_MULTI_RINGTONE_END_OLD))) {
            // multimedia ringtone start
            if (MyLog.DEBUG)
                MyLog.d(TAG, "stop multimedia ringtong");
            mIsMMRingtoneStart = false;
            mCallTime.startTimer();
            mStartTime = System.currentTimeMillis();
            // update notification.
            mHandler.sendMessageDelayed(mHandler.obtainMessage(MSG_NOTIFITY_INCALL), 3000);
        } else if (input.equals(MSG_CLOSE_CAMERA)) {
            // remote close camera
            if (MyLog.DEBUG)
                MyLog.d(TAG, "remote close camera");
            Toast.makeText(this, this.getString(R.string.remote_close_camera), Toast.LENGTH_SHORT)
                    .show();
        } else if (input.equals(MSG_OPEN_CAMERA)) {
            // remote open camera
            if (MyLog.DEBUG)
                MyLog.d(TAG, "remote open camera");
            Toast.makeText(this, this.getString(R.string.remote_open_camera), Toast.LENGTH_SHORT)
                    .show();
        } else if ((mVideoCallScreen != null) && (input.length() > 0)) {
            mVideoCallScreen.startLocalToneIfNeeded(input.charAt(0));
        }
    }

    private VTAppReceiver mAppReceiver = new VTAppReceiver();

    void registerVTAppReceiver() {

        if (MyLog.DEBUG)
            MyLog.d(TAG, "register headset receiver");
        IntentFilter filter = new IntentFilter();
        filter.addAction(Intent.ACTION_HEADSET_PLUG);
        // b413 ACTION_CONNECTION_STATE_CHANGED or ACTION_AUDIO_STATE_CHANGED
        filter.addAction(BluetoothHeadset.ACTION_CONNECTION_STATE_CHANGED);
        filter.addAction(Intent.ACTION_SCREEN_OFF);
        filter.addAction(Intent.ACTION_SCREEN_ON);
        // filter.addAction(Intent.ACTION_FLIP_CHANGED);
        registerReceiver(mAppReceiver, filter);
        IntentFilter filter2 = new IntentFilter();
        filter2.addAction(Intent.ACTION_MEDIA_EJECT);
        filter2.addAction("android.intent.action.MEDIA_KILL_ALL");
        filter2.addAction(Intent.ACTION_MEDIA_UNMOUNTED);
        filter2.addAction(Intent.ACTION_MEDIA_MOUNTED);
        filter2.addAction(Intent.ACTION_MEDIA_CHECKING);
        filter2.addDataScheme("file");
        registerReceiver(VTMediaAmountReceiver, filter2);

        IntentFilter filter3 = new IntentFilter();
        filter3.addAction("end_video_call");
        registerReceiver(VTEndCallReceiver, filter3);

        IntentFilter filter4 = new IntentFilter();
        filter4.addAction("restore_video_call");
        registerReceiver(VTRestoreVideoCall, filter4);

        IntentFilter mediaButtonIntentFilter = new IntentFilter(Intent.ACTION_MEDIA_BUTTON);
        //
        // Make sure we're higher priority than the media player's
        // MediaButtonIntentReceiver (which currently has the default
        // priority of zero; see apps/Music/AndroidManifest.xml.)
        mediaButtonIntentFilter.setPriority(1);
        // No media button receiver anymore, because Phone application support
        // MediaButton related internally
        // registerReceiver( mMediaButtonReceiver, mediaButtonIntentFilter);
    }

    void unregisterVTAppReceiver() {
        if (MyLog.DEBUG)
            MyLog.d(TAG, "unregister MediaunAmount receiver");
        unregisterReceiver(VTMediaAmountReceiver);
        if (MyLog.DEBUG)
            MyLog.d(TAG, "unregister headset receiver");
        unregisterReceiver(mAppReceiver);
        unregisterReceiver(VTEndCallReceiver);
        unregisterReceiver(VTRestoreVideoCall);
        // unregisterReceiver( mMediaButtonReceiver);
    }


    public final BroadcastReceiver VTRestoreVideoCall = new BroadcastReceiver() {

        @Override
        public void onReceive(Context context, Intent intent) {
            if (mVideoCallScreen != null) {
                if (mVideoCallScreen.hasStop) {
                    setResultData("finish");
                    Intent i = constuctRestoreIntent();
                    i.addFlags(Intent.FLAG_ACTIVITY_REORDER_TO_FRONT);
                    startActivity(i);
                }
            }

        }
    };

    public final BroadcastReceiver VTMediaAmountReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            if (action.equals(Intent.ACTION_MEDIA_EJECT)
                    || action.equals("android.intent.action.MEDIA_KILL_ALL")) {
                if (MyLog.DEBUG)
                    MyLog.d(TAG, "mReceiver:ACTION_MEDIA_EJECT sd card unavailable");
                int sdcardStatus = VTCallUtils.checkStorage();
                if (VTCallUtils.SDCARD_OK != sdcardStatus) {
                    VTCallUtils.showStorageToast(sdcardStatus, context);
                }
                stopAudioRecord();
                if (mVideoCallScreen != null && mVideoCallScreen.mScreenHandler != null)
                    mVideoCallScreen.mScreenHandler
                            .sendEmptyMessage(VideoCallScreen.MSG_STOP_RECORD);
            }
        }
    };

    public final BroadcastReceiver VTEndCallReceiver = new BroadcastReceiver() {

        @Override
        public void onReceive(Context context, Intent intent) {
            mVideoCallScreen.onEndCall();

        }
    };

    protected class InCallTonePlayer extends Thread {
        // The tone state
        static final int TONE_OFF = 0;
        static final int TONE_ON = 1;
        static final int TONE_STOPPED = 2;

        private int mState = TONE_OFF;

        @Override
        public void run() {
            ToneGenerator toneGenerator = new ToneGenerator(AudioManager.STREAM_VOICE_CALL, 80);
            synchronized (this) {
                if (mState != TONE_STOPPED) {
                    mState = TONE_ON;
                    toneGenerator.startTone(ToneGenerator.TONE_SUP_RINGTONE);
                    try {
                        wait(Integer.MAX_VALUE);
                    } catch (InterruptedException e) {
                        if (MyLog.DEBUG)
                            MyLog.d(TAG, "InCallTonePlayer: Stopped: " + e);
                    }
                    toneGenerator.stopTone();
                }
                if (MyLog.DEBUG)
                    MyLog.d(TAG, "InCallTonePlayer: Complete playing");
                toneGenerator.release();
                mState = TONE_OFF;
            }
        }

        public void stopTone() {
            synchronized (this) {
                if (mState == TONE_ON) {
                    notify();
                }
                mState = TONE_STOPPED;
            }
        }
    }

    /**
     * Start or stop playing the ringback tone when needed.
     */
    public void playInCallRingbackTone(Boolean playTone) {
        if (playTone) {
            // Only play when foreground call is in ALERTING state
            // to prevent a late coming playtone after ALERTING.
            // Don't play ringback tone if it is in play, otherwise it will cut
            // the current tone and replay it.
            if (mInCallRingbackTonePlayer == null) {
                mInCallRingbackTonePlayer = new InCallTonePlayer();
                mInCallRingbackTonePlayer.start();
            }
        } else {
            if (mInCallRingbackTonePlayer != null) {
                mInCallRingbackTonePlayer.stopTone();
                mInCallRingbackTonePlayer = null;
            }
        }
    }

    /**
     * Broadcast receiver for the ACTION_MEDIA_BUTTON broadcast intent.
     *
     * This functionality isn't lumped in with the other intents in
     * VTAppReceiver because we instantiate this as a totally separate
     * BroadcastReceiver instance, since we need to manually adjust its
     * IntentFilter's priority (to make sure we get these intents *before* the
     * media player.)
     */
    /*
     * private class MediaButtonReceiver extends BroadcastReceiver {
     *
     * private boolean mHeadsetDownProcessed = false; private static final int
     * HEADSET_KEY_LONG_PRESS_REPEAT = 4; static final String TAG =
     * "MediaButtonReceiver:";
     *
     * @Override public synchronized void onReceive(Context context, Intent
     * intent) {
     *
     * final VideoCallApp mApp = VideoCallApp.getInstance(); final Handler
     * mAppHandler = VideoCallApp.getInstance().mHandler;
     *
     * KeyEvent event = (KeyEvent)
     * intent.getParcelableExtra(Intent.EXTRA_KEY_EVENT); if (MyLog.DEBUG)
     * MyLog.d(TAG, "MediaButtonBroadcastReceiver.onReceive()...  event = " +
     * event); if( event == null) return;
     *
     * if ( (event.getKeyCode() == KeyEvent.KEYCODE_HEADSETHOOK)) {
     * if(event.getAction() == KeyEvent.ACTION_DOWN) { boolean longPress =
     * event.getRepeatCount() >= HEADSET_KEY_LONG_PRESS_REPEAT; if (MyLog.DEBUG)
     * MyLog.d(TAG, "handleHeadsetHook, longPress: " + longPress); if (
     * !mHeadsetDownProcessed && longPress ) { mAppHandler.obtainMessage(
     * VideoCallApp.MSG_HEADSET_SETHOOK_LONGPRESS).sendToTarget();
     * mHeadsetDownProcessed = true; } } else if( event.getAction() ==
     * KeyEvent.ACTION_UP) { if (MyLog.DEBUG) MyLog.d(TAG, "head set key up");
     * if (!mHeadsetDownProcessed) { mAppHandler.obtainMessage(
     * VideoCallApp.MSG_HEADSET_SETHOOK).sendToTarget(); } mHeadsetDownProcessed
     * = false; }
     *
     * //the intent is consumed already, just destroy it //abortBroadcast(); } }
     * }
     */
} // end of VideoCallApp class definition
