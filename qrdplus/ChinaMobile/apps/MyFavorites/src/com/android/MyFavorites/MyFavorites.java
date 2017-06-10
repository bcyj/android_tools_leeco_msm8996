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

import android.app.Activity;
import android.os.Bundle;
import android.widget.ListView;
import android.widget.SimpleAdapter;
import java.util.Vector;
import java.util.ArrayList;
import java.util.HashMap;
import android.content.res.Configuration;
import android.os.RemoteException;
import android.widget.AdapterView.OnItemClickListener;
import android.app.ListActivity;
import android.widget.AdapterView;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Toast;
import android.content.Intent;
import android.content.SharedPreferences;
import android.preference.ListPreference;
import android.preference.PreferenceActivity;
import android.net.Uri;
import android.util.Log;

public class MyFavorites extends Activity  {

    public static final int MY_FAVORITES_MUSIC = 0;
    public static final int MY_FAVORITES_VIDEO = 1;
    public static final int MY_FAVORITES_PICTURE = 2;
    public static final int MY_FAVORITES_ALL_FILE = 3;
    public static final int MY_FAVORITES_GAME = 4;
    public static final int MY_FAVORITES_INDEX = 5;

    SimpleAdapter listFavoritesItemAdapter = null;
    SimpleAdapter listFavoritesItemClickAdapter = null;
    ListView favoritesList = null;
    private static String[] favoritesItems = null;
    private static String[] favoritesItemsZhcn = null;
    Vector provinceCodeName = new Vector();
    String loc ="";
    Intent intent  = null;
    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.my_favorite_main);
        favoritesItems = new String[MY_FAVORITES_INDEX];
        favoritesItemsZhcn = new String[MY_FAVORITES_INDEX];
        favoritesList = (ListView) findViewById(R.id.favoritesId);


        for(int i=0;i< MY_FAVORITES_INDEX;i++) {
            switch (i) {
            case MY_FAVORITES_MUSIC:
              favoritesItems[i] = getString(R.string.MyFavorite_Music);
                break;
            case MY_FAVORITES_VIDEO:
              favoritesItems[i] =getString(R.string.MyFavorite_Videos);
                break;
            case MY_FAVORITES_PICTURE:
              favoritesItems[i] =getString(R.string.MyFavorite_Pictures);
                break;
            case MY_FAVORITES_ALL_FILE:
                favoritesItems[i] =getString(R.string.MyFavorite_AllFiles);
                break;
            case MY_FAVORITES_GAME:
                favoritesItems[i] =getString(R.string.MyFavorite_Ggame);
                break;
            }
        }

        ArrayList<HashMap<String, Object>> favoritesListItem = new ArrayList<HashMap<String, Object>>();

        for(int i=0;i< MY_FAVORITES_INDEX;i++)
        {
            HashMap<String, Object> favoritesMap = new HashMap<String, Object>();
            if(i==MY_FAVORITES_MUSIC) {
                favoritesMap.put("ItemImage", R.drawable.myfavorites_music);
            } else if(i==MY_FAVORITES_VIDEO) {
                favoritesMap.put("ItemImage", R.drawable.myfavorites_video);
            } else if(i==MY_FAVORITES_PICTURE) {
                favoritesMap.put("ItemImage", R.drawable.myfavorites_photo);
            } else if(i==MY_FAVORITES_ALL_FILE) {
                favoritesMap.put("ItemImage", R.drawable.myfavorites_allfile);
            } else if(i==MY_FAVORITES_GAME) {
                favoritesMap.put("ItemImage", R.drawable.myfavorites_game);
            }

            favoritesMap.put("ItemTitle", favoritesItems[i]);
            favoritesListItem.add(favoritesMap);
        }

        listFavoritesItemAdapter = new SimpleAdapter(this,favoritesListItem,
            R.layout.list_items,
            new String[] {"ItemImage","ItemTitle"},
            new int[] {R.id.ItemImage,R.id.ItemTitle}
        );

        favoritesList.setAdapter(listFavoritesItemAdapter);

        favoritesList.setOnItemClickListener(new OnItemClickListener() {
            public void onItemClick(AdapterView<?> arg0, View arg1, int arg2,    long arg3) {
                intent = new Intent();
                switch (arg2) {
                case MY_FAVORITES_MUSIC:
                    intent.setClassName( "com.android.music", "com.android.music.MusicBrowserActivity");//Music
                    startActivity(intent);
                    break;
                case MY_FAVORITES_VIDEO:
                    intent.setClass(MyFavorites.this, VideoActivity.class);
                    startActivity(intent);
                    //need vedio
                    break;
                case MY_FAVORITES_PICTURE:
                    //intent.setClassName( "com.android.gallery", "com.android.camera.GalleryPicker");//Gallery
                    intent.setClassName( "com.android.gallery3d", "com.android.gallery3d.app.Gallery");//Gallery
                    startActivity(intent);
                    break;
                case MY_FAVORITES_ALL_FILE:
                    //need FileManager
                    intent.setClassName("com.qualcomm.qti.explorer", "com.qualcomm.qti.explorer.ExplorerActivity");
                    try {
                        startActivity(intent);
                    }catch (Exception e) {
                        Log.e("MyFavorite", "can not to start FileManager :"+e);
                    }
                    break;
                case MY_FAVORITES_GAME:
                    Uri uri = Uri.parse("http://g.10086.cn");//Game
                    intent = new Intent(Intent.ACTION_VIEW, uri);
                    startActivity(intent);
                    break;
                }
            }//end of public void onItemClick
        }); //end of favoritesList.setOnItemClickListener
    }
}
