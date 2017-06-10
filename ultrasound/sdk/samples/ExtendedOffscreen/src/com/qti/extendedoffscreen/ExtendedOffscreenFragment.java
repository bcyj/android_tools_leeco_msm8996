/*===========================================================================
                           ExtendedOffscreenFragment.java

Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
==============================================================================*/

package com.qti.extendedoffscreen;

import com.qti.snapdragon.sdk.digitalpen.DigitalPenManager;
import com.qti.snapdragon.sdk.digitalpen.DigitalPenManager.OffScreenMode;
import com.qti.snapdragon.sdk.digitalpen.DigitalPenManager.Settings;

import android.app.Fragment;
import android.content.DialogInterface;
import android.content.DialogInterface.OnDismissListener;
import android.graphics.Bitmap;
import android.graphics.drawable.ColorDrawable;
import android.graphics.Color;
import android.os.Bundle;
import android.os.Handler;
import android.view.Display;
import android.view.LayoutInflater;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.widget.CompoundButton;
import android.widget.ImageView;
import android.widget.Toast;
import android.widget.ToggleButton;

public class ExtendedOffscreenFragment extends Fragment implements OnClickListener {

    static final String TAG = "ExtendedOffscreenFragment";

    protected static final long PREVIEW_UPDATE_PERIOD_MS = 100;

    private ImageView preview;

    private OffscreenPresentation offscreenPresentation;

    private boolean isPreviewEnabled;

    protected Bitmap offscreenCache;

    private final Runnable previewTask = new Runnable() {

        @Override
        public void run() {
            if (isPreviewEnabled && preview != null) {
                View topView = offscreenPresentation
                        .findViewById(android.R.id.content);
                if (!topView.isDrawingCacheEnabled()) {
                    topView.setDrawingCacheEnabled(true);
                }
                topView.buildDrawingCache();
                offscreenCache = topView.getDrawingCache();
                if (offscreenCache != null) {
                    preview.setImageBitmap(offscreenCache.copy(offscreenCache.getConfig(), false));
                    topView.destroyDrawingCache();
                }
            }
            handler.postDelayed(this, PREVIEW_UPDATE_PERIOD_MS);
        }

    };

    private Handler handler;

    private DigitalPenManager digitalPenManager;

    private CompoundButton enableBtn;

    public ExtendedOffscreenFragment() {
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        View topView = inflater.inflate(R.layout.fragment_extended_offscreen, container, false);
        enableBtn = (CompoundButton) topView.findViewById(R.id.toggleButtonOffscreenEnable);
        enableBtn.setOnClickListener(this);
        preview = (ImageView) topView.findViewById(R.id.imageViewOffscreenPreview);
        clearPreview();
        digitalPenManager = new DigitalPenManager(getActivity().getApplication());
        enableBtn.setEnabled(true);
        return topView;
    }

    private void onClickOffscreenEnable(View v) {
        if (digitalPenManager == null) {
            return;
        }
        ToggleButton btn = (ToggleButton) v;
        isPreviewEnabled = btn.isChecked();
        Settings settings = digitalPenManager.getSettings();
        if (isPreviewEnabled) {
            createOffscreenPresentation();
            settings.setOffScreenMode(OffScreenMode.EXTEND);
        } else {
            settings.setOffScreenMode(OffScreenMode.DISABLED);
            clearPreview();
        }
        if (!settings.apply()) {
            Toast.makeText(getActivity(), "Problem applying digital pen settings",
                    Toast.LENGTH_LONG);
        }
    }

    @Override
    public void onResume() {
        super.onResume();

        // restart presentation if enabled
        onClickOffscreenEnable(getActivity().findViewById(R.id.toggleButtonOffscreenEnable));

        // restart preview task
        handler = new Handler();
        previewTask.run();
    }

    @Override
    public void onPause() {
        handler.removeCallbacks(previewTask);
        super.onPause();
    }

    @Override
    public void onDestroy() {
        dismissOffscreenPresentation();
        super.onDestroy();
    }

    private void clearPreview() {
        preview.setImageDrawable(new ColorDrawable(Color.GRAY));
    }

    private void createOffscreenPresentation() {

        Display offscreenDisplay = digitalPenManager.getOffScreenDisplay();

        offscreenPresentation = new OffscreenPresentation(getActivity(), offscreenDisplay);
        offscreenPresentation.setOnDismissListener(new OnDismissListener() {

            @Override
            public void onDismiss(DialogInterface dialog) {
                isPreviewEnabled = false;
            }
        });
        offscreenPresentation.show();
    }

    private void dismissOffscreenPresentation() {
        if (offscreenPresentation != null) {
            offscreenPresentation.dismiss();
        }
    }

    @Override
    public void onClick(View arg0) {
        switch (arg0.getId()) {
            case R.id.toggleButtonOffscreenEnable:
                onClickOffscreenEnable(arg0);
                break;
        }
    }

}
