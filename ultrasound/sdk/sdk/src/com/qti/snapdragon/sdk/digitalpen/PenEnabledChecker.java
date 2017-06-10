/**
 * Copyright (c) 2014 Qualcomm Technologies, Inc.All Rights Reserved.
 * Qualcomm Technologies Confidential and Proprietary.
 */

package com.qti.snapdragon.sdk.digitalpen;

import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.DialogInterface.OnClickListener;
import android.content.Intent;

import com.qti.snapdragon.sdk.digitalpen.DigitalPenManager.Feature;

/**
 * Helper class for checking if Snapdragon Digital Pen is enabled, launching the
 * settings if not.
 */
public class PenEnabledChecker {

    /**
     * Checks if Snapdragon Digital Pen is enabled. If not, presents a dialog
     * which asks the user of they would like to launch the settings app to
     * enable the pen.
     *
     * @param context a context for creating dialogs and starting the settings
     *            activity
     */
    public static void checkEnabledAndLaunchSettings(Context context) {
        if (!DigitalPenManager.isFeatureSupported(Feature.DIGITAL_PEN_ENABLED)) {
            AlertDialog.Builder builder = new AlertDialog.Builder(context);
            builder.setTitle("Snapdragon Digital Pen Not Enabled");
            builder.setMessage("This application requires the digital pen to be enabled.\n\n" +
                    "Press OK to launch Snapdragon Digital Pen Settings.");
            final Context launchingContext = context;
            builder.setPositiveButton("OK", new OnClickListener() {

                @Override
                public void onClick(DialogInterface dialog, int which) {
                    launchingContext.startActivity(new Intent(
                            DigitalPenManager.ACTION_DIGITAL_PEN_SETTINGS));
                }
            });
            builder.setNegativeButton("Cancel", null);
            builder.show();
        }
    }

}
