/**********************************************************************
* Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved. *
* Qualcomm Technologies Proprietary and Confidential.                 *
**********************************************************************/
#ifndef CAC_INT_H
#define CAC_INT_H

#define ANDROID_BUILD
//#define BIT_MATCH
#if 0
#define CAC_DEBUG
#define NEON_OPT
#endif

#define NEW_INTERFACE


typedef signed char          int8_t;
typedef unsigned char        uint8_t;
typedef signed short         int16_t;
typedef unsigned short       uint16_t;

#ifndef ANDROID_BUILD

typedef signed long int      int32_t;
typedef unsigned long int    uint32_t;

#endif


typedef signed long long     int64_t;
typedef unsigned long long   uint64_t;
typedef char                 char_t;
typedef unsigned char        bool_t;




#ifndef TRUE
#define TRUE	1
#endif

#ifndef FALSE
#define FALSE	0
#endif

#ifndef NULL
#define NULL    0
#endif

#ifndef MAX2
#define MAX2(A, B)   ((A) > (B) ? (A) : (B))
#endif

#ifndef MIN2
#define MIN2(A, B)   ((A) < (B) ? (A) : (B))
#endif

#ifndef CLIP
#define CLIP(X, A, B) (MIN2(MAX2((X), (A)), (B)))
#endif

#ifndef ABS
#define ABS(X)     (((X) < 0) ? -(X) : (X))
#endif

#ifndef MAXVALUE
#define MAXVALUE 255
#endif

#ifndef SATURATEDTH
#define SATURATEDTH 0
#endif

#endif

