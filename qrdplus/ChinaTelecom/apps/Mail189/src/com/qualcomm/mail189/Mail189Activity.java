/**
 * Copyright (c) 2012-2013, Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.mail189;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;

import android.app.Activity;
import android.graphics.drawable.Drawable;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.content.pm.PackageManager.NameNotFoundException;
import android.net.Uri;
import android.os.Bundle;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.BaseAdapter;
import android.widget.ImageView;
import android.widget.ListView;
import android.widget.SimpleAdapter;
import android.widget.TextView;
import android.widget.AdapterView.OnItemClickListener;
import android.content.ActivityNotFoundException;

public class Mail189Activity extends Activity {

    private static final String TAG = "Mail189Activity";
    private static final Uri URL = Uri.parse("http://wapmail.189.cn");

    private String mBrowserPackageName = "com.android.browser";
    private Drawable[] mIcons = new Drawable[2];

    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        PackageManager pm = getPackageManager();
        boolean exist = false;
        try {
            PackageInfo info = pm.getPackageInfo("com.corp21cn.mail189",
                    PackageManager.GET_ACTIVITIES);
            if (info != null) {
                exist = true;
                mIcons[0] = info.applicationInfo.loadIcon(pm);
            }
        } catch (NameNotFoundException ex) {
        }

        if (!exist) {
            Log.w(TAG,
                    "We couldn't find the 189mail package, and we will load it in Browser");
            launchMail3G();
            finish();
            return;
        }

        try {
            PackageInfo info = pm.getPackageInfo(mBrowserPackageName,
                    PackageManager.GET_ACTIVITIES);
            if (info != null) {
                mIcons[1] = info.applicationInfo.loadIcon(pm);
            }
        } catch (NameNotFoundException ex) {
            Log.w(TAG, "We couldn't find the '" + mBrowserPackageName
                    + "' package.");
        }

        setContentView(R.layout.main);

        ListView listView = (ListView) findViewById(android.R.id.list);

        String text[] = { getResources().getString(R.string.mail_client),
                getResources().getString(R.string.mail_3g) };
        listView.setAdapter(new EfficientAdapter(this, mIcons, text));

        listView.setOnItemClickListener(new OnItemClickListener() {

            @Override
            public void onItemClick(AdapterView<?> arg0, View arg1, int arg2,
                    long arg3) {
                lauchApp(arg2);
            }
        });
    }

    private void launchMailClient() {
        PackageManager pm = getPackageManager();
        Intent intent = new Intent();
        intent = pm.getLaunchIntentForPackage("com.corp21cn.mail189");
        startActivity(intent);
    }

    private void launchMail3G() {
        Intent intent = new Intent();
        intent.setData(URL);
        intent.setAction(Intent.ACTION_VIEW);
        try {
            startActivity(intent);
        } catch (ActivityNotFoundException e) {
            Log.w(TAG, "e: " + e);
        }
    }

    private void lauchApp(int which) {
        switch (which) {
        case 0:
            launchMailClient();
            break;
        case 1:
            launchMail3G();
            break;
        }
        finish();
    }

    private static class EfficientAdapter extends BaseAdapter {
        private LayoutInflater mInflater;
        private Drawable mIcon[] = new Drawable[2];
        private String mText[] = new String[2];

        public EfficientAdapter(Context context, Drawable icon[], String text[]) {
            mInflater = LayoutInflater.from(context);

            mIcon = icon;
            mText = text;
        }

        public int getCount() {
            return mText.length;
        }

        public Object getItem(int position) {
            return position;
        }

        public long getItemId(int position) {
            return position;
        }

        public View getView(int position, View convertView, ViewGroup parent) {
            ViewHolder holder;

            if (convertView == null) {
                convertView = mInflater.inflate(R.layout.layout, null);

                holder = new ViewHolder();
                holder.text = (TextView) convertView.findViewById(R.id.text);
                holder.icon = (ImageView) convertView.findViewById(R.id.image);

                convertView.setTag(holder);
            } else {
                holder = (ViewHolder) convertView.getTag();
            }

            holder.text.setText(mText[position]);
            holder.icon.setBackgroundDrawable(mIcon[position]);

            return convertView;
        }

        static class ViewHolder {
            TextView text;
            ImageView icon;
        }
    }
}
