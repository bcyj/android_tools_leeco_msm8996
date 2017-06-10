/******************************************************************************
 * @file    QualcommSettings.java
 * @brief   Provides the settings for varoius features
 *
 *
 * ---------------------------------------------------------------------------
 *  Copyright (c) 2009-2014 Qualcomm Technologies, Inc.
 *  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
 *  ---------------------------------------------------------------------------
 *  ******************************************************************************/
package com.qualcomm.qualcommsettings;

import com.android.internal.telephony.ITelephony;
import com.android.internal.telephony.TelephonyProperties;
import com.qualcomm.qcrilhook.IQcRilHook;
import com.qualcomm.qcrilhook.QcRilHook;
import com.qualcomm.qcrilhook.QcRilHookCallback;
import android.os.IBinder;
import android.app.Activity;
import android.os.Bundle;
import android.content.pm.PackageManager;
import android.content.pm.PackageInfo;
import android.content.pm.PackageItemInfo;
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;
import android.content.SharedPreferences.OnSharedPreferenceChangeListener;
import android.preference.EditTextPreference;
import android.preference.Preference;
import android.preference.PreferenceActivity;
import android.preference.PreferenceCategory;
import android.preference.CheckBoxPreference;
import android.preference.ListPreference;
import android.preference.PreferenceGroup;
import android.preference.PreferenceScreen;
import android.util.Log;
import android.view.View;
import android.view.View.OnClickListener;
import android.media.AudioManager;
import android.media.AudioSystem;
import android.provider.Settings;
import android.os.RemoteException;
import android.provider.Settings.SettingNotFoundException;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.content.ComponentName;
import android.content.ContentResolver;
import android.widget.Toast;
import android.os.storage.IMountService;
import android.os.ServiceManager;
import android.os.SystemProperties;
import android.os.BatteryManager;
import android.os.SystemService;

import java.io.BufferedReader;
import java.io.File;
import java.io.IOException;
import java.io.InputStreamReader;


import android.telephony.TelephonyManager;
import android.nfc.NfcAdapter;

import java.util.List;
import java.io.*;

public class QualcommSettings extends PreferenceActivity implements
        OnSharedPreferenceChangeListener, Preference.OnPreferenceClickListener,
        Preference.OnPreferenceChangeListener {

   private static final String TAG = "QualcommSettings";
   public static final String QCOM_SETTINGS = "qualcomm_settings";
   public static final String DIM_SCREEN = "dim_screen";
   public static final String AUDIO_DEFAULT_DEVICE = "spdif_audio_device";
   public static final String STAY_ON_PLUGGED = "stay_on";
   public static final String SOCKET_DATA_CALL = "socket_data_call";
   public static final String AUTO_ANSWER_VOICE_CALL = "auto_answer_voice_call";
   public static final String GO_DORMANT = "go_dormant";
   public static final String USB_MASS_STORAGE = "usb_mass_storage";
   public static final String USB_REMOTE_WAKEUP = "usb_remote_wakeup";
   public static final String SD_POLLING_ENABLED = "sd_polling";
   public static final String USB_COMPOSITION = "usb_composition";
   public static final String USB_OTG_MODE = "usb_otg_mode";
   public static final String USB_OTG_FILE = "/sys/kernel/debug/otg/mode";
   public static final String DATA_USAGE = "data_usage";
   public static final String CABL_SETTINGS = "cabl_settings";
   public static final String SVI_SETTINGS = "svi_settings";
   public static final String AD_SETTINGS = "ad_settings";
   public static final String PP_SETTINGS = "pp_settings";
   public static final String DISABLE_HDMIAUDIO_OUT = "disable_hdmi_audio_output";
   public static final String ANC = "anc";
   public static final String SET_CDMA_SUB_SRC = "set_cdma_sub_src";
   public static final String FILE_MANAGER = "file_manager";
   public static final String SENSORS = "sensors";
   public static final String SENSORS_SETTINGS = "/persist/sensors/sensors_settings";
   public static final String DIAG_LOG = "diag_log";
   public static final String CDMA_NWK_AVOIDANCE = "cdma_nwk_avoidance";
   public static final String NFC_DEFAULT_ROUTE_SETTING = "nfc_default_route_setting";
   private static final String CABL_PACKAGE = "com.qualcomm.cabl";
   private static final String SVI_PACKAGE = "com.qualcomm.svi";
   private static final String AD_PACKAGE = "com.qualcomm.ad";
   private static final String PP_PACKAGE = "com.qualcomm.display";
   private static final String CABL_PREFS_CLASS = "com.qualcomm.cabl.CABLPreferences";
   private static final String SVI_PREFS_CLASS = "com.qualcomm.svi.SVIPreferences";
   private static final String AD_PREFS_CLASS = "com.qualcomm.ad.ADPreferences";
   private static final String PP_PREFS_CLASS = "com.qualcomm.display.PPPreference";
   private static final String GESTURES_TOUCH_INJECTION = "gestures_touch_injection";
   private static final String DATA_MONITOR = "data_monitor";
   private static final String DEFAULT_FILE_MANAGER = "def_file_manager";
   private static final String AUTO_ANSWER_TIMEOUT_SYSPROP = "persist.sys.tel.autoanswer.ms";
   public static final String MULTI_SIM_SETTING = "multi_sim_settings";

   /* Phone service name */
   private static final String PHONE_SERVICE = "phone";
   private static final int CDMA_SUB = 0;

   /* Service Interface */
   private com.qualcomm.qualcommsettings.IDun DunService;

   /* Service Bind */
   private boolean DunServiceBind=false;
   private boolean umsValue=false;
   private int sdSlot=1;
   private static boolean afterBoot=true;

   private PreferenceCategory QSettings;
   private CheckBoxPreference DimScreen;
   private CheckBoxPreference DefaultDeviceSpdif;
   private CheckBoxPreference StayOn;
   private CheckBoxPreference Ums;
   private CheckBoxPreference Urw;
   private CheckBoxPreference SdPoll;
   private CheckBoxPreference DisableHdmiAudio;
   private CheckBoxPreference Anc;
   private CheckBoxPreference Sensors;
   private CheckBoxPreference DiagLogging;
   private ListPreference mAutoAnswerVoiceCall;
   private EditTextPreference GoDormant;
   private ListPreference UsbComposition;
   private ListPreference UsbOtgMode;
   private Preference DataUsage;
   private Preference CABLSettings;
   private Preference SVISettings;
   private Preference ADSettings;
   private Preference PPSettings;
   private PreferenceScreen FileManager;
   private PreferenceGroup FileManagerList;
   private CheckBoxPreference Gestures_Touch_Injection;
   private CheckBoxPreference Data_Monitor_Status;
   private Preference SetCdmaSubSrc;
   private Preference MultiSimSettings;
   private Preference AvoidCdmaNwk;
   private Preference NfcDefaultRouteSetting;

   private AudioManager mAudioManager;
   private QcRilHook mQcRilHook;

   private ITelephony mPhone;

   private static boolean diagBinExists=false;
   private int test;
   Thread stdoutThread;
   BufferedReader reader;

   private Context mContext;

   /** Called when the activity is first created. */
   @Override

      public void onCreate(Bundle icicle)
      {
         super.onCreate(icicle);
         mContext = getApplicationContext();
         mAudioManager = (AudioManager)getSystemService(Context.AUDIO_SERVICE);
         mQcRilHook = new QcRilHook(mContext, mQcrilHookCb);
         SharedPreferences qcSharedPref = getSharedPreferences("settings_preferences", 0);
         addPreferencesFromResource(R.xml.settings_preferences);
         QSettings = (PreferenceCategory) findPreference(QCOM_SETTINGS);
         DimScreen = (CheckBoxPreference) findPreference(DIM_SCREEN);
         DefaultDeviceSpdif = (CheckBoxPreference) findPreference(AUDIO_DEFAULT_DEVICE);
         StayOn = (CheckBoxPreference) findPreference(STAY_ON_PLUGGED);
         Ums = (CheckBoxPreference)findPreference(USB_MASS_STORAGE);
         Urw = (CheckBoxPreference)findPreference(USB_REMOTE_WAKEUP);
         SdPoll = (CheckBoxPreference)findPreference(SD_POLLING_ENABLED);
         DisableHdmiAudio = (CheckBoxPreference)findPreference(DISABLE_HDMIAUDIO_OUT);
         Anc = (CheckBoxPreference)findPreference(ANC);
         Sensors = (CheckBoxPreference)findPreference(SENSORS);
         DiagLogging = (CheckBoxPreference)findPreference(DIAG_LOG);
         Gestures_Touch_Injection = (CheckBoxPreference)findPreference(GESTURES_TOUCH_INJECTION);
         Data_Monitor_Status = (CheckBoxPreference)findPreference(DATA_MONITOR);

         mAutoAnswerVoiceCall = (ListPreference) findPreference(AUTO_ANSWER_VOICE_CALL);

         GoDormant = (EditTextPreference) findPreference(GO_DORMANT);
         getPreferenceScreen().getSharedPreferences().registerOnSharedPreferenceChangeListener(this);
         DunServiceBind = bindService(new Intent(this, DunService.class), DunConnection,
                Context.BIND_AUTO_CREATE);
         UsbComposition = (ListPreference) findPreference(USB_COMPOSITION);
         //Deprecating the USBComposition menu
         QSettings.removePreference(UsbComposition);
         UsbOtgMode = (ListPreference) findPreference(USB_OTG_MODE);
         DataUsage = (Preference) findPreference(DATA_USAGE);
         DataUsage.setOnPreferenceClickListener(this);

         SetCdmaSubSrc = (Preference) findPreference(SET_CDMA_SUB_SRC);
         SetCdmaSubSrc.setOnPreferenceClickListener(this);

         MultiSimSettings = (Preference) findPreference(MULTI_SIM_SETTING);
         MultiSimSettings.setOnPreferenceClickListener(this);

         CABLSettings = (Preference) findPreference(CABL_SETTINGS);
         CABLSettings.setOnPreferenceClickListener(this);

         SVISettings = (Preference) findPreference(SVI_SETTINGS);
         SVISettings.setOnPreferenceClickListener(this);

         boolean isADSupported = false;
         String calib_file = SystemProperties.get("ro.qcom.ad.calib.data");
         if(SystemProperties.getBoolean("ro.qcom.ad", false) &&
             !calib_file.isEmpty()) {
             isADSupported = true;
         }

         ADSettings = (Preference) findPreference(AD_SETTINGS);
         if (!isADSupported) {
             QSettings.removePreference(ADSettings);
             ADSettings = null;
         } else {
             ADSettings.setOnPreferenceClickListener(this);
             ADSettings.setEnabled(true);
         }

         PPSettings = (Preference) findPreference(PP_SETTINGS);
         PPSettings.setOnPreferenceClickListener(this);

         getSDSlot();
         FileManager = (PreferenceScreen) findPreference(FILE_MANAGER);
         updateHDMIAudioOutput();
         updateANCStatus();
         updateSensorsStatus();
         updateGesturesTouchInjectionStatus();

         if (afterBoot==true)
         {
            DiagLogging.setChecked(false);
            Ums.setChecked(false);
            afterBoot=false;
         }
         umsValue = Ums.isChecked();

         CABLSettings.setEnabled(false);
         SVISettings.setEnabled(false);

         if(SystemProperties.getInt("ro.qualcomm.cabl", 0) > 0){
             CABLSettings.setEnabled(true);
             if(SystemProperties.getInt("ro.qualcomm.cabl", 0) == 2){
                 if(SystemProperties.getBoolean("ro.qualcomm.svi", false))
                     SVISettings.setEnabled(true);
             }
         }

         AvoidCdmaNwk = (Preference) findPreference(CDMA_NWK_AVOIDANCE);
         if (Boolean.parseBoolean(
                 SystemProperties.get(TelephonyProperties.PROPERTY_INECM_MODE))) {
             // In ECM mode, disable the avoid cdma network command
             Log.d(TAG, "Disable the avoid cdma network command as we are in ECM mode");
             AvoidCdmaNwk.setEnabled(false);
         }

         NfcDefaultRouteSetting = (Preference) findPreference(NFC_DEFAULT_ROUTE_SETTING);
         NfcDefaultRouteSetting.setOnPreferenceClickListener(this);
         NfcAdapter nfcAdapter = NfcAdapter.getDefaultAdapter(mContext);
         if (nfcAdapter == null) {
             NfcDefaultRouteSetting.setEnabled(false);
         } else {
             NfcDefaultRouteSetting.setEnabled(true);
         }
      }

    private QcRilHookCallback mQcrilHookCb = new QcRilHookCallback() {
        public void onQcRilHookReady() {
        }
        public void onQcRilHookDisconnected() {
        }
    };

   @Override
      protected void onResume() {
         super.onResume();
         updateDimScreen();
         updateStayOn();

         updateUsbMassStorageStatus();
         updateSdPollingStatus();

         resumeAutoAnswer();
         updateHDMIAudioOutput();
         updateANCStatus();
         updateSensorsStatus();
         updateGesturesTouchInjectionStatus();
      }

   @Override
      public boolean onPreferenceTreeClick(PreferenceScreen preferenceScreen,
                                            Preference preference) {
          if (preference == FileManager) {
              displayFileManagerList();
              return true;
         }
         return false;
      }

    public boolean onPreferenceClick(Preference preference) {
        if (preference == CABLSettings) {
            changeCABLSettings();
        } else if (preference == SVISettings) {
            changeSVISettings();
        } else if (preference == ADSettings) {
            changeADSettings();
        } else if (preference == PPSettings) {
            changePPSettings();
        } else if (preference == DataUsage) {
            changeDataUsageSettings();
        } else if (preference == SetCdmaSubSrc) {
            setCdmaSubSrc();
        } else if (preference == MultiSimSettings) {
            multiSimSettings();
        } else if (preference == NfcDefaultRouteSetting) {
            Intent intent = new Intent();
            intent.setAction("com.android.nfc.action.GET_DEFAULT_ISO_DEP_ROUTE");
            mContext.sendBroadcast(intent);
        } else {
            Preference p;
            int count = FileManagerList.getPreferenceCount();
            for (int i = 0; i < count; i++) {
                p = FileManagerList.getPreference(i);
                if (!(p.isEnabled())) {
                    p.setEnabled(true);
                }
            }
            /* Disable focus for the selected package name in the list menu */
            preference.setEnabled(false);
            String packageName = preference.getTitle().toString();
            /* Store file manager name in the data base */
            SharedPreferences qcSharedPref = getSharedPreferences("settings_preferences", 0);
            SharedPreferences.Editor editor = qcSharedPref.edit();
            editor.putString(DEFAULT_FILE_MANAGER, packageName);
            editor.commit();

        }
        return true;
    }

    public boolean onPreferenceChange(Preference preference, Object newValue) {
        return true;
    }


   /*
    * Display all installed packages.
    */
   private void displayFileManagerList() {

       FileManagerList =
          (PreferenceGroup) getPreferenceScreen().findPreference(FILE_MANAGER);

       /* remove old list */
       FileManagerList.removeAll();

       /* Read default file manager name from the data base */
       SharedPreferences qcSharedPref = getSharedPreferences("settings_preferences", 0);
       String fileManagerName = qcSharedPref.getString(DEFAULT_FILE_MANAGER, "invalid");
       /*
       * Read all installed packages information.
       * Add packege names to the display list
       */
       List<PackageInfo> packages =
            getPackageManager().getInstalledPackages(PackageManager.GET_ACTIVITIES);

       int count = packages.size();
       for (int p = 0 ; p < count ; p++) {
           PackageInfo info = packages.get(p);
           Preference fileManagerPreference = new Preference(this, null);
           fileManagerPreference.setTitle(info.packageName);
           fileManagerPreference.setShouldDisableView (true);

           if (info.packageName.equals(fileManagerName)) {
               fileManagerPreference.setEnabled(false);
           }
           fileManagerPreference.setOnPreferenceClickListener(this);
           FileManagerList.addPreference(fileManagerPreference);
      }
   }

   private void updateDimScreen() {
      boolean DimScreenValue = DimScreen.isChecked();
      DimScreen.setSummary(DimScreenValue ? "Screen will be Dim" : "Screen not Dim");
      Settings.System.putInt(getContentResolver(), Settings.System.DIM_SCREEN,
            DimScreenValue ? 1 : 0);
   }

   private void updateAudioDeviceSpdif() {
      boolean DefaultDeviceSpdifValue = DefaultDeviceSpdif.isChecked();
      DefaultDeviceSpdif.setSummary(DefaultDeviceSpdifValue ? "Device SPDIF" : "Device Mode Auto");
      SystemProperties.set("persist.sys.audio.default", (DefaultDeviceSpdifValue ? "spdif" : "none"));
   }

   private void updateStayOn() {
      boolean StayOnValue = StayOn.isChecked();
      StayOn.setSummary(StayOnValue ? "Screen On while charging"  : "Screen off while charging");
      Settings.System.putInt(getContentResolver(), Settings.System.STAY_ON_WHILE_PLUGGED_IN,
            StayOnValue ? (BatteryManager.BATTERY_PLUGGED_AC | BatteryManager.BATTERY_PLUGGED_USB) : 0);
   }

   private void changeCABLSettings() {
      Intent intent = new Intent();
      intent.setClassName(CABL_PACKAGE,CABL_PREFS_CLASS);
      startActivity(intent);
   }

   private void changeSVISettings() {
      Intent intent = new Intent();
      intent.setClassName(SVI_PACKAGE,SVI_PREFS_CLASS);
      startActivity(intent);
   }

   private void changeADSettings() {
      Intent intent = new Intent();
      intent.setClassName(AD_PACKAGE,AD_PREFS_CLASS);
      startActivity(intent);
   }

   private void changePPSettings() {
      Intent intent = new Intent();
      intent.setClassName(PP_PACKAGE,PP_PREFS_CLASS);
      startActivity(intent);
   }

   private void changeDataUsageSettings() {
      Intent intent = new Intent(this, DataUsage.class);
      startActivity(intent);
   }

   private void setCdmaSubSrc() {
       Intent intent = new Intent(this, SetCdmaSubSrc.class);
       startActivity(intent);
    }

   private void multiSimSettings() {
       Intent intent = new Intent(this, MultiSimSettings.class);
       startActivity(intent);
    }

   private void updateUsbRemoteWakeupStatus() {
       boolean UrwValue = Urw.isChecked();
       Urw.setSummary(UrwValue ? "Enable" : "Disable");
       try
       {
           File urw_file = null;
           FileOutputStream fos;
           DataOutputStream dos;
           urw_file = new File("/sys/devices/platform/msm_hsusb/gadget/wakeup");
           if (urw_file.exists() != true) {
                Log.e(TAG,
                        "updateUsbRemoteWakeupStatus: File does not exist "
                                + urw_file.getAbsolutePath());
               return;
           }
           fos = new FileOutputStream(urw_file);
           dos = new DataOutputStream(fos);
           if (UrwValue) {
               dos.writeInt(1);
           }
          else {
               dos.writeInt(0);
           }
           dos.close();
          fos.close();
       }
       catch (IOException e) {
          Log.e(TAG, "IOException caught while writing stream", e);
       }
    }

   private void resumeAutoAnswer() {
      int timeout = -1;
      timeout = SystemProperties.getInt(AUTO_ANSWER_TIMEOUT_SYSPROP , -1);
      if (timeout == -1) {
         mAutoAnswerVoiceCall.setValue("-1");
         return;
      }
      if (timeout == 15000) {
         mAutoAnswerVoiceCall.setValue("15000");
      }
      else if (timeout == 10000) {
         mAutoAnswerVoiceCall.setValue("10000");
      }
      else {
         mAutoAnswerVoiceCall.setValue("5000");
      }
   }

   private void updateAutoAnswerVoiceCall() {
      String AutoAnswerVoiceCallValue = mAutoAnswerVoiceCall.getValue();
      Log.d(TAG, "Setting Auto Answer System Property to " + AutoAnswerVoiceCallValue
              + " milliseconds.");
      SystemProperties.set(AUTO_ANSWER_TIMEOUT_SYSPROP, AutoAnswerVoiceCallValue);
   }

   private void updateUsbMassStorageStatus() {
      Ums.setSummary(umsValue ? "Enable"  : "Disable");
        IMountService mountService = IMountService.Stub.asInterface(ServiceManager
                .getService("mount"));
      if (mountService == null)
         return;

      if(umsValue) {
         try {
            //mountService.setAutoStartUms(true);
            mountService.setUsbMassStorageEnabled(true);
         } catch (RemoteException e) {
            Log.v(TAG, "Exception while Enabling/Disabling ums");
         }
      }
      else {
         try {
            // mountService.setAutoStartUms(false);
            mountService.setUsbMassStorageEnabled(false);
         } catch(RemoteException e) {
            Log.v(TAG, "Exception while Enabling/Disabling ums");
         }
      }

   }

   private boolean readSdStatus(int Slot)
   {
       char Status;
       boolean Enable = false;
       try
       {
           Reader sd_inputStream = null;
           File sd_file = null;
           switch(Slot)
           {
               case 1:
                   sd_file = new File("/sys/devices/platform/msm_sdcc.1/polling");
                   break;
               case 2:
                   sd_file = new File("//sys/devices/platform/msm_sdcc.2/polling");
                   break;
               case 3:
                   sd_file = new File("//sys/devices/platform/msm_sdcc.3/polling");
                   break;
               case 4:
                   sd_file = new File("//sys/devices/platform/msm_sdcc.4/polling");
                   break;
               default:
                   return false;
           }
           if(sd_file.exists() != true)
           {
               Log.e(TAG, "ReadPollingStatus: File does not exist " + sd_file.getAbsolutePath());
               return false;
           }

           sd_inputStream = new BufferedReader(new FileReader(sd_file));

           if(sd_inputStream.read() == 0x31)
               Enable = true;
           else
               Enable = false;
           sd_inputStream.close();
       } catch (IOException e) {
           Log.e(TAG, "IOException caught while reading stream", e);
       }
       return Enable;
   }
   private void writeSdStatus(int Slot, boolean Enable)
   {
       char [] sd_enable = {'1', '0', '0', '0'};
       char [] sd_disable = {'0', '0', '0', '0'};
       try
       {
           Writer sd_outputStream = null;
           File sd_file = null;
           switch(Slot)
           {
               case 1:
                   sd_file = new File("/sys/devices/platform/msm_sdcc.1/polling");
                   break;
               case 2:
                   sd_file = new File("/sys/devices/platform/msm_sdcc.2/polling");
                   break;
               case 3:
                   sd_file = new File("/sys/devices/platform/msm_sdcc.3/polling");
                   break;
               case 4:
                   sd_file = new File("/sys/devices/platform/msm_sdcc.4/polling");
                   break;
               default:
                   return;
           }
           if(sd_file.exists() != true)
           {
               Log.e(TAG, "WritePollingStatus: File does not exist " + sd_file.getAbsolutePath());
               return;
           }
           sd_outputStream = new BufferedWriter(new FileWriter(sd_file));

           if (Enable)
               sd_outputStream.write(sd_enable);
           else
               sd_outputStream.write(sd_disable);
           sd_outputStream.close();
       } catch (IOException e) {
           Log.e(TAG, "IOException caught while writing stream", e);
       }
   }
   private void updateSdPollingStatus()
   {
       boolean PollingEnabled = readSdStatus(sdSlot);
       SdPoll.setChecked(PollingEnabled);
       SdPoll.setSummary(PollingEnabled ? "SD Polling Enabled" : "SD Polling Disabled");
   }

   private void changeSdPollingStatus()
   {
       boolean PollingEnabled = SdPoll.isChecked();
       SdPoll.setSummary(PollingEnabled ? "SD Polling Enabled" : "SD Polling Disabled");
       writeSdStatus(sdSlot,PollingEnabled);
   }

   private void getSDSlot()
   {
       String target;
       target = SystemProperties.get("ro.board.platform");
       if (target.equals("msm7630_surf")) {
           sdSlot = 4;
       } else {
           sdSlot = 1;
       }

       // FLUID device detection
       //
       File hw_platformFile = null;
       BufferedReader hw_platformFileStream = null;
       String hw_platformName;
       try{
           hw_platformFile = new File("/sys/devices/soc0/hw_platform");

           if(hw_platformFile.exists()){
               hw_platformFileStream = new BufferedReader(new FileReader(hw_platformFile));
               hw_platformName = hw_platformFileStream.readLine();
               if(hw_platformName.equals("Fluid")){
                   sdSlot = 1;
               }
               hw_platformFileStream.close();
           }
           else{
                Log.e(TAG,
                        "getSDSlot: hw_platform File does not exist "
                                + hw_platformFile.getAbsolutePath());
           }

       }
       catch(IOException e){
           Log.e(TAG, "IOException caught while writing stream", e);
       }

   }

   private void updateHDMIAudioOutput()
   {
       int hdmiaudio_out_status=0;
       SharedPreferences qcSharedPref = getSharedPreferences("settings_preferences", 0);
       hdmiaudio_out_status = qcSharedPref.getInt(DISABLE_HDMIAUDIO_OUT, 0);

       if(hdmiaudio_out_status==1){
           DisableHdmiAudio.setChecked(true);
           DisableHdmiAudio.setSummary("HDMI audio disabled");
           AudioSystem.setDeviceConnectionState(AudioSystem.DEVICE_OUT_AUX_DIGITAL,
                                                AudioSystem.DEVICE_STATE_UNAVAILABLE,
                                                "hdmi_spkr");
       }
       else{
           DisableHdmiAudio.setChecked(false);
           DisableHdmiAudio.setSummary("HDMI audio enabled");
           AudioSystem.setDeviceConnectionState(AudioSystem.DEVICE_OUT_AUX_DIGITAL,
                                                AudioSystem.DEVICE_STATE_AVAILABLE,
                                                "hdmi_spkr");
       }
   }

   private void changeANCStatus()
   {
       boolean ANCEnabled = Anc.isChecked();
       mAudioManager.setParameters("anc_enabled="+(ANCEnabled ? "true" : "false"));
       Anc.setSummary(ANCEnabled ? "Enabled" : "Disabled");
       SharedPreferences qcSharedPref = getSharedPreferences("settings_preferences", 0);
       SharedPreferences.Editor editor = qcSharedPref.edit();
       editor.putInt(ANC, ANCEnabled ? 1 : 0);
       editor.commit();
   }

   private void updateANCStatus()
   {
       int ancstatus=0;
       SharedPreferences qcSharedPref = getSharedPreferences("settings_preferences", 0);
       ancstatus = qcSharedPref.getInt(ANC, 0);
       if(ancstatus==1){
           Log.e(TAG,"setting ancenabled to 1\n");
           Anc.setChecked(true);
           mAudioManager.setParameters("anc_enabled=true");
           Anc.setSummary("Enabled");
       }
       else{
           Log.e(TAG,"anc disabled");
           Anc.setChecked(false);
           mAudioManager.setParameters("anc_enabled=false");
           Anc.setSummary("Disabled");
       }
   }

   private void changeHdmiAudioStatus()
   {
        boolean HdmiAudioDisabled = DisableHdmiAudio.isChecked();
        SharedPreferences qcSharedPref = getSharedPreferences("settings_preferences", 0);
        SharedPreferences.Editor editor = qcSharedPref.edit();
        if (!HdmiAudioDisabled) {
            DisableHdmiAudio.setChecked(false);
            AudioSystem.setDeviceConnectionState(AudioSystem.DEVICE_OUT_AUX_DIGITAL,
                                                 AudioSystem.DEVICE_STATE_AVAILABLE,
                                                "hdmi_spkr");
            editor.putInt(DISABLE_HDMIAUDIO_OUT, 0);
        } else {
            DisableHdmiAudio.setChecked(true);
            AudioSystem.setDeviceConnectionState(AudioSystem.DEVICE_OUT_AUX_DIGITAL,
                                                 AudioSystem.DEVICE_STATE_UNAVAILABLE,
                                                "hdmi_spkr");
            editor.putInt(DISABLE_HDMIAUDIO_OUT, 1);
        }

        editor.commit();
        DisableHdmiAudio.setSummary(HdmiAudioDisabled ? "Disabled HDMI audio" : "Enabled HDMI Audio");
   }

   private boolean readSensorsStatus()
   {
       char Status;
       boolean Enable = false;
       try
       {
           Reader sensors_inputStream = null;
           File sensors_file = null;

           sensors_file = new File(SENSORS_SETTINGS);
           if(sensors_file.exists() != true)
           {
               Log.e(TAG, "ReadSensorsStatus: File does not exist " + sensors_file.getAbsolutePath());
               return false;
           }

           sensors_inputStream = new BufferedReader(new FileReader(sensors_file));

           if(sensors_inputStream.read() == 0x31)
               Enable = true;
           else
               Enable = false;
           sensors_inputStream.close();
       } catch (IOException e) {
           Log.e(TAG, "IOException caught while reading stream", e);
       }
       return Enable;
   }

   private boolean readDataMonitorStatus()
   {
       boolean enabled = false;

       try
       {
          // Read value from secure settings d/b
          int netStatsEnabled = Settings.Secure.getInt(getContentResolver(), "NETSTATS_ENABLED", 0 );
          enabled = (netStatsEnabled == 1);

       } catch (Exception e) {
           Log.e(TAG, "Exception caught while reading NETSTATS_ENABLED property ", e);
       }

       Log.d(TAG, "readDataMonitorStatus = " + enabled );
       return enabled;

   }


   private void writeDataMonitorStatus(boolean enable)
   {
       try
       {
           Log.d(TAG, (enable ? "Enabling" : "Disabling") + " Data Monitor Status");
           // Write new value to secure settings d/b
           Settings.Secure.putInt(getContentResolver(), "NETSTATS_ENABLED", enable ? 1 : 0 );

       } catch (Exception e) {
           Log.e(TAG, "Exception caught while reading NETSTATS_ENABLED property ", e);
       }
   }

   private boolean readTouchInjectionStatus()
   {
       boolean enabled = false;

       try
       {
          // Read system setting
          int reading = Settings.System.getInt(getContentResolver(), "GESTURES_TOUCH_INJECTION", 0 );
          enabled = (reading == 1);

       } catch (Exception e) {
           Log.e(TAG, "Exception caught while reading property", e);
       }

       Log.d(TAG, "readTouchInjectionStatus = " + enabled );
       return enabled;
   }


   private void writeTouchInjectionStatus(boolean enable)
   {
       try
       {
           Log.d(TAG, (enable ? "Enabling" : "Disabling") + " Gestures Touch Injection");

           // Add system setting
           Settings.System.putInt(getContentResolver(), "GESTURES_TOUCH_INJECTION", enable ? 1 : 0 );

           // Broadcast intent to clients
           broadcastTouchInjectionIntent();

       } catch (Exception e) {
           Log.e(TAG, "Exception caught while writing property", e);
       }
   }

   private void broadcastTouchInjectionIntent()
   {
        startService(new Intent("GESTURE_TOUCH_INJECTION_SERVICE"));
   }

   private void writeSensorsStatus(boolean Enable)
   {
       try
       {
           Writer sensors_outputStream = null;
           File sensors_file = null;

           sensors_file = new File(SENSORS_SETTINGS);
           if(sensors_file.exists() != true)
           {
               Log.e(TAG, "WriteSensorsStatus: File does not exist " + sensors_file.getAbsolutePath());
               return;
           }
           sensors_outputStream = new BufferedWriter(new FileWriter(sensors_file));

           if (Enable)
               sensors_outputStream.write('1');
           else
               sensors_outputStream.write('0');
           sensors_outputStream.close();
       } catch (IOException e) {
           Log.e(TAG, "IOException caught while writing stream", e);
       }
   }

   private void updateSensorsStatus()
   {
       boolean SensorsEnabled = readSensorsStatus();
       Sensors.setChecked(SensorsEnabled);
       Sensors.setSummary(SensorsEnabled ? "Enabled" : "Disabled");
   }

   private void updateGesturesTouchInjectionStatus()
   {
       boolean TouchInjectionEnabled = readTouchInjectionStatus();
       Gestures_Touch_Injection.setChecked(TouchInjectionEnabled);
       Gestures_Touch_Injection.setSummary(TouchInjectionEnabled ? "Enabled" : "Disabled");

       if( TouchInjectionEnabled )
       {
           broadcastTouchInjectionIntent();
       }
   }

   private void updateDataMonitorStatus()
   {
       boolean DataMonitorEnabled = readDataMonitorStatus();
       Data_Monitor_Status.setChecked(DataMonitorEnabled);
       Data_Monitor_Status.setSummary(DataMonitorEnabled ? "Enabled" : "Disabled");

   }

   private void changeDataMonitorStatus()
   {
       boolean DataMonitorEnabled = Data_Monitor_Status.isChecked();
       Data_Monitor_Status.setSummary(DataMonitorEnabled ? "Enabled" : "Disabled");
       writeDataMonitorStatus(DataMonitorEnabled);

   }

   private void changeSensorsStatus()
   {
       boolean SensorsEnabled = Sensors.isChecked();
       Sensors.setSummary(SensorsEnabled ? "Enabled" : "Disabled");
       writeSensorsStatus(SensorsEnabled);

   }

   private void changeTouchInjectionStatus()
   {
       boolean TouchInjectionEnabled = Gestures_Touch_Injection.isChecked();
       Gestures_Touch_Injection.setSummary(TouchInjectionEnabled ? "Enabled" : "Disabled");
       writeTouchInjectionStatus(TouchInjectionEnabled);

   }

  private void changeDIAGSettings()
  {
       boolean DiagEnabled = DiagLogging.isChecked();

       if(DiagEnabled)
       {
           String prog = "/system/bin/diag_mdlog";    /* command to run */
           File f = new File(prog);
           diagBinExists = f.exists();

           if(!diagBinExists)
           {
               DiagLogging.setSummary("Feature not available on this build");
               Log.e(DIAG_LOG, "Feature not available on this build.\n");
               return;
           }

           /* Starting the diag_mdlog service to perform On-Device logging */
           Log.e(DIAG_LOG, "\n... STARTING MDLOG\n");
           SystemService.start("diag_mdlog_start");
       }
       else
       {
           if(diagBinExists)
           {
               /* Start the diag_mdlog service that stops On-Device logging */
               Log.e(DIAG_LOG, "\n... KILLING MDLOG\n");
               SystemService.start("diag_mdlog_stop");
           }
       }
   }


   public void writeProductId(String product_id)
   {
       try {
           File composition_file =
                new File("/sys/module/g_android/parameters/product_id");
           Writer outputStream = null;
           outputStream = new BufferedWriter(new FileWriter(composition_file));
           outputStream.write(product_id);
           outputStream.close();
           SystemProperties.set("persist.sys.usbcomposition.id", product_id);
       } catch (IOException e) {
           Log.e(TAG, "IOException caught while writting stream", e);
       }
   }

   private void updateUsbComposition() {
      String UsbCompositionValue = UsbComposition.getValue();
      writeProductId(UsbCompositionValue);
   }

   public void writeUsbOtgMode(String mode)
   {
       try {
           Log.d(TAG, "writeUsbOtgMode=" + mode);
           File mode_file =
                new File(USB_OTG_FILE);
           Writer outputStream = null;
           outputStream = new BufferedWriter(new FileWriter(mode_file));
           outputStream.write(mode);
           outputStream.close();
       } catch (IOException e) {
           Log.e(TAG, "IOException caught while writing usb mode stream", e);
       }
   }
   private void updateUsbOtgMode() {
      String otgMode = UsbOtgMode.getValue();
      Log.d(TAG, "updateUsbOtgMode=" + otgMode);
      writeUsbOtgMode(otgMode);
   }

   public void onSharedPreferenceChanged(SharedPreferences sharedPreferences,
         String key) {
      if (key.equals(DIM_SCREEN)) {
         updateDimScreen();
      }
      else if (key.equals(AUDIO_DEFAULT_DEVICE)) {
        updateAudioDeviceSpdif();
      }
      else if (key.equals(STAY_ON_PLUGGED)) {
         updateStayOn();
      }
      else if (key.equals(AUTO_ANSWER_VOICE_CALL)) {
         updateAutoAnswerVoiceCall();
      }
      else if (key.equals(USB_MASS_STORAGE)) {
         umsValue = sharedPreferences.getBoolean(key, true);
         updateUsbMassStorageStatus();
      }
      else if (key.equals(USB_REMOTE_WAKEUP)) {
          updateUsbRemoteWakeupStatus();
      }
      else if (key.equals(SD_POLLING_ENABLED)) {
          changeSdPollingStatus();
      }
      else if (key.equals(USB_COMPOSITION)) {
         updateUsbComposition();
      }
      else if (key.equals(USB_OTG_MODE)) {
         updateUsbOtgMode();
      }
      else if (key.equals(GO_DORMANT)) {
         mQcRilHook.qcRilGoDormant(GoDormant.getText());
      }
      else if (key.equals(DISABLE_HDMIAUDIO_OUT)) {
         changeHdmiAudioStatus();
      }
      else if (key.equals(ANC)) {
         changeANCStatus();
      }
      else if (key.equals(SENSORS)) {
         changeSensorsStatus();
      }
      else if (key.equals(DIAG_LOG)) {
         changeDIAGSettings();
      }
      else if (key.equals(GESTURES_TOUCH_INJECTION)) {
         changeTouchInjectionStatus();
      }
      else if (key.equals(DATA_MONITOR)) {
         changeDataMonitorStatus();
      }
   }

   @Override
   protected void onDestroy() {
      super.onDestroy();
      if (DunConnection != null) {
          unbindService(DunConnection);
          DunConnection = null;
      }
      getPreferenceScreen().getSharedPreferences().unregisterOnSharedPreferenceChangeListener(this);
      mQcRilHook.dispose();
   }

   /* Service connection inteface with our binded service */

   private ServiceConnection DunConnection = new ServiceConnection() {
      public void onServiceConnected(ComponentName name, IBinder service) {
         DunService = com.qualcomm.qualcommsettings.IDun.Stub.asInterface(service);
      }
      public void onServiceDisconnected(ComponentName name) {
         DunService=null;
      }
   };
}
