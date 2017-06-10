/*
 * Copyright (c) 2012-2014, Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
package com.qrd.omadownload;

import android.app.Service;
import android.app.DownloadManager;
import android.content.Context;
import android.content.ContentResolver;
import android.content.ContentValues;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.content.pm.ResolveInfo;
import android.database.Cursor;
import android.database.ContentObserver;
import android.net.Uri;
import android.os.Handler;
import android.os.IBinder;
import android.os.SystemProperties;
import android.provider.Downloads;
import android.text.TextUtils;
import android.util.Log;
import android.util.Xml;
import android.webkit.MimeTypeMap;
import android.widget.Toast;

import java.io.File;
import java.io.FileInputStream;
import java.io.OutputStream;
import java.net.HttpURLConnection;
import java.net.URL;
import java.net.URI;
import java.util.ArrayList;
import java.util.List;

import org.xmlpull.v1.XmlPullParser;

public class OmaDownloadMonitorService extends Service {
    private final static String TAG = "OmaDownloadMonitorService";
    public static int mStartId;
    private static Context mContext = null;
    private DownloadManager mDownloadManager = null;
    private Cursor mMonitorCursor = null;
    ContentMonitorObserver mDownloadObserver = new ContentMonitorObserver(
            Downloads.Impl.CONTENT_URI);
    private ArrayList<DDMonitorInfo> ddUriMonitorList = new ArrayList<DDMonitorInfo>();
    private ArrayList<MMMonitorInfo> mmMonitorList = new ArrayList<MMMonitorInfo>();
    private String mLastObjectInstallNotifyUrl;
    private String mLastObjectNextUrl;
    private ArrayList<String> mLastObjectMimeType = new ArrayList<String>();
    private int statusReport = 0;
    public static String mmNotifyUrl = null;
    final String[] ddTagMapping = new String[] { "objecturi", "size", "type",
            "name", "vendor", "description", "installnotifyuri", "nexturl", "ddversion" };

    enum DD_TAG {
        OBJECTURI, SIZE, TYPE, NAME, VENDOR, DESCRIPTION, INSTALLNOTIFYURI, NEXTURL, DDVERSION ,INVALID
    };

    String mMimeType;
    String type;
    String mSize;
	static String mmFilename ;
	public static final int STATUS_SUCCESS = 900;
	public static final int STATUS_INSUFFICIENT_MEMORY = 901;
	public static final int STATUS_USER_CANCELLED = 902;
	public static final int STATUS_LOSS_OF_SERVICE = 903;
	public static final int STATUS_ATTRIBUTE_MISMATCH = 905;
	public static final int STATUS_INVALID_DESCRIPTOR = 906;
	public static final int STATUS_INVALID_DD_VERSION = 951;
	public static final int STATUS_DEVICE_ABORTED = 952;
	public static final int STATUS_NON_ACCEPTABLE_CONTENT = 953;
	public static final int STATUS_LOAD_ERROR = 954;
	public static final int STATUS_WAITING_TO_RETRY = 194;
	public static final int STATUS_INSUFFICIENT_SPACE_ERROR = 198;
	public static final int STATUS_DOWNLOAD_SUCCESS = 200;
	public static final int STATUS_DOWNLOAD_RUNNING = 192;

    private class MMMonitorInfo {
        public Uri mmUri;
        public String mmInstallUrl;
        public String mmNextUrl;
        public ArrayList<String> mmMimeType;

        public MMMonitorInfo() {
            mmUri = null;
            mmInstallUrl = null;
            mmNextUrl = null;
            mmMimeType = null;
        }

        public MMMonitorInfo(Uri uri, String installurl, String nexturl, ArrayList<String> mimeType) {
            mmUri = uri;
            mmInstallUrl = installurl;
            mmNextUrl = nexturl;
            mmMimeType = mimeType;
        }
    }

    private class DDMonitorInfo {
        public Uri ddDownloadUri;
        public String ddSrcUrl;
        public String mObjectURL = null;
        public String mObjectSize = null;
        public String mObjectType = null;
        public String mObjectName = null;
        public String mObjectVendor = null;
        public String mObjectDescription = null;
        public String mObjectInstallNotifyUrl = null;
        public String mObjectNextUrl = null;
        public String mObjectVersion = null;

        public DDMonitorInfo() {
            ddDownloadUri = null;
            ddSrcUrl = null;
        }

        public DDMonitorInfo(Uri uri, String url) {
            ddDownloadUri = uri;
            ddSrcUrl = url;
        }
    }

    private class ContentMonitorObserver extends ContentObserver {
        public ContentMonitorObserver(Uri uri) {
            super(new Handler());
        }

        @Override
        public void onChange(boolean selfChange) {
            Log.e(TAG, "Monitor_Onchange Trigger");

            for (int i = ddUriMonitorList.size() - 1; i >= 0; i--) {
                Uri ddObjectUri = ((DDMonitorInfo) ddUriMonitorList.get(i)).ddDownloadUri;
                Cursor downloadListCursor = mContext.getContentResolver()
                        .query(ddObjectUri,
                                new String[] { Downloads.Impl._DATA,
                                        Downloads.Impl.COLUMN_STATUS, }, null,
                                null, null);
                if (downloadListCursor != null
                        && downloadListCursor.moveToFirst()) {
                    final int filenameIndex = downloadListCursor
                            .getColumnIndexOrThrow(Downloads.Impl._DATA);
                    final int statusIndex = downloadListCursor
                            .getColumnIndexOrThrow(Downloads.Impl.COLUMN_STATUS);
                    String filename = downloadListCursor
                            .getString(filenameIndex);
                    int status = downloadListCursor.getInt(statusIndex);

                    if (status == STATUS_INSUFFICIENT_SPACE_ERROR){
                        statusReport = STATUS_INSUFFICIENT_MEMORY;
                    if (mmNotifyUrl != null) {
                        Log.e(TAG, "Insuff meme......mmNotifyUrl..." + mmNotifyUrl);
                        new Thread(new Runnable() {
                            @Override
                            public void run() {
                                postInstallNotification(mContext,
                                        mmNotifyUrl, statusReport);
                            }
                        }).start();
                    }
                    }

                    if (Downloads.Impl.isStatusCompleted(status)
                            && Downloads.Impl.isStatusSuccess(status)) {
                        Log.e(TAG, "dd name is " + filename + " dd status is "
                                + status);
                        if (parseDDFile(mContext, filename, i))
                            notifyConfirmWin(mContext, i);
                        deleteDDFile(mContext, filename);
                        ddUriMonitorList.remove(i);
                    }
                }
            }

            for (int i = mmMonitorList.size() - 1; i >= 0; i--) {
                if (((MMMonitorInfo) mmMonitorList.get(i)).mmUri != null) {
                    Uri mmObjectUri = ((MMMonitorInfo) mmMonitorList.get(i)).mmUri;
                    String mmNextUrl = ((MMMonitorInfo) mmMonitorList.get(i)).mmNextUrl;
                    mmNotifyUrl = ((MMMonitorInfo) mmMonitorList.get(i)).mmInstallUrl;
                    ArrayList<String> mmTypes = ((MMMonitorInfo) mmMonitorList.get(i)).mmMimeType;

                    if (!(OmaDownloadActivity.sIsOmaDlErrorHandlingEnabled)) {

                        Cursor mmObjectMonitorCur = mContext
                                .getContentResolver()
                                .query(mmObjectUri,
                                        new String[] { Downloads.Impl.COLUMN_STATUS, },
                                        null, null, null);
                        if (mmObjectMonitorCur != null
                                && mmObjectMonitorCur.moveToFirst()) {
                            final int index = mmObjectMonitorCur
                                    .getColumnIndexOrThrow(Downloads.Impl.COLUMN_STATUS);
                            int status = mmObjectMonitorCur.getInt(index);
                            if (Downloads.Impl.isStatusCompleted(status)
                                    && Downloads.Impl.isStatusSuccess(status)) {
                                Log.e(TAG, "MM Download complete");
                                if (mmNotifyUrl != null) {
                                    new Thread(new Runnable() {
                                        @Override
                                        public void run() {
                                            postInstallNotification(mContext, mmNotifyUrl);
                                        }
                                    }).start();
                                }
                                if (mmNextUrl != null)
                                    notifyResultWin(mContext, mmNextUrl);
                                ((MMMonitorInfo) mmMonitorList.get(i)).mmUri = null;
                                mmMonitorList.remove(i);
                            }

                        }
                    } else {

                        Cursor mmObjectMonitorCur = mContext
                                .getContentResolver()
                                .query(mmObjectUri,
                                        new String[] {
                                                Downloads.Impl.COLUMN_STATUS,
                                                Downloads.Impl.COLUMN_MIME_TYPE,
                                                Downloads.Impl._DATA }, null,
                                        null, null);

                        if (mmObjectMonitorCur != null
                                && mmObjectMonitorCur.moveToFirst()) {
                            final int mmFilenameIndex = mmObjectMonitorCur
                                    .getColumnIndexOrThrow(Downloads.Impl._DATA);
                            mmFilename = mmObjectMonitorCur
                                    .getString(mmFilenameIndex);

                            final int mmStatusIndex = mmObjectMonitorCur
                                    .getColumnIndexOrThrow(Downloads.Impl.COLUMN_STATUS);

                            mMimeType = mmObjectMonitorCur
                                    .getString(mmObjectMonitorCur
                                            .getColumnIndexOrThrow(Downloads.Impl.COLUMN_MIME_TYPE));

                            int status = mmObjectMonitorCur
                                    .getInt(mmStatusIndex);

                            boolean isNonAcceptableType = false;
                            for (String mime : mmTypes) {

                                if ((MimeTypeMap.getSingleton()
                                        .hasMimeType(mime))) {
                                    break;
                                }else{
                                    isNonAcceptableType = true;
                                }
                            }

                            if(isNonAcceptableType) {
                                statusReport = STATUS_NON_ACCEPTABLE_CONTENT;
                                Toast.makeText(getApplicationContext(),
                                        R.string.nonacceptable,
                                        Toast.LENGTH_SHORT).show();
                                mContext.getContentResolver().delete(
                                        mmObjectUri, null, null);

                            }else if (!mmTypes.contains(mMimeType)) {
                                statusReport = STATUS_ATTRIBUTE_MISMATCH;
                                mContext.getContentResolver().delete(
                                        mmObjectUri, null, null);
                                Toast.makeText(getApplicationContext(),
                                        R.string.mimemismatch,
                                        Toast.LENGTH_SHORT).show();
                            }else {
                                switch (status) {
                                case STATUS_WAITING_TO_RETRY:
                                    statusReport = STATUS_LOSS_OF_SERVICE;
                                    break;
                                case STATUS_INSUFFICIENT_SPACE_ERROR:
                                    statusReport = STATUS_INSUFFICIENT_MEMORY;
                                    break;
                                case STATUS_DOWNLOAD_SUCCESS:
                                    statusReport = STATUS_SUCCESS;
                                    break;
                                case STATUS_DOWNLOAD_RUNNING:
                                    statusReport = STATUS_SUCCESS;
                                    break;
                                default:
                                    break;
                                }
                            }

                            if (mmNotifyUrl != null) {
                                Log.i(TAG, "mmNotifyUrl..." + mmNotifyUrl);
                                new Thread(new Runnable() {
                                    @Override
                                    public void run() {
                                        postInstallNotification(mContext,
                                                mmNotifyUrl, statusReport);
                                    }
                                }).start();
                            }
                            if (mmNextUrl != null && statusReport == 0 || statusReport == 900)
                                notifyResultWin(mContext, mmNextUrl);
                            ((MMMonitorInfo) mmMonitorList.get(i)).mmUri = null;
                            mmMonitorList.remove(i);
                        }

                    }
                }
                }
            }
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        super.onStartCommand(intent, flags, startId);
        mStartId = startId;
        if (intent != null && intent.getExtra(OmaDownloadActivity.EXTRA_MONITOR_DD_URI) != null)
            ddUriMonitorList
                    .add(new DDMonitorInfo(
                            Uri.parse((String) intent
                                    .getExtra(OmaDownloadActivity.EXTRA_MONITOR_DD_URI)),
                            (String) intent
                                    .getExtra(OmaDownloadActivity.EXTRA_MONITOR_DD_SRC_URL)));

        if (intent != null && intent.getExtra(OmaDownloadActivity.EXTRA_MONITOR_MM_URI) != null)
            mmMonitorList.add(new MMMonitorInfo(Uri.parse((String) intent
                    .getExtra(OmaDownloadActivity.EXTRA_MONITOR_MM_URI)),
                    mLastObjectInstallNotifyUrl, mLastObjectNextUrl, mLastObjectMimeType));
        return START_STICKY;
    }

    @Override
    public void onCreate() {
        super.onCreate();
        mContext = this;
        mDownloadManager = (DownloadManager) getSystemService(Context.DOWNLOAD_SERVICE);
        mDownloadManager.setAccessAllDownloads(true);
        DownloadManager.Query baseQuery = new DownloadManager.Query()
                .setOnlyIncludeVisibleInDownloadsUi(true);
        mMonitorCursor = mDownloadManager.query(baseQuery);
        mMonitorCursor.registerContentObserver(mDownloadObserver);
    }

    @Override
    public void onDestroy() {
        mMonitorCursor.unregisterContentObserver(mDownloadObserver);
        super.onDestroy();
    }

    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }

    private boolean parseDDFile(Context ctx, String filename, int index) {
        FileInputStream fis = null;

        mLastObjectInstallNotifyUrl = null;
        mLastObjectNextUrl = null;
        mLastObjectMimeType = new ArrayList<String>();

        try {
            fis = new FileInputStream(filename);
            XmlPullParser parser = Xml.newPullParser();
            parser.setInput(fis, "UTF-8");
            int eventType = parser.getEventType();
            while (eventType != XmlPullParser.END_DOCUMENT) {
                switch (eventType) {
                case XmlPullParser.START_TAG:
                    int tagId = mapDDTag(parser.getName());
                    DD_TAG tag = DD_TAG.INVALID;
                    if (tagId != -1)
                        tag = DD_TAG.values()[tagId];

                    switch (tag) {
                    case OBJECTURI:
                        String tempObjectUrl = parser.nextText();
                        if(((DDMonitorInfo) ddUriMonitorList.get(index)).mObjectURL == null){
                        if ((Uri.parse(tempObjectUrl)).isRelative()) {
                            URI uriBase = new URI(
                                    ((DDMonitorInfo) ddUriMonitorList
                                            .get(index)).ddSrcUrl);
                            URI uriRelative = new URI(tempObjectUrl);
                            ((DDMonitorInfo) ddUriMonitorList.get(index)).mObjectURL = (uriBase
                                    .resolve(uriRelative)).toString();
                        } else
                            ((DDMonitorInfo) ddUriMonitorList.get(index)).mObjectURL
                                = tempObjectUrl;
                        }

                        break;
                    case SIZE:
                        mSize = parser.nextText();
                        if(((DDMonitorInfo) ddUriMonitorList.get(index)).mObjectSize == null){
                        ((DDMonitorInfo) ddUriMonitorList.get(index)).mObjectSize = mSize;
                        }
                        break;
                    case TYPE:
                        type = parser.nextText();
                        mLastObjectMimeType.add(type);

                        Log.e(TAG, "TYPE...." + type);
                            ((DDMonitorInfo) ddUriMonitorList.get(index)).mObjectType = OmaDownloadActivity.MIMETYPE_DRM_MESSAGE;

                        break;
                    case NAME:
                        if(((DDMonitorInfo) ddUriMonitorList.get(index)).mObjectName == null){
                        ((DDMonitorInfo) ddUriMonitorList.get(index)).mObjectName = parser
                                .nextText();
                        }
                        break;
                    case VENDOR:
                        if(((DDMonitorInfo) ddUriMonitorList.get(index)).mObjectVendor == null){
                        ((DDMonitorInfo) ddUriMonitorList.get(index)).mObjectVendor = parser
                                .nextText();
                        }
                        break;
                    case DESCRIPTION:
                        if(((DDMonitorInfo) ddUriMonitorList.get(index)).mObjectDescription == null){
                        ((DDMonitorInfo) ddUriMonitorList.get(index)).mObjectDescription = parser
                                .nextText();
                        }
                        break;
                    case INSTALLNOTIFYURI:
                        if(((DDMonitorInfo) ddUriMonitorList.get(index)).mObjectInstallNotifyUrl == null){
                        ((DDMonitorInfo) ddUriMonitorList.get(index)).mObjectInstallNotifyUrl =
                            parser.nextText();
                        mLastObjectInstallNotifyUrl = ((DDMonitorInfo) ddUriMonitorList
                                .get(index)).mObjectInstallNotifyUrl;
                        }
                        break;
                    case NEXTURL:
                        if(((DDMonitorInfo) ddUriMonitorList.get(index)).mObjectNextUrl == null){
                        ((DDMonitorInfo) ddUriMonitorList.get(index)).mObjectNextUrl = parser
                                .nextText();
                        mLastObjectNextUrl = ((DDMonitorInfo) ddUriMonitorList
                                .get(index)).mObjectNextUrl;
                        }
                        break;
                    case DDVERSION:
                        if(((DDMonitorInfo) ddUriMonitorList.get(index)).mObjectVersion == null){
                        ((DDMonitorInfo) ddUriMonitorList.get(index)).mObjectVersion = parser
                                .nextText();
                        }
                        break;
                    default:
                        break;
                    }
                    break;

                default:
                    break;
                }

                eventType = parser.next();
            }
        } catch (Exception e) {
            Log.e(TAG, "parse DD Failure" + e);
            if(OmaDownloadActivity.sIsOmaDlErrorHandlingEnabled){
            if (mLastObjectInstallNotifyUrl != null) {
                new Thread(new Runnable() {
                    @Override
                    public void run() {
                        statusReport = STATUS_INVALID_DESCRIPTOR;
                        postInstallNotification(mContext, mLastObjectInstallNotifyUrl,statusReport);
                    }
                }).start();
            }
            Toast.makeText(getApplicationContext(),
                    R.string.invalid_dd,
                    Toast.LENGTH_SHORT).show();
            }

            return false;
        }

        // print the tag value for test:
        if (((DDMonitorInfo) ddUriMonitorList.get(index)).mObjectURL != null)
            Log.e(TAG,
                    "mObjectURL is "
                            + ((DDMonitorInfo) ddUriMonitorList.get(index)).mObjectURL);
        if (((DDMonitorInfo) ddUriMonitorList.get(index)).mObjectSize != null)
            Log.e(TAG,
                    "mObjectSize is "
                            + ((DDMonitorInfo) ddUriMonitorList.get(index)).mObjectSize);
        if (((DDMonitorInfo) ddUriMonitorList.get(index)).mObjectType != null)
            Log.e(TAG,
                    "mObjectType is "
                            + ((DDMonitorInfo) ddUriMonitorList.get(index)).mObjectType);
        if (((DDMonitorInfo) ddUriMonitorList.get(index)).mObjectName != null)
            Log.e(TAG,
                    "mObjectName is "
                            + ((DDMonitorInfo) ddUriMonitorList.get(index)).mObjectName);
        if (((DDMonitorInfo) ddUriMonitorList.get(index)).mObjectVendor != null)
            Log.e(TAG,
                    "mObjectVendor is "
                            + ((DDMonitorInfo) ddUriMonitorList.get(index)).mObjectVendor);
        if (((DDMonitorInfo) ddUriMonitorList.get(index)).mObjectDescription != null)
            Log.e(TAG,
                    "mObjectDescription is "
                            + ((DDMonitorInfo) ddUriMonitorList.get(index)).mObjectDescription);
        if (((DDMonitorInfo) ddUriMonitorList.get(index)).mObjectInstallNotifyUrl != null)
            Log.e(TAG,
                    "mObjectInstallNotifyUrl is "
                    + ((DDMonitorInfo) ddUriMonitorList.get(index)).mObjectInstallNotifyUrl);
        if (((DDMonitorInfo) ddUriMonitorList.get(index)).mObjectNextUrl != null)
            Log.e(TAG,
                    "mObjectNextUrl is "
                            + ((DDMonitorInfo) ddUriMonitorList.get(index)).mObjectNextUrl);

        if (((DDMonitorInfo) ddUriMonitorList.get(index)).mObjectVersion != null)
            Log.e(TAG,
                    "mObjectVersion is "
                            + ((DDMonitorInfo) ddUriMonitorList.get(index)).mObjectVersion);
        // prints values : end

        if(OmaDownloadActivity.sIsOmaDlErrorHandlingEnabled){
        if ( ((DDMonitorInfo) ddUriMonitorList.get(index)).mObjectVersion != null) { // not starting with 1.0
            if	( !(((DDMonitorInfo) ddUriMonitorList.get(index)).mObjectVersion.startsWith("1"))) {
				if (mLastObjectInstallNotifyUrl != null) {
					new Thread(new Runnable() {
						@Override
						public void run() {
							statusReport = STATUS_INVALID_DD_VERSION;
							postInstallNotification(mContext, mLastObjectInstallNotifyUrl,
									statusReport);
						}
					}).start();
				}
				Toast.makeText(getApplicationContext(),
				        R.string.invalid_version,
                        Toast.LENGTH_SHORT).show();
				}
            return false;
        }
    }

        if (((DDMonitorInfo) ddUriMonitorList.get(index)).mObjectURL == null)
            return false;
        return true;
    }

    private void deleteDDFile(Context ctx, String filename) {
        String where = Downloads.Impl._DATA + "='" + filename + "'";
        ctx.getContentResolver()
                .delete(Downloads.Impl.CONTENT_URI, where, null);

        // Delete dd file from the real path
        String filepath = filename.replace("/storage/emulated/0", "/storage/emulated/legacy");
        Log.e(TAG, "delete dd file: " + filepath);
        File file = new File(filepath);
        if (file.exists()) {
            file.delete();
        }
    }

    private void notifyConfirmWin(Context ctx, int index) {
        Log.e(TAG, "Notify confirm Win");
        Intent target = new Intent(this, OmaDownloadActivity.class);
        target.setAction(OmaDownloadActivity.ACTION_OMADL_CONFIRM);
        if (((DDMonitorInfo) ddUriMonitorList.get(index)).mObjectURL != null)
            target.putExtra(OmaDownloadActivity.EXTRA_OBJECTURL,
                    ((DDMonitorInfo) ddUriMonitorList.get(index)).mObjectURL);
        if (((DDMonitorInfo) ddUriMonitorList.get(index)).mObjectName != null)
            target.putExtra(OmaDownloadActivity.EXTRA_OBJECTNAME,
                    ((DDMonitorInfo) ddUriMonitorList.get(index)).mObjectName);
        if (((DDMonitorInfo) ddUriMonitorList.get(index)).mObjectVendor != null)
            target.putExtra(OmaDownloadActivity.EXTRA_OBJECTVENDOR,
                    ((DDMonitorInfo) ddUriMonitorList.get(index)).mObjectVendor);
        if (((DDMonitorInfo) ddUriMonitorList.get(index)).mObjectDescription != null)
            target.putExtra(
                    OmaDownloadActivity.EXTRA_OBJECTDESCRIPTION,
                    ((DDMonitorInfo) ddUriMonitorList.get(index)).mObjectDescription);
        if (((DDMonitorInfo) ddUriMonitorList.get(index)).mObjectSize != null)
            target.putExtra(OmaDownloadActivity.EXTRA_OBJECTSIZE,
                    ((DDMonitorInfo) ddUriMonitorList.get(index)).mObjectSize);
        if (((DDMonitorInfo) ddUriMonitorList.get(index)).mObjectType != null)
            target.putExtra(OmaDownloadActivity.EXTRA_OBJECTTYPE,
                    ((DDMonitorInfo) ddUriMonitorList.get(index)).mObjectType);
        if(mLastObjectMimeType.toString() != null)
            target.putExtra(OmaDownloadActivity.EXTRA_OBJECTALLMIMETYPE, mLastObjectMimeType.toString());
        if((getResources().getBoolean(R.bool.enableOMADLErrorHandling))){
        if (((DDMonitorInfo) ddUriMonitorList.get(index)).mObjectInstallNotifyUrl != null)
            target.putExtra(OmaDownloadActivity.EXTRA_OBJECTINSTALLNOTIFYURL,
                    ((DDMonitorInfo) ddUriMonitorList.get(index)).mObjectInstallNotifyUrl);
        }

        ctx.startActivity(target.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK));
    }

    private void notifyResultWin(Context ctx, String nextUrl) {
        Log.e(TAG, "Notify notifyResultWin");
        if (nextUrl != null) {
            Intent target = new Intent(this, OmaDownloadActivity.class);
            target.setAction(OmaDownloadActivity.ACTION_OMADL_REDIRECT);
            target.putExtra(OmaDownloadActivity.EXTRA_OBJECTNEXTURL, nextUrl);
            ctx.startActivity(target.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK));
        }
    }

    private int mapDDTag(String tag) {
        int i = 0;
        for (; i < ddTagMapping.length; i++) {
            if (ddTagMapping[i].equalsIgnoreCase(tag))
                break;
        }
        if (i < ddTagMapping.length)
            return i;
        else
            return -1;
    }

    private void postInstallNotification(Context ctx, String postUrl) {
        String successResp = "900 Success";
        try {
            URL serverUrl = new URL(postUrl);
            HttpURLConnection conn = (HttpURLConnection) serverUrl
                    .openConnection();

            conn.setRequestMethod("POST");
            conn.setUseCaches(false);
            conn.setDoOutput(true);
            conn.setDoInput(true);

            byte[] postContent = successResp.getBytes("utf-8");
            conn.setRequestProperty("Content-length", "" + postContent.length);
            conn.setRequestProperty("Content-Type", "application/octet-stream");
            conn.setRequestProperty("Connection", "Keep-Alive");
            conn.setRequestProperty("Charset", "UTF-8");

            OutputStream ostream = conn.getOutputStream();
            ostream.write(postContent);
            ostream.close();

            int nErr = conn.getResponseCode();
            Log.e(TAG, "Post response " + nErr);
        } catch (Exception e) {
            Log.e(TAG, "Post Failed " + e);
        }
    }

    public static void postInstallNotification(Context ctx, String postUrl,int statusReport) {

    	int statusCode ;
    	String successResp ;
    	switch(statusReport){
    	case STATUS_SUCCESS :
    		successResp = "900 Success";
    		break;
    	case STATUS_INSUFFICIENT_MEMORY :
    		successResp = "901 Insufficient memory";
    		break;
    	case STATUS_USER_CANCELLED :
    		successResp = "902 User Cancelled";
    		break;
    	case STATUS_LOSS_OF_SERVICE :
    		successResp = "903 Loss of Service";
    		break;
    	case STATUS_ATTRIBUTE_MISMATCH :
    		successResp = "905 Attirbute mismatch";
    		break;
    	case STATUS_INVALID_DESCRIPTOR :
    		successResp = "906 Invalid descriptor";
    		break;
    	case STATUS_INVALID_DD_VERSION :
    		successResp = "951 Invalid DD version";
    		break;
    	case STATUS_DEVICE_ABORTED :
    		successResp = "952 Device aborted";
    		break;
    	case STATUS_NON_ACCEPTABLE_CONTENT :
    		successResp = "953 Non Acceptable content";
    		break;
    	case STATUS_LOAD_ERROR :
    		successResp = "954 Load error";
    		break;

    	default :
    		successResp = "952 Device aborted";
    		break;
    	}
        Log.i(TAG,"Status code="+successResp);
        try {
            URL serverUrl = new URL(postUrl);
            HttpURLConnection conn = (HttpURLConnection) serverUrl
                    .openConnection();

            conn.setRequestMethod("POST");
            conn.setUseCaches(false);
            conn.setDoOutput(true);
            conn.setDoInput(true);
            Log.i(TAG,"postInstallNotification;  successResp = "+successResp);


            byte[] postContent = successResp.getBytes("utf-8");
            conn.setRequestProperty("Content-length", "" + postContent.length);
            conn.setRequestProperty("Content-Type", "application/octet-stream");
            conn.setRequestProperty("Connection", "Keep-Alive");
            conn.setRequestProperty("Charset", "UTF-8");

            OutputStream ostream = conn.getOutputStream();
            ostream.write(postContent);
            ostream.close();

            int nErr = conn.getResponseCode();
            Log.e(TAG, "Post response " + nErr);
            if(nErr >= 200 && nErr < 300){//success
                return ;
            }else{
                deleteMMFile(ctx, mmFilename);
            }

        } catch (Exception e) {
            Log.e(TAG, "Post Failed " + e);
            deleteMMFile(ctx, mmFilename);
        }
        return;
    }

    //delete MM file
    private static void deleteMMFile(Context ctx, String filename) {
        String where = Downloads.Impl._DATA + "='" + filename + "'";
        ctx.getContentResolver()
                .delete(Downloads.Impl.CONTENT_URI, where, null);

        if(null == filename){
	    return;
        }
        String filepath = filename.replace("/storage/emulated/0", "/storage/emulated/legacy");
        Log.e(TAG, "delete MM file: " + filepath);
        File file = new File(filepath);
        if (file.exists()) {
            file.delete();
        }
    }
}
