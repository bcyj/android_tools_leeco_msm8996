/*============================================================================

   Copyright  2011 Qualcomm Technologies, Inc. All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.

============================================================================*/



/*========================================================================
                             Edit History

$Header:

when       who     what, where, why
--------   ---     -------------------------------------------------------
06/05/10   vp/wl   Initial version
========================================================================== */

/* =======================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */
#include "ppfdwtinterface.h"



/*============================================================================
                      DEFINITIONS and CONSTANTS
============================================================================*/

/*============================================================================
                        FUNCTIONS
============================================================================*/
/*===========================================================================

Function            : ppfDwtFetch

Description         : Fetch content from Pmem to heap

Input parameter(s)  : Input buffer Y pointer
                      Output buffer Y pointer
					  Input buffer Cb pointer
                      Output buffer Cb pointer
					  Input buffer Cr pointer
                      Output buffer Cr pointer
                      numberoflines
                      imagewidth
                      image format

Output parameter(s) : TRUE/FALSE

Return Value        : None

Side Effects        : None

=========================================================================== */
boolean ppfDwtFetch (uint8 *pYInputBuffer,
                  	 uint8 *pYOutputBuffer,
                  	 uint8 *pCbInputBuffer,
                  	 uint8 *pCbOutputBuffer,
                  	 uint8 *pCrInputBuffer,
                  	 uint8 *pCrOutputBuffer,
                     uint32 numberOfLines,
                     uint32 imageWidth,
                     subsample_format_type imageFormat)
{

	if (pYInputBuffer!=NULL && pYOutputBuffer!=NULL)
		memcpy(pYOutputBuffer,pYInputBuffer,(imageWidth * numberOfLines));

	if (imageFormat == DENOISE_H2V2)
	{
		if (pCbInputBuffer!=NULL && pCbOutputBuffer!=NULL)
			memcpy(pCbOutputBuffer,pCbInputBuffer,((imageWidth>>1) * (numberOfLines>>1)));

		if (pCrInputBuffer!=NULL && pCrOutputBuffer!=NULL)
			memcpy(pCrOutputBuffer,pCrInputBuffer,((imageWidth>>1) * (numberOfLines>>1)));
	}
	else if (imageFormat == DENOISE_H2V1)
	{
		if (pCbInputBuffer!=NULL && pCbOutputBuffer!=NULL)
			memcpy(pCbOutputBuffer,pCbInputBuffer,((imageWidth>>1) * (numberOfLines)));

		if (pCrInputBuffer!=NULL && pCrOutputBuffer!=NULL)
			memcpy(pCrOutputBuffer,pCrInputBuffer,((imageWidth>>1) * (numberOfLines)));
	}
	else if (imageFormat == DENOISE_H1V2)
	{
		if (pCbInputBuffer!=NULL && pCbOutputBuffer!=NULL)
			memcpy(pCbOutputBuffer,pCbInputBuffer,((imageWidth) * (numberOfLines>>1)));

		if (pCrInputBuffer!=NULL && pCrOutputBuffer!=NULL)
			memcpy(pCrOutputBuffer,pCrInputBuffer,((imageWidth) * (numberOfLines>>1)));
	}
	else if (imageFormat == DENOISE_H1V1)
	{
		if (pCbInputBuffer!=NULL && pCbOutputBuffer!=NULL)
			memcpy(pCbOutputBuffer,pCbInputBuffer,((imageWidth) * (numberOfLines)));

		if (pCrInputBuffer!=NULL && pCrOutputBuffer!=NULL)
			memcpy(pCrOutputBuffer,pCrInputBuffer,((imageWidth) * (numberOfLines)));
	}

	return TRUE;

}

/*===========================================================================

Function            : ppfDwtWrite

Description         : Write content from heap to pmem

Input parameter(s)  : Input buffer Y pointer
                      Output buffer Y pointer
					  Input buffer Cb pointer
                      Output buffer Cb pointer
					  Input buffer Cr pointer
                      Output buffer Cr pointer
                      numberoflines
                      imagewidth
                      image format

Output parameter(s) : TRUE/FALSE

Return Value        : none

Side Effects        : None

=========================================================================== */
boolean ppfDwtWrite (uint8 *pYInputBuffer,
                  uint8 *pYOutputBuffer,
                  uint8 *pCbInputBuffer,
                  uint8 *pCbOutputBuffer,
                  uint8 *pCrInputBuffer,
                  uint8 *pCrOutputBuffer,
                  uint32 numberOfLines,
                  uint32 imageWidth,
                  subsample_format_type imageFormat)

{

	if (pYInputBuffer!=NULL && pYOutputBuffer!=NULL)
		memcpy(pYOutputBuffer,pYInputBuffer,(imageWidth * numberOfLines));

	if (imageFormat==DENOISE_H2V2)
	{
		if (pCbInputBuffer!=NULL && pCbOutputBuffer!=NULL)
			memcpy(pCbOutputBuffer,pCbInputBuffer,((imageWidth>>1) * (numberOfLines>>1)));

		if (pCrInputBuffer!=NULL && pCrOutputBuffer!=NULL)
			memcpy(pCrOutputBuffer,pCrInputBuffer,((imageWidth>>1) * (numberOfLines>>1)));
	}
	else if (imageFormat==DENOISE_H2V1)
	{
		if (pCbInputBuffer!=NULL && pCbOutputBuffer!=NULL)
			memcpy(pCbOutputBuffer,pCbInputBuffer,((imageWidth>>1) * (numberOfLines)));

		if (pCrInputBuffer!=NULL && pCrOutputBuffer!=NULL)
			memcpy(pCrOutputBuffer,pCrInputBuffer,((imageWidth>>1) * (numberOfLines)));
	}
	else if (imageFormat==DENOISE_H1V2)
	{
		if (pCbInputBuffer!=NULL && pCbOutputBuffer!=NULL)
			memcpy(pCbOutputBuffer,pCbInputBuffer,((imageWidth) * (numberOfLines>>1)));

		if (pCrInputBuffer!=NULL && pCrOutputBuffer!=NULL)
			memcpy(pCrOutputBuffer,pCrInputBuffer,((imageWidth) * (numberOfLines>>1)));
	}
	else if (imageFormat==DENOISE_H1V1)
	{
		if (pCbInputBuffer!=NULL && pCbOutputBuffer!=NULL)
			memcpy(pCbOutputBuffer,pCbInputBuffer,((imageWidth) * (numberOfLines)));

		if (pCrInputBuffer!=NULL && pCrOutputBuffer!=NULL)
			memcpy(pCrOutputBuffer,pCrInputBuffer,((imageWidth) * (numberOfLines)));
	}


	return TRUE;
}
