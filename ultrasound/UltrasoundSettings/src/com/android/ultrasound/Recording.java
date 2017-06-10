/******************************************************************************
 * @file    RecordingAppActivity.java
 * @brief   Presents the welcome screen with the opportunity to fill in
 * the use's details and start the recording
 *
 *
 * ---------------------------------------------------------------------------
 *  Copyright (c) 2012-2013 Qualcomm Technologies, Inc.
 *  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
 *  ---------------------------------------------------------------------------
 *  ******************************************************************************/

package com.android.ultrasound;

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.text.SimpleDateFormat;
import java.util.Calendar;
import java.util.*;
import java.io.*;
import android.app.Activity;
import android.content.Intent;
import android.graphics.Color;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemSelectedListener;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.Spinner;
import android.widget.TextView;
import android.widget.Toast;

public class Recording extends Activity
{
  /* MEMBERS */
  // Elements
  private Button mBeginButton;
  private EditText mNameEditText;
  private EditText mLocationEditText;
  private EditText mCommentsEditText;
  private EditText mCfgFileEditText;
  private EditText mRecordLocationEditText;
  private TextView mUserNoticeTextview;
  private Spinner mTypeSpinner;

  // Current daemon that would be recording: deafult is Gesture daemon
  private int mCurrRecordingDaemon = GESTURE;

  // Files purposes
  private static final String GESTURE_DIRECTORY = "/data/usf/tester/gestures_database";
  private static final String HOVERING_DIRECTORY = "/data/usf/tester/hovering_database";
  private static final String PROXIMITY_DIRECTORY = "/data/usf/tester/proximity_database";
  private static final String TESTER_CFG_LINK_FILE_LOCATION = "/data/usf/tester/usf_tester.cfg";
  private static final String PROX_CFG_LINK_FILE_LOCATION = "/data/usf/proximity/usf_proximity.cfg";
  private static final String TEXT_FILENAME = "/info.txt";
  // Inner file names
  private static final String GESTURE_STARTING_DIR = "/gesture_";
  private static final String HOVERING_STARTING_DIR = "/hovering_";
  private static final String PROXIMITY_STARTING_DIR = "/proximity_";
  private static final String DATE_FORMAT = "yyyy-MM-dd_HH-mm-ss/";
  // Welcome screen
  private static final String NAME_INFORMATION_REQUEST = "Name: ";
  private static final String LOCATION_INFORMATION_REQUEST = "Location: ";
  private static final String COMMENT_INFORMATION_REQUEST = "Comments: ";
  private static final String EXTRA_KEY_DIR_NAME = "directory_name";
  private static final String EXTRA_KEY_FRAME_PARAM = "frame_file";
  private static final String FRAME_FILE_PARAM = "usf_frame_file";
  private static final String APPEND_TIMESTAMP_PARAM = "usf_append_timestamp";
  private static final String TESTER_DEFAULT_FRAME_DIR = "/data/usf/tester/rec/";
  private static final String PROX_DEFAULT_FRAME_DIR = "/data/usf/proximity/rec/";
  // Integers to be used in arrays. Enums are not used here because they cannot be used as indexes in arrays
  private static final int GESTURE = 0;
  private static final int HOVERING = 1;
  private static final int PROXIMITY = 2;
  private static final int NUM_OF_DAEMONS = 3;

  private static final DaemonData[] recordingDaemons = new DaemonData[NUM_OF_DAEMONS];
  // Classes
  class DaemonData
  {
    String cfgLinkFile;
    String daemonDirectory;
    String directoryFullName;
    String frameDirectory;
  }

  /*
   * ==================================================================
   * FUNCTION: onCreate()
   * ==================================================================
   */
  /**
   * onCreate() function creates the UI and captures the common elements
   */
  @Override
  public void onCreate(Bundle savedInstanceState)
  {
    super.onCreate(savedInstanceState);

    setContentView(R.layout.recording);

    // Capture elements
    mNameEditText = (EditText) findViewById(R.id.NameEditText);
    mLocationEditText = (EditText) findViewById(R.id.LocationEditText);
    mCommentsEditText = (EditText) findViewById(R.id.CommentsEditText);
    mCfgFileEditText = (EditText) findViewById(R.id.CfgFileEditText);
    mRecordLocationEditText = (EditText) findViewById(R.id.RecordLocationEditText);
    mUserNoticeTextview = (TextView) findViewById(R.id.UserNoticeTextView);
    mBeginButton = (Button) findViewById(R.id.BeginButton);

    if (null != mUserNoticeTextview)
    {
      mUserNoticeTextview.setTextColor(Color.RED);
    }

    // Setting the spinner
    mTypeSpinner = (Spinner) findViewById(R.id.TypeSpinner);

    if (null != mTypeSpinner)
    {
      ArrayAdapter<CharSequence> typeAdapter = ArrayAdapter.createFromResource(
          this, R.array.test_type_array, android.R.layout.simple_spinner_item);
      typeAdapter
          .setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
      mTypeSpinner.setAdapter(typeAdapter);
      mTypeSpinner.setOnItemSelectedListener(new MyOnItemSelectedListener());
    }

    // Gesture
    recordingDaemons[GESTURE] = new DaemonData();
    recordingDaemons[GESTURE].cfgLinkFile = TESTER_CFG_LINK_FILE_LOCATION;
    recordingDaemons[GESTURE].daemonDirectory = GESTURE_DIRECTORY;
    recordingDaemons[GESTURE].directoryFullName = GESTURE_DIRECTORY + GESTURE_STARTING_DIR;
    recordingDaemons[GESTURE].frameDirectory = TESTER_DEFAULT_FRAME_DIR;
    // Hovering
    recordingDaemons[HOVERING] = new DaemonData();
    recordingDaemons[HOVERING].cfgLinkFile = TESTER_CFG_LINK_FILE_LOCATION;
    recordingDaemons[HOVERING].daemonDirectory = HOVERING_DIRECTORY;
    recordingDaemons[HOVERING].directoryFullName = HOVERING_DIRECTORY + HOVERING_STARTING_DIR;
    recordingDaemons[HOVERING].frameDirectory = TESTER_DEFAULT_FRAME_DIR;
    // Proximity
    recordingDaemons[PROXIMITY] = new DaemonData();
    recordingDaemons[PROXIMITY].cfgLinkFile = PROX_CFG_LINK_FILE_LOCATION;
    recordingDaemons[PROXIMITY].daemonDirectory = PROXIMITY_DIRECTORY;
    recordingDaemons[PROXIMITY].directoryFullName = PROXIMITY_DIRECTORY + PROXIMITY_STARTING_DIR;
    recordingDaemons[PROXIMITY].frameDirectory = PROX_DEFAULT_FRAME_DIR;
  }

  /*
   * ==================================================================
   * FUNCTION: onStart()
   * ==================================================================
   */
  /**
   * onStart() function is always called after onCreate. This function sets the
   * begin button that creates the relevant directories, the info file, and
   * starts the relevant Activity.
   */
  @Override
  protected void onStart()
  {
    super.onStart();

    String linkCfg = (recordingDaemons[mCurrRecordingDaemon]).cfgLinkFile;

    // getting the cfg in the link cfg file
    String cfgFile = UltrasoundUtil.getCfgFileLocationFromLinkFile(linkCfg);
    // displaying the cfg found, and making it immutable
    mCfgFileEditText.setText(cfgFile);
    mCfgFileEditText.setEnabled(false);
    // Setting recording location
    String recordingLocation = (recordingDaemons[mCurrRecordingDaemon]).daemonDirectory;
    mRecordLocationEditText.setText(recordingLocation);
    mRecordLocationEditText.setEnabled(false);

    mBeginButton.setOnClickListener(new View.OnClickListener()
    {
      public void onClick(View v)
      {
        if (getCfgParamValue(APPEND_TIMESTAMP_PARAM).equals("1"))
        {
          Toast.makeText(getApplicationContext(),
                         "Please set usf_append_timestamp to 0 in cfg file",
                         Toast.LENGTH_SHORT).show();
          return;
        }

        // Getting current time and date
        Calendar calendar = Calendar.getInstance();
        SimpleDateFormat dateFormat = new SimpleDateFormat(DATE_FORMAT);

        // Check if the main directory exists else create it
        String directoryFullName = (recordingDaemons[mCurrRecordingDaemon]).directoryFullName;
        directoryFullName += dateFormat.format(calendar.getTime());

        File mainDirectory = new File(directoryFullName);
        if (!mainDirectory.exists())
        {
          if (!mainDirectory.mkdirs())
          {
            Log.e(this.toString(),
                  "Problem creating directory");
          }
        }

        generateFile(mainDirectory);

        // Create intent for gesture recording activity
        Intent intent;
        if (GESTURE == mCurrRecordingDaemon)
        {
          intent = new Intent(Recording.this,
                              GestureRecording.class);
        }
        else if (PROXIMITY == mCurrRecordingDaemon)
        {
          intent = new Intent(Recording.this,
                              ProximityRecording.class);
        }
        else
        {
          Log.e(this.toString(),
                "Recording Daemon is not yet supported");
          return;
        }
        // Create a bundle
        Bundle currBundle = new Bundle(); // Put directory in the bundle
        currBundle.putString(EXTRA_KEY_DIR_NAME, mainDirectory.toString());
        // Get frame file name
        String frameFileName = getCfgParamValue(FRAME_FILE_PARAM);
        if (null == frameFileName)
        {
          Log.e(this.toString(),
                "paramerter: " + FRAME_FILE_PARAM + " was not found");
          return;
        }
        // Put frame file path in bundle
        currBundle.putString(EXTRA_KEY_FRAME_PARAM, getFrameFilePath(frameFileName));
        // Put bundle in the intent
        intent.putExtras(currBundle);

        // Launch intent
        startActivity(intent);
      }
    });
  }

  /*===========================================================================
  INNER_CLASS:  getFrameFilePath
  ===========================================================================*/
  /**
   * This function returns the full path of the given frame file
   * value.
   * @param frameFileName the frame file value
   *
   * @return String full frame file path
   */
  private String getFrameFilePath(String frameFileName)
  {
    if(0 == frameFileName.indexOf("/"))
    { // Then frame file name starts with /
      return frameFileName;
    }
    else
    {
      String defaultFrameDir = (recordingDaemons[mCurrRecordingDaemon]).frameDirectory;
      return (defaultFrameDir + frameFileName);
    }
  }

  /*===========================================================================
  FUNCTION:  getCfgParamValue
  ===========================================================================*/
  /**
   * This function reads the current cfg file and looks up the
   * value of the given parameter
   *
   * @param paramToLookup parameter to return its value
   *
   * @return String the value of the given parameter
   *                or null when parameter is not found.
   */
  private String getCfgParamValue (String paramToLookup)
  {
    Log.d(this.toString(), " in getCfgParamValue()");
    try
    {
      String linkCfg = (recordingDaemons[mCurrRecordingDaemon]).cfgLinkFile;
      String cfgFilePath = UltrasoundUtil.getCfgFileLocationFromLinkFile(linkCfg);
      if (null == cfgFilePath)
      {
        return null;
      }
      FileInputStream fstream = new FileInputStream(cfgFilePath);
      DataInputStream in = new DataInputStream(fstream);
      BufferedReader br = new BufferedReader(new InputStreamReader(in));

      String strLine;
      String paramName;
      String paramValue;
      int i = 0;

      while (null != (strLine = br.readLine()))
      {
        // This line is a comment
        if (UltrasoundUtil.isCommentLine(strLine))
        {
          continue;
        }

        StringTokenizer st = new StringTokenizer(strLine);
        // This line is param and value
        if (st.countTokens() == 2)
        {
          paramName = st.nextToken();

          if (paramName.equals(paramToLookup))
          {
            paramValue = st.nextToken();
            in.close();
            return paramValue.substring(1, paramValue.length()-1);
          }
        }

      }
      in.close();
    }
    catch (Exception e)
    {
      Log.e(this.toString(), "Error: " + e.getMessage());
    }
    Log.e(this.toString(), paramToLookup + "parameter not found in cfg");
    return null;
  }

  /*
   * ==================================================================
   * FUNCTION: generateFile()
   * ==================================================================
   */
  /**
   * generateFile() function generates the info.txt file with the text inserted
   * by the user in the right directory
   */
  private void generateFile(File mainDirectory)
  {
    // Getting the text from the boxes
    String commentText = mCommentsEditText.getText().toString();
    String locationText = mLocationEditText.getText().toString();
    String nameText = mNameEditText.getText().toString();

    try {
      // Creating the file
      FileWriter frameFile;
      frameFile = new FileWriter(mainDirectory + TEXT_FILENAME);

      BufferedWriter out = new BufferedWriter(frameFile);

      out.write(NAME_INFORMATION_REQUEST + nameText);
      out.newLine();
      out.write(LOCATION_INFORMATION_REQUEST + locationText);
      out.newLine();
      out.write(COMMENT_INFORMATION_REQUEST + commentText);
      out.newLine();
      out.close();
    }
    catch (IOException e)
    {
      e.printStackTrace();
    }
  }

  /*
   * ==================================================================
   * INNER_CLASS: MyOnItemSelectedListener
   * ==================================================================
   */
  /**
   * MyOnItemSelectedListener class is used to determine which action to perform
   * when the user picks an item on the spinner element. In this case, it sets
   * the mCurrRecordingDaemon parameter.
   */
  public class MyOnItemSelectedListener implements OnItemSelectedListener
  {
    public void onItemSelected(AdapterView<?> parent, View view, int pos,
        long id) {
      mCurrRecordingDaemon = pos;
      Log.e(this.toString(),
            "Current pos:" + pos);
      // Setting destination recording directory
      String daemonDirectory = (recordingDaemons[mCurrRecordingDaemon]).daemonDirectory;
      mRecordLocationEditText.setText(daemonDirectory);

      // Settings currently used cfg file
      String linkCfg = (recordingDaemons[mCurrRecordingDaemon]).cfgLinkFile;
      // Getting the cfg from the link cfg file
      String cfgFile = UltrasoundUtil.getCfgFileLocationFromLinkFile(linkCfg);
      // Setting the cfg found
      mCfgFileEditText.setText(cfgFile);
    }

    public void onNothingSelected(AdapterView<?> arg0)
    {
      // Do nothing
    }
  }
}