/*
 * Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __WLAN_MSG_H_
#define __WLAN_MSG_H_

#include <linux/netlink.h>

#define WLAN_MSG_MAX_PAYLOAD   256
#define WLAN_MSG_FAMILY  NETLINK_USERSOCK
#define WLAN_MSG_MCAST_GRP_ID  0x01

#define WLAN_MSG_WLAN_STATUS_IND    0x106
#define WLAN_MSG_WLAN_VERSION_IND   0x107
#define WLAN_MSG_WLAN_TP_IND        0x109

#define WLAN_MSG_MAX_SSID_LEN    32
#define WLAN_MSG_MAX_BSSID_LEN   6
#define WLAN_MSG_MAX_STR_LEN     16
#define WLAN_MSG_MAX_NUM_CHAN    128
#define WLAN_MSG_COUNTRY_CODE_LEN 3

#define WLAN_MSG_BASE     0x10

#define WLAN_MSG_SVC      (WLAN_MSG_BASE + 0x0A)

struct wlan_hdr {
	unsigned short type;
	unsigned short length;
};

struct wlan_status_data {
	uint8_t lpss_support;
	uint8_t is_on;
	uint8_t vdev_id;
	uint8_t is_connected;
	int8_t rssi;
	uint8_t ssid_len;
	uint8_t country_code[WLAN_MSG_COUNTRY_CODE_LEN];
	uint32_t vdev_mode;
	uint32_t freq;
	uint32_t num_channels;
	uint8_t channel_list[WLAN_MSG_MAX_NUM_CHAN];
	uint8_t ssid[WLAN_MSG_MAX_SSID_LEN];
	uint8_t bssid[WLAN_MSG_MAX_BSSID_LEN];
};

struct wlan_version_data {
	uint32_t chip_id;
	char chip_name[WLAN_MSG_MAX_STR_LEN];
	char chip_from[WLAN_MSG_MAX_STR_LEN];
	char host_version[WLAN_MSG_MAX_STR_LEN];
	char fw_version[WLAN_MSG_MAX_STR_LEN];
};

#endif /* __WLAN_MSG_H_ */
