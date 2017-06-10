/*
 * Copyright (c) 2011-2012,2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef RADISH_ICMPV6_H_INCLUDED
#define RADISH_ICMPV6_H_INCLUDED

#include <sys/types.h>
#include <netinet/in.h>
#include <time.h>
#include "list.h"

#define RDSH_IFACE_NAME_MAX 11

#define ALL_HOSTS_MGROUP   "FF02::1"
#define ALL_ROUTERS_MGROUP "FF02::2"
#define ALL_ROUTERS_MLD2   "FF02::16"

#define MAX_IPV6_PREFIX 40

#define ICMPV6_PROTO 58

#define ICMPV6_TYPE_MLD1_LQ         130
#define ICMPV6_TYPE_MLD1_LR         131
#define ICMPV6_TYPE_ND_RS           133
#define ICMPV6_TYPE_ND_RA           134
#define ICMPV6_TYPE_ND_NS           135
#define ICMPV6_TYPE_ND_NA           136
#define ICMPV6_TYPE_ND_REDIRECT     137
#define ICMPV6_TYPE_MLD2_LR         143

#define ND_DEFAULT_PREFIX_VALID_LIFETIME     86400
#define ND_DEFAULT_PREFIX_PREFERRED_LIFETIME 14400
#define ND_DEFAULT_PREFIX_ON_LINK            0x80
#define ND_DEFAULT_PREFIX_AUTONOMOUS         0x40

#define ND_PREFIX_FLAG_ON_LINK               0x80
#define ND_PREFIX_FLAG_AUTONOMOUS            0x40
#define ND_PREFIX_PREFER_HIGH                0x18
#define ND_PREFIX_PREFER_MEDIUM              0x00
#define ND_PREFIX_PREFER_LOG                 0x10
#define ND_PREFIX_PREFER_MASK                0x18

#define ND_OPT_TYPE_SRC_LLA   1
#define ND_OPT_TYPE_TGT_LLA   2
#define ND_OPT_TYPE_PREF_INFO 3
#define ND_OPT_TYPE_MTU       5
#define ND_OPT_TYPE_ROUTE_INF 24

#define PARSED_ND(list) (LIST_OWNER(list, struct parsed_nd, the_list))
#define PARSED_OPT(list) (LIST_OWNER(list, struct parsed_nd_option, the_list))

#define PARSED_MLDP2_LR(list) (LIST_OWNER(list, struct parsed_mldp2_lr, the_list))
#define PARSED_MLDP2_MCREC(list) (LIST_OWNER(list, struct parsed_mldp2_mcrecord, the_list))

#define RADISH_KIF_IPV6_MULTICAST_NODE_LOCAL  "FF02::1"
#define RADISH_IPV6_MULTICAST_ROUTER_ADDR  "FF02::2"
#define RADISH_ND_ROUTER_SOLICIT    133

struct icmp6_header
{
    uint8_t type;
    uint8_t code;
    uint16_t checksum;
};

struct icmpv6_header{
	u_int8_t	icmp6_type;
	u_int8_t	icmp6_code;
	u_int16_t	icmp6_cksum;
	union {
		u_int32_t	icmp6_un_data32[1];
		u_int16_t	icmp6_un_data16[2];
		u_int8_t	icmp6_un_data8[4];
	} icmp6_dataun;
} __packed;

struct nd_rs
{
    struct icmp6_header icmp6hdr;
    uint32_t reserved;
    union nd_options
    {
        struct nd_opt_header
        {
            uint8_t type;
            /* Length of the option in units of 8 bytes */
            uint8_t length;
        } h;
        struct nd_opt_lla
        {
            uint8_t type;
            uint8_t length;

            /* For types 1 and 2: Source/Target Link-layer address */
            union
            {
                uint8_t lla[0];
                uint8_t ieee802lla[6];
            } addr;
        } lla;

        /* Type 3: Prefix information */
        struct nd_opt_prefix_info
        {
            uint8_t type;
            uint8_t length;
            uint8_t prefix_length;
            uint8_t flags;
            uint32_t valid_lifetime;
            uint32_t preferred_lifetime;
            uint32_t reserved2;
            struct in6_addr prefix;
        } prefix_info;

        /* Type 5: MTU */
        struct nd_opt_mtu
        {
            uint8_t type;
            uint8_t length;
            uint16_t reserved;
            uint32_t mtu;
        } mtu;
    } options[0];
};
#define MIN_RS_SIZE (sizeof(nd_rs) +8)

struct nd_ra
{
    struct icmp6_header icmp6hdr;
    uint8_t cur_hop_limit;

#define RA_FLAG_MANAGED 0x80
#define RA_FLAG_OTHER_CFG 0x40
    uint8_t flags;
    uint16_t router_lifetime;
    uint32_t reachable_time;
    uint32_t retrans_timer;
    union nd_options options[0];
};
#define MIN_RA_SIZE (sizeof(struct nd_ra) + 8)

struct nd_ns
{
    struct icmp6_header icmp6hdr;
    uint32_t reserved;
    struct in6_addr target_address;
    union nd_options options[0];
    /* Possible options: source link-layer address */
};
#define MIN_NS_SIZE (sizeof(nd_ra) + 8)

struct nd_na
{
    struct icmp6_header icmp6hdr;
    uint8_t flags;
    uint8_t reserved1;
    uint16_t reserved2;
    struct in6_addr target_address;
    union nd_options options[0];
    /* Possible options: source link-layer address */
};
#define MIN_NA_SIZE (sizeof(nd_ra) + 8)

struct nd_re
{
    struct icmp6_header icmp6hdr;
    uint32_t reserved;
    struct in6_addr target_address;
    struct in6_addr destination_address;
    union nd_options options[0];
};

struct parsed_nd_option
{
    List the_list;
    union nd_options *option;
};

struct parsed_nd
{
    List the_list;
    struct iovec *iov;
    int length;
    int packet_type; /* used to distinguish betn ICMPv6 and UDP */
    struct r_ip6_hdr *ipv6hdr;
    struct icmp6_header *icmphdr;
    List parsed_options;
    struct sockaddr_in6 src;
    struct sockaddr_in6 dst;
    struct timespec ts;
    void *userdata;
    /*
      proxy_cookie is used to identify the source interface
      where packet arrived first. It will help in logging
      proxy operation - source iface (proxy_cookie set to self)
      and dest iface (proxy_cookie set to source iface) pointing
      so the source iface.
     */
    uint16_t proxy_cookie;
    /* used for logging iface name */
    char iface_name[RDSH_IFACE_NAME_MAX];
};

/*
 * Multicast Listener Discovery Protocol V1 (MLDV1)
 * See RFC 2710
 *
 */
struct mldp1
{
    struct icmp6_header icmp6hdr;
    uint16_t max_response_delay;
    uint16_t reserved;
    struct in6_addr mcaddr;
};

/*
 * Multicast Listener Discovery Protocol V2 (MLDV2)
 * See RFC 3810
 *
 */
/* MLDPv2 Listener report. */
struct mldp2_lr
{
    struct icmp6_header icmp6hdr;
    uint16_t reserved;
    uint16_t nrecords;

    struct mcrecord {
        uint8_t type;
        uint8_t auxlen;
        uint16_t nsources;
        struct in6_addr mcaddr;
        struct in6_addr source[0];
    } mcrecord[0];
};

struct parsed_mldp2_mcrecord
{
    List the_list;
    struct mcrecord *record;
    void *aux;
};

struct parsed_mldp2_lr
{
    List the_list;
    struct mldp2_lr *lr;
    List mcrecords; /* List of parsed_mldp2_mcrecord's */
};

static inline size_t nd_opt_len(struct parsed_nd_option *opt)
{
    return opt->option->h.length * (size_t)8;
}

static inline int is_nd(struct icmp6_header *hdr)
{
    return hdr->type == ICMPV6_TYPE_ND_NA ||
            hdr->type == ICMPV6_TYPE_ND_NS ||
            hdr->type == ICMPV6_TYPE_ND_RA ||
            hdr->type == ICMPV6_TYPE_ND_RS ||
            hdr->type == ICMPV6_TYPE_ND_REDIRECT;
}

extern int radish_is_icmpv6_present
(
  const struct iovec *iov,
  struct r_ip6_hdr ** ipv6hdr,
  struct icmp6_header ** icmphdr
);
extern struct parsed_nd *nd_parse
(
  struct iovec *iov,
  size_t packet_size,
  int has_ip_hdr
);
struct parsed_nd *nd_create(uint8_t type, struct iovec *vec, int iovlen, int offset);
size_t nd_get_size(struct parsed_nd *ndp);
void nd_free(struct parsed_nd *pnd);

int nd_add_opt_pref_info(struct parsed_nd *ndp, struct sockaddr_in6 *addr,
        uint8_t pfxlen, uint32_t valid_lifetime, uint32_t preferred_life_time);
int nd_add_opt_lla(struct parsed_nd *ndp, uint8_t *addr, size_t addrlen);
int nd_update_options(struct parsed_nd *ndp);
struct parsed_nd_option *nd_get_option(struct parsed_nd *ndp, uint8_t option_type);

struct parsed_mldp2_lr *mldp2_lr_parse(struct iovec *iov, size_t packet_size, int has_ip_hdr);

struct sockaddr_in6 *msg_get_ancilliary_addr(struct msghdr *msg);
void socket_set_ancilliary_addr(int socket, struct sockaddr_in6 *addr);
uint16_t icmp6_checksum(struct r_ip6_hdr *ip6h, struct icmp6_header *icmp6h, uint16_t icmplen);

#endif
