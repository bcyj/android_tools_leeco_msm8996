#ifndef QCMAP_FIREWALL_UTIL_H
#define QCMAP_FIREWALL_UTIL_H

/*===========================================================================

                          QCMAP_FIREWALL_UTIL . H

DESCRIPTION

  Data structure definition for inbound IP firewall entries

Copyright (c) 2004-2009,2013 Qualcomm Technologies, Inc.
All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
===========================================================================*/


#define ps_ntohs(x)                                                      \
  (((((uint16)(x) & 0x00FF) << 8) | (((uint16)(x) & 0xFF00) >> 8)))

struct ps_in_addr                   /* structure defined for historic reason */
{
  uint32 ps_s_addr;                                        /* socket address */
};

/* Local IP wildcard address */
enum
{
  PS_INADDR_ANY  = 0
};

/* IPv6 address structure */
struct ps_in6_addr
{
  union
  {
    uint8   u6_addr8[16];
    uint16  u6_addr16[8];
    uint32  u6_addr32[4];
    uint64  u6_addr64[2];
  } in6_u;

#define ps_s6_addr    in6_u.u6_addr8
#define ps_s6_addr16  in6_u.u6_addr16
#define ps_s6_addr32  in6_u.u6_addr32
#define ps_s6_addr64  in6_u.u6_addr64
};

/*---------------------------------------------------------------------------
      IP protocol numbers - use in dss_socket() to identify protocols.
      Also contains the extension header types for IPv6.
---------------------------------------------------------------------------*/
typedef enum
{
  PS_IPV6_BASE_HDR        = 4,                  /* IPv6 Base Header           */
  PS_IPPROTO_HOP_BY_HOP_OPT_HDR = 0,            /* Hop-by-hop Option Header   */
  PS_IPPROTO_ICMP         = 1,                               /* ICMP protocol */
  PS_IPPROTO_IGMP         = 2,                               /* IGMP protocol */
  PS_IPPROTO_IP           = PS_IPV6_BASE_HDR,                /* IPv4          */
  PS_IPPROTO_TCP          = 6,                                /* TCP Protocol */
  PS_IPPROTO_UDP          = 17,                               /* UDP Protocol */
  PS_IPPROTO_IPV6         = 41,                 /* IPv6                       */
  PS_IPPROTO_ROUTING_HDR  = 43,                 /* Routing Header             */
  PS_IPPROTO_FRAG_HDR     = 44,                 /* Fragmentation Header       */
  PS_IPPROTO_GRE          = 47,                               /* GRE Protocol */
  PS_IPPROTO_ESP          = 50,                               /* ESP Protocol */
  PS_IPPROTO_AH           = 51,                 /* Authentication Header      */
  PS_IPPROTO_ICMP6        = 58,                 /* ICMPv6                     */
  PS_NO_NEXT_HDR          = 59,                 /* No Next Header for IPv6    */
  PS_IPPROTO_DEST_OPT_HDR = 60,                 /* Destination Options Header */
  PS_IPPROTO_MOBILITY_HDR = 135,                /* Mobility Header            */
  PS_IPPROTO_TCP_UDP      = 253                 /* Unspecified protocol*/
} ps_ip_protocol_enum_type;

typedef enum
{
  IP_V4 = 4,
  IP_V6 = 6
} ip_version_enum_type;

typedef uint8 ipfltr_ip4_hdr_field_mask_type;
typedef uint8 ipfltr_ip6_hdr_field_mask_type;
typedef uint8 ipfltr_tcp_hdr_field_mask_type;
typedef uint8 ipfltr_udp_hdr_field_mask_type;
typedef uint8 ipfltr_icmp_hdr_field_mask_type;
typedef uint8 ipfltr_esp_hdr_field_mask_type;
typedef uint8 ipfltr_tcp_udp_hdr_field_mask_type;

/* IPV4 hdr fields */
typedef enum
{
  IPFLTR_MASK_IP4_NONE          = 0x00,
  IPFLTR_MASK_IP4_SRC_ADDR      = 0x01,
  IPFLTR_MASK_IP4_DST_ADDR      = 0x02,
  IPFLTR_MASK_IP4_NEXT_HDR_PROT = 0x04,
  IPFLTR_MASK_IP4_TOS           = 0x08,
  IPFLTR_MASK_IP4_ALL           = 0x0f
} ipfltr_ip4_hdr_field_mask_enum_type;

/* IPV6 hdr fields */
typedef enum
{
  IPFLTR_MASK_IP6_NONE          = 0x00,
  IPFLTR_MASK_IP6_SRC_ADDR      = 0x01,
  IPFLTR_MASK_IP6_DST_ADDR      = 0x02,
  IPFLTR_MASK_IP6_NEXT_HDR_PROT = 0x04,
  IPFLTR_MASK_IP6_TRAFFIC_CLASS = 0x08,
  IPFLTR_MASK_IP6_FLOW_LABEL    = 0x10,
  IPFLTR_MASK_IP6_ALL           = 0x1f
} ipfltr_ip6_hdr_field_mask_enum_type;

/* Higher level protocol hdr parameters */

/* TCP hdr fields */
typedef enum
{
  IPFLTR_MASK_TCP_NONE          = 0x00,
  IPFLTR_MASK_TCP_SRC_PORT      = 0x01,
  IPFLTR_MASK_TCP_DST_PORT      = 0x02,
  IPFLTR_MASK_TCP_ALL           = 0x03
} ipfltr_tcp_hdr_field_mask_enum_type;

/* UDP hdr fields */
typedef enum
{
  IPFLTR_MASK_UDP_NONE          = 0x00,
  IPFLTR_MASK_UDP_SRC_PORT      = 0x01,
  IPFLTR_MASK_UDP_DST_PORT      = 0x02,
  IPFLTR_MASK_UDP_ALL           = 0x03
} ipfltr_udp_hdr_field_mask_enum_type;

/* ICMP hdr fields */
typedef enum
{
  IPFLTR_MASK_ICMP_NONE         = 0x00,
  IPFLTR_MASK_ICMP_MSG_TYPE     = 0x01,
  IPFLTR_MASK_ICMP_MSG_CODE     = 0x02,
  IPFLTR_MASK_ICMP_ALL          = 0x03
} ipfltr_icmp_hdr_field_mask_enum_type;

/* ESP hdr fields */
typedef enum
{
  IPFLTR_MASK_ESP_NONE          = 0x00,
  IPFLTR_MASK_ESP_SPI           = 0x01,
  IPFLTR_MASK_ESP_ALL           = 0x01
} ipfltr_esp_hdr_field_mask_enum_type;

/* TCP UDP hdr fields */
typedef enum
{
  IPFLTR_MASK_TCP_UDP_NONE          = 0x00,
  IPFLTR_MASK_TCP_UDP_SRC_PORT      = 0x01,
  IPFLTR_MASK_TCP_UDP_DST_PORT      = 0x02,
  IPFLTR_MASK_TCP_UDP_ALL           = 0x03
} ipfltr_tcp_udp_hdr_field_mask_enum_type;

/*---------------------------------------------------------------------------
TYPEDEF IP_FILTER_TYPE

DESCRIPTION
  This data structure defines the IP filter parameters for a Default filter
  type. A default filter contains all the common parameters required for most
  of the filtering needs and are processed by a default set of rules
  (ie pattern matching on parameters).

  All the address/port number fields must be specified in network byte
  order, everything else in host byte order.

  Rules:

---------------------------------------------------------------------------*/
typedef struct
{
  /* Mandatory Parameter - IP version of the filter (v4 or v6)	*/
  ip_version_enum_type  ip_vsn;

  /* Filter parameter values,  the ones set in field masks are only valid */
  /* Correspopnding err mask is set if a parameter value is invalid */
  union
  {
    struct
    {
      ipfltr_ip4_hdr_field_mask_type      field_mask;  /* In mask   */
      ipfltr_ip4_hdr_field_mask_type      err_mask;    /* Out mask  */

      struct
      {
        struct ps_in_addr  addr;
        struct ps_in_addr  subnet_mask;
      } src;

      struct
      {
        struct ps_in_addr  addr;
        struct ps_in_addr  subnet_mask;
      } dst;

      struct
      {
        uint8 val;
        uint8 mask;
      } tos;

      uint8 next_hdr_prot;
    } v4;

    struct
    {
      ipfltr_ip6_hdr_field_mask_type      field_mask;  /* In mask   */
      ipfltr_ip6_hdr_field_mask_type      err_mask;    /* Out mask  */

      struct
      {
        struct ps_in6_addr  addr;
        uint8               prefix_len;
      } src;

      struct
      {
        struct ps_in6_addr addr;
        uint8              prefix_len;
      } dst;

      struct
      {
        uint8   val;
        uint8   mask;
      } trf_cls;

      uint32   flow_label;
      uint8    next_hdr_prot;   /* This is transport level protocol header */
    } v6;
  } ip_hdr;

  /* next_hdr_prot field in v4 or v6 hdr must be set to specify a    */
  /* parameter from the next_prot_hdr                                */
  union
  {
    struct
    {
      ipfltr_tcp_hdr_field_mask_type      field_mask;  /* In mask   */
      ipfltr_tcp_hdr_field_mask_type      err_mask;    /* Out mask  */

      struct
      {
        uint16    port;
        uint16    range;
      } src;

      struct
      {
        uint16    port;
        uint16    range;
      } dst;
    } tcp;

    struct
    {
      ipfltr_udp_hdr_field_mask_type      field_mask;  /* In mask   */
      ipfltr_udp_hdr_field_mask_type      err_mask;    /* Out mask  */

      struct
      {
        uint16    port;
        uint16	  range;
      } src;

      struct
      {
        uint16    port;
        uint16    range;
      } dst;
    } udp;

    struct
    {
      ipfltr_icmp_hdr_field_mask_type     field_mask; /* In mask   */
      ipfltr_icmp_hdr_field_mask_type     err_mask;   /* Out mask  */

      uint8   type;
      uint8   code;
    } icmp;

    struct
    {
      ipfltr_esp_hdr_field_mask_type     field_mask; /* In mask   */
      ipfltr_esp_hdr_field_mask_type     err_mask;   /* Out mask  */

      uint32  spi;
    } esp;

    struct
    {
      ipfltr_tcp_udp_hdr_field_mask_type      field_mask;  /* In mask   */
      ipfltr_tcp_udp_hdr_field_mask_type      err_mask;    /* Out mask  */

      struct
      {
        uint16    port;
        uint16    range;
      } src;

      struct
      {
        uint16    port;
        uint16    range;
      } dst;

    } tcp_udp_port_range;
  } next_prot_hdr;

  struct
  {
    uint16 fi_id;          /* Filter ID */
    uint16 fi_precedence;  /* Filter precedence */
  } ipfltr_aux_info;

} ip_filter_type;

#endif /* QCMAP_FIREWALL_UTIL_H */
