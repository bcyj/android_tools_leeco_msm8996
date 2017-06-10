/* ====================================================================
 * Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 * =====================================================================
 * @file QdcmMobileMain.java
 *
 */
package com.qualcomm.qti.snapdragon.qdcm_mobile;

import java.io.IOException;
import java.io.InputStream;
import java.util.EnumSet;


import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.database.Cursor;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Color;
import android.graphics.drawable.ShapeDrawable;
import android.graphics.drawable.shapes.OvalShape;
import android.graphics.drawable.shapes.PathShape;
import android.graphics.drawable.shapes.RectShape;
import android.net.Uri;
import android.os.Bundle;
import android.provider.MediaStore;
import android.text.Html;
import android.util.DisplayMetrics;
import android.util.Log;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.LinearLayout;
import android.widget.ImageView;
import android.widget.SeekBar;
import android.widget.SeekBar.OnSeekBarChangeListener;
import android.widget.TextView;
import android.app.AlertDialog;
import android.content.DialogInterface;

import com.qti.snapdragon.sdk.display.ColorManager;
import com.qti.snapdragon.sdk.display.ColorManager.ColorManagerListener;
import com.qti.snapdragon.sdk.display.PictureAdjustmentConfig;
import com.qti.snapdragon.sdk.display.ColorManager.DCM_FEATURE;
import com.qti.snapdragon.sdk.display.ColorManager.DCM_DISPLAY_TYPE;
import com.qti.snapdragon.sdk.display.PictureAdjustmentConfig.PICTURE_ADJUSTMENT_PARAMS;
import com.qualcomm.qti.snapdragon.qdcm_mobile.R;

public class QdcmMobileMain extends Activity {

    private static String TAG = "QDCM_Mobile_Main";
    private static String MAJOR_VERSION = "3";
    private static String MINOR_VERSION = "0";
    private static String help_info =
        "<b>Adjustments:</b> They are basic sliders for color temperature " +
        "and picture adjustment. When the user modifies any setting, then " +
        "he/she is allowed to save settings by pressing \"<b>Save</b>\" button or " +
        "revert to last saved settings by pressing \"<b>Cancel</b>\" button." +
        "<br><br><b>View Image Only:</b> This icon can be found on the top " +
        "bar, which allows the user to view image only for better visibility. " +
        "Pressing the icon again will bring the settings back.<br><br>" +
        "<b>Change Background:</b> This setting allows the user to change the " +
        "background surface view of the app. The user is required to store " +
        "images in sdcard first to be able to select images.<br><br>" +
        "<b>Restore Settings:</b> This setting allows the user to restore the " +
        "display settings to the factory default. An important thing to note " +
        "is these settings are not user's last saved settings. After restoring " +
        "the factory default, the user can revert it to last saved setting " +
        "by pressing \"Cancel\" button or save it by pressing \"Save\" button.";

    SeekBar sb_cb, sb_hue, sb_sat, sb_int, sb_cont;
    TextView tv_cb, tv_hue, tv_sat, tv_int, tv_cont;
    TextView cb_value, hue_val, sat_val, int_val, cont_val;
    Button btn_save, btn_cancel;
    ColorManager cmgr;
    int colorTemp;
    boolean showImageOnly = false;
    public static Bitmap bitmap = null;
    private static final int IMAGE_PICKER_SELECT = 999;
    AlertDialog.Builder alertDialog;

    private int pahue = 0;
    private int pasaturation = 0;
    private int paintensity = 0;
    private int pacontrast = 0;
    int HueMIN = 0, HueMAX = 0;
    int SaturationMIN = 0, SaturationMAX = 0;
    int IntensityMIN = 0, IntensityMAX = 0;
    int ContrastMIN = 0, ContrastMAX = 0;
    boolean pa_enable = false;
    boolean settings_modified = false;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        /* Get display size to select xml file  */
        DisplayMetrics metrics = new DisplayMetrics();
        getWindowManager().getDefaultDisplay().getMetrics(metrics);
        int h_pixels = metrics.heightPixels;
        int w_pixels = metrics.widthPixels;
        int thumbPercentage = 75;

        if (w_pixels <= 480 || h_pixels <= 800) {
            setContentView(R.layout.firstscreensmall);
            thumbPercentage = 35;
        }
        else if (w_pixels <= 1080 || h_pixels <= 1920) {
            setContentView(R.layout.firstscreensmall);
            thumbPercentage = 75;

            int_val = (TextView)findViewById(R.id.int_Value);
            cont_val = (TextView)findViewById(R.id.cont_Value);

            btn_save = (Button) findViewById(R.id.save_button);
            btn_cancel = (Button) findViewById(R.id.cancel_button);
            btn_save.getBackground().setAlpha(0xBF);
            btn_cancel.getBackground().setAlpha(0xBF);
        }
        else {
            setContentView(R.layout.firstscreen);
            thumbPercentage = 75;
        }

        Log.d(TAG, "width-pixel: " + w_pixels + " --- height-pixel: " +
                h_pixels);
        setTitle("QDCM Mobile v" + MAJOR_VERSION + "." + MINOR_VERSION);

        alertDialog = new AlertDialog.Builder(this);

        sb_cb = (SeekBar)findViewById(R.id.colortemp);
        sb_hue = (SeekBar)findViewById(R.id.hue);
        sb_sat = (SeekBar)findViewById(R.id.sat);
        sb_int = (SeekBar)findViewById(R.id.intensity);
        sb_cont = (SeekBar)findViewById(R.id.cont);

        cb_value = (TextView)findViewById(R.id.CB_Value);
        hue_val = (TextView)findViewById(R.id.Hue_Value);
        sat_val = (TextView)findViewById(R.id.Sat_Value);
        int_val = (TextView)findViewById(R.id.int_Value);
        cont_val = (TextView)findViewById(R.id.cont_Value);

        btn_save = (Button) findViewById(R.id.save_button);
        btn_cancel = (Button) findViewById(R.id.cancel_button);
        btn_save.getBackground().setAlpha(0xBF);
        btn_cancel.getBackground().setAlpha(0xBF);

        ShapeDrawable thumb_cb = new ShapeDrawable(new OvalShape());
        ShapeDrawable thumb_hue = new ShapeDrawable(new OvalShape());
        ShapeDrawable thumb_sat = new ShapeDrawable(new OvalShape());
        ShapeDrawable thumb_int = new ShapeDrawable(new OvalShape());
        ShapeDrawable thumb_cont = new ShapeDrawable(new OvalShape());

        thumb_cb.getPaint().setColor(Color.rgb(0xa, 0x5a, 0x8c));
        thumb_cb.setIntrinsicHeight(thumbPercentage);
        thumb_cb.setIntrinsicWidth(thumbPercentage);
        thumb_cb.setAlpha(0x80);
        thumb_hue.getPaint().setColor(Color.rgb(0xa, 0x5a, 0x8c));
        thumb_hue.setIntrinsicHeight(thumbPercentage);
        thumb_hue.setIntrinsicWidth(thumbPercentage);
        thumb_hue.setAlpha(0x80);
        thumb_sat.getPaint().setColor(Color.rgb(0xa, 0x5a, 0x8c));
        thumb_sat.setIntrinsicHeight(thumbPercentage);
        thumb_sat.setIntrinsicWidth(thumbPercentage);
        thumb_sat.setAlpha(0x80);
        thumb_int.getPaint().setColor(Color.rgb(0xa, 0x5a, 0x8c));
        thumb_int.setIntrinsicHeight(thumbPercentage);
        thumb_int.setIntrinsicWidth(thumbPercentage);
        thumb_int.setAlpha(0x80);
        thumb_cont.getPaint().setColor(Color.rgb(0xa, 0x5a, 0x8c));
        thumb_cont.setIntrinsicHeight(thumbPercentage);
        thumb_cont.setIntrinsicWidth(thumbPercentage);
        thumb_cont.setAlpha(0x80);

        sb_cb.setThumb(thumb_cb);
        sb_hue.setThumb(thumb_hue);
        sb_sat.setThumb(thumb_sat);
        sb_int.setThumb(thumb_int);
        sb_cont.setThumb(thumb_cont);

        ColorManagerListener colorinterface = new ColorManagerListener() {
            @Override
            public void onConnected() {

                getHSICInstance();
                getColorBalanceInstance();
            }
        };
        int retVal = ColorManager.connect(this, colorinterface);
        if (retVal != ColorManager.RET_SUCCESS) {
            Log.e(TAG, "Connection failed");
        }

        if (cmgr == null)
            cmgr = ColorManager.getInstance(getApplication(), this,
                    DCM_DISPLAY_TYPE.DISP_PRIMARY);

        if (cmgr != null) {

            getHSICInstance();
            getColorBalanceInstance();
            updateSeekbarValues();
        }

        // Populate the image with a default image
        try {
            InputStream defaultImage = getAssets().open(
                    getResources().getString(R.string.default_image));
            QdcmMobileMain.bitmap = BitmapFactory.decodeStream(defaultImage);
        } catch (IOException e) {
            e.printStackTrace();
        }

        updateBackgroundImage();

        if (cmgr != null) {
            int[] activeMode = cmgr.getActiveMode();
            if (activeMode != null)
                settings_modified = (activeMode[1] == 0) ? false : true;
        }
        if (settings_modified == false) {
            btn_cancel.setAlpha((float)0.5);
            btn_save.setAlpha((float)0.5);
        }
        else {
            btn_cancel.setAlpha(1);
            btn_save.setAlpha(1);
        }

        btn_cancel.setEnabled(settings_modified);
        btn_save.setEnabled(settings_modified);

        btn_save.setOnClickListener(new OnClickListener() {

            @Override
            public void onClick(View arg0) {

                //User confirmation!
                alertDialog.setTitle("Saving!");
                alertDialog.setMessage("Are you sure you want to save settings?");
                alertDialog.setCancelable(false);
                alertDialog.setPositiveButton("Yes", new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int id) {

                        int modeID = cmgr.createNewMode("8909default");
                        Log.e(TAG, "ModeId=" + modeID);
                        if (modeID < 0) {
                            int[] activeMode = cmgr.getActiveMode();
                            modeID = activeMode[0];
                            Log.e(TAG, "ModeId=" + modeID);
                            cmgr.modifyMode(modeID, "8909default");
                        }
                        cmgr.setDefaultMode(modeID);

                        // hiding cancel/save button, since nothing to restore
                        btn_save.setAlpha((float)0.5);
                        btn_cancel.setAlpha((float)0.5);

                        settings_modified = false;
                        btn_cancel.setEnabled(settings_modified);
                        btn_save.setEnabled(settings_modified);
                    }
                });
                alertDialog.setNegativeButton("No", null);
                alertDialog.show();
            }
        });

        btn_cancel.setOnClickListener(new OnClickListener() {

            @Override
            public void onClick(View arg0) {

                //User confirmation!
                alertDialog.setTitle("Reverting Changes!");
                alertDialog.setMessage("Are you sure you want to revert changes?");
                alertDialog.setCancelable(false);
                alertDialog.setPositiveButton("Yes", new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int id) {

                        cmgr.setActiveMode(100);
                        getHSICInstance();
                        getColorBalanceInstance();
                        updateSeekbarValues();
                        setPaDisplayParams();

                        // hiding cancel/save button, since nothing to restore
                        btn_save.setAlpha((float)0.5);
                        btn_cancel.setAlpha((float)0.5);

                        settings_modified = false;
                        btn_cancel.setEnabled(settings_modified);
                        btn_save.setEnabled(settings_modified);
                    }
                });
                alertDialog.setNegativeButton("No", null);
                alertDialog.show();
            }
        });
    }

    private void updateSeekbarValues() {
        cb_value.setText("(" + colorTemp + ")");
        hue_val.setText("(" + pahue + ")");
        sat_val.setText("(" + pasaturation + ")");
        int_val.setText("(" + paintensity + ")");
        cont_val.setText("(" + pacontrast + ")");
    }

    private void getColorBalanceInstance() {

        if (cmgr == null) {
            cmgr = ColorManager.getInstance(getApplication(), this,
                    DCM_DISPLAY_TYPE.DISP_PRIMARY);
        }

        if (cmgr != null) {
            boolean isSupport = false;
            isSupport = cmgr.isFeatureSupported(DCM_FEATURE.FEATURE_COLOR_BALANCE);
            if (isSupport == false) {
                sb_cb.setEnabled(false);
                return;
            }
            colorTemp = cmgr.getColorBalance();
            if (colorTemp < ColorManager.COLOR_BALANCE_WARMTH_LOWER_BOUND
                    || colorTemp > ColorManager.COLOR_BALANCE_WARMTH_UPPER_BOUND) {
                colorTemp = 0;
                    }

            sb_cb.setMax(200);
            sb_cb.setProgress(colorTemp + 100);
            sb_cb.setOnSeekBarChangeListener(new OnSeekBarChangeListener() {

            @Override
            public void onStopTrackingTouch(SeekBar arg0) {
                //cmgr.setColorBalance(colorTemp);
            }

            @Override
            public void onStartTrackingTouch(SeekBar arg0) {
            }

            @Override
            public void onProgressChanged(SeekBar arg0, int arg1, boolean arg2) {
                colorTemp = arg1 - 100;

                long startTime = System.nanoTime();

                cmgr.setColorBalance(colorTemp);
                cb_value.setText("(" + colorTemp + ")");

                long endTime = System.nanoTime();
                long diff = endTime - startTime ;
                Log.e(TAG, "Elapsed microseconds: " + diff /1000);

                btn_cancel.setAlpha(1);
                btn_save.setAlpha(1);

                settings_modified = true;
                btn_cancel.setEnabled(settings_modified);
                btn_save.setEnabled(settings_modified);
            }
            });
        }
    }

    private void getHSICInstance() {

        if (cmgr == null) {
            cmgr = ColorManager.getInstance(getApplication(), this,
                    DCM_DISPLAY_TYPE.DISP_PRIMARY);
        }

        if (cmgr != null) {
            boolean isSupport = false;

            isSupport = cmgr
                .isFeatureSupported(DCM_FEATURE.FEATURE_GLOBAL_PICTURE_ADJUSTMENT);
            if (isSupport == false) {
                pa_enable = false;
                return;
            }

            // Initialize local variables
            PictureAdjustmentConfig paValues = cmgr.getPictureAdjustmentParams();
            if (null != paValues) {
                pahue = paValues.getHue();
                pasaturation = paValues.getSaturation();
                paintensity = paValues.getIntensity();
                pacontrast = paValues.getContrast();
                pa_enable = true;
            } else {
                Log.e(TAG, "getPictureAdjustmentParams returned null during init");
            }

            sb_hue.setEnabled(pa_enable);
            sb_sat.setEnabled(pa_enable);
            sb_int.setEnabled(pa_enable);
            sb_cont.setEnabled(pa_enable);

            setupHue();
            setupIntensity();
            setupSaturation();
            setupContrast();
        } else {
            Log.e(TAG, "Object creation failed");
        }
    }

    private void setPaDisplayParams() {
        PictureAdjustmentConfig newPaConfig = new PictureAdjustmentConfig(
                EnumSet.allOf(PictureAdjustmentConfig.PICTURE_ADJUSTMENT_PARAMS.class),
                pahue, pasaturation, paintensity, pacontrast, 0);
        cmgr.setPictureAdjustmentParams(newPaConfig);
    }

    private void setupHue() {
        // Initialize local variables for various bounds
        HueMAX = cmgr
            .getMaxLimitPictureAdjustment(PICTURE_ADJUSTMENT_PARAMS.HUE);
        HueMIN = cmgr
            .getMinLimitPictureAdjustment(PICTURE_ADJUSTMENT_PARAMS.HUE);

        sb_hue.setMax(HueMAX - HueMIN);
        sb_hue.setProgress(pahue - HueMIN);
        sb_hue.setOnSeekBarChangeListener(new OnSeekBarChangeListener() {

        @Override
        public void onStopTrackingTouch(SeekBar seekBar) {
        }

        @Override
        public void onStartTrackingTouch(SeekBar seekBar) {
        }

        @Override
        public void onProgressChanged(SeekBar seekBar, int progress,
            boolean fromUser) {

            pahue = progress + HueMIN;
            setPaDisplayParams();
            hue_val.setText("(" + pahue + ")");

            btn_save.setAlpha(1);
            btn_cancel.setAlpha(1);

            settings_modified = true;
            btn_cancel.setEnabled(settings_modified);
            btn_save.setEnabled(settings_modified);
        }
        });
    }

    private void setupSaturation() {

        // Initialize local variables for various bounds
        SaturationMAX = cmgr
            .getMaxLimitPictureAdjustment(PICTURE_ADJUSTMENT_PARAMS.SATURATION);
        SaturationMIN = cmgr
            .getMinLimitPictureAdjustment(PICTURE_ADJUSTMENT_PARAMS.SATURATION);

        sb_sat.setMax(SaturationMAX - SaturationMIN);
        sb_sat.setProgress(pasaturation - SaturationMIN);
        sb_sat.setOnSeekBarChangeListener(new OnSeekBarChangeListener() {

        @Override
        public void onStopTrackingTouch(SeekBar seekBar) {
        }

        @Override
        public void onStartTrackingTouch(SeekBar seekBar) {
        }

        @Override
        public void onProgressChanged(SeekBar seekBar,
            int progress, boolean fromUser) {

            pasaturation = progress + SaturationMIN;
            setPaDisplayParams();
            sat_val.setText("(" + pasaturation + ")");

            btn_save.setAlpha(1);
            btn_cancel.setAlpha(1);

            settings_modified = true;
            btn_cancel.setEnabled(settings_modified);
            btn_save.setEnabled(settings_modified);
        }
        });
    }

    private void setupIntensity() {

        // Initialize local variables for various bounds
        IntensityMAX = cmgr
            .getMaxLimitPictureAdjustment(PICTURE_ADJUSTMENT_PARAMS.INTENSITY);
        IntensityMIN = cmgr
            .getMinLimitPictureAdjustment(PICTURE_ADJUSTMENT_PARAMS.INTENSITY);

        sb_int.setMax(IntensityMAX - IntensityMIN);
        sb_int.setProgress(paintensity - IntensityMIN);
        sb_int.setOnSeekBarChangeListener(new OnSeekBarChangeListener() {

        @Override
        public void onStopTrackingTouch(SeekBar seekBar) {
        }

        @Override
        public void onStartTrackingTouch(SeekBar seekBar) {
        }

        @Override
        public void onProgressChanged(SeekBar seekBar,
            int progress, boolean fromUser) {

            paintensity = progress + IntensityMIN;
            setPaDisplayParams();
            int_val.setText("(" + paintensity + ")");

            btn_save.setAlpha(1);
            btn_cancel.setAlpha(1);

            settings_modified = true;
            btn_cancel.setEnabled(settings_modified);
            btn_save.setEnabled(settings_modified);
        }
        });
    }

    private void setupContrast() {

        // Initialize local variables for various bounds
        ContrastMAX = cmgr
            .getMaxLimitPictureAdjustment(PICTURE_ADJUSTMENT_PARAMS.CONTRAST);
        ContrastMIN = cmgr
            .getMinLimitPictureAdjustment(PICTURE_ADJUSTMENT_PARAMS.CONTRAST);

        sb_cont.setMax(ContrastMAX - ContrastMIN);
        sb_cont.setProgress(pacontrast - ContrastMIN);
        sb_cont.setOnSeekBarChangeListener(new OnSeekBarChangeListener() {

        @Override
        public void onStopTrackingTouch(SeekBar seekBar) {
        }

        @Override
        public void onStartTrackingTouch(SeekBar seekBar) {
        }

        @Override
        public void onProgressChanged(SeekBar seekBar,
            int progress, boolean fromUser) {

            pacontrast = progress + ContrastMIN;
            setPaDisplayParams();
            cont_val.setText("(" + pacontrast + ")");

            btn_save.setAlpha(1);
            btn_cancel.setAlpha(1);

            settings_modified = true;
            btn_cancel.setEnabled(settings_modified);
            btn_save.setEnabled(settings_modified);
        }
        });
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {

        MenuInflater menuInf = getMenuInflater();
        menuInf.inflate(R.menu.menu, menu);
        return super.onCreateOptionsMenu(menu);
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch(item.getItemId())
        {
            case R.id.viewImage:
                LinearLayout mainLayout = (LinearLayout)findViewById(R.id.MainLayout);
                if (showImageOnly == false) {
                    mainLayout.setVisibility(View.INVISIBLE);
                    item.setTitle("View Settings");
                    item.setIcon(R.drawable.view_image);
                    showImageOnly = true;
                }
                else {
                    mainLayout.setVisibility(View.VISIBLE);
                    item.setTitle("View Image Only");
                    item.setIcon(R.drawable.hide_image);
                    showImageOnly = false;
                }
                break;
            case R.id.changeBack:
                Intent i = new Intent(Intent.ACTION_PICK,
                        android.provider.MediaStore.Images.Media.EXTERNAL_CONTENT_URI);
                startActivityForResult(i, IMAGE_PICKER_SELECT);
                break;
            case R.id.restoreSettings:
                //User confirmation!
                alertDialog.setTitle("Restoring to Factory!");
                alertDialog.setMessage("Are you sure you want to restore factory setting?\n\n" +
                        "Note: Factory setting is not your last saved setting.\n");
                alertDialog.setCancelable(false);
                alertDialog.setPositiveButton("Yes", new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int id) {

                        int defMode = cmgr.getDefaultMode();
                        cmgr.setActiveMode(defMode);
                        getHSICInstance();
                        getColorBalanceInstance();
                        updateSeekbarValues();
                        setPaDisplayParams();

                        btn_save.setAlpha(1);
                        btn_cancel.setAlpha(1);

                        settings_modified = true;
                        btn_cancel.setEnabled(settings_modified);
                        btn_save.setEnabled(settings_modified);
                    }
                });
                alertDialog.setNegativeButton("No", null);
                alertDialog.show();
                break;
            case R.id.help:
                AlertDialog.Builder alertDialogBuilder = new AlertDialog.Builder(
                        this);
                alertDialogBuilder.setTitle("Helpful Tips!");
                alertDialogBuilder.setMessage(Html.fromHtml(help_info));
                alertDialogBuilder.setCancelable(true);
                alertDialogBuilder.setPositiveButton("Close", null);
                AlertDialog alertDialog = alertDialogBuilder.create();
                alertDialog.show();
                break;
        }
        return true;
    }

    public void updateBackgroundImage() {
        ImageView im = (ImageView) findViewById(R.id.surfaceimage);
        if (null != QdcmMobileMain.bitmap) {
            im.setImageBitmap(QdcmMobileMain.bitmap);
        }
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        if (requestCode == IMAGE_PICKER_SELECT
                && resultCode == Activity.RESULT_OK) {
            // Log.i(TAG, "onActivityResult on end of photo picker activity");
            QdcmMobileMain.bitmap = getBitmapFromCameraData(data, this);
            ImageView im = (ImageView) findViewById(R.id.surfaceimage);

            if (null != QdcmMobileMain.bitmap) {
                im.setImageBitmap(QdcmMobileMain.bitmap);
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
}

