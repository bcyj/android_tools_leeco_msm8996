/**
 * Copyright (c) 2015, Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.qti.engineertool.operation;

import android.content.ContentResolver;
import android.content.Context;
import android.database.Cursor;
import android.database.MatrixCursor;
import android.media.AudioManager;
import android.nfc.NfcAdapter;
import android.os.PowerManager;
import android.os.SystemProperties;
import android.os.UserManager;
import android.provider.Settings;
import android.text.TextUtils;
import android.util.Log;

import com.android.internal.view.RotationPolicy;

import java.lang.reflect.Method;
import java.util.ArrayList;

public class OperationUtils {
    private static final String TAG = "OperationUtils";

    private static final String COL_RESULT = "result";
    private static final String[] RES_PROJECTION = new String[] { COL_RESULT };

    public static Cursor getResult(Context context, String functionName, String[] params) {
        try {
            Method m = OperationUtils.class.getMethod(functionName, Context.class, String[].class);
            Cursor res = (Cursor) m.invoke(null, context, params);
            return res;
        } catch (Exception e) {
            Log.w(TAG, "Do not support this function now. Please add it.");
            return null;
        }
    }

    public static Cursor propR(Context context, String[] params) {
        if (params == null) return null;

        String propKey = params[0];
        if (TextUtils.isEmpty(propKey)) {
            Log.e(TAG, "Can not read the prop as the prop key is null.");
            return null;
        }
        String defValue = null;
        if (params.length > 1) {
            defValue = params[1];
        }
        String res = SystemProperties.get(propKey, defValue);
        return buildResCursor(res);
    }

    public static Cursor propW(Context context, String[] params) {
        if (params == null) return null;

        String propKey = params[0];
        if (TextUtils.isEmpty(propKey)) {
            Log.e(TAG, "Can not write the prop as the prop key is null.");
            return null;
        }
        String newValue = params[1];
        SystemProperties.set(propKey, newValue);
        return buildResCursor(1);
    }

    public static Cursor rotateGet(Context context, String[] params) {
        boolean locked = RotationPolicy.isRotationLocked(context);
        return buildResCursor(locked ? 0 : 1);
    }

    public static Cursor rotateSet(Context context, String[] params) {
        if (params == null) return null;

        boolean locked = "0".equals(params[0]) ? true : false;
        RotationPolicy.setRotationLock(context, locked);
        return buildResCursor(1);
    }

    public static Cursor nfcGet(Context context, String[] params) {
        NfcAdapter adapter = NfcAdapter.getDefaultAdapter(context);
        boolean enabled = adapter.isEnabled();
        return buildResCursor(enabled ? 1 : 0);
    }

    public static Cursor nfcSet(Context context, String[] params) {
        if (params == null) return null;

        NfcAdapter adapter = NfcAdapter.getDefaultAdapter(context);
        if ("0".equals(params[0])) {
            adapter.disable();
        } else {
            adapter.enable();
        }
        return buildResCursor(1);
    }

    public static Cursor gpsGet(Context context, String[] params) {
        try {
            int mode = Settings.Secure.getInt(context.getContentResolver(),
                    Settings.Secure.LOCATION_MODE, Settings.Secure.LOCATION_MODE_OFF);
            boolean enabled = (mode != Settings.Secure.LOCATION_MODE_OFF) && !isRestricted(context);
            return buildResCursor(enabled ? 1 : 0);
        } catch (Exception e) {
            return null;
        }
    }

    public static Cursor gpsSet(Context context, String[] params) {
        if (params == null) return null;

        try {
            if ("0".equals(params[0])) {
                Settings.Secure.putInt(context.getContentResolver(), Settings.Secure.LOCATION_MODE,
                        Settings.Secure.LOCATION_MODE_OFF);
            } else {
                Settings.Secure.putInt(context.getContentResolver(), Settings.Secure.LOCATION_MODE,
                        Settings.Secure.LOCATION_MODE_HIGH_ACCURACY);
            }
            return buildResCursor(1);
        } catch (Exception e) {
            return null;
        }
    }

    public static Cursor brightnessGet(Context context, String[] params) {
        try {
            int mode = Settings.System.getInt(context.getContentResolver(),
                    Settings.System.SCREEN_BRIGHTNESS_MODE);
            if (mode == Settings.System.SCREEN_BRIGHTNESS_MODE_AUTOMATIC) {
                return buildResCursor("Auto");
            } else {
                int brightness = Settings.System.getInt(context.getContentResolver(),
                        Settings.System.SCREEN_BRIGHTNESS);
                return buildResCursor(brightness);
            }
        } catch (Exception e) {
            return null;
        }
    }

    public static Cursor brightnessSet(Context context, String[] params) {
        if (params == null) return null;

        try {
            int brightness = Integer.parseInt(params[0]);
            ContentResolver cr = context.getContentResolver();

            // Set manual mode.
            Settings.System.putInt(cr, Settings.System.SCREEN_BRIGHTNESS_MODE,
                    Settings.System.SCREEN_BRIGHTNESS_MODE_MANUAL);

            PowerManager pm = (PowerManager) context.getSystemService(Context.POWER_SERVICE);
            int minBrightness = pm.getMinimumScreenBrightnessSetting();
            int maxBrightness = pm.getMaximumScreenBrightnessSetting();
            if (brightness < minBrightness) {
                brightness = minBrightness;
            } else if (brightness > maxBrightness) {
                brightness = maxBrightness;
            }

            // Set the brightness.
            pm.setBacklightBrightness(brightness);
            Settings.System.putInt(cr, Settings.System.SCREEN_BRIGHTNESS, brightness);
            return buildResCursor(1);
        } catch (Exception e) {
            return null;
        }
    }

    public static Cursor maxBrightnessGet(Context context, String[] params) {
        PowerManager pm = (PowerManager) context.getSystemService(Context.POWER_SERVICE);
        int maxBrightness = pm.getMaximumScreenBrightnessSetting();
        return buildResCursor(maxBrightness);
    }

    public static Cursor maxBrightnessSet(Context context, String[] params) {
        if (params == null) return null;

        String newValue = params[0];
        SystemProperties.set("persist.power.max.brightness", params[0]);

        // Check if current brightness more than max, if yes, set the brightness to max value.
        try {
            int brightness = Settings.System.getInt(context.getContentResolver(),
                    Settings.System.SCREEN_BRIGHTNESS);
            int newMaxBrightness = Integer.parseInt(newValue);
            if (newMaxBrightness < brightness) {
                return brightnessSet(context, params);
            }
            return buildResCursor(1);
        } catch (Exception e) {
            return null;
        }
    }

    public static Cursor timezoneGet(Context context, String[] params) {
        try {
            boolean auto = Settings.Global.getInt(context.getContentResolver(),
                    Settings.Global.AUTO_TIME_ZONE) > 0;
            return buildResCursor(auto ? 1 : 0);
        } catch (Exception e) {
            return null;
        }
    }

    public static Cursor timezoneSet(Context context, String[] params) {
        if (params == null) return null;

        try {
            Settings.Global.putInt(context.getContentResolver(), Settings.Global.AUTO_TIME_ZONE,
                    Integer.parseInt(params[0]));
            return buildResCursor(1);
        } catch (Exception e) {
            return null;
        }
    }

    public static Cursor musicVolumeGet(Context context, String[] params) {
        AudioManager am = (AudioManager) context.getSystemService(Context.AUDIO_SERVICE);
        int volume = am.getStreamVolume(AudioManager.STREAM_MUSIC);
        return buildResCursor(volume);
    }

    public static Cursor musicVolumeSet(Context context, String[] params) {
        if (params == null) return null;

        try {
            AudioManager am = (AudioManager) context.getSystemService(Context.AUDIO_SERVICE);
            int maxVolume = am.getStreamMaxVolume(AudioManager.STREAM_MUSIC);
            int newVolume = Integer.parseInt(params[0]);
            if (newVolume < 0) {
                newVolume = 0;
            } else if (newVolume > maxVolume) {
                newVolume = maxVolume;
            }
            am.setStreamVolume(AudioManager.STREAM_MUSIC, newVolume, 0);
            SystemProperties.set("persist.power.music.volume", String.valueOf(newVolume));
            return buildResCursor(1);
        } catch (Exception e) {
            return null;
        }
    }

    public static Cursor snapshotVolumeGet(Context context, String[] params) {
        AudioManager am = (AudioManager) context.getSystemService(Context.AUDIO_SERVICE);
        int volume = am.getMasterVolume();
        return buildResCursor(volume);
    }

    public static Cursor snapshotVolumeSet(Context context, String[] params) {
        if (params == null) return null;

        try {
            AudioManager am = (AudioManager) context.getSystemService(Context.AUDIO_SERVICE);
            int maxVolume = am.getMasterMaxVolume();
            int newVolume = Integer.parseInt(params[0]);
            if (newVolume < 0) {
                newVolume = 0;
            } else if (newVolume > maxVolume) {
                newVolume = maxVolume;
            }
            am.setMasterVolume(newVolume, 0);
            SystemProperties.set("persist.camera.snapshot.volume", String.valueOf(newVolume));
            return buildResCursor(1);
        } catch (Exception e) {
            return null;
        }
    }

    // Used by function: usb draw get or set.
    private static final String PROP_KEY_USB_DRAW = "persist.sys.usb.vbus.draw";
    private static final String PROP_VALUE_HIGHER_USB_DRAW = "600";

    public static Cursor usbDrawGet(Context context, String[] params) {
        String draw = SystemProperties.get(PROP_KEY_USB_DRAW, "");
        int res = PROP_VALUE_HIGHER_USB_DRAW.equals(draw) ? 1 : 0;
        return buildResCursor(res);
    }

    public static Cursor usbDrawSet(Context context, String[] params) {
        if (params == null) return null;

        String newValue = "";
        try {
            if (Integer.parseInt(params[0]) == 1) {
                newValue = PROP_VALUE_HIGHER_USB_DRAW;
            }
        } catch (NumberFormatException e) {
            // Do nothing.
        }
        SystemProperties.set(PROP_KEY_USB_DRAW, newValue);
        return buildResCursor(1);
    }

    public static Cursor captiveDetectionGet(Context context, String[] params) {
        try {
            int enabled = Settings.Global.getInt(context.getContentResolver(),
                    Settings.Global.CAPTIVE_PORTAL_DETECTION_ENABLED, 1);
            return buildResCursor(enabled);
        } catch (Exception e) {
            return null;
        }
    }

    public static Cursor captiveDetectionSet(Context context, String[] params) {
        if (params == null) return null;

        try {
            Settings.Global.putInt(context.getContentResolver(),
                    Settings.Global.CAPTIVE_PORTAL_DETECTION_ENABLED, Integer.parseInt(params[0]));
            return buildResCursor(1);
        } catch (Exception e) {
            return null;
        }
    }

    private static Cursor buildResCursor(Object res) {
        MatrixCursor cursor = new MatrixCursor(OperationUtils.RES_PROJECTION);
        ArrayList<Object> row = new ArrayList<Object>(OperationUtils.RES_PROJECTION.length);
        row.add(res);
        cursor.addRow(row);
        return cursor;
    }

    private static boolean isRestricted(Context context) {
        final UserManager um = (UserManager) context.getSystemService(Context.USER_SERVICE);
        return um.hasUserRestriction(UserManager.DISALLOW_SHARE_LOCATION);
    }
}
