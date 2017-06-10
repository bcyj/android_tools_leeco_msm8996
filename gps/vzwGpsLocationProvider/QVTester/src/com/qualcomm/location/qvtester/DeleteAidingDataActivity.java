package com.qualcomm.location.qvtester;

import android.content.Context;
import android.content.Intent;
import android.location.LocationManager;
import android.os.Bundle;
import android.util.Log;

public class DeleteAidingDataActivity extends AidingDataActivity {
	private static final String TAG = "DeleteAidingDataActivity";
	static final String AIDING_SELECTED = "Aiding Data Deleted";
	static final int AIDING_DELETE_REQUEST = DeleteAidingDataActivity.class.hashCode();
	private static final String[] AIDINGS =
	{
		"ephemeris",
		"almanac",
		"position",
		"time",
		"iono",
		"utc",
		"health",
		"svdir",
		"svsteer",
		"sadata",
		"rti",
		"celldb-info",
		"almanac-corr",
		"freq-bias-est",
		"ephemeris-GLO",
		"almanac-GLO",
		"svdir-GLO",
		"svsteer-GLO",
		"almanac-corr-GLO",
		"time-gps",
		"time-GLO"
	};
	
	private void deleteSelectedAiding() {
		int selectedAiding = getCheckSelections();

		if (0 != selectedAiding) {
			Bundle extras = new Bundle();
		
			for (int i = 0; i < AIDINGS.length && 0 != selectedAiding; i++) {
				int bit = 1<<i;
				if (bit == (bit & selectedAiding)) {
					extras.putBoolean(AIDINGS[i], true);
				}
			}
			
			LocationManager service = (LocationManager)getSystemService(Context.LOCATION_SERVICE);
			service.sendExtraCommand(LocationManager.GPS_PROVIDER, "delete_aiding_data", extras);
		}
	}
	
	void finishAndReport() {
		deleteSelectedAiding();
    	setResult(RESULT_OK, null);
    	finish();
	}
}
