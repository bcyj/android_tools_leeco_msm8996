/*
 * Copyright (c) 2011-2012,2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef R_IPV6_H_INCLUDED
#define R_IPV6_H_INCLUDED

#include "r_iptypes.h"

#define IPV6_ADDR_LEN 16 // in bytes

struct r_ip6_hdr {
    union {
        struct ip6_hdrctl {
            uint32_t ip6_un1_flow;   /* 24 bits of flow-ID */
            uint16_t ip6_un1_plen;   /* payload length */
            uint8_t  ip6_un1_nxt;    /* next header */
            uint8_t  ip6_un1_hlim;   /* hop limit */
        } ip6_un1;
        uint8_t ip6_un2_vfc;       /* 4 bits version, 4 bits priority */
    } ip6_ctlun;
    struct in6_addr ip6_src;      /* source address */
    struct in6_addr ip6_dst;      /* destination address */
};

#define ip6_vfc   ip6_ctlun.ip6_un2_vfc
#define ip6_flow  ip6_ctlun.ip6_un1.ip6_un1_flow
#define ip6_plen  ip6_ctlun.ip6_un1.ip6_un1_plen
#define ip6_nxt   ip6_ctlun.ip6_un1.ip6_un1_nxt
#define ip6_hlim  ip6_ctlun.ip6_un1.ip6_un1_hlim

struct r_ip6_extnhdr {
    uint8_t ip6h_nxt;
    uint8_t ip6h_len;
};

#define foreach_ip6_xhdr(nexthdr, nexttype, ipv6hdr) \
    for ( \
            nexthdr = ip6_nexthdr((uint8_t *)ipv6hdr, IPTYPE_IPV6, &nexttype); \
            nexthdr && is_ip6_xhdr(nexttype); \
            nexthdr = ip6_nexthdr((uint8_t *)nexthdr, nexttype, &nexttype) \
        )

static inline int is_ip6_xhdr(uint8_t nexttype)
{
    return nexttype == IPTYPE_HOPOPT ||
            nexttype == IPTYPE_IPV6_ROUTE ||
            nexttype == IPTYPE_IPV6_FRAG ||
            nexttype == IPTYPE_IPV6_OPTS ||
            nexttype == IPTYPE_AH ||
            nexttype == IPTYPE_ESP;
}

static inline uint8_t *ip6_nexthdr(uint8_t *hdr, uint8_t type, uint8_t *next_type)
{
    uint8_t *ret = NULL;
    if (!hdr)
    {
      LOGE("header pointer is NULL");
      return ret;
    }

    switch (type) {
        case IPTYPE_IPV6: {
            struct r_ip6_hdr *h = (struct r_ip6_hdr *)hdr;
            if (next_type)
                *next_type = h->ip6_nxt;
            ret = hdr + sizeof(struct r_ip6_hdr);
            break;
        }
        case IPTYPE_AH :
        case IPTYPE_ESP:
            /* Not currently supported */
            ret = NULL;
            break;
        case IPTYPE_HOPOPT:
        case IPTYPE_IPV6_ROUTE:
        case IPTYPE_IPV6_FRAG:
        case IPTYPE_IPV6_OPTS: {
            struct r_ip6_extnhdr *h = (struct r_ip6_extnhdr *)hdr;
            if (next_type)
                *next_type = h->ip6h_nxt;
            ret = hdr + (h->ip6h_len + 1) * 8;
        }
    }
    return ret;
}

/* Look for the first header of type 'to_find' in the packet starting
 * at hdr. Returns a pointer to the start of the found header or NULL
 * if not found
 */
static inline uint8_t *in6_find_header
(
  struct r_ip6_hdr *hdr,
  uint8_t to_find
)
{
  uint8_t *nexthdr = NULL;
  uint8_t nexttype;

  if (!hdr)
  {
    LOGE("hdr is NULL");
    return NULL;
  }

  /* loop through all extension headers */
  foreach_ip6_xhdr(nexthdr, nexttype, hdr) {
    if(nexttype == to_find) 
      return nexthdr;
  }

  /* if the one we are searching was not an extension header,
   *  it will be the last header in the packet 
   */
  if(nexttype == to_find)
    return nexthdr;

  return NULL;
}


extern uint8_t in6_solicited_mcast_pfx[13];

static inline int in6_make_solicited_mcast(struct in6_addr *dst, struct in6_addr *orig)
{
    if (!dst || !orig) return -1;

    *dst = *orig;
    memcpy(dst, in6_solicited_mcast_pfx, sizeof(in6_solicited_mcast_pfx));
    return 0;
}
/*
 * From RFC 3542
 * Advanced Sockets Application Program Interface (API) for IPv6
 *
 */
struct r_in6_pktinfo {
    struct in6_addr ipi6_addr;    /* src/dst IPv6 address */
    unsigned int    ipi6_ifindex; /* send/recv interface index */
};

static inline void r_ip6_init(struct r_ip6_hdr *ip6h, struct in6_addr *src, struct in6_addr *dst,
                         uint8_t nextproto, uint16_t protolen)
{
    ip6h->ip6_flow = 0;
    ip6h->ip6_plen = (uint16_t)htons(protolen);
    ip6h->ip6_nxt = nextproto;
    ip6h->ip6_hlim = 255;
    ip6h->ip6_vfc = 0x60;
    ip6h->ip6_src = *src;
    ip6h->ip6_dst =  *dst;
}

#endif /* R_IPV6_H_INCLUDED */
