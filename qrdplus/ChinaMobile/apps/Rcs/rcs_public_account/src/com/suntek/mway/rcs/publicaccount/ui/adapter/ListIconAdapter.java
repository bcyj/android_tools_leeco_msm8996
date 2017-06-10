/*
 * Copyright (c) 2014 pci-suntektech Technologies, Inc.  All Rights Reserved.
 * pci-suntektech Technologies Proprietary and Confidential.
 */

package com.suntek.mway.rcs.publicaccount.ui.adapter;

import com.suntek.mway.rcs.publicaccount.R;

import java.util.List;

import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.ImageView;
import android.widget.TextView;

public class ListIconAdapter extends ArrayAdapter<ListIconAdapter.ListIconItem> {
    
    protected LayoutInflater mInflater;

    private static final int mResource = R.layout.dialog_icon_list_item;

    private ViewHolder mViewHolder;

    public ListIconAdapter(Context context, List<ListIconItem> items) {
        super(context, mResource, items);
        mInflater = (LayoutInflater)context.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
    }

    @Override
    public View getView(int position, View convertView, ViewGroup parent) {
        View view;
        if (convertView == null) {
            view = mInflater.inflate(mResource, parent, false);
            mViewHolder = new ViewHolder(view);
            view.setTag(mViewHolder);
        } else {
            view = convertView;
            mViewHolder = (ViewHolder)view.getTag();
        }
        TextView text = mViewHolder.getTextView();
        text.setText(getItem(position).getTitle());
        ImageView image = mViewHolder.getImageView();
        image.setImageResource(getItem(position).getResource());
        return view;
    }

    public static class ListIconItem {
        private final String mTitle;

        private final int mResource;

        public ListIconItem( int res, String title) {
            mResource = res;
            mTitle = title;
        }

        public int getResource() {
            return mResource;
        }
        
        public String getTitle() {
            return mTitle;
        }
    }

    private class ViewHolder {

        private ImageView mImageView;

        private View mView;
        
        private TextView mTextView;

        public ViewHolder(View view) {
            mView = view;
        }

        public ImageView getImageView() {
            if (mImageView == null) {
                mImageView = (ImageView)mView.findViewById(R.id.item_image);
            }
            return mImageView;
        }

        public TextView getTextView() {
            if (mTextView == null) {
                mTextView = (TextView)mView.findViewById(R.id.item_text);
            }
            return mTextView;
        }
    }
}
