/*
 * Copyright (c) 2014 pci-suntektech Technologies, Inc.  All Rights Reserved.
 * pci-suntektech Technologies Proprietary and Confidential.
 */

package com.suntek.mway.rcs.nativeui.ui;

import java.lang.ref.SoftReference;
import java.util.HashMap;
import java.util.List;

import android.content.Context;
import android.graphics.Bitmap;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.ImageView;
import android.widget.TextView;
import com.suntek.mway.rcs.nativeui.R;
import com.suntek.mway.rcs.nativeui.image.ImageLoader;
import com.suntek.mway.rcs.client.aidl.plugin.entity.pubacct.PublicAccounts;

public class AccountListAdapter extends BaseAdapter {
    public static final String TAG = "AccountListAdapter";
    private static class ViewHolder {
        TextView text;
        ImageView image;
        TextView textAlpha;
        public ViewHolder(View view) {
            text = (TextView) view.findViewById(R.id.text);
            image = (ImageView) view.findViewById(R.id.image);
            textAlpha = (TextView) view.findViewById(R.id.text_alpha);
        }
    }
    private LayoutInflater inflater;
    private List<PublicAccounts> accountList;
    private HashMap<String, SoftReference<Bitmap>> mPhotoCache = new HashMap<String, SoftReference<Bitmap>>();
    private ImageLoader mImageLoader;
    
    public AccountListAdapter(Context context, List<PublicAccounts> accountList) {
        inflater = LayoutInflater.from(context);
        this.accountList = accountList;
        mImageLoader = new ImageLoader(context);
    }

    public void addPhotoMap(String url, Bitmap bitmap) {
        mPhotoCache.put(url, new SoftReference<Bitmap>(bitmap));
    }

    public Bitmap getBitmapViaUrl(String url) {
        if (mPhotoCache.containsKey(url)) {
            return mPhotoCache.get(url).get();
        }
        return null;
    }
    @Override
    public int getCount() {
        if (accountList == null) {
            return 0;
        }
        else {
            return accountList.size();
        }
    }

    @Override
    public PublicAccounts getItem(int arg0) {
        if (accountList == null) {
            return null;
        }
        return accountList.get(arg0);
    }

    @Override
    public long getItemId(int arg0) {
        return arg0;
    }

    @Override
    public View getView(int position, View convertView, ViewGroup arg2) {
        final ViewHolder holder;
        final PublicAccounts account = getItem(position);
        if (convertView == null || convertView.getTag() == null) {
            convertView = inflater.inflate(R.layout.account_list_item, null);
            holder = new ViewHolder(convertView);
            convertView.setTag(holder);
        } else {
            holder = (ViewHolder) convertView.getTag();
        }
        String name = account.getName();
        String imageUrl = account.getLogo();
//        if (mPhotoCache.containsKey(imageUrl)) {
//            holder.image.setImageBitmap(mPhotoCache.get(imageUrl).get());
//        }
        displayImage(holder.image, imageUrl);
        holder.text.setText(name);
        return convertView;
    }

    public void removeItemByUuid(String uuid) {
        int size = accountList.size();
        for (int i = 0; i < size; i++) {
            if (accountList.get(i).getPaUuid() == uuid) {
                accountList.remove(i);
                break;
            }
        }
    }

	private void displayImage(ImageView imageView, String url) {
		mImageLoader.load(imageView, url, R.drawable.public_account_default_ic,
				R.drawable.public_account_default_ic);
	}
}
