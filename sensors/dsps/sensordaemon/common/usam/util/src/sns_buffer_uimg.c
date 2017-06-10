/*============================================================================
  @file sns_buffer_uimg.c

  Circular buffer utility source file

  Copyright (c) 2010-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#include "sns_buffer.h"

/*=========================================================================
  FUNCTION:  sns_buffer_insert
  =======================================================================*/
/*!
    @brief Reset the buffer

    @param[i] buffer: pointer to buffer
    @param[i] data: pointer to data to be inserted in buffer

    @return None
*/
/*=======================================================================*/
void sns_buffer_insert(
   sns_buffer_type* buffer,
   const float* newData)
{
   uint32_t i;

   for (i=0; i < buffer->numDataAxes; i++)
   {
      SNS_BUF_ELEM(buffer, buffer->end, i) = newData[i];
   }

   buffer->end = SNS_BUF_INDEX(buffer->end+1, buffer->numDataValues);

   if (buffer->count < buffer->numDataValues)
   {
      buffer->count++;
   }
}
