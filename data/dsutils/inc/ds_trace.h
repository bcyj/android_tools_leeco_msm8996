
/******************************************************************************

                        D S _ T R A C E . H

******************************************************************************/

/******************************************************************************

  @file    ds_trace.h
  @brief   dsutils Ftrace Functions Header File

  DESCRIPTION
  Header file for dsutils Ftrace functions.

  ---------------------------------------------------------------------------
  Copyright (c) 2014 Qualcomm Technologies, Inc.
  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------

******************************************************************************/

/******************************************************************************

                      EDIT HISTORY FOR FILE

  $Id:$

when       who        what, where, why
--------   ---        -------------------------------------------------------
05/20/14   sr         Initial version

******************************************************************************/

#ifndef _DS_TRACE_H__
#define _DS_TRACE_H__

#ifdef FEATURE_DS_TRACE_ON

void ds_open_trace_marker_file_desc();

void ds_trace_marker_write(const char *format, ...);

/*===========================================================================
   DS_OPEN_TRACE_MARKER
===========================================================================*/
/*!
    @brief
    This macro opens a file descriptor for the trace_marker file.
    It must be called once prior to writing to trace_marker.
*/
/*=========================================================================*/
#define DS_OPEN_TRACE_MARKER ds_open_trace_marker_file_desc()

/*===========================================================================
   DS_MARK_TRACE_PARAMS
===========================================================================*/
/*!
    @brief
    This macro writes to the trace_marker file

    @param[in] format: The format string
    @param[in] ...:    Other parameters to be printed according to the format
                       string.

    DS_OPEN_TRACE_MARKER must be called prior to calling this function.
*/
/*=========================================================================*/
#define DS_MARK_TRACE_PARAMS(format, ...) ds_trace_marker_write(format, __VA_ARGS__)

/*===========================================================================
   DS_MARK_TRACE
===========================================================================*/
/*!
    @brief
    This macro writes to the trace_marker file

    @param[in] str: The string to write to trace_marker

    DS_OPEN_TRACE_MARKER must be called prior to calling this function.
*/
/*=========================================================================*/
#define DS_MARK_TRACE(str) ds_trace_marker_write(str)

#else /* FEATURE_DS_TRACE_ON */

#define DS_OPEN_TRACE_MARKER

#define DS_MARK_TRACE_PARAMS(format, ...)

#define DS_MARK_TRACE(str)

#endif  /* FEATURE_DS_TRACE_ON */

#endif /* _DS_TRACE_H__ */
