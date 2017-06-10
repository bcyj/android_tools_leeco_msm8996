/******************************************************************************

                        N E T M G R _ U T I L . C

******************************************************************************/

/******************************************************************************

  @file    netmgr_util.c
  @brief   Netowrk Manager Utility Functions Implementation File

  DESCRIPTION
  Implementation file for NetMgr utility functions.

******************************************************************************/
/*===========================================================================

  Copyright (c) 2010-2014 Qualcomm Technologies, Inc. All Rights Reserved

  Qualcomm Technologies Proprietary

  Export of this technology or software is regulated by the U.S. Government.
  Diversion contrary to U.S. law prohibited.

  All ideas, data and information contained in or disclosed by
  this document are confidential and proprietary information of
  Qualcomm Technologies, Inc. and all rights therein are expressly reserved.
  By accepting this material the recipient agrees that this material
  and the information contained therein are held in confidence and in
  trust and will not be used, copied, reproduced in whole or in part,
  nor its contents revealed in any manner to others without the express
  written permission of Qualcomm Technologies, Inc.

===========================================================================*/


/******************************************************************************

                      EDIT HISTORY FOR FILE

  $Id:$

when       who        what, where, why
--------   ---        -------------------------------------------------------
02/10/10   ar         Initial version (derived from DSC file)

******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <sys/stat.h>
#include <ctype.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/ioctl.h>
#include <asm/types.h>
#include <linux/if.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include "netmgr_util.h"
#include "ds_string.h"

/*===========================================================================
                     GLOBAL DEFINITIONS AND DECLARATIONS
===========================================================================*/

#define NETMGR_UTIL_MAX_V4_ADDR_BUF_SIZE (19)
#define NETMGR_UTIL_MAX_V6_ADDR_BUF_SIZE (44)

/* Control function entry/exit debug messages */
boolean function_debug = FALSE;

/*===========================================================================
  FUNCTION  netmgr_util_to_hex
===========================================================================*/
/*!
@brief
  Returns the character representation for the given hex number

@return
  Character corresponding to the given hex number

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
LOCAL char netmgr_util_to_hex(unsigned int x)
{
  char c = '0';
  static char to_hex_tbl[] =
  {
    '0', '1', '2', '3', '4', '5', '6', '7',
    '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'
  };

  if (x < 16)
  {
    c = to_hex_tbl[x];
  }

  return c;
}

/*===========================================================================
                            GLOBAL FUNCTION DEFINITIONS
===========================================================================*/

#ifdef NETMGR_TEST
/*===========================================================================
  FUNCTION  netmgr_debug_malloc
===========================================================================*/
/*!
@brief
  Debug wrapper for malloc

@return
  void* - Pointer to heap memeory allocation

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
void * netmgr_debug_malloc( size_t memsize )
{
  void* ptr;
  //netmgr_log_med("netmgr_debug_malloc: memsize=%d\n", (int)memsize);
  ptr = malloc( memsize );
  netmgr_log_med("netmgr_debug_malloc: ptr=%p  memsize=%d\n", ptr, (int)memsize);
  return ptr;
}

/*===========================================================================
  FUNCTION  netmgr_debug_free
===========================================================================*/
/*!
@brief
  Debug wrapper for free

@return
  None

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
void netmgr_debug_free( void* ptr )
{
  netmgr_log_med("netmgr_debug_free: ptr=%p\n", ptr);
  free( ptr );
}
#endif /* NETMGR_TEST */


/*===========================================================================
  FUNCTION  netmgr_daemonize
===========================================================================*/
/*!
@brief
 Performs typical tasks required to run a program as a daemon process.

@return
  void

@note

  - Dependencies
    - None

  - Side Effects
    - Original program will exit and a child is forked which will continue
      execution as the daemon.
*/
/*=========================================================================*/
void netmgr_daemonize (void)
{
  pid_t pid;
  pid_t sid;

  /* Fork and exit parent process to ensure that process is not a process
  ** group leader.
  */
  if ((pid = fork()) > 0)
  {
    exit(0);
  }

  if (pid < 0)
  {
    /* Could not create child process. Exit */
    NETMGR_ABORT("netmgr_daemonize: Could not create child process\n");
    return;
  }

  /* Become session group leader to disassociate from controlling terminal */
  sid = setsid();

  if (sid < 0)
  {
    NETMGR_ABORT("netmgr_daemonize: setsid() failed\n");
    return;
  }

  /* Set file mode creation mask to 0, to avoid having permissions of created
  ** files being inadvertently changed.
  */
  (void)umask(0);

  /* Change directory to root */
  if ((chdir("/")) < 0)
  {
    NETMGR_ABORT("netmgr_daemonize: chdir to root failed\n");
    return;
  }

  /* Redirect stdin, stdout and stderr to /dev/null. If running as a daemon,
  ** it is assumed that logging will be to syslog.
  */
  if (freopen("/dev/null", "r", stdin) == NULL)
  {
    NETMGR_ABORT("netmgr_daemonize: freopen of stdin failed\n");
    return;
  }

  if (freopen("/dev/null", "w", stdout) == NULL)
  {
    NETMGR_ABORT("netmgr_daemonize: freopen of stdout failed\n");
    return;
  }

  if (freopen("/dev/null", "w", stderr) == NULL)
  {
    NETMGR_ABORT("netmgr_daemonize: freopen of stderr failed\n");
    return;
  }
}

#ifdef FEATURE_DATA_IWLAN
/*===========================================================================
  FUNCTION  netmgr_util_convert_ip_addr_to_str
===========================================================================*/
/*!
@brief
  Converts the given Netmgr IP address type to a string

@return
  int - NETMGR_SUCCESS on operation success, NETMGR_FAILURE otherwise

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
int netmgr_util_convert_ip_addr_to_str
(
  netmgr_ip_address_t  *ip,
  unsigned int         prefix_len,
  char                 *buf,
  unsigned int         buf_len
)
{
  int ret = NETMGR_FAILURE;;

  if (!ip || !buf)
  {
    netmgr_log_err("invalid input");
    goto bail;
  }

  if (NETMGR_IPV4_ADDR == ip->type &&
      buf_len < NETMGR_UTIL_MAX_V4_ADDR_BUF_SIZE)
  {
    netmgr_log_err("insufficient storage [%u] for V4 address", buf_len);
    goto bail;
  }

  if (NETMGR_IPV6_ADDR == ip->type &&
      buf_len < NETMGR_UTIL_MAX_V6_ADDR_BUF_SIZE)
  {
    netmgr_log_err("insufficient storage [%u] for V6 address", buf_len);
    goto bail;
  }

  switch (ip->type)
  {
    case NETMGR_IPV4_ADDR:
      if (prefix_len)
      {
        snprintf(buf,
                 buf_len,
                 "%d.%d.%d.%d/%d",
                 (unsigned char)(ip->addr.v4),
                 (unsigned char)(ip->addr.v4 >> 8),
                 (unsigned char)(ip->addr.v4 >> 16),
                 (unsigned char)(ip->addr.v4 >> 24),
                 prefix_len);
      }
      else
      {
        snprintf(buf,
                 buf_len,
                 "%d.%d.%d.%d",
                 (unsigned char)(ip->addr.v4),
                 (unsigned char)(ip->addr.v4 >> 8),
                 (unsigned char)(ip->addr.v4 >> 16),
                 (unsigned char)(ip->addr.v4 >> 24));
      }
      break;

    case NETMGR_IPV6_ADDR:
      if (prefix_len)
      {
        snprintf(buf,
                 buf_len,
                 "%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x/%d",
                 (unsigned char)(ip->addr.v6_addr8[0]),
                 (unsigned char)(ip->addr.v6_addr8[1]),
                 (unsigned char)(ip->addr.v6_addr8[2]),
                 (unsigned char)(ip->addr.v6_addr8[3]),
                 (unsigned char)(ip->addr.v6_addr8[4]),
                 (unsigned char)(ip->addr.v6_addr8[5]),
                 (unsigned char)(ip->addr.v6_addr8[6]),
                 (unsigned char)(ip->addr.v6_addr8[7]),
                 (unsigned char)(ip->addr.v6_addr8[8]),
                 (unsigned char)(ip->addr.v6_addr8[9]),
                 (unsigned char)(ip->addr.v6_addr8[10]),
                 (unsigned char)(ip->addr.v6_addr8[11]),
                 (unsigned char)(ip->addr.v6_addr8[12]),
                 (unsigned char)(ip->addr.v6_addr8[13]),
                 (unsigned char)(ip->addr.v6_addr8[14]),
                 (unsigned char)(ip->addr.v6_addr8[15]),
                 prefix_len);
      }
      else
      {
        snprintf(buf,
                 buf_len,
                 "%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x",
                 (unsigned char)(ip->addr.v6_addr8[0]),
                 (unsigned char)(ip->addr.v6_addr8[1]),
                 (unsigned char)(ip->addr.v6_addr8[2]),
                 (unsigned char)(ip->addr.v6_addr8[3]),
                 (unsigned char)(ip->addr.v6_addr8[4]),
                 (unsigned char)(ip->addr.v6_addr8[5]),
                 (unsigned char)(ip->addr.v6_addr8[6]),
                 (unsigned char)(ip->addr.v6_addr8[7]),
                 (unsigned char)(ip->addr.v6_addr8[8]),
                 (unsigned char)(ip->addr.v6_addr8[9]),
                 (unsigned char)(ip->addr.v6_addr8[10]),
                 (unsigned char)(ip->addr.v6_addr8[11]),
                 (unsigned char)(ip->addr.v6_addr8[12]),
                 (unsigned char)(ip->addr.v6_addr8[13]),
                 (unsigned char)(ip->addr.v6_addr8[14]),
                 (unsigned char)(ip->addr.v6_addr8[15]));
      }

      break;

    default:
      netmgr_log_err("netmgr_util_convert_ip_addr_to_str: unknown address type=%d",
                     ip->type);
      goto bail;
  }

  ret = NETMGR_SUCCESS;

bail:
  return ret;
}

/*===========================================================================
  FUNCTION  netmgr_util_convert_qmi_ip_addr_to_str
===========================================================================*/
/*!
@brief
  Converts the given QMI IP address type to a string

@return
  int - NETMGR_SUCCESS on operation success, NETMGR_FAILURE otherwise

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
int netmgr_util_convert_qmi_ip_addr_to_str
(
  qmi_wds_ip_addr_type *ip,
  char                 *buf,
  unsigned int         buf_len
)
{
  int ret = NETMGR_FAILURE;;

  if (!ip || !buf)
  {
    netmgr_log_err("invalid input");
    goto bail;
  }

  if (QMI_WDS_IPV4_TYPE == ip->family &&
      buf_len < NETMGR_UTIL_MAX_V4_ADDR_BUF_SIZE)
  {
    netmgr_log_err("insufficient storage [%u] for V4 address", buf_len);
    goto bail;
  }

  if (QMI_WDS_IPV6_TYPE == ip->family &&
      buf_len < NETMGR_UTIL_MAX_V6_ADDR_BUF_SIZE)
  {
    netmgr_log_err("insufficient storage [%u] for V6 address", buf_len);
    goto bail;
  }

  switch (ip->family)
  {
    case QMI_WDS_IPV4_TYPE:
      snprintf(buf,
               buf_len,
               "%d.%d.%d.%d",
               (unsigned char)(ip->addr.ipv4 >> 24),
               (unsigned char)(ip->addr.ipv4 >> 16),
               (unsigned char)(ip->addr.ipv4 >> 8),
               (unsigned char)(ip->addr.ipv4));
      break;

    case QMI_WDS_IPV6_TYPE:
      snprintf(buf,
               buf_len,
               "%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x",
               (unsigned char)(ip->addr.ipv6[0]),
               (unsigned char)(ip->addr.ipv6[1]),
               (unsigned char)(ip->addr.ipv6[2]),
               (unsigned char)(ip->addr.ipv6[3]),
               (unsigned char)(ip->addr.ipv6[4]),
               (unsigned char)(ip->addr.ipv6[5]),
               (unsigned char)(ip->addr.ipv6[6]),
               (unsigned char)(ip->addr.ipv6[7]),
               (unsigned char)(ip->addr.ipv6[8]),
               (unsigned char)(ip->addr.ipv6[9]),
               (unsigned char)(ip->addr.ipv6[10]),
               (unsigned char)(ip->addr.ipv6[11]),
               (unsigned char)(ip->addr.ipv6[12]),
               (unsigned char)(ip->addr.ipv6[13]),
               (unsigned char)(ip->addr.ipv6[14]),
               (unsigned char)(ip->addr.ipv6[15]));
      break;

    default:
      netmgr_log_err("netmgr_util_convert_qmi_ip_addr_to_str: unknown address type=%d",
                     ip->family);
      goto bail;
  }

  ret = NETMGR_SUCCESS;

bail:
  return ret;
}

/*===========================================================================
  FUNCTION  netmgr_util_convert_qmi_ipsec_key_to_str
===========================================================================*/
/*!
@brief
  Converts the given IPSec hash/crypto key to a string

@return
  int - NETMGR_SUCCESS on operation success, NETMGR_FAILURE otherwise

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
int netmgr_util_convert_qmi_ipsec_key_to_str
(
  qmi_wds_ipsec_key_type  *key,
  char                    *buf,
  unsigned int            buf_len
)
{
  int ret = NETMGR_FAILURE;;
  int i, j = 0;

  if (!key || !buf)
  {
    netmgr_log_err("invalid input");
    goto bail;
  }

  /* Need to prepend 0x and each digit in key translates to two hex characters */
  if (buf_len < (unsigned int)(key->size * 2 + 3))
  {
    netmgr_log_err("insufficient storage [%u] for key size=%d", buf_len, key->size);
    goto bail;
  }

  *buf++ = '0';
  *buf++ = 'x';

  for (i = 0; i < key->size; ++i)
  {
    *buf++ = netmgr_util_to_hex((key->key[i] >> 4) & 0x0F);
    *buf++ = netmgr_util_to_hex(key->key[i] & 0x0F);
  }

  *buf = '\0';

  ret = NETMGR_SUCCESS;

bail:
  return ret;
}

/*===========================================================================
  FUNCTION  netmgr_util_get_ipsec_proto_str
===========================================================================*/
/*!
@brief
  Returns the string representation of the given IPSec protocol

@return
  Pointer to the IPSec proto string on success or NULL on error

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
const char *netmgr_util_get_ipsec_proto_str
(
  qmi_wds_ipsec_sa_proto_type  proto
)
{
  const char *proto_str = NULL;

  switch (proto)
  {
    case QMI_WDS_IPSEC_SA_PROTO_AH:
      proto_str = "ah";
      break;

    case QMI_WDS_IPSEC_SA_PROTO_ESP:
      proto_str = "esp";
      break;

    default:
      break;
  }

  return proto_str;
}

/*===========================================================================
  FUNCTION  netmgr_util_get_ipsec_algo_str
===========================================================================*/
/*!
@brief
  Returns the string representation of the given IPSec algorithm

@return
  Pointer to the IPSec algo string on success or NULL on error

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
const char *netmgr_util_get_ipsec_algo_str
(
  netmgr_ipsec_algo_type        type,
  qmi_wds_ipsec_sa_config_type  *sa_config
)
{
  const char *algo_str = NULL;
  qmi_wds_ipsec_hash_crypto_algo_type  algo;

  NETMGR_LOG_FUNC_ENTRY;

  switch (type)
  {
    case NETMGR_IPSEC_ALGO_HASH:
    {
      algo = sa_config->hash_algo;

      if (QMI_WDS_IPSEC_ALGO_AES128 == algo)
      {
        algo_str = "\'xcbc(aes)\'";
      }
      else if (QMI_WDS_IPSEC_ALGO_SHA == algo)
      {
        algo_str = "\'hmac(sha1)\'";
      }
      else if (QMI_WDS_IPSEC_ALGO_MD5 == algo)
      {
        algo_str = "\'hmac(md5)\'";
      }
    }
      break;

    case NETMGR_IPSEC_ALGO_CRYPTO:
    {
      algo = sa_config->crypto_algo;

      if (QMI_WDS_IPSEC_ALGO_AES128 == algo ||
          QMI_WDS_IPSEC_ALGO_AES256 == algo)
      {
        if (!(sa_config->param_mask & QMI_WDS_IPSEC_AES_MODE_PARAM_MASK) ||
             (QMI_WDS_IPSEC_AES_ALGO_MODE_CBC == sa_config->aes_mode))
        {
          algo_str = "\'cbc(aes)\'";
        }
        else if (QMI_WDS_IPSEC_AES_ALGO_MODE_CTR == sa_config->aes_mode)
        {
          algo_str = "\'rfc3686(ctr(aes))\'";
        }
        else
        {
          netmgr_log_err("%s(): invalid AES mode=%d received",
                         __func__, sa_config->aes_mode);
          break;
        }
      }
      else if (QMI_WDS_IPSEC_ALGO_DES == algo)
      {
        algo_str = "\'cbc(des)\'";
      }
      else if (QMI_WDS_IPSEC_ALGO_3DES == algo)
      {
        algo_str = "\'cbc(des3_ede)\'";
      }
    }
    break;

    default:
      break;
  }

  NETMGR_LOG_FUNC_EXIT;
  return algo_str;
}

/*===========================================================================
  FUNCTION  netmgr_util_get_ipsec_mode_str
===========================================================================*/
/*!
@brief
  Returns the string representation of the given IPSec encapsulation mode

@return
  Pointer to the IPSec mode string on success or NULL on error

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
const char *netmgr_util_get_ipsec_mode_str
(
  qmi_wds_ipsec_encapsulation_mode  mode
)
{
  const char *mode_str = NULL;

  switch (mode)
  {
    case QMI_WDS_IPSEC_ENCAP_MODE_TUNNEL:
      mode_str = "tunnel";
      break;

    default:
      break;
  }

  return mode_str;
}

/*===========================================================================
  FUNCTION  netmgr_util_circ_list_init
===========================================================================*/
/*!
@brief
  Initializes the given circular list

@return
  int - NETMGR_SUCCESS on operation success, NETMGR_FAILURE otherwise

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
int netmgr_util_circ_list_init
(
  netmgr_util_circ_list_type  *clist
)
{
  int rc = NETMGR_FAILURE;

  NETMGR_LOG_FUNC_ENTRY;

  if (!clist)
  {
    netmgr_log_err("invalid input");
    goto bail;
  }

  clist->r = 0;
  clist->w = 0;
  memset(clist->data, 0, sizeof(clist->data));

  rc = NETMGR_SUCCESS;

bail:
  NETMGR_LOG_FUNC_EXIT;
  return rc;
}

/*===========================================================================
  FUNCTION  netmgr_util_circ_list_destroy
===========================================================================*/
/*!
@brief
  Frees any remaining data on the list before initializing it

@return
  int - NETMGR_SUCCESS on operation success, NETMGR_FAILURE otherwise

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
int netmgr_util_circ_list_destroy
(
  netmgr_util_circ_list_type  *clist
)
{
  int rc = NETMGR_FAILURE;
  int i;

  NETMGR_LOG_FUNC_ENTRY;

  if (!clist)
  {
    netmgr_log_err("invalid input");
    goto bail;
  }

  /* If there are any elements in the list */
  if (clist->r != clist->w)
  {
    if (clist->r > clist->w)
    {
      for (i = clist->r; i < NETMGR_UTIL_MAX_CIRC_LIST_SIZE+1; ++i)
      {
        ds_free(clist->data[i]);
      }
      /* Start from 0 */
      clist->r = 0;
    }

    for (i = clist->r; i < clist->w; ++i)
    {
      ds_free(clist->data[i]);
    }
  }

  rc = netmgr_util_circ_list_init(clist);

bail:
  NETMGR_LOG_FUNC_EXIT;
  return rc;
}

/*===========================================================================
  FUNCTION  netmgr_util_enq_circ_list
===========================================================================*/
/*!
@brief
  Enqueues data into the circular list

@return
  int - NETMGR_SUCCESS on operation success, NETMGR_FAILURE otherwise

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
int netmgr_util_enq_circ_list
(
  netmgr_util_circ_list_type  *clist,
  void                        *data
)
{
  int rc = NETMGR_FAILURE, num_free = 0;

  NETMGR_LOG_FUNC_ENTRY;

  if (!clist || !data)
  {
    netmgr_log_err("invalid input");
    goto bail;
  }

  if (clist->w >= clist->r)
  {
    num_free = NETMGR_UTIL_MAX_CIRC_LIST_SIZE - (clist->w - clist->r);
  }
  else
  {
    num_free = clist->r - clist->w - 1;
  }

  if (function_debug)
  {
    netmgr_log_med("r=%d, w=%d, free=%d",
                   clist->r, clist->w, num_free);
  }

  if (0 == num_free)
  {
    netmgr_log_err("no more free space to store data=%p", data);
    goto bail;
  }

  clist->data[clist->w++] = data;

  if (clist->w > NETMGR_UTIL_MAX_CIRC_LIST_SIZE)
  {
    clist->w = 0;
  }

  rc = NETMGR_SUCCESS;

bail:
  NETMGR_LOG_FUNC_EXIT;
  return rc;
}

/*===========================================================================
  FUNCTION  netmgr_util_deq_circ_list
===========================================================================*/
/*!
@brief
  Dequeues data from the circular list and returns it via the out pointer

@return
  int - NETMGR_SUCCESS on operation success, NETMGR_FAILURE otherwise

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
int netmgr_util_deq_circ_list
(
  netmgr_util_circ_list_type  *clist,
  void                        **data
)
{
  int rc = NETMGR_FAILURE, num_avail = 0;

  NETMGR_LOG_FUNC_ENTRY;

  if (!clist || !data)
  {
    netmgr_log_err("invalid input");
    goto bail;
  }

  if (clist->w >= clist->r)
  {
    num_avail = clist->w - clist->r;
  }
  else
  {
    num_avail = NETMGR_UTIL_MAX_CIRC_LIST_SIZE + 1 - clist->r + clist->w;
  }

  if (function_debug)
  {
    netmgr_log_med("r=%d, w=%d, avail=%d",
                   clist->r, clist->w, num_avail);
  }

  if (0 == num_avail)
  {
    netmgr_log_err("no more data available on list=%p", clist);
    goto bail;
  }

  *data = clist->data[clist->r++];

  if (clist->r > NETMGR_UTIL_MAX_CIRC_LIST_SIZE)
  {
    clist->r = 0;
  }

  rc = NETMGR_SUCCESS;

bail:
  NETMGR_LOG_FUNC_EXIT;
  return rc;
}
#endif /* FEATURE_DATA_IWLAN */
