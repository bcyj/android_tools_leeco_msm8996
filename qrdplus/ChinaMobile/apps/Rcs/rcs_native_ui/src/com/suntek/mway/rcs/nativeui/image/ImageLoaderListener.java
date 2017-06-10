/*
 * Copyright (c) 2014 pci-suntektech Technologies, Inc.  All Rights Reserved.
 * pci-suntektech Technologies Proprietary and Confidential.
 */

package com.suntek.mway.rcs.nativeui.image;

import android.graphics.Bitmap;
import android.widget.ImageView;

public interface ImageLoaderListener {
    boolean onStartLoad();

    void onLoaded(String url, Bitmap bitmap, ImageView imageView);

    void onEndLoad();
}
