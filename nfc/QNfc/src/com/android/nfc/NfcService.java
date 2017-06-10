/*
 * Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 * Not a Contribution.
 * Apache license notifications and license are retained
 * for attribution purposes only.
 */

/*
 * Copyright (C) 2010 The Android Open Source Project
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

package com.android.nfc;

import com.android.nfc.Debug;
// <DTA>
import com.android.nfc.dhimpl.NativeNfcTag;
import android.nfc.NdefRecord;
import com.android.nfc.snep.SnepClient;
import com.android.nfc.snep.SnepServer;
import com.android.nfc.snep.SnepMessage;
import android.widget.Toast;
import java.util.Map;
// </DTA>
import android.app.ActivityManager;
import android.app.Application;
import android.app.KeyguardManager;
import android.app.PendingIntent;
import android.app.Service;
import android.app.admin.DevicePolicyManager;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.ContentResolver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.SharedPreferences;
import android.content.pm.IPackageManager;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.content.pm.UserInfo;
import android.content.res.Resources.NotFoundException;
import android.media.AudioManager;
import android.media.SoundPool;
import android.nfc.BeamShareData;
import android.nfc.ErrorCodes;
import android.nfc.FormatException;
import android.nfc.IAppCallback;
import android.nfc.INfcAdapter;
import android.nfc.INfcAdapterExtras;
import android.nfc.INfcCardEmulation;
import android.nfc.INfcTag;
//<DTA>
import android.nfc.dta.INfcTagDta;
import android.nfc.dta.IDtaHelper;
import android.nfc.dta.DtaHelper;
//import android.nfc.dta.NdefMessage; //TODO: Add this for SNEP
//</DTA>
import android.nfc.INfcUnlockHandler;
import android.nfc.NdefMessage;
import android.nfc.NfcAdapter;
import android.nfc.Tag;
import java.io.IOException;
import android.nfc.dta.TagDta; //<DTA>
import android.nfc.TechListParcel;
import android.nfc.TransceiveResult;
import android.nfc.tech.Ndef;
import android.nfc.tech.TagTechnology;
import android.os.AsyncTask;
import android.os.Binder;
import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.os.PowerManager;
import android.os.Process;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.os.UserHandle;
import android.os.UserManager;
import android.provider.Settings;

import com.android.nfc.DeviceHost.DeviceHostListener;
import com.android.nfc.DeviceHost.LlcpConnectionlessSocket;
import com.android.nfc.DeviceHost.LlcpServerSocket;
import com.android.nfc.DeviceHost.LlcpSocket;
import com.android.nfc.DeviceHost.NfcDepEndpoint;
import com.android.nfc.DeviceHost.TagEndpoint;
import com.android.nfc.cardemulation.CardEmulationManager;
import com.android.nfc.dhimpl.NativeNfcManager;
import com.android.nfc.handover.HandoverDataParser;

import java.io.FileDescriptor;
import java.io.PrintWriter;
import java.util.Arrays;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.NoSuchElementException;
import android.util.Log;

//NFCC info to nfc app
import android.nfc.IGetNFCByteArray;
//AID filter
import android.os.SystemProperties;

public class NfcService implements DeviceHostListener {
    static final boolean DBG = Debug.NfcService;
    static final String TAG = "NfcService";

    public static final String SERVICE_NAME = "nfc";

    public static final String PREF = "NfcServicePrefs";

    static final String PREF_NFC_ON = "nfc_on";
    static final boolean NFC_ON_DEFAULT = true;
    static final String PREF_NDEF_PUSH_ON = "ndef_push_on";
    static final boolean NDEF_PUSH_ON_DEFAULT = true;
    static final String PREF_FIRST_BEAM = "first_beam";
    static final String PREF_FIRST_BOOT = "first_boot";
    static final String PREF_AIRPLANE_OVERRIDE = "airplane_override";

    static final int MSG_NDEF_TAG = 0;
    static final int MSG_LLCP_LINK_ACTIVATION = 1;
    static final int MSG_LLCP_LINK_DEACTIVATED = 2;
    static final int MSG_MOCK_NDEF = 3;
    static final int MSG_LLCP_LINK_FIRST_PACKET = 4;
    static final int MSG_ROUTE_AID = 5;
    static final int MSG_UNROUTE_AID = 6;
    static final int MSG_COMMIT_ROUTING = 7;
    static final int MSG_INVOKE_BEAM = 8;
    static final int MSG_RF_FIELD_ACTIVATED = 9;
    static final int MSG_RF_FIELD_DEACTIVATED = 10;
    static final int MSG_UPDATE_COMMIT_DEFAULT_ROUTE = 11;
    static final int MSG_REQUEST_RESTART_NFC = 12;
    static final int MSG_EXTENTION_MSG =20;
    static final int MSG_HCI_EVT_CONNECTIVITY = 25;
    static final int MSG_RESUME_POLLING = 26;

    static final long MAX_POLLING_PAUSE_TIMEOUT = 40000;

    static final int TASK_ENABLE = 1;
    static final int TASK_DISABLE = 2;
    static final int TASK_BOOT = 3;

    // UI state of connectivity gate defined by HCI
    static final int HCI_UI_STATE_UNKNOWN = 0;
    static final int HCI_UI_STATE_AVAILABLE = 1;
    static final int HCI_UI_STATE_LOCKED_NOT_NOTIFIABLE = 2;
    static final int HCI_UI_STATE_LOCKED_NOTIFIABLE = 3;
    static final int HCI_UI_STATE_UNLOCKED_NOT_NOTIFIABLE = 4;

    // UI state for NFCC
    static final int NFCC_UI_STATE_OFF = 0;
    static final int NFCC_UI_STATE_UNLOCKED = 1;
    static final int NFCC_UI_STATE_LOCKED = 2;

    // Polling technology masks
    static final int NFC_POLL_A = 0x01;
    static final int NFC_POLL_B = 0x02;
    static final int NFC_POLL_F = 0x04;
    static final int NFC_POLL_ISO15693 = 0x08;
    static final int NFC_POLL_B_PRIME = 0x10;
    static final int NFC_POLL_KOVIO = 0x20;

    //keep in sync with below
    //packages/apps/Stk/src/com/android/stk/StkAppService.java
    private static final String SLOT_ID_KEY_STRING = "slot_id";

    //frameworks/opt/telephony/src/java/com/android/internal/telephony/cat/AppInterface.java
    private static final String CAT_ACTIVATE_NOTIFY_ACTION =
                                    "org.codeaurora.intent.action.stk.activate_notify";
    private static final String CAT_HCI_CONNECTIVITY_ACTION =
                                    "org.codeaurora.intent.action.stk.hci_connectivity";

    // minimum screen state that enables NFC polling
    static final int NFC_POLLING_MODE = ScreenStateHelper.SCREEN_STATE_ON_UNLOCKED;

    // Time to wait for NFC controller to initialize before watchdog
    // goes off. This time is chosen large, because firmware download
    // may be a part of initialization.
    static final int INIT_WATCHDOG_MS = 90000;

    // Time to wait for routing to be applied before watchdog
    // goes off
    static final int ROUTING_WATCHDOG_MS = 10000;

    // Default delay used for presence checks
    static final int DEFAULT_PRESENCE_CHECK_DELAY = 125;
    // Default Delay used for mifare classic presence check delay
    static final int MIFARE_CLASSIC_PRESENCE_CHECK_DELAY_MS = 500;

    // The amount of time we wait before manually launching
    // the Beam animation when called through the share menu.
    static final int INVOKE_BEAM_DELAY_MS = 1000;

    // RF field events as defined in NFC extras
    public static final String ACTION_RF_FIELD_ON_DETECTED =
            "com.android.nfc_extras.action.RF_FIELD_ON_DETECTED";
    public static final String ACTION_RF_FIELD_OFF_DETECTED =
            "com.android.nfc_extras.action.RF_FIELD_OFF_DETECTED";

    // for use with playSound()
    public static final int SOUND_START = 0;
    public static final int SOUND_END = 1;
    public static final int SOUND_ERROR = 2;

    public static final String ACTION_LLCP_UP =
            "com.android.nfc.action.LLCP_UP";

    public static final String ACTION_LLCP_DOWN =
            "com.android.nfc.action.LLCP_DOWN";

    // Timeout to re-apply routing if a tag was present and we postponed it
    private static final int APPLY_ROUTING_RETRY_TIMEOUT_MS = 5000;

    private final UserManager mUserManager;

    // NFC Execution Environment
    // fields below are protected by this
    private final ReaderModeDeathRecipient mReaderModeDeathRecipient =
            new ReaderModeDeathRecipient();
    private final NfcUnlockManager mNfcUnlockManager = NfcUnlockManager.getInstance();

    private boolean mDiscoveryEnabled = false;

    private final NfceeAccessControl mNfceeAccessControl;

    List<PackageInfo> mInstalledPackages; // cached version of installed packages

    // fields below are used in multiple threads and protected by synchronized(this)
    final HashMap<Integer, Object> mObjectMap = new HashMap<Integer, Object>();
    int mScreenState;
    int mOldScreenState;
    int mHciUiState;             // UI State defined by HCI spec
    int mNfccUiState;            // UI state notified to NFCC
    boolean mInProvisionMode; // whether we're in setup wizard and enabled NFC provisioning
    boolean mLockScreenPolling = false;
    boolean mIsNdefPushEnabled;
    NfcDiscoveryParameters mCurrentDiscoveryParameters;
    byte[] nfcc_info;            // Container of nfcc info reported by ncihal
    ReaderModeParams mReaderModeParams;

    // mState is protected by this, however it is only modified in onCreate()
    // and the default AsyncTask thread so it is read unprotected from that
    // thread
    int mState;  // one of NfcAdapter.STATE_ON, STATE_TURNING_ON, etc
    int mIsNfcDisabledReason;
    boolean mIsUpdatingDiscoveryDuringShuttingDown = false;

    // TO keep track who is disabling nfc
    static final int NFC_DISABLED_BY_SYSTEM = 0;
    static final int NFC_DISABLED_BY_USER = 1;
    static final int NFC_DISABLED_BY_AIRPLANEMODE = 2;
    boolean mNfcOnWhileAirplaneModeOn = false;

    // fields below are final after onCreate()
    Context mContext;
    private DeviceHost mDeviceHost;
    private SharedPreferences mPrefs;
    private SharedPreferences.Editor mPrefsEditor;
    private PowerManager.WakeLock mRoutingWakeLock;
    private PowerManager.WakeLock mScreenOffCmdWakeLock;
    private PowerManager.WakeLock mBertWakeLock;

    int mStartSound;
    int mEndSound;
    int mErrorSound;
    SoundPool mSoundPool; // playback synchronized on this
    P2pLinkManager mP2pLinkManager;
    TagService mNfcTagService;
    TagServiceDta mNfcTagServiceDta; // <DTA>
    NfcAdapterService mNfcAdapter;
    DtaHelperService mDtaHelperService; // <DTA>
    boolean mIsAirplaneSensitive;
    boolean mIsAirplaneToggleable;
    boolean mIsDebugBuild;
    boolean mIsHceCapable;
    boolean mPollingPaused;

    NfcDispatcher mNfcDispatcher;
    private PowerManager mPowerManager;
    private KeyguardManager mKeyguard;
    private HandoverDataParser mHandoverDataParser;
    private ContentResolver mContentResolver;
    CardEmulationManager mCardEmulationManager;

    private ScreenStateHelper mScreenStateHelper;
    private ForegroundUtils mForegroundUtils;

    private int mUserId;
    private static NfcService sService;
    // <DTA>
    private final HashMap<Integer, SnepClient> mSnepClients = new HashMap<Integer, SnepClient>();
    private final HashMap<Integer, SnepServer> mSnepServers = new HashMap<Integer, SnepServer>();
    private int mSnepClientHandleCounter = 0;
    private int mSnepServerHandleCounter = 0;
    private static final String DTA_PATTERN = "sys.dtapattern";
    // </DTA>
    private BertTask mBertTask;


    static final String PREF_ACTIVE_SECURE_ELEMENT = "active_secure_element";
    String  mActiveSecureElement = null; // current active secure element name
    static final String PREF_DEFAULT_ROUTE = "default_route";

    private boolean isAidFilterEnabled = false;

    public static NfcService getInstance() {
        return sService;
    }

    public DtaHelperService getDtaHelperService() {
        return mDtaHelperService;
    }

    @Override
    public void onRemoteEndpointDiscovered(TagEndpoint tag) {
        sendMessage(NfcService.MSG_NDEF_TAG, tag);
    }

    /**
     * Notifies transaction
     */
    @Override
    public void onCardEmulationDeselected() {
        Log.d(TAG,"onCardEmulationDeselected dummy implementation");
    }


    /**
     * Notifies transaction
     */
    @Override
    public void onCardEmulationAidSelected(byte[] dataBuf) {
        Log.d(TAG,"onCardEmulationAidSelected dummy implementation");
    }

    /**
     * Notifies HCI Event Connectivity (slot 0:SIM1, slot 1:SIM2)
     */
    @Override
    public void onCardEmulationHciEvtConnectivity(int slotId) {
        Message msg = mHandler.obtainMessage();
        msg.what = MSG_HCI_EVT_CONNECTIVITY;
        msg.arg1 = slotId;
        mHandler.sendMessage(msg);
    }

    /**
     * Notifies transaction
     */
    @Override
    public void onHostCardEmulationActivated() {
        if (mCardEmulationManager != null) {
            mCardEmulationManager.onHostCardEmulationActivated();
        }
    }

    @Override
    public void onHostCardEmulationData(byte[] data) {
        if (mCardEmulationManager != null) {
            mCardEmulationManager.onHostCardEmulationData(data);
        }
    }

    @Override
    public void onHostCardEmulationDeactivated() {
        if (mCardEmulationManager != null) {
            mCardEmulationManager.onHostCardEmulationDeactivated();
        }
    }

    /**
     * Notifies P2P Device detected, to activate LLCP link
     */
    @Override
    public void onLlcpLinkActivated(NfcDepEndpoint device) {
        sendMessage(NfcService.MSG_LLCP_LINK_ACTIVATION, device);
    }

    /**
     * Notifies P2P Device detected, to activate LLCP link
     */
    @Override
    public void onLlcpLinkDeactivated(NfcDepEndpoint device) {
        sendMessage(NfcService.MSG_LLCP_LINK_DEACTIVATED, device);
    }

    /**
     * Notifies P2P Device detected, first packet received over LLCP link
     */
    @Override
    public void onLlcpFirstPacketReceived(NfcDepEndpoint device) {
        sendMessage(NfcService.MSG_LLCP_LINK_FIRST_PACKET, device);
    }

    @Override
    public void onRemoteFieldActivated() {
        sendMessage(NfcService.MSG_RF_FIELD_ACTIVATED, null);
    }

    public void onRemoteFieldDeactivated() {
        sendMessage(NfcService.MSG_RF_FIELD_DEACTIVATED, null);
    }

    @Override
    public void onNfccInit(byte[] nfccinfo) {
        nfcc_info = Arrays.copyOf(nfccinfo, nfccinfo.length);
    }

    @Override
    public void noRequestRestartNfc() {
        sendMessage(NfcService.MSG_REQUEST_RESTART_NFC, null);
    }

    final class ReaderModeParams {
        public int flags;
        public IAppCallback callback;
        public int presenceCheckDelay;
    }

    private NfcService(Context context)
    {
        mContext = context;
        mNfceeAccessControl = new NfceeAccessControl(context);
        mUserManager = (UserManager) mContext.getSystemService(Context.USER_SERVICE);
    }

    public NfcService(Application nfcApplication) {
        this((Context) nfcApplication);
        init(nfcApplication, new NativeNfcManager(nfcApplication,this));
        ServiceManager.addService(SERVICE_NAME, mNfcAdapter);
    }

    public NfcService(Application nfcApplication, DeviceHost host, NfcServiceExtentionHandler extention){
        this((Context) nfcApplication);
        mExtentionHandler = extention;
        mExtentionHandler.setHandler(mHandler);
        init(nfcApplication, host);
    }
    public NfcService(Application nfcApplication, DeviceHost host) {
        this((Context) nfcApplication);
        init(nfcApplication, host);
    }
    private void init(Application nfcApplication, DeviceHost host)
    {
        mUserId = ActivityManager.getCurrentUser();

        mNfcTagService = new TagService();
        mNfcTagServiceDta = new TagServiceDta();
        mNfcAdapter = new NfcAdapterService();
        mDtaHelperService = new DtaHelperService();

        Log.i(TAG, "Starting NFC service");

        sService = this;

        mScreenStateHelper = new ScreenStateHelper(mContext);
        mContentResolver = mContext.getContentResolver();
        mDeviceHost = host;

        // <DTA> Enable DTA mode by default by setting pattern number to 0
        // To disable dta mode by default, set last_pattern number to -1.
        int last_pattern = mDeviceHost.dta_get_pattern_number();
        Log.d(TAG,"NFC service start: setting pattern number to:" + last_pattern);
        System.setProperty(DTA_PATTERN, String.valueOf(last_pattern));
        // </DTA>

        mHandoverDataParser = new HandoverDataParser();
        boolean isNfcProvisioningEnabled = false;
        try {
            isNfcProvisioningEnabled = mContext.getResources().getBoolean(
                    R.bool.enable_nfc_provisioning);
        } catch (NotFoundException e) {
        }

        if (isNfcProvisioningEnabled) {
            mInProvisionMode = Settings.Secure.getInt(mContentResolver,
                    Settings.Global.DEVICE_PROVISIONED, 0) == 0;
        } else {
            mInProvisionMode = false;
        }

        mNfcDispatcher = new NfcDispatcher(mContext, mHandoverDataParser, mInProvisionMode);
        mP2pLinkManager = new P2pLinkManager(mContext, mHandoverDataParser,
                mDeviceHost.getDefaultLlcpMiu(), mDeviceHost.getDefaultLlcpRwSize());

        mPrefs = mContext.getSharedPreferences(PREF, Context.MODE_PRIVATE);
        mPrefsEditor = mPrefs.edit();

        mState = NfcAdapter.STATE_OFF;
        mIsNdefPushEnabled = mPrefs.getBoolean(PREF_NDEF_PUSH_ON, NDEF_PUSH_ON_DEFAULT);
        setBeamShareActivityState(mIsNdefPushEnabled);

        mIsDebugBuild = "userdebug".equals(Build.TYPE) || "eng".equals(Build.TYPE);

        mPowerManager = (PowerManager) mContext.getSystemService(Context.POWER_SERVICE);

        mRoutingWakeLock = mPowerManager.newWakeLock(
            PowerManager.PARTIAL_WAKE_LOCK, "NfcService:mRoutingWakeLock");
        mScreenOffCmdWakeLock = mPowerManager.newWakeLock(
            PowerManager.PARTIAL_WAKE_LOCK, "NfcService:mScreenOffCmdWakeLock");
        mBertWakeLock = mPowerManager.newWakeLock(
            PowerManager.PARTIAL_WAKE_LOCK, "NfcService:mBertWakeLock");

        mKeyguard = (KeyguardManager) mContext.getSystemService(Context.KEYGUARD_SERVICE);

        mScreenState = mScreenStateHelper.checkScreenState();
        mOldScreenState = mScreenState;
        mHciUiState = HCI_UI_STATE_UNKNOWN;
        mNfccUiState = NFCC_UI_STATE_LOCKED;


        // Intents for all users
        IntentFilter filter = new IntentFilter(Intent.ACTION_SCREEN_OFF);
        filter.addAction(Intent.ACTION_SCREEN_ON);
        filter.addAction(Intent.ACTION_USER_PRESENT);
        filter.addAction(Intent.ACTION_USER_SWITCHED);
        registerForAirplaneMode(filter);
        mContext.registerReceiverAsUser(mReceiver, UserHandle.ALL, filter, null, null);

        IntentFilter ownerFilter = new IntentFilter(Intent.ACTION_EXTERNAL_APPLICATIONS_AVAILABLE);
        ownerFilter.addAction(Intent.ACTION_EXTERNAL_APPLICATIONS_UNAVAILABLE);
        mContext.registerReceiver(mOwnerReceiver, ownerFilter);

        ownerFilter = new IntentFilter();
        ownerFilter.addAction(Intent.ACTION_PACKAGE_ADDED);
        ownerFilter.addAction(Intent.ACTION_PACKAGE_REMOVED);
        ownerFilter.addDataScheme("package");
        mContext.registerReceiver(mOwnerReceiver, ownerFilter);

        IntentFilter policyFilter = new IntentFilter(DevicePolicyManager.ACTION_DEVICE_POLICY_MANAGER_STATE_CHANGED);
        mContext.registerReceiverAsUser(mPolicyReceiver, UserHandle.ALL, policyFilter, null, null);

        updatePackageCache();

        IntentFilter mProactiveFilter = new IntentFilter();
        mProactiveFilter.addAction(CAT_ACTIVATE_NOTIFY_ACTION);
        mContext.registerReceiver(mProactiveReceiver, mProactiveFilter);

        IntentFilter enableAfterUserConfirm = new IntentFilter();
        enableAfterUserConfirm.addAction("com.android.nfc.action.ALLOW_NFC_ENABLE");
        enableAfterUserConfirm.addAction("com.android.nfc.action.DENY_NFC_ENABLE");
        mContext.registerReceiver(enableReceiver,
                  enableAfterUserConfirm, "android.permission.NFC_ENABLE", null);

        PackageManager pm = mContext.getPackageManager();

        //Initialise NFCEE IDs mapping to Secure Elements by reading conf file
        Log.i(TAG, "Init NfceeIds mapping to SecureElements from Conf File");
        mDeviceHost.initNfceeIdSeMap();

        String seNameForDefaultRoute = mPrefs.getString(PREF_DEFAULT_ROUTE, "TBD");
        if (!seNameForDefaultRoute.equals("TBD")) {
            mDeviceHost.setDefaultRoute(seNameForDefaultRoute) ;
        }

        mIsHceCapable = pm.hasSystemFeature(PackageManager.FEATURE_NFC_HOST_CARD_EMULATION);
        if (mIsHceCapable) {
            mCardEmulationManager = new CardEmulationManager(mContext);
        }
        mForegroundUtils = ForegroundUtils.getInstance();

        String isisConfig = SystemProperties.get("persist.nfc.smartcard.isis");
        if ((isisConfig != null) && (isisConfig.equals("verizon"))) {
            isAidFilterEnabled = true;
        }

        new EnableDisableTask().execute(TASK_BOOT);  // do blocking boot tasks
    }

    private final BroadcastReceiver enableReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            if(action.equals("com.android.nfc.action.ALLOW_NFC_ENABLE")){
               try{
                   if(mNfcAdapter.enable())
                     Log.d(TAG, "NFC_ENABLE_SUCCESS");
                   else
                     Log.e(TAG, "NFC_ENABLE_FAIL");
               } catch (RemoteException e) {
                   e.printStackTrace();
               }
            }
        }
    };

    void initSoundPool() {
        synchronized (this) {
            if (mSoundPool == null) {
                mSoundPool = new SoundPool(1, AudioManager.STREAM_NOTIFICATION, 0);
                mStartSound = mSoundPool.load(mContext, R.raw.start, 1);
                mEndSound = mSoundPool.load(mContext, R.raw.end, 1);
                mErrorSound = mSoundPool.load(mContext, R.raw.error, 1);
            }
        }
    }

    void releaseSoundPool() {
        synchronized (this) {
            if (mSoundPool != null) {
                mSoundPool.release();
                mSoundPool = null;
            }
        }
    }

    void registerForAirplaneMode(IntentFilter filter) {
        final String airplaneModeRadios = Settings.System.getString(mContentResolver,
                Settings.Global.AIRPLANE_MODE_RADIOS);
        final String toggleableRadios = Settings.System.getString(mContentResolver,
                Settings.Global.AIRPLANE_MODE_TOGGLEABLE_RADIOS);

        mIsAirplaneSensitive = airplaneModeRadios == null ? true :
                airplaneModeRadios.contains(Settings.Global.RADIO_NFC);
        mIsAirplaneToggleable = toggleableRadios == null ? false :
                toggleableRadios.contains(Settings.Global.RADIO_NFC);

        if (mIsAirplaneSensitive) {
            filter.addAction(Intent.ACTION_AIRPLANE_MODE_CHANGED);
        }
    }

    void updatePackageCache() {
        PackageManager pm = mContext.getPackageManager();
        List<PackageInfo> packages = pm.getInstalledPackages(0, UserHandle.USER_OWNER);
        synchronized (this) {
            mInstalledPackages = packages;
        }
    }

    /**
     * Manages tasks that involve turning on/off the NFC controller.
     * <p/>
     * <p>All work that might turn the NFC adapter on or off must be done
     * through this task, to keep the handling of mState simple.
     * In other words, mState is only modified in these tasks (and we
     * don't need a lock to read it in these tasks).
     * <p/>
     * <p>These tasks are all done on the same AsyncTask background
     * thread, so they are serialized. Each task may temporarily transition
     * mState to STATE_TURNING_OFF or STATE_TURNING_ON, but must exit in
     * either STATE_ON or STATE_OFF. This way each task can be guaranteed
     * of starting in either STATE_OFF or STATE_ON, without needing to hold
     * NfcService.this for the entire task.
     * <p/>
     * <p>AsyncTask's are also implicitly queued. This is useful for corner
     * cases like turning airplane mode on while TASK_ENABLE is in progress.
     * The TASK_DISABLE triggered by airplane mode will be correctly executed
     * immediately after TASK_ENABLE is complete. This seems like the most sane
     * way to deal with these situations.
     * <p/>
     * <p>{@link #TASK_ENABLE} enables the NFC adapter, without changing
     * preferences
     * <p>{@link #TASK_DISABLE} disables the NFC adapter, without changing
     * preferences
     * <p>{@link #TASK_BOOT} does first boot work and may enable NFC
     */
    class EnableDisableTask extends AsyncTask<Integer, Void, Void> {
        @Override
        protected Void doInBackground(Integer... params) {
            // Sanity check mState
            switch (mState) {
                case NfcAdapter.STATE_TURNING_OFF:
                case NfcAdapter.STATE_TURNING_ON:
                    Log.e(TAG, "Processing EnableDisable task " + params[0] + " from bad state " +
                            mState);
                    return null;
            }

            /* AsyncTask sets this thread to THREAD_PRIORITY_BACKGROUND,
             * override with the default. THREAD_PRIORITY_BACKGROUND causes
             * us to service software I2C too slow for firmware download
             * with the NXP PN544.
             * TODO: move this to the DAL I2C layer in libnfc-nxp, since this
             * problem only occurs on I2C platforms using PN544
             */
            Process.setThreadPriority(Process.THREAD_PRIORITY_DEFAULT);

            switch (params[0].intValue()) {
                case TASK_ENABLE:
                    enableInternal();
                    break;
                case TASK_DISABLE:
                    disableInternal();
                    break;
                case TASK_BOOT:
                    Log.d(TAG, "checking on firmware download");
                    boolean airplaneOverride = mPrefs.getBoolean(PREF_AIRPLANE_OVERRIDE, false);
                    if (mPrefs.getBoolean(PREF_NFC_ON, NFC_ON_DEFAULT) &&
                            (!mIsAirplaneSensitive || !isAirplaneModeOn() || airplaneOverride)) {
                        Log.d(TAG, "NFC is on. Doing normal stuff");
                        /*
                         *  NFC Service might have crashed,
                         *  announce "turning on/turning off/off"
                         */
                        updateState(NfcAdapter.STATE_TURNING_ON);
                        updateState(NfcAdapter.STATE_TURNING_OFF);
                        updateState(NfcAdapter.STATE_OFF);
                        enableInternal();
                    } else {
                        Log.d(TAG, "NFC is off.  Checking firmware version");
                        mDeviceHost.checkFirmware();
                    }
                    if (mPrefs.getBoolean(PREF_FIRST_BOOT, true)) {
                        Log.i(TAG, "First Boot");
                        mPrefsEditor.putBoolean(PREF_FIRST_BOOT, false);
                        mPrefsEditor.apply();
                    }
                    break;
            }

            // Restore default AsyncTask priority
            Process.setThreadPriority(Process.THREAD_PRIORITY_BACKGROUND);
            return null;
        }

        void initActiveSecureElement() {
            if(mDeviceHost.getEeRoutingReloadAtReboot()) {
                mActiveSecureElement = mDeviceHost.getDefaultActiveSecureElement();
                mPrefsEditor.putString(PREF_ACTIVE_SECURE_ELEMENT, mActiveSecureElement);

                Log.d(TAG, "getting mActiveSecureElement from conf file:" + mActiveSecureElement);
            }
            else {
                mActiveSecureElement = mPrefs.getString(PREF_ACTIVE_SECURE_ELEMENT,
                                                        mDeviceHost.getDefaultActiveSecureElement());
                Log.d(TAG, "getting mActiveSecureElement from mPrefsEditor:" + mActiveSecureElement);
            }

            Log.d(TAG, "recording mActiveSecureElement in mPrefsEditor:" + mActiveSecureElement);
            mPrefsEditor.putString(PREF_ACTIVE_SECURE_ELEMENT, mActiveSecureElement);
            mPrefsEditor.apply();
        }

        /**
         * Enable NFC adapter functions.
         * Does not toggle preferences.
         */
        boolean enableInternal() {
            if (mState == NfcAdapter.STATE_ON) {
                return true;
            }
            Log.i(TAG, "Enabling NFC");
            updateState(NfcAdapter.STATE_TURNING_ON);
            if(isAirplaneModeOn())
            {
                Log.d(TAG, "Airplane Mode is ON while initializing Nfc Service");
                mNfcOnWhileAirplaneModeOn = true;
            }

            initActiveSecureElement();

            WatchDogThread watchDog = new WatchDogThread("enableInternal", INIT_WATCHDOG_MS);
            watchDog.start();
            try {
                mRoutingWakeLock.acquire();
                try {
                    if (!mDeviceHost.initialize()) {
                        Log.w(TAG, "Error enabling NFC");
                        updateState(NfcAdapter.STATE_OFF);
                        return false;
                    }
                } finally {
                    mRoutingWakeLock.release();
                }
            } finally {
                watchDog.cancel();
            }

            synchronized (NfcService.this) {
                mObjectMap.clear();
                mP2pLinkManager.enableDisable(mIsNdefPushEnabled, true);
                updateState(NfcAdapter.STATE_ON);
            }

            initSoundPool();

            mHciUiState = HCI_UI_STATE_UNKNOWN; // let update UI state to NFCC
            mOldScreenState = mScreenState;     // let it configure discovery
            applyRouting(true);

            if (mIsHceCapable) {
                // Generate the initial card emulation routing table
                mCardEmulationManager.onNfcEnabled();
            }

            if (isAidFilterEnabled)
                mDeviceHost.initClfAidFilterList();

            return true;
        }

        /**
         * Disable all NFC adapter functions.
         * Does not toggle preferences.
         */
        boolean disableInternal() {
            if (mState == NfcAdapter.STATE_OFF) {
                return true;
            }
            Log.i(TAG, "Disabling NFC");
            updateState(NfcAdapter.STATE_TURNING_OFF);

            /* Sometimes mDeviceHost.deinitialize() hangs, use a watch-dog.
             * Implemented with a new thread (instead of a Handler or AsyncTask),
             * because the UI Thread and AsyncTask thread-pools can also get hung
             * when the NFC controller stops responding */
            WatchDogThread watchDog = new WatchDogThread("disableInternal", ROUTING_WATCHDOG_MS);
            watchDog.start();

            if (mIsHceCapable) {
                mCardEmulationManager.onNfcDisabled();
            }

            mP2pLinkManager.enableDisable(false, false);

            // Stop watchdog if tag present
            // A convenient way to stop the watchdog properly consists of
            // disconnecting the tag. The polling loop shall be stopped before
            // to avoid the tag being discovered again.
            maybeDisconnectTarget();

            mNfcDispatcher.setForegroundDispatch(null, null, null);
            if(isAirplaneModeOn() &&(!mNfcOnWhileAirplaneModeOn))
            {
                Log.d(TAG, "NFC_DISABLED_BY_AIRPLANEMODE : " + mIsNfcDisabledReason);
                mIsNfcDisabledReason = NFC_DISABLED_BY_AIRPLANEMODE;
            }
            mDeviceHost.nfcShutdownReason(mIsNfcDisabledReason);


            boolean result = mDeviceHost.deinitialize();
            if (DBG) Log.d(TAG, "mDeviceHost.deinitialize() = " + result);

            watchDog.cancel();

            updateState(NfcAdapter.STATE_OFF);

            if(mScreenOffCmdWakeLock.isHeld()) {
                if(DBG) Log.d(TAG, "release Wakelock");
                mScreenOffCmdWakeLock.release();
            }
            releaseSoundPool();

            return result;
        }

        void updateState(int newState) {
            synchronized (NfcService.this) {
                if (newState == mState) {
                    return;
                }
                mState = newState;
                Intent intent = new Intent(NfcAdapter.ACTION_ADAPTER_STATE_CHANGED);
                intent.setFlags(Intent.FLAG_RECEIVER_REGISTERED_ONLY_BEFORE_BOOT);
                intent.putExtra(NfcAdapter.EXTRA_ADAPTER_STATE, mState);
                mContext.sendBroadcastAsUser(intent, UserHandle.CURRENT);
            }
        }
    }

    void saveNfcOnSetting(boolean on) {
        synchronized (NfcService.this) {
            mPrefsEditor.putBoolean(PREF_NFC_ON, on);
            mPrefsEditor.apply();
        }
    }

    public void playSound(int sound) {
        synchronized (this) {
            if (mSoundPool == null) {
                Log.w(TAG, "Not playing sound when NFC is disabled");
                return;
            }
            switch (sound) {
                case SOUND_START:
                    mSoundPool.play(mStartSound, 1.0f, 1.0f, 0, 0, 1.0f);
                    break;
                case SOUND_END:
                    mSoundPool.play(mEndSound, 1.0f, 1.0f, 0, 0, 1.0f);
                    break;
                case SOUND_ERROR:
                    mSoundPool.play(mErrorSound, 1.0f, 1.0f, 0, 0, 1.0f);
                    break;
            }
        }
    }
        // </DTA>
    synchronized int getUserId() {
        return mUserId;
    }

    void setBeamShareActivityState(boolean enabled) {
        UserManager um = (UserManager) mContext.getSystemService(Context.USER_SERVICE);
        // Propagate the state change to all user profiles related to the current
        // user. Note that the list returned by getUserProfiles contains the
        // current user.
        List <UserHandle> luh = um.getUserProfiles();
        for (UserHandle uh : luh){
            enforceBeamShareActivityPolicy(mContext, uh, enabled);
        }
    }

    void enforceBeamShareActivityPolicy(Context context, UserHandle uh,
            boolean isGlobalEnabled){
        UserManager um = (UserManager) context.getSystemService(Context.USER_SERVICE);
        IPackageManager mIpm = IPackageManager.Stub.asInterface(ServiceManager.getService("package"));
        boolean isActiveForUser =
                (!um.hasUserRestriction(UserManager.DISALLOW_OUTGOING_BEAM, uh)) &&
                isGlobalEnabled;
        if (DBG){
            Log.d(TAG, "Enforcing a policy change on user: " + uh +
                    ", isActiveForUser = " + isActiveForUser);
        }
        try {
            mIpm.setComponentEnabledSetting(new ComponentName(
                    BeamShareActivity.class.getPackageName$(),
                    BeamShareActivity.class.getName()),
                    isActiveForUser ?
                            PackageManager.COMPONENT_ENABLED_STATE_ENABLED :
                            PackageManager.COMPONENT_ENABLED_STATE_DISABLED,
                            PackageManager.DONT_KILL_APP,
                    uh.getIdentifier());
        } catch (RemoteException e) {
            Log.w(TAG, "Unable to change Beam status for user " + uh);
        }
    }

    // <DTA>
    /*
     * This Service implements the API extensions needed in DTA mode
     * It will be accessed via the DtaService.
     * TODO: Initialize only when in DTA mode
     */


    final class DtaHelperService extends IDtaHelper.Stub {
        /* Deactivates NFC target with selected command
         * @param deactivationType Integer value that determines the deactivation type:
         * 1 = NFC-DEP DSL request
         * 2 = NFC-DEP RLS request
         * 3 = General deactivation to sleep mode
         * 4 = General deactivation */
        @Override
        public boolean nfcDeactivate(int deactivationType) throws RemoteException {
            NfcPermissions.enforceUserPermissions(mContext);

            if (DBG) Log.d(TAG, "NfcAdapterService::nfcDeactivate: " + deactivationType);

            boolean ret = mDeviceHost.nfcDeactivate(deactivationType);

            // Presence checking is disabled so set tag presence on deactivate:
            // There should be only one TagEndPoint at a time, but still use loop, since
            // we do not know the native handle (NFC-DEP target/initiator is also a TagEndPoint).
            Object[] objectsToDeactivate;
            synchronized (this) {
                Object[] objectValues = mObjectMap.values().toArray();
                objectsToDeactivate = Arrays.copyOf(objectValues, objectValues.length);
                Log.d(TAG, "[DTA] number of objects in objectmap:" + objectsToDeactivate.length);
            }
            for (Object o : objectsToDeactivate) {
                Log.d(TAG, "[DTA] NfcAdapterService::nfcDeactivate: found object:" + o.getClass().getName());
                if (o instanceof TagEndpoint) {
                    Log.d(TAG, "[DTA] NfcAdapterService::nfcDeactivate: set TagEndPoint presence to false");
                    TagEndpoint tag = (TagEndpoint) o;
                    tag.setIsPresent(false);
                }
            }

            return ret;
        }

        @Override
        public void dta_set_pattern_number(int pattern) {
            synchronized(this) {
              NfcPermissions.enforceUserPermissions(mContext);
              Log.d(TAG, "dta_set_pattern_number");
              System.setProperty(DTA_PATTERN, String.valueOf(pattern));
              mDiscoveryEnabled = mDeviceHost.disableDiscovery();
              mDeviceHost.dta_set_pattern_number(pattern);
              {
                boolean shouldRestart = mDiscoveryEnabled;
                NfcDiscoveryParameters newParams = computeDiscoveryParameters(mScreenState);
                mDiscoveryEnabled = mDeviceHost.enableDiscovery(newParams, shouldRestart);
              }
            }

        }

        @Override
        public int dta_get_pattern_number() {
            NfcPermissions.enforceUserPermissions(mContext);
            return mDeviceHost.dta_get_pattern_number();
        }
        /**
         * Shows a toast message.
         *
         * @param text Text to be shown in the toast.
         */
        public void showToast(final String text) {
            new AsyncTask<String, Void, String>() {

                @Override
                protected String doInBackground(String... params) {
                    String toastMessage = params[0];
                    return toastMessage;
                }

                @Override
                protected void onPostExecute(String result) {
                    Toast.makeText(mContext, result, Toast.LENGTH_SHORT).show();
                }

            }.execute(text);
        }

        /**
         * Gets the text data from the first "text/plain" type NDEF message record.
         *
         * @return byte[] The text data of the NDEF message or null if no text was found.
         */
        public String get_text_from_ndef(NdefMessage ndefMessage) {
            NfcPermissions.enforceUserPermissions(mContext);
            try {
                Log.v(TAG, "getTextFromNdef");
                NdefRecord record = ndefMessage.getRecords()[0];

                // check that TNF and type are valid for RTD Text
                if ((record.getTnf() != NdefRecord.TNF_WELL_KNOWN) || !Arrays.equals(record.getType(), NdefRecord.RTD_TEXT)) {
                    return null;
                }

                byte[] payload = record.getPayload();
                String textEncoding = ((payload[0] & 0200) == 0) ? "UTF-8" : "UTF-16";
                int languageCodeLength = payload[0] & 0077;
                String text = new String(payload, languageCodeLength + 1, payload.length - languageCodeLength - 1, textEncoding);
                return text;
            } catch (Exception e) {
                return null;
            }
        }
        public int snep_client_create(String serviceName) {
            NfcPermissions.enforceUserPermissions(mContext);
            mSnepClientHandleCounter++;
            int clientHandle = mSnepClientHandleCounter;
            SnepClient snepClient = new SnepClient(serviceName);
            mSnepClients.put(clientHandle, snepClient);
            return clientHandle;
        }

        public boolean snep_client_connect(int handle) {
            NfcPermissions.enforceUserPermissions(mContext);
            SnepClient snepClient = (SnepClient)mSnepClients.get(handle);
            if (snepClient != null) {
                try {
                    snepClient.connect();
                    return true;
                } catch (IOException e) {
                    if (DBG) Log.d(TAG, "snep_client_connect failed: " + e.toString());
                }
            } else {
                if (DBG) Log.d(TAG, "snep_client_connect: handle " + handle + " not found!");
            }
            return false;
        }

        public boolean snep_client_put(int handle, NdefMessage ndefMessage) {
            NfcPermissions.enforceUserPermissions(mContext);
            SnepClient snepClient = (SnepClient)mSnepClients.get(handle);
            if (snepClient != null) {
                try {
                    snepClient.put(ndefMessage);
                    return true;
                } catch (IOException e) {
                    if (DBG) Log.d(TAG, "snep_client_put failed: " + e.toString());
                }
            } else {
                if (DBG) Log.d(TAG, "snep_client_put: handle " + handle + " not found!");
            }
            return false;
        }

        public NdefMessage snep_client_get(int handle, NdefMessage ndefMessage) {
            NfcPermissions.enforceUserPermissions(mContext);
            SnepClient snepClient = (SnepClient)mSnepClients.get(handle);
            if (snepClient != null) {
                try {
                    SnepMessage snepMessage = snepClient.get(ndefMessage);
                    return (snepMessage != null) ? snepMessage.getNdefMessage() : null;
                } catch (IOException e) {
                    return null;
                }
            } else {
                if (DBG) Log.d(TAG, "snep_client_get: handle " + handle + " not found!");
            }
            return null;
        }

        public void snep_client_close(int handle) {
            NfcPermissions.enforceUserPermissions(mContext);
            if (handle != -1) { // close and remove SNEP client by handle.
                SnepClient snepClient = (SnepClient)mSnepClients.get(handle);
                if (snepClient != null) {
                    snepClient.close();
                    mSnepClients.remove(handle);
                } else {
                    if (DBG) Log.d(TAG, "snep_client_close: handle " + handle + " not found!");
                }
            } else { // close and remove all SNEP clients.
                for (Map.Entry<Integer, SnepClient> entry : mSnepClients.entrySet()) {
                    SnepClient snepClient = (SnepClient)entry.getValue();
                    snepClient.close();
                }
                mSnepClients.clear();
            }
        }


        public int snep_server_create(String serviceName, final boolean enableExtendedDTAServer) {
            NfcPermissions.enforceUserPermissions(mContext);
            if (DBG) Log.d(TAG, "snep_server_create: " + serviceName + " enableExtendedDTAServer: " + enableExtendedDTAServer);
            mSnepServerHandleCounter++;
            int serverHandle = mSnepServerHandleCounter;
            SnepServer snepServer = new SnepServer(serviceName, SnepServer.DEFAULT_PORT, new SnepServer.Callback() {

                private NdefMessage storedNdef = null;

                public SnepMessage doPut(NdefMessage msg) {
                    if (DBG) Log.d(TAG, "NfcAdapterService::SnepMessage::doPut: " + msg);
                    if (msg != null) {
                        showToast("SnepServer PUT: " + get_text_from_ndef(msg));
                    }

                    // store the NDEF message if it is of type RTD Text
                    if (msg != null) {
                        NdefRecord record = msg.getRecords()[0];
                        if ((record.getTnf() == NdefRecord.TNF_WELL_KNOWN) && Arrays.equals(record.getType(), NdefRecord.RTD_TEXT)) {
                            storedNdef = msg;
                            if (DBG) Log.d(TAG, "NfcAdapterService::SnepMessage::doPut: NDEF Stored!");
                        }
                    }

                    return SnepMessage.getMessage(SnepMessage.RESPONSE_SUCCESS);
                }
                public SnepMessage doGet(int acceptableLength, NdefMessage msg) {
                    if (DBG) Log.d(TAG, "NfcAdapterService::SnepMessage::doGet: " + msg);
                    if (msg != null) {
                        showToast("SnepServer GET: " + get_text_from_ndef(msg));
                    }

                    /* 1. NFC Forum Default SNEP Server */
                    if (!enableExtendedDTAServer) {
                        return SnepMessage.getMessage(SnepMessage.RESPONSE_NOT_IMPLEMENTED);
                    }

                    /* 2. Extended DTA Server */
                    // return the stored NDEF message fully if we have a one, otherwise return "C0" - Not found.
                    if (storedNdef != null) {
                        // An NdefMessage is guaranteed to have one or more NDEF Records
                        NdefRecord firstRecord = msg.getRecords()[0];
                        if (Arrays.equals(firstRecord.getType(), NdefRecord.RTD_TEXT)) { // RTD_TEXT was requested
                            if (DBG) Log.d(TAG, "NfcAdapterService::SnepMessage::doGet: Requested data found!");

                            SnepMessage successResponse = SnepMessage.getSuccessResponse(storedNdef);
                            if (successResponse.getLength() > acceptableLength) {
                                if (DBG) Log.d(TAG, "NfcAdapterService::SnepMessage::doGet: Requested data is too long!");
                                return SnepMessage.getMessage(SnepMessage.RESPONSE_EXCESS_DATA);
                            } else {
                                if (DBG) Log.d(TAG, "NfcAdapterService::SnepMessage::doGet: Requested data length OK! Returning in Success Response");
                                return successResponse;
                            }
                        } else { // Other type was requested? return not found
                            if (DBG) Log.d(TAG, "NfcAdapterService::SnepMessage::doGet: NDEF of type other than RTD_TEXT requested. Returning not found.");
                            return SnepMessage.getNotFoundResponse(null);
                        }
                    } else {
                        return SnepMessage.getNotFoundResponse(null);
                    }
                }
                // <DTA>
                /* doClean should be called only while in snep testing */
                public void doClean(){
                    storedNdef = null;
                }
                // <DTA>
            });

            if (snepServer != null) {
                snepServer.start();
                mSnepServers.put(serverHandle, snepServer);

                // display a toast message to the user
                if (enableExtendedDTAServer) {
                    showToast("Extended DTA SNEP server started");
                } else {
                    showToast("NFC Forum Default SNEP server started");
                }
                return serverHandle;
            } else {
                return -1;
            }
        }

        public void snep_server_close(int handle) {
            NfcPermissions.enforceUserPermissions(mContext);
            if (handle != -1) { // remove SNEP server by given handle
                SnepServer snepServer = (SnepServer)mSnepServers.get(handle);
                if (snepServer != null) {
                    snepServer.stop();
                    mSnepServers.remove(handle);
                } else {
                    if (DBG) Log.d(TAG, "snep_server_close: handle " + handle + " not found!");
                }
            } else { // close and remove all SNEP servers.
                for (Map.Entry<Integer, SnepServer> entry : mSnepServers.entrySet()) {
                    SnepServer snepServer = (SnepServer)entry.getValue();
                    snepServer.stop();
                }
                mSnepServers.clear();
            }
        }
        @Override
        public boolean in_dta_mode() {
            return  mDeviceHost.dta_get_pattern_number() >= 0;
        }
        /**
         * Starts a connection-oriented echo service including a server
         * listening for incoming messages and a client for sending messages
         * received by the server.
         *
         * @param serviceNameIn service name (URN) of the connection-oriented inbound service
         * @param serviceNameOut service name (URN) of the connection-oriented outbound service
         */
        public void startLlcpCoEchoServer(String serviceNameIn, String serviceNameOut) {
            NfcPermissions.enforceUserPermissions(mContext);
            mP2pLinkManager.startLlcpCoEchoServer(serviceNameIn, serviceNameOut);
        }

        /**
         * Stops a running connection-oriented echo service.
         */
        public void stopLlcpCoEchoServer() {
            NfcPermissions.enforceUserPermissions(mContext);
            mP2pLinkManager.stopLlcpCoEchoServer();
        }
        /**
         * Starts a connectionless echo service including a server listening for incoming messages
         * and a client for sending messages received by the server.
         *
         * @param serviceNameIn service name (URN) of the connectionless inbound service
         * @param serviceNameOut service name (URN) of the connectionless outbound service
         */
        public void startLlcpClEchoServer(String serviceNameIn, String serviceNameOut) {
            mP2pLinkManager.startLlcpClEchoServer(serviceNameIn, serviceNameOut);
        }

        /**
         * Stops a running connectionless echo service.
         */
        public void stopLlcpClEchoServer() {
            mP2pLinkManager.stopLlcpClEchoServer();
        }
    } // </DTA>


    final class NfcAdapterService extends INfcAdapter.Stub {
        public boolean in_dta_mode() {
            return  mDeviceHost.dta_get_pattern_number() >= 0;
        }

        @Override
        public boolean enable() throws RemoteException {
            NfcPermissions.enforceAdminPermissions(mContext);

            saveNfcOnSetting(true);

            if (mIsAirplaneSensitive && isAirplaneModeOn()) {
                mNfcOnWhileAirplaneModeOn = true;
                if (!mIsAirplaneToggleable) {
                    Log.i(TAG, "denying enable() request (airplane mode)");
                    return false;
                }
                // Make sure the override survives a reboot
                mPrefsEditor.putBoolean(PREF_AIRPLANE_OVERRIDE, true);
                mPrefsEditor.apply();
            }
            new EnableDisableTask().execute(TASK_ENABLE);

            return true;
        }

        @Override
        public boolean disable(boolean saveState) throws RemoteException {
            NfcPermissions.enforceAdminPermissions(mContext);
            if(!isAirplaneModeOn())
            {
                mNfcOnWhileAirplaneModeOn =  false;
            }

            if (saveState) {
                mIsNfcDisabledReason = NFC_DISABLED_BY_USER;
                saveNfcOnSetting(false);
            }
            else
            {
                mIsNfcDisabledReason = NFC_DISABLED_BY_SYSTEM;
            }
            new EnableDisableTask().execute(TASK_DISABLE);

            return true;
        }

        @Override
        public void pausePolling(int timeoutInMs) {
            NfcPermissions.enforceAdminPermissions(mContext);

            if (timeoutInMs <= 0 || timeoutInMs > MAX_POLLING_PAUSE_TIMEOUT) {
                Log.e(TAG, "Refusing to pause polling for " + timeoutInMs + "ms.");
                return;
            }

            synchronized (NfcService.this) {
                mPollingPaused = true;
                mDiscoveryEnabled = mDeviceHost.disableDiscovery();
                mHandler.sendMessageDelayed(
                        mHandler.obtainMessage(MSG_RESUME_POLLING), timeoutInMs);
            }
        }

        @Override
        public void resumePolling() {
            NfcPermissions.enforceAdminPermissions(mContext);

            synchronized (NfcService.this) {
                if (!mPollingPaused) {
                    return;
                }

                mHandler.removeMessages(MSG_RESUME_POLLING);
                mPollingPaused = false;
                new ApplyRoutingTask().execute();
            }
        }

        @Override
        public boolean isNdefPushEnabled() throws RemoteException {
            synchronized (NfcService.this) {
                return mState == NfcAdapter.STATE_ON && mIsNdefPushEnabled;
            }
        }

        @Override
        public boolean enableNdefPush() throws RemoteException {
            NfcPermissions.enforceAdminPermissions(mContext);
            synchronized (NfcService.this) {
                if (mIsNdefPushEnabled) {
                    return true;
                }
                Log.i(TAG, "enabling NDEF Push");
                mPrefsEditor.putBoolean(PREF_NDEF_PUSH_ON, true);
                mPrefsEditor.apply();
                mIsNdefPushEnabled = true;
                setBeamShareActivityState(true);
                if (isNfcEnabled()) {
                    mP2pLinkManager.enableDisable(true, true);
                }
            }
            return true;
        }

        @Override
        public boolean disableNdefPush() throws RemoteException {
            NfcPermissions.enforceAdminPermissions(mContext);
            synchronized (NfcService.this) {
                if (!mIsNdefPushEnabled) {
                    return true;
                }
                Log.i(TAG, "disabling NDEF Push");
                mPrefsEditor.putBoolean(PREF_NDEF_PUSH_ON, false);
                mPrefsEditor.apply();
                mIsNdefPushEnabled = false;
                setBeamShareActivityState(false);
                if (isNfcEnabled()) {
                    mP2pLinkManager.enableDisable(false, true);
                }
            }
            return true;
        }

        @Override
        public void setForegroundDispatch(PendingIntent intent,
                IntentFilter[] filters, TechListParcel techListsParcel) {
            NfcPermissions.enforceUserPermissions(mContext);

            // Short-cut the disable path
            if (intent == null && filters == null && techListsParcel == null) {
                mNfcDispatcher.setForegroundDispatch(null, null, null);
                return;
            }

            // Validate the IntentFilters
            if (filters != null) {
                if (filters.length == 0) {
                    filters = null;
                } else {
                    for (IntentFilter filter : filters) {
                        if (filter == null) {
                            throw new IllegalArgumentException("null IntentFilter");
                        }
                    }
                }
            }

            // Validate the tech lists
            String[][] techLists = null;
            if (techListsParcel != null) {
                techLists = techListsParcel.getTechLists();
            }

            mNfcDispatcher.setForegroundDispatch(intent, filters, techLists);
        }


        @Override
        public void setAppCallback(IAppCallback callback) {
            NfcPermissions.enforceUserPermissions(mContext);

            // don't allow Beam for managed profiles, or devices with a device owner or policy owner
            UserInfo userInfo = mUserManager.getUserInfo(UserHandle.getCallingUserId());
            if(!mUserManager.hasUserRestriction(
                            UserManager.DISALLOW_OUTGOING_BEAM, userInfo.getUserHandle())) {
                mP2pLinkManager.setNdefCallback(callback, Binder.getCallingUid());
            } else if (DBG) {
                Log.d(TAG, "Disabling default Beam behavior");
            }
        }

        @Override
        public void verifyNfcPermission() {
            NfcPermissions.enforceUserPermissions(mContext);
        }        

        @Override
        public void invokeBeam() {
            NfcPermissions.enforceUserPermissions(mContext);

            if (mForegroundUtils.isInForeground(Binder.getCallingUid())) {
                mP2pLinkManager.onManualBeamInvoke(null);
            } else {
                Log.e(TAG, "Calling activity not in foreground.");
            }
        }

        @Override
        public void invokeBeamInternal(BeamShareData shareData) {
            NfcPermissions.enforceAdminPermissions(mContext);
            Message msg = Message.obtain();
            msg.what = MSG_INVOKE_BEAM;
            msg.obj = shareData;
            // We have to send this message delayed for two reasons:
            // 1) This is an IPC call from BeamShareActivity, which is
            //    running when the user has invoked Beam through the
            //    share menu. As soon as BeamShareActivity closes, the UI
            //    will need some time to rebuild the original Activity.
            //    Waiting here for a while gives a better chance of the UI
            //    having been rebuilt, which means the screenshot that the
            //    Beam animation is using will be more accurate.
            // 2) Similarly, because the Activity that launched BeamShareActivity
            //    with an ACTION_SEND intent is now in paused state, the NDEF
            //    callbacks that it has registered may no longer be valid.
            //    Allowing the original Activity to resume will make sure we
            //    it has a chance to re-register the NDEF message / callback,
            //    so we share the right data.
            //
            //    Note that this is somewhat of a hack because the delay may not actually
            //    be long enough for 2) on very slow devices, but there's no better
            //    way to do this right now without additional framework changes.
            mHandler.sendMessageDelayed(msg, INVOKE_BEAM_DELAY_MS);
        }

        @Override
        public INfcTag getNfcTagInterface() throws RemoteException {
            return mNfcTagService;
        }

        @Override
        public INfcCardEmulation getNfcCardEmulationInterface() {
            if (mIsHceCapable) {
                return mCardEmulationManager.getNfcCardEmulationInterface();
            } else {
                return null;
            }
        }

        @Override
        public int getState() throws RemoteException {
            synchronized (NfcService.this) {
                return mState;
            }
        }

        @Override
        protected void dump(FileDescriptor fd, PrintWriter pw, String[] args) {
            NfcService.this.dump(fd, pw, args);
        }

        @Override
        public void dispatch(Tag tag) throws RemoteException {
            NfcPermissions.enforceAdminPermissions(mContext);
            mNfcDispatcher.dispatchTag(tag);
        }

        @Override
        public void setP2pModes(int initiatorModes, int targetModes) throws RemoteException {
            NfcPermissions.enforceAdminPermissions(mContext);
            mDeviceHost.setP2pInitiatorModes(initiatorModes);
            mDeviceHost.setP2pTargetModes(targetModes);
            applyRouting(true);
        }

        @Override
        public void setReaderMode(IBinder binder, IAppCallback callback, int flags, Bundle extras)
                throws RemoteException {
            synchronized (NfcService.this) {
                if (flags != 0) {
                    try {
                        mReaderModeParams = new ReaderModeParams();
                        mReaderModeParams.callback = callback;
                        mReaderModeParams.flags = flags;
                        mReaderModeParams.presenceCheckDelay = extras != null
                                ? (extras.getInt(NfcAdapter.EXTRA_READER_PRESENCE_CHECK_DELAY,
                                        DEFAULT_PRESENCE_CHECK_DELAY))
                                : DEFAULT_PRESENCE_CHECK_DELAY;
                        binder.linkToDeath(mReaderModeDeathRecipient, 0);
                    } catch (RemoteException e) {
                        Log.e(TAG, "Remote binder has already died.");
                        return;
                    }
                } else {
                    try {
                        mReaderModeParams = null;
                        binder.unlinkToDeath(mReaderModeDeathRecipient, 0);
                    } catch (NoSuchElementException e) {
                        Log.e(TAG, "Reader mode Binder was never registered.");
                    }
                }
                applyRouting(false);
            }
        }

        @Override
        public INfcAdapterExtras getNfcAdapterExtrasInterface(String pkg) throws RemoteException {
            // nfc-extras implementation is no longer present in AOSP.
            return null;
        }

        @Override
        public void addNfcUnlockHandler(INfcUnlockHandler unlockHandler, int[] techList) {
            NfcPermissions.enforceAdminPermissions(mContext);

            int lockscreenPollMask = computeLockscreenPollMask(techList);
            synchronized (NfcService.this) {
                mNfcUnlockManager.addUnlockHandler(unlockHandler, lockscreenPollMask);
            }

            applyRouting(false);
        }

        @Override
        public void removeNfcUnlockHandler(INfcUnlockHandler token) throws RemoteException {
            synchronized (NfcService.this) {
                mNfcUnlockManager.removeUnlockHandler(token.asBinder());
            }

            applyRouting(false);
        }

        private int computeLockscreenPollMask(int[] techList) {

            Map<Integer, Integer> techCodeToMask = new HashMap<Integer, Integer>();

            techCodeToMask.put(TagTechnology.NFC_A, NfcService.NFC_POLL_A);
            techCodeToMask.put(TagTechnology.NFC_B,
                    NfcService.NFC_POLL_B | NfcService.NFC_POLL_B_PRIME);
            techCodeToMask.put(TagTechnology.NFC_V, NfcService.NFC_POLL_ISO15693);
            techCodeToMask.put(TagTechnology.NFC_F, NfcService.NFC_POLL_F);
            techCodeToMask.put(TagTechnology.NFC_BARCODE, NfcService.NFC_POLL_KOVIO);

            int mask = 0;

            for (int i = 0; i < techList.length; i++) {
                if (techCodeToMask.containsKey(techList[i])) {
                    mask |= techCodeToMask.get(techList[i]).intValue();
                }
            }

            return mask;
        }
    }

    final class ReaderModeDeathRecipient implements IBinder.DeathRecipient {
        @Override
        public void binderDied() {
            synchronized (NfcService.this) {
                if (mReaderModeParams != null) {
                    mReaderModeParams = null;
                    applyRouting(false);
                }
            }
        }
    }

    final class TagService extends INfcTag.Stub {
        @Override
        public int close(int nativeHandle) throws RemoteException {
            NfcPermissions.enforceUserPermissions(mContext);

            TagEndpoint tag = null;

            if (!isNfcEnabled()) {
                return ErrorCodes.ERROR_NOT_INITIALIZED;
            }

            /* find the tag in the hmap */
            tag = (TagEndpoint) findObject(nativeHandle);
            if (tag != null) {
                /* Remove the device from the hmap */
                unregisterObject(nativeHandle);
                tag.disconnect();
                return ErrorCodes.SUCCESS;
            }
            /* Restart polling loop for notification */
            applyRouting(true);
            return ErrorCodes.ERROR_DISCONNECT;
        }

        @Override
        public int connect(int nativeHandle, int technology) throws RemoteException {
            NfcPermissions.enforceUserPermissions(mContext);

            TagEndpoint tag = null;

            if (!isNfcEnabled()) {
                return ErrorCodes.ERROR_NOT_INITIALIZED;
            }

            /* find the tag in the hmap */
            tag = (TagEndpoint) findObject(nativeHandle);
            if (tag == null) {
                return ErrorCodes.ERROR_DISCONNECT;
            }

            if (!tag.isPresent()) {
                return ErrorCodes.ERROR_DISCONNECT;
            }

            // Note that on most tags, all technologies are behind a single
            // handle. This means that the connect at the lower levels
            // will do nothing, as the tag is already connected to that handle.
            if (tag.connect(technology)) {
                return ErrorCodes.SUCCESS;
            } else {
                return ErrorCodes.ERROR_DISCONNECT;
            }
        }

        @Override
        public int reconnect(int nativeHandle) throws RemoteException {
            NfcPermissions.enforceUserPermissions(mContext);

            TagEndpoint tag = null;

            // Check if NFC is enabled
            if (!isNfcEnabled()) {
                return ErrorCodes.ERROR_NOT_INITIALIZED;
            }

            /* find the tag in the hmap */
            tag = (TagEndpoint) findObject(nativeHandle);
            if (tag != null) {
                if (tag.reconnect()) {
                    return ErrorCodes.SUCCESS;
                } else {
                    return ErrorCodes.ERROR_DISCONNECT;
                }
            }
            return ErrorCodes.ERROR_DISCONNECT;
        }

        @Override
        public int[] getTechList(int nativeHandle) throws RemoteException {
            NfcPermissions.enforceUserPermissions(mContext);

            // Check if NFC is enabled
            if (!isNfcEnabled()) {
                return null;
            }

            /* find the tag in the hmap */
            TagEndpoint tag = (TagEndpoint) findObject(nativeHandle);
            if (tag != null) {
                return tag.getTechList();
            }
            return null;
        }

        @Override
        public boolean isPresent(int nativeHandle) throws RemoteException {
            TagEndpoint tag = null;

            // Check if NFC is enabled
            if (!isNfcEnabled()) {
                return false;
            }

            /* find the tag in the hmap */
            tag = (TagEndpoint) findObject(nativeHandle);
            if (tag == null) {
                return false;
            }

            return tag.isPresent();
        }

        @Override
        public boolean isNdef(int nativeHandle) throws RemoteException {
            NfcPermissions.enforceUserPermissions(mContext);

            TagEndpoint tag = null;

            // Check if NFC is enabled
            if (!isNfcEnabled()) {
                return false;
            }

            /* find the tag in the hmap */
            tag = (TagEndpoint) findObject(nativeHandle);
            int[] ndefInfo = new int[2];
            if (tag == null) {
                return false;
            }
            return tag.checkNdef(ndefInfo);
        }

        @Override
        public TransceiveResult transceive(int nativeHandle, byte[] data, boolean raw)
                throws RemoteException {
            NfcPermissions.enforceUserPermissions(mContext);

            TagEndpoint tag = null;
            byte[] response;

            // Check if NFC is enabled
            if (!isNfcEnabled()) {
                return null;
            }

            /* find the tag in the hmap */
            tag = (TagEndpoint) findObject(nativeHandle);
            if (tag != null) {
                // Check if length is within limits
                if (data.length > getMaxTransceiveLength(tag.getConnectedTechnology())) {
                    return new TransceiveResult(TransceiveResult.RESULT_EXCEEDED_LENGTH, null);
                }
                int[] targetLost = new int[1];
                response = tag.transceive(data, raw, targetLost);
                int result;
                if (response != null) {
                    result = TransceiveResult.RESULT_SUCCESS;
                } else if (targetLost[0] == 1) {
                    result = TransceiveResult.RESULT_TAGLOST;
                } else {
                    result = TransceiveResult.RESULT_FAILURE;
                }
                return new TransceiveResult(result, response);
            }
            return null;
        }

        @Override
        public NdefMessage ndefRead(int nativeHandle) throws RemoteException {
            NfcPermissions.enforceUserPermissions(mContext);

            TagEndpoint tag;

            // Check if NFC is enabled
            if (!isNfcEnabled()) {
                return null;
            }

            /* find the tag in the hmap */
            tag = (TagEndpoint) findObject(nativeHandle);
            if (tag != null) {
                byte[] buf = tag.readNdef();
                if (buf == null) {
                    return null;
                }

                /* Create an NdefMessage */
                try {
                    return new NdefMessage(buf);
                } catch (FormatException e) {
                    return null;
                }
            }
            return null;
        }

        @Override
        public int ndefWrite(int nativeHandle, NdefMessage msg) throws RemoteException {
            NfcPermissions.enforceUserPermissions(mContext);

            TagEndpoint tag;

            // Check if NFC is enabled
            if (!isNfcEnabled()) {
                return ErrorCodes.ERROR_NOT_INITIALIZED;
            }

            /* find the tag in the hmap */
            tag = (TagEndpoint) findObject(nativeHandle);
            if (tag == null) {
                return ErrorCodes.ERROR_IO;
            }

            if (msg == null) return ErrorCodes.ERROR_INVALID_PARAM;

            if (tag.writeNdef(msg.toByteArray())) {
                return ErrorCodes.SUCCESS;
            } else {
                return ErrorCodes.ERROR_IO;
            }

        }

        @Override
        public boolean ndefIsWritable(int nativeHandle) throws RemoteException {
            throw new UnsupportedOperationException();
        }

        @Override
        public int ndefMakeReadOnly(int nativeHandle) throws RemoteException {
            NfcPermissions.enforceUserPermissions(mContext);

            TagEndpoint tag;

            // Check if NFC is enabled
            if (!isNfcEnabled()) {
                return ErrorCodes.ERROR_NOT_INITIALIZED;
            }

            /* find the tag in the hmap */
            tag = (TagEndpoint) findObject(nativeHandle);
            if (tag == null) {
                return ErrorCodes.ERROR_IO;
            }

            if (tag.makeReadOnly()) {
                return ErrorCodes.SUCCESS;
            } else {
                return ErrorCodes.ERROR_IO;
            }
        }

        @Override
        public int formatNdef(int nativeHandle, byte[] key) throws RemoteException {
            NfcPermissions.enforceUserPermissions(mContext);

            TagEndpoint tag;

            // Check if NFC is enabled
            if (!isNfcEnabled()) {
                return ErrorCodes.ERROR_NOT_INITIALIZED;
            }

            /* find the tag in the hmap */
            tag = (TagEndpoint) findObject(nativeHandle);
            if (tag == null) {
                return ErrorCodes.ERROR_IO;
            }

            if (tag.formatNdef(key)) {
                return ErrorCodes.SUCCESS;
            } else {
                return ErrorCodes.ERROR_IO;
            }
        }

        @Override
        public Tag rediscover(int nativeHandle) throws RemoteException {
            NfcPermissions.enforceUserPermissions(mContext);

            TagEndpoint tag = null;

            // Check if NFC is enabled
            if (!isNfcEnabled()) {
                return null;
            }

            /* find the tag in the hmap */
            tag = (TagEndpoint) findObject(nativeHandle);
            if (tag != null) {
                // For now the prime usecase for rediscover() is to be able
                // to access the NDEF technology after formatting without
                // having to remove the tag from the field, or similar
                // to have access to NdefFormatable in case low-level commands
                // were used to remove NDEF. So instead of doing a full stack
                // rediscover (which is poorly supported at the moment anyway),
                // we simply remove these two technologies and detect them
                // again.
                tag.removeTechnology(TagTechnology.NDEF);
                tag.removeTechnology(TagTechnology.NDEF_FORMATABLE);
                tag.findAndReadNdef();
                // Build a new Tag object to return
                Tag newTag = new Tag(tag.getUid(), tag.getTechList(),
                        tag.getTechExtras(), tag.getHandle(), this);
                return newTag;
            }
            return null;
        }

        @Override
        public int setTimeout(int tech, int timeout) throws RemoteException {
            NfcPermissions.enforceUserPermissions(mContext);
            boolean success = mDeviceHost.setTimeout(tech, timeout);
            if (success) {
                return ErrorCodes.SUCCESS;
            } else {
                return ErrorCodes.ERROR_INVALID_PARAM;
            }
        }

        @Override
        public int getTimeout(int tech) throws RemoteException {
            NfcPermissions.enforceUserPermissions(mContext);

            return mDeviceHost.getTimeout(tech);
        }

        @Override
        public void resetTimeouts() throws RemoteException {
            NfcPermissions.enforceUserPermissions(mContext);

            mDeviceHost.resetTimeouts();
        }

        @Override
        public boolean canMakeReadOnly(int ndefType) throws RemoteException {
            return mDeviceHost.canMakeReadOnly(ndefType);
        }

        @Override
        public int getMaxTransceiveLength(int tech) throws RemoteException {
            return mDeviceHost.getMaxTransceiveLength(tech);
        }

        @Override
        public boolean getExtendedLengthApdusSupported() throws RemoteException {
            return mDeviceHost.getExtendedLengthApdusSupported();
        }
        // <DTA>
        public boolean in_dta_mode() {
            return mNfcAdapter.in_dta_mode();
        }
        // </DTA>
    }

    /*
     * The Tag Service in DTA mode
     */
    final class TagServiceDta extends INfcTagDta.Stub {
        @Override
        public int close(int nativeHandle) throws RemoteException {
            NfcPermissions.enforceUserPermissions(mContext);

            TagEndpoint tag = null;

            if (!isNfcEnabled()) {
                return ErrorCodes.ERROR_NOT_INITIALIZED;
            }

            /* find the tag in the hmap */
            tag = (TagEndpoint) findObject(nativeHandle);
            if (tag != null) {
                /* Remove the device from the hmap */
                unregisterObject(nativeHandle);
                tag.disconnect();
                return ErrorCodes.SUCCESS;
            }
            /* Restart polling loop for notification */
            applyRouting(true);
            return ErrorCodes.ERROR_DISCONNECT;
        }

        @Override
        public int connect(int nativeHandle, int technology) throws RemoteException {
            NfcPermissions.enforceUserPermissions(mContext);

            TagEndpoint tag = null;

            if (!isNfcEnabled()) {
                return ErrorCodes.ERROR_NOT_INITIALIZED;
            }

            /* find the tag in the hmap */
            tag = (TagEndpoint) findObject(nativeHandle);
            if (tag == null) {
                return ErrorCodes.ERROR_DISCONNECT;
            }

            if (!tag.isPresent()) {
                return ErrorCodes.ERROR_DISCONNECT;
            }

            // Note that on most tags, all technologies are behind a single
            // handle. This means that the connect at the lower levels
            // will do nothing, as the tag is already connected to that handle.
            if (tag.connect(technology)) {
                return ErrorCodes.SUCCESS;
            } else {
                return ErrorCodes.ERROR_DISCONNECT;
            }
        }

        @Override
        public int reconnect(int nativeHandle) throws RemoteException {
            NfcPermissions.enforceUserPermissions(mContext);

            TagEndpoint tag = null;

            // Check if NFC is enabled
            if (!isNfcEnabled()) {
                return ErrorCodes.ERROR_NOT_INITIALIZED;
            }

            /* find the tag in the hmap */
            tag = (TagEndpoint) findObject(nativeHandle);
            if (tag != null) {
                if (tag.reconnect()) {
                    return ErrorCodes.SUCCESS;
                } else {
                    return ErrorCodes.ERROR_DISCONNECT;
                }
            }
            return ErrorCodes.ERROR_DISCONNECT;
        }

        @Override
        public int[] getTechList(int nativeHandle) throws RemoteException {
            NfcPermissions.enforceUserPermissions(mContext);

            // Check if NFC is enabled
            if (!isNfcEnabled()) {
                return null;
            }

            /* find the tag in the hmap */
            TagEndpoint tag = (TagEndpoint) findObject(nativeHandle);
            if (tag != null) {
                return tag.getTechList();
            }
            return null;
        }

        @Override
        public boolean isPresent(int nativeHandle) throws RemoteException {
            TagEndpoint tag = null;

            // Check if NFC is enabled
            if (!isNfcEnabled()) {
                return false;
            }

            /* find the tag in the hmap */
            tag = (TagEndpoint) findObject(nativeHandle);
            if (tag == null) {
                return false;
            }

            return tag.isPresent();
        }

        @Override
        public boolean isNdef(int nativeHandle) throws RemoteException {
            NfcPermissions.enforceUserPermissions(mContext);

            TagEndpoint tag = null;

            // Check if NFC is enabled
            if (!isNfcEnabled()) {
                return false;
            }

            /* find the tag in the hmap */
            tag = (TagEndpoint) findObject(nativeHandle);
            int[] ndefInfo = new int[2];
            if (tag == null) {
                return false;
            }
            return tag.checkNdef(ndefInfo);
        }

        @Override
        public TransceiveResult transceive(int nativeHandle, byte[] data, boolean raw)
                throws RemoteException {
            NfcPermissions.enforceUserPermissions(mContext);

            TagEndpoint tag = null;
            byte[] response;

            // Check if NFC is enabled
            if (!isNfcEnabled()) {
                return null;
            }

            /* find the tag in the hmap */
            tag = (TagEndpoint) findObject(nativeHandle);
            if (tag != null) {
                // Check if length is within limits
                if (data.length > getMaxTransceiveLength(tag.getConnectedTechnology())) {
                    return new TransceiveResult(TransceiveResult.RESULT_EXCEEDED_LENGTH, null);
                }
                int[] targetLost = new int[1];
                response = tag.transceive(data, raw, targetLost);
                int result;
                if (response != null) {
                    result = TransceiveResult.RESULT_SUCCESS;
                } else if (targetLost[0] == 1) {
                    result = TransceiveResult.RESULT_TAGLOST;
                } else {
                    result = TransceiveResult.RESULT_FAILURE;
                }
                return new TransceiveResult(result, response);
            }
            return null;
        }

        @Override
        public NdefMessage ndefRead(int nativeHandle) throws RemoteException {
            NfcPermissions.enforceUserPermissions(mContext);

            TagEndpoint tag;

            // Check if NFC is enabled
            if (!isNfcEnabled()) {
                return null;
            }

            /* find the tag in the hmap */
            tag = (TagEndpoint) findObject(nativeHandle);
            if (tag != null) {
                byte[] buf = tag.readNdef();
                if (buf == null) {
                    return null;
                }

                /* Create an NdefMessage */
                try {
                    return new NdefMessage(buf);
                } catch (FormatException e) {
                    return null;
                }
            }
            return null;
        }

        @Override
        public int ndefWrite(int nativeHandle, NdefMessage msg) throws RemoteException {
            NfcPermissions.enforceUserPermissions(mContext);

            TagEndpoint tag;

            // Check if NFC is enabled
            if (!isNfcEnabled()) {
                return ErrorCodes.ERROR_NOT_INITIALIZED;
            }

            /* find the tag in the hmap */
            tag = (TagEndpoint) findObject(nativeHandle);
            if (tag == null) {
                return ErrorCodes.ERROR_IO;
            }

            if (msg == null) return ErrorCodes.ERROR_INVALID_PARAM;

            if (tag.writeNdef(msg.toByteArray())) {
                return ErrorCodes.SUCCESS;
            } else {
                return ErrorCodes.ERROR_IO;
            }

        }

        @Override
        public boolean ndefIsWritable(int nativeHandle) throws RemoteException {
            throw new UnsupportedOperationException();
        }

        @Override
        public int ndefMakeReadOnly(int nativeHandle) throws RemoteException {
            NfcPermissions.enforceUserPermissions(mContext);

            TagEndpoint tag;

            // Check if NFC is enabled
            if (!isNfcEnabled()) {
                return ErrorCodes.ERROR_NOT_INITIALIZED;
            }

            /* find the tag in the hmap */
            tag = (TagEndpoint) findObject(nativeHandle);
            if (tag == null) {
                return ErrorCodes.ERROR_IO;
            }

            if (tag.makeReadOnly()) {
                return ErrorCodes.SUCCESS;
            } else {
                return ErrorCodes.ERROR_IO;
            }
        }

        @Override
        public int formatNdef(int nativeHandle, byte[] key) throws RemoteException {
            NfcPermissions.enforceUserPermissions(mContext);

            TagEndpoint tag;

            // Check if NFC is enabled
            if (!isNfcEnabled()) {
                return ErrorCodes.ERROR_NOT_INITIALIZED;
            }

            /* find the tag in the hmap */
            tag = (TagEndpoint) findObject(nativeHandle);
            if (tag == null) {
                return ErrorCodes.ERROR_IO;
            }

            if (tag.formatNdef(key)) {
                return ErrorCodes.SUCCESS;
            } else {
                return ErrorCodes.ERROR_IO;
            }
        }

        @Override
        public android.nfc.dta.TagDta rediscover(int nativeHandle) throws RemoteException {
            NfcPermissions.enforceUserPermissions(mContext);

            TagEndpoint tag = null;

            // Check if NFC is enabled
            if (!isNfcEnabled()) {
                return null;
            }

            /* find the tag in the hmap */
            tag = (TagEndpoint) findObject(nativeHandle);
            if (tag != null) {
                // For now the prime usecase for rediscover() is to be able
                // to access the NDEF technology after formatting without
                // having to remove the tag from the field, or similar
                // to have access to NdefFormatable in case low-level commands
                // were used to remove NDEF. So instead of doing a full stack
                // rediscover (which is poorly supported at the moment anyway),
                // we simply remove these two technologies and detect them
                // again.
                tag.removeTechnology(TagTechnology.NDEF);
                tag.removeTechnology(TagTechnology.NDEF_FORMATABLE);
                tag.findAndReadNdef();
                // Build a new Tag object to return
                android.nfc.dta.TagDta newTag = new android.nfc.dta.TagDta(tag.getUid(), tag.getTechList(),
                        tag.getTechExtras(), tag.getHandle(), this);
                return newTag;
            }
            return null;
        }

        @Override
        public int setTimeout(int tech, int timeout) throws RemoteException {
            NfcPermissions.enforceUserPermissions(mContext);
            boolean success = mDeviceHost.setTimeout(tech, timeout);
            if (success) {
                return ErrorCodes.SUCCESS;
            } else {
                return ErrorCodes.ERROR_INVALID_PARAM;
            }
        }

        @Override
        public int getTimeout(int tech) throws RemoteException {
            NfcPermissions.enforceUserPermissions(mContext);

            return mDeviceHost.getTimeout(tech);
        }

        @Override
        public void resetTimeouts() throws RemoteException {
            NfcPermissions.enforceUserPermissions(mContext);

            mDeviceHost.resetTimeouts();
        }

        @Override
        public boolean canMakeReadOnly(int ndefType) throws RemoteException {
            return mDeviceHost.canMakeReadOnly(ndefType);
        }

        @Override
        public int getMaxTransceiveLength(int tech) throws RemoteException {
            return mDeviceHost.getMaxTransceiveLength(tech);
        }

        @Override
        public boolean getExtendedLengthApdusSupported() throws RemoteException {
            return mDeviceHost.getExtendedLengthApdusSupported();
        }
        // <DTA>
        //@Override
        public boolean in_dta_mode() {
            return mNfcAdapter.in_dta_mode();
        }
        // </DTA>
    }

    boolean isNfcEnabledOrShuttingDown() {
        synchronized (this) {
            return (mState == NfcAdapter.STATE_ON || mState == NfcAdapter.STATE_TURNING_OFF);
        }
    }

    boolean isNfcEnabled() {
        synchronized (this) {
            return mState == NfcAdapter.STATE_ON;
        }
    }

    class WatchDogThread extends Thread {
        final Object mCancelWaiter = new Object();
        final int mTimeout;
        boolean mCanceled = false;

        public WatchDogThread(String threadName, int timeout) {
            super(threadName);
            mTimeout = timeout;
        }

        @Override
        public void run() {
            try {
                synchronized (mCancelWaiter) {
                    mCancelWaiter.wait(mTimeout);
                    if (mCanceled) {
                        return;
                    }
                }
            } catch (InterruptedException e) {
                // Should not happen; fall-through to abort.
                Log.w(TAG, "Watchdog thread interruped.");
                interrupt();
            }
            Log.e(TAG, "Watchdog triggered, aborting.");
            mDeviceHost.doAbort();
        }

        public synchronized void cancel() {
            synchronized (mCancelWaiter) {
                mCanceled = true;
                mCancelWaiter.notify();
            }
        }
    }

    static byte[] hexStringToBytes(String s) {
        if (s == null || s.length() == 0) return null;
        int len = s.length();
        if (len % 2 != 0) {
            s = '0' + s;
            len++;
        }
        byte[] data = new byte[len / 2];
        for (int i = 0; i < len; i += 2) {
            data[i / 2] = (byte) ((Character.digit(s.charAt(i), 16) << 4)
                    + Character.digit(s.charAt(i + 1), 16));
        }
        return data;
    }

    /**
     * Read mScreenState and apply NFC-C polling and NFC-EE routing
     */
    void applyRouting(boolean force) {
        synchronized (this) {
            if (!isNfcEnabledOrShuttingDown()) {
                return;
            }
            // if exchanging APDU with eSE
            if (mDeviceHost.isExchangingApduWithEse()) {
                return;
            }

            WatchDogThread watchDog = new WatchDogThread("applyRouting", ROUTING_WATCHDOG_MS);
            if (mInProvisionMode) {
                mInProvisionMode = Settings.Secure.getInt(mContentResolver,
                        Settings.Global.DEVICE_PROVISIONED, 0) == 0;
                if (!mInProvisionMode) {
                    // Notify dispatcher it's fine to dispatch to any package now
                    // and allow handover transfers.
                    mNfcDispatcher.disableProvisioningMode();
                }
            }
            // Special case: if we're transitioning to unlocked state while
            // still talking to a tag, postpone re-configuration.
            if (mScreenState == ScreenStateHelper.SCREEN_STATE_ON_UNLOCKED && isTagPresent()) {
                Log.d(TAG, "Not updating discovery parameters, tag connected.");
                mHandler.sendMessageDelayed(mHandler.obtainMessage(MSG_RESUME_POLLING),
                        APPLY_ROUTING_RETRY_TIMEOUT_MS);
                return;
            }

            try {
                watchDog.start();

                Log.d(TAG, "Screen State change: "
                            + mOldScreenState + "(" + ScreenStateHelper.screenStateToString(mOldScreenState) + ") => "
                            + mScreenState + "(" + ScreenStateHelper.screenStateToString(mScreenState) + ")");

                if (mDeviceHost.isUiStateSupported()) {
                    //Check if Secure element is present
                    boolean isUiccSecureElementPresent=false;
                    String seListWithComma = mDeviceHost.getSecureElementList();
                    if (seListWithComma != null) {
                        isUiccSecureElementPresent=true;
                    }

                    if (mScreenState == mOldScreenState) {

                        // check whether we need to toggle lock screen polling
                        if(force || mLockScreenPolling != mNfcUnlockManager.isLockscreenPollingEnabled()) {

                            // in case of race condition, make NFCC respond to command
                            if (mOldScreenState <= ScreenStateHelper.SCREEN_STATE_OFF) {
                                mHciUiState = HCI_UI_STATE_LOCKED_NOT_NOTIFIABLE;
                                mNfccUiState = NFCC_UI_STATE_UNLOCKED;
                                if (DBG) Log.d(TAG, "force screen on");
                                mDeviceHost.updateHostPresence(mHciUiState, mNfccUiState);
                            }
                            mLockScreenPolling = mDeviceHost.updateLockScreenPollingMode(mNfcUnlockManager.isLockscreenPollingEnabled());
                        }

                        // Compute new polling parameters
                        NfcDiscoveryParameters newParams = computeDiscoveryParameters(mScreenState);
                        if (force || !newParams.equals(mCurrentDiscoveryParameters)) {

                            // in case of race condition, make NFCC respond to command
                            if (mOldScreenState <= ScreenStateHelper.SCREEN_STATE_OFF) {
                                mHciUiState = HCI_UI_STATE_LOCKED_NOT_NOTIFIABLE;
                                mNfccUiState = NFCC_UI_STATE_UNLOCKED;
                                if (DBG) Log.d(TAG, "force screen on");
                                mDeviceHost.updateHostPresence(mHciUiState, mNfccUiState);
                            }

                            if (newParams.shouldEnableDiscovery()) {
                                boolean shouldRestart = mDiscoveryEnabled;
                                mDiscoveryEnabled = mDeviceHost.enableDiscovery(newParams, shouldRestart);
                            } else {
                                mDiscoveryEnabled = mDeviceHost.disableDiscovery();
                            }
                            mCurrentDiscoveryParameters = newParams;
                        } else {
                            Log.d(TAG, "Discovery configuration equal, not updating.");
                        }

                    } else {
                        if ( (mOldScreenState>ScreenStateHelper.SCREEN_STATE_OFF ) &&
                             (mScreenState==ScreenStateHelper.SCREEN_STATE_OFF)    &&
                             (isUiccSecureElementPresent==false) ) {
                              Log.d(TAG, "IF SCREEN STATE OFF and routing to NFCEE disabled, deactivate to IDLE");
                              mDiscoveryEnabled = mDeviceHost.disableDiscovery();
                        }
                    }

                    int newHciUiState;
                    int newNfccUiState;

                    if (mScreenState == ScreenStateHelper.SCREEN_STATE_ON_UNLOCKED) {
                        newHciUiState = HCI_UI_STATE_AVAILABLE;
                        newNfccUiState = NFCC_UI_STATE_UNLOCKED;
                        if (isAidFilterEnabled)
                            mDeviceHost.disableClfAidFilterCondition((byte)0xF1);
                    } else if (mScreenState == ScreenStateHelper.SCREEN_STATE_ON_LOCKED){
                        newHciUiState = HCI_UI_STATE_LOCKED_NOT_NOTIFIABLE;
                        newNfccUiState = NFCC_UI_STATE_LOCKED; // HCE can be supported in screen locked
                        if (isAidFilterEnabled)
                            mDeviceHost.enableClfAidFilterCondition((byte)0xF1);
                    } else {
                        newHciUiState = HCI_UI_STATE_LOCKED_NOT_NOTIFIABLE;
                        newNfccUiState = NFCC_UI_STATE_OFF;
                        if (isAidFilterEnabled)
                            mDeviceHost.enableClfAidFilterCondition((byte)0xF1);
                    }

                    if ((mHciUiState != newHciUiState)||(mNfccUiState != newNfccUiState)) {
                        if (mInProvisionMode) {
                            Log.d(TAG, "Provision Mode");
                            /*
                             * Special case for setup provisioning
                             * locked --> unlocked
                             */
                            mHciUiState = newHciUiState;
                            if (newNfccUiState != NFCC_UI_STATE_OFF) {
                                mNfccUiState = NFCC_UI_STATE_UNLOCKED;
                            } else {
                                mNfccUiState = NFCC_UI_STATE_OFF;
                            }
                        } else {
                            Log.d(TAG, "Update UI State, mLockScreenPolling: " + mLockScreenPolling);
                            mHciUiState = newHciUiState;
                            mNfccUiState = newNfccUiState;
                            if (mLockScreenPolling == false) {
                                if ((mNfccUiState == NFCC_UI_STATE_LOCKED) ||
                                    (mNfccUiState == NFCC_UI_STATE_OFF)) {
                                    // deactivate RF interface if it's not for CE
                                    mDeviceHost.deactivateRfInterface();
                                }
                            } else {
                                if (mNfccUiState == NFCC_UI_STATE_OFF) {
                                    // deactivate RF interface if it's not for CE
                                    mDeviceHost.deactivateRfInterface();
                                }
                            }
                        }
                        mDeviceHost.updateHostPresence(mHciUiState, mNfccUiState);
                    }

                    if ((mOldScreenState==ScreenStateHelper.SCREEN_STATE_OFF ) &&
                        (mScreenState>ScreenStateHelper.SCREEN_STATE_OFF) &&
                        (isUiccSecureElementPresent==false)){
                        Log.d(TAG, "If SCREEN STATE switched from OFF to ON and no EE, enable discovery");
                        if (mCurrentDiscoveryParameters.shouldEnableDiscovery()) {
                            mDiscoveryEnabled = mDeviceHost.enableDiscovery(mCurrentDiscoveryParameters,false);
                        }
                    }

                    mOldScreenState = mScreenState;

                // if UI State update is not supported
                } else {
                    NfcDiscoveryParameters newParams = computeDiscoveryParameters(mScreenState);
                    if (force || !newParams.equals(mCurrentDiscoveryParameters)) {
                        if (newParams.shouldEnableDiscovery()) {
                            boolean shouldRestart = mDiscoveryEnabled;
                            mDiscoveryEnabled = mDeviceHost.enableDiscovery(newParams, shouldRestart);
                        } else {
                            mDiscoveryEnabled = mDeviceHost.disableDiscovery();
                        }
                        mCurrentDiscoveryParameters = newParams;
                    } else {
                        Log.d(TAG, "Discovery configuration equal, not updating.");
                    }
                }

            } finally {
                watchDog.cancel();
            }
        }

    }

    @Override
    public void onUpdateHostCallBack() {
        if(mScreenOffCmdWakeLock.isHeld()) {
            if(DBG) Log.d(TAG, "release mScreenOffCmdWakeLock in onUpdateHostCallBack");
            mScreenOffCmdWakeLock.release();
        }
    }

    private NfcDiscoveryParameters computeDiscoveryParameters(int screenState) {
        //let the polling be configured even on SCREEN_STATE_ON_LOCKED state
        //when UI state is supported (NFCC can enabled/disable polling depends on UI state)
        boolean uiState = mDeviceHost.isUiStateSupported();
        // Recompute discovery parameters based on screen state
        NfcDiscoveryParameters.Builder paramsBuilder = NfcDiscoveryParameters.newBuilder();

        // Polling
        if (screenState >= NFC_POLLING_MODE || uiState) {
            // Check if reader-mode is enabled
            if (mReaderModeParams != null) {
                int techMask = 0;
                if ((mReaderModeParams.flags & NfcAdapter.FLAG_READER_NFC_A) != 0)
                    techMask |= NFC_POLL_A;
                if ((mReaderModeParams.flags & NfcAdapter.FLAG_READER_NFC_B) != 0)
                    techMask |= NFC_POLL_B;
                if ((mReaderModeParams.flags & NfcAdapter.FLAG_READER_NFC_F) != 0)
                    techMask |= NFC_POLL_F;
                if ((mReaderModeParams.flags & NfcAdapter.FLAG_READER_NFC_V) != 0)
                    techMask |= NFC_POLL_ISO15693;
                if ((mReaderModeParams.flags & NfcAdapter.FLAG_READER_NFC_BARCODE) != 0)
                    techMask |= NFC_POLL_KOVIO;

                paramsBuilder.setTechMask(techMask);
                paramsBuilder.setEnableReaderMode(true);
            } else {
                paramsBuilder.setTechMask(NfcDiscoveryParameters.NFC_POLL_DEFAULT);
                paramsBuilder.setEnableP2p(mIsNdefPushEnabled);
            }
        } else if ((screenState == ScreenStateHelper.SCREEN_STATE_ON_LOCKED || uiState) &&
                mInProvisionMode) {
            paramsBuilder.setTechMask(NfcDiscoveryParameters.NFC_POLL_DEFAULT);
            // enable P2P for MFM/EDU/Corp provisioning
            paramsBuilder.setEnableP2p(true);
        } else if ((screenState == ScreenStateHelper.SCREEN_STATE_ON_LOCKED || uiState) &&
                mNfcUnlockManager.isLockscreenPollingEnabled()) {
            // For lock-screen tags, no low-power polling
            paramsBuilder.setTechMask(mNfcUnlockManager.getLockscreenPollMask());
            paramsBuilder.setEnableLowPowerDiscovery(false);
            paramsBuilder.setEnableP2p(false);
        }

        if (mIsHceCapable && (uiState || mScreenState >= ScreenStateHelper.SCREEN_STATE_ON_LOCKED)) {
            // Host routing is always enabled at lock screen or later
            if (mActiveSecureElement.equals("DH") ||
                mCardEmulationManager.hasAnyServices()) {
                Log.d(TAG, "NFC-HCE ON");
                paramsBuilder.setEnableHostRouting(true);
            }
        }

        if (uiState || mScreenState >= ScreenStateHelper.SCREEN_STATE_ON_LOCKED) { //TODO CE on screen state configuration
            if (!mActiveSecureElement.equals("DH")) {
                Log.d(TAG, "NFC-EE ON with " + mActiveSecureElement);
                paramsBuilder.setEnableOffHostRouting(mActiveSecureElement);
            }
        }

        return paramsBuilder.build();
    }

    private boolean isTagPresent() {
        for (Object object : mObjectMap.values()) {
            if (object instanceof TagEndpoint) {
                return ((TagEndpoint) object).isPresent();
            }
        }
        return false;
    }
    /**
     * Disconnect any target if present
     */
    void maybeDisconnectTarget() {
        if (!isNfcEnabledOrShuttingDown()) {
            return;
        }
        Object[] objectsToDisconnect;
        synchronized (this) {
            Object[] objectValues = mObjectMap.values().toArray();
            // Copy the array before we clear mObjectMap,
            // just in case the HashMap values are backed by the same array
            objectsToDisconnect = Arrays.copyOf(objectValues, objectValues.length);
            mObjectMap.clear();
        }
        for (Object o : objectsToDisconnect) {
            if (DBG) Log.d(TAG, "disconnecting " + o.getClass().getName());
            if (o instanceof TagEndpoint) {
                // Disconnect from tags
                TagEndpoint tag = (TagEndpoint) o;
                tag.disconnect();
            } else if (o instanceof NfcDepEndpoint) {
                // Disconnect from P2P devices
                NfcDepEndpoint device = (NfcDepEndpoint) o;
                if (device.getMode() == NfcDepEndpoint.MODE_P2P_TARGET) {
                    // Remote peer is target, request disconnection
                    device.disconnect();
                } else {
                    // Remote peer is initiator, we cannot disconnect
                    // Just wait for field removal
                }
            }
        }
    }

    Object findObject(int key) {
        synchronized (this) {
            Object device = mObjectMap.get(key);
            if (device == null) {
                Log.w(TAG, "Handle not found");
            }
            return device;
        }
    }

    void registerTagObject(TagEndpoint tag) {
        synchronized (this) {
            mObjectMap.put(tag.getHandle(), tag);
        }
    }

    void unregisterObject(int handle) {
        synchronized (this) {
            mObjectMap.remove(handle);
        }
    }

    /**
     * For use by code in this process
     */
    public LlcpSocket createLlcpSocket(int sap, int miu, int rw, int linearBufferLength)
            throws LlcpException {
        return mDeviceHost.createLlcpSocket(sap, miu, rw, linearBufferLength);
    }

    /**
     * For use by code in this process
     */
    public LlcpConnectionlessSocket createLlcpConnectionLessSocket(int sap, String sn)
            throws LlcpException {
        return mDeviceHost.createLlcpConnectionlessSocket(sap, sn);
    }

    /**
     * For use by code in this process
     */
    public LlcpServerSocket createLlcpServerSocket(int sap, String sn, int miu, int rw,
            int linearBufferLength) throws LlcpException {
        return mDeviceHost.createLlcpServerSocket(sap, sn, miu, rw, linearBufferLength);
    }

    public void sendMockNdefTag(NdefMessage msg) {
        sendMessage(MSG_MOCK_NDEF, msg);
    }

    public void routeAids(String aid, int route) {
        Message msg = mHandler.obtainMessage();
        msg.what = MSG_ROUTE_AID;
        msg.arg1 = route;
        msg.obj = aid;
        mHandler.sendMessage(msg);
    }

    public void unrouteAids(String aid) {
        sendMessage(MSG_UNROUTE_AID, aid);
    }

    public void commitRouting() {
        mHandler.sendEmptyMessage(MSG_COMMIT_ROUTING);
    }

    public int getNfceeIdOfSecureElement(String seName) {
        return mDeviceHost.getNfceeId(seName);
    }

    public void setDefaultRoute(String seName) {
        mPrefsEditor.putString(PREF_DEFAULT_ROUTE, seName);
        mPrefsEditor.apply();
        mCardEmulationManager.setDefaultRoute(seName);
        // send message to commit after processing requests from CardEmulationManager
        sendMessage(MSG_UPDATE_COMMIT_DEFAULT_ROUTE, seName);
    }

    public boolean sendData(byte[] data) {
        return mDeviceHost.sendRawFrame(data);
    }

    void sendMessage(int what, Object obj) {
        Message msg = mHandler.obtainMessage();
        msg.what = what;
        msg.obj = obj;
        mHandler.sendMessage(msg);
    }

    public static abstract class NfcServiceExtentionHandler {
        Handler baseHandler;
        private void setHandler(Handler handler) {
            baseHandler = handler;
        }
        public void sendMessage(Message msg) {
            Message encapsulatingMessage = baseHandler.obtainMessage();
            encapsulatingMessage.what=MSG_EXTENTION_MSG;
            encapsulatingMessage.obj=msg;
            baseHandler.sendMessage(encapsulatingMessage);
        }
        public void sendMessage(int what, Object obj) {
            Message msg = new Message();
            msg.what = what;
            msg.obj = obj;
            sendMessage(msg);
        }

        protected abstract void handleMessage(Message msg);
    }

    final class NfcServiceHandler extends Handler {
        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case MSG_ROUTE_AID: {
                    int route = msg.arg1;
                    String aid = (String) msg.obj;
                    if (aid.endsWith("*")) {
                        // remove '*'
                        String prefixAid = aid.substring(0, aid.length() - 1);
                        mDeviceHost.routeAid(hexStringToBytes(prefixAid), route, false, true);
                    } else {
                        mDeviceHost.routeAid(hexStringToBytes(aid), route, false, false);
                    }
                    // Restart polling config
                    break;
                }
                case MSG_UNROUTE_AID: {
                    String aid = (String) msg.obj;
                    if (aid.endsWith("*")) {
                        // remove '*'
                        String prefixAid = aid.substring(0, aid.length() - 1);
                        mDeviceHost.unrouteAid(hexStringToBytes(prefixAid));
                    } else {
                        mDeviceHost.unrouteAid(hexStringToBytes(aid));
                    }
                    break;
                }
                case MSG_INVOKE_BEAM: {
                    mP2pLinkManager.onManualBeamInvoke((BeamShareData)msg.obj);
                    break;
                }
                case MSG_COMMIT_ROUTING: {
                    if (mState == NfcAdapter.STATE_ON) {
                        applyRouting(true);
                    }
                    break;
                }
                case MSG_UPDATE_COMMIT_DEFAULT_ROUTE: {
                    String seName = (String) msg.obj;
                    mDeviceHost.setDefaultRoute(seName) ;
                    break;
                }
                case MSG_MOCK_NDEF: {
                    NdefMessage ndefMsg = (NdefMessage) msg.obj;
                    Bundle extras = new Bundle();
                    extras.putParcelable(Ndef.EXTRA_NDEF_MSG, ndefMsg);
                    extras.putInt(Ndef.EXTRA_NDEF_MAXLENGTH, 0);
                    extras.putInt(Ndef.EXTRA_NDEF_CARDSTATE, Ndef.NDEF_MODE_READ_ONLY);
                    extras.putInt(Ndef.EXTRA_NDEF_TYPE, Ndef.TYPE_OTHER);
                    // app is expecting a TagDTA
                    boolean delivered = false;
                    int dispatchStatus;
                    if (mNfcAdapter.in_dta_mode()) {
                        TagDta tag = TagDta.createMockTag(new byte[]{0x00},
                                new int[]{TagTechnology.NDEF},
                                new Bundle[]{extras});
                        if(mNfcDispatcher.dispatchTag(tag)) {
                            dispatchStatus = NfcDispatcher.DISPATCH_SUCCESS;
                        } else {
                            dispatchStatus = NfcDispatcher.DISPATCH_FAIL;
                        }
                        Log.d(TAG, tag.toString());
                    } else {
                        Tag tag = Tag.createMockTag(new byte[]{0x00},
                                new int[]{TagTechnology.NDEF},
                                new Bundle[]{extras});
                        dispatchStatus = mNfcDispatcher.dispatchTag(tag);
                        Log.d(TAG, tag.toString());
                    }
                    Log.d(TAG, "mock NDEF tag, starting corresponding activity");

                    if (dispatchStatus == NfcDispatcher.DISPATCH_SUCCESS) {
                        playSound(SOUND_END);
                    } else if (dispatchStatus == NfcDispatcher.DISPATCH_FAIL) {
                        playSound(SOUND_ERROR);
                    }
                    break;
                }

                case MSG_NDEF_TAG:
                    if (DBG) Log.d(TAG, "Tag detected, notifying applications");
                    TagEndpoint tag = (TagEndpoint) msg.obj;
                    ReaderModeParams readerParams = null;
                    int presenceCheckDelay = DEFAULT_PRESENCE_CHECK_DELAY;
                    DeviceHost.TagDisconnectedCallback callback =
                            new DeviceHost.TagDisconnectedCallback() {
                                @Override
                                public void onTagDisconnected(long handle) {
                                    applyRouting(false);
                                }
                            };
                    synchronized (NfcService.this) {
                        readerParams = mReaderModeParams;
                    }
                    if (readerParams != null) {
                        presenceCheckDelay = readerParams.presenceCheckDelay;
                        if ((readerParams.flags & NfcAdapter.FLAG_READER_SKIP_NDEF_CHECK) != 0) {
                            if (DBG) Log.d(TAG, "Skipping NDEF detection in reader mode");
                            tag.startPresenceChecking(presenceCheckDelay, callback);
                            dispatchTagEndpoint(tag, readerParams);
                            break;
                        }
                    }

                    boolean playSound = readerParams == null ||
                        (readerParams.flags & NfcAdapter.FLAG_READER_NO_PLATFORM_SOUNDS) == 0;
                    if (mScreenState == ScreenStateHelper.SCREEN_STATE_ON_UNLOCKED && playSound) {
                        playSound(SOUND_START);
                    }
                    if(tag.hasTech(TagTechnology.MIFARE_CLASSIC))
                    {
                        if (DBG) Log.d(TAG, "Mifare Classic Tag Op");
                        dispatchTagEndpoint(tag, readerParams);
                        tag.startPresenceChecking(MIFARE_CLASSIC_PRESENCE_CHECK_DELAY_MS,callback);
                        break;
                    }
                    if (tag.getConnectedTechnology() == TagTechnology.NFC_BARCODE) {
                        // When these tags start containing NDEF, they will require
                        // the stack to deal with them in a different way, since
                        // they are activated only really shortly.
                        // For now, don't consider NDEF on these.
                        if (DBG) Log.d(TAG, "Skipping NDEF detection for NFC Barcode");
                        tag.startPresenceChecking(presenceCheckDelay, callback);
                        dispatchTagEndpoint(tag, readerParams);
                        break;
                    }
                    // <!DTA>
                    if (!mNfcAdapter.in_dta_mode())
                    {
                        NdefMessage ndefMsg = tag.findAndReadNdef();
                        if (ndefMsg != null) {
                            tag.startPresenceChecking(presenceCheckDelay, callback);
                            dispatchTagEndpoint(tag, readerParams);
                        } else {
                            if (tag.reconnect()) {
                                tag.startPresenceChecking(presenceCheckDelay, callback);
                                dispatchTagEndpoint(tag, readerParams);
                            } else {
                                tag.disconnect();
                                playSound(SOUND_ERROR);
                            }
                        }
                        // </!DTA>
                        // <DTA>
                        } else {
                            if (mNfcAdapter.in_dta_mode()) {
                                if (DBG) Log.d(TAG, "[DTA] Tag detected, dta_fake_ndef_tech");
                                //TODO: remove below, no DTA flag to set. DtaHelper is updated in
                                //the constructor of NfcService
                                ((NativeNfcTag)tag).dta_fake_ndef_tech(); // sets DTA flag to true
                                if (DBG) Log.d(TAG, "[DTA] Tag detected,dispatching tag endpoint");
                            }
                            dispatchDtaTagEndpoint(tag, readerParams);
                            if (DBG) Log.d(TAG, "Tag detected, NOT starting presence checking.");
                            // If we are in DTA mode, do not start presence checking so that it
                            // doesn't disrupt the DTA tests. Just mark the tag present.
                            tag.setIsPresent(true);
                        }
                        // </DTA>
                       break;
            case MSG_LLCP_LINK_ACTIVATION:
                    if (mIsDebugBuild) {
                        Intent actIntent = new Intent(ACTION_LLCP_UP);
                        mContext.sendBroadcast(actIntent);
                    }
                    llcpActivated((NfcDepEndpoint) msg.obj);
                    break;

                case MSG_LLCP_LINK_DEACTIVATED:
                    if (mIsDebugBuild) {
                        Intent deactIntent = new Intent(ACTION_LLCP_DOWN);
                        mContext.sendBroadcast(deactIntent);
                    }
                    NfcDepEndpoint device = (NfcDepEndpoint) msg.obj;
                    boolean needsDisconnect = false;

                    Log.d(TAG, "LLCP Link Deactivated message. Restart polling loop.");
                    synchronized (NfcService.this) {
                        /* Check if the device has been already unregistered */
                        if (mObjectMap.remove(device.getHandle()) != null) {
                            /* Disconnect if we are initiator */
                            if (device.getMode() == NfcDepEndpoint.MODE_P2P_TARGET) {
                                if (DBG) Log.d(TAG, "disconnecting from target");
                                needsDisconnect = true;
                            } else {
                                if (DBG) Log.d(TAG, "not disconnecting from initiator");
                            }
                        }
                    }
                    if (needsDisconnect) {
                        device.disconnect();  // restarts polling loop
                    }

                    mP2pLinkManager.onLlcpDeactivated();
                    break;
                case MSG_LLCP_LINK_FIRST_PACKET:
                    mP2pLinkManager.onLlcpFirstPacketReceived();
                    break;
                case MSG_RF_FIELD_ACTIVATED:
                    Intent fieldOnIntent = new Intent(ACTION_RF_FIELD_ON_DETECTED);
                    sendNfcEeAccessProtectedBroadcast(fieldOnIntent);
                    break;
                case MSG_RF_FIELD_DEACTIVATED:
                    Intent fieldOffIntent = new Intent(ACTION_RF_FIELD_OFF_DETECTED);
                    sendNfcEeAccessProtectedBroadcast(fieldOffIntent);
                    break;
                case MSG_REQUEST_RESTART_NFC:
                    {
                        Log.d(TAG, "MSG_REQUEST_RESTART_NFC");
                        new EnableDisableTask().execute(TASK_DISABLE);
                        new EnableDisableTask().execute(TASK_ENABLE);
                    }
                    break;
                case MSG_EXTENTION_MSG:
                    if (mExtentionHandler!=null)
                    {
                        mExtentionHandler.handleMessage((Message) msg.obj);
                    }
                    break;

                case MSG_HCI_EVT_CONNECTIVITY: {
                    int slotId = msg.arg1;

                    if (DBG) Log.d(TAG, "MSG_HCI_EVT_CONNECTIVITY slotID = " + slotId);
                    Intent catIntent = new Intent();
                    catIntent.setAction(CAT_HCI_CONNECTIVITY_ACTION);
                    catIntent.putExtra("SLOT_ID", slotId);
                    mContext.sendBroadcast(catIntent);
                    break;
                }
                case MSG_RESUME_POLLING:
                    mNfcAdapter.resumePolling();
                    break;

                default:
                    Log.e(TAG, "Unknown message received");
                    break;
            }
        }
        // <DTA>
        // Generates a mock NDEF intent which can be used in NTA to detect LLCP link activation
        // Works only if pattern number is greater than 0x1200
        private void indicateLlcpLinkActivation() {
            boolean enableMockLlcpLinkActivatedCallback = true;
            if ( enableMockLlcpLinkActivatedCallback &&  mDeviceHost.dta_get_pattern_number() >= 0x1200) {
                try {
                    String txt = "dta_llcp_activated"; // Just something
                    NdefRecord textRecord = NdefRecord.createMime("text/plain", txt.getBytes());
                    NdefMessage ndef = new NdefMessage(textRecord);
                    if (DBG) Log.d(TAG, "[DTA] Signalling LLCP link activation using a fake tag event");
                    sendMockNdefTag(ndef);
                } catch (Exception e) {
                    if (DBG) Log.e(TAG, "[DTA] failed mock NdefMessage generation");
                }
            }
        }
        // </DTA>

        private void sendNfcEeAccessProtectedBroadcast(Intent intent) {
            intent.addFlags(Intent.FLAG_INCLUDE_STOPPED_PACKAGES);
            // Resume app switches so the receivers can start activites without delay
            mNfcDispatcher.resumeAppSwitches();

            synchronized (this) {
                for (PackageInfo pkg : mInstalledPackages) {
                    if (pkg != null && pkg.applicationInfo != null) {
                        if (mNfceeAccessControl.check(pkg.applicationInfo)) {
                            intent.setPackage(pkg.packageName);
                            mContext.sendBroadcast(intent);
                        }
                    }
                }
            }
        }

        private boolean llcpActivated(NfcDepEndpoint device) {
            Log.d(TAG, "LLCP Activation message");

            if (device.getMode() == NfcDepEndpoint.MODE_P2P_TARGET) {
                if (DBG) Log.d(TAG, "NativeP2pDevice.MODE_P2P_TARGET");
                if (device.connect()) {
                    /* Check LLCP compliancy */
                    if (mDeviceHost.doCheckLlcp()) {
                        /* Activate LLCP Link */
                        if (mDeviceHost.doActivateLlcp()) {
                            if (DBG) Log.d(TAG, "Initiator Activate LLCP OK");
                            synchronized (NfcService.this) {
                                // Register P2P device
                                mObjectMap.put(device.getHandle(), device);
                            }
                            mP2pLinkManager.onLlcpActivated();
                            //<DTA>
                            indicateLlcpLinkActivation();
                            //</DTA>
                            return true;
                        } else {
                            /* should not happen */
                            Log.w(TAG, "Initiator LLCP activation failed. Disconnect.");
                            device.disconnect();
                        }
                    } else {
                        if (DBG) Log.d(TAG, "Remote Target does not support LLCP. Disconnect.");
                        device.disconnect();
                    }
                } else {
                    if (DBG) Log.d(TAG, "Cannot connect remote Target. Polling loop restarted.");
                    /*
                     * The polling loop should have been restarted in failing
                     * doConnect
                     */
                }
            } else if (device.getMode() == NfcDepEndpoint.MODE_P2P_INITIATOR) {
                if (DBG) Log.d(TAG, "NativeP2pDevice.MODE_P2P_INITIATOR");
                /* Check LLCP compliancy */
                if (mDeviceHost.doCheckLlcp()) {
                    /* Activate LLCP Link */
                    if (mDeviceHost.doActivateLlcp()) {
                        if (DBG) Log.d(TAG, "Target Activate LLCP OK");
                        synchronized (NfcService.this) {
                            // Register P2P device
                            mObjectMap.put(device.getHandle(), device);
                        }
                        mP2pLinkManager.onLlcpActivated();
                        //<DTA>
                        indicateLlcpLinkActivation();
                        //</DTA>
                        return true;
                    }
                } else {
                    Log.w(TAG, "checkLlcp failed");
                }
            }

            return false;
        }

        private void dispatchTagEndpoint(TagEndpoint tagEndpoint, ReaderModeParams readerParams) {
            Tag tag = new Tag(tagEndpoint.getUid(), tagEndpoint.getTechList(),
                    tagEndpoint.getTechExtras(), tagEndpoint.getHandle(), mNfcTagService);
            registerTagObject(tagEndpoint);
            if (readerParams != null) {
                try {
                    if ((readerParams.flags & NfcAdapter.FLAG_READER_NO_PLATFORM_SOUNDS) == 0) {
                        playSound(SOUND_END);
                    }
                    if (readerParams.callback != null) {
                        readerParams.callback.onTagDiscovered(tag);
                        return;
                    } else {
                        // Follow normal dispatch below
                    }
                } catch (RemoteException e) {
                    Log.e(TAG, "Reader mode remote has died, falling back.", e);
                    // Intentional fall-through
                } catch (Exception e) {
                    // Catch any other exception
                    Log.e(TAG, "App exception, not dispatching.", e);
                    return;
                }
            }
            int dispatchResult = mNfcDispatcher.dispatchTag(tag);
            if (dispatchResult == NfcDispatcher.DISPATCH_FAIL) {
                unregisterObject(tagEndpoint.getHandle());
                playSound(SOUND_ERROR);
            } else if (dispatchResult == NfcDispatcher.DISPATCH_SUCCESS) {
                playSound(SOUND_END);
            }
        }

        private void dispatchDtaTagEndpoint(TagEndpoint tagEndpoint, ReaderModeParams readerParams) {
            TagDta tag = new TagDta(tagEndpoint.getUid(), tagEndpoint.getTechList(),
                    tagEndpoint.getTechExtras(), tagEndpoint.getHandle(), mNfcTagServiceDta);
            registerTagObject(tagEndpoint);
            if (readerParams != null) {
                Log.e(TAG, "ReaderModeParams not supported in DTA mode");
            }
            if (!mNfcDispatcher.dispatchTag(tag)) {
                unregisterObject(tagEndpoint.getHandle());
                playSound(SOUND_ERROR);
            } else {
                playSound(SOUND_END);
            }
        }
    }

    void sendNfcEeAccessProtectedBroadcast(Intent intent) {
        mHandler.sendNfcEeAccessProtectedBroadcast(intent);
    }
    private NfcServiceExtentionHandler mExtentionHandler = null;
    private NfcServiceHandler mHandler = new NfcServiceHandler();

    class ApplyRoutingTask extends AsyncTask<Integer, Void, Void> {
        @Override
        protected Void doInBackground(Integer... params) {
            synchronized (NfcService.this) {
                if (params == null || params.length != 1) {
                    // force apply current routing
                    applyRouting(true);
                    return null;
                }
                mScreenState = params[0].intValue();

                mRoutingWakeLock.acquire();
                try {
                    applyRouting(false);
                } finally {
                    mRoutingWakeLock.release();
                }
                return null;
            }
        }
    }

    private final BroadcastReceiver mProactiveReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            if (action.equals(CAT_ACTIVATE_NOTIFY_ACTION))
            {
                Log.d(TAG, "recieved ProactiveActivate intent");
                int slot = intent.getIntExtra(SLOT_ID_KEY_STRING,-1);
                if(slot <= -1)
                    Log.e(TAG, "malformed ProactiveActivate intent");
                else
                     ((NativeNfcManager)mDeviceHost).doActivateSwp((byte)slot);
            }
        }
    };


    private final BroadcastReceiver mReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            if (action.equals(Intent.ACTION_SCREEN_ON)
                    || action.equals(Intent.ACTION_SCREEN_OFF)
                    || action.equals(Intent.ACTION_USER_PRESENT)) {
                // Perform applyRouting() in AsyncTask to serialize blocking calls
                int screenState = ScreenStateHelper.SCREEN_STATE_OFF;
                if (action.equals(Intent.ACTION_SCREEN_OFF)) {
                    if (mState == NfcAdapter.STATE_ON) {
                        if (DBG) Log.d(TAG, "acquiring mScreenOffCmdWakeLock for screen off cmd");
                        mScreenOffCmdWakeLock.acquire();
                    }
                } else if (action.equals(Intent.ACTION_SCREEN_ON)) {
                    screenState = mKeyguard.isKeyguardLocked()
                            ? ScreenStateHelper.SCREEN_STATE_ON_LOCKED
                            : ScreenStateHelper.SCREEN_STATE_ON_UNLOCKED;
                } else if (action.equals(Intent.ACTION_USER_PRESENT)) {
                    screenState = ScreenStateHelper.SCREEN_STATE_ON_UNLOCKED;
                }

                new ApplyRoutingTask().execute(Integer.valueOf(screenState));
            } else if (action.equals(Intent.ACTION_AIRPLANE_MODE_CHANGED)) {
                boolean isAirplaneModeOn = intent.getBooleanExtra("state", false);
                // Query the airplane mode from Settings.System just to make sure that
                // some random app is not sending this intent
                if (isAirplaneModeOn != isAirplaneModeOn()) {
                    return;
                }
                if (!mIsAirplaneSensitive) {
                    return;
                }
                mPrefsEditor.putBoolean(PREF_AIRPLANE_OVERRIDE, false);
                mPrefsEditor.apply();
                if (isAirplaneModeOn) {
                    new EnableDisableTask().execute(TASK_DISABLE);
                } else if (!isAirplaneModeOn && mPrefs.getBoolean(PREF_NFC_ON, NFC_ON_DEFAULT)) {
                    mNfcOnWhileAirplaneModeOn = false;
                    new EnableDisableTask().execute(TASK_ENABLE);
                }
            } else if (action.equals(Intent.ACTION_USER_SWITCHED)) {
                int userId = intent.getIntExtra(Intent.EXTRA_USER_HANDLE, 0);
                synchronized (this) {
                    mUserId = userId;
                }
                mP2pLinkManager.onUserSwitched(getUserId());
                if (mIsHceCapable) {
                    mCardEmulationManager.onUserSwitched(getUserId());
                }
            }
        }
    };


    private final BroadcastReceiver mOwnerReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            if (action.equals(Intent.ACTION_PACKAGE_REMOVED) ||
                    action.equals(Intent.ACTION_PACKAGE_ADDED) ||
                    action.equals(Intent.ACTION_EXTERNAL_APPLICATIONS_AVAILABLE) ||
                    action.equals(Intent.ACTION_EXTERNAL_APPLICATIONS_UNAVAILABLE)) {
                updatePackageCache();

                if (action.equals(Intent.ACTION_PACKAGE_REMOVED)) {
                    // Clear the NFCEE access cache in case a UID gets recycled
                    mNfceeAccessControl.invalidateCache();
                }
            }
        }
    };

    private final BroadcastReceiver mPolicyReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent){
            String action = intent.getAction();
            if (DevicePolicyManager.ACTION_DEVICE_POLICY_MANAGER_STATE_CHANGED
                    .equals(action)) {
                enforceBeamShareActivityPolicy(context,
                        new UserHandle(getSendingUserId()), mIsNdefPushEnabled);
            }
        }
    };

    /**
     * Returns true if airplane mode is currently on
     */
    boolean isAirplaneModeOn() {
        return Settings.System.getInt(mContentResolver,
                Settings.Global.AIRPLANE_MODE_ON, 0) == 1;
    }

    /**
     * for debugging only - no i18n
     */
    static String stateToString(int state) {
        switch (state) {
            case NfcAdapter.STATE_OFF:
                return "off";
            case NfcAdapter.STATE_TURNING_ON:
                return "turning on";
            case NfcAdapter.STATE_ON:
                return "on";
            case NfcAdapter.STATE_TURNING_OFF:
                return "turning off";
            default:
                return "<error>";
        }
    }

    void dump(FileDescriptor fd, PrintWriter pw, String[] args) {
        if (mContext.checkCallingOrSelfPermission(android.Manifest.permission.DUMP)
                != PackageManager.PERMISSION_GRANTED) {
            pw.println("Permission Denial: can't dump nfc from from pid="
                    + Binder.getCallingPid() + ", uid=" + Binder.getCallingUid()
                    + " without permission " + android.Manifest.permission.DUMP);
            return;
        }

        synchronized (this) {
            pw.println("mState=" + stateToString(mState));
            pw.println("mIsZeroClickRequested=" + mIsNdefPushEnabled);
            pw.println("mScreenState=" + ScreenStateHelper.screenStateToString(mScreenState));
            pw.println("mIsAirplaneSensitive=" + mIsAirplaneSensitive);
            pw.println("mIsAirplaneToggleable=" + mIsAirplaneToggleable);
            pw.println(mCurrentDiscoveryParameters);
            mP2pLinkManager.dump(fd, pw, args);
            if (mIsHceCapable) {
                mCardEmulationManager.dump(fd, pw, args);
            }
            mNfcDispatcher.dump(fd, pw, args);
            pw.println(mDeviceHost.dump());
        }
    }

    class BertTask extends AsyncTask<Integer[], Void, Void> {
        @Override
        protected Void doInBackground(Integer[]... params) {
            Integer[] techRate = params[0];
            /* Initialize and turn carrier on */
            mDeviceHost.PrbsOn(techRate[0], techRate[1], true);
            /* Execute in loop until cancelled */
            do {
                mDeviceHost.PrbsOn(techRate[0], techRate[1], false);
                try {
                    Thread.sleep(10);
                } catch (InterruptedException e) {
                    Log.w(TAG, "Interrupted!");
                }
            } while (isCancelled() != true);
            return null;
        }
    }

    void PrbsOn(int tech, int rate) {
        NfcPermissions.enforceUserPermissions(mContext);
        synchronized(this) {
            mBertWakeLock.acquire();
            new EnableDisableTask().execute(TASK_DISABLE);
            Integer[] techRate = {tech, rate};
            mBertTask = new BertTask();
            mBertTask.execute(techRate);
        }
    }

    void PrbsOff() {
        NfcPermissions.enforceUserPermissions(mContext);
        synchronized(this) {
            mBertTask.cancel(true);
            mDeviceHost.PrbsOff();
            if(mBertWakeLock.isHeld())
                mBertWakeLock.release();
            boolean airplaneOverride = mPrefs.getBoolean(PREF_AIRPLANE_OVERRIDE, false);
            if (mPrefs.getBoolean(PREF_NFC_ON, NFC_ON_DEFAULT) &&
                    (!mIsAirplaneSensitive || !isAirplaneModeOn() || airplaneOverride)) {
                Log.d(TAG,"Turn NFC back on!");
                new EnableDisableTask().execute(TASK_ENABLE);
            }
        }
    }

    public byte[] NfccInfo(){
        if(DBG) {
            Log.d(TAG,"COPIED NFCC INFO length" + nfcc_info.length);
            for(int i=0;i<nfcc_info.length;i++)
            {
                Log.d(TAG,"COPIED NFCC INFO" + nfcc_info[i]);
            }
        }
        return nfcc_info;
    }

    public static class InfoService extends Service {
        private IGetNFCByteArray.Stub myService = new IGetNFCByteArray.Stub() {
            @Override
            public byte[] GetNfccInfo(){
                return sService.NfccInfo();
            }

            @Override
            public byte[] CollectRamDump(int addr , int len){
                byte[] result = sService.mDeviceHost.GetRamDump(addr ,len);
                return result;
            }

            @Override
            public void doPrbsOn(int tech, int rate) {
                sService.PrbsOn(tech, rate);
            }

            @Override
            public void doPrbsOff() {
                sService.PrbsOff();
            }
        };

        @Override
        public IBinder onBind(Intent intent) {
        Log.d(getClass().getSimpleName(),"onBind()");
        return myService.asBinder();
        }

        @Override
        public void onDestroy() {
        Log.d(getClass().getSimpleName(),"onDestroy()");
        super.onDestroy();
        }
    }
}
