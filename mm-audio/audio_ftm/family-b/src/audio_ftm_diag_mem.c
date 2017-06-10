#ifdef __cplusplus
extern "C" {
#endif

/**

  @file   DALSYS_common.c
  @brief  DALSYS OS dependent functions mapping to Linux
====================================================================================================
Copyright (c) 2011 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.

$Header: //source/qcom/qct/multimedia2/Audio/drivers/ftm/8x60/linux/rel/1.0/src/audio_ftm_diag_mem.c#1 $
$DateTime: 2011/04/05 20:05:46 $
$Author: zhongl $

Revision History:
                            Modification     Tracking
Author (core ID)                Date         CR Number   Description of Changes
-------------------------   ------------    ----------   -------------------------------------------
ZhongL                      05/30/2010                    File creation.


====================================================================================================
                                        INCLUDE FILES
==================================================================================================*/

#include "audio_ftm_diag_mem.h"

/*==================================================================================================
                                     LOCAL CONSTANTS
==================================================================================================*/


/*==================================================================================================
                          LOCAL TYPEDEFS (STRUCTURES, UNIONS, ENUMS)
==================================================================================================*/
typedef struct
{
  uint8 command_code;
}diagpkt_hdr_type;

typedef struct
{
  uint32 diag_data_type; /* This will be used to identify whether the data passed to DCM is an event, log, F3 or response.*/
  uint8 rest_of_data;
}diag_data;

#define DIAG_REST_OF_DATA_POS (FPOS(diag_data, rest_of_data))
#define DIAG_DATA_TYPE_RESPONSE      3
#define DIAGPKT_HDR_PATTERN (0xDEADD00DU)
#define DIAGPKT_OVERRUN_PATTERN (0xDEADU)

typedef struct
{
  unsigned int pattern;     /* Pattern to check validity of committed pointers. */
  unsigned int size;        /* Size of usable buffer (diagpkt_q_type->pkt) */
  unsigned int length;      /* Size of packet */

  byte pkt[20];               /* Sized by 'length' field. */
}diagpkt_rsp_type;


typedef struct
{
  diag_cmd_rsp rsp_func; /* If !NULL, this is called in lieu of comm layer */
  void *rsp_func_param;

  diagpkt_rsp_type rsp; /* see diagi.h */
}diagpkt_lsm_rsp_type;

typedef struct
{
  uint8 command_code;
  uint8 subsys_id;
  uint16 subsys_cmd_code;
}
diagpkt_subsys_hdr_type;

/*==================================================================================================
                                        LOCAL MACROS
==================================================================================================*/



/*==================================================================================================
                                      LOCAL VARIABLES
==================================================================================================*/


/*==================================================================================================
                                     GLOBAL VARIABLES
==================================================================================================*/


/*==================================================================================================
                                 LOCAL FUNCTION PROTOTYPES
==================================================================================================*/

/*==================================================================================================
                                     LOCAL FUNCTIONS
==================================================================================================*/


/*==================================================================================================
                                       GLOBAL FUNCTIONS
==================================================================================================*/


PACK(void *) audio_ftm_diag_alloc (diagpkt_cmd_code_type code, unsigned int length)
{
    diagpkt_lsm_rsp_type *item = NULL;
    diagpkt_hdr_type *pkt = NULL;
    PACK(uint16 *)pattern = NULL;    /* Overrun pattern. */
    unsigned char *p;
    diag_data* pdiag_data = NULL;
     unsigned int size = 0;


     size = DIAG_REST_OF_DATA_POS + FPOS (diagpkt_lsm_rsp_type, rsp.pkt) + length + sizeof (uint16);

    /*-----------------------------------------------
      Try to allocate a buffer.  Size of buffer must
      include space for overhead and CRC at the end.
    -----------------------------------------------*/
      DALSYS_Malloc (size, (void *)&pdiag_data);
      if(NULL == pdiag_data)
      {
         /* Alloc not successful.  Return NULL. DiagSvc_Malloc() allocates memory
	  from client's heap using a malloc call if the pre-malloced buffers are not available.
	  So if this fails, it means that the client is out of heap. */
         return NULL;
      }
      /* Fill in the fact that this is a response */
      pdiag_data->diag_data_type = DIAG_DATA_TYPE_RESPONSE;
      // WM7 prototyping: advance the pointer now
      item = (diagpkt_lsm_rsp_type*)((byte*)(pdiag_data)+DIAG_REST_OF_DATA_POS);

    /* This pattern is written to verify pointers elsewhere in this
       service  are valid. */
    item->rsp.pattern = DIAGPKT_HDR_PATTERN;    /* Sanity check pattern */

    /* length ==  size unless packet is resized later */
    item->rsp.size = length;
    item->rsp.length = length;

    pattern = (PACK(uint16 *)) & item->rsp.pkt[length];

    /* We need this to meet alignment requirements - MATS */
    p = (unsigned char *) pattern;
    p[0] = (DIAGPKT_OVERRUN_PATTERN >> 8) & 0xff;
    p[1] = (DIAGPKT_OVERRUN_PATTERN >> 0) & 0xff;

    pkt = (diagpkt_hdr_type *) & item->rsp.pkt;

    if (pkt)
    {
        pkt->command_code = code;
    }
    return (PACK(void *)) pkt;

}               /* diagpkt_alloc */


PACK(void *) audio_ftm_diagpkt_subsys_alloc (diagpkt_subsys_id_type id,
              diagpkt_subsys_cmd_code_type code, unsigned int length)
{
  diagpkt_subsys_hdr_type *hdr = NULL;

  hdr = (diagpkt_subsys_hdr_type *) audio_ftm_diag_alloc (DIAG_SUBSYS_CMD_F, length);

  if( hdr != NULL )
  {
      hdr->subsys_id = id;
      hdr->subsys_cmd_code = code;

  }

  return (PACK(void *)) hdr;

}               /* diagpkt_subsys_alloc */


#ifdef __cplusplus
}
#endif
