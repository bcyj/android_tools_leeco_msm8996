/*
 * Copyright (c) 2014 pci-suntektech Technologies, Inc.  All Rights Reserved.
 * pci-suntektech Technologies Proprietary and Confidential.
 */

package com.suntek.mway.rcs.nativeui.ui;

import com.suntek.mway.rcs.client.aidl.plugin.entity.mcloudfile.FileNode;
import com.suntek.mway.rcs.nativeui.image.ImageLoader;
import com.suntek.mway.rcs.nativeui.utils.ImageUtils;
import com.suntek.mway.rcs.nativeui.R;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.drawable.Drawable;
import android.text.TextUtils;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.ImageView;
import android.widget.TextView;

import java.io.IOException;
import java.io.InputStream;
import java.net.HttpURLConnection;
import java.net.URL;
import java.util.ArrayList;

public class SaiunFileShareAdapter extends BaseAdapter {

    private ArrayList<FileNode> mFileNodeList = new ArrayList<FileNode>();

    private Context mContext;
    private ImageLoader imageLoader;

    public SaiunFileShareAdapter(Context context) {
        this.mContext = context;
        imageLoader = new ImageLoader(context);
    }

    public void setDatas(ArrayList<FileNode> shareNodeList) {
        mFileNodeList.clear();
        mFileNodeList.addAll(shareNodeList);
        this.notifyDataSetChanged();
    }

    public int getCount() {
        return mFileNodeList.size();
    }

    public Object getItem(int position) {
        return mFileNodeList.get(position);
    }

    public long getItemId(int position) {
        return 0;
    }

    public View getView(int position, View convertView, ViewGroup parent) {
        ViewHolder holder = null;
        if (convertView == null) {
            convertView = LayoutInflater.from(
                    mContext).inflate(
                    R.layout.rcs_saiun_file_share_item_view, null);
            holder = new ViewHolder();

            holder.mFileName = (TextView) convertView
                    .findViewById(R.id.fileName);
            holder.mFileImage = (ImageView) convertView
                    .findViewById(R.id.file_image);
            holder.mFileCreateTime = (TextView) convertView
                    .findViewById(R.id.file_create_time);
            holder.mFileSize = (TextView) convertView
                    .findViewById(R.id.file_size);

            convertView.setTag(holder);
        } else {
            holder = (ViewHolder) convertView.getTag();
        }
        FileNode mFileNode = (FileNode) getItem(position);
        holder.mFileName.setTag(mFileNode);
        holder.mFileName.setText(mFileNode.getName());
        holder.mFileCreateTime.setText(mFileNode.getCreateTime());

        if (mFileNode.isFile()) {
            holder.mFileSize.setVisibility(View.VISIBLE);
            holder.mFileSize.setText(ImageUtils.getFileSize(
                    mFileNode.getSize(), 1024, "KB"));
        } else {
            holder.mFileSize.setVisibility(View.GONE);
        }
        String imagePath = mFileNode.getThumbnailURL();
        Log.i("RCS_UI", "the imagePath=" + imagePath);
        // setBitmap(holder.mFileImage,imagePath);
        imageLoader.load(holder.mFileImage, imagePath, R.drawable.rcs_ic_cloud,
                R.drawable.rcs_ic_cloud);
        return convertView;
    }

    public static class ViewHolder {
        TextView mFileName;
        ImageView mFileImage;
        TextView mFileCreateTime;
        TextView mFileSize;
    }

    public static void setBitmap(final ImageView imageView, final String url) {
        new Thread(new Runnable() {
            @Override
            public void run() {
                final Bitmap bitmap = getBitmap(url);
                imageView.post(new Runnable() {
                    @Override
                    public void run() {
                        imageView.setImageBitmap(bitmap);
                    }
                });
            }

        }).start();
    }

    public static Bitmap getBitmap(String path) {
        try {
            URL url = new URL(path);
            HttpURLConnection conn = (HttpURLConnection) url.openConnection();
            conn.setConnectTimeout(5000);
            conn.setRequestMethod("GET");
            if (conn.getResponseCode() == 200) {
                InputStream inputStream = conn.getInputStream();
                Bitmap bitmap = BitmapFactory.decodeStream(inputStream);
                return bitmap;
            }
        } catch (IOException e) {
            return null;
        }
        return null;
    }

    public void destroy() {
        imageLoader.destroy();
    }

}
