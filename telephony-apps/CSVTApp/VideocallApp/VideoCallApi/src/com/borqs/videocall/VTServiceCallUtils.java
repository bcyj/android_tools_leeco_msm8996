/* BORQS Software Solutions Pvt Ltd. CONFIDENTIAL
 * Copyright (c) 2012 All rights reserved.
 *
 * The source code contained or described herein and all documents
 * related to the source code ("Material") are owned by BORQS Software
 * Solutions Pvt Ltd. No part of the Material may be used,copied,
 * reproduced, modified, published, uploaded,posted, transmitted,
 * distributed, or disclosed in any way without BORQS Software
 * Solutions Pvt Ltd. prior written permission.
 *
 * No license under any patent, copyright, trade secret or other
 * intellectual property right is granted to or conferred upon you
 * by disclosure or delivery of the Materials, either expressly, by
 * implication, inducement, estoppel or otherwise. Any license
 * under such intellectual property rights must be express and
 * approved by BORQS Software Solutions Pvt Ltd. in writing.
 *
 */

package com.borqs.videocall;

import android.content.Context;
import android.graphics.Bitmap;
import android.media.MediaScanner;
import android.text.TextUtils;
import android.util.Log;
import android.widget.Toast;

import android.os.ParcelFileDescriptor;
import android.os.Process;
import android.os.Environment;
import android.os.StatFs;
import android.os.SystemClock;
import android.graphics.SurfaceTexture;

import java.io.FileReader;
import java.io.File;
import java.io.IOException;
import java.util.Calendar;

import java.io.FileNotFoundException;

public class VTServiceCallUtils {
    static final String TAG = " VTServiceCallUtils.";

    // return the stored file name which is dynamically generated
    public static String storeImageToFile(Context ctx, Bitmap source, String storeDir) {
        Log.e(TAG, "=========enter storeImageToFile============");
        if (source == null) {
            int resID = getResourseIdByName("com.borqs.videocall", "string",
                    "capture_picture_failure");
            Toast.makeText(ctx, ctx.getText(resID), Toast.LENGTH_SHORT).show();
            // BALDEV
            Log.e(TAG, "Error in storeimagetofile: source is null");
            return null;
        }

        MediaScanner Scanner;
        Scanner = new MediaScanner(ctx);

        // todo read dir from preference,file name should be more readable
        File sdDir;
        try {
            sdDir = new File(storeDir);
            if (!sdDir.exists()) {
                if (MyLog.DEBUG)
                    MyLog.v(TAG, "no such path mk dir " + storeDir);
                if (false == sdDir.mkdirs()) {
                    if (MyLog.DEBUG)
                        MyLog.v(TAG, "error to create dir " + storeDir);
                }
            }
        } catch (Exception ex) {
            Log.e(TAG, "Error in create dir " + storeDir + " " + ex);
            return null;
        }
        Calendar mCalendar = Calendar.getInstance();
        String month, day, hour, min, sec;
        if ((mCalendar.get(Calendar.MONTH) + 1) < 10) {
            month = "0" + (mCalendar.get(Calendar.MONTH) + 1);
        } else {
            month = "" + (mCalendar.get(Calendar.MONTH) + 1);
        }
        if (mCalendar.get(Calendar.DAY_OF_MONTH) < 10) {
            day = "0" + mCalendar.get(Calendar.DAY_OF_MONTH);
        } else {
            day = "" + mCalendar.get(Calendar.DAY_OF_MONTH);
        }
        if (mCalendar.get(Calendar.HOUR_OF_DAY) < 10) {
            hour = "0" + mCalendar.get(Calendar.HOUR_OF_DAY);
        } else {
            hour = "" + mCalendar.get(Calendar.HOUR_OF_DAY);
        }
        if (mCalendar.get(Calendar.MINUTE) < 10) {
            min = "0" + mCalendar.get(Calendar.MINUTE);
        } else {
            min = "" + mCalendar.get(Calendar.MINUTE);
        }
        if (mCalendar.get(Calendar.HOUR_OF_DAY) < 10) {
            sec = "0" + mCalendar.get(Calendar.SECOND);
        } else {
            sec = "" + mCalendar.get(Calendar.SECOND);
        }
        String fileName = storeDir + "/VT" + mCalendar.get(Calendar.YEAR) + month + day + "_"
                + hour + min + sec + ".jpg";

        Log.e(TAG, "Write file : " + fileName);
        try {
            java.io.OutputStream fos = new java.io.FileOutputStream(fileName);
            source.compress(Bitmap.CompressFormat.JPEG, 100, fos);
            fos.close();
            Scanner.scanSingleFile(fileName, "external", "image/jpeg");
            int resID = getResourseIdByName("com.borqs.videocall", "string", "finish_capture_photo");
            Toast.makeText(ctx, ctx.getText(resID) + storeDir, Toast.LENGTH_SHORT).show();
        } catch (Exception ex) {
            // BALDEV Toast.makeText(ctx, R.string.capture_picture_failure,
            // Toast.LENGTH_SHORT).show();
            int resID = getResourseIdByName("com.borqs.videocall", "string",
                    "capture_picture_failure");
            Toast.makeText(ctx, ctx.getText(resID), Toast.LENGTH_SHORT).show();
            Log.e(TAG, "Error in storeimagetofile:" + ex);
            return null;
        }
        Log.e(TAG, "leave storeImageToFile");

        return "fakefile";
    }

    public static int getResourseIdByName(String packageName, String type, String name) {

        return VTServiceInterface.getAppContext().getResources().getIdentifier(name, type, packageName);
     }

}
