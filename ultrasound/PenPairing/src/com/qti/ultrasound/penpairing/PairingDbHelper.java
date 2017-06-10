/******************************************************************************
 * ---------------------------------------------------------------------------
 *  Copyright (C) 2013-2014 Qualcomm Technologies, Inc.
 *  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
 *  ---------------------------------------------------------------------------
 *******************************************************************************/

package com.qti.ultrasound.penpairing;

import com.qti.ultrasound.penpairing.PairingContract.PairingEntry;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.ContentValues;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.database.Cursor;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteOpenHelper;
import android.text.InputFilter;
import android.text.InputType;
import android.text.Spanned;
import android.widget.EditText;

import java.util.ArrayList;

public class PairingDbHelper extends SQLiteOpenHelper {
    // If you change the database schema, you must increment the database
    // version.
    public static final int DATABASE_VERSION = 3;

    public static final String DATABASE_NAME = "PenPairing.db";

    private static final String TEXT_TYPE = " TEXT";

    private static final String INTEGER_TYPE = " INTEGER";

    private static final String NOT_NULL_CONSTARINT = " NOT NULL";

    private static final String COMMA_SEP = ",";

    private static final String SQL_CREATE_ENTRIES = "CREATE TABLE " + PairingEntry.TABLE_NAME
            + " (" + PairingEntry._ID + INTEGER_TYPE + " PRIMARY KEY" + NOT_NULL_CONSTARINT
            + COMMA_SEP + PairingEntry.COLUMN_NAME_PEN_ID + INTEGER_TYPE + NOT_NULL_CONSTARINT
            + COMMA_SEP + PairingEntry.COLUMN_NAME_PEN_TYPE + TEXT_TYPE + NOT_NULL_CONSTARINT
            + COMMA_SEP + PairingEntry.COLUMN_NAME_PEN_NAME + TEXT_TYPE + NOT_NULL_CONSTARINT
            + " UNIQUE" + " )";

    private static final String SQL_DELETE_ENTRIES = "DROP TABLE IF EXISTS "
            + PairingEntry.TABLE_NAME;

    private Context context;

    public static final InputFilter penNameFilter = new InputFilter() {
        @Override
        public CharSequence filter(CharSequence source, int start, int end, Spanned dest,
                int dstart, int dend) {
            if (source.length() != 0) {
                char currentChar = source.charAt(0);
                if (dstart <= 15
                        && (Character.isLetterOrDigit(currentChar)
                                || Character.isSpaceChar(currentChar)
                                || Character.toString(currentChar).equals("_") || Character
                                .toString(currentChar).equals("-"))) {
                    return source;
                }
            }
            return "";
        }
    };

    public PairingDbHelper(Context context) {
        super(context, DATABASE_NAME, null, DATABASE_VERSION);
        this.context = context;
    }

    @Override
    public void onCreate(SQLiteDatabase db) {
        db.execSQL(SQL_CREATE_ENTRIES);
    }

    @Override
    public void onUpgrade(SQLiteDatabase db, int oldVersion, int newVersion) {
        // When scheme is changes, reset the DB.
        db.execSQL(SQL_DELETE_ENTRIES);
        onCreate(db);
    }

    public int addNamedPen(int penId, String penType, String penName) {
        SQLiteDatabase db = getWritableDatabase();
        ContentValues values = new ContentValues();

        values.put(PairingEntry.COLUMN_NAME_PEN_ID, penId);
        values.put(PairingEntry.COLUMN_NAME_PEN_TYPE, penType);
        values.put(PairingEntry.COLUMN_NAME_PEN_NAME, penName);

        if (-1 == db.insert(PairingEntry.TABLE_NAME, null, values)) {
            return -1;
        }
        return 0;
    }

    public void addPen(final int penId, final String penType) {
        final EditText edit = new EditText(context);
        edit.setInputType(InputType.TYPE_TEXT_FLAG_NO_SUGGESTIONS);
        edit.setFilters(new InputFilter[] {
                PairingDbHelper.penNameFilter
        });

        final PairingDbHelper pairingDbHelper = this;
        AlertDialog.Builder alert = new AlertDialog.Builder(context);

        alert.setTitle("Enter a pen name");
        alert.setMessage("Enter a name for the new pen");
        alert.setView(edit);
        alert.setPositiveButton("OK", new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int whichButton) {
                String penName = edit.getText().toString();
                SQLiteDatabase db = getWritableDatabase();
                ContentValues values = new ContentValues();

                values.put(PairingEntry.COLUMN_NAME_PEN_ID, penId);
                values.put(PairingEntry.COLUMN_NAME_PEN_TYPE, penType);
                values.put(PairingEntry.COLUMN_NAME_PEN_NAME, penName);

                if ("".equals(penName) || -1 == db.insert(PairingEntry.TABLE_NAME, null, values)) {

                    AlertDialog.Builder alert = new AlertDialog.Builder(context);
                    String message = "".equals(penName) ? "Empty name entered"
                            : "Pen name already exists";
                    alert.setTitle("Pen name error").setMessage(message)
                    .setPositiveButton("Ok", new DialogInterface.OnClickListener() {
                        @Override
                        public void onClick(DialogInterface dialog, int whichButton) {
                            addPen(penId, penType);
                        }
                    });
                    alert.show();
                } else { // No error in name
                    PairedPensActivity.changeCurrent(penName, pairingDbHelper, context
                            .getSharedPreferences(PairedPensActivity.NAME_SHARED_PREFERENCE, 0)
                            .edit());
                    PairedPensActivity.restartPenDaemon(context);
                    context.startActivity(new Intent(context, PairedPensActivity.class));
                    ((Activity)context).finish();
                }
            }
        });

        alert.setNegativeButton("Cancel", new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int whichButton) {
                if (context instanceof SemiAutomaticActivity) {
                    ((Activity)context).finish();
                }
            }
        });
        alert.setCancelable(false);
        alert.show();
    }

    public boolean changePenName(String to, String from) {
        SQLiteDatabase db = getReadableDatabase();

        Cursor c = db.query(PairingEntry.TABLE_NAME, new String[] {
                PairingEntry.COLUMN_NAME_PEN_NAME, PairingEntry.COLUMN_NAME_PEN_TYPE, PairingEntry.COLUMN_NAME_PEN_ID
        }, PairingEntry.COLUMN_NAME_PEN_NAME + "=?", new String[] {
                to
        }, null, null, null);

        if (null != c && 0 < c.getCount()) {
            AlertDialog.Builder alert = new AlertDialog.Builder(context);
            alert.setTitle("Pen Renaming Error");
            alert.setMessage("Name '" + to + "' already exists. Please enter a new name.");
            alert.setPositiveButton("Ok", new DialogInterface.OnClickListener() {
                @Override
                public void onClick(DialogInterface dialog, int whichButton) {
                }
            });
            db.close();
            alert.setCancelable(false);
            alert.show();
            return false;
        }
        c = db.query(PairingEntry.TABLE_NAME, new String[] {
                PairingEntry.COLUMN_NAME_PEN_NAME, PairingEntry.COLUMN_NAME_PEN_TYPE, PairingEntry.COLUMN_NAME_PEN_ID
        }, PairingEntry.COLUMN_NAME_PEN_NAME + "=?", new String[] {
                from
        }, null, null, null);
        c.moveToFirst();
        String penId = c.getString(c.getColumnIndex(PairingEntry.COLUMN_NAME_PEN_ID));
        String penType = c.getString(c.getColumnIndex(PairingEntry.COLUMN_NAME_PEN_TYPE));

        db = getWritableDatabase();

        ContentValues values = new ContentValues();
        values.put(PairingEntry.COLUMN_NAME_PEN_ID, penId);
        values.put(PairingEntry.COLUMN_NAME_PEN_TYPE, penType);
        values.put(PairingEntry.COLUMN_NAME_PEN_NAME, to);

        db.delete(PairingEntry.TABLE_NAME, PairingEntry.COLUMN_NAME_PEN_NAME + "=?", new String[] {
                from
        });
        db.insert(PairingEntry.TABLE_NAME, null, values);
        db.close();
        return true;
    }

    public void removePen(String penName) {
        SQLiteDatabase db = getWritableDatabase();
        db.delete(PairingEntry.TABLE_NAME, PairingEntry.COLUMN_NAME_PEN_NAME + "=?", new String[] {
                penName
        });
        db.close();
    }

    public String[] getPenNames() {
        SQLiteDatabase db = getReadableDatabase();

        Cursor c = db.query(PairingEntry.TABLE_NAME, new String[] {
                PairingEntry.COLUMN_NAME_PEN_NAME
        }, null, null, null, null, null);
        ArrayList<String> names = new ArrayList<String>();
        c.moveToFirst();
        while (!c.isAfterLast()) {
            names.add(c.getString(0)); // only one column
            c.moveToNext();
        }
        db.close();
        return names.toArray(new String[names.size()]);
    }

    public int getPenId(String penName) {
        SQLiteDatabase db = getReadableDatabase();
        Cursor c = db.query(PairingEntry.TABLE_NAME, new String[] {
                PairingEntry.COLUMN_NAME_PEN_ID
        }, PairingEntry.COLUMN_NAME_PEN_NAME + "=?", new String[] {
                penName
        }, null, null, null);
        c.moveToFirst();
        int penId = Integer.valueOf(c.getInt(0));
        db.close();
        return penId;
    }

    public String getPenType(String penName) {
        SQLiteDatabase db = getReadableDatabase();
        Cursor c = db.query(PairingEntry.TABLE_NAME, new String[] {
                PairingEntry.COLUMN_NAME_PEN_TYPE
        }, PairingEntry.COLUMN_NAME_PEN_NAME + "=?", new String[] {
                penName
        }, null, null, null);
        c.moveToFirst();
        String penType = c.getString(0);
        db.close();
        return penType;
    }
}
