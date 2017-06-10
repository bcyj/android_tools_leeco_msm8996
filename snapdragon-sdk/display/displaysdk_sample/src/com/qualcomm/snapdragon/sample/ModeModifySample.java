/* ====================================================================
 * Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * QTI Proprietary and Confidential.
 * =====================================================================
 * @file SviSample.java
 *
 */
package com.qualcomm.snapdragon.sample;

import android.app.ActionBar;
import android.app.ActionBar.Tab;
import android.app.Activity;
import android.app.AlertDialog;
import android.app.Fragment;
import android.app.FragmentTransaction;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.database.Cursor;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.net.Uri;
import android.os.Bundle;
import android.provider.MediaStore;
import android.text.InputFilter;
import android.util.Log;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.widget.EditText;
import android.widget.Toast;

import com.qti.snapdragon.sdk.display.ColorManager;
import com.qti.snapdragon.sdk.display.ColorManager.ColorManagerListener;
import com.qti.snapdragon.sdk.display.ColorManager.DCM_DISPLAY_TYPE;

public class ModeModifySample extends Activity {
    private static String TAG = "DisplaySDK-ModeModify";

    // Code for our image picker select action.
    private static final int IMAGE_PICKER_SELECT = 999;

    private String modeName = null;
    private int modeID = -1;
    private ColorManager cMgr;

    // Fragments for each display SDK feature
    CbFragment cbFragment = null;
    SviFragment sviFragment = null;
    PaFragment paFragment = null;
    McFoliageFragment mcFoliageFragment = null;
    McSkinFragment mcSkinFragment = null;
    McSkyFragment mcSkyFragment = null;
    AbFragment abFragment = null;

    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        setContentView(R.layout.modemodifysample);
        setTitle(R.string.display_tuning_title);

        ActionBar.Tab cbTab, sviTab, paTab, mcFoliageTab, mcSkinTab, mcSkyTab, abTab;
        ActionBar actionBar = getActionBar();
        actionBar.setNavigationMode(ActionBar.NAVIGATION_MODE_TABS);

        cbFragment = new CbFragment();
        cbTab = actionBar.newTab().setText(R.string.cb_mode);
        cbTab.setTabListener(new displayTabListener(cbFragment));
        actionBar.addTab(cbTab);

        sviFragment = new SviFragment();
        sviTab = actionBar.newTab().setText(R.string.svi_mode);
        sviTab.setTabListener(new displayTabListener(sviFragment));
        actionBar.addTab(sviTab);

        paFragment = new PaFragment();
        paTab = actionBar.newTab().setText(R.string.pa_mode);
        paTab.setTabListener(new displayTabListener(paFragment));
        actionBar.addTab(paTab);

        mcFoliageFragment = new McFoliageFragment();
        mcFoliageTab = actionBar.newTab().setText(R.string.mc_foliage_mode);
        mcFoliageTab.setTabListener(new displayTabListener(mcFoliageFragment));
        actionBar.addTab(mcFoliageTab);

        mcSkinFragment = new McSkinFragment();
        mcSkinTab = actionBar.newTab().setText(R.string.mc_skin_mode);
        mcSkinTab.setTabListener(new displayTabListener(mcSkinFragment));
        actionBar.addTab(mcSkinTab);

        mcSkyFragment = new McSkyFragment();
        mcSkyTab = actionBar.newTab().setText(R.string.mc_sky_mode);
        mcSkyTab.setTabListener(new displayTabListener(mcSkyFragment));
        actionBar.addTab(mcSkyTab);

        abFragment = new AbFragment();
        abTab = actionBar.newTab().setText(R.string.ab_mode);
        abTab.setTabListener(new displayTabListener(abFragment));
        actionBar.addTab(abTab);

        // populate the mode name if we are editing an existing mode
        Intent intent = getIntent();
        modeName = intent.getStringExtra(MenuActivity.MODIFY_MODE_NAME);
        modeID = intent.getIntExtra(MenuActivity.MODIFY_MODE_ID, -1);

        ColorManagerListener colorinterface = new ColorManagerListener() {
            @Override
            public void onConnected() {
                getColorManagerInstance();
            }
        };
        int retVal = ColorManager.connect(this, colorinterface);
        if (retVal != ColorManager.RET_SUCCESS) {
            Toast.makeText(getApplicationContext(),
                    R.string.err_service_connect_error, Toast.LENGTH_LONG)
                    .show();
            Log.e(TAG, "Connection failed");
        }
    }

    private void getColorManagerInstance() {
        cMgr = ColorManager.getInstance(getApplication(), this,
                DCM_DISPLAY_TYPE.DISP_PRIMARY);
        if (null != cbFragment) {
            cbFragment.colorManagerInit(cMgr);
        }
        if (null != sviFragment) {
            sviFragment.colorManagerInit(cMgr);
        }
        if (null != paFragment) {
            paFragment.colorManagerInit(cMgr);
        }
        if (null != mcFoliageFragment) {
            mcFoliageFragment.colorManagerInit(cMgr);
        }
        if (null != mcSkinFragment) {
            mcSkinFragment.colorManagerInit(cMgr);
        }
        if (null != mcSkyFragment) {
            mcSkyFragment.colorManagerInit(cMgr);
        }
        if (null != abFragment) {
            abFragment.colorManagerInit(cMgr);
        }

    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        MenuInflater inflater = getMenuInflater();
        inflater.inflate(R.menu.modemodifyoptionsmenu, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        // Handle item selection
        switch (item.getItemId()) {
        case R.id.save_mode:
            AlertDialog msgBox = new AlertDialog.Builder(this).create();
            msgBox.setTitle(R.string.save_dialog_title);

            final EditText inputText = new EditText(getApplication()
                    .getApplicationContext());
            inputText.setTextColor(getResources().getColor(R.color.black));
            inputText
                    .setFilters(new InputFilter[] { new InputFilter.LengthFilter(
                            30) });
            if (null != modeName) {
                inputText.setText(modeName);
            }
            msgBox.setView(inputText);
            msgBox.setButton(AlertDialog.BUTTON_POSITIVE, getResources()
                    .getString(R.string.btn_ok),
                    new DialogInterface.OnClickListener() {
                        public void onClick(DialogInterface dialog, int which) {
                            modeName = inputText.getText().toString();
                            if (null != cMgr) {
                                if (-1 == modeID) {
                                    int retVal = cMgr.createNewMode(modeName);
                                    if (retVal < 0) {
                                        Toast.makeText(getApplicationContext(),
                                                R.string.err_new_mode_fail,
                                                Toast.LENGTH_LONG).show();
                                    }
                                } else {
                                    int retVal = cMgr.modifyMode(modeID,
                                            modeName);
                                    if (retVal < 0) {
                                        Toast.makeText(getApplicationContext(),
                                                R.string.err_modify_mode_fail,
                                                Toast.LENGTH_LONG).show();
                                    }
                                }
                                ModeModifySample.this.finish();
                            } else {
                                Toast.makeText(getApplicationContext(),
                                        R.string.err_instance_na,
                                        Toast.LENGTH_LONG).show();
                            }
                        }
                    });
            msgBox.show();
            // Log.i("ModeModify: ", " Save mode cliked on option menu");
            return true;
        case R.id.pick_photo:
            // Log.i(TAG, "Pick photo item clicked");
            Intent i = new Intent(
                    Intent.ACTION_PICK,
                    android.provider.MediaStore.Images.Media.EXTERNAL_CONTENT_URI);
            startActivityForResult(i, IMAGE_PICKER_SELECT);
            return false;
        default:
            return super.onOptionsItemSelected(item);
        }
    }

    /**
     * Photo Selection result
     */
    @Override
    public void onActivityResult(int requestCode, int resultCode, Intent data) {
        if (requestCode == IMAGE_PICKER_SELECT
                && resultCode == Activity.RESULT_OK) {
            MenuActivity.bitmap = getBitmapFromCameraData(data, this);

            if (null != cbFragment) {
                cbFragment.updateBackgroundImage();
            }
            if (null != sviFragment) {
                sviFragment.updateBackgroundImage();
            }
            if (null != paFragment) {
                paFragment.updateBackgroundImage();
            }
            if (null != mcFoliageFragment) {
                mcFoliageFragment.updateBackgroundImage();
            }
            if (null != mcSkinFragment) {
                mcSkinFragment.updateBackgroundImage();
            }
            if (null != mcSkyFragment) {
                mcSkyFragment.updateBackgroundImage();
            }
            if (null != abFragment) {
                abFragment.updateBackgroundImage();
            }
        }
    }

    /**
     * Use for decoding camera response data.
     */
    public static Bitmap getBitmapFromCameraData(Intent data, Context context) {
        Uri selectedImage = data.getData();
        String[] filePathColumn = { MediaStore.Images.Media.DATA };
        Cursor cursor = context.getContentResolver().query(selectedImage,
                filePathColumn, null, null, null);
        cursor.moveToFirst();
        int columnIndex = cursor.getColumnIndex(filePathColumn[0]);
        String picturePath = cursor.getString(columnIndex);
        cursor.close();
        return BitmapFactory.decodeFile(picturePath);
    }

    public class displayTabListener implements ActionBar.TabListener {
        Fragment fragment;

        public displayTabListener(Fragment f) {
            this.fragment = f;
        }

        @Override
        public void onTabSelected(Tab tab, FragmentTransaction ft) {
            ft.replace(R.id.sdk_feature_container, fragment);
        }

        @Override
        public void onTabUnselected(Tab tab, FragmentTransaction ft) {
            ft.remove(fragment);

        }

        @Override
        public void onTabReselected(Tab tab, FragmentTransaction ft) {
            // TODO Auto-generated method stub

        }

    }
}