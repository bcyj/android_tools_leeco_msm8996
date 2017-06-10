/******************************************************************************
 * @file    P2P.java
 * @brief   Provides P2P UI options.
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
import android.widget.EditText;

public class P2P extends UltrasoundCfg {
  private final String xmlFileName = "p2p";
  private final String thisDaemon = "usf_p2p";
  private final String thisFileName = "P2P.java";
  private final String tag = "P2PSettings";
  private final String cfgFileLocation =
            "/data/usf/p2p/cfg/usf_p2p.cfg";  // Default cfg file
  private final String cfgLinkFile = "/data/usf/p2p/usf_p2p.cfg";
  private final String cfgFileDir = "/data/usf/p2p/cfg/";
  private final String cfgFileExpressionDir = "/data/usf/p2p/.*/";
  private final String recFileDir = "/data/usf/p2p/rec/";
  private final String dspVerFile = "/data/usf/p2p/usf_dsp_ver.txt";
  private EditText dspVersionEditText = null;
  private EditText deviceIDEditText = null;
  private EditText patternTypeEditText = null;
  /* paramsNames - these are the names of the P2P params which are
     exposed to the user. */
  private String[] paramsNames = new String[] {"usf_p2p_device_uid",
                                               "usf_p2p_pattern_type"};

  /*===========================================================================
  FUNCTION:  onCreate
  ===========================================================================*/
  /**
    onCreate() function create the UI and set the private params
    (params that apear only in the P2P UI).
  */
  @Override
  protected void onCreate(Bundle savedInstanceState) {
      super.onCreate(savedInstanceState);

      // Find and set the private params
      dspVersionEditText = (EditText)findViewById(R.id.edtdspver);
      if (null != dspVersionEditText) {
        dspVersionEditText.setText("");
        dspVersionEditText.setEnabled(false);
      }

      deviceIDEditText = (EditText)findViewById(R.id.edtdevid);
      if (null != deviceIDEditText) {
        deviceIDEditText.setText("none");
      }
      cfgFileParams.put(paramsNames[0], "none");
      setOnKeyListener(deviceIDEditText, paramsNames[0]);
      privateParams.put(paramsNames[0], deviceIDEditText);

      patternTypeEditText = (EditText)findViewById(R.id.edtpatterntype);
      if (null != patternTypeEditText) {
        patternTypeEditText.setText("0");
      }
      cfgFileParams.put(paramsNames[1], "0");
      setOnKeyListener(patternTypeEditText, paramsNames[1]);
      privateParams.put(paramsNames[1], patternTypeEditText);


      Log.d(getTag(), "Finished to set the p2p private params");

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

  /*===========================================================================
  FUNCTION:  getDSPVerFile
  ===========================================================================*/
  /**
    See description at UltrasoundCfg.java.
  */
  @Override
  protected String getDSPVerFile() {
    return dspVerFile;
  }

  /*===========================================================================
  FUNCTION:  updateDSPVersion
  ===========================================================================*/
  /**
    See description at UltrasoundCfg.java.
  */
  @Override
  protected void updateDSPVersion() {
    if (null != dspVersionEditText)
    {
      String DSPVer = getDSPVersion();
      if (null == DSPVer)
      {
        return;
      }
      if (DSPVer.equals("0.0.0.0"))
      {
        dspVersionEditText.setText("Stub");
      }
      else
      {
        dspVersionEditText.setText(DSPVer);
      }
    }
  }

  /*===========================================================================
  FUNCTION:  clearDSPVersion
  ===========================================================================*/
  /**
    See description at UltrasoundCfg.java.
  */
  @Override
  protected void clearDSPVersion() {
    if (null != dspVersionEditText)
    {
      dspVersionEditText.setText("");
    }
  }

}

