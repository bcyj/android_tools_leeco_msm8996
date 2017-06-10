/*************************************************************************
        Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
                   Qualcomm Technologies Proprietary and Confidential.
 **************************************************************************/

package com.android.iperfapp;

import java.io.File;
import java.util.ArrayList;
import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.os.Bundle;
import android.text.method.ScrollingMovementMethod;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemSelectedListener;
import android.widget.ArrayAdapter;
import android.widget.EditText;
import android.widget.ScrollView;
import android.widget.Spinner;
import android.widget.TextView;
import android.widget.ToggleButton;

public class IPerfAppActivity extends Activity {
    private static final int DIALOG_ABOUT_ID = 1;

    private static final int DIALOG_SAVED_ID = 2;

    private static final String IPERF_STRING = "/data/data/com.android.iperfapp/iperf";

    /** Called when the activity is first created. */
    private TextView resultView;

    private TextView deviceIP;

    private ToggleButton runButton;

    private ToggleButton dispButton;

    private ToggleButton cleanButton;

    private ToggleButton fileButton;

    private EditText iPerfEditText;

    private ScrollView sv;

    IPerfConfig ipc = null;

    boolean clickRun = false;

    IPerfProcess pp = null;

    String deviceipaddr = null;

    IPerfUtils pu = null;

    int runNum = 1;

    static boolean firstLaunch = false;

    static boolean dispState = true;

    static boolean fileState = true;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main);
        this.findAllViewsById();
        if (readAssetText() == false) {
            AlertDialog.Builder alert = new AlertDialog.Builder(this);
            alert.setTitle("Alert!")
                    .setMessage(
                            "iperf has not been found or installed properly!"
                           + " Install it and give it execute permissions "
                           + "to use iperfApp properly. Restart app after installation.")
                    .setNeutralButton("OK", null)
                    .show();

        }
        runButton.setOnClickListener(runButtonListener);
        dispButton.setOnClickListener(dispButtonListener);
        cleanButton.setOnClickListener(cleanButtonListener);
        fileButton.setOnClickListener(fileButtonListener);
        iPerfEditText.setText("-s -u -i2");
        pu = new IPerfUtils();
        deviceipaddr = pu.getLocalIpAddress();
        ipc = new IPerfConfig();
        deviceIP.setText("Device IP Address is: " + deviceipaddr);
        firstLaunch = false;
        sv.post(new Runnable() {
            public void run() {
                sv.fullScroll(View.FOCUS_DOWN);
            }
        });
    }

    private boolean readAssetText() {
       boolean iperfasset = false; 
       try {
            File f = new File(IPERF_STRING);
            if (f.exists() && f.canExecute()) {
                iperfasset = true;
            }
        } catch (Exception e) {

        }
        return iperfasset;
    }

    private void findAllViewsById() {
        resultView = (TextView)findViewById(R.id.search_type_text_view);
        resultView.setMovementMethod(new ScrollingMovementMethod());
        deviceIP = (TextView)findViewById(R.id.deviceip);
        runButton = (ToggleButton)findViewById(R.id.ok);
        dispButton = (ToggleButton)findViewById(R.id.Display);
        cleanButton = (ToggleButton)findViewById(R.id.clean);
        fileButton = (ToggleButton)findViewById(R.id.File);
        iPerfEditText = (EditText)findViewById(R.id.entry);
        sv = (ScrollView)findViewById(R.id.scroller);
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        MenuInflater inflater = getMenuInflater();
        inflater.inflate(R.layout.menu, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        // Handle item selection
        switch (item.getItemId()) {
            case R.id.about:
                showDialog(DIALOG_ABOUT_ID);
                return true;
            case R.id.saved:
                showDialog(DIALOG_SAVED_ID);
                return true;
            default:
                return super.onOptionsItemSelected(item);
        }
    }

    protected void onPause() {
        super.onPause();
    }

    @Override
    protected void onDestroy() {
        if (pp != null) {
            pp.CancelTask();
        }
        super.onStop();
    }

    protected void onRestart() {
        super.onRestart();
    }

    @Override
    @SuppressWarnings("unchecked")
    protected Dialog onCreateDialog(int id) {
        Dialog dialog = null;
        switch (id) {
            case DIALOG_ABOUT_ID:
                AlertDialog.Builder builder1 = new AlertDialog.Builder(this);
                builder1.setTitle("About iPerfApp");
                builder1.setMessage(R.string.describe);
                dialog = builder1.create();
                break;
            case DIALOG_SAVED_ID:
                dialog = new Dialog(this);
                dialog.setContentView(R.layout.spinner);
                dialog.setTitle("iPerf Commands");
                Spinner spinner = (Spinner)dialog.findViewById(R.id.spin);
                ArrayList<String> al = new ArrayList<String>();
                if (pu != null) {
                    al = pu.getChoices();
                    if (al.size() == 0) {
                        al.add("There is no input file.");
                    }
                } else
                    al.add("Problem with input file.");

                ArrayAdapter<String> adapter = new ArrayAdapter<String>(this,
                        android.R.layout.simple_spinner_item, al);
                adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
                spinner.setAdapter(adapter);
                spinner.setOnItemSelectedListener(new MyOnItemSelectedListener());
                break;
            default:
                dialog = null;
        }
        return dialog;
    }

    private OnClickListener runButtonListener = new OnClickListener() {
        public void onClick(View v) {
            if (clickRun == true) {
                cancelTask("\n");
            } else {
                String params = iPerfEditText.getText().toString().trim();
                if (params != null) {
                    resultView.setText("Starting iPerf tool on Device...\n");
                    pp = new IPerfProcess(params, resultView, dispState, fileState);
                    clickRun = true;
                    runButton.setText("Stop");
                }

            }
        }
    };

    private OnClickListener dispButtonListener = new OnClickListener() {
        public void onClick(View v) {
            dispState = dispButton.isChecked();
            if (pp != null)
                pp.SetDispState(dispState);
        }
    };

    private OnClickListener fileButtonListener = new OnClickListener() {
        public void onClick(View v) {
            fileState = fileButton.isChecked();
            if (pp != null)
                pp.SetFileState(fileState);
        }
    };

    private OnClickListener cleanButtonListener = new OnClickListener() {
        public void onClick(View v) {
            // Cleaning up output files
            if (pu != null) {
                pu.cleanUpMyDepot();
                if (clickRun == true) {
                    if (pp != null) {
                        cancelTask("Stopped running iPerf while cleaning up takes place.\n");
                    }
                }
            }
        }
    };

    public class MyOnItemSelectedListener implements OnItemSelectedListener {

        public void onItemSelected(AdapterView<?> parent, View view, int pos, long id) {
            iPerfEditText.setText(parent.getSelectedItem().toString());
        }

        public void onNothingSelected(AdapterView parent) {
            // Do nothing.
        }
    }

    protected void cancelTask(String text) {
        runButton.setText("Run");
        clickRun = false;
        pp.CancelTask();
        resultView.setText(text);
        firstLaunch = true;
    }
}
