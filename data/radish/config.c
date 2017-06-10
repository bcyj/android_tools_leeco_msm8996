/*
 * Copyright (c) 2011,2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#include <string.h>
#include <stdlib.h>
#include "radish.h"
#include "config.h"
#include "list.h"
#include "icmpv6.h"
#include <arpa/inet.h>

#define IPV6_ADDR_BITLEN 128

const char* iface_find_default_iface(void)
{
    return "usb0";
}

void iface_default_params(struct iface_params *params)
{
    if(!params) return;

    params->adv_send_advertisements = TRUE;
    params->max_rtr_adv_interval = 600;
    params->min_rtr_adv_interval = 9;
    params->adv_managed = FALSE;
    params->adv_other_config = FALSE;
    params->adv_link_mtu = 0;
    params->adv_reachable_time = 0;
    params->adv_retrans_timer = 0;
    params->adv_cur_hop_limit = 0;
    params->adv_default_life_time = 1800;
    params->proxy = FALSE;
}

int iface_cfg_new(const char *iface_name, IfaceCfg **thecfg)
{
    IfaceCfg *cfg = NULL;

    if (!thecfg) return -1;

    if (!iface_name) {
        iface_name = iface_find_default_iface();
    }

    cfg = calloc(1,sizeof(IfaceCfg));
    if (!cfg)
    {
      LOGE("memory error: cfg calloc failed");
      return -1;
    }

    cfg->iface_name = strdup(iface_name);
    iface_default_params(&cfg->iface_params);
    cfg->exit = FALSE;
    cfg->need_adv = FALSE;
    rlist_init(&cfg->adv_prefix_list);
    rlist_init(&cfg->forwarded_packets_list);
    pthread_mutex_init(&cfg->proxy_lock, NULL);

    *thecfg = cfg;
    return 0;
}

int prefix_item_new(const char *prefix, uint8_t prefixlen, PrefixItem **item) {
    PrefixItem *ret;

    if (!item || !prefix || prefixlen > IPV6_ADDR_BITLEN) return -1;

    ret = calloc(1, sizeof(PrefixItem));
    if (!ret) return -1;

    ret->adv_valid_lifetime = ND_DEFAULT_PREFIX_VALID_LIFETIME;
    ret->adv_preferred_life_time = ND_DEFAULT_PREFIX_PREFERRED_LIFETIME;
    ret->flags = ND_PREFIX_FLAG_ON_LINK | ND_PREFIX_FLAG_AUTONOMOUS;
    ret->prefix_length = prefixlen;

    if (inet_pton(AF_INET6, prefix, &ret->addr.sin6_addr) <= 0) {
        LOGE("Unable to parse %s as an ip address\n", prefix);
        free(ret);
        return -1;
    }

    *item = ret;
    return 0;
}

int iface_cfg_add_prefix(IfaceCfg *thecfg, const char *prefix, uint8_t prefixlen, int flags, int useflags)
{
    PrefixItem *new_item;
    int ret;
    if (!thecfg) {
        LOGE("Invalid config passed to %s\n", __func__);
        return -1;
    }

    ret = prefix_item_new(prefix, prefixlen, &new_item);
    if (ret) {
        return ret;
    }

    rlist_append(&thecfg->adv_prefix_list, &(new_item->the_list));
    if (useflags)
        new_item->flags = (uint8_t)flags;
    LOGD("Added new prefix %s/%d to interface %s\n",prefix, prefixlen, thecfg->iface_name);
    return 0;
}
