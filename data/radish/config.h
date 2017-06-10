/*
 * Copyright (c) 2011-2013 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef RADISH_CONFIG_H_INCLUDED
#define RADISH_CONFIG_H_INCLUDED
#define TRUE 1
#define FALSE 0

#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <list.h>
#include <pthread.h>
#include <stdlib.h>
#include "icmpv6.h"

#define IOV_LEN 2
#define IOV_BUFF_LEN 1500
#define IOV_PER_THREAD 6

#define RADISH_DEFAULT_BACKHAUL "rmnet"

struct prefix_item;
typedef struct prefix_item PrefixItem;
struct iface_cfg;
typedef struct iface_cfg IfaceCfg;
struct server_config;
typedef struct server_config ServerCfg;

extern ServerCfg server_cfg;

struct prefix_item {
    List the_list;

    uint32_t adv_valid_lifetime;
    uint32_t adv_preferred_life_time;

    uint8_t flags;

    uint8_t prefix_length;
    struct sockaddr_in6 addr;
};

struct buffer_item {
    List the_list;

    struct iovec iov[IOV_LEN];
};

struct iface_params {
    uint32_t max_rtr_adv_interval;
    uint32_t min_rtr_adv_interval;
    uint32_t adv_link_mtu;
    uint32_t adv_reachable_time;
    uint32_t adv_retrans_timer;
    uint32_t adv_cur_hop_limit;
    uint32_t adv_default_life_time;

    uint8_t preserve_config:1; /* Keep the interface's configuration even when it goes away */
    uint8_t adv_send_advertisements:1;
    uint8_t adv_managed:1;
    uint8_t adv_other_config:1;
    uint8_t proxy:1;
};

struct iface_cfg {
    List the_list;

    char *iface_name;
    unsigned iface_idx;
    struct iface_params iface_params;
    /* List of prefixes to advertise on this interface */
    List adv_prefix_list;

    ServerCfg *owner;
    pthread_t adv_thread;
    int adv_socket;
    int rcv_socket;
    /*struct iovec iov[IOV_LEN];*/
    int wake_socket[2];
    pthread_mutex_t adv_time_lock;
    pthread_mutex_t proxy_lock;
    List forwarded_packets_list;
    uint8_t exit;
    uint8_t need_adv;
    int nfds;
    fd_set rfds;
    fd_set wfds;
    fd_set efds;
    struct in6_addr ip6_addr;
};

struct server_config
{
    List iface_list;
    IfaceCfg global_params;
    uint8_t enable_adv_timer:1;
    char *backhaul;
    uint8_t is_bridge_mode_on;
    uint8_t use_policy_based_route;
    char *policy_route_table_name;
};

int iface_cfg_new(const char *iface_name, IfaceCfg **thecfg);
int iface_cfg_add_prefix(IfaceCfg *thecfg, const char *prefix, uint8_t prefixlen, int flags, int useflags);
int iface_cfg_delete(IfaceCfg *thecfg);
void iface_default_params(struct iface_params *params);
#endif
