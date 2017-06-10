#ifndef _FUZZERLIBS_H_
#define _FUZZERLIBS_H_

/* ========================================================================= *
   Purpose:  FuzzerLibs header file

   Copyright (c) 2005-2013 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.

* ========================================================================= */
#if defined(KERNEL_MODULE_COMPILE)
/* Necessary includes for device drivers */
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>     /* printk() */
#include <linux/slab.h>       /* kmalloc() */
#include <linux/fs.h>         /* everything... */
#include <linux/errno.h>      /* error codes */
#include <linux/types.h>      /* size_t */
#include <linux/proc_fs.h>
#include <linux/fcntl.h>      /* O_ACCMODE */
#include <linux/workqueue.h>  /* work queues, like tasklets */
#include <asm/system.h>       /* cli(), *_flags */
#include <asm/uaccess.h>      /* copy_from/to_user */
#include <linux/device.h>     /* class_creatre */
#include <linux/cdev.h>       /* cdev_init */
#endif

#define LIB_VERSION   "5"
#define LIB_MAJOR     "1"
#define LIB_MINOR     "3"

#define VERSION_INFO LIB_VERSION"."LIB_MAJOR"."LIB_MINOR

#define RET_SUCCESS 0
#define RET_FAILURE -1

#define MAX_FILE_LEN     256
#define MAX_API_NAME_LEN 64
#define MAX_ARGS         20

// Structure holding the fuzzed input arguments
#pragma pack(1)
typedef struct FuzzedArgs
{
    void *vArg[MAX_ARGS];
    void *vReturnArg;
    int   nRetMemSize;
}FuzzedArgs;
#pragma pack()

typedef int (*injAPI_t)(FuzzedArgs*); // Invocation Function pointer definition

// Enum describing the input argument type
typedef enum
{
    DTYPE_FIRST,
    DTYPE_VOID,
    DTYPE_CHAR,
    DTYPE_SHORT_INT,
    DTYPE_INT,
    DTYPE_LONG_INT,
    DTYPE_BOOL,
    DTYPE_FLOAT,
    DTYPE_DOUBLE,
    DTYPE_LONG_DOUBLE,
    DTYPE_LAST,
}DataType;

// Enum describing the data specifier
typedef enum
{
    DSPECIFIER_FIRST,
    DSPECIFIER_SIGNED,
    DSPECIFIER_UNSIGNED,
    DSPECIFIER_LAST
}DataSpecifier;

//Structure that holds the details of the API arguments.
#pragma pack(1)
typedef struct ArgDefn
{
    int               nPosition;
    bool              bIsPointer;
    bool              bIsPrimitive;
    bool              bIsStructure;
    bool              bIsArray;
    bool              bIsUnknownType;
    bool              bIsUserDefined;
    int               nArraySize;
    int               nMemSize;
    DataType          vDataType;
    DataSpecifier     vDataSpecifier;
    int               nStructID;
}ArgDefn;
#pragma pack()

// Structure holding the API Description
#pragma pack(1)
typedef struct APIDefn
{
    char             cLibraryName[MAX_FILE_LEN];
    char             cAPIName[MAX_API_NAME_LEN];
    int              nNumArgs;
    injAPI_t         vFuncPtr;
    ArgDefn          vArgDefn[MAX_ARGS];
    ArgDefn          vReturnType;
}APIDefn;
#pragma pack()


typedef int (*regFunc_t)(APIDefn *vAPI);                                                        // Registration function pointer definition
typedef int (*signalCondVar_t)(const char *cpCallbFuncName, void *vRetValue, int nRetBytes);    // Signal Wait function pointer definition

#endif
