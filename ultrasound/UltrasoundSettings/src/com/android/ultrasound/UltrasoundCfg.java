/******************************************************************************
 * @file    UltrasoundCfg.java
 * @brief   Provides common behavior for Ultrasound cfg UI.
 *
 *
 * ---------------------------------------------------------------------------
 *  Copyright (c) 2011-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 *  Qualcomm Technologies Proprietary and Confidential.
 *  ---------------------------------------------------------------------------
 *******************************************************************************/

package com.android.ultrasound;

import android.app.Activity;
import android.graphics.Color;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.os.SystemProperties;
import android.os.SystemService;
import android.util.Log;
import android.view.KeyEvent;
import android.view.View;
import android.view.View.OnKeyListener;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemSelectedListener;
import android.widget.ArrayAdapter;
import android.widget.CheckBox;
import android.widget.EditText;
import android.widget.Spinner;

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.DataInputStream;
import java.io.File;
import java.io.FileFilter;
import java.io.FileInputStream;
import java.io.FileWriter;
import java.io.IOException;
import java.io.InputStreamReader;
import java.util.Collection;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;
import java.util.LinkedHashMap;
import java.util.Map;
import java.util.Set;
import java.util.StringTokenizer;

public abstract class UltrasoundCfg extends Activity {
    private CheckBox statusCheckbox;

    private CheckBox startOnBootCheckbox;

    private CheckBox debugModeCheckbox;

    private CheckBox appendTimestampCheckbox;

    private EditText frameFileEditText;

    private EditText frameCountEditText;

    private final String autoStartFile = "/data/usf/auto_start.txt";

    /* cfgFileParams - map between param name to its value.
       This map keeps the updated values of the params. */
    protected Map<String, String> cfgFileParams = new HashMap<String, String>();

    /* cfgFile - map between line number to param name or comment.
       Keep input order. After reading the cfg file this map is used
       to update the file in the same order of the reading. */
    private LinkedHashMap<Integer, String> cfgFile = new LinkedHashMap<Integer, String>();

    /* privateParams - map between param name to the param View.
       This map is used when updating the UI with updated values.
       The map contain parameters exposed to the user but only in
       the current daemon. */
    protected Map<String, View> privateParams = new HashMap<String, View>();

    private RefreshHandler redrawHandler = new RefreshHandler();

    private final int REFRESH_UI_INTERVAL_MS = 3000;

    private final int WAIT_FOR_FRAME_FILE_INTERVAL_MS = 500;

    /* paramsNames - these are the names of the params which are
       exposed to the user and common to all daemons. */
    private String[] commonParamsNames = new String[] {
            "usf_frame_file", "usf_frame_count", "usf_frame_file_format", "usf_append_timestamp"
    };

    // commonParamNames
    private final int USF_FRMAE_FILE = 0;

    private final int USF_FRAME_COUNT = 1;

    private final int USF_FRAME_FILE_FORMAT = 2;

    private final int USF_APPEND_TIMESTAMP = 3;

    private Spinner frameFileFormatSpinner;

    private CfgFilePicker mDaemonCfgFilePicker;

    /*===========================================================================
    INNER_CLASS:  FileFormetItemSelectedListener
    ===========================================================================*/
    /**
     * FileFormetItemSelectedListener class is used in the Spinner
     * that shows all the formats to the frame_file.
     */
    public class FileFormetItemSelectedListener implements OnItemSelectedListener {
        @Override
        public void onItemSelected(AdapterView<?> parent, View view, int pos, long id) {
            if ((null != parent) && (null != parent.getItemAtPosition(pos))) {
                cfgFileParams.put(commonParamsNames[USF_FRAME_FILE_FORMAT], Integer.toString(pos));
            }
            updateCfgFile();
        }

        @Override
        public void onNothingSelected(AdapterView<?> parent) {
            // Do nothing.
        }
    }

    /*===========================================================================
    INNER_CLASS:  RefreshHandler
    ===========================================================================*/
    /**
     * RefreshHandler class is used to refresh the statusCheckbox -
     * every period the handler calls to updateUI() function.
     */
    class RefreshHandler extends Handler {
        @Override
        public void handleMessage(Message msg) {
            UltrasoundCfg.this.updateUI();
        }

        public void sleep(long delayMillis) {
            this.removeMessages(0);
            sendMessageDelayed(obtainMessage(0), delayMillis);
        }
    };

    /*===========================================================================
    FUNCTION:  updateUI
    ===========================================================================*/
    /**
     * updateUI() function checks if the current daemon is running or
     * not and updates the statusCheckbox accordingly.
     */
    protected void updateUI() {
        redrawHandler.sleep(REFRESH_UI_INTERVAL_MS);

        if (null == statusCheckbox) {
            return;
        }

        if ((SystemProperties.get("init.svc." + getDaemonName(), "stopped")).equals("stopped")) {
            statusCheckbox.setEnabled(true);
            statusCheckbox.setTextColor(Color.WHITE);
            statusCheckbox.setChecked(false);
        } else if ((SystemProperties.get("init.svc." + getDaemonName(), "stopped"))
                .equals("running")) {
            statusCheckbox.setEnabled(true);
            statusCheckbox.setTextColor(Color.WHITE);
            statusCheckbox.setChecked(true);
            updateDSPVersion();
        }
        // Daemon is restarting or stopping
        else {
            // If daemon is restarting text of checkbox is painted in red
            // and checkbox is disabled.
            statusCheckbox.setTextColor(Color.RED);
            statusCheckbox.setEnabled(false);
        }
        // If daemon is not running clear the DSP version and return debug mode
        // to white.
        if (!(SystemProperties.get("init.svc." + getDaemonName(), "stopped")).equals("running")) {
            clearDSPVersion();
            if (null != debugModeCheckbox) {
                debugModeCheckbox.setTextColor(Color.WHITE);
            }
            return;
        }

        // If daemon is running checks if recording of debug mode has finished.
        if ((null == frameFileEditText) || (null == debugModeCheckbox)
                || (null == frameCountEditText)) {
            return;
        }
        File frameCountFile = new File(getRecFilesDir() + frameFileEditText.getText().toString());
        if (frameCountFile.exists() && frameCountFile.isFile()) {
            long len_1 = frameCountFile.length();
            if ((0 < len_1) && (debugModeCheckbox.getCurrentTextColor() != Color.GREEN)
                    && !frameCountEditText.getText().toString().equals("0")) {
                // Wait some time before checking if file size has changed.
                try {
                    Thread.sleep(WAIT_FOR_FRAME_FILE_INTERVAL_MS);
                    long len_2 = frameCountFile.length();
                    // If file size has not changed then record is done.
                    if (len_1 == len_2) {
                        debugModeCheckbox.setTextColor(Color.GREEN);
                    }
                } catch (InterruptedException e) {
                    // Mark text in red to indicate that an error occured
                    debugModeCheckbox.setTextColor(Color.RED);
                }
            }
        }
    }

    /*===========================================================================
    FUNCTION:  useNewFileNBackupOld
    ===========================================================================*/
    /**
     * This function backups the old cfg file and changes the name
     * of the new cfg file to the old one, so that it could be used.
     *
     * @param filename the name of the old cfg file
     */
    private void useNewFileNBackupOld(String filename) {
        File newFile = new File(filename + "_new");
        File oldFile = new File(filename);
        File backup = new File(filename + "_backup");

        if (!newFile.exists() || !newFile.isFile()) {
            Log.e(getTag(), "File doesn't exist or is not a file");
            return;
        }
        if (backup.isFile() && backup.exists()) {
            Log.d(getTag(), "Backup file exists, deleting it.");
            backup.delete();
        }
        oldFile.renameTo(backup);
        newFile.renameTo(oldFile);
    }

    /*===========================================================================
    FUNCTION:  onCreate
    ===========================================================================*/
    /**
     * onCreate() function create the UI and set the common params
     * (params that apear in every ultrasound UI).
     */
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        int layoutId = getResources().getIdentifier(getXmlFileName(), "layout", getPackageName());
        setContentView(layoutId);

        /* Find and set the enable/disable ,start_on_reboot checkboxes,
           the choose_settings_file spinner, the frame_file_name editText
           and the frame_count editText */
        statusCheckbox = (CheckBox)findViewById(R.id.status);

        if (null != statusCheckbox) {
            statusCheckbox.setEnabled(true);
            updateUI();

            statusCheckbox.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View view) {
                    onStatusCheckboxClick(view);
                }
            });
        }

        startOnBootCheckbox = (CheckBox)findViewById(R.id.start_on_boot);

        if (null != startOnBootCheckbox) {
            startOnBootCheckbox.setEnabled(true);
            if (updateAutoStartFile(false, true)) {
                startOnBootCheckbox.setChecked(true);
            } else {
                startOnBootCheckbox.setChecked(false);
            }

            startOnBootCheckbox.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View view) {
                    if (((CheckBox)view).isChecked()) {
                        updateAutoStartFile(true, false);
                    } else {
                        updateAutoStartFile(false, false);
                    }
                }
            });
        }

        mDaemonCfgFilePicker = new CfgFilePicker(this,
                (Spinner)findViewById(R.id.choose_file_spinner), "cfg", getCfgLinkFileLocation(),
                getDefaultCfgFileName(), getCfgFilesDir(), getCfgExpressionDir()) {
            @Override
            protected void onCfgItemSelected(AdapterView<?> parent, int pos) {
                super.onCfgItemSelected(parent, pos);
                cfgFile.clear();
                cfgFileParams.clear();
                readCfgFileAndSetDisp(true);
            }
        };

        mDaemonCfgFilePicker.setCfgFileSpinner();

        setFrameFileParams();

        // Find the Debug Mode checkbox
        debugModeCheckbox = (CheckBox)findViewById(R.id.debug);

        if ((null != debugModeCheckbox) && (null != frameCountEditText)
                && (null != frameFileEditText)) {
            debugModeCheckbox.setTextColor(Color.WHITE);

            debugModeCheckbox.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View view) {
                    debugModeCheckbox.setTextColor(Color.WHITE);
                    if (((CheckBox)view).isChecked()) {
                        appendTimestampCheckbox.setEnabled(true);
                        frameFileEditText.setEnabled(true);
                        frameCountEditText.setEnabled(true);
                        frameFileFormatSpinner.setEnabled(true);
                    } else {
                        readCfgFileAndSetDisp(false);
                        appendTimestampCheckbox.setEnabled(false);
                        frameFileEditText.setEnabled(false);
                        frameCountEditText.setText("0");
                        frameCountEditText.setEnabled(false);
                        frameFileFormatSpinner.setEnabled(false);
                        cfgFileParams.put(commonParamsNames[USF_FRAME_COUNT], "0");
                        updateCfgFile();
                    }
                }
            });
        }
    }

    /*===========================================================================
    FUNCTION:  onStatusCheckboxClick
    ===========================================================================*/
    /**
     * onCreaonStatusCheckboxClick() function is called when pressing
     * the main status checkbox in each daemon activity.
     */
    protected void onStatusCheckboxClick(View view) {
        Log.d(this.toString(), "onStatusCheckboxClick original");
        if (((CheckBox)view).isChecked()) {
            SystemService.start(getDaemonName());
        } else {
            int pid = UltrasoundUtil.getPid(getDaemonName());
            if (-1 == pid) { // Problem with pid file, killing daemon with
                // SIGTERM
                SystemService.stop(getDaemonName());
            } else {
                try {
                    Runtime.getRuntime().exec("kill -15 " + pid);
                } catch (IOException e) {
                    Log.e(this.toString(), "Exec threw IOException");
                    SystemService.stop(getDaemonName()); // Stop daemon with
                    // SIGKILL
                }
            }
            clearDSPVersion();
        }
    }

    /*===========================================================================
    FUNCTION:  setFrameFileParams
    ===========================================================================*/
    /**
     * setFrameFileParams() function sets the frame file parameters.
     */
    void setFrameFileParams() {
        frameFileEditText = (EditText)findViewById(R.id.edtfile);

        if (null != frameFileEditText) {
            frameFileEditText.setText("");
            cfgFileParams.put(commonParamsNames[USF_FRMAE_FILE], "");
            setOnKeyListener(frameFileEditText, commonParamsNames[USF_FRMAE_FILE]);
            privateParams.put(commonParamsNames[USF_FRMAE_FILE], frameFileEditText);
            frameFileEditText.setEnabled(false);
        }

        appendTimestampCheckbox = (CheckBox)findViewById(R.id.timestamp);
        if (null != appendTimestampCheckbox) {
            appendTimestampCheckbox.setEnabled(false);
            privateParams.put(commonParamsNames[USF_APPEND_TIMESTAMP], appendTimestampCheckbox);
        }
        appendTimestampCheckbox.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                if (((CheckBox)view).isChecked()) {
                    readCfgFileAndSetDisp(false);
                    cfgFileParams.put(commonParamsNames[USF_APPEND_TIMESTAMP], "1");
                    updateCfgFile();
                } else {
                    readCfgFileAndSetDisp(false);
                    cfgFileParams.put(commonParamsNames[USF_APPEND_TIMESTAMP], "0");
                    updateCfgFile();
                }
            }
        });

        frameCountEditText = (EditText)findViewById(R.id.edtcount);

        if (null != frameCountEditText) {
            frameCountEditText.setText("0");
            cfgFileParams.put(commonParamsNames[USF_FRAME_COUNT], "0");
            setOnKeyListener(frameCountEditText, commonParamsNames[USF_FRAME_COUNT]);
            privateParams.put(commonParamsNames[USF_FRAME_COUNT], frameCountEditText);
            frameCountEditText.setEnabled(false);
        }

        frameFileFormatSpinner = (Spinner)findViewById(R.id.spinnerformat);
        String[] fileFormats = getResources().getStringArray(R.array.files_format);
        ArrayAdapter<String> adapter = new ArrayAdapter<String>(this,
                android.R.layout.simple_spinner_item, fileFormats);
        adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        if (null != frameFileFormatSpinner) {
            frameFileFormatSpinner.setAdapter(adapter);
            frameFileFormatSpinner.setOnItemSelectedListener(new FileFormetItemSelectedListener());
            frameFileFormatSpinner.setEnabled(false);

            /*int spinnerPosition = adapter.getPosition(cfgFileName);
              chooseFileSpinner.setSelection(spinnerPosition);*/
        }
        privateParams.put(commonParamsNames[USF_FRAME_FILE_FORMAT], frameFileFormatSpinner);

        Log.d(getTag(), "Finished to set the common params");
    }

    /*===========================================================================
    FUNCTION:  onStart
    ===========================================================================*/
    /**
     * onStart() function start the handler.
     */
    @Override
    protected void onStart() {
        super.onStart();
        updateUI();
    }

    /*===========================================================================
    FUNCTION:  onPause
    ===========================================================================*/
    /**
     * onPause() function pause the handler.
     */
    @Override
    protected void onPause() {
        super.onPause();
        redrawHandler.removeMessages(0);
    }

    /*===========================================================================
    FUNCTION:  setDisplay
    ===========================================================================*/
    /**
     * setDisplay() function sets the display according to a
     * parameter read from the cfg.
     * This function is used internally from the function
     * readCfgFileAndSetDisp().
     * @param paramName the name of the parameter to update
     * @param paramValue the value of the parameter to update
     */
    protected void setDisplay(String paramName, String paramValue) {
        for (String privateParamName : privateParams.keySet()) {
            if (paramName.equals(privateParamName)) {

                if (privateParams.get(privateParamName).getClass() == EditText.class) {
                    ((EditText)privateParams.get(privateParamName)).setText(paramValue);
                    return;
                } else if (privateParams.get(privateParamName).getClass() == Spinner.class) {
                    // Check whether the given value could be interpreted as int
                    int intVal = isNumeric(paramValue);
                    if (-1 != intVal) { // The given number could be interpreted
                        // as int.
                        ((Spinner)privateParams.get(privateParamName)).setSelection(intVal);
                        return;
                    }
                    ArrayAdapter<?> myAdap = (ArrayAdapter<?>)((Spinner)privateParams
                            .get(privateParamName)).getAdapter();
                    int spinnerPosition;
                    for (spinnerPosition = 0; spinnerPosition < myAdap.getCount(); spinnerPosition++) {
                        if (myAdap.getItem(spinnerPosition).toString().equals(paramValue)) {
                            ((Spinner)privateParams.get(privateParamName))
                            .setSelection(spinnerPosition);
                            return;
                        }
                    }
                } else if (privateParams.get(privateParamName).getClass() == CheckBox.class) {
                    if (paramValue.equals("1")) {
                        ((CheckBox)privateParams.get(privateParamName)).setChecked(true);
                    } else {
                        ((CheckBox)privateParams.get(privateParamName)).setChecked(false);
                    }
                    return;
                } else {
                    Log.w(getTag(), "Read cfg file: param type mismatch");
                }

            }
        }
    }

    /*===========================================================================
    FUNCTION:  isNumeric
    ===========================================================================*/
    /**
     * This function checks whether the given string value could be
     * parsed to int. If so, it returns the value, else returns -1;
     * Note: the function assumes the parsed number would not be -1.
     *
     * @param value to be parsed
     * @return int -1 - error
     *         else - the number parsed
     */
    public static int isNumeric(String value) {
        int i;
        try {
            i = Integer.parseInt(value);
        } catch (NumberFormatException nfe) {
            return -1;
        }
        return i;
    }

    /*===========================================================================
    FUNCTION:  restoreCfgFile
    ===========================================================================*/
    /**
     * This function restores the cfg from the backup cfg file.
     *
     * @return boolean true - if the function succeeded
     *         false - if the function failed.
     */
    private void restoreCfgFile() {
        Log.w(getTag(), "Cfg file is corrupted, using backup file");
        File corrupted = new File(mDaemonCfgFilePicker.getCfgFileLocation());
        File backup = new File(mDaemonCfgFilePicker.getCfgFileLocation() + "_backup");

        corrupted.delete();

        if (true != backup.renameTo(corrupted)) {
            Log.e(getTag(), "Cannot use backup cfg file");
        }
    }

    /*===========================================================================
    FUNCTION:  readCfgFileAndSetDisp
    ===========================================================================*/
    /**
     * readCfgFileAndSetDisp() function reads the cfg file
     * line after line. In case setDisp parameter is set to true it
     * also sets the UI according to the file.
     * The function is called before any of the cfg file parameters
     * are updated.
     * @param setDisp in case true - it sets the UI
     *            in case false - it does not set the UI
     */
    protected void readCfgFileAndSetDisp(boolean setDisp) {
        if (false == isFileValid(mDaemonCfgFilePicker.getCfgFileLocation())) {
            restoreCfgFile();
        }

        try {
            FileInputStream fstream = new FileInputStream(mDaemonCfgFilePicker.getCfgFileLocation());
            DataInputStream in = new DataInputStream(fstream);
            BufferedReader br = new BufferedReader(new InputStreamReader(in));

            String strLine;
            String paramName;
            String paramValue;
            int i = 0;

            while (null != (strLine = br.readLine())) {
                // This line is a comment
                if (UltrasoundUtil.isCommentLine(strLine)) {
                    cfgFile.put(i, strLine);
                    i++;
                    continue;
                }
                StringTokenizer st = new StringTokenizer(strLine);
                // This line is param and value - need to update cfgFileParams
                // dict
                if (st.countTokens() >= 2) {
                    paramName = st.nextToken();
                    cfgFile.put(i, paramName);
                    i++;
                    // The value is all what is in the '[] chars'
                    int sIndex = strLine.indexOf('[') + 1;
                    int lIndex = strLine.indexOf(']');
                    paramValue = strLine.substring(sIndex, lIndex);
                    if (true == setDisp) {
                        setDisplay(paramName, paramValue);
                    }
                    cfgFileParams.put(paramName, paramValue);
                    continue;
                }
                Log.e(getTag(), "Can't parse line: " + strLine);
            }
            in.close();

            if (true == setDisp) {
                if (Integer.valueOf(frameCountEditText.getText().toString()) > 0) {
                    appendTimestampCheckbox.setEnabled(true);
                    debugModeCheckbox.setChecked(true);
                    frameFileEditText.setEnabled(true);
                    frameCountEditText.setEnabled(true);
                    frameFileFormatSpinner.setEnabled(true);
                } else {
                    appendTimestampCheckbox.setEnabled(false);
                    debugModeCheckbox.setChecked(false);
                    frameFileEditText.setEnabled(false);
                    frameCountEditText.setEnabled(false);
                    frameFileFormatSpinner.setEnabled(false);
                }
            }
        } catch (Exception e) {
            Log.e(getTag(), "Error: " + e.getMessage());
        }
    }

    /*===========================================================================
    FUNCTION:  isFileValid
    ===========================================================================*/
    /**
     * isFileValid() function checks whether the given file is
     * valid by checking it exists and that its length is longer
     * than 0.
     *
     * @param filename the file name
     * @return boolean true - the file is valid
     *         false - the file is not valid.
     */
    private boolean isFileValid(String filename) {
        File f = new File(filename);
        long fileLength = 0;
        if (f.exists() && f.isFile()) {
            fileLength = f.length();
        }
        return fileLength == 0 ? false : true;
    }

    /*===========================================================================
    FUNCTION:  updateCfgFile
    ===========================================================================*/
    /**
     * updateCfgFile() function writes the cfg file
     * with update values.
     */
    protected void updateCfgFile() {
        try {
            // Write cfg file content to a new filename for backup purposes
            FileWriter fstream = new FileWriter(mDaemonCfgFilePicker.getCfgFileLocation() + "_new");
            BufferedWriter out = new BufferedWriter(fstream);
            String paramName;
            Set<String> writtenParams = new HashSet<String>();

            Collection<?> c = cfgFile.values();
            Iterator<?> itr = c.iterator();

            while (itr.hasNext()) {
                paramName = itr.next().toString();
                // This line is a comment
                if (UltrasoundUtil.isCommentLine(paramName)) {
                    out.write(paramName);
                }
                // This line is param and value - find the value in
                // cfgFileParams dict
                else {
                    out.write(paramName + " [" + cfgFileParams.get(paramName) + "]");
                    writtenParams.add(paramName);
                }
                out.newLine();
            }

            writeMissingCfgParams(out, writtenParams, cfgFileParams);

            out.close();
        } catch (Exception e) {
            Log.e(getTag(), "Error: " + e.getMessage());
        }
        useNewFileNBackupOld(mDaemonCfgFilePicker.getCfgFileLocation());
    }

    /*===========================================================================
    FUNCTION:  writeMissingCfgParams
    ===========================================================================*/
    /**
     * This function finds and writes cfg file parameters that do
     * not exist in the current cfg file to the given cfg file
     * buffer.
     *
     * @param out output buffer to write to
     * @param writtenParams cfg parameters already written to the
     *            cfg file buffer.
     * @param cfgFileParams all cfg file parameters to be written
     */
    void writeMissingCfgParams(BufferedWriter out, Set<String> writtenParams,
            Map<String, String> cfgFileParams) {
        Set<String> symmDiff = getSymmetricDifference(writtenParams, cfgFileParams.keySet());
        try {
            for (String paramName : symmDiff) {
                out.write(paramName + " [" + cfgFileParams.get(paramName) + "]");
                out.newLine();
            }
        } catch (Exception e) {
            Log.e(getTag(), "Error: " + e.getMessage());
        }
    }

    /*===========================================================================
    FUNCTION:  getSymmetricDifference
    ===========================================================================*/
    /**
     * This function returns the symmetric difference between two
     * given sets.
     *
     * @param T - type of the given set
     * @param set1 first set to return it's symmetric difference
     * @param set1 second et to return it's symmetric difference
     * @return Set<T> symmetric difference of the given sets
     */
    public <T> Set<T> getSymmetricDifference(Set<T> set1, Set<T> set2) {
        Set<T> symmDiff = new HashSet<T>(set1);
        symmDiff.addAll(set2); // Cotnains the union

        Set<T> intersection = new HashSet<T>(set1);
        intersection.retainAll(set2); // Contains intersection

        symmDiff.removeAll(intersection); // Contains symmetric difference

        return symmDiff;
    }

    /*===========================================================================
    FUNCTION:  updateAutoStartFile
    ===========================================================================*/
    /**
     * updateAutoStartFile() function adds to the auto_start cfg file
     * the name of the current daemon if it needs to be auto_started,
     * else it removes the daemon name from the file.
     * If the checkAutoStartStatus is true then no update to the file
     * is made and the function returns true if the daemon is in the
     * file, else it return false.
     */
    private boolean updateAutoStartFile(boolean startOnReboot, boolean checkAutoStartStatus) {
        try {
            File f = new File(autoStartFile);
            if (!f.exists()) {
                f.createNewFile();
            }

            FileInputStream fistream = new FileInputStream(autoStartFile);
            DataInputStream in = new DataInputStream(fistream);
            BufferedReader br = new BufferedReader(new InputStreamReader(in));
            String strLine;
            String daemon;
            StringBuilder fileBufferBuilder = new StringBuilder("");
            String fileBuffer;
            StringBuilder updateFileBufferBuilder = new StringBuilder("");
            String updateFileBuffer;
            boolean daemonInFile = false;

            // Read the auto_start file to a buffer
            while (null != (strLine = br.readLine())) {
                fileBufferBuilder.append(strLine + " ");
            }
            in.close();

            fileBuffer = fileBufferBuilder.toString();

            // Parse the file and remove the current daemon
            StringTokenizer st = new StringTokenizer(fileBuffer);
            while (st.hasMoreTokens()) {
                daemon = st.nextToken();
                if (!daemon.equals(getDaemonName())) {
                    updateFileBufferBuilder.append(daemon + " ");
                } else {
                    daemonInFile = true;
                }
            }

            // Add the daemon if needed
            if (startOnReboot) {
                updateFileBufferBuilder.append(getDaemonName() + " ");
            }

            if (checkAutoStartStatus) {
                return daemonInFile;
            }

            updateFileBuffer = updateFileBufferBuilder.toString();

            // Update the auto_start file
            FileWriter fwstream = new FileWriter(autoStartFile);
            BufferedWriter out = new BufferedWriter(fwstream);
            out.write(updateFileBuffer);
            out.close();
        } catch (Exception e) {
            Log.e(getTag(), "Error: " + e.getMessage());
        }

        return true;
    }

    /*===========================================================================
    FUNCTION:  setOnKeyListener
    ===========================================================================*/
    /**
     * setOnKeyListener() function updates the cfgFileParams every
     * time the user updates an editText from the UI.
     */
    protected void setOnKeyListener(final EditText editText, final String nameOfParam) {
        editText.setOnKeyListener(new OnKeyListener() {
            @Override
            public boolean onKey(View v, int keyCode, KeyEvent event) {
                // If the event is a key-down event on the "enter" button
                if ((event.getAction() == KeyEvent.ACTION_DOWN)
                        && (keyCode == KeyEvent.KEYCODE_ENTER)) {
                    readCfgFileAndSetDisp(false);
                    cfgFileParams.put(nameOfParam, editText.getText().toString());
                    updateCfgFile();
                    return true;
                }
                return false;
            }
        });
    }

    /*===========================================================================
    FUNCTION:  getXmlFileName
    ===========================================================================*/
    /**
     * getXmlFileName() function returns the name of the xml file of
     * this activity (the xml that define the UI).
     */
    protected abstract String getXmlFileName();

    /*===========================================================================
    FUNCTION:  getThisFileName
    ===========================================================================*/
    /**
     * getThisFileName() function returns this file name.
     */
    protected abstract String getThisFileName();

    /*===========================================================================
    FUNCTION:  getTag
    ===========================================================================*/
    /**
     * getTag() function returns this activity tag.
     */
    protected abstract String getTag();

    /*===========================================================================
    FUNCTION:  getDaemonName
    ===========================================================================*/
    /**
     * getDaemonName() function returns this daemon name.
     */
    protected abstract String getDaemonName();

    /*===========================================================================
    FUNCTION:  getCfgLinkFileLocation
    ===========================================================================*/
    /**
     * getCfgLinkFileLocation() function returns the name of the link
     * file to the cfg file of this activity.
     */
    protected abstract String getCfgLinkFileLocation();

    /*===========================================================================
    FUNCTION:  getCfgFilesDir
    ===========================================================================*/
    /**
     * getCfgFilesDir() function returns the path to the cfg dir of
     * this activity.
     */
    protected abstract String getCfgFilesDir();

    /*===========================================================================
    FUNCTION:  getCfgExpressionDir
    ===========================================================================*/
    /**
     * getCfgExpressionDir() function returns the general expression
     * path to the cfg dir of this activity.
     */
    protected abstract String getCfgExpressionDir();

    /*===========================================================================
    FUNCTION:  getRecFilesDir
    ===========================================================================*/
    /**
     * getRecFilesDir() function returns the path to the rec dir of
     * this activity.
     */
    protected abstract String getRecFilesDir();

    /*===========================================================================
    FUNCTION:  getDefaultCfgFileName
    ===========================================================================*/
    /**
     * getDefaultCfgFileName() function returns the name of default
     * cfg file of this activity.
     */
    protected abstract String getDefaultCfgFileName();

    /*===========================================================================
    FUNCTION:  getDSPVerFile
    ===========================================================================*/
    /**
     * getDSPVerFile() function returns the name of the file
     * containing the DSP version.
     */
    protected String getDSPVerFile() {
        // Currently need this only in epos and therefore the implementation
        // of the function here and not abstract.
        return "";
    }

    /*===========================================================================
    FUNCTION:  updateDSPVersion
    ===========================================================================*/
    /**
     * updateDSPVersion() function update the DSP version number of
     * the daemon lib if any or write "Stub" for stub lib.
     */
    protected void updateDSPVersion() {
        // Do nothing.
        // Currently this update is needed only in epos and therefore the
        // function
        // is implemented and not abstract.
    }

    /*===========================================================================
    FUNCTION:  clearDSPVersion
    ===========================================================================*/
    /**
     * clearDSPVersion() function clear DSP version number of
     * the daemon lib if any.
     */
    protected void clearDSPVersion() {
        // Do nothing.
        // Currently this update is needed only in epos and therefore the
        // function
        // is implemented and not abstract.
    }

    /*===========================================================================
    FUNCTION:  getDSPVersion
    ===========================================================================*/
    /**
     * getDSPVersion() function reads the DSP version from the
     * DSPVerFile and returns the version.
     */
    protected String getDSPVersion() {
        String strLine = null;
        try {
            FileInputStream fstream = new FileInputStream(getDSPVerFile());
            DataInputStream in = new DataInputStream(fstream);
            BufferedReader br = new BufferedReader(new InputStreamReader(in));
            strLine = br.readLine();
            in.close();
        } catch (Exception e) {
            Log.e(getTag(), "Error: " + e.getMessage());
            return strLine;
        }
        return strLine;
    }

}

