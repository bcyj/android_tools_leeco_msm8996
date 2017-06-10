/*
 * Copyright (c) 2011-2012 Qualcomm Atheros, Inc.
 * All Rights Reserved.
 * Qualcomm Atheros Proprietary and Confidential.
 */

#ifndef __NL80211_UTILS_H
#define __NL80211_UTILS_H

#include <netlink/genl/genl.h>
#include <netlink/genl/family.h>
#include <netlink/genl/ctrl.h>

#ifndef CONFIG_LIBNL20
#define nl_sock nl_handle
#endif

struct btcoex_nl_info {
	struct nl_sock *nl_sock;
	struct nl_cache *nl_cache;
	struct genl_family *nl80211;
};

int btcoex_nl_init(struct btcoex_nl_info **state);
void btcoex_nl_deinit(struct btcoex_nl_info *state);
int nl80211_get_info(struct btcoex_nl_info *state, char *name);
int nl80211_send_btcoex_cmd(struct btcoex_nl_info *state, int devidx,
			 char *data, int len);

/*
 * below defines is shared with kernel
 * so when private patch to nl80211.h
 * changes below macros change here too
 */
#define NL80211_CMD_BTCOEX 91
#define NL80211_ATTR_BTCOEX_DATA 161

enum nl80211_btcoex_cmds {
	NL80211_WMI_SET_BT_STATUS = 0,
	NL80211_WMI_SET_BT_PARAMS,
	NL80211_WMI_SET_BT_FT_ANT,
	NL80211_WMI_SET_COLOCATED_BT_DEV,
	NL80211_WMI_SET_BT_INQUIRY_PAGE_CONFIG,
	NL80211_WMI_SET_BT_SCO_CONFIG,
	NL80211_WMI_SET_BT_A2DP_CONFIG,
	NL80211_WMI_SET_BT_ACLCOEX_CONFIG,
	NL80211_WMI_SET_BT_DEBUG,
	NL80211_WMI_SET_BT_OPSTATUS,
	NL80211_WMI_GET_BT_CONFIG,
	NL80211_WMI_GET_BT_STATS,
#ifdef HID_PROFILE_SUPPORT
	NL80211_WMI_SET_BT_HID_CONFIG,
#endif
	NL80211_WMI_BT_MAX,
};

struct btcoex_ioctl{
	char		*cmd;
	unsigned int	cmd_len;
};

#endif /* __NL80211_UTILS_H */
