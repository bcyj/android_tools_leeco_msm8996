/*===========================================================================
    FILE:           acdb_utility.c

    OVERVIEW:       This file contains the acdb init utility functions
                    implemented specifically in the win32 environment.

    DEPENDENCIES:   None

                    Copyright (c) 2010-2014 Qualcomm Technologies, Inc.
                    All Rights Reserved.
                    Qualcomm Technologies Proprietary and Confidential.
========================================================================== */

/*===========================================================================
    EDIT HISTORY FOR MODULE

    This section contains comments describing changes made to the module.
    Notice that changes are listed in reverse chronological order. Please
    use ISO format for dates.

    $Header: //source/qcom/qct/multimedia2/Audio/audcal4/acdb_sw/main/latest/acdb/src/acdb_utility.c#2 $

    when        who     what, where, why
    ----------  ---     -----------------------------------------------------
    2010-07-08  vmn     Initial revision.

========================================================================== */

/* ---------------------------------------------------------------------------
 * Include Files
 *--------------------------------------------------------------------------- */

#include "acdb_utility.h"

int32_t AcdbDataCompareIndices(uint32_t* lookup,uint32_t *key,
                                           int num_params)
{
   int32_t i = 0;
   uint32_t* lookupVal = NULL;
   uint32_t* keyVal = NULL;

   // Assuming that the indices are all uint32_t.
   for(i = 0;i<num_params; i++)
      {
          lookupVal = lookup+i;
          keyVal = key+i;

          if(*lookupVal > *keyVal)
          {
              return 1;
          }
          else if(*lookupVal < *keyVal)
          {
              return -1;
          }
      }
   return 0;
}
int32_t AcdbDataBinarySearch(void *voidLookUpArray, int32_t max,int32_t indexCount,
            void *pCmd, int32_t nNoOfIndsCount,uint32_t *index)
{
   int32_t result = SEARCH_ERROR;
   int32_t min = 0;
   int32_t mid = 0;
   int32_t compareResult = 0;

   uint32_t *lookUpArray = (uint32_t *)voidLookUpArray;

   while(max >= min)
   {
       mid = (min + max)/2;

       compareResult = AcdbDataCompareIndices(&lookUpArray[indexCount * mid],
                                   (uint32_t *)pCmd,nNoOfIndsCount);

       if(compareResult > 0) // search upper array
       {
           max = mid - 1;
       }
       else if(compareResult < 0) // search lower array
       {
           min = mid + 1;
       }
       else
       {
		   // If its a partial search then the found index could no be the very first item
		   // Find the first occurence of this element by going backward
		   while(0 == AcdbDataCompareIndices(&lookUpArray[indexCount * (mid-1)],
                                   (uint32_t *)pCmd,nNoOfIndsCount))
		   {
			   mid = mid - 1;
		   }
		   *index = (uint32_t)mid;
           result = SEARCH_SUCCESS;
           break;
       }
   }

   return result;
}
