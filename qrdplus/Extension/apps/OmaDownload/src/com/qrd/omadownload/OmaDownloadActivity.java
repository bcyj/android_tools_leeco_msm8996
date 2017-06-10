/******************************************************************************
  Copyright (c) 2012-2014, Qualcomm Technologies, Inc.
  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
 *******************************************************************************/
package com.qrd.omadownload;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.Context;
import android.content.ContentResolver;
import android.content.ContentValues;
import android.content.DialogInterface;
import android.content.DialogInterface.OnClickListener;
import android.content.DialogInterface.OnCancelListener;
import android.content.Intent;
import android.database.Cursor;
import android.net.Uri;
import android.os.Environment;
import android.os.Bundle;
import android.os.SystemProperties;
import android.provider.Downloads;
import android.util.Log;
import android.widget.Toast;
import com.qrd.omadownload.OmaDownloadMonitorService;
import com.qrd.omadownload.R;

public class OmaDownloadActivity extends Activity implements OnClickListener , OnCancelListener{
    private static final String LOG_TAG = "OMADL_ACTIVITY";
    public static final String ACTION_OMADL_CONFIRM = "android.intent.omadownload.ddconfirm";
    public static final String ACTION_OMADL_REDIRECT = "android.intent.omadownload.urldirect";
    public static final String EXTRA_OBJECTURL = "objecturl";
    public static final String EXTRA_OBJECTSIZE = "objectsize";
    public static final String EXTRA_OBJECTTYPE = "objecttype";
    public static final String EXTRA_OBJECTALLMIMETYPE = "objectallmimetype";
    public static final String EXTRA_OBJECTNAME = "objectname";
    public static final String EXTRA_OBJECTVENDOR = "objectvendor";
    public static final String EXTRA_OBJECTDESCRIPTION = "objectdescription";
    public static final String EXTRA_OBJECTINSTALLNOTIFYURL = "objectinstallnotifyurl";
    public static final String EXTRA_OBJECTNEXTURL = "objectnexturl";
    public static final String EXTRA_MONITOR_MM_URI = "mon_mm_uri";
    public static final String EXTRA_MONITOR_DD_URI = "mon_dd_uri";
    public static final String EXTRA_MONITOR_DD_SRC_URL = "mon_dd_url";
    private String mObjecturl = null;
    private String mObjectname = null;
    private String mObjectsize = null;
    private String mObjecttype = null;
    private String mObjectvendor = null;
    private String mObjectInstallNotifyUrl = null;

    private String mObjectdescription = null;
    private String mObjectnexturl = null;
    private DialogInterface mResultDialog = null;
    private DialogInterface mConfirmDialog = null;

    public static final String MIMETYPE_DRM_MESSAGE = "application/vnd.oma.drm.message";
    public static final String MIMETYPE_DRM_CONTENT = "application/vnd.oma.drm.content";
    private int statusReport;
    public  static boolean sIsOmaDlErrorHandlingEnabled;

    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        sIsOmaDlErrorHandlingEnabled = SystemProperties.getBoolean(
                "persist.omadlhandleer.enable", false)
                || getResources().getBoolean(R.bool.enableOMADLErrorHandling);
    }

    @Override
    protected void onResume() {
        super.onResume();
        String action = getIntent().getAction();
        String scheme = getIntent().getScheme() != null ? getIntent().getScheme():"";
        if (action.equals(OmaDownloadActivity.ACTION_OMADL_CONFIRM))
            handleDDConfirm((Context)this, getIntent());
        else if (action.equals(OmaDownloadActivity.ACTION_OMADL_REDIRECT))
            handleDDRedirect((Context)this, getIntent());
        else if (action.equals(Intent.ACTION_VIEW) && (scheme.equals("http")
                || scheme.equals("https")))
            handleDDDownload(this, getIntent().getData());
        else
            finish();
    }

    public void onClick(DialogInterface dialog, int which) {
        if (which == DialogInterface.BUTTON_POSITIVE) {
            if (dialog != mResultDialog) {
                handleMMDownload(this);
            } else {
                handleMMNextUrl(this);
            }
        } else if (sIsOmaDlErrorHandlingEnabled) {
            if (which == DialogInterface.BUTTON_NEGATIVE) {
                statusReport = OmaDownloadMonitorService.STATUS_USER_CANCELLED;
                if (mObjectInstallNotifyUrl != null) {
                    new Thread(new Runnable() {
                        @Override
                        public void run() {
                            OmaDownloadMonitorService.postInstallNotification(
                                    OmaDownloadActivity.this,
                                    mObjectInstallNotifyUrl, statusReport);
                        }
                    }).start();
                }
            }
        }
        finish();
    }

    public void onCancel(DialogInterface dialog) {
        finish();
    }

    private void handleDDRedirect(Context ctx, Intent intent) {
        mObjectnexturl = (String)intent.getExtra(EXTRA_OBJECTNEXTURL);
        if (mResultDialog != null)
            mResultDialog.dismiss();
        if (mConfirmDialog != null)
            mConfirmDialog.dismiss();
        mResultDialog = new AlertDialog.Builder(ctx)
        .setTitle(R.string.download_complete)
        .setMessage(R.string.download_sucessful)
        .setPositiveButton(R.string.next_page, this)
        .setNegativeButton(R.string.cancel, this)
        .setOnCancelListener(this)
        .show();
    }

    private void handleDDConfirm(Context ctx, Intent intent) {
        mObjecturl = (String)intent.getExtra(EXTRA_OBJECTURL);
        mObjectname = (String)intent.getExtra(EXTRA_OBJECTNAME);
        mObjectsize = (String)intent.getExtra(EXTRA_OBJECTSIZE);
        mObjecttype = (String)intent.getExtra(EXTRA_OBJECTTYPE);
        String mAllMimetype = (String)intent.getExtra(EXTRA_OBJECTALLMIMETYPE);
        mObjectvendor = (String)intent.getExtra(EXTRA_OBJECTVENDOR);
        mObjectdescription = (String)intent.getExtra(EXTRA_OBJECTDESCRIPTION);
        if (sIsOmaDlErrorHandlingEnabled)
        mObjectInstallNotifyUrl = (String)intent.getExtra(EXTRA_OBJECTINSTALLNOTIFYURL);
        String ddContent = (mObjectname != null ? (getText(R.string.filename) + mObjectname + "\n") : "")
              + (mAllMimetype != null ? (getText(R.string.filetype) + mAllMimetype + "\n") : "")
              + (mObjectsize != null ? (getText(R.string.filesize) + mObjectsize + "\n") : "")
              + (mObjectvendor != null ? (getText(R.string.filevendor) + mObjectvendor + "\n") : "")
              + (mObjectdescription != null ? (getText(R.string.description) + mObjectdescription + "\n") : "");
        if (mResultDialog != null)
            mResultDialog.dismiss();
        if (mConfirmDialog != null)
            mConfirmDialog.dismiss();
        mConfirmDialog = new AlertDialog.Builder(ctx)
        .setTitle(R.string.download_info)
        .setMessage(ddContent)
        .setPositiveButton(R.string.download, this)
        .setNegativeButton(R.string.cancel, this)
        .setOnCancelListener(this)
        .show();
    }

    private Uri handleDownload(Context ctx, Uri data, String filename, String mimetype) {
        String status = Environment.getExternalStorageState();
        String url = data.toString();
        if (!status.equals(Environment.MEDIA_MOUNTED)) {
            Toast.makeText(this, R.string.cannot_download, Toast.LENGTH_SHORT)
            .show();
            return null;
        }
        ContentValues values = new ContentValues();
        values.put(Downloads.Impl.COLUMN_URI, url);
        values.put(Downloads.Impl.COLUMN_VISIBILITY,
            Downloads.Impl.VISIBILITY_VISIBLE_NOTIFY_COMPLETED);
        values.put(Downloads.Impl.COLUMN_DESTINATION,
            Downloads.Impl.DESTINATION_EXTERNAL);
        if (mimetype != null)
            values.put(Downloads.Impl.COLUMN_MIME_TYPE, mimetype);
        if (filename != null)
            values.put(Downloads.Impl.COLUMN_FILE_NAME_HINT, filename);
        values.put(Downloads.Impl.COLUMN_DESCRIPTION, data.getHost());

        Log.e(LOG_TAG, "inser value: url is:" + url + "host:" + data.getHost() + "ctx:"
            + ctx + " resolver:" + ctx.getContentResolver());

        return ctx.getContentResolver().insert(Downloads.Impl.CONTENT_URI, values);
    }

    private void handleDDDownload(Context ctx, Uri data) {
        String filename = null;
        String url = data.toString();
        if (!url.endsWith("/")) {
            int index = url.lastIndexOf('/') + 1;
            if (index > 0) {
                filename = url.substring(index);
            }
        }
        if (filename == null)
            filename = "temp_download";
        if(!filename.endsWith(".dd"))
            filename = filename + ".dd";
        Uri ddUri = handleDownload(ctx, data, filename, "application/vnd.oma.dd+xml");
        if (ddUri != null)
        {
        startService(new Intent(this, OmaDownloadMonitorService.class).putExtra(
                EXTRA_MONITOR_DD_URI, ddUri.toString()).putExtra(EXTRA_MONITOR_DD_SRC_URL,
                data.toString()));
        }
        finish();
    }

    private void handleMMDownload(Context ctx) {
        String filename = null;
        if (mObjectname != null) {
            filename = mObjectname.replaceAll("[/\\\\, ]", "_");
        }

        if (filename == null) {
            if (!mObjecturl.endsWith("/")) {
                int index = mObjecturl.lastIndexOf('/') + 1;
                if (index > 0) {
                    filename = mObjecturl.substring(index);
                }
            }
        }
        if (filename == null)
            filename = "temp_download";
        if(filename.indexOf(".") < 0) {
            String fileExt = "dat";
            if (mObjecttype != null) {
                if (mObjecttype.equals(MIMETYPE_DRM_MESSAGE)) {
                    fileExt = "dm";
                } else if (mObjecttype.equals(MIMETYPE_DRM_CONTENT)) {
                    fileExt = "dcf";
                } else {
                    int index = mObjecttype.lastIndexOf('/') + 1;
                    if (index > 0)
                       fileExt = mObjecttype.substring(index);
                }
            }
            filename = filename + "." + fileExt;
        }
        Uri mmUri = handleDownload(ctx, Uri.parse(mObjecturl), filename,
            (mObjecttype != null ? mObjecttype : null));
        if (mmUri != null)
            startService(new Intent(this, OmaDownloadMonitorService.class).putExtra(
                EXTRA_MONITOR_MM_URI, mmUri.toString()));
        finish();
    }

    private void handleMMNextUrl(Context ctx) {
        if (mObjectnexturl != null)
            ctx.startActivity(new Intent(Intent.ACTION_VIEW, Uri.parse(mObjectnexturl)));
        finish();
    }
}
