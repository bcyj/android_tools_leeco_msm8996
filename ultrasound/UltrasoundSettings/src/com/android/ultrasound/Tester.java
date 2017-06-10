/******************************************************************************
 * @file    Tester.java
 * @brief   Provides Tester UI options.
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
import android.widget.Spinner;
import android.widget.AdapterView.OnItemSelectedListener;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.os.Bundle;
import android.view.View;


public class Tester extends UltrasoundCfg {
  private final String xmlFileName = "tester";
  private final String thisDaemon = "usf_tester";
  private final String thisFileName = "Tester.java";
  private final String tag = "TesterSettings";
  private final String cfgFileLocation =
              "/data/usf/tester/cfg/usf_tester.cfg";  // Default cfg file
  private final String cfgLinkFile = "/data/usf/tester/usf_tester.cfg";
  private final String cfgFileDir = "/data/usf/tester/cfg/";
  private final String cfgFileExpressionDir = "/data/usf/tester/.*/";
  private final String recFileDir = "/data/usf/tester/rec/";
  private Spinner frameFileFormatSpinner;

  /*===========================================================================
  FUNCTION:  onCreate
  ===========================================================================*/
  /**
    onCreate() function create the UI and set the private params
    (params that apear only in the Tester UI).
  */
  @Override
  protected void onCreate(Bundle savedInstanceState) {
      super.onCreate(savedInstanceState);
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

