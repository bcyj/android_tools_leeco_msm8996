/******************************************************************************
 * ---------------------------------------------------------------------------
 *  Copyright (C) 2013-2014 Qualcomm Technologies, Inc.
 *  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
 *  ---------------------------------------------------------------------------
 *******************************************************************************/

package com.qti.ultrasound.penpairing;
/******************************************************************************
 * ---------------------------------------------------------------------------
 *  Copyright (C) 2014 Qualcomm Technologies, Inc.
 *  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
 *  ---------------------------------------------------------------------------
 *******************************************************************************/
import android.app.Activity;
import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.NumberPicker;

public class PairingActivity extends Activity implements OnClickListener {

    public static final String KEY_PEN_ID = "penId";

    public static final int MIN_SERIES_ID = 1;

    public static final int MAX_SERIES_ID = 35;

    private String mPenType = "";

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_pairing);

        Button buttonManual = (Button)findViewById(R.id.manual_pairing_button);
        Button buttonSemiAutomatic = (Button)findViewById(R.id.semiautomatic_pairing_button);

        buttonManual.setOnClickListener(this);
        buttonSemiAutomatic.setOnClickListener(this);

        Bundle b = getIntent().getExtras();
        mPenType = b.getString("penType");
    }

    @Override
    public void onClick(View view) {
        Button b = (Button)view;
        int buttonId = b.getId();

        switch (buttonId) {
            case (R.id.manual_pairing_button):
                LayoutInflater inflater = (LayoutInflater)getSystemService(Context.LAYOUT_INFLATER_SERVICE);
            View v = inflater.inflate(R.layout.activity_manual, null);
            final NumberPicker picker = (NumberPicker)v.findViewById(R.id.series_picker);
            final PairingDbHelper pairingDbHelper = new PairingDbHelper(this);

            picker.setDescendantFocusability(ViewGroup.FOCUS_BLOCK_DESCENDANTS);
            picker.setMaxValue(MAX_SERIES_ID);
            picker.setMinValue(MIN_SERIES_ID);

            AlertDialog.Builder alert = new AlertDialog.Builder(this);
            alert.setTitle("Manual Pairing").setView(v)
            .setPositiveButton("OK", new DialogInterface.OnClickListener() {
                @Override
                public void onClick(DialogInterface dialog, int which) {
                    pairingDbHelper.addPen(picker.getValue(), mPenType);
                }
            }).setNegativeButton("Cancel", new DialogInterface.OnClickListener() {
                @Override
                public void onClick(DialogInterface dialog, int which) {
                }
            });
            alert.show();
            break;
            case (R.id.semiautomatic_pairing_button):
                Intent i = new Intent(this, SemiAutomaticActivity.class);
                i.putExtra("penType", mPenType);
            startActivity(i);
            break;
        }
    }
}
