/*
 * Copyright (c) 2014 pci-suntektech Technologies, Inc.  All Rights Reserved.
 * pci-suntektech Technologies Proprietary and Confidential.
 */

package com.suntek.mway.rcs.nativeui.image;

import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.util.Log;

import java.io.InputStream;
import java.net.HttpURLConnection;
import java.net.URL;

public class NetImageGetter extends ImageGetter {

    public NetImageGetter(ImageTask imageTask, ImageLoaderListener listener) {
        super(imageTask, listener);
    }

    @Override
    public void loadImage(String path) {
        Log.i("Imageloader", "load net image:" + path);
        imageTask.setLoading(true);
        // load image from net
        // Bitmap bitmap = null;
        // try {
        // bitmap = BitmapFactory.decodeStream(new URL(path).openStream());
        // } catch (MalformedURLException e) {
        // e.printStackTrace();
        // } catch (IOException e) {
        // e.printStackTrace();
        // }
        Bitmap bitmap = getHttpBitmap(path);
        listener.onLoaded(path, bitmap, imageTask.getImageView());

        imageTask.setLoading(false);
    }

    // 第一种方法
    public Bitmap getHttpBitmap(String path) {
        Bitmap bitmap = null;
        try {
            // 初始化一个URL对象
            URL url = new URL(path);
            // 获得HTTPConnection网络连接对象
            HttpURLConnection connection = (HttpURLConnection) url.openConnection();
            connection.setConnectTimeout(5 * 1000);
            connection.setDoInput(true);
            connection.connect();
            // 得到输入流
            InputStream is = connection.getInputStream();
            bitmap = BitmapFactory.decodeStream(is);
            // 关闭输入流
            is.close();
            // 关闭连接
            connection.disconnect();
        } catch (Exception e) {
            e.printStackTrace();
        }
        return bitmap;
    }

}
