/******************************************************************************
 * @file    GestureRecording.java
 * @brief   Presents three sessions, asking the user to perform three kinds of
 * gestures and records them.
 * ---------------------------------------------------------------------------
 *  Copyright (c) 2012 Qualcomm Technologies, Inc.
 *  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
 *  ---------------------------------------------------------------------------
 *  ***************************************************************************/

package com.android.ultrasound;

import android.os.Bundle;
import android.os.Message;
import android.util.Log;
import android.widget.TextView;


public class GestureRecording extends BasicDaemonRecording
{
  private final int LAST_INSTRUCTION = 2;

  /*===========================================================================
   FUNCTION:  onCreate
  ===========================================================================*/
  /**
   * onCreate() function calls the super's onCreate function and
   * launches the SessionThread.
   */
  @Override
  protected void onCreate(Bundle savedInstanceState)
  {
    Log.d(this.toString(),
          "GestureRecording: onCreate()");
    super.onCreate(savedInstanceState);

    setContentView(R.layout.gesture_recording);
    mTextView = (TextView) findViewById(R.id.textView);

    // Start the sessionThread
    mSessionThread = new GestureSessionThread();
    mSessionThread.setName("SessionThread");
    mSessionThread.start();
  }

  /*===========================================================================
   CLASS:  GestureSessionThread
  ===========================================================================*/
  /**
   * This thread launches the suitable sessions for the current
   * recording.
   */
  class GestureSessionThread extends SessionThread
  {
    /*===========================================================================
    FUNCTION:  run
    ===========================================================================*/
    /**
     * run() function launches the three sessions.
     */
    @Override
    public void run()
    {
      for (Instruction currInstruction : Instruction.values())
      {
        if (LAST_INSTRUCTION < currInstruction.num)
        {
          continue;
        }
        Log.d(this.toString(),
              currInstruction.instruction);
        startSession(currInstruction.num,
                     currInstruction.fileName);
      }

      // presenting last instruction on screen
      Message msg = mHandler.obtainMessage(GREETING_NUM);
      mHandler.sendMessage(msg);
    }
  }
}
