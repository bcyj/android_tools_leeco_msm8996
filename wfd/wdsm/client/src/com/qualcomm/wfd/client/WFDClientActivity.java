
/*
* Copyright (c) 2012 - 2014 Qualcomm Technologies, Inc. All Rights Reserved.
* Qualcomm Technologies Confidential and Proprietary.
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

package com.qualcomm.wfd.client;

import android.app.ActionBar;
import android.app.Activity;
import android.app.AlertDialog;
import android.app.ProgressDialog;
import android.content.ActivityNotFoundException;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.preference.PreferenceManager;
import android.content.SharedPreferences;
import android.content.SharedPreferences.OnSharedPreferenceChangeListener;

import android.content.pm.ActivityInfo;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.content.pm.PackageManager.NameNotFoundException;
import android.content.res.Configuration;
import android.graphics.Color;
import android.net.Uri;
import android.net.wifi.p2p.WifiP2pConfig;
import android.net.wifi.p2p.WifiP2pInfo;
import android.net.wifi.p2p.WifiP2pDevice;
import android.net.wifi.p2p.WifiP2pDeviceList;
import android.net.wifi.p2p.WifiP2pManager;
import android.net.wifi.p2p.WifiP2pManager.ActionListener;
import android.net.wifi.p2p.WifiP2pManager.Channel;
import android.net.wifi.p2p.WifiP2pManager.ChannelListener;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.os.RemoteException;
import java.lang.InterruptedException;
import android.util.Log;
import android.view.ContextMenu;
import android.view.ContextMenu.ContextMenuInfo;
import android.view.InputEvent;
import android.view.KeyEvent;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.MotionEvent;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.view.Window;
import android.view.WindowManager;
import android.view.View.OnClickListener;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.AdapterView.OnItemLongClickListener;
import android.widget.Button;
import android.widget.FrameLayout;
import android.widget.FrameLayout.LayoutParams;
import android.widget.LinearLayout;
import android.widget.ListView;
import android.widget.RelativeLayout;
import android.widget.SimpleAdapter;
import android.widget.TableLayout;
import android.widget.TableRow;
import android.widget.TextView;
import android.widget.Toast;
import android.widget.ToggleButton;

import com.qualcomm.wfd.WfdDevice;
import com.qualcomm.wfd.WfdEnums;
import com.qualcomm.wfd.WfdStatus;
import com.qualcomm.wfd.WfdEnums.WFDDeviceType;
import com.qualcomm.wfd.client.DeviceListFragment.DeviceActionListener;
import com.qualcomm.wfd.client.P2pWfdDeviceInfo;
import com.qualcomm.wfd.client.R;
import com.qualcomm.wfd.client.DeviceListFragment.EventHandler;
import com.qualcomm.wfd.client.ServiceUtil.ServiceFailedToBindException;
import com.qualcomm.wfd.client.WfdOperationUtil.WfdOperation;
import com.qualcomm.wfd.client.WfdOperationUtil.WfdOperationTask;
import com.qualcomm.wfd.service.IWfdActionListener;
import static com.qualcomm.wfd.client.WfdOperationUtil.*;
import com.qualcomm.wfd.WfdEnums.AVPlaybackMode;

public class WFDClientActivity extends Activity implements ChannelListener, DeviceActionListener, SharedPreferences.OnSharedPreferenceChangeListener {
	public static final String TAG = "Client.WFDClientActivity";

	private TextView mynameTextView;
	private TextView mymacaddressTextView;
	private TextView mystatusTextView;
	private TableLayout controlbuttonsTableLayout;
	private ToggleButton playpauseButton;
	private ToggleButton standbyresumeButton;
	private Button teardownButton;
	private ToggleButton enableDisableUibcButton;
    private ToggleButton selectTransportButton;
    private ToggleButton flushButton;

	private MenuItem sourceItem;
	private MenuItem sinkItem;
	private Boolean modeSource = true;
	private int localDeviceType = WFDDeviceType.SOURCE.getCode();
	private boolean startingSession = false;
        private boolean mUIBCM14 = false;
	private boolean inSession = false;
	ProgressDialog startSessionProgressDialog = null;
    ProgressDialog ipDialog = null;
	private WifiP2pDevice device;
	private boolean isWifiP2pEnabled = false;
	private boolean retryChannel = false;
	private final IntentFilter intentFilter = new IntentFilter();
	private BroadcastReceiver receiver = null;
	boolean isDiscoveryRequested = false;
	private static WiFiUtil wifiUtil;
	private ClientEventHandler eventHandler;
	private IWfdActionListener mActionListener;
	private SharedPreferences mSharedPrefs = null;
        private WfdDevice localWfdDev = null, peerWfdDev = null;
	private final int START_SURFACE = 1;

	/**
	 * @param isWifiP2pEnabled
	 *            the isWifiP2pEnabled to set
	 */
	public void setIsWifiP2pEnabled(boolean isWifiP2pEnabled) {
		this.isWifiP2pEnabled = isWifiP2pEnabled;
		if (!this.isWifiP2pEnabled) {
		    Log.d(TAG, "Find selected from action bar while p2p off");
			Toast.makeText(WFDClientActivity.this,
					R.string.p2p_off_warning, Toast.LENGTH_SHORT)
					.show();
		}
	}

	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.main);
        	eventHandler = new ClientEventHandler();
		wifiUtil = WiFiUtil.getInstance(this, eventHandler);
		wifiUtil.send_wfd_set(true, WFDDeviceType.getValue(localDeviceType));
		// add necessary intent values to be matched.
		intentFilter.addAction(WifiP2pManager.WIFI_P2P_STATE_CHANGED_ACTION);
		intentFilter.addAction(WifiP2pManager.WIFI_P2P_CONNECTION_CHANGED_ACTION);
		intentFilter.addAction(WifiP2pManager.WIFI_P2P_THIS_DEVICE_CHANGED_ACTION);
		intentFilter.addAction(WifiP2pManager.WIFI_P2P_PEERS_CHANGED_ACTION);

		try {
                    ServiceUtil.bindService(getApplicationContext(), eventHandler);
                } catch (ServiceFailedToBindException e) {
                    Log.e(TAG, "ServiceFailedToBindException received");
                }

        mynameTextView = (TextView) this.findViewById(R.id.tv_my_name);
		mymacaddressTextView = (TextView) this.findViewById(R.id.tv_my_mac_address);
		mystatusTextView = (TextView) this.findViewById(R.id.tv_my_status);
		controlbuttonsTableLayout = (TableLayout) this.findViewById(R.id.tl_control_buttons);
		playpauseButton = (ToggleButton) this.findViewById(R.id.btn_play_pause);
		standbyresumeButton = (ToggleButton) this.findViewById(R.id.btn_standby_resume);
		teardownButton = (Button) this.findViewById(R.id.btn_teardown);
		enableDisableUibcButton = (ToggleButton) this.findViewById(R.id.btn_start_stop_uibc);
        selectTransportButton = (ToggleButton) this.findViewById(R.id.btn_transport_udp_tcp);
        flushButton = (ToggleButton) this.findViewById(R.id.btn_flush);
        mSharedPrefs = PreferenceManager.getDefaultSharedPreferences(this);
        PreferenceManager.setDefaultValues(this, R.xml.preferences, false);
        mSharedPrefs.registerOnSharedPreferenceChangeListener(this);
	}

	@Override
	protected void onDestroy() {
		super.onDestroy();
        if (mSharedPrefs != null)
        { mSharedPrefs.unregisterOnSharedPreferenceChangeListener(this);
          mSharedPrefs = null;
        }
		try {
		    unbindWfdService();
	        } catch (IllegalArgumentException e) {
	            Log.e(TAG,"Illegal Argument Exception ",e);
		}
		Log.d(TAG, "onDestroy() called");
	}

       /** To prevent dynamic configuration changes from destroying activity */
       @Override
	public void onConfigurationChanged (Configuration newConfig) {
	Log.e(TAG, "onConfigurationChanged called due to"+ newConfig.toString());
	super.onConfigurationChanged(newConfig);
       }

	/** register the BroadcastReceiver with the intent values to be matched */
	@Override
	public void onResume() {
        super.onResume();
        Log.d(TAG, "onResume() called");
        receiver = new WiFiDirectBroadcastReceiver(wifiUtil.manager,
        wifiUtil.channel, this);
        registerReceiver(receiver, intentFilter);
        WifiP2pManager.ActionListener p2pStateListener = new WifiP2pManager.ActionListener() {

            @Override
            public void onSuccess() {
                    Log.e(TAG,"WFD Settings Success");
                    Message messageInit = eventHandler.obtainMessage(SET_WFD_FINISHED);
                    messageInit.arg1 = 0;
                    eventHandler.sendMessage(messageInit);
            }

            @Override
            public void onFailure(int reasonCode) {
                    Log.e(TAG,"WFD Settings Failure");
                    Message messageInit = eventHandler.obtainMessage(SET_WFD_FINISHED);
                    messageInit.arg1 = -1;
                    eventHandler.sendMessage(messageInit);
            }
        };
    }

	@Override
	public void onPause() {
		super.onPause();
		unregisterReceiver(receiver);
		Log.d(TAG, "onPause() called");
	}

    @Override
	public void onSharedPreferenceChanged(SharedPreferences sharedPreferences,String key){

        Log.d(TAG, "onSharedPreferenceChanged Listner");

        if (mSharedPrefs == null) {
            Log.e(TAG, "Null objects!");
            return;
        }

        if (key.equals("udp_decoder_latency_value")){

            Log.d(TAG, "decoder latency preference changed");
            int decoderLatency = Integer.parseInt(mSharedPrefs.getString("udp_decoder_latency_value", "5"));

            if (peerWfdDev != null
                    && decoderLatency != peerWfdDev.decoderLatency && inSession) {
                Log.d(TAG, "setting decoder latency");
                startWfdOperationTask(this, WfdOperation.SET_DECODER_LATENCY);
                peerWfdDev.decoderLatency = decoderLatency;
            }
        }
    }

	@Override
	public boolean onPrepareOptionsMenu(Menu menu) {
	    super.onPrepareOptionsMenu(menu);
	    sourceItem = menu.findItem(R.id.menu_select_source);
	    sinkItem = menu.findItem(R.id.menu_select_sink);
	    return true;
	}

    public void playPauseWfdOperation(View view) {
        Log.d(TAG, "playPauseControlOperation() called");
        if (!playpauseButton.isChecked()) {
            Log.d(TAG, "playPauseWfdOperation: Play button clicked");
            playpauseButton.toggle();
            startWfdOperationTask(this, WfdOperation.PLAY);
        } else {
            Log.d(TAG, "playPauseWfdOperation: Pause button clicked");
            playpauseButton.toggle();
            startWfdOperationTask(this, WfdOperation.PAUSE);
        }
    }

    public void standbyResumeWfdOperation(View view) {
        Log.d(TAG, "standbyResumeWfdOperation() called");
        if (!standbyresumeButton.isChecked()) {
            Log.d(TAG, "standbyResumeWfdOperation: Standby button clicked");
            standbyresumeButton.toggle();
            startWfdOperationTask(this, WfdOperation.STANDBY);
        } else {
            Log.d(TAG, "standbyResumeWfdOperation: Resume button clicked");
            standbyresumeButton.toggle();
            startWfdOperationTask(this, WfdOperation.RESUME);
        }
    }

    public void startStopUibcWfdOperation(View view) {
        Log.d(TAG, "startStopUibcControlOperation() called");
        enableDisableUibcButton.toggle();
        if (!mUIBCM14) {
            if (!enableDisableUibcButton.isChecked()) {
                Log.d(TAG, "Start UIBC button clicked");
                startWfdOperationTask(this, WfdOperation.START_UIBC);
            } else {
                Log.d(TAG, "Stop UIBC button clicked");
                startWfdOperationTask(this, WfdOperation.STOP_UIBC);
            }
        } else {
            int ret = -1;
            try {
                ret = ServiceUtil.getInstance().setUIBC();
            } catch (RemoteException e) {
                e.printStackTrace();
            }
            Toast.makeText(getApplicationContext(), "UIBC set: " + ret,
                    Toast.LENGTH_SHORT).show();
            if (0 == ret) {
                enableDisableUibcButton.setText("Start UIBC");
                mUIBCM14 = false;
            } else {
                enableDisableUibcButton.setText("Send M14");
                mUIBCM14 = true;
            }
        }
    }


    public void toggleRTPTransport(View view) {
        Log.d(TAG, "toggleRTPTransport() called");
        if (ServiceUtil.getmServiceAlreadyBound()) {
            try {
                WfdStatus wfdStatus = ServiceUtil.getInstance().getStatus();
                if (wfdStatus.state != WfdEnums.SessionState.PLAY.ordinal()) {
                    Log.d(TAG, "Not allowed in non-PLAY state");
                    selectTransportButton.toggle();
                    Toast.makeText(getApplicationContext(),
                            "Not honouring TCP/UDP switch in non-PLAY state",
                            Toast.LENGTH_SHORT).show();
                    return;
                }
            } catch (RemoteException e) {
                e.printStackTrace();
            }
        }
        if (selectTransportButton.isChecked()) {
            Log.d(TAG, "Start TCP Transport");
            startWfdOperationTask(this, WfdOperation.SELECT_TCP);
            selectTransportButton.setText("UDP");
        } else {
            Log.d(TAG, "Start UDP Transport");
            startWfdOperationTask(this, WfdOperation.SELECT_UDP);
            selectTransportButton.setText("TCP");
        }
    }

    public void flushOperation(View view) {
        Log.d(TAG, "flushOperation() called");
        startWfdOperationTask(this, WfdOperation.FLUSH);
    }

    public void teardownWfdOperation(View view) {
        Log.d(TAG, "Teardown button clicked");
        WfdOperationTask task = new WfdOperationTask(this, "Tearing Down WFD Session", WfdOperation.TEARDOWN);
        task.execute();
    }

	private class PreTeardownTaskSessionCheck extends AsyncTask<Void, Void, Boolean> {
        @Override
        protected Boolean doInBackground(Void... params) {
            try {
                //verify session state
                WfdStatus wfdStatus = ServiceUtil.getInstance().getStatus();
                Log.d(TAG, "wfdStatus.state= " + wfdStatus.state);
                if (wfdStatus.state == WfdEnums.SessionState.INVALID.ordinal() ||
                        wfdStatus.state == WfdEnums.SessionState.INITIALIZED.ordinal()) {
                    Log.d(TAG, "wfdStatus.state= " + wfdStatus.state);
                    return false;
                } else {
                    return true;
                }
            } catch (RemoteException e) {
                Log.e(TAG, "PreTeardownTaskSessionCheck- Remote exception", e);
            }
            return true;
        }

        @Override
        protected void onPostExecute(Boolean callTeardown) {
            Log.d(TAG, "PreTeardownTaskSessionCheck- onPostExecute");
            Log.d(TAG, "PreTeardownTaskSessionCheck- callTeardown: " + callTeardown);
            if (callTeardown) {
                Log.d(TAG, "PreTeardownTaskSessionCheck- now calling teardown");
            } else {
                Log.d(TAG, "PreTeardownTaskSessionCheck- not in a WFD session, not going to call teardownOperation");
            }
        }
    }


	public class StartSessionTask extends AsyncTask<WifiP2pDevice, Void, Integer> {

        AlertDialog.Builder builder;
        Boolean showBuilder = false;

		@Override
		protected Integer doInBackground(WifiP2pDevice... devices) {
			Log.d(TAG, "StartSessionTask: doInBackground called");
			int ret = 0;
			WifiP2pDevice peerDevice = devices[0];
			if (peerDevice.wfdInfo == null) {
				Log.e(TAG, "StartSessionTask: doInBackground- Device is missing wfdInfo");
				Message messageInit = eventHandler.obtainMessage(INVALID_WFD_DEVICE);
				eventHandler.sendMessage(messageInit);
				return ret;
			}

			try {
				Log.d(TAG, "StartSessionTask- doInBackground- Setting surface to a surface");
				if (WFDClientActivity.this.device == null) {
                    builder = new AlertDialog.Builder(WFDClientActivity.this);
                    builder.setTitle("Start session cannot proceed")
                         .setMessage("This device is null")
                         .setPositiveButton("Ok", new DialogInterface.OnClickListener() {
                         public void onClick(DialogInterface dialog, int id) {
                             return;
                         }
                    });
				    this.showBuilder = true;
				    return ret;
				}
                int initReturn = ServiceUtil.getInstance().init(
                        WFDClientActivity.this.mActionListener, localWfdDev);
	            if (!(initReturn == 0 || initReturn == WfdEnums.ErrorType.ALREADY_INITIALIZED.getCode())) {
                    Log.e(TAG, "StartSessionTask- doInBackground: init failed with error- " + initReturn);
                }
                if (peerWfdDev.ipAddress == null && modeSource == false) {
				    Log.e(TAG, "StartSessionTask- doInBackground: ipaddress null pop up");
                    builder = new AlertDialog.Builder(WFDClientActivity.this);
		            builder.setTitle("Start session cannot proceed")
    		            .setMessage("Unable to obtain peer ip address")
    		            .setPositiveButton("Ok", new DialogInterface.OnClickListener() {
    		                public void onClick(DialogInterface dialog, int id) {
    		                    return;
    		                }
    		            });
                    showBuilder = true;
		            return ret;
				}
                ret = ServiceUtil.getInstance().startWfdSession(peerWfdDev);
			} catch (RemoteException e) {
				Log.e(TAG, "Remote exception", e);
			}
			return ret;
		}

		@Override
		protected void onPreExecute() {
			Log.d(TAG, "StartSessionTask- onPreExecute() called");
			startSessionProgressDialog = ProgressDialog.show(WFDClientActivity.this,
					"Starting WFD Session", "Press back to cancel", true, true);

			startSessionProgressDialog.setCancelable(true);
			startSessionProgressDialog.setOnCancelListener(new  DialogInterface.OnCancelListener() {
				@Override
				public void onCancel(DialogInterface dialog) {
					Log.d(TAG, "StartSessionTask- onCancel called before");
					cancel(true);
					try {
                                            if(ServiceUtil.getmServiceAlreadyBound()) {
                                               ServiceUtil.getInstance().teardown();
                                            }
					} catch (RemoteException e) {
						Log.e(TAG, "RemoteException in teardown");
					}
					Log.d(TAG, "StartSessionTask- onCancel called after");
				}
			});

                	if (!modeSource) {
                   		showHideWfdSurface();
                   		Log.d(TAG, "StartSessionTask: onPreExecute- device is sink, showHideWfdSurface");
                	}

	        	Log.d(TAG, "StartSessionTask: onPreExecute- end");
		};

		@Override
		protected void onCancelled() {
            		Log.d(TAG, "StartSessionTask: onCancelled called because" +
				" startSessionTask returned");
		}

		@Override
		protected void onPostExecute(Integer sessionId) {
            Log.d(TAG, "StartSessionTask: onPostExecute() called");

            Boolean startSessionSuccess = true;
            if (showBuilder) {
                Log.d(TAG, "StartSessionTask: onPostExecute- showBuilder = true, showing unable to obtain peer ip address alert");
                startSessionSuccess = false;
                AlertDialog alert = builder.create();
                alert.show();
                Message messageFailSession = eventHandler.obtainMessage(CLEAR_UI);
                eventHandler.sendMessage(messageFailSession);
            }

			if (sessionId < 0) {
                Log.d(TAG, "StartSessionTask: onPostExecute- sessionId < 0, Failed to start session with error: " + sessionId);
				Toast.makeText(WFDClientActivity.this,
						"Failed to start session with error: " + sessionId,
						1500).show();
				return;
			} else if (sessionId == WfdEnums.ErrorType.OPERATION_TIMED_OUT.getCode()) {
                Log.d(TAG, "StartSessionTask: onPostExecute- Start session is taking longer than expected");
				AlertDialog.Builder builder = new AlertDialog.Builder(WFDClientActivity.this);
				builder.setTitle("Start session is taking longer than expected")
					   .setMessage("Would you like to continue waiting?")
				       .setPositiveButton("Yes", new DialogInterface.OnClickListener() {
				           public void onClick(DialogInterface dialog, int id) {
                               Log.d(TAG, "StartSessionTask: onPostExecute- User clicked yes (would like to continue waiting)");
				        	   return;
				           }
				       })
				       .setNegativeButton("No", new DialogInterface.OnClickListener() {
				           public void onClick(DialogInterface dialog, int id) {
                                Log.d(TAG, "StartSessionTask: onPostExecute- User clicked no (would not like to continue waiting)");
                                teardownWfdOperation(null);
				           }
				       });
				AlertDialog alert = builder.create();
				alert.show();
            } else if (startSessionSuccess) {
                Log.d(TAG, "StartSessionTask: completed successfully.");
                inSession = true;
			} else {
                Log.d(TAG, "StartSessionTask: completed unsuccessfully.");
			}
		}
	}

	public void connectClickHandler(View v) {
		Log.d(TAG, "connectClickHandler received click");
		DeviceListFragment fragmentList = (DeviceListFragment) getFragmentManager()
				.findFragmentById(R.id.frag_list);
		fragmentList.handleConnectClick(v);
	}

	public void connectOptionsClickHandler(View v) {
		Log.d(TAG, "connectOptionsClickHandler received click");
		DeviceListFragment fragmentList = (DeviceListFragment) getFragmentManager()
				.findFragmentById(R.id.frag_list);
		fragmentList.handleConnectOptionsClick(v);
	}

	public void disconnectClickHandler(View v) {
		Log.d(TAG, "disconnectClickHandler received click");
		disconnect();
	}

	public void startClientClickHandler(View v) {
		Log.d(TAG, "startClientClickHandler received click");
		DeviceListFragment fragmentList = (DeviceListFragment) getFragmentManager()
				.findFragmentById(R.id.frag_list);
		fragmentList.handleStartClientClick(v);
	}

	public void filterClickHandler(View V) {
		Log.d(TAG, "filterClickHandler received click");
		DeviceListFragment fragmentList = (DeviceListFragment) getFragmentManager()
			.findFragmentById(R.id.frag_list);
		fragmentList.handleFilterClick();
	}

	/**
	 * Remove all peers and clear all fields. This is called on
	 * BroadcastReceiver receiving a state change event.
	 */
	public void resetData() {
		Log.d(TAG, "resetData() called");
		DeviceListFragment fragmentList = (DeviceListFragment) getFragmentManager()
				.findFragmentById(R.id.frag_list);
		if (fragmentList != null) {
			fragmentList.clearPeers();
		}
	}

	@Override
	public void connect(WifiP2pConfig config, final WifiP2pDevice device) {
		Log.d(TAG, "connect(,) called");
		wifiUtil.manager.connect(wifiUtil.channel, config,
				new ActionListener() {

					@Override
					public void onSuccess() {
						final DeviceListFragment fragment = (DeviceListFragment)
					 		getFragmentManager().findFragmentById(R.id.frag_list);
						fragment.setConnectedDevice(device);
					}

					@Override
					public void onFailure(int reason) {
						Toast.makeText(WFDClientActivity.this,
								"Connect failed. Retry.", Toast.LENGTH_SHORT)
								.show();
					}
				});
	}

	public void disconnect() {
	    Log.d(TAG, "disconnect() called");
	    wifiUtil.manager.removeGroup(wifiUtil.channel, new ActionListener() {

	        @Override
	        public void onFailure(int reasonCode) {
	            Log.d(TAG, "Disconnect failed. Reason :" + reasonCode);

	        }

	        @Override
	        public void onSuccess() {
	        }

	    });
	}

	@Override
	public void onChannelDisconnected() {
		Log.d(TAG, "onChannelDisconnected() called");
		// we will try once more
		if (wifiUtil.manager != null && !retryChannel) {
			Toast.makeText(this, "Channel lost. Trying again",
					Toast.LENGTH_LONG).show();
			resetData();
			retryChannel = true;
			wifiUtil.manager.initialize(this, getMainLooper(), this);
		} else {
			Toast.makeText(
					this,
					"Severe! Channel is probably lost premanently. Try Disable/Re-Enable P2P.",
					Toast.LENGTH_LONG).show();
		}
	}

	@Override
	public void cancelDisconnect() {
		/*
		 * A cancel abort request by user. Disconnect i.e. removeGroup if
		 * already connected. Else, request WifiP2pManager to abort the ongoing
		 * request
		 */
		Log.d(TAG, "cancelDisconnect() called");
		if (wifiUtil.manager != null) {
			if (this.device == null ||
				this.device.status == WifiP2pDevice.CONNECTED) {
				disconnect();
			} else if (this.device.status == WifiP2pDevice.AVAILABLE
					|| this.device.status == WifiP2pDevice.INVITED) {

				wifiUtil.manager.cancelConnect(wifiUtil.channel,
						new ActionListener() {

							@Override
							public void onSuccess() {
								Toast.makeText(WFDClientActivity.this,
										"Aborting connection",
										Toast.LENGTH_SHORT).show();
							}

							@Override
							public void onFailure(int reasonCode) {
								Toast.makeText(
										WFDClientActivity.this,
										"Connect abort request failed. Reason Code: "
												+ reasonCode,
										Toast.LENGTH_SHORT).show();
							}
						});
			}
		}
	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		MenuInflater inflater = getMenuInflater();
		inflater.inflate(R.menu.main_menu, menu);
		return true;
	}

	@Override
	public boolean onOptionsItemSelected(MenuItem item) {
		// Handle item selection
		switch (item.getItemId()) {
		/*case R.id.menu_advanced_settings:
			startActivity(new Intent(this, AdvancedPreferences.class));
			break;*/
		case R.id.menu_preferences:
			Log.d(TAG, "prefences clicked");
			startActivity(new Intent(this, Preferences.class));
			break;
		default:
			Log.d(TAG, "No menu item selected");
		}
		return super.onOptionsItemSelected(item);
	}

	public void onOptionSelectSourceOrSink(MenuItem i) {
		Log.d(TAG, "Source or Sink selected from action bar");
		if (R.id.menu_select_source == i.getItemId()) {
			Log.d(TAG, "Source selected from action bar");
			localDeviceType = WFDDeviceType.SOURCE.getCode();
			if (ServiceUtil.getmServiceAlreadyBound()) {
				try {
					ServiceUtil.getInstance().setDeviceType(localDeviceType);
				} catch (RemoteException e) {
                                        Log.e(TAG, "Remote exception when setting device type", e);
				}
			} else {
				try {
					ServiceUtil.bindService(getApplicationContext(), eventHandler);
				} catch (ServiceFailedToBindException e) {}
			}
                     	wifiUtil.send_wfd_set(true, WFDDeviceType.SOURCE);
			i.setTitle(R.string.menu_source_selected);
                	modeSource = true;
                	Toast.makeText(WFDClientActivity.this,
                       		       "This device is now Source",
                        		Toast.LENGTH_SHORT).show();
            		sinkItem.setTitle(R.string.menu_sink);
		} else {
			Log.d(TAG, "Sink selected from action bar");
			localDeviceType = WFDDeviceType.PRIMARY_SINK.getCode();
			if (ServiceUtil.getmServiceAlreadyBound()) {
				try {
					ServiceUtil.getInstance().setDeviceType(localDeviceType);
				} catch (RemoteException e) {
                    Log.e(TAG, "Remote exception when setting device type", e);
				}
			} else {
				try {
					ServiceUtil.bindService(getApplicationContext(), eventHandler);
				} catch (ServiceFailedToBindException e) {}
			}
                        wifiUtil.send_wfd_set(true, WFDDeviceType.PRIMARY_SINK);
                	i.setTitle(R.string.menu_sink_selected);
                	modeSource = false;
                	Toast.makeText(WFDClientActivity.this,
                       	"This device is now Sink",
                       	Toast.LENGTH_SHORT).show();
			sourceItem.setTitle(R.string.menu_source);
		}
	}

	public void onOptionFind(MenuItem i) {
                try {
                        ServiceUtil.bindService(getApplicationContext(), eventHandler);
                } catch (ServiceFailedToBindException e) {
                        Log.e(TAG, "ServiceFailedToBindException received");
                }
		if (!isWifiP2pEnabled) {
			Log.d(TAG, "Find selected from action bar while p2p off");
			Toast.makeText(WFDClientActivity.this,
					R.string.p2p_off_warning, Toast.LENGTH_SHORT)
					.show();
		} else {
			Log.d(TAG, "Find selected from action bar");
			isDiscoveryRequested = true;
			final DeviceListFragment fragment = (DeviceListFragment) getFragmentManager()
					.findFragmentById(R.id.frag_list);
			fragment.onInitiateDiscovery();
			wifiUtil.manager.discoverPeers(wifiUtil.channel,
					new WifiP2pManager.ActionListener() {

						@Override
						public void onSuccess() {
							Toast.makeText(WFDClientActivity.this,
									"Discovery Initiated",
									Toast.LENGTH_SHORT).show();
						}

						@Override
						public void onFailure(int reasonCode) {
							Toast.makeText(WFDClientActivity.this,
									"Discovery Failed : " + reasonCode,
									Toast.LENGTH_SHORT).show();
						}
					});
		}
	}

	public void onOptionCreateGroup(MenuItem i) {
		if (!isWifiP2pEnabled) {
			Log.d(TAG, "Create Group selected from menu while p2p off");
			Toast.makeText(WFDClientActivity.this, R.string.p2p_off_warning,
					Toast.LENGTH_SHORT).show();
        } else {
			Log.d(TAG, "Create Group selected from menu");
			wifiUtil.manager.createGroup(wifiUtil.channel, new WifiP2pManager.ActionListener
					() {
				@Override
				public void onSuccess() {
					Toast.makeText(WFDClientActivity.this, "Group created",
							Toast.LENGTH_SHORT).show();
				}

				@Override
				public void onFailure(int reasonCode) {
                            Toast.makeText(WFDClientActivity.this,
                                    "Group create failed : " + reasonCode,
							Toast.LENGTH_SHORT).show();
				}
			});
		}
	}

	public void onOptionRemoveGroup(MenuItem i) {
		if (!isWifiP2pEnabled) {
			Log.d(TAG, "Remove Group selected from menu while p2p off");
			Toast.makeText(WFDClientActivity.this, R.string.p2p_off_warning,
					Toast.LENGTH_SHORT).show();
        } else {
			Log.d(TAG, "Remove Group selected from menu");
			wifiUtil.manager.removeGroup(wifiUtil.channel, new WifiP2pManager.ActionListener
					() {
				@Override
				public void onSuccess() {
					Toast.makeText(WFDClientActivity.this, "Group removed",
							Toast.LENGTH_SHORT).show();
				}

				@Override
				public void onFailure(int reasonCode) {
                            Toast.makeText(WFDClientActivity.this,
                                    "Group remove failed : " + reasonCode,
							Toast.LENGTH_SHORT).show();
				}
			});
		}
	}

	public void onOptionPreferences(MenuItem i) {
		Log.d(TAG, "Preferences selected from menu");
		startActivity(new Intent(this, Preferences.class));
	}

	public void onOptionVersion(MenuItem i) {
		Log.d(TAG, "Version selected from menu");
		try {
		    String version = getPackageManager().getPackageInfo(getPackageName(), 0).versionName;
			AlertDialog alertDialog = new AlertDialog.Builder(this).create();
			alertDialog.setTitle("Version #");
			alertDialog.setMessage(version);
			alertDialog.show();
		} catch (NameNotFoundException e) {
		    Log.e("tag", e.getMessage());
		}
	}

	public void onOptionShowHideSurface(MenuItem i) {
		showHideWfdSurface();
	}

	public void onOptionShowHideWfdOperationButtons(MenuItem i) {
	    setVisibleControlButtons();
	}

	void unbindWfdService() {
              Log.d(TAG, "unbindWfdService() called");
              if(ServiceUtil.getmServiceAlreadyBound()) {
                 ServiceUtil.unbindService(getApplicationContext());
              }
        }

    @Override
    public void startSession(WifiP2pDevice inDevice) {
        Log.d(TAG, "startSession() called");
        final DeviceListFragment fragmentList = (DeviceListFragment) getFragmentManager()
                .findFragmentById(R.id.frag_list);
        WifiP2pInfo localDevInfo = null;

        if (fragmentList != null) {
            localDevInfo = fragmentList.getInfo();
        } else {
            Log.e(TAG, "Something amiss!! Fragment List is null");
            return;
        }
        if( localDeviceType == WFDDeviceType.SOURCE.getCode()) {
             wifiUtil.manager.setMiracastMode(wifiUtil.manager.MIRACAST_SOURCE);
        }
        if(modeSource && ServiceUtil.getmServiceAlreadyBound()) {
             String avMode = mSharedPrefs.getString("av_mode_type", "Audio_Video");
             AVPlaybackMode av = AVPlaybackMode.AUDIO_VIDEO;
             if(avMode.equals("Audio_Only")) {
                  av = AVPlaybackMode.AUDIO_ONLY;
             } else if(avMode.equals("Video_Only")) {
                  av = AVPlaybackMode.VIDEO_ONLY;
             }
             Log.d(TAG, "AV Mode = " + av);
             try {
                     ServiceUtil.getInstance().setAvPlaybackMode(av.ordinal());
             } catch (RemoteException e) {
                     Log.e(TAG, "Remote exception", e);
             }
        }
        if (localDevInfo != null) {
            if (localDevInfo.isGroupOwner) {
                localWfdDev = convertToWfdDevice(device, localDevInfo, false);
                if (!modeSource) {
                    final getIPTask Task = new getIPTask();
                    Task.execute(inDevice);
                    ipDialog = ProgressDialog.show(this,
                            "Awaiting IP allocation of peer from DHCP",
                            "Press back to cancel", true, true,
                            new DialogInterface.OnCancelListener() {
                                @Override
                                public void onCancel(DialogInterface dialog) {
                                        Log.e(TAG,
                                                "IP Dialog cancelled by listener");
                                    Task.cancel(false);
                                }
                            });
                } else {
                    // Source doesn't require sink IP
                    peerWfdDev = convertToWfdDevice(inDevice, null, false);
                    peerWfdDev.decoderLatency =
                    Integer.parseInt(mSharedPrefs.getString("udp_decoder_latency_value", "5"));
                    StartSessionTask task = new StartSessionTask();
                    Log.d(TAG, "Decoder LAtency " + peerWfdDev.decoderLatency);
                    Log.d(TAG, "Start session with " + inDevice.deviceAddress);
                    task.execute(inDevice);
                }
            } else {
                Log.e(TAG, "Local Device is not a group owner");
                localWfdDev = convertToWfdDevice(device, null, false);
                // Get the local device IP
                /*
                 * localWfdDev.ipAddress = wifiUtil.getLocalIp();
                 */
                // peer is GO, so set its IP as the GO IP from localDdevInfo
                peerWfdDev = convertToWfdDevice(inDevice, localDevInfo, false);
                peerWfdDev.decoderLatency =
                Integer.parseInt(mSharedPrefs.getString("udp_decoder_latency_value", "5"));
                StartSessionTask task = new StartSessionTask();
                Log.d(TAG, "Start session with " + inDevice.deviceAddress);
                task.execute(inDevice);
            }
        } else if (localDevInfo == null) {
            Log.e(TAG, "Something amiss!! Local Device Info is null.");
        }
    }

    class getIPTask extends AsyncTask<WifiP2pDevice, Integer, Boolean> {
        private int timeout = 0;
        String show = new String("Failed to get IP from lease file in attempt ");
        private WifiP2pDevice peerDevice = null;
        @Override
        protected Boolean doInBackground(WifiP2pDevice... devices) {
            peerDevice = devices[0];
            while (timeout++ < 60) { // try 60 seconds for IP to get populated else
                               // give up
                peerWfdDev = convertToWfdDevice(peerDevice, null, true);
                if (peerWfdDev != null && peerWfdDev.ipAddress != null) {
                    return true;
                }
                if (isCancelled()) {
                    Log.e(TAG, "getIP Task was cancelled");
                    return false;
                }
                publishProgress(timeout);
                Log.e(TAG, show + timeout);
                try {
                    Thread.sleep(1000);
                } catch (InterruptedException e) {
                    Log.e(TAG, "Background thread of async task interrupted");
                }
            }
            return false;
        }

        @Override
        protected void onProgressUpdate(Integer... progress) {
            ipDialog.setMessage(show + progress[0]);
        }

        @Override
        protected void onPostExecute(Boolean success) {
            if (ipDialog != null) {
                ipDialog.dismiss();
            }
            if (success) {
                Log.e(TAG, "Successful in obtaining Peer Device IP");
                StartSessionTask task = new StartSessionTask();
                Log.d(TAG, "Start session with " + peerDevice.deviceAddress);
                task.execute(peerDevice);
            } else {
                Log.e(TAG, "Unsuccessful in obtaining Peer Device IP");
            }
        }
    }
	@Override
	public void stopSession(int sessionId) {
		Log.d(TAG, "stopSession called");
		try {
			ServiceUtil.getInstance().stopWfdSession();
		} catch (RemoteException e) {
			Log.e(TAG, "Remote exception", e);
		}
		setGoneControlButtons();
	}

	@Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {
        if (keyCode == KeyEvent.KEYCODE_BACK && inSession) {
            Log.d(TAG, "Back button pressed while starting session");
            teardownWfdOperation(null);
            return true;
        } else if (keyCode == KeyEvent.KEYCODE_BACK) {
            Log.d(TAG, "Back button pressed");
            AlertDialog.Builder builder = new AlertDialog.Builder(WFDClientActivity.this);
			builder.setMessage(R.string.exit_alert_dialog_message)
			       .setPositiveButton(R.string.exit_alert_dialog_positive_button,
			              new DialogInterface.OnClickListener() {
			           public void onClick(DialogInterface dialog, int id) {
			                WFDClientActivity.this.finish();
			           }
			       })
			       .setNegativeButton(R.string.exit_alert_dialog_negative_button,
			              new DialogInterface.OnClickListener() {
			           public void onClick(DialogInterface dialog, int id) {
			                dialog.cancel();
			           }
			       });
			AlertDialog alert = builder.create();
			alert.show();
        }
        return super.onKeyDown(keyCode, event);
    }

	private static String getDeviceStatus(int deviceStatus) {
		Log.d(TAG, "getDeviceStatus:" + deviceStatus);
		switch (deviceStatus) {
		case WifiP2pDevice.AVAILABLE:
			return "Available";
		case WifiP2pDevice.INVITED:
			return "Invited";
		case WifiP2pDevice.CONNECTED:
			return "Connected";
		case WifiP2pDevice.FAILED:
			return "Failed";
		case WifiP2pDevice.UNAVAILABLE:
			return "Unavailable";
		default:
			return "Unknown";
		}
	}

	/**
	 * Update UI for this device.
	 *
     * @param device WifiP2pDevice object
	 */
	public void updateThisDevice(WifiP2pDevice device) {
		Log.d(TAG, "updateThisDevice called");
		this.device = device;
		mynameTextView.setText(device.deviceName);
		mymacaddressTextView.setText(device.deviceAddress);
		mystatusTextView.setText(getDeviceStatus(device.status));
	}

	@Override
	public void peersReceived() {
		Log.d(TAG, "peersReceived() called");
		isDiscoveryRequested = false;
	}

	@Override
	public int getLocalDeviceType() {
		Log.d(TAG, "getLocalDeviceType() called");
		return localDeviceType;
	}

    private void setUIBCButton() {
        Bundle cfgItem = new Bundle();
        int ret = 0;
        try {
            ret = ServiceUtil.getInstance().getConfigItems(cfgItem);
        } catch (RemoteException e) {
            e.printStackTrace();
        }
        int[] configArr = new int[WfdEnums.ConfigKeys.TOTAL_CFG_KEYS.ordinal()];
        configArr = cfgItem.getIntArray(WfdEnums.CONFIG_BUNDLE_KEY);
        if (configArr!=null && 1 == configArr[WfdEnums.ConfigKeys.UIBC_M14.ordinal()]) {
            mUIBCM14 = true;
            enableDisableUibcButton.setText("Send M14");
        } else {
            mUIBCM14 = false;
        }
        Log.d(TAG, "setUIBCButton() M14 support is " + mUIBCM14);
    }

	public void setVisibleControlButtons() {
		Log.d(TAG, "setVisibleControlButtons() called");
		controlbuttonsTableLayout.setVisibility(View.VISIBLE);
        setUIBCButton();
	}

	public void setGoneControlButtons() {
		Log.d(TAG, "setGoneControlButtons() called");
		controlbuttonsTableLayout.setVisibility(View.GONE);
	}

	public void teardownWFDService() {
		Log.d(TAG, "teardownWFDService() called");
		PreTeardownTaskSessionCheck task = new PreTeardownTaskSessionCheck();
		task.execute();
	}

	public void deinitWFDService() {
		try {
			Log.e(TAG, "WiFi Direct deinit do in background started");
			ServiceUtil.getInstance().deinit();
            Log.e(TAG, "WiFi Direct deinit do in background finished");
		} catch (RemoteException e) {
            Log.e(TAG, "Remote exception", e);
		}
	}


	/**
	 * Class for internal event handling in WFDClientActivity. Must run on UI thread.
	 */
	class ClientEventHandler extends Handler {

	    @Override
	    public void handleMessage(Message msg) {
	        Log.d(TAG, "Event handler received: " + msg.what);

	        if (WfdOperationUtil.wfdOperationProgressDialog != null) {
	            Log.d(TAG, "clientProgressDialog != null");
	            if (WfdOperationUtil.wfdOperationProgressDialog.isShowing()) {
	                Log.d(TAG, "clientProgressDialog isShowing");
	            } else {
	                Log.d(TAG, "clientProgressDialog not isShowing");
	            }
	        } else {
	            Log.d(TAG, "clientProgressDialog == null");
	        }

	        switch (msg.what) {

	            case PLAY_CALLBACK: {
	                playpauseButton.setChecked(false);
	                standbyresumeButton.setChecked(true);
	                if (WfdOperationUtil.wfdOperationProgressDialog != null && WfdOperationUtil.wfdOperationProgressDialog.isShowing()) {
	                    WfdOperationUtil.wfdOperationProgressDialog.dismiss();
	                    Log.d(TAG, "clientProgressDialog dismissed");
	                }
	            }
	            break;
	            case PAUSE_CALLBACK: {
                    playpauseButton.setChecked(true);
                    standbyresumeButton.setChecked(true);
                    if (WfdOperationUtil.wfdOperationProgressDialog != null && WfdOperationUtil.wfdOperationProgressDialog.isShowing()) {
                        WfdOperationUtil.wfdOperationProgressDialog.dismiss();
                        Log.d(TAG, "clientProgressDialog dismissed");
                    }
                }
                break;
	            case STANDBY_CALLBACK: {
                    standbyresumeButton.setChecked(false);
                    if (WfdOperationUtil.wfdOperationProgressDialog != null && WfdOperationUtil.wfdOperationProgressDialog.isShowing()) {
                        WfdOperationUtil.wfdOperationProgressDialog.dismiss();
                        Log.d(TAG, "clientProgressDialog dismissed");
                    }
                }
                break;
	            case SET_WFD_FINISHED: {
	                if (msg.arg1 == -1) {
	                    AlertDialog.Builder builder = new AlertDialog.Builder(
	                            WFDClientActivity.this);
	                    builder.setTitle("WFD failed to turn on")
	                    .setMessage("Would you like to exit the application?")
	                    .setCancelable(false)
	                    .setPositiveButton("Yes", new DialogInterface.OnClickListener() {
	                        public void onClick(DialogInterface dialog, int id) {
	                            WFDClientActivity.this.finish();
	                        }
	                    })
	                    .setNegativeButton("No", new DialogInterface.OnClickListener() {
	                        public void onClick(DialogInterface dialog, int id) {
	                            dialog.cancel();
	                        }
	                    });
	                    AlertDialog alert = builder.create();
	                    alert.show();
	                } else if (msg.arg1 == 0) {
	                    //Toast.makeText(WFDClientActivity.this, "WFD turned on", Toast.LENGTH_SHORT).show();
	                }
	            }
	            break;
	            case INVALID_WFD_DEVICE: {
	                AlertDialog.Builder builder = new AlertDialog.Builder(WFDClientActivity.this);
	                builder.setTitle("Selected device does not support WFD")
	                .setMessage("Please select another device to start a WFD session")
	                .setCancelable(false)
	                .setPositiveButton("Ok", new DialogInterface.OnClickListener() {
	                    public void onClick(DialogInterface dialog, int id) {
	                        dialog.cancel();
	                    }
	                });
	                AlertDialog alert = builder.create();
	                alert.show();
	            }
	            break;
	            case SERVICE_BOUND: {
	                WFDClientActivity.this.mActionListener = WfdOperationUtil.createmActionListener(eventHandler);
	                try {
	                    //set local device type
	                    int setDeviceTypeRet = ServiceUtil.getInstance().setDeviceType(localDeviceType);
	                    if (setDeviceTypeRet == 0) {
	                        Log.d(TAG, "mWfdService.setDeviceType called.");
	                    } else {
	                        Log.d(TAG, "mWfdService.setDeviceType failed with error code: "
	                                + setDeviceTypeRet);
	                    }

	                    //verify session state
	                    WfdStatus wfdStatus = ServiceUtil.getInstance().getStatus();
	                    Log.d(TAG, "wfdStatus.state= " + wfdStatus.state);
	                    if (wfdStatus.state == WfdEnums.SessionState.INVALID.ordinal() ||
	                            wfdStatus.state == WfdEnums.SessionState.INITIALIZED.ordinal()) {
	                        Log.d(TAG, "wfdStatus.state is INVALID or INITIALIZED");
	                        setGoneControlButtons();
	                    } else {
	                        setVisibleControlButtons();
	                    }


	                } catch (RemoteException e) {
	                    Log.e(TAG, "WfdService init() failed", e);
	                    return;
	                }
	                Toast.makeText(getApplicationContext(), getResources().getString(
	                        R.string.wfd_service_connected), Toast.LENGTH_SHORT).show();
	            }
	            break;
                    case CLEAR_UI: {
                        if (startSessionProgressDialog != null) {
                            startSessionProgressDialog.dismiss();
                        }
                        if(!modeSource) {
                           Log.e(TAG, "Calling finish on SurfaceActivity");
                           try {
                                 finishActivity (START_SURFACE);
                           } catch (ActivityNotFoundException e) {
                               Log.e(TAG, "Surface Activity not yet started/already finished");
                           }
                        }
                    }
                    //fall through to TEARDOWN_CALLBACK case handling for cleanup
	            case TEARDOWN_CALLBACK: {
	                if (WfdOperationUtil.wfdOperationProgressDialog != null && WfdOperationUtil.wfdOperationProgressDialog.isShowing()) {
	                    Log.d(TAG, "EventHandler: teardownCallback- dismiss clientProgressDialog");
	                    WfdOperationUtil.wfdOperationProgressDialog.dismiss();
	                }

                        if (startSessionProgressDialog != null) {
                            startSessionProgressDialog.dismiss();
                        }

	                try {
                            if(ServiceUtil.getmServiceAlreadyBound() != false) {
	                        ServiceUtil.getInstance().deinit();
                            }
	                } catch (RemoteException e) {
                        Log.e(TAG, "EventHandler: teardownCallback- Remote exception when calling deinit()", e);
	                }
                        cancelDisconnect();
	                DeviceListFragment fragmentList = (DeviceListFragment) getFragmentManager()
	                .findFragmentById(R.id.frag_list);
	                if (fragmentList != null) {
	                    Log.d(TAG, "EventHandler: teardownCallback- teardown fragment");
	                    fragmentList.doTeardown();
	                }
	                Log.d(TAG, "EventHandler: teardownCallback- setGoneControlButtons");
	                setGoneControlButtons();
	                enableDisableUibcButton.setChecked(false);
                        selectTransportButton.setChecked(false);
	                Log.d(TAG, "EventHandler: teardownCallback-  control buttons gone");
	                wifiUtil.send_wfd_set(true, WFDDeviceType.getValue(localDeviceType));
                        wifiUtil.manager.setMiracastMode(wifiUtil.manager.MIRACAST_DISABLED);
	                Toast.makeText(WFDClientActivity.this, "WiFi Direct Teardown",
	                        Toast.LENGTH_SHORT).show();
                        unbindWfdService();
	                Log.d(TAG, "EventHandler: teardownCallback- completed");
	                inSession = false;
	            }
	            break;
	            case START_SESSION_ON_UI: {
	                if (startSessionProgressDialog != null
	                        && startSessionProgressDialog.isShowing()) {
	                    startSessionProgressDialog.dismiss();
	                    Log.d(TAG, "EventHandler: startSessionOnUI- progress dialog closed");
	                }
	                if (modeSource) {
	                    Log.d(TAG, "EventHandler: startSessionOnUI- device is source, setVisibleControlButtons");
	                    setVisibleControlButtons();
	                }
	                wifiUtil.send_wfd_set(false, WFDDeviceType.getValue(localDeviceType));
	                Log.d(TAG, "EventHandler: startSessionOnUI- completed");
	            }
	            break;
	            case UIBC_ACTION_COMPLETED: {
	            	enableDisableUibcButton.toggle();
                       if (enableDisableUibcButton.isChecked()) {
                           if (WfdOperationUtil.wfdOperationProgressDialog != null && WfdOperationUtil.wfdOperationProgressDialog.isShowing()) {
                               WfdOperationUtil.wfdOperationProgressDialog.dismiss();
                               Log.d(TAG,"clientProgressDialog dismissed on start UIBC successful");
                           }
                       } else {
                           if (WfdOperationUtil.wfdOperationProgressDialog != null && WfdOperationUtil.wfdOperationProgressDialog.isShowing()) {
                               WfdOperationUtil.wfdOperationProgressDialog.dismiss();
                               Log.d(TAG,"clientProgressDialog dismissed on stop UIBC successful");
                           }
	                }
                    Log.d(TAG, "EventHandler: uibcActionCompleted- completed");
                }
                break;
		default:
				Log.e(TAG, "Unknown event received: " + msg.what);
			}
		}
	}


    /**
     * Method to convert a WifiP2PDevice to a WFDDevice
     *
     * @param wifiP2pDevice
     *            the WifiP2pDevice to be converted to WfdDevice
     * @param info
     *            WifiP2pInfo of local device use null as a poison input to set
     *            IP of device or not
     * @param parseLeaseFile
     *            flag to distinguish whether to parse lease file or not
     * @return WfdDevice the converted WfdDevice
     */
    private static WfdDevice convertToWfdDevice(WifiP2pDevice wifiP2pDevice,
            WifiP2pInfo info, boolean parseLeaseFile) {
        if (wifiP2pDevice == null) {
            Log.e(TAG,
                    "convertToWfdDevice Something amiss!! wifiP2pDevice is null");
            return null;
        }
        if (wifiP2pDevice.wfdInfo == null) {
            Log.e(TAG,
                    "convertToWfdDevice Something fishy!! WFDInfo is null for device");
            return null;
        }
        WfdDevice wfdDevice = new WfdDevice();
        wfdDevice.deviceType = wifiP2pDevice.wfdInfo.getDeviceType();
        wfdDevice.macAddress = wifiP2pDevice.deviceAddress;
        Log.d(TAG, "convertToWfdDevice: device macAddress= "
                + wfdDevice.macAddress);
        wfdDevice.deviceName = wifiP2pDevice.deviceName;
        P2pWfdDeviceInfo extraWfdInfo = null;

        if (!parseLeaseFile) {
            if (info != null) {
                wfdDevice.ipAddress = info.groupOwnerAddress.getHostAddress();
                Log.d(TAG, "convertToWfdDevice- IP: " + wfdDevice.ipAddress);
            }
        } else {
            Log.d(TAG, "convertToWfdDevice parsing lease file");
            wfdDevice.ipAddress = WiFiUtil.getPeerIP(wfdDevice.macAddress);
            Log.d(TAG, "convertToWfdDevice- ipAddress: " + wfdDevice.ipAddress);
        }

        if (wfdDevice.ipAddress == null) {
            Log.e(TAG, "convertToWfdDevice- no ipAddress was found");
        }

        wfdDevice.rtspPort = wifiP2pDevice.wfdInfo.getControlPort();
        wfdDevice.isAvailableForSession = wifiP2pDevice.wfdInfo.isSessionAvailable();
        wfdDevice.addressOfAP = null;
        wfdDevice.coupledSinkStatus = 0;
        wfdDevice.preferredConnectivity = 0;

        return wfdDevice;
    }

    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
              Log.d(TAG, "onActivityResult() called with "+ resultCode +"becasue of activity"
                 + "started with " + requestCode);
            if(requestCode == START_SURFACE && resultCode == RESULT_CANCELED) {
               Log.e(TAG, "Surface Activity has been destroyed , clear up UI");
               Message messageClearUI = eventHandler.obtainMessage(CLEAR_UI);
               eventHandler.sendMessage(messageClearUI);
            }
    }
    public void showHideWfdSurface() {
        startActivityForResult(new Intent(this, SurfaceActivity.class), START_SURFACE);
    }
}
