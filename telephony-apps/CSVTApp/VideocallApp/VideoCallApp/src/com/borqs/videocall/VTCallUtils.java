/*
 * ©2012-2014 Borqs Software Solutions Pvt Ltd. All rights reserved.
 */

package com.borqs.videocall;

import java.io.File;
import java.io.IOException;
import java.util.Calendar;

import com.android.internal.telephony.CallerInfo;
import com.android.internal.telephony.CallerInfoAsyncQuery;
import com.android.internal.telephony.CallManager;
import com.android.internal.telephony.Phone;

import android.telephony.TelephonyManager;
//import android.telephony.MSimTelephonyManager;
import android.provider.MediaStore;
import android.provider.Settings;

import android.app.AlertDialog;
import android.content.ComponentName;
import android.content.ContentResolver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.ServiceConnection;
import android.content.SharedPreferences;
import android.graphics.Bitmap;
import android.media.AudioManager;
import android.media.MediaScanner;
import android.telephony.PhoneNumberUtils;
import android.telephony.ServiceState;
import android.text.TextUtils;
import android.util.Log;
import android.view.WindowManager;
import android.widget.Toast;

import android.content.Intent;
import android.database.Cursor;
import android.net.Uri;

import android.os.IBinder;
import android.os.ParcelFileDescriptor;
import android.os.Process;
import android.os.Environment;
import android.os.StatFs;
import android.os.SystemClock;
import android.provider.Contacts;

import java.io.FileReader;
import android.os.ServiceManager;
import android.os.IPowerManager;
import android.os.RemoteException;
import android.os.Handler;

import java.io.FileNotFoundException;
import org.codeaurora.ims.csvt.ICsvtService;
import android.content.BroadcastReceiver;
import android.telephony.SubscriptionManager;

class VTBtHeadsetReceiver extends BroadcastReceiver {

    private static final String ACTION_BT_HEADSET_STATE_CHANGED = "android.bluetooth.headset.profile.action.CONNECTION_STATE_CHANGED";
    private static final String EXTRA_STATE = "android.bluetooth.profile.extra.STATE";
    private static final int STATE_CONNECTED = 2;
    private static final int STATE_DISCONNETED = 0;
    private static final String TAG = "VTBTHEADSETRECVR";

    @Override
    public void onReceive(Context context, Intent intent) {
        String action = intent.getAction();
        final VideoCallApp mApp = VideoCallApp.getInstance();
        final Handler mAppHandler = VideoCallApp.getInstance().mHandler;
        if (action == null)
            return;
        if (action.equals(ACTION_BT_HEADSET_STATE_CHANGED)) {
            if (MyLog.DEBUG)
                MyLog.d(TAG, "Received (1) ACTION_BT_HEADSET_STATE_CHANGED");
            int extraData = intent.getIntExtra(EXTRA_STATE, STATE_DISCONNETED);
            if (extraData == STATE_CONNECTED) {
                if (MyLog.DEBUG)
                    MyLog.d(TAG, "BT HEADSET IS CONNECTED");
                if (mAppHandler != null) {
                    mAppHandler.sendMessage(mAppHandler.obtainMessage(
                            VideoCallApp.MSG_BLUETOOTH_PLUG, 1, 0));
                }
                mApp.bBtHeadsetPlugged = true;
            } else if (extraData == STATE_DISCONNETED) {
                if (MyLog.DEBUG)
                    MyLog.d(TAG, "BT HEADSET IS DISCONNECTED");
                if (mAppHandler != null) {
                    mAppHandler.sendMessage(mAppHandler.obtainMessage(
                            VideoCallApp.MSG_BLUETOOTH_PLUG, 0));
                }
                mApp.bBtHeadsetPlugged = false;
            }
        }
    }
}

public class VTCallUtils {

    static final String TAG = " VTCallUtils.";

    static final int SDCARD_OK = 0;
    static final int SDCARD_NOTFOUND = -1;
    static final int SDCARD_UNWRITABLE = -2;
    static final int SDCARD_FULL = -3;
    static final int SDCARD_BAD = -4;
    static final int SDCARD_BUSY = -5;
    static final float SDCARD_FULL_LEFT = 10485760F;
    private static final String CUSTOM_RINGTON = "rington_in_sd_card";
    private static final String HEADSET_OLD_STATE_PATH = "/sys/class/accessory/headset/online";
    private static final String HEADSET_STATE_PATH = "/sys/class/switch/h2w/state";
    private static final int BIT_HEADSET = (1 << 0);
    private static final int BIT_HEADSET_NO_MIC = (1 << 1);
    private static final int BIT_TTY = (1 << 2);
    private static final int BIT_FM_HEADSET = (1 << 3);
    private static final int BIT_FM_SPEAKER = (1 << 4);

    private static final String MINI_THUMBNAILS_DIRECTORY = "/sdcard/.thumbnails/minithumb";
    private static final String THUMBNAILS_DIRECTORY = "/sdcard/.thumbnails/thumb";
    public static final String DIRECTORY = Environment.getExternalStorageDirectory().toString()
            + "/Pictures";;

    private static final Uri SCIMITAR_FILTER_URI = Uri.parse("content://scimitar/contacts/filter");
    private static int BLOCK_TYPE_SMS = 1;
    private static int BLOCK_TYPE_VOICE = 2;
    private static int BLOCK_TYPE_VT_CALL = 4;

    private static Byte[] lockToCheckWriteAble = new Byte[0];

    public static final int OUTGOING_FAILED_TYPE = 4;
    public static final int INCOMING_CSVT_TYPE = 1;
    public static final int OUTGOING_CSVT_TYPE = 2;
    public static final int MISSED_CSVT_TYPE = 3;

    private static ICsvtService mCsvtService;

    public static class Alerter {
        public static AlertDialog doAlert(Context context, String msg) {
            return new AlertDialog.Builder(context).setMessage(msg)
                    .setIcon(R.drawable.cmcc_dialog_information)
                    .setTitle(R.string.alert_dialog_title)
                    .setPositiveButton(android.R.string.ok, null).setCancelable(false).show();
        }
    }

    // return the stored file name which is dynamically generated
    public static String storeImageToFile(Context ctx, Bitmap source, String storeDir) {
        Log.e(TAG, "=========enter storeImageToFile============");
        if (source == null) {
            Toast.makeText(ctx, ctx.getText(R.string.capture_picture_failure), Toast.LENGTH_SHORT)
                    .show();
            Log.e(TAG, "Error in storeimagetofile: source is null");
            return null;
        }

        MediaScanner Scanner;
        Scanner = new MediaScanner(ctx);

        // todo read dir from preference,file name should be more readable
        File sdDir;
        try {
            sdDir = new File(storeDir);
            if (!sdDir.exists()) {
                if (MyLog.DEBUG)
                    MyLog.v(TAG, "no such path mk dir " + storeDir);
                if (false == sdDir.mkdirs()) {
                    if (MyLog.DEBUG)
                        MyLog.v(TAG, "error to create dir " + storeDir);
                }
            }
        } catch (Exception ex) {
            Log.e(TAG, "Error in create dir " + storeDir + " " + ex);
            return null;
        }
        Calendar mCalendar = Calendar.getInstance();
        String month, day, hour, min, sec;
        if ((mCalendar.get(Calendar.MONTH) + 1) < 10) {
            month = "0" + (mCalendar.get(Calendar.MONTH) + 1);
        } else {
            month = "" + (mCalendar.get(Calendar.MONTH) + 1);
        }
        if (mCalendar.get(Calendar.DAY_OF_MONTH) < 10) {
            day = "0" + mCalendar.get(Calendar.DAY_OF_MONTH);
        } else {
            day = "" + mCalendar.get(Calendar.DAY_OF_MONTH);
        }
        if (mCalendar.get(Calendar.HOUR_OF_DAY) < 10) {
            hour = "0" + mCalendar.get(Calendar.HOUR_OF_DAY);
        } else {
            hour = "" + mCalendar.get(Calendar.HOUR_OF_DAY);
        }
        if (mCalendar.get(Calendar.MINUTE) < 10) {
            min = "0" + mCalendar.get(Calendar.MINUTE);
        } else {
            min = "" + mCalendar.get(Calendar.MINUTE);
        }
        if (mCalendar.get(Calendar.HOUR_OF_DAY) < 10) {
            sec = "0" + mCalendar.get(Calendar.SECOND);
        } else {
            sec = "" + mCalendar.get(Calendar.SECOND);
        }
        String fileName = storeDir + "/VT" + mCalendar.get(Calendar.YEAR) + month + day + "_"
                + hour + min + sec + ".jpg";

        Log.e(TAG, "Write file : " + fileName);
        try {
            java.io.OutputStream fos = new java.io.FileOutputStream(fileName);
            source.compress(Bitmap.CompressFormat.JPEG, 100, fos);
            fos.close();
            Scanner.scanSingleFile(fileName, "external", "image/jpeg");
            Toast.makeText(ctx, ctx.getText(R.string.finish_capture_photo) + storeDir,
                    Toast.LENGTH_SHORT).show();
        } catch (Exception ex) {
            Toast.makeText(ctx, R.string.capture_picture_failure, Toast.LENGTH_SHORT).show();
            Log.e(TAG, "Error in storeimagetofile:" + ex);
            return null;
        }
        Log.e(TAG, "leave storeImageToFile");

        return "fakefile";
    }

    private static final int QUERY_TOKEN = -1;

    /*
     * Launch 2G voice call
     *
     * argus: String number
     */

    static public boolean launch2GCall(String number, Context ctx) {
        if (TextUtils.isEmpty(number)) {
            return false;
        }

        Intent intent = new Intent(Intent.ACTION_CALL);
        /*
         * FIXME: is emergency calling necessary for VT? //
         * OutgongCallBroadcaster will check this action against the number if
         * (PhoneNumberUtils.isEmergencyNumber(number)) {
         * intent.setAction(Intent.ACTION_CALL_EMERGENCY); }
         */
        if(TelephonyManager.getDefault().isMultiSimEnabled()){
            intent.putExtra("subscription",0);
        }
        intent.setData(Uri.fromParts("tel", number, null));
        /* not sure about the use of 'NEW_TASK_LAUNCH' flag */
        // intent.setLaunchFlags(Intent.EXCLUDE_FROM_RECENTS_LAUNCH |
        // Intent.NEW_TASK_LAUNCH);
        intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_ACTIVITY_EXCLUDE_FROM_RECENTS);
        ctx.startActivity(intent);
        return true;
    }

    public static int checkStorage() {
        int sdcardStatus = hasStorage(true);
        if (MyLog.DEBUG)
            MyLog.d(TAG, "checkStorage sdcardStatus:" + sdcardStatus);
        if (sdcardStatus != SDCARD_OK) {
            return sdcardStatus;
        } else {
            // todo this path really doesn't belong here
            /*
             * if (picturesRemaining() <= 0) { sdcardStatus = SDCARD_FULL; }
             */
        }
        return sdcardStatus;
    }

    public static long picturesRemaining() {
        try {
            if (SDCARD_OK != hasStorage(true))
                return 0;
            String storageDirectory = Environment.getExternalStorageDirectory().toString();
            StatFs stat = new StatFs(storageDirectory);
            float remaining = ((float) stat.getAvailableBlocks() * (float) stat.getBlockSize())
                    / SDCARD_FULL_LEFT;
            return (long) remaining;
        } catch (Exception ex) {
            // if we can't stat the filesystem then we don't know how many
            // pictures are remaining. it might be zero but just leave it
            // blank since we really don't know.
            return -1;
        }
    }

    public static void showStorageToast(int sdcardStatus, Context ctx) {
        String noStorageText = null;

        switch (sdcardStatus) {
        case SDCARD_UNWRITABLE:
            noStorageText = ctx.getString(R.string.sdcard_unwritable);
            break;
        case SDCARD_FULL:
            noStorageText = ctx.getString(R.string.not_enough_space);
            break;
        case SDCARD_NOTFOUND:
            noStorageText = ctx.getString(R.string.no_storage);
            break;
        case SDCARD_BAD:
            noStorageText = ctx.getString(R.string.bad_storage);
            break;
        case SDCARD_BUSY:
            noStorageText = ctx.getString(R.string.storage_card_is_busy);
            break;
        default:
            break;
        }

        if (noStorageText != null) {
            Toast toast;
            toast = Toast.makeText(ctx, "", Toast.LENGTH_LONG);

            toast.setText(noStorageText);
            toast.show();
        }
    }

    static private boolean isFileSystemEdiatable() {
        synchronized (lockToCheckWriteAble) {
            String directoryName = Environment.getExternalStorageDirectory().toString()
                    + "/Pictures";
            File directory = new File(directoryName);
            if ((!directory.isDirectory()) && (!directory.mkdirs())) {
                return false;
            }
            File minithumbDir = new File(MINI_THUMBNAILS_DIRECTORY);
            if ((!minithumbDir.isDirectory()) && (!minithumbDir.mkdirs())) {
                return false;
            }
            File thumbDir = new File(THUMBNAILS_DIRECTORY);
            if ((!thumbDir.isDirectory()) && (!thumbDir.mkdirs())) {
                return false;
            }
            File file = new File(directoryName, ".probe");
            try {
                // Remove existing file if any
                if (file.exists()) {
                    file.delete();
                }
                if (!file.createNewFile())
                    return false;
                file.delete();
                return true;
            } catch (IOException ex) {
                return false;
            }
        }
    }

    static float calculateRemainStorage() {
        String storageDirectory = Environment.getExternalStorageDirectory().toString();
        StatFs stat = new StatFs(storageDirectory);
        float remainStorage = (float) stat.getAvailableBlocks() * (float) stat.getBlockSize();
        return remainStorage;
    }

    static private int hasStorage(boolean requireWriteAccess) {
        String state = Environment.getExternalStorageState();
        if (Environment.MEDIA_MOUNTED.equals(state)) {
            if (requireWriteAccess) {
                String storageDirectory = Environment.getExternalStorageDirectory().toString();
                StatFs stat;
                try {
                    stat = new StatFs(storageDirectory);
                } catch (Exception ex) {
                    return SDCARD_BAD;
                }
                if (MyLog.DEBUG)
                    MyLog.d(TAG, "stat.getAvailableBlocks:" + (float) stat.getAvailableBlocks()
                            + "getBlockSize:" + (float) stat.getBlockSize());
                File dir = new File(DIRECTORY);
                dir.mkdirs();
                if (!dir.isDirectory() || !dir.canWrite()) {
                    return SDCARD_NOTFOUND;
                }

                if ((float) stat.getAvailableBlocks() * (float) stat.getBlockSize() < SDCARD_FULL_LEFT) {
                    return SDCARD_FULL;
                }
                boolean writable = isFileSystemEdiatable();
                if (writable) {
                    return SDCARD_OK;
                } else {
                    return SDCARD_UNWRITABLE;
                }
            } else {
                return SDCARD_OK;
            }
        } else if (Environment.MEDIA_SHARED.equals(state)) {
            return SDCARD_BUSY;
        } else if (!requireWriteAccess && Environment.MEDIA_MOUNTED_READ_ONLY.equals(state)) {
            return SDCARD_OK;
        }
        return SDCARD_NOTFOUND;
    }


    private static int getBlockType(Context ctx, String number) {
        int ret = 0;
        try {
            ContentResolver cr = ctx.getContentResolver();

            Uri uri = Uri.withAppendedPath(SCIMITAR_FILTER_URI, number);
            Cursor c = cr.query(uri, new String[] { "type" }, null, null, null);
            if (c != null) {
                try {
                    if (0 == c.getCount()) {
                        ret = 0;
                    } else {
                        c.moveToFirst();
                        int type = c.getInt(0);
                        if (MyLog.DEBUG)
                            Log.d(TAG, "isBlocked(), number: " + number + ", type: " + type);
                        ret = type;
                    }
                } finally {
                    c.close();
                }
            }
        } catch (Exception e) {
            if (MyLog.DEBUG)
                Log.d(TAG, "Exception: " + e.getMessage());
        }

        return ret;
    }

    /*
     * check the new connection is in black list or not @param number the number
     * to be checked @return true means this number is in black list; false
     * otherwise
     */
    static boolean isBlockedNumber(Context ctx, String number) {
        return (BLOCK_TYPE_VT_CALL & getBlockType(ctx, number)) > 0;
    }

    public static boolean isCSVTEnabled() {
        return true;
    }

    public static ICsvtService getCsvtService() {
        return mCsvtService;
    }

    public static void createCsvtService(Context ctx) {
        // feature.
        Log.d(TAG, "in createCsvtService");

        if (mCsvtService != null)
            return;

        if (isCSVTEnabled()) {
            try {
                Intent intent = new Intent(ICsvtService.class.getName());
                intent.setClassName("org.codeaurora.ims",
                        "com.qualcomm.ims.csvt.CsvtService");
                Log.d(TAG, "createCsvtService: intent = " + intent);
                boolean bound = ctx.bindService(intent, mCsvtServiceConnection,
                        Context.BIND_AUTO_CREATE);
                Log.d(TAG, "ICsvtService bound request : " + bound);
            } catch (NoClassDefFoundError e) {
                Log.w(TAG, "Ignoring ICsvtService class not found exception " + e);
            }
        }
    }

    private static ServiceConnection mCsvtServiceConnection = new ServiceConnection() {
        public void onServiceConnected(ComponentName name, IBinder service) {
            mCsvtService = ICsvtService.Stub.asInterface(service);
            Log.d(TAG, "Csvt Service Connected" + mCsvtService);
        }

        public void onServiceDisconnected(ComponentName arg0) {
            Log.w(TAG, "Csvt Service onServiceDisconnected");
        }
    };

    static boolean isVoiceIdle() {

        if (mCsvtService != null) {
            try {
                if (!mCsvtService.isNonCsvtIdle())
                    return false;
            } catch (Exception e) {
                Log.e(TAG, "Exception when getIccFdnEnabled");
            }
        } else {
            Log.e(TAG, "Error, cannot get videophoneinterfacemanager");
        }
        return true;
    }

    static boolean isVTActive() {
        if (mCsvtService != null) {
            try {
                if (mCsvtService.isActive())
                    return true;
            } catch (Exception e) {
                Log.e(TAG, "Exception when getIccFdnEnabled");
            }
        } else {
            Log.e(TAG, "Error, cannot get videophoneinterfacemanager");
        }
        return false;
    }

    static boolean isFdnEnabled() {
        // ICsvtService vtInterface = null;
        // vtInterface =
        // ICsvtService.Stub.asInterface(ServiceManager.checkService(VideoCallApp.VIDEO_CALL_SERVICE));
        // if(vtInterface != null) {
        // try {
        // return vtInterface.getIccFdnEnabled();
        // } catch (Exception e) {
        // Log.e(TAG, "Exception when getIccFdnEnabled");
        // }
        // }
        // else {
        // Log.e(TAG, "Error, cannot get videophoneinterfacemanager");
        // }
        return true;
    }

    // return 0 means successfull, else return a error message resource id
    static int checkMOInitStatus(ContentResolver cr, String number) {
        if (MyLog.DEBUG)
            MyLog.d(TAG, "checkMOInitStatus, number" + number);

        if (number == null || number.equals("")) {
            if (MyLog.DEBUG)
                MyLog.d(TAG, "checkMOInitStatus: null number");
            return R.string.no_phone_number_supplied_prompt;
        }

        if (isInvalidNumber(number)) {
            if (MyLog.DEBUG)
                MyLog.d(TAG, "checkMOInitStatus: invalid number");
            return R.string.no_phone_number_supplied_prompt;
        }

        // check for emrgency number
        if (PhoneNumberUtils.isEmergencyNumber(number)) {
            if (MyLog.DEBUG)
                MyLog.d(TAG, "checkMOInitStatus, emergency number = " + number);
            return R.string.emergency_number_prompt;
        }

        // check flying mode
        int airplanMode = Settings.System.getInt(cr, Settings.System.AIRPLANE_MODE_ON, 0);
        if (MyLog.DEBUG)
            MyLog.d(TAG, "checkMOInitStatus, airplanMode = " + airplanMode);
        if (airplanMode != 0) {
            return R.string.flying_mode_prompt;
        }

        if (TelephonyManager.getDefault().isMultiSimEnabled()) { // Multiple
                                                                     // simcards
                                                                     // are
                                                                     // active
             // check whether sim card is ready
            int simState0 = TelephonyManager.getDefault().getSimState(0);
            int simState1 = TelephonyManager.getDefault().getSimState(1);
            if (MyLog.DEBUG)
                MyLog.d(TAG, "checkMOInitStatus, simState0 = " + simState0);
                MyLog.d(TAG, "checkMOInitStatus, simState1 = " + simState1);
            if (simState0 != TelephonyManager.SIM_STATE_READY
                    && simState1 != TelephonyManager.SIM_STATE_READY) {
                return R.string.emergency_only_prompt;
            }

            //slot-1
            int type0 = TelephonyManager.NETWORK_TYPE_UNKNOWN;
            int[] subIds0 = SubscriptionManager.getSubId(0);
            if (subIds0 != null && subIds0.length > 0) {
                //[0] contains sub id
                type0 = TelephonyManager.getDefault().getVoiceNetworkType(subIds0[0]);
            }
            //slot-2
            int type1 = TelephonyManager.NETWORK_TYPE_UNKNOWN;
            int[] subIds1 = SubscriptionManager.getSubId(1);
            if (subIds1 != null && subIds1.length > 0) {
                //[0] contains sub id
                type1 = TelephonyManager.getDefault().getVoiceNetworkType(subIds1[0]);
            }

            int simOn3G = 3;
            if (MyLog.DEBUG)
                MyLog.d(TAG, "checkMOInitStatus, network type0 = " + type0);
                MyLog.d(TAG, "checkMOInitStatus, network type1 = " + type1);
            if ((type0 < TelephonyManager.NETWORK_TYPE_UMTS)
                    || (type0 >= TelephonyManager.NETWORK_TYPE_GSM)) {
                if ((type1 < TelephonyManager.NETWORK_TYPE_UMTS)
                        || (type1 >= TelephonyManager.NETWORK_TYPE_GSM)){
                    return R.string.not_3G_network;
                } else {
                    simOn3G = 1;
                    MyLog.d(TAG, "checkMOInitStatus, network register on SUB" + simOn3G);
                }
            } else {
                simOn3G = 0;
                MyLog.d(TAG, "checkMOInitStatus, network register on SUB" + simOn3G);
            }

            // check call state
            int callState0 = TelephonyManager.getDefault().getCallState(0);
            int callState1 = TelephonyManager.getDefault().getCallState(1);
            if (MyLog.DEBUG)
                MyLog.d(TAG, "checkMOInitStatus, Call State [sub0] = " + callState0);
            if (MyLog.DEBUG)
                MyLog.d(TAG, "checkMOInitStatus, Call State [sub1] = " + callState1);
            if ((callState0 != TelephonyManager.CALL_STATE_IDLE)
                    || (callState1 != TelephonyManager.CALL_STATE_IDLE)) {
                return R.string.call_state_busy_prompt;
            }
        } else {
            // check whether sim card is ready
            int simState = TelephonyManager.getDefault().getSimState();
            if (MyLog.DEBUG)
                MyLog.d(TAG, "checkMOInitStatus, simState = " + simState);
            if (simState != TelephonyManager.SIM_STATE_READY) {
                return R.string.emergency_only_prompt;
            }

            int type = TelephonyManager.getDefault().getVoiceNetworkType();
            if (MyLog.DEBUG)
                MyLog.d(TAG, "checkMOInitStatus, net work type = " + type);
            if ((type < TelephonyManager.NETWORK_TYPE_UMTS)
                    || (type >= TelephonyManager.NETWORK_TYPE_GSM)) {
                return R.string.not_3G_network;
            }

            // check call state
            int callState = TelephonyManager.getDefault().getCallState();
            if (MyLog.DEBUG)
                MyLog.d(TAG, "checkMOInitStatus, Call State = " + callState);
            if (callState != TelephonyManager.CALL_STATE_IDLE) {
                return R.string.call_state_busy_prompt;
            }
        }

        return 0;
    }

    static boolean isInvalidNumber(String number) {
        if (MyLog.DEBUG)
            MyLog.d(TAG, "isInvalidNumber, number = " + number);

        boolean ret = false;
        String phoneNumber = number;

        phoneNumber = PhoneNumberUtils.convertKeypadLettersToDigits(phoneNumber);
        phoneNumber = PhoneNumberUtils.stripSeparators(phoneNumber);
        phoneNumber = PhoneNumberUtils.formatNumber(phoneNumber);

        if (phoneNumber == null || phoneNumber.equals("")) {
            ret = true;
        }

        if (MyLog.DEBUG)
            MyLog.d(TAG, "isInvalidNumber ret = " + ret);
        return ret;
    }

    // below function is used to force application exit, which is
    // useful while the application is incorrectly restarted by system after a
    // crash.
    static void exitApplication() {
        /*
         * Application app = VideoCallApp.getInstance(); ActivityManager
         * activityManager = (ActivityManager)app
         * .getSystemService(Context.ACTIVITY_SERVICE);
         * activityManager.restartPackage(app.getPackageName());
         */
        try {
            Process.killProcess(Process.myPid());
        } catch (Exception e) {
            if (MyLog.DEBUG)
                MyLog.d(TAG, "kill process notificaton exception." + e.getMessage());
        }
        return;
    }

    /* **********************************************
     * checkHeadsetStatus return value : true: headset pluged false: no headset
     * ********************************************
     */
    synchronized static final boolean checkHeadsetStatus() {
        AudioManager mgr = VideoCallApp.getInstance().mAudioManager;
        if (mgr == null) return false;
        boolean result  = mgr.isWiredHeadsetOn();
        Log.e(TAG, "checkHeadsetStatus return + " + result);
        return result;
    }

    static boolean isHeadSetIn(int state) {
        if ((state & BIT_HEADSET) > 0 || (state & BIT_HEADSET_NO_MIC) > 0) {
            return true;
        } else {
            return false;
        }
    }

    // Backlight range is from 0 - 255. Need to make sure that user
    // doesn't set the backlight to 0 and get stuck
    static final int MIN_BACKLIGHT = android.os.PowerManager.BRIGHTNESS_OFF + 30;
    static final int MAX_BACKLIGHT = android.os.PowerManager.BRIGHTNESS_ON;

    static boolean setScreenBrightness(int brightness) {
        boolean ret = true;
        try {
            if (MyLog.DEBUG)
                MyLog.d(TAG, "set screen brightness: " + brightness + "; Max, Min:" + MIN_BACKLIGHT
                        + "," + MAX_BACKLIGHT);
            WindowManager.LayoutParams lp = VideoCallApp.getInstance().mVideoCallScreen.getWindow()
                    .getAttributes();
            if (0 <= brightness && brightness <= 255) {
                lp.screenBrightness = brightness / 255.00f;
                Log.e(TAG, "lp.screenBrightness " + lp.screenBrightness);
            }
            VideoCallApp.getInstance().mVideoCallScreen.getWindow().setAttributes(lp);
        } catch (Exception doe) {
            if (MyLog.DEBUG)
                MyLog.d(TAG, "exception while set screen Brightness!:" + doe.getMessage());
            ret = false;
        }
        return ret;
    }

    public static boolean getRingtoneMode(Context context) {
        return 1 == Settings.System.getInt(context.getContentResolver(), "custom_ring_mode", 0) ? true
                : false;
    }

    public static Uri getRingtoneUri(Context context) {
        String fileUri = Settings.System.getString(context.getContentResolver(), "custom_ring_uri");
        if ("".equals(fileUri)) {
            return null;
        } else {
            return Uri.parse(convertFileToContent(context, fileUri));
        }
    }

    private static String convertFileToContent(Context context, String value) {
        if (TextUtils.isEmpty(value)) {
            return "";
        }
        ParcelFileDescriptor p = null;
        try {
            p = context.getContentResolver().openFileDescriptor(Uri.parse(value), "r");
        } catch (FileNotFoundException e) {
            if (MyLog.DEBUG)
                MyLog.d(TAG, "In convertContentToFile, file not found: " + value);
        }
        if (p == null) {
            return "";
        }
        String prefix = "file://";
        String path = null;
        if (value.startsWith(prefix)) {
            path = value.substring(prefix.length());
        }
        if (path == null) {
            return "";
        }

        Uri ringUri = getRingUri(context, path);

        if (ringUri != null) {
            return ringUri.toString();
        } else {
            return "";
        }
    }

    private static Uri getRingUri(Context context, String path) {
        ContentResolver resolver = context.getContentResolver();
        String[] cols = new String[] { MediaStore.Audio.Media._ID, MediaStore.Audio.Media.DATA };
        String newPath = path.replace("'", "''");
        String where = MediaStore.Audio.Media.DATA + "=" + "'" + newPath + "'";
        Uri contentUri = MediaStore.Audio.Media.EXTERNAL_CONTENT_URI;
        Cursor c = resolver.query(contentUri, cols, where, null, null);
        Uri ringUri = null;
        if (null == c) {
            contentUri = MediaStore.Audio.Media.INTERNAL_CONTENT_URI;
            c = resolver.query(contentUri, cols, where, null, null);
        }

        if (null != c) {
            try {
                if (c.moveToFirst()) {
                    long _id = c.getLong(c.getColumnIndexOrThrow(MediaStore.Audio.Media._ID));
                    ringUri = Uri.withAppendedPath(contentUri, String.valueOf(_id));
                }
            } catch (Exception ex) {
                ex.printStackTrace();
            } finally {
                c.close();
            }
        }

        return ringUri;
    }

    static private long lastToasterDisappearTime = 0;
    static private final int time_intervals = 2000;

    synchronized static void notifyCameraKeyDisable(Context context, CharSequence notifyMsg) {
        long now = SystemClock.elapsedRealtime();
        if (lastToasterDisappearTime <= now) {
            lastToasterDisappearTime = now + time_intervals;
            Toast.makeText(context, notifyMsg, time_intervals).show();
        }
    }

    static boolean isDmLocked() {
        String dirName = "/local/lawmoLock";
        File file = new File(dirName);
        if (file.exists() && file.isDirectory()) {
            return true;
        }
        return false;
    }
}// end of VTCallUtils
