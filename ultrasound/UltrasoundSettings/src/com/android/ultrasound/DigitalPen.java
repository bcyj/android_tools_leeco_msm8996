/******************************************************************************
 * @file    DigitalPen.java
 * @brief   Provides Digital Pen options.
 *
 *
 * ---------------------------------------------------------------------------
 *  Copyright (c) 2011-2012, 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 *  Qualcomm Technologies Proprietary and Confidential.
 *  ---------------------------------------------------------------------------
 *******************************************************************************/

package com.android.ultrasound;

import android.app.AlertDialog;
import android.content.DialogInterface;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.EditText;
import android.widget.Spinner;

import java.io.File;

public class DigitalPen extends UltrasoundCfg {
    private final String xmlFileName = "digital_pen";

    private final String thisDaemon = "usf_epos";

    private final String thisFileName = "DigitalPen.java";

    private final String tag = "DigitalPenSettings";

    // Default daemon cfg file
    private final String daemonCfgFileLocation = "/data/usf/epos/cfg/usf_epos.cfg";

    // Default service cfg file
    private String serviceCfgFileLocation = "/data/usf/epos/cfg/service_settings.xml";

    private final String daemonCfgLinkFile = "/data/usf/epos/usf_epos.cfg";

    private final String serviceCfgLinkFile = "/data/usf/epos/service_settings.xml";

    private final String cfgFileDir = "/data/usf/epos/cfg/";

    private final String cfgFileExpressionDir = "/data/usf/epos/.*/";

    private final String recFileDir = "/data/usf/epos/rec/";

    private final String dspVerFile = "/data/usf/epos/usf_dsp_ver.txt";

    private final String persistFileParamName = "usf_epos_persistent_packet";

    private Button clrPersistButton = null;

    private EditText dspVersionEditText = null;

    private EditText coordFileEditText = null;

    private EditText coordCountEditText = null;

    private EditText timeoutEditText = null;

    private File persistentFile = null;

    private CheckBox calibModeCheckbox;

    /* eposParamsNames - these are the names of the params which are
       exposed to the user only in epos daemon. */
    private String[] eposParamsNames = new String[] {
            "usf_epos_coord_file", "usf_epos_coord_count", "usf_epos_timeout_to_coord_rec"
    };

    private CfgFilePicker mServiceCfgFilePicker = null;

    /*===========================================================================
    FUNCTION:  onCreate
    ===========================================================================*/
    /**
     * onCreate() function create the UI and set the private params
     * (params that apear only in the Digital Pen UI).
     */
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        Log.d(getTag(), getThisFileName() + " in OnCreate()");

        super.onCreate(savedInstanceState);

        // Find and set the private params
        dspVersionEditText = (EditText)findViewById(R.id.edtdspver);
        if (null != dspVersionEditText) {
            dspVersionEditText.setText("");
            dspVersionEditText.setEnabled(false);
        }

        clrPersistButton = (Button)findViewById(R.id.clr_persist);
        coordFileEditText = (EditText)findViewById(R.id.edtcoordfile);
        coordCountEditText = (EditText)findViewById(R.id.edtcoordcount);
        timeoutEditText = (EditText)findViewById(R.id.edttimeout);

        setCalibParams(coordFileEditText, 0);
        setCalibParams(coordCountEditText, 1);
        setCalibParams(timeoutEditText, 2);

        // Find the Calib Mode checkbox
        calibModeCheckbox = (CheckBox)findViewById(R.id.calib);

        if ((null != calibModeCheckbox) && (null != coordCountEditText)
                && (null != coordFileEditText) && (null != timeoutEditText)) {

            calibModeCheckbox.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View view) {
                    if (((CheckBox)view).isChecked()) {
                        coordFileEditText.setEnabled(true);
                        coordCountEditText.setEnabled(true);
                        timeoutEditText.setEnabled(true);
                    } else {
                        readCfgFileAndSetDisp(false);
                        coordFileEditText.setEnabled(false);
                        coordCountEditText.setText("0");
                        coordCountEditText.setEnabled(false);
                        cfgFileParams.put(eposParamsNames[1], "0");
                        timeoutEditText.setText("0");
                        timeoutEditText.setEnabled(false);
                        cfgFileParams.put(eposParamsNames[2], "0");
                        updateCfgFile();
                    }
                }
            });
        }

        Log.d(getTag(), "Finished to set the digital pen private params");

        readCfgFileAndSetDisp(true);

        boolean isCalib = false;
        if ((Integer.valueOf(coordCountEditText.getText().toString()) > 0)
                || (Integer.valueOf(timeoutEditText.getText().toString()) > 0)) {
            isCalib = true;
        }
        calibModeCheckbox.setChecked(isCalib);
        coordFileEditText.setEnabled(isCalib);
        coordCountEditText.setEnabled(isCalib);
        timeoutEditText.setEnabled(isCalib);

        clrPersistButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                persistentFile = new File(cfgFileParams.get(persistFileParamName));
                if (!persistentFile.exists()) { // Sanity check
                    Log.w(getTag(), "File does not exist");
                    return;
                }
                if (persistentFile.delete()) {
                    clrPersistButton.setEnabled(false);
                    Log.d(getTag(), "File removed");
                } else {
                    Log.e(getTag(), "Failed to remove persistent data file");
                }
            }
        });

        mServiceCfgFilePicker = new CfgFilePicker(this,
                (Spinner)findViewById(R.id.choose_xml_file_spinner), "xml",
                getServiceCfgLinkFileLocation(), getDefaultServiceCfgFileName(), getCfgFilesDir(),
                getCfgExpressionDir());
        mServiceCfgFilePicker.setCfgFileSpinner();
    }

    /*===========================================================================
    FUNCTION:  onStatusCheckboxClick
    ===========================================================================*/
    /**
     * onStatusCheckboxClick() function is called when pressing the
     * main status checkbox in each daemon activity.
     */
    @Override
    protected void onStatusCheckboxClick(View view) {
        Log.d(this.toString(), "onStatusCheckboxClick override");

        final CheckBox status = (CheckBox)view;
        final boolean isChecked = status.isChecked();

        AlertDialog.Builder alertDialogBuilder = new AlertDialog.Builder(this);

        alertDialogBuilder.setTitle("Digital Pen status change warning");

        alertDialogBuilder
        .setMessage(
                "Warning! Using this application to enable or disable the digital pen will cause unexpected behavior in pen applications. Use SDP settings to enable or disable the pen. If you want to change the lower level pen configuration (.cfg file):\n"
                        + "1. Disable the pen using SDP settings (not this app).\n"
                        + "2. Change the lower level configuration using this app.\n"
                        + "3. Enable the pen using SDP settings (not this app).")
                        .setCancelable(false)
                        .setPositiveButton("Continue", new DialogInterface.OnClickListener() {
                            @Override
                            public void onClick(DialogInterface dialog, int id) {
                                status.setChecked(isChecked);
                                superOnStatusCheckboxClick(status);
                            }
                        }).setNegativeButton("Abort", new DialogInterface.OnClickListener() {
                            @Override
                            public void onClick(DialogInterface dialog, int id) {
                                // if this cancel button is clicked, just close
                                // the dialog box and do nothing
                                dialog.cancel();
                            }
                        });

        // create alert dialog
        AlertDialog alertDialog = alertDialogBuilder.create();

        // show it
        alertDialog.show();
    }

    /*===========================================================================
    FUNCTION:  superOnStatusCheckboxClick
    ===========================================================================*/
    /**
     * superOnStatusCheckboxClick() calls UltrasoundCfg's
     * implementation of onStatusCheckboxClick().
     */
    private void superOnStatusCheckboxClick(View view) {
        super.onStatusCheckboxClick(view);
    }

    /*===========================================================================
    FUNCTION:  setCalibParams
    ===========================================================================*/
    /**
     * Set all the epos calibration edit text, coords file name,
     * coords count and timeout, to default and update
     * eposParamsNames.
     */
    private void setCalibParams(EditText et, int calibParamNum) {
        if (null != et) {
            et.setText("0");
            cfgFileParams.put(eposParamsNames[calibParamNum], "0");
            if (0 == calibParamNum) {
                et.setText("");
                cfgFileParams.put(eposParamsNames[calibParamNum], "");
            }
            setOnKeyListener(et, eposParamsNames[calibParamNum]);
            privateParams.put(eposParamsNames[calibParamNum], et);
            et.setEnabled(false);
        }
    }

    /*===========================================================================
    FUNCTION:  updateUI
    ===========================================================================*/
    /**
     * Override updateUI to also check if persistent data exists
     */
    @Override
    protected void updateUI() {
        super.updateUI();

        if (null != cfgFileParams.get(persistFileParamName)) {
            persistentFile = new File(cfgFileParams.get(persistFileParamName));
            if (null != persistentFile) {
                if (persistentFile.exists()) {
                    clrPersistButton.setEnabled(true);
                }
            }
        }
    }

    /*===========================================================================
    FUNCTION:  readCfgFileAndSetDisp
    ===========================================================================*/
    /**
     * Override readCfgFileAndSetDisp() method to add a check for the persistent
     * data file
     */
    @Override
    protected void readCfgFileAndSetDisp(boolean setDisp) {
        super.readCfgFileAndSetDisp(setDisp);

        try {
            if (setDisp) {
                persistentFile = new File(cfgFileParams.get(persistFileParamName));
                // The button should be enabled only if file exists
                clrPersistButton.setEnabled(persistentFile.exists());
            }
        } catch (Exception ex) {
            Log.w(getTag(), "Could not check if persistent file exists. Disabling"
                    + "Clear persistent button");
            clrPersistButton.setEnabled(false);
        }
    }

    /*===========================================================================
    FUNCTION:  getXmlFileName
    ===========================================================================*/
    /**
     * See description at UltrasoundCfg.java.
     */
    @Override
    protected String getXmlFileName() {
        return xmlFileName;
    }

    /*===========================================================================
    FUNCTION:  getThisFileName
    ===========================================================================*/
    /**
     * See description at UltrasoundCfg.java.
     */
    @Override
    protected String getThisFileName() {
        return thisFileName;
    }

    /*===========================================================================
    FUNCTION:  getTag
    ===========================================================================*/
    /**
     * See description at UltrasoundCfg.java.
     */
    @Override
    protected String getTag() {
        return tag;
    }

    /*===========================================================================
    FUNCTION:  getDaemonName
    ===========================================================================*/
    /**
     * See description at UltrasoundCfg.java.
     */
    @Override
    protected String getDaemonName() {
        return thisDaemon;
    }

    /*===========================================================================
    FUNCTION:  getCfgLinkFileLocation
    ===========================================================================*/
    /**
     * See description at UltrasoundCfg.java.
     */
    @Override
    protected String getCfgLinkFileLocation() {
        return daemonCfgLinkFile;
    }

    /*===========================================================================
    FUNCTION:  getServiceCfgLinkFileLocation
    ===========================================================================*/
    /**
     * Returns the service cfg file location.
     */
    private String getServiceCfgLinkFileLocation() {
        return serviceCfgLinkFile;
    }

    /*===========================================================================
    FUNCTION:  getCfgFilesDir
    ===========================================================================*/
    /**
     * See description at UltrasoundCfg.java.
     */
    @Override
    protected String getCfgFilesDir() {
        return cfgFileDir;
    }

    /*===========================================================================
    FUNCTION:  getCfgExpressionDir
    ===========================================================================*/
    /**
     * See description at UltrasoundCfg.java.
     */
    @Override
    protected String getCfgExpressionDir() {
        return cfgFileExpressionDir;
    }

    /*===========================================================================
    FUNCTION:  getRecFilesDir
    ===========================================================================*/
    /**
     * See description at UltrasoundCfg.java.
     */
    @Override
    protected String getRecFilesDir() {
        return recFileDir;
    }

    /*===========================================================================
    FUNCTION:  getDefaultCfgFileName
    ===========================================================================*/
    /**
     * See description at UltrasoundCfg.java.
     */
    @Override
    protected String getDefaultCfgFileName() {
        return daemonCfgFileLocation;
    }

    /*===========================================================================
    FUNCTION:  getDefaultServiceCfgFileName
    ===========================================================================*/
    /**
     * Returns the service cfg file name.
     */
    private String getDefaultServiceCfgFileName() {
        return serviceCfgFileLocation;
    }

    /*===========================================================================
    FUNCTION:  setServiceCfgFileLocation
    ===========================================================================*/
    /**
     * Updates the new name of the service cfg file location.
     */
    protected void setServiceCfgFileLocation(String serviceCfgFileLocation) {
        if (null == serviceCfgFileLocation) {
            return;
        }
        this.serviceCfgFileLocation = serviceCfgFileLocation;
        Log.d(getTag(), "Setting serviceCfgFileLocation to " + serviceCfgFileLocation);
    }

    /*===========================================================================
    FUNCTION:  getDSPVerFile
    ===========================================================================*/
    /**
     * See description at UltrasoundCfg.java.
     */
    @Override
    protected String getDSPVerFile() {
        return dspVerFile;
    }

    /*===========================================================================
    FUNCTION:  updateDSPVersion
    ===========================================================================*/
    /**
     * See description at UltrasoundCfg.java.
     */
    @Override
    protected void updateDSPVersion() {
        if (null != dspVersionEditText) {
            String DSPVer = getDSPVersion();
            if (null == DSPVer) {
                return;
            }
            if (DSPVer.equals("0.0.0.0")) {
                dspVersionEditText.setText("Stub");
            } else {
                dspVersionEditText.setText(DSPVer);
            }
        }
    }

    /*===========================================================================
    FUNCTION:  clearDSPVersion
    ===========================================================================*/
    /**
     * See description at UltrasoundCfg.java.
     */
    @Override
    protected void clearDSPVersion() {
        if (null != dspVersionEditText) {
            dspVersionEditText.setText("");
        }
    }

}

