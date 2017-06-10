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

package com.dean.areasearch;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.List;
import android.app.ActionBar;
import android.app.Activity;
import android.app.AlertDialog;
import android.app.TabActivity;
import android.content.ContentResolver;
import android.content.ContentUris;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.SharedPreferences;
import android.database.Cursor;
import android.database.sqlite.SQLiteException;
import android.net.Uri;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.view.Gravity;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.View.OnLongClickListener;
import android.view.ViewGroup;
import android.view.inputmethod.InputMethodManager;
import android.widget.AbsListView;
import android.widget.ArrayAdapter;
import android.widget.BaseExpandableListAdapter;
import android.widget.CursorAdapter;
import android.widget.ExpandableListView;
import android.widget.ListView;
import android.widget.SearchView;
import android.widget.TabHost;
import android.widget.TabHost.OnTabChangeListener;
import android.widget.TextView;
import android.widget.Toast;
import android.widget.SearchView.OnQueryTextListener;
import com.dean.areasearch.AreaCodeTables.AreaCodeTable;


@SuppressWarnings("deprecation")
public class AreaSearchFirstPage extends TabActivity {

    public static AreaSearchFirstPage instance = null;
    private SearchView mSearchView;
    String cityName = null;
    ExpandableListView list1;
    ExpandableListView list2;
    AreaCodeAdapter adapter3;
    ListView list3;
    View from;
    View to;
    public static final String TAG = "AreaSearchFirstPage";
    private Cursor mCursor;
    private Cursor nCursor;
    private LayoutInflater mFactory;
    private List<String> mGroupArray;
    private List<List<String>> mChildArray;
    private List<String> mGroupArray2;
    private List<String> mGroupArray3;
    private List<List<String>> mChildArray2;
    ExpandableAdapter2 expand2;
    MyHandler myHandler;
    ExpandableAdapter expand1;
    private ArrayList<String> mNameArray = new ArrayList<String>();
    ArrayAdapter<String> adapter = null;
    ArrayAdapter<String> sadapter = null;
    private static final String[] PROJECTION = new String[] {
            AreaCodeTable._ID, // 0
            AreaCodeTable.CODE, // 1
            AreaCodeTable.CITY_NAME // 2
    };
    private static final Context Context = null;
    SharedPreferences pref;
    SharedPreferences.Editor editor;
    int selected_id;
    ActionBar aBar;

    private static final int SHOW_TOAST = 0;
    private static final int NOTIFY_DATA_CHANGED = 1;

    @Override
    public void onCreate(Bundle savedInstanceState) {

        super.onCreate(savedInstanceState);

        TabHost tab = getTabHost();

        LayoutInflater.from(this).inflate(R.layout.first_page,
                tab.getTabContentView(), true);
        from = findViewById(R.id.frame_page_1);
        findViewById(R.id.frame_page_2).setVisibility(View.INVISIBLE);
        findViewById(R.id.frame_page_3).setVisibility(View.INVISIBLE);
        tab.setOnTabChangedListener(new OnTabChangeListener() {

            @Override
            public void onTabChanged(String tabId) {
                // TODO Auto-generated method stub
                if (tabId.equals("international")) {
                    findViewById(R.id.frame_page_1).setVisibility(View.VISIBLE);
                    findViewById(R.id.frame_page_2).setVisibility(
                            View.INVISIBLE);
                    findViewById(R.id.frame_page_3).setVisibility(
                            View.INVISIBLE);
                }

                if (tabId.equals("domestic")) {
                    findViewById(R.id.frame_page_1).setVisibility(
                            View.INVISIBLE);
                    findViewById(R.id.frame_page_2).setVisibility(View.VISIBLE);
                    findViewById(R.id.frame_page_3).setVisibility(
                            View.INVISIBLE);
                }

                if (tabId.equals("custom")) {
                    findViewById(R.id.frame_page_1).setVisibility(
                            View.INVISIBLE);
                    findViewById(R.id.frame_page_2).setVisibility(
                            View.INVISIBLE);
                    findViewById(R.id.frame_page_3).setVisibility(View.VISIBLE);

                }
            }
        });

        tab.addTab(tab.newTabSpec("international")
                .setIndicator(getText(R.string.international))
                .setContent(R.id.cache));

        tab.addTab(tab.newTabSpec("domestic")
                .setIndicator(getText(R.string.domestic))
                .setContent(R.id.cache));

        tab.addTab(tab.newTabSpec("custom")
                .setIndicator(getText(R.string.custom)).setContent(R.id.cache));
        pref = this.getSharedPreferences("pref", MODE_PRIVATE);
        if (!pref.contains("FIRST_RUN")) {
            AreaCodeTables.InitDB(this);
            // build the acnumber.db

            editor = pref.edit();
            editor.putInt("currentversion", 1);
            editor.putInt("selected_id1", 0);
            editor.putInt("selected_id2", 0);
            editor.putBoolean("FIRST_RUN", false);
            editor.apply();
            nCursor = managedQuery(AreaCodeTable.CONTENT_URI, null, null, null,
                    null);

        }

        onConfigurationChanged(this.getResources().getConfiguration());

        myHandler = new MyHandler();
        editor = pref.edit();

        //custom number
        mFactory = LayoutInflater.from(this);

        mCursor = managedQuery(AreaCodeTable.CONTENT_URI, PROJECTION, null,
                null, null);
        list3 = (ListView) findViewById(R.id.list3);
        adapter3 = new AreaCodeAdapter(this, mCursor);
        list3.setAdapter(adapter3);

        //international number
        mGroupArray = new ArrayList<String>();
        mChildArray = new ArrayList<List<String>>();

        new Thread(new Runnable() {
            @Override
            public void run() {
                dbQuery();
            }
        }).start();
    }

    private void dbQuery() {
        String[] continent = getResources().getStringArray(R.array.continent);
        String string;
        for (int i = 0; i < continent.length; i++) {
            mGroupArray.add(continent[i].split(",")[1]);
        }

        Cursor cursor = null;
        boolean doQuery = true;
        while (doQuery) {
            try {
                cursor = getContentResolver().query(
                        AreaCodeTable.NATIVE_INTER_URI, null, null, null, null);

                for (int i = 0; i < mGroupArray.size(); i++) {

                    List<String> tempArray = new ArrayList<String>();
                    if (cursor.moveToFirst()) {
                        do {
                            if (continent[i].split(",")[1].equals(cursor.getString(4))) {

                                string = cursor.getString(3) + ", 00"
                                        + cursor.getInt(1);
                                tempArray.add(string);
                            }
                        } while (cursor.moveToNext());

                    }
                    mChildArray.add(tempArray);
                }
                doQuery = false;
            } catch (SQLiteException e) {
                android.util.Log.e(TAG, "SQLite exception: " + e);
            } finally {
                if (cursor != null) {
                    cursor.close();
                }
                if (doQuery) {
                    try {
                        Thread.sleep(5);
                    } catch (InterruptedException e) {
                    }
                }
            }
        }

        expand1 = new ExpandableAdapter(this);
        //domestic number
        mGroupArray2 = new ArrayList<String>();
        mGroupArray3 = new ArrayList<String>();
        mChildArray2 = new ArrayList<List<String>>();

        Cursor cursor2 = null;
        Cursor cursor3 = null;

        doQuery = true;
        while (doQuery) {
            try {
                cursor2 = getContentResolver().query(
                        AreaCodeTable.NATIVE_DOMES_URI, null, null, null, null);

                cursor3 = getContentResolver().query(
                        AreaCodeTable.NATIVE_DOMES_URI, null, "parent_id='000000'",
                        null, null);

                if (cursor3.moveToFirst()) {
                    do {
                        mGroupArray2.add(cursor3.getString(1));
                        mGroupArray3.add(cursor3.getString(2));

                    } while (cursor3.moveToNext());
                }

                for (int i = 0; i < mGroupArray2.size(); i++) {
                    List<String> tempArray = new ArrayList<String>();

                    if (cursor2.moveToFirst()) {
                        do {
                            if (cursor2.getString(3).substring(0, 2)
                                    .equals(mGroupArray3.get(i))
                                    && cursor2.getString(3).length() < 5
                                    && (!(cursor2.getString(4).equals("2")))) {

                                string = cursor2.getString(1) + ", "
                                        + cursor2.getString(5);
                                tempArray.add(string);
                            }
                        } while (cursor2.moveToNext());

                    }
                    mChildArray2.add(tempArray);
                }
                doQuery = false;
            } catch (SQLiteException e) {
                android.util.Log.e(TAG, "SQLite exception: " + e);
            } finally {
                if (cursor2 != null) {
                    cursor2.close();
                }
                if (cursor3 != null) {
                    cursor3.close();
                }
                if (doQuery) {
                    try {
                        Thread.sleep(5);
                    } catch (InterruptedException e) {
                    }
                }
            }
        }
        expand2 = new ExpandableAdapter2(Context, this);
        Message notifyMsg = new Message();
        notifyMsg.what = NOTIFY_DATA_CHANGED;
        myHandler.sendMessage(notifyMsg);
    }

    // action bar
    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        getMenuInflater().inflate(R.menu.actionbar_menu, menu);
        setupSearchView(menu);
        return super.onCreateOptionsMenu(menu);
    }

    private void setupSearchView(Menu menu) {
        mSearchView = (SearchView) menu.findItem(R.id.menu_search)
                .getActionView();

        mSearchView.setFocusable(false);
        mSearchView.setOnQueryTextListener(new OnQueryTextListener() {

            @Override
            public boolean onQueryTextSubmit(String arg0) {
                // TODO Auto-generated method stub

                mNameArray.clear();
                InputMethodManager imm = (InputMethodManager) getSystemService(INPUT_METHOD_SERVICE);
                imm.hideSoftInputFromWindow(mSearchView.getWindowToken(), 0);
                implQuery(arg0);
                return false;
            }

            @Override
            public boolean onQueryTextChange(String newText) {

                return true;
            }
        });
        mSearchView.setSubmitButtonEnabled(true);
    }

    @Override
    public boolean onPrepareOptionsMenu(Menu menu) {

        this.mSearchView.clearFocus();
        return super.onPrepareOptionsMenu(menu);
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {


        switch (item.getItemId()) {
        case R.id.Update: {

            new AlertDialog.Builder(AreaSearchFirstPage.this)
                    .setIconAttribute(android.R.attr.alertDialogIcon)
                    .setTitle(R.string.message_box_title)
                    .setMessage(getString(R.string.updateDB))
                    .setNegativeButton(R.string.button_cancel, null)
                    .setPositiveButton(android.R.string.ok,
                            new DialogInterface.OnClickListener() {
                                public void onClick(DialogInterface dialog,
                                        int whichButton) {
                                    (new Thread(new DownloadDatabase()))
                                            .start();
                                }
                            }).show();

            break;
        }
        default:
            break;
        case R.id.Add: {

            startActivity(new Intent(AreaSearchFirstPage.this,
                    EditAreaCode.class));
            break;
        }
        case R.id.Help: {
            AlertDialog dlg = new AlertDialog.Builder(AreaSearchFirstPage.this)
                    .setIcon(android.R.drawable.ic_menu_help)
                    .setTitle(R.string.help_acti_title)
                    .setMessage(R.string.help_text).setCancelable(true)
                    .create();

            dlg.setCanceledOnTouchOutside(true);
            dlg.show();
        }
        }

        return super.onOptionsItemSelected(item);
    }

    public class ExpandableAdapter extends BaseExpandableListAdapter {
        Activity activity;

        public ExpandableAdapter(Activity a) {
            activity = a;
        }

        public Object getChild(int groupPosition, int childPosition) {
            return mChildArray.get(groupPosition).get(childPosition);
        }

        public long getChildId(int groupPosition, int childPosition) {
            return childPosition;
        }

        public int getChildrenCount(int groupPosition) {
            return mChildArray.get(groupPosition).size();
        }

        public View getChildView(int groupPosition, int childPosition,
                boolean isLastChild, View convertView, ViewGroup parent) {
            String string = mChildArray.get(groupPosition).get(childPosition);
            View v = activity.getLayoutInflater().inflate(
                    R.layout.expand_child, null);
            TextView nameTV = (TextView) v.findViewById(R.id.child_city_name);
            TextView codeTV = (TextView) v.findViewById(R.id.child_city_code);
            nameTV.setText(string.substring(0, string.indexOf(",")));
            codeTV.setText(string.substring(string.indexOf(",") + 1,
                    string.length()));
            return v;
        }

        public Object getGroup(int groupPosition) {
            return mGroupArray.get(groupPosition);
        }

        public int getGroupCount() {
            return mGroupArray.size();
        }

        public long getGroupId(int groupPosition) {
            return groupPosition;
        }

        public View getGroupView(int groupPosition, boolean isExpanded,
                View convertView, ViewGroup parent) {
            String string = mGroupArray.get(groupPosition);
            convertView = mFactory.inflate(R.layout.expand_parent, null);
            ((TextView) convertView.findViewById(R.id.app_info))
                    .setText(string);
            return convertView;
        }

        public TextView getGenericView(String string) {
            AbsListView.LayoutParams layoutParams = new AbsListView.LayoutParams(
                    ViewGroup.LayoutParams.FILL_PARENT, 52);
            TextView text = new TextView(activity);
            text.setLayoutParams(layoutParams);
            text.setGravity(Gravity.CENTER_VERTICAL | Gravity.LEFT);
            text.setPadding(48, 0, 0, 0);
            text.setText(string);
            text.setTextColor(android.graphics.Color.BLACK);
            text.setTextSize(22);
            return text;
        }

        public boolean hasStableIds() {
            return true;
        }

        public boolean isChildSelectable(int groupPosition, int childPosition) {
            return true;
        }
    }

    public class ExpandableAdapter2 extends BaseExpandableListAdapter {
        Activity activity;

        public ExpandableAdapter2(Context context, Activity a) {
            activity = a;
        }

        public Object getChild(int groupPosition, int childPosition) {
            return mChildArray2.get(groupPosition).get(childPosition);
        }

        public long getChildId(int groupPosition, int childPosition) {
            return childPosition;
        }

        public int getChildrenCount(int groupPosition) {
            return mChildArray2.get(groupPosition).size();
        }

        public View getChildView(int groupPosition, int childPosition,
                boolean isLastChild, View convertView, ViewGroup parent) {
            String string = mChildArray2.get(groupPosition).get(childPosition);
            View tv = activity.getLayoutInflater().inflate(
                    R.layout.expand_child, null);
            TextView nameTV = (TextView) tv.findViewById(R.id.child_city_name);
            TextView codeTV = (TextView) tv.findViewById(R.id.child_city_code);
            nameTV.setText(string.substring(0, string.indexOf(",")));
            codeTV.setText(string.substring(string.indexOf(",") + 1,
                    string.length()));
            return tv;
        }

        public Object getGroup(int groupPosition) {
            return mGroupArray2.get(groupPosition);
        }

        public int getGroupCount() {
            return mGroupArray2.size();
        }

        public long getGroupId(int groupPosition) {
            return groupPosition;
        }

        public View getGroupView(int groupPosition, boolean isExpanded,
                View convertView, ViewGroup parent) {

            String string = mGroupArray2.get(groupPosition);
            convertView = mFactory.inflate(R.layout.expand_parent, null);
            ((TextView) convertView.findViewById(R.id.app_info))
                    .setText(string);

            return convertView;

        }

        public TextView getGenericView(String string) {
            AbsListView.LayoutParams layoutParams = new AbsListView.LayoutParams(
                    ViewGroup.LayoutParams.FILL_PARENT, 52);
            TextView text = new TextView(activity);
            text.setLayoutParams(layoutParams);
            text.setGravity(Gravity.CENTER_VERTICAL | Gravity.LEFT);
            text.setPadding(48, 0, 0, 0);
            text.setText(string);
            text.setTextColor(android.graphics.Color.BLACK);
            text.setTextSize(20);
            return text;
        }

        public boolean hasStableIds() {
            return true;
        }

        public boolean isChildSelectable(int groupPosition, int childPosition) {
            return true;
        }
    }

    private class AreaCodeAdapter extends CursorAdapter {

        public AreaCodeAdapter(Context context, Cursor cursor) {
            super(context, cursor);
        }

        @Override
        public View newView(Context context, Cursor cursor, ViewGroup parent) {

            View view = mFactory
                    .inflate(R.layout.list_view_item, parent, false);

            return view;
        }

        @Override
        public void bindView(View view, Context context, Cursor cursor) {
            final int id = cursor.getInt(0);
            final String areaCode = cursor.getString(1);
            final String cityName = cursor.getString(2);
            final ContentResolver contentResolver = getContentResolver();

            ((TextView) view.findViewById(R.id.city_name)).setText(cityName);
            ((TextView) view.findViewById(R.id.city_code)).setText(areaCode);
            view.setLongClickable(true);
            view.setOnLongClickListener(new OnLongClickListener() {

                @Override
                public boolean onLongClick(View v) {
                    // TODO Auto-generated method stub

                    new AlertDialog.Builder(AreaSearchFirstPage.this)
                            .setIconAttribute(android.R.attr.alertDialogIcon)
                            .setTitle(R.string.message_box_title)
                            .setMessage(
                                    getText(R.string.delete_query) + "<"
                                            + cityName + ", " + areaCode
                                            + "> ?")
                            .setNegativeButton(android.R.string.cancel, null)
                            .setPositiveButton(android.R.string.ok,
                                    new DialogInterface.OnClickListener() {
                                        public void onClick(
                                                DialogInterface dialog,
                                                int whichButton) {
                                            // delete selected data
                                            Uri uri = ContentUris
                                                    .withAppendedId(
                                                            AreaCodeTable.CONTENT_URI,
                                                            id);
                                            contentResolver.delete(uri, null,
                                                    null);
                                            // finish();
                                        }
                                    }).show();
                    return true;
                }
            });
        }
    }

    public void implQuery(String inputStr) {

        this.mSearchView.clearFocus();

        if (inputStr.length() == 0) {
            mNameArray.add(getString(R.string.search_null));
            updateListView();
            return;
        }

        if ((inputStr.substring(0, 1).equals("+"))
                || (inputStr.substring(0, 1).equals("\uff0b"))) {
            inputStr = inputStr.substring(1);

        }

        // Chinese

        if (Character.isLetter(inputStr.charAt(0))) {
            // custom code
            Cursor cursor = getContentResolver().query(
                    AreaCodeTable.CONTENT_URI, null, null, null, null);
            if (cursor == null) {
                return;
            }

            if (cursor.moveToFirst()) {
                String str1;
                do {
                    str1 = cursor.getString(2);

                    if (str1.contains(inputStr)) {
                        mNameArray.add(str1 + ", " + cursor.getString(1));
                    }
                } while (cursor.moveToNext());
                cursor.close();

            }

            // international
            Cursor cursor2 = getContentResolver().query(
                    AreaCodeTable.NATIVE_INTER_URI, null, null, null, null);
            if (cursor2 == null) {
                return;
            }

            if (this.getResources().getConfiguration().locale.getLanguage()
                    .equals("zh")) {

                if (isChinese(inputStr.charAt(0))) {
                    if (cursor2.moveToFirst()) {
                        String str1;

                        do {
                            str1 = cursor2.getString(3);

                            if (str1.contains(inputStr)) {
                                mNameArray.add(str1 + ", 00"
                                        + cursor2.getString(1));
                            }
                        } while (cursor2.moveToNext());
                        cursor2.close();

                    }
                }
            } else {
                if (isEnglish(inputStr.charAt(0))) {

                    if (cursor2.moveToFirst()) {
                        String str1;

                        do {
                            str1 = cursor2.getString(3);

                            if (str1.contains(inputStr)) {
                                mNameArray.add(str1 + ", 00"
                                        + cursor2.getString(1));
                            }
                        } while (cursor2.moveToNext());
                        cursor2.close();

                    }
                }

            }

            //domestic
            Cursor cursor3 = getContentResolver().query(
                    AreaCodeTable.NATIVE_DOMES_URI, null, null, null, null);
            if (cursor3 == null) {
                return;
            }

            if (cursor3.moveToFirst()) {
                String str1;

                do {
                    str1 = cursor3.getString(1);

                    if (str1.contains(inputStr)
                            && (cursor3.getString(5) != null)
                            && (cursor3.getString(2) != null)) {
                        mNameArray.add(str1 + ", " + cursor3.getString(5));
                    }
                } while (cursor3.moveToNext());
                cursor3.close();
                if (mNameArray.size() != 0) {
                    updateListView();
                    return;
                }

            }
            cityName = getString(R.string.str_query_fail_tip);
            mNameArray.clear();
            mNameArray.add(cityName);
            updateListView();
            return;
        }

       //number
        Cursor cursor = getContentResolver().query(AreaCodeTable.NATIVE_URI,
                null, inputStr, null, null);
        if (cursor == null) {
            cityName = getString(R.string.str_query_fail_tip);
            mNameArray.clear();
            mNameArray.add(cityName);
            updateListView();
            return;
        }
        if (cursor.moveToFirst()) {
            String str1;
            do {
                str1 = cursor.getString(0);
                mNameArray.add(str1);
            } while (cursor.moveToNext());
            cursor.close();
            updateListView();
            return;
        }

        cityName = getString(R.string.str_query_fail_tip);
        mNameArray.clear();
        mNameArray.add(cityName);
        updateListView();
        return;
    }

    // search results (list view)
    private void updateListView() {

        adapter = new ArrayAdapter<String>(this, R.layout.search_list,
                mNameArray);

        AlertDialog.Builder builder = new AlertDialog.Builder(
                AreaSearchFirstPage.this).setNegativeButton(null, null)
                .setTitle(R.string.search_title).setAdapter(adapter, null);

        AlertDialog dlg = builder.create();
        dlg.setCanceledOnTouchOutside(true);
        dlg.show();

    }

    public static void showMessageBox(Context context, int msgId) {
        new AlertDialog.Builder(context)
                .setIconAttribute(android.R.attr.alertDialogIcon)
                .setTitle(R.string.message_box_title).setMessage(msgId)
                .setPositiveButton(android.R.string.ok, null).show();
    }

    class MyHandler extends Handler {

        public MyHandler() {
        }

        public MyHandler(Looper L) {
            super(L);
        }

        @Override
        public void handleMessage(Message msg) {

            super.handleMessage(msg);
            switch (msg.what) {
                case SHOW_TOAST:
                    Bundle data = msg.getData();
                    boolean bSuccess = data.getBoolean("isSuccess");
                    Toast toast;
                    if(bSuccess)
                        toast = Toast.makeText(getBaseContext(),
                                R.string.update_success, 3);
                    else
                        toast = Toast.makeText(getBaseContext(),
                                R.string.update_lastest, 3);
                    toast.show();
                    break;
                case NOTIFY_DATA_CHANGED:
                    list1 = (ExpandableListView) findViewById(R.id.list1);
                    list1.setAdapter(expand1);
                    expand1.notifyDataSetChanged();
                    list2 = (ExpandableListView) findViewById(R.id.list2);
                    list2.setAdapter(expand2);
                    expand2.notifyDataSetChanged();
                    break;
            }
        }
    }

    public static String phoneStrorage() {
        String path = null;
        File file = null;

        Method method;
        try {
            method = Environment.class.getMethod("getExternalStorageDirectory",
                    (Class[]) null);
            file = (File) method.invoke(null, (Object[]) null);
            path = file.getPath();

        } catch (NoSuchMethodException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        } catch (IllegalArgumentException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        } catch (IllegalAccessException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        } catch (InvocationTargetException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }

        return path;
    }
    public class DownloadDatabase implements Runnable {

        @Override
        public void run() {
            // TODO Auto-generated method stub

            String dbpath = phoneStrorage();
            boolean bSuccess=false;
            try {
                if(!dbpath.endsWith("/"))
                    dbpath=dbpath+"/";
                FileInputStream dbfrom = new FileInputStream(dbpath+"AreaSearch/acnumber.db");
                FileOutputStream dbto = new FileOutputStream("/data/data/com.dean.areasearch/files/acnumber.db");
                byte bt[] = new byte[1024];
                int count;
                while ((count = dbfrom.read(bt))>0){

                    dbto.write(bt, 0, count);
                    dbto.flush();
                }

                dbfrom.close();
                dbto.close();
                bSuccess=true;
            } catch (FileNotFoundException e) {
                // TODO Auto-generated catch block
                e.printStackTrace();
            } catch (IOException e) {
                // TODO Auto-generated catch block
                e.printStackTrace();
            }

            Message copySuccessMsg = new Message();
            Bundle data = new Bundle();
            data.putBoolean("isSuccess",bSuccess);
            copySuccessMsg.setData(data);
            copySuccessMsg.what = SHOW_TOAST;
            AreaSearchFirstPage.this.myHandler.sendMessage(copySuccessMsg);
        }
    }

    public static boolean isDigitalStr(String str) {
        int count = str.length();
        if (count == 0)
            return false;
        for (int i = 0; i < count; i++) {
            if (!(str.charAt(i) >= '0' && str.charAt(i) <= '9')) {
                return false;
            }
        }
        return true;
    }

    public static boolean isChinese(char c) {

        Character.UnicodeBlock ub = Character.UnicodeBlock.of(c);

        if (ub == Character.UnicodeBlock.CJK_UNIFIED_IDEOGRAPHS
                || ub == Character.UnicodeBlock.CJK_COMPATIBILITY_IDEOGRAPHS
                || ub == Character.UnicodeBlock.CJK_UNIFIED_IDEOGRAPHS_EXTENSION_A
                || ub == Character.UnicodeBlock.GENERAL_PUNCTUATION
                || ub == Character.UnicodeBlock.CJK_SYMBOLS_AND_PUNCTUATION
                || ub == Character.UnicodeBlock.HALFWIDTH_AND_FULLWIDTH_FORMS) {

            return true;
        }
        return false;
    }

    public static boolean isEnglish(char c) {

        if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) {

            return true;
        }
        return false;
    }

    public static String toHexString(byte[] b) {
        char HEX_DIGITS[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
                'A', 'B', 'C', 'D', 'E', 'F' };

        StringBuilder sb = new StringBuilder(b.length * 2);
        for (int i = 0; i < b.length; i++) {

            sb.append(HEX_DIGITS[(b[i] & 0xf0) >>> 4]);
            sb.append(HEX_DIGITS[b[i] & 0x0f]);

        }

        return sb.toString();

    }
}
