/*
 * Copyright (c) 2011-2012 Qualcomm Atheros, Inc.
 * All Rights Reserved.
 * Qualcomm Atheros Proprietary and Confidential.
 */

#include <errno.h>
#include <linux/nl80211.h>
#include <net/if.h>
#include "nl80211_utils.h"

#ifndef CONFIG_LIBNL20
/* libnl 2.0 compatibility code */

static inline int __genl_ctrl_alloc_cache(struct nl_sock *h, struct nl_cache **cache)
{
	struct nl_cache *tmp = genl_ctrl_alloc_cache(h);
	if (!tmp)
		return -ENOMEM;
	*cache = tmp;
	return 0;
}
#define genl_ctrl_alloc_cache __genl_ctrl_alloc_cache
#define nl_socket_alloc nl_handle_alloc
#define nl_socket_free nl_handle_destroy

#endif /* CONFIG_LIBNL20 */

int iw_debug = 0;

int btcoex_nl_init(struct btcoex_nl_info **nl)
{
	int err;
	struct btcoex_nl_info *state;

	*nl = malloc(sizeof(struct btcoex_nl_info));

	if(!(*nl)) {
		fprintf(stderr, "Failed to allocate buffer.\n");
		return -ENOMEM;
	}
	state = *nl;

	state->nl_sock = nl_socket_alloc();
	if (!state->nl_sock) {
		fprintf(stderr, "Failed to allocate netlink socket.\n");
		return -ENOMEM;
	}

	if (genl_connect(state->nl_sock)) {
		fprintf(stderr, "Failed to connect to generic netlink.\n");
		err = -ENOLINK;
		goto out_handle_destroy;
	}

	if (genl_ctrl_alloc_cache(state->nl_sock, &state->nl_cache)) {
		fprintf(stderr, "Failed to allocate generic netlink cache.\n");
		err = -ENOMEM;
		goto out_handle_destroy;
	}

	state->nl80211 = genl_ctrl_search_by_name(state->nl_cache, "nl80211");
	if (!state->nl80211) {
		fprintf(stderr, "nl80211 not found.\n");
		err = -ENOENT;
		goto out_cache_free;
	}
	printf("Initialization of NL80211 \n");
	return 0;

 out_cache_free:
	nl_cache_free(state->nl_cache);
 out_handle_destroy:
	nl_socket_free(state->nl_sock);
	return err;
}

void btcoex_nl_deinit(struct btcoex_nl_info *state)
{
	if (!state->nl80211)
		goto cleanup;

	genl_family_put(state->nl80211);
	nl_cache_free(state->nl_cache);
	nl_socket_free(state->nl_sock);

cleanup:
	free(state);
}
static int error_handler(struct sockaddr_nl *nla, struct nlmsgerr *err,
			 void *arg)
{
	int *ret = arg;
	*ret = err->error;
	return NL_STOP;
}

static int finish_handler(struct nl_msg *msg, void *arg)
{
	int *ret = arg;
	*ret = 0;
	return NL_SKIP;
}

static int ack_handler(struct nl_msg *msg, void *arg)
{
	int *ret = arg;
	*ret = 0;
	return NL_STOP;
}

struct work_data {
	unsigned char id;
	void *user_data;
};

static int nl_send_and_recv(struct nl_sock *nl_sock,
		struct nl_msg *msg,
		int (*valid_handler)(struct nl_msg *, void *),
		void *valid_data)
{
	struct nl_cb *cb;
	int err = -ENOMEM;

	cb = nl_cb_alloc(iw_debug ? NL_CB_DEBUG : NL_CB_DEFAULT);
	if (!cb)
		goto out_msg;

	err = nl_send_auto_complete(nl_sock, msg);
	if (err < 0) {
		printf("\nSend auto error %s:%d\n", __FUNCTION__, __LINE__);
		goto out;
	}

	err = 1;

	nl_cb_err(cb, NL_CB_CUSTOM, error_handler, &err);
	nl_cb_set(cb, NL_CB_FINISH, NL_CB_CUSTOM, finish_handler, &err);
	nl_cb_set(cb, NL_CB_ACK, NL_CB_CUSTOM, ack_handler, &err);

	if (valid_handler)
		nl_cb_set(cb, NL_CB_VALID, NL_CB_CUSTOM,
				valid_handler, valid_data);

	while (err > 0)
		nl_recvmsgs(nl_sock, cb);
out:
	nl_cb_put(cb);
out_msg:
	nlmsg_free(msg);
	return err;
}

int nl80211_send_btcoex_cmd(struct btcoex_nl_info *state, int devidx,
			 char *datap, int len)
{
	struct nl_msg *msg;
	int ret = -1;

	if (!state->nl80211) {
		fprintf(stderr, "netlink Handle not found\n");
		return -EINVAL;
	}

	msg = nlmsg_alloc();
	if (!msg) {
		fprintf(stderr, "failed to allocate netlink message\n");
		return -ENOMEM;
	}

	genlmsg_put(msg, 0, 0, genl_family_get_id(state->nl80211), 0,
		    0, NL80211_CMD_BTCOEX, 0);

	NLA_PUT_U32(msg, NL80211_ATTR_IFINDEX, devidx);
	NLA_PUT(msg, NL80211_ATTR_BTCOEX_DATA, len, datap);

	ret = nl_send_and_recv(state->nl_sock, msg, NULL, NULL);

nla_put_failure:
	return ret;
}

static int info_msg_cb(struct nl_msg *msg, void *arg)
{

	struct work_data *wrk_data = (struct work_data*)arg;
	struct nlattr *tb[NL80211_ATTR_MAX + 1];
	struct genlmsghdr *gnlh = nlmsg_data(nlmsg_hdr(msg));

	nla_parse(tb, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0),
			genlmsg_attrlen(gnlh, 0), NULL);
	if (tb[NL80211_ATTR_IFINDEX])
		*((int*)wrk_data->user_data) =
			nla_get_u32(tb[NL80211_ATTR_IFINDEX]);

	wrk_data->id = 0x00;
	return 0;
}

int nl80211_get_info(struct btcoex_nl_info *state, char *name)
{
	struct nl_cb *cb;
	struct nl_msg *msg;
	int err = 0;
	int devidx = if_nametoindex("wlan0");
	struct work_data wrk_data;

	if (!name)
		devidx = if_nametoindex("wlan0");
	else
		devidx = if_nametoindex(name);

	if (!state->nl80211 || devidx < 0) {
		fprintf(stderr, "netlink Handle not found\n");
		return -EINVAL;
	}

	msg = nlmsg_alloc();
	if (!msg) {
		fprintf(stderr, "failed to allocate netlink message\n");
		return -ENOMEM;
	}

	cb = nl_cb_alloc(iw_debug ? NL_CB_DEBUG : NL_CB_DEFAULT);
	if (!cb) {
		fprintf(stderr, "failed to allocate netlink callbacks\n");
		err = 2;
		goto out_free_msg;
	}
	genlmsg_put(msg, 0, 0, genl_family_get_id(state->nl80211), 0,
			0, NL80211_CMD_GET_INTERFACE, 0);

	wrk_data.id = 1;
	wrk_data.user_data = &devidx;

	NLA_PUT_U32(msg, NL80211_ATTR_IFINDEX, devidx);
	nl_cb_set(cb, NL_CB_VALID, NL_CB_CUSTOM, info_msg_cb, &wrk_data);

	err = nl_send_auto_complete(state->nl_sock, msg);
	if (err < 0)
		goto out;

	err = 1;

	nl_cb_err(cb, NL_CB_CUSTOM, error_handler, &err);
	nl_cb_set(cb, NL_CB_FINISH, NL_CB_CUSTOM, finish_handler, &err);
	nl_cb_set(cb, NL_CB_ACK, NL_CB_CUSTOM, ack_handler, &err);

	if (err != 1)
		wrk_data.id = 0x00;

	while (wrk_data.id > 0 && err == 1)
		nl_recvmsgs(state->nl_sock, cb);

out:
	nl_cb_put(cb);
out_free_msg:
	nlmsg_free(msg);
	return devidx;
nla_put_failure:
	nl_cb_put(cb);
	fprintf(stderr, "building message failed\n");
	return -1;
}
