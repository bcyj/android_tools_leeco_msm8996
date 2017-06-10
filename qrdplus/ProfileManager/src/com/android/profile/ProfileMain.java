/*
 * Copyright (c) 2011-2013, QUALCOMM Technologies, Inc.
 * All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 * Developed by QRD Engineering team.
 */
package com.android.profile;

import static com.android.profile.ProfileConst.DEFAULT_BRIGHTNESS;
import static com.android.profile.ProfileConst.DEFAULT_VOLUME_RATIO;
import static com.android.profile.ProfileConst.DISABLE_CONNECTIVITY;
import static com.android.profile.ProfileConst.EDIT_PROFILE;
import static com.android.profile.ProfileConst.ENABLE_CONNECTIVITY;
import static com.android.profile.ProfileConst.LOG;
import static com.android.profile.ProfileConst.NEW_PROFILE;
import static com.android.profile.ProfileConst.PROFILE_GENERAL;
import static com.android.profile.ProfileConst.PROFILE_MEETING;
import static com.android.profile.ProfileConst.PROFILE_OUTING;
import static com.android.profile.ProfileConst.PROFILE_SILENT;
import static com.android.profile.ProfileConst.PROFILE_POWERSAVE;
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
import static com.android.profile.ProfileItemView.STATE_CURRENT_PROFILE;
import static com.android.profile.ProfileItemView.STATE_SAVED_PROFILE;

import java.util.ArrayList;
import java.util.List;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.bluetooth.BluetoothAdapter;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.SharedPreferences;
import android.database.ContentObserver;
import android.database.Cursor;
import android.location.LocationManager;
import android.media.AudioManager;
import android.media.RingtoneManager;
import android.net.ConnectivityManager;
import android.net.Uri;
import android.net.wifi.WifiManager;
import android.os.Bundle;
import android.os.Handler;
import android.provider.Settings;
import android.provider.Settings.SettingNotFoundException;
import android.provider.Settings.System;
import android.telephony.TelephonyManager;
import android.text.format.Time;
import android.util.Log;
import android.view.ContextMenu;
import android.view.ContextMenu.ContextMenuInfo;
import android.view.KeyEvent;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuItem;
import android.view.SubMenu;
import android.view.View;
import android.view.ViewGroup;
import android.view.WindowManager;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.BaseAdapter;
import android.widget.ListView;
import android.widget.RadioButton;
import android.widget.TextView;
import android.widget.Toast;

public class ProfileMain extends Activity {

    static String TAG = "ProfileMain";

    private Profile mProfileCur = new Profile();
    List<Profile> mProfileList = new ArrayList<Profile>();
    private int savedProfileNum;
    private final int CODE_REQUEST_SETTING = 1;
    private final int CODE_REQUEST_NEW_PROFILE = 2;

    private int maxRingVolume = 7;
    static Time timeAndDate = new Time();
    private ListView mListView = null;

    final int MENU_ADD = Menu.FIRST;

    final int MENU_RESET = Menu.FIRST + 1;
    ProfileDataBaseAdapter mDataBaseAdapter = null;
    private static final int DIALOG_EDIT = 1;
    private static final int DIALOG_RESET = 2;
    private static final int DIALOG_DELETE = 3;
    private static final int SUB_1 = 0;
    private static final int SUB_2 = 1;

    static final int PROFILE_EDIT = 0;
    static final int PROFILE_DELETE = 1;
    static final int PROFILE_SAVE = 2;
    private final int MAX_VOLUME = 0;
    private final int DEFAULT_VOLUME = 1;
    private final int ZERO_VOLUME = 2;
    private static AudioManager mAudioManager = null;
    private static ConnectivityManager mConnectivityManager = null;
    private static boolean isFirstStart = true;

    private static WifiManager mWifiManager = null;
    private static BluetoothAdapter mBluetoothAdapter = null;

    private static Context mContext = null;
    private LayoutInflater mInflater = null;
    private static ProfileFunctionConfig mProfileFunctionConfig = null;

    private final int DEFAULT_PROFILE_COUNT = 5;

    private boolean mModifyInThisApp = false;
    private ContentObserver mBrightnessObserver =
            new ContentObserver(new Handler()) {
        @Override
        public void onChange(boolean selfChange) {
            // For distinguish if the bar changes the brightness
            mModifyInThisApp = false;
            int brightnessCur = -1;
            try {
                brightnessCur = Settings.System.getInt(mContext.getContentResolver(),
                        "screen_brightness");
            } catch (SettingNotFoundException e) {
                e.printStackTrace();
            }

            if (!nullJudge(mDataBaseAdapter, "mDataBaseAdapter")) {
                Cursor mCursor = mDataBaseAdapter.fetchAllProfiles();
                int brightnessIndex = mCursor.getColumnIndex(BRIGHTNESS);
                int i;
                for (mCursor.moveToFirst(), i = 0; !(mCursor.isAfterLast()); mCursor.moveToNext(), i++) {
                    if (brightnessCur == mCursor.getInt(brightnessIndex)) {
                        mModifyInThisApp = true;
                        break;
                    }
                }
                mCursor.close();
            }

            if (!mModifyInThisApp) {
                setSelectedId(-1);
                notifyChanged();
            }
        }
    };

    @Override
    public void onCreateContextMenu(ContextMenu menu, View v, ContextMenuInfo menuInfo) {

        AdapterView.AdapterContextMenuInfo mMenuInfo = (AdapterView.AdapterContextMenuInfo) menuInfo;
        ProfileItemView mView = (ProfileItemView) mMenuInfo.targetView;
        int position = mMenuInfo.position;
        menu.setHeaderTitle(getString(R.string.dialog_edit_title));
        switch (mView.getState()) {
        case STATE_SAVED_PROFILE:
            menu.add(0, PROFILE_EDIT, 0, getString(R.string.edit));
            if (position >= DEFAULT_PROFILE_COUNT)
                menu.add(0, PROFILE_DELETE, 0, getString(R.string.delete));
            break;
        case STATE_CURRENT_PROFILE:
            menu.add(0, PROFILE_EDIT, 0, getString(R.string.edit));
            // menu.add(0, PROFILE_SAVE, 0, getString(R.string.save));
            break;
        default:
            break;
        }
        super.onCreateContextMenu(menu, v, menuInfo);
    }

    @Override
    public boolean onContextItemSelected(MenuItem item) {

        AdapterView.AdapterContextMenuInfo mMenuInfo = (AdapterView.AdapterContextMenuInfo) item
                .getMenuInfo();
        ProfileItemView mView = (ProfileItemView) mMenuInfo.targetView;

        switch (item.getItemId()) {
        case PROFILE_EDIT:
            ProfileSetting.id = mView.getId();
            Intent intent = new Intent(ProfileMain.this, ProfileSetting.class);
            intent.putExtra("action", EDIT_PROFILE);
            startActivityForResult(intent, CODE_REQUEST_SETTING);
            break;
        case PROFILE_DELETE:
            ProfileSetting.id = mView.getId();
            showDialog(DIALOG_DELETE);
            break;
        case PROFILE_SAVE:
            Profile mProfile = getCurrentSettings();
            mProfile.setProfileName(getString(R.string.default_name));
            mDataBaseAdapter.insertProfile(mProfile);
            getAllProfiles();
            notifyChanged();
            break;
        default:
            break;
        }
        return super.onContextItemSelected(item);
    }

    private void init(Context context) {

        mContext = context;
        mProfileFunctionConfig = new ProfileFunctionConfig(context);
        mProfileFunctionConfig.getConfigFromXml(R.xml.profile_function_config);
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {

        logd("");
        init(this);
        super.onCreate(savedInstanceState);
        // ====open db====
        if (mDataBaseAdapter == null) {
            mDataBaseAdapter = new ProfileDataBaseAdapter(this);
            mDataBaseAdapter.openDataBase();
        }
        nullJudge(mDataBaseAdapter, "mDataBaseAdapter");

        getService();

        maxRingVolume = mAudioManager.getStreamMaxVolume(AudioManager.STREAM_RING);

        // ====show the default profiles ===
        if (getSelectedId() < 0) {
            resetProfileDataBase(true);
        }
        setContentView(R.layout.main_layout);
        mListView = (ListView) findViewById(R.id.list);
        mListView.setAdapter(mBaseAdapter);
        registerForContextMenu(mListView);
        mListView.setOnItemClickListener(new OnItemClickListener() {

            public void onItemClick(AdapterView<?> arg0, View view, int position, long id) {// position

                // 0-based

                int profileId = mProfileList.get(position).getId();

                applyProfile(profileId);

                ProfileMain.this.finish();
            }
        });

    }// onCreate

    private void getService() {

        mAudioManager = (AudioManager) getSystemService(Context.AUDIO_SERVICE);
        nullJudge(mAudioManager, "mAudioManager");
        mConnectivityManager = (ConnectivityManager) getSystemService(Context.CONNECTIVITY_SERVICE);
        nullJudge(mConnectivityManager, "mConnectivityManager");
        mWifiManager = (WifiManager) getSystemService(Context.WIFI_SERVICE);
        nullJudge(mWifiManager, "mWifiManager");
        mBluetoothAdapter = BluetoothAdapter.getDefaultAdapter();
        nullJudge(mBluetoothAdapter, "mBluetoothAdapter");
        mInflater = LayoutInflater.from(mContext);
    }

    private void soundSetting(boolean mSilent, boolean mVibrate, int mVolume) {

        int vibInSilent = 1;

        // Variable to sync the vibrate of system settings with profile manager,
        // 1 enable vibrate, 0 disable.
        Log.d(TAG, "mSilent:" + mSilent + "|mVibrate:" + mVibrate + "|mVolume:"
                + mVolume);
        int vibInRinging = 1;
        if (mSilent) {
            // muteAllSound(true);
            setOtherVolume(ZERO_VOLUME);
            if (mVibrate) { // only vibrate
                mAudioManager.setRingerMode(AudioManager.RINGER_MODE_VIBRATE);
                mAudioManager.setVibrateSetting(AudioManager.VIBRATE_TYPE_RINGER,
                        AudioManager.VIBRATE_SETTING_ON);
                mAudioManager.setVibrateSetting(AudioManager.VIBRATE_TYPE_NOTIFICATION,
                        AudioManager.VIBRATE_SETTING_ON);
                vibInSilent = 1;
                vibInRinging = 1;
            } else
            // neither vibrate nor ringer
            {
                mAudioManager.setRingerMode(AudioManager.RINGER_MODE_SILENT);
                mAudioManager.setVibrateSetting(AudioManager.VIBRATE_TYPE_RINGER,
                        AudioManager.VIBRATE_SETTING_OFF);
                mAudioManager.setVibrateSetting(AudioManager.VIBRATE_TYPE_NOTIFICATION,
                        AudioManager.VIBRATE_SETTING_OFF);
                vibInSilent = 0;
                vibInRinging = 0;
            }

        } else {
            // muteAllSound(false);
            //when it's not slient all sounds, needn't set the music and alarm sound volume by profile
            //setOtherVolume(DEFAULT_VOLUME);

            // both vibrate and ringer
            mAudioManager.setRingerMode(AudioManager.RINGER_MODE_NORMAL);
            if (mVibrate) {
                mAudioManager.setVibrateSetting(AudioManager.VIBRATE_TYPE_RINGER,
                        AudioManager.VIBRATE_SETTING_ON);
                mAudioManager.setVibrateSetting(AudioManager.VIBRATE_TYPE_NOTIFICATION,
                        AudioManager.VIBRATE_SETTING_ON);
                vibInSilent = 1;
                vibInRinging = 1;
            } else {
                mAudioManager.setVibrateSetting(AudioManager.VIBRATE_TYPE_RINGER,
                        AudioManager.VIBRATE_SETTING_OFF);
                mAudioManager.setVibrateSetting(AudioManager.VIBRATE_TYPE_NOTIFICATION,
                        AudioManager.VIBRATE_SETTING_OFF);
                vibInSilent = 0;
                vibInRinging = 0;
            }
            // ring_volume & notification_volume setting
            if (ProfileFunctionConfig.isRingVolumeCtrlEnabled()) {
                mAudioManager.setStreamVolume(AudioManager.STREAM_RING, mVolume,
                        AudioManager.FLAG_REMOVE_SOUND_AND_VIBRATE);
                mAudioManager.setStreamVolume(AudioManager.STREAM_NOTIFICATION, mVolume,
                        AudioManager.FLAG_REMOVE_SOUND_AND_VIBRATE);
            }
        }
        Settings.System
                .putInt(getContentResolver(), Settings.System.VIBRATE_IN_SILENT, vibInSilent);
        Settings.System
                .putInt(getContentResolver(), Settings.System.VIBRATE_WHEN_RINGING, vibInRinging);
        Settings.System
                .putInt(getContentResolver(), ProfileSetting.VIBRATE_WHEN_RINGING2, vibInRinging);
    }

    private void setOtherVolume(int mode) {

        int music_volume = 0, alarm_volume = 0;
        switch (mode) {
        case ZERO_VOLUME:
            break;
        case DEFAULT_VOLUME:
            music_volume = Math.round(DEFAULT_VOLUME_RATIO
                    * mAudioManager.getStreamMaxVolume(AudioManager.STREAM_MUSIC));
            alarm_volume = Math.round(DEFAULT_VOLUME_RATIO
                    * mAudioManager.getStreamMaxVolume(AudioManager.STREAM_ALARM));
            setMusicAlarmVolume(music_volume, alarm_volume);
            break;
        case MAX_VOLUME:
            music_volume = mAudioManager.getStreamMaxVolume(AudioManager.STREAM_MUSIC);
            alarm_volume = mAudioManager.getStreamMaxVolume(AudioManager.STREAM_ALARM);
            setMusicAlarmVolume(music_volume, alarm_volume);
            break;
        default:
            break;
        }
    }

    private void setMusicAlarmVolume(int musicVolume, int alarmVolume) {
        mAudioManager.setStreamVolume(AudioManager.STREAM_MUSIC, musicVolume,
                AudioManager.FLAG_REMOVE_SOUND_AND_VIBRATE);
        mAudioManager.setStreamVolume(AudioManager.STREAM_ALARM, alarmVolume,
                AudioManager.FLAG_REMOVE_SOUND_AND_VIBRATE);
    }

    // Should be unmuted by the same object, so it is not recommended.
    // private void muteAllSound(boolean mute) {
    // mAudioManager.setStreamMute(AudioManager.STREAM_MUSIC, mute);
    // mAudioManager.setStreamMute(AudioManager.STREAM_ALARM, mute);
    // mAudioManager.setStreamMute(AudioManager.STREAM_RING, mute);
    // mAudioManager.setStreamMute(AudioManager.STREAM_NOTIFICATION, mute);
    // mAudioManager.setStreamMute(AudioManager.STREAM_SYSTEM, mute);
    // }

    private void dataSetting(boolean data) {

        if (!ProfileFunctionConfig.isDataCtrlEnabled())
            return;
        if (data == DISABLE_CONNECTIVITY)
            setDataEnabled(DISABLE_CONNECTIVITY);
        else
            setDataEnabled(ENABLE_CONNECTIVITY);

    }

    public void setDataEnabled(boolean para) {

        TelephonyManager telephonyManager = TelephonyManager.from(mContext);
        if (telephonyManager != null)
            telephonyManager.setDataEnabled(para);
        else
            Log.e(TAG, "phone=null");
    }

    private void wifiSetting(boolean wifi) {

        boolean airplane = (System.getInt(getContentResolver(), System.AIRPLANE_MODE_ON, 0) != 0);
        if (airplane) {
            Log.i(TAG, "airplane on");
            return;
        }
        if (!ProfileFunctionConfig.isWifiCtrlEnabled())
            return;
        if (mWifiManager != null)
            mWifiManager.setWifiEnabled(wifi);
        else
            Log.e(TAG, "mWifiManager=null");
    }

    private void bluetoothSetting(boolean bluetooth) {

        if (!ProfileFunctionConfig.isBluetoothCtrlEnabled())
            return;
        if (mBluetoothAdapter != null) {
            if (bluetooth)
                mBluetoothAdapter.enable();
            else
                mBluetoothAdapter.disable();
        }
    }

    private void gpsLocationSetting(boolean gpsLocation) {

        if (!ProfileFunctionConfig.isGpsLocationCtrlEnabled())
            return;

        try {
            Settings.Secure.setLocationProviderEnabled(getContentResolver(),
                    LocationManager.GPS_PROVIDER, gpsLocation);
        } catch (Exception e) {
        }
    }

    private void networkLocationSetting(boolean networkLocation) {

        if (!ProfileFunctionConfig.isNetworkLocationCtrlEnabled())
            return;

        try {
            Settings.Secure.setLocationProviderEnabled(getContentResolver(),
                    LocationManager.NETWORK_PROVIDER, networkLocation);
        } catch (Exception e) {

        }
    }

    private void ringtoneSetting(String ringtone1, String ringtong2) {
        if (!ProfileFunctionConfig.isRingtoneCtrlEnabled())
            return;
        Uri uri1 = (ringtone1 != null) ? Uri.parse(ringtone1) : null;
        Uri uri2 = (ringtong2 != null) ? Uri.parse(ringtong2) : null;
        RingtoneManager.setActualRingtoneUriBySubId(this, SUB_1, uri1);
        RingtoneManager.setActualRingtoneUriBySubId(this, SUB_2, uri2);
    }

    private void brightnessSetting(int brightness) {

        Settings.System.putInt(getContentResolver(), Settings.System.SCREEN_BRIGHTNESS_MODE,
                Settings.System.SCREEN_BRIGHTNESS_MODE_MANUAL);
        if (!android.provider.Settings.System.putInt(getContentResolver(),
                android.provider.Settings.System.SCREEN_BRIGHTNESS, brightness))
            Log.e(TAG, "SCREEN_BRIGHTNESS writing error");
        Uri localUri = Settings.System.getUriFor("screen_brightness");
        getContentResolver().notifyChange(localUri, null);
        int value;
        value = brightness;
        if (brightness <= 0)
            value = 1;
        else if (brightness > 255)
            value = 255;
        value = brightness <= 0 ? 1 : brightness;
        WindowManager.LayoutParams mParam = getWindow().getAttributes();
        mParam.screenBrightness = value / 255.0f; // 0.0 - 1.0
        getWindow().setAttributes(mParam);
    }

    public void getAllProfiles() {

        logd("");
        if (nullJudge(mDataBaseAdapter, "mDataBaseAdapter"))
            return;
        Cursor mCursor = mDataBaseAdapter.fetchAllProfiles();
        int idIndex = mCursor.getColumnIndex(ID);
        int nameIndex = mCursor.getColumnIndex(PROFILE_NAME);
        int silentIndex = mCursor.getColumnIndex(SILENT);
        int vibrateIndex = mCursor.getColumnIndex(VIBRATE);
        int volumeIndex = mCursor.getColumnIndex(RING_VOLUME);
        int dataIndex = mCursor.getColumnIndex(DATA);
        int wifiIndex = mCursor.getColumnIndex(WIFI);
        int bluetoothIndex = mCursor.getColumnIndex(BLUETOOTH);
        int gpsLocationIndex = mCursor.getColumnIndex(GPS_LOCATION);
        int networkLocationIndex = mCursor.getColumnIndex(NETWORK_LOCATION);
        int brightnessIndex = mCursor.getColumnIndex(BRIGHTNESS);
        int ringtone1Index = mCursor.getColumnIndex(RINGTONE1);
        int ringtone2Index = mCursor.getColumnIndex(RINGTONE2);

        savedProfileNum = mCursor.getCount();
        logd("savedProfileNum=" + savedProfileNum);
        int i;
        mProfileList.clear();
        for (mCursor.moveToFirst(), i = 0; !(mCursor.isAfterLast()); mCursor.moveToNext(), i++) {
            mProfileList.add(new Profile());
            mProfileList.get(i).setId(mCursor.getInt(idIndex));
            mProfileList.get(i).setProfileName(mCursor.getString(nameIndex));
            mProfileList.get(i).setSilent(mCursor.getInt(silentIndex) > 0 ? true : false);
            mProfileList.get(i).setVibrate(mCursor.getInt(vibrateIndex) > 0 ? true : false);
            mProfileList.get(i).setRingVolume(mCursor.getInt(volumeIndex));
            mProfileList.get(i).setData(mCursor.getInt(dataIndex) > 0 ? true : false);
            mProfileList.get(i).setWifi(mCursor.getInt(wifiIndex) > 0 ? true : false);
            mProfileList.get(i).setBluetooth(mCursor.getInt(bluetoothIndex) > 0 ? true : false);
            mProfileList.get(i).setGpsLocation(mCursor.getInt(gpsLocationIndex) > 0 ? true : false);
            mProfileList.get(i).setNetworkLocation(
                    mCursor.getInt(networkLocationIndex) > 0 ? true : false);
            mProfileList.get(i).setBrightness(mCursor.getInt(brightnessIndex));
            mProfileList.get(i).setRingtone1(mCursor.getString(ringtone1Index));
            mProfileList.get(i).setRingtone2(mCursor.getString(ringtone2Index));
            mProfileList.get(i).setMatchCurProfile(false);// reset to false
        }
        mCursor.close();

        if (!mModifyInThisApp) {
            boolean isMatch = false;
            for (i = 0; i < savedProfileNum; i++) {
                isMatch = matchCurSetting(mProfileList.get(i));
                mProfileList.get(i).setMatchCurProfile(isMatch);
                if (isMatch)
                    break;
            }
            if (!isMatch) {
                setSelectedId(-1);
            }
        }
    }

    /**
     * Apply the current profile
     *
     * @param profileId the profile id in the table of profile database
     */
    private void applyProfile(int profileId) {
        if (profileId == -1) {
            return;
        }

        if (mDataBaseAdapter == null) {
            mDataBaseAdapter = new ProfileDataBaseAdapter(this);
            mDataBaseAdapter.openDataBase();
        }

        Cursor cursor = mDataBaseAdapter.fetchProfile(profileId);
        if (cursor == null) {
            return;
        }

        // get profile
        Profile profile = getSavedSettings(cursor);
        cursor.close();

        // Set profile
        soundSetting(profile.isSilent(), profile.isVibrate(), profile.getRingVolume());
        dataSetting(profile.isData());
        wifiSetting(profile.isWifi());
        bluetoothSetting(profile.isBluetooth());
        gpsLocationSetting(profile.isGpsLocation());
        networkLocationSetting(profile.isNetworkLocation());
        brightnessSetting(profile.getBrightness());
        ringtoneSetting(profile.getRingtone1(), profile.getRingtone2());

        displayToast(ProfileUtils.getShowProfileName(mContext, profile.getProfileName())
                + " " + getString(R.string.profile_enable));

        setSelectedId(profileId);

        mModifyInThisApp = true;
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {

        SubMenu addMenu = menu.addSubMenu(0, MENU_ADD, 0, R.string.menu_add);
        addMenu.setIcon(android.R.drawable.ic_menu_add);

        SubMenu resetMenu = menu.addSubMenu(0, MENU_RESET, 0, R.string.menu_reset);
        resetMenu.setIcon(android.R.drawable.ic_menu_revert);
        return true;

    }

    void resetProfileDataBase(boolean newFlashed) {

        int def_ring_volume = mContext.getResources().getInteger(R.integer.def_volume);
        int def_brightness = DEFAULT_BRIGHTNESS;
        String def_ringtone1 = null;
        String def_ringtone2 = null;

        final SharedPreferences mSharedPreferences = getSharedPreferences(
                "com.android.profile_preferences", 0);
        mDataBaseAdapter.deleteAllProfiles();
        if (newFlashed) {
            Profile mProfile = getCurrentSettings();
            def_brightness = mProfile.getBrightness();
            def_ringtone1 = getCurrentRingtone(0);
            def_ringtone2 = getCurrentRingtone(1);

            SharedPreferences.Editor editor = mSharedPreferences.edit();
            editor.putInt(ProfileUtils.KEY_DEFAUT_RINGVOLUME, def_ring_volume);
            editor.putInt(ProfileUtils.KEY_DEFAUT_BRIGHTNESS, def_brightness);
            editor.putString(ProfileUtils.KEY_DEFAUT_RINGTONE1, def_ringtone1);
            editor.putString(ProfileUtils.KEY_DEFAUT_RINGTONE2, def_ringtone2);
            editor.commit();
        } else {
            def_ring_volume = mSharedPreferences.getInt(ProfileUtils.KEY_DEFAUT_RINGVOLUME,
                    def_ring_volume);
            def_brightness = mSharedPreferences.getInt(ProfileUtils.KEY_DEFAUT_BRIGHTNESS,
                    def_brightness);
            def_ringtone1 = mSharedPreferences.getString(ProfileUtils.KEY_DEFAUT_RINGTONE1,
                    def_ringtone1);
            def_ringtone2 = mSharedPreferences.getString(ProfileUtils.KEY_DEFAUT_RINGTONE2,
                    def_ringtone2);
        }
        // Profile(String profile_name, boolean silent, boolean vibrate, String
        // ringtone1,
        // String ringtone2, int ring_volume, boolean data, boolean wifi,
        // boolean bluetooth,
        // boolean gpsLocation, boolean networkLocation, int brightness)
        Profile generalProfile = new Profile(PROFILE_GENERAL, false, false,
                def_ringtone1, def_ringtone2, def_ring_volume, true, false, false, false, false,
                def_brightness);
        Profile meetingProfile = new Profile(PROFILE_MEETING, true, true,
                def_ringtone1, def_ringtone2, 0, true, false, false, false, false, def_brightness);
        Profile outingProfile = new Profile(PROFILE_OUTING, false, true, def_ringtone1,
                def_ringtone2, maxRingVolume, true, false, false, false, false, (int) (0.7 * 255));
        Profile silentProfile = new Profile(PROFILE_SILENT, true, false, def_ringtone1,
                def_ringtone2, 0, true, false, false, false, false, def_brightness);
        Profile powersaveProfile = new Profile(PROFILE_POWERSAVE, false, false, def_ringtone1,
                def_ringtone2, def_ring_volume, false, false, false, false, false, 1);

        mDataBaseAdapter.insertProfile(generalProfile);
        mDataBaseAdapter.insertProfile(meetingProfile);
        mDataBaseAdapter.insertProfile(outingProfile);
        mDataBaseAdapter.insertProfile(silentProfile);
        mDataBaseAdapter.insertProfile(powersaveProfile);
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {

        switch (item.getItemId()) {
        case MENU_ADD:
            Intent intent = new Intent(ProfileMain.this, ProfileSetting.class);
            intent.putExtra("action", NEW_PROFILE);
            startActivityForResult(intent, CODE_REQUEST_NEW_PROFILE);
            break;
        case MENU_RESET:
            showDialog(DIALOG_RESET);
            break;

        default:
            break;
        }
        return super.onOptionsItemSelected(item);
    }

    @Override
    protected void onResume() {

        logd("");
        // IntentFilter filter = new IntentFilter();
        // filter.addAction(AudioManager.VOLUME_CHANGED_ACTION);
        // registerReceiver(mReceiver, filter);

        if (mDataBaseAdapter == null) {
            mDataBaseAdapter = new ProfileDataBaseAdapter(this);
            mDataBaseAdapter.openDataBase();
        }
        mProfileCur = getCurrentSettings();
        getAllProfiles();
        notifyChanged();
        super.onResume();

        mContext.getContentResolver().registerContentObserver(
                Settings.System.getUriFor(Settings.System.SCREEN_BRIGHTNESS),
                true, mBrightnessObserver);
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {

        logd(resultCode);
        if (requestCode == CODE_REQUEST_SETTING) {
            if (resultCode != RESULT_CANCELED) {
                if (data != null){
                    int profileId = data.getIntExtra(ProfileConst.KEY_PROFILE_ID, -1);
                    if (profileId == getSelectedId()){
                        applyProfile(profileId);
                    }
                }
            }
        }

        // Back form new profile
        if (requestCode == CODE_REQUEST_NEW_PROFILE) {
            if (resultCode != RESULT_CANCELED) {
                // Get the profile database id
                if (data != null) {
                    long profileId = data.getLongExtra(ProfileConst.KEY_PROFILE_ID, -1);

                    // Active the new profile
                    applyProfile((int) profileId);
                }
            }
        }
        super.onActivityResult(requestCode, resultCode, data);
    }

    @Override
    protected void onPause() {

        logd("");
        if (mDataBaseAdapter != null) {
            mDataBaseAdapter.closeDataBase();
            mDataBaseAdapter = null;
        }

        mModifyInThisApp = false;
        mContext.getContentResolver().unregisterContentObserver(mBrightnessObserver);
        super.onPause();
    }

    @Override
    protected void onStop() {

        logd("");
        super.onStop();
    }

    public void notifyChanged() {

        logd("========");
        mBaseAdapter.notifyDataSetChanged();
    }

    BaseAdapter mBaseAdapter = new BaseAdapter() {

        public Object getItem(int arg0) {

            return null;
        }

        public long getItemId(int arg0) {

            return 0;
        }

        public View getView(int index, View convertView, ViewGroup parent) {

            // convertView is the last data

            if (convertView == null)
                convertView = mInflater.inflate(R.layout.main_list_row, null);

            ProfileItemView mProfileItemView = (ProfileItemView) convertView;
            Profile mProfile = mProfileList.get(index);
            logd((index + 1) + ":" + ProfileUtils.getShowProfileName(mContext, mProfile.getProfileName()));

            // ImageView imageView = (ImageView)
            // convertView.findViewById(R.id.profile_image);
            TextView nameView = (TextView) convertView.findViewById(R.id.profile_name);
            RadioButton checkButton = (RadioButton) convertView.findViewById(R.id.choice);
            // imageView.setImageResource(R.drawable.menu_add);

            nameView.setText("" + ProfileUtils.getShowProfileName(mContext, mProfile.getProfileName()));
            mProfileItemView.setId(mProfile.getId());
            mProfileItemView.setState(STATE_SAVED_PROFILE);

            if (matchCurSetting(mProfile) || mProfile.getId() == getSelectedId()) {
                checkButton.setChecked(true);
                logd("Match:" + ProfileUtils.getShowProfileName(mContext, mProfile.getProfileName()));
                if (index == savedProfileNum) {
                    mProfileItemView.setState(STATE_CURRENT_PROFILE);
                }
            } else {
                checkButton.setChecked(false);
            }
            return convertView;
        }

        public int getCount() {

            if (mProfileList != null)
                return mProfileList.size();
            else
                return 0;

        };
    };

    public void displayToast(Object s) {

        Toast.makeText(this, s + "", Toast.LENGTH_SHORT).show();
    }

    @Override
    protected Dialog onCreateDialog(int dialog) {

        switch (dialog) {
        case DIALOG_EDIT:
            return new AlertDialog.Builder(ProfileMain.this).setTitle(R.string.dialog_edit_title)
                    .setItems(R.array.dialog_edit_item, new DialogInterface.OnClickListener() {

                        public void onClick(DialogInterface dialog, int which) {

                            switch (which) {
                            case PROFILE_EDIT:
                                Intent intent = new Intent(ProfileMain.this, ProfileSetting.class);
                                intent.putExtra("action", EDIT_PROFILE);
                                startActivity(intent);
                                break;
                            case PROFILE_DELETE:
                                if (mDataBaseAdapter.deleteProfile(ProfileSetting.id) == false)
                                    loge("Delete Failure");
                                getAllProfiles();
                                notifyChanged();
                                break;
                            default:
                                break;
                            }
                        }
                    }).create();
        case DIALOG_RESET:
            return new AlertDialog.Builder(ProfileMain.this).setTitle(R.string.dialog_reset_title)
                    .setMessage(R.string.dialog_reset_message).setPositiveButton(R.string.ok,
                            new DialogInterface.OnClickListener() {

                                public void onClick(DialogInterface dialog, int whichButton) {

                                    resetProfileDataBase(false);
                                    //apply GENERAL profile
                                    applyProfile(mProfileList.get(0).getId());
                                    mProfileCur = getCurrentSettings();
                                    getAllProfiles();
                                    notifyChanged();
                                }
                            }).setNegativeButton(R.string.cancel,
                            new DialogInterface.OnClickListener() {

                                public void onClick(DialogInterface dialog, int whichButton) {

                                }
                            }).create();
        case DIALOG_DELETE:
            return new AlertDialog.Builder(ProfileMain.this).setMessage(R.string.deleteConfirmation)
                .setPositiveButton(R.string.ok,
                        new DialogInterface.OnClickListener() {
                            public void onClick(DialogInterface dialog, int whichButton) {
                                if (mDataBaseAdapter.deleteProfile(ProfileSetting.id) == false) {
                                    loge("Delete Failure");
                                }
                                else if (ProfileSetting.id == getSelectedId()){
                                    //delete success, apply GENERAL profile
                                    applyProfile(mProfileList.get(0).getId());
                                }
                                getAllProfiles();
                                notifyChanged();
                            }
                        }).setNegativeButton(R.string.cancel,
                        new DialogInterface.OnClickListener() {
                            public void onClick(DialogInterface dialog, int whichButton) {
                            }
                        }).create();
        }
        return null;
    }

    private static String getCurrentRingtone(int slot) {

        int ringtoneTypes[] = {
                RingtoneManager.TYPE_RINGTONE, ProfileSetting.TYPE_RINGTONE_2
        };

        if (slot < 0 || slot > 1)
            return null;
        Uri ring = RingtoneManager.getActualDefaultRingtoneUri(mContext, ringtoneTypes[slot]);
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
        String ringtone1 = null;
        String ringtone2 = null;
        int ring_volume = 0;
        boolean data = true;
        boolean wifi = false;
        boolean bluetooth = false;
        boolean gpsLocation = false;
        boolean networkLocation = false;
        int brightness = 20;

        profile_name = mCursor.getString(mCursor.getColumnIndex(PROFILE_NAME));
        silent = mCursor.getInt(mCursor.getColumnIndex(SILENT)) > 0 ? true : false;
        vibrate = mCursor.getInt(mCursor.getColumnIndex(VIBRATE)) > 0 ? true : false;
        ringtone1 = mCursor.getString(mCursor.getColumnIndex(RINGTONE1));
        ringtone2 = mCursor.getString(mCursor.getColumnIndex(RINGTONE2));
        ring_volume = mCursor.getInt(mCursor.getColumnIndex(RING_VOLUME));
        data = mCursor.getInt(mCursor.getColumnIndex(DATA)) > 0 ? true : false;
        wifi = mCursor.getInt(mCursor.getColumnIndex(WIFI)) > 0 ? true : false;
        bluetooth = mCursor.getInt(mCursor.getColumnIndex(BLUETOOTH)) > 0 ? true : false;
        gpsLocation = mCursor.getInt(mCursor.getColumnIndex(GPS_LOCATION)) > 0 ? true : false;
        networkLocation = mCursor.getInt(mCursor.getColumnIndex(NETWORK_LOCATION)) > 0 ? true
                : false;
        brightness = mCursor.getInt(mCursor.getColumnIndex(BRIGHTNESS));

        mProfile.setProfileName(profile_name);
        mProfile.setSilent(silent);
        mProfile.setVibrate(vibrate);
        mProfile.setRingtone1(ringtone1);
        mProfile.setRingtone2(ringtone2);
        mProfile.setRingVolume(ring_volume);
        mProfile.setData(data);
        mProfile.setWifi(wifi);
        mProfile.setBluetooth(bluetooth);
        mProfile.setGpsLocation(gpsLocation);
        mProfile.setNetworkLocation(networkLocation);
        mProfile.setBrightness(brightness);

        return mProfile;
    }

    @SuppressWarnings("unused")
    private BroadcastReceiver mReceiver = new BroadcastReceiver() {

        @Override
        public void onReceive(Context arg0, Intent arg1) {

            if (LOG) {
                loge(arg1.getAction());
                // loge(arg1.getExtra(AudioManager.EXTRA_VOLUME_STREAM_TYPE));
            }
        }

    };

    static Profile getCurrentSettings() {

        Profile mProfile = new Profile();
        boolean silentCur = false;
        boolean vibrateCur = false;
        int ring_volumeCur;
        boolean dataCur = true;
        boolean wifiCur = false;
        boolean bluetoothCur = false;
        boolean gpsLocationCur = false;
        boolean networkLocationCur = false;
        int brightnessCur = DEFAULT_BRIGHTNESS;
        String ringtone1Cur = null;
        String ringtone2Cur = null;

        // whether in silent
        silentCur = false;
        if (mAudioManager.getRingerMode() == AudioManager.RINGER_MODE_SILENT
                || mAudioManager.getRingerMode() == AudioManager.RINGER_MODE_VIBRATE) {
            silentCur = true;
        }

        if (LOG) {
            if (mAudioManager.getRingerMode() == AudioManager.RINGER_MODE_VIBRATE)
                logd("RingerMode=Vibrate");
            if (mAudioManager.getRingerMode() == AudioManager.RINGER_MODE_SILENT)
                logd("RingerMode=Silent");
            if (mAudioManager.getRingerMode() == AudioManager.RINGER_MODE_NORMAL)
                logd("RingerMode=Normal");

            if (mAudioManager.getVibrateSetting(AudioManager.VIBRATE_TYPE_RINGER) == AudioManager.VIBRATE_SETTING_ONLY_SILENT)
                logd("VIBRATE_TYPE_RINGER=VIBRATE_SETTING_ONLY_SILENT");
            if (mAudioManager.getVibrateSetting(AudioManager.VIBRATE_TYPE_RINGER) == AudioManager.VIBRATE_SETTING_ON)
                logd("VIBRATE_TYPE_RINGER=VIBRATE_SETTING_ON");
            if (mAudioManager.getVibrateSetting(AudioManager.VIBRATE_TYPE_RINGER) == AudioManager.VIBRATE_SETTING_OFF)
                logd("VIBRATE_TYPE_RINGER=VIBRATE_SETTING_OFF");

            if (mAudioManager.getVibrateSetting(AudioManager.VIBRATE_TYPE_NOTIFICATION) == AudioManager.VIBRATE_SETTING_ONLY_SILENT)
                logd("VIBRATE_TYPE_NOTIFICATION=VIBRATE_SETTING_ONLY_SILENT");
            if (mAudioManager.getVibrateSetting(AudioManager.VIBRATE_TYPE_NOTIFICATION) == AudioManager.VIBRATE_SETTING_ON)
                logd("VIBRATE_TYPE_NOTIFICATION=VIBRATE_SETTING_ON");
            if (mAudioManager.getVibrateSetting(AudioManager.VIBRATE_TYPE_NOTIFICATION) == AudioManager.VIBRATE_SETTING_OFF)
                logd("VIBRATE_TYPE_NOTIFICATION=VIBRATE_SETTING_OFF");

            boolean vibeInSilent = (Settings.System.getInt(mContext.getContentResolver(),
                    Settings.System.VIBRATE_IN_SILENT, 1) == 1);
            logd("vibeInSilent=" + vibeInSilent);
        }
        // Fetch the value of vibrator according to method soundSetting which
        // was set before.
        vibrateCur = Settings.System.getInt(mContext.getContentResolver(),
                Settings.System.VIBRATE_IN_SILENT, 1) == 1;

        if (mConnectivityManager != null)
            dataCur = mConnectivityManager.getMobileDataEnabled();

        if (mWifiManager != null) {
            int wifiState = mWifiManager.getWifiState();
            if (wifiState == WifiManager.WIFI_STATE_ENABLED
                    || wifiState == WifiManager.WIFI_STATE_ENABLING)
                wifiCur = true;
            else
                wifiCur = false;
        }
        if (mBluetoothAdapter != null) {
            int bluetoothState = mBluetoothAdapter.getState();
            if (bluetoothState == BluetoothAdapter.STATE_ON
                    || bluetoothState == BluetoothAdapter.STATE_TURNING_ON)
                bluetoothCur = true;
            else
                bluetoothCur = false;
        }

        gpsLocationCur = Settings.Secure.isLocationProviderEnabled(mContext.getContentResolver(),
                LocationManager.GPS_PROVIDER);

        networkLocationCur = Settings.Secure.isLocationProviderEnabled(mContext
                .getContentResolver(), LocationManager.NETWORK_PROVIDER);

        try {
            brightnessCur = Settings.System.getInt(mContext.getContentResolver(),
                    "screen_brightness");
        } catch (SettingNotFoundException e) {
            e.printStackTrace();
        }

        ring_volumeCur = mAudioManager.getStreamVolume(AudioManager.STREAM_RING);

        ringtone1Cur = getCurrentRingtone(0);
        ringtone2Cur = getCurrentRingtone(1);

        mProfile.setSilent(silentCur);
        mProfile.setVibrate(vibrateCur);
        mProfile.setRingtone1(ringtone1Cur);
        mProfile.setRingtone2(ringtone2Cur);
        mProfile.setRingVolume(ring_volumeCur);
        mProfile.setData(dataCur);
        mProfile.setWifi(wifiCur);
        mProfile.setBluetooth(bluetoothCur);
        mProfile.setGpsLocation(gpsLocationCur);
        mProfile.setNetworkLocation(networkLocationCur);
        mProfile.setBrightness(brightnessCur);
        return mProfile;
    }

    private boolean matchCurSetting(Profile mProfile) {

        if (getSelectedId() != mProfile.getId())
            return false;
        logd("ID match success, to match profile config>>");
        /** Current Profile */
        boolean silentCur = mProfileCur.isSilent();
        boolean vibrateCur = mProfileCur.isVibrate();
        int ring_volumeCur = mProfileCur.getRingVolume();
        boolean dataCur = mProfileCur.isData();
        boolean wifiCur = mProfileCur.isWifi();
        boolean bluetoothCur = mProfileCur.isBluetooth();
        boolean gpsLocationCur = mProfileCur.isGpsLocation();
        boolean networkLocationCur = mProfileCur.isNetworkLocation();
        int brightnessCur = mProfileCur.getBrightness();
        String ringtone1Cur = mProfileCur.getRingtone1();
        String ringtone2Cur = mProfileCur.getRingtone2();

        // If on BRIGHTNESS_MODE_AUTOMATIC, makes it always mismatch
        if (Settings.System
                .getInt(getContentResolver(), Settings.System.SCREEN_BRIGHTNESS_MODE, -1) == Settings.System.SCREEN_BRIGHTNESS_MODE_AUTOMATIC)
            brightnessCur = -1;

        /** Matching Profile */
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

        /** Function Enable or not */
        boolean silentCtrlEnabled = ProfileFunctionConfig.isSilentCtrlEnabled();
        boolean vibrateCtrlEnabled = ProfileFunctionConfig.isVibrateCtrlEnabled();
        boolean ringVolumeCtrlEnabled = ProfileFunctionConfig.isRingVolumeCtrlEnabled();
        boolean dataCtrlEnabled = ProfileFunctionConfig.isDataCtrlEnabled();
        boolean wifiCtrlEnabled = ProfileFunctionConfig.isWifiCtrlEnabled();
        boolean bluetoothCtrlEnabled = ProfileFunctionConfig.isBluetoothCtrlEnabled();
        boolean gpsLocationCtrlEnabled = ProfileFunctionConfig.isGpsLocationCtrlEnabled();
        boolean networkLocationCtrlEnabled = ProfileFunctionConfig.isNetworkLocationCtrlEnabled();
        boolean brightnessCtrlEnabled = ProfileFunctionConfig.isBrightnessCtrlEnabled();
        boolean ringtoneCtrlEnabled = ProfileFunctionConfig.isRingtoneCtrlEnabled();

        //add to disable the flag of silentCtrlEnabled when slient is false to match with the flag of silentCur
        if (false == silent) {
            silentCtrlEnabled = false;
        }
        else {
            silentCtrlEnabled = true;
        }

        // boolean value
        // if silentCtrl is disabled, !silentCtrlEnabled will be always true,
        // so it is equal to not check this item
        if ((silentCur == silent || !silentCtrlEnabled)
                && (vibrateCur == vibrate || !vibrateCtrlEnabled)
                && (brightnessCur == brightness || !brightnessCtrlEnabled)
                && (dataCur == data || !dataCtrlEnabled) && (wifiCur == wifi || !wifiCtrlEnabled)
                && (bluetoothCur == bluetooth || !bluetoothCtrlEnabled)
                && (gpsLocationCur == gpsLocation || !gpsLocationCtrlEnabled)
                && (networkLocationCur == networkLocation || !networkLocationCtrlEnabled)) {
            // non-boolean value
            // ringtone
            if (ringtoneCtrlEnabled) {
                if ((ringtone1 == null && ringtone1Cur != null)
                        || (ringtone2 == null && ringtone2Cur != null)
                        || (ringtone1Cur == null && ringtone1 != null)
                        || (ringtone2Cur == null && ringtone2 != null))
                    return false;
                if ((ringtone1 != null && ringtone1Cur != null && (ringtone1
                        .compareTo(ringtone1Cur) != 0))
                        || (ringtone2 != null && ringtone2Cur != null && (ringtone2
                                .compareTo(ringtone2Cur)) != 0))
                    return false;
            }
            // ringVolume
            if (ringVolumeCtrlEnabled) {

                if (ring_volumeCur == ring_volume)
                    return true;
                else if (silentCur == true)
                    return true;
            } else
                return true;

        }
        return false;
    }

    // using when get reference
    private <T> boolean nullJudge(T ref, String pos) {

        if (ref == null) {
            loge(pos + "=null");
            ProfileMain.this.finish();
            return true;
        }else
            return false;
    }

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

    void setSelectedId(int id) {

        ProfileUtils.saveIntValue(mContext, ProfileUtils.KEY_SELECTED_ID, id);
    }

    int getSelectedId() {

        return ProfileUtils.getIntValueSaved(mContext, ProfileUtils.KEY_SELECTED_ID, -1);
    }

    @Override
    public boolean onKeyUp(int keyCode, KeyEvent event) {

        if (keyCode == KeyEvent.KEYCODE_VOLUME_DOWN || keyCode == KeyEvent.KEYCODE_VOLUME_UP) {
            mModifyInThisApp = false;
            mProfileCur = getCurrentSettings();
            getAllProfiles();
            notifyChanged();
        }
        return super.onKeyUp(keyCode, event);
    }
}
