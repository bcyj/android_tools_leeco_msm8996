/*
 *
 * Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
package com.qualcomm.universaldownload;

import android.app.AlarmManager;
import android.app.PendingIntent;
import android.app.Service;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.res.AssetManager;
import android.net.ConnectivityManager;
import android.os.AsyncTask;
import android.os.Build;
import android.os.IBinder;
import android.os.PowerManager;
import android.os.RemoteException;
import android.os.SystemClock;
import android.telephony.TelephonyManager;
import android.util.Log;
import com.qualcomm.qcrilhook.QcRilHook;
import com.qualcomm.qcrilhook.QcRilHookCallback;
import org.apache.http.HttpResponse;
import org.apache.http.HttpStatus;
import org.apache.http.HttpVersion;
import org.apache.http.client.ClientProtocolException;
import org.apache.http.client.HttpClient;
import org.apache.http.client.methods.HttpGet;
import org.apache.http.client.methods.HttpPost;
import org.apache.http.conn.ClientConnectionManager;
import org.apache.http.conn.ConnectTimeoutException;
import org.apache.http.conn.scheme.PlainSocketFactory;
import org.apache.http.conn.scheme.Scheme;
import org.apache.http.conn.scheme.SchemeRegistry;
import org.apache.http.conn.ssl.SSLSocketFactory;
import org.apache.http.entity.StringEntity;
import org.apache.http.impl.client.DefaultHttpClient;
import org.apache.http.impl.conn.tsccm.ThreadSafeClientConnManager;
import org.apache.http.params.BasicHttpParams;
import org.apache.http.params.HttpConnectionParams;
import org.apache.http.params.HttpParams;
import org.apache.http.params.HttpProtocolParams;
import org.apache.http.protocol.HTTP;
import org.xml.sax.SAXException;

import javax.xml.parsers.ParserConfigurationException;
import javax.xml.parsers.SAXParser;
import javax.xml.parsers.SAXParserFactory;

import java.io.DataInputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;
import java.io.UnsupportedEncodingException;
import java.lang.ref.WeakReference;
import java.security.KeyManagementException;
import java.security.KeyStore;
import java.security.KeyStoreException;
import java.security.NoSuchAlgorithmException;
import java.security.UnrecoverableKeyException;
import java.security.cert.Certificate;
import java.security.cert.CertificateException;
import java.security.cert.CertificateFactory;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Calendar;
import java.util.Date;
import java.util.List;
import java.util.Random;

public class DownloadService extends Service{

    private static final String TAG = "DownloadService";
    public static final long TEN_DAYS_MILLISECONDS = 10 * 24 * 60 * 60 * 1000;
    private static final long TWELVE_HOURS_MILLISECONDS = 12 * 60 * 60 * 1000;
    private static final int TIME_OUT_DELAY = 5000;
    public static final String ACTION_UPDATE = "com.qualcomm.universaldownload.UPDATE";
    public static final String EXTRA_UPDATE_ADDRESS = "extra_update_address";
    public static final String EXTRA_IS_AUTO_UPDATE = "extra_auto_update";
    private static final String WAP_PUSH_RECEIVED = "android.provider.Telephony.WAP_PUSH_RECEIVED";
    public static final String ACTION_SIMSTATE_CHANGED = "android.intent.action.SIM_STATE_CHANGED";
    private static final boolean DEBUG = true;
    private static final boolean ENABLE_DATA_AT_PUSH = true;

    private static boolean sRunning;
    private DownloadSharePreference mPref;
    private ArrayList<UpdateInfo> mUpdateInfos = new ArrayList<UpdateInfo>();
    private StringBuilder strPostXml = new StringBuilder();
    private StringBuilder tempStrBuilder = new StringBuilder();
    private IDownloadListener mListener;
    private DownloadFile mDownloadingFile;
    private AlarmManager mAlarmManager;
    private boolean isAutoUpdate;
    private DownloadInfoManager mDownloadInfoManager;
    private boolean mIsfromActivity = false;
    private BroadcastReceiver mConnectReceiver = null;
    private BroadcastReceiver mSimStateReceiver = null;
    private QcRilHook mQcRilHook;

    @Override
    public void onCreate() {
        logd("onCreate in");
        super.onCreate();
        mPref = DownloadSharePreference.Instance(this);
        mDownloadInfoManager = DownloadInfoManager.instance(this);
        mAlarmManager = (AlarmManager) getSystemService(Context.ALARM_SERVICE);
        mPref.setonAutoUpdateChanged(new DownloadSharePreference.autoUpdateChangedListener() {
            @Override
            public void onAutoUpdateChanged(boolean autoUpdate) {
                Intent intent = new Intent(ACTION_UPDATE);
                intent.putExtra(EXTRA_IS_AUTO_UPDATE, true);
                if(autoUpdate) {
                    long current = System.currentTimeMillis();
                    long alarmtime = mPref.getAlarmTime();
                    if(alarmtime < current) {
                        if(!sRunning) {
                            new DownloadTask().execute(intent);
                        }
                    } else {
                        sendAutoUpdate(alarmtime - current, true);
                    }
                } else {
                    cancelAutoUpdate(true);
                }
            }
        });
        mQcRilHook = new QcRilHook(this, mQcrilHookCb);
    }

    private boolean mIsQcRilHookReady = false;
    private QcRilHookCallback mQcrilHookCb = new QcRilHookCallback() {
        public void onQcRilHookReady() {
            mIsQcRilHookReady = true;
        }
        public void onQcRilHookDisconnected() {
            mIsQcRilHookReady = false;
        }
    };

    private void registerSimStateReceiver() {
        if(mSimStateReceiver == null) {
            mSimStateReceiver = new BroadcastReceiver() {
                @Override
                public void onReceive(Context context, Intent intent) {
                    logd("receive sim state changed action " + intent);
                    if(NetManager.isCTCardInsert(context) &&
                            NetManager.haveNetConnective(context)) {
                        if(!sRunning) {
                            new DownloadTask().execute(intent);
                        }
                    }
                }
            };
        }

        IntentFilter filter = new IntentFilter();
        filter.addAction(ACTION_SIMSTATE_CHANGED);
        registerReceiver(mSimStateReceiver, filter);
    }

    private void registerConnectReceiver() {
        if(mConnectReceiver == null) {
            mConnectReceiver = new BroadcastReceiver() {
                @Override
                public void onReceive(Context context, Intent intent) {
                    if(NetManager.isCTCardInsert(context) &&
                            NetManager.haveNetConnective(context)) {
                        if(!sRunning) {
                            new DownloadTask().execute(intent);
                        }
                    }
                }
            };
        }

        IntentFilter filter = new IntentFilter();
        filter.addAction(ConnectivityManager.CONNECTIVITY_ACTION);
        registerReceiver(mConnectReceiver, filter);
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        logd("onStartCommand intent is " + intent);
        if(intent == null || (!ACTION_UPDATE.equals(intent.getAction()) &&
                !Intent.ACTION_BOOT_COMPLETED.equals(intent.getAction()) &&
                !ACTION_SIMSTATE_CHANGED.equals(intent.getAction()) &&
                !WAP_PUSH_RECEIVED.equals(intent.getAction()))) {
            return super.onStartCommand(intent, flags, startId);
        }

        PowerManager pm = (PowerManager) getSystemService(Context.POWER_SERVICE);
        PowerManager.WakeLock wl = pm.newWakeLock(PowerManager.PARTIAL_WAKE_LOCK, TAG);
        wl.acquire(5000);
        if(!sRunning) {
            new DownloadTask().execute(intent);
        }
        return super.onStartCommand(intent, flags, startId);
    }

    @Override
    public IBinder onBind(Intent intent) {
        logd("onBind");
        return mBinder;
    }

    @Override
    public boolean onUnbind(Intent intent) {
        unregisterListener();
        return super.onUnbind(intent);
    }

    private final IBinder mBinder = new ServiceStub(this);
    private class ServiceStub extends IDownloadService.Stub {
        private WeakReference<DownloadService> mService;
        public ServiceStub(DownloadService service){
            mService = new WeakReference<DownloadService>(service);
        }

        @Override
        public void checkUpdate(String requestUrl) throws RemoteException {
            mService.get().checkUpdate(requestUrl);
        }

        @Override
        public UpdateInfo getUpdateInfo(String capabilityName) {
            return mService.get().getUpdateInfo(capabilityName);
        }

        @Override
        public void cancelUpdate() throws RemoteException {
        }

        @Override
        public void downloadData(List<UpdateInfo> infos) throws RemoteException {
            mService.get().downloadData(infos);
        }

        @Override
        public void cancelDownload() throws RemoteException {
            mService.get().cancelDownload();
        }

        @Override
        public void registerListener(IDownloadListener listener) throws RemoteException {
            mService.get().registerListener(listener);
        }

        @Override
        public void unregisterListener() throws RemoteException {
            mService.get().unregisterListener();
        }
    }

    private void cancelDownload() {
        if(mDownloadingFile != null) {
            mDownloadingFile.setDownloadState(false);
        }
    }

    private void registerListener(IDownloadListener listener) {
        mListener = listener;
    }

    private void unregisterListener() {
        mListener = null;
    }

    private void downloadData(List<UpdateInfo> infos) {
        final List<DownloadInfo> downloadInfos = new ArrayList<DownloadInfo>();
        for (UpdateInfo info : infos) {
            logd("downloadData info " + info);
            DownloadInfo downloadInfo = mDownloadInfoManager.getDownloadInfo(info.getCapability());
            if(downloadInfo == null) {
                downloadInfo = new DownloadInfo(info);
                mDownloadInfoManager.saveDownloadInfo(downloadInfo);
            }
            downloadInfos.add(downloadInfo);
        }
        new Thread(new Runnable() {
            @Override
            public void run() {
                downloadDataInternal(downloadInfos);
            }
        }).start();
    }

    private void downloadDataInternal(final List<DownloadInfo> infos) {
        final int count = infos.size();
        for (int i = 0; i < count; i++) {
            final DownloadInfo downloadInfo = infos.get(i);
            mDownloadingFile = new DownloadFile(downloadInfo, getFilesDir());
            mDownloadInfoManager.updateDownloadInfo(downloadInfo, mDownloadingFile.getFilePath());
            final int fileSize = mDownloadingFile.getFileSize();
            logd("downloadDataInternal fileSize is " + fileSize);
            try {
                final int finalI = i;
                mDownloadingFile.download(new DownloadFile.DownloadListener() {
                    @Override
                    public void onProgressUpdate(int downLength) {
                        logd("onProgressUpdate downlength is " + downLength);
                        mDownloadInfoManager.updateDownloadInfo(downloadInfo,
                                DownloadInfo.NOT_DOWNLOAD, downLength);
                        try {
                            if(mListener != null) {
                                int progress = (finalI * 100) / count +
                                        (downLength * 100) / (fileSize * count);
                                mListener.onDownloadProgressUpdate(progress);
                            }
                        } catch (RemoteException e) {
                            e.printStackTrace();
                        }
                    }

                    @Override
                    public void onComplete() {
                        logd("download onComplete");
                        mDownloadInfoManager.updateDownloadInfo(downloadInfo,
                                DownloadInfo.DOWNLOADED, 0);
                        if(mPref.getPassiveAddress() != null) {
                            mPref.setPassiveAddress(null);
                            mPref.save();
                        }
                        try {
                            if(mListener != null) {
                                mListener.onOneFileComplete(finalI + 1);
                                if(finalI == count - 1) {
                                    mListener.onAllComplete();
                                }
                            }
                        } catch (RemoteException e) {
                            e.printStackTrace();
                        }
                        if(finalI == count - 1) {
                            updateData();
                            postResponse();
                        }
                        if(ENABLE_DATA_AT_PUSH) {
                            if(NetManager.isConnectSettingChanged()) {
                                NetManager.resetDataConnect(DownloadService.this);
                            }
                        }
                    }

                    @Override
                    public void onDownloadFailed() {

                    }
                });
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
    }

    private void updateData() {
        List<DownloadInfo> dowloadedInfo = mDownloadInfoManager.getDownloadInfos(
                DownloadInfo.DOWNLOADED);
        if(tempStrBuilder.length() != 0) {
            tempStrBuilder.delete(0, tempStrBuilder.length());
        }
        for(DownloadInfo info : dowloadedInfo) {
            boolean result = checkMd5(info.getDownloadFile(), info.getMd5());
            logd("checkMd5 result is " + result);
            if(result) {
                if(result = updatePlmntoRIL(info)) {
                    mPref.changeCapability(info.getCapability(), info.getVersion());
                    mPref.save();
                }
            }
            tempStrBuilder.append("<capability>" + info.getCapability() +
                    "</capability>\n");
            tempStrBuilder.append("<cver>" + mPref.getVersion(info.getCapability()) +
                    "</cver>\n");
            tempStrBuilder.append("<cstate>" + (result ? 1 : 0) +  "</cstate>\n");
            mDownloadInfoManager.deleteDownloadInfo(info.getCapability());
        }
        mDownloadInfoManager.closeDb();
        mPref.updateLastTime(String.valueOf(System.currentTimeMillis()));
        mPref.save();
    }

    /**
     * Get the newest data info according url, If get success,
     * it will update updateinfos, server address and response server.
     * @param requesturl the url which get
     * */
    private void checkUpdate(final String requesturl) {
        mIsfromActivity = true;

       new Thread(new Runnable() {
           @Override
           public void run() {
               checkUpdateInternal(requesturl);
           }
       }).start();
    }

    private void checkUpdateInternal(String requesturl) {
        if(!NetManager.isCTCardInsert(this)) {
            return;
        }
        mUpdateInfos.clear();
        HttpGet httpRequest = new HttpGet(requesturl);
        HttpClient httpClient = createSafehttpClient();
        httpClient.getParams().setIntParameter(HttpConnectionParams.SO_TIMEOUT,
                TIME_OUT_DELAY); // request timeout
        httpClient.getParams().setIntParameter(HttpConnectionParams.CONNECTION_TIMEOUT,
                TIME_OUT_DELAY);// connection timeout

        int trytime = 3;
        do {
            try {
                HttpResponse httpResponse = httpClient.execute(httpRequest);
                if(httpResponse.getStatusLine().getStatusCode() == HttpStatus.SC_OK) {
                    SAXParserFactory factory = SAXParserFactory.newInstance();
                    SAXParser parser = factory.newSAXParser();
                    XMLContentHandler handler = new XMLContentHandler();
                    parser.parse(httpResponse.getEntity().getContent(), handler);
                    mUpdateInfos = handler.getUpdateInfos();

                    String serverIP = handler.getServerIP();
                    if(serverIP != null && !serverIP.isEmpty()) {
                        mPref.setServerAddress(serverIP);
                    }

                    mPref.setResponseAddr(handler.getResponseServer());
                    mPref.save();
                    if(mListener != null) {
                        mListener.onUpdateComplete(true);
                    }
                    break;
                } else {
                    logd("Http get failed status is " +
                            httpResponse.getStatusLine().getStatusCode());
                    trytime--;
                }
            } catch (ClientProtocolException e) {
                trytime--;
                Log.e(TAG, "ClientProtocolException " + e);
            } catch (ConnectTimeoutException e) {
                trytime--;
                Log.e(TAG, "ConnectTimeoutException " + e);
            } catch(IOException e) {
                trytime--;
                Log.e(TAG, "IOException " + e);
            } catch (ParserConfigurationException e) {
                trytime--;
                Log.e(TAG, "ParserConfigurationException " + e);
            } catch (SAXException e) {
                trytime--;
                Log.e(TAG, "SAXException " + e);
            } catch (RemoteException e) {
                Log.e(TAG, "remote exception " + e);
                break;
            }
        } while(trytime > 0);
        if(trytime == 0) {
            if(mIsfromActivity) {
                try {
                    if(mListener != null) {
                        mListener.onUpdateComplete(false);
                    }
                } catch (RemoteException e) {
                    Log.e(TAG, "remote call error");
                }
                mIsfromActivity = false;
            } else {
                logd("check update failed 3 times, restart after 12 hours");
                sendAutoUpdate(TWELVE_HOURS_MILLISECONDS, isAutoUpdate);
            }
        }
    }

    private HttpClient createSafehttpClient() {
        HttpParams params = new BasicHttpParams();
        HttpProtocolParams.setVersion(params, HttpVersion.HTTP_1_1);
        HttpProtocolParams.setContentCharset(params,
                HTTP.DEFAULT_CONTENT_CHARSET);
        HttpProtocolParams.setUseExpectContinue(params, true);
        SchemeRegistry schReg = new SchemeRegistry();
        KeyStore keyStore = null;
        AssetManager am = getAssets();
        try {
            keyStore = KeyStore.getInstance(KeyStore.getDefaultType());
            keyStore.load(null, null);

            InputStream in = am.open("ca.crt");
            CertificateFactory cerFactory = CertificateFactory.getInstance("X.509");
            Certificate cer = cerFactory.generateCertificate(in);
            keyStore.setCertificateEntry("trust", cer);

            in = am.open("tomcatrootv3.crt");
            cer = cerFactory.generateCertificate(in);
            keyStore.setCertificateEntry("trust", cer);
            schReg.register(new Scheme("http", PlainSocketFactory
                    .getSocketFactory(), 80));
            schReg.register(new Scheme("https", new SSLSocketFactory(keyStore), 443));
        } catch (KeyStoreException e) {
            e.printStackTrace();
        } catch (CertificateException e) {
            e.printStackTrace();
        } catch (NoSuchAlgorithmException e) {
            e.printStackTrace();
        } catch (IOException e) {
            e.printStackTrace();
        } catch (UnrecoverableKeyException e) {
            e.printStackTrace();
        } catch (KeyManagementException e) {
            e.printStackTrace();
        }
        ClientConnectionManager conMgr = new ThreadSafeClientConnManager(
                params, schReg);
        return new DefaultHttpClient(conMgr, params);
    }

    private void sendDelayAlarm(String address) {
        Intent newIntent = new Intent(DownloadService.ACTION_UPDATE);
        newIntent.putExtra(DownloadService.EXTRA_UPDATE_ADDRESS, address);
        newIntent.putExtra(DownloadService.EXTRA_IS_AUTO_UPDATE, false);
        PendingIntent pendingIntent = PendingIntent.getBroadcast(this, 0, newIntent,
                PendingIntent.FLAG_UPDATE_CURRENT);
        Random rnd = new Random();
        long milliseconds = (rnd.nextInt(23) + 1) * 60 * 60 * 1000;
        long alarmTime = SystemClock.elapsedRealtime() + milliseconds;
        logd("Send wappush update at " + new Date(alarmTime));
        AlarmManager alarm = (AlarmManager) getSystemService(Context.ALARM_SERVICE);
        alarm.set(AlarmManager.ELAPSED_REALTIME_WAKEUP, alarmTime, pendingIntent);
    }

    private class DownloadTask extends AsyncTask<Intent, Integer, Void>{
        private String mUrl;

        @Override
        protected void onPreExecute() {
            super.onPreExecute();
            sRunning = true;
        }

        @Override
        protected Void doInBackground(Intent... intents) {
            logd("DownloadTask run in background params " + intents);
            Intent intent = intents[0];
            String action = intent.getAction();
            boolean doCheck = true;
            boolean doDownload = true;
            //update and post response will do in the download complete callback.
            //so it does not call it. but if we need update only, we should set them to true
            boolean doUpdate = false;
            boolean doPostResponse = false;
            List<DownloadInfo> downloadInfoList = null;

            if(intent.hasExtra(EXTRA_UPDATE_ADDRESS)) {
                mUrl = intent.getStringExtra(EXTRA_UPDATE_ADDRESS);
                mPref.setPassiveAddress(mUrl);
                mPref.save();
            } else {
                mUrl = mPref.getServerAddr();
            }

            if(WAP_PUSH_RECEIVED.equals(intent.getAction())) {
                // when receive wap push, update it at 24 hours.
                sendDelayAlarm(mUrl);
                return null;
            }

            if(!NetManager.isCTCardInsert(DownloadService.this)) {
                // There is not CT card was inserted, so we register sim state receiver
                // this procedure will called again once recognized CT Card
                registerSimStateReceiver();
                return null;
            }

            if(!NetManager.haveNetConnective(DownloadService.this)) {
                // There is not have available network. So we register a receiver which receive
                // connect state. This procedure will called again once have available network.
                registerConnectReceiver();
                if(ENABLE_DATA_AT_PUSH) {
                    // It will open the data connection if this called by wap push receiver.
                    if(mPref.getPassiveAddress() != null &&
                            NetManager.CTCardAvailable(DownloadService.this, 0)) {
                        NetManager.setDataEnableTo(DownloadService.this, 0);
                    }
                }
                return null;
            }

            if(ACTION_UPDATE.equals(action)) {
                isAutoUpdate = intent.getBooleanExtra(EXTRA_IS_AUTO_UPDATE, true);
            } else {
                isAutoUpdate = false;
            }

            if(Intent.ACTION_BOOT_COMPLETED.equals(action) ||
                    ACTION_SIMSTATE_CHANGED.equals(action) ||
                    ConnectivityManager.CONNECTIVITY_ACTION.equals(action)) {
                /* check whether have not downloaded info and not updated info*/
                downloadInfoList = mDownloadInfoManager.getDownloadInfos(
                        DownloadInfo.NOT_DOWNLOAD);
                List<DownloadInfo> downloadedInfos = mDownloadInfoManager.getDownloadInfos(
                        DownloadInfo.DOWNLOADED);
                boolean shouldCheck = mPref.getLatsUpdateTime().equals("");
                long shouldchecktime = mPref.getAlarmTime();
                shouldCheck = shouldCheck || (shouldchecktime < System.currentTimeMillis());
                if(shouldCheck) {
                    isAutoUpdate = true;
                }
                if(downloadInfoList.size() > 0) {
                    // have some downloading data, need continue download
                    // it needn't check data
                    doCheck = false;
                } else if(downloadedInfos.size() > 0){
                    //if have no update data, update only.
                    doCheck = false;
                    doDownload = false;
                    doUpdate = true;
                    doPostResponse = true;
                } else if(mPref.getPassiveAddress() != null) {
                    mUrl = mPref.getPassiveAddress();
                    isAutoUpdate = false;
                } else if(shouldCheck) {
                    //The check time is reached, but didn't ever check.
                    //do update all
                } else {
                    if(Intent.ACTION_BOOT_COMPLETED.equals(action)) {
                        //set alarm manger if the phone restart
                        sendAutoUpdate(shouldchecktime - System.currentTimeMillis(), true);
                    }
                    return null;
                }
            }

            if(doCheck) {
                //get the info data
                checkUpdateInternal(mUrl);
                if(mUpdateInfos.size() == 0) {
                    return null;
                }
                if(isAutoUpdate) {
                    // start next update thread after ten days
                    sendAutoUpdate(TEN_DAYS_MILLISECONDS, true);
                }
            }

            if(doDownload) {
                if(downloadInfoList == null || downloadInfoList.size() == 0) {
                    List<UpdateItem> updateItems = mPref.getCheckedItem();
                    downloadInfoList = new ArrayList<DownloadInfo>();
                    for(UpdateItem item : updateItems){
                        UpdateInfo updateInfo = getUpdateInfo(item.getCapabilityName());
                        if(updateInfo != null) {
                            if(item.getVersion().equals("") || updateInfo.getVersion().
                                    compareToIgnoreCase(item.getVersion()) > 0) {
                                DownloadInfo info = mDownloadInfoManager.getDownloadInfo(
                                        updateInfo.getCapability());
                                if(info == null) {
                                    info = new DownloadInfo(updateInfo);
                                    mDownloadInfoManager.saveDownloadInfo(info);
                                }
                                downloadInfoList.add(info);
                            }
                        }
                    }
                }
                if(downloadInfoList != null && downloadInfoList.size() > 0) {
                    //download the data if have update
                    downloadDataInternal(downloadInfoList);
                } else {
                    doPostResponse = true;
                }
            }
            if(doUpdate) {
                List<DownloadInfo> downloadInfos =
                        mDownloadInfoManager.getDownloadInfos(DownloadInfo.DOWNLOADED);
                if( downloadInfos != null &&downloadInfos.size() > 0) {
                    //update data to modem
                    updateData();
                }
            }
            //post response
            if(doPostResponse) {
                postResponse();
            }

            if(mConnectReceiver != null) {
                unregisterReceiver(mConnectReceiver);
                mConnectReceiver = null;
            }
            if(mSimStateReceiver != null) {
                unregisterReceiver(mSimStateReceiver);
                mSimStateReceiver = null;
            }
            return null;
        }

        @Override
        protected void onPostExecute(Void aVoid) {
            sRunning = false;
            super.onPostExecute(aVoid);
        }

        public String getDownloadingUrl() {
            return mUrl;
        }
    }

    private void sendAutoUpdate(long milliseconds, boolean autoUpdate) {
        Intent intent = new Intent(ACTION_UPDATE);
        intent.putExtra(EXTRA_IS_AUTO_UPDATE, autoUpdate);
        PendingIntent pendingIntent = PendingIntent.getBroadcast(this, 0, intent,
                PendingIntent.FLAG_UPDATE_CURRENT);
        long alarmTime = System.currentTimeMillis() + milliseconds;
        mPref.setAlarmTime(alarmTime);
        mPref.save();
        mAlarmManager.set(AlarmManager.RTC_WAKEUP, alarmTime, pendingIntent);
    }

    private void cancelAutoUpdate(boolean autoUpdate) {
        Intent intent = new Intent(ACTION_UPDATE);
        intent.putExtra(EXTRA_IS_AUTO_UPDATE, autoUpdate);
        AlarmManager am = (AlarmManager) getSystemService(ALARM_SERVICE);
        PendingIntent pendingIntent = PendingIntent.getBroadcast(DownloadService.this,
                0, intent, PendingIntent.FLAG_UPDATE_CURRENT);
        am.cancel(pendingIntent);
    }

    private boolean checkMd5(String filePath, String md5) {
        logd("check md5, md5 is " + md5);
        logd("check md5, the " + filePath + "'s md5 is " + Md5CheckSum.md5Sum(filePath));
        if(md5.toUpperCase().equals(Md5CheckSum.md5Sum(filePath))) {
            return true;
        }
        return false;
    }

    private boolean updatePlmntoRIL(DownloadInfo info) {
        if(!mIsQcRilHookReady) {
            return false;
        }
        boolean result = true;
        File plmnFile = new File(info.getDownloadFile());
        DataInputStream dis = null;
        byte[] datas;
        if(plmnFile.exists()) {
            try {
                dis = new DataInputStream(new FileInputStream(plmnFile));
                int num = 0;
                logd("updatePlmntoRIL, file length is " + dis.available());
                if(dis.available() % 5 != 0) {
                    return false;
                }
                datas = new byte[dis.available() + 2];
                byte[] buffer = new byte[5];
                int pos = 2;
                while(dis.available() > 0) {
                    //read one data
                    dis.read(buffer, 0, 5);
                    datas[pos++] = buffer[2];// low byte on the high
                    datas[pos++] = buffer[1];
                    datas[pos++] = buffer[0];
                    datas[pos++] = buffer[4]; //low byte is at high
                    datas[pos++] = buffer[3];
                    num++;
                }
                logd("updatePlmntoRIL, it have total  " + num + " datas");
                datas[0] = (byte) num; //low byte is at high
                datas[1] = (byte) (num >> 8);
            } catch (FileNotFoundException e) {
                Log.e(TAG, "read plmn file error: " + e);
                return false;
            } catch (IOException e) {
                Log.e(TAG, "read plmn file error: " + e);
                return false;
            } finally {
                if(dis != null) {
                    try {
                        dis.close();
                    } catch (IOException e) {
                        e.printStackTrace();
                    }
                }
            }
        } else {
            return false;
        }
        TelephonyManager mTelephonyManager = TelephonyManager.getDefault();
        for(int i = 0; i < mTelephonyManager.getPhoneCount(); i++) {
            if(NetManager.CTCardAvailable(this, i)) {
                result = result && mQcRilHook.qcrilSetBuiltInPLMNList(datas, i);
            }
        }
        return result;
    }

    private void postResponse() {
        logd("postResponse");
        if(strPostXml.length() != 0) {
            strPostXml.delete(0, strPostXml.length());
        }
        HttpPost httpRequest = new HttpPost(mPref.getResponseAddr());
        HttpClient httpClient = createSafehttpClient();

        strPostXml.append("<xml>\n");
        strPostXml.append("<ctime>" + getCurrentTimeString() + "</ctime>\n");
        strPostXml.append("<cversion>" + Build.DEVICE + "</cversion>\n");
        strPostXml.append("<model>" + Build.MODEL+ "</model>\n");
        strPostXml.append("<cimsi>" + NetManager.getIMSI(this) + "</cimsi>\n");
        strPostXml.append("<cmeid>" + NetManager.getMEID(this) + "</cmeid>\n");
        strPostXml.append(tempStrBuilder);
        strPostXml.append("</xml>\n");

        try {
            StringEntity s = new StringEntity(strPostXml.toString(), HTTP.UTF_8);
            s.setContentType("text/xml charset=utf-8");
            httpRequest.setEntity(s);
            HttpResponse re = httpClient.execute(httpRequest);
            if (re.getStatusLine().getStatusCode() != 200) {
                Log.e(TAG, "postResponse state is " + re.getStatusLine().getStatusCode());
            }
        } catch (UnsupportedEncodingException e) {
            Log.e(TAG, "post response error: " + e);
        } catch (ClientProtocolException e) {
            Log.e(TAG, "post response error: " + e);
        } catch (IOException e) {
            Log.e(TAG, "post response error: " + e);
        }
    }

    private String getCurrentTimeString() {
        Calendar cal = Calendar.getInstance();
        SimpleDateFormat sdf = new SimpleDateFormat("yyyyMMddHHmmss");
        String date = sdf.format(cal.getTime());
        return date;
    }

    private UpdateInfo getUpdateInfo(String capabilityName) {
        for(UpdateInfo updateInfo : mUpdateInfos) {
            if(updateInfo.getCapability().equals(capabilityName)) {
                return updateInfo;
            }
        }
        return null;
    }

    private void logd(String msg) {
        if(DEBUG) {
            Log.d(TAG, msg);
        }
    }
}
