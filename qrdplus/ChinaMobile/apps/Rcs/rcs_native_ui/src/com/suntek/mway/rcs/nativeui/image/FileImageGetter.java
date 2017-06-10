/*
 * Copyright (c) 2014 pci-suntektech Technologies, Inc.  All Rights Reserved.
 * pci-suntektech Technologies Proprietary and Confidential.
 */

package com.suntek.mway.rcs.nativeui.image;

import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.util.Log;

public class FileImageGetter extends ImageGetter {

    public FileImageGetter(ImageTask imageTask, ImageLoaderListener listener) {
        super(imageTask, listener);
    }

    @Override
    public void loadImage(String path) {
        Log.i("Imageloader", "load file image:" + path);
        imageTask.setLoading(true);
        // load image from local file
        Bitmap bitmap = BitmapFactory.decodeFile(path);
        listener.onLoaded(path, bitmap, imageTask.getImageView());

        imageTask.setLoading(false);
    }

}
