/*
 * Copyright (c) 2011-2013, QUALCOMM Technologies, Inc.
 * All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 * Developed by QRD Engineering team.
 */
package com.android.profile;

import static com.android.profile.ProfileConst.DEFAULT_BRIGHTNESS;
import static com.android.profile.ProfileConst.DEFAULT_VOLUME;
import static com.android.profile.ProfileConst.EDIT_PROFILE;
import static com.android.profile.ProfileConst.ENABLE_TAB_RINGTONE_SETTING;
import static com.android.profile.ProfileConst.KEY_CLASS;
import static com.android.profile.ProfileConst.KEY_PACKAGE;
import static com.android.profile.ProfileConst.LOG;
import static com.android.profile.ProfileConst.NEW_PROFILE;
import static com.android.profile.ProfileConst.TARGET_CLASS;
import static com.android.profile.ProfileConst.TARGET_PACKAGE;
import static com.android.profile.ProfileDataBaseAdapter.BLUETOOTH;
import static com.android.profile.ProfileDataBaseAdapter.BRIGHTNESS;
import static com.android.profile.ProfileDataBaseAdapter.DATA;
import static com.android.profile.ProfileDataBaseAdapter.GPS_LOCATION;
import static com.android.profile.ProfileDataBaseAdapter.ID;
import static com.android.profile.ProfileDataBaseAdapter.NETWORK_LOCATION;
import static com.android.profile.ProfileDataBaseAdapter.PROFILE_NAME;
import static com.android.profile.ProfileDataBaseAdapter.RINGTONE1;
import static com.android.profile.ProfileDataBaseAdapter.RINGTONE2;
import static com.android.profile.ProfileDataBaseAdapter.RING_VOLUME;
import static com.android.profile.ProfileDataBaseAdapter.SILENT;
import static com.android.profile.ProfileDataBaseAdapter.VIBRATE;
import static com.android.profile.ProfileDataBaseAdapter.WIFI;
import static com.android.profile.ProfileItemView.ID_CURRENT_PROFILE;
import static com.android.profile.ProfileUtils.getStringValueSaved;
import android.os.SystemProperties;
import android.app.AlertDialog;
import android.app.Dialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.SharedPreferences.OnSharedPreferenceChangeListener;
import android.database.Cursor;
import android.media.AudioManager;
import android.media.RingtoneManager;
import android.net.Uri;
import android.os.Bundle;
import android.preference.CheckBoxPreference;
import android.preference.EditTextPreference;
import android.preference.Preference;
import android.preference.PreferenceActivity;
import android.preference.PreferenceManager;
import android.preference.PreferenceScreen;
import android.preference.Preference.OnPreferenceClickListener;
import android.provider.Settings.System;
import android.telephony.TelephonyManager;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.SeekBar;
import android.widget.TextView;
import android.widget.Toast;
import android.widget.SeekBar.OnSeekBarChangeListener;

public class ProfileSetting extends PreferenceActivity implements OnSharedPreferenceChangeListener {

    private final static String TAG = "ProfileSetting";
    private static Context mContext = null;
    private int volumeSeekBarValue = DEFAULT_VOLUME;
    private int brighnessSeekBarValue = DEFAULT_BRIGHTNESS;
    private int ringVolumeInitValue;
    private int brightnessInitValue;
    EditTextPreference nameEditTextPreference;
    CheckBoxPreference silentBoxPreference;
    CheckBoxPreference vibrateBoxPreference;
    PreferenceScreen volumePreferenceScreen;
    CheckBoxPreference dataBoxPreference;
    CheckBoxPreference wifiBoxPreference;
    CheckBoxPreference bluetoothBoxPreference;
    CheckBoxPreference gpsLocationBoxPreference;
    CheckBoxPreference networkLocationBoxPreference;
    ProfileRingtonePreference slot1Ringtone;
    ProfileRingtonePreference slot2Ringtone;
    ProfileRingtonePreference singleSimRingtone;
    PreferenceScreen ringtonePreferenceScreen;
    PreferenceScreen brightnesScreen;
    Button button_save, button_cancel;
    ProfileDataBaseAdapter mDataBaseAdapter = null;
    PreferenceScreen mPreferenceScreen = null;
    private int action = NEW_PROFILE;
    static int id;
    static Profile curProfile = new Profile();
    private int maxRingVolume;
    private static final int DIALOG_ADJUST_VOLUME = 0;
    private static final int DIALOG_ADJUST_BRIGHTNESS = 1;
    AudioManager mAudioManager = null;
    private boolean mCablAvailable;
    private boolean mTempDisableCabl;
    private static final String SETTING_TAG = ":android:profile_setting";
    private boolean mNameChanged;

    /**
     * Type that refers to sounds that are used for the phone ringer
     *
     * @hide
     */
    static final int TYPE_RINGTONE_2 = 8;

    /**
     * Whether the phone vibrates when it is ringing due to an incoming call.
     * This will be used by Phone and Setting apps; it shouldn't affect other
     * apps. The value is boolean (1 or 0).
     * Note: this is not same as "vibrate on ring", which had been available
     * until ICS. It was about AudioManager's setting and thus affected all
     * the applications which relied on the setting, while this is purely about
     * the vibration setting for incoming calls.
     *
     * @hide
     */
    static final String VIBRATE_WHEN_RINGING2 = "vibrate_when_ringing2";

    /**
     * Channel name for subcription one and two i.e. channele name 1,
     * channel name 2
     *
     * @hide
     */
    static final String[] MULTI_SIM_NAME = {
        "perferred_name_sub1", "perferred_name_sub2"
    };

    private void bindView() {

        logd("");
        nameEditTextPreference = (EditTextPreference) findPreference("name");
        // Set the profile name edittext single line.
        nameEditTextPreference.getEditText().setSingleLine();
        silentBoxPreference = (CheckBoxPreference) findPreference("silent");
        vibrateBoxPreference = (CheckBoxPreference) findPreference("vibrate");
        volumePreferenceScreen = (PreferenceScreen) findPreference("ring_volume");
        brightnesScreen = (PreferenceScreen) findPreference("brightness");
        dataBoxPreference = (CheckBoxPreference) findPreference("data");
        wifiBoxPreference = (CheckBoxPreference) findPreference("wifi");
        bluetoothBoxPreference = (CheckBoxPreference) findPreference("bluetooth");
        gpsLocationBoxPreference = (CheckBoxPreference) findPreference("gps_location");
        networkLocationBoxPreference = (CheckBoxPreference) findPreference("network_location");
        brightnesScreen = (PreferenceScreen) findPreference("brightness");
        button_save = (Button) findViewById(R.id.save);
        button_cancel = (Button) findViewById(R.id.cancel);
        mPreferenceScreen = (PreferenceScreen) findPreference("profile");
        singleSimRingtone = (ProfileRingtonePreference) findPreference("single_sim_ringtone");
        slot1Ringtone = (ProfileRingtonePreference) findPreference("slot1_ringtone");
        slot2Ringtone = (ProfileRingtonePreference) findPreference("slot2_ringtone");
        ringtonePreferenceScreen = (PreferenceScreen) findPreference("ring_tone");

        // use TabActivity to replace two dialogs
        if (ENABLE_TAB_RINGTONE_SETTING) {
            getPreferenceScreen().removePreference(slot1Ringtone);
            getPreferenceScreen().removePreference(slot2Ringtone);
            getPreferenceScreen().removePreference(singleSimRingtone);
        } else {
            getPreferenceScreen().removePreference(ringtonePreferenceScreen);
            // if (!TelephonyManager.isMultiSimEnabled()) {//APINotAvailable
            if (!TelephonyManager.getDefault().isMultiSimEnabled()) {
                // single sim mode
                getPreferenceScreen().removePreference(slot1Ringtone);
                getPreferenceScreen().removePreference(slot2Ringtone);
                singleSimRingtone.setRingtoneType(RingtoneManager.TYPE_RINGTONE);
            } else {
                // dual sim mode
                getPreferenceScreen().removePreference(singleSimRingtone);
            }
        }

        // disable some control item
        if (!ProfileFunctionConfig.isAllCtrlEnabled()) {
            if (!ProfileFunctionConfig.isSilentCtrlEnabled()) {
                // remove dependency relationship
                ringtonePreferenceScreen.setDependency(null);
                volumePreferenceScreen.setDependency(null);
                getPreferenceScreen().removePreference(silentBoxPreference);
            }
            if (!ProfileFunctionConfig.isVibrateCtrlEnabled())
                getPreferenceScreen().removePreference(vibrateBoxPreference);
            if (!ProfileFunctionConfig.isRingVolumeCtrlEnabled())
                getPreferenceScreen().removePreference(volumePreferenceScreen);
            if (!ProfileFunctionConfig.isRingtoneCtrlEnabled()) {
                getPreferenceScreen().removePreference(slot1Ringtone);
                getPreferenceScreen().removePreference(slot2Ringtone);
                getPreferenceScreen().removePreference(ringtonePreferenceScreen);
            }
            if (!ProfileFunctionConfig.isDataCtrlEnabled())
                getPreferenceScreen().removePreference(dataBoxPreference);
            if (!ProfileFunctionConfig.isWifiCtrlEnabled())
                getPreferenceScreen().removePreference(wifiBoxPreference);
            if (!ProfileFunctionConfig.isBluetoothCtrlEnabled())
                getPreferenceScreen().removePreference(bluetoothBoxPreference);
            if (!ProfileFunctionConfig.isGpsLocationCtrlEnabled())
                getPreferenceScreen().removePreference(gpsLocationBoxPreference);
            if (!ProfileFunctionConfig.isNetworkLocationCtrlEnabled())
                getPreferenceScreen().removePreference(networkLocationBoxPreference);
            if (!ProfileFunctionConfig.isBrightnessCtrlEnabled())
                getPreferenceScreen().removePreference(brightnesScreen);
        }
        // Display WLAN if feature is use wlan.
        if (SystemProperties.getBoolean("persist.env.profilemgr.showwlan", false)) {
            wifiBoxPreference.setTitle(R.string.wlan_title);
        }
    }

    private Profile getSettings() {

        logd("");
        Profile mProfile = new Profile();
        String profile_name = "new";
        boolean silent = false;
        boolean vibrate = true;
        int ring_volume = 0;
        boolean data = true;
        boolean wifi = false;
        boolean bluetooth = false;
        boolean gpsLocation = false;
        boolean networkLocation = false;
        int brightness = 0;
        String ringtone1 = null;
        String ringtone2 = null;
        profile_name = ProfileUtils.getDBProfileName(mContext, nameEditTextPreference.getText().trim());
        silent = silentBoxPreference.isChecked();
        vibrate = vibrateBoxPreference.isChecked();
        ring_volume = volumeSeekBarValue;
        data = dataBoxPreference.isChecked();
        wifi = wifiBoxPreference.isChecked();
        bluetooth = bluetoothBoxPreference.isChecked();
        gpsLocation = gpsLocationBoxPreference.isChecked();
        networkLocation = networkLocationBoxPreference.isChecked();
        brightness = brighnessSeekBarValue;

        if (ENABLE_TAB_RINGTONE_SETTING) {
            ringtone1 = getStringValueSaved(mContext, "ringtone1", null);
            ringtone2 = getStringValueSaved(mContext, "ringtone2", null);

        } else {
//            ringtone1 = TelephonyManager.isMultiSimEnabled() ? slot1Ringtone.getRingtone()
//                    : singleSimRingtone.getRingtone();//APINotAvailable
            ringtone1 = TelephonyManager.getDefault().isMultiSimEnabled()
                    ? slot1Ringtone.getRingtone() : singleSimRingtone.getRingtone();
            ringtone2 = slot2Ringtone.getRingtone();
        }

        mProfile.setProfileName(profile_name);
        mProfile.setSilent(silent);
        mProfile.setVibrate(vibrate);
        mProfile.setRingVolume(ring_volume);
        mProfile.setData(data);
        mProfile.setWifi(wifi);
        mProfile.setBluetooth(bluetooth);
        mProfile.setGpsLocation(gpsLocation);
        mProfile.setNetworkLocation(networkLocation);
        mProfile.setBrightness(brightness);
        mProfile.setRingtone1(ringtone1);
        mProfile.setRingtone2(ringtone2);

        mProfile.setId(id);

        return mProfile;
    }

    private void setRingtoneState() {

//        if (!TelephonyManager.isMultiSimEnabled())
//            return;//APINotAvailable
        if (!TelephonyManager.getDefault().isMultiSimEnabled())
            return;
        ProfileRingtonePreference[] slotsRingtone = {
                slot1Ringtone, slot2Ringtone
        };
        for (int subScription = 0; subScription < 2; ++subScription) {
//            int slotState = TelephonyManager.getSubscriptionState(subScription);
//            int cardState = TelephonyManager.getDefault().getSimState(subScription);
//            boolean airplane = (System.getInt(getContentResolver(), System.AIRPLANE_MODE_ON, 0) != 0);
//
//            slotsRingtone[subScription].setEnabled((slotState == TelephonyManager.SUB_ACTIVATED)
//                    && (cardState == TelephonyManager.SIM_STATE_READY) && !airplane);
        }
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {

        logd("");
        mContext = this;
        super.onCreate(savedInstanceState);
        // ========open database=====
        if (mDataBaseAdapter == null) {
            mDataBaseAdapter = new ProfileDataBaseAdapter(this);
            mDataBaseAdapter.openDataBase();
        }
        action = getIntent().getExtras().getInt("action");
        mAudioManager = (AudioManager) getSystemService(Context.AUDIO_SERVICE);
        maxRingVolume = mAudioManager.getStreamMaxVolume(AudioManager.STREAM_RING);

        Profile mProfile = null;
        Profile savedProfile = null;
        Bundle restoredBuddle = null;
        if (savedInstanceState != null) {
            restoredBuddle = savedInstanceState.getBundle(SETTING_TAG);
        }

        switch (action) {
        case NEW_PROFILE:
            setTitle(R.string.setting_title);
            mProfile = getDefaultSettings();
            break;
        case EDIT_PROFILE:
            setTitle(R.string.edit_title);
            if (id == ID_CURRENT_PROFILE) {
                mProfile = ProfileMain.getCurrentSettings();
                mProfile.setProfileName(getString(R.string.default_name));
            } else {
                Cursor mCursor = mDataBaseAdapter.fetchProfile(id);
                if (mCursor != null && mCursor.moveToFirst()) {
                    mProfile = getSavedSettings(mCursor); // update the variable
                } else {
                    mProfile = getDefaultSettings();
                }
                if (mCursor != null)
                    mCursor.close();
            }
            break;
        default:
            mProfile = getDefaultSettings();
            break;
        }

        if (restoredBuddle != null) {
            savedProfile = getInstanceStatus(restoredBuddle);
            if (mNameChanged) {
                // Load the customized  name
                mProfile.setProfileName(savedProfile.getProfileName());
            }
        }

        // this step must be placed before addPreferencesFromResource()
        writeSavedPreference(mProfile);

        addPreferencesFromResource(R.xml.setting_preference_screen);

        setContentView(R.layout.setting_main_layout);

        bindView();

        setView(mProfile);
        // =======Register button click========
        button_save.setOnClickListener(new Button.OnClickListener() {

            public void onClick(View v) {

                Profile userSetting = getSettings();

                if(getSettings().getProfileName().length() <= 0){
                    Toast.makeText(getApplicationContext(), getString(R.string.name_empty_alert), Toast.LENGTH_LONG).show();
                    return;
                }

                // Check whether mDataBaseAdapter is null
                if (null == mDataBaseAdapter) {
                    mDataBaseAdapter = new ProfileDataBaseAdapter(ProfileSetting.this);
                    mDataBaseAdapter.openDataBase();
                }

                long currentProfileId = 0l;
                // If we want to new a profile, it should not get here.
                if (id == ID_CURRENT_PROFILE && action != NEW_PROFILE) {
                    currentProfileId = mDataBaseAdapter.insertProfile(userSetting);
                    Cursor mCursor = mDataBaseAdapter.fetchAllProfiles();
                    if (mCursor != null && mCursor.moveToLast()) {
                        setResult(mCursor.getInt(mCursor.getColumnIndex(ID)));
                    }
                    if (mCursor != null)
                        mCursor.close();
                    ProfileSetting.this.finish();
                }
                if (action == NEW_PROFILE) {
                    long profileId = 0l;
                    if (id != ID_CURRENT_PROFILE) {
                        profileId = mDataBaseAdapter.insertProfile(userSetting);
                    } else {
                        profileId = currentProfileId;
                    }
                    // Set new profile database id
                    Intent data = getIntent();
                    data.putExtra(ProfileConst.KEY_PROFILE_ID, profileId);
                    setResult(RESULT_OK, data);
                }
                else if (action == EDIT_PROFILE) {

                    mDataBaseAdapter.updateProfile(userSetting);

                    //set current edit profile id to check whether need applyProfile
                    Intent data = getIntent();
                    data.putExtra(ProfileConst.KEY_PROFILE_ID, id);
                    setResult(RESULT_OK, data);
                }

                ProfileSetting.this.finish();

            }
        });
        button_cancel.setOnClickListener(new Button.OnClickListener() {

            public void onClick(View v) {

                ProfileSetting.this.finish();
            }
        });

        volumePreferenceScreen.setOnPreferenceClickListener(new OnPreferenceClickListener() {

            public boolean onPreferenceClick(Preference preference) {

                showDialog(DIALOG_ADJUST_VOLUME);
                return false;
            }
        });

        brightnesScreen.setOnPreferenceClickListener(new OnPreferenceClickListener() {

            public boolean onPreferenceClick(Preference preference) {

                showDialog(DIALOG_ADJUST_BRIGHTNESS);
                return false;
            }
        });
        // ========register SharedPreferenceChangeListener
        SharedPreferences mSharedPreferences = PreferenceManager.getDefaultSharedPreferences(this);
        mSharedPreferences.registerOnSharedPreferenceChangeListener(this);
    }

    @Override
    protected void onPause() {

        logd("");
        super.onPause();
        if (mDataBaseAdapter != null) {
            mDataBaseAdapter.closeDataBase();
            mDataBaseAdapter = null;
        }
    }

    @Override
    protected void onResume() {

        logd("");
        super.onResume();
        if (mDataBaseAdapter == null) {
            mDataBaseAdapter = new ProfileDataBaseAdapter(this);
            mDataBaseAdapter.openDataBase();
        }
        setRingtoneState();

    }

    public void showToast(Object s) {

        Toast.makeText(this, s + "", Toast.LENGTH_SHORT).show();
    }

    private Profile getDefaultSettings() {

        logd("");
        Profile mProfile = new Profile();
        SharedPreferences mSharedPreferences = getSharedPreferences(
                "com.android.profile_preferences", 0);
        String profile_name = ""; //getString(R.string.default_name);
        boolean silent = false;
        boolean vibrate = false;
        int ring_volume = mSharedPreferences.getInt("def_ring_volume", 5);
        boolean data = true;
        boolean wifi = false;
        boolean bluetooth = false;
        boolean gpsLocation = false;
        boolean networkLocation = false;
        int brightness = mSharedPreferences.getInt("def_brightness", DEFAULT_BRIGHTNESS);
        String ringtone1 = mSharedPreferences.getString("def_ringtone1", null);
        String ringtone2 = mSharedPreferences.getString("def_ringtone2", null);

        volumeSeekBarValue = ring_volume;
        brighnessSeekBarValue = brightness;

        mProfile.setProfileName(profile_name);
        mProfile.setSilent(silent);
        mProfile.setVibrate(vibrate);
        mProfile.setRingVolume(ring_volume);
        mProfile.setData(data);
        mProfile.setWifi(wifi);
        mProfile.setBluetooth(bluetooth);
        mProfile.setGpsLocation(gpsLocation);
        mProfile.setNetworkLocation(networkLocation);
        mProfile.setBrightness(brightness);
        mProfile.setRingtone1(ringtone1);
        mProfile.setRingtone2(ringtone2);

        return mProfile;
    }

    @SuppressWarnings("unused")
    private String getCurrentRingtone(int slot) {

        int ringtoneTypes[] = {
                RingtoneManager.TYPE_RINGTONE, TYPE_RINGTONE_2
        };

        if (slot < 0 || slot > 1)
            return null;
        Uri ring = RingtoneManager.getActualDefaultRingtoneUri(this, ringtoneTypes[slot]);
        if (ring == null)
            return null;
        return ring.toString();
    }

    private Profile getSavedSettings(Cursor mCursor) {

        logd("");
        Profile mProfile = new Profile();
        String profile_name = "new";
        boolean silent = false;
        boolean vibrate = true;
        int ring_volume = 0;
        boolean data = true;
        boolean wifi = false;
        boolean bluetooth = false;
        boolean gpsLocation = false;
        boolean networkLocation = false;
        int brightness = 0;
        String ringtone1 = null;
        String ringtone2 = null;

        profile_name = mCursor.getString(mCursor.getColumnIndex(PROFILE_NAME));
        silent = mCursor.getInt(mCursor.getColumnIndex(SILENT)) > 0 ? true : false;
        vibrate = mCursor.getInt(mCursor.getColumnIndex(VIBRATE)) > 0 ? true : false;
        ring_volume = mCursor.getInt(mCursor.getColumnIndex(RING_VOLUME));
        data = mCursor.getInt(mCursor.getColumnIndex(DATA)) > 0 ? true : false;
        wifi = mCursor.getInt(mCursor.getColumnIndex(WIFI)) > 0 ? true : false;
        bluetooth = mCursor.getInt(mCursor.getColumnIndex(BLUETOOTH)) > 0 ? true : false;
        gpsLocation = mCursor.getInt(mCursor.getColumnIndex(GPS_LOCATION)) > 0 ? true : false;
        networkLocation = mCursor.getInt(mCursor.getColumnIndex(NETWORK_LOCATION)) > 0 ? true
                : false;
        brightness = mCursor.getInt(mCursor.getColumnIndex(BRIGHTNESS));
        ringtone1 = mCursor.getString(mCursor.getColumnIndex(RINGTONE1));
        ringtone2 = mCursor.getString(mCursor.getColumnIndex(RINGTONE2));

        // volumeSeekBarValue = ring_volume;
        // brighnessSeekBarValue = brightness;

        mProfile.setProfileName(profile_name);
        mProfile.setSilent(silent);
        mProfile.setVibrate(vibrate);
        mProfile.setRingVolume(ring_volume);
        mProfile.setData(data);
        mProfile.setWifi(wifi);
        mProfile.setBluetooth(bluetooth);
        mProfile.setGpsLocation(gpsLocation);
        mProfile.setNetworkLocation(networkLocation);
        mProfile.setBrightness(brightness);
        mProfile.setRingtone1(ringtone1);
        mProfile.setRingtone2(ringtone2);

        return mProfile;
    }

    private void setView(Profile mProfile) {

        logd("");
        String profile_name = ProfileUtils.getShowProfileName(mContext, mProfile.getProfileName());
        int ring_volume = mProfile.getRingVolume();
        int brightness = mProfile.getBrightness();
        String ringtone1 = mProfile.getRingtone1();
        String ringtone2 = mProfile.getRingtone2();
        nameEditTextPreference.setSummary(profile_name);
        volumePreferenceScreen.setSummary(String.valueOf(ring_volume) + " / "
                + String.valueOf(maxRingVolume));
        brightnesScreen.setSummary(String.valueOf(brightness) + " / 255");

        if (ENABLE_TAB_RINGTONE_SETTING) {

        } else {

//            if (!TelephonyManager.isMultiSimEnabled()) {//APINotAvailable
            if (!TelephonyManager.getDefault().isMultiSimEnabled()) {
                // single sim mode
                singleSimRingtone.setRingtoneType(RingtoneManager.TYPE_RINGTONE);
            } else {
                // dual sim mode
                slot1Ringtone.setRingtoneType(RingtoneManager.TYPE_RINGTONE);
                slot1Ringtone.setRingtone(ringtone1);
                slot2Ringtone.setRingtoneType(RingtoneManager.TYPE_RINGTONE);
                slot2Ringtone.setRingtone(ringtone2);
            }

        }

        ringVolumeInitValue = volumeSeekBarValue = ring_volume;
        brightnessInitValue = brighnessSeekBarValue = brightness;
    }

    public void onSharedPreferenceChanged(SharedPreferences sharedPreferences, String key) {

        if (key.compareTo(nameEditTextPreference.getKey()) == 0) {
            nameEditTextPreference.setSummary(nameEditTextPreference.getText());
            mNameChanged = true;
        }

    }

    void writeSavedPreference(Profile mProfile) {

        SharedPreferences sp = PreferenceManager.getDefaultSharedPreferences(mContext);
        SharedPreferences.Editor editor = sp.edit();

        String profile_name = ProfileUtils.getShowProfileName(mContext, mProfile.getProfileName());
        boolean silent = mProfile.isSilent();
        boolean vibrate = mProfile.isVibrate();
        int ring_volume = mProfile.getRingVolume();
        boolean data = mProfile.isData();
        boolean wifi = mProfile.isWifi();
        boolean bluetooth = mProfile.isBluetooth();
        boolean gpsLocation = mProfile.isGpsLocation();
        boolean networkLocation = mProfile.isNetworkLocation();
        int brightness = mProfile.getBrightness();
        String ringtone1 = mProfile.getRingtone1();
        String ringtone2 = mProfile.getRingtone2();

        editor.putString("name", profile_name);
        editor.putBoolean("silent", silent);
        editor.putBoolean("vibrate", vibrate);
        editor.putInt("ring_volume", ring_volume);
        editor.putBoolean("data", data);
        editor.putBoolean("wifi", wifi);
        editor.putBoolean("bluetooth", bluetooth);
        editor.putBoolean("gps_location", gpsLocation);
        editor.putBoolean("network_location", networkLocation);
        editor.putInt("brightness", brightness);
        editor.putString("ringtone1", ringtone1);
        editor.putString("ringtone2", ringtone2);

        editor.commit();
    }

    // Save all of UI states
    private Bundle saveInstanceStatus(Profile profile) {
        Bundle bundle = new Bundle();
        String profile_name = ProfileUtils.getShowProfileName(mContext, profile.getProfileName());

        bundle.putString("name", profile_name);
        bundle.putBoolean("name_changed", mNameChanged);
        return bundle;
    }

    private Profile getInstanceStatus(Bundle bundle) {
        Profile profile = new Profile();

        profile.setProfileName(bundle.getString("name", ""));
        mNameChanged = bundle.getBoolean("name_changed", false);
        return profile;
    }

    private OnSeekBarChangeListener brightnessChangeListener = new OnSeekBarChangeListener() {

        public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {

            if (fromUser) {
                setBrightness(seekBar.getProgress());
            }

        }

        public void onStartTrackingTouch(SeekBar seekBar) {

        }

        public void onStopTrackingTouch(SeekBar seekBar) {

        }

    };

    private void setBrightness(int value) {
        WindowManager.LayoutParams layoutParams = getWindow().getAttributes();
        if (value <= 0)
            value = 1;
        else if (value > 255)
            value = 255;
        layoutParams.screenBrightness = (float) value / 255;
        getWindow().setAttributes(layoutParams);
    }

    public boolean onPreferenceTreeClick(PreferenceScreen preferenceScreen, Preference preference) {

        if (ringtonePreferenceScreen != null && preference == ringtonePreferenceScreen) {
            Intent intent = ringtonePreferenceScreen.getIntent();
            intent.putExtra(KEY_PACKAGE, TARGET_PACKAGE);
            intent.putExtra(KEY_CLASS, TARGET_CLASS);
            startActivity(intent);
        }
        return true;
    };

    @Override
    protected void onPrepareDialog(int id, Dialog dialog) {
        SeekBar ringVolumeSeekBar = (SeekBar)dialog.findViewById(R.id.ring_volume);
        if(ringVolumeSeekBar!=null){
            ringVolumeSeekBar.setProgress(volumeSeekBarValue);
        }else{
            SeekBar brightnessSeekBar = (SeekBar)dialog.findViewById(R.id.brightness);
            if(brightnessSeekBar!=null){
                brightnessSeekBar.setProgress(brighnessSeekBarValue);
            }
        }
    }


    @Override
    protected Dialog onCreateDialog(int dialog) {

        logd("");
        final boolean REMOVE = true;
        switch (dialog) {
        case DIALOG_ADJUST_VOLUME:
            LayoutInflater volumeInflater = LayoutInflater.from(this);
            final View volumeAdjustView = volumeInflater.inflate(R.layout.dialog_adjust_volume,
                    null);
            // Ringtone Volume
            final SeekBar ringVolumeSeekBar = (SeekBar) volumeAdjustView
                    .findViewById(R.id.ring_volume);
            ringVolumeSeekBar.setMax(maxRingVolume);
            ringVolumeSeekBar.setProgress(ringVolumeInitValue);
            final ProfileVolumePlayer mRingVolumePlayer = new ProfileVolumePlayer(mContext,
                    AudioManager.STREAM_RING);
            mRingVolumePlayer.preparePlayer();
            ringVolumeSeekBar.setOnSeekBarChangeListener(new OnSeekBarChangeListener() {

                public void onStopTrackingTouch(SeekBar seekBar) {

                    mRingVolumePlayer.setStreamVolume(seekBar.getProgress());
                    mRingVolumePlayer.play();
                }
                public void onStartTrackingTouch(SeekBar arg0) {

                }
                public void onProgressChanged(SeekBar arg0, int arg1, boolean arg2) {

                }
            });

            // Music Volume
            final SeekBar musicVolumeSeekBar = (SeekBar) volumeAdjustView
                    .findViewById(R.id.music_volume);
            musicVolumeSeekBar.setMax(mAudioManager.getStreamMaxVolume(AudioManager.STREAM_MUSIC));
            musicVolumeSeekBar.setProgress(ringVolumeInitValue);
            final TextView musicTextView = (TextView) volumeAdjustView
                    .findViewById(R.id.music_title);

            final SeekBar alarmVolumeSeekBar = (SeekBar) volumeAdjustView
                    .findViewById(R.id.alarm_volume);
            alarmVolumeSeekBar.setMax(mAudioManager.getStreamMaxVolume(AudioManager.STREAM_ALARM));
            alarmVolumeSeekBar.setProgress(ringVolumeInitValue);
            final TextView alarmTextView = (TextView) volumeAdjustView
                    .findViewById(R.id.alarm_title);

            if (REMOVE) {
                ringVolumeSeekBar.setPadding(23, 0, 23, 18);
                musicVolumeSeekBar.setVisibility(View.GONE);
                musicTextView.setVisibility(View.GONE);
                alarmVolumeSeekBar.setVisibility(View.GONE);
                alarmTextView.setVisibility(View.GONE);
            }

            return new AlertDialog.Builder(ProfileSetting.this).setTitle(
                    R.string.dialog_sound_volume).setView(volumeAdjustView).setPositiveButton(
                    R.string.ok, new DialogInterface.OnClickListener() {

                        public void onClick(DialogInterface dialog, int whichButton) {

                            volumeSeekBarValue = ringVolumeSeekBar.getProgress();
                            volumePreferenceScreen.setSummary(String.valueOf(volumeSeekBarValue)
                                    + " / " + String.valueOf(maxRingVolume));
                            mRingVolumePlayer.releasePlayer();
                        }
                    }).setNegativeButton(R.string.cancel, new DialogInterface.OnClickListener() {

                public void onClick(DialogInterface dialog, int whichButton) {

                    mRingVolumePlayer.releasePlayer();
                }
            }).create();

        case DIALOG_ADJUST_BRIGHTNESS:
            LayoutInflater brightenessInflater = LayoutInflater.from(this);
            final View brightnessAdjustView = brightenessInflater.inflate(
                    R.layout.dialog_adjust_brightness, null);
            final SeekBar brightnessSeekBar = (SeekBar) brightnessAdjustView
                    .findViewById(R.id.brightness);
            brightnessSeekBar.setOnSeekBarChangeListener(brightnessChangeListener);
            brightnessSeekBar.setProgress(brightnessInitValue);

            /**
             * stop CABL case it is running
             */
            mCablAvailable = SystemProperties.getBoolean("ro.qualcomm.cabl", false);
            mTempDisableCabl = false;
            if(mCablAvailable && SystemProperties.get("init.svc.abld").equals("running")) {
                SystemProperties.set("ctl.stop", "abld");
                mTempDisableCabl = true;
            }
            return new AlertDialog.Builder(ProfileSetting.this).setTitle(R.string.dialog_display)
                    .setView(brightnessAdjustView).setPositiveButton(R.string.ok,
                            new DialogInterface.OnClickListener() {

                                public void onClick(DialogInterface dialog, int whichButton) {

                                    brighnessSeekBarValue = brightnessSeekBar.getProgress();
                                    brightnesScreen.setSummary(String
                                            .valueOf(brighnessSeekBarValue)
                                            + " / 255");
                                    /**
                                     * start CABL when exit from the profilemgr
                                     */
                                    if(mCablAvailable && mTempDisableCabl) {
                                        SystemProperties.set("ctl.start", "abld");
                                        mTempDisableCabl = false;
                                    }
                                }
                            }).setNegativeButton(R.string.cancel,
                            new DialogInterface.OnClickListener() {

                                public void onClick(DialogInterface dialog, int whichButton) {
                                    // Set brightness to old value.
                                    setBrightness(brighnessSeekBarValue);
                                }
                            }).create();
        }
        return null;
    }

    @SuppressWarnings("unused")
    private static void loge(Object e) {

        if (!LOG)
            return;
        Thread mThread = Thread.currentThread();
        StackTraceElement[] mStackTrace = mThread.getStackTrace();
        String mMethodName = mStackTrace[3].getMethodName();
        e = "[" + mMethodName + "] " + e;
        Log.e(TAG, e + "");
    }

    private static void logd(Object s) {

        if (!LOG)
            return;
        Thread mThread = Thread.currentThread();
        StackTraceElement[] mStackTrace = mThread.getStackTrace();
        String mMethodName = mStackTrace[3].getMethodName();

        s = "[" + mMethodName + "] " + s;
        Log.d(TAG, s + "");
    }

    @Override
    protected void onSaveInstanceState(Bundle outState) {
        super.onSaveInstanceState(outState);
        Bundle container;
        Profile profile = getSettings();
        // Save all of UI states
        container = saveInstanceStatus(profile);
        outState.putBundle(SETTING_TAG, container);
    }
}
