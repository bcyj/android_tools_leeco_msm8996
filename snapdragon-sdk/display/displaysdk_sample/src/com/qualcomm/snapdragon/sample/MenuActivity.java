/* ====================================================================
 * Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * QTI Proprietary and Confidential.
 * =====================================================================
 * @file MenuActivity.java
 *
 */
package com.qualcomm.snapdragon.sample;

import java.io.IOException;
import java.io.InputStream;

import android.app.Activity;
import android.content.Intent;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.Toast;

public class MenuActivity extends Activity {

    private static String TAG = "ColorMgrSample-Menu";

    private final static String PM_INTENT = "MODE_MANAGER";
    public final static String MODE_MODIFY_INTENT = "MODE_MODIFY";

    public final static String MODIFY_MODE_NAME = "com.qualcomm.snapdragon.sample.ModifyModeName";
    public final static String MODIFY_MODE_ID = "com.qualcomm.snapdragon.sample.ModifyModeID";
    public static Bitmap bitmap = null;

    Button displayfeatures, modeman;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.firstscreen);

        displayfeatures = new Button(this);
        displayfeatures = (Button) findViewById(R.id.btndisplayfeatures);
        displayfeatures.setOnClickListener(new OnClickListener() {

            @Override
            public void onClick(View v) {
                launchFeatureActivity(MODE_MODIFY_INTENT, null, -1);
            }
        });

        modeman = new Button(this);
        modeman = (Button) findViewById(R.id.btnmodeman);
        modeman.setOnClickListener(new OnClickListener() {

            @Override
            public void onClick(View v) {
                launchModeManager(PM_INTENT, PM_INTENT);
            }
        });

        // Populate the image with a default image
        try {
            InputStream defaultImage = getAssets().open(
                    getResources().getString(R.string.default_image));
            MenuActivity.bitmap = BitmapFactory.decodeStream(defaultImage);
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    void launchModeManager(String intentName, String title) {
        Log.i(TAG, "Launching activity for mode manager");
        Intent intent = new Intent();
        intent.setAction(intentName);
        intent.putExtra("title", title);
        try {
            startActivity(intent);
        } catch (Throwable t) {
            Toast.makeText(this, "Error starting " + intentName,
                    Toast.LENGTH_SHORT).show();
        }
    }

    void launchFeatureActivity(String intentName, String modeName, int modeID) {
        try {
            Log.i(TAG, "Launching activity for display feature tuning");
            Intent intent = new Intent();
            intent.setAction(intentName);
            intent.putExtra(MODIFY_MODE_NAME, modeName);
            intent.putExtra(MODIFY_MODE_ID, modeID);
            startActivityForResult(intent, 0);
        } catch (Throwable t) {
            // Toast.makeText(this, "Error starting " + intentName,
            // Toast.LENGTH_SHORT).show();
        }
    }

}
