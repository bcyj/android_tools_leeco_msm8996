/*
 * Copyright (c) 2014 pci-suntektech Technologies, Inc.  All Rights Reserved.
 * pci-suntektech Technologies Proprietary and Confidential.
 */

package com.suntek.mway.rcs.nativeui.image;

public abstract class ImageGetter implements Runnable {
    protected ImageTask imageTask;
    protected ImageLoaderListener listener;

    public ImageGetter(ImageTask imageTask, ImageLoaderListener listener) {
        super();
        this.imageTask = imageTask;
        this.listener = listener;
    }

    @Override
    public void run() {
        // if onStartLoad() return true,then start load bitmap,
        // else this task may cancel if return false.
        if (this.listener.onStartLoad()) {
            loadImage(imageTask.getUrl());
        }
        this.listener.onEndLoad();
    }

    public abstract void loadImage(String path);

}
