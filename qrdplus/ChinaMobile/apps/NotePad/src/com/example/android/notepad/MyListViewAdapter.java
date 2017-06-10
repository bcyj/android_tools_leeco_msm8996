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

package com.example.android.notepad;

import android.app.Activity;
import android.content.Context;
import android.database.Cursor;
import android.text.format.DateFormat;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.CursorAdapter;
import android.widget.ListView;
import android.widget.TextView;

public class MyListViewAdapter extends CursorAdapter implements ConstInterfaceNotesList {
    private static final String TAG = "MyListViewAdapter";
    private LayoutInflater mInflater;
    private Context mContext;
    private ListView mListView;

    public MyListViewAdapter(Context context, Cursor c) {
        super(context, c);
        mContext = context;
        mListView = (ListView) ((Activity) context).findViewById(R.id.note);
        mInflater = LayoutInflater.from(context);
    }

    @Override
    public void bindView(View view, Context context, Cursor cursor) {
        // Don't read title which is not used. To read content instead.
        final String strTitle = cursor.getString(COLUMN_INDEX_NOTE);
        final Long lModifiedDate = cursor.getLong(COLUMN_INDEX_MODIFIED_DATE);
        String strModifiedDate = null;
        strModifiedDate = DateFormat.getMediumDateFormat(context).format(lModifiedDate);
        // It will cut the string like the mode "Jan 2" from "Jan 2, 2013"
        int iYearIdx = strModifiedDate.indexOf(",");
        if (iYearIdx != -1)
            strModifiedDate = strModifiedDate.substring(0, iYearIdx);
        TextView textNote = (TextView) view.findViewById(R.id.noteContent);
        TextView textModifiedDate = (TextView) view.findViewById(R.id.noteDate);
        textNote.setText(strTitle);
        textModifiedDate.setText(strModifiedDate);

        if (mListView.isItemChecked(cursor.getPosition())) {
            if (view != null) {
                view.setBackgroundColor(mContext.getResources().getColor(
                        android.R.color.holo_blue_dark));
            }
        } else {
            if (view != null) {
                view.setBackgroundColor(mContext.getResources().getColor(
                        android.R.color.transparent));
            }
        }
    }

    @Override
    public View newView(Context context, Cursor cursor, ViewGroup parent) {
        View ret = mInflater.inflate(R.layout.noteslist_item, parent, false);
        return ret;
    }

}
