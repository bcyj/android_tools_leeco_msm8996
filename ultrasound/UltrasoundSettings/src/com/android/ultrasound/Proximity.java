/******************************************************************************
 * @file    Proximity.java
 * @brief   Provides Proximity options.
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


public class Proximity extends UltrasoundCfg {
  private final String xmlFileName = "proximity";
  private final String thisDaemon = "usf_proximity";
  private final String thisFileName = "Proximity.java";
  private final String tag = "ProximitySettings";
  private final String cfgFileLocation =
            "/data/usf/proximity/cfg/usf_proximity.cfg";  // Default cfg file
  private final String cfgLinkFile = "/data/usf/proximity/usf_proximity.cfg";
  private final String cfgFileDir = "/data/usf/proximity/cfg/";
  private final String cfgFileExpressionDir = "/data/usf/proximity/.*/";
  private final String recFileDir = "/data/usf/proximity/rec/";

  /*===========================================================================
  FUNCTION:  onCreate
  ===========================================================================*/
  /**
    onCreate() function create the UI and set the private params
    (params that apear only in the Proximity UI).
  */
  @Override
  protected void onCreate(Bundle savedInstanceState) {
      super.onCreate(savedInstanceState);

      // Find and set the private params

      // No private params for now

      Log.d(getTag(), "Finished to set the proximity private params");

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

