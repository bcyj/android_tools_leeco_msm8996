/*
 * Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.qti.update;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.net.Uri;
import android.util.Log;
import com.qualcomm.update.InstallReceiver;
import com.qualcomm.update.R;

import java.io.File;

public class StartUpdate extends BroadcastReceiver {

    @Override
    public void onReceive(final Context context, Intent intent) {

        if (context.getResources().getBoolean(R.bool.customize_replace_sdcard_dir)) {
            Log.d("StartUpdate", "Start update received !!!");
            String filePath = intent.getStringExtra("filePath");
            filePath = filePath.replace("/mnt/sdcard", "/storage/sdcard0");// recheck for URI operations
            File upgradeFile = new File(filePath);
            Log.d("StartUpdate", "install file:" + upgradeFile);
            intent = new Intent(InstallReceiver.ACTION_REBOOT);
            intent.setData(Uri.fromFile(upgradeFile));
            intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            context.startActivity(intent);
        } else {
            Log.d("StartUpdate", "FR replace sdcard dir not enabel!");
        }
    }
}
