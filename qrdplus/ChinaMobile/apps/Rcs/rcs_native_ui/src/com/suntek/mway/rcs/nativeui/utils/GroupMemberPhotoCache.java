/*
 * Copyright (c) 2014 pci-suntektech Technologies, Inc.  All Rights Reserved.
 * pci-suntektech Technologies Proprietary and Confidential.
 */
package com.suntek.mway.rcs.nativeui.utils;

import com.suntek.mway.rcs.client.api.impl.callback.ConferenceCallback;
import com.suntek.mway.rcs.client.aidl.plugin.entity.profile.Avatar;
import com.suntek.mway.rcs.client.api.util.ServiceDisconnectedException;
import com.suntek.mway.rcs.nativeui.RcsApiManager;

import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.drawable.Drawable;
import android.os.RemoteException;
import android.text.TextUtils;
import android.util.Base64;
import android.widget.QuickContactBadge;

import java.lang.ref.SoftReference;
import java.util.HashMap;

public class GroupMemberPhotoCache {
    private final static int IMAGE_PIXEL = 120;
    private HashMap<String, SoftReference<Bitmap>> mImageCache;

    private static GroupMemberPhotoCache sInstance;

    private GroupMemberPhotoCache() {
        mImageCache = new HashMap<String, SoftReference<Bitmap>>();
    }

    public void removeCache(String number){
        mImageCache.remove(number);
    }

    public static void loadGroupMemberPhoto(String rcsGroupId,String addr,final QuickContactBadge mAvatar,final Drawable sDefaultContactImage){
        getInstance().loadGroupMemberPhoto(rcsGroupId, addr, new GroupMemberPhotoCache.ImageCallback() {
            
            @Override
            public void loadImageCallback(Bitmap bitmap) {
                if( bitmap != null){
                    mAvatar.setImageBitmap(bitmap);
                }else{
                    mAvatar.setImageDrawable(sDefaultContactImage);
                }
            }
        });
    }

    public static GroupMemberPhotoCache getInstance() {
        if (sInstance == null) {
            sInstance = new GroupMemberPhotoCache();
        }
        return sInstance;
    }

    public synchronized void loadGroupMemberPhoto(String groupId, final String number,
            final ImageCallback callback) {
        if (TextUtils.isEmpty(number)) {
            callback.loadImageCallback(null);
            return;
        }

        if (mImageCache.containsKey(number)) {
            SoftReference<Bitmap> softReference = mImageCache.get(number);
            Bitmap bitmap = softReference.get();
            if (bitmap != null) {
                callback.loadImageCallback(bitmap);
                return;
            } else {
                mImageCache.remove(number);
            }
        }
        try {
            RcsApiManager.getConfApi().queryMemberHeadPic(groupId, number, IMAGE_PIXEL,
                    new ConferenceCallback() {

                        @Override
                        public void onRefreshAvatar(Avatar avatar, int resultCode, String resultDesc)
                                throws RemoteException {
                            super.onRefreshAvatar(avatar, resultCode, resultDesc);
                            if (resultCode == 0 && avatar != null) {
                                String str = avatar.getImgBase64Str();
                                byte[] imageByte = Base64.decode(str,
                                        Base64.DEFAULT);
                                Bitmap bitmap = BitmapFactory.decodeByteArray(imageByte, 0,
                                        imageByte.length);
                                mImageCache.put(number, new SoftReference<Bitmap>(bitmap));
                                callback.loadImageCallback(bitmap);
                                return;
                            }
                        }
                    });
        } catch (ServiceDisconnectedException e) {
            e.printStackTrace();
        }
    }

    public interface ImageCallback {
        public void loadImageCallback(Bitmap bitmap);
    }

}
