/*
 * Copyright (c) 2014 pci-suntektech Technologies, Inc.  All Rights Reserved.
 * pci-suntektech Technologies Proprietary and Confidential.
 */

package com.suntek.mway.rcs.nativeui.ui;

import java.lang.ref.WeakReference;
import android.app.ListActivity;
import android.content.AsyncQueryHandler;
import android.content.ContentResolver;
import android.content.Context;
import android.content.Intent;
import android.database.Cursor;
import android.net.Uri;
import android.os.Bundle;
import android.provider.ContactsContract;
import android.provider.ContactsContract.PhoneLookup;
import android.widget.CursorAdapter;
import android.text.TextUtils;
import android.view.LayoutInflater;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.ListView;
import android.widget.TextView;
import android.widget.Toast;
import com.suntek.mway.rcs.nativeui.R;
public class MutiMmsSelectList extends ListActivity implements OnClickListener{

    static final String TAG = "MutiMmsSelectList";
    private Bundle mChoiceSet;
    private MmsItemListAdapter mAdapter;
    private QueryHandler mQueryHandler;
    private Context mContext;
    private Button mOKButton;
    private Button mCancelButton;
    private CheckBox mSelectAllCheckBox;
    private static final Uri MMSSMS_CANONICAL_ADDRESSES = Uri.parse("content://mms-sms/canonical-addresses");
    private static final Uri MMSSMS_FULL_CONVERSATION_URI = Uri.parse("content://mms-sms/conversations");
    private static final Uri CONVERSATION_URI =  MMSSMS_FULL_CONVERSATION_URI.buildUpon().appendQueryParameter("simple", "true").build();
    private static final String[] ALL_THREADS_PROJECTION = {
        "_id","date","message_count","recipient_ids","snippet","snippet_cs","read","archived","type","error",
        "has_attachment","top","top_time","is_group_chat"
    };
    static final String[] RECORDSPHONES_PROJECTION = new String[] {
        PhoneLookup._ID,
        PhoneLookup.DISPLAY_NAME,
};

    private static final int RECIPIENT_IDS  = 3;
    private static final int SNIPPET  = 4;

    private static final int THREAD_LIST_QUERY_TOKEN = 11;
    private static final int MMSSMS_CANONICAL_NUMBER  = 1;
    private static final int MAX_COUNT_SELECT_IN_MMS  = 10;

    @Override
    protected void onCreate(Bundle icicle){
        super.onCreate(icicle);
        setContentView(R.layout.select_mms_list);
        mChoiceSet = new Bundle();
        mAdapter = new MmsItemListAdapter(this);
        getListView().setAdapter(mAdapter);
        mQueryHandler = new QueryHandler(this);
        mContext = getApplicationContext();
        initLayoutResource();
        startQuery();
    }

    @Override
    public void onDestroy() {
        mQueryHandler.removeCallbacksAndMessages(THREAD_LIST_QUERY_TOKEN);
        if (mAdapter.getCursor() != null) {
            mAdapter.getCursor().close();
        }
        super.onDestroy();
    }

    private void initLayoutResource() {
        mOKButton = (Button) findViewById(R.id.btn_ok);
        mOKButton.setOnClickListener(this);
        mOKButton.setText(getOKString());
        mCancelButton = (Button) findViewById(R.id.btn_cancel);
        mCancelButton.setOnClickListener(this);
        mSelectAllCheckBox = (CheckBox) findViewById(R.id.select_all_checkbox);
        mSelectAllCheckBox.setOnClickListener(this);
    }

    public void startQuery() {
      mQueryHandler.startQuery(THREAD_LIST_QUERY_TOKEN, null, CONVERSATION_URI,
              ALL_THREADS_PROJECTION, "is_group_chat = " + 0, null, null);
    }

    private final class MmsItemCache {
        String recipient_ids;
        String name;
        String number;
        String snippet;
    }
    private final class MmsItemListAdapter extends CursorAdapter{
        private LayoutInflater mFactory;
        Cursor mCursor = null;
    public MmsItemListAdapter(Context context) {
            super(context, null);
            // TODO Auto-generated constructor stub
            mFactory = LayoutInflater.from(context);
        }

    @Override
    public View getView(int position, View convertView, ViewGroup parent) {
        View v;
        if (!mCursor.moveToPosition(position)) {
            throw new IllegalStateException(
                    "couldn't move cursor to position " + position);
        }
        if (convertView != null && convertView.getTag() != null) {
            v = convertView;
        } else {
            v = newView(mContext, mCursor, parent);
        }
        bindView(v,mContext,mCursor);
        return v;
    }

    @Override
    public void bindView(View view, Context context, Cursor cursor) {
        // TODO Auto-generated method stub
        MmsItemCache cache = (MmsItemCache) view.getTag();
        String number = null;
        String name = null;
        StringBuffer s1=new StringBuffer();
        cache.recipient_ids = cursor.getString(RECIPIENT_IDS);
        cache.snippet = cursor.getString(SNIPPET);
        Cursor numberCursor = null;
        try{
             numberCursor = mContext.getContentResolver().query(MMSSMS_CANONICAL_ADDRESSES, null,
                 "_id = " + cache.recipient_ids, null, null);
             if(numberCursor != null && numberCursor.moveToNext()){
                number = numberCursor.getString(MMSSMS_CANONICAL_NUMBER);
                name = getContactNameByNumber(number);
             }
        } finally {
            if (numberCursor != null) {
                numberCursor.close();
            }
        }
         TextView numberText = (TextView) view.findViewById(R.id.pick_mms_number);
         //numberText.setText(number);
         TextView snippetText = (TextView) view.findViewById(R.id.pick_mms_snippet);
         snippetText.setText(cache.snippet);
         if(null != name){
             s1.append(number).append("(").append(name).append(")");
             numberText.setText(s1);
         } else {
            s1.append(number);
             numberText.setText(s1);
         }
         CheckBox checkBox = (CheckBox) view.findViewById(R.id.pick_mms_check);
         checkBox.setChecked(false);
    }

    @Override
    public View newView(Context context, Cursor cursor, ViewGroup parent) {
        // TODO Auto-generated method stub
        View v = null;
            v = mFactory.inflate(R.layout.pick_mms_item, parent, false);
            MmsItemCache cache = new MmsItemCache();
            v.setTag(cache);
        return v;
    }
    @Override
    public void changeCursor(Cursor cursor) {
        mCursor = cursor;
        super.changeCursor(cursor);
    }
}

    private class QueryHandler extends AsyncQueryHandler {
        protected WeakReference<MutiMmsSelectList> mActivity;

        public QueryHandler(Context context) {
            super(context.getContentResolver());
        }

        @Override
        protected void onQueryComplete(int token, Object cookie, Cursor cursor) {
            // In the case of low memory, the WeakReference object may be
            // recycled.
            if (mActivity == null || mActivity.get() == null) {
                mActivity = new WeakReference<MutiMmsSelectList>(
                      MutiMmsSelectList.this);
            }
            final MutiMmsSelectList activity = mActivity.get();
            activity.mAdapter.changeCursor(cursor);
            if (cursor == null || cursor.getCount() == 0) {
                Toast.makeText(mContext, mContext.getString(R.string.no_mms),
                        Toast.LENGTH_SHORT).show();
            }
        }
    }

    @Override
    public void onClick(View view) {
        // TODO Auto-generated method stub
        int id = view.getId();
        switch(id){
        case R.id.btn_ok:{
            Intent intent = new Intent();
            Bundle bundle = new Bundle();
            bundle.putBundle("result", mChoiceSet);
            intent.putExtras(bundle);
            this.setResult(RESULT_OK, intent);
            finish();
        }
        break;
        case R.id.btn_cancel:{
            this.setResult(this.RESULT_CANCELED);
            finish();
        }
        break;
        case R.id.select_all_checkbox:{
            if (mSelectAllCheckBox.isChecked()) {
                selectAll(true);
            } else {
                selectAll(false);
            }
        }
        break;
        default:
            break;
        }

    }

    private void selectAll(boolean isSelected) {

        Cursor cursor = mAdapter.getCursor();
        if (cursor == null) {
            //log("cursor is null.");
            return;
        }

        cursor.moveToPosition(-1);
        while (cursor.moveToNext()) {
            String id = null;
            String[] value = null;
            id  = cursor.getString(RECIPIENT_IDS);
            Cursor numberCursor = null;
            try{
                numberCursor = mContext.getContentResolver().query(MMSSMS_CANONICAL_ADDRESSES, null,
                       "_id = " + id, null, null);
                if(numberCursor != null && numberCursor.moveToNext()){
                    value = new String[]{
                     null,
                    numberCursor.getString(MMSSMS_CANONICAL_NUMBER)
                   };
                }
            } finally {
                if (numberCursor != null) {
                    numberCursor.close();
                }
            }
            if (isSelected) {
                mChoiceSet.putStringArray(id, value);
            } else {
                mChoiceSet.remove(id);
            }
        }

        // update UI items.
        mOKButton.setText(getOKString());
        ListView mList = getListView();
        int count = mList.getChildCount();
        for (int i = 0; i < count; i++) {
            View v = mList.getChildAt(i);
            CheckBox checkBox = (CheckBox) v.findViewById(R.id.pick_mms_check);
            checkBox.setChecked(isSelected);
        }
    }

    private String getOKString() {
        if (0 == mChoiceSet.size()) {
            mOKButton.setEnabled(false);
        } else {
            mOKButton.setEnabled(true);
        }

        return "OK" + "(" + mChoiceSet.size() + ")";
    }

    @Override
    protected void onListItemClick(ListView l, View v, int position, long id) {
        CheckBox checkBox = (CheckBox) v.findViewById(R.id.pick_mms_check);
        boolean isChecked = !checkBox.isChecked();
        checkBox.setChecked(isChecked);
        if (isChecked) {
            String[] value = null;
            MmsItemCache cache = (MmsItemCache) v.getTag();
            Cursor numberCursor = mContext.getContentResolver().query(MMSSMS_CANONICAL_ADDRESSES, null,
                  "_id = " + cache.recipient_ids, null, null);
           if(numberCursor != null && numberCursor.moveToNext()){
              value = new String[]{
                    null,
                  numberCursor.getString(MMSSMS_CANONICAL_NUMBER)
              };
           }
            mChoiceSet.putStringArray(String.valueOf(id), value);
        } else {
            mChoiceSet.remove(String.valueOf(id));
            mSelectAllCheckBox.setChecked(false);
        }
        mOKButton.setText(getOKString());
    }

    private String getContactNameByNumber(String number) {
        if (TextUtils.isEmpty(number)) {
            return null;
        }

        final ContentResolver resolver = mContext.getContentResolver();

        Uri lookupUri = null;
        String[] projection = new String[]
                { PhoneLookup._ID, PhoneLookup.DISPLAY_NAME };
        Cursor cursor = null;
        try {
            lookupUri = Uri.withAppendedPath(PhoneLookup.CONTENT_FILTER_URI, Uri.encode(number));
             cursor =resolver.query(lookupUri, projection, null, null, null);
        } catch (Exception ex) {
            try {
                lookupUri = Uri.withAppendedPath(android.provider.Contacts.Phones.CONTENT_FILTER_URL,
                Uri.encode(number));
                cursor = resolver.query(lookupUri, projection, null, null, null);
            } catch (Exception e) {
            }
        }
        String name = null;
        if (cursor != null && cursor.getCount() > 0 && cursor.moveToFirst()) {
        name = cursor.getString(cursor.getColumnIndex(ContactsContract.CommonDataKinds.Phone.DISPLAY_NAME));
        }
        cursor.close();
        return name;
    }
}
