/*************************************************************************
* Company                       : Tata Elxsi Ltd
* File                          : TEL_FLACDec_typedefs.h
* Author                        : TEL Audio Team
* Version                       : 1.0
* Date                          : 12-December-2009
* Modified Date                 : 14-October-2009
* Operating Environment         : Windows XP
* Compiler with Version         : Visual studio on PC
* Description                   : This file contains the typedefs for basic
*                               : data types
* Copyright                     : <2009> Tata Elxsi Ltd.
*                               : All rights reserved.
*                               : his document/code contains information
*                                 that is proprietary to Tata Elxsi Ltd.
*                                 No part of this document/code may be
*                                 reproduced or used in whole or part in
*                                 any form or by any means - graphic,
*                                 electronic or mechanical without the
*                                 written permission of Tata Elxsi Ltd.
*************************************************************************/
#ifndef _TEL_FLACDEC_TYPEDEFS_H_
#define _TEL_FLACDEC_TYPEDEFS_H_

#include <string.h>
#include <stdio.h>
//#include <inttypes.h>

typedef unsigned char       bool8;
typedef char                int8;
typedef unsigned char       uint8;
typedef short               int16;
typedef unsigned short      uint16;
typedef int                 int32;
typedef unsigned int        uint32;
typedef signed long long    int64;          /* Signed 64 bit value          */
typedef unsigned long long  uint64;         /* Unsigned 64 bit value        */
typedef signed long long    int40;          /* Signed 40 bit value          */
typedef void                void32;


typedef void* FLACDecContext;

#ifndef NULL
#define NULL (void*)0
#endif  /* NULL */

#ifndef TRUE
#define TRUE 1
#endif  /* TRUE */

#ifndef FALSE
#define FALSE 0
#endif  /* FALSE */

#ifdef FLAC_DEBUG
#define  FLACDEC_PRINTF printf
#endif /* FLAC_DEBUG */

#define  MEMCPY         memcpy


#define MAXPARTIALFRAMESIZE 8192*8 /* Number of samples in partial frame per channel */
#define MAXINPBUFFER    8192*8*2*2 /* 64K per channel * num Channels * 2 bytes per sample */
//#define MAXINPBUFFER    4700 /* 64K per channel * num Channels * 2 bytes per sample */
#define MAXOUTBUFFSIZE  MAXPARTIALFRAMESIZE
#define MAXOUTCHANNELS  8
#define STRING_LENGTH   100
#define SUBSET_DATAREQUIREMENT 32768 /* Minimum number of bytes required in subset mode */
#define SUBSET_BLOCKSIZE 4608 /* Maximum number of samples per channel in subset mode*/
#define ALGOE


  /* for supporting superset mode of Flac, the worst case one Frame can be
     64K*2*2 (64K per channel * num Channels * 2 bytes per sample)
     and output frame can be also 64K*2*2. To avoid data starvation at AFE
     output partial frame process is implemented in superset mode.
     partial frame size is 8k * 2* 2*/
static const uint32 FLAC_EXT_PARAMS_START  = 200;
static const uint32 FLAC_INP_BUF_SIZE      = 8192*8*4*2;
static const uint32 FLAC_OUT_BUF_SIZE      = 8192*4*2;
static const uint32 FLAC_THREAD_STACK_SIZE = 8192;
static const int FLAC_KIPS                 = 70000;
static const uint32 FLAC_32BIT_Q_FORMAT    = 23;
static const uint32 PCM_16BIT_Q_FORMAT     = 15; //VNOTE: check
static const uint32 BYTE_UPDOWN_CONV_SHIFT_FACT = 8; //VNOTE: check

  /* This value is to indicate decoder svc to wait for format block
     before calling decoder process. */
static const uint32 FLAC_FORMAT_BLOCK_REQ  = 1;


#ifdef __cplusplus
#endif



#endif /*_TEL_FLACDEC_TYPEDEFS_H_ */

/*************************************************************************
 * End of file
 ************************************************************************/

