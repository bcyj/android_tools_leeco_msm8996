/**
 * Copyright (c) 2013 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package oms.drmservice;

import java.io.IOException;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.util.Log;

import java.io.File;
import java.io.FileInputStream;

import android.drm.mobile1.DrmException;

//import android.provider.Telephony.Sms.Intents.WAP_PUSH_RECEIVED_ACTION;

import java.io.*;
import java.util.*;
import android.drm.DrmManagerClient;
import android.drm.DrmRights;

public class DrmReceiver extends BroadcastReceiver {

    private final static String TAG = "DRMReceiver xxxxxxxxxxxxxxxxxxz";
    /**
     * The "application/vnd.oma.drm.rights+xml" mime type.
     */
    public static final String DRM_MIMETYPE_RIGHTS_XML_STRING =
            "application/vnd.oma.drm.rights+xml";

    /**
     * The "application/vnd.oma.drm.rights+wbxml" mime type.
     */
    public static final String DRM_MIMETYPE_RIGHTS_WBXML_STRING =
            "application/vnd.oma.drm.rights+wbxml";
    private Intent i = null;

    @Override
    public void onReceive(Context context, Intent intent) {
        Log.d(TAG, " INTENT = " + intent.getAction());

        int result = -1;

        String action = intent.getAction();
        Log.d(TAG, "starService over.");

        if (action.equals(DrmIntent.SHOW_PROPERTIES)) {
            Intent i = new Intent();
            i.setAction(intent.getAction());
            i.putExtra(DrmIntent.EXTRA_FILE_PATH, intent.getStringExtra(DrmIntent.EXTRA_FILE_PATH));
            i.putExtra(DrmIntent.EXTRA_DRM_TYPE, intent.getStringExtra(DrmIntent.EXTRA_DRM_TYPE));
            i.putExtra("PhotoPage", intent.getBooleanExtra("PhotoPage", false));
            i.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            Log.d(TAG, "going to start activity for show properties");
            context.startActivity(i);

        } else if (action.equals(DrmIntent.BUY_LICENSE)) {
            Intent i = new Intent();
            i.setAction(intent.getAction());
            i.putExtra(DrmIntent.EXTRA_FILE_PATH, intent.getStringExtra(DrmIntent.EXTRA_FILE_PATH));
            i.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            Log.d(TAG, "going to start activity for buy license");
            context.startActivity(i);
        } else if (action.equals(DrmIntent.WAP_PUSH)) {
            Log.d(TAG, "Receiving DrmIntent.WAP_PUSH");

            // Get right mimetype.
            String rightMimeType = intent.getType();
            Log.d(TAG, "rightMimeType " + rightMimeType);

            if (DRM_MIMETYPE_RIGHTS_XML_STRING.equals(rightMimeType) ||
                    DRM_MIMETYPE_RIGHTS_WBXML_STRING.equals(rightMimeType)) {
                // Get right data.
                byte[] rightData = (byte[]) intent.getExtra("data");
                if (rightData == null) {
                    Log.d(TAG, "The rights data is invalid.");
                    return;
                }
                String tempRightsFilePath = "/data/local/.Drm/rights.temp";
                try {
                    File rightsFile = new File(tempRightsFilePath);
                    if (rightsFile.exists()) {
                        rightsFile.delete();
                        // rightsFile.createNewFile();
                    }
                    rightsFile.createNewFile();
                    if (rightsFile != null) {
                        Runtime.getRuntime().exec("chmod 777 " + tempRightsFilePath);
                    } else {
                        Log.d(TAG, "The rights data is NULL, unable to store rights information");
                        return;
                    }

                    FileOutputStream os = null;
                    os = new FileOutputStream(rightsFile, true);
                    os.write(rightData, 0, rightData.length);
                    DrmRights drmRights = new DrmRights(tempRightsFilePath, rightMimeType);
                    DrmManagerClient drmClient = new DrmManagerClient(context);
                    drmClient.saveRights(drmRights, tempRightsFilePath, null);
                    rightsFile.delete();
                } catch (FileNotFoundException e) {
                    Log.e(TAG, "FileNotFoundException.");
                } catch (IOException e) {
                    Log.e(TAG, "IOException.");
                }
                return;
            }
            Log.d(TAG, "This is not drm rights push mimetype.");
        }
    }
}
