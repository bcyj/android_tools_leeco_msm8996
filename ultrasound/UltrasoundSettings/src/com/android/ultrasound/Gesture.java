/******************************************************************************
 * @file    Gesture.java
 * @brief   Provides Gesture options.
 *
 *
 * ---------------------------------------------------------------------------
 *  Copyright (C) 2011-2012 Qualcomm Technologies, Inc.
 *  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
 *  ---------------------------------------------------------------------------
 *******************************************************************************/

package com.android.ultrasound;

import com.android.ultrasound.UltrasoundCfg;
import android.util.Log;
import android.os.Bundle;


public class Gesture extends UltrasoundCfg {
  private final String xmlFileName = "gesture";
  private final String thisDaemon = "usf_gesture";
  private final String thisFileName = "Gesture.java";
  private final String tag = "GestureSettings";
  private final String cfgFileLocation =
            "/data/usf/gesture/cfg/usf_gesture.cfg";  // Default cfg file
  private final String cfgLinkFile = "/data/usf/gesture/usf_gesture.cfg";
  private final String cfgFileDir = "/data/usf/gesture/cfg/";
  private final String cfgFileExpressionDir = "/data/usf/gesture/.*/";
  private final String recFileDir = "/data/usf/gesture/rec/";

  /*===========================================================================
  FUNCTION:  onCreate
  ===========================================================================*/
  /**
    onCreate() function create the UI and set the private params
    (params that apear only in the Gesture UI).
  */
  @Override
  protected void onCreate(Bundle savedInstanceState) {
      super.onCreate(savedInstanceState);

      // Find and set the private params

      // No private params for now

      Log.d(getTag(), "Finished to set the gesture private params");

      readCfgFileAndSetDisp(true);
  }

  /*===========================================================================
  FUNCTION:  getXmlFileName
  ===========================================================================*/
  /**
    See description at UltrasoundCfg.java.
  */
  @Override
  protected String getXmlFileName() {
    return xmlFileName;
  }

  /*===========================================================================
  FUNCTION:  getThisFileName
  ===========================================================================*/
  /**
    See description at UltrasoundCfg.java.
  */
  @Override
  protected String getThisFileName() {
    return thisFileName;
  }

  /*===========================================================================
  FUNCTION:  getTag
  ===========================================================================*/
  /**
    See description at UltrasoundCfg.java.
  */
  @Override
  protected String getTag() {
    return tag;
  }

  /*===========================================================================
  FUNCTION:  getDaemonName
  ===========================================================================*/
  /**
    See description at UltrasoundCfg.java.
  */
  @Override
  protected String getDaemonName() {
    return thisDaemon;
  }

  /*===========================================================================
  FUNCTION:  getCfgLinkFileLocation
  ===========================================================================*/
  /**
    See description at UltrasoundCfg.java.
  */
  @Override
  protected String getCfgLinkFileLocation() {
    return cfgLinkFile;
  }

  /*===========================================================================
  FUNCTION:  getCfgFilesDir
  ===========================================================================*/
  /**
    See description at UltrasoundCfg.java.
  */
  @Override
  protected String getCfgFilesDir() {
    return cfgFileDir;
  }

  /*===========================================================================
  FUNCTION:  getCfgExpressionDir
  ===========================================================================*/
  /**
    See description at UltrasoundCfg.java.
  */
  @Override
  protected String getCfgExpressionDir() {
    return cfgFileExpressionDir;
  }

  /*===========================================================================
  FUNCTION:  getRecFilesDir
  ===========================================================================*/
  /**
    See description at UltrasoundCfg.java.
  */
  @Override
  protected String getRecFilesDir() {
    return recFileDir;
  }

  /*===========================================================================
  FUNCTION:  getDefaultCfgFileName
  ===========================================================================*/
  /**
    See description at UltrasoundCfg.java.
  */
  @Override
  protected String getDefaultCfgFileName() {
    return cfgFileLocation;
  }

}

