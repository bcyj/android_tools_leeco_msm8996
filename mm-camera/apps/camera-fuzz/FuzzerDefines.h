#ifndef _FUZZERDEFINES_H_
#define _FUZZERDEFINES_H_
/* ========================================================================= * 
   Module:   FuzzerDefines.h
   Date:     12/2/2011
   Author:   cdsilva
   Purpose:  FuzzerDefines header file

           -------------------------------------------------------
         Copyright © 2012 Qualcomm Technologies, Inc. All Rights Reserved.
               Qualcomm Technologies Proprietary and Confidential.
* ========================================================================= */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <getopt.h>
#include <dirent.h>
#include <string.h>
#include <strings.h>
#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <limits.h>
#include <semaphore.h>
#include <time.h>
 
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

// shared memory
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>

/* ---------------------------------------------------------------------------- *
                                    Typedefs
 * ---------------------------------------------------------------------------- */

typedef           char       int8 ;
typedef  unsigned char      uint8 ;
typedef           short      int16 ;
typedef  unsigned short     uint16 ;
typedef           long       int32 ;
typedef  unsigned long      uint32 ;

typedef enum 
{
    FUZZ_LIBS,
    FUZZ_TESTS,
    FUZZ_TRACES,
} ListType ;

/* ---------------------------------------------------------------------------- *
                                     Defines
 * ---------------------------------------------------------------------------- */

#define  VERSION   "3"
#define  MAJOR     "1"
#define  MINOR     "0"

#define  FUZZER_VERSION VERSION"."MAJOR"."MINOR

#define RET_SUCCESS 0
#define RET_FAILURE -1
#define RET_ABORT   -2

#define INT_UNDEF -1

#define MAX_FILE_LEN     256
#define FILE_EXT_LEN     8
#define BUFFER_LEN       512
#define MAX_VAR_NAME_LEN 50
#define MAX_VAR_VAL_LEN  50
#define MAX_ARG_LEN      128
#define MAX_IF_LEN       128

#define FUZZ_SOCKET_PATH  "/data/cFuzzer/sockets"

#define FUZZ_LIBS_FOLDER  "/data/cFuzzer/cFuzzerLibs"
#define FUZZ_TESTS_FOLDER "/data/cFuzzer/cFuzzerTests"
#define FUZZ_TRACE_FOLDER "/data/cFuzzer/cFuzzerTraces"

#define MAX(X,Y) ((X>=Y)?X:Y)
#define MIN(X,Y) ((X<=Y)?X:Y)

#define DELETE_MSG(MSG)                       \
{                                             \
	if(FUZZER_CMD_RESULTS == MSG->hdr.cmd)    \
	{                                         \
		if(MSG->u.res.buffer)                 \
		{                                     \
			delete[] MSG->u.res.buffer ;      \
		}                                     \
	}                                         \
	delete MSG ;                              \
}

extern FILE *traceMallocFP ;
extern int reallocCnt ;
extern int mallocCnt ;
extern int freeCnt ;
extern int newCnt ;
extern int delCnt ;

/* ---------------------------------------------------------------------------- *
                                Memory Instrumentation
 * ---------------------------------------------------------------------------- */

#if defined(FUZZER_MEM_CHECK)

#define MALLOC(SIZE)      malloc_mem(SIZE, __PRETTY_FUNCTION__, __LINE__) 
#define REALLOC(PT1,SIZE) realloc_mem(PT1, SIZE, __PRETTY_FUNCTION__, __LINE__) 
#define FREE(PT)          free_mem(PT, true, __PRETTY_FUNCTION__, __LINE__) 
#define NOFREE(PT)        free_mem(PT, false, __PRETTY_FUNCTION__, __LINE__) 

#define NEWCHAR(SIZE)  new_char_mem(SIZE, __FUNCTION__, __LINE__)
#define NEWINT(SIZE)   new_int_mem(SIZE, __FUNCTION__, __LINE__)
#define DELCHAR(PT)    del_char_mem(PT, __FUNCTION__, __LINE__)
#define DELINT(PT)     del_int_mem(PT, __FUNCTION__, __LINE__)

__inline char *
new_char_mem(int size, const char *fun, int line) 
{
char *pt = NULL ;

	newCnt++ ;
	pt = new char[size]() ;
	if(traceMallocFP)
	{
		fprintf(traceMallocFP, "    new[%p] bytes[%05d] cnt[%05d] @ %.55s %05d\n", pt, size, newCnt, fun, line) ; 
	}
	return pt ;
}

__inline int *
new_int_mem(int size, const char *fun, int line) 
{
int *pt = NULL ;

	newCnt++ ;
	pt = new int[size]() ;
	if(traceMallocFP)
	{
		fprintf(traceMallocFP, "    new[%p] bytes[%05d] cnt[%05d] @ %.55s %05d\n", pt, size, newCnt, fun, line) ; 
	}
	return pt ;
}

__inline void 
del_char_mem(char *pt, const char *fun, int line)
{
	if(!pt) return ;
	delCnt++ ;
	delete[] (char *)pt ;
	if(traceMallocFP)
	{
		fprintf(traceMallocFP, " delete[%p] bytes[xxxxx] cnt[%05d] @ %.55s %.05d\n", pt, delCnt, fun, line) ; 
	}
}

__inline void 
del_int_mem(int *pt, const char *fun, int line)
{
	if(!pt) return ;
	delCnt++ ;
	delete[] (int *)pt ;
	if(traceMallocFP)
	{
		fprintf(traceMallocFP, " delete[%p] bytes[xxxxx] cnt[%05d] @ %.55s %.05d\n", pt, delCnt, fun, line) ; 
	}
}

__inline void *
malloc_mem(int SIZE, const char *fun, int line) 
{ 
void *pt = NULL ;

	mallocCnt++ ;
	pt = malloc(SIZE) ; 
	if(traceMallocFP)
	{
		fprintf(traceMallocFP, " malloc[%p] bytes[%05d] cnt[%05d] @ %.55s %05d\n", pt, SIZE, mallocCnt, fun, line) ; 
	}
	return pt ;
}

__inline void *
realloc_mem(void *rap, int size, const char *fun, int line) 
{ 
void *pt = NULL ;
void *sv = rap ;

	pt = realloc(sv, size) ; 

	if(traceMallocFP)
	{
		if(pt != rap)
		{
//			printf("realloc_mem: new pt(%p) == old rap(%p) sv(%p)\n", pt, rap, sv) ;
			fprintf(traceMallocFP, "realloc[%p] bytes[%05d] cnt[%05d] @ %.55s %05d\n", pt, size, reallocCnt, fun, line) ; 
			reallocCnt++ ;
		}
		if((pt != rap) && (0 != rap))
		{
			fprintf(traceMallocFP, " refree sv[%p] (rap[%p]==pt[%p])? bytes[xxxxx] cnt[%05d] @ %.55s %.05d\n", sv, rap, pt, reallocCnt, fun, line) ; 
//			printf("realloc_mem: eureka -- new pt(%p) != old rap(%p) sv(%p)\n", pt, rap, sv) ;
		}
	}
	return pt ;
}

__inline void
free_mem(void *PT, bool freeMem, const char *fun, int line) 
{
	if(!PT) return ;
	freeCnt++ ;

	if(false == freeMem)
	{
		fprintf(traceMallocFP, " nofree[%p] bytes[xxxxx] cnt[%05d] @ %.55s %.05d\n", PT, freeCnt, fun, line) ; 
		return ;
	}
	if(traceMallocFP)
	{
		fprintf(traceMallocFP, "   free[%p] bytes[xxxxx] cnt[%05d] @ %.55s %.05d\n", PT, freeCnt, fun, line) ; 
	}
	free(PT) ;
}
#else

#define MALLOC(SIZE)     malloc(SIZE)
#define REALLOC(PT,SIZE) realloc(PT,SIZE)
#define FREE(PT)         free(PT) 
#endif

#endif
