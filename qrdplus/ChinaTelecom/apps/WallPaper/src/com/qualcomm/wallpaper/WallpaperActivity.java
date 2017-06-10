/**
 * Copyright (c) 2012-2013, Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.wallpaper;

import android.app.Activity;
import android.app.WallpaperManager;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.util.Log;

import java.io.IOException;

public class WallpaperActivity extends Activity {

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Intent intent = new Intent();
        intent.setClassName("com.android.launcher", "com.android.launcher2.WallpaperChooser");
        intent.putExtra("packagename", getPackageName());
        startActivityForResult(intent, 0);
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        if (resultCode != RESULT_OK || data == null) {
            finish();
            return;
        }

        try {
            int resId = data.getIntExtra("resid", R.drawable.wallpaper_1);
            WallpaperManager wpm = (WallpaperManager) getSystemService(Context.WALLPAPER_SERVICE);
            wpm.setResource(resId);
        } catch (IOException e) {
            Log.e("WallpaperActivity", "Failed to set wallpaper: " + e);
        } finally {
            finish();
        }
    }

}
