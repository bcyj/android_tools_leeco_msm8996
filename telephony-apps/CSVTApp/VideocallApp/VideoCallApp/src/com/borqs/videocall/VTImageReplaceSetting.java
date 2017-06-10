/*
 * ©2012-2014 Borqs Software Solutions Pvt Ltd. All rights reserved.
 */

package com.borqs.videocall;

import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.nio.ByteBuffer;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.ActivityNotFoundException;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.ContentResolver;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.SharedPreferences;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.drawable.BitmapDrawable;
import android.os.Bundle;
import android.provider.Settings;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.Toast;
import android.net.Uri;
import android.widget.ArrayAdapter;

import android.view.Gravity;
import android.view.LayoutInflater;
import android.widget.TextView;
import android.widget.PopupMenu;
import android.widget.PopupMenu.OnMenuItemClickListener;

import android.view.MenuInflater;
import android.view.View.OnClickListener;


public class VTImageReplaceSetting extends Activity
{
    //VTSCListView mListView;
    public static final String PREFS_NAME = "VTImageReplaceSetting_PREF";
    public static final String CURRENT_IMAGE_ID = "CurrentImageID";

    private static final int FALL_BACK_SETTING_MENU_RESET = 1;
	private static final int FALL_BACK_SETTING_MENU_PICK = 2;

    private static final int mDefaultImageWidth = 176;
    private static final int mDefaultImageHeight = 144;
    private static final int mDefaultImageSize = mDefaultImageWidth * mDefaultImageHeight * 4;

    public static final String mStrFN = "VTImg.raw";
    public static final String mStrFN1 = "VTImg2.raw";
    private static final String mStrPicturePathKeyName = "VTReplacePicPath";
    private String mStrPicturePath = null;
    private static int PICK_PHOTO_ACTIVITY_ID = 100;
	private LayoutInflater layoutInflater;

    ImageView mImageView;
    private View mTriangleAffordance;
    private int mSettingVal;

    static final int SETTING_REPLACE_ENABLE = 1;
    static final int SETTING_REPLACE_DISABLE = 0;
    static final int SETTING_REPLACE_NULL = -1;

    static String TAG = "VTImageReplaceSetting";
    private Context mContext;
    public static final String SELECTABLE_ACTION =
        "com.borqs.videocall.VTImageReplaceSetting.selectable";

    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        mContext = this;
        setContentView(R.layout.img_replace_setting);
        mTriangleAffordance = findViewById(R.id.photo_triangle_affordance);
        mImageView = (ImageView)findViewById( R.id.photoImage);

        // If app's status is connected, do not allow user to set the alternative picture.
        VideoCallApp app = VideoCallApp.getInstance();
        boolean btnEnabled = true;
        if (app.getStatus() == VideoCallApp.APP_STATUS_CONNECTED) {
            btnEnabled = false;
        }

        // If the mSettingVal equals null, it indicates that master reset is performed.
        // As the master reset will clean up the mStrPicturePathKeyName item.
        final ContentResolver cr = getContentResolver();
        mStrPicturePath = Settings.System.getString(cr, mStrPicturePathKeyName);

        mImageView.setEnabled(btnEnabled);
        mImageView.setOnClickListener(new View.OnClickListener() {
			public void onClick(View view) {
                showPopupMenu();
            }
        });

		mTriangleAffordance.setEnabled(btnEnabled);
		mTriangleAffordance.setOnClickListener(new View.OnClickListener() {
		    public void onClick(View view) {
                showPopupMenu();
            }
        });

        LoadImage();
        IntentFilter filter = new IntentFilter();
        filter.addAction(SELECTABLE_ACTION);
        registerReceiver(mSelectableListener, filter);
    }

    private void showPopupMenu() {
	    PopupMenu popMenu = new PopupMenu(this, mImageView);
		popMenu.inflate(R.menu.photo_replace_menu);
		popMenu.setOnMenuItemClickListener(new OnMenuItemClickListener() {
			public boolean onMenuItemClick(MenuItem item) {
				switch(item.getItemId()) {
				case R.id.photo_reset:
                    Bitmap bm = setDefaultImage(mContext);
                    updateImageView(bm);
                    break;
				case R.id.photo_pick:
                    onPickPhoto(mDefaultImageWidth, mDefaultImageHeight);
                    Toast.makeText(mContext, mContext.getString(R.string.enable_rep_img_prompt), Toast.LENGTH_SHORT).show();
                    Settings.System.putString(getContentResolver(), mStrPicturePathKeyName, mStrFN);
                    mStrPicturePath = mStrFN;
                    mSettingVal = SETTING_REPLACE_ENABLE;
                    LoadImage();
				    break;
				}
				return false;
			}
		});
        popMenu.show();
    }

    private BroadcastReceiver mSelectableListener = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            if (action.equals(SELECTABLE_ACTION)) {
                boolean selectable = (intent.getBooleanExtra("selectable", false) == true);
                setButtonEnabled(selectable);
            }
        }
    };

    private void setButtonEnabled(boolean selectable) {
      //  mBtnSelectImage = (Button) findViewById(R.id.btn_select_image);
      //  if (mBtnSelectImage != null) mBtnSelectImage.setEnabled(selectable);
        if (mImageView != null) mImageView.setEnabled(selectable);

       // mBtnEnableReplaceImg = (Button) findViewById(R.id.btn_enable);
       // if (mBtnEnableReplaceImg != null) mBtnEnableReplaceImg.setEnabled(selectable);
    }

    private void LoadImage() {
        Bitmap bm;

        if (mStrPicturePath == null) {
            bm = setDefaultImage( this);
        } else {
            bm = queryImage();
            if( bm == null){
                if (MyLog.DEBUG) MyLog.d(TAG, "failed to query Image, and restore to default image.");
                bm = setDefaultImage( this);
                //mSettingVal = SETTING_REPLACE_ENABLE;
                //mBtnEnableReplaceImg.setText(R.string.disable_rep_img);
            }
        }
        updateImageView(bm);
    }

    @Override
	public boolean onCreateOptionsMenu(Menu menu) {
		// TODO Auto-generated method stub
	menu.add(0, FALL_BACK_SETTING_MENU_RESET, 0, R.string.image_replace_setting_reset).setIcon(R.drawable.cmcc_toolbar_resetpicture);
	   //return super.onCreateOptionsMenu(menu);
		return false;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {

        switch (item.getItemId()) {
            case FALL_BACK_SETTING_MENU_RESET:
            {
                Bitmap bm = setDefaultImage( this);
                updateImageView(bm);
            }
            break;
        }
        return super.onOptionsItemSelected(item);
    }

    public static String getCurrentReplaceImagePath( Context ctx){
	if (MyLog.DEBUG) MyLog.d(TAG, "getCurrentReplaceImagePath.");

        final ContentResolver cr = ctx.getContentResolver();
        String value = Settings.System.getString(cr, mStrPicturePathKeyName);
	if (MyLog.DEBUG) MyLog.d(TAG, "getCurrentReplaceImagePath: "+value);
		if(value == null || value.equals("disable")){
			return null;
		}
        return  ctx.getFilesDir().getPath()+"/"+value;
    }

    @Override
	protected void onResume() {
		// TODO Auto-generated method stub
		super.onResume();
	}

	@Override
    protected void onStop(){
       super.onStop();
    }

	@Override
	protected void onDestroy() {
		super.onDestroy();
		unregisterReceiver(mSelectableListener);
	}

    public void onPickPhoto( int width, int height){
	    //Intent intent = new Intent(Intent.ACTION_GET_CONTENT, null);
		Intent intent = new Intent(Intent.ACTION_PICK,
		                           android.provider.MediaStore.Images.Media.EXTERNAL_CONTENT_URI);
	    // TODO: get these values from constants somewhere
	    intent.setType("image/*");
	    intent.putExtra("crop", "true");
	    intent.putExtra("aspectX", width);
	    intent.putExtra("aspectY", height);
	    intent.putExtra("outputX", width);
	    intent.putExtra("outputY", height);

	    try {
		intent.putExtra("return-data", true);
		startActivityForResult(intent, PICK_PHOTO_ACTIVITY_ID);
	    }
	    catch (ActivityNotFoundException e) {
		new AlertDialog.Builder( this)
		.setTitle(R.string.image_replace_errorDialogTitle)
		.setMessage(R.string.image_replace_photoPickerNotFoundText)
		.setPositiveButton(R.string.image_replace_okButtonText, null)
		.show();
	    }
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {

	if (MyLog.DEBUG) MyLog.d( TAG, "onActivityResult, return "+ resultCode + ". Id, " + requestCode);
        if (resultCode != RESULT_OK) {
            return;
        }

        BitmapFactory.Options options = new BitmapFactory.Options();
        options.inScaled = false;
        options.inPreferredConfig = Bitmap.Config.RGB_565;
        options.inDither = true;
        Uri uri = data.getData();
        Bitmap ret = null;
        int id = requestCode;

        final Bundle extras = data.getExtras();
        if (extras != null) {
		ret = extras.getParcelable("data");
        }
        if(ret == null &&  uri != null && uri.toString().startsWith("file://")){

            ret = BitmapFactory.decodeFile(uri.getPath(), options);
        }
        if (ret != null) {
            updateImageView( ret);
            setImage(this, ret);
        } else {
            // Failed to set alternative picture.
            Toast.makeText(mContext, R.string.image_replace_setting_failure, Toast.LENGTH_LONG).show();
        }
    }

    //the first time to set the default image as the replace image
    public static Bitmap setDefaultImage( Context ctx){
	BitmapDrawable bm = (BitmapDrawable)ctx.getResources().getDrawable( R.drawable.cmcc_bg_nocamera);

	setImage(ctx, bm.getBitmap());
	//save the state
	//SharedPreferences settings = ctx.getSharedPreferences(PREFS_NAME, 0);
	//SharedPreferences.Editor editor = settings.edit();
	//editor.putInt(CURRENT_IMAGE_ID, SETTING_REPLACE_ENABLE);
      // Don't forget to commit your edits!!!
	//editor.commit();

	return bm.getBitmap();
    }

    private static void setImage(Context ctx, Bitmap bm){
	FileOutputStream output = null;

        if (bm == null) return;

	try{
                // work-around: the crop image return size is larger than request size, hence scale it.
                bm = Bitmap.createScaledBitmap(bm, mDefaultImageWidth, mDefaultImageHeight, true);
                output = ctx.openFileOutput( mStrFN, 1);
		ByteBuffer buffer = ByteBuffer.allocate(mDefaultImageSize);
		//mBitmap.copyPixelsToBuffer( buffer);
		//ByteBuffer buffer = ByteBuffer.allocate( 4);
		int pixel;
		final int h = ( mDefaultImageHeight<=bm.getHeight())?mDefaultImageHeight:bm.getHeight();
		final int w = ( mDefaultImageWidth<=bm.getWidth())?mDefaultImageWidth:bm.getWidth();

		for( int y=0; y<h; y++){
			for( int x=0; x<w; x++){
				pixel = bm.getPixel(x, y);
				buffer.putInt( pixel);
			}
		}
		output.write(buffer.array());
		output.close();
	}catch( FileNotFoundException e){
		Log.e(TAG, "Failed to open file to write." + e.getMessage());
	}catch( IOException e){
		Log.e(TAG, "Failed to write image data." + e.getMessage());
	}
    }

    public void updateImageView(Bitmap bm){

		if( bm != null){
			mImageView.setImageBitmap( bm);
		}
    }

    private Bitmap queryImage(){

	try{
		byte[] buffer = new byte[mDefaultImageSize];
		FileInputStream input;

            input = this.openFileInput( mStrFN);

		int ns = input.read(buffer);
		if (MyLog.DEBUG) MyLog.d(TAG, "Read bytes " + ns);
		if( ns != mDefaultImageSize){
			this.deleteFile( mStrFN);
			if (MyLog.DEBUG) MyLog.d(TAG, "exit checkItemImage because the wrong file size.");
			return null;
		}
		//to update the image content
		ByteBuffer btBuf= ByteBuffer.wrap(buffer);

		Bitmap bm = Bitmap.createBitmap( mDefaultImageWidth, mDefaultImageHeight, Bitmap.Config.ARGB_8888);
		for( int y=0; y<mDefaultImageHeight; y++)
			for( int x=0; x<mDefaultImageWidth; x++)
				bm.setPixel(x, y, btBuf.getInt());
		input.close();
		return bm;
		}catch( FileNotFoundException e){
			if (MyLog.DEBUG) MyLog.d(TAG, "File not found, " + e.getMessage());
		}catch( IOException eio){
			if (MyLog.DEBUG) MyLog.d(TAG, "Failed to read, " + eio.getMessage());
		}
		return null;
    }

}
