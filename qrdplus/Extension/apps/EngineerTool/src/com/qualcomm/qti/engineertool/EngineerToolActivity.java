/**
 * Copyright (c) 2015, Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.qti.engineertool;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.content.Loader;
import android.content.Loader.OnLoadCompleteListener;
import android.os.Bundle;
import android.os.Parcelable;
import android.util.Log;
import android.view.MenuItem;

import com.qualcomm.qti.engineertool.Utils.WaitingDialog;

public class EngineerToolActivity extends Activity implements OnLoadCompleteListener<Parcelable> {
    private static final String TAG = "EngineerToolActivity";

    public static final String EXTRA_KEY_CONTENT_VIEW = "content_view";

    private WaitingDialog mDialog = null;
    private ContentViewLoader mLoader = null;

    private boolean mInLoading = false;

    public static Intent buildIntentWithContent(Context packageContext, Parcelable contentView) {
        Intent intent = new Intent(packageContext, EngineerToolActivity.class);
        intent.putExtra(EXTRA_KEY_CONTENT_VIEW, contentView);
        return intent;
    }

    @Override
    protected void onCreate(Bundle bundle) {
        super.onCreate(bundle);

        setContentView(R.layout.main_activity);

        if (bundle == null) {
            Intent intent = getIntent();
            Parcelable contentView = intent.getParcelableExtra(EXTRA_KEY_CONTENT_VIEW);
            if (contentView == null) {
                mLoader = new ContentViewLoader(this);
                mLoader.registerListener(0, this);
                mLoader.startLoading();
                mInLoading = true;
                mDialog = WaitingDialog.newInstance(null, R.string.loading_content,
                        WaitingDialog.DISMISS_BY_USER);
            } else {
                // Already provider the contentView as Parcelable, we could build the view
                // from this Parcelable.
                getFragmentManager().beginTransaction()
                        .add(R.id.fragment, EngineerToolFragment.newInstance(contentView))
                        .commit();
            }
        }
    }

    @Override
    protected void onResume() {
        super.onResume();
        if (mDialog != null && mInLoading) {
            mDialog.show(getFragmentManager(), WaitingDialog.TAG_LABEL);
        }
    }

    @Override
    public void onLoadComplete(Loader<Parcelable> loader, Parcelable data) {
        mInLoading = false;
        // Dismiss the dialog.
        if (mDialog != null) {
            mDialog.dismiss();
        }

        if (data == null) {
            Log.e(TAG, "Can not load the content view. Please check xml.");
            return;
        }

        // Load UI from the data.
        getFragmentManager().beginTransaction()
                .add(R.id.fragment, EngineerToolFragment.newInstance(data))
                .commit();
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        if (item.getItemId() == R.id.load) {
            Log.e("yingying", "onOptionsItemSelected");
            return true;
        }
        return super.onOptionsItemSelected(item);
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        // TODO Auto-generated method stub
        super.onActivityResult(requestCode, resultCode, data);
    }

}
