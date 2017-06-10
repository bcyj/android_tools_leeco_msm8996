/**
 * Copyright (c) 2013 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package oms.drmservice;

import android.app.Activity;
import android.os.Bundle;
import android.content.Intent;
import android.content.DialogInterface;
import android.content.Context;
import android.net.Uri;
import android.util.Log;
import android.database.Cursor;
import android.content.ContentValues;
import android.provider.Downloads;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.TextView;
import android.view.Window;
import android.os.Environment;
import android.widget.Toast;
import android.drm.DrmManagerClientWrapper;
import android.drm.DrmRights;
import android.content.ActivityNotFoundException;
import java.io.*;
import java.util.*;
import java.io.File;
import java.io.FileInputStream;
import java.text.SimpleDateFormat;
import java.text.ParseException;

public class DrmActivity extends Activity implements OnClickListener {
    private final static String TAG = "DrmActivity";
    private Intent intent;
    private Context context;
    private String action;

    private TextView tv;
    private Button okButton;
    private Button noButton;

    private String intentPath;
    private Uri intentUri;

    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState) {
        Log.d(TAG, "enter onCreate!");
        super.onCreate(savedInstanceState);
        intent = getIntent();
        requestWindowFeature(Window.FEATURE_NO_TITLE);
    }

    @Override
    public void onResume() {
        super.onResume();
        Log.d(TAG, "enter onResume!");
        action = intent.getAction();
        context = this;
        if (action.equals(DrmIntent.OPEN_FILES_LICENSE)) {
            receiveLicense();
        } else if (action.equals(DrmIntent.OPEN_FILES_CONTENT)) {
            receiveContent();
        } else if (action.equals(DrmIntent.SHOW_PROPERTIES)) {
            showProperties();
        } else if (action.equals(DrmIntent.BUY_LICENSE)) {
            buyLicense();
        } else if (action.equals(DrmIntent.FILE_NOT_EXIST)) {
            fileNotExist();
        }
    }

    private void fileNotExist() {
        try {
            setContentView(R.layout.file_not_exist);
            okButton = (Button) findViewById(R.id.okFileNotExist);
            okButton.setOnClickListener(this);
            tv = (TextView) findViewById(R.id.fileNotExist);
            tv.setText("File " + intent.getStringExtra(DrmIntent.EXTRA_DCF_FILEPATH)
                    + " doesn't exist :(");
        } catch (Exception e) {
            Log.e(TAG, e.toString());
        }

    }

    private void receiveLicense() {
        try {
            intentUri = intent.getData();
            Log.d(TAG, "uri is " + intentUri.toString());
            intentPath = intentUri.getPath();
            Log.d(TAG, "path is " + intentPath);
            // requestWindowFeature(Window.FEATURE_NO_TITLE);
            setContentView(R.layout.open_files);
            okButton = (Button) findViewById(R.id.openFileOk);
            okButton.setOnClickListener(this);
            noButton = (Button) findViewById(R.id.openFileNo);
            noButton.setOnClickListener(this);
            tv = (TextView) findViewById(R.id.openFile);

            String str = context.getString(R.string.tv_finishdownload);
            String strMsg = str.replace("%%%", intentPath);
            tv.setText(strMsg);

        } catch (Exception e) {
            Log.d(TAG, e.toString());
            finish();
        }
    }

    private void receiveContent() {
        try {
            intentUri = intent.getData();
            Log.d(TAG, "uri is " + intentUri.toString());
            intentPath = intentUri.getPath();
            Log.d(TAG, "path is " + intentPath);
            // requestWindowFeature(Window.FEATURE_NO_TITLE);
            setContentView(R.layout.open_files2);
            okButton = (Button) findViewById(R.id.openFileOk2);
            okButton.setOnClickListener(this);
            noButton = (Button) findViewById(R.id.openFileNo2);
            noButton.setOnClickListener(this);
            tv = (TextView) findViewById(R.id.openFile2);
            String str = context.getString(R.string.tv_finishdownload);
            String strMsg = str.replace("%%%", intentPath);
            tv.setText(strMsg);

        } catch (Exception e) {
            Log.d(TAG, e.toString());
            finish();
        }
    }

    private void showProperties() {
        Log.d(TAG, "Receiving DrmIntent.SHOW_PROPERTIES");

        String content = intent.getStringExtra(DrmIntent.EXTRA_FILE_PATH);
        boolean isPhotoPage = intent.getBooleanExtra("PhotoPage", false);
        Uri uri = null;
        String path = null;

        if (content.startsWith("content://")) {
            uri = Uri.parse(content);
            Cursor cursor = null;

            String[] projection = new String[] {
                    "_id", "_data"
            };
            cursor = context.getContentResolver().query(uri, projection, null, null, null);

            if ((cursor == null) || (cursor.getCount() == 0)) {
                Log.d(TAG, "can't get cursor." + "count " + cursor.getCount());
            } else {
                cursor.moveToFirst();
                path = cursor.getString(1);
                Log.d(TAG, "path " + path);
            }
        } else if (content.startsWith("file://")) {
            Log.d(TAG, content + " is a file Uri");
            uri = Uri.parse(content);
            path = uri.getPath();
        } else {
            Log.d(TAG, content + " is a real file path");
            path = content;
        }

        DrmManagerClientWrapper drmClient = new DrmManagerClientWrapper(this);
        ContentValues play_constraints = drmClient.getConstraints(path, 1);
        ContentValues display_constraints = drmClient.getConstraints(path, 7);
        boolean unlimited = false;
        boolean withInvalidRights = true;
        String message = new String();

        if (play_constraints.getAsInteger("valid") != 0) {
            withInvalidRights = false;
            if (play_constraints.getAsInteger("unlimited") != 0) {
                unlimited = true;
            } else {
                message += getString(R.string.play_permission) + "\n";
                message = formatMsg(message, play_constraints, isPhotoPage, false);
            }
        }
        if (display_constraints.getAsInteger("valid") != 0 && unlimited == false) {
            withInvalidRights = false;
            if (display_constraints.getAsInteger("unlimited") != 0) {
                unlimited = true;
            } else {
                message += getString(R.string.display_permission) + "\n";
                message = formatMsg(message, display_constraints, isPhotoPage, true);
            }
        }

        if (withInvalidRights) {
            message += getString(R.string.no_rights) + "\n";
        }
        if (unlimited) {
            Log.d(TAG, "This is a unlimited right.");
            unlimited = true;
        }

        setContentView(R.layout.show_properties);
        okButton = (Button) findViewById(R.id.okButton);
        okButton.setOnClickListener(this);
        tv = (TextView) findViewById(R.id.properties);
        if (unlimited == false) {
            tv.setText(message);
        } else {
            tv.setText(R.string.tv_fl);
        }
        Log.d(TAG, "end SHOW_PROPERTIES");
    }

    private void buyLicense() {
        Log.d(TAG, "Receiving DrmIntent.BUY_LICENSE");
        intentPath = intent.getStringExtra(DrmIntent.EXTRA_FILE_PATH);
        Log.d(TAG, "intentPath is: " + intentPath);

        if (intentPath == null || intentPath.equals("")) {
            setContentView(R.layout.buy_license);
            okButton = (Button) findViewById(R.id.nouri);
            okButton.setOnClickListener(this);
            tv = (TextView) findViewById(R.id.buyLicense);
            tv.setText(R.string.tv_cantbuy);
        } else {
            // requestWindowFeature(Window.FEATURE_NO_TITLE);
            setContentView(R.layout.buy_license2);
            okButton = (Button) findViewById(R.id.buyOk);
            okButton.setOnClickListener(this);
            noButton = (Button) findViewById(R.id.buyNo);
            noButton.setOnClickListener(this);
            tv = (TextView) findViewById(R.id.buyLicense2);
            tv.setText(R.string.tv_wantbuy);
        }
    }

    private String formatMsg(String message, ContentValues constraints, boolean isPhotoPage,
            boolean isDisplayRights) {
        int count = 0;
        long interval = 0;
        Date startDate = null;
        Date endDate = null;

        count = constraints.getAsInteger("count");
        long startDateL = constraints.getAsLong("startDateTime");
        long endDateL = constraints.getAsLong("endDateTime");
        interval = constraints.getAsLong("interval");

        if (startDateL > 0) {
            startDate = new Date(startDateL);
        }
        if (endDateL > 0) {
            endDate = new Date(endDateL);
        }

        Log.d(TAG, "y:count=" + count);
        Log.d(TAG, "y:startDate=" + Long.toString(startDateL));
        Log.d(TAG, "y:endDate= " + Long.toString(endDateL));
        Log.d(TAG, "y:intervalTime= " + Long.toString(interval));

        if (count > 0) {
            if (isPhotoPage == true && isDisplayRights == true) {
                count = count - 1;
            }
            if (count < 0)
                count = 0;
            message += getString(R.string.count) + ": " + String.valueOf(count) + "\n";
        }

        if (interval > 0) {
            message += getString(R.string.interval_str) + ": " + String.valueOf(interval / 1000)
                    + " " + getString(R.string.seconds) + "\n";
        }

        if (startDate != null) {
            Log.d(TAG, "startDate>0" + startDate);
            message += getString(R.string.start_date) + ": " + startDate.toString() + "\n";
        }

        if (endDate != null) {
            Log.d(TAG, "endDate>0" + endDate);
            message += getString(R.string.end_date) + ": " + endDate.toString() + "\n\n";
        }

        return message;
    }

    public void onClick(View view) {
        switch (view.getId()) {
            case R.id.okButton:
                Log.d(TAG, "ok here");
                finish();
                break;
            case R.id.openFileOk:
                Log.d(TAG, "yes!");
                /*
                 * try { Log.d(TAG, "stroage state " +
                 * Environment.getExternalStorageState());
                 * if(Environment.MEDIA_SHARED
                 * .equals(Environment.getExternalStorageState())){
                 * Log.d(TAG,"sd card busy in openfile"); Toast.makeText(this,
                 * R.string.sdcard_busy, Toast.LENGTH_SHORT).show(); } else {
                 * Intent downloadIntent = new Intent();
                 * downloadIntent.setAction(DrmIntent.CLEAR_NOTIFICATION);
                 * downloadIntent.putExtra(DrmIntent.EXTRA_FILE_PATH,
                 * intentPath); context.sendBroadcast(downloadIntent); File
                 * handle = new File(intentPath); FileInputStream fis = new
                 * FileInputStream(handle); DrmServiceRawContent rawContent =
                 * new DrmServiceRawContent(fis, (int) handle.length(),
                 * DrmServiceRawContent.DRM_MIMETYPE_CONTENT_STRING); String
                 * mimeType = rawContent.getContentType(); Intent commIntent =
                 * new Intent();
                 * commIntent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                 * commIntent.setAction(Intent.ACTION_VIEW);
                 * commIntent.setDataAndType(intentUri, mimeType);
                 * context.startActivity(commIntent); Log.d(TAG,
                 * "here we finish openfile ok"); } finish(); } catch (Exception
                 * e){ Log.d(TAG, e.toString()); Log.d(TAG, "file " + intentPath
                 * + " not found"); Intent fileIntent = new Intent();
                 * fileIntent.setAction(DrmIntent.FILE_NOT_EXIST);
                 * fileIntent.putExtra(DrmIntent.EXTRA_DCF_FILEPATH,
                 * intentPath);
                 * fileIntent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                 * context.startActivity(fileIntent); Log.e(TAG,
                 * "get rights fail." + e ); finish(); }
                 */
                break;
            case R.id.openFileNo:
                Log.e(TAG, "no!");
                Log.d(TAG, "here we finish open file no");
                finish();
                break;
            case R.id.openFileOk2:
                Log.d(TAG, "yes!");
                try {
                    Log.d(TAG, "stroage state " + Environment.getExternalStorageState());
                    if (Environment.MEDIA_SHARED.equals(Environment.getExternalStorageState())) {
                        Log.d(TAG, "sd card busy in openfile");
                        Toast.makeText(this, R.string.sdcard_busy, Toast.LENGTH_SHORT).show();
                    } else {
                        Intent downloadIntent = new Intent();
                        downloadIntent.setAction(DrmIntent.CLEAR_NOTIFICATION);
                        downloadIntent.putExtra(DrmIntent.EXTRA_FILE_PATH, intentPath);
                        context.sendBroadcast(downloadIntent);

                        String contentType = intent.getStringExtra(DrmIntent.EXTRA_DRM_TYPE);
                        Intent commIntent = new Intent();
                        commIntent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                        commIntent.setAction(Intent.ACTION_VIEW);
                        commIntent.setDataAndType(intentUri, contentType);
                        context.startActivity(commIntent);
                        Log.d(TAG, "here we finish openfile ok2");
                    }
                    finish();
                    break;
                } catch (Exception e) {
                    Log.e(TAG, "get rights fail." + e);
                }
            case R.id.openFileNo2:
                Log.e(TAG, "no!");
                Log.d(TAG, "here we finish open file no2");
                finish();
                break;
            case R.id.nouri:
                Log.e(TAG, "ok!");
                finish();
                break;
            case R.id.buyOk:
                Log.d(TAG, "path is at buyok " + intentPath);
                Intent i = new Intent(Intent.ACTION_VIEW);
                i.setData(Uri.parse(intentPath.trim()));
                try {
                    startActivity(i);
                } catch (ActivityNotFoundException e) {
                    Log.e(TAG, "ActivityNotFoundException=" + e.toString());
                    String str = context.getString(R.string.no_activitiy_found);
                    Toast.makeText(this, str + intentPath, Toast.LENGTH_LONG).show();
                }
                finish();
                break;
            case R.id.buyNo:
                Log.d(TAG, "no!");
                finish();
                break;
            case R.id.okFileNotExist:
                Log.d(TAG, "file doesn't exist");
                finish();
                break;
        }
    }

}
