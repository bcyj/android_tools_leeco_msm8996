/*
 * Copyright (c) 2015 pci-suntektech Technologies, Inc.  All Rights Reserved.
 * pci-suntektech Technologies Proprietary and Confidential.
 */

package com.suntek.mway.rcs.nativeui.ui;

import java.util.List;
import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.ImageView;
import android.widget.TextView;
import com.suntek.mway.rcs.client.aidl.plugin.entity.pubacct.PublicAccounts;
import com.suntek.mway.rcs.nativeui.R;
import com.suntek.mway.rcs.nativeui.image.ImageLoader;

public class RecommendListAdapter extends BaseAdapter {

	private Context mContext;
	private List<PublicAccounts> mList;
	private ImageLoader mImageLoader;
	
	public RecommendListAdapter(Context context, List<PublicAccounts> list) {
		mContext = context;
		mList = list;
		mImageLoader = new ImageLoader(context);
	}

	@Override
	public int getCount() {
		return mList.size();
	}

	@Override
	public Object getItem(int position) {
		return mList.get(position);
	}

	@Override
	public long getItemId(int position) {
		return position;
	}

	@Override
	public View getView(int position, View convertView, ViewGroup parent) {
		ViewHolder holder = null;
        PublicAccounts account = mList.get(position);
        if (convertView == null) {
        	holder = new ViewHolder();
            convertView = LayoutInflater.from(mContext).inflate(R.layout.account_list_item, null);
            holder.mAlphaTv = (TextView) convertView.findViewById(R.id.text_alpha);
            holder.mNameTv = (TextView) convertView.findViewById(R.id.text);
            holder.mHeadIv = (ImageView) convertView.findViewById(R.id.image);
            convertView.setTag(holder);
        } else {
            holder = (ViewHolder) convertView.getTag();
        }
        String name = account.getName();
        String imageUrl = account.getLogo();
        holder.mNameTv.setText(name);
        displayImage(holder.mHeadIv, imageUrl);
        return convertView;
	}

	private static class ViewHolder {
		TextView mNameTv;
		ImageView mHeadIv;
		TextView mAlphaTv;
	}

	private void displayImage(ImageView imageView, String url) {
		mImageLoader.load(imageView, url, R.drawable.public_account_default_ic,
				R.drawable.public_account_default_ic);
	}
}
