/*
 * Copyright (c) 2013 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Qualcomm Technologies Confidential and Proprietary.
 *
 * Not a Contribution.
 * Apache license notifications and license are retained
 * for attribution purposes only.
 */
/*
 * Copyright (C) 2007 The Android Open Source Project
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

import android.app.ActionBar;
import android.app.Activity;
import android.app.AlertDialog;
import android.content.ContentValues;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.res.Resources;
import android.database.Cursor;
import android.net.Uri;
import android.os.Bundle;
import android.text.Editable;
import android.text.InputFilter;
import android.text.Spanned;
import android.text.TextWatcher;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.widget.EditText;
import android.widget.Toast;

public class NoteEditor extends Activity implements ConstInterfaceNotesList {
    // For logging and debugging purposes
    private static final String TAG = "NoteEditor";
    // A label for the saved state of the activity
    private static final String ORIGINAL_CONTENT = "origContent";

    private static final int MENU_ITEM_SAVE = Menu.FIRST;
    private static final int MENU_ITEM_REVERT = Menu.FIRST + 1;
    private static final int MENU_ITEM_DELETESINGLE = Menu.FIRST + 2;
    private Context mContext;
    private static final int STATE_EDIT = 0;
    private static final int STATE_INSERT = 1;
    public static final int MAX_LENGTH = 500;
    // Global mutable variables
    private int mState;
    private Uri mUri;
    private static final int MODE_SAVE = 1;
    private static final int MODE_DELETE = 2;
    private static final int MODE_REVERT = 3;
    private Cursor mCursor;
    private EditText mEditText;
    private String mOriginalContent;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.note_editor);
        mContext = this;
        final Intent intent = getIntent();
        final String action = intent.getAction();

        // For an edit action:
        if (Intent.ACTION_EDIT.equals(action)) {

            // Sets the Activity state to EDIT, and gets the URI for the data to
            // be edited.
            mState = STATE_EDIT;
            mUri = intent.getData();

            // For an insert or paste action:
        } else if (Intent.ACTION_INSERT.equals(action)) {

            // Sets the Activity state to INSERT, gets the general note URI, and
            // inserts an
            // empty record in the provider
            mState = STATE_INSERT;
            mUri = getContentResolver().insert(intent.getData(), null);
            setResult(RESULT_OK, (new Intent()).setAction(mUri.toString()));
            Log.d(TAG, "SetResult for EditNote");
            if (mUri == null) {

                // Writes the log identifier, a message, and the URI that
                // failed.
                Log.e(TAG, "Failed to insert new note into " + getIntent().getData());

                // Closes the activity.
                finish();
                return;
            }
        }
        else
        {
            // Logs an error that the action was not understood, finishes the
            // Activity, and
            // returns RESULT_CANCELED to an originating Activity.
            Log.e(TAG, "Unknown action, exiting");
            finish();
            return;
        }

        mCursor = getContentResolver().query(mUri, PROJECTION, null, null, null);

        // Gets a handle to the EditText in the the layout.
        mEditText = (EditText) findViewById(R.id.note);
        mEditText.addTextChangedListener(mNoteEditorWatcher);
        mEditText.setFocusable(true);
        mEditText.setFilters(new InputFilter[] {
                new InputFilter.LengthFilter(MAX_LENGTH) {

                    @Override
                    public CharSequence filter(CharSequence source, int start, int end,
                            Spanned dest, int dstart, int dend) {
                        //calculate remaining space
                        int space = MAX_LENGTH - (dest.length() - (dend - dstart));
                        //do check when space less than input length
                        if (space < end - start) {
                            CheckContentLength(mEditText.getText().toString(), MAX_LENGTH);
                        }
                        return super.filter(source, start, end, dest, dstart, dend);
                    }

                }
        });

        /*
         * If this Activity had stopped previously, its state was written the
         * ORIGINAL_CONTENT location in the saved Instance state. This gets the
         * state.
         */
        if (savedInstanceState != null) {
            mOriginalContent = savedInstanceState.getString(ORIGINAL_CONTENT);
        }
        ActionBar actionBar = getActionBar();
        actionBar.setDisplayHomeAsUpEnabled(true);
    }

    private final TextWatcher mNoteEditorWatcher = new TextWatcher() {

        @Override
        public void afterTextChanged(Editable arg0) {

        }

        @Override
        public void beforeTextChanged(CharSequence arg0, int arg1, int arg2,
                int arg3) {

        }

        @Override
        public void onTextChanged(CharSequence arg0, int arg1, int arg2,
                int arg3) {
            String str = arg0.toString();
            Log.d(TAG, "onTextChanged " + str);
            CheckContentLength(str, MAX_LENGTH);
        }

    };

    public void CheckContentLength(String str, int ContentLengthLimit)
    {

        if (str.length() >= ContentLengthLimit) {
            Toast.makeText(this, R.string.content_full, Toast.LENGTH_SHORT).show();
        }
    }

    private void saveNoteData() {
        if (mCursor != null) {

            // Get the current note text.
            String text = mEditText.getText().toString();
            int length = text.length();

            if (isFinishing() && (length == 0)) {
                setResult(RESULT_CANCELED);
                deleteNote();
            } else if (mState == STATE_EDIT) {

                updateNote(text, null);
            } else if (mState == STATE_INSERT) {
                updateNote(text, text);
                mState = STATE_EDIT;
            }
        }
    }

    @Override
    protected void onPause() {
        super.onPause();
        saveNoteData();
    }

    @Override
    protected void onSaveInstanceState(Bundle outState) {
        // Save away the original text, so we still have it if the activity
        // needs to be killed while paused.
        outState.putString(ORIGINAL_CONTENT, mOriginalContent);
    }

    private void ConfigTitleName() {
        if (mState == STATE_EDIT) {
            // Set the title of the Activity to include the note title
            int colTitleIndex = mCursor.getColumnIndex(NotePad.Notes.COLUMN_NAME_NOTE);// COLUMN_NAME_TITLE);
            String title = mCursor.getString(colTitleIndex);
            Resources res = getResources();
            String text = String.format(res.getString(R.string.title_edit), title);
            setTitle(text);
            // Sets the title to "create" for inserts
        } else if (mState == STATE_INSERT) {
            setTitle(getText(R.string.title_create));
        }
    }

    private void ResumeNote() {
        if (mCursor != null) {
            // Requery in case something changed while paused (such as the
            // title)
            mCursor.requery();
            mCursor.moveToFirst();
            ConfigTitleName();
            int colNoteIndex = mCursor.getColumnIndex(NotePad.Notes.COLUMN_NAME_NOTE);
            String note = mCursor.getString(colNoteIndex);
            mEditText.setTextKeepState(note);

            // Stores the original note text, to allow the user to revert
            // changes.
            if (mOriginalContent == null) {
                mOriginalContent = note;
            }
        } else {
            setTitle(getText(R.string.error_title));
            mEditText.setText(getText(R.string.error_message));
        }
    }

    @Override
    protected void onResume() {
        super.onResume();
        ResumeNote();
    }

    @Override
    public boolean onPrepareOptionsMenu(Menu menu) {
        // Check if note has changed and enable/disable the revert option
        menu.clear();
        menu.add(0, MENU_ITEM_DELETESINGLE, 0, R.string.menu_delete)
                .setIcon(R.drawable.ic_menu_delete)
                .setShowAsAction(MenuItem.SHOW_AS_ACTION_ALWAYS);
        return super.onPrepareOptionsMenu(menu);
    }

    /**
     * This method is called when a menu item is selected.
     */
    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        // Handle all of the possible menu actions.
        switch (item.getItemId()) {
            case MENU_ITEM_SAVE:
                OperateNote(MODE_SAVE);
                break;
            case MENU_ITEM_DELETESINGLE:
                OperateNote(MODE_DELETE);
                break;
            case MENU_ITEM_REVERT:
                OperateNote(MODE_REVERT);
                break;
        }
        return super.onOptionsItemSelected(item);
    }

    private final void updateNote(String text, String title) {

        // Sets up a map to contain values to be updated in the provider.
        ContentValues values = new ContentValues();
        values.put(NotePad.Notes.COLUMN_NAME_MODIFICATION_DATE, System.currentTimeMillis());

        // This puts the desired notes text into the map.
        values.put(NotePad.Notes.COLUMN_NAME_NOTE, text);

        getContentResolver().update(mUri, values, null, null);
    }

    /**
     * This helper method cancels the work done on a note. It deletes the note
     * if it was newly created, or reverts to the original text of the note i
     */
    private final void cancelNote() {
        if (mCursor != null) {
            if (mState == STATE_EDIT) {
                // Put the original note text back into the database
                mCursor.close();
                mCursor = null;
                ContentValues values = new ContentValues();
                values.put(NotePad.Notes.COLUMN_NAME_NOTE, mOriginalContent);
                getContentResolver().update(mUri, values, null, null);
            } else if (mState == STATE_INSERT) {
                // We inserted an empty note, make sure to delete it
                deleteNote();
            }
        }
        setResult(RESULT_CANCELED);
        finish();
    }

    private final void OperateNote(int mOperMode) {
        if (mOperMode == MODE_SAVE) {
            String text = mEditText.getText().toString();
            updateNote(text, null);
            finish();
        }
        if (mOperMode == MODE_DELETE) {
            String text = mEditText.getText().toString();
            if (text != null && text.length() > 0) {
                new AlertDialog.Builder(mContext)
                        .setTitle(R.string.menu_delete)
                        .setMessage(R.string.confirm_delete_notes)
                        .setNegativeButton(android.R.string.cancel, null)
                        .setPositiveButton(android.R.string.ok,
                                new DialogInterface.OnClickListener() {
                                    @Override
                                    public void onClick(DialogInterface arg0, int arg1) {
                                        deleteNote();
                                        finish();
                                    }
                                }).show();
            } else {
                deleteNote();
                finish();
            }
        }
        if (mOperMode == MODE_REVERT) {
            cancelNote();
        }

    }

    /**
     * Take care of deleting a note. Simply deletes the entry.
     */
    private final void deleteNote() {
        if (mCursor != null) {
            mCursor.close();
            mCursor = null;
            getContentResolver().delete(mUri, null, null);
            mEditText.setText("");
        }
    }

}
