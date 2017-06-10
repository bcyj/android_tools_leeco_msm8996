/******************************************************************************

                           R A D I S H . C

******************************************************************************/

/******************************************************************************

  @file    radish.c
  @brief   User space Proxy NDP implementation

  DESCRIPTION
  User space Proxy NDP implementation

******************************************************************************/
/*===========================================================================

  Copyright (c) 2011 - 2015 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

===========================================================================*/

#include <time.h>
#include <strings.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <sys/stat.h>
#include <linux/sockios.h>
#include <linux/types.h>
#include <linux/ipv6_route.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>
#include <linux/filter.h>
#include "radish.h"
#include "list.h"
#include "config.h"
#include "radish.h"
#include "icmpv6.h"
#include "r_ipv6.h"

#define MRT6_INIT 200
#define MAX_IFACE_LEN 50
#define IS_PREFIX_LIST_EMPTY(head) ({ head.next == &head;})
#define RADISH_TIMER_EVT(list) (LIST_OWNER(list, struct radish_timer_evt, the_list))

ServerCfg server_cfg;
char g_cur_iface[MAX_IFACE_LEN] ;


#define MAX_CMD_LEN 200
#define RADISH_ADD_OPERATION      1
#define RADISH_DELETE_OPERATION   0
#define RADISH_ADD                "add"
#define RADISH_DELETE             "del"
#define RADISH_RULE_PRIORITY_TETHERING   18000

#define RADISH_NS_LIST_CLEANUP_SLEEP_TIME       10

#define IN6_IS_ADDR_UNSPECIFIED_32(a)  \
     (((&(a)->s6_addr32[0]) == 0) &&  \
      ((&(a)->s6_addr32[1]) == 0) &&  \
      ((&(a)->s6_addr32[2]) == 0) &&  \
      ((&(a)->s6_addr32[3]) == 0))

#define IPV6_LINK_LOCAL_ADDRESS  "fe80::/64"

static struct in6_addr all_routers_in6_addr;
static struct in6_addr all_mld2_routers_in6_addr;
static struct in6_addr all_hosts_in6_addr;

static List available_buffers;
static pthread_mutex_t available_lock = PTHREAD_MUTEX_INITIALIZER;

static struct option radish_opts[] = {
  { "prefix", 1, 0, 'p' },
  { "proxy",  0, 0, 'x'},
  { NULL,     0, 0, 0}
};

typedef void (*radish_timer_cb) (void *userdata);
struct radish_timer_evt
{
  List the_list;
  void *userdata;
  radish_timer_cb callback;
  struct timespec time;
};

struct mldv1_evt
{
  IfaceCfg *cfg;
  struct buffer_item *itm;
  int iovlen;
  struct r_ip6_hdr *ip6h;
  struct icmp6_header *icmp6h;
};

struct radish_timer
{
  pthread_mutex_t mutex;
  timer_t timer;
  struct timespec nextevt;
  List next_events;
};

/* There must be only one timer in the process */
struct radish_timer global_timer;

struct ra_cache
{
  struct nd_ra  global_ra;
};

struct ra_cache ra_from_modem;

#define log_sockaddr(addr) ({ \
    struct sockaddr_in6 *sa = (addr); \
    char saddr[MAX_IPV6_PREFIX + 1]; \
    memset(saddr, 0, sizeof(saddr)); \
    inet_ntop(sa->sin6_family, &sa->sin6_addr, saddr, MAX_IPV6_PREFIX); \
    LOGD("%s: %s", __func__, saddr);})

#ifdef RADISH_OFFLINE
#ifdef FEATURE_DATA_LINUX_LE
#define RADISH_PROXY_LOG_FILE "/var/rdsh_proxy_log.dat"
#define RADISH_PROXY_RAW_FILE "/var/rdsh_proxy_raw.dat"
#else
#define RADISH_PROXY_LOG_FILE "/local/tmp/rdsh_proxy_log.dat"
#define RADISH_PROXY_RAW_FILE "/local/tmp/rdsh_proxy_raw.dat"
#endif /*FEATURE_DATA_LINUX_LE*/
#else
#define RADISH_PROXY_LOG_FILE "/data/radio/rdsh_proxy_log.dat"
#define RADISH_PROXY_RAW_FILE "/data/radio/rdsh_proxy_raw.dat"
#endif /*RADISH_OFFLINE*/

#define RADISH_BRIDGE_INTERFACE_NAME  "bridge0"

static FILE * radish_log_f;
static FILE * radish_raw_f;
static int log_f_size;
static int raw_f_size;
#define RADISH_MAX_LOG_FILE_SIZE 10000

#if defined FEATURE_DATA_LINUX_ANDROID
#define ROUTE_TABLE_LOCAL_NETWORK 97
#endif /* FEATURE_DATA_LINUX_ANDROID */

#define RADISH_RESET_LOG_FILE(f_handle, f_size) \
  do \
  { \
    if (f_size >= RADISH_MAX_LOG_FILE_SIZE) \
    { \
      fseek(f_handle, 0, SEEK_SET); \
      f_size = 0; \
    } \
  } while(0)


#define RADISH_ADD_COUNTER_FORWARD_DROPALL 1
#define RADISH_DEL_COUNTER_FORWARD_DROPALL 0
#define RADISH_ADD_COUNTER_FORWARD_RULE 1
#define RADISH_DEL_COUNTER_FORWARD_RULE 0
#define RADISH_TETHER_COUNTERS_CHAIN "natctrl_tether_counters"
#define RADISH_FORWARD_CHAIN "natctrl_FORWARD"

int radish_send_nd_packet(int socket, struct parsed_nd *ndp);
int radish_send_packet(int socket, struct iovec *iov, int iovlen,
                       struct sockaddr_in6 *dst);
static void radish_schedule_mldv1_lq(IfaceCfg *cfg, struct mldv1_evt *mevt);
int is_timer_expired(struct radish_timer *timer);
static int __is_timer_expired(struct radish_timer *timer);
static uint16_t radish_nd_checksum(struct parsed_nd *pnd);
static uint16_t radish_icmp6_checksum(struct r_ip6_hdr *ip6h, struct icmp6_header *icmp6h, int len);
void radish_register_to_mcgroup(IfaceCfg *cfg, struct in6_addr *mcgroup);
void radish_modify_policy_based_route(char *table_name, int operation);
void radish_modify_link_local_route(int operation);
static int radish_setup_ipv6_stat_counters
(
  char *iface,
  char *tether_counters_chain,
  char *forward_chain
);

static inline void radish_log_proxy_operation
(
  struct parsed_nd * pnd,
  const char * operation
)
{
  do
  {
    char saddr[MAX_IPV6_PREFIX+1], daddr[MAX_IPV6_PREFIX+1];
    int count=0;

    if (!pnd)
    {
      LOGE("log_proxy: invalid pnd");
    }
    else
    {
      inet_ntop(pnd->src.sin6_family,
                &(pnd->src.sin6_addr),
                saddr,
                MAX_IPV6_PREFIX);

      inet_ntop(pnd->dst.sin6_family,
                &(pnd->dst.sin6_addr),
                daddr,
                MAX_IPV6_PREFIX);

      if (radish_log_f)
      {
        if (pnd->packet_type == ICMPV6_PROTO && pnd->icmphdr)
        {
          count = fprintf(radish_log_f, "%s,%d,%s,%d,%s,%s\n",
                          operation, pnd->proxy_cookie,
                          pnd->iface_name,
                          pnd->icmphdr->type,
                          saddr, daddr);
        }
        else if (pnd->packet_type == IPPROTO_UDP)
        {
          count = fprintf(radish_log_f, "%s,%d,%s,%d,%s,%s\n",
                          operation, pnd->proxy_cookie,
                          pnd->iface_name,
                          pnd->ipv6hdr->ip6_nxt, saddr, daddr);
        }
        else
        {
          LOGE("invalid pnd packet type - ignore");
        }

        log_f_size += count;

        /* flush the content */
        fflush(radish_log_f);

        /* contain log file to a certain size */
        RADISH_RESET_LOG_FILE(radish_log_f, log_f_size);
      }
    }
  } while (0);
}

/*
 * Description:
 * stores the message as-is into the given raw log file
 *
 * Parameters:
 * iov: READ-ONLY message data
 *
 * Return:
 * void
 */
static inline void
radish_log_raw_msg
(
  struct iovec *iov
)
{
  int i=0;

  if (!radish_raw_f)
    return;

  /* preemble */
  (void)fprintf(radish_raw_f, "%8.8x", 0xffff);
  i=(int)fwrite(iov->iov_base, 1, iov->iov_len, radish_raw_f);

  /* postemble */
  (void)fprintf(radish_raw_f, "%8.8x", 0xffff);
  raw_f_size += i;

  if (fflush(radish_raw_f))
    LOGE("couldn't flush raw file %p", radish_raw_f);

  RADISH_RESET_LOG_FILE(radish_raw_f, raw_f_size);
}

/* generate random hash number that is only used as
 * a unique identifier for the log message
 */
static inline int radish_random_hash
(
  struct iovec * iov
)
{
  unsigned long i;
  intptr_t hash=0;
  int *ptr=NULL;

  if (!iov)
  {
    LOGE("programming err");
    return 0;
  }

  ptr = (int *)iov->iov_base;

  /* add up every alternate integer from raw packet
   * to come up with random hash */
  for (i=0;
      i<iov->iov_len && i<IOV_BUFF_LEN;
      ptr++,i+=(2*(sizeof(*ptr))))
  {
    hash += (intptr_t)iov->iov_base;
  }

  return (int)hash;
}



int iface_addr_start_traverse
(
  void
)
{
#define IP6_ADDR_PATH "/proc/net/if_inet6"
  int f;
  f = open(IP6_ADDR_PATH, O_RDONLY);
  return f;
}

void iface_addr_end_traverse
(
  int f
)
{
  close(f);
}

/**
 * Get the next address for interface iface_name
 * iface_name is the interface to look for. If not provided, all
 *            the interfaces will be traversed, but the interface
 *            name will not be known. Provide a valid buffer in
 *            iface_name, and give it's length in nmlen to get
 *            the name
 * nmlen Must be zero if an interface name is being provided in
 *            iface_name. If its not zero, and iface_name is not
 *            null, it is considered to be the length of a buffer
 *            provided to store the interface name, and all
 *            interfaces will be traversed.
 * addr: The place where to store the address.
 *
 * Returns 0 on success, -1 on failure.
 */
static int get_next_iface_addr
(
  int f,
  char *iface_name,
  int nmlen,
  struct in6_addr *addr
)
{
#define ADDR_BUF_LEN 54
#define IFACE_START 45
  char ln[ADDR_BUF_LEN + 1];
  char *curifname;
  int ret = -1;
  ssize_t res;
  int all_ifaces = !iface_name || nmlen > 0;
  int i;

  /* TODO: use rtnl
   * The following format is assumed while parsing the file:
   *           1         2         3         4         5
   * 0         0         0         0         0         0    5
   * IIIIIIIIIIIIIIIIIIIIIIIIIIIIIIII xx xx xx xx  ifacenm!
   *
   * where I... represents the ipv6 address
   * xx: Unused fields
   * ifacenm: Name of the interface.
   * !: the new line character
   * Each line is assumed 54 bytes in length (including a NL character)
   * The interface name is right-aligned, and can start at position 45
   *
   * The function modifies the buffer in place to have the following:
   * IIII:IIII:IIII:IIII:IIII:IIII:IIII:IIII.
   *
   **/

  RADISH_LOG_FUNC_ENTRY;

  if (!addr || (!all_ifaces && !iface_name) ||
        (all_ifaces && iface_name && nmlen <= 0))
  {
    LOGE("%s: invalid params", __func__);
    RADISH_LOG_FUNC_EXIT;
    return -1;
  }

  curifname = &ln[IFACE_START];
  for (res = read(f, ln, ADDR_BUF_LEN);
      res > 0 && !all_ifaces && !strstr(curifname, iface_name);
      res = read(f, ln, ADDR_BUF_LEN)); // Do nothing

  if (res > 0 )
  {
    memset(addr, 0, sizeof(*addr));
    curifname = &ln[IFACE_START];

    /*
     * Skip the whitespace that may be at the start of the iface name
     */
    while (*curifname == ' ' && curifname < ln + ADDR_BUF_LEN) curifname ++;

    if (curifname >= ln + ADDR_BUF_LEN)
    {
      ret = -1;
      goto finish;
    }
    if (all_ifaces && iface_name)
    {
      strlcpy(iface_name, curifname, (size_t)nmlen);
    }

    /*
     * The ip address is in a compact format without ':' chars
     * stretch it out to insert the colons. There are 8 groups
     * of 4 hex chars each, and we add 7 ':' chars
     */
    for (i = 7; i > 0; i--)
    {
      ln[i * 4 + i + 3] = ln[i * 4 + 3];
      ln[i * 4 + i + 2] = ln[i * 4 + 2];
      ln[i * 4 + i + 1] = ln[i * 4 + 1];
      ln[i * 4 + i] = ln[i * 4];
      ln[i * 4 + i - 1] = ':';
    }
    ln[4*8 + 7] = 0;
    LOGD("%s: Found address %s", __func__, ln);
    inet_pton(AF_INET6, ln, addr);
    ret = 0;
  }

  finish:
  RADISH_LOG_FUNC_EXIT;
  return ret;
}

int radish_find_ip6_addr
(
  IfaceCfg *cfg,
  struct in6_addr *dst
)
{
  char addr[MAX_IPV6_PREFIX + 1];
  int f;
  int ret = -1;

  RADISH_LOG_FUNC_ENTRY;

  if (!dst || !cfg)
  {
    LOGE("bad parameters");
    RADISH_LOG_FUNC_EXIT;
    return -1;
  }

  memset(dst, 0, sizeof(*dst));

  f = iface_addr_start_traverse();
  LOGD("%s: Trying to find ip for interface %s", __func__, cfg->iface_name);
  if (!get_next_iface_addr(f, cfg->iface_name, 0, dst))
  {
    inet_ntop(AF_INET6, dst, addr, MAX_IPV6_PREFIX);
    LOGD("%s: Address found (%s) : %s", __func__, cfg->iface_name, addr);
  }
  iface_addr_end_traverse(f);

  RADISH_LOG_FUNC_EXIT;
  return ret;
}


static struct buffer_item *radish_fetch_available_buffer
(
  const char * calling_func
)
{
  List *itm;
  struct buffer_item *buf = NULL;

  if (!calling_func)
  {
    LOGE("%s: programming err: calling func NULL", __func__);
    return NULL;
  }

  pthread_mutex_lock(&available_lock);
  itm = rlist_pop_first(&available_buffers);
  LOGD("%s: avail itm popped: %p", calling_func, itm);
  pthread_mutex_unlock(&available_lock);

  if (itm)
  {
    buf = BUFF_ITM(itm);
  }

  return buf;
}

static void radish_release_buffer
(
  struct buffer_item *buf,
  const char * calling_func
)
{
  if (!buf || !calling_func)
  {
    LOGE("%s: programming err: bad params", __func__);
    return;
  }

  pthread_mutex_lock(&available_lock);
  memset(buf->iov[0].iov_base, 0, IOV_BUFF_LEN);
  buf->iov[0].iov_len = IOV_BUFF_LEN;
  memset(buf->iov[1].iov_base, 0, IOV_BUFF_LEN);
  buf->iov[1].iov_len = IOV_BUFF_LEN;
  rlist_append(&available_buffers, &buf->the_list);
  LOGV("%s: avail itm appended: %p", calling_func, buf);
  pthread_mutex_unlock(&available_lock);
}

static void radish_exec_debug_command
(
  char * cmd
)
{
  FILE *fp;
  char res[1035];

  if (cmd == NULL)
  {
    LOGE("Got NULL as command");
    return;
  }

  /* Open the command for reading. */
  fp = popen(cmd, "r");
  if (fp == NULL)
  {
    LOGE("Cannot execute command : %s", cmd);
    return;
  }

  /* Read the output a line at a time - output it. */
  while (fgets(res, sizeof(res)-1, fp) != NULL)
  {
    LOGD("%s", res);
  }
  /* close */
  pclose(fp);
}

static void radish_bridge_debug_cmds
(
)
{
  char cmd[MAX_CMD_LEN + 1];

  snprintf(cmd,
           MAX_CMD_LEN,
           "ebtables -t broute -L");
  LOGD("%s: %s", __func__, cmd);
  radish_exec_debug_command(cmd);

  snprintf(cmd,
           MAX_CMD_LEN,
           "brctl show");
  LOGD("%s: %s", __func__, cmd);
  radish_exec_debug_command(cmd);

    snprintf(cmd,
           MAX_CMD_LEN,
           "netstat -ap");
  LOGD("%s: %s", __func__, cmd);
  radish_exec_debug_command(cmd);
}

static int
radish_init_bridge
(
)
{
  char cmd[MAX_CMD_LEN + 1];


  radish_bridge_debug_cmds();

  /*Command to create a 802.1d bridge interface*/
  snprintf(cmd,
           MAX_CMD_LEN,
           "brctl addbr %s",
           RADISH_BRIDGE_INTERFACE_NAME);

  LOGD("%s: %s", __func__, cmd);

  if (system(cmd) != 0)
  {
    LOGE("Cannot create the bridge interface");
    /*Ignore this error for now, until cleanup handler implemented*/
    //return RADISH_ERROR;
  }

  /* Set optimistic dad and retransmission timer. This is to ensure that the interface has a valid
   * address by the time we start sending packets. */
  snprintf(cmd,
           MAX_CMD_LEN,
           "echo 1 > /proc/sys/net/ipv6/conf/%s/optimistic_dad",
           RADISH_BRIDGE_INTERFACE_NAME);

  LOGD("%s: %s", __func__, cmd);

  if (system(cmd) != 0)
  {
    LOGE("Failed to set optimistic dad");
    return RADISH_ERROR;
  }

  snprintf(cmd,
           MAX_CMD_LEN,
           "echo 10 > /proc/sys/net/ipv6/neigh/%s/retrans_time_ms",
           RADISH_BRIDGE_INTERFACE_NAME);

  LOGD("%s: %s", __func__, cmd);

  if (system(cmd) != 0)
  {
    LOGE("Failed to set the retransmission timer");
    return RADISH_ERROR;
  }

  /*Set the bridge interface state to UP*/
  snprintf(cmd,
           MAX_CMD_LEN,
           "ifconfig %s up",
           RADISH_BRIDGE_INTERFACE_NAME);

  LOGD("%s: %s", __func__, cmd);

  if (system(cmd) != 0)
  {
    LOGE("Cannot Set the bridge interface state to UP");
    return RADISH_ERROR;
  }

#ifndef FEATURE_DATA_LINUX_LE
  /*ebtable filters to drop IPV4 packets on the bridge*/
  snprintf(cmd,
           MAX_CMD_LEN,
           "ebtables -t broute -A BROUTING -p ipv4 -j DROP");

  LOGD("%s: %s", __func__, cmd);

  if (system(cmd) != 0)
  {
    LOGE("Cannot set ebtables to drop IPV4");
    return RADISH_ERROR;
  }

  /*ebtable filters to drop arp packets on the bridge*/
  snprintf(cmd,
           MAX_CMD_LEN,
           "ebtables -t broute -A BROUTING -p arp -j DROP");

  LOGD("%s: %s", __func__, cmd);

  if (system(cmd) != 0)
  {
    LOGE("Cannot set ebtables to drop arp");
    return RADISH_ERROR;
  }
#endif

/* Add link local routes in local network table to ensure that link local packets such as dhpcv6
 * or solicited RA's are transmitted. Multicast packets are taken care by ff00::/8 routes
 * in local table */
#if defined FEATURE_DATA_LINUX_ANDROID
  radish_modify_link_local_route(RADISH_ADD_OPERATION);
#endif /* FEATURE_DATA_LINUX_ANDROID */

  return RADISH_SUCCESS;
}

static void
radish_setup_bridge
(
   ServerCfg *server_cfg
)
{
  const char *cur_iface = NULL;
  IfaceCfg *cur_icfg = NULL;
  unsigned iface_idx;

  if (RADISH_ERROR == radish_init_bridge())
  {
    LOGE("There was a problem setting up the bridge network interface, "
           "Radish cannot proceed");
    exit(-1);
  }

  cur_iface = "bridge0";

  if (cur_iface == NULL)
  {
    LOGE("Need an argument for option 'interface'");
    exit(1);
  }

  iface_idx = if_nametoindex(cur_iface);
  if (!iface_idx)
  {
    LOGE("Couldn't find interface %s", cur_iface);
    exit(1);
  }

  if (iface_cfg_new(cur_iface, &cur_icfg))
  {
    LOGE("Unable to create new interface config for %s",cur_iface);
    exit(1);
  }

  if (cur_icfg == NULL)
  {
    LOGE("Need to specify an interface to proxy");
    exit(1);
  }

  cur_icfg->iface_idx = iface_idx;
  cur_icfg->owner = server_cfg;
  rlist_append(&server_cfg->iface_list, &cur_icfg->the_list);

  cur_icfg->iface_params.proxy = TRUE;
}


static int radish_create_forward_dropall
(
  char *forward_chain,
  int  add
)
{
  char cmd[MAX_CMD_LEN];
  if (!forward_chain ||
      ((add != RADISH_ADD_COUNTER_FORWARD_DROPALL) &&
      (add != RADISH_DEL_COUNTER_FORWARD_DROPALL))) {
    LOGE("%s(): Input parameters are invalid", __func__);
    return RADISH_ERROR;
  }
  memset(cmd, 0, MAX_CMD_LEN);
  snprintf(cmd,
           MAX_CMD_LEN,
           "ip6tables %s %s -j DROP",
           add == RADISH_ADD_COUNTER_FORWARD_RULE ? "-A":"-D",
           forward_chain);
  LOGD("%s: %s", __func__, cmd);

  if ((system(cmd) != RADISH_SUCCESS) && (add == RADISH_ADD_COUNTER_FORWARD_DROPALL))
  {
    LOGD("Error creating drop for forward chain %s", forward_chain);
    return RADISH_ERROR;
  }
  return RADISH_SUCCESS;
}

static int radish_create_forward_rules
(
  char *forward_chain,
  char *tether_counters_chain,
  char *intIface,
  char *extIface,
  int  add
)
{
  char cmd[3][MAX_CMD_LEN];
  if (!forward_chain || !tether_counters_chain || !intIface || !extIface ||
      ((add != RADISH_ADD_COUNTER_FORWARD_RULE) &&
      (add != RADISH_DEL_COUNTER_FORWARD_RULE))) {
    LOGE("%s(): Input parameters are invalid", __func__);
    return RADISH_ERROR;
  }

  memset(cmd[0], 0, MAX_CMD_LEN);
  snprintf(cmd[0],
           MAX_CMD_LEN,
           "ip6tables %s %s -i %s -o %s -m state --state ESTABLISHED,RELATED,NEW -g %s",
           add == RADISH_ADD_COUNTER_FORWARD_RULE ? "-A":"-D",
           forward_chain,
           extIface,
           intIface,
           tether_counters_chain);
  LOGD("%s: %s", __func__, cmd[0]);

  if ((system(cmd[0]) != RADISH_SUCCESS) && (add == RADISH_ADD_COUNTER_FORWARD_RULE))
  {
    LOGD("Error creating forward rule for interface pair %s-%s", extIface, intIface);
    return RADISH_ERROR;
  }

  memset(cmd[1], 0, MAX_CMD_LEN);
  snprintf(cmd[1],
           MAX_CMD_LEN,
           "ip6tables %s %s -i %s -o %s -m state --state INVALID -j DROP",
           add == RADISH_ADD_COUNTER_FORWARD_RULE ? "-A":"-D",
           forward_chain,
           intIface,
           extIface);
  LOGD("%s: %s", __func__, cmd[1]);

  if ((system(cmd[1]) != RADISH_SUCCESS) && (add == RADISH_ADD_COUNTER_FORWARD_RULE))
  {
    LOGD("Error creating drop rule for interface pair %s-%s", extIface, intIface);
    goto error_invalid_drop_ei;
  }

  memset(cmd[2], 0, MAX_CMD_LEN);
  snprintf(cmd[2],
           MAX_CMD_LEN,
           "ip6tables %s %s -i %s -o %s -g %s",
           add == RADISH_ADD_COUNTER_FORWARD_RULE ? "-A":"-D",
           forward_chain,
           intIface,
           extIface,
           tether_counters_chain);
  LOGD("%s: %s", __func__, cmd[2]);

  if ((system(cmd[2]) != RADISH_SUCCESS) && (add == RADISH_ADD_COUNTER_FORWARD_RULE))
  {
    LOGD("Error creating forward rule for interface pair %s-%s", intIface, extIface);
    goto error_invalid_forward_ie;
  }

  return RADISH_SUCCESS;

error_invalid_forward_ie:
  cmd[1][11] = 'D';
  system(cmd[1]);
  LOGD("%s: %s", __func__, cmd[1]);
error_invalid_drop_ei:
  cmd[0][11] = 'D';
  system(cmd[0]);
  LOGD("%s: %s", __func__, cmd[2]);
  return RADISH_ERROR;
}

static int radish_create_counter_rules
(
  char *tether_counters_chain,
  char *intIface,
  char *extIface
)
{
  char cmd[MAX_CMD_LEN];
  char quota_name[MAX_CMD_LEN];

  if (!tether_counters_chain || !intIface || !extIface) {
    LOGE("%s(): Input parameters are invalid", __func__);
    return RADISH_ERROR;
  }

  memset(cmd, 0, MAX_CMD_LEN);
  memset(quota_name, 0, MAX_CMD_LEN);

  snprintf(quota_name, MAX_CMD_LEN, "%s_%s", intIface, extIface);

  memset(cmd, 0, MAX_CMD_LEN);
  snprintf(cmd,
           MAX_CMD_LEN,
           "/proc/net/xt_quota/%s",
           quota_name);
  LOGD("%s: Checking quota %s", __func__, cmd);

  if (open(cmd, O_RDONLY) >= 0)
  {
    LOGD("Quota for interface pair %s-%s already exists", intIface, extIface);
    return RADISH_SUCCESS;
  }

  memset(cmd, 0, MAX_CMD_LEN);
  snprintf(cmd,
           MAX_CMD_LEN,
           "ip6tables -A %s -i %s -o %s -m quota2 --name %s --grow -j RETURN",
           tether_counters_chain,
           intIface,
           extIface,
           quota_name);
  LOGD("%s: %s", __func__, cmd);

  if (system(cmd) != RADISH_SUCCESS)
  {
    LOGD("Error creating counter rule for interface pair %s-%s", intIface, extIface);
    return RADISH_ERROR;
  }

  return RADISH_SUCCESS;
}

static int radish_delete_tether_counter_chain(char *tether_counters_chain){
  char cmd[MAX_CMD_LEN];

  if (!tether_counters_chain ) {
    LOGE("%s(): Input parameters are invalid", __func__);
    return RADISH_ERROR;
  }

  memset(cmd, 0, MAX_CMD_LEN);

  snprintf(cmd,
           MAX_CMD_LEN,
           "ip6tables -F %s",
           tether_counters_chain);
  LOGD("%s: %s", __func__, cmd);

  if (system(cmd) != RADISH_SUCCESS)
  {
    LOGD("Tether counter chain %s deletion failed/not exsist ", tether_counters_chain);
    return RADISH_ERROR;
  }

  memset(cmd, 0, MAX_CMD_LEN);
  snprintf(cmd,
           MAX_CMD_LEN,
           "ip6tables -X %s",
           tether_counters_chain);
  LOGD("%s: %s", __func__, cmd);

  if (system(cmd) != RADISH_SUCCESS)
  {
    LOGD("Tether counter chain %s deletion failed/not exsist ", tether_counters_chain);
    return RADISH_ERROR;
  }
    return RADISH_SUCCESS;
}


static int radish_delete_counter_rules
(
  char *tether_counters_chain,
  char *intIface,
  char *extIface
)
{
  char cmd[MAX_CMD_LEN];
  char quota_name[MAX_CMD_LEN];

  if (!tether_counters_chain || !intIface || !extIface) {
    LOGE("%s(): Input parameters are invalid", __func__);
    return RADISH_ERROR;
  }

  memset(cmd, 0, MAX_CMD_LEN);
  memset(quota_name, 0, MAX_CMD_LEN);

  snprintf(quota_name, MAX_CMD_LEN, "%s_%s", intIface, extIface);

  memset(cmd, 0, MAX_CMD_LEN);
  snprintf(cmd,
           MAX_CMD_LEN,
           "ip6tables -D %s -i %s -o %s -m quota2 --name %s --grow -j RETURN",
           tether_counters_chain,
           intIface,
           extIface,
           quota_name);
  LOGD("%s: %s", __func__, cmd);

  if (system(cmd) != RADISH_SUCCESS)
  {
    LOGD("Error creating counter rule for interface pair %s-%s", intIface, extIface);
    return RADISH_ERROR;
  }

  return RADISH_SUCCESS;
}


static int radish_setup_ipv6_stat_counters
(
  char *iface,
  char *tether_counters_chain,
  char *forward_chain
)
{
  char cmd[MAX_CMD_LEN];

  if (!iface || !tether_counters_chain || !forward_chain) {
    LOGE("%s(): Input parameters are invalid", __func__);
    return RADISH_ERROR;
  }

  memset(cmd, 0, MAX_CMD_LEN);

  snprintf(cmd,
           MAX_CMD_LEN,
           "ip6tables -N %s",
           tether_counters_chain);
  LOGD("%s: %s", __func__, cmd);

  if (system(cmd) != RADISH_SUCCESS)
  {
    LOGD("Tether counter chain %s already exists", tether_counters_chain);
  }

  if ((!radish_create_counter_rules(tether_counters_chain,
                                    RADISH_BRIDGE_INTERFACE_NAME,
                                    iface)) &&
      (!radish_create_counter_rules(tether_counters_chain,
                                    iface,
                                    RADISH_BRIDGE_INTERFACE_NAME))) {
    if (!radish_create_forward_rules(forward_chain,
                                     tether_counters_chain,
                                     RADISH_BRIDGE_INTERFACE_NAME,
                                     iface,
                                     RADISH_ADD_COUNTER_FORWARD_RULE)) {
       radish_create_forward_dropall(forward_chain,
                                     RADISH_DEL_COUNTER_FORWARD_DROPALL);
       if (!radish_create_forward_dropall(forward_chain,
                                          RADISH_ADD_COUNTER_FORWARD_DROPALL)) {
         LOGD("Stats will be recorded on %s-%s interface", iface, RADISH_BRIDGE_INTERFACE_NAME);
         return RADISH_SUCCESS;
       }
    }
  }

  return RADISH_ERROR;
}

static void radish_parse_args
(
  ServerCfg *server_cfg,
  int argc,
  char *argv[]
)
{
  const char *cur_iface = NULL;
  IfaceCfg *cur_icfg = NULL;
  char cur_prefix[MAX_IPV6_PREFIX + 1];
  int require_timer = 0;
  int opt;
  List *cur_list;
  int i;
  uint8_t is_bridge_established = FALSE;

  RADISH_LOG_FUNC_ENTRY;

  while ( (opt = getopt_long(argc, argv, "t:k:b:i:p:x", radish_opts, NULL)) != -1)
  {
    switch (opt)
    {
      case 'b': server_cfg->backhaul = optarg; break;

      case 'k': server_cfg->is_bridge_mode_on = FALSE; break;

/* This option allows clients of radish to pass the table name for the upstream
 * interface which is to be used as part of the policy based routing needed when
 * tethering is enabled using a non default apn. Radish adds the rule which looks
 * up this table and routes the ipv6 traffic based on this table.*/
      case 't': {
        server_cfg->policy_route_table_name = strdup(optarg);
        if (!server_cfg->policy_route_table_name)
        {
          LOGE("Error duplicating string - %s, not using policy based route.", strerror(errno));
          break;
        }
        server_cfg->use_policy_based_route = TRUE;
        LOGD("Using policy based route, table name is %s", server_cfg->policy_route_table_name);
        if (server_cfg->use_policy_based_route)
        {
          radish_modify_policy_based_route(server_cfg->policy_route_table_name,
                                           RADISH_ADD_OPERATION);
        }
        break;

      }

      case 'i': {
          unsigned iface_idx;
          cur_iface = optarg;
          char cmd[MAX_CMD_LEN + 1];

          if (cur_iface == NULL)
          {
            LOGE("Need an argument for option 'interface'");
            exit(1);
          }

          if (strstr(cur_iface, server_cfg->backhaul) ||
                (!server_cfg->is_bridge_mode_on))
          {
            iface_idx = if_nametoindex(cur_iface);
            if (!iface_idx)
            {
              LOGE("Couldn't find interface %s", cur_iface);
              exit(1);
            }

            if (iface_cfg_new(cur_iface, &cur_icfg))
            {
              LOGE("Unable to create new interface config for %s",cur_iface);
              exit(1);
            }

            cur_icfg->iface_idx = iface_idx;
            cur_icfg->owner = server_cfg;
            rlist_append(&server_cfg->iface_list, &cur_icfg->the_list);

            snprintf(cmd,
                     MAX_CMD_LEN,
                     "echo 0 > /proc/sys/net/ipv6/conf/%s/accept_ra_prefix_route",
                     cur_iface);
            memset(g_cur_iface,'0',sizeof(g_cur_iface));
            memcpy(g_cur_iface,cur_iface,strlen(cur_iface)+1);

            LOGD("%s: %s", __func__, cmd);

            if(radish_setup_ipv6_stat_counters((char *)cur_iface,
                                               RADISH_TETHER_COUNTERS_CHAIN,
                                               RADISH_FORWARD_CHAIN) != RADISH_SUCCESS){
              LOGE("Unable to setup ipv6 stat counters");
            }

            if (system(cmd) != 0)
            {
              LOGE("Disable IPV6 prefix route addition on upstream interface");
            }

          }
          else if (server_cfg->is_bridge_mode_on)
          {
            if (!is_bridge_established)
            {
               radish_setup_bridge(server_cfg);
               is_bridge_established = TRUE;
            }
            /*Add the interface to the bridge*/
            LOGD("Adding %s to bridge0",cur_iface);

            snprintf(cmd,
                     MAX_CMD_LEN,
                     "brctl addif bridge0 %s",
                     cur_iface);

            LOGD("%s: %s", __func__, cmd);

            if (system(cmd) != 0)
            {
              LOGE("Unable to add the interface to the bridge");
            }
          }

          break;
        }

      case 'p': {
          char *slash;
          long prefixlen = 128;

          if (cur_icfg == NULL)
          {
            LOGE("Need to specify an interface before prefix");
            exit(1);
          }

          slash = strchr(optarg, '/');

          if (slash)
          {
            prefixlen = strtol(slash+1, NULL, 10);
          }

          if ( (slash && ((slash - optarg) >= MAX_IPV6_PREFIX
                          || slash == optarg
                          || (strlen(slash+1) < 1)))
               || prefixlen > 128 )
          {
            LOGE("Invalid prefix: %s", optarg);
            exit(1);
          }

          if (slash == 0)
          {
            slash = optarg + strlen(optarg);
          }

          memcpy(cur_prefix, optarg, (size_t)(slash - optarg));

          cur_prefix[slash - optarg] = '\0';

          if (rlist_is_empty(&cur_icfg->adv_prefix_list))
          {
            require_timer ++;
          }

          if (iface_cfg_add_prefix(cur_icfg, cur_prefix, (uint8_t)prefixlen, 0, 0))
          {
            LOGE("Unable to create prefix configuration for %s [%s]", cur_iface, optarg);
            exit(1);
          }
          break;
        }

      case 'x': {
          if (cur_icfg == NULL)
          {
            LOGE(" FATAL: Need to specify an interface to proxy");

            //if (strstr(cur_iface, "rmnet"))
            //    exit(1);
            break;
          }

          cur_icfg->iface_params.proxy = TRUE;
          break;
        }
    }

  }

  server_cfg->enable_adv_timer = !!require_timer;

  if (!rlist_is_empty(&server_cfg->iface_list))
  {
    LOGD("Interfaces found: \n");
  }

  i = 0;
  rlist_foreach(cur_list, &server_cfg->iface_list) {
    i++;
    List *prefix_list;
    cur_icfg = IFACECFG(cur_list);
    if (!cur_icfg)
    {
      LOGE("IFACECFG returned NULL");
      exit(1);
    }
    LOGD("%s\n", cur_icfg->iface_name);
    rlist_foreach(prefix_list, &cur_icfg->adv_prefix_list) {
      PrefixItem *cur_pitem = PREFIX(prefix_list);
      if (!cur_pitem)
      {
        LOGE("PREFIX returned NULL");
        exit(1);
      }
      inet_ntop(AF_INET6, &cur_pitem->addr, cur_prefix, MAX_IPV6_PREFIX);
      LOGD("    [%s/%d]\n",cur_prefix,(int)cur_pitem->prefix_length);
    }
  }
  if (i < 2)
  {
    LOGE("Need at least 2 interfaces");
    exit(0);
  }

  RADISH_LOG_FUNC_EXIT;
}

static void radish_wakeup
(
  int ws
)
{
  write(ws, " ", 1);
}

static int cmp_time
(
  struct timespec *t1,
  struct timespec *t2
)
{
  int ret = 0;
  ret = (int)(t1->tv_sec - t2->tv_sec);

  if (!ret) ret = (int)(t1->tv_nsec - t2->tv_nsec);

  return ret;
}

void radish_schedule_timer
(
  struct radish_timer *timer,
  struct timespec *ts,
  radish_timer_cb callback,
  void *user
)
{
  List *cur;
  struct radish_timer_evt *evt = NULL, *o;
  struct itimerspec its;

  RADISH_LOG_FUNC_ENTRY;

  if (user && (!ts || !callback))
  {
    LOGE("%s: provided user without ts or callback", __func__);
    RADISH_LOG_FUNC_EXIT;
    return;
  }
  pthread_mutex_lock(&timer->mutex);

  if (user)
  {
    evt = calloc(1, sizeof(struct radish_timer_evt));
    if (!evt)
    {
      LOGE("memory error: calloc failed for evt");
      pthread_mutex_unlock(&timer->mutex);
      RADISH_LOG_FUNC_EXIT;
      return;
    }
    evt->userdata = user;
    evt->callback = callback;
    evt->time = *ts;
    rlist_foreach(cur, &timer->next_events) {
      o = RADISH_TIMER_EVT(cur);
      if (cmp_time(&evt->time, &o->time) < 0)
      {
        break;
      }
    }
    if (!cur)
      cur = &timer->next_events;
    rlist_insert(cur, &evt->the_list);
    if (rlist_first(&timer->next_events) != &evt->the_list)
    {
      evt = NULL;
    }
  }
  else
  {
    if (__is_timer_expired(timer))
    {
      evt = RADISH_TIMER_EVT(rlist_first(&timer->next_events));
    }
  }

  if (evt)
  {
    its.it_interval.tv_sec = its.it_interval.tv_nsec = 0;
    its.it_value = evt->time;

    LOGD("%s: seconds: %d nsec: %d", __func__,
         (int) evt->time.tv_sec,
         (int)evt->time.tv_nsec);

    timer_settime(timer->timer, TIMER_ABSTIME, &its, NULL);
  }

  pthread_mutex_unlock(&timer->mutex);
  RADISH_LOG_FUNC_EXIT;
  return;
}

static void radish_timer_expired
(
  union sigval sval
)
{
  struct radish_timer *timer = (struct radish_timer *) sval.sival_ptr;
  struct radish_timer_evt *evt;

  RADISH_LOG_FUNC_ENTRY;

  pthread_mutex_lock(&timer->mutex);
  evt = RADISH_TIMER_EVT(rlist_pop_first(&timer->next_events));
  pthread_mutex_unlock(&timer->mutex);

  if (evt)
  {
    LOGD("%s Timer expired. Event: %p",__func__, (unsigned*) evt);
    evt->callback(evt->userdata);
    free(evt);
    // Schedule next pending event
    radish_schedule_timer(timer, NULL, NULL, NULL);
  }
  else
  {
    LOGD(" %s Timer expired without event", __func__);
  }

  RADISH_LOG_FUNC_EXIT;
}

static int radish_create_socket
(
  int af,
  int type,
  int proto,
  int iface_idx,
  const char *iface_name,
  int proxy
)
{
  struct ipv6_mreq mreq;
  int res = -1;
  int sock;
  struct ifreq ifr;
  /* According to RFC Hop Limit must be set to 255 in order
   * to protect against off-link packets.
   */
  int hop_limit = 255;
  int true = 1;
  int false = 0;

  RADISH_LOG_FUNC_ENTRY;

  /* Create socket and set required options */
  sock = socket(af, type, proto);
  if (sock < 0)
  {
    goto error;
  }

  if (setsockopt(sock, SOL_SOCKET, SO_BINDTODEVICE, iface_name, (socklen_t)strlen(iface_name)) < 0)
  {
    LOGE("%s: Unable to bind socket to interface %s", __func__, iface_name);
  }
  if (setsockopt(sock,
                 IPPROTO_IPV6,
                 IPV6_MULTICAST_IF,
                 &iface_idx,
                 sizeof(iface_idx)) < 0) goto error;

  memset(&mreq, 0, sizeof(mreq));
  mreq.ipv6mr_multiaddr = all_routers_in6_addr;
  mreq.ipv6mr_interface = iface_idx;

  if (setsockopt(sock,
                 IPPROTO_IPV6,
                 IPV6_ADD_MEMBERSHIP,
                 &mreq,
                 sizeof(mreq)) < 0) goto error;

  if (proxy)
  {
    memset(&mreq, 0, sizeof(mreq));
    mreq.ipv6mr_multiaddr = all_hosts_in6_addr;
    mreq.ipv6mr_interface = iface_idx;
    if (setsockopt(sock,
                   IPPROTO_IPV6,
                   IPV6_ADD_MEMBERSHIP,
                   &mreq,
                   sizeof(mreq)) < 0) goto error;

    memset(&mreq, 0, sizeof(mreq));
    mreq.ipv6mr_multiaddr = all_mld2_routers_in6_addr;
    mreq.ipv6mr_interface = iface_idx;
    if (setsockopt(sock,
                   IPPROTO_IPV6,
                   IPV6_ADD_MEMBERSHIP,
                   &mreq,
                   sizeof(mreq)) < 0) goto error;
  }

  if (setsockopt(sock,
                 IPPROTO_IPV6,
                 IPV6_MULTICAST_HOPS,
                 &hop_limit,
                 sizeof(hop_limit)) < 0) goto error;

  if (setsockopt(sock,
                 IPPROTO_IPV6,
                 IPV6_UNICAST_HOPS,
                 &hop_limit,
                 sizeof(hop_limit)) < 0) goto error;

  if (setsockopt(sock,
                 IPPROTO_IPV6,
                 IPV6_MULTICAST_LOOP,
                 &false,
                 sizeof(false)) < 0) goto error;

  if ( (res = setsockopt(sock,
                         IPPROTO_IPV6,
                         IPV6_RECVPKTINFO,
                         &true,
                         sizeof(true))) < 0)
  {
    LOGD("Error %d setting IPV6_PKTINFO option", res);
    goto error;
  }

  strlcpy(ifr.ifr_name, iface_name, IFNAMSIZ);

  if (ioctl(sock, SIOCGIFFLAGS, &ifr) < 0)
  {
    goto error;
  }

  ifr.ifr_flags |= IFF_PROMISC;

  if (ioctl(sock, SIOCSIFFLAGS, &ifr) < 0)
  {
    LOGD("%s (%s) Setting PROMISC flag failed. Trying ALLMULTI", __func__, iface_name);
    ifr.ifr_flags & ~IFF_PROMISC;
  }
  else
  {
    LOGD("%s (%s) Set PROMISC mode", __func__, iface_name);
  }

  ifr.ifr_flags |= IFF_ALLMULTI;
  if (ioctl(sock, SIOCSIFFLAGS, &ifr) < 0)
  {
    LOGD("%s (%s) Setting ALLMULTI failed", __func__, iface_name);
  }
  else
  {
    LOGD("%s (%s) Set ALLMULTI mode", __func__, iface_name);
  }

  RADISH_LOG_FUNC_EXIT;
  return sock;

error:
  if (sock >=0)
  {
    close(sock);
  }
  RADISH_LOG_FUNC_EXIT;
  return -1;
}

static void radish_timer_init
(
  struct radish_timer *timer
)
{
  struct sigevent sev;

  memset(timer, 0, sizeof(*timer));
  pthread_mutex_init(&timer->mutex, NULL);

  sev.sigev_notify = SIGEV_THREAD;
  sev.sigev_notify_function = radish_timer_expired;
  sev.sigev_value.sival_ptr = timer;
  sev.sigev_notify_attributes = NULL;

  if (timer_create(CLOCK_MONOTONIC, &sev, &timer->timer) < 0)
  {
    LOGE("%s: Error initializing timer", __func__);
  }
  rlist_init(&timer->next_events);
}

#define ERRBUFSIZE 255
static int radish_set_address
(
  int sfd,
  struct in6_addr *addr,
  uint32_t ifindex
)
{
  struct sockaddr_in6 saddr;
  struct sockaddr_in6 * __attribute__((__may_alias__)) saddr_ptr;

  if (!addr || sfd < 0)
  {
    LOGE("%s: Invalid socket or address", __func__);
    return -1;
  }

  memset(&saddr, 0, sizeof(saddr));
  saddr.sin6_family = AF_INET6;
  saddr.sin6_scope_id = ifindex;
  saddr.sin6_addr = *addr;
  saddr_ptr = &saddr;

  if (bind(sfd, (struct sockaddr*)saddr_ptr, sizeof(saddr)) < 0)
  {
    LOGE("%s: (iface %d): Unable to bind to address", __func__, ifindex);
    return -1;
  }

  LOGD("%s (iface %d): Address set", __func__, ifindex);

  return 0;
}

static void radish_mldv1_expired
(
  void *user
)
{
  struct mldv1_evt *mevt = (struct mldv1_evt *)user;
  struct buffer_item *itm;
  IfaceCfg *cfg;
  struct r_ip6_hdr *ip6h;
  struct icmp6_header *icmp6h;
  struct sockaddr_in6 sdaddr;

  RADISH_LOG_FUNC_ENTRY;

  if (!mevt)
  {
    RADISH_LOG_FUNC_EXIT;
    return;
  }

  memset(&sdaddr, 0, sizeof(sdaddr));
  itm = mevt->itm;
  cfg = mevt->cfg;
  ip6h = mevt->ip6h;
  icmp6h = mevt->icmp6h;

  if (IN6_IS_ADDR_UNSPECIFIED_32(&cfg->ip6_addr))
  {
    radish_find_ip6_addr(cfg, &cfg->ip6_addr);
    if (IN6_IS_ADDR_UNSPECIFIED_32(&cfg->ip6_addr))
    {
      radish_schedule_mldv1_lq(cfg, mevt);
      RADISH_LOG_FUNC_EXIT;
      return;
    }
    radish_set_address(cfg->adv_socket, &cfg->ip6_addr, cfg->iface_idx);
    ip6h->ip6_src = cfg->ip6_addr;
    icmp6h->checksum = 0;
    icmp6h->checksum = radish_icmp6_checksum(ip6h, icmp6h, ntohs(ip6h->ip6_plen));
  }

  sdaddr.sin6_family = AF_INET6;
  sdaddr.sin6_addr = ip6h->ip6_dst;
  sdaddr.sin6_scope_id = cfg->iface_idx;
  radish_send_packet(cfg->adv_socket, itm->iov, mevt->iovlen, &sdaddr);
  radish_schedule_mldv1_lq(cfg, mevt);
  RADISH_LOG_FUNC_EXIT;
}

static void radish_schedule_mldv1_lq
(
  IfaceCfg *cfg,
  struct mldv1_evt *mevt
)
{
  struct buffer_item *itm;
  struct mldp1 *mldp;
  struct r_ip6_hdr *ip6hdr;
  uint8_t *end;
  struct in6_addr src;
  struct in6_addr dst;
  struct timespec ts;
  size_t pktlen = 0;
  uint16_t paylen = 0;

  RADISH_LOG_FUNC_ENTRY;
  if (!mevt)
  {
    itm = radish_fetch_available_buffer(__func__);

    if (!itm) goto error;

    mevt = calloc(1, sizeof(struct mldv1_evt));
    if (!mevt) goto error;
    ip6hdr = (struct r_ip6_hdr *) itm->iov[0].iov_base;

    mldp = (struct mldp1 *) (((uint8_t *) ip6hdr) + sizeof(struct r_ip6_hdr));
    end = ((uint8_t *) mldp) + sizeof(struct mldp1);
    paylen = (uint16_t)(end - (uint8_t *)mldp);

    radish_find_ip6_addr(cfg, &src);
    dst = all_hosts_in6_addr;
    pktlen = (size_t) (end - (uint8_t *)ip6hdr);
    memset(ip6hdr, 0, pktlen);
    itm->iov[0].iov_len = pktlen;
    r_ip6_init(ip6hdr, &src, &dst, IPPROTO_ICMPV6, paylen);
    mldp->icmp6hdr.type = ICMPV6_TYPE_MLD1_LQ;
    mldp->icmp6hdr.code = 0;
    mldp->icmp6hdr.checksum = 0;
    mldp->max_response_delay = (uint16_t)htons(1000);

    mldp->icmp6hdr.checksum = radish_icmp6_checksum(ip6hdr,
                                                    &mldp->icmp6hdr,
                                                    (int)(end - (uint8_t *)&mldp->icmp6hdr));
    mevt->ip6h = ip6hdr;
    mevt->icmp6h = &mldp->icmp6hdr;
    mevt->itm = itm;
    mevt->iovlen = 1;
    mevt->cfg = cfg;
  }

  clock_gettime(CLOCK_MONOTONIC, &ts);
  ts.tv_sec += 1;

  radish_schedule_timer(&global_timer, &ts, radish_mldv1_expired, mevt);

  RADISH_LOG_FUNC_EXIT;
  return;
  error:
  RADISH_LOG_FUNC_EXIT;
  radish_release_buffer(itm, __func__);
}

/*
 * Description:
 * Will create packet socket bound to the given interface with filter
 * attached to only receive icmp6 traffic
 *
 * Parameters:
 * iface_idx:  interface where socket will be bound to
 * iface_name: name of the interface
 *
 * Return:
 * valid sock fd on success, -1 otherwise
 */
static int radish_create_icmp6_packet_socket
(
  int iface_idx,
  const char * iface_name
)
{
  int sock=-1;
  int ret=-1;
  struct sockaddr_ll sa;
  struct sockaddr_ll* __attribute__((__may_alias__)) sa_ptr;

  struct sock_filter filter[] =
  {
    /* originally derived from
     * `tcpdump -d`, needs to be
     * adjusted depending on where
     * the message is captured
     * opcode, jt, jf, value
     * jt = jump jt number of rules if true
     * jf = jump jf number of rules if false
     * opcode 0x30 = BPF_LD | BPF_B | BPF_ABS
     * opcode 0x15 = BPF_JMP | BPF_JEQ | BPF_K */
    /* offsets are adjusted assuming
     * message is captured at IPv6
     * header */
    /* goto first byte and
       compare version 6 */
    { 0x30, 0, 0, 0x00000000 },
    { 0x15, 0, 9, 0x00000060},
    /* go further 6 bytes ahead and
       compare ICMPv6 type 0x3a */
    { 0x30, 0, 0, 0x00000006},
    { 0x15, 6, 0, 0x0000003a},
    /* compare UDP type 0x11 */
    { 0x15, 0, 6, 0x00000011},
    /* go 42 bytes ahead and
       compare DST port 546 or 547 (word) */
    { 0x30, 0, 0, 0x0000002a},
    { 0x15, 0, 4, 0x00000002},
    { 0x30, 0, 0, 0x0000002b},
    /* 546 = 0x222 */
    { 0x15, 1, 0, 0x00000022}, /* dst 546 - dhcp6 resp */
    /* 547 = 0x223 */
    { 0x15, 0, 1, 0x00000023}, /* dst 547 - dhcp6 req */
    /* return */
    { 0x6, 0, 0, 0x00000500}, /* PASS - return upto 1280 bytes */
    { 0x6, 0, 0, 0x00000000}  /* FAIL - return no bytes */
  };
  struct sock_fprog fprog =
  {
    .len = sizeof(filter)/sizeof(struct sock_filter),
           .filter = filter
  };

  RADISH_LOG_FUNC_ENTRY;

  if (!iface_name)
  {
    LOGE("bad parameters received");
    RADISH_LOG_FUNC_EXIT;
    return ret;
  }

  LOGD("creating packet socket for iface %s", iface_name);

  do
  {
    sock = socket(AF_PACKET, SOCK_DGRAM, htons(ETH_P_IPV6));
    if (sock < 0)
    {
      LOGE("socket creation failed. sock=%d, errno=%d", sock, errno);
      break;
    }
    LOGD("socket %d successfully created", sock);

    /* bind this socket to specific iface */
    sa.sll_family = AF_PACKET;
    sa.sll_protocol = (uint16_t)htons(ETH_P_IPV6);
    sa.sll_ifindex = iface_idx;
    sa_ptr = &sa;

    if (bind(sock, (struct sockaddr*)sa_ptr, sizeof(sa)) != 0)
    {
      LOGE("couldn't bind socket %d to iface %d, errno=%d",
           sock, iface_idx, errno);
      break;
    }

    /* install filter to only receive ICMPv6 traffic */
    if (setsockopt(sock,
                   SOL_SOCKET,
                   SO_ATTACH_FILTER,
                   &fprog,
                   sizeof(fprog)) == -1)
    {
      LOGE("couldn't attach BPF filter on sock %d, errno=%d", sock, errno);
      break;
    }

    ret = sock;
  }while (0);

  if (-1 == ret)
  {
    if (sock != -1)
    {
      LOGD("closing socket %d", sock);
      close(sock);
    }
  }

  RADISH_LOG_FUNC_EXIT;
  return ret;
}

static int radish_setup_listener
(
  IfaceCfg *cfg
)
{
  int i,j;
  struct buffer_item *itm;
  List tmp_list;
  List *cur_list;

  RADISH_LOG_FUNC_ENTRY;

  if (!cfg)
  {
    LOGE("bad parameters received");
    RADISH_LOG_FUNC_EXIT;
    return -1;
  }

  LOGD("setting listener for iface %s", cfg->iface_name);
  cfg->ip6_addr = in6addr_any;

  /* Setup write socket with IPPROTO_RAW */
  cfg->adv_socket = radish_create_socket(AF_INET6, SOCK_RAW,
                                         IPPROTO_RAW,
                                         (int)cfg->iface_idx,
                                         cfg->iface_name,
                                         cfg->iface_params.proxy);

  if (cfg->adv_socket < 0)
  {
    LOGE("%s: Unable to setup raw socket for %s", __func__, cfg->iface_name);
    goto error;
  }
  else
    LOGD("Created adv_socket (RAW) %d for iface %s", cfg->adv_socket, cfg->iface_name);

  /* IPPROTO_RAW sockets are write-only. Use this to receive packets */
  cfg->rcv_socket = radish_create_icmp6_packet_socket((int)cfg->iface_idx,
                                                      cfg->iface_name);
  if (cfg->rcv_socket < 0)
  {
    LOGE("%s: Unable to setup recv socket for %s", __func__, cfg->iface_name);
    goto error;
  }

  rlist_init(&tmp_list);
  for (i = 0; i < IOV_PER_THREAD; i++)
  {
    itm = calloc(1, sizeof(struct buffer_item));
    if (!itm)
    {
      goto error;
    }

    LOGD("%s (%s): Buffer at %p", __func__,
         cfg->iface_name, (unsigned*) itm);

    for (j = 0; j < IOV_LEN; j++)
    {
      itm->iov[j].iov_base = calloc(1, IOV_BUFF_LEN);

      if (!itm->iov[j].iov_base)
      {
        goto error;
      }

      LOGD("%s (%s): -> [%d,%d]: %p", __func__,
           cfg->iface_name, i, j,
           (unsigned*) itm->iov[j].iov_base);
      itm->iov[j].iov_len = IOV_BUFF_LEN;
    }
    rlist_append(&tmp_list, &itm->the_list);
  }

  pthread_mutex_lock(&available_lock);

  rlist_move(&available_buffers, &tmp_list);

  i = 0;

  rlist_foreach(cur_list, &available_buffers) {

    itm = BUFF_ITM(cur_list);
    if (!itm)
    {
      LOGE("BUFF_ITM returned NULL");
      goto error;
    }

    i ++;
  }
  pthread_mutex_unlock(&available_lock);

  //radish_schedule_mldv1_lq(cfg, NULL);

  LOGD("< %s(%s)", __func__, cfg->iface_name);
  RADISH_LOG_FUNC_EXIT;
  return 0;


error:
  LOGE("%s: error setting up listener", __func__);
  if (cfg->adv_socket >= 0)
  {
    close(cfg->adv_socket);
    cfg->adv_socket = -1;
  }
  if (cfg->rcv_socket >= 0)
  {
    close(cfg->rcv_socket);
    cfg->rcv_socket = -1;
  }
  for (cur_list = rlist_pop_first(&tmp_list);
      cur_list;
      cur_list = rlist_pop_first(&tmp_list))
  {
    itm = BUFF_ITM(cur_list);
    if (!itm)
      continue;
    for (i = 0; i < IOV_LEN; i++)
    {
      if (itm->iov[i].iov_base)
      {
        free(itm->iov[i].iov_base);
        itm->iov[i].iov_base = NULL;
        itm->iov[i].iov_len = 0;
      }
    }
  }
  RADISH_LOG_FUNC_EXIT;
  return -1;
}

void radish_prepare_fds
(
  IfaceCfg *cfg
)
{
  RADISH_LOG_FUNC_ENTRY;

  if (!cfg)
  {
    LOGE("bad parameters received");
    RADISH_LOG_FUNC_EXIT;
    return;
  }

  RAD_LOGI("preparing fds on iface [%s]",
           cfg->iface_name);
  FD_ZERO(&cfg->rfds);
  FD_ZERO(&cfg->wfds);
  FD_ZERO(&cfg->efds);
  cfg->nfds = 0;

  FD_SET(cfg->rcv_socket, &cfg->rfds);
  if (cfg->rcv_socket > cfg->nfds)
  {
    cfg->nfds = cfg->rcv_socket;
  }
  FD_SET(cfg->wake_socket[1], &cfg->rfds);
  if (cfg->wake_socket[1] > cfg->nfds)
  {
    cfg->nfds = cfg->wake_socket[1];
  }
  cfg->nfds ++;

  RADISH_LOG_FUNC_EXIT;
}
static int __is_timer_expired(struct radish_timer *timer)
{
  struct itimerspec ts;
  memset(&ts, 0, sizeof(ts));
  timer_gettime(timer->timer, &ts);
  return ts.it_value.tv_sec == 0 && ts.it_value.tv_nsec == 0;
}
int is_timer_expired(struct radish_timer *timer)
{
  int ret = 0;
  pthread_mutex_lock(&timer->mutex);
  ret = __is_timer_expired(timer);
  pthread_mutex_unlock(&timer->mutex);
  return ret;
}

int radish_gen_rand_time
(
  int min,
  int max
)
{
  int64_t rndval;
  int rndsecs;

  rndval = random();
  rndsecs = (((int)rndval * (max - min) ) / RAND_MAX ) + min;

  return rndsecs;
}

void radish_sched_timer_rand
(
  struct radish_timer *timer,
  int min,
  int max
)
{
  int rndsecs;
  rndsecs = radish_gen_rand_time(min, max);
  if (rndsecs > max || rndsecs < min)
  {
    LOGD("Randomized timer outside parameters: [%d, %d]: %d, %p",
         min, max,
         rndsecs,
         (unsigned*) timer);
  }
}

void radish_wait_for_events
(
  IfaceCfg *cfg
)
{
  RADISH_LOG_FUNC_ENTRY;

  if (!cfg)
  {
    LOGE("bad parameters received");
    RADISH_LOG_FUNC_EXIT;
    return;
  }

  RAD_LOGI("[%s]", cfg->iface_name);

  if (select(cfg->nfds, &cfg->rfds, NULL, NULL, NULL) < 0)
  {
    LOGE("Error in select: %d", errno);
  }

  RADISH_LOG_FUNC_EXIT;
}

void radish_find_hw_addr
(
  IfaceCfg *cfg,
  uint8_t *addr
)
{
  struct ifreq req;
  int res;

  if (!cfg || !addr)
  {
    LOGE("bad parameters received");
    return;
  }

  memset(&req, 0, sizeof(req));
  strlcpy(req.ifr_name, cfg->iface_name, IF_NAMESIZE);
  res = ioctl(cfg->rcv_socket, SIOCGIFHWADDR, &req);
  if (res <0)
  {
    LOGE("Error getting hw address: %d", errno);
    return;
  }

  memcpy(addr, &req.ifr_hwaddr.sa_data, IFHWADDRLEN);
  RAD_LOGI("Found address %2.2x:%2.2x:%2.2x:%2.2x:%2.2x:%2.2x",
           (int)addr[0], (int)addr[1], (int)addr[2],
           (int)addr[3], (int)addr[4], (int)addr[5]);
}

int radish_send_packet
(
  int socket,
  struct iovec *iov,
  int iovlen,
  struct sockaddr_in6 *dst
)
{
  struct msghdr msg;
  int ret=-1;

  if (!iov || !dst)
  {
    LOGE("bad parameters received");
    return ret;
  }

  LOGD("> %s (%d, %p, %d, %p)",
       __func__, socket,
       (unsigned*)iov, iovlen,
       (unsigned*)dst);

  memset(&msg, 0, sizeof(msg));

  msg.msg_name = dst;
  msg.msg_namelen = sizeof(*dst);
  msg.msg_iov = iov;
  msg.msg_iovlen = (size_t)iovlen;
  msg.msg_control = iov[1].iov_base;
  msg.msg_controllen = 0;

  ret = sendmsg(socket, &msg, 0);
  LOGD("< %s (%d)", __func__, errno);
  return ret;
}

/*
 * Description:
 * send the packet over the socket using sendmsg
 *
 * Parameters:
 * ndp: pointer to parsed_nd that contains packet
 * socket: socket fd
 *
 * Return:
 * -1 if failed, otherwise return value from sendmsg
 */
int radish_send_nd_packet
(
  int socket,
  struct parsed_nd *ndp
)
{
  struct msghdr msg;
  int res;
  char saddr[MAX_IPV6_PREFIX + 1];
  char daddr[MAX_IPV6_PREFIX + 1];

  RADISH_LOG_FUNC_ENTRY;

  if (!ndp)
  {
    LOGE("bad parameters received");
    RADISH_LOG_FUNC_EXIT;
    return -1;
  }

  memset(&msg, 0, sizeof(msg));
  msg.msg_name = &ndp->dst;
  msg.msg_namelen = sizeof(ndp->dst);
  msg.msg_iov = (struct iovec *)&ndp->iov[0];
  msg.msg_iovlen = 1;

  if (ndp->ipv6hdr == NULL)
  {
    LOGD("%s: Setting ancilliary address", __func__);
    socket_set_ancilliary_addr(socket, &ndp->src);
  }

  inet_ntop(AF_INET6, &ndp->src.sin6_addr, saddr, sizeof(saddr));
  inet_ntop(AF_INET6, &ndp->dst.sin6_addr, daddr, sizeof(daddr));
  LOGD("Sending from %s%%%d to %s%%%d (len=%zu) on socket %d",
       saddr, ndp->src.sin6_scope_id, daddr, ndp->dst.sin6_scope_id,
       msg.msg_iov->iov_len, socket);
  res = sendmsg(socket, &msg, 0);

  if (res < 0)
  {
    LOGE("Error while sending message: %d", errno);
  }
  else if (res == 0)
  {
    LOGE("EOF while sending message");
  }
  else
  {
    LOGE("%s sent %d bytes", __func__, res);
  }

  return res;
  RADISH_LOG_FUNC_EXIT;
}

/*TODO:check if this fn can be removed*/
void radish_process_router_solicitation
(
  IfaceCfg *cfg,
  struct parsed_nd *sol
)
{
  struct buffer_item *buff=0;
  uint8_t lladdr[6];
  struct nd_rs *h;
  List *cur_pref_list;
  int len;
  struct parsed_nd_option *popt;
  struct parsed_nd *adv=0;
  List *cur;

  RADISH_LOG_FUNC_ENTRY;

  if (!cfg || !sol)
  {
    LOGE("bad parameters received");
    RADISH_LOG_FUNC_EXIT;
    return;
  }

  h = (struct nd_rs *)sol->icmphdr;
  len = sol->length;

  /* TODO: Validate checksum */
  rlist_foreach(cur, &sol->parsed_options) {
    popt = PARSED_OPT(cur);
    if (!popt)
    {
      LOGE("PARSED_OPT returned NULL");
      RADISH_LOG_FUNC_EXIT;
      return;
    }
    switch (popt->option->h.type)
    {
      case ND_OPT_TYPE_SRC_LLA: {
          uint8_t *a = &popt->option->lla.addr.ieee802lla[0];
          LOGD("Received solicitation (len %d) with lla of"
               "%2.2x:%2.2x:%2.2x:%2.2x:%2.2x:%2.2x",
               popt->option->prefix_info.length,
               (int)a[0], (int)a[1], (int)a[2],
               (int)a[3], (int)a[4], (int)a[5]);
          break;
        }
    }
  }

  buff = radish_fetch_available_buffer(__func__);
  if (!buff)
  {
    LOGE("Couldn't fetch available buffer");
    goto error;
  }
  adv = nd_create(ICMPV6_TYPE_ND_RA, buff->iov, IOV_LEN, 0);
  if (!adv)
  {
    LOGE("nd_create failed");
    goto error;
  }

  rlist_foreach(cur_pref_list, &cfg->adv_prefix_list) {
    PrefixItem *pf = PREFIX(cur_pref_list);
    if (!pf)
    {
      LOGE("PREFIX returned NULL");
      goto error;
    }
    nd_add_opt_pref_info(adv,
                         &pf->addr,
                         pf->prefix_length,
                         pf->adv_valid_lifetime,
                         pf->adv_preferred_life_time);
  }

  radish_find_hw_addr(cfg, &lladdr[0]);
  nd_add_opt_lla(sol, lladdr, sizeof(lladdr));
  radish_send_nd_packet(cfg->adv_socket, adv);

error:

  if (buff)
    radish_release_buffer(buff, __func__);
  if (adv)
    nd_free(adv);

  RADISH_LOG_FUNC_EXIT;
}

static void radish_delete_all_prefix_based_route
(
  int prefix_len,
  struct in6_addr *addr
)
{
  char cmd[MAX_CMD_LEN + 1];
  char ipaddr[MAX_IPV6_PREFIX + 1];

  RADISH_LOG_FUNC_ENTRY;

  if (!addr || IN6_IS_ADDR_UNSPECIFIED(addr))
  {
    LOGE("%s: NULL params rcvd", __func__);
    RADISH_LOG_FUNC_EXIT;
    return;
  }

  inet_ntop(AF_INET6, addr, ipaddr, MAX_IPV6_PREFIX);

  /*Delete all old/stale prefix based routes*/
  snprintf(cmd,
           MAX_CMD_LEN,
           "ip route del %s/%d",
           ipaddr,
           prefix_len);

  LOGD("%s: %s", __func__, cmd);

  if (system(cmd) != 0)
  {
    LOGE("Unable to del old prefix based route");
  }
  RADISH_LOG_FUNC_EXIT;
}

static void radish_delete_prefix_based_route
(
  int prefix_len,
  struct in6_addr *addr
)
{
  char cmd[MAX_CMD_LEN + 1];
  char ipaddr[MAX_IPV6_PREFIX + 1];
  int retval = 0;

  RADISH_LOG_FUNC_ENTRY;

  if (!addr || IN6_IS_ADDR_UNSPECIFIED_32(addr))
  {
    LOGE("%s: NULL params rcvd", __func__);
    RADISH_LOG_FUNC_EXIT;
    return;
  }

  inet_ntop(AF_INET6, addr, ipaddr, MAX_IPV6_PREFIX);

  /*Delete old/stale prefix based route*/
  retval = snprintf(cmd,
                    MAX_CMD_LEN,
                    "ip route del %s/%d dev bridge0",
                    ipaddr,
                    prefix_len);
  if (retval >= MAX_CMD_LEN)
  {
    LOGE("%s() String truncation occurred.", __func__);
    return;
  }

  LOGD("%s: %s", __func__, cmd);

  if (system(cmd) != 0)
  {
    LOGE("Unable to delete old prefix based route in the main table");
  }

#if defined FEATURE_DATA_LINUX_ANDROID
  /*Delete old/stale route in the local_network table*/
  retval = snprintf(cmd,
                    MAX_CMD_LEN,
                    "ip -6 route del %s/%d dev bridge0 scope link table %d",
                    ipaddr,
                    prefix_len,
                    ROUTE_TABLE_LOCAL_NETWORK);
  if (retval >= MAX_CMD_LEN)
  {
    LOGE("%s() String truncation occurred.", __func__);
    return;
  }

  LOGD("%s: %s", __func__, cmd);

  if (system(cmd) != 0)
  {
    LOGE("Unable to delete old prefix based route in the local_network table");
  }
#endif /* FEATURE_DATA_LINUX_ANDROID */

  RADISH_LOG_FUNC_EXIT;
}

void radish_add_prefix_based_route
(
  IfaceCfg *cfg,
  int prefix_len,
  struct in6_addr *addr
)
{
  char cmd[MAX_CMD_LEN + 1];
  char ipaddr[MAX_IPV6_PREFIX + 1];
  int retval = 0;

  RADISH_LOG_FUNC_ENTRY;

  if (!addr || IN6_IS_ADDR_UNSPECIFIED_32(addr) || !cfg)
  {
    LOGE("%s: NULL params rcvd", __func__);
    RADISH_LOG_FUNC_EXIT;
    return;
  }

  inet_ntop(AF_INET6, addr, ipaddr, MAX_IPV6_PREFIX);

  /*Adding new prefix based route*/
  retval = snprintf(cmd,
                    MAX_CMD_LEN,
                    "ip route add %s/%d dev bridge0",
                    ipaddr,
                    prefix_len);
  if (retval >= MAX_CMD_LEN)
  {
    LOGE("%s() String truncation occurred.", __func__);
    return;
  }

  LOGD("%s: %s", __func__, cmd);

  if (system(cmd) != 0)
  {
    LOGE("Unable to add a new prefix based route in the main table");
  }

#if defined FEATURE_DATA_LINUX_ANDROID
  /*Adding new route in the local_network table*/
  retval = snprintf(cmd,
                    MAX_CMD_LEN,
                    "ip -6 route add %s/%d dev bridge0 scope link table %d",
                    ipaddr,
                    prefix_len,
                    ROUTE_TABLE_LOCAL_NETWORK);
  if (retval >= MAX_CMD_LEN)
  {
    LOGE("%s() String truncation occurred.", __func__);
    return;
  }

  LOGD("%s: %s", __func__, cmd);

  if (system(cmd) != 0)
  {
    LOGE("Unable to add a new prefix based route in the local_network table");
  }
#endif /* FEATURE_DATA_LINUX_ANDROID */

  RADISH_LOG_FUNC_EXIT;
}

void radish_modify_policy_based_route
(
  char *table_name,
  int  operation
)
{
  char cmd[MAX_CMD_LEN];

  if ((!table_name) ||
      ((operation != RADISH_ADD_OPERATION) && (operation != RADISH_DELETE_OPERATION)))
  {
    LOGE("%s: Invalid params received", __func__);
    return;
  }

  memset(cmd, 0, MAX_CMD_LEN);
  /*Modify policy based route*/
  snprintf(cmd,
           MAX_CMD_LEN,
           "ip -6 rule %s from all iif %s to all lookup %s prio %d",
           operation ? RADISH_ADD : RADISH_DELETE,
           RADISH_BRIDGE_INTERFACE_NAME,
           table_name,
           RADISH_RULE_PRIORITY_TETHERING);

  LOGD("%s: %s", __func__, cmd);

  if (system(cmd) != 0)
  {
    LOGE("Unable to %s policy based route", operation ? RADISH_ADD : RADISH_DELETE);
  }
}

void radish_modify_link_local_route
(
  int operation
)
{
  char cmd[MAX_CMD_LEN];
  int retval = 0;

  if ((operation != RADISH_ADD_OPERATION) && (operation != RADISH_DELETE_OPERATION))
  {
    LOGE("%s: Invalid params received", __func__);
    return;
  }

  /*Add link local route in local network table*/
  retval = snprintf(cmd,
                    MAX_CMD_LEN,
                    "ip -6 route %s %s dev bridge0 scope link table %d",
                    operation ? RADISH_ADD : RADISH_DELETE,
                    IPV6_LINK_LOCAL_ADDRESS,
                    ROUTE_TABLE_LOCAL_NETWORK);
  if (retval >= MAX_CMD_LEN)
  {
    LOGE("%s() String truncation occurred.", __func__);
    return;
  }

  LOGD("%s: %s", __func__, cmd);

  if (system(cmd) != 0)
  {
    LOGE("Unable to %s link local route in local network table",
         operation ? RADISH_ADD : RADISH_DELETE);
  }
}

#define UNUSEDARG(x)  x __attribute__ ((unused))
void radish_process_router_advertisement
(
  IfaceCfg * cfg,
  struct parsed_nd * pnd,
  struct nd_ra * h,
  int UNUSEDARG(len)
)
{
  struct sockaddr_in6 router_prefix;
  struct parsed_nd_option *popt = NULL;
  List *o;

#if defined FEATURE_DATA_LINUX_ANDROID
  struct sockaddr_in6 dest6;
#endif /* FEATURE_DATA_LINUX_ANDROID */

  RADISH_LOG_FUNC_ENTRY;

  if (!cfg || !h || !pnd)
  {
    LOGE("%s: bad parameters received", __func__);
    RADISH_LOG_FUNC_EXIT;
    return;
  }

  rlist_foreach(o, &pnd->parsed_options)
  {
    popt = PARSED_OPT(o);
    if (!popt)
    {
      LOGE("%s: PARSED_OPT failed", __func__);
      return;
    }

    if (popt->option->prefix_info.type == ND_OPT_TYPE_PREF_INFO)
    {

      router_prefix.sin6_family = AF_INET6;
      router_prefix.sin6_addr = popt->option->prefix_info.prefix;

      log_sockaddr(&router_prefix);
      if (!memcmp(&ra_from_modem.global_ra.options->prefix_info.prefix,
                  &popt->option->prefix_info.prefix,
                  sizeof(struct in6_addr )))
      {
      }
      else
      {
#if defined FEATURE_DATA_PREFIX_DELEGATION
        int i = 0, j = 0;
        int firstprefix = FALSE;
        int stale_v6_prefix_length = 0;
        char v6_addr[INET6_ADDRSTRLEN];
        struct in6_addr v6_unspecified, stale_v6_prefix;

        inet_pton(AF_INET6, "::" , &v6_unspecified);

        inet_ntop(AF_INET6, &(ra_from_modem.global_ra.options->prefix_info.prefix),
                  v6_addr, INET6_ADDRSTRLEN);
        stale_v6_prefix_length = ra_from_modem.global_ra.options->prefix_info.prefix_length;
        LOGD("Old prefix is %s, prefix length is %d\n", v6_addr, stale_v6_prefix_length);
        inet_pton(AF_INET6, v6_addr , &stale_v6_prefix);

        inet_ntop(AF_INET6, &(popt->option->prefix_info.prefix), v6_addr, INET6_ADDRSTRLEN);
        LOGD("New prefix is %s\n",v6_addr);

        if (!memcmp(&ra_from_modem.global_ra.options->prefix_info.prefix,
                    &v6_unspecified,
                    sizeof(struct in6_addr ))) {
          firstprefix = TRUE;
        }

        /*Do prefix matching only when a prefix already exists*/
        if (firstprefix == FALSE) {
          LOGD("Finding the shortest prefix");

          for (i = 0 ; i < IPV6_ADDR_LEN ; i++) {
            LOGD("modem.s6_addr[i] is %x, newprefix.s6_addr[i] is %x",
                 ra_from_modem.global_ra.options->prefix_info.prefix.s6_addr[i],
                 popt->option->prefix_info.prefix.s6_addr[i]);

            /* Do a byte wise comparison of the IPv6 addresses. If there is a mismatch,
             * proceed to do a bit wise comparison of that byte. Byte wise comparison must be
             * only till the size of the shortest prefix already calculated.*/
            if ((ra_from_modem.global_ra.options->prefix_info.prefix.s6_addr[i]
                != popt->option->prefix_info.prefix.s6_addr[i]) ||
                (i == stale_v6_prefix_length / 8)) {
              uint8_t prefixxor = 0;

              /* Find the bitwise XOR of the mismatched byte. This will give the exact
               * bits which are not matched. We need to find the position of the
               * most significant bit set. We can find this by obtaining the result of
               * the bitwise AND of the XOR obtained earlier with 1 shifted from the highest
               * bit to the lowest bit. Bit wise comparison must be
               * till only till the size of the shortest prefix already calculated.*/
              prefixxor = ra_from_modem.global_ra.options->prefix_info.prefix.s6_addr[i] ^
                          popt->option->prefix_info.prefix.s6_addr[i];
              LOGD("prefixxor is %x", prefixxor);

              for (j =7; j >=0; j--) {
                if (j == (((i+1)*8) - (stale_v6_prefix_length + 1))) {
                    LOGD("max prefix length possible is at %d\n", j);
                    break;
                }
                if (prefixxor & (1 << j)){
                    LOGD("encountered difference at %d", j);
                    break;
                }
              }

               /* After finding the position of the most significant bit, clear out the
                * lesser significant bits.*/
              ra_from_modem.global_ra.options->prefix_info.prefix.s6_addr[i] =
                (popt->option->prefix_info.prefix.s6_addr[i]>>(j+1))<<(j+1);

              /* Prefix length is the actual number of matching bits. Alternatively, it
               * is the number of bytes till this iteration minus the bits not matched*/
              ra_from_modem.global_ra.options->prefix_info.prefix_length = ((i+1)*8) - (j+1);
              /* Clear out the remaining bytes.*/
              i++;
              while(i < IPV6_ADDR_LEN) {
                ra_from_modem.global_ra.options->prefix_info.prefix.s6_addr[i] = 0;
                i++;
              }
              break;
            }
          }
        } else {
          LOGD("We assign the prefix directly since there is no older prefix");
          ra_from_modem.global_ra.options->prefix_info.prefix_length =
            popt->option->prefix_info.prefix_length;
          ra_from_modem.global_ra.options->prefix_info.prefix =
            popt->option->prefix_info.prefix;
        }
#else
        ra_from_modem.global_ra.options->prefix_info.prefix_length =
          popt->option->prefix_info.prefix_length;
        ra_from_modem.global_ra.options->prefix_info.prefix =
          popt->option->prefix_info.prefix;
#endif /* FEATURE_DATA_PREFIX_DELEGATION */

        ra_from_modem.global_ra.options->prefix_info.type =
          popt->option->prefix_info.type;

        router_prefix.sin6_family = AF_INET6;
        router_prefix.sin6_addr =
          ra_from_modem.global_ra.options->prefix_info.prefix;

        log_sockaddr(&router_prefix);

#if defined FEATURE_DATA_PREFIX_DELEGATION
        /* Delete existing prefix based route after we determine the shortest route.
         * We add the shortest route first before deleting the existing one
         * to ensure that kernel does not try to use the default embedded route for
         * tethered hosts. Modify the routing table if it is not identical to what
         * is already present. */
        if (!(memcmp(&ra_from_modem.global_ra.options->prefix_info.prefix,
                     &stale_v6_prefix,
                     sizeof(struct in6_addr))) &&
              (stale_v6_prefix_length ==
               ra_from_modem.global_ra.options->prefix_info.prefix_length))
        {
          /* If calculated and previous shortest prefixes are same, do nothing.*/
          return;
        }

        /* If this is the first prefix, delete all other routes with same prefix.
         * This is to ensure that there are no conflicting routes between bridge and
         * upstream interfaces. If we are using prefix delegation, then the backhaul
         * interface will have a different prefix compared to the tethered client and
         * hence there will not be any conflicts in the routes with the bridge
         * interface. */
        if (firstprefix == TRUE)
#endif /* FEATURE_DATA_PREFIX_DELEGATION */
          radish_delete_all_prefix_based_route(
                ra_from_modem.global_ra.options->prefix_info.prefix_length,
                &ra_from_modem.global_ra.options->prefix_info.prefix);

        radish_add_prefix_based_route(cfg,
                                      ra_from_modem.global_ra.options->prefix_info.prefix_length,
                                      &ra_from_modem.global_ra.options->prefix_info.prefix);

#if defined FEATURE_DATA_PREFIX_DELEGATION
        radish_delete_prefix_based_route(stale_v6_prefix_length, &stale_v6_prefix);
#endif /* FEATURE_DATA_PREFIX_DELEGATION */
      }
    }
  }

#if defined FEATURE_DATA_LINUX_ANDROID
  memset(&dest6, 0, sizeof(dest6));
  inet_pton(AF_INET6, RADISH_KIF_IPV6_MULTICAST_NODE_LOCAL, &dest6.sin6_addr);
  dest6.sin6_family = AF_INET6;
  pnd->dst.sin6_addr = dest6.sin6_addr;
  pnd->ipv6hdr->ip6_dst = dest6.sin6_addr;
  log_sockaddr(&pnd->dst);
#endif /* FEATURE_DATA_LINUX_ANDROID */
}

void radish_add_to_cache
(
  const char *iface,
  struct in6_addr *addr
)
{
  /* TODO: Use rtnl instead */
  char cmd[MAX_CMD_LEN + 1];
  char ipaddr[MAX_IPV6_PREFIX + 1];

  RADISH_LOG_FUNC_ENTRY;

  if (!addr || IN6_IS_ADDR_UNSPECIFIED_32(addr) || !iface)
  {
    LOGE("%s: invalid params", __func__);
    RADISH_LOG_FUNC_EXIT;
    return;
  }

  inet_ntop(AF_INET6, addr, ipaddr, MAX_IPV6_PREFIX);

  snprintf(cmd, MAX_CMD_LEN, "ip -6 neighbor add proxy %s dev %s", ipaddr, iface);
  LOGD("Adding proxy entry for %s to %s", ipaddr, iface);
  if (0 && system(cmd))
  {
    LOGE("Unable to add proxy entry to the cache");
  }

  RADISH_LOG_FUNC_EXIT;
}

#if 0
int is_intf_has_ip
(
  IfaceCfg *cfg,
  struct in6_addr *addr
)
{
  struct in6_addr tmp;
  int f;
  int res = -1, ret = 0;

  if (!cfg || !addr || !cfg->iface_name )
  {
    LOGE("bad parameters received");
    return ret;
  }

  f = iface_addr_start_traverse();
  while (!(res = get_next_iface_addr(f, cfg->iface_name, 0, &tmp)))
  {
    if (!memcmp(addr, &tmp, sizeof(tmp)))
    {
      ret = 1;
      break;
    }
  }
  iface_addr_end_traverse(f);
  return ret;
}
#endif

static uint16_t radish_nd_checksum
(
  struct parsed_nd *pnd
)
{
  struct r_ip6_hdr *hdr;
  struct r_ip6_hdr tmp;
  uint16_t icmplen = 0;

  if (pnd->ipv6hdr)
  {
    hdr = pnd->ipv6hdr;
    icmplen = (uint16_t)ntohs(hdr->ip6_plen);
  }
  else
  {
    tmp.ip6_flow = 0;
    tmp.ip6_hlim = 255;
    tmp.ip6_nxt = IPTYPE_IPV6_ICMP;
    icmplen = (uint16_t)pnd->length;
    tmp.ip6_plen = (uint16_t)htons(icmplen);
    tmp.ip6_vfc = 0x60;
    tmp.ip6_dst = pnd->dst.sin6_addr;
    tmp.ip6_src = pnd->src.sin6_addr;
    hdr = &tmp;
  }

  LOGD("%s: icmplen is deteremined to be %d", __func__, icmplen);

  return icmp6_checksum(hdr, pnd->icmphdr, icmplen);
}

static uint16_t radish_icmp6_checksum
(
  struct r_ip6_hdr *ip6h,
  struct icmp6_header *icmp6h,
  int len
)
{
  struct r_ip6_hdr *hdr;

  hdr = ip6h;
  return icmp6_checksum(hdr, icmp6h, (uint16_t)len);
}

void radish_send_proxy
(
  IfaceCfg *cfg,
  struct parsed_nd *pnd
)
{
  IfaceCfg *iface;
  List *lst = NULL;
  uint8_t type;
  struct buffer_item *itm = NULL;
  struct parsed_nd *newnd = NULL;
  int i;

  RADISH_LOG_FUNC_ENTRY;

  type = pnd->icmphdr->type;

  rlist_foreach(lst, &(server_cfg.iface_list)) {
    if (!lst)
    {
      LOGE("lst null!!!!");
      goto error;
    }
    iface = IFACECFG(lst);
    if (!iface)
    {
      LOGE("iface null!!!!");
      goto error;
    }
    if (iface != cfg)
    {
      if (iface->iface_params.proxy)
      {
        struct r_ip6_hdr *ip6h;
        struct in6_addr mcgrp;
        if (type == ICMPV6_TYPE_ND_RA)
        {
          /*
           * Some clients send NS to the solicited multicast group
           * instead of the address of the router.
           * If we see a router advertisement, listen to the
           * multicast group for the source address in preparation
           * for future NS
           */
          in6_make_solicited_mcast(&mcgrp, &pnd->src.sin6_addr);
          radish_register_to_mcgroup(iface, &mcgrp);
        }
        itm = radish_fetch_available_buffer(__func__);
        if (itm)
        {
          for (i =0; i < IOV_LEN; i++)
          {
            RAD_LOGI("[%s] Copying from %p to %p", cfg->iface_name,
                     (unsigned*) pnd->iov[i].iov_base,
                     (unsigned*) itm->iov[i].iov_base);
            memcpy(itm->iov[i].iov_base, pnd->iov[i].iov_base, (size_t)IOV_BUFF_LEN);
            itm->iov[i].iov_len = (size_t)pnd->length;
            RAD_LOGI("%s: itm->iov[%d].iov_len = %zu", __func__, i, itm->iov[i].iov_len);
          }
          newnd = nd_parse(itm->iov, (size_t)pnd->length, pnd->ipv6hdr != NULL);
          if (!newnd)
          {
            LOGE("nd_parse returned NULL newnd");
            goto error;
          }
          if (pnd->ipv6hdr != NULL)
          {
            ip6h = (struct r_ip6_hdr *)itm->iov[0].iov_base;
            RAD_LOGI("%s (%s): len: %d, next: %d", __func__, cfg->iface_name,
                     ntohs(ip6h->ip6_plen), ip6h->ip6_nxt);
          }
          newnd->dst = pnd->dst;
          LOGD("NEW Dest:");
          log_sockaddr(&newnd->dst);
          newnd->src = pnd->src;
          LOGD("NEW Src:");
          log_sockaddr(&newnd->src);
          newnd->packet_type = pnd->packet_type;
          /* set source pnd cookie */
          newnd->proxy_cookie = pnd->proxy_cookie;
          /* copy the destination iface name */
          if (iface->iface_name) {
            strlcpy(newnd->iface_name,
                    iface->iface_name,
                    RDSH_IFACE_NAME_MAX);
            LOGD("forward this packet to %s", iface->iface_name);
          }
          radish_add_to_cache(cfg->iface_name, &pnd->dst.sin6_addr);
          pthread_mutex_lock(&iface->proxy_lock);
          rlist_append(&iface->forwarded_packets_list, &newnd->the_list);
          pthread_mutex_unlock(&iface->proxy_lock);
          radish_wakeup(iface->wake_socket[0]);
        }
        else
        {
          LOGE("Couldn't get buffer. Packet not sent to %s", iface->iface_name);
        }
      }
    }
    else
    {
      LOGD("%s Skipping interface %s",__func__, cfg->iface_name);
    }
  }

  RADISH_LOG_FUNC_EXIT;
  return;

error:
  if (itm)
    radish_release_buffer(itm, __func__);
  if (newnd)
    nd_free(newnd);

  RADISH_LOG_FUNC_EXIT;
  return;
}

void radish_recv_proxy
(
  IfaceCfg *cfg,
  struct parsed_nd *pnd
)
{
  List *c;
  struct parsed_nd_option *opt;
  int updated = 0;

  RADISH_LOG_FUNC_ENTRY;

  if (!cfg || !cfg->iface_name || !pnd)
  {
    LOGE("%s: invalid params", __func__);
    goto exit;
  }

  if (pnd->packet_type == ICMPV6_PROTO)
  {
    if (pnd->icmphdr->type == ICMPV6_TYPE_ND_RA ||
        pnd->icmphdr->type == ICMPV6_TYPE_ND_RS ||
        pnd->icmphdr->type == ICMPV6_TYPE_ND_NA ||
        pnd->icmphdr->type == ICMPV6_TYPE_ND_NS ||
        pnd->icmphdr->type == ICMPV6_TYPE_ND_REDIRECT)
    {

      rlist_foreach(c, &pnd->parsed_options) {
        opt = PARSED_OPT(c);
        if (!opt)
        {
          LOGE("PARSED_OPT is NULL");
          return;
        }
        LOGD("popt type: %d, len: %d", opt->option->h.type, opt->option->h.length);

        if (opt->option->h.type == ND_OPT_TYPE_SRC_LLA
            || opt->option->h.type == ND_OPT_TYPE_TGT_LLA)
        {
          updated = 1;
          radish_find_hw_addr(cfg, opt->option->lla.addr.ieee802lla);
          pnd->icmphdr->checksum = 0;
          pnd->icmphdr->checksum = radish_nd_checksum(pnd);
        }
      }

      radish_add_to_cache(cfg->iface_name, &pnd->src.sin6_addr);
    }
  }
  else if (pnd->packet_type == IPPROTO_UDP)
  {
    LOGD("UDP packet received via proxy operation, "
         "to be sent out on %s iface", cfg->iface_name);
  }
  else
  {
    LOGE("unknown packet_type %d received - ignore",
         pnd->packet_type);
    goto exit;
  }

  LOGD("dst scope id changed from %d to %d",
       pnd->dst.sin6_scope_id, cfg->iface_idx);
  pnd->dst.sin6_scope_id = cfg->iface_idx;
  if (0<radish_send_nd_packet(cfg->adv_socket, pnd))
  {
    LOGD("sent ND packet to iface [%s]", cfg->iface_name);
    /* logs proxy operation */
    radish_log_proxy_operation(pnd, "SENT");
  }

exit:
  RADISH_LOG_FUNC_EXIT;
  return;
}

void radish_register_to_mcgroup
(
  IfaceCfg *cfg,
  struct in6_addr *mcgroup
)
{
  List *ifl;
  char addr[MAX_IPV6_PREFIX + 1];

  RADISH_LOG_FUNC_ENTRY;

  if (!mcgroup || !cfg)
  {
    LOGE("invalid params");
    RADISH_LOG_FUNC_EXIT;
    return;
  }

  if (cfg)
  {
    struct ipv6_mreq mreq= {.ipv6mr_interface = (int)cfg->iface_idx,
      .ipv6mr_multiaddr = *mcgroup };
    inet_ntop(AF_INET6, mcgroup, addr, MAX_IPV6_PREFIX);
    LOGD("%s: Adding mcaddr %s to interface %s", __func__, addr, cfg->iface_name);
    if (setsockopt(cfg->rcv_socket,
                   IPPROTO_IPV6,
                   IPV6_ADD_MEMBERSHIP,
                   &mreq,
                   sizeof(mreq)) < 0)
    {
      LOGD("Unable to add membership, errno:%d", errno);
    }
  }
  else
  {
    rlist_foreach(ifl, &server_cfg.iface_list) {
      cfg = IFACECFG(ifl);
      if (!cfg) continue;
      radish_register_to_mcgroup(cfg, mcgroup);
    }
  }

  RADISH_LOG_FUNC_EXIT;
  return;
}

/*
 * Description:
 * Processes the raw message (iovec) and updates the
 * parsed_nd structure accordingly
 *
 * Parameters:
 * cfg: READ-ONLY iface configuration
 * iov: message data
 * pnd: pointer to parsed_nd (to be updated)
 *
 * Return:
 * void
 */
void radish_update_parsed_nd
(
  const IfaceCfg * cfg,
  struct iovec * iov,
  struct parsed_nd * pnd
)
{
  /* sanity */
  if (!cfg || !cfg->iface_name || !iov || !pnd)
  {
    LOGE("%s: programming err: null params", __func__);
    return;
  }

  /* UPDATE HEADERS */

  /* check for icmpv6 and also update ipv6 header */
  if (!radish_is_icmpv6_present(iov, &pnd->ipv6hdr, &pnd->icmphdr))
  {
    pnd->packet_type = IPPROTO_UDP;
  }
  else
  {
    LOGD("[%s] type: %d. code: %d checksum: %d. iov_len: %zu "
         "icmp6h: %p",
         cfg->iface_name, pnd->icmphdr->type,
         pnd->icmphdr->code, pnd->icmphdr->checksum,
         iov->iov_len, (unsigned*)pnd->icmphdr);
    pnd->packet_type = ICMPV6_PROTO;
  }

  /* update source and destination IPv6 addresses */
  memset(&pnd->src, 0, sizeof(pnd->src));
  pnd->src.sin6_family = AF_INET6;
  pnd->src.sin6_scope_id = cfg->iface_idx; /* src iface id */
  pnd->src.sin6_addr = pnd->ipv6hdr->ip6_src;

  memset(&pnd->dst, 0, sizeof(pnd->dst));
  pnd->dst.sin6_family = AF_INET6;
  /* dst scope id will be updated by radish_send_nd_packet */
  pnd->dst.sin6_scope_id = cfg->iface_idx;
  pnd->dst.sin6_addr = pnd->ipv6hdr->ip6_dst;

  /* log addresses */
  log_sockaddr(&pnd->src);
  log_sockaddr(&pnd->dst);

  /* update pnd with proxy log meta data */
  if (!pnd->icmphdr)
  {
    pnd->proxy_cookie = (uint16_t)radish_random_hash(iov);
  }
  else
  {
    pnd->proxy_cookie = pnd->icmphdr->checksum;
  }
  memset(pnd->iface_name, 0, RDSH_IFACE_NAME_MAX);
  if (cfg->iface_name)
    strlcpy(pnd->iface_name,
            cfg->iface_name,
            RDSH_IFACE_NAME_MAX);

  /* UPDATE DATA */
  pnd->iov = iov;
  pnd->length = (int)iov->iov_len;

  return;
}

/*
 * Description:
 * Processes the raw message passed in iovec parameter
 * The incoming UDP message is forwarded to other proxy
 * interfaces.
 *
 * Parameters:
 * cfg: READ-ONLY iface config on which message arrived
 * iov: message data
 *
 * Return:
 * void
 */
void radish_process_udp
(
  const IfaceCfg * cfg,
  struct iovec * iov
)
{
  struct buffer_item * itm = NULL;
  struct parsed_nd * pnd = NULL;
  struct parsed_nd * newnd = NULL;
  List *lst = NULL;
  IfaceCfg * iface = NULL;

  RADISH_LOG_FUNC_ENTRY;

  /* sanity */
  if (!cfg || !iov || iov->iov_len > IOV_BUFF_LEN)
  {
    LOGE("bad parameters received");
    RADISH_LOG_FUNC_EXIT;
    return;
  }

  /* create parsed_nd for this incoming message */
  pnd = (struct parsed_nd *)
        calloc(1, sizeof(struct parsed_nd));
  if (!pnd)
  {
    LOGE("memory error");
    goto failure;
  }
  LOGD("%s: calloc parsed_nd %p", __func__, pnd);

  /* update incoming message in pnd */
  radish_update_parsed_nd(cfg, iov, pnd);

  /* logs proxy operation */
  radish_log_proxy_operation(pnd, "RCVD");

  /* forward this pnd to proxy ifaces */
  rlist_foreach(lst, &(server_cfg.iface_list))
  {
    if (!lst)
    {
      LOGE("List error");
      goto failure;
    }
    iface = IFACECFG(lst);
    /* forward this message to all ifaces other than
       current iface on which the message arrived on */
    if (NULL != iface && iface != cfg && iface->iface_params.proxy)
    {
      /* local item, once this is forwarded,
       * it will be released by the iface thread
       * that receives it */
      itm = radish_fetch_available_buffer(__func__);
      if (!itm)
      {
        LOGE("no more buffers available");
        goto failure;
      }

      /* copy message into local item */
      memcpy(itm->iov[0].iov_base,
             iov->iov_base,
             IOV_BUFF_LEN);
      itm->iov[0].iov_len = (size_t)pnd->length;

      /* store the iov from item into parsed_udp */
      newnd = (struct parsed_nd *)
              calloc(1, sizeof(struct parsed_nd));
      if (!newnd)
      {
        LOGE("memory error");
        goto failure;
      }
      LOGD("%s: calloc parsed_nd %p", __func__, newnd);

      /* update newnd with original iface cfg (cfg)
       * newnd will inherit original iface's scope id */
      radish_update_parsed_nd(cfg,
                              &itm->iov[0],
                              newnd);
      /* override iface_name for logging as the newnd
       * is indeed going out on proxy iface */
      if (iface->iface_name)
        strlcpy(newnd->iface_name,
                iface->iface_name,
                RDSH_IFACE_NAME_MAX);

      /* forward it, wake up the targeted thread */
      pthread_mutex_lock(&iface->proxy_lock);
      rlist_append(&iface->forwarded_packets_list,
                   &newnd->the_list);
      pthread_mutex_unlock(&iface->proxy_lock);
      radish_wakeup(iface->wake_socket[0]);

      /* reset item pointer */
      itm = NULL;
    }
  }

  /* no need for local parsed_nd any more */
  if (pnd)
  {
    nd_free(pnd);
  }
  RADISH_LOG_FUNC_EXIT;
  return;

failure:
  RADISH_LOG_FUNC_EXIT;
  /* no need for local parsed_nd any more */
  if (pnd)
  {
    nd_free(pnd);
  }
  /* if we failed after fetching an item, but before it's
     successfully forwarded, release it now */
  if (itm)
  {
    radish_release_buffer(itm, __func__);
  }
  return;
}

/*
 * Description:
 * Processes the ND message passed in iovec parameter
 * The incoming message is forwarded to other proxy
 * interfaces.
 *
 * Parameters:
 * cfg: iface config on which message arrived
 * iov: message data
 *
 * Return:
 * void
 */
void radish_process_nd
(
  IfaceCfg * cfg,
  struct iovec * iov
)
{
  struct parsed_nd * pnd = NULL;

  RADISH_LOG_FUNC_ENTRY;

  /* sanity */
  if (!cfg || !iov || iov->iov_len > IOV_BUFF_LEN)
  {
    LOGE("bad parameters received");
    RADISH_LOG_FUNC_EXIT;
    return;
  }

  /* create parsed_nd for this incoming message */
  pnd = nd_parse(iov, iov->iov_len, 1);
  if (!pnd)
  {
    LOGE("nd_parse error");
    return;
  }

  /* update incoming message in pnd with original
   * iface config (scope id, src, dst) */
  radish_update_parsed_nd(cfg, iov, pnd);

  /* logs proxy operation */
  radish_log_proxy_operation(pnd, "RCVD");

  /* forward the packet to proxy ifaces */
  if (cfg->iface_params.proxy)
  {

    if (pnd->icmphdr->type == ICMPV6_TYPE_ND_RA)
    {
      radish_process_router_advertisement(cfg,
                                          pnd,
                                          (struct nd_ra*)pnd->icmphdr,
                                          (int)iov->iov_len);

    }
    radish_send_proxy(cfg, pnd);
  }

  if (pnd)
  {
    nd_free(pnd);
  }
  RADISH_LOG_FUNC_EXIT;
  return;
}

/*
 * Description:
 * Processes the MLD message passed in iovec parameter
 *
 * Parameters:
 * cfg: iface config on which message arrived
 * iov: message data
 *
 * Return:
 * void
 */
void radish_process_mld
(
  IfaceCfg * cfg,
  struct iovec * iov
)
{
  struct in6_addr *mcaddr = NULL;
  struct mldp1 *mldp = NULL;
  struct icmp6_header * local_icmphdr = NULL;
  struct parsed_mldp2_lr *plr = NULL;
  struct parsed_mldp2_mcrecord *mcrec =NULL;
  List *l = NULL;

  RADISH_LOG_FUNC_ENTRY;

  if (!cfg || !iov || iov->iov_len > IOV_BUFF_LEN)
  {
    LOGE("bad parameters received");
    RADISH_LOG_FUNC_EXIT;
    return;
  }

  if (!radish_is_icmpv6_present(iov,0,&local_icmphdr))
  {
    LOGE("%s: programming err", __func__);
    return;
  }

  switch (local_icmphdr->type)
  {
    case ICMPV6_TYPE_MLD1_LR:
      {
        mldp = (struct mldp1 *)iov->iov_base;
        LOGD("%s: received MLDV1 Listener Report.",
             __func__);
        mcaddr = &mldp->mcaddr;
        radish_register_to_mcgroup(cfg, mcaddr);
        break;
      }
    case ICMPV6_TYPE_MLD2_LR:
      {
        plr = mldp2_lr_parse(iov, iov->iov_len, 1);
        if (!plr)
        {
          LOGE("plr is NULL");
          return;
        }
        LOGD("%s: received MLDV2 Listener Report. "
             "nrecords: %d",
             __func__, ntohs(plr->lr->nrecords));
        rlist_foreach(l, &plr->mcrecords)
        {
          char addr[MAX_IPV6_PREFIX + 1];
          LOGD("%s: new record. l: %p mc: %p",
               __func__, (unsigned*)l,
               (unsigned*) PARSED_MLDP2_MCREC(l));
          mcrec = PARSED_MLDP2_MCREC(l);
          if (!mcrec)
          {
            LOGE("PARSED_MLDP2_MCREC returned NULL");
            return;
          }
          inet_ntop(AF_INET6,
                    &mcrec->record->mcaddr,
                    addr,
                    MAX_IPV6_PREFIX);
          LOGD("%s: MC Addr: %s. nsources: %d",
               __func__, addr, mcrec->record->nsources);
          radish_register_to_mcgroup(cfg, &mcrec->record->mcaddr);
        }
      }
  }

  RADISH_LOG_FUNC_EXIT;
  return;
}

/*
 * Description:
 * Receives and processes the incoming message
 *
 * Parameters:
 * cfg: READ-ONLY iface config on which message arrived
 *
 * Return:
 * void
 */
void radish_process_msg
(
  IfaceCfg *cfg
)
{
  struct msghdr msg;
  struct icmp6_header * local_icmphdr = NULL;
  struct buffer_item *itm = NULL;
  int res=0;

  RADISH_LOG_FUNC_ENTRY;

  if (!cfg || !cfg->iface_name)
  {
    LOGE("bad parameters received");
    RADISH_LOG_FUNC_EXIT;
    return;
  }

  itm = radish_fetch_available_buffer(__func__);
  if (!itm)
  {
    LOGE("process_msg: Unable to fetch buffer");
    RADISH_LOG_FUNC_EXIT;
    return;
  }

  /* prepare msg for receiving */
  memset(&msg, 0, sizeof(msg));
  msg.msg_iov = &itm->iov[0];
  msg.msg_iovlen = 1;
  msg.msg_control = itm->iov[1].iov_base;

  /* recv msg */
  res = recvmsg(cfg->rcv_socket, &msg, 0);

  if (res > 0)
  {
    LOGD("[%s] process_msg: rcvd %d bytes",
         cfg->iface_name, res);

    /* update iov len on the original item */
    itm->iov[0].iov_len = (size_t)res;

    radish_log_raw_msg(msg.msg_iov);

    if (!radish_is_icmpv6_present(msg.msg_iov,0,&local_icmphdr))
    {
      LOGD("process_msg: UDP packet rcvd");
      radish_process_udp(cfg, msg.msg_iov);
    }
    else if (is_nd(local_icmphdr))
    {
      LOGD("process msg: ND packet rcvd");
      radish_process_nd(cfg, msg.msg_iov);
    }
    else
    {
      LOGE("process msg: ICMPv6 type [%d] not supported",
           local_icmphdr->type);
    }
  }
  else
  {
    LOGE("msg received with length %d, errno %d",
         res, errno);
  }

  if (itm)
    radish_release_buffer(itm, __func__);

  RADISH_LOG_FUNC_EXIT;
  return;
}

void radish_process_events
(
  IfaceCfg *cfg
)
{
  List *recv;
  struct buffer_item *itm;
  struct parsed_nd *rnd=0;

  RADISH_LOG_FUNC_ENTRY;

  if (!cfg)
  {
    LOGE("bad parameters received");
    RADISH_LOG_FUNC_EXIT;
    return;
  }


  if (FD_ISSET(cfg->wake_socket[1], &cfg->rfds))
  {
    char str[1];

    LOGD("[%s] Receiving in wake socket", cfg->iface_name);
    read(cfg->wake_socket[1], str, sizeof(str));

    if (cfg->iface_params.proxy)
    {
      pthread_mutex_lock(&cfg->proxy_lock);
      recv = rlist_pop_first(&cfg->forwarded_packets_list);
      pthread_mutex_unlock(&cfg->proxy_lock);
      rnd = PARSED_ND(recv);

      if (recv && rnd)
      {
        log_sockaddr(&rnd->src);
        log_sockaddr(&rnd->dst);

        radish_recv_proxy(cfg, rnd);

        itm = IOV_OWNER(rnd->iov);
        radish_release_buffer(itm, __func__);
        nd_free(rnd);
      }
      else
      {
        LOGE("null from proxy queue recv: %p", (unsigned*)recv);
      }

    }

  }

  if (FD_ISSET(cfg->rcv_socket, &cfg->rfds))
  {
    radish_process_msg(cfg);
  }

  RADISH_LOG_FUNC_EXIT;
  return;
}

#if defined FEATURE_DATA_LINUX_ANDROID

/*===========================================================================
  FUNCTION  radish_send_icmpv6_router_solicitation
===========================================================================*/
/*!
@brief
  Sends a ICMPV6 router solicitation message
*/
/*=========================================================================*/
int
radish_send_icmpv6_router_solicitation (const char *iface_name)
{
  int sock_fd = -1;
  struct icmpv6_header router_solicit;
  struct sockaddr_in6 dest6;
  struct sockaddr_in6* __attribute__((__may_alias__)) dest6_ptr;
  int ret = -1;
  int hop_limit = 255;

  if ((sock_fd = socket(AF_INET6, SOCK_RAW, IPPROTO_ICMPV6)) < 0)
  {
    LOGE("router solicitation socket() failed:");
    goto bail;
  }

  LOGE("router solicitation setting hoplimit[%d] interface[%s]",
                 hop_limit, iface_name);

  /* Set the multicast hop limit */
  if( -1 == setsockopt( sock_fd,
                        IPPROTO_IPV6,
                        IPV6_MULTICAST_HOPS,
                        (char *) &hop_limit,
                        sizeof(hop_limit)) )
  {
    LOGE("router solicitation setsockopt() failed to set hop limit:");
    goto bail;
  }

  /* Bind to the specific link interface */
  if( -1 == setsockopt( sock_fd,
                        SOL_SOCKET,
                        SO_BINDTODEVICE,
                        (char *)iface_name,
                        (socklen_t)(strlen(iface_name)+ 1)))
  {
    LOGE("router solicitation setsockopt() failed on iface bind:");
    goto bail;
  }

  router_solicit.icmp6_type = RADISH_ND_ROUTER_SOLICIT;
  router_solicit.icmp6_code = 0;

  memset(&dest6, 0, sizeof(dest6));
  inet_pton(AF_INET6, RADISH_IPV6_MULTICAST_ROUTER_ADDR, &dest6.sin6_addr);
  dest6.sin6_family = AF_INET6;
  dest6_ptr = &dest6;


  if (sendto(sock_fd,
             &router_solicit,
             sizeof(router_solicit),
             0,
             (struct sockaddr*)dest6_ptr,
             sizeof(dest6)) < 0)
  {
    LOGE("router solication sendto() failed:");
    goto bail;
  }

  LOGD("sending router solicitation sent succesfully");

  ret = 0;

bail:
  if (-1 != sock_fd)
  {
    close(sock_fd);
  }

  return ret;
}

#endif /* FEATURE_DATA_LINUX_ANDROID */


static void *radish_advertise_thread
(
  void *arg
)
{
  IfaceCfg *cfg = (IfaceCfg *)arg;

  RADISH_LOG_FUNC_ENTRY;

  LOGD("> %s (%s)", __func__, cfg->iface_name);
  if (radish_setup_listener(cfg))
  {
    LOGE("Error setting up listener");
    goto error;
  }

  while (!cfg->exit)
  {
    radish_prepare_fds(cfg);
    radish_wait_for_events(cfg);
    radish_process_events(cfg);
  }

error:
  LOGD("< %s (%s)", __func__, cfg->iface_name);
  RADISH_LOG_FUNC_EXIT;
  return NULL;
}

void
radish_cleanup_handler
(
  void
)
{
  char cmd[MAX_CMD_LEN + 1];

  /*Delete the prefix based route on the bridge interface*/
  radish_delete_prefix_based_route(ra_from_modem.global_ra.options->prefix_info.prefix_length,
                                   &ra_from_modem.global_ra.options->prefix_info.prefix);

  if (server_cfg.is_bridge_mode_on)
  {
    radish_create_forward_dropall(RADISH_FORWARD_CHAIN,
                                  RADISH_DEL_COUNTER_FORWARD_DROPALL);


    if(NULL != g_cur_iface)
    {
       radish_delete_counter_rules(RADISH_TETHER_COUNTERS_CHAIN,
                                   RADISH_BRIDGE_INTERFACE_NAME,
                                   g_cur_iface);

       radish_delete_counter_rules(RADISH_TETHER_COUNTERS_CHAIN,
                                   g_cur_iface,
                                   RADISH_BRIDGE_INTERFACE_NAME);

      radish_create_forward_rules(RADISH_FORWARD_CHAIN,
                                  RADISH_TETHER_COUNTERS_CHAIN,
                                  RADISH_BRIDGE_INTERFACE_NAME,
                                  g_cur_iface,
                                  RADISH_DEL_COUNTER_FORWARD_RULE);
    }
    radish_delete_tether_counter_chain(RADISH_TETHER_COUNTERS_CHAIN);

    if (server_cfg.use_policy_based_route)
    {
      radish_modify_policy_based_route(server_cfg.policy_route_table_name,
                                       RADISH_DELETE_OPERATION);
      if (server_cfg.policy_route_table_name) {
        free(server_cfg.policy_route_table_name);
      }
    }

#if defined FEATURE_DATA_LINUX_ANDROID
  radish_modify_link_local_route(RADISH_DELETE_OPERATION);
#endif /* FEATURE_DATA_LINUX_ANDROID */

    /*The bridge interface has to be down before it can be removed*/
    snprintf(cmd,
             MAX_CMD_LEN,
             "ifconfig %s down",
             RADISH_BRIDGE_INTERFACE_NAME);

    LOGD("%s: %s", __func__, cmd);

    if (system(cmd) != 0)
    {
      LOGE("Cannot Set the bridge interface state to UP");
    }

    snprintf(cmd,
             MAX_CMD_LEN,
             "brctl delbr %s",
             RADISH_BRIDGE_INTERFACE_NAME);

    LOGD("%s: %s", __func__, cmd);

    if (system(cmd) != 0)
    {
      LOGE("Cannot delete the bridge interface");
    }

    /*Flush ebtables rules in the brouting chain*/
    snprintf(cmd,
             MAX_CMD_LEN,
             "ebtables -t broute -F");

    LOGD("%s: %s", __func__, cmd);

    if (system(cmd) != 0)
    {
      LOGE("Cannot set ebtables to drop IPV4");
    }

    if(NULL != g_cur_iface)
    {
        snprintf(cmd,
                 MAX_CMD_LEN,
                 "echo 1 > /proc/sys/net/ipv6/conf/%s/accept_ra_prefix_route",
                 g_cur_iface);

        LOGD("%s: %s", __func__, cmd);

        if (system(cmd) != 0)
        {
          LOGE("Cannot reset accept_ra_prefix_route to default value");
        }
    }
  }

#ifdef RADISH_LOG_QXDM
  (void) Diag_LSM_DeInit();
#endif
}

static void radish_signal_handler
(
  int sig
)
{
  switch (sig)
  {
  case SIGTERM:
  /* On TERM signal, exit() so atexit
   * cleanup functions gets called
   */
    LOGE("caught SIGTERM signal");
    exit(0);
    break;

  default:
    break;
  }
}


/*
 * Arguments are:
 * radish -i <interface1> [-x] [-p <prefix1>/prefix_length1] [-p <prefix2>/prefix_length2..] [-i <interface2> -p...]
 *  -i <interface> Specify an interface to advertise on or to proxy
 *  -x Proxy an interface (forward Neighbor Discovery Protocol packets)
 *  -p <prefix>/prefix_length: Prefix to advertise on interface
 * e.g. radish -i usb0 -p 2001:db8:1234::/64
 */
int main(int argc, char *argv[])
{
  int ret = 0;
  List *cfg_lst = NULL;

  struct sigaction action;
  memset (&action, 0, sizeof(action));

#ifdef RADISH_LOG_QXDM
  Diag_LSM_Init(NULL);
#endif

  RAD_LOGI("radish v%s Starting", RADISH_VERSION);

  inet_pton(AF_INET6, ALL_ROUTERS_MGROUP, &all_routers_in6_addr);
  inet_pton(AF_INET6, ALL_ROUTERS_MLD2, &all_mld2_routers_in6_addr);
  inet_pton(AF_INET6, ALL_HOSTS_MGROUP, &all_hosts_in6_addr);

  srandom((unsigned int)time(NULL));

  rlist_init(&server_cfg.iface_list);
  rlist_init(&available_buffers);

  iface_default_params(&server_cfg.global_params.iface_params);

  inet_pton(AF_INET6, "::" , &ra_from_modem.global_ra.options->prefix_info.prefix);
  ra_from_modem.global_ra.options->prefix_info.prefix_length = 0;
  server_cfg.backhaul = RADISH_DEFAULT_BACKHAUL;
  server_cfg.is_bridge_mode_on = TRUE;
  server_cfg.use_policy_based_route = FALSE;

  atexit(radish_cleanup_handler);
  action.sa_handler = radish_signal_handler;
  sigaction(SIGTERM, &action, 0);

  radish_parse_args(&server_cfg, argc, argv);

  radish_timer_init(&global_timer);

  /* open radish_proxy_log file and write column headers to it */
  radish_log_f = fopen(RADISH_PROXY_LOG_FILE, "w");
  if (!radish_log_f)
  {
    LOGD("couldn't open radish_log_f, errno = %d", errno);
  }
  else
    fprintf(radish_log_f, "%s,%s,%s,%s,%s,%s\n",
            "OPERATION",
            "HANDLE",
            "IFACE",
            "TYPE",
            "SRC",
            "DST");
  radish_raw_f = fopen(RADISH_PROXY_RAW_FILE, "w");
  if (!radish_log_f)
    LOGD("couldn't open radish_log_f, errno = %d", errno);

  rlist_foreach(cfg_lst, &server_cfg.iface_list) {
    IfaceCfg *icfg = IFACECFG(cfg_lst);
    if (!icfg)
    {
      LOGE("IFACECFG returned NULL");
      LOGE("main() failed...can't continue");
      exit(1);
    }
    if (pthread_mutex_init(&icfg->adv_time_lock, NULL) < 0)
    {
      LOGE("Error initializing Wake Socket lock");
      continue;
    }

    if (pthread_mutex_init(&icfg->proxy_lock, NULL) < 0)
    {
      LOGE("Error initializing proxy lock");
      continue;
    }

    if (socketpair(AF_LOCAL, SOCK_DGRAM, 0, icfg->wake_socket) < 0)
    {
      LOGE("Error creating control socketpair for iface %s", icfg->iface_name);
      continue;
    }

    RAD_LOGI("Listening on interface %s", icfg->iface_name);

    pthread_create( &icfg->adv_thread, NULL, radish_advertise_thread, icfg);
  }

#if defined FEATURE_DATA_LINUX_ANDROID
  sleep(1);

  rlist_foreach(cfg_lst, &server_cfg.iface_list) {
    IfaceCfg *icfg = IFACECFG(cfg_lst);
    if (icfg){
      radish_send_icmpv6_router_solicitation(icfg->iface_name);
    }
  }
#endif /* FEATURE_DATA_LINUX_ANDROID */

  rlist_foreach(cfg_lst, &server_cfg.iface_list) {
    IfaceCfg *icfg = IFACECFG(cfg_lst);
    if (!icfg)
    {
      LOGE("IFACECFG returned NULL");
      LOGE("main() failed...can't continue");
      exit(1);
    }
    if (pthread_join(icfg->adv_thread, NULL))
    {
      LOGD("Error joining thread for interface %s", icfg->iface_name);
    }
  }

  return ret;
}
