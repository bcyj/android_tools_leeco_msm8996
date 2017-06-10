/******************************************************************************
 * @file    VoiceCallFeatures.java
 * @brief   Provides the settings to enable/disable voice call related
 *          features
 *
 *
 * ---------------------------------------------------------------------------
 *  Copyright (c) 2014 Qualcomm Technologies, Inc.
 *  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
 *  ---------------------------------------------------------------------------
 *  ******************************************************************************/
package com.qualcomm.qualcommsettings;

import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.provider.Settings;
import android.preference.Preference;
import android.preference.PreferenceActivity;
import android.preference.CheckBoxPreference;
import android.preference.ListPreference;
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;
import android.content.SharedPreferences.OnSharedPreferenceChangeListener;
import android.media.AudioManager;
import android.media.AudioSystem;
import android.os.SystemProperties;
import android.util.Log;

public class VoiceCallFeatures extends PreferenceActivity implements
        OnSharedPreferenceChangeListener {

   private static final String TAG = "VoiceCallFeatures";
   public static final String FLUENCE = "fluence";
   public static final String FLUENCE_MODE = "fluencemode";
   public static final String SLOWTALK = "slowtalk";
   public static final String DEVICE_MUTE = "devicemute";
   public static final String DIRECTION = "direction";
   public static final String DIRECTION_RX = "direction_rx";
   public static final String DIRECTION_TX = "direction_tx";
   public static final String INCALLMUSIC = "incallmusic";
   public static final String HDVOICE = "hdvoice";

   private CheckBoxPreference IncallMusic;
   private ListPreference FluenceMode;
   private CheckBoxPreference FluenceType;
   private CheckBoxPreference SlowTalk;
   private CheckBoxPreference DeviceMute;
   private ListPreference Direction;
   private CheckBoxPreference HDVoice;

   private AudioManager mAudioManager;

   /** Called when the activity is first created. */
   @Override
   public void onCreate(Bundle icicle) {
       super.onCreate(icicle);
       mAudioManager = (AudioManager)getSystemService(Context.AUDIO_SERVICE);
       SharedPreferences vtSharedPref = getSharedPreferences("voice_call_features", 0);
       addPreferencesFromResource(R.xml.voice_call_features);

       IncallMusic = (CheckBoxPreference)findPreference(INCALLMUSIC);
       FluenceMode = (ListPreference) findPreference(FLUENCE);
       FluenceType = (CheckBoxPreference) findPreference(FLUENCE_MODE);
       SlowTalk = (CheckBoxPreference) findPreference(SLOWTALK);
       DeviceMute = (CheckBoxPreference) findPreference(DEVICE_MUTE);
       Direction = (ListPreference) findPreference(DIRECTION);
       HDVoice = (CheckBoxPreference) findPreference(HDVOICE);

       getPreferenceScreen().getSharedPreferences().registerOnSharedPreferenceChangeListener(this);

       changeFluenceStatus();
       updateFluenceTypeStatus();
       updateSlowTalkStatus();
       updateDeviceMuteStatus();
       updateHDVoiceStatus();
   }

   @Override
   protected void onResume() {
       super.onResume();
       changeFluenceStatus();
       updateFluenceTypeStatus();
       updateSlowTalkStatus();
       updateDeviceMuteStatus();
       updateHDVoiceStatus();
   }

  /* Fluence */
   private void updateFluenceTypeStatus() {
       String fmodestatus = mAudioManager.getParameters("fluence");
       if (fmodestatus.equals("fluence=none")) {
           FluenceType.setChecked(false);
           FluenceType.setSummary("Disabled");
       }
       else {
           FluenceType.setChecked(true);
           FluenceType.setSummary("Enabled");
       }
   }

   private void changeFluenceStatus() {
       String fluenceMode = FluenceMode.getValue();
       Log.e(TAG, "changeFluenceStatus=" + fluenceMode);
       String property = SystemProperties.get("ro.qc.sdk.audio.fluencetype");
       if (property != null) {
           //mAudioManager.setParameters("fluence="+fluenceMode);
           //Do not allow changing of fluence type
           FluenceMode.setEnabled(false);
           //Set default Fluence type by reading audio parameter
           fluenceMode = mAudioManager.getParameters("fluence");
       }
       FluenceMode.setSummary(fluenceMode);
   }

   private void changeFluenceTypeStatus() {
       boolean FluenceModeEnabled = FluenceType.isChecked();
       mAudioManager.setParameters("fluence=" + (FluenceModeEnabled ? "dualmic" : "none"));
       String currentStatus = mAudioManager.getParameters("fluence");
       FluenceType.setSummary(currentStatus.equals("fluence=dualmic") ? "Enabled" : "Disabled");
       SharedPreferences vtSharedPref = getSharedPreferences("voice_call_features", 0);
       SharedPreferences.Editor editor = vtSharedPref.edit();
       editor.putInt(FLUENCE_MODE, currentStatus.equals("fluence=dualmic") ? 1: 0);
       editor.commit();
   }

   /* Incall Music */
   private void changeIncallMusicStatus() {
       boolean INCALLMUSICEnabled = IncallMusic.isChecked();
       mAudioManager.setParameters("incall_music_enabled="+(INCALLMUSICEnabled ? "true" : "false"));
       IncallMusic.setSummary(INCALLMUSICEnabled ? "Enabled" : "Disabled");
       Log.e(TAG,"In changeINCALLMUSIC status, incallmusic enabled= "+INCALLMUSICEnabled);
       Settings.System.putInt(getContentResolver(), INCALLMUSIC,
               INCALLMUSICEnabled ? 1 : 0);
   }

   /* Slow Talk */
   private void updateSlowTalkStatus() {
       int st_status = 0;
       SharedPreferences vtSharedPref = getSharedPreferences("voice_call_features", 0);
       st_status = vtSharedPref.getInt(SLOWTALK, 0);
       mAudioManager.setParameters("st_enable=" + ((st_status == 1) ? "true" : "false"));
       SlowTalk.setSummary((st_status == 1)? "Enabled" : "Disabled");
   }

   private void changeSlowTalkStatus() {
       boolean SlowTalkEnabled = SlowTalk.isChecked();
       mAudioManager.setParameters("st_enable=" + (SlowTalkEnabled ? "true" : "false"));
       String currentStatus = mAudioManager.getParameters("st_enable");
       SlowTalk.setSummary(currentStatus.equals("st_enable=true") ? "Enabled" : "Disabled");
       SharedPreferences vtSharedPref = getSharedPreferences("voice_call_features", 0);
       SharedPreferences.Editor editor = vtSharedPref.edit();
       editor.putInt(SLOWTALK, currentStatus.equals("st_enable=true") ? 1: 0);
       editor.commit();
   }

   /* Device Mute */
   private void updateDeviceMuteStatus() {
       int dmute_status = 0;
       SharedPreferences vtSharedPref = getSharedPreferences("voice_call_features", 0);
       dmute_status = vtSharedPref.getInt(DEVICE_MUTE, 0);
       if (dmute_status == 1) {
           String dir = Direction.getValue();
           mAudioManager.setParameters("device_mute=true" + ";" + (dir.equals("Rx") ? "direction=rx" : "direction=tx"));

           if ((vtSharedPref.getInt(DIRECTION_RX,0) == 1) && (vtSharedPref.getInt(DIRECTION_TX,0) == 1))
               Direction.setSummary("Rx, Tx");
           else if ((vtSharedPref.getInt(DIRECTION_RX,0) == 1) && (vtSharedPref.getInt(DIRECTION_TX,0) == 0))
               Direction.setSummary("Rx");
           else
               Direction.setSummary("Tx");

           DeviceMute.setSummary("Enabled");
       }
       else {
           Direction.setEnabled(false);
           DeviceMute.setSummary("Disabled");
       }
   }

   private boolean changeDeviceMuteDirection() {
       String dir = Direction.getValue();
       SharedPreferences vtSharedPref = getSharedPreferences("voice_call_features", 0);
       SharedPreferences.Editor editor = vtSharedPref.edit();

       if (dir.equals("Rx"))
           editor.putInt(DIRECTION_RX, 1);
       else
           editor.putInt(DIRECTION_TX, 1);

       editor.commit();
       mAudioManager.setParameters("device_mute=true" + ";" + (dir.equals("Rx") ? "direction=rx" : "direction=tx"));
       DeviceMute.setSummary("Enabled");
       if ((vtSharedPref.getInt(DIRECTION_RX,0) == 1) && (vtSharedPref.getInt(DIRECTION_TX,0) == 1))
           Direction.setSummary("Rx, Tx");
       else if ((vtSharedPref.getInt(DIRECTION_RX,0) == 1) && (vtSharedPref.getInt(DIRECTION_TX,0) == 0))
           Direction.setSummary("Rx");
       else
           Direction.setSummary("Tx");
       editor.putInt(DEVICE_MUTE, 1);
       editor.commit();
       return true;
   }

   private void changeDeviceMuteStatus() {
       boolean DeviceMuteEnabled = DeviceMute.isChecked();
       SharedPreferences vtSharedPref = getSharedPreferences("voice_call_features", 0);
       SharedPreferences.Editor editor = vtSharedPref.edit();
       if (Direction.getValue() != null && DeviceMuteEnabled) {
           String dir = Direction.getValue();
           mAudioManager.setParameters("device_mute=true" + ";" + (dir.equals("Rx") ? "direction=rx" : "direction=tx"));

           if (dir.equals("Rx"))
               editor.putInt(DIRECTION_RX, 1);
           else
               editor.putInt(DIRECTION_TX, 1);

           editor.commit();

           if ((vtSharedPref.getInt(DIRECTION_RX,0) == 1) && (vtSharedPref.getInt(DIRECTION_TX,0) == 1))
               Direction.setSummary("Rx, Tx");
           else if ((vtSharedPref.getInt(DIRECTION_RX,0) == 1) && (vtSharedPref.getInt(DIRECTION_TX,0) == 0))
               Direction.setSummary("Rx");
           else
               Direction.setSummary("Tx");

           DeviceMute.setSummary("Enabled");
           editor.putInt(DEVICE_MUTE, 1);
           editor.commit();
       }
       else {
           if (DeviceMuteEnabled)
               DeviceMute.setSummary("Set direction for device mute. Currently Disabled!");
           else {
               mAudioManager.setParameters("device_mute=false;direction=rx");
               mAudioManager.setParameters("device_mute=false;direction=tx");
               editor.putInt(DIRECTION_RX, 0);
               editor.putInt(DIRECTION_TX, 0);
               editor.commit();
               Direction.setSummary("Set device mute direction i.e. Rx/Tx");
               DeviceMute.setSummary("Disabled");
           }
       }

       if (DeviceMuteEnabled) {
           Direction.setEnabled(true);
       }
       else {
           Direction.setEnabled(false);
           editor.putInt(DEVICE_MUTE, 0);
           editor.commit();
       }
   }

   /* HD Voice */
   private void updateHDVoiceStatus() {
       int hd_status = 0;
       SharedPreferences vtSharedPref = getSharedPreferences("voice_call_features", 0);
       hd_status = vtSharedPref.getInt(HDVOICE, 0);
       mAudioManager.setParameters("hd_voice=" + ((hd_status == 1) ? "true" : "false"));
       HDVoice.setSummary((hd_status == 1)? "Enabled" : "Disabled");
   }

   private void changeHDVoiceStatus() {
       boolean HDVoiceEnabled = HDVoice.isChecked();
       mAudioManager.setParameters("hd_voice=" + (HDVoiceEnabled ? "true" : "false"));
       String currentStatus = mAudioManager.getParameters("hd_voice");
       HDVoice.setSummary(currentStatus.equals("hd_voice=true") ? "Enabled" : "Disabled");
       SharedPreferences vtSharedPref = getSharedPreferences("voice_call_features", 0);
       SharedPreferences.Editor editor = vtSharedPref.edit();
       editor.putInt(HDVOICE, currentStatus.equals("hd_voice=true") ? 1: 0);
       editor.commit();
   }

   public void onSharedPreferenceChanged(SharedPreferences sharedPreferences,
       String key) {
       if (key.equals(FLUENCE)) {
           changeFluenceStatus();
       }
       else if (key.equals(FLUENCE_MODE)) {
           changeFluenceTypeStatus();
           changeFluenceStatus();
       }
       else if (key.equals(SLOWTALK)) {
           changeSlowTalkStatus();
       }
       else if (key.equals(DEVICE_MUTE)) {
           changeDeviceMuteStatus();
       }
       else if (key.equals(DIRECTION)) {
           changeDeviceMuteDirection();
       }
       else if (key.equals(INCALLMUSIC)) {
           changeIncallMusicStatus();
       }
       else if (key.equals(HDVOICE)) {
           changeHDVoiceStatus();
       }
   }

   @Override
   protected void onDestroy() {
       super.onDestroy();
       getPreferenceScreen().getSharedPreferences().unregisterOnSharedPreferenceChangeListener(
           this);
   }
}

