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

package com.android.firewall;

import java.util.ArrayList;
import android.app.ListActivity;
import android.app.Dialog;
import android.app.ProgressDialog;
import android.app.AlertDialog;
import android.content.ContentUris;
import android.content.ContentValues;
import android.content.Context;
import android.content.Intent;
import android.content.AsyncQueryHandler;
import android.content.DialogInterface;
import android.content.DialogInterface.OnClickListener;
import android.database.Cursor;
import android.net.Uri;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.os.PowerManager;
import android.provider.ContactsContract;
import android.provider.ContactsContract.PhoneLookup;
import android.provider.ContactsContract.Contacts;
import android.provider.ContactsContract.CommonDataKinds.Phone;
import android.util.Log;
import android.util.SparseBooleanArray;
import android.view.ContextMenu;
import android.view.ContextMenu.ContextMenuInfo;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.view.ViewTreeObserver;
import android.view.KeyEvent;
import android.widget.AdapterView.AdapterContextMenuInfo;
import android.widget.ListView;
import android.widget.Toast;
import android.widget.ResourceCursorAdapter;
import android.widget.EditText;
import android.widget.TextView;
import android.widget.CheckedTextView;
import android.text.InputType;
import android.text.TextUtils;
import android.text.method.DialerKeyListener;
import android.database.ContentObserver;
import java.util.HashMap;
import java.util.LinkedList;
import java.lang.ref.WeakReference;
import android.database.sqlite.SQLiteConstraintException;
//import com.android.internal.telephony.CallerInfo;
import com.android.firewall.FirewallProvider;

import android.provider.CallLog;
import android.provider.CallLog.Calls;
import android.text.format.DateUtils;
import android.widget.ImageButton;
import android.widget.PopupMenu;

public class FirewallHistoryPage extends ListActivity
        implements View.OnCreateContextMenuListener {
    // ===========================================================
    // Constants
    // ===========================================================

    static final String TAG = "FirewallHistoryPage";
    public static final int FIREWALL_LIST_MAX_ITEM_NUM = 100;
    public static final String CONTACT_ITEM_DEFAULT_SORT_ORDER = "person ASC";

    private static final int ID_INDEX = 0;
    private static final int NUMBER_INDEX = 1;
    private static final int DATE_INDEX = 2;
    private static final int NAME_INDEX = 3;
    private static final int TYPE_INDEX = 4;

    private static final int CONTEXT_MENU_DELETE = Menu.FIRST;

    private static final int MENU_MULTI_DEL = Menu.FIRST;

    private final static int DELETE_ITEM = 0;

    private static final int MENU_DELETE = MENU_MULTI_DEL + 1;
    private static final int MENU_RESELECT = MENU_DELETE + 1;
    private static final int MENU_SELECT_ALL = MENU_RESELECT + 1;

    // handle operate msg id
    static final int OPERATE_ERROR = 1;
    static final int OPERATE_SUCCESS = 2;
    static final int OPERATE_ONE = 3;
    static final int OPERATE_CANCEL = 4;
    static final int OPERATE_CLOSE = 5;
    static final int OPERATE_DEL_ALL_OVER = 6;
    static final int OPERATE_DEL_SINGLE_OVER = 7;
    static final int SHOW_TOAST = 8;
    static final int UPDATE_INFO = 9;
    static final int UPDATE_VIEW = 10;

    static final int ACTION_NONE = -1;
    static final int ACTION_ADD = 0;
    static final int ACTION_ADD_FR_CONTACTS = 1;
    static final int ACTION_EDIT = 2;
    static final int ACTION_DELETE = 3;
    static final int ACTION_MULTI_DELETE = 4;

    private static final int DIALOG_ADD_OR_EDIT = 0;
    private static final int DIALOG_SEL_NUM = 1;

    public static final int REQUEST_CONTACT_ITEM = 1;

    private static final String DELETE_SELECTION = "_id=?";

    static final int ID_COLUMN_INDEX = 0;
    static final int PERSON_ID_COLUMN_INDEX = 0;
    static final int DISPLAY_NAME_COLUMN_INDEX = 1;

    private static final int QUERY_TOKEN = 77;

    // ===========================================================
    // Fields
    // ===========================================================

    // for single select(edit or delete)--The position in the list
    private int mSelectedKey = -1;
    // for single select(edit or delete)--The number of the seleted item
    private String mSelectednumber;
    FirewallListAdapter mAdapter;
    private QueryHandler mQueryHandler;
    private ListView mListView;
    PowerManager.WakeLock mWakeLock;
    private boolean mHasOpCanceled = false;
    private ProgressDialog pd = null;
    private int mOpMaxValue = 0;
    private int mOpProgress = 0;
    private static boolean mContactInited = false;
    private ContactsInitChecker mContactsInitChecker;
    private int list_settings_title = R.string.firewall_blacklist_settings_title;
    private int list_settings_edit_dialog_title = R.string.firewall_blacklist_settings_edit_dialog_title;
    private int list_settings_add_dialog_title = R.string.firewall_blacklist_settings_add_dialog_title;
    private Uri mListUri;
    private Bundle mBundle = null;
    private int m_Action = ACTION_NONE;
    ArrayList<ContentValues> res = null;
    private String mContactId;
    public String[] mPhoneNumber;
    private int reject_calltype = 5;

    private Long mCurrentTimeMillisForTest;
    /** The projection to use when querying the phones table */
    static final String[] RECORDSPHONES_PROJECTION = new String[] {
            PhoneLookup._ID,
            PhoneLookup.DISPLAY_NAME,
    };
    private static final String[] CALL_LOG_PROJECTION = new String[] {
            CallLog.Calls._ID,
            CallLog.Calls.NUMBER,
            CallLog.Calls.DATE,
            CallLog.Calls.CACHED_NAME,
            CallLog.Calls.TYPE,
    };

    static final class ContactInfo {
        public String name;
        public String number;
        public int person_id;
        public static ContactInfo EMPTY = new ContactInfo();
    }

    public static final class FirewallListItemViews {
        CheckedTextView selectView;
        TextView line1View;
        TextView numberView;
        TextView dateView;
        String name = "";
        String number = "";
        long date;
        ImageButton ibListItemMenu;
        int position;
    }

    static final class FirewallInfoQuery {
        String number;
        long date;
        String name;
    }

    private final class QueryHandler extends AsyncQueryHandler {
        private final WeakReference<FirewallHistoryPage> mActivity;

        public QueryHandler(Context context) {
            super(context.getContentResolver());
            mActivity = new WeakReference<FirewallHistoryPage>(
                    (FirewallHistoryPage) context);
        }

        @Override
        protected void onQueryComplete(int token, Object cookie, Cursor cursor) {
            final FirewallHistoryPage activity = mActivity.get();
            if (activity != null && !activity.isFinishing()) {
                final FirewallHistoryPage.FirewallListAdapter callsAdapter = activity.mAdapter;
                callsAdapter.setLoading(false);
                callsAdapter.changeCursor(cursor);
            } else {
                cursor.close();
            }
            do {
                if (mBundle != null) {
                    // if not available count and is add number
                    String number = null;
                    String name = null;
                    int person_id = -1;
                    // other application call for insert
                    if (mBundle.containsKey("number")) {
                        number = mBundle.getString("number");
                    } else {
                        Message msg = Message.obtain();
                        msg.what = SHOW_TOAST;
                        msg.obj = getString(R.string.firewall_insert_no_number);
                        uihandler.sendMessage(msg);
                        break;
                    }

                    if (mBundle.containsKey("name")) {
                        name = mBundle.getString("name");
                    }

                    if (mBundle.containsKey("personid")) {
                        person_id = mBundle.getInt("personid", -1);
                    }
                }
            } while (false);
            mBundle = null;
            getListView().invalidateViews();
        }
    }

    /** Adapter class to fill in data for the Firewall listview */
    final class FirewallListAdapter extends ResourceCursorAdapter
            implements Runnable, ViewTreeObserver.OnPreDrawListener {
        HashMap<String, ContactInfo> mContactInfo;
        private final LinkedList<FirewallInfoQuery> mRequests;
        private volatile boolean mDone;
        private boolean mLoading = true;
        ViewTreeObserver.OnPreDrawListener mPreDrawListener;
        private static final int REDRAW = 1;
        private static final int START_THREAD = 2;
        private boolean mFirst;
        private Thread mFirewallItemThread;

        public boolean onPreDraw() {
            if (mFirst) {
                updateTitle();
                mHandler.sendEmptyMessageDelayed(START_THREAD, 1000);
                mFirst = false;
            }
            return true;
        }

        private Handler mHandler = new Handler() {
            @Override
            public void handleMessage(Message msg) {
                switch (msg.what) {
                    case REDRAW:
                        Log.d(TAG, "REDRAW");
                        notifyDataSetChanged();
                        break;
                    case START_THREAD:
                        startRequestProcessing();
                        break;
                }
            }
        };

        public FirewallListAdapter() {
            super(FirewallHistoryPage.this, R.layout.firewall_content_view_history, null);

            mContactInfo = new HashMap<String, ContactInfo>();
            mRequests = new LinkedList<FirewallInfoQuery>();
            mPreDrawListener = null;
        }

        void setLoading(boolean loading) {
            if (loading)
                updateTitle();
            mLoading = loading;
        }

        @Override
        public boolean isEmpty() {
            if (mLoading) {
                // We don't want the empty state to show when loading.
                return false;
            } else {
                return super.isEmpty();
            }
        }

        public void startRequestProcessing() {
            Log.d(TAG, " startRequestProcessing ");
            mDone = false;
            mFirewallItemThread = new Thread(this);
            mFirewallItemThread.setPriority(Thread.MIN_PRIORITY);
            mFirewallItemThread.start();
        }

        public void stopRequestProcessing() {
            Log.d(TAG, " stopRequestProcessing ");
            mDone = true;
            if (mFirewallItemThread != null)
                mFirewallItemThread.interrupt();
        }

        public void clearCache() {
            synchronized (mContactInfo) {
                mContactInfo.clear();
            }
        }

        private void enqueueRequest(String number, String name) {
            FirewallInfoQuery biq = new FirewallInfoQuery();
            biq.number = number;
            biq.name = name;
            synchronized (mRequests) {
                mRequests.add(biq);
                mRequests.notifyAll();
            }
        }

        // query in contacts,because the name will be changed or deleted
        private void queryContactInfo(FirewallInfoQuery biq) {
            // First check if there was a prior request for the same number
            // that was already satisfied
            Log.d(TAG, " queryContactInfo biq: " + biq.name + " " + biq.number);
            // the number should be unique
            ContactInfo info = mContactInfo.get(biq.number);
            if (info != null && info != ContactInfo.EMPTY) {
                synchronized (mRequests) {
                    if (mRequests.isEmpty()) {
                        mHandler.sendEmptyMessage(REDRAW);
                    }
                }
            } else {
                String number = FirewallProvider.bestNumMatch(biq.number, FirewallHistoryPage.this);
                if (null == number) {
                    Log.i(TAG, "contact number: " + number);
                    number = biq.number;
                }
                Log.i(TAG, "number: " + number);
                boolean full_match = false;

                if (!full_match) {
                    Cursor phonesCursor2 =
                            FirewallHistoryPage.this.getContentResolver().query(
                                    Uri.withAppendedPath(PhoneLookup.CONTENT_FILTER_URI,
                                            Uri.encode(number)),
                                    RECORDSPHONES_PROJECTION, null,
                                    null, /* CONTACT_ITEM_DEFAULT_SORT_ORDER */null);

                    if (phonesCursor2 != null) {
                        Log.i(TAG, "phonesCursor2 : " + phonesCursor2);
                        if (phonesCursor2.moveToFirst()) {
                            info = new ContactInfo();
                            info.name = phonesCursor2.getString(DISPLAY_NAME_COLUMN_INDEX);
                            info.number = biq.number;
                            int id = phonesCursor2.getInt(ID_COLUMN_INDEX);

                            Uri baseUri = ContentUris.withAppendedId(Contacts.CONTENT_URI, id);
                            Uri dataUri = Uri.withAppendedPath(baseUri,
                                    Contacts.Data.CONTENT_DIRECTORY);
                            Cursor c = getContentResolver().query(dataUri,
                                    new String[] {
                                        Phone.CONTACT_ID
                                    },
                                    null, null, null);

                            if (c != null && c.moveToFirst()) {
                                info.person_id = c.getInt(PERSON_ID_COLUMN_INDEX);
                            }
                            Log.d(TAG, "info.name : " + info.name);
                            Log.d(TAG, "info.number : " + info.number);
                            Log.d(TAG, "info.person_id : " + info.person_id);
                            mContactInfo.put(biq.number, info);
                            full_match = true;
                            // Inform list to update this item, if in view
                            synchronized (mRequests) {
                                if (mRequests.isEmpty()) {
                                    mHandler.sendEmptyMessage(REDRAW);
                                }
                            }
                        }
                        phonesCursor2.close();
                    }
                }
            }
            if (info != null) {
                // update database
                updateFirewallItem(biq, info);
            }
        }

        // update the database item
        private void updateFirewallItem(FirewallInfoQuery ciq, ContactInfo ci) {
            // Check if they are different. If not, don't update.
            if (TextUtils.equals(ciq.name, ci.name)) {
                return;
            }
            ContentValues values = new ContentValues(1);

            Log.d(TAG, "name : " + ci.name);
            Log.d(TAG, "number : " + ci.number);

            values.put("name", ci.name);
            FirewallHistoryPage.this.getContentResolver().update(
                    mListUri,
                    values, "number" + "='" + ciq.number + "'", null);
        }

        /*
         * Handles requests for contact name and number type
         * @see java.lang.Runnable#run()
         */
        public void run() {
            while (!mDone) {
                FirewallInfoQuery biq = null;
                synchronized (mRequests) {
                    if (!mRequests.isEmpty()) {
                        biq = mRequests.removeFirst();
                    } else {
                        try {
                            mRequests.wait(1000);
                        } catch (InterruptedException ie) {
                            // Ignore and continue processing requests
                        }
                    }
                }
                if (biq != null) {
                    queryContactInfo(biq);
                }
            }
        }

        @Override
        public View newView(Context context, Cursor cursor, ViewGroup parent) {
            View view = super.newView(context, cursor, parent);

            // Get the views to bind to
            FirewallListItemViews views = new FirewallListItemViews();
            views.line1View = (TextView) view.findViewById(R.id.line1);
            views.numberView = (TextView) view.findViewById(R.id.number);
            views.dateView = (TextView) view.findViewById(R.id.date);
            views.selectView = (CheckedTextView) view.findViewById(R.id.FirewallItemSelect);
            views.ibListItemMenu = (ImageButton) view.findViewById(R.id.list_item_menu);

            view.setTag(views);

            return view;
        }

        private long getCurrentTimeMillis() {
            if (mCurrentTimeMillisForTest == null) {
                return System.currentTimeMillis();
            } else {
                return mCurrentTimeMillisForTest;
            }
        }

        private FirewallListItemViews getFirewallListItemViews(View v) {
            if (null == v.getTag()) {
                return getFirewallListItemViews((View) v.getParent());
            }
            return (FirewallListItemViews) v.getTag();
        }

        private View.OnClickListener mListItemMenuClickListener = new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                mSelectedKey = getFirewallListItemViews(v).position;
                PopupMenu popup = new PopupMenu(FirewallHistoryPage.this, v);
                Menu menu = popup.getMenu();

                menu.add(0, DELETE_ITEM, 0, R.string.firewall_delete);

                popup.setOnMenuItemClickListener(new PopupMenu.OnMenuItemClickListener() {
                    public boolean onMenuItemClick(MenuItem item) {
                        FirewallHistoryPage.this.onContextItemSelected(item);
                        return true;
                    }
                });
                popup.show();
            }
        };

        @Override
        public void bindView(View view, Context context, Cursor c) {
            Log.d(TAG, "bindView");
            final FirewallListItemViews views = (FirewallListItemViews) view.getTag();
            views.ibListItemMenu.setOnClickListener(mListItemMenuClickListener);
            views.position = c.getPosition();
            String number = getNumberByCursor(c);
            long date = getDateByCursor(c);
            String name = getNameByCursor(c);

            // Lookup contacts with this number
            ContactInfo info = mContactInfo.get(number);
            if (info == null) {
                // Mark it as empty and queue up a request to find the name
                // The db request should happen on a non-UI thread
                info = ContactInfo.EMPTY;
                mContactInfo.put(number, info);
                if (mContactInited) {
                    enqueueRequest(number, name);
                }
            } else if (info != ContactInfo.EMPTY) {
                // Has been queried
                // Check if any data is different from the data cached in the
                // calls db. If so, queue the request so that we can update
                // the calls db.
                if (!TextUtils.equals(info.name, name)) {
                    // Something is amiss, so sync up.
                    enqueueRequest(number, name);
                }
            }

            Log.d(TAG, "bindView info.name: " + info.name);
            Log.d(TAG, "bindView info.number: " + info.number);

            CharSequence dateText =
                    DateUtils.getRelativeTimeSpanString(date,
                            getCurrentTimeMillis(),
                            DateUtils.MINUTE_IN_MILLIS,
                            DateUtils.FORMAT_ABBREV_RELATIVE);

            // Set the text lines
            if (null == name || name.length() == 0) {
                // fill for no name
                name = new String(getString(R.string.firewall_display_noname));
            }
            views.name = name;
            views.number = number;
            views.line1View.setText(name);
            views.numberView.setVisibility(View.VISIBLE);
            views.numberView.setText(number);
            views.dateView.setText(dateText);
            views.dateView.setVisibility(View.VISIBLE);

            if (true) {
                views.selectView.setVisibility(View.GONE);// hide the CheckBox
                views.selectView.setChecked(false);
            } else {
                views.selectView.setChecked(mListView.isItemChecked(c.getPosition()));
                // show the CheckBox
                views.selectView.setVisibility(View.VISIBLE);
            }

            // Listen for the first draw
            if (mPreDrawListener == null) {
                mFirst = true;
                mPreDrawListener = this;
                view.getViewTreeObserver().addOnPreDrawListener(this);
            }
        }
    }

    private String buildDeleteTips(int allCount, int sucessCount) {
        StringBuilder buffer = new StringBuilder();
        String toastStr = Integer.toString(sucessCount)
                + "/" + Integer.toString(allCount);
        toastStr = getString(R.string.firewall_delete_count, toastStr);
        buffer.append(toastStr);
        return buffer.toString();
    }

    private void disenableSleep(Context context) {
        if (mWakeLock == null) {
            PowerManager pm =
                    (PowerManager) context.getSystemService(Context.POWER_SERVICE);
            mWakeLock = pm.newWakeLock(PowerManager.PARTIAL_WAKE_LOCK,
                    "FirewallListOperating");
            mWakeLock.setReferenceCounted(false);
        }
        mWakeLock.acquire();
    }

    private String getNumberByCursor(Cursor c) {
        return c.getString(NUMBER_INDEX);
    }

    private long getDateByCursor(Cursor c) {
        return c.getLong(DATE_INDEX);
    }

    private int getIdByCursor(Cursor c) {
        return c.getInt(ID_INDEX);
    }

    private String getNameByCursor(Cursor c) {
        return c.getString(NAME_INDEX);
    }

    private int getFirewallListAvailableCount() {
        int hasCount = mAdapter.getCount();
        Log.d(TAG, "getFirewallListAvailableCount hasCount: " + hasCount);
        return (FIREWALL_LIST_MAX_ITEM_NUM - hasCount);
    }

    private Handler uihandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            switch (msg.what)
            {
                case OPERATE_ONE: {
                    Log.d(TAG, "uihandler OPERATE_ONE");
                    mOpProgress++;
                    if (pd != null)
                    {
                        pd.incrementProgressBy(1);
                    }
                    if (m_Action == ACTION_ADD_FR_CONTACTS) {
                        pd.setMessage(/* mNumfail + mInsertfail */"TEST");
                    }
                    if (mOpProgress >= mOpMaxValue)
                    {
                        if (pd != null)
                        {
                            // pd.dismiss();
                        }
                    }
                    else
                    {
                        return;
                    }
                }
                    break;
                case OPERATE_SUCCESS: {
                    Log.d(TAG, "uihandler OPERATE_SUCCESS");

                    if (pd != null)
                    {
                        pd.dismiss();
                    }

                    String toastStr = (String) msg.obj;
                    Toast.makeText(FirewallHistoryPage.this, toastStr,
                            Toast.LENGTH_LONG).show();
                    getListView().invalidateViews();
                }
                    break;
                case OPERATE_CANCEL: {
                    Log.d(TAG, "uihandler OPERATE_CANCEL");

                    if (pd != null)
                    {
                        pd.dismiss();
                    }
                    String toastStr = (String) msg.obj;
                    Toast.makeText(FirewallHistoryPage.this, toastStr,
                            Toast.LENGTH_LONG).show();
                }
                    break;
                case OPERATE_ERROR: {
                    Log.d(TAG, "uihandler OPERATE_ERROR");
                    if (pd != null)
                    {
                        pd.dismiss();
                    }

                    String toastStr = (String) msg.obj;
                    Toast.makeText(FirewallHistoryPage.this, toastStr,
                            Toast.LENGTH_LONG).show();
                }
                    break;
                case OPERATE_CLOSE: {
                    Log.d(TAG, "uihandler OPERATE_CLOSE");

                    if (pd != null)
                    {
                        pd.dismiss();
                    }

                }
                    break;
                case OPERATE_DEL_ALL_OVER: {
                    Log.d(TAG, "uihandler OPERATE_DEL_ALL_OVER");

                }
                    break;
                case OPERATE_DEL_SINGLE_OVER: {
                    Log.d(TAG, "uihandler OPERATE_DEL_SINGLE_OVER");
                    int result = msg.arg1;
                    if (result > 0)
                    {
                        Toast.makeText(FirewallHistoryPage.this, R.string.firewall_del_success,
                                Toast.LENGTH_LONG).show();
                    }
                    else
                    {
                        Toast.makeText(FirewallHistoryPage.this, R.string.firewall_del_failed,
                                Toast.LENGTH_LONG).show();
                    }
                    return;
                }
                case SHOW_TOAST: {
                    Log.d(TAG, "uihandler SHOW_TOAST");
                    String toastStr = (String) msg.obj;
                    Toast.makeText(FirewallHistoryPage.this, toastStr,
                            Toast.LENGTH_LONG).show();
                    return;
                }
                case UPDATE_INFO: {
                    Log.d(TAG, "uihandler UPDATE_INFO");
                    setTitle(getString(list_settings_title) +
                            "(" + (FIREWALL_LIST_MAX_ITEM_NUM - getFirewallListAvailableCount())
                            + ")");
                    return;
                }
                case UPDATE_VIEW: {
                    getListView().invalidateViews();
                }
            }
        }
    };

    @Override
    protected void onCreate(Bundle icicle) {
        super.onCreate(icicle);
        Log.d(TAG, "onCreate");

        setContentView(R.layout.firewall_list_view_layout);

        Intent intent = getIntent();
        Bundle extras = intent.getExtras();
        list_settings_title = R.string.firewall_blacklist_settings_title;
        list_settings_edit_dialog_title = R.string.firewall_blacklist_settings_edit_dialog_title;
        list_settings_add_dialog_title = R.string.firewall_blacklist_settings_add_dialog_title;

        mListUri = CallLog.Calls.CONTENT_URI;

        Log.d(TAG, "get intent: " + intent.toString());
        String action = intent.getAction();
        if (action != null && action.equals(Intent.ACTION_INSERT)) {
            /*
             * other app call insert Bundle bundle = new Bundle();
             * bundle.putString("name", "test");//optional
             * bundle.putString("number", "1234"); bundle.putInt("personid",
             * 20);//optional Intent intent = new Intent(xxx.this,
             * FirewallHistoryPage.class);
             * intent.setAction(Intent.ACTION_INSERT); intent.putExtras(bundle);
             * startActivity(intent);
             */
            mBundle = extras;
        }

        mAdapter = new FirewallListAdapter();
        mListView = getListView();
        mListView.setOnCreateContextMenuListener(this);
        setListAdapter(mAdapter);
        getListView().setTextFilterEnabled(true);
        mSelectednumber = null;
        mQueryHandler = new QueryHandler(this);
        mContactsInitChecker = new ContactsInitChecker(5000);
        new Thread(mContactsInitChecker, "Contact Init checker for firewall timer").start();
    }

    @Override
    protected void onResume() {
        super.onResume();
        Log.d(TAG, "onResume");
        if (mAdapter != null) {
            mAdapter.clearCache();
        }
        startQuery();
        // Let it restart the thread after next draw
        mAdapter.mPreDrawListener = null;
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        Log.d(TAG, "onDestroy");
        mAdapter.stopRequestProcessing();
        Cursor cursor = mAdapter.getCursor();
        if (cursor != null && !cursor.isClosed()) {
            cursor.close();
        }
    }

    @Override
    protected void onPause() {
        super.onPause();
        Log.d(TAG, "onPause");
        mAdapter.stopRequestProcessing();
    }

    // update activity title with info
    private void updateTitle() {
        Message msg = Message.obtain();
        msg.what = UPDATE_INFO;
        uihandler.sendMessage(msg);
    }

    // do query the database for whole list of firewall list now.
    private void startQuery() {

        StringBuilder where = new StringBuilder("type=");
        where.append(reject_calltype);

        mAdapter.setLoading(true);
        mQueryHandler.cancelOperation(QUERY_TOKEN);
        mQueryHandler.startQuery(QUERY_TOKEN, null,
                mListUri, CALL_LOG_PROJECTION, where.toString(), null,
                CallLog.Calls.DEFAULT_SORT_ORDER);
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        super.onCreateOptionsMenu(menu);

        menu.add(0, MENU_MULTI_DEL, 0,
                getResources().getString(R.string.firewall_multi_select));
        return true;
    }

    @Override
    public boolean onPrepareOptionsMenu(Menu menu) {
        super.onPrepareOptionsMenu(menu);
        // check box is not visible when user can do add, add from contacts,
        // multiselect actions
        menu.findItem(MENU_MULTI_DEL).setVisible(true);
        menu.findItem(MENU_MULTI_DEL).setEnabled(!mAdapter.isEmpty());
        // check box is visible when user can do select all and confirm delete,
        // re-select
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {

        switch (item.getItemId()) {
            case MENU_MULTI_DEL:
                m_Action = ACTION_MULTI_DELETE;
                showConfirmMultiDelDialog();
                // enterMultiSelectView();
                return true;
        }

        return super.onOptionsItemSelected(item);
    }

    // when long click the item, open this menu
    public void onCreateContextMenu(ContextMenu menu, View v, ContextMenuInfo menuInfo) {

        // if(isCheckBoxVisible) return;

        AdapterContextMenuInfo adapterMenuInfo = (AdapterContextMenuInfo) menuInfo;
        // set single selection info for delete or edit
        mSelectedKey = adapterMenuInfo.position;
        mSelectednumber = getNumberByCursor(((Cursor) mAdapter.getItem(mSelectedKey)));
        menu.setHeaderTitle(getTitle());
        menu.add(0, DELETE_ITEM, 0,
                R.string.user_dict_settings_context_menu_delete_title);
    }

    @Override
    public boolean onContextItemSelected(MenuItem item) {
        switch (item.getItemId()) {
            case CONTEXT_MENU_DELETE:
            case DELETE_ITEM:
                // delete the selected item(single)
                Log.d(TAG, "onCreateContextMenu and mSelectedKey is " + mSelectedKey);

                m_Action = ACTION_DELETE;
                showConfirmDelDialog();
                return true;
        }

        return false;
    }

    private void deleteNumberbyPos(int pos) {
        int key_id;
        key_id = getIdByCursor(((Cursor) mAdapter.getItem(pos)));
        Log.d(TAG, "deleteNumberbyPos: " + "id=" + pos + "key_id=" + key_id);
        int row = getContentResolver().delete(mListUri, DELETE_SELECTION,
                new String[] {
                    String.valueOf(key_id)
                });

        Message msg = Message.obtain();
        msg.what = SHOW_TOAST;
        if (row < 0) {
            // del failed
            msg.obj = getString(R.string.firewall_del_failed);
        } else {
            // del successfully
            msg.obj = getString(R.string.firewall_del_success);
        }
        uihandler.sendMessage(msg);

        mSelectedKey = -1;
        mSelectednumber = null;
        updateTitle();
    }

    // show confirm dialog(query the user) for multi select delete
    private void showConfirmDelDialog() {
        // boolean is_muti = false;
        OnClickListener y_l = null;
        if (m_Action == ACTION_DELETE) {
            if (mSelectedKey < 0) {
                return;
            }
            y_l = new DeleteFirewallListListener();
        }

        DeleteFirewallListCancelListener n_l = new DeleteFirewallListCancelListener();
        confirmDeleteDialog(y_l, n_l, false);
    }

    private void showConfirmMultiDelDialog() {
        // boolean is_muti = false;
        OnClickListener y_l = null;
        if (m_Action == ACTION_MULTI_DELETE) {
            y_l = new DeleteMultiFirewallListListener();
            // is_muti = true;
        }
        DeleteFirewallListCancelListener n_l = new DeleteFirewallListCancelListener();
        confirmMultiDeleteDialog(y_l, n_l, false);
    }

    // listener of the confirm multi-del dialog(ok button)
    private class DeleteMultiFirewallListListener implements OnClickListener {
        public void onClick(DialogInterface dialog, int whichButton)
        {
            int count = mAdapter.getCount();
            int row = -1;
            for (int i = 0; i < count; i++)
            {
                int key_id;
                key_id = getIdByCursor(((Cursor) mAdapter.getItem(i)));
                Log.d(TAG, "deleteNumberbyPos: " + "id=" + i + "key_id=" + key_id);
                row = getContentResolver().delete(mListUri, DELETE_SELECTION,
                        new String[] {
                            String.valueOf(key_id)
                        });
            }
            Message msg = Message.obtain();
            msg.what = SHOW_TOAST;
            if (row < 0) {
                // del failed
                msg.obj = getString(R.string.firewall_del_failed);
            } else {
                // del successfully
                msg.obj = getString(R.string.firewall_del_success);
            }
            uihandler.sendMessage(msg);

            mSelectedKey = -1;
            mSelectednumber = null;
            updateTitle();

        }
    }

    // listener of the confirm del dialog(ok button)
    private class DeleteFirewallListListener implements OnClickListener {
        public void onClick(DialogInterface dialog, int whichButton)
        {
            assert (mSelectedKey >= 0);
            deleteNumberbyPos(mSelectedKey);
        }
    }

    // listener of the confirm del dialog(ok button)
    private class DeleteFirewallListCancelListener implements OnClickListener {
        public void onClick(DialogInterface dialog, int whichButton)
        {
            mSelectedKey = -1;
            mSelectednumber = null;
            m_Action = ACTION_NONE;
        }
    }

    private void confirmDeleteDialog(OnClickListener y_listener, OnClickListener n_listener,
            boolean hasMany) {
        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        builder.setTitle(R.string.firewall_confirm_dialog_title);
        builder.setCancelable(false);
        if (hasMany)
        {
            builder.setMessage(R.string.firewall_confirm_delete_query);
        }
        else
        {
            builder.setMessage(R.string.firewall_confirm_delete_query);
        }
        builder.setPositiveButton(R.string.firewall_confirm_choose_yes, y_listener);
        builder.setNegativeButton(R.string.firewall_confirm_choose_no, n_listener);
        builder.show();
    }

    private void confirmMultiDeleteDialog(OnClickListener y_listener, OnClickListener n_listener,
            boolean hasMany) {
        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        builder.setTitle(R.string.firewall_confirm_dialog_title);
        builder.setCancelable(false);
        if (hasMany)
        {
            builder.setMessage(R.string.firewall_confirm_delete_all);
        }
        else
        {
            builder.setMessage(R.string.firewall_confirm_delete_all);
        }
        builder.setPositiveButton(R.string.firewall_confirm_choose_yes, y_listener);
        builder.setNegativeButton(R.string.firewall_confirm_choose_no, n_listener);
        builder.show();
    }

    private static final Uri FIREWALL_SWITCHER_CONTENT_URI = Uri
            .parse("content://com.android.firewall/setting/1");
    private static final Uri FIREWALL_MODE_CONTENT_URI = Uri
            .parse("content://com.android.firewall/setting/2");

    private class ContactsInitChecker implements Runnable {
        // private long mInterval;

        public ContactsInitChecker(long interval) {
            // mInterval = interval;
        }

        public void run() {
            /*
             * while(true){ try { Thread.sleep(mInterval); } catch
             * (InterruptedException e) { } Log.d(TAG,
             * "Contacts.UsimRecords.getUsimLoaderStatus(Contacts.DEVICE_TYPE_UIM): "
             * +
             * Contacts.UsimRecords.getUsimLoaderStatus(Contacts.DEVICE_TYPE_UIM
             * )); Log.d(TAG,
             * "Contacts.UsimRecords.getUsimLoaderStatus(Contacts.DEVICE_TYPE_SIM): "
             * +
             * Contacts.UsimRecords.getUsimLoaderStatus(Contacts.DEVICE_TYPE_SIM
             * ));
             * if(Contacts.UsimRecords.getUsimLoaderStatus(Contacts.DEVICE_TYPE_UIM
             * )!=0&&
             * Contacts.UsimRecords.getUsimLoaderStatus(Contacts.DEVICE_TYPE_SIM
             * )!=0){ mContactInited = true; break; } }
             */
            mContactInited = true;
        }
    }
}
