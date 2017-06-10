/* ====================================================================
 * Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * QTI Proprietary and Confidential.
 * =====================================================================
 * @file ModeManagerSample.java
 *
 */
package com.qualcomm.snapdragon.sample;

import java.util.ArrayList;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.database.Cursor;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.Typeface;
import android.net.Uri;
import android.os.Bundle;
import android.provider.MediaStore;
import android.util.Log;
import android.view.ActionMode;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup.LayoutParams;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.Toast;

import com.qti.snapdragon.sdk.display.ColorManager;
import com.qti.snapdragon.sdk.display.ColorManager.ColorManagerListener;
import com.qti.snapdragon.sdk.display.ColorManager.DCM_DISPLAY_TYPE;
import com.qti.snapdragon.sdk.display.ColorManager.DCM_FEATURE;
import com.qti.snapdragon.sdk.display.ColorManager.MODE_TYPE;
import com.qti.snapdragon.sdk.display.ModeInfo;

public class ModeManagerSample extends Activity {

    private static String TAG = "DisplaySDK-ModeManager";

    ActionMode mActionMode = null;
    private ColorManager cMgr;

    // Code for our image picker select action.
    private static final int IMAGE_PICKER_SELECT = 999;
    private static final int MODE_MODIFY_RESULT = 998;

    // This variable keeps track of last mode that was activated
    // by the user. This also keeps track of currently active
    // mode as in this app a mode activated (clicked on) by the
    // user is set as activeMode in display
    private ModeButton currentActiveView = null;
    private ModeInfoWrapper activeModeWrapper = null;

    // This variable keeps track of which mode is presently
    // the default mode
    private int defaultModeID = -1;

    // This variable keeps track of when the activity is in delete mode
    private boolean inDeleteMode = false;

    public ArrayList<ModeInfoWrapper> modeList;
    LinearLayout modeGallery;

    protected Bitmap deleteBitmap = null;
    protected Paint paintDefault = null;

    public class ModeInfoWrapper {
        public ModeInfo mode;
        public String modename;
        public int modeID;

        ModeInfoWrapper(ModeInfo displayMode) {
            mode = displayMode;
            modename = displayMode.getName();
            modeID = displayMode.getId();
        }

        @Override
        public String toString() {
            return modename;
        }

        public void resetName() {
            modename = mode.getName();
        }
    }

    private class ModeButton extends Button {
        protected ModeInfoWrapper modewrapper;

        public ModeButton(Context context, ModeInfoWrapper modeInfoWrapper) {
            super(context);
            this.modewrapper = modeInfoWrapper;
            this.setText(modeInfoWrapper.toString());
            this.setTextAlignment(View.TEXT_ALIGNMENT_CENTER);
            this.setTextSize(13);
            this.setWidth(300);
            this.setHeight(100);
            this.setLayoutParams(new LayoutParams(LayoutParams.WRAP_CONTENT,
                    LayoutParams.MATCH_PARENT));
            if (ColorManager.MODE_TYPE.MODE_USER == modeInfoWrapper.mode
                    .getModeType()) {
                this.setTypeface(Typeface.SERIF, Typeface.ITALIC);
                this.setBackgroundResource(R.drawable.mode_style_user);
            } else {
                this.setTypeface(Typeface.SERIF, Typeface.BOLD);
                this.setBackgroundResource(R.drawable.mode_style_system);
            }

            if (!inDeleteMode) {
                this.setOnClickListener(modeButtonOnClickListener);
            } else if (modeInfoWrapper.mode.getModeType() == ColorManager.MODE_TYPE.MODE_USER) {
                this.setOnClickListener(modeButtonOnClickListenerDelete);

                // special case for active mode- it cannot be deleted
                // clear out the listener
                if (activeModeWrapper != null
                        && activeModeWrapper.modeID == modeInfoWrapper.modeID) {
                    this.setOnClickListener(null);
                }
                // special case for default mode- it cannot be deleted
                // clear out the listener
                if (defaultModeID == modeInfoWrapper.modeID) {
                    this.setOnClickListener(null);
                }
            }
        }

        @Override
        protected void onDraw(Canvas canvas) {
            try {
                super.onDraw(canvas);
                if (inDeleteMode
                        && modewrapper.mode.getModeType() == ColorManager.MODE_TYPE.MODE_USER) {
                    if (null != deleteBitmap) {
                        int activeModeID = -1;
                        // if a mode is active get the correct ID
                        if (null != activeModeWrapper) {
                            activeModeID = activeModeWrapper.modeID;
                        }
                        // show the delete bitmap only if the mode is not active
                        // and is not the default mode
                        if (activeModeID != modewrapper.modeID
                                && defaultModeID != modewrapper.modeID) {
                            canvas.drawBitmap(deleteBitmap,
                                    canvas.getWidth() - 100, 0, null);
                        }
                    } else {
                        Log.w(TAG, "delete bitmap not found for deletable mode");
                    }
                }
                if (defaultModeID == modewrapper.modeID && null!=paintDefault) {
                    // This is called only for the button of default mode
                    // for each call to populateModesOnScreen
                    canvas.drawText("*", (float) (canvas.getWidth() * 0.1),
                            (float) (canvas.getHeight() * 0.9), paintDefault);
                }
            } catch (Exception e) {
                e.printStackTrace();
            }
        }

        OnClickListener modeButtonOnClickListener = new OnClickListener() {
            @Override
            public void onClick(View v) {
                mActionMode = v.startActionMode(mActionModeCallback);
                cMgr.setActiveMode(modewrapper.modeID);

                // reset the selection of the previously active mode's
                // view
                if (null != currentActiveView) {
                    currentActiveView.setSelected(false);
                }
                // set the newly active button as selected
                currentActiveView = (ModeButton) v;
                activeModeWrapper = modewrapper;
                v.setSelected(true);

                Toast.makeText(getApplicationContext(),
                        R.string.mode_activated, Toast.LENGTH_SHORT).show();

                // Log.i(TAG, "item selected and set as active: "
                // + modewrapper.modename + " : " + modewrapper.modeID);

            }
        };

        OnClickListener modeButtonOnClickListenerDelete = new OnClickListener() {
            @Override
            public void onClick(View v) {
                // Log.i(TAG, " Deleting mode " + modewrapper.modename
                // + " ModeID " + modewrapper.modeID);
                if (null != cMgr) {
                    // Valid reference to display SDK manager available,
                    // skip deletion if system mode other wise delete the mode
                    MODE_TYPE mType = modewrapper.mode.getModeType();
                    if (mType == MODE_TYPE.MODE_SYSTEM) {
                        showDialog(getResources().getString(
                                R.string.msg_system_mode_non_deletable));
                    }
                    int retValue = cMgr.deleteMode(modewrapper.modeID);
                    if (retValue < 0) {
                        showDialog(getResources().getString(
                                R.string.err_delete_fail));
                    } else {
                        populateModesOnScreen();
                        Toast.makeText(getApplicationContext(),
                                R.string.mode_deleted, Toast.LENGTH_SHORT)
                                .show();
                    }
                } else {
                    Toast.makeText(getApplicationContext(),
                            R.string.err_instance_na, Toast.LENGTH_LONG).show();
                }
            }
        };
    }

    private void createModeList(ModeInfo[] pa) {
        modeList = new ArrayList<ModeInfoWrapper>();
        for (ModeInfo i : pa)
            modeList.add(new ModeInfoWrapper(i));
    }

    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        setContentView(R.layout.modemansample);
        setTitle(R.string.btn_mode_manager);

        modeGallery = (LinearLayout) findViewById(R.id.modegallery);
        updateBackgroundImage();
        Bitmap temp = BitmapFactory.decodeResource(getResources(),
                R.drawable.cross_red);
        deleteBitmap = Bitmap.createScaledBitmap(temp, 100, 100, false);
        paintDefault = new Paint();
        paintDefault.setColor(Color.BLACK);
        paintDefault.setTextSize(50f);

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

    public void updateBackgroundImage() {
        ImageView im = (ImageView) findViewById(R.id.surfaceimage);
        if (null != MenuActivity.bitmap) {
            im.setImageBitmap(MenuActivity.bitmap);
        }
    }

    private void getColorManagerInstance() {
        cMgr = ColorManager.getInstance(getApplication(), this,
                DCM_DISPLAY_TYPE.DISP_PRIMARY);

        if (cMgr != null) {
            boolean isSupported = cMgr
                    .isFeatureSupported(DCM_FEATURE.FEATURE_COLOR_MODE_SELECTION);
            if (isSupported) {
                boolean populated = populateModesOnScreen();
                if (!populated) {
                    Toast.makeText(getApplicationContext(),
                            R.string.err_no_modes, Toast.LENGTH_LONG).show();
                }
            } else {
                showDialog(getResources().getString(
                        R.string.err_mode_selection_na));
            }
        } else {
            showDialog(getResources().getString(R.string.err_instance_na));
        }
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        if (requestCode == IMAGE_PICKER_SELECT
                && resultCode == Activity.RESULT_OK) {
            // Log.i(TAG, "onActivityResult on end of photo picker activity");
            MenuActivity.bitmap = getBitmapFromCameraData(data, this);
            ImageView im = (ImageView) findViewById(R.id.surfaceimage);
            if (null != MenuActivity.bitmap) {
                im.setImageBitmap(MenuActivity.bitmap);
            }
        } else if (requestCode == MODE_MODIFY_RESULT) {
            boolean populated = populateModesOnScreen();
            if (!populated) {
                Toast.makeText(getApplicationContext(), R.string.err_no_modes,
                        Toast.LENGTH_LONG).show();
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

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        MenuInflater inflater = getMenuInflater();
        inflater.inflate(R.menu.menu, menu);
        return true;
    }

    @Override
    public boolean onPrepareOptionsMenu(Menu menu) {
        // Log.i(TAG, "onPrepareOptionsMenu delete mode is " + inDeleteMode);
        menu.findItem(R.id.new_mode).setVisible(!inDeleteMode);
        menu.findItem(R.id.pick_photo).setVisible(!inDeleteMode);
        menu.findItem(R.id.delete_mode).setVisible(!inDeleteMode);
        menu.findItem(R.id.cancel_delete).setVisible(inDeleteMode);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        invalidateOptionsMenu();

        // Handle item selection
        switch (item.getItemId()) {
        case R.id.new_mode:
            launchActivity(MenuActivity.MODE_MODIFY_INTENT, null, -1);
            // Log.i(TAG,
            // "onOptionsItemSelected New mode clicked on options menu");
            return true;
        case R.id.delete_mode:
            // Log.i(TAG, "onOptionsItemSelected delete mode pressed");
            inDeleteMode = true;
            populateModesOnScreen();
            return true;
        case R.id.cancel_delete:
            // Log.i(TAG, "onOptionsItemSelected cancel delete pressed");
            inDeleteMode = false;
            populateModesOnScreen();
            return true;
        case R.id.pick_photo:
            // Log.i(TAG, "onOptionsItemSelected Pick photo item clicked");
            Intent i = new Intent(
                    Intent.ACTION_PICK,
                    android.provider.MediaStore.Images.Media.EXTERNAL_CONTENT_URI);
            startActivityForResult(i, IMAGE_PICKER_SELECT);
            return false;
        default:
            return super.onOptionsItemSelected(item);
        }
    }

    private ActionMode.Callback mActionModeCallback = new ActionMode.Callback() {

        @Override
        public boolean onCreateActionMode(ActionMode mode, Menu menu) {
            MenuInflater inflater = mode.getMenuInflater();
            inflater.inflate(R.menu.context_menu, menu);
            return true;
        }

        @Override
        public boolean onPrepareActionMode(ActionMode mode, Menu menu) {
            return false; // Return false if nothing is done
        }

        @Override
        public boolean onActionItemClicked(ActionMode actionmode, MenuItem item) {
            // Action mode items are setup only when a mode has been activated
            // by
            // clicking on screen so currentActiveView will be valid
            switch (item.getItemId()) {
            case R.id.edit_mode:
                // Edit the mode
                if (null != cMgr) {
                    ModeInfoWrapper aMode = activeModeWrapper;
                    if (null != aMode) {
                        // Log.i(TAG,
                        // " Edit mode clicked in contexual action menu for "
                        // + aMode.modename + ", ID "
                        // + aMode.modeID);
                        MODE_TYPE mType = aMode.mode.getModeType();
                        if (mType == MODE_TYPE.MODE_SYSTEM) {
                            showSystemModeMessage();
                        } else {
                            launchActivity(MenuActivity.MODE_MODIFY_INTENT,
                                    aMode.modename, aMode.modeID);
                        }
                    } else {
                        Toast.makeText(getApplicationContext(),
                                R.string.err_select_mode, Toast.LENGTH_LONG)
                                .show();
                    }
                } else {
                    Toast.makeText(getApplicationContext(),
                            R.string.err_instance_na, Toast.LENGTH_LONG).show();
                }
                return true;
            case R.id.set_default_mode:
                // Set as default
                // set the mode as default by calling setDefaultMode(int
                // modeId) for mode last activated by user
                if (null != cMgr) {
                    ModeInfoWrapper aMode = activeModeWrapper;
                    if (null != aMode) {
                        // Log.i(TAG,
                        // "Default mode clicked in contextual action menu for "
                        // + aMode.modename + ", ID "
                        // + aMode.modeID);
                        int retValue = cMgr.setDefaultMode(aMode.modeID);
                        if (retValue < 0) {
                            showDialog(getResources().getString(
                                    R.string.err_default_fail));
                        } else {
                            defaultModeID = cMgr.getDefaultMode();
                            boolean populated = populateModesOnScreen();
                            if (!populated) {
                                Toast.makeText(getApplicationContext(),
                                        R.string.err_no_modes,
                                        Toast.LENGTH_LONG).show();
                            }
                        }
                    } else {
                        Toast.makeText(getApplicationContext(),
                                R.string.err_select_mode, Toast.LENGTH_LONG)
                                .show();
                    }
                } else {
                    Toast.makeText(getApplicationContext(),
                            R.string.err_instance_na, Toast.LENGTH_LONG).show();
                }
                return true;
            default:
                return false;
            }
        }

        // Called when the user exits the action mode
        @Override
        public void onDestroyActionMode(ActionMode mode) {

            mActionMode = null;
        }
    };

    /*
     * Name - id combination: 1) SomeName - id : modify user mode 2) NULL - id :
     * Modify system mode 3) NULL - -1 : New mode
     */
    void launchActivity(String intentName, String modeName, int modeID) {
        try {
            if (cMgr != null) {
                // do below if so that before modifying, the selected mode is
                // set active and if system mode being modified, then try to
                // create a new mode
                if (modeID >= 0) {
                    cMgr.setActiveMode(modeID);
                    if (modeName == null) {
                        modeID = -1;
                    }
                }

                Intent intent = new Intent();
                intent.setAction(intentName);
                intent.putExtra(MenuActivity.MODIFY_MODE_NAME, modeName);
                intent.putExtra(MenuActivity.MODIFY_MODE_ID, modeID);
                startActivityForResult(intent, MODE_MODIFY_RESULT);
            }
        } catch (Throwable t) {
            Log.e(TAG, "Error starting INTENT " + intentName + "for mode "
                    + modeName);
        }
    }

    // This method recreates the Mode list on the screen everytime there is a
    // change in display modes in system or when moving in or out of delete mode
    private boolean populateModesOnScreen() {
        ModeInfo[] modeDataArray = null;
        if (cMgr == null) {
            Log.e(TAG, "populateModesOnScreen(): Display SDK manager is null!");
            return false;
        }
        modeDataArray = cMgr.getModes(MODE_TYPE.MODE_ALL);
        currentActiveView = null;
        activeModeWrapper = null;

        // Clear the current list of modes from the UI
        modeGallery.removeAllViews();
        if (modeDataArray != null) {
            createModeList(modeDataArray);
            boolean firstUserMode = true;
            int[] activeMode = cMgr.getActiveMode();
            // Log.i(TAG, "Currently Active modeID is " + activeMode[0]);
            defaultModeID = cMgr.getDefaultMode();
            // Log.i(TAG,"Default mode is "+defaultMode);
            for (ModeInfoWrapper mode : modeList) {

                // If this mode is active update update the
                // book keeping before creating buttons as
                // our custom button class depends on book
                // keeping
                if (activeMode[0] == mode.modeID) {
                    activeModeWrapper = mode;
                }

                ModeButton button = new ModeButton(this, mode);

                // Insert divider if this is the first user mode
                if (firstUserMode == true
                        && mode.mode.getModeType() == ColorManager.MODE_TYPE.MODE_USER) {
                    firstUserMode = false;
                    ImageView divider = new ImageView(this);
                    LinearLayout.LayoutParams params = new LinearLayout.LayoutParams(
                            10, LayoutParams.MATCH_PARENT);
                    params.setMargins(10, 1, 10, 1);
                    divider.setLayoutParams(params);
                    divider.setBackgroundColor(Color.WHITE);
                    modeGallery.addView(divider);
                }

                // If this mode is active update the button
                if (activeMode[0] == mode.modeID) {
                    currentActiveView = button;
                    button.setSelected(true);
                }

                if (null != modeGallery) {
                    modeGallery.addView(button);
                }
                // Log.i(TAG, "Button added for mode " + mode.toString());
            }
            return true;
        }
        return false;
    }

    private void showDialog(String message) {
        AlertDialog msgBox = new AlertDialog.Builder(this).create();
        msgBox.setMessage(message);
        msgBox.setButton(AlertDialog.BUTTON_POSITIVE,
                getResources().getString(R.string.btn_ok),
                new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int which) {
                    }
                });
        msgBox.show();
    }

    private void showSystemModeMessage() {
        AlertDialog msgBox = new AlertDialog.Builder(this).create();
        msgBox.setMessage(getResources().getString(
                R.string.msg_system_mode_non_editable));
        msgBox.setButton(AlertDialog.BUTTON_POSITIVE,
                getResources().getString(R.string.btn_yes),
                new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int which) {
                        ModeInfoWrapper aMode = activeModeWrapper;
                        launchActivity(MenuActivity.MODE_MODIFY_INTENT, null,
                                aMode.modeID);
                        // Log.i(TAG, "Creating new mode based selected mode"
                        // + aMode.modename);
                    }
                });
        msgBox.setButton(AlertDialog.BUTTON_NEGATIVE,
                getResources().getString(R.string.btn_no),
                new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int which) {
                    }
                });
        msgBox.show();
    }

}