/*
 * Copyright (c) 2014 pci-suntektech Technologies, Inc.  All Rights Reserved.
 * pci-suntektech Technologies Proprietary and Confidential.
 */

package com.suntek.mway.rcs.nativeui.image;

import android.widget.ImageView;

public class ImageTask {
    private String url;
    private ImageView imageView;
    private int emptyImgID = -1;
    private int errorImgID = -1;
    private boolean canceled;
    private boolean loading;

    public ImageTask(String url, ImageView imageView) {
        this(url, imageView, -1, -1);
    }

    public ImageTask(String url, ImageView imageView, int emptyImgID, int errorImgID) {
        super();
        this.url = url;
        this.imageView = imageView;
        this.emptyImgID = emptyImgID;
        this.errorImgID = errorImgID;
        this.canceled = false;
        this.loading = false;
    }

    public String getUrl() {
        return url;
    }

    public void setUrl(String url) {
        this.url = url;
    }

    public ImageView getImageView() {
        return imageView;
    }

    public void setImageView(ImageView imageView) {
        this.imageView = imageView;
    }

    public int getEmptyImgID() {
        return emptyImgID;
    }

    public void setEmptyImgID(int emptyImgID) {
        this.emptyImgID = emptyImgID;
    }

    public int getErrorImgID() {
        return errorImgID;
    }

    public void setErrorImgID(int errorImgID) {
        this.errorImgID = errorImgID;
    }

    public boolean isCanceled() {
        return canceled;
    }

    public void setCanceled(boolean canceled) {
        this.canceled = canceled;
    }

    public boolean isLoading() {
        return loading;
    }

    public void setLoading(boolean loading) {
        this.loading = loading;
    }

}
