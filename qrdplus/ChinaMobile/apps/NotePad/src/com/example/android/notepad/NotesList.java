/*
 * Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 * Not a Contribution, Apache license notifications and license are retained
 * for attribution purposes only.
 */
/*
 * Copyright (C) 2011 The Android Open Source Project
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

import java.lang.reflect.Field;

import com.example.android.notepad.NotePad;
import com.example.android.notepad.NotePad.Notes;

import android.widget.AbsListView;
import android.widget.AdapterView;
import android.widget.LinearLayout;
import android.widget.ListView;
import android.widget.SpinnerAdapter;
import android.widget.TextView;
import android.view.ActionMode;
import android.view.Gravity;
import android.view.KeyEvent;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.app.ActionBar;
import android.app.Activity;
import android.app.AlertDialog;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.ViewGroup.LayoutParams;
import android.content.Intent;
import android.database.Cursor;
import android.database.DataSetObserver;
import android.net.Uri;
import android.os.Bundle;
import android.util.Log;
import android.util.SparseBooleanArray;
import android.content.ContentUris;
import android.content.Context;
import android.content.DialogInterface;
import android.widget.Spinner;

public class NotesList extends Activity implements ConstInterfaceNotesList {

    // For logging and debugging
    private static final String TAG = "NotesList";

    private static final int DEFAULT_SELECTION_ITEM = 2;

    private ActionMode Lactionmode;
    private ActionBar mActionBar;
    private Boolean mSeleting = false;
    private ListView mListView;
    private Uri muri;

    private Context mContext;
    private Cursor mCursorNotepad;
    private MyListViewAdapter mListAdapter = null;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        setContentView(R.layout.main);
        mContext = this;

        setDefaultKeyMode(DEFAULT_KEYS_SHORTCUT);
        mListView = (ListView) findViewById(R.id.note);

        Intent intent = getIntent();
        if (intent.getData() == null) {
            intent.setData(NotePad.Notes.CONTENT_URI);
        }

        mListView.setChoiceMode(ListView.CHOICE_MODE_MULTIPLE_MODAL);
        mListView.setMultiChoiceModeListener(new ModeCallback());
        mActionBar = getActionBar();
        mActionBar.show();

        mListView.setOnItemClickListener(new AdapterView.OnItemClickListener() {

            @Override
            public void onItemClick(AdapterView<?> arg0, View v, int position,
                    long id) {
                // Constructs a new URI from the incoming URI and the row ID
                Uri uri = ContentUris.withAppendedId(getIntent().getData(), id);

                // Gets the action from the incoming Intent
                String action = getIntent().getAction();

                // Handles requests for note data
                if (Intent.ACTION_PICK.equals(action) || Intent.ACTION_GET_CONTENT.equals(action)) {

                    // Sets the result to return to the component that called
                    // this Activity. The
                    // result contains the new URI
                    setResult(RESULT_OK, new Intent().setData(uri));
                } else {

                    // Sends out an Intent to start an Activity that can handle
                    // ACTION_EDIT. The
                    // Intent's data is the note ID URI. The effect is to call
                    // NoteEdit.
                    startActivity(new Intent(Intent.ACTION_EDIT, uri));
                }
            }
        });

        // Sets the ListView's adapter to be the cursor adapter that was just
        // created.
        UpdateAdapterData();

    }

    public void UpdateAdapterData()
    {

        StringBuilder path = new StringBuilder();
        muri = Uri.withAppendedPath(Notes.CONTENT_URI, path.toString());

        if (mCursorNotepad != null) {
            mCursorNotepad.close();
            mCursorNotepad = null;
        }
        mCursorNotepad = getContentResolver().query(muri, PROJECTION, null, null, null);

        if (mCursorNotepad != null) {
            mListAdapter = new MyListViewAdapter(this, mCursorNotepad);
            mListView.setAdapter(mListAdapter);
        }
    }

    @Override
    public boolean onPrepareOptionsMenu(Menu menu) {
        super.onPrepareOptionsMenu(menu);
        if (mListAdapter == null || mListAdapter.getCount() == 0) {
            menu.findItem(R.id.menu_delete_sel).setEnabled(false);
            menu.findItem(R.id.menu_delete_all).setEnabled(false);
        } else {
            menu.findItem(R.id.menu_delete_sel).setEnabled(true);
            menu.findItem(R.id.menu_delete_all).setEnabled(true);
        }
        return true;
    }

    private class ModeCallback implements ListView.MultiChoiceModeListener,
            Spinner.OnItemSelectedListener, SpinnerAdapter
    {
        private View mMultiSelectActionBarView;
        private Spinner mSpin;
        private View mTitleView;
        private View mDropView;
        private TextView mTextView;
        private TextView mTitleTextView;

        public ModeCallback() {
            mTitleView = null;
            mDropView = null;
        }

        @Override
        public int getCount() {
            return 1;
        }

        @Override
        public Object getItem(int position) {
            return getSelectTitle();
        }

        @Override
        public long getItemId(int position) {
            return position;
        }

        @Override
        public int getItemViewType(int position) {
            return 0;
        }

        @Override
        public View getView(int position, View convertView, ViewGroup parent) {
            if (mTitleView == null) {
                mTitleView = new LinearLayout(NotesList.this);
                ((LinearLayout) mTitleView).setOrientation(LinearLayout.VERTICAL);
                mTitleTextView = new TextView(NotesList.this);
                LinearLayout.LayoutParams params = new LinearLayout.LayoutParams(
                        LayoutParams.FILL_PARENT, LayoutParams.WRAP_CONTENT);
                mTitleTextView.setLayoutParams(params);
                ((LinearLayout) mTitleView).addView(mTitleTextView);
                mTitleView.setLayoutParams(new LinearLayout.LayoutParams(LayoutParams.FILL_PARENT,
                        LayoutParams.FILL_PARENT));
            }
            mTitleTextView.setText(getSelectTitle());
            return mTitleView;
        }

        @Override
        public int getViewTypeCount() {
            return 1;
        }

        @Override
        public boolean hasStableIds() {
            return false;
        }

        @Override
        public boolean isEmpty() {
            return false;
        }

        @Override
        public void registerDataSetObserver(DataSetObserver observer) {
            // 0 for the first item, set the default selection item 2 to
            // avoid working when touch other area.
            mSpin.setSelection(DEFAULT_SELECTION_ITEM);
        }

        @Override
        public void unregisterDataSetObserver(DataSetObserver observer) {
            long i = mSpin.getSelectedItemId();
            if (i == 0) {
                boolean selected = !isAllSelected();
                int all = mListView.getCount();
                for (int j = 0; j < all; ++j) {
                    mListView.setItemChecked(j, selected);
                }
                mListView.refreshDrawableState();
            }
        }

        @Override
        public View getDropDownView(int position, View convertView, ViewGroup parent) {
            if (mDropView == null) {
                mDropView = new LinearLayout(NotesList.this);
                ((LinearLayout) mDropView).setOrientation(LinearLayout.HORIZONTAL);
                mTextView = new TextView(NotesList.this);
                LinearLayout.LayoutParams params = new LinearLayout.LayoutParams(
                        LayoutParams.FILL_PARENT, LayoutParams.WRAP_CONTENT);
                params.setMargins(20, 0, 0, 0);
                params.gravity = Gravity.CENTER_VERTICAL;
                mTextView.setLayoutParams(params);
                ((LinearLayout) mDropView).addView(mTextView);
                mDropView.setLayoutParams(new LinearLayout.LayoutParams(LayoutParams.MATCH_PARENT,
                        LayoutParams.MATCH_PARENT));
            }
            if (mTitleView != null)
                mDropView.setLayoutParams(new AbsListView.LayoutParams(mTitleView.getWidth() * 2,
                        mTitleView.getHeight() * 2));
            if (isAllSelected())
                mTextView.setText(R.string.menu_deselect_all);
            else
                mTextView.setText(R.string.menu_select_all);
            return mDropView;
        }

        private void prepareObj() {
            if (mMultiSelectActionBarView == null) {

                mMultiSelectActionBarView = (ViewGroup) LayoutInflater.from(mContext)
                        .inflate(R.layout.conversation_list_multi_select_actionbar, null);
                mSpin = ((Spinner) mMultiSelectActionBarView.findViewById(R.id.select_actions));
                mSpin.setAdapter(this);
                mSpin.setEnabled(true);
                mSpin.setOnItemSelectedListener(this);
                mSpin.setSelected(true);
                mSpin.setSelection(DEFAULT_SELECTION_ITEM);
            }
        }

        private String getSelectTitle() {
            int listCnt = mListView.getCheckedItemCount();
            String title = NotesList.this.getString(R.string.menu_delete_sel);
            return "" + listCnt + " " + title;
        }

        private boolean isAllSelected() {
            int listSelCnt = mListView.getCheckedItemCount();
            int listCnt = mListView.getCount();
            if (listSelCnt == listCnt)
                return true;
            else
                return false;
        }

        public boolean onCreateActionMode(ActionMode mode, Menu menu) {
            MenuInflater inflater = getMenuInflater();
            inflater.inflate(R.menu.conversation_multi_select_menu, menu);
            prepareObj();
            mode.setCustomView(mMultiSelectActionBarView);
            EnterMultiSelecting();
            return true;
        }

        public boolean onPrepareActionMode(ActionMode mode, Menu menu) {
            prepareObj();
            mode.setCustomView(mMultiSelectActionBarView);
            return true;
        }

        public boolean onActionItemClicked(ActionMode mode, MenuItem item) {
            Lactionmode = mode;

            switch (item.getItemId()) {
                case R.id.delete: {
                    confirmDeleteMultiActionMode();
                    break;
                }
                default:
                    break;
            }
            return true;
        }

        public void onDestroyActionMode(ActionMode mode) {
            exitMultiSelectView();
        }

        public void onItemCheckedStateChanged(ActionMode mode, int position,
                long id, boolean checked) {
            prepareObj();
            if (mTitleView != null) {
                mTitleTextView.setText(getSelectTitle());
                mTitleTextView.refreshDrawableState();
                mTitleTextView.invalidate();
            }
            mSpin.invalidate();
            mode.setTitle(getSelectTitle());
            Uri uri = ContentUris.withAppendedId(getIntent().getData(), id);
            setListViewBk(mSeleting, position);
            String action = getIntent().getAction();
            if (Intent.ACTION_PICK.equals(action)
                    || Intent.ACTION_GET_CONTENT.equals(action)) {
                setResult(RESULT_OK, new Intent().setData(uri));
            }
        }

        @Override
        public void onItemSelected(AdapterView<?> arg0, android.view.View arg1,
                int arg2, long arg3) {
        }

        @Override
        public void onNothingSelected(AdapterView<?> arg0) {
        }

    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
            case R.id.menu_add:
                startActivity(new Intent(Intent.ACTION_INSERT, getIntent().getData()));
                return true;
            case R.id.menu_delete_sel:
                ModeCallback mc = new ModeCallback();
                ActionMode am = mListView.startActionMode(mc);
                try {
                    Field field = mListView.getClass().getSuperclass()
                            .getDeclaredField("mChoiceActionMode");
                    field.setAccessible(true);
                    field.set(mListView, am);
                } catch (Exception ignored) {
                }

                mListView.setMultiChoiceModeListener(mc);
                return true;
            case R.id.menu_delete_all:
                deleteAll();
                return true;
            default:
                return super.onOptionsItemSelected(item);
        }
    }

    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {
        switch (keyCode) {
            case KeyEvent.KEYCODE_BACK:
                if (mSeleting) {
                    exitMultiSelectView();
                    Log.d(TAG, "ExitMultiSelectView");
                    return true;
                }
                break;

        }
        Log.d(TAG, "onKeyDown key:" + keyCode + " event:" + event);
        return super.onKeyDown(keyCode, event);
    }

    private void clearSelect() {
        mListView.clearChoices();
        mListView.invalidateViews();
    }

    public void EnterMultiSelecting() {
        mSeleting = true;
        mListView.invalidateViews();
    }

    private void exitMultiSelectView() {
        clearSelect();
        mSeleting = false;
        setListViewBk(mSeleting, 1);
        try {
            Field field = mListView.getClass().getSuperclass()
                    .getDeclaredField("mChoiceActionMode");
            field.setAccessible(true);
            field.set(mListView, null);
        } catch (Exception ignored) {
        }
    }

    public void deleteAll() {
        final Uri noteUri = getIntent().getData();
        new AlertDialog.Builder(mContext)
                .setTitle(R.string.menu_delete_all)
                .setMessage(R.string.confirm_delete_notes)
                .setNegativeButton(android.R.string.cancel, null)
                .setPositiveButton(android.R.string.ok, new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface arg0, int arg1) {
                        getContentResolver().delete(noteUri, null, null);
                    }
                }).show();
    }

    private void confirmDeleteMultiActionMode() {
        new AlertDialog.Builder(this)
                .setTitle(R.string.menu_delete_sel)
                .setMessage(R.string.confirm_delete_notes)
                .setNegativeButton(android.R.string.cancel, null)
                .setPositiveButton(android.R.string.ok,
                        new DialogInterface.OnClickListener() {

                            @Override
                            public void onClick(DialogInterface arg0, int arg1) {
                                // Delete the note that the context menu is for
                                Uri noteUri;
                                SparseBooleanArray booleanArray = mListView
                                        .getCheckedItemPositions();
                                int size = booleanArray.size();
                                int topIndex = mListView.getFirstVisiblePosition();
                                for (int j = 0; j < size; j++) {
                                    int position = booleanArray.keyAt(j);
                                    if (mListView.isItemChecked(position)) {
                                        noteUri = ContentUris.withAppendedId(
                                                getIntent().getData(),
                                                mListView.getAdapter().getItemId(position));

                                        getContentResolver().delete(noteUri,
                                                null, null);
                                        if(j>=topIndex) {
                                            View view = mListView.getChildAt(position-topIndex);
                                            if(view!=null)
                                                view.setBackgroundColor(
                                                        mContext.getResources().getColor(
                                                        android.R.color.transparent));
                                        }
                                    }
                                }
                                exitMultiSelectView();
                                Lactionmode.finish();
                            }

                        }).show();
    }

    private void setListViewBk(boolean state, int position) {
        int topIndex = mListView.getFirstVisiblePosition();
        if (state == false) {
            int count = mListView.getCount();
            for (int i = 0; i < count; i++) {
                if(i>=topIndex) {
                    View view = mListView.getChildAt(i-topIndex);
                    if(view!=null)
                        view.setBackgroundColor(getResources()
                            .getColor(android.R.color.transparent));
                }
            }
            Log.d(TAG, "SetListViewBk state:false");
        } else {
            if (mListView.isItemChecked(position)) {
                View view = mListView.getChildAt(position-topIndex);
                if(view!=null)
                    view.setBackgroundColor(getResources().getColor(
                        android.R.color.holo_blue_dark));

            } else {
                View view = mListView.getChildAt(position-topIndex);
                if(view!=null)
                    view.setBackgroundColor(getResources()
                        .getColor(android.R.color.transparent));
            }
        }
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        super.onCreateOptionsMenu(menu);
        MenuInflater inflater = getMenuInflater();
        inflater.inflate(R.menu.list_options_menu, menu);
        return true;
    }
}
