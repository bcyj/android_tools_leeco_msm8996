package com.qualcomm.location.qvtester;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.CheckBox;

public class AidingDataActivity extends Activity {
	private static final String TAG = "AidingDataActivity";
	static final String AIDING_SELECTED = "Aiding Data Selected";
	static final int AIDING_SELECT_REQUEST = AidingDataActivity.class.hashCode();
	
	private AidingDataActivity mMe;
	
	private Button mButtonAllAiding;
	private Button mButtonDone;
	
	private CheckBox mCheckBox0;
	private CheckBox mCheckBox1;
	private CheckBox mCheckBox2;
	private CheckBox mCheckBox3;
	private CheckBox mCheckBox4;
	private CheckBox mCheckBox5;
	private CheckBox mCheckBox6;
	private CheckBox mCheckBox7;
	private CheckBox mCheckBox8;
	private CheckBox mCheckBox9;
	private CheckBox mCheckBox10;
	private CheckBox mCheckBox11;
	private CheckBox mCheckBox12;
	private CheckBox mCheckBox13;
	private CheckBox mCheckBox14;
	private CheckBox mCheckBox15;
	private CheckBox mCheckBox16;
	private CheckBox mCheckBox17;
	private CheckBox mCheckBox18;
	private CheckBox mCheckBox19;
	private CheckBox mCheckBox20;

	/** Called when the activity is first created. */
	@Override
	public void onCreate(Bundle savedInstanceState) {
	    super.onCreate(savedInstanceState);
	    
	    mMe = this;
	
	    this.setContentView(R.layout.aiding_data);
	    
	    mButtonAllAiding = (Button) findViewById(R.id.ButtonAllAiding);
	    mButtonAllAiding.setOnClickListener(
	    		new SelectButtonListener(mButtonAllAiding, "Select All Aiding", "Deselect All Aiding", false));
	    
	    mButtonDone = (Button) findViewById(R.id.ButtonDone);
	    mButtonDone.setOnClickListener(new OnClickListener() {
			@Override
			public void onClick(View v) {
				mMe.finishAndReport();
			}
	    });
	    
	    mCheckBox0 = (CheckBox) findViewById(R.id.checkBox0);
	    mCheckBox1 = (CheckBox) findViewById(R.id.checkBox1);
	    mCheckBox2 = (CheckBox) findViewById(R.id.checkBox2);
	    mCheckBox3 = (CheckBox) findViewById(R.id.checkBox3);
	    mCheckBox4 = (CheckBox) findViewById(R.id.checkBox4);
	    mCheckBox5 = (CheckBox) findViewById(R.id.checkBox5);
	    mCheckBox6 = (CheckBox) findViewById(R.id.checkBox6);
	    mCheckBox7 = (CheckBox) findViewById(R.id.checkBox7);
	    mCheckBox8 = (CheckBox) findViewById(R.id.checkBox8);
	    mCheckBox9 = (CheckBox) findViewById(R.id.checkBox9);
	    mCheckBox10 = (CheckBox) findViewById(R.id.checkBox10);
	    mCheckBox11 = (CheckBox) findViewById(R.id.checkBox11);
	    mCheckBox12 = (CheckBox) findViewById(R.id.checkBox12);
	    mCheckBox13 = (CheckBox) findViewById(R.id.checkBox13);
	    mCheckBox14 = (CheckBox) findViewById(R.id.checkBox14);
	    mCheckBox15 = (CheckBox) findViewById(R.id.checkBox15);
	    mCheckBox16 = (CheckBox) findViewById(R.id.checkBox16);
	    mCheckBox17 = (CheckBox) findViewById(R.id.checkBox17);
	    mCheckBox18 = (CheckBox) findViewById(R.id.checkBox18);
	    mCheckBox19 = (CheckBox) findViewById(R.id.checkBox19);
	    mCheckBox20 = (CheckBox) findViewById(R.id.checkBox20);
	}
	
	int getCheckSelections() {
		int total = 0;
		if (mCheckBox0.isChecked()) total |= 1<<0;
		if (mCheckBox1.isChecked()) total |= 1<<1;
		if (mCheckBox2.isChecked()) total |= 1<<2;
		if (mCheckBox3.isChecked()) total |= 1<<3;
		if (mCheckBox4.isChecked()) total |= 1<<4;
		if (mCheckBox5.isChecked()) total |= 1<<5;
		if (mCheckBox6.isChecked()) total |= 1<<6;
		if (mCheckBox7.isChecked()) total |= 1<<7;
		if (mCheckBox8.isChecked()) total |= 1<<8;
		if (mCheckBox9.isChecked()) total |= 1<<9;
		if (mCheckBox10.isChecked()) total |= 1<<10;
		if (mCheckBox11.isChecked()) total |= 1<<11;
		if (mCheckBox12.isChecked()) total |= 1<<12;
		if (mCheckBox13.isChecked()) total |= 1<<13;
		if (mCheckBox14.isChecked()) total |= 1<<14;
		if (mCheckBox15.isChecked()) total |= 1<<15;
		if (mCheckBox16.isChecked()) total |= 1<<16;
		if (mCheckBox17.isChecked()) total |= 1<<17;
		if (mCheckBox18.isChecked()) total |= 1<<18;
		if (mCheckBox19.isChecked()) total |= 1<<19;
		if (mCheckBox20.isChecked()) total |= 1<<20;
		
		return total;
	}
	
	void finishAndReport() {
    	Intent intent = new Intent();
    	intent.setComponent(getCallingActivity());
    	intent.putExtra(AIDING_SELECTED, getCheckSelections());
    	setResult(RESULT_OK, intent);
    	finish();
	}

	private class SelectButtonListener implements OnClickListener {
		final private Button mButton;
		final private CharSequence mText1;
		final private CharSequence mText2;
		private CharSequence mCurrentText;
		private boolean mCheckState;
		
		private SelectButtonListener(Button self, CharSequence initialText, CharSequence alternativeText, boolean initialCheckState) {
			mButton = self;
			mText1 = initialText;
			mText2 = alternativeText;
			mCurrentText = mText1;
			mCheckState = initialCheckState;
		}
		
		@Override
		public void onClick(View v) {
			mCurrentText = (mCurrentText == mText1) ? mText2 : mText1;
			mCheckState = !mCheckState;
			
			mCheckBox0.setChecked(mCheckState);
			mCheckBox1.setChecked(mCheckState);
			mCheckBox2.setChecked(mCheckState);
			mCheckBox3.setChecked(mCheckState);
			mCheckBox4.setChecked(mCheckState);
			mCheckBox5.setChecked(mCheckState);
			mCheckBox6.setChecked(mCheckState);
			mCheckBox7.setChecked(mCheckState);
			mCheckBox8.setChecked(mCheckState);
			mCheckBox9.setChecked(mCheckState);
			mCheckBox10.setChecked(mCheckState);
			mCheckBox11.setChecked(mCheckState);
			mCheckBox12.setChecked(mCheckState);
			mCheckBox13.setChecked(mCheckState);
			mCheckBox14.setChecked(mCheckState);
			mCheckBox15.setChecked(mCheckState);
			mCheckBox16.setChecked(mCheckState);
			mCheckBox17.setChecked(mCheckState);
			mCheckBox18.setChecked(mCheckState);
			mCheckBox19.setChecked(mCheckState);
			mCheckBox20.setChecked(mCheckState);
			
			mButton.setText(mCurrentText);
		}
	}
}
