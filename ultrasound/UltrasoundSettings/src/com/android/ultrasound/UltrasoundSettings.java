/******************************************************************************
 * @file    UltrasoundSettings.java
 * @brief   Provides 4 options of Ultrasound UI:
 *          Tester UI, Sync Gesture UI, Gesture UI, P2P UI, Digital Pen UI.
 *
 *
 * ---------------------------------------------------------------------------
 *  Copyright (c) 2011-2013 Qualcomm Technologies, Inc.
 *  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
 *  ---------------------------------------------------------------------------
 *  ******************************************************************************/
package com.android.ultrasound;

import com.android.ultrasound.DigitalPen;
import com.android.ultrasound.Gesture;
import com.android.ultrasound.SyncGesture;
import com.android.ultrasound.Tester;
import com.android.ultrasound.P2P;
import com.android.ultrasound.Hovering;
import com.android.ultrasound.Proximity;
import android.util.Log;
import android.content.Intent;
import android.app.Activity;
import android.os.Bundle;
import android.view.View;
import android.view.View.OnClickListener;
import android.preference.Preference;
import android.preference.PreferenceActivity;
import android.os.SystemProperties;
import java.util.*;
import java.io.*;

public class UltrasoundSettings extends PreferenceActivity implements
  Preference.OnPreferenceClickListener
{
  private static final String FORM_FACTOR_FILE = "/data/usf/form_factor.cfg";
  private static final String NUM_OF_MICS_PROP = "num_of_mics";
  private static final String NUM_OF_SPEAKS_PROP = "num_of_speakers";
  private final String tag = "UltrasoundSettings";
  public static final String TESTER_SETTINGS = "tester_settings";
  public static final String DIG_PEN_SETTINGS = "digital_pen_settings";
  public static final String GESTURE_SETTINGS = "gesture_settings";
  public static final String SYNC_GESTURE_SETTINGS = "sync_gesture_settings";
  public static final String HOVERING_SETTINGS = "hovering_settings";
  public static final String P2P_SETTINGS = "p2p_settings";
  public static final String RECORDING_SETTINGS = "recording_settings";
  public static final String PROXIMITY_SETTINGS = "proximity_settings";
  public static final String VERSION_SETTINGS = "version_settings";
  public static final String VERSION_FILE = "/data/usf/version.txt";

  private static final int TESTER_ACTIVITY = 1;
  private static final int DIGITAL_PEN_ACTIVITY = 2;
  private static final int GESTURE_ACTIVITY = 3;
  private static final int HOVERING_ACTIVITY = 4;
  private static final int P2P_ACTIVITY = 5;
  private static final int RECORDING_ACTIVITY = 6;
  private static final int PROXIMITY_ACTIVITY = 7;
  private static final int SYNC_GESTURE_ACTIVITY = 8;

  private Preference TesterSettings;
  private Preference DigitalPenSettings;
  private Preference GestureSettings;
  private Preference SyncGestureSettings;
  private Preference HoveringSettings;
  private Preference P2PSettings;
  private Preference RecordingSettings;
  private Preference ProximitySettings;
  private Preference VersionSettings;

  /*===========================================================================
  FUNCTION:  onCreate
  ===========================================================================*/
  /**
    onCreate() function creates the UI.
  */
  @Override
  public void onCreate(Bundle icicle)
  {
    super.onCreate(icicle);
    addPreferencesFromResource(R.xml.settings_preferences);

    TesterSettings = (Preference) findPreference(TESTER_SETTINGS);
    DigitalPenSettings = (Preference) findPreference(DIG_PEN_SETTINGS);
    GestureSettings = (Preference) findPreference(GESTURE_SETTINGS);
    SyncGestureSettings = (Preference) findPreference(SYNC_GESTURE_SETTINGS);
    HoveringSettings = (Preference) findPreference(HOVERING_SETTINGS);
    P2PSettings = (Preference) findPreference(P2P_SETTINGS);
    RecordingSettings = (Preference) findPreference(RECORDING_SETTINGS);
    ProximitySettings = (Preference) findPreference(PROXIMITY_SETTINGS);
    VersionSettings = (Preference) findPreference(VERSION_SETTINGS);

    initializeVersionButton();

    // If ultrasound feature does not enabled we prevent
    // the ultrasound options.
    if (isUltrasoundEnabled())
    {
      if (null != TesterSettings)
      {
        TesterSettings.setOnPreferenceClickListener(this);
        TesterSettings.setEnabled(true);
      }
      if (null != DigitalPenSettings)
      {
        DigitalPenSettings.setOnPreferenceClickListener(this);
        DigitalPenSettings.setEnabled(true);
      }
      if (null != GestureSettings)
      {
        GestureSettings.setOnPreferenceClickListener(this);
        GestureSettings.setEnabled(true);
      }
      if (null != SyncGestureSettings)
      {
        SyncGestureSettings.setOnPreferenceClickListener(this);
        SyncGestureSettings.setEnabled(true);
      }
      if (null != HoveringSettings)
      {
        HoveringSettings.setOnPreferenceClickListener(this);
        HoveringSettings.setEnabled(true);
      }
      if (null != P2PSettings)
      {
        P2PSettings.setOnPreferenceClickListener(this);
        P2PSettings.setEnabled(true);
      }
      if (null != RecordingSettings)
      {
        RecordingSettings.setOnPreferenceClickListener(this);
        RecordingSettings.setEnabled(true);
      }
      if (null != ProximitySettings)
      {
        ProximitySettings.setOnPreferenceClickListener(this);
        ProximitySettings.setEnabled(true);
      }
    }
    else
    {
      if (null != TesterSettings) {
        TesterSettings.setEnabled(false);
      }
      if (null != DigitalPenSettings) {
        DigitalPenSettings.setEnabled(false);
      }
      if (null != GestureSettings) {
        GestureSettings.setEnabled(false);
      }
      if (null != SyncGestureSettings) {
        SyncGestureSettings.setEnabled(false);
      }
      if (null != HoveringSettings) {
        HoveringSettings.setEnabled(false);
      }
      if (null != P2PSettings) {
        P2PSettings.setEnabled(false);
      }
      if (null != RecordingSettings) {
        RecordingSettings.setEnabled(false);
      }
      if (null != ProximitySettings) {
        ProximitySettings.setEnabled(false);
      }
    }
  }

  /*===========================================================================
  FUNCTION:  onPreferenceClick
  ===========================================================================*/
  /**
    onPreferenceClick() function calls to the wanted activity.
  */
  public boolean onPreferenceClick(Preference preference) {
    if (preference.equals(TesterSettings))
    {
      Intent intent = new Intent(this, Tester.class);
      startActivityForResult(intent, TESTER_ACTIVITY);
    }
    else if (preference.equals(DigitalPenSettings))
    {
      Intent intent = new Intent(this, DigitalPen.class);
      startActivityForResult(intent, DIGITAL_PEN_ACTIVITY);
    }
    else if (preference.equals(GestureSettings))
    {
      Intent intent = new Intent(this, Gesture.class);
      startActivityForResult(intent, GESTURE_ACTIVITY);
    }
    else if (preference.equals(SyncGestureSettings))
    {
      Intent intent = new Intent(this, SyncGesture.class);
      startActivityForResult(intent, SYNC_GESTURE_ACTIVITY);
    }
    else if (preference.equals(HoveringSettings))
    {
      Intent intent = new Intent(this, Hovering.class);
      startActivityForResult(intent, HOVERING_ACTIVITY);
    }
    else if (preference.equals(P2PSettings))
    {
      Intent intent = new Intent(this, P2P.class);
      startActivityForResult(intent, P2P_ACTIVITY);
    }
    else if (preference.equals(RecordingSettings))
    {
      Intent intent = new Intent(this, Recording.class);
      startActivityForResult(intent, RECORDING_ACTIVITY);
    }
    else if (preference.equals(ProximitySettings))
    {
      Intent intent = new Intent(this, Proximity.class);
      startActivityForResult(intent, PROXIMITY_ACTIVITY);
    }
    return true;
  }

  /*===========================================================================
  FUNCTION:  isUltrasoundEnabled
  ===========================================================================*/
  /**
    isUltrasoundEnabled() reads the form factor file and if number
    of mics and number of speakers is 0 (no mics and no speakers)
    it return false, else it returns true.
  */
  public boolean isUltrasoundEnabled()
  {
    boolean mics = true;
    boolean speaks = true;
    String strLine;
    String paramName;
    String paramValue;

    try
    {
      FileInputStream fstream = new FileInputStream(FORM_FACTOR_FILE);
      DataInputStream in = new DataInputStream(fstream);
      BufferedReader br = new BufferedReader(new InputStreamReader(in));
      while (null != (strLine = br.readLine()))
      {
        StringTokenizer st = new StringTokenizer(strLine);
        if (st.countTokens() == 2)
        {
          paramName = st.nextToken();
          paramValue = st.nextToken();
          paramValue = paramValue.substring(1, paramValue.length()-1);
          if (paramName.equals(NUM_OF_MICS_PROP))
          {
            if (paramValue.equals("0")) {
              mics = false;
            }
          }
          if (paramName.equals(NUM_OF_SPEAKS_PROP))
          {
            if (paramValue.equals("0")) {
              speaks = false;
            }
          }
        }
      }
      in.close();
    }
    catch (Exception e)
    {
      Log.e(getTag(), "Error: " + e.getMessage());
      return false;
    }

    return (mics||speaks);
  }

  /*===========================================================================
  FUNCTION:  getTag
  ===========================================================================*/
  /**
    getTag() function returns this activity tag.
  */
  protected String getTag() {
    return tag;
  }

  /*===========================================================================
  FUNCTION:  getFrameworkVersion
  ===========================================================================*/
  /**
    getFrameworkVersion() function returns the framework version
    on the target.
  */
  protected String getFrameworkVersion() {
    String strLine = "";
    try
    {
      FileInputStream fstream = new FileInputStream(VERSION_FILE);
      DataInputStream in = new DataInputStream(fstream);
      BufferedReader br = new BufferedReader(new InputStreamReader(in));
      strLine = br.readLine();
      in.close();
    }
    catch (Exception e)
    {
      Log.e(getTag(), "Error: " + e.getMessage());
    }
    return strLine;
  }

  /*===========================================================================
  FUNCTION:  initializeVersionButton
  ===========================================================================*/
  /**
    initializeVersionButton() sets the version of the current
    framework as the summary of the Version button.
  */
  private void initializeVersionButton() {
    VersionSettings.setEnabled(false); // Grey out the version 'button'
    VersionSettings.setSummary(getFrameworkVersion());
  }

}
