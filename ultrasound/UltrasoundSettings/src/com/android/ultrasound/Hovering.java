/******************************************************************************
 * @file    Hovering.java
 * @brief   Provides Hovering UI options.
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

public class Hovering extends UltrasoundCfg {
  private final String xmlFileName = "hovering";
  private final String thisDaemon = "usf_hovering";
  private final String thisFileName = "Hovering.java";
  private final String tag = "HoveringSettings";
  private final String cfgFileLocation =
            "/data/usf/hovering/cfg/usf_hovering.cfg";  // Default cfg file
  private final String cfgLinkFile = "/data/usf/hovering/usf_hovering.cfg";
  private final String cfgFileDir = "/data/usf/hovering/cfg/";
  private final String cfgFileExpressionDir = "/data/usf/hovering/.*/";
  private final String recFileDir = "/data/usf/hovering/rec/";
  private final String dspVerFile = "/data/usf/hovering/usf_dsp_ver.txt";
  private EditText dspVersionEditText = null;

  /*===========================================================================
  FUNCTION:  onCreate
  ===========================================================================*/
  /**
    onCreate() function create the UI and set the private params
    (params that apear only in the hovering UI).
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

      Log.d(getTag(), "Finished to set the hovering private params");

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

