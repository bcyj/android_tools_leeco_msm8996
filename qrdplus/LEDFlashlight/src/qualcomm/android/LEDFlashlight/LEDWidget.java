/*
    Copyright (c) 2011-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
    Qualcomm Technologies Proprietary and Confidential.
*/

package qualcomm.android.LEDFlashlight;

import android.app.PendingIntent;
import android.appwidget.AppWidgetManager;
import android.appwidget.AppWidgetProvider;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.hardware.Camera;
import android.hardware.Camera.Parameters;
import android.os.PowerManager;
import android.util.Log;
import android.widget.RemoteViews;
import qualcomm.android.LEDFlashlight.R;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;

public class LEDWidget extends AppWidgetProvider {
    public static final byte[] LIGHTE_ON = { '1', '2', '7' };
    public static final byte[] LIGHTE_OFF = { '0' };
    public static final byte[] LIGHT_TORCH = {'1'};
    public static final byte[] LIGHT_DEFAULT = {'0'};

    public final static String TAG = "LED Flashlight";
    private static boolean mLightsOn = false;

    // LED light is on or off before pass PowerKey.
    private static boolean mLightsOnBeforePower = false;

    private static Parameters mCameraParas = null;

    private static PowerManager.WakeLock mWakeLock;

    private static boolean mIsLockAcquired = false;

    public static final String broadCastString = "org.codeaurora.snapcam.action.CLOSE_FLASHLIGHT";
    private final String broadCastPowerKey = "qualcomm.android.LEDFlashlight.LEDUpdate";
    private final String broadCastKilled = "com.android.LEDFlashlight.processKilled";
    private final String brodaCastShoudown = "android.intent.action.ACTION_SHUTDOWN";
    // LED node used in different chipsets
    private final static String COMMON_FLASHLIGHT_BRIGHTNESS =
            "/sys/class/leds/flashlight/brightness";
    private static boolean DBG = false;

    /**
     * used when remove a wiget
     * */
    // @Override
    public void onDeleted(Context context, int[] appWidgetIds) {
        if (DBG)
            Log.v(TAG, "onDeleted");

        super.onDeleted(context, appWidgetIds);
    }

    /**
     * used when the last widget is removed
     * */
    @Override
    public void onDisabled(Context context) {
        if (DBG)
            Log.v(TAG, "onDisabled");
        // When all the widget is deleted, we should assure that the flashlight
        // is turn off.
        // So when all the widget is deleted,if the flashlight state is turn on,
        // turn off it.
        if(mLightsOn){
            mLightsOn = false;
            setLEDStatus(mLightsOn);
        }
        super.onDisabled(context);
    }

    /**
     * used when the widget is created first
     * */
    @Override
    public void onEnabled(Context context) {
        if (DBG)
            Log.v(TAG, "onEnabled");
        super.onEnabled(context);

        if (mWakeLock == null) {
            PowerManager pm = (PowerManager) context.getSystemService(Context.POWER_SERVICE);
            mWakeLock = pm.newWakeLock(PowerManager.PARTIAL_WAKE_LOCK , "ledflashlight");
        }
    }


    /**
     * accept broadcast
     * */
    @Override
    public void onReceive(Context context, Intent intent) {
        String action = intent.getAction();
        if (broadCastString.equals(action)) {
            // receive a broadcast when opening the camera
            boolean fromCamera = intent.getBooleanExtra("camera_led", false);

            ComponentName mThisWidget = new ComponentName(context,LEDWidget.class);
            RemoteViews mRemoteViews = new RemoteViews(context.getPackageName(),
                    R.layout.appwidgetlayout);
            //to avoid invalid pendingIntent after flightmode off->on
            intent.removeExtra("camera_led");
            PendingIntent pendingIntent = PendingIntent.getBroadcast(context, 0,
                intent, 0);
            mRemoteViews.setOnClickPendingIntent(R.id.LEDSwitch, pendingIntent);

            if (DBG)
                Log.d(TAG, "fromCamera = " + fromCamera);

            if (fromCamera) {
                if (mLightsOn) {
                    mLightsOn = false;
                } else {
                    // from the camera, and there is no need to update the
                    // widget
                    // so just return
                    return;
                }
            } else {
                if (DBG)
                    Log.d(TAG, "onSingleTapUp");
                mLightsOn = !mLightsOn;

            }
            mLightsOnBeforePower = mLightsOn;
            updateLEDStatus(context, mRemoteViews, mThisWidget);

            return;
        } else if ("android.intent.action.FAST_BOOT_START".equals(action)) {
            Log.d(TAG, "receive the fast boot action");
            // we receive the fast boot action, and we need turn off the led.
            shutLEDFlashlight(context);
        } else if (action.equals(broadCastPowerKey)) {
            // receive the broadcast when pass PowerKey, and turn off the led.
            boolean fromPowerKey = intent.getBooleanExtra("power_led", false);

            ComponentName mThisWidget = new ComponentName(context, LEDWidget.class);
            RemoteViews mRemoteViews = new RemoteViews(context.getPackageName(),
                    R.layout.appwidgetlayout);
            intent.removeExtra("power_led");

            if (mLightsOnBeforePower) {
                mLightsOn = fromPowerKey;
            } else {
                return;
            }

            updateLEDStatus(context, mRemoteViews, mThisWidget);

        } else if (brodaCastShoudown.equals(action)){
            if (mLightsOn){
                shutLEDFlashlight(context);
            }
        } else if (broadCastKilled.equals(action)){
            String packageName = intent.getStringExtra(Intent.EXTRA_PACKAGES);
            String currentPackageName = context.getPackageName();
            if (packageName != null && packageName.equals(currentPackageName)){
                //when process killed by force close in settings, turn off the led.
                shutLEDFlashlight(context);
            }
        }
        super.onReceive(context, intent);
    }

    private void shutLEDFlashlight(Context context){
        ComponentName thisWidget = new ComponentName(context,LEDWidget.class);
        RemoteViews remoteViews = new RemoteViews(context.getPackageName(),
                R.layout.appwidgetlayout);

        mLightsOn = false;
        mLightsOnBeforePower = false;
        updateLEDStatus(context, remoteViews, thisWidget);
    }

    private void updateLEDStatus(Context context, RemoteViews remoteView, ComponentName name){


         setLEDStatus(mLightsOn);

         int srcId = mLightsOn?R.drawable.flashlight_on:R.drawable.flashlight_off;

         remoteView.setImageViewResource(R.id.LEDSwitch, srcId);
         AppWidgetManager.getInstance(context).updateAppWidget(name, remoteView);
    }

    /**
     * used when user action or the widget is placed on the home screen
     * */
    @Override
    public void onUpdate(Context context, AppWidgetManager appWidgetManager,
            int[] appWidgetIds) {
        if (DBG)
            Log.v(TAG, "onUpdate");
        ComponentName mThisWidget = new ComponentName(context,LEDWidget.class);
        RemoteViews mRemoteViews = new RemoteViews(context.getPackageName(),
                R.layout.appwidgetlayout);
        // create an intent
        Intent intent = new Intent();
        intent.setAction(broadCastString);

        PendingIntent pendingIntent = PendingIntent.getBroadcast(context, 0,
                intent, 0);
        mRemoteViews.setOnClickPendingIntent(R.id.LEDSwitch, pendingIntent);

        //also update the duplicated LED widget status(when add second LED widget on screen and the previous LED widget is on)
        updateLEDStatus(context , mRemoteViews, mThisWidget);

    }

    private void setLEDStatus(boolean status) {
        FileOutputStream red;
        FileOutputStream mode;

        if (DBG) {
            Log.d(TAG, "current LED status " + status);
        }

        if (mWakeLock != null && status && !mIsLockAcquired) {
            mWakeLock.acquire();

            mIsLockAcquired = status;

        } else if (mWakeLock != null && !status && mIsLockAcquired) {
            mWakeLock.release();

            mIsLockAcquired = status;
        }

        // for MSM8x26, BSP add MSM8226_TORCH_NODE for control torch brightness
        changeLEDFlashBrightness(status, COMMON_FLASHLIGHT_BRIGHTNESS);
    }

    private void changeLEDFlashBrightness(boolean status, String node) {
        try {
            byte[] ledData = status ? LIGHTE_ON : LIGHTE_OFF;
            FileOutputStream brightness = new FileOutputStream(node);
            brightness.write(ledData);
            brightness.close();
        } catch (FileNotFoundException e) {
            Log.d(TAG, e.toString());
        } catch (IOException e) {
            e.printStackTrace();
        }

    }

    private boolean isFileExists(String filePath) {
        File file = new File(filePath);
        return file.exists();
    }


}
