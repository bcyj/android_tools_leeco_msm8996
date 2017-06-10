#ifndef _SCI_TYPES_H_
#define _SCI_TYPES_H_

//#include <utils/log.h>
#include <android/log.h>
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,"DMLIB ==>",__VA_ARGS__)

#define PUBLIC
#define LOCAL static

#ifndef PNULL
#define PNULL 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef MIN
#define MIN(a,b) ((a)<(b)? (a):(b))
#endif

//#define LOGE
#define DM_TRACE LOGE

typedef int BOOLEAN;

typedef int Boolean;

typedef unsigned int uint32;

typedef unsigned char uint8;

typedef unsigned short uint16;

typedef short int16;

typedef int int32;

typedef char int8;

typedef char wchar;

#endif
