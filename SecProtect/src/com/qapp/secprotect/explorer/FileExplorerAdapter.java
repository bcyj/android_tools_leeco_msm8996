/*
 * Copyright (c) 2015 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
package com.qapp.secprotect.explorer;

import java.io.File;
import java.util.ArrayList;

import android.app.Activity;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.ImageView;
import android.widget.TextView;

import com.qapp.secprotect.Configs;
import com.qapp.secprotect.R;

public class FileExplorerAdapter extends BaseAdapter {
    private ArrayList<File> mFileList;
    private LayoutInflater mLayoutInflater;
    private Activity mActivity;
    private Bitmap mFileIcon;
    private Bitmap mFolderIcon;

    public FileExplorerAdapter(Activity activity, ArrayList<File> fileList) {
        mFileList = fileList;
        mLayoutInflater = LayoutInflater.from(activity);
        mActivity = activity;
        mFolderIcon = BitmapFactory.decodeResource(activity.getResources(),
                R.drawable.folder);
        mFileIcon = BitmapFactory.decodeResource(activity.getResources(),
                R.drawable.file);
    }

    @Override
    public int getCount() {
        return mFileList.size();
    }

    @Override
    public Object getItem(int index) {
        return mFileList.get(index);
    }

    @Override
    public long getItemId(int id) {
        return id;
    }

    private class ViewData {
        ImageView fileIcon;
        ImageView lockIcon;
        TextView fileName;
    }

    @Override
    public View getView(int position, View convertView, ViewGroup parent) {
        ViewData viewData = null;
        if (convertView == null) {
            convertView = mLayoutInflater
                    .inflate(R.layout.file_item_view, null);
            viewData = new ViewData();
            viewData.fileIcon = (ImageView) convertView
                    .findViewById(R.id.file_icon);
            viewData.lockIcon = (ImageView) convertView
                    .findViewById(R.id.lock_icon);
            viewData.fileName = (TextView) convertView
                    .findViewById(R.id.file_name);
            convertView.setTag(viewData);
        } else {
            viewData = (ViewData) convertView.getTag();
        }

        File file = mFileList.get(position);
        if (file.isFile()) {
            viewData.fileIcon.setImageBitmap(mFileIcon);
            if (file.isFile()
                    && file.getName().endsWith(Configs.ENCRYPT_FILE_SUFFIX)) {
                viewData.lockIcon.setVisibility(View.VISIBLE);
            } else {
                viewData.lockIcon.setVisibility(View.INVISIBLE);
            }
        } else {
            viewData.fileIcon.setImageBitmap(mFolderIcon);
            viewData.lockIcon.setVisibility(View.INVISIBLE);
        }
        viewData.fileName.setText(file.getName());
        return convertView;
    }
}
