/*
 * Copyright (c) 2011-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "radish.h"
#include "icmpv6.h"
#include "r_ipv6.h"

uint8_t in6_solicited_mcast_pfx[13] = {
        0xff,
        0x02,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x01,
        0xff
};

#define nd_start_of_options(buffer) nd_type_size(buffer)

static inline size_t nd_type_size(uint8_t *buffer)
{
    struct icmp6_header *h = (struct icmp6_header *) buffer;
    size_t ret = 0;

    RADISH_LOG_FUNC_ENTRY;

    switch (h->type)
    {
        case ICMPV6_TYPE_ND_RS:
            ret = sizeof(struct nd_rs);
            break;
        case ICMPV6_TYPE_ND_RA:
            ret = sizeof(struct nd_ra);
            break;
        case ICMPV6_TYPE_ND_NS:
            ret = sizeof(struct nd_ns);
            break;
        case ICMPV6_TYPE_ND_NA:
            ret = sizeof(struct nd_na);
            break;
        case ICMPV6_TYPE_ND_REDIRECT:
            ret = sizeof(struct nd_re);
            break;
    }

    RADISH_LOG_FUNC_EXIT;
    return ret;
}

/*
 * Description:
 * checks if icmpv6 header is present or not
 *
 * Parameters:
 * iov: READ-ONLY message data
 * ipv6hdr: OPTIONAL placeholder for pointer to ipv6 hdr
 * icmphdr: OPTIONAL placeholder for pointer to icmp hdr
 * Return:
 * 1 for success, 0 for failure
 */
int radish_is_icmpv6_present
(
  const struct iovec *iov,
  struct r_ip6_hdr ** ipv6hdr,
  struct icmp6_header ** icmphdr
)
{
  struct r_ip6_hdr * local_ipv6hdr = NULL;
  struct icmp6_header * local_icmphdr = NULL;
  int ret = 0;

  if (!iov)
  {
    LOGE("%s: programming err", __func__);
    return 0;
  }

  /* IPv6 header must be present */
  local_ipv6hdr = (struct r_ip6_hdr *)iov->iov_base;
  if (!local_ipv6hdr)
  {
    LOGE("%s: programming err: iov_base is NULL", __func__);
    return 0;
  }

  /* see if ICMPv6 header is present */
  local_icmphdr = (struct icmp6_header *)in6_find_header(
    local_ipv6hdr, IPPROTO_ICMPV6);
  if (local_icmphdr)
  {
    /* icmpv6 header found */
    ret = 1;
  }

  /* return the header offsets found in iov into the
     placeholders if any provided */
  if (ipv6hdr)
    *ipv6hdr = local_ipv6hdr;
  if (icmphdr)
    *icmphdr = local_icmphdr;

  return ret;
}

struct parsed_nd *nd_parse
(
  struct iovec *iov,
  size_t packet_size,
  int has_ip_hdr
)
{
    struct parsed_nd *ret = NULL;
    uint8_t hdrtype;
    uint8_t *buffer = iov->iov_base;
    uint8_t *curpos = buffer;
    size_t ndsize = 0;
    struct r_ip6_hdr *ipv6hdr = NULL;
    struct icmp6_header *icmp6hdr = NULL;
    union nd_options *curopt = NULL;
    size_t num_trailing_zeroes = 0;

    RADISH_LOG_FUNC_ENTRY;

    ret = calloc(1, sizeof(struct parsed_nd));
    LOGD("%s: calloc parsed_nd %p", __func__, ret);
    if (!ret)
    {
      LOGE("memory error");
      goto error;
    }

    if (has_ip_hdr) {
        ipv6hdr = (struct r_ip6_hdr *)curpos;

        foreach_ip6_xhdr(curpos, hdrtype, ipv6hdr) {
            LOGD("%s: hdrtype: %d", __func__, hdrtype);
        }

        if (!curpos || hdrtype != IPTYPE_IPV6_ICMP) {
          LOGE("Programming Err: Not an ICMP packet!");
          goto error;
        }
    }

    icmp6hdr = (struct icmp6_header *) curpos;

    if (!is_nd(icmp6hdr)) {
        LOGD("%s: Not an ND packet (type %d)", __func__, icmp6hdr->type);
        goto error;
    }

    if (has_ip_hdr) {
        ret->dst.sin6_family = AF_INET6;
        ret->dst.sin6_addr = ipv6hdr->ip6_dst;
        ret->src.sin6_family = AF_INET6;
        ret->src.sin6_addr = ipv6hdr->ip6_src;
    }

    ret->ipv6hdr = ipv6hdr;
    ret->icmphdr = icmp6hdr;
    ndsize = nd_type_size(curpos);

    if (ndsize == 0)
    {
        LOGE("Invalid size 0");
        goto error;
    }

    ret->iov = iov;
    ret->length = (int)packet_size;
    ret->packet_type = ICMPV6_PROTO;
    /* Jump straight into the start of the options */
    curpos += ndsize;

    rlist_init(&ret->parsed_options);
    for (curopt = (union nd_options *)curpos;
            curpos < buffer + packet_size;
            (curpos += curopt->h.length * 8), curopt = (union nd_options *) curpos)
    {
        struct parsed_nd_option *popt = calloc(1, sizeof(struct parsed_nd_option));
        if (!popt)
        {
          LOGE("memory error: callc popt failed");
          goto error;
        }
        LOGD("%s: calloc option %d (len %d): %p", __func__,
                curopt->h.type, curopt->h.length, (unsigned*) popt);
        popt->option = curopt;
        if(popt->option->h.type == ND_OPT_TYPE_PREF_INFO) {
          LOGI("%s found option %d", __func__, popt->option->h.type);
          if (curpos + curopt->h.length * (size_t)8 > buffer + packet_size)
          {
            /*
             * Option header length is always multiple of 8 bytes
             * as per design
             */
            LOGI("curpos + curopt->h.length*8 = %p",
                 curpos + curopt->h.length*8);
            LOGI("buffer + packet_size = %p", buffer+packet_size);
            /*
             * if option header length indicates that there are
             * bytes exceeding the packet_size, they must be
             * trailing zeroes discarded when the message was
             * captured by using the packet sockets
             */
            num_trailing_zeroes =
              ((size_t)curpos + curopt->h.length* (size_t)8 - (size_t)(buffer + packet_size));
            LOGD("trailing %zu 0s detected, length will be amended to %zu",
                 num_trailing_zeroes, packet_size + num_trailing_zeroes);
            ret->length = (int)(packet_size + num_trailing_zeroes);
            ret->iov[0].iov_len += num_trailing_zeroes;
            LOGD("pnd->length set to %d, iov_len set to %zu",
                 ret->length, ret->iov[0].iov_len);
          }
        }

        rlist_append(&ret->parsed_options, &popt->the_list);
    }

    RADISH_LOG_FUNC_EXIT;
    return ret;
error:
    if (ret) LOGD("%s: freeing parsed_nd: %p\n", __func__, (unsigned*) ret);
    nd_free(ret);
    RADISH_LOG_FUNC_EXIT;
    return NULL;
}

static int ra_init_header(struct parsed_nd *ndp, uint8_t code, uint8_t flags,
        uint16_t router_lifetime)
{
    struct nd_ra *ra;

    if (!ndp->iov || !ndp->iov[0].iov_base
            || !ndp->icmphdr || ndp->iov[0].iov_len < MIN_RA_SIZE)
    {
        return -1;
    }

    ra = (struct nd_ra *) ndp->icmphdr;
    ra->icmp6hdr.type = ICMPV6_TYPE_ND_RA;
    ra->icmp6hdr.code = code;
    ra->cur_hop_limit = 255;
    ra->flags = flags;
    ra->reachable_time = 0;
    ra->retrans_timer = 0;
    ra->router_lifetime = router_lifetime;
    return 0;
}

int nd_get_type(uint8_t *buff, int offset)
{
    struct icmp6_header *hdr = (struct icmp6_header *) (buff + offset);

    return hdr->type;
}

int parsed_nd_get_type(struct parsed_nd *ndp)
{
    return nd_get_type((uint8_t *)ndp->icmphdr, 0);
}

/*
 * Before calling this function, opt must have been allocated.
 * This function will set opt->option to point to the buffer space
 * after the last option, and add opt to the parsed_options list.
 * Only after calling this function, can the values in opt->option be changed
 */
static int nd_add_opt(struct parsed_nd *ndp, struct parsed_nd_option *opt,
        size_t optlen)
{
    size_t curpos = nd_start_of_options((uint8_t *)ndp->icmphdr);
    size_t pktsize = (size_t)ndp->length;

    if ((pktsize + optlen) > ndp->iov[0].iov_len || !opt)
        goto error;

    opt->option = (union nd_options *) (((uint8_t *)ndp->icmphdr) + curpos);

    if (!rlist_is_empty(&ndp->parsed_options))
    {
        List *last = rlist_last(&ndp->parsed_options);
        struct parsed_nd_option *o = 0;

        if (!last)
            goto error;

        o = PARSED_OPT(last);
        if (!o)
        {
          LOGE("PARSED_OPT returned NULL pointer");
          goto error;
        }

        opt->option = (union nd_options *) (((uint8_t *)o->option) + o->option->h.length * 8);
    }

    opt->option->h.type = ND_OPT_TYPE_PREF_INFO;
    opt->option->h.length = (uint8_t)(optlen / 8);

    rlist_append(&ndp->parsed_options, &opt->the_list);

    return 0;
error:
    return -1;
}
int nd_add_opt_pref_info(struct parsed_nd *ndp, struct sockaddr_in6 *addr,
        uint8_t pfxlen, uint32_t valid_lifetime, uint32_t preferred_life_time)
{
    struct parsed_nd_option *popt = NULL;
    size_t optlen;

    optlen = sizeof(struct nd_opt_prefix_info);
    popt = calloc(1, sizeof(struct parsed_nd_option));

    if (!ndp || nd_add_opt(ndp, popt, optlen) < 0 || !addr)
        goto error;

    LOGD("%s: Allocated option %p\n", __func__, (unsigned*)popt);
    /* Set values *after* calling nd_add_opt, since popt->option is set to the buffer in ndp*/
    popt->option->prefix_info.flags = 0;
    popt->option->prefix_info.preferred_lifetime = preferred_life_time;
    popt->option->prefix_info.prefix = addr->sin6_addr;
    popt->option->prefix_info.prefix_length = pfxlen;
    popt->option->prefix_info.valid_lifetime = valid_lifetime;
    free(popt);
    return 0;
error:
    LOGD("%s: Error allocating option\n", __func__);
    LOGD("%s: Freeing parsed_nd_option: %p\n", __func__, (unsigned*)popt);
    if (popt) {
        free(popt);
    }
    return -1;
}

int nd_add_opt_lla(struct parsed_nd *ndp, uint8_t *addr, size_t addrlen)
{
    struct parsed_nd_option *popt = NULL;
    size_t optlen;

    optlen = sizeof(struct nd_opt_header) + addrlen;
    popt = calloc(1, sizeof(struct parsed_nd_option));

    if (!ndp || nd_add_opt(ndp, popt, optlen) < 0 || !addr)
        goto error;

    LOGD("%s: Allocated parsed option %p (len %zu)", __func__, (unsigned*)popt, optlen);
    memcpy(&popt->option->lla.addr.lla[0], addr, addrlen);

    free(popt);
    return 0;
error:
    LOGD("%s: Error allocating option", __func__);
    if (popt) {
      LOGD("%s: Freeing parsed_nd_option: %p\n", __func__, (unsigned*) popt);
      free(popt);
    }
    return -1;
}

struct parsed_nd_option *nd_get_option(struct parsed_nd *ndp, uint8_t option_type)
{
    List *cur;
    struct parsed_nd_option *opt = NULL;

    rlist_foreach(cur, &ndp->parsed_options) {
        opt = PARSED_OPT(cur);
        if (!opt)
        {
          LOGE("PARSED_OPT failed");
          break;
        }
        if (opt->option->h.type == option_type)
            break;
    }
    return opt;
}

size_t nd_get_size(struct parsed_nd *ndp)
{
    size_t ret = 0;
    List *cur;
    struct parsed_nd_option *opt;

    ret += nd_type_size((uint8_t *)ndp->icmphdr);
    rlist_foreach(cur, &ndp->parsed_options) {
        opt = PARSED_OPT(cur);
        if (opt) {
            ret += nd_opt_len(opt);
        }
    }
    return ret;
}

int nd_update_options(struct parsed_nd *ndp)
{
    struct parsed_nd_option *popt = NULL;
    uint8_t *tmpbuf=0;
    size_t headsize = nd_type_size((uint8_t *)ndp->icmphdr);
    uint8_t *opstart = ((uint8_t *)ndp->icmphdr) + headsize;
    List *o;
    struct icmp6_header *icmp = ndp->icmphdr;
    size_t tmpsize = 0;
    size_t cur_pos = 0;
    uint8_t *pktstart;

    rlist_foreach(o, &ndp->parsed_options)
    {
        popt = PARSED_OPT(o);
        if (!popt)
        {
          LOGE("PARSED_OPT failed");
          goto error;
        }
        tmpsize += nd_opt_len(popt);
    }

    tmpbuf = calloc(1, tmpsize);
    if (!tmpbuf)
        return -1;

    rlist_foreach(o, &ndp->parsed_options)
    {
        popt = PARSED_OPT(o);
        if (!popt)
        {
          LOGE("PARSED_OPT failed");
          goto error;
        }
        memcpy(tmpbuf + cur_pos, popt->option, nd_opt_len(popt));
        popt->option = (union nd_options *) (opstart + cur_pos);
        cur_pos += nd_opt_len(popt);
    }
    memcpy(opstart, tmpbuf, tmpsize);
    //memset(opstart + tmpsize, 0, ndp->iov[0].iov_len - headsize - tmpsize);
    pktstart = (ndp->ipv6hdr) ? (uint8_t *) ndp->ipv6hdr : (uint8_t *)ndp->icmphdr;
    ndp->length = (int)((opstart + headsize + tmpsize) - pktstart);
    icmp->checksum = 0;
    if (tmpbuf)
      free(tmpbuf);
    return 0;

error:
    if (tmpbuf)
      free(tmpbuf);
    return -1;
}

struct parsed_nd *nd_create(uint8_t type, struct iovec *vec, int iovlen, int offset)
{
    struct parsed_nd *ret;

    ret = calloc(1, sizeof(struct parsed_nd));
    if (!ret)
        goto error;
    LOGD("%s: Allocated parsed_nd: %p. iovlen: %d\n", __func__, (unsigned*) ret, iovlen);

    ret->iov = vec;
    rlist_init(&ret->parsed_options);
    ret->icmphdr = (struct icmp6_header *)(((uint8_t *)vec[0].iov_base) + offset);
    ret->icmphdr->type = type;
    ret->length = (int)nd_get_size(ret);
    switch (type)
    {
        case ICMPV6_TYPE_ND_RS:
            break;
        case ICMPV6_TYPE_ND_RA:
            /*             code, flags, router_lifetime*/
            if (ra_init_header(ret, 0, 0, 0) < 0)
                goto error;
    }
    return ret;
error:
    LOGD("%s: Error allocating nd structure", __func__);
    if (ret) {
      LOGD("%s: Freeing parsed_nd: %p\n", __func__, (unsigned*)ret);
      free(ret);
    }
    return NULL;
}

void nd_free(struct parsed_nd *pnd)
{
    List *l;
    struct parsed_nd_option *popt;


    if (!pnd) return;

    for (l = rlist_pop_first(&pnd->parsed_options); l; l = rlist_pop_first(&pnd->parsed_options)) {
        popt = PARSED_OPT(l);
        LOGD("%s: Freeing parsed_nd_option: %p\n", __func__, (unsigned*)popt);
        if(popt)
          free(popt);
    }

    LOGD("%s: Freeing parsed_nd: %p\n", __func__, (unsigned*)pnd);
    free(pnd);
}

struct parsed_mldp2_lr *mldp2_lr_parse(struct iovec *iov, size_t packet_size, int has_ip_hdr)
{
    struct parsed_mldp2_lr *ret = NULL;
    uint8_t hdrtype;
    uint8_t *buffer = iov->iov_base;
    uint8_t *curpos = buffer;
    struct mcrecord *rec = NULL;
    struct r_ip6_hdr *ip6hdr = has_ip_hdr ? (struct r_ip6_hdr *)buffer : NULL;
    struct icmp6_header *icmp6hdr = NULL;
    struct parsed_mldp2_mcrecord *parsedrecord;
    int i;
    int nrecs;

    if (has_ip_hdr) {
        foreach_ip6_xhdr(curpos, hdrtype, ip6hdr) {
            LOGD("%s: hdrtype: %d", __func__, hdrtype);
        }

        if (!curpos || hdrtype != IPTYPE_IPV6_ICMP) {
            LOGD("%s: Not an ICMP packet!", __func__);
            return NULL;
        }
    }

    icmp6hdr = (struct icmp6_header *) curpos;
    if (icmp6hdr->type != ICMPV6_TYPE_MLD2_LR) {
        LOGD("%s: Not and MLD2 Listener Report packet (type %d)", __func__, icmp6hdr->type);
        goto error;
    }

    ret = calloc(1, sizeof(struct parsed_mldp2_lr));
    if (!ret)
    {
        LOGD("%s: Unable to allocate %zu bytes", __func__, sizeof(struct parsed_mldp2_lr));
        goto error;
    }
    LOGD("%s: Allocated parsed_mldp2_lr: %p\n", __func__, (unsigned*) ret);

    ret->lr = (struct mldp2_lr *) icmp6hdr;
    rlist_init(&ret->mcrecords);
    curpos += sizeof(struct mldp2_lr);

    nrecs = ntohs(ret->lr->nrecords);
    LOGD("%s: Found %d records. curpos-buffer: %zu packet size: %zu", __func__, nrecs, ((size_t)(curpos-buffer)), packet_size);
    for (i = 0; i < (int) nrecs && ((size_t)(curpos - buffer)) < packet_size; i++) {
        char addr[MAX_IPV6_PREFIX];
        rec = (struct mcrecord *) curpos;
        parsedrecord = calloc(1, sizeof(struct parsed_mldp2_mcrecord));
        if (!parsedrecord) goto error;
        LOGD("%s: Allocated parsed_mldp2_mcrecord: %p\n", __func__, (unsigned*) parsedrecord);
        parsedrecord->record = rec;

        curpos += ntohs(rec->nsources) * sizeof(struct in6_addr);
        parsedrecord->aux = rec->auxlen ? curpos : NULL;

        curpos += rec->auxlen;
        inet_ntop(AF_INET6, &rec->mcaddr, addr, MAX_IPV6_PREFIX);
        LOGD("%s: Adding record with address: %s\n", __func__, addr);
        rlist_append(&ret->mcrecords, &parsedrecord->the_list);
    }
    if ( ((size_t)(curpos - buffer)) >= packet_size) {
        LOGE("%s: Invalid packet received", __func__);
        goto error;
    }

    return ret;
error:
    LOGD("%s: Error parsing record", __func__);
    if (ret) {
        for (parsedrecord = PARSED_MLDP2_MCREC(rlist_pop_first(&ret->mcrecords));
                parsedrecord;
                parsedrecord = PARSED_MLDP2_MCREC(rlist_pop_first(&ret->mcrecords))) {
            if (parsedrecord) {
                LOGD("%s: Freeing parsed_mldp2_mcrec: %p\n", __func__, (unsigned*)parsedrecord);
                free(parsedrecord);
            }
        }
    }
    if (ret) {
        LOGD("%s: Freeing parsed_mldp2_lr: %p\n", __func__, (unsigned*) ret);
        free(ret);
    }
    return NULL;
}

struct sockaddr_in6 *msg_get_ancilliary_addr(struct msghdr *msg)
{
    struct cmsghdr *cmsg;
    struct r_in6_pktinfo *info;
    struct sockaddr_in6 *ret;

    for(cmsg = CMSG_FIRSTHDR(msg) ; cmsg; cmsg = CMSG_NXTHDR(msg, cmsg)) {
        LOGD("%s cmsg level %d type %d len: %zu. data len: %zu", __func__,
                cmsg->cmsg_level,
                cmsg->cmsg_type, cmsg->cmsg_len,
                cmsg->cmsg_len - sizeof(struct r_in6_pktinfo));
        if (cmsg->cmsg_level == IPPROTO_IPV6 && cmsg->cmsg_type == IPV6_PKTINFO) {
            LOGD("got pktinfo");
            info = (struct r_in6_pktinfo *)CMSG_DATA(cmsg);
            ret = calloc(1, sizeof(struct sockaddr_in6));
            if (!ret)
            {
              LOGE("memory error: calloc for ret failed");
              return NULL;
            }
            ret->sin6_addr = info->ipi6_addr;
            ret->sin6_family = AF_INET6;
            ret->sin6_scope_id = info->ipi6_ifindex;
            return ret;
        }
    }
    LOGD("ancillary address not found");
    return NULL;
}

void socket_set_ancilliary_addr(int socket, struct sockaddr_in6 *addr)
{
    int res;
    struct r_in6_pktinfo info;

    info.ipi6_addr = addr->sin6_addr;
    info.ipi6_ifindex = addr->sin6_scope_id;

    if ( (res = setsockopt(socket, IPPROTO_IPV6, IPV6_PKTINFO, &info, sizeof(struct r_in6_pktinfo)) < 0) ) {
        LOGE("%s: Error setting IPV6_PKTINFO: %d", __func__, res);
    }
}

#define CARRY16(x) (((x) & 0x10000) ? 1 : 0)
/* Use this only when the containing type can't overflow due to the carry-overs */
#define CARRYS16(x) (((x) & (~0xffff)) >> 16)

static uint16_t ones_comp_sum(uint16_t *ptr, size_t size)
{
    uint32_t ret = 0;
    size_t i;
    uint16_t *cur;
    for (cur = ptr, i = 0; i < size; i++) {
        ret += ntohs(cur[i]);
        ret = (ret + CARRY16(ret)) & 0xffff;
    }
    return (uint16_t)ret;
}

uint16_t icmp6_checksum(struct r_ip6_hdr *ip6h, struct icmp6_header *icmp6h, uint16_t icmplen)
{
    uint32_t ret = 0;
    struct r_ip6_hdr tmph;
    struct in6_addr* __attribute__((__may_alias__)) ip6_addr;

    tmph = *ip6h;
    tmph.ip6_nxt = IPTYPE_IPV6_ICMP;
    tmph.ip6_plen = icmplen;

    /* Checksum computation must contain a pseudo-header, comprised of:
     * - src and dst ip addresses
     * - payload length (32 bit value)
     * - next header
     */

    ip6_addr = &ip6h->ip6_src;
    ret = ones_comp_sum((uint16_t *)ip6_addr, sizeof(struct in6_addr)/sizeof(uint16_t));
    ip6_addr = &ip6h->ip6_dst;
    ret += ones_comp_sum((uint16_t *)ip6_addr, sizeof(struct in6_addr)/sizeof(uint16_t));
    ret += ntohs(ip6h->ip6_plen);

    if (icmplen != ntohs(ip6h->ip6_plen))
      LOGE("payload length %d does not match with icmplen %d", ip6h->ip6_plen, icmplen);

    ret += (uint16_t) IPTYPE_IPV6_ICMP;
    ret += ones_comp_sum((uint16_t *)icmp6h, icmplen / sizeof(uint16_t));
    ret += (((ret) & (0xffff0000)) >> 16);

    return htons(~ret & 0xffff);
}
