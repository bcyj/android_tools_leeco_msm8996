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
package com.android.cmcc.service;

import com.android.cmcc.R;
import com.android.cmcc.customer.CustomerService;
import com.android.cmcc.customer.CustomerServiceList;

import android.app.ListActivity;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.net.Uri;
import android.os.Bundle;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.ListView;
import android.widget.TextView;
//WYF add 20120104
import android.os.SystemProperties;
import java.util.ArrayList;
import java.util.List;

//WYF add end

public class CmccServer extends ListActivity {

    public boolean haveSTK;

    private static final int ACTION_GAME = 0;
    private static final int ACTION_NEWS = 1;
    private static final int ACTION_VEDIO = 2;
    private static final int ACTION_PHONEBOOK = 3;
    private static final int ACTION_ECONOMIC = 4;
    private static final int ACTION_NEWSERVICE = 5;
    private static final int ACTION_CUSTOMERSERVICE = 6;
    private static final int ACTION_MYMONTERNET = 7;
    private static final int ACTION_MONTERNET = 8;
    private static final int ACTION_STK = 9;
    private static final int ACTION_SEARCH = 10;
    private static final int LIST_COUNT = 11;
    private int[] ActionId;

    private ArrayList<String> ActionName = new ArrayList<String>();
    private ArrayList<String> strArrList = new ArrayList<String>();

    private int index = 0;
    private int index1 = 0;
    private int i = 0;
    private boolean mAllowgames = false;
    private boolean mAllowvideo = false;
    private boolean mAllowpim = false;

    /* goto wap browser */
    public static void gotoWapBrowser(Context context, String uri) {
        Uri localUri = Uri.parse(uri);
        Intent localIntent = new Intent("android.intent.action.VIEW", localUri);
        localIntent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        localIntent.addFlags(Intent.FLAG_ACTIVITY_RESET_TASK_IF_NEEDED);
        localIntent.putExtra("data_connection", "wap");
        context.startActivity(localIntent);
    }

    private boolean haveSTKService() {
        boolean isStk;
        PackageManager pm = getPackageManager();
        Intent localIntent = new Intent("android.intent.action.MAIN");
        // Create a new component identifier from a Context and class name.
        ComponentName comName = new ComponentName("com.android.stk",
                "com.android.stkStk.LauncherActivity");
        localIntent.setComponent(comName);
        // Retrieve all activities that can be performed for the given intent
        if (pm.queryIntentActivities(localIntent,
                PackageManager.MATCH_DEFAULT_ONLY).size() > 0)
            isStk = true;
        else {
            isStk = false;
        }
        return isStk;
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        boolean isHaveStk = haveSTKService();
        this.haveSTK = isHaveStk;
        StringBuilder localStringBuilder = new StringBuilder();

        int[] intArrList = new int[14];

        mAllowgames = getResources().getBoolean(R.bool.enable_games);
        if (mAllowgames) {
            strArrList.add(index, getString(R.string.list_game_menu));
            intArrList[index] = ACTION_GAME;
            index++;
        }
        strArrList.add(index, getString(R.string.list_news_menu));
        intArrList[index] = ACTION_NEWS;

        mAllowvideo = getResources().getBoolean(R.bool.enable_video);
        if (mAllowvideo) {
            index++;
            strArrList.add(index, getString(R.string.list_vedio_menu));
            intArrList[index] = ACTION_VEDIO;
        }

        mAllowpim = getResources().getBoolean(R.bool.enable_pim);
        if (mAllowpim) {
            index++;
            strArrList.add(index, getString(R.string.list_phonebook_menu));
            intArrList[index] = ACTION_PHONEBOOK;
        }

        index++;
        strArrList.add(index, getString(R.string.list_economic_menu));
        intArrList[index] = ACTION_ECONOMIC;
        index++;
        strArrList.add(index, getString(R.string.list_newservice_menu));
        intArrList[index] = ACTION_NEWSERVICE;
        index++;
        strArrList.add(index, getString(R.string.list_customerservice_menu));
        intArrList[index] = ACTION_CUSTOMERSERVICE;
        index++;
        strArrList.add(index, getString(R.string.list_mymonternet_menu));
        intArrList[index] = ACTION_MYMONTERNET;
        index++;
        strArrList.add(index, getString(R.string.list_monternet_menu));
        intArrList[index] = ACTION_MONTERNET;
        index1 = index;
        if (this.haveSTK) {
            index++;
            strArrList.add(index, getString(R.string.list_stk_menu));
            intArrList[index] = ACTION_STK;
            index++;
            strArrList.add(index, getString(R.string.list_search_menu));
            intArrList[index] = ACTION_SEARCH;
        } else {
            index++;
            strArrList.add(index, getString(R.string.list_search_menu));
            intArrList[index] = ACTION_SEARCH;
        }
        this.ActionId = intArrList;
        this.ActionName = strArrList;
        if (this.ActionName.get(index1).startsWith("What")) {
            this.ActionName.add(index1, "What's New!");
        }
        EfficientAdapter localEfficientAdapter = new EfficientAdapter(this,
                this.ActionName, this.haveSTK);
        setListAdapter(localEfficientAdapter);
    }

    @Override
    protected void onListItemClick(ListView l, View v, int position, long id) {
        Log.i("CmccServer", "onListItemClick---position=" + position + " id="
                + id);
        switch (this.ActionId[position]) {

        case ACTION_GAME: {
            gotoWapBrowser(this, Links.GAME_LINK);
            break;
        }
        case ACTION_NEWS: {
            gotoWapBrowser(this, Links.NEWS_LINK);
            break;
        }

        case ACTION_VEDIO: {
            Intent tv = new Intent("android.intent.action.MAIN");
            ComponentName localComponentName3 = new ComponentName(
                    "com.mediatek.cmmb.app", "com.mediatek.cmmb.app.MainScreen");
            tv.setComponent(localComponentName3);
            tv.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            tv.addFlags(Intent.FLAG_ACTIVITY_RESET_TASK_IF_NEEDED);
            startActivity(tv);
            break;
        }

        case ACTION_PHONEBOOK: {
            Intent phonebook = new Intent("android.intent.action.MAIN");
            ComponentName localComponentName4 = new ComponentName(
                    "com.android.pim", "com.android.pim.Pim");
            phonebook.setComponent(localComponentName4);
            phonebook.addCategory(Intent.CATEGORY_LAUNCHER);
            phonebook.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            phonebook.addFlags(Intent.FLAG_ACTIVITY_RESET_TASK_IF_NEEDED);
            startActivity(phonebook);
            break;
        }
        case ACTION_ECONOMIC: {
            gotoWapBrowser(this, Links.ECONOMIC_LINK);
            break;
        }
        case ACTION_NEWSERVICE: {
            gotoWapBrowser(this, Links.NEW_SERVICE_LINK);
            break;
        }
        case ACTION_CUSTOMERSERVICE: {
            Intent customer = new Intent("android.intent.action.MAIN");
            ComponentName customerCom = new ComponentName(CmccServer.this,
                    CustomerService.class);
            customer.setComponent(customerCom);
            customer.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            customer.addFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP);
            startActivity(customer);
            break;
        }
        case ACTION_MYMONTERNET: {
            Intent myMounternet = new Intent("android.intent.action.MAIN");
            ComponentName serverList = new ComponentName(CmccServer.this,
                    CustomerServiceList.class);
            myMounternet.setComponent(serverList);
            myMounternet.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            myMounternet.addFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP);
            myMounternet.putExtra("type",
                    CustomerServiceList.MYMONTERNET_LIST_TYPE);
            startActivity(myMounternet);
            break;
        }
        case ACTION_MONTERNET: {
            gotoWapBrowser(this, Links.MONTERNET_LINK);
            break;
        }
        case ACTION_SEARCH: {
            gotoWapBrowser(this, Links.SEARCH_LINK);
            break;
        }
        case ACTION_STK: {

            break;
        }
        }
    }

    class EfficientAdapter extends BaseAdapter {
        private Bitmap[] Icons;
        private ArrayList<String> ListLabels;
        private Context mContext;
        private LayoutInflater mInflater;

        public EfficientAdapter(CmccServer cmccServer) {
            LayoutInflater localLayoutInflater = (LayoutInflater) cmccServer
                    .getSystemService(Context.LAYOUT_INFLATER_SERVICE);
            ;
            this.mInflater = localLayoutInflater;
            this.mContext = cmccServer;
        }

        public EfficientAdapter(CmccServer cmccServer,
                ArrayList<String> actionName, boolean haveSTK) {
            LayoutInflater localLayoutInflater = (LayoutInflater) cmccServer
                    .getSystemService(Context.LAYOUT_INFLATER_SERVICE);
            this.mInflater = localLayoutInflater;
            this.mContext = cmccServer;
            this.ListLabels = actionName;

            Bitmap[] arrBmp = new Bitmap[this.ListLabels.size()];

            if (mAllowgames) {
                arrBmp[i] = BitmapFactory.decodeResource(
                        cmccServer.getResources(), R.drawable.game);
                i++;
            }
            arrBmp[i] = BitmapFactory.decodeResource(cmccServer.getResources(),
                    R.drawable.news);
            if (mAllowvideo) {
                i++;
                arrBmp[i] = BitmapFactory.decodeResource(
                        cmccServer.getResources(), R.drawable.vedio);
            }
            if (mAllowpim) {
                i++;
                arrBmp[i] = BitmapFactory.decodeResource(
                        cmccServer.getResources(), R.drawable.phonbook);
            }
            i++;
            arrBmp[i++] = BitmapFactory.decodeResource(
                    cmccServer.getResources(), R.drawable.economic);
            arrBmp[i++] = BitmapFactory.decodeResource(
                    cmccServer.getResources(), R.drawable.newservice);
            arrBmp[i++] = BitmapFactory.decodeResource(
                    cmccServer.getResources(), R.drawable.csicon);
            arrBmp[i++] = BitmapFactory.decodeResource(
                    cmccServer.getResources(), R.drawable.mymonternet);
            arrBmp[i] = BitmapFactory.decodeResource(cmccServer.getResources(),
                    R.drawable.monternet);
            if (haveSTK) {
                i++;
                arrBmp[i] = BitmapFactory.decodeResource(
                        cmccServer.getResources(), R.drawable.stk);
                i++;
                arrBmp[i] = BitmapFactory.decodeResource(
                        cmccServer.getResources(), R.drawable.search);
            } else {
                i++;
                arrBmp[i] = BitmapFactory.decodeResource(
                        cmccServer.getResources(), R.drawable.search);
            }
            this.Icons = arrBmp;
        }

        public int getCount() {
            return this.ListLabels.size();
        }

        public Object getItem(int item) {
            return Integer.valueOf(item);
        }

        public long getItemId(int id) {
            return id;
        }

        public View getView(int pos, View view, ViewGroup viewGroup) {
            ViewHolder localViewHolder;
            Log.i("CmccServer", "_____getView        pos______" + pos);
            Context localContext = this.mContext;
            view = new LinearLayout(localContext);
            LayoutInflater localLayoutInflater = this.mInflater;
            LinearLayout localLinearLayout = (LinearLayout) view;
            localLayoutInflater.inflate(R.layout.cmcc_server_list,
                    localLinearLayout);
            localViewHolder = new ViewHolder();
            TextView txt1 = (TextView) view.findViewById(R.id.server_list_text);
            txt1.setText(this.ListLabels.get(pos));
            localViewHolder.text = txt1;
            ImageView icon1 = (ImageView) view.findViewById(R.id.server_list_icon);
            icon1.setImageBitmap(Icons[pos]);
            localViewHolder.icon = icon1;
            view.setTag(localViewHolder);
            return view;
        }

        class ViewHolder {
            ImageView icon;
            TextView text;
        }
    }
}
