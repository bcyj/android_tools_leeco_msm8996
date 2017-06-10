/******************************************************************************
 * @file    CfgFilePicker.java
 * @brief   Helper class for setting up the cfg file picker.
 *
 *
 * ---------------------------------------------------------------------------
 *  Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 *  Qualcomm Technologies Proprietary and Confidential.
 *  ---------------------------------------------------------------------------
 *******************************************************************************/

package com.android.ultrasound;

import android.content.Context;
import android.util.Log;
import android.view.View;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemSelectedListener;
import android.widget.ArrayAdapter;
import android.widget.Spinner;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileFilter;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;

public class CfgFilePicker {
    private final Context mContext;

    private final Spinner mChooseFileSpinner;

    private final String mFileType;

    private final String mCfgLinkFileLocation;

    private String mCfgFileLocation;

    private final String mCfgFilesDir;

    private final String mCfgExpressionDir;

    public CfgFilePicker(Context c, Spinner chooseFileSpinner, String fileType,
            String cfgLinkFileLocation, String cfgFileLocation, String cfgFilesDir,
            String cfgExpressionDir) {
        mContext = c;
        mChooseFileSpinner = chooseFileSpinner;
        mFileType = fileType;
        mCfgLinkFileLocation = cfgLinkFileLocation;
        mCfgFileLocation = cfgFileLocation;
        mCfgFilesDir = cfgFilesDir;
        mCfgExpressionDir = cfgExpressionDir;
    }

    /*===========================================================================
    INNER_CLASS:  FileCfgItemSelectedListener
    ===========================================================================*/
    /**
     * FileCfgItemSelectedListener class is used in the Spinner that
     * shows all the cfg files in the cfg file dir.
     */
    public class FileCfgItemSelectedListener implements OnItemSelectedListener {
        @Override
        public void onItemSelected(AdapterView<?> parent, View view, int pos, long id) {
            if ((null != parent) && (null != parent.getItemAtPosition(pos))) {
                onCfgItemSelected(parent, pos);
            }
        }

        @Override
        public void onNothingSelected(AdapterView<?> parent) {
            // Do nothing.
        }
    }

    /*===========================================================================
    FUNCTION:  onCfgItemSelected
    ===========================================================================*/
    /**
     * onCfgItemSelected() handles what happen when a file is pressed in the
     * spinner.
     */
    protected void onCfgItemSelected(AdapterView<?> parent, int pos) {
        Log.d(this.toString(), "in onItemSelected in FileCfgItemSelectedListener");
        setCfgFileLocation(mCfgFilesDir + parent.getItemAtPosition(pos).toString());
        updateLinkCfgFile();
    }

    /*===========================================================================
    FUNCTION:  updateLinkCfgFile
    ===========================================================================*/
    /**
     * updateLinkCfgFile() function links the link cfg
     * file to the new name of the cfg file.
     */
    private void updateLinkCfgFile() {
        try {
            Process proc1 = Runtime.getRuntime().exec("rm " + mCfgLinkFileLocation);

            // Handling the streams
            ProcessHandler inputStream = new ProcessHandler(proc1.getInputStream());
            ProcessHandler errorStream = new ProcessHandler(proc1.getErrorStream());

            // Start the stream threads
            inputStream.start();
            errorStream.start();

            Process proc2 = Runtime.getRuntime().exec(
                    "ln -s " + mCfgFileLocation + " " + mCfgLinkFileLocation);

            // Handling the streams
            ProcessHandler inputStream2 = new ProcessHandler(proc2.getInputStream());
            ProcessHandler errorStream2 = new ProcessHandler(proc2.getErrorStream());

            // Start the stream threads
            inputStream2.start();
            errorStream2.start();
        } catch (IOException e) {
            Log.e(this.toString(), "Exec threw IOException" + e);
        }
    }

    /*===========================================================================
    FUNCTION:  setCfgFileLocation
    ===========================================================================*/
    /**
     * setCfgFileLocation() function updates the new name of the
     * cfg file location.
     */
    private void setCfgFileLocation(String cfgFileLocation) {
        if (null == cfgFileLocation) {
            return;
        }
        mCfgFileLocation = cfgFileLocation;
        Log.d(this.toString(), "Setting cfgFileLocation to " + cfgFileLocation);
    }

    /*===========================================================================
    FUNCTION:  getCfgFileLocation
    ===========================================================================*/
    /**
     * setCfgFileLocation() function returns the name of the
     * cfg file location.
     */
    public String getCfgFileLocation() {
        return mCfgFileLocation;
    }

    /*===========================================================================
    FUNCTION:  setCfgFileSpinner
    ===========================================================================*/
    /**
     * Finds the service cfg file. If no such file exist or no cfg
     * dir exist then the UI will be empty
     */
    public void setCfgFileSpinner() {
        try {
            File dir = new File(mCfgFilesDir);
            File[] children = dir.listFiles(new FileFilter() {
                @Override
                public boolean accept(File pathname) {
                    if (pathname.getName().endsWith(mFileType) == true) {
                        return true;
                    }
                    return false;
                }
            });
            String[] childrenStrings = null;
            if (null != children) {
                childrenStrings = new String[children.length];
                for (int i = 0; i < children.length; i++) {
                    childrenStrings[i] = children[i].getName();
                }
            }

            String cfgFileName = UltrasoundUtil
                    .getCfgFileLocationFromLinkFile(mCfgLinkFileLocation);
            Log.d(mContext.toString(), "Symbolic link: " + mCfgLinkFileLocation + "->"
                    + cfgFileName);
            // If cfgFileName is null then default file is choosen
            if (null == cfgFileName) {
                cfgFileName = mCfgFileLocation;
            }
            cfgFileName = cfgFileName.replaceFirst(mCfgExpressionDir, "");
            setCfgFileLocation(mCfgFilesDir + cfgFileName);

            Spinner chooseFileSpinner = mChooseFileSpinner;

            if (null != chooseFileSpinner) {
                ArrayAdapter<String> chooseFileAdapter = new ArrayAdapter<String>(mContext,
                        android.R.layout.simple_spinner_item, childrenStrings);
                chooseFileAdapter
                .setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
                chooseFileSpinner.setAdapter(chooseFileAdapter);
                chooseFileSpinner.setOnItemSelectedListener(new FileCfgItemSelectedListener());

                int spinnerPosition = chooseFileAdapter.getPosition(cfgFileName);
                chooseFileSpinner.setSelection(spinnerPosition);
            }
        } catch (Exception e) {
            Log.e(mContext.toString(),
                    "Exception while setting service cfg spinner: " + e.getMessage());
        }
    }

    /*-===========================================================================
    CLASS:  ProcessHandler
    ===========================================================================*/
    /**
     * This class handles the given streams and closes them. This
     * class is needed because when using Runtime.getRuntime.exec
     * some native platforms only provide limited buffer size for
     * standard input and output streams, failure to promptly write
     * the input stream or read the output stream of the subprocess
     * may cause the subprocess to block, and even deadlock. This
     * class prevents these issues.
     */
    public class ProcessHandler extends Thread {
        InputStream inputStream;

        /*===========================================================================
        FUNCTION:  ProcessHandler
        ===========================================================================*/
        /**
         * Constructor
         * @param inpStr The input stream
         */
        public ProcessHandler(InputStream inpStr) {
            this.inputStream = inpStr;
        }

        /*===========================================================================
        FUNCTION:  run
        ===========================================================================*/
        /**
         * This function is the main thread function. It closes the
         * given stream.
         */
        @Override
        public void run() {
            try {
                InputStreamReader inpStreamReader = new InputStreamReader(inputStream);
                BufferedReader buffReader = new BufferedReader(inpStreamReader);

                buffReader.close();
            } catch (IOException e) {
                Log.e(this.toString(), "IOException:" + e);
            }
        }
    }

}

