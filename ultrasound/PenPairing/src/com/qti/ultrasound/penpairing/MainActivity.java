/******************************************************************************
 * ---------------------------------------------------------------------------
 *  Copyright (c) 2013-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 *  Qualcomm Technologies Proprietary and Confidential.
 *  ---------------------------------------------------------------------------
 *******************************************************************************/

package com.qti.ultrasound.penpairing;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.res.AssetManager;
import android.os.Bundle;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.Spinner;
import android.widget.Toast;

import java.io.BufferedReader;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.util.ArrayList;

public class MainActivity extends Activity implements OnClickListener {

    public static final int PAIRING = 0;

    public static final int PAIRED_PENS = 1;

    public static final int NUM_OF_PAIRING_TYPES = 2;

    private static final String PEN_TYPE = "pen_type";

    private static final String PEN_PRODUCT_FILE_NAME = "product_calib";

    private static final String PEN_UNIT_FILE_NAME = "unit_calib";

    private static final String CALIB_FILE_EXTENSION = ".dat";

    private static final String PAIRING_DATA_DIR = "/data/usf/pairing/";

    private static final String PEN_SELECTED_SERIES_DIR_LINK = PAIRING_DATA_DIR + PEN_TYPE;

    private static final String PEN_SELECTED_PRODUCT_LINK = PAIRING_DATA_DIR + PEN_PRODUCT_FILE_NAME
            + CALIB_FILE_EXTENSION;

    private static final String PEN_SELECTED_UNIT_LINK = PAIRING_DATA_DIR + PEN_UNIT_FILE_NAME
            + CALIB_FILE_EXTENSION;

    private static final String PEN_SERIES_PATH = "/persist/usf/pen_pairing/";

    private static final String PEN_PERSIST_PATH = "/persist/usf/epos/";

    private Context context;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        context = this;
        setContentView(R.layout.activity_main);

        Button pairedButton = (Button)findViewById(R.id.main_paired_pens_button);
        Button pairingButton = (Button)findViewById(R.id.main_pairing_button);
        Button calibrationButton = (Button)findViewById(R.id.main_sw_calibration_button);
        Button testerButton = (Button)findViewById(R.id.main_tester_button);

        pairedButton.setOnClickListener(this);
        pairingButton.setOnClickListener(this);
        calibrationButton.setOnClickListener(this);
        testerButton.setOnClickListener(this);
    }

    @Override
    public void onClick(View v) {
        switch (v.getId()) {
            case R.id.main_paired_pens_button:
                startActivity(new Intent(this, PairedPensActivity.class));
                break;
            case R.id.main_pairing_button:
                choosePenRefAndStartActivity("Pen Pairing", PairingActivity.class);
                break;
            case R.id.main_sw_calibration_button:
                choosePenRefAndStartActivity("Software Calibration", SWCalibrationActivity.class);
                break;
            case R.id.main_tester_button:
                choosePenRefAndStartActivity("Service Center Tester", TesterActivity.class);
                break;
            default:
                Log.wtf(this.toString(), "View id not possible");
                finish();
                break;
        }
    }

    private void choosePenRefAndStartActivity(String title, final Class<?> cls) {
        LayoutInflater inflater = (LayoutInflater)getSystemService(Context.LAYOUT_INFLATER_SERVICE);
        final View penPickerView = inflater.inflate(R.layout.pen_picker, null);

        setPenSpinner(penPickerView);

        AlertDialog.Builder alert = new AlertDialog.Builder(this);
        alert.setTitle(title).setView(penPickerView)
        .setPositiveButton("OK", new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                removeCalibPacketLinks();
                final Spinner choosePenSpinner = (Spinner)penPickerView.findViewById(R.id.pen_spinner);
                String penType = choosePenSpinner.getSelectedItem().toString().toLowerCase();
                String productFile = PEN_PERSIST_PATH + PEN_PRODUCT_FILE_NAME + "_" + penType + ".dat";
                try {
                    Runtime.getRuntime().exec("ln -s " + productFile + " " + PEN_SELECTED_PRODUCT_LINK);
                } catch (IOException e) {
                    Log.e(MainActivity.class.getName(), "Failed creating link to product file "
                          + productFile);
                }
                String unitFile = PEN_PERSIST_PATH + PEN_UNIT_FILE_NAME + "_" + penType + ".dat";
                try {
                    Runtime.getRuntime().exec("ln -s " + unitFile + " " + PEN_SELECTED_UNIT_LINK);
                } catch (IOException e) {
                    Log.e(MainActivity.class.getName(), "Failed creating link to unit file "
                          + unitFile);
                }
                String seriesDir = PEN_SERIES_PATH + penType;
                try {
                    Runtime.getRuntime().exec("ln -s " + seriesDir + " " + PEN_SELECTED_SERIES_DIR_LINK);
                } catch (IOException e) {
                    Log.e(MainActivity.class.getName(), "Failed creating link to series directory "
                          + seriesDir);
                }
                Intent intent = new Intent(context, cls);
                intent.putExtra("penType", penType);
                startActivity(intent);
            }
        }).setNegativeButton("Cancel", new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
            }
        });
        alert.show();
    }

    private void setPenSpinner(View penPickerView) {
        final Spinner choosePenSpinner = (Spinner)penPickerView.findViewById(R.id.pen_spinner);
        if (null != choosePenSpinner) {
            ArrayAdapter<String> choosePenAdapter = new ArrayAdapter<String>(context,
                    android.R.layout.simple_spinner_item, getPenNames());
            choosePenAdapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
            choosePenSpinner.setAdapter(choosePenAdapter);
        }
        choosePenSpinner.setDescendantFocusability(ViewGroup.FOCUS_BLOCK_DESCENDANTS);
    }

    private String[] getPenNames() {
        AssetManager assetManager = this.getAssets();
        String[] assetsFolderslist = null;
        try {
            assetsFolderslist = assetManager.list("");
        } catch (IOException e) {
            e.printStackTrace();
        }
        ArrayList<String> penNames = new ArrayList<String>();

        for (String s : assetsFolderslist) {
            String penName = null;
            try {
                penName = readFromInputStream(assetManager.open(s + "/pen_name.txt"));
            } catch (IOException e) {
            }
            if (null != penName) {
                penNames.add(penName);
            }
        }
        if (penNames.size() == 0) {
            Toast.makeText(this, "No pens available", Toast.LENGTH_SHORT).show();
            finish();
        }
        return penNames.toArray(new String[0]);
    }

    private String readFromInputStream(InputStream fileStream) {
        String fileContent = null;
        try {
            if (fileStream != null) {
                BufferedReader br = new BufferedReader(new InputStreamReader(fileStream));
                String readString = "";
                StringBuilder sb = new StringBuilder();

                while ((readString = br.readLine()) != null) {
                    sb.append(readString);
                }

                fileStream.close();
                fileContent = sb.toString();
            }
        } catch (FileNotFoundException e) {
            Log.e(this.toString(), "File not found: " + e.toString());
        } catch (IOException e) {
            Log.e(this.toString(), "Cannot read file: " + e.toString());
        }
        return fileContent;
    }

    private static void removeCalibPacketLinks() {
        try {
            Runtime.getRuntime().exec("rm -rf " + PEN_SELECTED_SERIES_DIR_LINK);
        } catch (IOException e) {
            Log.e(PairedPensActivity.class.getName(), "Failed removing " + PEN_SELECTED_SERIES_DIR_LINK);
        }
        try {
            Runtime.getRuntime().exec("rm -rf " + PEN_SELECTED_PRODUCT_LINK);
        } catch (IOException e) {
            Log.e(PairedPensActivity.class.getName(), "Failed removing " + PEN_SELECTED_PRODUCT_LINK);
        }
        try {
            Runtime.getRuntime().exec("rm -rf " + PEN_SELECTED_UNIT_LINK);
        } catch (IOException e) {
            Log.e(PairedPensActivity.class.getName(), "Failed removing " + PEN_SELECTED_UNIT_LINK);
        }
    }
}
