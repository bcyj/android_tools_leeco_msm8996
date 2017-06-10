/*
 * Copyright (c) 2013, Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
package com.qualcomm.factory.Framework;

import java.io.File;
import java.io.FileInputStream;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import org.xmlpull.v1.XmlPullParser;
import org.xmlpull.v1.XmlPullParserFactory;

import android.app.ActivityManager;
import android.app.AlertDialog;
import android.app.ListActivity;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.Intent.ShortcutIconResource;
import android.content.IntentFilter;
import android.content.pm.ActivityInfo;
import android.content.pm.PackageManager;
import android.content.pm.ResolveInfo;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.media.AudioManager;
import android.net.Uri;
import android.os.Bundle;
import android.os.Handler;
import android.util.Log;
import android.view.KeyEvent;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.MotionEvent;
import android.view.SubMenu;
import android.view.View;
import android.view.ViewGroup;
import android.view.WindowManager;
import android.widget.BaseAdapter;
import android.widget.ImageView;
import android.widget.ListView;
import android.widget.TextView;
import android.widget.Toast;

import com.qualcomm.factory.R;
import com.qualcomm.factory.Utilities;
import com.qualcomm.factory.Values;
import com.qualcomm.factory.Framework.MainApp.FunctionItem;

public class Framework extends ListActivity {
    
    private final static String TAG = "Framework";
    private boolean mExitFlag = false;
    private Handler mHandler = new Handler();
    private LayoutInflater mInflater;
    Context mContext;
    String TempFile;
    private long curBackButtonTime = 0;
    private long lastBackButtonTime = 0;
    private int positionClicked = 1;
    final static int[] flagList = new int[99];
    final static int[] resultCodeList = new int[99];
    private static final int MENU_CLEAN_STATE = Menu.FIRST;
    private static final int MENU_UNINSTALL = Menu.FIRST + 1;
    private Map<String, FunctionItem> mFunctionItems = new HashMap<String, FunctionItem>();

    private Bitmap passBitmap;
    private Bitmap failBitmap;
    private boolean toStartAutoTest = false;
    private final int AUTO_TEST_TIME_INTERVAL = 900;
    private boolean originChargingStatus = true;
    TextView nvTextView;
    
    static {
        logd("Loading libqcomfm_jni.so");
        System.loadLibrary("qcomfm_jni");
    }
    
    void init(Context context) {
        
        mContext = context;
        lastBackButtonTime = 0;
        curBackButtonTime = 0;
        
        // getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN,
        // WindowManager.LayoutParams.FLAG_FULLSCREEN);
        setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_PORTRAIT);

        mInflater = LayoutInflater.from(context);
        
        passBitmap = BitmapFactory.decodeResource(context.getResources(), R.drawable.test_pass);
        failBitmap = BitmapFactory.decodeResource(context.getResources(), R.drawable.test_fail);
        
        /** nv factory_data_3 */
        if (Values.ENABLE_NV) {
        }
        
        // Write TestLog Start message
        Utilities.writeTestLog("\n=========Start=========", null);
        
        originChargingStatus = getChargingStatus();
        logd("originChargingStatus=" + originChargingStatus);

        // To save test time, enable some devices first
        // Utilities.enableWifi(mContext, true);
        // Utilities.enableBluetooth(true);
        // Utilities.enableGps(mContext, true);
        // Utilities.configScreenTimeout(mContext, 1800000); // 1 min
        // Utilities.configMultiSim(mContext);
        // configSoundEffects(false);
        // createShortcut(context);// add shortcut
        
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        
        super.onCreate(savedInstanceState);
        if (ActivityManager.isUserAMonkey())
            finish();
        else {
            init(getApplicationContext());
            
            String hwPlatform = Utilities.getPlatform();
            setTitle(getString(R.string.app_name) + " " + hwPlatform);
            logd(hwPlatform);
            
            /** Get Test Items */
            FunctionItem functionItems = null;
            String configFile = null;
            for (String tmpConfigFile : Values.CONFIG_FILE_SEARCH_LIST) {
                File tmp = new File(tmpConfigFile);
                if (tmp.exists() && tmp.canRead()) {
                    configFile = tmpConfigFile;
                    logd("Found config file: " + tmpConfigFile);
                    break;
                }
            }
            
            XmlPullParser xmlPullParser = null;
            if (configFile != null) {
                XmlPullParserFactory xmlPullParserFactory;
                try {
                    xmlPullParserFactory = XmlPullParserFactory.newInstance();
                    xmlPullParserFactory.setNamespaceAware(true);
                    xmlPullParser = xmlPullParserFactory.newPullParser();
                    FileInputStream fileInputStream = new FileInputStream(configFile);
                    xmlPullParser.setInput(fileInputStream, "utf-8");
                } catch (Exception e) {
                    e.printStackTrace();
                }
                
            } else {
                if (Values.PRODUCT_MSM7627A_SKU1.equals(hwPlatform)) {
                    xmlPullParser = getResources().getXml(R.xml.item_config_7627a_sku1);
                } else if (Values.PRODUCT_MSM7627A_SKU3.equals(hwPlatform)) {
                    xmlPullParser = getResources().getXml(R.xml.item_config_7627a_sku3);
                } else if (Values.PRODUCT_MSM8X25_SKU5.equals(hwPlatform)) {
                    xmlPullParser = getResources().getXml(R.xml.item_config_8x25_sku5);
                } else if (Values.PRODUCT_MSM7X27_SKU5A.equals(hwPlatform)) {
                    xmlPullParser = getResources().getXml(R.xml.item_config_7x27_sku5a);
                } else if (Values.PRODUCT_MSM7627A_SKU7.equals(hwPlatform)) {
                    xmlPullParser = getResources().getXml(R.xml.item_config_7627a_sku7);
                } else if (Values.PRODUCT_MSM8X25Q_SKUD.equals(hwPlatform)) {
                    xmlPullParser = getResources().getXml(R.xml.item_config_8x25q_skud);
                } else if (Values.PRODUCT_MSM8226.equals(hwPlatform))
                    xmlPullParser = getResources().getXml(R.xml.item_config_8x26);
                else if (Values.PRODUCT_MSM8610.equals(hwPlatform))
                    xmlPullParser = getResources().getXml(R.xml.item_config_8610);
                else
                    xmlPullParser = getResources().getXml(R.xml.item_config_default);
            }
            try {
                int mEventType = xmlPullParser.getEventType();
                /** Parse the xml */
                while (mEventType != XmlPullParser.END_DOCUMENT) {
                    if (mEventType == XmlPullParser.START_TAG) {
                        String name = xmlPullParser.getName();
                        
                        if (name.equals("FunctionItem")) {
                            functionItems = null;
                            String enable = xmlPullParser.getAttributeValue(null, "enable");
                            
                            if (enable != null && enable.equals("true")) {
                                functionItems = new FunctionItem();
                                functionItems.name = xmlPullParser.getAttributeValue(null, "name");
                                functionItems.auto = xmlPullParser.getAttributeValue(null, "auto");
                                functionItems.packageName = xmlPullParser.getAttributeValue(null, "packageName");
                                Utilities.parseParameter(xmlPullParser.getAttributeValue(null, "parameter"),
                                        functionItems.parameter);
                            }
                        }
                    } else if (mEventType == XmlPullParser.END_TAG) {
                        String tagName = xmlPullParser.getName();
                        
                        if (functionItems != null && tagName.equals("FunctionItem")) {
                            // add
                            mFunctionItems.put(functionItems.packageName, functionItems);
                        }
                    }
                    mEventType = xmlPullParser.next();
                }
            } catch (Exception e) {
                loge(e);
            }
            
            // put ItemList into MainApp.getInstance().mItemList
            MainApp.getInstance().mItemList = getItemList(mFunctionItems);
            if (Values.ENABLE_BACKGROUND_SERVICE)
                startService(new Intent(mContext, AutoService.class));
            else {
                // To save test time, enable some devices first
                Utilities.enableWifi(mContext, true);
                Utilities.enableBluetooth(true);
                Utilities.enableGps(mContext, true);
                Utilities.configScreenTimeout(mContext, 1800000);
                Utilities.configMultiSim(mContext);
                Utilities.enableCharging(true);
            }
            setListAdapter(mBaseAdapter);
        }
        
    }

    @Override
    protected void onDestroy() {
        if (!ActivityManager.isUserAMonkey()) {
            
            Utilities.enableWifi(mContext, false);
            Utilities.enableBluetooth(false);
            Utilities.enableGps(mContext, false);
            Utilities.configScreenTimeout(mContext, 60000); // 1 min
            // enableCharging(false);
            if (Values.ENABLE_BACKGROUND_SERVICE) {
                MainApp.getInstance().clearAllService();
                stopService(new Intent(mContext, AutoService.class));
            } else {
                Utilities.enableWifi(mContext, false);
                Utilities.enableBluetooth(false);
                Utilities.enableGps(mContext, false);
                Utilities.configScreenTimeout(mContext, 60000); // 1 min
            }
        }
        super.onDestroy();
    }

    private boolean getChargingStatus() {
        String value = Utilities.getSystemProperties(Values.PROP_CHARGE_DISABLE, null);
        if ("1".equals(value))
            return false;
        else
            return true;
        
    }

    private void configSoundEffects(boolean enable) {
        AudioManager mAudioManager = (AudioManager) getSystemService(Context.AUDIO_SERVICE);
        if (enable)
            mAudioManager.loadSoundEffects();
        else
            mAudioManager.unloadSoundEffects();
    }
    
    public void createShortcut(Context context) {
        Intent intent = new Intent("com.android.launcher.action.INSTALL_SHORTCUT");
        intent.putExtra(Intent.EXTRA_SHORTCUT_NAME, context.getString(R.string.app_name));
        intent.putExtra("duplicate", false);
        Intent appIntent = new Intent();
        appIntent.setAction(Intent.ACTION_MAIN);
        appIntent.setClass(context, getClass());
        intent.putExtra(Intent.EXTRA_SHORTCUT_INTENT, appIntent);
        ShortcutIconResource iconRes = Intent.ShortcutIconResource.fromContext(context, R.drawable.icon);
        intent.putExtra(Intent.EXTRA_SHORTCUT_ICON_RESOURCE, iconRes);
        context.sendBroadcast(intent);
    }
    
    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        
        if (!ActivityManager.isUserAMonkey()) {
            int groupId = 0;

            SubMenu addMenu = menu.addSubMenu(groupId, MENU_CLEAN_STATE, Menu.NONE,
                    R.string.clean_state);
            addMenu.setIcon(android.R.drawable.ic_menu_revert);

            SubMenu resetMenu = menu.addSubMenu(groupId, MENU_UNINSTALL, Menu.NONE,
                    R.string.uninstall);
            resetMenu.setIcon(android.R.drawable.ic_menu_delete);

            MenuInflater inflater = getMenuInflater();
            inflater.inflate(R.menu.action_bar, menu);
        }
        
        return super.onCreateOptionsMenu(menu);
    }
    
    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        
        logd(item.getItemId());
        switch (item.getItemId()) {
        case (MENU_CLEAN_STATE):
            cleanTestState();
            break;
        case (MENU_UNINSTALL):
            Uri uri = Uri.fromParts("package", "com.qualcomm.factory", null);
            startActivity(new Intent(Intent.ACTION_DELETE, uri));
            break;
        case R.id.run_auto_items:
            toStartAutoTest = true;
            positionClicked = getNextUntestedItem(MainApp.getInstance().mItemList);
            loge("pos=" + positionClicked);
            if (positionClicked < 0) {
                toStartAutoTest = false;
            } else {
                Intent intent = (Intent) MainApp.getInstance().mItemList.get(positionClicked).get("intent");
                intent.putExtra(Values.KEY_SERVICE_INDEX, positionClicked);
                startActivityForResult(intent, positionClicked);
            }
            break;
        case R.id.pause_auto_items:
            toStartAutoTest = false;
            break;
        }
        return super.onOptionsItemSelected(item);
    }
    
    @Override
    protected void onResume() {
        if (!ActivityManager.isUserAMonkey()) {
            IntentFilter filter = new IntentFilter(Values.BROADCAST_UPDATE_MAINVIEW);
            registerReceiver(mViewBroadcastReceiver, filter);
        }
        super.onResume();
    }
    
    @Override
    protected void onPause() {
        if (!ActivityManager.isUserAMonkey())
            unregisterReceiver(mViewBroadcastReceiver);
        super.onPause();
    }
    
    private List getItemList(Map<String, FunctionItem> functionItems) {
        
        List<Map> mList = new ArrayList<Map>();
        
        Intent mIntent = new Intent(Intent.ACTION_MAIN, null);
        mIntent.addCategory("android.category.factory.kit");
        
        PackageManager packageManager = getPackageManager();
        /** Retrieve all activities that can be performed for the given intent */
        List<ResolveInfo> list = packageManager.queryIntentActivities(mIntent, 0);
        
        if (list == null)
            super.finish();
        
        int len = list.size();
        for (int i = 0; i < len; i++) {
            ResolveInfo resolveInfo = list.get(i);
            
            String className = resolveInfo.activityInfo.name.substring(0,
                    resolveInfo.activityInfo.name.lastIndexOf('.'));
            FunctionItem functionItem = functionItems.get(className);
            if (functionItem == null) {
                continue;
            }
            Intent intent = new Intent();
            intent.setClassName(resolveInfo.activityInfo.applicationInfo.packageName, resolveInfo.activityInfo.name);
            addItem(mList, intent, functionItem);
        }
        
        return mList;
    }
    
    private void addItem(List<Map> list, Intent intent, FunctionItem functionItem) {

        Map<String, Object> temp = new HashMap<String, Object>();
        temp.put("intent", intent);
        temp.put("title", functionItem.name);
        temp.put("auto", functionItem.auto);
        temp.put("parameter", functionItem.parameter);
        temp.put("result", Utilities.getStringValueSaved(mContext, functionItem.name, "NULL"));
        list.add(temp);
    }
    
    @Override
    protected void onListItemClick(ListView l, View v, int position, long id) {
        
        // Gets the data associated with the specified position in the list.
        logd("click pos=" + position);
        if (!toStartAutoTest) {
            Map map = (Map) l.getItemAtPosition(position);
            positionClicked = position;
            Intent intent = (Intent) map.get("intent");
            intent.putExtra(Values.KEY_SERVICE_INDEX, position);
            startActivityForResult(intent, position);
        }
    }

    protected void updateView(int requestCode, int resultCode) {
        
        resultCodeList[requestCode] = resultCode;
        Map map = (Map) getListView().getItemAtPosition(requestCode);
        String name = (String) map.get("title");
        String result = (resultCode == RESULT_OK ? "OK" : "FAIL");
        map.put("result", result);
        
        mBaseAdapter.notifyDataSetChanged();
    }
    
    protected void cleanTestState() {

        if (MainApp.getInstance().mItemList ==  null)
            return;

        int size = MainApp.getInstance().mItemList.size();
        
        for (int i = 0; i < size; i++) {
            
            Map map = (Map) this.getListView().getItemAtPosition(i);
            map.put("result", "NULL");
            Utilities.saveStringValue(mContext, (String) map.get("title"), null);
        }
        
        mBaseAdapter.notifyDataSetChanged();
    }
    
    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        
        if (positionClicked == requestCode) {
            flagList[requestCode] = 1;
            resultCodeList[requestCode] = resultCode;
            Map map = (Map) this.getListView().getItemAtPosition(requestCode);
            String name = (String) map.get("title");
            String result = (resultCode == RESULT_OK ? "Pass" : "Failed");
            map.put("result", result);
            logd("Test:" + name + "result=" + result);
            Utilities.saveStringValue(mContext, name, result);
            
            mBaseAdapter.notifyDataSetChanged();
            Utilities.writeTestLog(name, result);
            if (toStartAutoTest) {
                mHandler.postDelayed(mRunnable, AUTO_TEST_TIME_INTERVAL);
                int nexPos = getNextUntestedItem(MainApp.getInstance().mItemList);
                if (nexPos > 4)
                    setSelection(nexPos - 4);
            }
            // /** auto test */
            // if (toStartAutoTest) {
            // positionClicked =
            // getNextAutoItem(MainApp.getInstance().mItemList);
            // loge("pos=" + positionClicked);
            // if (positionClicked < 0) {
            // toStartAutoTest = false;
            // } else {
            // Intent intent = (Intent)
            // MainApp.getInstance().mItemList.get(positionClicked).get("intent");
            // startActivityForResult(intent, positionClicked);
            // }
            // }
        }
        
    }
    
    private int getNextUntestedItem(List<? extends Map<String, ?>> list) {
        int pos = -1;
        for (int i = 0; i < list.size(); i++) {
            Map<String, ?> item = list.get(i);
            if ("NULL".equals(item.get("result"))) {
                // if ("true".equals(item.get("auto")) &&
                // "NULL".equals(item.get("result"))) {
                pos = i;
                break;
            }
        }
        return pos;
    }
    
    @Override
    public boolean onKeyUp(int keyCode, KeyEvent event) {
        if (keyCode == KeyEvent.KEYCODE_BACK) {
            curBackButtonTime = System.currentTimeMillis();
            if (curBackButtonTime - lastBackButtonTime < 2000)
                mExitFlag = true;
            lastBackButtonTime = curBackButtonTime;
        }

        return super.onKeyUp(keyCode, event);
    }

    public void finish() {
        
        /*
         * if (mExitFlag == false) { new
         * AlertDialog.Builder(this).setTitle(getString
         * (R.string.control_center_quit_confirm))
         * .setPositiveButton(getString(R.string.yes), new
         * DialogInterface.OnClickListener() {
         * 
         * public void onClick(DialogInterface dialog, int which) {
         * 
         * mExitFlag = true; finish(); }
         * }).setNegativeButton(getString(R.string.no), new
         * DialogInterface.OnClickListener() {
         * 
         * public void onClick(DialogInterface dialog, int which) {
         * 
         * } }).setCancelable(false).show(); return; }
         */
        
        if (!mExitFlag) {
            toast("Press BACK again to quit");
            return;
        }
        /** write NV_FACTORY_DATA_3_I result */
        if (Values.ENABLE_NV) {
        }
        super.finish();
    }
    
    private BaseAdapter mBaseAdapter = new BaseAdapter() {
        
        public View getView(int position, View convertView, ViewGroup parent) {
            
            if (convertView == null)
                convertView = mInflater.inflate(R.layout.list_item, null);
            
            ImageView image = (ImageView) convertView.findViewById(R.id.icon_center);
            TextView text = (TextView) convertView.findViewById(R.id.text_center);
            text.setText((String) (MainApp.getInstance().mItemList.get(position).get("title")));
            
            String result = (String) (String) (MainApp.getInstance().mItemList.get(position).get("result"));
            if (result.equals("NULL") == false)
                image.setImageBitmap(result.equals("Pass") ? passBitmap : failBitmap);
            else
                image.setImageBitmap(null);
            
            return convertView;
        }
        
        public int getCount() {
            
            return MainApp.getInstance().mItemList.size();
        }
        
        public Object getItem(int position) {
            
            return MainApp.getInstance().mItemList.get(position);
        }
        
        public long getItemId(int arg0) {
            
            return 0;
        }
        
    };
    
    private void autoTestItems(List<? extends Map<String, ?>> list) {
        int testNum = list.size();
        for (int pos = 0; pos < testNum; pos++) {
        }
    }
    
    private Runnable mRunnable = new Runnable() {
        
        @Override
        public void run() {
            /** auto test */
            if (toStartAutoTest) {
                positionClicked = getNextUntestedItem(MainApp.getInstance().mItemList);
                loge("pos=" + positionClicked);
                if (positionClicked < 0) {
                    toStartAutoTest = false;
                } else {
                    Intent intent = (Intent) MainApp.getInstance().mItemList.get(positionClicked).get("intent");
                    intent.putExtra(Values.KEY_SERVICE_INDEX, positionClicked);
                    startActivityForResult(intent, positionClicked);
                }
            }
        }
    };
    
    BroadcastReceiver mViewBroadcastReceiver = new BroadcastReceiver() {
        
        @Override
        public void onReceive(Context arg0, Intent arg1) {
            mBaseAdapter.notifyDataSetChanged();
        }
    };

    /** Fixed */
    
    public void toast(Object s) {
        
        if (s == null)
            return;
        Toast.makeText(this, s + "", Toast.LENGTH_SHORT).show();
    }
    
    private void loge(Object e) {
        
        if (e == null)
            return;
        Thread mThread = Thread.currentThread();
        StackTraceElement[] mStackTrace = mThread.getStackTrace();
        String mMethodName = mStackTrace[3].getMethodName();
        e = "[" + mMethodName + "] " + e;
        Log.e(TAG, e + "");
    }
    
    private static void logd(Object s) {
        
        Thread mThread = Thread.currentThread();
        StackTraceElement[] mStackTrace = mThread.getStackTrace();
        String mMethodName = mStackTrace[3].getMethodName();
        
        s = "[" + mMethodName + "] " + s;
        Log.d(TAG, s + "");
    }
}
