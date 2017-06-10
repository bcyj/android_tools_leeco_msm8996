/******************************************************************************
 * @file    BasicDaemonRecording.java
 * @brief   Holds basic functions to be used by recording activities.
 * ---------------------------------------------------------------------------
 *  Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
 *  Qualcomm Technologies Proprietary and Confidential.
 *  ---------------------------------------------------------------------------
 *  ***************************************************************************/

package com.android.ultrasound;

import java.io.File;
import android.os.SystemService;
import android.os.SystemProperties;
import android.app.Activity;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.util.Log;
import android.widget.TextView;
import java.util.Calendar;

public class BasicDaemonRecording extends Activity
{
  private static final int NUM_OF_COUNTDOWN = 3;
  private static final String GREETING = "Thank you";
  protected static final int GREETING_NUM = 2;
  protected static final int COUNTDOWN_NUM = 1;
  protected static final int INSTRUCTION_NUM = 0;

  // Daemon
  private final String THIS_DAEMON = "usf_tester";
  private static final String EXTRA_KEY_DIR_NAME = "directory_name";
  private static final String EXTRA_KEY_FRAME_PARAM = "frame_file";
  // Elements
  protected TextView mTextView;
  // Handles the writing of countdowns and instructions on screen
  protected SessionThread mSessionThread;
  // The directory where current recording would be stored
  private String mDirectoryToMoveTo;
  // The directory where the original record is stored
  protected String mRecOriginalDir;
  /*
   * mHandler - This callback is called whenever a msg is sent to the handler.
   * This handler is used to write text on the screen.
   */
  final Handler mHandler = new Handler()
  {
    /*==================================================================
    FUNCTION: handleMessage
    ==================================================================*/
    public void handleMessage(Message msg)
    {
      switch (msg.what)
      {
      case GREETING_NUM:
      {
        String message = GREETING + "\n\n" +
                         "Recordings are stored in: " + mDirectoryToMoveTo;
        mTextView.setText(message);
        break;
      }
      case COUNTDOWN_NUM:
        mTextView.setText("" + msg.arg1);
        break;
      case INSTRUCTION_NUM:
        for (Instruction currInstruction : Instruction.values())
        {
          if (msg.arg1 == currInstruction.num)
          {
            mTextView.setText(currInstruction.instruction);
          }
        }
        break;
      default:
        Log.e(this.toString(),
              "Invalid handler message");
      }
    }
  };

  /*===========================================================================
  ENUM:  SleepTime
  ===========================================================================*/
  /**
   * SleepTime class contains the times a thread would sleep in different
   * situations
   */
  protected enum SleepTime
  {
    COUNTDOWN(1000),
    MOVE_FILE(1000),
    INSTRUCTION(1000),
    WAIT_DAEMON(2000),
    WRITE_FILE(500),
    WAIT_PROXIMITY(6000);

    protected final int value;

    private SleepTime(int value)
    {
      this.value = value;
    }
  }

 /*==================================================================
   ENUM_INNER_CLASS: Instruction
   ==================================================================*/
  /**
   * Instruction class is used to save all the instructions shown to the user
   */
  protected enum Instruction
  {
    RIGHT("swipe Right", "frame_right_rec", 0),
    LEFT("swipe left","frame_left_rec", 1),
    SELECT("Select", "frame_select_rec", 2),
    APPROACH("approach screen", "frame_approach_rec", 3);

    protected final String instruction;
    protected final String fileName;
    protected final int num;

    Instruction(String instruction,
                String fileName,
                int num)
    {
      this.instruction = instruction;
      this.fileName = fileName;
      this.num = num;
    }
  }


  /*===========================================================================
  FUNCTION:  onCreate
  ===========================================================================*/
  /**
   * onCreate() function creates the UI and captures the common
   * elements.
   */
  @Override
  protected void onCreate(Bundle savedInstanceState)
  {
    Log.d(this.toString(),
          "BasicDaemonRecording: onCreate()");
    super.onCreate(savedInstanceState);

    // Getting the main file name from RecordingAppActivity
    Bundle bundle = getIntent().getExtras();
    mDirectoryToMoveTo = bundle.getString(EXTRA_KEY_DIR_NAME);
    mRecOriginalDir = bundle.getString(EXTRA_KEY_FRAME_PARAM);
    Log.d(this.toString(),
          "current mRecOriginalDir is:" + mRecOriginalDir + "and dir:" + mDirectoryToMoveTo);
  }

  /*===========================================================================
  FUNCTION:  getCurrentTime
  ===========================================================================*/
  /**
   * This function returns the current time
   *
   * @return String the current time
   */
  String getCurrentTime()
  {
    Calendar calendar = Calendar.getInstance();

    return Long.toString(calendar.getTimeInMillis());
  }

  /*===========================================================================
  CLASS:  SessionThread
  ===========================================================================*/
  /**
   * SessionThread is used to perform the three sessions. It presents a
   * countdown to the screen, starts recording and moves the recorded bin file
   * to the correct destination.
   * This thread is a basic thread that other recording daemon
   * threads iherit from.
   */
  abstract class SessionThread extends Thread
  {

    /*===========================================================================
    FUNCTION:  run
    ===========================================================================*/
    /**
     * run() function is a stub function to be overridden by other
     * recording activities.
     */
    public abstract void run();

    /*===========================================================================
    FUNCTION:  startSession
    ===========================================================================*/
    /**
     * startSession function performs one session. First it presents a countdown
     * on the screen, then the relevant instruction and then starts a daemon to
     * start recording. At the end, the function waits for the daemon to stop
     * and moves the file created to the right directory.
     */
    protected void startSession(int direction,
                                String fileName)
    {
      presentCountdown();

      // Writing instruction on screen
      Message msg = mHandler.obtainMessage(INSTRUCTION_NUM,
                                           direction,
                                           -1);
      mHandler.sendMessage(msg);

      // Starting daemon
      Log.d(this.toString(),
            "starting to record...");
      SystemService.start(THIS_DAEMON);

      // Waiting for daemon to stop recording
      while ((SystemProperties.get("init.svc." + THIS_DAEMON, "stopped"))
             .equals("running"))
      {
        try
        {
          Thread.sleep(SleepTime.WAIT_DAEMON.value);
        }
        catch (InterruptedException e)
        {
          e.printStackTrace();
        }
      }

      // Waiting for the daemon's file to be written
      try
      {
        sleep(SleepTime.MOVE_FILE.value);
      }
      catch (InterruptedException e)
      {
        e.printStackTrace();
      }

      // File to be moved
      File file = new File(mRecOriginalDir);
      moveRecordingFile(file,
                        fileName);
    }

    /*==================================================================
    FUNCTION: presentCountdown
    ==================================================================*/
    /**
     * This function presents a countdown on the screen.
     */
    protected void presentCountdown()
    {
      // Creating countdown
      for (int i = NUM_OF_COUNTDOWN; i > 0; i--)
      {
        try
        {
          sleep(SleepTime.COUNTDOWN.value);
          Message msg = mHandler.obtainMessage(COUNTDOWN_NUM,
                                               i,
                                               -1);
          mHandler.sendMessage(msg);
        }
        catch (InterruptedException e)
        {
          e.printStackTrace();
        }
      }
      try
      {
        sleep(SleepTime.INSTRUCTION.value);
      }
      catch (InterruptedException e)
      {
        e.printStackTrace();
      }
    }

    /*==================================================================
    FUNCTION: moveRecordingFile
    ==================================================================*/
    /**
     * This function moves the given file to the given file name.
     *
     * @param file The file to move.
     * @param fileName The destination name.
     */
    protected void moveRecordingFile(File file,
                                     String fileName)
    {
      // Destination directory
      File dir = new File(mDirectoryToMoveTo);
      Log.d(this.toString(),
            "moving the file from " + mRecOriginalDir
            + " to " +mDirectoryToMoveTo);
      fileName +=  ("_" + getCurrentTime()); // Append current time to filename
      // Move file to new directory
      boolean success = file.renameTo(new File(dir, fileName));
      if (!success)
      {
        Log.e(this.toString(),
              "Error moving record file");
      }
    }
  }
}

