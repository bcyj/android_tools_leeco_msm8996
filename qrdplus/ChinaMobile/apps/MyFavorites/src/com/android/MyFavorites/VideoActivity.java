/*
 * Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 * Not a Contribution, Apache license notifications and license are retained
 * for attribution purposes only.
 */

/*
 * Copyright (C) 2008 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
package com.android.MyFavorites;

import java.io.File;
import java.text.Collator;
import java.util.Collections;
import java.util.Comparator;
import java.util.LinkedList;

import android.app.AlertDialog;
import android.app.ListActivity;
import android.content.DialogInterface;
import android.content.Intent;
import android.database.Cursor;
import android.net.Uri;
import android.os.Bundle;
import android.provider.MediaStore;
import android.util.Log;
import android.view.ContextMenu;
import android.view.ContextMenu.ContextMenuInfo;
import android.view.LayoutInflater;
import android.view.MenuItem;
import android.view.View;
import android.view.View.OnCreateContextMenuListener;
import android.view.ViewGroup;
import android.widget.AdapterView.AdapterContextMenuInfo;
import android.widget.BaseAdapter;
import android.widget.ListView;
import android.widget.TextView;
import android.widget.Toast;

public class VideoActivity extends ListActivity {
    private static final String TAG="VideoActivity";
    private LinkedList<MovieInfo> playList = new LinkedList<MovieInfo>();
    private static final int DELETE_ITEM=0;
    private Uri videoListUri = MediaStore.Video.Media.EXTERNAL_CONTENT_URI;
    public class MovieInfo {
        String displayName;
        String path;
    }
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        fillAdapterList();
        setListAdapter(new VideoListAdapter(playList));
        getListView().setOnCreateContextMenuListener(new OnCreateContextMenuListener() {

            public void onCreateContextMenu(ContextMenu menu, View v,
                    ContextMenuInfo menuInfo) {
                if (!(menuInfo instanceof AdapterContextMenuInfo)) return;
                AdapterContextMenuInfo adapterMenuInfo = (AdapterContextMenuInfo) menuInfo;
                menu.setHeaderTitle(getTitleByPosition(adapterMenuInfo.position));
                Log.d(TAG, "title is :"+getTitleByPosition(adapterMenuInfo.position));
                menu.add(0, DELETE_ITEM, 0, getResources().getString(R.string.menu_delete));
            }
        });
    }
    private String getTitleByPosition(int position) {
        String temp = playList.get(position).displayName;
//        if(temp.lastIndexOf(".")!=-1) {
//            return temp.substring(0, temp.lastIndexOf("."));
//        }else {
            return temp;
//        }

    }
    private String getPathByPosition(int position) {
        return playList.get(position).path;
    }
    @Override
    public boolean onContextItemSelected(MenuItem item) {
        ContextMenuInfo menuInfo = item.getMenuInfo();
        if (!(menuInfo instanceof AdapterContextMenuInfo)) return false;

        AdapterContextMenuInfo adapterMenuInfo = (AdapterContextMenuInfo) menuInfo;
        Log.d(TAG, "path is :"+getPathByPosition(adapterMenuInfo.position));
        switch (item.getItemId()) {
            case DELETE_ITEM:
                //To delete
                showDeleteDialog(adapterMenuInfo.position);
                break;

            default:
                break;
        }
        return super.onContextItemSelected(item);
    }
    private void showDeleteDialog(final int position){
        AlertDialog d = new AlertDialog.Builder(VideoActivity.this).setTitle(R.string.menu_delete).setIcon(
                android.R.drawable.ic_dialog_alert).setMessage(R.string.prompt_delete).setPositiveButton(
                R.string.menu_done, new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int which) {
                        deleteFile(position);
                    }
                }).setNegativeButton(R.string.menu_cancel,
                new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int which) {
                        dialog.dismiss();
                    }
                }).create();
        d.show();
    }

    private void deleteFile(int position) {
        String path = getPathByPosition(position);
        String where = "_data='" + path+"'";
        File f = new File(path);
        if (f.exists()) {
            try {
                if (!f.delete()) {
                    Log.e(TAG, "Failed to delete file " + path);
                    return;
                }
            }
            catch (Exception e) {
                // TODO Auto-generated catch block
                e.printStackTrace();
                return;
            }
        }
        int row = getContentResolver().delete(videoListUri, where, null);
        Log.d(TAG, "delete count is :"+row);
        playList.remove(position);
        ((BaseAdapter) getListAdapter()).notifyDataSetChanged();
    }
    private void fillAdapterList() {
        playList.clear();
        if (android.os.Environment.getExternalStorageState().equals(
                android.os.Environment.MEDIA_MOUNTED)) {
            Cursor cursor=null;
            try{
                cursor = getContentResolver().query(videoListUri, new String[] {
                        "_display_name", "_data" }, null, null, null);
                if (cursor != null) {
                    Log.d(TAG, "cursor != null");
                    int n = cursor.getCount();
                    cursor.moveToFirst();
                    LinkedList<MovieInfo> playList2 = new LinkedList<MovieInfo>();
                    for (int i = 0; i != n; ++i) {
                        MovieInfo mInfo = new MovieInfo();
                        mInfo.displayName = cursor.getString(cursor
                                .getColumnIndex("_display_name"));
                        mInfo.path = cursor.getString(cursor
                                .getColumnIndex("_data"));
                        playList2.add(mInfo);
                        cursor.moveToNext();
                    }
                    Collections.sort(playList2,new Comparator<MovieInfo>() {
                        public int compare(MovieInfo object1, MovieInfo object2) {
                            return Collator.getInstance().compare(object1.displayName, object2.displayName);
                        };
                    });
                    playList = playList2;
                    if(playList.isEmpty()) {
                        Toast.makeText(this, R.string.no_video, Toast.LENGTH_LONG).show();
                    }
                } else {
                    Log.d(TAG, "cursor == null");
//                    getVideoFile(playList, new File("/sdcard/"));
                }
            }catch(Exception e){
                Log.e(TAG,e.getMessage());
            }finally{
                if(cursor!=null){
                    cursor.close();
                    cursor=null;
                }
            }
        }else {
            Toast.makeText(this, R.string.no_sd_card, Toast.LENGTH_LONG).show();
        }
    }
    @Override
    protected void onListItemClick(ListView l, View v, int position, long id) {
        if(position>=playList.size()) {
            return;
        }
        openFile(playList.get(position).path);
    }

    private void openFile(String path) {
        Intent intent = new Intent();
        intent.addFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP);
        intent.setAction(android.content.Intent.ACTION_VIEW);

        File f = new File(path);
        String type = getMIMEType(f.getName());
        intent.setDataAndType(Uri.fromFile(f), type);
        try {
            startActivity(intent);
            this.overridePendingTransition(0, 0);
        }
        catch (Exception e) {
            e.printStackTrace();
        }
    }
  public static String getMIMEType(String name) {
      String type = "";
      String end = name.substring(name.lastIndexOf(".") + 1, name.length()).toLowerCase();
      if (end.equals("apk")) {
          return "application/vnd.android.package-archive";
      }
      else if (end.equals("mp4") || end.equals("avi") || end.equals("3gp")
              || end.equals("m4v") || end.equals("rmvb")) {
          type = "video";
      }
      else if (end.equals("m4a") || end.equals("mp3") || end.equals("mid")
              || end.equals("xmf") || end.equals("ogg") || end.equals("wav")) {
          type = "audio";
      }
      else if (end.equals("jpg") || end.equals("gif") || end.equals("png")
              || end.equals("jpeg") || end.equals("bmp")) {
          type = "image";
      }
      else if (end.equals("txt") || end.equals("log")) {
          type = "text";
      }
      else {
          type = "video";
      }
      type += "/*";
      return type;
  }
    public class VideoListAdapter extends BaseAdapter{
        private LinkedList<MovieInfo> mContentList;
        private LayoutInflater mInflater=null;
        public VideoListAdapter() {
        }
        public VideoListAdapter(LinkedList<MovieInfo> playList) {
            this.mContentList = playList;
            if(mInflater==null){
                mInflater = getLayoutInflater();
            }
        }
        public int getCount() {
            return mContentList.size();
        }

        public Object getItem(int arg0) {
            return arg0;
        }

        public long getItemId(int arg0) {
            return arg0;
        }

        public View getView(int arg0, View convertView, ViewGroup arg2) {
            if(convertView==null){
                convertView = mInflater.inflate(R.layout.simple_list_item, null);
            }
            TextView text = (TextView) convertView.findViewById(R.id.text1);
            text.setText(playList.get(arg0).displayName);
            return convertView;
        }
  }
}
