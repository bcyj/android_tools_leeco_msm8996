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
import android.content.pm.ActivityInfo;
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
import android.view.WindowManager;
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
import com.android.firewall.FirewallProvider;
import android.widget.ImageButton;
import android.widget.PopupMenu;
import android.provider.ContactsContract.Contacts;
import java.util.Iterator;
import java.util.Set;
import com.suntek.mway.rcs.client.api.support.RcsSupportApi;

public class FirewallListPage extends ListActivity
        implements View.OnCreateContextMenuListener {
    // ===========================================================
    // Constants
    // ===========================================================

    static final String TAG = "FirewallListPage";
    public static final int FIREWALL_LIST_MAX_ITEM_NUM = 100;
    public static final String CONTACT_ITEM_DEFAULT_SORT_ORDER = "person ASC";

    private static final int ID_INDEX = 0;
    private static final int NUMBER_INDEX = 1;
    private static final int PERSONID_INDEX = 2;
    private static final int NAME_INDEX = 3;

    private static final int CONTEXT_MENU_EDIT = Menu.FIRST;
    private static final int CONTEXT_MENU_DELETE = Menu.FIRST + 1;

    private static final int MENU_NEW = Menu.FIRST;
    private static final int MENU_NEW_FR_CONTACTS = MENU_NEW + 1;
    private static final int MENU_NEW_FR_CALL_LOG = MENU_NEW_FR_CONTACTS + 1;
    private static final int MENU_NEW_FR_MMS = MENU_NEW_FR_CALL_LOG + 1;
    private static final int MENU_MULTI_DEL = MENU_NEW_FR_MMS + 1;

    private static final Uri FIREWALL_SWITCHER_CONTENT_URI = Uri
            .parse("content://com.android.firewall/setting/1");
    private static final Uri FIREWALL_MODE_CONTENT_URI = Uri
            .parse("content://com.android.firewall/setting/2");

    private final static int DELETE_ITEM = 1;
    private final static int EDIT_ITEM = 0;
    private final static int VIEW_ITEM = 2;

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
    static final int ACTION_ADD_FR_CALL_LOG = 5;
    static final int ACTION_ADD_FR_MMS = 6;
    private static final int DIALOG_ADD_OR_EDIT = 0;
    private static final int DIALOG_SEL_NUM = 1;

    public static final int REQUEST_CONTACT_ITEM = 1;
    public static final int REQUEST_CONTACT_CALL_ITEM = 2;
    public static final int REQUEST_MMS_ITEM = 3;
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
    // for single select(edit or delete)--The number of the selected item
    private String mSelectednumber;
    // for single select(edit or delete)--The name of the selected item
    private String mSelectedname;
    FirewallListAdapter mAdapter;
    private QueryHandler mQueryHandler;
    private ListView mListView;
    private static Boolean isCheckBoxVisible;
    PowerManager.WakeLock mWakeLock;
    private boolean mHasOpCanceled = false;
    private ProgressDialog pd = null;
    private int mOpMaxValue = 0;
    private int mOpProgress = 0;
    private int list_settings_title = R.string.firewall_blacklist_settings_title;
    private int list_settings_edit_dialog_title = R.string.firewall_blacklist_settings_edit_dialog_title;
    private int list_settings_add_dialog_title = R.string.firewall_blacklist_settings_add_dialog_title;
    private Uri mListUri;
    private Bundle mBundle = null;
    private int m_Action = ACTION_NONE;
    ArrayList<ContentValues> res = null;
    private String mContactId;
    public String[] mPhoneNumber;
    /** The projection to use when querying the phones table */
    static final String[] RECORDSPHONES_PROJECTION = new String[] {
            PhoneLookup._ID,
            PhoneLookup.DISPLAY_NAME,
    };
    private RcsSupportApi mSupportApi = new RcsSupportApi();

    ProgressDialog mProgressDialog = null;
    Runnable mShowProgress = null;

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
        String name = "";
        String number = "";
        // ImageButton ibListItemMenu;
        int position;
    }

    static final class FirewallInfoQuery {
        String number;
        String name;
        int person_id;
    }

    private final class QueryHandler extends AsyncQueryHandler {
        private final WeakReference<FirewallListPage> mActivity;

        public QueryHandler(Context context) {
            super(context.getContentResolver());
            mActivity = new WeakReference<FirewallListPage>(
                    (FirewallListPage) context);
        }

        @Override
        protected void onQueryComplete(int token, Object cookie, Cursor cursor) {
            final FirewallListPage activity = mActivity.get();
            if (activity != null && !activity.isFinishing()) {
                final FirewallListPage.FirewallListAdapter callsAdapter = activity.mAdapter;
                callsAdapter.setLoading(false);
                callsAdapter.changeCursor(cursor);
            } else {
                cursor.close();
            }
            do {
                if (mBundle != null) {
                    // if not available count and is add number
                    if (getFirewallListAvailableCount() <= 0) {
                        Message msg = Message.obtain();
                        msg.what = SHOW_TOAST;
                        msg.obj = getString(R.string.firewall_reach_maximun);
                        uihandler.sendMessage(msg);
                        // return;
                    } else {
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
                        // make sure mSelectedKey is -1 for insert new
                        onAddOrEditFinished2(number, name, person_id);
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
            super(FirewallListPage.this, R.layout.firewall_content_view, null);

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

        private void enqueueRequest(String number, String name, int person_id) {
            FirewallInfoQuery biq = new FirewallInfoQuery();
            biq.number = number;
            biq.person_id = person_id;
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
                String number = FirewallProvider.bestNumMatch(biq.number, FirewallListPage.this);
                if (null == number) {
                    Log.i(TAG, "contact number: " + number);
                    number = biq.number;
                }
                Log.i(TAG, "number: " + number);
                boolean full_match = false;
                if (!full_match) {
                    Cursor phonesCursor2 =
                            FirewallListPage.this.getContentResolver().query(
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
                                c.close();
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
            values.put("name", ci.name);
            values.put("person_id", ci.person_id);
            // values.put("person_id", p_id);
            FirewallListPage.this.getContentResolver().update(
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
            views.line1View = (TextView) view.findViewById(R.id.list_line1);
            views.numberView = (TextView) view.findViewById(R.id.list_number);
            views.selectView = (CheckedTextView) view.findViewById(R.id.FirewallItemSelect);
            // views.ibListItemMenu = (ImageButton)
            // view.findViewById(R.id.list_item_menu);

            view.setTag(views);

            return view;
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
                mSelectednumber = getNumberByCursor(((Cursor) mAdapter.getItem(mSelectedKey)));
                mSelectedname = getNameByCursor(((Cursor) mAdapter.getItem(mSelectedKey)));
                PopupMenu popup = new PopupMenu(FirewallListPage.this, v);
                Menu menu = popup.getMenu();

                menu.add(0, EDIT_ITEM, 0, R.string.user_dict_settings_context_menu_edit_title);
                menu.add(0, DELETE_ITEM, 0, R.string.user_dict_settings_context_menu_delete_title);
                menu.add(0, VIEW_ITEM, 0, R.string.view_block_record);
                popup.setOnMenuItemClickListener(new PopupMenu.OnMenuItemClickListener() {
                    public boolean onMenuItemClick(MenuItem item) {
                        FirewallListPage.this.onContextItemSelected(item);
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
            view.setOnClickListener(mListItemMenuClickListener);
            views.position = c.getPosition();
            String number = getNumberByCursor(c);
            String name = getNameByCursor(c);
            int p_id = getPidByCursor(c);

            // If there's no name cached in our hashmap, but there's one in the
            // calls db, use the one in the calls db. Otherwise the name in our
            // hashmap is more recent, so it has precedence.

            // Lookup contacts with this number
            ContactInfo info = mContactInfo.get(number);
            if (info == null) {
                // Mark it as empty and queue up a request to find the name
                // The db request should happen on a non-UI thread
                info = ContactInfo.EMPTY;
                mContactInfo.put(number, info);
            } else if (info != ContactInfo.EMPTY) { // Has been queried
                // Check if any data is different from the data cached in the
                // calls db. If so, queue the request so that we can update
                // the calls db.
                if (!TextUtils.equals(info.name, name)) {
                    // Something is amiss, so sync up.
                    enqueueRequest(number, name, p_id);
                }
            }

            Log.d(TAG, "bindView info.name: " + info.name);
            Log.d(TAG, "bindView info.number: " + info.number);
            Log.d(TAG, "bindView info.person_id: " + info.person_id);

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

            if (true) {
                views.selectView.setVisibility(View.GONE);// hide the CheckBox
                views.selectView.setChecked(false);
            } else {
                views.selectView.setChecked(mListView.isItemChecked(c.getPosition()));
                views.selectView.setVisibility(View.VISIBLE);// show the
                                                             // CheckBox
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

    private void enalbeSleep() {
        if (mWakeLock != null) {
            mWakeLock.release();
        }
    }

    private int getIdByCursor(Cursor c) {
        return c.getInt(ID_INDEX);
    }

    private String getNumberByCursor(Cursor c) {
        return c.getString(NUMBER_INDEX);
    }

    private int getPidByCursor(Cursor c) {
        return c.getInt(PERSONID_INDEX);
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

                    // String toastStr = (String) msg.obj;
                    // Toast.makeText(FirewallListPage.this, toastStr,
                    // Toast.LENGTH_LONG).show();
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
                    Toast.makeText(FirewallListPage.this, toastStr,
                            Toast.LENGTH_SHORT).show();
                }
                    break;
                case OPERATE_ERROR: {
                    Log.d(TAG, "uihandler OPERATE_ERROR");
                    if (pd != null)
                    {
                        pd.dismiss();
                    }

                    String toastStr = (String) msg.obj;
                    Toast.makeText(FirewallListPage.this, toastStr,
                            Toast.LENGTH_SHORT).show();
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
                        Toast.makeText(FirewallListPage.this, R.string.firewall_del_success,
                                Toast.LENGTH_SHORT).show();
                    }
                    else
                    {
                        Toast.makeText(FirewallListPage.this, R.string.firewall_del_failed,
                                Toast.LENGTH_SHORT).show();
                    }
                    return;
                }
                case SHOW_TOAST: {
                    Log.d(TAG, "uihandler SHOW_TOAST");
                    String toastStr = (String) msg.obj;
                    Toast.makeText(FirewallListPage.this, toastStr,
                            Toast.LENGTH_SHORT).show();
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
        if (extras.getString("mode").equals("blacklist")) {
            list_settings_title = R.string.firewall_blacklist_settings_title;
            list_settings_edit_dialog_title = R.string.firewall_blacklist_settings_edit_dialog_title;
            list_settings_add_dialog_title = R.string.firewall_blacklist_settings_add_dialog_title;
            mListUri = FirewallProvider.Blacklist.CONTENT_URI;
        } else {
            list_settings_title = R.string.firewall_whitelist_settings_title;
            list_settings_edit_dialog_title = R.string.firewall_whitelist_settings_edit_dialog_title;
            list_settings_add_dialog_title = R.string.firewall_whitelist_settings_add_dialog_title;
            mListUri = FirewallProvider.Whitelist.CONTENT_URI;
        }
        Log.d(TAG, "get intent: " + intent.toString());
        String action = intent.getAction();
        if (action != null && action.equals(Intent.ACTION_INSERT)) {
            mBundle = extras;
        }
        isCheckBoxVisible = false;
        mAdapter = new FirewallListAdapter();
        mListView = getListView();
        mListView.setOnCreateContextMenuListener(this);
        // mListView.setOn(this);
        setListAdapter(mAdapter);
        getListView().setTextFilterEnabled(true);
        mSelectednumber = null;
        mSelectedname = null;
        mQueryHandler = new QueryHandler(this);

        mProgressDialog = new ProgressDialog(FirewallListPage.this);
        mProgressDialog.setProgressStyle(ProgressDialog.STYLE_SPINNER);
        mProgressDialog.setCancelable(false);
        mProgressDialog.setMessage(getString(R.string.please_wait));
        mProgressDialog.setTitle(R.string.adding_title);

        mShowProgress = new Runnable() {
            public void run() {
                mProgressDialog.show();
                WindowManager.LayoutParams lp = mProgressDialog.getWindow().getAttributes();
                lp.screenOrientation = ActivityInfo.SCREEN_ORIENTATION_NOSENSOR;
                mProgressDialog.getWindow().setAttributes(lp);
            }
        };
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

    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {
        switch (keyCode)
        {
            case KeyEvent.KEYCODE_BACK:
                if (isCheckBoxVisible)
                {
                    // exitMultiSelectView();
                    return true;
                }
                break;

        }
        return super.onKeyDown(keyCode, event);
    }

    // update activity title with info
    private void updateTitle() {
        Message msg = Message.obtain();
        msg.what = UPDATE_INFO;
        uihandler.sendMessage(msg);
    }

    // do query the database for whole list of firewall list now.
    private void startQuery() {
        mAdapter.setLoading(true);
        mQueryHandler.cancelOperation(QUERY_TOKEN);
        mQueryHandler.startQuery(QUERY_TOKEN, null,
                mListUri, new String[] {
                        "_id", "number", "person_id", "name"
                }, null, null,
                FirewallProvider.Blacklist.DEFAULT_SORT_ORDER);
    }

    @Override
    protected void onListItemClick(ListView l, View v, int position, long id) {
        if (!isCheckBoxVisible) {
            mSelectedKey = position;
            mSelectednumber = getNumberByCursor(((Cursor) mAdapter.getItem(mSelectedKey)));
            mSelectedname = getNameByCursor(((Cursor) mAdapter.getItem(mSelectedKey)));
            Dialog alertDialog = new AlertDialog.Builder(this)
                    .setTitle(mSelectednumber)
                    .setItems(R.array.item_click_menu, new DialogInterface.OnClickListener() {
                        @Override
                        public void onClick(DialogInterface dialog, int which) {
                            switch (which)
                            {
                                case DELETE_ITEM:
                                    // delete the selected item(single)
                                    m_Action = ACTION_DELETE;
                                    showConfirmDelDialog();
                                    break;
                                case EDIT_ITEM:
                                    // edit the selected item(single)
                                    m_Action = ACTION_EDIT;
                                    showAddOrEditDialog();
                                    break;
                                default:
                                    break;
                            }
                        }
                    })
                    .create();
            alertDialog.show();
        }
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        super.onCreateOptionsMenu(menu);

        menu.add(0, MENU_NEW, 0,
                getResources().getString(R.string.firewall_menu_new_item));

        menu.add(0, MENU_NEW_FR_CONTACTS, 0,
                getResources().getString(R.string.firewall_add_from_contacts));

        menu.add(0, MENU_NEW_FR_CALL_LOG, 0,
                getResources().getString(R.string.firewall_add_from_call_log));
        if (mSupportApi.isRcsSupported()) {
            menu.add(0, MENU_NEW_FR_MMS, 0, getResources()
                    .getString(R.string.firewall_add_from_mms));
        }
        menu.add(0, MENU_MULTI_DEL, 0,
                getResources().getString(R.string.firewall_multi_select));
        return true;
    }

    @Override
    public boolean onPrepareOptionsMenu(Menu menu) {
        super.onPrepareOptionsMenu(menu);
        // check box is not visible when user can do add, add from contacts,
        // multiselect actions
        menu.findItem(MENU_NEW).setVisible(!isCheckBoxVisible);
        menu.findItem(MENU_NEW_FR_CONTACTS).setVisible(!isCheckBoxVisible);
        menu.findItem(MENU_NEW_FR_CALL_LOG).setVisible(!isCheckBoxVisible);
        MenuItem mmsItem = menu.findItem(MENU_NEW_FR_MMS);
        if (mmsItem != null) {
            mmsItem.setVisible(!isCheckBoxVisible);
        }
        menu.findItem(MENU_MULTI_DEL).setVisible(!isCheckBoxVisible);
        menu.findItem(MENU_MULTI_DEL).setEnabled(!mAdapter.isEmpty());
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {

        switch (item.getItemId()) {
            case MENU_NEW:
                m_Action = ACTION_ADD;
                addNewFirewallItem();
                return true;

            case MENU_NEW_FR_CONTACTS:
                m_Action = ACTION_ADD_FR_CONTACTS;
                addFrContacts();
                return true;

            case MENU_NEW_FR_CALL_LOG:
                m_Action = ACTION_ADD_FR_CALL_LOG;
                addFrCallLog();
                return true;

            case MENU_NEW_FR_MMS:
                m_Action = ACTION_ADD_FR_MMS;
                addFrMms();
                return true;

            case MENU_MULTI_DEL:
                m_Action = ACTION_MULTI_DELETE;
                showConfirmMultiDelDialog();
                return true;
        }

        return super.onOptionsItemSelected(item);
    }

    // 1.add number
    private void addNewFirewallItem() {
        mSelectedKey = -1;
        mSelectednumber = null;
        mSelectedname = null;
        showAddOrEditDialog();
    }

    // 2.add from contacts
    private void addFrContacts() {
        mSelectedKey = -1;
        mSelectednumber = null;
        mSelectedname = null;
        selectFromContacts();
    }

    // 3.add from call log
    private void addFrCallLog() {
        mSelectedKey = -1;
        mSelectednumber = null;
        mSelectedname = null;
        selectFromCallLog();
    }

    // 4.add from mms log
    private void addFrMms() {
        mSelectedKey = -1;
        mSelectednumber = null;
        mSelectedname = null;
        selectFromMms();
    }

    // send intent to pick contact items
    private void selectFromContacts() {
        Intent mContactListIntent = new Intent("com.android.contacts.action.MULTI_PICK",
                Contacts.CONTENT_URI);
        startActivityForResult(mContactListIntent, REQUEST_CONTACT_ITEM);
    }

    private void selectFromCallLog() {
        Intent mContactListIntent = new Intent("com.android.contacts.action.MULTI_PICK_CALL");
        mContactListIntent.putExtra("selectcalllog", true);
        startActivityForResult(mContactListIntent, REQUEST_CONTACT_CALL_ITEM);
    }

    private void selectFromMms() {
        Intent mMmsListIntent = new Intent("com.android.rcs.mms.action.MULTI_PICK_MMS");
        startActivityForResult(mMmsListIntent, REQUEST_MMS_ITEM);
    }

    // help fuction for sending intent
    @Override
    public void startActivityForResult(Intent intent, int requestCode) {
        // requestCode >= 0 means the activity in question is a sub-activity.
        if (requestCode >= 0) {
            // mWaitingForSubActivity = true;
        }

        super.startActivityForResult(intent, requestCode);
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        Log.v(TAG, "onActivityResult: requestCode=" + requestCode
                + ", resultCode=" + resultCode + ", data=" + data);
        // mWaitingForSubActivity = false;

        if (resultCode != RESULT_OK) {
            // Make sure if there was an error that our message
            // type remains correct.
            // convertMessageIfNeeded(HAS_ATTACHMENT, hasAttachment());
            return;
        }

        switch (requestCode) {
            case REQUEST_CONTACT_ITEM:
            case REQUEST_CONTACT_CALL_ITEM:
            case REQUEST_MMS_ITEM:
                UiThread thread = new UiThread(data);
                thread.start();
                break;
            default:
                break;
        }
    }

    public class UiThread extends Thread {

        private Intent mData;

        public UiThread(Intent data) {

            mData = data;
        }

        @Override
        public void run() {
            // TODO Auto-generated method stub
            super.run();
            addNumberFromContacts(mData);
        }

    }

    // get data from contact select screen,and parse the data then add into
    // database
    public void addNumberFromContacts(Intent data) {

        Bundle b = data.getExtras();
        Bundle choiceSet = b.getBundle("result");
        if (choiceSet == null)
        {
            return;
        }
        uihandler.postDelayed(mShowProgress, 100);
        boolean bWillShow = true;
        Set<String> set = choiceSet.keySet();
        Iterator<String> iterator = set.iterator();
        while (iterator.hasNext()) {
            String[] contactInfo = choiceSet.getStringArray(iterator.next());
            if (null != contactInfo)
            {
                String mNumber = contactInfo[1];
                String mName = contactInfo[0];
                onAddOrEditFinished1(mNumber, mName);
                if (!iterator.hasNext())
                {
                    uihandler.removeCallbacks(mShowProgress);
                    uihandler.postDelayed(new Runnable() {
                        public void run() {
                            if (mProgressDialog != null && mProgressDialog.isShowing())
                            {
                                mProgressDialog.dismiss();
                            }
                        }
                    }, 101);
                    bWillShow = false;
                    break;
                }
            }
        }
        if (bWillShow)
        {
            uihandler.removeCallbacks(mShowProgress);
            uihandler.postDelayed(new Runnable() {
                public void run() {
                    if (mProgressDialog != null && mProgressDialog.isShowing())
                    {
                        mProgressDialog.dismiss();
                    }
                }
            }, 101);
        }
    }

    @Override
    public boolean onContextItemSelected(MenuItem item) {
        switch (item.getItemId()) {
            case VIEW_ITEM:
                startActivity(new Intent(this, BlockRecordTabActivity.class).putExtra(
                        BlockRecordTabActivity.NUMBER_KEY, mSelectednumber));
                break;

            case DELETE_ITEM:
                // delete the selected item(single)
                m_Action = ACTION_DELETE;
                showConfirmDelDialog();
                return true;

            case EDIT_ITEM:
                // edit the selected item(single)
                m_Action = ACTION_EDIT;
                showAddOrEditDialog();
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
        mSelectedname = null;
        updateTitle();
    }

    private void showAddOrEditDialog() {
        showDialog(DIALOG_ADD_OR_EDIT);
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

    // show confirm dialog(query the user) for multi select delete
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
            mSelectedname = null;
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
            mSelectedname = null;
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

    // show dialog for add number directly
    @Override
    protected Dialog onCreateDialog(int id) {
        switch (id) {
            case DIALOG_ADD_OR_EDIT: {
                View content = getLayoutInflater().inflate(R.layout.firewall_dialog_edittext, null);
                final EditText editText = (EditText) content.findViewById(R.id.edittext);
                editText.setInputType(InputType.TYPE_CLASS_NUMBER);
                editText.setFocusable(true);
                final EditText editName = (EditText) content.findViewById(R.id.editname);

                return new AlertDialog.Builder(this)
                        .setTitle(mSelectedKey >= 0
                                ? list_settings_edit_dialog_title
                                : list_settings_add_dialog_title)
                        .setView(content)
                        .setPositiveButton(android.R.string.ok,
                                new DialogInterface.OnClickListener() {
                                    public void onClick(DialogInterface dialog, int which) {
                                        m_Action = ACTION_EDIT;
                                        String name = editName.getText().toString();
                                        if (name == null)
                                            name = "";
                                        if (onAddOrEditFinished1(editText.getText().toString(),
                                                name))
                                        {
                                            Message msg = Message.obtain();
                                            msg.what = SHOW_TOAST;
                                            msg.obj = getString(R.string.firewall_save_success);
                                            uihandler.sendMessage(msg);
                                        }
                                    }
                                })
                        .setNegativeButton(android.R.string.cancel,
                                new DialogInterface.OnClickListener() {
                                    public void onClick(DialogInterface dialog, int which) {
                                        mSelectedKey = -1;
                                        mSelectednumber = null;
                                        mSelectedname = null;
                                    }
                                })
                        .create();
            }
            case DIALOG_SEL_NUM: {
                Cursor phoneNumber = getContentResolver().query(
                        ContactsContract.CommonDataKinds.Phone.CONTENT_URI, null,
                        ContactsContract.CommonDataKinds.Phone.CONTACT_ID + " = " + mContactId,
                        null, null);
                int count = phoneNumber.getCount();
                if (2 > count) {
                    phoneNumber.close();
                    return null;
                }
                else {
                    mPhoneNumber = new String[count];
                    int i = 0;
                    while (phoneNumber.moveToNext() && i < count) {
                        mPhoneNumber[i++] = phoneNumber.getString(phoneNumber
                                .getColumnIndex(ContactsContract.CommonDataKinds.Phone.NUMBER));
                    }
                    phoneNumber.close();
                    return new AlertDialog.Builder(this)
                            .setItems(mPhoneNumber, new DialogInterface.OnClickListener() {
                                public void onClick(DialogInterface dialog, int which) {
                                    onAddOrEditFinished1(mPhoneNumber[which].toString());
                                }
                            })
                            .create();
                }
            }
        }

        return null;
    }

    @Override
    protected void onPrepareDialog(int id, Dialog d) {
        switch (id) {
            case DIALOG_ADD_OR_EDIT: {
                AlertDialog dialog = (AlertDialog) d;
                d.setTitle(mSelectedKey >= 0
                        ? list_settings_edit_dialog_title
                        : list_settings_add_dialog_title);
                EditText editText = (EditText) dialog.findViewById(R.id.edittext);
                editText.setKeyListener(DialerKeyListener.getInstance());
                editText.setInputType(InputType.TYPE_CLASS_NUMBER);
                editText.setText(mSelectednumber);
                editText = (EditText) dialog.findViewById(R.id.editname);
                editText.setKeyListener(DialerKeyListener.getInstance());
                editText.setInputType(InputType.TYPE_CLASS_TEXT);
                editText.setText(mSelectedname);
            }
                break;
            case DIALOG_SEL_NUM:
                removeDialog(DIALOG_SEL_NUM);
                break;
        }
    }

    // helper function to add item to database(number only)
    public boolean onAddOrEditFinished1(String number) {
        return onAddOrEditFinished2(number, null, -1);
    }

    public boolean onAddOrEditFinished1(String number, String name) {
        return onAddOrEditFinished2(number, name, -1);
    }

    // helper function to add item to database(full info)
    private boolean onAddOrEditFinished2(String number, String name, int p_id) {
        boolean ret = true;
        ContentValues values = new ContentValues();
        String matchNumber = null;
        Cursor phonesCursor = null;
        int mPersonId = -1;
        String mName = null;
        Log.d(TAG, "onAddOrEditFinished number:" + number);
        Log.d(TAG, "onAddOrEditFinished name:" + name);
        Log.d(TAG, "onAddOrEditFinished p_id:" + p_id);
        number = number.replaceAll(" ", "");
        number = number.replaceAll("-", "");
        if (number.length() <= 0) {
            // number length is not allowed 0-
            Message msg = Message.obtain();
            msg.what = SHOW_TOAST;
            msg.obj = getString(R.string.firewall_number_len_not_valid);
            uihandler.sendMessage(msg);
            return false;
        }
        String compareNumber = number;
        int len = compareNumber.length();
        if (len > 11){
            compareNumber = number.substring(len - 11, len);
        }
        if(checkNumberInDB(compareNumber)){
            return false;
        }
        // if not available count and is add number
        if (getFirewallListAvailableCount() <= 0 && mSelectedKey < 0) {
            // can not be filled
            Message msg = Message.obtain();
            msg.what = SHOW_TOAST;
            msg.obj = getString(R.string.firewall_reach_maximun);
            uihandler.sendMessage(msg);
            return false;
        }

        values.put("number", number);
        if (null == name || p_id < 0) {
            // here should look for the number in contacts
            matchNumber = FirewallProvider.bestNumMatch(number, FirewallListPage.this);
            if (null == matchNumber) {
                matchNumber = number;
            }

            Log.d(TAG, "matchNumber:" + matchNumber);
            phonesCursor = FirewallListPage.this.getContentResolver().query(
                    Uri.withAppendedPath(PhoneLookup.CONTENT_FILTER_URI,
                            Uri.encode(matchNumber)), RECORDSPHONES_PROJECTION, null, null, null);

            if (null != phonesCursor && phonesCursor.getCount() > 0) {
                if (phonesCursor.moveToFirst()) {
                    int id = phonesCursor.getInt(ID_COLUMN_INDEX);
                    mName = phonesCursor.getString(DISPLAY_NAME_COLUMN_INDEX);
                    if (name != null)
                        name = mName;

                    Uri baseUri = ContentUris.withAppendedId(Contacts.CONTENT_URI, id);
                    Uri dataUri = Uri.withAppendedPath(baseUri, Contacts.Data.CONTENT_DIRECTORY);
                    Cursor c = getContentResolver().query(dataUri,
                            new String[] {
                                Phone.CONTACT_ID
                            },
                            null, null, null);

                    if (c != null) {
                        if (c.moveToFirst()) {
                            mPersonId = c.getInt(PERSON_ID_COLUMN_INDEX);
                            p_id = mPersonId;
                        }
                        c.close();
                    }
                } else {
                }
            }
            if (null != phonesCursor) {
                phonesCursor.close();
            }
        }
        values.put("person_id", p_id);
        values.put("name", name);

        if (mSelectedKey >= 0) {
            // edit
            int id = getIdByCursor(((Cursor) mAdapter.getItem(mSelectedKey)));
            try {
                getContentResolver().update(mListUri, values,
                        "_id=?", new String[] {
                            String.valueOf(id)
                        });
                if (number.length() == 0) {
                    getContentResolver().delete(mListUri, "_id=?",
                            new String[] {
                                String.valueOf(id)
                            });
                }
            } catch (SQLiteConstraintException ex) {
                Log.d(TAG, "write database error" + ex);
                // if (19 == ex.getErrorCode()) {
                ret = DBErrorHandle(ACTION_EDIT);
                // }
            }
        }
        else {
            // add new
            Uri mUri = getContentResolver().insert(mListUri, values);
            if (mUri == null) {
                ret = DBErrorHandle(ACTION_ADD);
            }
        }

        mSelectedKey = -1;
        mSelectednumber = null;
        mSelectedname = null;
        updateTitle();
        return ret;
    }

    private boolean checkNumberInDB(String number){

         Cursor cu = null;
        try{
            Uri otherUri = (mListUri == FirewallProvider.Whitelist.CONTENT_URI)
                    ? FirewallProvider.Blacklist.CONTENT_URI
                    : FirewallProvider.Whitelist.CONTENT_URI;
             cu = getContentResolver().query(otherUri,
                     new String[] {
                             "_id", "number", "person_id", "name"
                     },
                     "number" + " LIKE '%" + number + "'",
                     null,
                     null);
             if (cu != null) {
                 if (cu.getCount() > 0) {
                     cu.close();
                     cu = null;
                     Message msg = Message.obtain();
                     msg.what = SHOW_TOAST;
                     msg.obj = getString((mListUri == FirewallProvider.Whitelist.CONTENT_URI)
                            ? R.string.firewall_number_in_black
                             : R.string.firewall_number_in_white);
                     uihandler.sendMessage(msg);
                     return true;
                }
             }
        } finally {
            if(cu != null){
                 cu.close();
                 cu = null;
            }
        }
        Cursor cu2 = null;
       try{
            cu2 = getContentResolver().query(mListUri,
                    new String[] {
                            "_id", "number", "person_id", "name"
                    },
                    "number" + " LIKE '%" + number + "'",
                    null,
                    null);
            if (cu2 != null){
                if (cu2.getCount() > 0) {
                    cu2.close();
                    cu2 = null;
                    Message msg = Message.obtain();
                    msg.what = SHOW_TOAST;
                    msg.obj = getString((mListUri == FirewallProvider.Whitelist.CONTENT_URI)
                           ? R.string.firewall_number_in_white
                            : R.string.firewall_number_in_black);
                    uihandler.sendMessage(msg);
                    return true;
               }
            }
       } finally {
           if(cu2 != null){
               cu2.close();
               cu2 = null;
           }
       }
       return false;
}

    private boolean isFirewallEnabled() {
        boolean isFirewallEnabled = true;
        if (getContentResolver() == null)
        {
            return false;
        }
        Cursor c = getContentResolver().query(FIREWALL_SWITCHER_CONTENT_URI,
                new String[] {
                    "value"
                },
                "_id = ?",
                new String[] {
                    "1"
                },
                null);
        if (c != null) {
            if (c.moveToFirst()) {
                isFirewallEnabled = c.getString(0).equals("1");
            }
            c.close();
        }
        return isFirewallEnabled;
    }

    private String getFirewallMode() {
        String firewallMode = "";
        if (getContentResolver() == null)
        {
            return "";
        }
        Cursor c = getContentResolver().query(FIREWALL_MODE_CONTENT_URI,
                new String[] {
                    "value"
                },
                "_id = ?",
                new String[] {
                    "2"
                },
                null);
        if (c != null)
        {
            if (c.moveToFirst()) {
                firewallMode = c.getString(0);
            }
            c.close();
        }
        return firewallMode;
    }

    private boolean DBErrorHandle(int eCode) {
        Log.d(TAG, "DBErrorHandle" + eCode);
        if (eCode == ACTION_ADD || eCode == ACTION_EDIT) {
            // error,don't know the error code.In firewall app,insert error code
            // is always 19
            Message msg = Message.obtain();
            msg.what = SHOW_TOAST;
            msg.obj = getString(R.string.firewall_insert_collision);
            uihandler.sendMessage(msg);
        }
        return false;
    }

}
