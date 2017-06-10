/*****************************************************************************
* Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.       *
* Qualcomm Technologies Proprietary and Confidential.                        *
*****************************************************************************/

#ifndef __QICOMMON_H__
#define __QICOMMON_H__

#include <unistd.h>
#include <math.h>
#include <stdint.h>


/** Jpeg framework error values
*    QI_SUCCESS - success
*    QI_ERR_GENERAL - any generic errors which cannot be defined
*    QI_ERR_NO_MEMORY - memory failure ION or heap
*    QI_ERR_NOT_SUPPORTED -  mode or operation not supported
*    QI_ERR_INVALID_INPUT - input passed by the user is invalid
*    QI_ERR_INVALID_OPERATION - operation sequence is invalid
*    QI_ERR_TIMEOUT - operation timed out
*    QI_ERR_NOT_FOUND - object is not found
*    QI_ERR_OUT_OF_BOUNDS - input to the function is out of
*                           bounds
**/
#define QI_SUCCESS                   0
#define QI_ERR_GENERAL              -1
#define QI_ERR_NO_MEMORY            -2
#define QI_ERR_NOT_SUPPORTED        -3
#define QI_ERR_INVALID_INPUT        -4
#define QI_ERR_INVALID_OPERATION    -5
#define QI_ERR_TIMEOUT              -6
#define QI_ERR_NOT_FOUND            -7
#define QI_ERR_OUT_OF_BOUNDS        -8

/** Jpeg marker type
**/
typedef uint8_t jpeg_marker_t;

/** QISubsampling: Image subsampling type
*    QI_H2V2 - h2v2 subsampling (4:2:0)
*    QI_H2V1 - h2v1 subsampling (4:2:2)
*    QI_H1V2 - h1v2 subsampling (4:2:2)
*    QI_H1V1 - h1v1 subsampling (4:4:4)
**/
typedef enum _QISubsampling {
  QI_H2V2,
  QI_H2V1,
  QI_H1V2,
  QI_H1V1,
} QISubsampling;

/** QISubsampling: Image format
*    QI_YCRCB_SP   - YCrCb semiplanar
*    QI_YCBCR_SP   - YCbCr semiplanar
*    QI_IYUV       - YCrCb planar
*    QI_YUV2       - YCbCr planar
*    QI_MONOCHROME - monochrome format
*    QI_BITSTREAM  - bitstream format
**/
typedef enum QIFormat{
  QI_YCRCB_SP,
  QI_YCBCR_SP,
  QI_IYUV,
  QI_YUV2,
  QI_MONOCHROME,
  QI_BITSTREAM,
}QIFormat;

/** QIMIN - returns the minimum of 2 variables
*    @x - variable 1
*    @y - variable 2
**/
#define QIMIN(x,y) (((x)<(y)) ? (x) : (y))

/** QIMAX - returns the maximum of 2 variables
*    @x - variable 1
*    @y - variable 2
**/
#define QIMAX(x,y) (((x)>(y)) ? (x) : (y))

/** QIF_EQUAL - returns true if the floating pt values are same
*    @a - variable 1
*    @b - variable 2
**/
#define QIF_EQUAL(a, b) \
  ( fabs(a-b) < 1e-4 )

/** QIABS - returns the absolute value of the integer
*    @x - variable
**/
#define QIABS(x) (((x) < 0) ? -(x) : (x))

/** QICLAMP - clamps the value between min and max
*    @x - variable
*    @min - minimum value
*    @max - maximum value
**/
#define QICLAMP(x, min, max) QIMAX (QIMIN (x, max), min)

/** CEILING16 - rounds the value to ceiling 16
*    @x - value
**/
#define CEILING16(X) (((X) + 0x000F) & 0xFFF0)

/** CEILING8 - rounds the value to ceiling 8
*    @x - value
**/
#define CEILING8(X)  (((X) + 0x0007) & 0xFFF8)

/** CEILING2 - rounds the value to ceiling 2
*    @x - value
**/
#define CEILING2(X)  (((X) + 0x0001) & 0xFFFE)

/** FLOOR16 - rounds the value to floor 16
*    @x - value
**/
#define FLOOR16(X) ((X) & 0xFFF0)

/** FLOOR8 - rounds the value to floor 8
*    @x - value
**/
#define FLOOR8(X)  ((X) & 0xFFF8)

/** FLOAT_TO_Q - converts the value in float to exponential of 2
*    @exp - power to 2 for which the variable needs to be
*           converted
*    @f   - floating point value
**/
#define FLOAT_TO_Q(exp, f) \
  ((int32_t)(((f)*(1<<(exp))) + (((f)<0) ? -0.5 : 0.5)))

/** QI_LOCK - abstraction for mutex lock.
*    @m - pointer to mutex variable
**/
#define QI_LOCK(m) pthread_mutex_lock(m)

/** QI_UNLOCK - abstraction for mutex unlock.
*    @m - pointer to mutex variable
**/
#define QI_UNLOCK(m) pthread_mutex_unlock(m)

/** QI_SIGNAL - abstraction for condition signal.
*    @c - pointer to condition variable
**/
#define QI_SIGNAL(c) pthread_cond_signal(c)

/** QI_WAIT - abstraction for condition wait.
*    @c - pointer to condition variable
*    @m - pointer to mutex variable
**/
#define QI_WAIT(c, m) pthread_cond_wait(c, m)

/** QI_MUTEX_INIT - abstraction for mutex initialization.
*    @m - pointer to mutex variable
**/
#define QI_MUTEX_INIT(m) pthread_mutex_init(m, NULL)

/** QI_COND_INIT - abstraction for condition initialization.
*    @c - pointer to condition variable
**/
#define QI_COND_INIT(c) pthread_cond_init(c, NULL)

/** QI_MUTEX_DESTROY - abstraction for mutex destroy.
*    @m - pointer to mutex variable
**/
#define QI_MUTEX_DESTROY(m) pthread_mutex_destroy(m)

/** QI_COND_DESTROY - abstraction for condition destroy.
*    @c - pointer to condition variable
**/
#define QI_COND_DESTROY(c) pthread_cond_destroy(c)

/** QI_ERROR - returns true if the status is error
*    @v - status variable
**/
#define QI_ERROR(v) (v != QI_SUCCESS)

/** QI_SUCCEEDED - returns true if the status is success
*    @v - status variable
**/
#define QI_SUCCEEDED(v) (v == QI_SUCCESS)

/** QI_ERROR_RET - returns the function if the status is error
*    @v - status variable
**/
#define QI_ERROR_RET(v) ({ \
  int rc = (v); \
  if (rc != QI_SUCCESS) { \
    QIDBG_ERROR("%s:%d] failed", __func__, __LINE__); \
    return rc; \
  } \
})

/** QI_ERROR_RET_UNLOCK - returns the function if the status is
*                         error. The mutex is unlocked before
*                         returning
*    @v - status variable
**/
#define QI_ERROR_RET_UNLOCK(v, m) ({ \
  int rc = (v); \
  if (rc != QI_SUCCESS) { \
    QIDBG_ERROR("%s:%d] failed", __func__, __LINE__); \
    QI_UNLOCK(m); \
    return rc; \
  } \
})

/** DUMP_TO_FILE - dump the image to file
*    @filename - output filename
*    @p_addr - buffer address
*    @len - buffer length
**/
#define DUMP_TO_FILE(filename, p_addr, len) ({ \
  int rc = 0; \
  FILE *fp = fopen(filename, "w+"); \
  if (fp) { \
    rc = fwrite(p_addr, 1, len, fp); \
    fclose(fp); \
  } \
})
#endif //__QICOMMON_H__
