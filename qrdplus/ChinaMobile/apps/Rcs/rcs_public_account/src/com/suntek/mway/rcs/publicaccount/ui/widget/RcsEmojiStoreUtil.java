/*
 * Copyright (c) 2014 pci-suntektech Technologies, Inc.  All Rights Reserved.
 * pci-suntektech Technologies Proprietary and Confidential.
 */

package com.suntek.mway.rcs.publicaccount.ui.widget;

import java.lang.ref.SoftReference;
import java.util.HashMap;
import java.util.Map;
import java.util.concurrent.LinkedBlockingQueue;

import com.suntek.mway.rcs.client.aidl.plugin.entity.emoticon.EmoticonConstant;
import com.suntek.mway.rcs.client.api.util.ServiceDisconnectedException;
import com.suntek.mway.rcs.publicaccount.RcsApiManager;

import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.os.Handler;
import android.os.Message;
import android.text.TextUtils;
import android.widget.ImageView;

public class RcsEmojiStoreUtil {

    public static final int EMO_STATIC_FILE = EmoticonConstant.EMO_STATIC_FILE;

    public static final int EMO_DYNAMIC_FILE = EmoticonConstant.EMO_DYNAMIC_FILE;

    public static final int EMO_PACKAGE_FILE = EmoticonConstant.EMO_PACKAGE_FILE;

    private Map<String, SoftReference<Bitmap>> mCaches;

    private LinkedBlockingQueue<LoaderImageTask> mTaskQueue;

    private boolean mIsRuning = false;

    private static RcsEmojiStoreUtil mInstance;

    public static RcsEmojiStoreUtil getInstance() {
        if (mInstance == null) {
            mInstance = new RcsEmojiStoreUtil();
        }
        return mInstance;
    }

    private RcsEmojiStoreUtil() {
        mCaches = new HashMap<String, SoftReference<Bitmap>>();
        mTaskQueue = new LinkedBlockingQueue<RcsEmojiStoreUtil.LoaderImageTask>();
        mIsRuning = true;
        new Thread(runnable).start();
    }

    public void loadImageAsynById(ImageView imageView, String imageId,
            int loaderType) {
        if(imageView == null){
            return;
        }
        imageView.setTag(imageId);
        if (mCaches.containsKey(imageId)) {
            SoftReference<Bitmap> rf = mCaches.get(imageId);
            Bitmap bitmap = rf.get();
            if (bitmap == null) {
                mCaches.remove(imageId);
            } else {
                imageView.setImageBitmap(bitmap);
                return;
            }
        }
        mTaskQueue.add(new LoaderImageTask(imageId, imageView, loaderType));
    }

    private Handler mHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            LoaderImageTask task = (LoaderImageTask) msg.obj;
            String imageId = (String) task.imageView.getTag();
            if(!TextUtils.isEmpty(imageId) && imageId.equals(task.imageId)){
                task.imageView.setImageBitmap(task.bitmap);
            }
        }
    };

    private Runnable runnable = new Runnable() {
        @Override
        public void run() {
            while (mIsRuning) {
                try {
                    LoaderImageTask task = mTaskQueue.take();
                    String imageId = (String) task.imageView.getTag();
                    if(!TextUtils.isEmpty(imageId) && imageId.equals(task.imageId)){
                        task.bitmap = getbitmap(task.loaderType, task.imageId);
                        if (mHandler != null) {
                            Message msg = mHandler.obtainMessage();
                            msg.obj = task;
                            mHandler.sendMessage(msg);
                        }
                    }
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
            }
        }
    };

    private Bitmap getbitmap(int loaderType, String emoticonId) {
        byte[] imageByte = null;
        Bitmap bitmap = null;
        try {
            if (loaderType == EMO_STATIC_FILE) {
                imageByte = RcsApiManager.getEmoticonApi().decrypt2Bytes(
                        emoticonId, EMO_STATIC_FILE);
            } else if (loaderType == EMO_PACKAGE_FILE) {
                imageByte = RcsApiManager.getEmoticonApi().decrypt2Bytes(
                        emoticonId, EMO_PACKAGE_FILE);
            }
        } catch (ServiceDisconnectedException e) {
            e.printStackTrace();
        }
        if (imageByte != null) {
            bitmap = BitmapFactory.decodeByteArray(imageByte, 0,
                    imageByte.length);
        }
        if (bitmap != null) {
            mCaches.put(emoticonId, new SoftReference<Bitmap>(bitmap));
        }
        return bitmap;
    }

    public class LoaderImageTask {
        String imageId;

        ImageView imageView;

        Bitmap bitmap;

        int loaderType;

        public LoaderImageTask(String imageId, ImageView imageView,
                int loaderType) {
            this.imageId = imageId;
            this.imageView = imageView;
            this.loaderType = loaderType;
        }
    }

}
