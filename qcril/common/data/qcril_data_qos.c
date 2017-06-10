/*!
  @file
  qcril_data_qos.c

  @brief
  Handles RIL requests for DATA QOS services.

*/

/*===========================================================================

  Copyright (c) 2011-2013 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

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

/*===========================================================================

                        EDIT HISTORY FOR MODULE

This section contains comments describing changes made to the module.
Notice that changes are listed in reverse chronological order.

$Header: //linux/pkgs/proprietary/qc-ril/main/source/qcril_data.c#17 $

when       who     what, where, why
--------   ---     ----------------------------------------------------------
04/13/11   ar      Initial version

===========================================================================*/

/*===========================================================================

                           INCLUDE FILES

===========================================================================*/


#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <assert.h>
#include <unistd.h>
#include <errno.h>
#ifdef QCRIL_DATA_OFFTARGET
#include <netinet/in.h>
#endif
#include <net/if.h>
#include <linux/socket.h>
#include <arpa/inet.h>
#include <string.h>

#include "comdef.h"
#include "ril.h"
#include "qcrili.h"
#include "qcril_reqlist.h"
#include "qcril_data.h"
#include "qcril_data_defs.h"
#include "dsi_netctrl_qos.h"

#if (RIL_QCOM_VERSION >= 2)


/*===========================================================================

                   INTERNAL DEFINITIONS AND TYPES

===========================================================================*/

/* Error codes */
#define SUCCESS  (0)
#define FAILURE  (-1)
#define INVALID  (0xFFFFFFFF)

/* QOS spec processing constants */
#define DS_RIL_MIN_QOS_SETUP_PARAMS         (2)
#define DS_RIL_MIN_QOS_RELEASE_PARAMS       (1)
#define DS_RIL_MIN_QOS_MODIFY_PARAMS        (2)
#define DS_RIL_MIN_QOS_STATUS_PARAMS        (1)
#define DS_RIL_MIN_QOS_SUSPEND_PARAMS       (1)
#define DS_RIL_MIN_QOS_RESUME_PARAMS        (1)
#define DS_RIL_MAX_QOS_TX_FLOWS_PER_SPEC    (1)
#define DS_RIL_MAX_QOS_RX_FLOWS_PER_SPEC    (1)
#define DS_RIL_MAX_QOS_TX_FILTERS_PER_SPEC  (DSI_QOS_MAX_FLOW_FILTERS)
#define DS_RIL_MAX_QOS_RX_FILTERS_PER_SPEC  (DSI_QOS_MAX_FLOW_FILTERS)
#define DS_RIL_QOS_DIR_TX                   (RIL_QOS_TX)
#define DS_RIL_QOS_DIR_RX                   (RIL_QOS_RX)
#define DS_RIL_QOS_DIR_MAX                  (DS_RIL_QOS_DIR_RX)
#define DS_RIL_SPEC_ID_INVALID              (-1)
#define DS_RIL_FLOW_ID_INVALID              (-1)
#define DS_RIL_FLOW_ID_PRIMARY              (0)  /* Default flow ID */
#define DS_RIL_FILTER_ID_INVALID            (-1)

/* QOS spec string parsing items */
#define QCRIL_QOS_SPEC_DELIMITER     ","
#define QCRIL_QOS_KEY_DELIMITER      "="
#define QCRIL_QOS_SUBNET_DELIMITER   "/"
#define QCRIL_QOS_IPV4_DELIMITER     "."
#define QCRIL_QOS_IPV6_DELIMITER     ":"
#define QCRIL_QOS_IPV6_DELIMITER2    "::"
#define QCRIL_QOS_IPV6_DELIMITER2_LEN (2)
#define QCRIL_QOS_IPV6_ADDR_STR_LEN   (39)

#define QCRIL_QOS_ADDR_FAMILY_IPV4    (4)
#define QCRIL_QOS_ADDR_FAMILY_IPV6    (6)
#define QCRIL_QOS_ADDR_FAMILY_INVALID (INVALID)
#define QCRIL_QOS_ADDR_SUBNET_DEFAULT (0xFFFFFFFF)

#define QCRIL_QOS_ADDR_IPV4_MIN       (0x00)
#define QCRIL_QOS_ADDR_IPV4_MAX       (0xFF)
#define QCRIL_QOS_ADDR_IPV4_OFFSET_I  (24)    /* Offset initializer */
#define QCRIL_QOS_ADDR_IPV4_OFFSET_D  (8)     /* Offset decrement */
#define QCRIL_QOS_ADDR_IPV4_SHIFT_I   (32)    /* Shift initializer */

#define QCRIL_QOS_ADDR_IPV6_MIN       (0x00)
#define QCRIL_QOS_ADDR_IPV6_MAX       (0xFFFF)
#define QCRIL_QOS_ADDR_IPV6_ARRAY_M   (8)     /* Array maximum */
#define QCRIL_QOS_ADDR_IPV6_PFX_LEN   (64)

#define QCRIL_QOS_FLOW_STR_SIZE       (2048)

typedef struct parsing_track_s
{
  boolean       valid;          /* Valid element */
  unsigned int  id;             /* Current array element */
} qcril_parsing_track_s_type;

struct parsing_hndlr_s;

typedef struct parsing_context_s
{
  boolean                    dirty;         /* Flag for init */
  unsigned int               spec_id;       /* Current QOS spec array element */
  unsigned int               prev_spec_id;  /* Previous QOS spec array element */
  unsigned int               spec_cnt;      /* Maximum spec expected; limit check */
  unsigned int               direction;     /* Current QOS Flow/Filter direction */
  qcril_parsing_track_s_type tx_flow;       /* TX flow tracking */
  qcril_parsing_track_s_type rx_flow;       /* RX flow tracking */
  qcril_parsing_track_s_type tx_filter;     /* TX flow tracking */
  qcril_parsing_track_s_type rx_filter;     /* RX flow tracking */
  struct parsing_hndlr_s    *parse_ptr;     /* Parse callback table pointer */
} qcril_parsing_context_s_type;

typedef int (*parsing_handler_cb)
(
  qcril_parsing_context_s_type     *context_ptr,
  const char                       *value,
  dsi_qos_spec_type                *qos_spec_ptr
);

typedef struct parsing_hndlr_s
{
  RIL_QosSpecKeys     key;
  char               *key_name;
  parsing_handler_cb  handler;
} qcril_parsing_hndlr_s_type;

#define QCRIL_DECALRE_QOS_KEY_HNDLR(cb_name)                        \
  static int cb_name( qcril_parsing_context_s_type *context_ptr,    \
                      const char                   *value,          \
                      dsi_qos_spec_type            *qos_spec_ptr);


#define QCRIL_VERIFY_CONTEXT_VALID( ptr, label )                    \
  if( ptr && ptr->parse_ptr ) {                                     \
    if( (INVALID == ptr->spec_id) ||                                \
        (INVALID == ptr->direction) ||                              \
        (INVALID == ptr->parse_ptr) ) {               \
      QCRIL_LOG_ERROR( "parsing context data invalid - "            \
                       "spec_id[%d] dir[%d] ptr[%p]",               \
                       ptr->spec_id, ptr->direction,                \
                       ptr->parse_ptr);                             \
      goto label;                                                   \
    }                                                               \
  } else {                                                          \
    QCRIL_LOG_ERROR( "%s", "NULL parsing pointers" );               \
    goto label;                                                     \
  }

#define QCRIL_CONVERT_BASE( str, dest, base, label )                \
  if( str ) {                                                       \
    errno = SUCCESS;                                                \
    dest = strtol( str, NULL, (base) );                             \
    if( SUCCESS != errno ) {                                        \
      QCRIL_LOG_ERROR( "failed on integer conversion: %s", str );   \
      goto label;                                                   \
    }                                                               \
  } else {                                                          \
    QCRIL_LOG_ERROR( "%S", "NULL string specified for conversion" );\
    goto label;                                                     \
  }

#define QCRIL_BASE_DEC (10)
#define QCRIL_BASE_HEX (16)
#define QCRIL_CONVERT_INT( str, dest, label )  QCRIL_CONVERT_BASE( (str), (dest), QCRIL_BASE_DEC, label )
#define QCRIL_CONVERT_HEX( str, dest, label )  QCRIL_CONVERT_BASE( (str), (dest), QCRIL_BASE_HEX, label )

#define QCRIL_DATA_GEN_FLOW_STRING(key_bm,mask,key,data)                      \
  if( (key_bm) & (mask) )                                                     \
  {                                                                           \
    parse_ptr = qcril_data_find_parse_rec_by_key( (key) );                    \
    if( parse_ptr ) {                                                         \
      len = snprintf( sptr, size, "%s=%lu,",                                  \
                      parse_ptr->key_name, (unsigned long int)data);          \
      size -= len;                                                            \
      sptr += len;                                                            \
    } else {                                                                  \
      QCRIL_LOG_ERROR( "failed on find_parse_rec_by_key[%s]", (key) );        \
      return ret;                                                             \
    }                                                                         \
  }


/* Translate between QMI and RIL QOS status values */
static RIL_QosStatus qcril_data_qos_status_map[] =
{
  (RIL_QosStatus)INVALID,   /* Undefined */
  RIL_QOS_STATUS_ACTIVATED, /* QMI_QOS_STATUS_ACTIVATED */
  RIL_QOS_STATUS_SUSPENDED, /* QMI_QOS_STATUS_SUSPENDED */
  RIL_QOS_STATUS_NONE       /* QMI_QOS_STATUS_GONE      */
};


/*===========================================================================

                    INTERNAL FUNCTION PROTOTYPES

===========================================================================*/
QCRIL_DECALRE_QOS_KEY_HNDLR( qcril_data_process_context );
QCRIL_DECALRE_QOS_KEY_HNDLR( qcril_data_process_traffic_class );
QCRIL_DECALRE_QOS_KEY_HNDLR( qcril_data_process_flow_datarate );
QCRIL_DECALRE_QOS_KEY_HNDLR( qcril_data_process_flow_latency );
QCRIL_DECALRE_QOS_KEY_HNDLR( qcril_data_process_flow_profileID );
QCRIL_DECALRE_QOS_KEY_HNDLR( qcril_data_process_flow_priority );
QCRIL_DECALRE_QOS_KEY_HNDLR( qcril_data_process_filter_address );
QCRIL_DECALRE_QOS_KEY_HNDLR( qcril_data_process_filter_version );
QCRIL_DECALRE_QOS_KEY_HNDLR( qcril_data_process_filter_tos );
QCRIL_DECALRE_QOS_KEY_HNDLR( qcril_data_process_filter_port );


/*===========================================================================

                         GLOBAL VARIABLES

===========================================================================*/

/* The following table is used to parse the QOS specification string
 * received from RIL API.  The spec string is made up of
 * <token>=<value> pairs, comma separated.  The parsing logic
 * operates within nested loops, will first tokenize based on comma,
 * then tokenize based on '='.  Lookup table is used to match on
 * target string, yeilding the handler callback invoked to actually
 * process the value string.  The output will be the DSI/QMI
 * data structure populated based on the input QOS specs string values.
 */

static qcril_parsing_hndlr_s_type qos_key_hndlr_tbl[] =
{
  /* RIL API key,                              Parser target string,                        Parsing handler callback */
  { RIL_QOS_SPEC_INDEX,                        "RIL_QOS_SPEC_INDEX",                        qcril_data_process_context },
  { RIL_QOS_FLOW_DIRECTION,                    "RIL_QOS_FLOW_DIRECTION",                    qcril_data_process_context },
  { RIL_QOS_FLOW_TRAFFIC_CLASS,                "RIL_QOS_FLOW_TRAFFIC_CLASS",                qcril_data_process_traffic_class },
  { RIL_QOS_FLOW_DATA_RATE_MIN,                "RIL_QOS_FLOW_DATA_RATE_MIN",                qcril_data_process_flow_datarate },
  { RIL_QOS_FLOW_DATA_RATE_MAX,                "RIL_QOS_FLOW_DATA_RATE_MAX",                qcril_data_process_flow_datarate },
  { RIL_QOS_FLOW_LATENCY,                      "RIL_QOS_FLOW_LATENCY",                      qcril_data_process_flow_latency },
  { RIL_QOS_FLOW_3GPP2_PROFILE_ID,             "RIL_QOS_FLOW_3GPP2_PROFILE_ID",             qcril_data_process_flow_profileID },
  { RIL_QOS_FLOW_3GPP2_PRIORITY,               "RIL_QOS_FLOW_3GPP2_PRIORITY",               qcril_data_process_flow_priority },
  { RIL_QOS_FILTER_INDEX,                      "RIL_QOS_FILTER_INDEX",                      qcril_data_process_context },
  { RIL_QOS_FILTER_DIRECTION,                  "RIL_QOS_FILTER_DIRECTION",                  qcril_data_process_context },
  { RIL_QOS_FILTER_IPVERSION,                  "RIL_QOS_FILTER_IPVERSION",                  qcril_data_process_filter_version },
  { RIL_QOS_FILTER_IPV4_SOURCE_ADDR,           "RIL_QOS_FILTER_IPV4_SOURCE_ADDR",           qcril_data_process_filter_address },
  { RIL_QOS_FILTER_IPV4_DESTINATION_ADDR,      "RIL_QOS_FILTER_IPV4_DESTINATION_ADDR",      qcril_data_process_filter_address },
  { RIL_QOS_FILTER_IPV4_TOS,                   "RIL_QOS_FILTER_IPV4_TOS",                   qcril_data_process_filter_tos },
  { RIL_QOS_FILTER_IPV4_TOS_MASK,              "RIL_QOS_FILTER_IPV4_TOS_MASK",              qcril_data_process_filter_tos },
  { RIL_QOS_FILTER_TCP_SOURCE_PORT_START,      "RIL_QOS_FILTER_TCP_SOURCE_PORT_START",      qcril_data_process_filter_port },
  { RIL_QOS_FILTER_TCP_SOURCE_PORT_RANGE,      "RIL_QOS_FILTER_TCP_SOURCE_PORT_RANGE",      qcril_data_process_filter_port },
  { RIL_QOS_FILTER_TCP_DESTINATION_PORT_START, "RIL_QOS_FILTER_TCP_DESTINATION_PORT_START", qcril_data_process_filter_port },
  { RIL_QOS_FILTER_TCP_DESTINATION_PORT_RANGE, "RIL_QOS_FILTER_TCP_DESTINATION_PORT_RANGE", qcril_data_process_filter_port },
  { RIL_QOS_FILTER_UDP_SOURCE_PORT_START,      "RIL_QOS_FILTER_UDP_SOURCE_PORT_START",      qcril_data_process_filter_port },
  { RIL_QOS_FILTER_UDP_SOURCE_PORT_RANGE,      "RIL_QOS_FILTER_UDP_SOURCE_PORT_RANGE",      qcril_data_process_filter_port },
  { RIL_QOS_FILTER_UDP_DESTINATION_PORT_START, "RIL_QOS_FILTER_UDP_DESTINATION_PORT_START", qcril_data_process_filter_port },
  { RIL_QOS_FILTER_UDP_DESTINATION_PORT_RANGE, "RIL_QOS_FILTER_UDP_DESTINATION_PORT_RANGE", qcril_data_process_filter_port },
  { RIL_QOS_FILTER_IPV6_SOURCE_ADDR,           "RIL_QOS_FILTER_IPV6_SOURCE_ADDR",           qcril_data_process_filter_address },
  { RIL_QOS_FILTER_IPV6_DESTINATION_ADDR,      "RIL_QOS_FILTER_IPV6_DESTINATION_ADDR",      qcril_data_process_filter_address },
  { RIL_QOS_FILTER_IPV6_TRAFFIC_CLASS,         "RIL_QOS_FILTER_IPV6_TRAFFIC_CLASS",         NULL },
  { RIL_QOS_FILTER_IPV6_FLOW_LABEL,            "RIL_QOS_FILTER_IPV6_FLOW_LABEL",            NULL },
#if 0 // Not supported in RIL API yet
  { RIL_QOS_FILTER_IP_NEXT_HEADER_PROTOCOL,    "RIL_QOS_FILTER_IP_NEXT_HEADER_PROTOCOL",    NULL },
#endif
};

#define QCRIL_KEY_HNDLR_TBL_SIZE (sizeof(qos_key_hndlr_tbl)/sizeof(qos_key_hndlr_tbl[0]))



/*===========================================================================

                                FUNCTIONS

===========================================================================*/

#if 0 /* Not needed at this time */
/*===========================================================================

  FUNCTION: qcril_data_addr_ntoh_str

===========================================================================*/
/*!
    @brief
    Swap the byte oder for an arbitrary long array of octets.
    Typically used for IPV6 address buffer, like ntoh().

    @pre

    @return
    SUCCESS
    FAILURE
*/
/*=========================================================================*/
static int qcril_data_addr_ntoh_str
(
  const qcril_data_addr6_t * in_ptr,
  unsigned char            * out_ptr,
  unsigned int               array_len
)
{
  int i,j,offset=0;
  qcril_data_addr6_t temp;
  int ret = SUCCESS;

  /* Validate input parameters */
  if( !in_ptr || !out_ptr || !array_len ) {
    QCRIL_LOG_ERROR( "%s", "invalid input" );
    return FAILURE;
  }
  if( array_len < 16 ) {
    QCRIL_LOG_ERROR( "%s", "output array too small" );
    return FAILURE;
  }

  /* Swap octets front-to-back */
  for( i=1; i>=0; i-- ) {
    for( j=0; j<8; j++ ) {
      out_ptr[offset++] = ( (in_ptr->in6_u.u6_addr64[i]>>(j*8)) & 0xFF );
    }
  }

  return ret;
} /* qcril_data_addr_ntoh_str() */
#endif /* 0 */

/*===========================================================================

  FUNCTION: qcril_data_qos_match_flow

===========================================================================*/
/*!
    @brief
    Callback funciton to match flow objects on list search

    @pre

    @return
    TRUE
    FALSE
*/
/*=========================================================================*/
static int qcril_data_qos_match_flow
(
  void* item_ptr,
  void* compare_val
)
{
  int result = ((qcril_data_qos_state_type*)item_ptr)->flow_id =
               (dsi_qos_id_type)compare_val;
  return result;
} /* qcril_data_qos_match_flow() */


/*===========================================================================

  FUNCTION: qcril_data_qos_add_flows

===========================================================================*/
/*!
    @brief
    Add list for QOS flows to the specified call object

    @pre

    @return
    SUCCESS
    FAILURE
*/
/*=========================================================================*/
static int qcril_data_qos_add_flows
(
  qcril_data_call_info_tbl_type  *info_tbl_ptr,
  const dsi_qos_id_type          *qos_flow_id_arr,
  unsigned int                    num_flows
)
{
  qcril_data_qos_state_type  *flow_ptr = NULL;
  int           ret = FAILURE;
  unsigned int  i;

  QCRIL_LOG_ENTRY;

  /* Loop over array of flow IDs */
  for( i=0; i<num_flows; i++ )
  {
    /* Allocate new QOS flow object */
    flow_ptr = malloc( sizeof(qcril_data_qos_state_type) );
    if( !flow_ptr )
    {
      QCRIL_LOG_ERROR( "%s", "failed to allocate QOS flow" );
      goto err_label;
    }
    flow_ptr->flow_id = qos_flow_id_arr[i];

    /* Append to list of QOS flows for call */
    list_push_back( &info_tbl_ptr->qos_flow_list, &flow_ptr->link );
    QCRIL_LOG_DEBUG( "added flow[0x%08x] to cid[%d], num_flows[%d]",
                     qos_flow_id_arr[i], info_tbl_ptr->cid,
                     list_size(&info_tbl_ptr->qos_flow_list) );
  }
  ret = SUCCESS;

 err_label:
  QCRIL_LOG_EXIT;
  return ret;
} /* qcril_data_qos_add_flows() */


/*===========================================================================

  FUNCTION: qcril_data_qos_del_flows

===========================================================================*/
/*!
    @brief
    Delete list for QOS flows to the specified call object

    @pre

    @return
    SUCCESS
    FAILURE
*/
/*=========================================================================*/
static int qcril_data_qos_del_flows
(
  qcril_data_call_info_tbl_type  *info_tbl_ptr,
  const dsi_qos_id_type          *qos_flow_id_arr,
  unsigned int                    num_flows
)
{
  qcril_data_qos_state_type  *flow_ptr = NULL;
  int           ret = FAILURE;
  unsigned int  i;

  QCRIL_LOG_ENTRY;

  /* Loop over array of flow IDs */
  for( i=0; i<num_flows; i++ )
  {
    flow_ptr = (qcril_data_qos_state_type*)list_linear_search( &info_tbl_ptr->qos_flow_list,
                                                               qcril_data_qos_match_flow,
                                                               (void*)qos_flow_id_arr[i] );
    if( !flow_ptr )
    {
      QCRIL_LOG_ERROR( "%s", "failed to find QOS flow" );
      goto err_label;
    }

    /* Release QOS flow object */
    list_pop_item( &info_tbl_ptr->qos_flow_list, &flow_ptr->link );
    free( flow_ptr );
    QCRIL_LOG_DEBUG( "deleted flow[0x%08x] to cid[%d]",
                     qos_flow_id_arr[i], info_tbl_ptr->cid );
  }
  ret = SUCCESS;

 err_label:
  QCRIL_LOG_EXIT;
  return ret;
} /* qcril_data_qos_del_flows() */


/*===========================================================================

  FUNCTION: qcril_data_find_flowid

===========================================================================*/
/*!
    @brief
    Find the specified QOS flow in call object list.

    @pre

    @return
    Pointer to QOS flow object if found
    NULL otherwise
*/
/*=========================================================================*/
static qcril_data_qos_state_type* qcril_data_qos_find_flow
(
  qcril_data_call_info_tbl_type  *info_tbl_ptr,
  dsi_qos_id_type                 qos_flowid
)
{
  qcril_data_qos_state_type * flow_ptr = NULL;

  /* Check for valid call */
  if( VALIDATE_LOCAL_DATA_OBJ( info_tbl_ptr ) )
  {
    /* Search list of flow objects */
    flow_ptr = (qcril_data_qos_state_type*)list_linear_search( &info_tbl_ptr->qos_flow_list,
                                                               qcril_data_qos_match_flow,
                                                               (void*)qos_flowid );
    if( flow_ptr )
    {
      QCRIL_LOG_DEBUG( "found matching QOS ID [0x%08x], CID [%d]",
                       qos_flowid, info_tbl_ptr->cid );
    }
  }

  return flow_ptr;
} /* qcril_data_qos_find_flow() */

/*===========================================================================

  FUNCTION: qcril_data_find_call_by_flowid

===========================================================================*/
/*!
    @brief
    Loop over call table to find the specified QOS flow ID.

    @pre

    @return
    Pointer to call record if found
    NULL otherwise
*/
/*=========================================================================*/
static qcril_data_call_info_tbl_type* qcril_data_find_call_by_flowid
(
  dsi_qos_id_type  qos_flowid
)
{
  qcril_data_call_info_tbl_type *call_ptr = NULL;
  qcril_data_qos_state_type *flow_ptr = NULL;
  int        ret = FAILURE;
  int        i;

  QCRIL_LOG_ENTRY;

  /* Loop over call table */
  for( i = 0; i < MAX_CONCURRENT_UMTS_DATA_CALLS; i++ )
  {
    flow_ptr = qcril_data_qos_find_flow( &info_tbl[i], qos_flowid );
    if( flow_ptr )
    {
      call_ptr = &info_tbl[i];
      ret = SUCCESS;
      break;
    }
  }/* for(i) */

  QCRIL_LOG_EXIT;
  return call_ptr;
} /* qcril_data_find_call_by_flowid() */


/*===========================================================================

  FUNCTION: qcril_data_find_parse_rec_by_name

===========================================================================*/
/*!
    @brief
    Loop over parse table to find the specified paramater name.

    @pre

    @return
    Pointer to parse record if found
    NULL otherwise
*/
/*=========================================================================*/
static qcril_parsing_hndlr_s_type* qcril_data_find_parse_rec_by_name
(
  const char * key_name
)
{
  unsigned int i;

  for( i = 0; i < QCRIL_KEY_HNDLR_TBL_SIZE; i++ )
  {
    /* Compare key name */
    if( !strcmp( key_name, qos_key_hndlr_tbl[i].key_name) )
    {
      return &qos_key_hndlr_tbl[i];
    }
  }
  return NULL;
} /* qcril_data_find_parse_rec_by_name() */


/*===========================================================================

  FUNCTION: qcril_data_find_parse_rec_by_key

===========================================================================*/
/*!
    @brief
    Loop over parse table to find the specified paramater key.

    @pre

    @return
    Pointer to parse record if found
    NULL otherwise
*/
/*=========================================================================*/
static qcril_parsing_hndlr_s_type* qcril_data_find_parse_rec_by_key
(
  RIL_QosSpecKeys  key
)
{
  unsigned int i;

  for( i = 0; i < QCRIL_KEY_HNDLR_TBL_SIZE; i++ )
  {
    /* Compare key value */
    if( key == qos_key_hndlr_tbl[i].key )
    {
      return &qos_key_hndlr_tbl[i];
    }
  }
  return NULL;
} /* qcril_data_find_parse_rec_by_key() */


/*===========================================================================

  FUNCTION: qcril_data_update_qos_spec_counts

===========================================================================*/
/*!
    @brief
    Update the QOS spec structure based on parsing context.

    @pre

    @return
    SUCCESS
    FAILURE
*/
/*=========================================================================*/
static int qcril_data_update_qos_spec_counts
(
  qcril_parsing_context_s_type *context_ptr,
  dsi_qos_spec_type            *qos_spec_ptr
)
{
  int ret = SUCCESS;

  QCRIL_LOG_ENTRY;

  /* New QOS spec, update flow info */
  qos_spec_ptr[context_ptr->spec_id].num_tx_flow_req += (context_ptr->tx_flow.valid)? 1 : 0;
  qos_spec_ptr[context_ptr->spec_id].num_rx_flow_req += (context_ptr->rx_flow.valid)? 1 : 0;

  /* Reset parsing flags for next QOS spec */
  context_ptr->tx_flow.valid   = FALSE;
  context_ptr->rx_flow.valid   = FALSE;

  QCRIL_LOG_EXIT;
  return ret;
} /* qcril_data_update_qos_spec_counts() */


/*===========================================================================

  FUNCTION: qcril_data_process_context

===========================================================================*/
/*!
    @brief
    Process the QOS key=value pair for spec index and direction.

    The context values dictate the direction (TX/RX) and flow/filter
    index within the DSI/QMI data structure to be populated by later
    parsing operations.

    @pre

    @return
    SUCCESS
    FAILURE
*/
/*=========================================================================*/
static int qcril_data_process_context
(
  qcril_parsing_context_s_type *context_ptr,
  const char                   *value,
  dsi_qos_spec_type            *qos_spec_ptr
)
{
  int        ret = FAILURE;
  long int   temp;

  QCRIL_LOG_ENTRY;

  if( !(context_ptr && context_ptr->parse_ptr &&
        ( NULL != context_ptr->parse_ptr)) )
  {
    QCRIL_LOG_ERROR( "%s", "parsing context data invalid" );
    goto err_label;
  }

  QCRIL_CONVERT_INT( value, temp, err_label );

  /* Update parsing context for current QOS spec */
  switch( ((qcril_parsing_hndlr_s_type*)context_ptr->parse_ptr)->key )
  {
    case RIL_QOS_SPEC_INDEX:
      /* Check max number of flow specs per request */
      if( MIN(DS_RIL_MAX_QOS_SPECS_PER_REQ, context_ptr->spec_cnt) < (unsigned)temp )
      {
        QCRIL_LOG_ERROR( "QOS spec index[%d] exceeds supported limit[%d]",
                         temp, (MIN(DS_RIL_MAX_QOS_SPECS_PER_REQ, context_ptr->spec_cnt)-1) );
        goto err_label;
      }
      context_ptr->spec_id = temp;
      QCRIL_LOG_DEBUG( "Decode SPEC_ID: %d", context_ptr->spec_id );

      if( context_ptr->dirty && context_ptr->spec_id != context_ptr->prev_spec_id )
      {
        /* New QOS spec, update QMI spec, filter & flow flags */
        qcril_data_update_qos_spec_counts( context_ptr, qos_spec_ptr );

        context_ptr->prev_spec_id = context_ptr->spec_id++;
      }
      else
      {
        context_ptr->prev_spec_id = context_ptr->spec_id;
        context_ptr->dirty = TRUE;
      }
      break;

    case RIL_QOS_FILTER_INDEX:
      /* Check max number of filters/flow (same TX & RX limit) */
      if( DS_RIL_MAX_QOS_TX_FILTERS_PER_SPEC < (unsigned)temp )
      {
        QCRIL_LOG_ERROR( "QOS filter index[%d] exceeds supported limit[%d]",
                         temp, (DS_RIL_MAX_QOS_TX_FILTERS_PER_SPEC-1) );
        goto err_label;
      }
      switch( context_ptr->direction ) {
        case DS_RIL_QOS_DIR_TX:
          context_ptr->tx_filter.id = temp;
          qos_spec_ptr[context_ptr->spec_id].num_tx_filter_req++;
          break;

        case DS_RIL_QOS_DIR_RX:
          context_ptr->rx_filter.id = temp;
          qos_spec_ptr[context_ptr->spec_id].num_rx_filter_req++;
          break;

        default:
          QCRIL_LOG_ERROR( "%s","QOS direction undefined, aborting" );
          goto err_label;
      }
      QCRIL_LOG_DEBUG( "Decode FILTER_ID: %d", temp );
      break;

    case RIL_QOS_FLOW_DIRECTION:
    case RIL_QOS_FILTER_DIRECTION:
      if( DS_RIL_QOS_DIR_MAX < temp )
      {
        QCRIL_LOG_ERROR( "QOS direction[%d] exceeds supported limit[%d]",
                         temp, DS_RIL_QOS_DIR_MAX );
        goto err_label;
      }
      context_ptr->direction = temp;
      QCRIL_LOG_DEBUG( "Decode DIRECTION: %d", context_ptr->direction );
      break;

    default:
      QCRIL_LOG_ERROR( "unsupported key[%s] for value[%s]",
                       ((qcril_parsing_hndlr_s_type*)context_ptr->parse_ptr)->key_name, value );
      goto err_label;
  }
  ret = SUCCESS;

 err_label:
  QCRIL_LOG_EXIT;
  return ret;
} /* qcril_data_process_context() */


/*===========================================================================

  FUNCTION: qcril_data_process_traffic_class

===========================================================================*/
/*!
    @brief
    Process the QOS key=value pair for flow traffic class

    @pre

    @return
    SUCCESS
    FAILURE
*/
/*=========================================================================*/
static int qcril_data_process_traffic_class
(
  qcril_parsing_context_s_type *context_ptr,
  const char                   *value,
  dsi_qos_spec_type            *qos_spec_ptr
)
{
  int        ret = FAILURE;
  long int   temp;
  QCRIL_LOG_ENTRY;

  QCRIL_VERIFY_CONTEXT_VALID( context_ptr, err_label );

  QCRIL_CONVERT_INT( value, temp, err_label );

  /* Update parsing context for current QOS spec */
  switch( context_ptr->direction )
  {
    case DS_RIL_QOS_DIR_TX:
      assert(DS_RIL_FLOW_ID_INVALID != context_ptr->tx_flow.id);
      qos_spec_ptr[context_ptr->spec_id].tx_flow_req_array[context_ptr->tx_flow.id].
        umts_flow_desc.traffic_class = (dsi_qos_umts_traffic_class_type)temp;
      qos_spec_ptr[context_ptr->spec_id].tx_flow_req_array[context_ptr->tx_flow.id].
        umts_flow_desc.param_mask |= QMI_QOS_UMTS_FLOW_PARAM_TRAFFIC_CLASS;
      QCRIL_LOG_VERBOSE( "QOS spec[%d] Tx flow[%d]  - traffic_class[%d]",
                         context_ptr->spec_id, context_ptr->tx_flow.id, temp);
      context_ptr->tx_flow.valid = TRUE;
      break;

    case DS_RIL_QOS_DIR_RX:
      assert(DS_RIL_FLOW_ID_INVALID != context_ptr->rx_flow.id);
      qos_spec_ptr[context_ptr->spec_id].rx_flow_req_array[context_ptr->rx_flow.id].
        umts_flow_desc.traffic_class = (dsi_qos_umts_traffic_class_type)temp;
      qos_spec_ptr[context_ptr->spec_id].rx_flow_req_array[context_ptr->rx_flow.id].
        umts_flow_desc.param_mask |= QMI_QOS_UMTS_FLOW_PARAM_TRAFFIC_CLASS;
      QCRIL_LOG_VERBOSE( "QOS spec[%d] Rx flow[%d]  - traffic_class[%d]",
                         context_ptr->spec_id, context_ptr->rx_flow.id, temp);
      context_ptr->rx_flow.valid = TRUE;
      break;

    default:
      QCRIL_LOG_ERROR( "unsupported direction[%d] value[%s] for key[%s] ",
                       context_ptr->direction, value,
                       ((qcril_parsing_hndlr_s_type*)context_ptr->parse_ptr)->key_name );
      goto err_label;
  }
  ret = SUCCESS;

 err_label:
  QCRIL_LOG_EXIT;
  return ret;
} /* qcril_data_process_traffic_class() */

/*===========================================================================

  FUNCTION: qcril_data_process_flow_datarate

===========================================================================*/
/*!
    @brief
    Process the QOS key=value pair for flow traffic class

    @pre

    @return
    SUCCESS
    FAILURE
*/
/*=========================================================================*/
static int qcril_data_process_flow_datarate
(
  qcril_parsing_context_s_type *context_ptr,
  const char                   *value,
  dsi_qos_spec_type            *qos_spec_ptr
)
{
  int        ret = FAILURE;
  long int   temp;

  QCRIL_LOG_ENTRY;

  QCRIL_VERIFY_CONTEXT_VALID( context_ptr, err_label );

  QCRIL_CONVERT_INT( value, temp, err_label );

  /* Update parsing context for current QOS spec */
  switch( context_ptr->direction )
  {
    case DS_RIL_QOS_DIR_TX:
      assert(DS_RIL_FLOW_ID_INVALID != context_ptr->tx_flow.id);
      if( RIL_QOS_FLOW_DATA_RATE_MIN ==
          ((qcril_parsing_hndlr_s_type*)context_ptr->parse_ptr)->key ) {
        qos_spec_ptr[context_ptr->spec_id].tx_flow_req_array[context_ptr->tx_flow.id].
          umts_flow_desc.data_rate.guaranteed_rate = temp;
        QCRIL_LOG_VERBOSE( "QOS spec[%d] Tx flow[%d] - minimum datarate[%d]",
                           context_ptr->spec_id, context_ptr->tx_flow.id, temp);
      } else {
        qos_spec_ptr[context_ptr->spec_id].tx_flow_req_array[context_ptr->tx_flow.id].
          umts_flow_desc.data_rate.max_rate = temp;
        QCRIL_LOG_VERBOSE( "QOS spec[%d] Tx flow[%d] - maximum datarate[%d]",
                           context_ptr->spec_id, context_ptr->tx_flow.id, temp);
      }
      qos_spec_ptr[context_ptr->spec_id].tx_flow_req_array[context_ptr->tx_flow.id].
        umts_flow_desc.param_mask |= QMI_QOS_UMTS_FLOW_PARAM_DATA_RATE;
      context_ptr->tx_flow.valid = TRUE;
      break;

    case DS_RIL_QOS_DIR_RX:
      assert(DS_RIL_FLOW_ID_INVALID != context_ptr->rx_flow.id);
      if( RIL_QOS_FLOW_DATA_RATE_MIN ==
          ((qcril_parsing_hndlr_s_type*)context_ptr->parse_ptr)->key ) {
        qos_spec_ptr[context_ptr->spec_id].rx_flow_req_array[context_ptr->rx_flow.id].
          umts_flow_desc.data_rate.guaranteed_rate = temp;
        QCRIL_LOG_VERBOSE( "QOS spec[%d] Rx flow[%d] - minimum datarate[%d]",
                           context_ptr->spec_id, context_ptr->rx_flow.id, temp);
      } else {
        qos_spec_ptr[context_ptr->spec_id].rx_flow_req_array[context_ptr->rx_flow.id].
          umts_flow_desc.data_rate.max_rate = temp;
        QCRIL_LOG_VERBOSE( "QOS spec[%d] Rx flow[%d] - maximum datarate[%d]",
                           context_ptr->spec_id, context_ptr->rx_flow.id, temp);
      }
      qos_spec_ptr[context_ptr->spec_id].rx_flow_req_array[context_ptr->rx_flow.id].
        umts_flow_desc.param_mask |= QMI_QOS_UMTS_FLOW_PARAM_DATA_RATE;
      context_ptr->rx_flow.valid = TRUE;
      break;

    default:
      QCRIL_LOG_ERROR( "unsupported direction[%d] value[%s] for key[%s] ",
                       context_ptr->direction, value,
                       ((qcril_parsing_hndlr_s_type*)context_ptr->parse_ptr)->key_name );
      goto err_label;
  }
  ret = SUCCESS;

 err_label:
  QCRIL_LOG_EXIT;
  return ret;
} /* qcril_data_process_flow_datarate() */


/*===========================================================================

  FUNCTION: qcril_data_process_flow_latency

===========================================================================*/
/*!
    @brief
    Process the QOS key=value pair for flow latency

    @pre

    @return
    SUCCESS
    FAILURE
*/
/*=========================================================================*/
static int qcril_data_process_flow_latency
(
  qcril_parsing_context_s_type *context_ptr,
  const char                   *value,
  dsi_qos_spec_type            *qos_spec_ptr
)
{
  int        ret = FAILURE;
  long int   temp;
  QCRIL_LOG_ENTRY;

  QCRIL_VERIFY_CONTEXT_VALID( context_ptr, err_label );

  QCRIL_CONVERT_INT( value, temp, err_label );

  /* Update parsing context for current QOS spec */
  switch( context_ptr->direction )
  {
    case DS_RIL_QOS_DIR_TX:
      assert(DS_RIL_FLOW_ID_INVALID != context_ptr->tx_flow.id);
      qos_spec_ptr[context_ptr->spec_id].tx_flow_req_array[context_ptr->tx_flow.id].
        umts_flow_desc.max_delay = temp;
      qos_spec_ptr[context_ptr->spec_id].tx_flow_req_array[context_ptr->tx_flow.id].
        umts_flow_desc.param_mask |= QMI_QOS_UMTS_FLOW_PARAM_MAX_DELAY;
      QCRIL_LOG_VERBOSE( "QOS spec[%d] Tx flow[%d]  - latency[%d]",
                         context_ptr->spec_id, context_ptr->tx_flow.id, temp);
      context_ptr->tx_flow.valid = TRUE;
      break;

    case DS_RIL_QOS_DIR_RX:
      assert(DS_RIL_FLOW_ID_INVALID != context_ptr->rx_flow.id);
      qos_spec_ptr[context_ptr->spec_id].rx_flow_req_array[context_ptr->rx_flow.id].
        umts_flow_desc.max_delay = temp;
      qos_spec_ptr[context_ptr->spec_id].rx_flow_req_array[context_ptr->rx_flow.id].
        umts_flow_desc.param_mask |= QMI_QOS_UMTS_FLOW_PARAM_MAX_DELAY;
      QCRIL_LOG_VERBOSE( "QOS spec[%d] Rx flow[%d]  - latency[%d]",
                         context_ptr->spec_id, context_ptr->rx_flow.id, temp);
      context_ptr->rx_flow.valid = TRUE;
      break;

    default:
      QCRIL_LOG_ERROR( "unsupported direction[%d] value[%s] for key[%s] ",
                       context_ptr->direction, value,
                       ((qcril_parsing_hndlr_s_type*)context_ptr->parse_ptr)->key_name );
      goto err_label;
  }
  ret = SUCCESS;

 err_label:
  QCRIL_LOG_EXIT;
  return ret;
} /* qcril_data_process_flow_latency() */


/*===========================================================================

  FUNCTION: qcril_data_process_flow_profileID

===========================================================================*/
/*!
    @brief
    Process the QOS key=value pair for flow 3GPP2 profile ID

    @pre

    @return
    SUCCESS
    FAILURE
*/
/*=========================================================================*/
static int qcril_data_process_flow_profileID
(
  qcril_parsing_context_s_type *context_ptr,
  const char                   *value,
  dsi_qos_spec_type            *qos_spec_ptr
)
{
  int        ret = FAILURE;
  long int   temp;
  QCRIL_LOG_ENTRY;

  QCRIL_VERIFY_CONTEXT_VALID( context_ptr, err_label );

  QCRIL_CONVERT_INT( value, temp, err_label );

  /* Update parsing context for current QOS spec */
  switch( context_ptr->direction )
  {
    case DS_RIL_QOS_DIR_TX:
      assert(DS_RIL_FLOW_ID_INVALID != context_ptr->tx_flow.id);
      qos_spec_ptr[context_ptr->spec_id].tx_flow_req_array[context_ptr->tx_flow.id].
        cdma_flow_desc.profile_id = temp;
      qos_spec_ptr[context_ptr->spec_id].tx_flow_req_array[context_ptr->tx_flow.id].
        cdma_flow_desc.param_mask |= QMI_QOS_CDMA_FLOW_PARAM_PROFILE_ID;
      QCRIL_LOG_VERBOSE( "QOS spec[%d] Tx flow[%d]  - profileID[%d]",
                         context_ptr->spec_id, context_ptr->tx_flow.id, temp);
      context_ptr->tx_flow.valid = TRUE;
      break;

    case DS_RIL_QOS_DIR_RX:
      assert(DS_RIL_FLOW_ID_INVALID != context_ptr->rx_flow.id);
      qos_spec_ptr[context_ptr->spec_id].rx_flow_req_array[context_ptr->rx_flow.id].
        cdma_flow_desc.profile_id = temp;
      qos_spec_ptr[context_ptr->spec_id].rx_flow_req_array[context_ptr->rx_flow.id].
        cdma_flow_desc.param_mask |= QMI_QOS_CDMA_FLOW_PARAM_PROFILE_ID;
      QCRIL_LOG_VERBOSE( "QOS spec[%d] Rx flow spec[%d] - profileID[%d]",
                         context_ptr->spec_id, context_ptr->rx_flow.id, temp);
      context_ptr->rx_flow.valid = TRUE;
      break;

    default:
      QCRIL_LOG_ERROR( "unsupported direction[%d] value[%s] for key[%s] ",
                       context_ptr->direction, value,
                       ((qcril_parsing_hndlr_s_type*)context_ptr->parse_ptr)->key_name );
      goto err_label;
  }
  ret = SUCCESS;

 err_label:
  QCRIL_LOG_EXIT;
  return ret;
} /* qcril_data_process_flow_profileID() */


/*===========================================================================

  FUNCTION: qcril_data_process_flow_priority

===========================================================================*/
/*!
    @brief
    Process the QOS key=value pair for flow priority

    @pre

    @return
    SUCCESS
    FAILURE
*/
/*=========================================================================*/
static int qcril_data_process_flow_priority
(
  qcril_parsing_context_s_type *context_ptr,
  const char                   *value,
  dsi_qos_spec_type            *qos_spec_ptr
)
{
  int        ret = FAILURE;
  long int   temp;
  QCRIL_LOG_ENTRY;

  QCRIL_VERIFY_CONTEXT_VALID( context_ptr, err_label );

  QCRIL_CONVERT_INT( value, temp, err_label );

  /* Update parsing context for current QOS spec */
  switch( context_ptr->direction )
  {
    case DS_RIL_QOS_DIR_TX:
      assert(DS_RIL_FLOW_ID_INVALID != context_ptr->tx_flow.id);
      qos_spec_ptr[context_ptr->spec_id].tx_flow_req_array[context_ptr->tx_flow.id].
        umts_flow_desc.flow_priority_3gpp2 = temp;
      qos_spec_ptr[context_ptr->spec_id].tx_flow_req_array[context_ptr->tx_flow.id].
        umts_flow_desc.param_mask |= QMI_QOS_UMTS_FLOW_PARAM_3GPP2_FLOW_PRIO;
      QCRIL_LOG_VERBOSE( "QOS spec[%d] Tx flow[%d] - priority[%d]",
                         context_ptr->spec_id, context_ptr->tx_flow.id, temp);
      context_ptr->tx_flow.valid = TRUE;
      break;

    case DS_RIL_QOS_DIR_RX:
      assert(DS_RIL_FLOW_ID_INVALID != context_ptr->rx_flow.id);
      qos_spec_ptr[context_ptr->spec_id].rx_flow_req_array[context_ptr->rx_flow.id].
        umts_flow_desc.flow_priority_3gpp2 = temp;
      qos_spec_ptr[context_ptr->spec_id].rx_flow_req_array[context_ptr->rx_flow.id].
        umts_flow_desc.param_mask |= QMI_QOS_UMTS_FLOW_PARAM_3GPP2_FLOW_PRIO;
      QCRIL_LOG_VERBOSE( "QOS spec[%d] Rx flow[%d] - priority[%d]",
                         context_ptr->spec_id, context_ptr->rx_flow.id, temp);
      context_ptr->rx_flow.valid = TRUE;
      break;

    default:
      QCRIL_LOG_ERROR( "unsupported direction[%d] value[%s] for key[%s] ",
                       context_ptr->direction, value,
                       ((qcril_parsing_hndlr_s_type*)context_ptr->parse_ptr)->key_name );
      goto err_label;
  }
  ret = SUCCESS;

 err_label:
  QCRIL_LOG_EXIT;
  return ret;
} /* qcril_data_process_flow_priority() */


/*===========================================================================

  FUNCTION: qcril_data_process_filter_version

===========================================================================*/
/*!
    @brief
    Process the QOS key=value pair for filter IP version

    @pre

    @return
    SUCCESS
    FAILURE
*/
/*=========================================================================*/
static int qcril_data_process_filter_version
(
  qcril_parsing_context_s_type *context_ptr,
  const char                   *value,
  dsi_qos_spec_type            *qos_spec_ptr
)
{
  int        ret = FAILURE;
  long int   temp = 0;
  QCRIL_LOG_ENTRY;

  QCRIL_VERIFY_CONTEXT_VALID( context_ptr, err_label );

  /* Convert IP family to numeric */
  if( !strncmp( QCRIL_DATA_IP_FAMILY_V6, value, sizeof(QCRIL_DATA_IP_FAMILY_V6) ) ) {
    temp = QCRIL_DATA_IPV6;
  }
  else if( !strncmp( QCRIL_DATA_IP_FAMILY_V4, value, sizeof(QCRIL_DATA_IP_FAMILY_V4) ) ) {
    temp = QCRIL_DATA_IPV4;
  }
  else {
    temp = QCRIL_DATA_IPV4;
    QCRIL_LOG_VERBOSE( "unrecognized filter version[%s], assuming version[%d]",
                       value, temp);
  }

  /* Update parsing context for current QOS spec */
  switch( context_ptr->direction )
  {
    case DS_RIL_QOS_DIR_TX:
      assert(DS_RIL_FILTER_ID_INVALID != context_ptr->tx_filter.id);
      switch( ((qcril_parsing_hndlr_s_type*)context_ptr->parse_ptr)->key )
      {
        case RIL_QOS_FILTER_IPVERSION:
          qos_spec_ptr[context_ptr->spec_id].tx_filter_req_array[context_ptr->tx_filter.id].
            ip_version = temp;
          QCRIL_LOG_VERBOSE( "QOS spec[%d] Tx filter[%d] - IP version[%d]",
                             context_ptr->spec_id, context_ptr->tx_filter.id, temp);
          break;
        default:
          QCRIL_LOG_ERROR( "unsupported key[%s] ",
                           ((qcril_parsing_hndlr_s_type*)context_ptr->parse_ptr)->key_name );
          goto err_label;
      }
      break;

    case DS_RIL_QOS_DIR_RX:
      assert(DS_RIL_FILTER_ID_INVALID != context_ptr->rx_filter.id);
      switch( ((qcril_parsing_hndlr_s_type*)context_ptr->parse_ptr)->key )
      {
        case RIL_QOS_FILTER_IPVERSION:
          qos_spec_ptr[context_ptr->spec_id].rx_filter_req_array[context_ptr->rx_filter.id].
            ip_version = temp;
          QCRIL_LOG_VERBOSE( "QOS spec[%d] Rx filter[%d] - IP version[%d]",
                             context_ptr->spec_id, context_ptr->rx_filter.id, temp);
          break;
        default:
          QCRIL_LOG_ERROR( "unsupported key[%s] ",
                           ((qcril_parsing_hndlr_s_type*)context_ptr->parse_ptr)->key_name );
          goto err_label;
      }
      break;

    default:
      QCRIL_LOG_ERROR( "unsupported direction[%d] value[%s] for key[%s] ",
                       context_ptr->direction, value,
                       ((qcril_parsing_hndlr_s_type*)context_ptr->parse_ptr)->key_name );
      goto err_label;
  }
  ret = SUCCESS;

 err_label:
  QCRIL_LOG_EXIT;
  return ret;
} /* qcril_data_process_filter_version() */


/*===========================================================================

  FUNCTION: qcril_data_process_filter_tos

===========================================================================*/
/*!
    @brief
    Process the QOS key=value pair for filter type of service (TOS)

    @pre

    @return
    SUCCESS
    FAILURE
*/
/*=========================================================================*/
static int qcril_data_process_filter_tos
(
  qcril_parsing_context_s_type *context_ptr,
  const char                   *value,
  dsi_qos_spec_type            *qos_spec_ptr
)
{
  int        ret = FAILURE;
  long int   temp;
  QCRIL_LOG_ENTRY;

  QCRIL_VERIFY_CONTEXT_VALID( context_ptr, err_label );

  QCRIL_CONVERT_INT( value, temp, err_label );

  /* Update parsing context for current QOS spec */
  switch( context_ptr->direction )
  {
    case DS_RIL_QOS_DIR_TX:
      assert(DS_RIL_FILTER_ID_INVALID != context_ptr->tx_filter.id);
      switch( ((qcril_parsing_hndlr_s_type*)context_ptr->parse_ptr)->key )
      {
        case RIL_QOS_FILTER_IPV4_TOS:
          qos_spec_ptr[context_ptr->spec_id].tx_filter_req_array[context_ptr->tx_filter.id].
            filter_desc.tos.tos_value = temp;
          qos_spec_ptr[context_ptr->spec_id].tx_filter_req_array[context_ptr->tx_filter.id].
            filter_desc.param_mask |= QMI_QOS_FILTER_PARAM_TOS;
          QCRIL_LOG_VERBOSE( "QOS spec[%d] Tx filter[%d]  - type of service[%d]",
                             context_ptr->spec_id, context_ptr->tx_filter.id, temp);
          context_ptr->tx_filter.valid = TRUE;
          break;
        case RIL_QOS_FILTER_IPV4_TOS_MASK:
          qos_spec_ptr[context_ptr->spec_id].tx_filter_req_array[context_ptr->tx_filter.id].
            filter_desc.tos.tos_mask = temp;
          QCRIL_LOG_VERBOSE( "QOS [%d] Tx filter spec[%d] - type of service mask[%d]",
                             context_ptr->spec_id, context_ptr->tx_filter.id, temp);
          break;
        default:
          QCRIL_LOG_ERROR( "unsupported key[%s] ",
                           ((qcril_parsing_hndlr_s_type*)context_ptr->parse_ptr)->key_name );
          goto err_label;
      }
      break;

    case DS_RIL_QOS_DIR_RX:
      assert(DS_RIL_FILTER_ID_INVALID != context_ptr->rx_filter.id);
      switch( ((qcril_parsing_hndlr_s_type*)context_ptr->parse_ptr)->key )
      {
        case RIL_QOS_FILTER_IPV4_TOS:
          qos_spec_ptr[context_ptr->spec_id].rx_filter_req_array[context_ptr->rx_filter.id].
            filter_desc.tos.tos_value = temp;
          qos_spec_ptr[context_ptr->spec_id].rx_filter_req_array[context_ptr->rx_filter.id].
            filter_desc.param_mask |= QMI_QOS_FILTER_PARAM_TOS;
          QCRIL_LOG_VERBOSE( "QOS spec[%d] Rx filter[%d] - type of service[%d]",
                             context_ptr->spec_id, context_ptr->rx_filter.id, temp);
          context_ptr->rx_filter.valid = TRUE;
          break;
        case RIL_QOS_FILTER_IPV4_TOS_MASK:
          qos_spec_ptr[context_ptr->spec_id].rx_filter_req_array[context_ptr->rx_filter.id].
            filter_desc.tos.tos_mask = temp;
          QCRIL_LOG_VERBOSE( "QOS spec[%d] Rx filter[%d] - type of service mask[%d]",
                             context_ptr->spec_id, context_ptr->rx_filter.id, temp);
          break;
        default:
          QCRIL_LOG_ERROR( "unsupported key[%s] ",
                           ((qcril_parsing_hndlr_s_type*)context_ptr->parse_ptr)->key_name );
          goto err_label;
      }
      break;

    default:
      QCRIL_LOG_ERROR( "unsupported direction[%d] value[%s] for key[%s] ",
                       context_ptr->direction, value,
                       ((qcril_parsing_hndlr_s_type*)context_ptr->parse_ptr)->key_name );
      goto err_label;
  }
  ret = SUCCESS;

 err_label:
  QCRIL_LOG_EXIT;
  return ret;
} /* qcril_data_process_filter_tos() */


/*===========================================================================

  FUNCTION: qcril_data_process_filter_port

===========================================================================*/
/*!
    @brief
    Process the QOS key=value pair for filter port items

    @pre

    @return
    SUCCESS
    FAILURE
*/
/*=========================================================================*/
static int qcril_data_process_filter_port
(
  qcril_parsing_context_s_type *context_ptr,
  const char                   *value,
  dsi_qos_spec_type            *qos_spec_ptr
)
{
  int        ret = FAILURE;
  long int   temp;
  QCRIL_LOG_ENTRY;

  QCRIL_VERIFY_CONTEXT_VALID( context_ptr, err_label );

  QCRIL_CONVERT_INT( value, temp, err_label );

  /* Update parsing context for current QOS spec */
  switch( context_ptr->direction )
  {
    case DS_RIL_QOS_DIR_TX:
      assert(DS_RIL_FILTER_ID_INVALID != context_ptr->tx_filter.id);
      switch( ((qcril_parsing_hndlr_s_type*)context_ptr->parse_ptr)->key )
      {
        case RIL_QOS_FILTER_TCP_SOURCE_PORT_START:
          qos_spec_ptr[context_ptr->spec_id].tx_filter_req_array[context_ptr->tx_filter.id].
            filter_desc.tcp_src_ports.start_port = (unsigned short)temp;
          qos_spec_ptr[context_ptr->spec_id].tx_filter_req_array[context_ptr->tx_filter.id].
            filter_desc.param_mask |= QMI_QOS_FILTER_PARAM_TCP_SRC_PORTS;
          qos_spec_ptr[context_ptr->spec_id].tx_filter_req_array[context_ptr->tx_filter.id].
            filter_desc.protocol = QMI_QOS_TRANS_PROT_TCP;
          qos_spec_ptr[context_ptr->spec_id].tx_filter_req_array[context_ptr->tx_filter.id].
            filter_desc.param_mask |= QMI_QOS_FILTER_PARAM_TRANS_PROTOCOL;
          QCRIL_LOG_VERBOSE( "QOS spec[%d] Tx filter[%d] - TCP source port start[%d]",
                             context_ptr->spec_id, context_ptr->tx_filter.id, temp);
          context_ptr->tx_filter.valid = TRUE;
          break;

        case RIL_QOS_FILTER_TCP_SOURCE_PORT_RANGE:
          qos_spec_ptr[context_ptr->spec_id].tx_filter_req_array[context_ptr->tx_filter.id].
            filter_desc.tcp_src_ports.range = temp;
          QCRIL_LOG_VERBOSE( "QOS spec[%d] Tx filter[%d]  - TCP source port range[%d]",
                             context_ptr->spec_id, context_ptr->tx_filter.id, temp);
          break;

        case RIL_QOS_FILTER_TCP_DESTINATION_PORT_START:
          qos_spec_ptr[context_ptr->spec_id].tx_filter_req_array[context_ptr->tx_filter.id].
            filter_desc.tcp_dest_ports.start_port = (unsigned short)temp;
          qos_spec_ptr[context_ptr->spec_id].tx_filter_req_array[context_ptr->tx_filter.id].
            filter_desc.param_mask |= QMI_QOS_FILTER_PARAM_TCP_DEST_PORTS;
          qos_spec_ptr[context_ptr->spec_id].tx_filter_req_array[context_ptr->tx_filter.id].
            filter_desc.protocol = QMI_QOS_TRANS_PROT_TCP;
          qos_spec_ptr[context_ptr->spec_id].tx_filter_req_array[context_ptr->tx_filter.id].
            filter_desc.param_mask |= QMI_QOS_FILTER_PARAM_TRANS_PROTOCOL;
          QCRIL_LOG_VERBOSE( "QOS spec[%d] Tx filter[%d]  - TCP destination port start[%d]",
                             context_ptr->spec_id, context_ptr->tx_filter.id, temp);
          context_ptr->tx_filter.valid = TRUE;
          break;

        case RIL_QOS_FILTER_TCP_DESTINATION_PORT_RANGE:
          qos_spec_ptr[context_ptr->spec_id].tx_filter_req_array[context_ptr->tx_filter.id].
            filter_desc.tcp_dest_ports.range = temp;
          QCRIL_LOG_VERBOSE( "QOS Tx filter spec[%d] - TCP destination port range[%d]",
                             context_ptr->spec_id, context_ptr->tx_filter.id, temp);
          break;

        case RIL_QOS_FILTER_UDP_SOURCE_PORT_START:
          qos_spec_ptr[context_ptr->spec_id].tx_filter_req_array[context_ptr->tx_filter.id].
            filter_desc.udp_src_ports.start_port = (unsigned short)temp;
          qos_spec_ptr[context_ptr->spec_id].tx_filter_req_array[context_ptr->tx_filter.id].
            filter_desc.param_mask |= QMI_QOS_FILTER_PARAM_UDP_SRC_PORTS;
          qos_spec_ptr[context_ptr->spec_id].tx_filter_req_array[context_ptr->tx_filter.id].
            filter_desc.protocol = QMI_QOS_TRANS_PROT_UDP;
          qos_spec_ptr[context_ptr->spec_id].tx_filter_req_array[context_ptr->tx_filter.id].
            filter_desc.param_mask |= QMI_QOS_FILTER_PARAM_TRANS_PROTOCOL;
          QCRIL_LOG_VERBOSE( "QOS spec[%d] Tx filter[%d]  - UDP source port start[%d]",
                             context_ptr->spec_id, context_ptr->tx_filter.id, temp);
          context_ptr->tx_filter.valid = TRUE;
          break;

        case RIL_QOS_FILTER_UDP_SOURCE_PORT_RANGE:
          qos_spec_ptr[context_ptr->spec_id].tx_filter_req_array[context_ptr->tx_filter.id].
            filter_desc.udp_src_ports.range = temp;
          QCRIL_LOG_VERBOSE( "QOS spec[%d] Tx filter[%d]  - UDP source port range[%d]",
                             context_ptr->spec_id, context_ptr->tx_filter.id, temp);
          break;

        case RIL_QOS_FILTER_UDP_DESTINATION_PORT_START:
          qos_spec_ptr[context_ptr->spec_id].tx_filter_req_array[context_ptr->tx_filter.id].
            filter_desc.udp_dest_ports.start_port = (unsigned short)temp;
          qos_spec_ptr[context_ptr->spec_id].tx_filter_req_array[context_ptr->tx_filter.id].
            filter_desc.param_mask |= QMI_QOS_FILTER_PARAM_UDP_DEST_PORTS;
          qos_spec_ptr[context_ptr->spec_id].tx_filter_req_array[context_ptr->tx_filter.id].
            filter_desc.protocol = QMI_QOS_TRANS_PROT_UDP;
          qos_spec_ptr[context_ptr->spec_id].tx_filter_req_array[context_ptr->tx_filter.id].
            filter_desc.param_mask |= QMI_QOS_FILTER_PARAM_TRANS_PROTOCOL;
          QCRIL_LOG_VERBOSE( "QOS spec[%d] Tx filter[%d] - UDP destination port start[%d]",
                             context_ptr->spec_id, context_ptr->tx_filter.id, temp);
          context_ptr->tx_filter.valid = TRUE;
          break;

        case RIL_QOS_FILTER_UDP_DESTINATION_PORT_RANGE:
          qos_spec_ptr[context_ptr->spec_id].tx_filter_req_array[context_ptr->tx_filter.id].
            filter_desc.udp_dest_ports.range = temp;
          QCRIL_LOG_VERBOSE( "QOS spec[%d] Tx filter[%d] - UDP destination port range[%d]",
                             context_ptr->spec_id, context_ptr->tx_filter.id, temp);
          break;

        default:
          QCRIL_LOG_ERROR( "unsupported key[%s] ",
                           ((qcril_parsing_hndlr_s_type*)context_ptr->parse_ptr)->key_name );
          goto err_label;
      }
      break;

    case DS_RIL_QOS_DIR_RX:
      assert(DS_RIL_FILTER_ID_INVALID != context_ptr->rx_filter.id);
      switch( ((qcril_parsing_hndlr_s_type*)context_ptr->parse_ptr)->key )
      {
        case RIL_QOS_FILTER_TCP_SOURCE_PORT_START:
          qos_spec_ptr[context_ptr->spec_id].rx_filter_req_array[context_ptr->rx_filter.id].
            filter_desc.tcp_src_ports.start_port = (unsigned short)temp;
          qos_spec_ptr[context_ptr->spec_id].rx_filter_req_array[context_ptr->rx_filter.id].
            filter_desc.param_mask |= QMI_QOS_FILTER_PARAM_TCP_SRC_PORTS;
          qos_spec_ptr[context_ptr->spec_id].rx_filter_req_array[context_ptr->rx_filter.id].
            filter_desc.protocol = QMI_QOS_TRANS_PROT_TCP;
          qos_spec_ptr[context_ptr->spec_id].rx_filter_req_array[context_ptr->rx_filter.id].
            filter_desc.param_mask |= QMI_QOS_FILTER_PARAM_TRANS_PROTOCOL;
          QCRIL_LOG_VERBOSE( "QOS spec[%d] Rx filter[%d] - TCP source port start[%d]",
                             context_ptr->spec_id, context_ptr->rx_filter.id, temp);
          context_ptr->rx_filter.valid = TRUE;
          break;

        case RIL_QOS_FILTER_TCP_SOURCE_PORT_RANGE:
          qos_spec_ptr[context_ptr->spec_id].rx_filter_req_array[context_ptr->rx_filter.id].
            filter_desc.tcp_src_ports.range = temp;
          QCRIL_LOG_VERBOSE( "QOS spec[%d] Rx filter[%d]  - TCP source port range[%d]",
                             context_ptr->spec_id, context_ptr->rx_filter.id, temp);
          break;

        case RIL_QOS_FILTER_TCP_DESTINATION_PORT_START:
          qos_spec_ptr[context_ptr->spec_id].rx_filter_req_array[context_ptr->rx_filter.id].
            filter_desc.tcp_dest_ports.start_port = (unsigned short)temp;
          qos_spec_ptr[context_ptr->spec_id].rx_filter_req_array[context_ptr->rx_filter.id].
            filter_desc.param_mask |= QMI_QOS_FILTER_PARAM_TCP_DEST_PORTS;
          qos_spec_ptr[context_ptr->spec_id].rx_filter_req_array[context_ptr->rx_filter.id].
            filter_desc.protocol = QMI_QOS_TRANS_PROT_TCP;
          qos_spec_ptr[context_ptr->spec_id].rx_filter_req_array[context_ptr->rx_filter.id].
            filter_desc.param_mask |= QMI_QOS_FILTER_PARAM_TRANS_PROTOCOL;
          QCRIL_LOG_VERBOSE( "QOS spec[%d] Rx filter[%d]  - TCP destination port start[%d]",
                             context_ptr->spec_id, context_ptr->rx_filter.id, temp);
          context_ptr->rx_filter.valid = TRUE;
          break;

        case RIL_QOS_FILTER_TCP_DESTINATION_PORT_RANGE:
          qos_spec_ptr[context_ptr->spec_id].rx_filter_req_array[context_ptr->rx_filter.id].
            filter_desc.tcp_dest_ports.range = temp;
          QCRIL_LOG_VERBOSE( "QOS Rx filter spec[%d] - TCP destination port range[%d]",
                             context_ptr->spec_id, context_ptr->rx_filter.id, temp);
          break;

        case RIL_QOS_FILTER_UDP_SOURCE_PORT_START:
          qos_spec_ptr[context_ptr->spec_id].rx_filter_req_array[context_ptr->rx_filter.id].
            filter_desc.udp_src_ports.start_port = (unsigned short)temp;
          qos_spec_ptr[context_ptr->spec_id].rx_filter_req_array[context_ptr->rx_filter.id].
            filter_desc.param_mask |= QMI_QOS_FILTER_PARAM_UDP_SRC_PORTS;
          qos_spec_ptr[context_ptr->spec_id].rx_filter_req_array[context_ptr->rx_filter.id].
            filter_desc.protocol = QMI_QOS_TRANS_PROT_UDP;
          qos_spec_ptr[context_ptr->spec_id].rx_filter_req_array[context_ptr->rx_filter.id].
            filter_desc.param_mask |= QMI_QOS_FILTER_PARAM_TRANS_PROTOCOL;
          QCRIL_LOG_VERBOSE( "QOS spec[%d] Rx filter[%d]  - UDP source port start[%d]",
                             context_ptr->spec_id, context_ptr->rx_filter.id, temp);
          context_ptr->rx_filter.valid = TRUE;
          break;

        case RIL_QOS_FILTER_UDP_SOURCE_PORT_RANGE:
          qos_spec_ptr[context_ptr->spec_id].rx_filter_req_array[context_ptr->rx_filter.id].
            filter_desc.udp_src_ports.range = temp;
          QCRIL_LOG_VERBOSE( "QOS spec[%d] Rx filter[%d]  - UDP source port range[%d]",
                             context_ptr->spec_id, context_ptr->rx_filter.id, temp);
          break;

        case RIL_QOS_FILTER_UDP_DESTINATION_PORT_START:
          qos_spec_ptr[context_ptr->spec_id].rx_filter_req_array[context_ptr->rx_filter.id].
            filter_desc.udp_dest_ports.start_port = (unsigned short)temp;
          qos_spec_ptr[context_ptr->spec_id].rx_filter_req_array[context_ptr->rx_filter.id].
            filter_desc.param_mask |= QMI_QOS_FILTER_PARAM_UDP_DEST_PORTS;
          qos_spec_ptr[context_ptr->spec_id].rx_filter_req_array[context_ptr->rx_filter.id].
            filter_desc.protocol = QMI_QOS_TRANS_PROT_UDP;
          qos_spec_ptr[context_ptr->spec_id].rx_filter_req_array[context_ptr->rx_filter.id].
            filter_desc.param_mask |= QMI_QOS_FILTER_PARAM_TRANS_PROTOCOL;
          QCRIL_LOG_VERBOSE( "QOS spec[%d] Rx filter[%d] - UDP destination port start[%d]",
                             context_ptr->spec_id, context_ptr->rx_filter.id, temp);
          context_ptr->rx_filter.valid = TRUE;
          break;

        case RIL_QOS_FILTER_UDP_DESTINATION_PORT_RANGE:
          qos_spec_ptr[context_ptr->spec_id].rx_filter_req_array[context_ptr->rx_filter.id].
            filter_desc.udp_dest_ports.range = temp;
          QCRIL_LOG_VERBOSE( "QOS spec[%d] Rx filter[%d] - UDP destination port range[%d]",
                             context_ptr->spec_id, context_ptr->rx_filter.id, temp);
          break;

        default:
          QCRIL_LOG_ERROR( "unsupported key[%s] ",
                           ((qcril_parsing_hndlr_s_type*)context_ptr->parse_ptr)->key_name );
          goto err_label;
      }
      break;

    default:
      QCRIL_LOG_ERROR( "unsupported direction[%d] value[%s] for key[%s] ",
                       context_ptr->direction, value,
                       ((qcril_parsing_hndlr_s_type*)context_ptr->parse_ptr)->key_name );
      goto err_label;
  }
  ret = SUCCESS;

 err_label:
  QCRIL_LOG_EXIT;
  return ret;
} /* qcril_data_process_filter_port() */



/*===========================================================================

  FUNCTION: qcril_data_parse_ipv4_address

===========================================================================*/
/*!
    @brief
    Decode IPv4 address string into internal numeric representation
    Expected format: xxx.xxx.xxx.xxx

    @pre

    @return
    SUCCESS
    FAILURE
*/
/*=========================================================================*/
static int qcril_data_parse_ipv4_address
(
  char             *data,
  unsigned long    *value
)
{
  int          ret = FAILURE;
  int          temp;
  char        *buffer = NULL;
  char        *token = NULL;
  char        *save_ptr = NULL;
  unsigned int offset = QCRIL_QOS_ADDR_IPV4_OFFSET_I;
  unsigned int addr = 0;
  QCRIL_LOG_ENTRY;

  QCRIL_DS_ASSERT( data  != NULL, "validate data" );
  QCRIL_DS_ASSERT( value != NULL, "validate value" );

  if( (NULL == data) || (NULL == value) )
  {
    goto err_label;
  }
  /* Process each segment of address into numeric */
  token = strtok_r( data, QCRIL_QOS_IPV4_DELIMITER, &save_ptr );
  while( token && (QCRIL_QOS_ADDR_IPV4_OFFSET_I >= offset) )
  {
    QCRIL_CONVERT_INT( token, temp, err_label );
    /* Ensure token in range [0,255] */
    if( QCRIL_QOS_ADDR_IPV4_MIN > temp || QCRIL_QOS_ADDR_IPV4_MAX < temp )
    {
      QCRIL_LOG_ERROR( "invalid address string value[%s]", value );
      goto err_label;
    }

    /* Add in the numeric value of token */
    addr |= (temp << offset);

    /* Advance to next token */
    token = strtok_r( NULL, QCRIL_QOS_IPV4_DELIMITER, &save_ptr );
    offset -= QCRIL_QOS_ADDR_IPV4_OFFSET_D;
  }

  /* Check that the 4 required tokens were processed.  Offset will
   * be underflowed in this case. */
  if( QCRIL_QOS_ADDR_IPV4_OFFSET_I >= offset )
  {
    QCRIL_LOG_ERROR( "invalid address string value[%s]", value );
    goto err_label;
  }

  /* Pass the numeric address value back to caller */
  *value = addr;
  ret = SUCCESS;

 err_label:
  QCRIL_LOG_EXIT;
  return ret;
} /* qcril_data_parse_ipv4_address() */


/*===========================================================================

  FUNCTION: qcril_data_decode_ipv4_address

===========================================================================*/
/*!
    @brief
    Decode IPv4 address string into internal numeric representation
    Expected format: xxx.xxx.xxx.xxx/yy

    @pre

    @return
    SUCCESS
    FAILURE
*/
/*=========================================================================*/
static int qcril_data_decode_ipv4_address
(
  const char                    *value,
  dsi_qos_ipv4_addr_filter_type *addr_ptr
)
{
  int           ret = FAILURE;
  unsigned int  temp;
  char         *buffer = NULL;
  char         *token = NULL;
  char         *token2 = NULL;
  char         *save_ptr = NULL;
  char         *save2_ptr = NULL;
  unsigned int  offset = 0;
  unsigned long addr = 0;
  QCRIL_LOG_ENTRY;

  QCRIL_DS_ASSERT( value != NULL, "validate value" );
  QCRIL_DS_ASSERT( addr_ptr != NULL, "validate addr_ptr" );

  if( (NULL == value ) || (NULL == addr_ptr) )
  {
    goto err_label;
  }

  /* Copy source data into working buffer */
  QCRIL_DATA_ALLOC_STORAGE( buffer, (strlen(value)+1), err_label );
  memcpy( buffer, value, (strlen(value)+1) );

  /* First step is to break string into IP address and subnet mask (if specified) */
  token = strtok_r( buffer, QCRIL_QOS_SUBNET_DELIMITER, &save_ptr );
  if( NULL != token )
  {
    if( SUCCESS != qcril_data_parse_ipv4_address( token, &addr ) )
    {
      QCRIL_LOG_ERROR( "invalid address string value[%s]", value );
      goto mem_label;
    }

    addr_ptr->ipv4_ip_addr = addr;
  }
  else
  {
    QCRIL_LOG_ERROR( "invalid address string value[%s]", value );
    goto mem_label;
  }

  token = strtok_r( NULL, QCRIL_QOS_SUBNET_DELIMITER, &save_ptr );
  if( NULL != token )
  {
    /* Subnet mask processing, convert "yy" into bit-set */
    QCRIL_CONVERT_INT( token, temp, err_label );
    int shift = (QCRIL_QOS_ADDR_IPV4_SHIFT_I - temp);
    if( 0 > shift )
    {
      QCRIL_LOG_ERROR( "invalid subnet mask string value[%s]", value );
      goto mem_label;
    }
    temp = (QCRIL_QOS_ADDR_SUBNET_DEFAULT >> shift) << shift;
    addr_ptr->ipv4_subnet_mask = temp;
  }
  else
  {
    /* Assume default subnet mask */
    addr_ptr->ipv4_subnet_mask = ntohl( QCRIL_QOS_ADDR_SUBNET_DEFAULT );
  }

  ret = SUCCESS;

 mem_label:
  QCRIL_DATA_RELEASE_STORAGE( buffer );

 err_label:
  QCRIL_LOG_EXIT;
  return ret;
} /* qcril_data_decode_ipv4_address() */


/*===========================================================================

  FUNCTION: qcril_data_parse_ipv6_address

===========================================================================*/
/*!
    @brief
    Decode IPv6 address string into internal numeric representation
    Expected format:   xxxx:xxxx:xxxx:xxxx:xxxx:xxxx:xxxx:xxxx
    Special case: Use of "::" indicates all zeros between delimiters

    @pre

    @return
    SUCCESS
    FAILURE
*/
/*=========================================================================*/
static int qcril_data_parse_ipv6_address
(
  char               *data,
  qcril_data_addr6_t *addr6
)
{
  int          ret = FAILURE;
  uint32       temp = 0;
  char        *buffer = NULL;
  char        *token = NULL;
  char        *save_ptr = NULL;
  char         data2[QCRIL_QOS_IPV6_ADDR_STR_LEN+1] = "0000:0000:0000:0000:0000:0000:0000:0000";
  unsigned int i=0;

  QCRIL_LOG_ENTRY;

  QCRIL_DS_ASSERT( data  != NULL, "validate data" );
  QCRIL_DS_ASSERT( addr6 != NULL, "validate addr6" );

  if( (NULL == data) || (NULL == addr6))
  {
    goto err_label;
  }

  /* Check for presence of '::' delimiter */
  if( NULL != (token = strstr( data, QCRIL_QOS_IPV6_DELIMITER2 )) )
  {
    /* Recast address string, replacing '::' with zero padding */
    int addrlen = strlen(data);
    int end_offset = addrlen - ((token-data)+QCRIL_QOS_IPV6_DELIMITER2_LEN);
    if( 0 > end_offset )
    {
      QCRIL_LOG_ERROR( "failed on delimiter end offset: data[%p] token[%p] len[%d]",
                       data, token, addrlen );
      return FAILURE;
    }

    int data2_len = strlen(data2);
    memcpy( data2, data, (token-data) );             /* pre-delimiter portion  */
    memcpy( (data2 + (data2_len - end_offset)),
            (token+QCRIL_QOS_IPV6_DELIMITER2_LEN),
            end_offset );                            /* post-delimiter portion */
    data2[QCRIL_QOS_IPV6_ADDR_STR_LEN] = 0;          /* Ensure NULL terminated */
    buffer = data2;
  }
  else
  {
    /* Use original buffer */
    buffer = data;
  }

  memset( addr6->in6_u.u6_addr8, 0x0, sizeof(addr6->in6_u.u6_addr8) );

  /* Process each segment of address into numeric */
  token = strtok_r( buffer, QCRIL_QOS_IPV6_DELIMITER, &save_ptr );
  while( token && (QCRIL_QOS_ADDR_IPV6_ARRAY_M > i) )
  {
    QCRIL_CONVERT_HEX( token, temp, err_label );
    /* Ensure token in range [0,0xFFFF] */
    if( QCRIL_QOS_ADDR_IPV6_MAX < temp )
    {
      QCRIL_LOG_ERROR( "invalid address string value[%s]", token );
      goto err_label;
    }

    /* Swap bytes of token */
    addr6->in6_u.u6_addr16[i++] = ((temp & 0xFF00)>>8) | ((temp & 0x00FF)<<8);

    /* Advance to next token */
    token = strtok_r( NULL, QCRIL_QOS_IPV6_DELIMITER, &save_ptr );
  }

  /* Check that the 8 required tokens were processed. */
  if( QCRIL_QOS_ADDR_IPV6_ARRAY_M > i )
  {
    QCRIL_LOG_ERROR( "invalid address string value[%s]", data );
    goto err_label;
  }

  ret = SUCCESS;

 err_label:
  QCRIL_LOG_EXIT;
  return ret;
} /* qcril_data_parse_ipv6_address() */



/*===========================================================================

  FUNCTION: qcril_data_decode_ipv6_address

===========================================================================*/
/*!
    @brief
    Decode IPv6 address string into internal numeric representation
    Expected format:   xxxx:xxxx:xxxx:xxxx:xxxx:xxxx:xxxx:xxxx/yyy

    @pre

    @return
    SUCCESS
    FAILURE
*/
/*=========================================================================*/
static int qcril_data_decode_ipv6_address
(
  const char                    *value,
  dsi_qos_ipv6_addr_filter_type *addr_ptr
)
{
  int           ret = FAILURE;
  int           temp;
  char         *buffer = NULL;
  char         *token = NULL;
  char         *token2 = NULL;
  char         *save_ptr = NULL;
  char         *save2_ptr = NULL;
  char         *bad_ptr = NULL;
  unsigned int  offset = 0;
  unsigned long addr = 0;
  qcril_data_addr6_t addr6;

  QCRIL_LOG_ENTRY;

  QCRIL_DS_ASSERT( value != NULL, "validate value" );
  QCRIL_DS_ASSERT( addr_ptr != NULL, "validate addr_ptr" );

  if( (NULL == value) || (NULL == addr_ptr) )
  {
    goto err_label;
  }

  /* Copy source data into working buffer */
  QCRIL_DATA_ALLOC_STORAGE( buffer, (strlen(value)+1), err_label );
  memcpy( buffer, value, (strlen(value)+1) );

  /* First step is to break string into IP address and subnet mask (if specified) */
  token = strtok_r( buffer, QCRIL_QOS_SUBNET_DELIMITER, &save_ptr );
  if( NULL != token )
  {
    if( SUCCESS != qcril_data_parse_ipv6_address( token, &addr6 ) )
    {
      QCRIL_LOG_ERROR( "invalid address string value[%s]", value );
      goto mem_label;
    }

#if 0 /* QMI team indicates not needed at this time */
    if( SUCCESS != qcril_data_addr_ntoh_str( &addr6,
                                             addr_ptr->ipv6_ip_addr,
                                             sizeof(addr6.in6_u.u6_addr8) ) )
    {
      QCRIL_LOG_ERROR( "invalid address string value[%s]", value );
      goto mem_label;
    }
#else
    memcpy( addr_ptr->ipv6_ip_addr, addr6.in6_u.u6_addr8,
            sizeof(addr_ptr->ipv6_ip_addr) );
#endif
  }
  else
  {
    QCRIL_LOG_ERROR( "invalid address string value[%s]", value );
    goto mem_label;
  }

  token = strtok_r( NULL, QCRIL_QOS_SUBNET_DELIMITER, &save_ptr );
  if( NULL != token )
  {
    /* Prefix length processing */
    addr_ptr->ipv6_filter_prefix_len = strtol( token, &bad_ptr, 0 );
    if( *bad_ptr != 0 )
    {
      QCRIL_LOG_ERROR( "invalid prefix string value[%s]", token );
      goto mem_label;
    }
  }
  else
  {
    /* Assume default prefix length */
    addr_ptr->ipv6_filter_prefix_len = QCRIL_QOS_ADDR_IPV6_PFX_LEN;
  }

  ret = SUCCESS;

 mem_label:
  QCRIL_DATA_RELEASE_STORAGE( buffer );

 err_label:
  QCRIL_LOG_EXIT;
  return ret;
} /* qcril_data_decode_ipv6_address() */



/*===========================================================================

  FUNCTION: qcril_data_process_filter_address

===========================================================================*/
/*!
    @brief
    Process the QOS key=value pair for filter IP address

    @pre

    @return
    SUCCESS
    FAILURE
*/
/*=========================================================================*/
static int qcril_data_process_filter_address
(
  qcril_parsing_context_s_type *context_ptr,
  const char                   *value,
  dsi_qos_spec_type            *qos_spec_ptr
)
{
  int          ret = FAILURE;
  long int     temp;

  QCRIL_LOG_ENTRY;

  QCRIL_VERIFY_CONTEXT_VALID( context_ptr, err_label );

  switch( context_ptr->direction )
  {
    case DS_RIL_QOS_DIR_TX:
    {
      assert(DS_RIL_FILTER_ID_INVALID != context_ptr->tx_filter.id);
      switch( ((qcril_parsing_hndlr_s_type*)context_ptr->parse_ptr)->key )
      {
        case RIL_QOS_FILTER_IPV4_SOURCE_ADDR:
          if( SUCCESS !=
              qcril_data_decode_ipv4_address( value,
                                              &qos_spec_ptr[context_ptr->spec_id].
                                              tx_filter_req_array[context_ptr->tx_filter.id].
                                              filter_desc.src_addr) )
          {
            QCRIL_LOG_ERROR( "failed on TX qcril_data_decode_ipv4_address key[%s]",
                             ((qcril_parsing_hndlr_s_type*)context_ptr->parse_ptr)->key_name );
            goto err_label;
          }
          qos_spec_ptr[context_ptr->spec_id].tx_filter_req_array[context_ptr->tx_filter.id].
            filter_desc.param_mask |= QMI_QOS_FILTER_PARAM_SRC_ADDR;
          context_ptr->tx_filter.valid = TRUE;
          break;

        case RIL_QOS_FILTER_IPV4_DESTINATION_ADDR:
          if( SUCCESS !=
              qcril_data_decode_ipv4_address( value,
                                              &qos_spec_ptr[context_ptr->spec_id].
                                              tx_filter_req_array[context_ptr->tx_filter.id].
                                              filter_desc.dest_addr) )
          {
            QCRIL_LOG_ERROR( "failed on TX qcril_data_decode_ipv4_address key[%s]",
                             ((qcril_parsing_hndlr_s_type*)context_ptr->parse_ptr)->key_name );
            goto err_label;
          }
          qos_spec_ptr[context_ptr->spec_id].tx_filter_req_array[context_ptr->tx_filter.id].
            filter_desc.param_mask |= QMI_QOS_FILTER_PARAM_DEST_ADDR;
          context_ptr->tx_filter.valid = TRUE;
          break;

        case RIL_QOS_FILTER_IPV6_SOURCE_ADDR:
          if( SUCCESS !=
              qcril_data_decode_ipv6_address( value,
                                              &qos_spec_ptr[context_ptr->spec_id].
                                              tx_filter_req_array[context_ptr->tx_filter.id].
                                              filter_desc.ipv6_src_addr) )
          {
            QCRIL_LOG_ERROR( "failed on TX qcril_data_decode_ipv6_address key[%s]",
                             ((qcril_parsing_hndlr_s_type*)context_ptr->parse_ptr)->key_name );
            goto err_label;
          }
          qos_spec_ptr[context_ptr->spec_id].tx_filter_req_array[context_ptr->tx_filter.id].
            filter_desc.param_mask |= QMI_QOS_FILTER_PARAM_IPV6_SRC_ADDR;
          context_ptr->tx_filter.valid = TRUE;
          break;

        case RIL_QOS_FILTER_IPV6_DESTINATION_ADDR:
          if( SUCCESS !=
              qcril_data_decode_ipv6_address( value,
                                              &qos_spec_ptr[context_ptr->spec_id].
                                              tx_filter_req_array[context_ptr->tx_filter.id].
                                              filter_desc.ipv6_dest_addr) )
          {
            QCRIL_LOG_ERROR( "failed on TX qcril_data_decode_ipv6_address key[%s]",
                             ((qcril_parsing_hndlr_s_type*)context_ptr->parse_ptr)->key_name );
            goto err_label;
          }
          qos_spec_ptr[context_ptr->spec_id].tx_filter_req_array[context_ptr->tx_filter.id].
            filter_desc.param_mask |= QMI_QOS_FILTER_PARAM_IPV6_DEST_ADDR;
          context_ptr->tx_filter.valid = TRUE;
          break;

        default:
          break;
      } /* key */
      break;
    } /* DS_RIL_QOS_DIR_TX */


    case DS_RIL_QOS_DIR_RX:
    {
      assert(DS_RIL_FILTER_ID_INVALID != context_ptr->rx_filter.id);
      switch( ((qcril_parsing_hndlr_s_type*)context_ptr->parse_ptr)->key )
      {
        case RIL_QOS_FILTER_IPV4_SOURCE_ADDR:
        {
          if( SUCCESS !=
              qcril_data_decode_ipv4_address( value,
                                              &qos_spec_ptr[context_ptr->spec_id].
                                              rx_filter_req_array[context_ptr->rx_filter.id].
                                              filter_desc.src_addr) )
          {
            QCRIL_LOG_ERROR( "failed on RX qcril_data_decode_ipv4_address key[%s]",
                             ((qcril_parsing_hndlr_s_type*)context_ptr->parse_ptr)->key_name );
            goto err_label;
          }
          qos_spec_ptr[context_ptr->spec_id].rx_filter_req_array[context_ptr->rx_filter.id].
            filter_desc.param_mask |= QMI_QOS_FILTER_PARAM_SRC_ADDR;
          context_ptr->rx_filter.valid = TRUE;
          break;
        }

        case RIL_QOS_FILTER_IPV4_DESTINATION_ADDR:
        {
          if( SUCCESS !=
              qcril_data_decode_ipv4_address( value,
                                              &qos_spec_ptr[context_ptr->spec_id].
                                              rx_filter_req_array[context_ptr->rx_filter.id].
                                              filter_desc.dest_addr) )
          {
            QCRIL_LOG_ERROR( "failed on RX qcril_data_decode_ipv4_address key[%s]",
                             ((qcril_parsing_hndlr_s_type*)context_ptr->parse_ptr)->key_name );
            goto err_label;
          }
          qos_spec_ptr[context_ptr->spec_id].rx_filter_req_array[context_ptr->rx_filter.id].
            filter_desc.param_mask |= QMI_QOS_FILTER_PARAM_DEST_ADDR;
          context_ptr->rx_filter.valid = TRUE;
          break;
        }

        case RIL_QOS_FILTER_IPV6_SOURCE_ADDR:
          if( SUCCESS !=
              qcril_data_decode_ipv6_address( value,
                                              &qos_spec_ptr[context_ptr->spec_id].
                                              rx_filter_req_array[context_ptr->rx_filter.id].
                                              filter_desc.ipv6_src_addr) )
          {
            QCRIL_LOG_ERROR( "failed on RX qcril_data_decode_ipv6_address key[%s]",
                             ((qcril_parsing_hndlr_s_type*)context_ptr->parse_ptr)->key_name );
            goto err_label;
          }
          qos_spec_ptr[context_ptr->spec_id].rx_filter_req_array[context_ptr->rx_filter.id].
            filter_desc.param_mask |= QMI_QOS_FILTER_PARAM_IPV6_SRC_ADDR;
          context_ptr->rx_filter.valid = TRUE;
          break;

        case RIL_QOS_FILTER_IPV6_DESTINATION_ADDR:
          if( SUCCESS !=
              qcril_data_decode_ipv6_address( value,
                                              &qos_spec_ptr[context_ptr->spec_id].
                                              rx_filter_req_array[context_ptr->rx_filter.id].
                                              filter_desc.ipv6_dest_addr) )
          {
            QCRIL_LOG_ERROR( "failed on RX qcril_data_decode_ipv6_address key[%s]",
                             ((qcril_parsing_hndlr_s_type*)context_ptr->parse_ptr)->key_name );
            goto err_label;
          }
          qos_spec_ptr[context_ptr->spec_id].rx_filter_req_array[context_ptr->rx_filter.id].
            filter_desc.param_mask |= QMI_QOS_FILTER_PARAM_IPV6_DEST_ADDR;
          context_ptr->rx_filter.valid = TRUE;
          break;

        default:
          break;
      }
      break;
    } /* DS_RIL_QOS_DIR_RX */

    default:
      break;
  } /* direction */

  ret = SUCCESS;

 err_label:
  QCRIL_LOG_EXIT;
  return ret;
} /* qcril_data_process_filter_address() */


/*===========================================================================

  FUNCTION: qcril_data_process_qos_key_value

===========================================================================*/
/*!
    @brief
    Parse the QOS key=value pair to populate the DSI QOS data structure.

    @pre

    @return
    SUCCESS
    FAILURE
*/
/*=========================================================================*/
static int qcril_data_parse_qos_key_value
(
  const char                    *data,
  dsi_qos_spec_type             *qos_spec_ptr,
  qcril_parsing_context_s_type  *context_ptr
)
{
  char         *buffer = NULL;
  char         *token = NULL;
  char         *save_ptr = NULL;
  char         *key_name = NULL;
  char         *value = NULL;
  int           ret = FAILURE;
  int           rc = FAILURE;
  unsigned int  i = 0;

  QCRIL_LOG_ENTRY;

  /* Copy source data into working buffer */
  QCRIL_DATA_ALLOC_STORAGE( buffer, (strlen(data)+1), err_label );
  memcpy( buffer, data, (strlen(data)+1) );

  /* Tokenize the buffer using QOS key delimiter.  Only single
   * key=value token expected. */
  token = strtok_r( buffer, QCRIL_QOS_KEY_DELIMITER, &save_ptr );
  if( NULL != token )
  {
    key_name = token;
    value = strtok_r( NULL, QCRIL_QOS_KEY_DELIMITER, &save_ptr );

    /* Check for non-NULL token value; ignore if NULL */
    if( NULL != value )
    {
      QCRIL_LOG_DEBUG("processing key[%s] value[%s]", key_name, value);

      /* Lookup the parseing handler based on key name */
      context_ptr->parse_ptr = qcril_data_find_parse_rec_by_name( key_name );
      if( NULL == context_ptr->parse_ptr )
      {
        QCRIL_LOG_ERROR( "failed to find key in parse table: %s", key_name );
        goto err_label;
      }

      rc = context_ptr->parse_ptr->handler( context_ptr,
                                            value,
                                            qos_spec_ptr );
      if( SUCCESS != rc )
      {
        QCRIL_LOG_ERROR( "failed to process key-value token: %s", key_name );
        goto err_label;
      }
    }
    else
    {
      QCRIL_LOG_DEBUG("ignoring key[%s] value[%s]", key_name, value);
    }

    ret = SUCCESS;
  }
  else
  {
    QCRIL_LOG_ERROR( "failed to parse key-value token: %s", data );
  }

err_label:
  QCRIL_DATA_RELEASE_STORAGE( buffer );
  QCRIL_LOG_EXIT;
  return ret;
} /* qcril_data_parse_qos_key_value() */


/*===========================================================================

  FUNCTION: qcril_data_process_qos_spec

===========================================================================*/
/*!
    @brief
    Parse the QOS specification to populate the DSI QOS data structure.

    @pre

    @return
    SUCCESS
    FAILURE
*/
/*=========================================================================*/
static int qcril_data_parse_qos_spec
(
  const qcril_request_params_type *const params_ptr,
  dsi_qos_spec_type               *qos_spec_ptr,
  qcril_parsing_context_s_type    *context_ptr,
  unsigned int                     records
)
{
  const char  **data;
  char         *buffer = NULL;
  char         *token = NULL;
  char         *save_ptr = NULL;
  int           ret = FAILURE;
  unsigned int  i = 0;
  unsigned int  cnt = 0;

  QCRIL_LOG_ENTRY;

  if ( (params_ptr == NULL) || (qos_spec_ptr == NULL) ||
         (context_ptr == NULL) )
  {
    QCRIL_LOG_ERROR( "BAD input, params_ptr [%#x], qos_spec_ptr [%#x], "
                     "context_ptr [%#x]", params_ptr,
                     qos_spec_ptr, context_ptr);
    return ret;
  }

  data = params_ptr->data;

  /****************************************************
   Parsing Algorithm:
   * Loop over number of data[] records, assuming paired
     records for Tx/Rx portion of QOS spec.
   * Tokenize data[i] by ','
     * Loop over set of tokens
     * If spec_index changes, increment QOS spec count,
       update flow/filter counts if valid
     * Tokenize token by '='
     * Lookup key in handler table
     * Invoke handler, passing key, value string and QMI spec struct
       * Translate string to required type
       * Populate QOS spec struct element
       * Update parsing context for valid flow/filter
   * On any error, return GENERIC_FAILURE to caller
  ****************************************************/

  /* Initialize parsing context structure */
  memset( context_ptr, 0x0, sizeof(qcril_parsing_context_s_type) );
  context_ptr->prev_spec_id   = DS_RIL_SPEC_ID_INVALID;
  context_ptr->direction      = INVALID;

  /* Loop over list of data strings containing QOS specs.
   * Start at element 1 per RIL API. */
  QCRIL_LOG_DEBUG("parsing records[%d]", (records-1));
  for( i = 1; i < records; i++ )
  {
    QCRIL_LOG_VERBOSE("parsing record[%d] len=%d: %s", i, strlen(data[i]), data[i]);
    if( 0 == strlen(data[i]) )
    {
      QCRIL_LOG_ERROR( "%s", "NULL string detected, aborting" );
      goto err_label;
    }

    /* Copy source data into working buffer */
    QCRIL_DATA_ALLOC_STORAGE( buffer, (strlen(data[i])+1), err_label );
    memcpy( buffer, data[i], (strlen( data[i])+1) );

    /* Tokenize the buffer using QOS spec delimiter, and process each
     * token until list exhausted.*/
    token = strtok_r( buffer, QCRIL_QOS_SPEC_DELIMITER, &save_ptr );
    while( token )
    {
      /* Process the key=value token */
      QCRIL_LOG_VERBOSE("processing token: %s", token);
      if( SUCCESS != qcril_data_parse_qos_key_value( token,
                                                     qos_spec_ptr,
                                                     context_ptr ) )
      {
        goto err_label;
      }

      /* Advance to next token */
      token = strtok_r( NULL, QCRIL_QOS_SPEC_DELIMITER, &save_ptr );
    }
    QCRIL_DATA_RELEASE_STORAGE( buffer );
  }
  ret = SUCCESS;

  /* Update QOS spec for last parsing iteration */
  qcril_data_update_qos_spec_counts( context_ptr, qos_spec_ptr );

err_label:
  QCRIL_DATA_RELEASE_STORAGE( buffer );
  QCRIL_LOG_EXIT;
  return ret;
} /* qcril_data_parse_qos_spec() */


/*===========================================================================

  FUNCTION: qcril_data_generate_qos_flow_string

===========================================================================*/
/*!
    @brief
    Generate the QOS spec strings in RIL format from the specified
    QMI data structure.

    @pre

    @return
    SUCCESS
    FAILURE
*/
/*=========================================================================*/
static int qcril_data_generate_qos_flow_string
(
  const dsi_qos_granted_flow_type   *qos_flow_ptr,
  char                              *flow_str,
  unsigned int                      *len_ptr
)
{
  int           ret = FAILURE;
  char         *sptr = flow_str;
  unsigned int  size = 0;
  int           len = 0;
  qcril_parsing_hndlr_s_type  *parse_ptr = NULL;

  QCRIL_LOG_ENTRY;

  if ( (qos_flow_ptr == NULL) || (flow_str == NULL) ||
         (len_ptr == NULL) )
  {
    QCRIL_LOG_ERROR( "BAD input, qos_flow_ptr [%#x], flow_str [%#x], "
                     "len_ptr [%#x]", qos_flow_ptr,
                     flow_str, len_ptr);
    return ret;
  }

  size = *len_ptr;

  /* 3GPP parameters*/
  QCRIL_DATA_GEN_FLOW_STRING( QMI_QOS_UMTS_FLOW_PARAM_TRAFFIC_CLASS,
                              qos_flow_ptr->qos_flow_granted.umts_flow_desc.param_mask,
                              RIL_QOS_FLOW_TRAFFIC_CLASS,
                              qos_flow_ptr->qos_flow_granted.umts_flow_desc.traffic_class );

  QCRIL_DATA_GEN_FLOW_STRING( QMI_QOS_UMTS_FLOW_PARAM_DATA_RATE,
                              qos_flow_ptr->qos_flow_granted.umts_flow_desc.param_mask,
                              RIL_QOS_FLOW_DATA_RATE_MIN,
                              qos_flow_ptr->qos_flow_granted.umts_flow_desc.data_rate.guaranteed_rate );
  QCRIL_DATA_GEN_FLOW_STRING( QMI_QOS_UMTS_FLOW_PARAM_DATA_RATE,
                              qos_flow_ptr->qos_flow_granted.umts_flow_desc.param_mask,
                              RIL_QOS_FLOW_DATA_RATE_MAX,
                              qos_flow_ptr->qos_flow_granted.umts_flow_desc.data_rate.max_rate );

  QCRIL_DATA_GEN_FLOW_STRING( QMI_QOS_UMTS_FLOW_PARAM_MAX_DELAY,
                              qos_flow_ptr->qos_flow_granted.umts_flow_desc.param_mask,
                              RIL_QOS_FLOW_LATENCY,
                              qos_flow_ptr->qos_flow_granted.umts_flow_desc.max_delay );

  /* 3GPP2 parameters*/
  QCRIL_DATA_GEN_FLOW_STRING( QMI_QOS_CDMA_FLOW_PARAM_PROFILE_ID,
                              qos_flow_ptr->qos_flow_granted.cdma_flow_desc.param_mask,
                              RIL_QOS_FLOW_3GPP2_PROFILE_ID,
                              qos_flow_ptr->qos_flow_granted.cdma_flow_desc.profile_id );

  /* Note: this paramater is in UMTS structure for some reason */
  QCRIL_DATA_GEN_FLOW_STRING( QMI_QOS_UMTS_FLOW_PARAM_3GPP2_FLOW_PRIO,
                              qos_flow_ptr->qos_flow_granted.umts_flow_desc.param_mask,
                              RIL_QOS_FLOW_3GPP2_PRIORITY,
                              qos_flow_ptr->qos_flow_granted.umts_flow_desc.flow_priority_3gpp2 );

  /* Pass back the update string */
  *len_ptr = size;

  ret = SUCCESS;
  QCRIL_LOG_EXIT;
  return ret;
} /* qcril_data_generate_qos_flow_string */

/*===========================================================================

  FUNCTION: qcril_data_generate_qos_flow_strings

===========================================================================*/
/*!
    @brief
    Generate the QOS spec strings in RIL format from the specified
    QMI data structure.  This routine dynamically allocates the strings
    from heap; it is callers responsibility to release memory.

    @pre

    @return
    SUCCESS
    FAILURE
*/
/*=========================================================================*/
static int qcril_data_generate_qos_flow_strings
(
  const dsi_qos_granted_info_type   *qos_info_ptr,
  char                             **tx_flow_str,
  char                             **rx_flow_str
)
{
  int           ret = FAILURE;
  unsigned int  size = 0;
  unsigned int  len = 0;
  char         *sptr = NULL;
  qcril_parsing_hndlr_s_type *parse_ptr = NULL;

  QCRIL_LOG_ENTRY;

  if ( (qos_info_ptr == NULL) || (tx_flow_str == NULL) ||
         (rx_flow_str == NULL) )
  {
    QCRIL_LOG_ERROR( "BAD input, qos_info_ptr [%#x], tx_flow_str [%#x], "
                     "rx_flow_str [%#x]", qos_info_ptr,
                     tx_flow_str, rx_flow_str);
    return ret;
  }

  *tx_flow_str = *rx_flow_str = NULL;

  /* Check for valid TX flow spec */
  if( qos_info_ptr->tx_granted_flow_data_is_valid )
  {
    size = QCRIL_QOS_FLOW_STR_SIZE;
    sptr = *tx_flow_str = malloc( size );
    if( NULL == sptr )
    {
      QCRIL_LOG_ERROR( "%s", "failed to allocate string buffer" );
      goto err_label;
    }
    memset( sptr, 0x0, size );

    parse_ptr = qcril_data_find_parse_rec_by_key( RIL_QOS_SPEC_INDEX );
    if( NULL == parse_ptr )
    {
      QCRIL_LOG_ERROR( "%s", "failed to find TX parse record" );
      if( *tx_flow_str ) free( *tx_flow_str );
      goto err_label;
    }

    len = snprintf( sptr, size, "%s=%d,", parse_ptr->key_name,
                    qos_info_ptr->tx_granted_flow_data.ip_flow_index );
    size -= len;
    sptr += len;

    parse_ptr = qcril_data_find_parse_rec_by_key( RIL_QOS_FLOW_DIRECTION );
    if( NULL == parse_ptr )
    {
      QCRIL_LOG_ERROR( "%s", "failed to find TX parse record" );
      if( *tx_flow_str ) free( *tx_flow_str );
      goto err_label;
    }

    len = snprintf( sptr, size, "%s=%d,", parse_ptr->key_name, RIL_QOS_TX );
    size -= len;
    sptr += len;

    if( SUCCESS !=
        qcril_data_generate_qos_flow_string( &qos_info_ptr->tx_granted_flow_data,
                                             sptr,
                                             &size ) )
    {
      QCRIL_LOG_ERROR( "%s", "failed to generate TX flow string" );
      if( *tx_flow_str ) free( *tx_flow_str );
      goto err_label;
    }

    /* Remove trailing comma */
    len = strlen(*tx_flow_str)-1;
    (*tx_flow_str)[len] = '\0';
  }

  /* Check for valid RX flow spec */
  if( qos_info_ptr->rx_granted_flow_data_is_valid )
  {
    size = QCRIL_QOS_FLOW_STR_SIZE;
    sptr = *rx_flow_str = malloc( size );
    if (sptr == NULL)
    {
      QCRIL_LOG_ERROR( "%s", "Failed to allocate memory for rx flow spec" );
      goto err_label;
    }
    memset( sptr, 0x0, size );

    parse_ptr = qcril_data_find_parse_rec_by_key( RIL_QOS_SPEC_INDEX );
    if( NULL == parse_ptr )
    {
      QCRIL_LOG_ERROR( "%s", "failed to find RX parse record" );
      if( *rx_flow_str ) free( *rx_flow_str );
      goto err_label;
    }

    len = snprintf( sptr, size, "%s=%d,", parse_ptr->key_name,
                    qos_info_ptr->rx_granted_flow_data.ip_flow_index );
    size -= len;
    sptr += len;

    parse_ptr = qcril_data_find_parse_rec_by_key( RIL_QOS_FLOW_DIRECTION );
    if( NULL == parse_ptr )
    {
      QCRIL_LOG_ERROR( "%s", "failed to find RX parse record" );
      if( *rx_flow_str ) free( *rx_flow_str );
      goto err_label;
    }

    len = snprintf( sptr, size, "%s=%d,", parse_ptr->key_name, RIL_QOS_RX );
    size -= len;
    sptr += len;

    if( SUCCESS !=
        qcril_data_generate_qos_flow_string( &qos_info_ptr->rx_granted_flow_data,
                                             sptr,
                                             &size ) )
    {
      QCRIL_LOG_ERROR( "%s", "failed to generate RX flow string" );
      if( *rx_flow_str ) free( *rx_flow_str );
      goto err_label;
    }

    /* Remove trailing comma */
    len = strlen(*rx_flow_str)-1;
    (*rx_flow_str)[len] = '\0';
  }

  ret = SUCCESS;

 err_label:
  QCRIL_LOG_EXIT;
  return ret;
}/* qcril_data_generate_qos_spec_string */




/*=========================================================================
  FUNCTION:  qcril_data_unsol_qos_state_changed

===========================================================================*/
/*!
    @brief
    send RIL_UNSOL_QOS_STATE_CHANGED_IND to QCRIL

    @return
    None
*/
/*=========================================================================*/
void qcril_data_unsol_qos_state_changed
(
  qcril_instance_id_e_type instance_id,
  void                    *response,
  size_t                   response_len
)
{
  qcril_unsol_resp_params_type unsol_resp;
  int           ret = SUCCESS;

  QCRIL_LOG_ENTRY;
  QCRIL_LOG_DEBUG( "%s", "sending RIL_UNSOL_QOS_STATE_CHANGED_IND" );

  qcril_default_unsol_resp_params( instance_id, (int)RIL_UNSOL_QOS_STATE_CHANGED_IND, &unsol_resp );
  unsol_resp.resp_pkt = (void*)response;
  unsol_resp.resp_len = response_len;

  qcril_send_unsol_response( &unsol_resp );

  QCRIL_LOG_EXIT;
  return;

} /* qcril_data_unsol_qos_state_changed() */


/*===========================================================================

                        FUNCTIONS REGISTERED WITH QCRIL

===========================================================================*/



/*===========================================================================

  FUNCTION: qcril_data_request_setup_qos

===========================================================================*/
/*!
    @brief
    Handles RIL_REQUEST_SETUP_QOS

    @pre
    Before calling, info_tbl_mutex must not be locked by the calling thread

    @return
    None.
*/
/*=========================================================================*/
void qcril_data_request_setup_qos
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr /*!< Output parameter */
)
{
  qcril_data_call_response_type  response;
  size_t                         response_len = 0;
  qcril_instance_id_e_type  instance_id;
  qcril_modem_id_e_type     modem_id;
  dsi_qos_req_opcode_type   req_opcode;
  dsi_qos_id_type          *qos_id_list = NULL;
  dsi_qos_err_rsp_type     *qos_spec_err_list = NULL;
  dsi_qos_spec_type        *qos_spec_ptr = NULL;
  dsi_qos_flow_req_type    *qos_tx_flow_list = NULL;
  dsi_qos_flow_req_type    *qos_rx_flow_list = NULL;
  dsi_qos_filter_req_type  *qos_tx_filter_list = NULL;
  dsi_qos_filter_req_type  *qos_rx_filter_list = NULL;
  int                       cid;
  int                       ret = FAILURE;
  unsigned int              records;
  unsigned int              spec_cnt = 0;
  unsigned int              i,j;
  unsigned int              pend_req = DS_RIL_REQ_INVALID;
  qcril_reqlist_u_type      u_info;
  qcril_reqlist_public_type reqlist_entry;
  qcril_parsing_context_s_type  context;
  char                     *bad_ptr = NULL;

  QCRIL_LOG_ENTRY;

  /* Validate input parameters*/
  QCRIL_DS_ASSERT( params_ptr != NULL, "validate params_ptr" );
  QCRIL_DS_ASSERT( ret_ptr    != NULL, "validate ret_ptr" );
  QCRIL_DS_ASSERT ( ( params_ptr != NULL) && ( ( params_ptr->datalen % sizeof(char*) ) == 0 ), "validate datalen" );
  if( ( params_ptr == NULL ) ||
      ( params_ptr->data == NULL ) ||
      ( ret_ptr == NULL ) ||
      ( ( params_ptr->datalen % sizeof(char*) ) != 0) )
  {
    QCRIL_LOG_ERROR( "BAD input");
    goto err_bad_input;
  }

  instance_id = params_ptr->instance_id;
  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  if( instance_id >= QCRIL_MAX_INSTANCE_ID )
  {
    goto err_bad_input;
  }

  memset( &response, 0x0, sizeof(response) );
  response_len = sizeof(response.qos_resp.setup);

  QCRIL_DATA_MUTEX_LOCK(&info_tbl_mutex);

  /* Check mandatory parameters: callID, single QOS spec */
  if( params_ptr->datalen < DS_RIL_MIN_QOS_SETUP_PARAMS )
  {
    QCRIL_LOG_ERROR( "BAD input, datalen [%d] < [%d]",
                     params_ptr->datalen, DS_RIL_MIN_QOS_SETUP_PARAMS );
    goto err_label2;
  }

  modem_id = params_ptr->modem_id;
  QCRIL_ASSERT( modem_id < QCRIL_MAX_MODEM_ID );
  if( modem_id >= QCRIL_MAX_MODEM_ID )
  {
    goto err_label2;
  }

  /* Extract call ID and find data call instance */
  cid = strtol( ((const char **)params_ptr->data)[0], &bad_ptr, 0 );
  if( *bad_ptr != 0 )
  {
    QCRIL_LOG_ERROR( "BAD input, conversion error on [%s] at [%s]",
                     ((const char **)params_ptr->data)[0], bad_ptr );
    goto err_label2;
  }
  QCRIL_LOG_DEBUG( "RIL says CID [%d], len [%d]",
                   cid, params_ptr->datalen );

  for( i = 0; i < MAX_CONCURRENT_UMTS_DATA_CALLS; i++ )
  {
    QCRIL_LOG_DEBUG("info_tbl CID [%d], index [%d]", info_tbl[i].cid, i);

    if( ( VALIDATE_LOCAL_DATA_OBJ( &info_tbl[i] ) ) && ( info_tbl[i].cid == cid ) )
    {
      QCRIL_LOG_DEBUG("found matching CID [%d], index [%d]", cid, i);
      break;
    }
  }/* for() */

  if( i == MAX_CONCURRENT_UMTS_DATA_CALLS )
  {
    QCRIL_LOG_ERROR( "no valid CID [%d] match found", cid );
    goto err_label2;
  }


  /* Check whether another REQ is pending for this call */
  if( qcril_data_util_is_req_pending( &info_tbl[i], &pend_req ) )
  {
    QCRIL_LOG_INFO( "UNEXPECTED RIL REQ pend [%s], cid [%d], index [%d]",
                    qcril_log_lookup_event_name( pend_req ), info_tbl[i].cid, info_tbl[i].index );
    /* if request is already pending, ignore another one */
    if (pend_req == RIL_REQUEST_SETUP_QOS)
    {
        QCRIL_LOG_INFO("RIL_REQUEST_SETUP_QOS already pending, cid [%d], index [%d]",
                          info_tbl[i].cid, info_tbl[i].index);
    }
    goto err_label;
  }

  /* Assume data[] elements are QOS specs in TX/RX pairs, per RIL API contract */
  /* QOS specs' start at data[1] */
  records = params_ptr->datalen / sizeof(char*);
  spec_cnt = (records-1) / 2;

  if( DS_RIL_MAX_QOS_SPECS_PER_REQ < spec_cnt )
  {
    QCRIL_LOG_ERROR( "too many QOS specs in request[%d], limit[%d]",
                     spec_cnt, DS_RIL_MAX_QOS_SPECS_PER_REQ );
    goto err_label;
  }
  /* Check for unpaired records */
  if( 0 != (records-1)%2 )
  {
    QCRIL_LOG_INFO( "%s", "warning: unpaired records for QOS spec, incrementing count" );
    spec_cnt++;
  }
  /* Check if no specs are present at all */
  if( 0 == spec_cnt )
  {
    QCRIL_LOG_ERROR( "%s", "no QOS specs in request, aborting" );
    goto err_label;
  }
  context.spec_cnt = spec_cnt;

  /* Allocate storage for QOS specification elements. */
  QCRIL_DATA_ALLOC_STORAGE( qos_spec_ptr, sizeof(dsi_qos_spec_type) * spec_cnt, err_label );
  memset( qos_spec_ptr, 0x0, (sizeof(dsi_qos_spec_type) * spec_cnt) );

  QCRIL_DATA_ALLOC_STORAGE( qos_tx_flow_list,
                            (sizeof(dsi_qos_flow_req_type) * DS_RIL_MAX_QOS_TX_FLOWS_PER_SPEC * spec_cnt),
                            err_label );
  memset( qos_tx_flow_list, 0x0, (sizeof(dsi_qos_flow_req_type) * DS_RIL_MAX_QOS_TX_FLOWS_PER_SPEC * spec_cnt) );

  QCRIL_DATA_ALLOC_STORAGE( qos_rx_flow_list,
                            (sizeof(dsi_qos_flow_req_type) * DS_RIL_MAX_QOS_RX_FLOWS_PER_SPEC * spec_cnt),
                            err_label );
  memset( qos_rx_flow_list, 0x0, (sizeof(dsi_qos_flow_req_type) * DS_RIL_MAX_QOS_RX_FLOWS_PER_SPEC * spec_cnt) );

  QCRIL_DATA_ALLOC_STORAGE( qos_tx_filter_list,
                            (sizeof(dsi_qos_filter_req_type) * DS_RIL_MAX_QOS_TX_FILTERS_PER_SPEC * spec_cnt),
                            err_label );
  memset( qos_tx_filter_list, 0x0, (sizeof(dsi_qos_filter_req_type) * DS_RIL_MAX_QOS_TX_FILTERS_PER_SPEC * spec_cnt) );

  QCRIL_DATA_ALLOC_STORAGE( qos_rx_filter_list,
                            (sizeof(dsi_qos_filter_req_type) * DS_RIL_MAX_QOS_RX_FILTERS_PER_SPEC * spec_cnt),
                            err_label );
  memset( qos_rx_filter_list, 0x0, (sizeof(dsi_qos_filter_req_type) * DS_RIL_MAX_QOS_RX_FILTERS_PER_SPEC * spec_cnt) );

  /* Setup QOS spec array */
  for( j=0; j < spec_cnt; j++ )
  {
    qos_spec_ptr[j].tx_flow_req_array = &qos_tx_flow_list[j];
    qos_spec_ptr[j].rx_flow_req_array = &qos_rx_flow_list[j];
    qos_spec_ptr[j].tx_filter_req_array = &qos_tx_filter_list[j];
    qos_spec_ptr[j].rx_filter_req_array = &qos_rx_filter_list[j];
  }

  /* Process the request data[] array */
  if( SUCCESS != qcril_data_parse_qos_spec( params_ptr, qos_spec_ptr, &context, records ) )
  {
    QCRIL_LOG_ERROR( "%s", "failed to parse QOS spec" );
    goto err_label;
  }

  QCRIL_DATA_ALLOC_STORAGE( qos_id_list,
                            (sizeof(dsi_qos_id_type) * spec_cnt),
                            err_label );
  QCRIL_DATA_ALLOC_STORAGE( qos_spec_err_list,
                            (sizeof(dsi_qos_err_rsp_type) * spec_cnt),
                            err_label );
  memset( qos_id_list, INVALID, (sizeof(dsi_qos_id_type) * spec_cnt) );
  memset( qos_spec_err_list, INVALID, (sizeof(dsi_qos_err_rsp_type) * spec_cnt) );

  /* Start new RIL transaction, which is released in response generation */
  memset( &u_info, 0x0, sizeof( qcril_reqlist_u_type ) );
  u_info.ds.info = (void *)&info_tbl[i];
  qcril_reqlist_default_entry( params_ptr->t,
                               params_ptr->event_id,
                               modem_id,
                               QCRIL_REQ_AWAITING_MORE_AMSS_EVENTS,
                               QCRIL_EVT_DATA_EVENT_CALLBACK,
                               &u_info,
                               &reqlist_entry );
  if( E_SUCCESS != qcril_reqlist_new( instance_id, &reqlist_entry ) )
  {
    QCRIL_LOG_ERROR( "%s", "qcril_reqlist_new" );
    goto err_label;
  }

  info_tbl[i].pend_tok    = params_ptr->t;
  info_tbl[i].pend_req    = params_ptr->event_id;
  QCRIL_LOG_INFO( "DEBUG: %s step %d - info_tbl_ptr->pend_tok[0x%x] info_tbl_ptr->pend_req[0x%X]",
                  __func__, 0, info_tbl[i].pend_tok, info_tbl[i].pend_req );
  QCRIL_LOG_VERBOSE( "%s", "reqlist node insert complete, request QOS setup" );

  /* Invoke DSI API to send command to Modem */
  if( DSI_SUCCESS != dsi_request_qos( info_tbl[i].dsi_hndl,
                                      spec_cnt,
                                      qos_spec_ptr,
                                      DSI_QOS_REQUEST,
                                      qos_id_list,
                                      qos_spec_err_list ) )
  {
    QCRIL_LOG_ERROR( "%s", "failed on dsi_request_qos" );
    QCRIL_DATA_RELEASE_STORAGE( qos_id_list );
    (void)qcril_reqlist_free( modem_id, params_ptr->t );
    goto err_label;
  }

  /* Update call record QOS flow ID list */
  if( SUCCESS != qcril_data_qos_add_flows( &info_tbl[i], qos_id_list, spec_cnt ) )
  {
    QCRIL_LOG_ERROR( "%s", "failed on qcril_data_qos_add_flows" );
    QCRIL_DATA_RELEASE_STORAGE( qos_id_list );
    (void)qcril_reqlist_free( modem_id, params_ptr->t );
    goto err_label;
  }

  /* Generate command response to upper layers */
  asprintf( &response.qos_resp.setup.return_code, "%d", RIL_E_SUCCESS );
  response_len = sizeof(char*);

  for( j=0; j < spec_cnt; j++ )
  {
    /* Report flow ID for each QOS spec */
    asprintf( &response.qos_resp.setup.qos_flow_id[j], "%d", (int)qos_id_list[j] );
    response_len += sizeof(char*);
  }

  qcril_data_response_success( instance_id,
                               params_ptr->t,
                               params_ptr->event_id,
                               &response.qos_resp,
                               response_len );

  /* Release dynamic memory */
  QCRIL_DATA_RELEASE_STORAGE( response.qos_resp.setup.return_code );
  for( j=0; j < spec_cnt; j++ )
  {
    QCRIL_DATA_RELEASE_STORAGE( response.qos_resp.setup.qos_flow_id[j] );
  }

  /* Modem QOS status indications report outcome of network negotiation. */
  ret = SUCCESS;

err_label:
  if( NULL != info_tbl[i].pend_tok )
  {
    info_tbl[i].pend_tok   = NULL;
    info_tbl[i].pend_req   = DS_RIL_REQ_INVALID;  /* is there a RIL_REQUEST_INVALID */
  }

err_label2:
  /* Post error to RIL framework */
  if( SUCCESS != ret )
  {
    QCRIL_LOG_DEBUG("%s", "respond to QCRIL with error");

    memset( &response, 0x0, sizeof(response) );
    response_len = sizeof(response.qos_resp.setup);

    asprintf( &response.qos_resp.setup.return_code, "%d", RIL_E_GENERIC_FAILURE );
    qcril_data_response_success( instance_id,
                                 params_ptr->t,
                                 params_ptr->event_id,
                                 &response.qos_resp,
                                 response_len );
    QCRIL_DATA_RELEASE_STORAGE( response.qos_resp.setup.return_code );
  }

  /* Cleanup dynamic memory */
  QCRIL_DATA_RELEASE_STORAGE( qos_spec_err_list );
  QCRIL_DATA_RELEASE_STORAGE( qos_rx_filter_list );
  QCRIL_DATA_RELEASE_STORAGE( qos_tx_filter_list );
  QCRIL_DATA_RELEASE_STORAGE( qos_rx_flow_list );
  QCRIL_DATA_RELEASE_STORAGE( qos_tx_flow_list );
  QCRIL_DATA_RELEASE_STORAGE( qos_spec_ptr );

  QCRIL_DATA_MUTEX_UNLOCK(&info_tbl_mutex);

err_bad_input:
  QCRIL_LOG_EXIT;
  return;
} /* qcril_data_request_setup_qos() */

/*===========================================================================

  FUNCTION: qcril_data_request_release_qos

===========================================================================*/
/*!
    @brief
    Handles RIL_REQUEST_RELEASE_QOS

    @pre
    Before calling, info_tbl_mutex must not be locked by the calling thread

    @return
    None.
*/
/*=========================================================================*/
void qcril_data_request_release_qos
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr /*!< Output parameter */
)
{
  qcril_data_call_response_type  response;
  size_t                         response_len = 0;
  qcril_instance_id_e_type  instance_id;
  qcril_modem_id_e_type     modem_id;
  dsi_qos_id_type           flowid;
  int                       ret = FAILURE;
  int                       i;
  unsigned int              pend_req = DS_RIL_REQ_INVALID;
  qcril_data_call_info_tbl_type  *info_tbl_ptr = NULL;
  qcril_reqlist_u_type      u_info;
  qcril_reqlist_public_type reqlist_entry;
  char                     *bad_ptr = NULL;

  QCRIL_LOG_ENTRY;

  /* Validate input parameters*/
  QCRIL_DS_ASSERT( params_ptr != NULL, "validate params_ptr" );
  QCRIL_DS_ASSERT( ret_ptr    != NULL, "validate ret_ptr" );
  QCRIL_DS_ASSERT ( ( params_ptr != NULL) && ( ( params_ptr->datalen % sizeof(char*) ) == 0 ), "validate datalen" );
  if( ( params_ptr == NULL ) ||
      ( params_ptr->data == NULL ) ||
      ( ret_ptr == NULL ) ||
      ( ( params_ptr->datalen % sizeof(char*) ) != 0) )
  {
    QCRIL_LOG_ERROR( "BAD input, params_ptr [%p], ret_ptr [%p], datalen [%d]",
                     params_ptr, ret_ptr,
                     params_ptr ? params_ptr->datalen : 0 );
    goto err_bad_input;
  }

  instance_id = params_ptr->instance_id;
  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  if( instance_id >= QCRIL_MAX_INSTANCE_ID )
  {
    goto err_bad_input;
  }

  memset( &response, 0x0, sizeof(response) );
  response_len = sizeof(response.qos_resp.result);

  QCRIL_DATA_MUTEX_LOCK(&info_tbl_mutex);

  /* Check mandatory parameters: QOS flow ID */
  if( params_ptr->datalen < DS_RIL_MIN_QOS_RELEASE_PARAMS )
  {
    QCRIL_LOG_ERROR( "BAD input, datalen [%d] < [%d]",
                     params_ptr->datalen, DS_RIL_MIN_QOS_RELEASE_PARAMS );
    goto err_label;
  }

  modem_id = params_ptr->modem_id;
  QCRIL_ASSERT( modem_id < QCRIL_MAX_MODEM_ID );
  if( modem_id >= QCRIL_MAX_MODEM_ID )
  {
    goto err_label;
  }

  flowid = strtol( ((const char **)params_ptr->data)[0], &bad_ptr, 0 );
  if( *bad_ptr != 0 )
  {
    QCRIL_LOG_ERROR( "BAD input, conversion error on [%s] at [%s]",
                     ((const char **)params_ptr->data)[0], bad_ptr );
    goto err_label;
  }
  QCRIL_LOG_DEBUG( "RIL says QOS FlowID [0x%08x], len [%d]",
                   flowid, params_ptr->datalen );

  info_tbl_ptr = qcril_data_find_call_by_flowid( flowid );
  if( NULL == info_tbl_ptr )
  {
    QCRIL_LOG_ERROR( "no valid QOS flow ID [0x%x] match found", flowid );
    goto err_label;
  }

  /* Check whether another REQ is pending for this call */
  if( qcril_data_util_is_req_pending( info_tbl_ptr, &pend_req ) )
  {
    QCRIL_LOG_INFO( "UNEXPECTED RIL REQ pend [%s], cid [%d], index [%d]",
                    qcril_log_lookup_event_name( pend_req ), info_tbl_ptr->cid, info_tbl_ptr->index );
    /* if request is already pending, ignore another one */
    if( pend_req == RIL_REQUEST_RELEASE_QOS )
    {
      QCRIL_LOG_INFO( "RIL_REQUEST_RELEASE_QOS already pending, cid [%d], index [%d]",
                      info_tbl_ptr->cid, info_tbl_ptr->index);
    }
    goto err_label;
  }

  /* Start new RIL transaction, which is released in response generation */
  memset( &u_info, 0x0, sizeof( qcril_reqlist_u_type ) );
  u_info.ds.info = (void *)info_tbl_ptr;
  qcril_reqlist_default_entry( params_ptr->t,
                               params_ptr->event_id,
                               modem_id,
                               QCRIL_REQ_AWAITING_MORE_AMSS_EVENTS,
                               QCRIL_EVT_DATA_EVENT_CALLBACK,
                               &u_info,
                               &reqlist_entry );
  if( E_SUCCESS != qcril_reqlist_new( instance_id, &reqlist_entry ) )
  {
    QCRIL_LOG_ERROR( "%s", "qcril_reqlist_new" );
    goto err_label;
  }

  info_tbl_ptr->pend_tok    = params_ptr->t;
  info_tbl_ptr->pend_req    = params_ptr->event_id;
  QCRIL_LOG_INFO( "DEBUG: %s step %d - info_tbl_ptr->pend_tok[0x%x] info_tbl_ptr->pend_req[0x%X]",
                  __func__, 0, info_tbl_ptr->pend_tok, info_tbl_ptr->pend_req );
  QCRIL_LOG_VERBOSE( "%s", "reqlist node insert complete, request QOS release" );

  /* Invoke DSI API to send command to Modem */
  if( DSI_SUCCESS != dsi_release_qos( info_tbl_ptr->dsi_hndl,
                                      1, &flowid ) )
  {
    QCRIL_LOG_ERROR( "%s", "failed on dsi_release_qos" );
    (void)qcril_reqlist_free( modem_id, params_ptr->t );
    goto err_label;
  }

  /* Flow is removed from call object when async event arrives */

  /* Generate command response to upper layers */
  asprintf( &response.qos_resp.result.return_code, "%d", RIL_E_SUCCESS );

  qcril_data_response_success( instance_id,
                               params_ptr->t,
                               params_ptr->event_id,
                               &response.qos_resp,
                               response_len );

  /* Release dynamic memory */
  QCRIL_DATA_RELEASE_STORAGE( response.qos_resp.result.return_code );

  /* Modem QOS status indications report outcome of network negotiation. */
  ret = SUCCESS;

err_label:
  /* Post error to RIL framework */
  if( SUCCESS != ret )
  {
    QCRIL_LOG_DEBUG("%s", "respond to QCRIL with error");

    memset( &response, 0x0, sizeof(response) );
    response_len = sizeof(response.qos_resp.setup);

    asprintf( &response.qos_resp.result.return_code, "%d", RIL_E_GENERIC_FAILURE );
    qcril_data_response_success( instance_id,
                                 params_ptr->t,
                                 params_ptr->event_id,
                                 &response.qos_resp,
                                 response_len );
    QCRIL_DATA_RELEASE_STORAGE( response.qos_resp.result.return_code );
  }
  if( info_tbl_ptr && (NULL != info_tbl_ptr->pend_tok) )
  {
    info_tbl_ptr->pend_tok   = NULL;
    info_tbl_ptr->pend_req   = DS_RIL_REQ_INVALID;  /* is there a RIL_REQUEST_INVALID */
  }

  QCRIL_DATA_MUTEX_UNLOCK(&info_tbl_mutex);

err_bad_input:
  QCRIL_LOG_EXIT;
  return;
} /* qcril_data_request_release_qos() */


/*===========================================================================

  FUNCTION: qcril_data_request_modify_qos

===========================================================================*/
/*!
    @brief

    @pre

    @return
    None.
*/
/*=========================================================================*/
void qcril_data_request_modify_qos
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr /*!< Output parameter */
)
{
  qcril_data_call_response_type  response;
  size_t                         response_len = 0;
  qcril_instance_id_e_type  instance_id;
  qcril_modem_id_e_type     modem_id;
  dsi_qos_req_opcode_type   req_opcode;
  dsi_qos_err_rsp_type     *qos_spec_err_list = NULL;
  dsi_qos_spec_type        *qos_spec_ptr = NULL;
  qcril_data_call_info_tbl_type  *info_tbl_ptr = NULL;
  int                       ret = FAILURE;
  dsi_qos_id_type           flowid;
  unsigned int              records;
  unsigned int              spec_cnt = 0;
  unsigned int              i,j;
  unsigned int              pend_req = DS_RIL_REQ_INVALID;
  qcril_reqlist_u_type      u_info;
  qcril_reqlist_public_type reqlist_entry;
  qcril_parsing_context_s_type  context;
  char                     *bad_ptr = NULL;

  QCRIL_LOG_ENTRY;

  /* Validate input parameters*/
  QCRIL_DS_ASSERT( params_ptr != NULL, "validate params_ptr" );
  QCRIL_DS_ASSERT( ret_ptr    != NULL, "validate ret_ptr" );
  QCRIL_DS_ASSERT ( ( params_ptr != NULL) && ( ( params_ptr->datalen % sizeof(char*) ) == 0 ), "validate datalen" );
  if( ( params_ptr == NULL ) ||
      ( params_ptr->data == NULL ) ||
      ( ret_ptr == NULL ) ||
      ( ( params_ptr->datalen % sizeof(char*) ) != 0) )
  {
    QCRIL_LOG_ERROR( "BAD input, params_ptr [%p], ret_ptr [%p], datalen [%d]",
                     params_ptr, ret_ptr,
                     params_ptr ? params_ptr->datalen : 0 );
    goto err_bad_input;
  }

  instance_id = params_ptr->instance_id;
  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  if( instance_id >= QCRIL_MAX_INSTANCE_ID )
  {
    goto err_bad_input;
  }

  memset( &response, 0x0, sizeof(response) );
  response_len = sizeof(response.qos_resp.setup);

  QCRIL_DATA_MUTEX_LOCK(&info_tbl_mutex);

  /* Check mandatory parameters: callID, single QOS spec */
  if( params_ptr->datalen < DS_RIL_MIN_QOS_MODIFY_PARAMS )
  {
    QCRIL_LOG_ERROR( "BAD input, datalen [%d] < [%d]",
                     params_ptr->datalen, DS_RIL_MIN_QOS_MODIFY_PARAMS );
    goto err_label;
  }

  modem_id = params_ptr->modem_id;
  QCRIL_ASSERT( modem_id < QCRIL_MAX_MODEM_ID );
  if( modem_id >= QCRIL_MAX_MODEM_ID )
  {
    goto err_label;
  }

  flowid = strtol( ((const char **)params_ptr->data)[0], &bad_ptr, 0 );
  if( *bad_ptr != 0 )
  {
    QCRIL_LOG_ERROR( "BAD input, conversion error on [%s] at [%s]",
                     ((const char **)params_ptr->data)[0], bad_ptr );
    goto err_label;
  }
  QCRIL_LOG_DEBUG( "RIL says QOS FlowID [0x%08x], len [%d]",
                   flowid, params_ptr->datalen );

  info_tbl_ptr = qcril_data_find_call_by_flowid( flowid );
  if( NULL == info_tbl_ptr )
  {
    QCRIL_LOG_ERROR( "no valid QOS flow ID [0x%x] match found", flowid );
    goto err_label;
  }


  /* Check whether another REQ is pending for this call */
  if( qcril_data_util_is_req_pending( info_tbl_ptr, &pend_req ) )
  {
    QCRIL_LOG_INFO( "UNEXPECTED RIL REQ pend [%s], cid [%d], index [%d]",
                    qcril_log_lookup_event_name( pend_req ), info_tbl_ptr->cid, info_tbl_ptr->index );
    /* if request is already pending, ignore another one */
    if (pend_req == RIL_REQUEST_MODIFY_QOS)
    {
      QCRIL_LOG_INFO("RIL_REQUEST_SETUP_QOS already pending, cid [%d], index [%d]",
                     info_tbl_ptr->cid, info_tbl_ptr->index);
    }
    goto err_label;
  }

  /* Assume data[] elements are QOS specs in TX/RX pairs, per RIL API contract */
  /* QOS specs' start at data[1] */
  records = params_ptr->datalen / sizeof(char*);
  spec_cnt = (records-1) / 2;

  if( DS_RIL_MAX_QOS_SPECS_PER_REQ < spec_cnt )
  {
    QCRIL_LOG_ERROR( "too many QOS specs in request[%d], limit[%d]",
                     spec_cnt, DS_RIL_MAX_QOS_SPECS_PER_REQ );
    goto err_label;
  }
  /* Check for unpaired records */
  if( 0 != (records-1)%2 )
  {
    QCRIL_LOG_INFO( "%s", "warning: unpaired records for QOS spec, incrementing count" );
    spec_cnt++;
  }
  /* Check if no specs are present at all */
  if( 0 == spec_cnt )
  {
    QCRIL_LOG_ERROR( "%s", "no QOS specs in request, aborting" );
    goto err_label;
  }

  /* Verify only one QOS spec present for primary flow */
  if( (DS_RIL_FLOW_ID_PRIMARY == flowid) && (1 < spec_cnt) )
  {
    QCRIL_LOG_ERROR( "%s", "only one flow spec allowed for modify primary flow, aborting" );
    goto err_label;
  }

  /* Allocate storage for QOS specification elements. */
  QCRIL_DATA_ALLOC_STORAGE( qos_spec_ptr, sizeof(dsi_qos_spec_type) * spec_cnt, err_label );
  memset( qos_spec_ptr, 0x0, (sizeof(dsi_qos_spec_type) * spec_cnt) );

  /* Setup QOS spec array */
  for( j=0; j < spec_cnt; j++ )
  {
    QCRIL_DATA_ALLOC_STORAGE( qos_spec_ptr[j].tx_flow_req_array,
                              (sizeof(dsi_qos_flow_req_type) * DS_RIL_MAX_QOS_TX_FLOWS_PER_SPEC),
                              err_label );
    QCRIL_DATA_ALLOC_STORAGE( qos_spec_ptr[j].rx_flow_req_array,
                              (sizeof(dsi_qos_flow_req_type) * DS_RIL_MAX_QOS_RX_FLOWS_PER_SPEC),
                              err_label );
    QCRIL_DATA_ALLOC_STORAGE( qos_spec_ptr[j].tx_filter_req_array,
                              (sizeof(dsi_qos_filter_req_type) * DS_RIL_MAX_QOS_TX_FILTERS_PER_SPEC),
                              err_label );
    QCRIL_DATA_ALLOC_STORAGE( qos_spec_ptr[j].rx_filter_req_array,
                              (sizeof(dsi_qos_filter_req_type) * DS_RIL_MAX_QOS_RX_FILTERS_PER_SPEC),
                              err_label );
    memset( qos_spec_ptr[j].tx_flow_req_array, 0x0, (sizeof(dsi_qos_flow_req_type) * DS_RIL_MAX_QOS_TX_FLOWS_PER_SPEC) );
    memset( qos_spec_ptr[j].rx_flow_req_array, 0x0, (sizeof(dsi_qos_flow_req_type) * DS_RIL_MAX_QOS_RX_FLOWS_PER_SPEC) );
    memset( qos_spec_ptr[j].tx_filter_req_array, 0x0, (sizeof(dsi_qos_filter_req_type) * DS_RIL_MAX_QOS_TX_FILTERS_PER_SPEC) );
    memset( qos_spec_ptr[j].rx_filter_req_array, 0x0, (sizeof(dsi_qos_filter_req_type) * DS_RIL_MAX_QOS_RX_FILTERS_PER_SPEC) );
  }

  /* Process the request data[] array */
  if( SUCCESS != qcril_data_parse_qos_spec( params_ptr, qos_spec_ptr, &context, records ) )
  {
    QCRIL_LOG_ERROR( "%s", "failed to parse QOS spec" );
    goto err_label;
  }

  /* Only support modify single QOS flow currently */
  qos_spec_ptr->qos_identifier = flowid;

  QCRIL_DATA_ALLOC_STORAGE( qos_spec_err_list,
                            (sizeof(dsi_qos_err_rsp_type) * spec_cnt),
                            err_label );
  memset( qos_spec_err_list, INVALID, (sizeof(dsi_qos_err_rsp_type) * spec_cnt) );

  /* Start new RIL transaction, which is released in response generation */
  memset( &u_info, 0x0, sizeof( qcril_reqlist_u_type ) );
  u_info.ds.info = (void *)info_tbl_ptr;
  qcril_reqlist_default_entry( params_ptr->t,
                               params_ptr->event_id,
                               modem_id,
                               QCRIL_REQ_AWAITING_MORE_AMSS_EVENTS,
                               QCRIL_EVT_DATA_EVENT_CALLBACK,
                               &u_info,
                               &reqlist_entry );
  if( E_SUCCESS != qcril_reqlist_new( instance_id, &reqlist_entry ) )
  {
    QCRIL_LOG_ERROR( "%s", "qcril_reqlist_new" );
    goto err_label;
  }

  info_tbl_ptr->pend_tok    = params_ptr->t;
  info_tbl_ptr->pend_req    = params_ptr->event_id;
  QCRIL_LOG_INFO( "DEBUG: %s step %d - info_tbl_ptr->pend_tok[0x%x] info_tbl_ptr->pend_req[0x%X]",
                  __func__, 0, info_tbl_ptr->pend_tok, info_tbl_ptr->pend_req );
  QCRIL_LOG_VERBOSE( "%s", "reqlist node insert complete, request QOS setup" );

  /* Invoke DSI API to send command to Modem */
  if( DSI_SUCCESS != dsi_modify_qos( info_tbl_ptr->dsi_hndl,
                                     spec_cnt,
                                     qos_spec_ptr,
                                     qos_spec_err_list ) )
  {
    QCRIL_LOG_ERROR( "%s", "failed on dsi_request_qos" );
    (void)qcril_reqlist_free( modem_id, params_ptr->t );
    goto err_label;
  }

  /* Generate command response to upper layers */
  asprintf( &response.qos_resp.result.return_code, "%d", RIL_E_SUCCESS );
  response_len = sizeof(char*);

  qcril_data_response_success( instance_id,
                               params_ptr->t,
                               params_ptr->event_id,
                               &response.qos_resp,
                               response_len );

  /* Release dynamic memory */
  QCRIL_DATA_RELEASE_STORAGE( response.qos_resp.result.return_code );

  /* Modem QOS status indications report outcome of network negotiation. */
  ret = SUCCESS;

err_label:
  /* Post error to RIL framework */
  if( SUCCESS != ret )
  {
    QCRIL_LOG_DEBUG("%s", "respond to QCRIL with error");

    memset( &response, 0x0, sizeof(response) );
    response_len = sizeof(response.qos_resp.setup);

    asprintf( &response.qos_resp.result.return_code, "%d", RIL_E_GENERIC_FAILURE );
    qcril_data_response_success( instance_id,
                                 params_ptr->t,
                                 params_ptr->event_id,
                                 &response.qos_resp,
                                 response_len );
    QCRIL_DATA_RELEASE_STORAGE( response.qos_resp.result.return_code );
  }

  if( info_tbl_ptr && (NULL != info_tbl_ptr->pend_tok) )
  {
    info_tbl_ptr->pend_tok   = NULL;
    info_tbl_ptr->pend_req   = DS_RIL_REQ_INVALID;  /* is there a RIL_REQUEST_INVALID */
  }

  /* Cleanup dynamic memory */
  if( NULL != qos_spec_ptr )
  {
    for( j=0; j < spec_cnt; j++ )
    {
      QCRIL_DATA_RELEASE_STORAGE( qos_spec_ptr[j].rx_filter_req_array );
      QCRIL_DATA_RELEASE_STORAGE( qos_spec_ptr[j].tx_filter_req_array );
      QCRIL_DATA_RELEASE_STORAGE( qos_spec_ptr[j].rx_flow_req_array );
      QCRIL_DATA_RELEASE_STORAGE( qos_spec_ptr[j].tx_flow_req_array );
    }
  }
  QCRIL_DATA_RELEASE_STORAGE( qos_spec_err_list );
  QCRIL_DATA_RELEASE_STORAGE( qos_spec_ptr );

  QCRIL_DATA_MUTEX_UNLOCK(&info_tbl_mutex);

err_bad_input:
  QCRIL_LOG_EXIT;
  return;
} /* qcril_data_request_modify_qos() */


/*===========================================================================

  FUNCTION: qcril_data_request_suspend_qos

===========================================================================*/
/*!
    @brief

    @pre

    @return
    None.
*/
/*=========================================================================*/
void qcril_data_request_suspend_qos
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr /*!< Output parameter */
)
{
  qcril_data_call_response_type  response;
  size_t                         response_len = 0;
  qcril_instance_id_e_type  instance_id;
  qcril_modem_id_e_type     modem_id;
  dsi_qos_id_type           flowid;
  int                       ret = FAILURE;
  int                       i;
  unsigned int              pend_req = DS_RIL_REQ_INVALID;
  qcril_data_call_info_tbl_type  *info_tbl_ptr = NULL;
  qcril_reqlist_u_type      u_info;
  qcril_reqlist_public_type reqlist_entry;
  char                     *bad_ptr = NULL;

  QCRIL_LOG_ENTRY;

  /* Validate input parameters*/
  QCRIL_DS_ASSERT( params_ptr != NULL, "validate params_ptr" );
  QCRIL_DS_ASSERT( ret_ptr    != NULL, "validate ret_ptr" );
  QCRIL_DS_ASSERT ( ( params_ptr != NULL) && ( ( params_ptr->datalen % sizeof(char*) ) == 0 ), "validate datalen" );
  if( ( params_ptr == NULL ) ||
      ( params_ptr->data == NULL ) ||
      ( ret_ptr == NULL ) ||
      ( ( params_ptr->datalen % sizeof(char*) ) != 0) )
  {
    QCRIL_LOG_ERROR( "BAD input, params_ptr [%p], ret_ptr [%p], datalen [%d]",
                     params_ptr, ret_ptr,
                     params_ptr ? params_ptr->datalen : 0 );
    goto err_bad_input;
  }

  instance_id = params_ptr->instance_id;
  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  if( instance_id >= QCRIL_MAX_INSTANCE_ID )
  {
    goto err_bad_input;
  }

  memset( &response, 0x0, sizeof(response) );
  response_len = sizeof(response.qos_resp.result);

  QCRIL_DATA_MUTEX_LOCK(&info_tbl_mutex);

  /* Check mandatory parameters: QOS flow ID */
  if( params_ptr->datalen < DS_RIL_MIN_QOS_SUSPEND_PARAMS )
  {
    QCRIL_LOG_ERROR( "BAD input, datalen [%d] < [%d]",
                     params_ptr->datalen, DS_RIL_MIN_QOS_SUSPEND_PARAMS );
    goto err_label;
  }

  modem_id = params_ptr->modem_id;
  QCRIL_ASSERT( modem_id < QCRIL_MAX_MODEM_ID );
  if( modem_id >= QCRIL_MAX_MODEM_ID )
  {
    goto err_label;
  }

  flowid = strtol( ((const char **)params_ptr->data)[0], &bad_ptr, 0 );
  if( *bad_ptr != 0 )
  {
    QCRIL_LOG_ERROR( "BAD input, conversion error on [%s] at [%s]",
                     ((const char **)params_ptr->data)[0], bad_ptr );
    goto err_label;
  }
  QCRIL_LOG_DEBUG( "RIL says QOS FlowID [0x%08x], len [%d]",
                   flowid, params_ptr->datalen );

  info_tbl_ptr = qcril_data_find_call_by_flowid( flowid );
  if( NULL == info_tbl_ptr )
  {
    QCRIL_LOG_ERROR( "no valid QOS flow ID [0x%x] match found", flowid );
    goto err_label;
  }

  /* Check whether another REQ is pending for this call */
  if( qcril_data_util_is_req_pending( info_tbl_ptr, &pend_req ) )
  {
    QCRIL_LOG_INFO( "UNEXPECTED RIL REQ pend [%s], cid [%d], index [%d]",
                    qcril_log_lookup_event_name( pend_req ), info_tbl_ptr->cid, info_tbl_ptr->index );
    /* if request is already pending, ignore another one */
    if( pend_req == RIL_REQUEST_SUSPEND_QOS )
    {
        QCRIL_LOG_INFO( "RIL_REQUEST_SUSPEND_QOS already pending, cid [%d], index [%d]",
                        info_tbl_ptr->cid, info_tbl_ptr->index);
    }
    goto err_label;
  }

  /* Start new RIL transaction, which is released in response generation */
  memset( &u_info, 0x0, sizeof( qcril_reqlist_u_type ) );
  u_info.ds.info = (void *)info_tbl_ptr;
  qcril_reqlist_default_entry( params_ptr->t,
                               params_ptr->event_id,
                               modem_id,
                               QCRIL_REQ_AWAITING_MORE_AMSS_EVENTS,
                               QCRIL_EVT_DATA_EVENT_CALLBACK,
                               &u_info,
                               &reqlist_entry );
  if( E_SUCCESS != qcril_reqlist_new( instance_id, &reqlist_entry ) )
  {
    QCRIL_LOG_ERROR( "%s", "qcril_reqlist_new" );
    goto err_label;
  }

  info_tbl_ptr->pend_tok    = params_ptr->t;
  info_tbl_ptr->pend_req    = params_ptr->event_id;
  QCRIL_LOG_INFO( "DEBUG: %s step %d - info_tbl_ptr->pend_tok[0x%x] info_tbl_ptr->pend_req[0x%X]",
                  __func__, 0, info_tbl_ptr->pend_tok, info_tbl_ptr->pend_req );
  QCRIL_LOG_VERBOSE( "%s", "reqlist node insert complete, request QOS suspend" );

  /* Invoke DSI API to send command to Modem */
  if( DSI_SUCCESS != dsi_suspend_qos( info_tbl_ptr->dsi_hndl,
                                      1, &flowid ) )
  {
    QCRIL_LOG_ERROR( "%s", "failed on dsi_suspend_qos" );
    (void)qcril_reqlist_free( modem_id, params_ptr->t );
    goto err_label;
  }

  /* Generate command response to upper layers */
  asprintf( &response.qos_resp.result.return_code, "%d", RIL_E_SUCCESS );

  qcril_data_response_success( instance_id,
                               params_ptr->t,
                               params_ptr->event_id,
                               &response.qos_resp,
                               response_len );

  /* Release dynamic memory */
  QCRIL_DATA_RELEASE_STORAGE( response.qos_resp.result.return_code );

  /* Modem QOS status indications report outcome of network negotiation. */
  ret = SUCCESS;

err_label:
  /* Post error to RIL framework */
  if( SUCCESS != ret )
  {
    QCRIL_LOG_DEBUG("%s", "respond to QCRIL with error");

    memset( &response, 0x0, sizeof(response) );
    response_len = sizeof(response.qos_resp.setup);

    asprintf( &response.qos_resp.result.return_code, "%d", RIL_E_GENERIC_FAILURE );
    qcril_data_response_success( instance_id,
                                 params_ptr->t,
                                 params_ptr->event_id,
                                 &response.qos_resp,
                                 response_len );
    QCRIL_DATA_RELEASE_STORAGE( response.qos_resp.result.return_code );
  }
  if( info_tbl_ptr && (NULL != info_tbl_ptr->pend_tok) )
  {
    info_tbl_ptr->pend_tok   = NULL;
    info_tbl_ptr->pend_req   = DS_RIL_REQ_INVALID;  /* is there a RIL_REQUEST_INVALID */
  }

  QCRIL_DATA_MUTEX_UNLOCK(&info_tbl_mutex);

err_bad_input:
  QCRIL_LOG_EXIT;
  return;
} /* qcril_data_request_suspend_qos() */


/*===========================================================================

  FUNCTION: qcril_data_request_resume_qos

===========================================================================*/
/*!
    @brief

    @pre

    @return
    None.
*/
/*=========================================================================*/
void qcril_data_request_resume_qos
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr /*!< Output parameter */
)
{
  qcril_data_call_response_type  response;
  size_t                         response_len = 0;
  qcril_instance_id_e_type  instance_id;
  qcril_modem_id_e_type     modem_id;
  dsi_qos_id_type           flowid;
  int                       ret = FAILURE;
  int                       i;
  unsigned int              pend_req = DS_RIL_REQ_INVALID;
  qcril_data_call_info_tbl_type  *info_tbl_ptr = NULL;
  qcril_reqlist_u_type      u_info;
  qcril_reqlist_public_type reqlist_entry;
  char                     *bad_ptr = NULL;

  QCRIL_LOG_ENTRY;

  /* Validate input parameters*/
  QCRIL_DS_ASSERT( params_ptr != NULL, "validate params_ptr" );
  QCRIL_DS_ASSERT( ret_ptr    != NULL, "validate ret_ptr" );
  QCRIL_DS_ASSERT ( ( params_ptr != NULL) && ( ( params_ptr->datalen % sizeof(char*) ) == 0 ), "validate datalen" );
  if( ( params_ptr == NULL ) ||
      ( params_ptr->data == NULL ) ||
      ( ret_ptr == NULL ) ||
      ( ( params_ptr->datalen % sizeof(char*) ) != 0) )
  {
    QCRIL_LOG_ERROR( "BAD input, params_ptr [%p], ret_ptr [%p], datalen [%d]",
                     params_ptr, ret_ptr,
                     params_ptr ? params_ptr->datalen : 0 );
    goto err_bad_input;
  }

  instance_id = params_ptr->instance_id;
  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  if( instance_id >= QCRIL_MAX_INSTANCE_ID )
  {
    goto err_bad_input;
  }

  memset( &response, 0x0, sizeof(response) );
  response_len = sizeof(response.qos_resp.result);

  QCRIL_DATA_MUTEX_LOCK(&info_tbl_mutex);

  /* Check mandatory parameters: QOS flow ID */
  if( params_ptr->datalen < DS_RIL_MIN_QOS_RESUME_PARAMS )
  {
    QCRIL_LOG_ERROR( "BAD input, datalen [%d] < [%d]",
                     params_ptr->datalen, DS_RIL_MIN_QOS_RESUME_PARAMS );
    goto err_label;
  }

  modem_id = params_ptr->modem_id;
  QCRIL_ASSERT( modem_id < QCRIL_MAX_MODEM_ID );
  if( modem_id >= QCRIL_MAX_MODEM_ID )
  {
    goto err_label;
  }

  flowid = strtol( ((const char **)params_ptr->data)[0], &bad_ptr, 0 );
  if( *bad_ptr != 0 )
  {
    QCRIL_LOG_ERROR( "BAD input, conversion error on [%s] at [%s]",
                     ((const char **)params_ptr->data)[0], bad_ptr );
    goto err_label;
  }
  QCRIL_LOG_DEBUG( "RIL says QOS FlowID [0x%08x], len [%d]",
                   flowid, params_ptr->datalen );

  info_tbl_ptr = qcril_data_find_call_by_flowid( flowid );
  if( NULL == info_tbl_ptr )
  {
    QCRIL_LOG_ERROR( "no valid QOS flow ID [0x%x] match found", flowid );
    goto err_label;
  }


  /* Check whether another REQ is pending for this call */
  if( qcril_data_util_is_req_pending( info_tbl_ptr, &pend_req ) )
  {
    QCRIL_LOG_INFO( "UNEXPECTED RIL REQ pend [%s], cid [%d], index [%d]",
                    qcril_log_lookup_event_name( pend_req ), info_tbl_ptr->cid, info_tbl_ptr->index );
    /* if request is already pending, ignore another one */
    if( pend_req == RIL_REQUEST_RESUME_QOS )
    {
        QCRIL_LOG_INFO( "RIL_REQUEST_RESUME_QOS already pending, cid [%d], index [%d]",
                        info_tbl_ptr->cid, info_tbl_ptr->index);
    }
    goto err_label;
  }

  /* Start new RIL transaction, which is released in response generation */
  memset( &u_info, 0x0, sizeof( qcril_reqlist_u_type ) );
  u_info.ds.info = (void *)info_tbl_ptr;
  qcril_reqlist_default_entry( params_ptr->t,
                               params_ptr->event_id,
                               modem_id,
                               QCRIL_REQ_AWAITING_MORE_AMSS_EVENTS,
                               QCRIL_EVT_DATA_EVENT_CALLBACK,
                               &u_info,
                               &reqlist_entry );
  if( E_SUCCESS != qcril_reqlist_new( instance_id, &reqlist_entry ) )
  {
    QCRIL_LOG_ERROR( "%s", "qcril_reqlist_new" );
    goto err_label;
  }

  info_tbl_ptr->pend_tok    = params_ptr->t;
  info_tbl_ptr->pend_req    = params_ptr->event_id;
  QCRIL_LOG_INFO( "DEBUG: %s step %d - info_tbl_ptr->pend_tok[0x%x] info_tbl_ptr->pend_req[0x%X]",
                  __func__, 0, info_tbl_ptr->pend_tok, info_tbl_ptr->pend_req );
  QCRIL_LOG_VERBOSE( "%s", "reqlist node insert complete, request QOS resume" );

  /* Invoke DSI API to send command to Modem */
  if( DSI_SUCCESS != dsi_resume_qos( info_tbl_ptr->dsi_hndl,
                                      1, &flowid ) )
  {
    QCRIL_LOG_ERROR( "%s", "failed on dsi_resume_qos" );
    (void)qcril_reqlist_free( modem_id, params_ptr->t );
    goto err_label;
  }

  /* Generate command response to upper layers */
  asprintf( &response.qos_resp.result.return_code, "%d", RIL_E_SUCCESS );

  qcril_data_response_success( instance_id,
                               params_ptr->t,
                               params_ptr->event_id,
                               &response.qos_resp,
                               response_len );

  /* Release dynamic memory */
  QCRIL_DATA_RELEASE_STORAGE( response.qos_resp.result.return_code );

  /* Modem QOS status indications report outcome of network negotiation. */
  ret = SUCCESS;

err_label:
  /* Post error to RIL framework */
  if( SUCCESS != ret )
  {
    QCRIL_LOG_DEBUG("%s", "respond to QCRIL with error");

    memset( &response, 0x0, sizeof(response) );
    response_len = sizeof(response.qos_resp.setup);

    asprintf( &response.qos_resp.result.return_code, "%d", RIL_E_GENERIC_FAILURE );
    qcril_data_response_success( instance_id,
                                 params_ptr->t,
                                 params_ptr->event_id,
                                 &response.qos_resp,
                                 response_len );
    QCRIL_DATA_RELEASE_STORAGE( response.qos_resp.result.return_code );
  }
  if( info_tbl_ptr && (NULL != info_tbl_ptr->pend_tok) )
  {
    info_tbl_ptr->pend_tok   = NULL;
    info_tbl_ptr->pend_req   = DS_RIL_REQ_INVALID;  /* is there a RIL_REQUEST_INVALID */
  }

  QCRIL_DATA_MUTEX_UNLOCK(&info_tbl_mutex);

err_bad_input:
  QCRIL_LOG_EXIT;
  return;
} /* qcril_data_request_resume_qos() */

/*===========================================================================

  FUNCTION: qcril_data_request_get_qos_status

===========================================================================*/
/*!
    @brief
    Handles RIL_REQUEST_GET_QOS_STATUS

    @pre
    Before calling, info_tbl_mutex must not be locked by the calling thread

    @return
    None.
*/
/*=========================================================================*/
void qcril_data_request_get_qos_status
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr /*!< Output parameter */
)
{
  qcril_instance_id_e_type  instance_id;
  qcril_modem_id_e_type     modem_id;
  dsi_qos_id_type           flowid;
  int                       ret = FAILURE;
  int                       i;
  unsigned int              pend_req = DS_RIL_REQ_INVALID;
  qcril_data_call_info_tbl_type  *info_tbl_ptr = NULL;
  qcril_reqlist_u_type      u_info;
  qcril_reqlist_public_type reqlist_entry;
  dsi_qos_granted_info_type qos_info;
  dsi_qos_status_type       dsi_flow_status;
  RIL_QosStatus             ril_flow_status;
  qcril_data_call_response_type  response;
  size_t                         response_len = 0;
  char                     *bad_ptr = NULL;

  QCRIL_LOG_ENTRY;

  /* Validate input parameters*/
  QCRIL_DS_ASSERT( params_ptr != NULL, "validate params_ptr" );
  QCRIL_DS_ASSERT( ret_ptr    != NULL, "validate ret_ptr" );
  QCRIL_DS_ASSERT ( ( params_ptr != NULL) && ( ( params_ptr->datalen % sizeof(char*) ) == 0 ), "validate datalen" );
  if( ( params_ptr == NULL ) ||
      ( params_ptr->data == NULL ) ||
      ( ret_ptr == NULL ) ||
      ( ( params_ptr->datalen % sizeof(char*) ) != 0) )
  {
    QCRIL_LOG_ERROR( "BAD input, params_ptr [%p], ret_ptr [%p], datalen [%d]",
                     params_ptr, ret_ptr,
                     params_ptr ? params_ptr->datalen : 0 );
    goto err_bad_input;
  }

  instance_id = params_ptr->instance_id;
  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  if( instance_id >= QCRIL_MAX_INSTANCE_ID )
  {
    goto err_bad_input;
  }

  memset( &response, 0x0, sizeof(response) );
  response_len = sizeof(response.qos_resp.get_status);

  QCRIL_DATA_MUTEX_LOCK(&info_tbl_mutex);

  /* Check mandatory parameters: QOS flow ID */
  if( params_ptr->datalen < DS_RIL_MIN_QOS_STATUS_PARAMS )
  {
    QCRIL_LOG_ERROR( "BAD input, datalen [%d] < [%d]",
                     params_ptr->datalen, DS_RIL_MIN_QOS_STATUS_PARAMS );
    goto err_label;
  }

  modem_id = params_ptr->modem_id;
  QCRIL_ASSERT( modem_id < QCRIL_MAX_MODEM_ID );
  if( modem_id >= QCRIL_MAX_MODEM_ID )
  {
    goto err_label;
  }

  /* Extract QOS flow ID and find data call instance */
  flowid = strtol( ((const char **)params_ptr->data)[0], &bad_ptr, 0 );
  if( *bad_ptr != 0 )
  {
    QCRIL_LOG_ERROR( "BAD input, conversion error on [%s] at [%s]",
                     ((const char **)params_ptr->data)[0], bad_ptr );
    goto err_label;
  }
  QCRIL_LOG_DEBUG( "RIL says QOS FlowID [0x%08x], len [%d]",
                   flowid, params_ptr->datalen );

  info_tbl_ptr = qcril_data_find_call_by_flowid( flowid );
  if( NULL == info_tbl_ptr )
  {
    QCRIL_LOG_ERROR( "no valid QOS flow ID [0x%x] match found", flowid );
    goto err_label;
  }

  /* Check whether another REQ is pending for this call */
  if( qcril_data_util_is_req_pending( info_tbl_ptr, &pend_req ) )
  {
    QCRIL_LOG_INFO( "UNEXPECTED RIL REQ pend [%s], cid [%d], index [%d]",
                    qcril_log_lookup_event_name( pend_req ), info_tbl_ptr->cid, info_tbl_ptr->index );
    /* if request is already pending, ignore another one */
    if( pend_req == RIL_REQUEST_GET_QOS_STATUS )
    {
        QCRIL_LOG_INFO( "RIL_REQUEST_GET_QOS_STATUS already pending, cid [%d], index [%d]",
                        info_tbl_ptr->cid, info_tbl_ptr->index);
    }
    goto err_label;
  }

  /* Start new RIL transaction, which is released in response generation */
  memset( &u_info, 0x0, sizeof( qcril_reqlist_u_type ) );
  u_info.ds.info = (void *)info_tbl_ptr;
  qcril_reqlist_default_entry( params_ptr->t,
                               params_ptr->event_id,
                               modem_id,
                               QCRIL_REQ_AWAITING_MORE_AMSS_EVENTS,
                               QCRIL_EVT_DATA_EVENT_CALLBACK,
                               &u_info,
                               &reqlist_entry );
  if( E_SUCCESS != qcril_reqlist_new( instance_id, &reqlist_entry ) )
  {
    QCRIL_LOG_ERROR( "%s", "qcril_reqlist_new" );
    goto err_label;
  }

  info_tbl_ptr->pend_tok    = params_ptr->t;
  info_tbl_ptr->pend_req    = params_ptr->event_id;
  QCRIL_LOG_INFO( "DEBUG: %s step %d - info_tbl_ptr->pend_tok[0x%x] info_tbl_ptr->pend_req[0x%X]",
                  __func__, 0, info_tbl_ptr->pend_tok, info_tbl_ptr->pend_req );
  QCRIL_LOG_VERBOSE( "%s", "reqlist node insert complete, request QOS release" );

  /* Invoke DSI API to send commands to Modem */
  memset( &qos_info, 0x0, sizeof(qos_info) );
  if( DSI_SUCCESS != dsi_get_granted_qos( info_tbl_ptr->dsi_hndl,
                                          flowid,
                                          AF_INET,
                                          &qos_info ) )
  {
    QCRIL_LOG_ERROR( "%s", "failed on dsi_get_granted_qos" );
    (void)qcril_reqlist_free( modem_id, params_ptr->t );
    goto err_label;
  }

  if( DSI_SUCCESS != dsi_get_qos_status( info_tbl_ptr->dsi_hndl,
                                         flowid,
                                         &dsi_flow_status ) )
  {
    QCRIL_LOG_ERROR( "%s", "failed on dsi_get_qos_status" );
    (void)qcril_reqlist_free( modem_id, params_ptr->t );
    goto err_label;
  }

  /* Generate string representation of QOS specification */
  if( SUCCESS != qcril_data_generate_qos_flow_strings( &qos_info,
                                                       &response.qos_resp.get_status.qos_spec1,
                                                       &response.qos_resp.get_status.qos_spec2 ) )
  {
    QCRIL_LOG_ERROR( "%s", "failed on qcril_data_generate_qos_flow_strings" );
    (void)qcril_reqlist_free( modem_id, params_ptr->t );
    goto err_label;
  }
  asprintf( &response.qos_resp.get_status.return_code, "%d", RIL_E_SUCCESS );

  /* Generate command response to upper layers */
  ril_flow_status = qcril_data_qos_status_map[ dsi_flow_status ];
  if( (RIL_QOS_STATUS_NONE != ril_flow_status) &&
      (RIL_QOS_STATUS_ACTIVATED != ril_flow_status) &&
      (RIL_QOS_STATUS_SUSPENDED != ril_flow_status) )
  {
    QCRIL_LOG_ERROR( "%s", "failed on mapping flow status" );
    (void)qcril_reqlist_free( modem_id, params_ptr->t );
    goto err_label;
  }
  asprintf( &response.qos_resp.get_status.status, "%d", ril_flow_status );

  QCRIL_LOG_DEBUG( "%s", ">>>RIL RSP READY>>> result SUCCESS" );
  qcril_data_response_success( instance_id,
                               params_ptr->t,
                               info_tbl_ptr->pend_req,
                               &response.qos_resp,
                               response_len );

  info_tbl_ptr->pend_tok   = NULL;
  info_tbl_ptr->pend_req   = DS_RIL_REQ_INVALID;  /* is there a RIL_REQUEST_INVALID */

  /* Release dynamic memory */
  QCRIL_DATA_RELEASE_STORAGE( response.qos_resp.get_status.return_code );
  QCRIL_DATA_RELEASE_STORAGE( response.qos_resp.get_status.status );
  QCRIL_DATA_RELEASE_STORAGE( response.qos_resp.get_status.qos_spec1 );
  QCRIL_DATA_RELEASE_STORAGE( response.qos_resp.get_status.qos_spec2 );

  ret = SUCCESS;

 err_label:
  /* Post error to RIL framework */
  if( SUCCESS != ret )
  {
    QCRIL_LOG_DEBUG("%s", "respond to QCRIL with error");

    memset( &response, 0x0, sizeof(response) );
    response_len = sizeof(response.qos_resp.get_status);

    asprintf( &response.qos_resp.get_status.return_code, "%d", RIL_E_GENERIC_FAILURE );
    qcril_data_response_success( instance_id,
                                 params_ptr->t,
                                 params_ptr->event_id,
                                 &response.qos_resp,
                                 response_len );
    QCRIL_DATA_RELEASE_STORAGE( response.qos_resp.get_status.return_code );
  }
  if( info_tbl_ptr && (NULL != info_tbl_ptr->pend_tok) )
  {
    info_tbl_ptr->pend_tok   = NULL;
    info_tbl_ptr->pend_req   = DS_RIL_REQ_INVALID;  /* is there a RIL_REQUEST_INVALID */
  }

  QCRIL_DATA_MUTEX_UNLOCK(&info_tbl_mutex);

err_bad_input:
  QCRIL_LOG_EXIT;
  return;
} /* qcril_data_request_get_qos_status() */


/*===========================================================================

  FUNCTION: qcril_data_qos_status_ind_handler

===========================================================================*/
/*!
    @brief
    Process QOS status indication messages received from lower-layer.
    The appropriate response buffer and length are returned, along with
    status and matching command request.  If no request match, response is
    assumed to be unsolicited.

    @pre

    @return
    SUCCESS
    FAILURE
*/
/*=========================================================================*/
int qcril_data_qos_status_ind_handler
(
  const qcril_data_event_data_t  *evt_info_ptr,
  void                          **response_pptr,
  size_t                         *response_len_ptr
)
{
  qcril_data_call_info_tbl_type  *info_tbl_ptr = NULL;
  qcril_data_qos_state_type * flow_ptr = NULL;
  RIL_QosIndStates state;
  int  ret = SUCCESS;

  QCRIL_LOG_ENTRY;

  /* Input Validation */
  QCRIL_DS_ASSERT( evt_info_ptr != NULL, "validate input evt_info_ptr" );
  QCRIL_DS_ASSERT( response_pptr != NULL, "validate input response_pptr" );
  QCRIL_DS_ASSERT( response_len_ptr  != NULL, "validate input response_len_ptr" );

  if( (NULL == evt_info_ptr) || (NULL == response_pptr) || (NULL == response_len_ptr) )
  {
    return FAILURE;
  }

  /* Info table pointer validated by caller */
  info_tbl_ptr = (qcril_data_call_info_tbl_type*)evt_info_ptr->data;

  /* Process QOS event */
  switch( evt_info_ptr->payload.qos_info.status_evt )
  {
    case DSI_QOS_ACTIVATED_EV:
      QCRIL_LOG_INFO( "processing ACTIVATED event for flow[0x%08x] type[0x%x]",
                      evt_info_ptr->payload.qos_info.flow_id,
                      evt_info_ptr->payload.qos_info.flow_type);

      /* Populate the unsolicited response buffers */
      asprintf( &(*(qcril_data_call_response_type**)response_pptr)->qos_resp.unsol.qos_flow_id,
                "%d", (unsigned int)evt_info_ptr->payload.qos_info.flow_id );
      state = (DSI_QOS_FLOW_TYPE_NW_INIT == evt_info_ptr->payload.qos_info.flow_type)?
              RIL_QOS_ACTIVATED_NETWORK : RIL_QOS_ACTIVATED;
      asprintf( &(*(qcril_data_call_response_type**)response_pptr)->qos_resp.unsol.status_ind,
                "%d", state );

      *response_pptr = (void*)&(*(qcril_data_call_response_type**)response_pptr)->qos_resp.unsol;
      *response_len_ptr = sizeof((*(qcril_data_call_response_type**)response_pptr)->qos_resp.unsol);


      /* Check if flow was client-initiated, as it will already be in flow list */
      flow_ptr = qcril_data_qos_find_flow( info_tbl_ptr, evt_info_ptr->payload.qos_info.flow_id );
      if( NULL == flow_ptr )
      {
        /* Flow object not found, assume network-initiated */
        QCRIL_LOG_DEBUG( "no flow found, adding network initiated flow to CID [%d]",
                         evt_info_ptr->payload.qos_info.flow_id, info_tbl_ptr->cid );
        if( SUCCESS != qcril_data_qos_add_flows( info_tbl_ptr, &evt_info_ptr->payload.qos_info.flow_id, 1 ) )
        {
          QCRIL_LOG_ERROR( "%s", "failed on qcril_data_qos_add_flows" );
        }
      }
      break;

    case DSI_QOS_GONE_EV:
      QCRIL_LOG_INFO( "processing GONE event for flow[0x%08x]",
                      evt_info_ptr->payload.qos_info.flow_id );

      asprintf( &(*(qcril_data_call_response_type**)response_pptr)->qos_resp.unsol.qos_flow_id,
                "%d", (unsigned int)evt_info_ptr->payload.qos_info.flow_id );
      asprintf( &(*(qcril_data_call_response_type**)response_pptr)->qos_resp.unsol.status_ind,
                "%d", RIL_QOS_NETWORK_RELEASE ); /* Assuming NW/Modem flow release */

      *response_pptr = (void*)&(*(qcril_data_call_response_type**)response_pptr)->qos_resp.unsol;
      *response_len_ptr = sizeof((*(qcril_data_call_response_type**)response_pptr)->qos_resp.unsol);

      /* Update call record QOS flow ID list */
      if( SUCCESS != qcril_data_qos_del_flows( info_tbl_ptr, &evt_info_ptr->payload.qos_info.flow_id, 1 ) )
      {
        QCRIL_LOG_ERROR( "%s", "failed on qcril_data_qos_del_flows" );
      }
      break;

    case DSI_QOS_SUSPENDED_EV:
      QCRIL_LOG_INFO( "processing SUSPENDED event for flow[0x%08x]",
                      evt_info_ptr->payload.qos_info.flow_id );

      asprintf( &(*(qcril_data_call_response_type**)response_pptr)->qos_resp.unsol.qos_flow_id,
                "%d", (unsigned int)evt_info_ptr->payload.qos_info.flow_id );
      asprintf( &(*(qcril_data_call_response_type**)response_pptr)->qos_resp.unsol.status_ind,
                "%d", RIL_QOS_SUSPENDED ); /* TODO: Not sure what to use here */

      *response_pptr = (void*)&(*(qcril_data_call_response_type**)response_pptr)->qos_resp.unsol;
      *response_len_ptr = sizeof((*(qcril_data_call_response_type**)response_pptr)->qos_resp.unsol);
      break;

    case DSI_QOS_MODIFY_ACCEPTED_EV:
      QCRIL_LOG_INFO( "processing MODIFY ACCEPTED event for flow[0x%08x]",
                      evt_info_ptr->payload.qos_info.flow_id );

      /* Populate the unsolicited response buffers */
      asprintf( &(*(qcril_data_call_response_type**)response_pptr)->qos_resp.unsol.qos_flow_id,
                "%d", (unsigned int)evt_info_ptr->payload.qos_info.flow_id );
      asprintf( &(*(qcril_data_call_response_type**)response_pptr)->qos_resp.unsol.status_ind,
                "%d", RIL_QOS_MODIFIED );

      *response_pptr = (void*)&(*(qcril_data_call_response_type**)response_pptr)->qos_resp.unsol;
      *response_len_ptr = sizeof((*(qcril_data_call_response_type**)response_pptr)->qos_resp.unsol);
      break;

    case DSI_QOS_MODIFY_REJECTED_EV:
      QCRIL_LOG_INFO( "processing MODIFY REJECTED event for flow[0x%08x]",
                      evt_info_ptr->payload.qos_info.flow_id );

      /* Populate the unsolicited response buffers */
      asprintf( &(*(qcril_data_call_response_type**)response_pptr)->qos_resp.unsol.qos_flow_id,
                "%d", (unsigned int)evt_info_ptr->payload.qos_info.flow_id );
      asprintf( &(*(qcril_data_call_response_type**)response_pptr)->qos_resp.unsol.status_ind,
                "%d", RIL_QOS_ERROR_UNKNOWN );

      *response_pptr = (void*)&(*(qcril_data_call_response_type**)response_pptr)->qos_resp.unsol;
      *response_len_ptr = sizeof((*(qcril_data_call_response_type**)response_pptr)->qos_resp.unsol);
      break;

    case DSI_QOS_INFO_CODE_UPDATED_EV:
      QCRIL_LOG_INFO( "ignoring QOS event[%d] for pending request[%d]",
                      info_tbl_ptr->pend_req );
      break;

    default:
      QCRIL_LOG_ERROR( "unsupported QOS event [%d]",
                       evt_info_ptr->payload.qos_info.status_evt );
      ret = FAILURE;
  }

  QCRIL_LOG_EXIT;
  return ret;
} /* qcril_data_qos_status_ind_handler() */


/*===========================================================================

  FUNCTION:  qcril_data_qos_init

===========================================================================*/
/*!
    @brief
    Initialize the DATA QOS subsystem of the RIL.

    @return
    None.
*/
/*=========================================================================*/
void qcril_data_qos_init( void )
{
  int  ret = SUCCESS;
  unsigned int i;

  QCRIL_LOG_ENTRY;

  /* Initialize QOS flow list element */
  for( i=0; i < MAX_CONCURRENT_UMTS_DATA_CALLS; i++ )
  {
    list_init( &info_tbl[i].qos_flow_list );
  }

  QCRIL_LOG_EXIT;
  return;
} /* qcril_data_qos_init() */
#endif
