/******************************************************************************
 * @file    ProximityRecording.java
 * @brief   Presents one sessions, asking the user to approach the screen and
 * records it.
 * ---------------------------------------------------------------------------
 *  Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
 *  Qualcomm Technologies Proprietary and Confidential.
 *  ---------------------------------------------------------------------------
 *  ***************************************************************************/

package com.android.ultrasound;

import java.io.File;
import android.os.SystemService;
import android.util.Log;
import android.os.Message;
import android.widget.TextView;
import android.os.Bundle;
import java.io.*;

public class ProximityRecording extends BasicDaemonRecording
{
  private final String THIS_DAEMON = "usf_proximity";
  private final int TIME_WRITE_FILE = 5;

  /*===========================================================================
  FUNCTION:  onCreate
  ===========================================================================*/
  /**
   * onCreate() function creates the UI, captures the common elements, updates
   * the link cfg file and starts the SessionThread.
   */
  @Override
  protected void onCreate(Bundle savedInstanceState)
  {
    Log.d(this.toString(),
          "ProximityRecording: onCreate()");
    super.onCreate(savedInstanceState);

    setContentView(R.layout.gesture_recording);
    mTextView = (TextView) findViewById(R.id.textView);

    // Start the sessionThread
    mSessionThread = new ProximitySessionThread();
    mSessionThread.setName("SessionThread");
    mSessionThread.start();
  }

  /*===========================================================================
   CLASS:  ProximitySessionThread
  ===========================================================================*/
  /**
   * SessionThread is used to perform the three sessions. It presents a
   * countdown to the screen, starts recording and moves the recorded bin file
   * to the correct destination
   */
  class ProximitySessionThread extends SessionThread
  {
    /*===========================================================================
    FUNCTION:  run
    ===========================================================================*/
    /**
     * run() function launches the three sessions
     */
    @Override
    public void run()
    {
      startSession(Instruction.APPROACH.num,
                   Instruction.APPROACH.fileName);
      // presenting last instruction on screen
      Message msg = mHandler.obtainMessage(GREETING_NUM);
      mHandler.sendMessage(msg);
    }

    /*===========================================================================
    FUNCTION:  startSession
    ===========================================================================*/
    /**
     * startSession function performs one session. First it presents a countdown
     * on the screen, then the relevant instruction and then starts a daemon to
     * start recording. At the end, the function waits for the daemon to stop
     * and moves the file created to the right directory.
     */
    @Override
    protected void startSession(int direction,
                              String fileName)
    {
      Log.d(this.toString(),
            "StartSession in ProximitySessionThread");
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
      try
      {
        Thread.sleep(SleepTime.WAIT_PROXIMITY.value);
      }
      catch (InterruptedException e)
      {
        e.printStackTrace();
      }
      // Waiting for daemon to stop recording
      File recFile = new File(mRecOriginalDir);
      for (int i = 0; i < TIME_WRITE_FILE; i++)
      {
        if (true == isFileWriteComplete(recFile))
        {
          Log.d(this.toString(),
                "file FINISHED writing");
          break;
        }
        try
        {
          Thread.sleep(SleepTime.WAIT_DAEMON.value);
        }
        catch (InterruptedException e)
        {
          e.printStackTrace();
        }
        Log.d(this.toString(),
              "file didn't finish writing");
      }

      moveRecordingFile(recFile,
                        fileName);

      stopDaemon();

    }

    /*===========================================================================
    FUNCTION:  stopDaemon
    ===========================================================================*/
    /**
     * This function stops the daemon.
     */
    private void stopDaemon()
    {
      int pid = UltrasoundUtil.getPid(THIS_DAEMON);
      if (-1 == pid)
      { // Problem with pid file, killing daemon with SIGTERM
        SystemService.stop(THIS_DAEMON);
      }
      else
      {
        try
        {
          Runtime.getRuntime().exec("kill -15 " + pid);
        }
        catch (IOException e)
        {
          Log.e(this.toString(),
                "Exec threw IOException");
          SystemService.stop(THIS_DAEMON); // Stop daemon with SIGKILL
        }
      }
    }

    /*===========================================================================
    FUNCTION:  isFileWriteComplete
    ===========================================================================*/
    /**
     * This function checks whether the writing of the given file
     * was completed.
     *
     * @param file The file to check.
     *
     * @return boolean true - File write is complete.
     *                 false - Fiel write is not complete.
     */
    boolean isFileWriteComplete(File file)
    {
      if (!file.exists() ||
          !file.isFile())
      {
        return false;
      }

      long len_1 = file.length();
      // Wait some time before checking if file size has changed.
      try
      {
        sleep(SleepTime.WRITE_FILE.value);
        long len_2 = file.length();
        // If file size has not changed then record is done.
        if (len_1 == len_2)
        {
          return true;
        }
      }
      catch (InterruptedException e)
      {
        e.printStackTrace();
      }
      // File size has changed
      return false;
    }
  }
}

