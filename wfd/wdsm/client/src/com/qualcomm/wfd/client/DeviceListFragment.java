/*
 * Copyright (c) 2012-2014 Qualcomm Technologies, Inc.
 * All Rights Reserved.
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

import java.util.ArrayList;
import java.util.List;

import com.qualcomm.wfd.WfdEnums;
import com.qualcomm.wfd.client.R;

import android.app.AlertDialog;
import android.app.ListFragment;
import android.app.ProgressDialog;
import android.app.SearchManager.OnCancelListener;
import android.content.Context;
import android.content.DialogInterface;
import android.content.SharedPreferences;
import android.graphics.Color;
import android.net.wifi.WifiInfo;
import android.net.wifi.WpsInfo;
import android.net.wifi.p2p.WifiP2pConfig;
import android.net.wifi.p2p.WifiP2pDevice;
import android.net.wifi.p2p.WifiP2pDeviceList;
import android.net.wifi.p2p.WifiP2pInfo;
import android.net.wifi.p2p.WifiP2pWfdInfo;
import android.net.wifi.p2p.WifiP2pManager.ConnectionInfoListener;
import android.net.wifi.p2p.WifiP2pManager.PeerListListener;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.preference.PreferenceManager;
import android.text.InputFilter.LengthFilter;
import android.util.Log;
import android.view.ContextMenu;
import android.view.LayoutInflater;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.view.ContextMenu.ContextMenuInfo;
import android.view.View.OnClickListener;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.FrameLayout;
import android.widget.ImageButton;
import android.widget.LinearLayout;
import android.widget.ListView;
import android.widget.TextView;
import android.widget.Toast;
import android.widget.ToggleButton;

/**
 * A ListFragment that displays available peers on discovery and requests the
 * parent activity to handle user interaction events
 */
public class DeviceListFragment extends ListFragment implements
		PeerListListener {
	private static final String TAG = "Client.DeviceListFragment";
	private static final int WLAN_EVENT_P2P_PEER_LIST_FINISH = 0;
	public static List<WifiP2pDevice> peers = new ArrayList<WifiP2pDevice>();
	private Boolean isFilterOn = false;
	ProgressDialog progressDialog = null;
	ProgressDialog connectProgressDialog = null;
	View mContentView = null;
	private Button toggleButton;
	private FrameLayout deviceDetailsFrameLayout;
	private EventHandler eventHandler;
	private WifiP2pDevice tempDevice;
	private WifiP2pDevice connectedDevice = null;
	private WifiP2pInfo info;
	public static final int connectionInfoAvailable = 1;

	@Override
	public void onActivityCreated(Bundle savedInstanceState) {
		super.onActivityCreated(savedInstanceState);
		this.setListAdapter(new WiFiPeerListAdapter(getActivity(),
				R.layout.row_devices, peers));
		eventHandler = new EventHandler();
	}

	@Override
	public View onCreateView(LayoutInflater inflater, ViewGroup container,
			Bundle savedInstanceState) {
		mContentView = inflater.inflate(R.layout.device_list, null);

		eventHandler = new EventHandler();

		return mContentView;
	}

	private static String getDeviceStatus(int deviceStatus) {
		Log.d(WFDClientActivity.TAG, "getDeviceStatus:" + deviceStatus);
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

        private static WfdEnums.WFDDeviceType convertP2pDevToWfd(int devtype) {
		switch (devtype) {

			case WifiP2pWfdInfo.WFD_SOURCE: return WfdEnums.WFDDeviceType.SOURCE;
			case WifiP2pWfdInfo.PRIMARY_SINK: return WfdEnums.WFDDeviceType.PRIMARY_SINK;
                        case WifiP2pWfdInfo.SECONDARY_SINK: return WfdEnums.WFDDeviceType.SECONDARY_SINK;
			case WifiP2pWfdInfo.SOURCE_OR_PRIMARY_SINK: return WfdEnums.WFDDeviceType.SOURCE_PRIMARY_SINK;
		}
                return WfdEnums.WFDDeviceType.UNKNOWN;
	}

	/**
	 * Array adapter for ListFragment that maintains QCWifiP2pDevice list.
	 */
	private class WiFiPeerListAdapter extends ArrayAdapter<WifiP2pDevice> {

		private List<WifiP2pDevice> items;

		/**
		 * @param context
		 * @param textViewResourceId
		 * @param objects
		 */
		public WiFiPeerListAdapter(Context context, int textViewResourceId,
				List<WifiP2pDevice> objects) {
			super(context, textViewResourceId, objects);
			items = objects;
		}

		@Override
		public View getView(int position, View convertView, ViewGroup parent) {
			Log.d(TAG, "getView called with position: " + position);
			View v = convertView;
			if (v == null) {
				LayoutInflater vi = (LayoutInflater) getActivity()
						.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
				v = vi.inflate(R.layout.row_devices, null);
				v.setTag(R.string.position, position);
				v.setTag(R.string.expanded, false);
			}
			v.setTag(R.string.position, position);

			final WifiP2pDevice device = items.get(position);
			if (device != null) {
				TextView top = (TextView) v.findViewById(R.id.device_name);
				TextView middle = (TextView) v
						.findViewById(R.id.device_mac_address);
				TextView bottom = (TextView) v
						.findViewById(R.id.device_details);
				if (top != null) {
					top.setText(device.deviceName);
				}
				if (middle != null) {
					middle.setText(device.deviceAddress);
				}
				if (bottom != null) {
					bottom.setText(getDeviceStatus(device.status));
				}

				if (device.status == WifiP2pDevice.CONNECTED) {
					Log.d(TAG, "getView called on connected device");
					if (connectProgressDialog != null && connectProgressDialog.isShowing()) {
						connectProgressDialog.dismiss();
						Log.d(TAG, "ConnectProgressDialog.dismiss() 1");
					}
					//connectedDevice = device;
					setConnectedDevice(device);
					deviceDetailsFrameLayout = (FrameLayout) v
							.findViewById(R.id.fl_device_detail);
					LinearLayout linLayoutLevel1 = (LinearLayout) deviceDetailsFrameLayout
							.getParent();
					linLayoutLevel1.setTag(R.string.expanded, true);
					deviceDetailsFrameLayout.setVisibility(View.VISIBLE);
					TextView deviceinfoTextView = (TextView) deviceDetailsFrameLayout
							.findViewById(R.id.tv_device_info);
					deviceinfoTextView.setText(device.toString()+
                                        ((device.wfdInfo != null)?(new P2pWfdDeviceInfo(device.wfdInfo.getDeviceInfoHex()).toString()):""));
					if (info != null) {
						onConnectionInfoAvailable(info);
					}
					} else if (device.status == WifiP2pDevice.FAILED) {
					Log.d(TAG, "getView called on device that failed to connect");
					if (connectProgressDialog != null && connectProgressDialog.isShowing()) {
						connectProgressDialog.dismiss();
						Log.d(TAG, "ConnectProgressDialog.dismiss() 2");
					}
				}
			}
			return v;
		}
	}

	/**
	 * Initiate a connection with the peer.
	 */
	@Override
	public void onListItemClick(ListView l, View v, int position, long id) {
		Log.d(TAG, "onListItemClick called on position: " + position);

		WifiP2pDevice device = (WifiP2pDevice) getListAdapter().getItem(position);

		// Expand deviceDetail if not already
		FrameLayout clickedDeviceDetailsFrameLayout = (FrameLayout) v
				.findViewById(R.id.fl_device_detail);
		LinearLayout linLayoutLevel1 = (LinearLayout) clickedDeviceDetailsFrameLayout
				.getParent();
		boolean expanded = (Boolean) linLayoutLevel1.getTag(R.string.expanded);
		if (!expanded) {
			Log.d(TAG, "onListItemClick called on list item that wasn't expanded"+
			((device.wfdInfo != null)? device.wfdInfo.getDeviceInfoHex():""));
			linLayoutLevel1.setTag(R.string.expanded, true);
			clickedDeviceDetailsFrameLayout.setVisibility(View.VISIBLE);
			TextView deviceinfoTextView = (TextView) clickedDeviceDetailsFrameLayout
					.findViewById(R.id.tv_device_info);
			deviceinfoTextView.setText(device.toString()+
			((device.wfdInfo != null)?(new P2pWfdDeviceInfo(device.wfdInfo.getDeviceInfoHex()).toString()):""));
		} else {
			Log.d(TAG, "onListItemClick called on list item that was already expanded");
			linLayoutLevel1.setTag(R.string.expanded, false);
			clickedDeviceDetailsFrameLayout.setVisibility(View.GONE);
		}
	}

	public void handleConnectClick(View v) {
		Log.d(TAG, "handleConnectClick called");
		LinearLayout linLayoutLevel4 = (LinearLayout) v.getParent();
		LinearLayout linLayoutLevel3 = (LinearLayout) linLayoutLevel4
				.getParent();
		deviceDetailsFrameLayout = (FrameLayout) linLayoutLevel3.getParent();
		LinearLayout linLayoutLevel1 = (LinearLayout) deviceDetailsFrameLayout
				.getParent();
		int position = (Integer) linLayoutLevel1.getTag(R.string.position);
		final WifiP2pDevice device = ((WiFiPeerListAdapter) getListAdapter())
				.getItem(position);

		if (device.wfdInfo == null) {
	        Log.d(TAG, "handleConnectClick- device.WfdInfo == null");
			AlertDialog.Builder builder = new AlertDialog.Builder(getActivity());
			builder.setTitle("Selected device is not a WFD Capable Device")
				   .setMessage("Would you like to select another device instead?")
			       .setPositiveButton("Yes", new DialogInterface.OnClickListener() {
			           public void onClick(DialogInterface dialog, int id) {
			        	   return;
			           }
			       })
			       .setNegativeButton("No", new DialogInterface.OnClickListener() {
			           public void onClick(DialogInterface dialog, int id) {
			       			handleConnectClickHelper(device);
			           }
			       });
			AlertDialog alert = builder.create();
			alert.show();
		} else if (device.wfdInfo.isSessionAvailable() == false){
		    Log.d(TAG, "handleConnectClick- device.wfdInfo.isSessionAvailable() == false");
		    AlertDialog.Builder builder = new AlertDialog.Builder(getActivity());
            builder.setTitle("Selected device is not currently available for a WFD session")
                   .setMessage("Would you like to select another device instead?")
                   .setPositiveButton("Yes", new DialogInterface.OnClickListener() {
                       public void onClick(DialogInterface dialog, int id) {
                           return;
                       }
                   })
                   .setNegativeButton("No", new DialogInterface.OnClickListener() {
                       public void onClick(DialogInterface dialog, int id) {
                            handleConnectClickHelper(device);
                       }
                   });
            AlertDialog alert = builder.create();
            alert.show();
        } else if (ServiceUtil.getmServiceAlreadyBound()){
            try{
                if(ServiceUtil.getInstance().getStatus().state !=
                    WfdEnums.SessionState.INVALID.ordinal()){
                    Log.d(TAG,"Attempt to click Connect button when session state is valid");
                    AlertDialog.Builder builder = new AlertDialog.Builder(getActivity());
                    builder.setTitle("Connect Button clicked unexpectedly")
                           .setMessage("Please teardown current session before attempting to connect to another device")
                           .setNeutralButton("Ok", new DialogInterface.OnClickListener() {
                               public void onClick(DialogInterface dialog, int id) {
                                   return;
                               }
                            });
                    AlertDialog alert = builder.create();
                    alert.show();
                } else {
                    Log.d(TAG,"Session State is invalid. Processing connect click");
                    Log.d(TAG, "handleConnectClick- call handleConnectClickHelper");
                    handleConnectClickHelper(device);
                }
            }catch (Exception e) {
                e.printStackTrace();
            }
        }
        else {
            Log.d(TAG, "handleConnectClick- call handleConnectClickHelper");
			handleConnectClickHelper(device);
		}
	}

	private void handleConnectClickHelper(WifiP2pDevice device) {
		SharedPreferences sharedPrefs = PreferenceManager
		.getDefaultSharedPreferences(getActivity());
		WifiP2pConfig config = new WifiP2pConfig();
		config.deviceAddress = device.deviceAddress;
		config.wps.setup = WpsInfo.PBC;
		if (sharedPrefs.getBoolean("perform_custom_group_owner_intent_value",
				false) == true) {
			config.groupOwnerIntent = Integer.parseInt(sharedPrefs.getString(
					"group_owner_intent_value", "3"));
		}
		Toast.makeText(getActivity(), "group owner intent value: "
				+ config.groupOwnerIntent, Toast.LENGTH_LONG);
		Log.d(TAG, "group owner intent value: " + config.groupOwnerIntent);

		if (progressDialog != null && progressDialog.isShowing()) {
			progressDialog.dismiss();
		}
		connectProgressDialog = ProgressDialog.show(getActivity(),
				"Press back to cancel", "Connecting to :"
				+ device.deviceAddress, true, true);
		((DeviceActionListener) getActivity()).connect(config, device);
	}

	public void handleConnectOptionsClick(View v) {
		Log.d(TAG, "handleConnectOptionsClick called");
		LinearLayout linLayoutLevel4 = (LinearLayout) v.getParent();
		LinearLayout linLayoutLevel3 = (LinearLayout) linLayoutLevel4
				.getParent();
		deviceDetailsFrameLayout = (FrameLayout) linLayoutLevel3.getParent();
		LinearLayout linLayoutLevel1 = (LinearLayout) deviceDetailsFrameLayout
				.getParent();
		int position = (Integer) linLayoutLevel1.getTag(R.string.position);
		tempDevice = ((WiFiPeerListAdapter) getListAdapter()).getItem(position);

		registerForContextMenu(v);
		getActivity().openContextMenu(v);
	}

	public void handleStartClientClick(View v) {
		Log.d(TAG, "handleStartClientClick called");
		LinearLayout linLayoutLevel4 = (LinearLayout) v.getParent();
		LinearLayout linLayoutLevel3 = (LinearLayout) linLayoutLevel4
				.getParent();
		FrameLayout frameLayoutLevel2 = (FrameLayout) linLayoutLevel3
				.getParent();
		LinearLayout linLayoutLevel1 = (LinearLayout) frameLayoutLevel2
				.getParent();
		int position = (Integer) linLayoutLevel1.getTag(R.string.position);
		final WifiP2pDevice peerDevice = ((WiFiPeerListAdapter) getListAdapter())
				.getItem(position);
		Log.d(TAG, "handleStartClientClick: peer macAddress= " + peerDevice.deviceAddress);
		if (peerDevice.wfdInfo == null || peerDevice.wfdInfo.isSessionAvailable() == false){
                    Log.d(TAG, "handleStartClientClick: peerDevice not available for session");
                    AlertDialog.Builder builder = new AlertDialog.Builder(getActivity());
                    builder.setTitle(peerDevice.deviceAddress + " is not currently available for a WFD session")
                    .setMessage(
                           "Disconnect and try again after resetting peer device if it worked earlier"
                            +"\n"+"Or select another device if peer device is not WFD capable")
                    .setNeutralButton("Ok",
                            new DialogInterface.OnClickListener() {
                                public void onClick(DialogInterface dialog, int id) {
                                        return;
                                }
                             });
			AlertDialog alert = builder.create();
			alert.show();
		} else {
            if (deviceDetailsFrameLayout != null) {
                deviceDetailsFrameLayout.findViewById(R.id.btn_start_client)
                        .setEnabled(false);
            } else {
                Log.e(TAG, "deviceDetailsFrameLayout is null. Really??");
                return;
            }
		    ((DeviceActionListener) getActivity()).startSession(peerDevice);
		}
	}

	public void handleFilterClick() {
		Log.d(TAG, "handleFilterClick called");
		isFilterOn = !isFilterOn;
		((WiFiPeerListAdapter) getListAdapter()).notifyDataSetChanged();
	}

	@Override
	public void onPeersAvailable(WifiP2pDeviceList peerList) {
		Log.d(TAG, "onPeersAvailable called");
		if (progressDialog != null && progressDialog.isShowing()) {
			progressDialog.dismiss();
		}

		if (peerList.getDeviceList().size() == 0) {
			Log.d(TAG, "No devices found");
			return;
		}
		((DeviceActionListener) getActivity()).peersReceived();
		Message message = eventHandler
				.obtainMessage(WLAN_EVENT_P2P_PEER_LIST_FINISH);
		message.obj = peerList;

		eventHandler.sendMessage(message);
	}

	public void clearPeers() {
		Log.d(TAG, "clearPeers called");
		peers.clear();
		((WiFiPeerListAdapter) getListAdapter()).notifyDataSetChanged();
	}

	/**
     *
     */
	public void onInitiateDiscovery() {
		Log.d(TAG, "onInitiateDiscovery() called");
		if (progressDialog != null && progressDialog.isShowing()) {
			progressDialog.dismiss();
		}
		progressDialog = ProgressDialog.show(getActivity(),
				"Press back to cancel", "finding peers", true, true,
				new DialogInterface.OnCancelListener() {

					@Override
					public void onCancel(DialogInterface dialog) {

					}
				});
	}

	@Override
	public void onCreateContextMenu(ContextMenu menu, View v,
			ContextMenuInfo menuInfo) {
		super.onCreateContextMenu(menu, v, menuInfo);
		MenuInflater menuInflater = getActivity().getMenuInflater();
		menuInflater.inflate(R.menu.connection_menu, menu);
	}

	@Override
	public boolean onContextItemSelected(MenuItem item) {
		String option = (String) item.getTitle();
		Toast.makeText(getActivity(),
				"Connection option " + option + " selected", 1500).show();
		Log.d(TAG, "Connection option " + option + " selected");

		Log.d(TAG, "BEFORE new wifip2pconfig");
		WifiP2pConfig config = new WifiP2pConfig();
		Log.d(TAG, "After new wifip2pconfig");
		config.deviceAddress = tempDevice.deviceAddress;

		SharedPreferences sharedPrefs = PreferenceManager
				.getDefaultSharedPreferences(getActivity());

		switch (item.getItemId()) {
		case R.id.WPS_PBC:
			config.wps.setup = WpsInfo.PBC;
			if (sharedPrefs.getBoolean(
					"perform_custom_group_owner_intent_value", false) == true) {
				config.groupOwnerIntent = Integer.parseInt(sharedPrefs
						.getString("group_owner_intent_value", "3"));
			}
			Log.d(TAG, "group owner intent value: " + config.groupOwnerIntent);
			break;
		case R.id.WPS_DISPLAY:
			config.wps.setup = WpsInfo.DISPLAY;
			break;
		case R.id.WPS_KEYPAD:
			config.wps.setup = WpsInfo.KEYPAD;
			Log.d(TAG, "config.wps.pin: " + config.wps.pin);
			break;
		default:
			Toast.makeText(getActivity(), "Not a valid selection", 4000);
			Log.d(TAG, "Not a valid selection");
			break;
		}

		Log.d(TAG, "config.wps.setup: " + config.wps.setup);
		if (progressDialog != null && progressDialog.isShowing()) {
			progressDialog.dismiss();
		}
		progressDialog = ProgressDialog.show(getActivity(),
				"Press back to cancel", "Connecting to :"
						+ tempDevice.deviceAddress, true, true
				);
		((DeviceActionListener) getActivity()).connect(config, tempDevice);
		return true;
	}


	public void onConnectionInfoAvailable(final WifiP2pInfo info) {
		Log.d(TAG, "onConnectionInfoAvailable() called");
		if (connectProgressDialog != null && connectProgressDialog.isShowing()) {
			connectProgressDialog.dismiss();
		}
		this.info = info;
		Message message = eventHandler.obtainMessage(connectionInfoAvailable);

		eventHandler.sendMessage(message);
	}

	public void setConnectedDevice(WifiP2pDevice device) {
		Log.d(TAG, "setConnectedDevice() called");
		this.connectedDevice = device;
	}

	/**
	 * Clears UI fields after a teardown
	 */
	public void doTeardown() {
		Log.d(TAG, "doTeardown() called");
		if (null != deviceDetailsFrameLayout &&
		        null != deviceDetailsFrameLayout.findViewById(R.id.btn_start_client))
		{
			deviceDetailsFrameLayout.findViewById(R.id.btn_start_client)
				.setVisibility(View.VISIBLE);
		} else {
			Log.d(TAG, "in doTeardown()- btn_start_client is null");
		}
	}

	/**
	 * Class for internal event handling. Must run on UI thread.
	 */
	class EventHandler extends Handler {
		private static final String TAG = "Client.DeviceListFragment";

		@Override
		public void handleMessage(Message msg) {
			Log.d(TAG, "Event handler received: " + msg.what);
			switch (msg.what) {

			/* Timer events */

			case WLAN_EVENT_P2P_PEER_LIST_FINISH: {
				peers.clear();
				WifiP2pDeviceList peerList = (WifiP2pDeviceList) msg.obj;
				for (WifiP2pDevice peer : peerList.getDeviceList()) {
					Log.d(TAG, "WifiP2pDevice: " + peer.deviceAddress);
					peers.add((WifiP2pDevice) peer);
				}
				((WiFiPeerListAdapter) getListAdapter()).notifyDataSetChanged();
			}
				break;
			case connectionInfoAvailable: {
				Log.d(TAG, info.toString());
				if (deviceDetailsFrameLayout != null) {
					deviceDetailsFrameLayout.setVisibility(View.VISIBLE);
					deviceDetailsFrameLayout.findViewById(R.id.btn_disconnect)
							.setVisibility(View.VISIBLE);

					// The owner IP is now known.
					TextView view = (TextView) deviceDetailsFrameLayout
							.findViewById(R.id.tv_group_owner);
					view.setText(getResources().getString(
							R.string.group_owner_text)
							+ ((info.isGroupOwner == true) ? getResources()
									.getString(R.string.yes) : getResources()
									.getString(R.string.no)));
					view.setVisibility(View.VISIBLE);

					// InetAddress from WifiP2pInfo struct.
					view = (TextView) deviceDetailsFrameLayout
							.findViewById(R.id.tv_group_ip);
					view.setText("Group Owner IP - "
							+ info.groupOwnerAddress.getHostAddress());
					view.setVisibility(View.VISIBLE);

					int localDeviceType = ((DeviceActionListener)getActivity()).getLocalDeviceType();
								Log.d(TAG, "connectionInfoAvailable: localDeviceType- " + localDeviceType);
					// Don't show the start session button if the two
					// devices are of the same type
					deviceDetailsFrameLayout.findViewById(R.id.btn_start_client).setVisibility(View.VISIBLE);
                    if (ServiceUtil.getmServiceAlreadyBound()) {
                        try {
                                if (ServiceUtil.getInstance().getStatus().state == WfdEnums.SessionState.INVALID
                                    .ordinal()) {
                                deviceDetailsFrameLayout.findViewById(
                                        R.id.btn_start_client).setEnabled(true);
                            }
                        } catch (Exception e) {
                            e.printStackTrace();
                        }
                    }
					if (connectedDevice != null && connectedDevice.wfdInfo != null) {
						Log.d(TAG, "connectionInfoAvailable: connectedDevice and connectedDevice.wfdInfo are not null");
						if (convertP2pDevToWfd(connectedDevice.wfdInfo.getDeviceType()).getCode() == localDeviceType) {
							Log.d(TAG, "connectionInfoAvailable: localDeviceType- " + localDeviceType +
								", connectedDeviceType- " + connectedDevice.wfdInfo.getDeviceType());
				                        	deviceDetailsFrameLayout.findViewById(R.id.btn_start_client).setEnabled(false);
						}
					}

					// hide the connect buttons
					deviceDetailsFrameLayout.findViewById(R.id.btn_connect)
							.setVisibility(View.GONE);
					deviceDetailsFrameLayout.findViewById(
							R.id.btn_connect_options).setVisibility(View.GONE);
				}
			}
				break;
			default:
				Log.e(TAG, "Unknown event received: " + msg.what);
			}
		}
	}

	public WifiP2pInfo getInfo() {
		return this.info;
	}


	/**
	 * An interface-callback for the activity to listen to fragment interaction
	 * events.
	 */
	public interface DeviceActionListener {

	    void cancelDisconnect();

	    void connect(WifiP2pConfig config, WifiP2pDevice device);

	    void startSession(WifiP2pDevice device);

	    void stopSession(int sessionId);

	    int getLocalDeviceType();

	    void peersReceived();
	}
}
