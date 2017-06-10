/* ========================================================================= * 
   Purpose:  Shared object library used for fuzzing APIs

           -------------------------------------------------------
      Copyright © 2012 Qualcomm Technologies, Inc. All Rights Reserved.
             Qualcomm Technologies Proprietary and Confidential.
* ========================================================================= */
#include <stdio.h>
#include <string.h>
#include "cFuzzerLog.h"

#include "mmstillomxencHelper.h" 
#include "mmstillomxenc.h" 
#include "mmstill_jpeg_omx_enc.h" 

const char *lib_name="libFuzzmmstillomxenc" ;
cFuzzerLog cFL ;

#define NUM_OF_APIS 39 
#define NUM_OF_MACROS 1 
/*  DO NOT REMOVE --<UpdateCodeTag_IncludeHeaderFiles>-- */ 
signalCondVar_t g_signalCondVar = NULL;

void InitArg(ArgDefn *vArgDefn);

/*==========================================================================*
	Name: int init_module()

	Input:
	       None   

	Return: 
           SUCCESS : RET_SUCCESS ( 0) 
           FAILURE : RET_FAILURE (-1)

	Purpose:
	  This is the init function. Make all initializations here 
*==========================================================================*/
extern "C" int init_module()
{
	//LogClientSetup(&cFL, lib_name) ;
    //DBGPRT(LEVEL_INFO, (char *)"*****Init Called********");
    
    // Please add your initialization code in init_module_helper() 
	// and NOT HERE
	init_module_helper();    
    
    return RET_SUCCESS;
}

/*==========================================================================*
	Name: int deinit_module()

	Input:
	       None   

	Return: 
           SUCCESS : RET_SUCCESS ( 0) 
           FAILURE : RET_FAILURE (-1)

	Purpose:
	  This is the de-init function. Free all resources here 
*==========================================================================*/
extern "C" int deinit_module()
{
    //DBGPRT(LEVEL_INFO, (char *)"*****Deinit Called********");
    
    // Please add your de initialization code in deinit_module_helper() 
	// and NOT HERE
	deinit_module_helper();    
    
    return RET_SUCCESS;
}

/*==========================================================================*
	Name: int sigWaitFunc()

	Input:
	       signalCondVar_t signalCondVar : Condition variable signalling
                                           function pointer           

	Return: 
           SUCCESS : RET_SUCCESS ( 0) 
           FAILURE : RET_FAILURE (-1)

	Purpose:
	  This is the function that registers the function pointer needed to 
      signal the condition variable
*==========================================================================*/
extern "C" int sigWaitFunc(signalCondVar_t signalCondVar)
{
    //DBGPRT(LEVEL_INFO, (char *)"*****sigWaitFunc Called********");
    
    //
    //  Below code is generated. Do not modify.
    //
    //
	g_signalCondVar = signalCondVar;    
    
    return RET_SUCCESS;
}

/*==========================================================================*
	Name: int register_apis(regFunc_t regFunc) 

	Input:
	       regFunc_t regFunc : Registration function pointer   

	Return: 
           SUCCESS : RET_SUCCESS ( 0) 
           FAILURE : RET_FAILURE (-1)

	Purpose:
	  This is the registration section. Code generated. Do not modify.
*==========================================================================*/
extern "C" int register_apis(regFunc_t regFunc)
{
    //
    //  Below code is generated. Do not modify.
    //
    //
    
    //DBGPRT(LEVEL_INFO, (char *)"*****Register APIs Called********");
    int nRetVal = 0;
    
	{
		APIDefn vAPI;
		strlcpy(vAPI.cLibraryName, "libFuzzmmstillomxenc.so", MAX_FILE_LEN);
		strlcpy(vAPI.cAPIName, "JpegOMX_FreeHandle", MAX_API_NAME_LEN);
		vAPI.vFuncPtr     = &JpegOMX_FreeHandle_WRAP;
		vAPI.nNumArgs     = 1;

		InitArg(&(vAPI.vArgDefn[0]));
		vAPI.vArgDefn[0].nPosition = 0;
		vAPI.vArgDefn[0].nMemSize = sizeof(int);
		vAPI.vArgDefn[0].vDataType = DTYPE_INT;
		vAPI.vArgDefn[0].vDataSpecifier = DSPECIFIER_FIRST;
		vAPI.vArgDefn[0].bIsPrimitive = true;

		InitArg(&(vAPI.vReturnType));
		vAPI.vReturnType.nMemSize = sizeof(int);
		vAPI.vReturnType.vDataType = DTYPE_INT;
		vAPI.vReturnType.vDataSpecifier = DSPECIFIER_FIRST;
		vAPI.vReturnType.bIsPrimitive = true;
		nRetVal = (*regFunc)(&vAPI);
		if(RET_SUCCESS != nRetVal)
		{
			//DBGPRT(LEVEL_ERROR, (char *)"Function : JpegOMX_FreeHandle registration failed!!!");
			return RET_FAILURE;
		}
	}
#ifndef OMX_CODEC_V1_WRAPPER
	{
		APIDefn vAPI;
		strlcpy(vAPI.cLibraryName, "libFuzzmmstillomxenc.so", MAX_FILE_LEN);
		strlcpy(vAPI.cAPIName, "JpegOMX_SetParameter_ACbCrOffset", MAX_API_NAME_LEN);
		vAPI.vFuncPtr     = &JpegOMX_SetParameter_ACbCrOffset_WRAP;
		vAPI.nNumArgs     = 2;

		InitArg(&(vAPI.vArgDefn[0]));
		vAPI.vArgDefn[0].nPosition = 0;
		vAPI.vArgDefn[0].nMemSize = sizeof(int);
		vAPI.vArgDefn[0].vDataType = DTYPE_INT;
		vAPI.vArgDefn[0].vDataSpecifier = DSPECIFIER_FIRST;
		vAPI.vArgDefn[0].bIsPrimitive = true;

		InitArg(&(vAPI.vArgDefn[1]));
		vAPI.vArgDefn[1].nPosition = 1;
		vAPI.vArgDefn[1].nMemSize = sizeof( omx_jpeg_buffer_offset );
		vAPI.vArgDefn[1].vDataType = DTYPE_FIRST;
		vAPI.vArgDefn[1].vDataSpecifier = DSPECIFIER_FIRST;
		vAPI.vArgDefn[1].bIsStructure = true;

		InitArg(&(vAPI.vReturnType));
		vAPI.vReturnType.nMemSize = sizeof(int);
		vAPI.vReturnType.vDataType = DTYPE_INT;
		vAPI.vReturnType.vDataSpecifier = DSPECIFIER_FIRST;
		vAPI.vReturnType.bIsPrimitive = true;
		nRetVal = (*regFunc)(&vAPI);
		if(RET_SUCCESS != nRetVal)
		{
			//DBGPRT(LEVEL_ERROR, (char *)"Function : JpegOMX_SetParameter_ACbCrOffset registration failed!!!");
			return RET_FAILURE;
		}
	}
#endif
	{
		APIDefn vAPI;
		strlcpy(vAPI.cLibraryName, "libFuzzmmstillomxenc.so", MAX_FILE_LEN);
		strlcpy(vAPI.cAPIName, "JpegOMX_DeInit", MAX_API_NAME_LEN);
		vAPI.vFuncPtr     = &JpegOMX_DeInit_WRAP;
		vAPI.nNumArgs     = 0;

		InitArg(&(vAPI.vReturnType));
		vAPI.vReturnType.nMemSize = sizeof(int);
		vAPI.vReturnType.vDataType = DTYPE_INT;
		vAPI.vReturnType.vDataSpecifier = DSPECIFIER_FIRST;
		vAPI.vReturnType.bIsPrimitive = true;
		nRetVal = (*regFunc)(&vAPI);
		if(RET_SUCCESS != nRetVal)
		{
			//DBGPRT(LEVEL_ERROR, (char *)"Function : JpegOMX_DeInit registration failed!!!");
			return RET_FAILURE;
		}
	}

	{
		APIDefn vAPI;
		strlcpy(vAPI.cLibraryName, "libFuzzmmstillomxenc.so", MAX_FILE_LEN);
		strlcpy(vAPI.cAPIName, "JpegOMX_SetParameter_Thumbnail", MAX_API_NAME_LEN);
		vAPI.vFuncPtr     = &JpegOMX_SetParameter_Thumbnail_WRAP;
		vAPI.nNumArgs     = 8;

		InitArg(&(vAPI.vArgDefn[0]));
		vAPI.vArgDefn[0].nPosition = 0;
		vAPI.vArgDefn[0].nMemSize = sizeof(int);
		vAPI.vArgDefn[0].vDataType = DTYPE_INT;
		vAPI.vArgDefn[0].vDataSpecifier = DSPECIFIER_FIRST;
		vAPI.vArgDefn[0].bIsPrimitive = true;

		InitArg(&(vAPI.vArgDefn[1]));
		vAPI.vArgDefn[1].nPosition = 1;
		vAPI.vArgDefn[1].nMemSize = sizeof(int);
		vAPI.vArgDefn[1].vDataType = DTYPE_INT;
		vAPI.vArgDefn[1].vDataSpecifier = DSPECIFIER_FIRST;
		vAPI.vArgDefn[1].bIsPrimitive = true;

		InitArg(&(vAPI.vArgDefn[2]));
		vAPI.vArgDefn[2].nPosition = 2;
		vAPI.vArgDefn[2].nMemSize = sizeof(int);
		vAPI.vArgDefn[2].vDataType = DTYPE_INT;
		vAPI.vArgDefn[2].vDataSpecifier = DSPECIFIER_FIRST;
		vAPI.vArgDefn[2].bIsPrimitive = true;

		InitArg(&(vAPI.vArgDefn[3]));
		vAPI.vArgDefn[3].nPosition = 3;
		vAPI.vArgDefn[3].nMemSize = sizeof(int);
		vAPI.vArgDefn[3].vDataType = DTYPE_INT;
		vAPI.vArgDefn[3].vDataSpecifier = DSPECIFIER_FIRST;
		vAPI.vArgDefn[3].bIsPrimitive = true;

		InitArg(&(vAPI.vArgDefn[4]));
		vAPI.vArgDefn[4].nPosition = 4;
		vAPI.vArgDefn[4].nMemSize = sizeof(int);
		vAPI.vArgDefn[4].vDataType = DTYPE_INT;
		vAPI.vArgDefn[4].vDataSpecifier = DSPECIFIER_FIRST;
		vAPI.vArgDefn[4].bIsPrimitive = true;

		InitArg(&(vAPI.vArgDefn[5]));
		vAPI.vArgDefn[5].nPosition = 5;
		vAPI.vArgDefn[5].nMemSize = sizeof(int);
		vAPI.vArgDefn[5].vDataType = DTYPE_INT;
		vAPI.vArgDefn[5].vDataSpecifier = DSPECIFIER_FIRST;
		vAPI.vArgDefn[5].bIsPrimitive = true;

		InitArg(&(vAPI.vArgDefn[6]));
		vAPI.vArgDefn[6].nPosition = 6;
		vAPI.vArgDefn[6].nMemSize = sizeof(int);
		vAPI.vArgDefn[6].vDataType = DTYPE_INT;
		vAPI.vArgDefn[6].vDataSpecifier = DSPECIFIER_FIRST;
		vAPI.vArgDefn[6].bIsPrimitive = true;

		InitArg(&(vAPI.vArgDefn[7]));
		vAPI.vArgDefn[7].nPosition = 7;
		vAPI.vArgDefn[7].nMemSize = sizeof(int);
		vAPI.vArgDefn[7].vDataType = DTYPE_INT;
		vAPI.vArgDefn[7].vDataSpecifier = DSPECIFIER_FIRST;
		vAPI.vArgDefn[7].bIsPrimitive = true;

		InitArg(&(vAPI.vReturnType));
		vAPI.vReturnType.nMemSize = sizeof(int);
		vAPI.vReturnType.vDataType = DTYPE_INT;
		vAPI.vReturnType.vDataSpecifier = DSPECIFIER_FIRST;
		vAPI.vReturnType.bIsPrimitive = true;
		nRetVal = (*regFunc)(&vAPI);
		if(RET_SUCCESS != nRetVal)
		{
			//DBGPRT(LEVEL_ERROR, (char *)"Function : JpegOMX_SetParameter_Thumbnail registration failed!!!");
			return RET_FAILURE;
		}
	}

	{
		APIDefn vAPI;
		strlcpy(vAPI.cLibraryName, "libFuzzmmstillomxenc.so", MAX_FILE_LEN);
		strlcpy(vAPI.cAPIName, "JpegOMX_SetParameter_UserPreferences", MAX_API_NAME_LEN);
		vAPI.vFuncPtr     = &JpegOMX_SetParameter_UserPreferences_WRAP;
		vAPI.nNumArgs     = 4;

		InitArg(&(vAPI.vArgDefn[0]));
		vAPI.vArgDefn[0].nPosition = 0;
		vAPI.vArgDefn[0].nMemSize = sizeof(int);
		vAPI.vArgDefn[0].vDataType = DTYPE_INT;
		vAPI.vArgDefn[0].vDataSpecifier = DSPECIFIER_FIRST;
		vAPI.vArgDefn[0].bIsPrimitive = true;

		InitArg(&(vAPI.vArgDefn[1]));
		vAPI.vArgDefn[1].nPosition = 1;
		vAPI.vArgDefn[1].nMemSize = sizeof(int);
		vAPI.vArgDefn[1].vDataType = DTYPE_INT;
		vAPI.vArgDefn[1].vDataSpecifier = DSPECIFIER_FIRST;
		vAPI.vArgDefn[1].bIsPrimitive = true;

		InitArg(&(vAPI.vArgDefn[2]));
		vAPI.vArgDefn[2].nPosition = 2;
		vAPI.vArgDefn[2].nMemSize = sizeof(int);
		vAPI.vArgDefn[2].vDataType = DTYPE_INT;
		vAPI.vArgDefn[2].vDataSpecifier = DSPECIFIER_FIRST;
		vAPI.vArgDefn[2].bIsPrimitive = true;

		InitArg(&(vAPI.vArgDefn[3]));
		vAPI.vArgDefn[3].nPosition = 3;
		vAPI.vArgDefn[3].nMemSize = sizeof(int);
		vAPI.vArgDefn[3].vDataType = DTYPE_INT;
		vAPI.vArgDefn[3].vDataSpecifier = DSPECIFIER_FIRST;
		vAPI.vArgDefn[3].bIsPrimitive = true;

		InitArg(&(vAPI.vReturnType));
		vAPI.vReturnType.nMemSize = sizeof(int);
		vAPI.vReturnType.vDataType = DTYPE_INT;
		vAPI.vReturnType.vDataSpecifier = DSPECIFIER_FIRST;
		vAPI.vReturnType.bIsPrimitive = true;
		nRetVal = (*regFunc)(&vAPI);
		if(RET_SUCCESS != nRetVal)
		{
			//DBGPRT(LEVEL_ERROR, (char *)"Function : JpegOMX_SetParameter_UserPreferences registration failed!!!");
			return RET_FAILURE;
		}
	}

	{
		APIDefn vAPI;
		strlcpy(vAPI.cLibraryName, "libFuzzmmstillomxenc.so", MAX_FILE_LEN);
		strlcpy(vAPI.cAPIName, "JpegOMX_SendJpegCommand", MAX_API_NAME_LEN);
		vAPI.vFuncPtr     = &JpegOMX_SendJpegCommand_WRAP;
		vAPI.nNumArgs     = 2;

		InitArg(&(vAPI.vArgDefn[0]));
		vAPI.vArgDefn[0].nPosition = 0;
		vAPI.vArgDefn[0].nMemSize = sizeof(int);
		vAPI.vArgDefn[0].vDataType = DTYPE_INT;
		vAPI.vArgDefn[0].vDataSpecifier = DSPECIFIER_FIRST;
		vAPI.vArgDefn[0].bIsPrimitive = true;

		InitArg(&(vAPI.vArgDefn[1]));
		vAPI.vArgDefn[1].nPosition = 1;
		vAPI.vArgDefn[1].nMemSize = sizeof( OMX_U32 );
		vAPI.vArgDefn[1].vDataType = DTYPE_LONG_INT;
		vAPI.vArgDefn[1].vDataSpecifier = DSPECIFIER_UNSIGNED;
		vAPI.vArgDefn[1].bIsPrimitive = true;

		InitArg(&(vAPI.vReturnType));
		vAPI.vReturnType.nMemSize = sizeof(int);
		vAPI.vReturnType.vDataType = DTYPE_INT;
		vAPI.vReturnType.vDataSpecifier = DSPECIFIER_FIRST;
		vAPI.vReturnType.bIsPrimitive = true;
		nRetVal = (*regFunc)(&vAPI);
		if(RET_SUCCESS != nRetVal)
		{
			//DBGPRT(LEVEL_ERROR, (char *)"Function : JpegOMX_SendJpegCommand registration failed!!!");
			return RET_FAILURE;
		}
	}

	{
		APIDefn vAPI;
		strlcpy(vAPI.cLibraryName, "libFuzzmmstillomxenc.so", MAX_FILE_LEN);
		strlcpy(vAPI.cAPIName, "StartEncode", MAX_API_NAME_LEN);
		vAPI.vFuncPtr     = &StartEncode_WRAP;
		vAPI.nNumArgs     = 0;

		InitArg(&(vAPI.vReturnType));
		vAPI.vReturnType.nMemSize = sizeof(int);
		vAPI.vReturnType.vDataType = DTYPE_INT;
		vAPI.vReturnType.vDataSpecifier = DSPECIFIER_FIRST;
		vAPI.vReturnType.bIsPrimitive = true;
		nRetVal = (*regFunc)(&vAPI);
		if(RET_SUCCESS != nRetVal)
		{
			//DBGPRT(LEVEL_ERROR, (char *)"Function : StartEncode registration failed!!!");
			return RET_FAILURE;
		}
	}

	{
		APIDefn vAPI;
		strlcpy(vAPI.cLibraryName, "libFuzzmmstillomxenc.so", MAX_FILE_LEN);
		strlcpy(vAPI.cAPIName, "JpegOMX_SetParameter_ImageInit", MAX_API_NAME_LEN);
		vAPI.vFuncPtr     = &JpegOMX_SetParameter_ImageInit_WRAP;
		vAPI.nNumArgs     = 8;

		InitArg(&(vAPI.vArgDefn[0]));
		vAPI.vArgDefn[0].nPosition = 0;
		vAPI.vArgDefn[0].nMemSize = sizeof(int);
		vAPI.vArgDefn[0].vDataType = DTYPE_INT;
		vAPI.vArgDefn[0].vDataSpecifier = DSPECIFIER_FIRST;
		vAPI.vArgDefn[0].bIsPrimitive = true;

		InitArg(&(vAPI.vArgDefn[1]));
		vAPI.vArgDefn[1].nPosition = 1;
		vAPI.vArgDefn[1].nMemSize = sizeof(int);
		vAPI.vArgDefn[1].vDataType = DTYPE_INT;
		vAPI.vArgDefn[1].vDataSpecifier = DSPECIFIER_FIRST;
		vAPI.vArgDefn[1].bIsPrimitive = true;

		InitArg(&(vAPI.vArgDefn[2]));
		vAPI.vArgDefn[2].nPosition = 2;
		vAPI.vArgDefn[2].nMemSize = sizeof(int);
		vAPI.vArgDefn[2].vDataType = DTYPE_INT;
		vAPI.vArgDefn[2].vDataSpecifier = DSPECIFIER_FIRST;
		vAPI.vArgDefn[2].bIsPrimitive = true;

		InitArg(&(vAPI.vArgDefn[3]));
		vAPI.vArgDefn[3].nPosition = 3;
		vAPI.vArgDefn[3].nMemSize = sizeof(int);
		vAPI.vArgDefn[3].vDataType = DTYPE_INT;
		vAPI.vArgDefn[3].vDataSpecifier = DSPECIFIER_FIRST;
		vAPI.vArgDefn[3].bIsPrimitive = true;

		InitArg(&(vAPI.vArgDefn[4]));
		vAPI.vArgDefn[4].nPosition = 4;
		vAPI.vArgDefn[4].nMemSize = sizeof( OMX_U8 );
		vAPI.vArgDefn[4].vDataType = DTYPE_CHAR;
		vAPI.vArgDefn[4].vDataSpecifier = DSPECIFIER_UNSIGNED;
		vAPI.vArgDefn[4].bIsPrimitive = true;

		InitArg(&(vAPI.vArgDefn[5]));
		vAPI.vArgDefn[5].nPosition = 5;
		vAPI.vArgDefn[5].nMemSize = sizeof( OMX_U8 );
		vAPI.vArgDefn[5].vDataType = DTYPE_CHAR;
		vAPI.vArgDefn[5].vDataSpecifier = DSPECIFIER_UNSIGNED;
		vAPI.vArgDefn[5].bIsPrimitive = true;

		InitArg(&(vAPI.vArgDefn[6]));
		vAPI.vArgDefn[6].nPosition = 6;
		vAPI.vArgDefn[6].nMemSize = sizeof( OMX_U8 );
		vAPI.vArgDefn[6].vDataType = DTYPE_CHAR;
		vAPI.vArgDefn[6].vDataSpecifier = DSPECIFIER_UNSIGNED;
		vAPI.vArgDefn[6].bIsPrimitive = true;

		InitArg(&(vAPI.vArgDefn[7]));
		vAPI.vArgDefn[7].nPosition = 7;
		vAPI.vArgDefn[7].nMemSize = sizeof( OMX_U8 );
		vAPI.vArgDefn[7].vDataType = DTYPE_CHAR;
		vAPI.vArgDefn[7].vDataSpecifier = DSPECIFIER_UNSIGNED;
		vAPI.vArgDefn[7].bIsPrimitive = true;

		InitArg(&(vAPI.vReturnType));
		vAPI.vReturnType.nMemSize = sizeof(int);
		vAPI.vReturnType.vDataType = DTYPE_INT;
		vAPI.vReturnType.vDataSpecifier = DSPECIFIER_FIRST;
		vAPI.vReturnType.bIsPrimitive = true;
		nRetVal = (*regFunc)(&vAPI);
		if(RET_SUCCESS != nRetVal)
		{
			//DBGPRT(LEVEL_ERROR, (char *)"Function : JpegOMX_SetParameter_ImageInit registration failed!!!");
			return RET_FAILURE;
		}
	}

	{
		APIDefn vAPI;
		strlcpy(vAPI.cLibraryName, "libFuzzmmstillomxenc.so", MAX_FILE_LEN);
		strlcpy(vAPI.cAPIName, "JpegOMX_SetAllConfigs", MAX_API_NAME_LEN);
		vAPI.vFuncPtr     = &JpegOMX_SetAllConfigs_WRAP;
		vAPI.nNumArgs     = 0;

		InitArg(&(vAPI.vReturnType));
		vAPI.vReturnType.nMemSize = sizeof(int);
		vAPI.vReturnType.vDataType = DTYPE_INT;
		vAPI.vReturnType.vDataSpecifier = DSPECIFIER_FIRST;
		vAPI.vReturnType.bIsPrimitive = true;
		nRetVal = (*regFunc)(&vAPI);
		if(RET_SUCCESS != nRetVal)
		{
			//DBGPRT(LEVEL_ERROR, (char *)"Function : JpegOMX_SetAllConfigs registration failed!!!");
			return RET_FAILURE;
		}
	}

	{
		APIDefn vAPI;
		strlcpy(vAPI.cLibraryName, "libFuzzmmstillomxenc.so", MAX_FILE_LEN);
		strlcpy(vAPI.cAPIName, "JpegOMX_SetParameter_Exif", MAX_API_NAME_LEN);
		vAPI.vFuncPtr     = &JpegOMX_SetParameter_Exif_WRAP;
		vAPI.nNumArgs     = 8;

		InitArg(&(vAPI.vArgDefn[0]));
		vAPI.vArgDefn[0].nPosition = 0;
		vAPI.vArgDefn[0].nMemSize = sizeof(int);
		vAPI.vArgDefn[0].vDataType = DTYPE_INT;
		vAPI.vArgDefn[0].vDataSpecifier = DSPECIFIER_FIRST;
		vAPI.vArgDefn[0].bIsPrimitive = true;

		InitArg(&(vAPI.vArgDefn[1]));
		vAPI.vArgDefn[1].nPosition = 1;
		vAPI.vArgDefn[1].nMemSize = sizeof(int);
		vAPI.vArgDefn[1].vDataType = DTYPE_INT;
		vAPI.vArgDefn[1].vDataSpecifier = DSPECIFIER_FIRST;
		vAPI.vArgDefn[1].bIsPrimitive = true;

		InitArg(&(vAPI.vArgDefn[2]));
		vAPI.vArgDefn[2].nPosition = 2;
		vAPI.vArgDefn[2].nMemSize = sizeof(int);
		vAPI.vArgDefn[2].vDataType = DTYPE_INT;
		vAPI.vArgDefn[2].vDataSpecifier = DSPECIFIER_FIRST;
		vAPI.vArgDefn[2].bIsPrimitive = true;

		InitArg(&(vAPI.vArgDefn[3]));
		vAPI.vArgDefn[3].nPosition = 3;
		vAPI.vArgDefn[3].nMemSize = sizeof(int);
		vAPI.vArgDefn[3].vDataType = DTYPE_INT;
		vAPI.vArgDefn[3].vDataSpecifier = DSPECIFIER_FIRST;
		vAPI.vArgDefn[3].bIsPrimitive = true;

		InitArg(&(vAPI.vArgDefn[4]));
		vAPI.vArgDefn[4].nPosition = 4;
		vAPI.vArgDefn[4].nMemSize = sizeof(int);
		vAPI.vArgDefn[4].vDataType = DTYPE_INT;
		vAPI.vArgDefn[4].vDataSpecifier = DSPECIFIER_FIRST;
		vAPI.vArgDefn[4].bIsPrimitive = true;

		InitArg(&(vAPI.vArgDefn[5]));
		vAPI.vArgDefn[5].nPosition = 5;
		vAPI.vArgDefn[5].nMemSize = sizeof(int);
		vAPI.vArgDefn[5].vDataType = DTYPE_INT;
		vAPI.vArgDefn[5].vDataSpecifier = DSPECIFIER_FIRST;
		vAPI.vArgDefn[5].bIsPrimitive = true;

		InitArg(&(vAPI.vArgDefn[6]));
		vAPI.vArgDefn[6].nPosition = 6;
		vAPI.vArgDefn[6].nMemSize = sizeof(int);
		vAPI.vArgDefn[6].vDataType = DTYPE_INT;
		vAPI.vArgDefn[6].vDataSpecifier = DSPECIFIER_FIRST;
		vAPI.vArgDefn[6].bIsPrimitive = true;

		InitArg(&(vAPI.vArgDefn[7]));
		vAPI.vArgDefn[7].nPosition = 7;
		vAPI.vArgDefn[7].nMemSize = sizeof(char);
		vAPI.vArgDefn[7].bIsPointer = true;
		vAPI.vArgDefn[7].vDataType = DTYPE_CHAR;
		vAPI.vArgDefn[7].vDataSpecifier = DSPECIFIER_FIRST;
		vAPI.vArgDefn[7].bIsPrimitive = true;

		InitArg(&(vAPI.vReturnType));
		vAPI.vReturnType.nMemSize = sizeof(int);
		vAPI.vReturnType.vDataType = DTYPE_INT;
		vAPI.vReturnType.vDataSpecifier = DSPECIFIER_FIRST;
		vAPI.vReturnType.bIsPrimitive = true;
		nRetVal = (*regFunc)(&vAPI);
		if(RET_SUCCESS != nRetVal)
		{
			//DBGPRT(LEVEL_ERROR, (char *)"Function : JpegOMX_SetParameter_Exif registration failed!!!");
			return RET_FAILURE;
		}
	}

	{
		APIDefn vAPI;
		strlcpy(vAPI.cLibraryName, "libFuzzmmstillomxenc.so", MAX_FILE_LEN);
		strlcpy(vAPI.cAPIName, "JpegOMX_GetConfig", MAX_API_NAME_LEN);
		vAPI.vFuncPtr     = &JpegOMX_GetConfig_WRAP;
		vAPI.nNumArgs     = 3;

		InitArg(&(vAPI.vArgDefn[0]));
		vAPI.vArgDefn[0].nPosition = 0;
		vAPI.vArgDefn[0].nMemSize = sizeof(int);
		vAPI.vArgDefn[0].vDataType = DTYPE_INT;
		vAPI.vArgDefn[0].vDataSpecifier = DSPECIFIER_FIRST;
		vAPI.vArgDefn[0].bIsPrimitive = true;

		InitArg(&(vAPI.vArgDefn[1]));
		vAPI.vArgDefn[1].nPosition = 1;
		vAPI.vArgDefn[1].nMemSize = sizeof(int);
		vAPI.vArgDefn[1].vDataType = DTYPE_INT;
		vAPI.vArgDefn[1].vDataSpecifier = DSPECIFIER_FIRST;
		vAPI.vArgDefn[1].bIsPrimitive = true;

		InitArg(&(vAPI.vArgDefn[2]));
		vAPI.vArgDefn[2].nPosition = 2;
		vAPI.vArgDefn[2].nMemSize = sizeof(int);
		vAPI.vArgDefn[2].vDataType = DTYPE_INT;
		vAPI.vArgDefn[2].vDataSpecifier = DSPECIFIER_FIRST;
		vAPI.vArgDefn[2].bIsPrimitive = true;

		InitArg(&(vAPI.vReturnType));
		vAPI.vReturnType.nMemSize = sizeof(int);
		vAPI.vReturnType.vDataType = DTYPE_INT;
		vAPI.vReturnType.vDataSpecifier = DSPECIFIER_FIRST;
		vAPI.vReturnType.bIsPrimitive = true;
		nRetVal = (*regFunc)(&vAPI);
		if(RET_SUCCESS != nRetVal)
		{
			//DBGPRT(LEVEL_ERROR, (char *)"Function : JpegOMX_GetConfig registration failed!!!");
			return RET_FAILURE;
		}
	}

	{
		APIDefn vAPI;
		strlcpy(vAPI.cLibraryName, "libFuzzmmstillomxenc.so", MAX_FILE_LEN);
		strlcpy(vAPI.cAPIName, "JpegOMX_AllFreeBuffer", MAX_API_NAME_LEN);
		vAPI.vFuncPtr     = &JpegOMX_AllFreeBuffer_WRAP;
		vAPI.nNumArgs     = 0;

		InitArg(&(vAPI.vReturnType));
		vAPI.vReturnType.nMemSize = sizeof(int);
		vAPI.vReturnType.vDataType = DTYPE_INT;
		vAPI.vReturnType.vDataSpecifier = DSPECIFIER_FIRST;
		vAPI.vReturnType.bIsPrimitive = true;
		nRetVal = (*regFunc)(&vAPI);
		if(RET_SUCCESS != nRetVal)
		{
			//DBGPRT(LEVEL_ERROR, (char *)"Function : JpegOMX_AllFreeBuffer registration failed!!!");
			return RET_FAILURE;
		}
	}

	{
		APIDefn vAPI;
		strlcpy(vAPI.cLibraryName, "libFuzzmmstillomxenc.so", MAX_FILE_LEN);
		strlcpy(vAPI.cAPIName, "JpegOMX_EmptyThisBuffer", MAX_API_NAME_LEN);
		vAPI.vFuncPtr     = &JpegOMX_EmptyThisBuffer_WRAP;
		vAPI.nNumArgs     = 2;

		InitArg(&(vAPI.vArgDefn[0]));
		vAPI.vArgDefn[0].nPosition = 0;
		vAPI.vArgDefn[0].nMemSize = sizeof(int);
		vAPI.vArgDefn[0].vDataType = DTYPE_INT;
		vAPI.vArgDefn[0].vDataSpecifier = DSPECIFIER_FIRST;
		vAPI.vArgDefn[0].bIsPrimitive = true;

		InitArg(&(vAPI.vArgDefn[1]));
		vAPI.vArgDefn[1].nPosition = 1;
		vAPI.vArgDefn[1].nMemSize = sizeof(int);
		vAPI.vArgDefn[1].vDataType = DTYPE_INT;
		vAPI.vArgDefn[1].vDataSpecifier = DSPECIFIER_FIRST;
		vAPI.vArgDefn[1].bIsPrimitive = true;

		InitArg(&(vAPI.vReturnType));
		vAPI.vReturnType.nMemSize = sizeof(int);
		vAPI.vReturnType.vDataType = DTYPE_INT;
		vAPI.vReturnType.vDataSpecifier = DSPECIFIER_FIRST;
		vAPI.vReturnType.bIsPrimitive = true;
		nRetVal = (*regFunc)(&vAPI);
		if(RET_SUCCESS != nRetVal)
		{
			//DBGPRT(LEVEL_ERROR, (char *)"Function : JpegOMX_EmptyThisBuffer registration failed!!!");
			return RET_FAILURE;
		}
	}

	{
		APIDefn vAPI;
		strlcpy(vAPI.cLibraryName, "libFuzzmmstillomxenc.so", MAX_FILE_LEN);
		strlcpy(vAPI.cAPIName, "JpegOMX_ComponentTunnelRequest", MAX_API_NAME_LEN);
		vAPI.vFuncPtr     = &JpegOMX_ComponentTunnelRequest_WRAP;
		vAPI.nNumArgs     = 5;

		InitArg(&(vAPI.vArgDefn[0]));
		vAPI.vArgDefn[0].nPosition = 0;
		vAPI.vArgDefn[0].nMemSize = sizeof(int);
		vAPI.vArgDefn[0].vDataType = DTYPE_INT;
		vAPI.vArgDefn[0].vDataSpecifier = DSPECIFIER_FIRST;
		vAPI.vArgDefn[0].bIsPrimitive = true;

		InitArg(&(vAPI.vArgDefn[1]));
		vAPI.vArgDefn[1].nPosition = 1;
		vAPI.vArgDefn[1].nMemSize = sizeof(unsigned int);
		vAPI.vArgDefn[1].vDataType = DTYPE_INT;
		vAPI.vArgDefn[1].vDataSpecifier = DSPECIFIER_UNSIGNED;
		vAPI.vArgDefn[1].bIsPrimitive = true;

		InitArg(&(vAPI.vArgDefn[2]));
		vAPI.vArgDefn[2].nPosition = 2;
		vAPI.vArgDefn[2].nMemSize = sizeof(int);
		vAPI.vArgDefn[2].vDataType = DTYPE_INT;
		vAPI.vArgDefn[2].vDataSpecifier = DSPECIFIER_FIRST;
		vAPI.vArgDefn[2].bIsPrimitive = true;

		InitArg(&(vAPI.vArgDefn[3]));
		vAPI.vArgDefn[3].nPosition = 3;
		vAPI.vArgDefn[3].nMemSize = sizeof(unsigned int);
		vAPI.vArgDefn[3].vDataType = DTYPE_INT;
		vAPI.vArgDefn[3].vDataSpecifier = DSPECIFIER_UNSIGNED;
		vAPI.vArgDefn[3].bIsPrimitive = true;

		InitArg(&(vAPI.vArgDefn[4]));
		vAPI.vArgDefn[4].nPosition = 4;
		vAPI.vArgDefn[4].nMemSize = sizeof( OMX_TUNNELSETUPTYPE );
		vAPI.vArgDefn[4].vDataType = DTYPE_FIRST;
		vAPI.vArgDefn[4].vDataSpecifier = DSPECIFIER_FIRST;
		vAPI.vArgDefn[4].bIsStructure = true;

		InitArg(&(vAPI.vReturnType));
		vAPI.vReturnType.nMemSize = sizeof(int);
		vAPI.vReturnType.vDataType = DTYPE_INT;
		vAPI.vReturnType.vDataSpecifier = DSPECIFIER_FIRST;
		vAPI.vReturnType.bIsPrimitive = true;
		nRetVal = (*regFunc)(&vAPI);
		if(RET_SUCCESS != nRetVal)
		{
			//DBGPRT(LEVEL_ERROR, (char *)"Function : JpegOMX_ComponentTunnelRequest registration failed!!!");
			return RET_FAILURE;
		}
	}

	{
		APIDefn vAPI;
		strlcpy(vAPI.cLibraryName, "libFuzzmmstillomxenc.so", MAX_FILE_LEN);
		strlcpy(vAPI.cAPIName, "JpegOMX_FreeJpegHandle", MAX_API_NAME_LEN);
		vAPI.vFuncPtr     = &JpegOMX_FreeJpegHandle_WRAP;
		vAPI.nNumArgs     = 0;

		InitArg(&(vAPI.vReturnType));
		vAPI.vReturnType.nMemSize = sizeof(int);
		vAPI.vReturnType.vDataType = DTYPE_INT;
		vAPI.vReturnType.vDataSpecifier = DSPECIFIER_FIRST;
		vAPI.vReturnType.bIsPrimitive = true;
		nRetVal = (*regFunc)(&vAPI);
		if(RET_SUCCESS != nRetVal)
		{
			//DBGPRT(LEVEL_ERROR, (char *)"Function : JpegOMX_FreeJpegHandle registration failed!!!");
			return RET_FAILURE;
		}
	}

	{
		APIDefn vAPI;
		strlcpy(vAPI.cLibraryName, "libFuzzmmstillomxenc.so", MAX_FILE_LEN);
		strlcpy(vAPI.cAPIName, "JpegOMX_SetAllParameters", MAX_API_NAME_LEN);
		vAPI.vFuncPtr     = &JpegOMX_SetAllParameters_WRAP;
		vAPI.nNumArgs     = 0;

		InitArg(&(vAPI.vReturnType));
		vAPI.vReturnType.nMemSize = sizeof(int);
		vAPI.vReturnType.vDataType = DTYPE_INT;
		vAPI.vReturnType.vDataSpecifier = DSPECIFIER_FIRST;
		vAPI.vReturnType.bIsPrimitive = true;
		nRetVal = (*regFunc)(&vAPI);
		if(RET_SUCCESS != nRetVal)
		{
			//DBGPRT(LEVEL_ERROR, (char *)"Function : JpegOMX_SetAllParameters registration failed!!!");
			return RET_FAILURE;
		}
	}

	{
		APIDefn vAPI;
		strlcpy(vAPI.cLibraryName, "libFuzzmmstillomxenc.so", MAX_FILE_LEN);
		strlcpy(vAPI.cAPIName, "JpegOMX_SetConfig_InputCrop", MAX_API_NAME_LEN);
		vAPI.vFuncPtr     = &JpegOMX_SetConfig_InputCrop_WRAP;
		vAPI.nNumArgs     = 6;

		InitArg(&(vAPI.vArgDefn[0]));
		vAPI.vArgDefn[0].nPosition = 0;
		vAPI.vArgDefn[0].nMemSize = sizeof(int);
		vAPI.vArgDefn[0].vDataType = DTYPE_INT;
		vAPI.vArgDefn[0].vDataSpecifier = DSPECIFIER_FIRST;
		vAPI.vArgDefn[0].bIsPrimitive = true;

		InitArg(&(vAPI.vArgDefn[1]));
		vAPI.vArgDefn[1].nPosition = 1;
		vAPI.vArgDefn[1].nMemSize = sizeof( OMX_U32 );
		vAPI.vArgDefn[1].vDataType = DTYPE_LONG_INT;
		vAPI.vArgDefn[1].vDataSpecifier = DSPECIFIER_UNSIGNED;
		vAPI.vArgDefn[1].bIsPrimitive = true;

		InitArg(&(vAPI.vArgDefn[2]));
		vAPI.vArgDefn[2].nPosition = 2;
		vAPI.vArgDefn[2].nMemSize = sizeof( OMX_U32 );
		vAPI.vArgDefn[2].vDataType = DTYPE_LONG_INT;
		vAPI.vArgDefn[2].vDataSpecifier = DSPECIFIER_UNSIGNED;
		vAPI.vArgDefn[2].bIsPrimitive = true;

		InitArg(&(vAPI.vArgDefn[3]));
		vAPI.vArgDefn[3].nPosition = 3;
		vAPI.vArgDefn[3].nMemSize = sizeof( OMX_U32 );
		vAPI.vArgDefn[3].vDataType = DTYPE_LONG_INT;
		vAPI.vArgDefn[3].vDataSpecifier = DSPECIFIER_UNSIGNED;
		vAPI.vArgDefn[3].bIsPrimitive = true;

		InitArg(&(vAPI.vArgDefn[4]));
		vAPI.vArgDefn[4].nPosition = 4;
		vAPI.vArgDefn[4].nMemSize = sizeof( OMX_U32 );
		vAPI.vArgDefn[4].vDataType = DTYPE_LONG_INT;
		vAPI.vArgDefn[4].vDataSpecifier = DSPECIFIER_UNSIGNED;
		vAPI.vArgDefn[4].bIsPrimitive = true;

		InitArg(&(vAPI.vArgDefn[5]));
		vAPI.vArgDefn[5].nPosition = 5;
		vAPI.vArgDefn[5].nMemSize = sizeof( OMX_U32 );
		vAPI.vArgDefn[5].vDataType = DTYPE_LONG_INT;
		vAPI.vArgDefn[5].vDataSpecifier = DSPECIFIER_UNSIGNED;
		vAPI.vArgDefn[5].bIsPrimitive = true;

		InitArg(&(vAPI.vReturnType));
		vAPI.vReturnType.nMemSize = sizeof(int);
		vAPI.vReturnType.vDataType = DTYPE_INT;
		vAPI.vReturnType.vDataSpecifier = DSPECIFIER_FIRST;
		vAPI.vReturnType.bIsPrimitive = true;
		nRetVal = (*regFunc)(&vAPI);
		if(RET_SUCCESS != nRetVal)
		{
			//DBGPRT(LEVEL_ERROR, (char *)"Function : JpegOMX_SetConfig_InputCrop registration failed!!!");
			return RET_FAILURE;
		}
	}

	{
		APIDefn vAPI;
		strlcpy(vAPI.cLibraryName, "libFuzzmmstillomxenc.so", MAX_FILE_LEN);
		strlcpy(vAPI.cAPIName, "JpegOMX_FreeBuffer", MAX_API_NAME_LEN);
		vAPI.vFuncPtr     = &JpegOMX_FreeBuffer_WRAP;
		vAPI.nNumArgs     = 3;

		InitArg(&(vAPI.vArgDefn[0]));
		vAPI.vArgDefn[0].nPosition = 0;
		vAPI.vArgDefn[0].nMemSize = sizeof(int);
		vAPI.vArgDefn[0].vDataType = DTYPE_INT;
		vAPI.vArgDefn[0].vDataSpecifier = DSPECIFIER_FIRST;
		vAPI.vArgDefn[0].bIsPrimitive = true;

		InitArg(&(vAPI.vArgDefn[1]));
		vAPI.vArgDefn[1].nPosition = 1;
		vAPI.vArgDefn[1].nMemSize = sizeof( OMX_U32 );
		vAPI.vArgDefn[1].vDataType = DTYPE_LONG_INT;
		vAPI.vArgDefn[1].vDataSpecifier = DSPECIFIER_UNSIGNED;
		vAPI.vArgDefn[1].bIsPrimitive = true;

		InitArg(&(vAPI.vArgDefn[2]));
		vAPI.vArgDefn[2].nPosition = 2;
		vAPI.vArgDefn[2].nMemSize = sizeof(int);
		vAPI.vArgDefn[2].vDataType = DTYPE_INT;
		vAPI.vArgDefn[2].vDataSpecifier = DSPECIFIER_FIRST;
		vAPI.vArgDefn[2].bIsPrimitive = true;

		InitArg(&(vAPI.vReturnType));
		vAPI.vReturnType.nMemSize = sizeof(int);
		vAPI.vReturnType.vDataType = DTYPE_INT;
		vAPI.vReturnType.vDataSpecifier = DSPECIFIER_FIRST;
		vAPI.vReturnType.bIsPrimitive = true;
		nRetVal = (*regFunc)(&vAPI);
		if(RET_SUCCESS != nRetVal)
		{
			//DBGPRT(LEVEL_ERROR, (char *)"Function : JpegOMX_FreeBuffer registration failed!!!");
			return RET_FAILURE;
		}
	}

	{
		APIDefn vAPI;
		strlcpy(vAPI.cLibraryName, "libFuzzmmstillomxenc.so", MAX_FILE_LEN);
		strlcpy(vAPI.cAPIName, "JpegOMX_AllocateBuffer", MAX_API_NAME_LEN);
		vAPI.vFuncPtr     = &JpegOMX_AllocateBuffer_WRAP;
		vAPI.nNumArgs     = 4;

		InitArg(&(vAPI.vArgDefn[0]));
		vAPI.vArgDefn[0].nPosition = 0;
		vAPI.vArgDefn[0].nMemSize = sizeof(int);
		vAPI.vArgDefn[0].vDataType = DTYPE_INT;
		vAPI.vArgDefn[0].vDataSpecifier = DSPECIFIER_FIRST;
		vAPI.vArgDefn[0].bIsPrimitive = true;

		InitArg(&(vAPI.vArgDefn[1]));
		vAPI.vArgDefn[1].nPosition = 1;
		vAPI.vArgDefn[1].nMemSize = sizeof( OMX_U32 );
		vAPI.vArgDefn[1].vDataType = DTYPE_LONG_INT;
		vAPI.vArgDefn[1].vDataSpecifier = DSPECIFIER_UNSIGNED;
		vAPI.vArgDefn[1].bIsPrimitive = true;

		InitArg(&(vAPI.vArgDefn[2]));
		vAPI.vArgDefn[2].nPosition = 2;
		vAPI.vArgDefn[2].nMemSize = sizeof(int);
		vAPI.vArgDefn[2].vDataType = DTYPE_INT;
		vAPI.vArgDefn[2].vDataSpecifier = DSPECIFIER_FIRST;
		vAPI.vArgDefn[2].bIsPrimitive = true;

		InitArg(&(vAPI.vArgDefn[3]));
		vAPI.vArgDefn[3].nPosition = 3;
		vAPI.vArgDefn[3].nMemSize = sizeof(unsigned int);
		vAPI.vArgDefn[3].vDataType = DTYPE_INT;
		vAPI.vArgDefn[3].vDataSpecifier = DSPECIFIER_UNSIGNED;
		vAPI.vArgDefn[3].bIsPrimitive = true;

		InitArg(&(vAPI.vReturnType));
		vAPI.vReturnType.nMemSize = sizeof(int);
		vAPI.vReturnType.vDataType = DTYPE_INT;
		vAPI.vReturnType.vDataSpecifier = DSPECIFIER_FIRST;
		vAPI.vReturnType.bIsPrimitive = true;
		nRetVal = (*regFunc)(&vAPI);
		if(RET_SUCCESS != nRetVal)
		{
			//DBGPRT(LEVEL_ERROR, (char *)"Function : JpegOMX_AllocateBuffer registration failed!!!");
			return RET_FAILURE;
		}
	}

	{
		APIDefn vAPI;
		strlcpy(vAPI.cLibraryName, "libFuzzmmstillomxenc.so", MAX_FILE_LEN);
		strlcpy(vAPI.cAPIName, "JpegOMX_GetHandle", MAX_API_NAME_LEN);
		vAPI.vFuncPtr     = &JpegOMX_GetHandle_WRAP;
		vAPI.nNumArgs     = 3;

		InitArg(&(vAPI.vArgDefn[0]));
		vAPI.vArgDefn[0].nPosition = 0;
		vAPI.vArgDefn[0].nMemSize = sizeof(char);
		vAPI.vArgDefn[0].bIsPointer = true;
		vAPI.vArgDefn[0].vDataType = DTYPE_CHAR;
		vAPI.vArgDefn[0].vDataSpecifier = DSPECIFIER_FIRST;
		vAPI.vArgDefn[0].bIsPrimitive = true;

		InitArg(&(vAPI.vArgDefn[1]));
		vAPI.vArgDefn[1].nPosition = 1;
		vAPI.vArgDefn[1].nMemSize = sizeof(char);
		vAPI.vArgDefn[1].bIsPointer = true;
		vAPI.vArgDefn[1].vDataType = DTYPE_CHAR;
		vAPI.vArgDefn[1].vDataSpecifier = DSPECIFIER_FIRST;
		vAPI.vArgDefn[1].bIsPrimitive = true;

		InitArg(&(vAPI.vArgDefn[2]));
		vAPI.vArgDefn[2].nPosition = 2;
		vAPI.vArgDefn[2].nMemSize = sizeof( OMX_CALLBACKTYPE );
		vAPI.vArgDefn[2].bIsPointer = true;
		vAPI.vArgDefn[2].vDataType = DTYPE_FIRST;
		vAPI.vArgDefn[2].vDataSpecifier = DSPECIFIER_FIRST;
		vAPI.vArgDefn[2].bIsStructure = true;

		InitArg(&(vAPI.vReturnType));
		vAPI.vReturnType.nMemSize = sizeof(int);
		vAPI.vReturnType.vDataType = DTYPE_INT;
		vAPI.vReturnType.vDataSpecifier = DSPECIFIER_FIRST;
		vAPI.vReturnType.bIsPrimitive = true;
		nRetVal = (*regFunc)(&vAPI);
		if(RET_SUCCESS != nRetVal)
		{
			//DBGPRT(LEVEL_ERROR, (char *)"Function : JpegOMX_GetHandle registration failed!!!");
			return RET_FAILURE;
		}
	}

	{
		APIDefn vAPI;
		strlcpy(vAPI.cLibraryName, "libFuzzmmstillomxenc.so", MAX_FILE_LEN);
		strlcpy(vAPI.cAPIName, "JpegOMX_SetConfig", MAX_API_NAME_LEN);
		vAPI.vFuncPtr     = &JpegOMX_SetConfig_WRAP;
		vAPI.nNumArgs     = 3;

		InitArg(&(vAPI.vArgDefn[0]));
		vAPI.vArgDefn[0].nPosition = 0;
		vAPI.vArgDefn[0].nMemSize = sizeof(int);
		vAPI.vArgDefn[0].vDataType = DTYPE_INT;
		vAPI.vArgDefn[0].vDataSpecifier = DSPECIFIER_FIRST;
		vAPI.vArgDefn[0].bIsPrimitive = true;

		InitArg(&(vAPI.vArgDefn[1]));
		vAPI.vArgDefn[1].nPosition = 1;
		vAPI.vArgDefn[1].nMemSize = sizeof(int);
		vAPI.vArgDefn[1].vDataType = DTYPE_INT;
		vAPI.vArgDefn[1].vDataSpecifier = DSPECIFIER_FIRST;
		vAPI.vArgDefn[1].bIsPrimitive = true;

		InitArg(&(vAPI.vArgDefn[2]));
		vAPI.vArgDefn[2].nPosition = 2;
		vAPI.vArgDefn[2].nMemSize = sizeof(int);
		vAPI.vArgDefn[2].vDataType = DTYPE_INT;
		vAPI.vArgDefn[2].vDataSpecifier = DSPECIFIER_FIRST;
		vAPI.vArgDefn[2].bIsPrimitive = true;

		InitArg(&(vAPI.vReturnType));
		vAPI.vReturnType.nMemSize = sizeof(int);
		vAPI.vReturnType.vDataType = DTYPE_INT;
		vAPI.vReturnType.vDataSpecifier = DSPECIFIER_FIRST;
		vAPI.vReturnType.bIsPrimitive = true;
		nRetVal = (*regFunc)(&vAPI);
		if(RET_SUCCESS != nRetVal)
		{
			//DBGPRT(LEVEL_ERROR, (char *)"Function : JpegOMX_SetConfig registration failed!!!");
			return RET_FAILURE;
		}
	}

	{
		APIDefn vAPI;
		strlcpy(vAPI.cLibraryName, "libFuzzmmstillomxenc.so", MAX_FILE_LEN);
		strlcpy(vAPI.cAPIName, "JpegOMX_SetParameter_ThumbQuality", MAX_API_NAME_LEN);
		vAPI.vFuncPtr     = &JpegOMX_SetParameter_ThumbQuality_WRAP;
		vAPI.nNumArgs     = 2;

		InitArg(&(vAPI.vArgDefn[0]));
		vAPI.vArgDefn[0].nPosition = 0;
		vAPI.vArgDefn[0].nMemSize = sizeof(int);
		vAPI.vArgDefn[0].vDataType = DTYPE_INT;
		vAPI.vArgDefn[0].vDataSpecifier = DSPECIFIER_FIRST;
		vAPI.vArgDefn[0].bIsPrimitive = true;

		InitArg(&(vAPI.vArgDefn[1]));
		vAPI.vArgDefn[1].nPosition = 1;
		vAPI.vArgDefn[1].nMemSize = sizeof(int);
		vAPI.vArgDefn[1].vDataType = DTYPE_INT;
		vAPI.vArgDefn[1].vDataSpecifier = DSPECIFIER_FIRST;
		vAPI.vArgDefn[1].bIsPrimitive = true;

		InitArg(&(vAPI.vReturnType));
		vAPI.vReturnType.nMemSize = sizeof(int);
		vAPI.vReturnType.vDataType = DTYPE_INT;
		vAPI.vReturnType.vDataSpecifier = DSPECIFIER_FIRST;
		vAPI.vReturnType.bIsPrimitive = true;
		nRetVal = (*regFunc)(&vAPI);
		if(RET_SUCCESS != nRetVal)
		{
			//DBGPRT(LEVEL_ERROR, (char *)"Function : JpegOMX_SetParameter_ThumbQuality registration failed!!!");
			return RET_FAILURE;
		}
	}

	{
		APIDefn vAPI;
		strlcpy(vAPI.cLibraryName, "libFuzzmmstillomxenc.so", MAX_FILE_LEN);
		strlcpy(vAPI.cAPIName, "JpegOMX_SetConfig_OutputCrop", MAX_API_NAME_LEN);
		vAPI.vFuncPtr     = &JpegOMX_SetConfig_OutputCrop_WRAP;
		vAPI.nNumArgs     = 6;

		InitArg(&(vAPI.vArgDefn[0]));
		vAPI.vArgDefn[0].nPosition = 0;
		vAPI.vArgDefn[0].nMemSize = sizeof(int);
		vAPI.vArgDefn[0].vDataType = DTYPE_INT;
		vAPI.vArgDefn[0].vDataSpecifier = DSPECIFIER_FIRST;
		vAPI.vArgDefn[0].bIsPrimitive = true;

		InitArg(&(vAPI.vArgDefn[1]));
		vAPI.vArgDefn[1].nPosition = 1;
		vAPI.vArgDefn[1].nMemSize = sizeof( OMX_U32 );
		vAPI.vArgDefn[1].vDataType = DTYPE_LONG_INT;
		vAPI.vArgDefn[1].vDataSpecifier = DSPECIFIER_UNSIGNED;
		vAPI.vArgDefn[1].bIsPrimitive = true;

		InitArg(&(vAPI.vArgDefn[2]));
		vAPI.vArgDefn[2].nPosition = 2;
		vAPI.vArgDefn[2].nMemSize = sizeof( OMX_U32 );
		vAPI.vArgDefn[2].vDataType = DTYPE_LONG_INT;
		vAPI.vArgDefn[2].vDataSpecifier = DSPECIFIER_UNSIGNED;
		vAPI.vArgDefn[2].bIsPrimitive = true;

		InitArg(&(vAPI.vArgDefn[3]));
		vAPI.vArgDefn[3].nPosition = 3;
		vAPI.vArgDefn[3].nMemSize = sizeof( OMX_U32 );
		vAPI.vArgDefn[3].vDataType = DTYPE_LONG_INT;
		vAPI.vArgDefn[3].vDataSpecifier = DSPECIFIER_UNSIGNED;
		vAPI.vArgDefn[3].bIsPrimitive = true;

		InitArg(&(vAPI.vArgDefn[4]));
		vAPI.vArgDefn[4].nPosition = 4;
		vAPI.vArgDefn[4].nMemSize = sizeof( OMX_U32 );
		vAPI.vArgDefn[4].vDataType = DTYPE_LONG_INT;
		vAPI.vArgDefn[4].vDataSpecifier = DSPECIFIER_UNSIGNED;
		vAPI.vArgDefn[4].bIsPrimitive = true;

		InitArg(&(vAPI.vArgDefn[5]));
		vAPI.vArgDefn[5].nPosition = 5;
		vAPI.vArgDefn[5].nMemSize = sizeof( OMX_U32 );
		vAPI.vArgDefn[5].vDataType = DTYPE_LONG_INT;
		vAPI.vArgDefn[5].vDataSpecifier = DSPECIFIER_UNSIGNED;
		vAPI.vArgDefn[5].bIsPrimitive = true;

		InitArg(&(vAPI.vReturnType));
		vAPI.vReturnType.nMemSize = sizeof(int);
		vAPI.vReturnType.vDataType = DTYPE_INT;
		vAPI.vReturnType.vDataSpecifier = DSPECIFIER_FIRST;
		vAPI.vReturnType.bIsPrimitive = true;
		nRetVal = (*regFunc)(&vAPI);
		if(RET_SUCCESS != nRetVal)
		{
			//DBGPRT(LEVEL_ERROR, (char *)"Function : JpegOMX_SetConfig_OutputCrop registration failed!!!");
			return RET_FAILURE;
		}
	}

	{
		APIDefn vAPI;
		strlcpy(vAPI.cLibraryName, "libFuzzmmstillomxenc.so", MAX_FILE_LEN);
		strlcpy(vAPI.cAPIName, "JpegOMX_FillThisBuffer", MAX_API_NAME_LEN);
		vAPI.vFuncPtr     = &JpegOMX_FillThisBuffer_WRAP;
		vAPI.nNumArgs     = 2;

		InitArg(&(vAPI.vArgDefn[0]));
		vAPI.vArgDefn[0].nPosition = 0;
		vAPI.vArgDefn[0].nMemSize = sizeof(int);
		vAPI.vArgDefn[0].vDataType = DTYPE_INT;
		vAPI.vArgDefn[0].vDataSpecifier = DSPECIFIER_FIRST;
		vAPI.vArgDefn[0].bIsPrimitive = true;

		InitArg(&(vAPI.vArgDefn[1]));
		vAPI.vArgDefn[1].nPosition = 1;
		vAPI.vArgDefn[1].nMemSize = sizeof(int);
		vAPI.vArgDefn[1].vDataType = DTYPE_INT;
		vAPI.vArgDefn[1].vDataSpecifier = DSPECIFIER_FIRST;
		vAPI.vArgDefn[1].bIsPrimitive = true;

		InitArg(&(vAPI.vReturnType));
		vAPI.vReturnType.nMemSize = sizeof(int);
		vAPI.vReturnType.vDataType = DTYPE_INT;
		vAPI.vReturnType.vDataSpecifier = DSPECIFIER_FIRST;
		vAPI.vReturnType.bIsPrimitive = true;
		nRetVal = (*regFunc)(&vAPI);
		if(RET_SUCCESS != nRetVal)
		{
			//DBGPRT(LEVEL_ERROR, (char *)"Function : JpegOMX_FillThisBuffer registration failed!!!");
			return RET_FAILURE;
		}
	}

	{
		APIDefn vAPI;
		strlcpy(vAPI.cLibraryName, "libFuzzmmstillomxenc.so", MAX_FILE_LEN);
		strlcpy(vAPI.cAPIName, "JpegOMX_GetState", MAX_API_NAME_LEN);
		vAPI.vFuncPtr     = &JpegOMX_GetState_WRAP;
		vAPI.nNumArgs     = 1;

		InitArg(&(vAPI.vArgDefn[0]));
		vAPI.vArgDefn[0].nPosition = 0;
		vAPI.vArgDefn[0].nMemSize = sizeof(int);
		vAPI.vArgDefn[0].vDataType = DTYPE_INT;
		vAPI.vArgDefn[0].vDataSpecifier = DSPECIFIER_FIRST;
		vAPI.vArgDefn[0].bIsPrimitive = true;

		InitArg(&(vAPI.vReturnType));
		vAPI.vReturnType.nMemSize = sizeof(int);
		vAPI.vReturnType.vDataType = DTYPE_INT;
		vAPI.vReturnType.vDataSpecifier = DSPECIFIER_FIRST;
		vAPI.vReturnType.bIsPrimitive = true;
		nRetVal = (*regFunc)(&vAPI);
		if(RET_SUCCESS != nRetVal)
		{
			//DBGPRT(LEVEL_ERROR, (char *)"Function : JpegOMX_GetState registration failed!!!");
			return RET_FAILURE;
		}
	}

	{
		APIDefn vAPI;
		strlcpy(vAPI.cLibraryName, "libFuzzmmstillomxenc.so", MAX_FILE_LEN);
		strlcpy(vAPI.cAPIName, "JpegOMX_SetParameter_ImagePortFormat", MAX_API_NAME_LEN);
		vAPI.vFuncPtr     = &JpegOMX_SetParameter_ImagePortFormat_WRAP;
		vAPI.nNumArgs     = 10;

		InitArg(&(vAPI.vArgDefn[0]));
		vAPI.vArgDefn[0].nPosition = 0;
		vAPI.vArgDefn[0].nMemSize = sizeof(int);
		vAPI.vArgDefn[0].vDataType = DTYPE_INT;
		vAPI.vArgDefn[0].vDataSpecifier = DSPECIFIER_FIRST;
		vAPI.vArgDefn[0].bIsPrimitive = true;

		InitArg(&(vAPI.vArgDefn[1]));
		vAPI.vArgDefn[1].nPosition = 1;
		vAPI.vArgDefn[1].nMemSize = sizeof(int);
		vAPI.vArgDefn[1].vDataType = DTYPE_INT;
		vAPI.vArgDefn[1].vDataSpecifier = DSPECIFIER_FIRST;
		vAPI.vArgDefn[1].bIsPrimitive = true;

		InitArg(&(vAPI.vArgDefn[2]));
		vAPI.vArgDefn[2].nPosition = 2;
		vAPI.vArgDefn[2].nMemSize = sizeof(int);
		vAPI.vArgDefn[2].vDataType = DTYPE_INT;
		vAPI.vArgDefn[2].vDataSpecifier = DSPECIFIER_FIRST;
		vAPI.vArgDefn[2].bIsPrimitive = true;

		InitArg(&(vAPI.vArgDefn[3]));
		vAPI.vArgDefn[3].nPosition = 3;
		vAPI.vArgDefn[3].nMemSize = sizeof(int);
		vAPI.vArgDefn[3].vDataType = DTYPE_INT;
		vAPI.vArgDefn[3].vDataSpecifier = DSPECIFIER_FIRST;
		vAPI.vArgDefn[3].bIsPrimitive = true;

		InitArg(&(vAPI.vArgDefn[4]));
		vAPI.vArgDefn[4].nPosition = 4;
		vAPI.vArgDefn[4].nMemSize = sizeof(int);
		vAPI.vArgDefn[4].vDataType = DTYPE_INT;
		vAPI.vArgDefn[4].vDataSpecifier = DSPECIFIER_FIRST;
		vAPI.vArgDefn[4].bIsPrimitive = true;

		InitArg(&(vAPI.vArgDefn[5]));
		vAPI.vArgDefn[5].nPosition = 5;
		vAPI.vArgDefn[5].nMemSize = sizeof(int);
		vAPI.vArgDefn[5].vDataType = DTYPE_INT;
		vAPI.vArgDefn[5].vDataSpecifier = DSPECIFIER_FIRST;
		vAPI.vArgDefn[5].bIsPrimitive = true;

		InitArg(&(vAPI.vArgDefn[6]));
		vAPI.vArgDefn[6].nPosition = 6;
		vAPI.vArgDefn[6].nMemSize = sizeof( OMX_U8 );
		vAPI.vArgDefn[6].vDataType = DTYPE_CHAR;
		vAPI.vArgDefn[6].vDataSpecifier = DSPECIFIER_UNSIGNED;
		vAPI.vArgDefn[6].bIsPrimitive = true;

		InitArg(&(vAPI.vArgDefn[7]));
		vAPI.vArgDefn[7].nPosition = 7;
		vAPI.vArgDefn[7].nMemSize = sizeof( OMX_U8 );
		vAPI.vArgDefn[7].vDataType = DTYPE_CHAR;
		vAPI.vArgDefn[7].vDataSpecifier = DSPECIFIER_UNSIGNED;
		vAPI.vArgDefn[7].bIsPrimitive = true;

		InitArg(&(vAPI.vArgDefn[8]));
		vAPI.vArgDefn[8].nPosition = 8;
		vAPI.vArgDefn[8].nMemSize = sizeof( OMX_U8 );
		vAPI.vArgDefn[8].vDataType = DTYPE_CHAR;
		vAPI.vArgDefn[8].vDataSpecifier = DSPECIFIER_UNSIGNED;
		vAPI.vArgDefn[8].bIsPrimitive = true;

		InitArg(&(vAPI.vArgDefn[9]));
		vAPI.vArgDefn[9].nPosition = 9;
		vAPI.vArgDefn[9].nMemSize = sizeof( OMX_U8 );
		vAPI.vArgDefn[9].vDataType = DTYPE_CHAR;
		vAPI.vArgDefn[9].vDataSpecifier = DSPECIFIER_UNSIGNED;
		vAPI.vArgDefn[9].bIsPrimitive = true;

		InitArg(&(vAPI.vReturnType));
		vAPI.vReturnType.nMemSize = sizeof(int);
		vAPI.vReturnType.vDataType = DTYPE_INT;
		vAPI.vReturnType.vDataSpecifier = DSPECIFIER_FIRST;
		vAPI.vReturnType.bIsPrimitive = true;
		nRetVal = (*regFunc)(&vAPI);
		if(RET_SUCCESS != nRetVal)
		{
			//DBGPRT(LEVEL_ERROR, (char *)"Function : JpegOMX_SetParameter_ImagePortFormat registration failed!!!");
			return RET_FAILURE;
		}
	}

	{
		APIDefn vAPI;
		strlcpy(vAPI.cLibraryName, "libFuzzmmstillomxenc.so", MAX_FILE_LEN);
		strlcpy(vAPI.cAPIName, "JpegOMX_SetParameter_BufferOffset", MAX_API_NAME_LEN);
		vAPI.vFuncPtr     = &JpegOMX_SetParameter_BufferOffset_WRAP;
		vAPI.nNumArgs     = 5;

		InitArg(&(vAPI.vArgDefn[0]));
		vAPI.vArgDefn[0].nPosition = 0;
		vAPI.vArgDefn[0].nMemSize = sizeof(int);
		vAPI.vArgDefn[0].vDataType = DTYPE_INT;
		vAPI.vArgDefn[0].vDataSpecifier = DSPECIFIER_FIRST;
		vAPI.vArgDefn[0].bIsPrimitive = true;

		InitArg(&(vAPI.vArgDefn[1]));
		vAPI.vArgDefn[1].nPosition = 1;
		vAPI.vArgDefn[1].nMemSize = sizeof(int);
		vAPI.vArgDefn[1].vDataType = DTYPE_INT;
		vAPI.vArgDefn[1].vDataSpecifier = DSPECIFIER_FIRST;
		vAPI.vArgDefn[1].bIsPrimitive = true;

		InitArg(&(vAPI.vArgDefn[2]));
		vAPI.vArgDefn[2].nPosition = 2;
		vAPI.vArgDefn[2].nMemSize = sizeof(int);
		vAPI.vArgDefn[2].vDataType = DTYPE_INT;
		vAPI.vArgDefn[2].vDataSpecifier = DSPECIFIER_FIRST;
		vAPI.vArgDefn[2].bIsPrimitive = true;

		InitArg(&(vAPI.vArgDefn[3]));
		vAPI.vArgDefn[3].nPosition = 3;
		vAPI.vArgDefn[3].nMemSize = sizeof(int);
		vAPI.vArgDefn[3].vDataType = DTYPE_INT;
		vAPI.vArgDefn[3].vDataSpecifier = DSPECIFIER_FIRST;
		vAPI.vArgDefn[3].bIsPrimitive = true;

		InitArg(&(vAPI.vArgDefn[4]));
		vAPI.vArgDefn[4].nPosition = 4;
		vAPI.vArgDefn[4].nMemSize = sizeof(int);
		vAPI.vArgDefn[4].vDataType = DTYPE_INT;
		vAPI.vArgDefn[4].vDataSpecifier = DSPECIFIER_FIRST;
		vAPI.vArgDefn[4].bIsPrimitive = true;

		InitArg(&(vAPI.vReturnType));
		vAPI.vReturnType.nMemSize = sizeof(int);
		vAPI.vReturnType.vDataType = DTYPE_INT;
		vAPI.vReturnType.vDataSpecifier = DSPECIFIER_FIRST;
		vAPI.vReturnType.bIsPrimitive = true;
		nRetVal = (*regFunc)(&vAPI);
		if(RET_SUCCESS != nRetVal)
		{
			//DBGPRT(LEVEL_ERROR, (char *)"Function : JpegOMX_SetParameter_BufferOffset registration failed!!!");
			return RET_FAILURE;
		}
	}

	{
		APIDefn vAPI;
		strlcpy(vAPI.cLibraryName, "libFuzzmmstillomxenc.so", MAX_FILE_LEN);
		strlcpy(vAPI.cAPIName, "JpegOMX_GetComponentVersion", MAX_API_NAME_LEN);
		vAPI.vFuncPtr     = &JpegOMX_GetComponentVersion_WRAP;
		vAPI.nNumArgs     = 5;

		InitArg(&(vAPI.vArgDefn[0]));
		vAPI.vArgDefn[0].nPosition = 0;
		vAPI.vArgDefn[0].nMemSize = sizeof(int);
		vAPI.vArgDefn[0].vDataType = DTYPE_INT;
		vAPI.vArgDefn[0].vDataSpecifier = DSPECIFIER_FIRST;
		vAPI.vArgDefn[0].bIsPrimitive = true;

		InitArg(&(vAPI.vArgDefn[1]));
		vAPI.vArgDefn[1].nPosition = 1;
		vAPI.vArgDefn[1].nMemSize = sizeof(char);
		vAPI.vArgDefn[1].bIsPointer = true;
		vAPI.vArgDefn[1].vDataType = DTYPE_CHAR;
		vAPI.vArgDefn[1].vDataSpecifier = DSPECIFIER_FIRST;
		vAPI.vArgDefn[1].bIsPrimitive = true;

		InitArg(&(vAPI.vArgDefn[2]));
		vAPI.vArgDefn[2].nPosition = 2;
		vAPI.vArgDefn[2].nMemSize = sizeof(int);
		vAPI.vArgDefn[2].vDataType = DTYPE_INT;
		vAPI.vArgDefn[2].vDataSpecifier = DSPECIFIER_FIRST;
		vAPI.vArgDefn[2].bIsPrimitive = true;

		InitArg(&(vAPI.vArgDefn[3]));
		vAPI.vArgDefn[3].nPosition = 3;
		vAPI.vArgDefn[3].nMemSize = sizeof(int);
		vAPI.vArgDefn[3].vDataType = DTYPE_INT;
		vAPI.vArgDefn[3].vDataSpecifier = DSPECIFIER_FIRST;
		vAPI.vArgDefn[3].bIsPrimitive = true;

		InitArg(&(vAPI.vArgDefn[4]));
		vAPI.vArgDefn[4].nPosition = 4;
		vAPI.vArgDefn[4].nMemSize = sizeof(int);
		vAPI.vArgDefn[4].vDataType = DTYPE_INT;
		vAPI.vArgDefn[4].vDataSpecifier = DSPECIFIER_FIRST;
		vAPI.vArgDefn[4].bIsPrimitive = true;

		InitArg(&(vAPI.vReturnType));
		vAPI.vReturnType.nMemSize = sizeof(int);
		vAPI.vReturnType.vDataType = DTYPE_INT;
		vAPI.vReturnType.vDataSpecifier = DSPECIFIER_FIRST;
		vAPI.vReturnType.bIsPrimitive = true;
		nRetVal = (*regFunc)(&vAPI);
		if(RET_SUCCESS != nRetVal)
		{
			//DBGPRT(LEVEL_ERROR, (char *)"Function : JpegOMX_GetComponentVersion registration failed!!!");
			return RET_FAILURE;
		}
	}

	{
		APIDefn vAPI;
		strlcpy(vAPI.cLibraryName, "libFuzzmmstillomxenc.so", MAX_FILE_LEN);
		strlcpy(vAPI.cAPIName, "JpegOMX_GetExtIdx", MAX_API_NAME_LEN);
		vAPI.vFuncPtr     = &JpegOMX_GetExtIdx_WRAP;
		vAPI.nNumArgs     = 2;

		InitArg(&(vAPI.vArgDefn[0]));
		vAPI.vArgDefn[0].nPosition = 0;
		vAPI.vArgDefn[0].nMemSize = sizeof(int);
		vAPI.vArgDefn[0].vDataType = DTYPE_INT;
		vAPI.vArgDefn[0].vDataSpecifier = DSPECIFIER_FIRST;
		vAPI.vArgDefn[0].bIsPrimitive = true;

		InitArg(&(vAPI.vArgDefn[1]));
		vAPI.vArgDefn[1].nPosition = 1;
		vAPI.vArgDefn[1].nMemSize = sizeof(char);
		vAPI.vArgDefn[1].bIsPointer = true;
		vAPI.vArgDefn[1].vDataType = DTYPE_CHAR;
		vAPI.vArgDefn[1].vDataSpecifier = DSPECIFIER_FIRST;
		vAPI.vArgDefn[1].bIsPrimitive = true;

		InitArg(&(vAPI.vReturnType));
		vAPI.vReturnType.nMemSize = sizeof(int);
		vAPI.vReturnType.vDataType = DTYPE_INT;
		vAPI.vReturnType.vDataSpecifier = DSPECIFIER_FIRST;
		vAPI.vReturnType.bIsPrimitive = true;
		nRetVal = (*regFunc)(&vAPI);
		if(RET_SUCCESS != nRetVal)
		{
			//DBGPRT(LEVEL_ERROR, (char *)"Function : JpegOMX_GetExtIdx registration failed!!!");
			return RET_FAILURE;
		}
	}

	{
		APIDefn vAPI;
		strlcpy(vAPI.cLibraryName, "libFuzzmmstillomxenc.so", MAX_FILE_LEN);
		strlcpy(vAPI.cAPIName, "JpegOMX_Init", MAX_API_NAME_LEN);
		vAPI.vFuncPtr     = &JpegOMX_Init_WRAP;
		vAPI.nNumArgs     = 0;

		InitArg(&(vAPI.vReturnType));
		vAPI.vReturnType.nMemSize = sizeof(int);
		vAPI.vReturnType.vDataType = DTYPE_INT;
		vAPI.vReturnType.vDataSpecifier = DSPECIFIER_FIRST;
		vAPI.vReturnType.bIsPrimitive = true;
		nRetVal = (*regFunc)(&vAPI);
		if(RET_SUCCESS != nRetVal)
		{
			//DBGPRT(LEVEL_ERROR, (char *)"Function : JpegOMX_Init registration failed!!!");
			return RET_FAILURE;
		}
	}

	{
		APIDefn vAPI;
		strlcpy(vAPI.cLibraryName, "libFuzzmmstillomxenc.so", MAX_FILE_LEN);
		strlcpy(vAPI.cAPIName, "JpegOMX_GetJpegHandle", MAX_API_NAME_LEN);
		vAPI.vFuncPtr     = &JpegOMX_GetJpegHandle_WRAP;
		vAPI.nNumArgs     = 0;

		InitArg(&(vAPI.vReturnType));
		vAPI.vReturnType.nMemSize = sizeof(int);
		vAPI.vReturnType.vDataType = DTYPE_INT;
		vAPI.vReturnType.vDataSpecifier = DSPECIFIER_FIRST;
		vAPI.vReturnType.bIsPrimitive = true;
		nRetVal = (*regFunc)(&vAPI);
		if(RET_SUCCESS != nRetVal)
		{
			//DBGPRT(LEVEL_ERROR, (char *)"Function : JpegOMX_GetJpegHandle registration failed!!!");
			return RET_FAILURE;
		}
	}

	{
		APIDefn vAPI;
		strlcpy(vAPI.cLibraryName, "libFuzzmmstillomxenc.so", MAX_FILE_LEN);
		strlcpy(vAPI.cAPIName, "JpegOMX_AllUseBuffers", MAX_API_NAME_LEN);
		vAPI.vFuncPtr     = &JpegOMX_AllUseBuffers_WRAP;
		vAPI.nNumArgs     = 0;

		InitArg(&(vAPI.vReturnType));
		vAPI.vReturnType.nMemSize = sizeof(int);
		vAPI.vReturnType.vDataType = DTYPE_INT;
		vAPI.vReturnType.vDataSpecifier = DSPECIFIER_FIRST;
		vAPI.vReturnType.bIsPrimitive = true;
		nRetVal = (*regFunc)(&vAPI);
		if(RET_SUCCESS != nRetVal)
		{
			//DBGPRT(LEVEL_ERROR, (char *)"Function : JpegOMX_AllUseBuffers registration failed!!!");
			return RET_FAILURE;
		}
	}

	{
		APIDefn vAPI;
		strlcpy(vAPI.cLibraryName, "libFuzzmmstillomxenc.so", MAX_FILE_LEN);
		strlcpy(vAPI.cAPIName, "JpegOMX_SendCommand", MAX_API_NAME_LEN);
		vAPI.vFuncPtr     = &JpegOMX_SendCommand_WRAP;
		vAPI.nNumArgs     = 4;

		InitArg(&(vAPI.vArgDefn[0]));
		vAPI.vArgDefn[0].nPosition = 0;
		vAPI.vArgDefn[0].nMemSize = sizeof(int);
		vAPI.vArgDefn[0].vDataType = DTYPE_INT;
		vAPI.vArgDefn[0].vDataSpecifier = DSPECIFIER_FIRST;
		vAPI.vArgDefn[0].bIsPrimitive = true;

		InitArg(&(vAPI.vArgDefn[1]));
		vAPI.vArgDefn[1].nPosition = 1;
		vAPI.vArgDefn[1].nMemSize = sizeof(int);
		vAPI.vArgDefn[1].vDataType = DTYPE_INT;
		vAPI.vArgDefn[1].vDataSpecifier = DSPECIFIER_FIRST;
		vAPI.vArgDefn[1].bIsPrimitive = true;

		InitArg(&(vAPI.vArgDefn[2]));
		vAPI.vArgDefn[2].nPosition = 2;
		vAPI.vArgDefn[2].nMemSize = sizeof( OMX_U32 );
		vAPI.vArgDefn[2].vDataType = DTYPE_LONG_INT;
		vAPI.vArgDefn[2].vDataSpecifier = DSPECIFIER_UNSIGNED;
		vAPI.vArgDefn[2].bIsPrimitive = true;

		InitArg(&(vAPI.vArgDefn[3]));
		vAPI.vArgDefn[3].nPosition = 3;
		vAPI.vArgDefn[3].nMemSize = sizeof( OMX_MARKTYPE );
		vAPI.vArgDefn[3].vDataType = DTYPE_FIRST;
		vAPI.vArgDefn[3].vDataSpecifier = DSPECIFIER_FIRST;
		vAPI.vArgDefn[3].bIsStructure = true;

		InitArg(&(vAPI.vReturnType));
		vAPI.vReturnType.nMemSize = sizeof(int);
		vAPI.vReturnType.vDataType = DTYPE_INT;
		vAPI.vReturnType.vDataSpecifier = DSPECIFIER_FIRST;
		vAPI.vReturnType.bIsPrimitive = true;
		nRetVal = (*regFunc)(&vAPI);
		if(RET_SUCCESS != nRetVal)
		{
			//DBGPRT(LEVEL_ERROR, (char *)"Function : JpegOMX_SendCommand registration failed!!!");
			return RET_FAILURE;
		}
	}

	{
		APIDefn vAPI;
		strlcpy(vAPI.cLibraryName, "libFuzzmmstillomxenc.so", MAX_FILE_LEN);
		strlcpy(vAPI.cAPIName, "JpegOMX_SetParameter", MAX_API_NAME_LEN);
		vAPI.vFuncPtr     = &JpegOMX_SetParameter_WRAP;
		vAPI.nNumArgs     = 3;

		InitArg(&(vAPI.vArgDefn[0]));
		vAPI.vArgDefn[0].nPosition = 0;
		vAPI.vArgDefn[0].nMemSize = sizeof(int);
		vAPI.vArgDefn[0].vDataType = DTYPE_INT;
		vAPI.vArgDefn[0].vDataSpecifier = DSPECIFIER_FIRST;
		vAPI.vArgDefn[0].bIsPrimitive = true;

		InitArg(&(vAPI.vArgDefn[1]));
		vAPI.vArgDefn[1].nPosition = 1;
		vAPI.vArgDefn[1].nMemSize = sizeof(int);
		vAPI.vArgDefn[1].vDataType = DTYPE_INT;
		vAPI.vArgDefn[1].vDataSpecifier = DSPECIFIER_FIRST;
		vAPI.vArgDefn[1].bIsPrimitive = true;

		InitArg(&(vAPI.vArgDefn[2]));
		vAPI.vArgDefn[2].nPosition = 2;
		vAPI.vArgDefn[2].nMemSize = sizeof(int);
		vAPI.vArgDefn[2].vDataType = DTYPE_INT;
		vAPI.vArgDefn[2].vDataSpecifier = DSPECIFIER_FIRST;
		vAPI.vArgDefn[2].bIsPrimitive = true;

		InitArg(&(vAPI.vReturnType));
		vAPI.vReturnType.nMemSize = sizeof(int);
		vAPI.vReturnType.vDataType = DTYPE_INT;
		vAPI.vReturnType.vDataSpecifier = DSPECIFIER_FIRST;
		vAPI.vReturnType.bIsPrimitive = true;
		nRetVal = (*regFunc)(&vAPI);
		if(RET_SUCCESS != nRetVal)
		{
			////DBGPRT(LEVEL_ERROR, (char *)"Function : JpegOMX_SetParameter registration failed!!!");
			return RET_FAILURE;
		}
	}

	{
		APIDefn vAPI;
		strlcpy(vAPI.cLibraryName, "libFuzzmmstillomxenc.so", MAX_FILE_LEN);
		strlcpy(vAPI.cAPIName, "JpegOMX_SetParameter_PortDef", MAX_API_NAME_LEN);
		vAPI.vFuncPtr     = &JpegOMX_SetParameter_PortDef_WRAP;
		vAPI.nNumArgs     = 19;

		InitArg(&(vAPI.vArgDefn[0]));
		vAPI.vArgDefn[0].nPosition = 0;
		vAPI.vArgDefn[0].nMemSize = sizeof(int);
		vAPI.vArgDefn[0].vDataType = DTYPE_INT;
		vAPI.vArgDefn[0].vDataSpecifier = DSPECIFIER_FIRST;
		vAPI.vArgDefn[0].bIsPrimitive = true;

		InitArg(&(vAPI.vArgDefn[1]));
		vAPI.vArgDefn[1].nPosition = 1;
		vAPI.vArgDefn[1].nMemSize = sizeof(int);
		vAPI.vArgDefn[1].vDataType = DTYPE_INT;
		vAPI.vArgDefn[1].vDataSpecifier = DSPECIFIER_FIRST;
		vAPI.vArgDefn[1].bIsPrimitive = true;

		InitArg(&(vAPI.vArgDefn[2]));
		vAPI.vArgDefn[2].nPosition = 2;
		vAPI.vArgDefn[2].nMemSize = sizeof(int);
		vAPI.vArgDefn[2].vDataType = DTYPE_INT;
		vAPI.vArgDefn[2].vDataSpecifier = DSPECIFIER_FIRST;
		vAPI.vArgDefn[2].bIsPrimitive = true;

		InitArg(&(vAPI.vArgDefn[3]));
		vAPI.vArgDefn[3].nPosition = 3;
		vAPI.vArgDefn[3].nMemSize = sizeof( OMX_U32 );
		vAPI.vArgDefn[3].vDataType = DTYPE_LONG_INT;
		vAPI.vArgDefn[3].vDataSpecifier = DSPECIFIER_UNSIGNED;
		vAPI.vArgDefn[3].bIsPrimitive = true;

		InitArg(&(vAPI.vArgDefn[4]));
		vAPI.vArgDefn[4].nPosition = 4;
		vAPI.vArgDefn[4].nMemSize = sizeof( OMX_U32 );
		vAPI.vArgDefn[4].vDataType = DTYPE_LONG_INT;
		vAPI.vArgDefn[4].vDataSpecifier = DSPECIFIER_UNSIGNED;
		vAPI.vArgDefn[4].bIsPrimitive = true;

		InitArg(&(vAPI.vArgDefn[5]));
		vAPI.vArgDefn[5].nPosition = 5;
		vAPI.vArgDefn[5].nMemSize = sizeof( OMX_U32 );
		vAPI.vArgDefn[5].vDataType = DTYPE_LONG_INT;
		vAPI.vArgDefn[5].vDataSpecifier = DSPECIFIER_UNSIGNED;
		vAPI.vArgDefn[5].bIsPrimitive = true;

		InitArg(&(vAPI.vArgDefn[6]));
		vAPI.vArgDefn[6].nPosition = 6;
		vAPI.vArgDefn[6].nMemSize = sizeof( OMX_U32 );
		vAPI.vArgDefn[6].vDataType = DTYPE_LONG_INT;
		vAPI.vArgDefn[6].vDataSpecifier = DSPECIFIER_UNSIGNED;
		vAPI.vArgDefn[6].bIsPrimitive = true;

		InitArg(&(vAPI.vArgDefn[7]));
		vAPI.vArgDefn[7].nPosition = 7;
		vAPI.vArgDefn[7].nMemSize = sizeof( OMX_U32 );
		vAPI.vArgDefn[7].vDataType = DTYPE_LONG_INT;
		vAPI.vArgDefn[7].vDataSpecifier = DSPECIFIER_UNSIGNED;
		vAPI.vArgDefn[7].bIsPrimitive = true;

		InitArg(&(vAPI.vArgDefn[8]));
		vAPI.vArgDefn[8].nPosition = 8;
		vAPI.vArgDefn[8].nMemSize = sizeof(int);
		vAPI.vArgDefn[8].vDataType = DTYPE_INT;
		vAPI.vArgDefn[8].vDataSpecifier = DSPECIFIER_FIRST;
		vAPI.vArgDefn[8].bIsPrimitive = true;

		InitArg(&(vAPI.vArgDefn[9]));
		vAPI.vArgDefn[9].nPosition = 9;
		vAPI.vArgDefn[9].nMemSize = sizeof(int);
		vAPI.vArgDefn[9].vDataType = DTYPE_INT;
		vAPI.vArgDefn[9].vDataSpecifier = DSPECIFIER_FIRST;
		vAPI.vArgDefn[9].bIsPrimitive = true;

		InitArg(&(vAPI.vArgDefn[10]));
		vAPI.vArgDefn[10].nPosition = 10;
		vAPI.vArgDefn[10].nMemSize = sizeof(int);
		vAPI.vArgDefn[10].vDataType = DTYPE_INT;
		vAPI.vArgDefn[10].vDataSpecifier = DSPECIFIER_FIRST;
		vAPI.vArgDefn[10].bIsPrimitive = true;

		InitArg(&(vAPI.vArgDefn[11]));
		vAPI.vArgDefn[11].nPosition = 11;
		vAPI.vArgDefn[11].nMemSize = sizeof( OMX_STRING );
		vAPI.vArgDefn[11].bIsPointer = true;
		vAPI.vArgDefn[11].vDataType = DTYPE_CHAR;
		vAPI.vArgDefn[11].vDataSpecifier = DSPECIFIER_FIRST;
		vAPI.vArgDefn[11].bIsPrimitive = true;

		InitArg(&(vAPI.vArgDefn[12]));
		vAPI.vArgDefn[12].nPosition = 12;
		vAPI.vArgDefn[12].nMemSize = sizeof( OMX_U32 );
		vAPI.vArgDefn[12].vDataType = DTYPE_LONG_INT;
		vAPI.vArgDefn[12].vDataSpecifier = DSPECIFIER_UNSIGNED;
		vAPI.vArgDefn[12].bIsPrimitive = true;

		InitArg(&(vAPI.vArgDefn[13]));
		vAPI.vArgDefn[13].nPosition = 13;
		vAPI.vArgDefn[13].nMemSize = sizeof( OMX_U32 );
		vAPI.vArgDefn[13].vDataType = DTYPE_LONG_INT;
		vAPI.vArgDefn[13].vDataSpecifier = DSPECIFIER_UNSIGNED;
		vAPI.vArgDefn[13].bIsPrimitive = true;

		InitArg(&(vAPI.vArgDefn[14]));
		vAPI.vArgDefn[14].nPosition = 14;
		vAPI.vArgDefn[14].nMemSize = sizeof( OMX_S32 );
		vAPI.vArgDefn[14].vDataType = DTYPE_LONG_INT;
		vAPI.vArgDefn[14].vDataSpecifier = DSPECIFIER_FIRST;
		vAPI.vArgDefn[14].bIsPrimitive = true;

		InitArg(&(vAPI.vArgDefn[15]));
		vAPI.vArgDefn[15].nPosition = 15;
		vAPI.vArgDefn[15].nMemSize = sizeof( OMX_U32 );
		vAPI.vArgDefn[15].vDataType = DTYPE_LONG_INT;
		vAPI.vArgDefn[15].vDataSpecifier = DSPECIFIER_UNSIGNED;
		vAPI.vArgDefn[15].bIsPrimitive = true;

		InitArg(&(vAPI.vArgDefn[16]));
		vAPI.vArgDefn[16].nPosition = 16;
		vAPI.vArgDefn[16].nMemSize = sizeof(int);
		vAPI.vArgDefn[16].vDataType = DTYPE_INT;
		vAPI.vArgDefn[16].vDataSpecifier = DSPECIFIER_FIRST;
		vAPI.vArgDefn[16].bIsPrimitive = true;

		InitArg(&(vAPI.vArgDefn[17]));
		vAPI.vArgDefn[17].nPosition = 17;
		vAPI.vArgDefn[17].nMemSize = sizeof( OMX_U32 );
		vAPI.vArgDefn[17].vDataType = DTYPE_LONG_INT;
		vAPI.vArgDefn[17].vDataSpecifier = DSPECIFIER_UNSIGNED;
		vAPI.vArgDefn[17].bIsPrimitive = true;

		InitArg(&(vAPI.vArgDefn[18]));
		vAPI.vArgDefn[18].nPosition = 18;
		vAPI.vArgDefn[18].nMemSize = sizeof( OMX_U32 );
		vAPI.vArgDefn[18].vDataType = DTYPE_LONG_INT;
		vAPI.vArgDefn[18].vDataSpecifier = DSPECIFIER_UNSIGNED;
		vAPI.vArgDefn[18].bIsPrimitive = true;

		InitArg(&(vAPI.vReturnType));
		vAPI.vReturnType.nMemSize = sizeof(int);
		vAPI.vReturnType.vDataType = DTYPE_INT;
		vAPI.vReturnType.vDataSpecifier = DSPECIFIER_FIRST;
		vAPI.vReturnType.bIsPrimitive = true;
		nRetVal = (*regFunc)(&vAPI);
		if(RET_SUCCESS != nRetVal)
		{
			////DBGPRT(LEVEL_ERROR, (char *)"Function : JpegOMX_SetParameter_PortDef registration failed!!!");
			return RET_FAILURE;
		}
	}

	{
		APIDefn vAPI;
		strlcpy(vAPI.cLibraryName, "libFuzzmmstillomxenc.so", MAX_FILE_LEN);
		strlcpy(vAPI.cAPIName, "JpegOMX_SetConfig_Rotate", MAX_API_NAME_LEN);
		vAPI.vFuncPtr     = &JpegOMX_SetConfig_Rotate_WRAP;
		vAPI.nNumArgs     = 3;

		InitArg(&(vAPI.vArgDefn[0]));
		vAPI.vArgDefn[0].nPosition = 0;
		vAPI.vArgDefn[0].nMemSize = sizeof(int);
		vAPI.vArgDefn[0].vDataType = DTYPE_INT;
		vAPI.vArgDefn[0].vDataSpecifier = DSPECIFIER_FIRST;
		vAPI.vArgDefn[0].bIsPrimitive = true;

		InitArg(&(vAPI.vArgDefn[1]));
		vAPI.vArgDefn[1].nPosition = 1;
		vAPI.vArgDefn[1].nMemSize = sizeof( OMX_U32 );
		vAPI.vArgDefn[1].vDataType = DTYPE_LONG_INT;
		vAPI.vArgDefn[1].vDataSpecifier = DSPECIFIER_UNSIGNED;
		vAPI.vArgDefn[1].bIsPrimitive = true;

		InitArg(&(vAPI.vArgDefn[2]));
		vAPI.vArgDefn[2].nPosition = 2;
		vAPI.vArgDefn[2].nMemSize = sizeof( OMX_S32 );
		vAPI.vArgDefn[2].vDataType = DTYPE_LONG_INT;
		vAPI.vArgDefn[2].vDataSpecifier = DSPECIFIER_FIRST;
		vAPI.vArgDefn[2].bIsPrimitive = true;

		InitArg(&(vAPI.vReturnType));
		vAPI.vReturnType.nMemSize = sizeof(int);
		vAPI.vReturnType.vDataType = DTYPE_INT;
		vAPI.vReturnType.vDataSpecifier = DSPECIFIER_FIRST;
		vAPI.vReturnType.bIsPrimitive = true;
		nRetVal = (*regFunc)(&vAPI);
		if(RET_SUCCESS != nRetVal)
		{
			////DBGPRT(LEVEL_ERROR, (char *)"Function : JpegOMX_SetConfig_Rotate registration failed!!!");
			return RET_FAILURE;
		}
	}

	{
		APIDefn vAPI;
		strlcpy(vAPI.cLibraryName, "libFuzzmmstillomxenc.so", MAX_FILE_LEN);
		strlcpy(vAPI.cAPIName, "JpegOMX_UseBuffer", MAX_API_NAME_LEN);
		vAPI.vFuncPtr     = &JpegOMX_UseBuffer_WRAP;
		vAPI.nNumArgs     = 5;

		InitArg(&(vAPI.vArgDefn[0]));
		vAPI.vArgDefn[0].nPosition = 0;
		vAPI.vArgDefn[0].nMemSize = sizeof(int);
		vAPI.vArgDefn[0].vDataType = DTYPE_INT;
		vAPI.vArgDefn[0].vDataSpecifier = DSPECIFIER_FIRST;
		vAPI.vArgDefn[0].bIsPrimitive = true;

		InitArg(&(vAPI.vArgDefn[1]));
		vAPI.vArgDefn[1].nPosition = 1;
		vAPI.vArgDefn[1].nMemSize = sizeof( OMX_U32 );
		vAPI.vArgDefn[1].vDataType = DTYPE_LONG_INT;
		vAPI.vArgDefn[1].vDataSpecifier = DSPECIFIER_UNSIGNED;
		vAPI.vArgDefn[1].bIsPrimitive = true;

		InitArg(&(vAPI.vArgDefn[2]));
		vAPI.vArgDefn[2].nPosition = 2;
		vAPI.vArgDefn[2].nMemSize = sizeof(int);
		vAPI.vArgDefn[2].vDataType = DTYPE_INT;
		vAPI.vArgDefn[2].vDataSpecifier = DSPECIFIER_FIRST;
		vAPI.vArgDefn[2].bIsPrimitive = true;

		InitArg(&(vAPI.vArgDefn[3]));
		vAPI.vArgDefn[3].nPosition = 3;
		vAPI.vArgDefn[3].nMemSize = sizeof(unsigned int);
		vAPI.vArgDefn[3].vDataType = DTYPE_INT;
		vAPI.vArgDefn[3].vDataSpecifier = DSPECIFIER_UNSIGNED;
		vAPI.vArgDefn[3].bIsPrimitive = true;

		InitArg(&(vAPI.vArgDefn[4]));
		vAPI.vArgDefn[4].nPosition = 4;
		vAPI.vArgDefn[4].nMemSize = sizeof(int);
		vAPI.vArgDefn[4].vDataType = DTYPE_INT;
		vAPI.vArgDefn[4].vDataSpecifier = DSPECIFIER_FIRST;
		vAPI.vArgDefn[4].bIsPrimitive = true;

		InitArg(&(vAPI.vReturnType));
		vAPI.vReturnType.nMemSize = sizeof(int);
		vAPI.vReturnType.vDataType = DTYPE_INT;
		vAPI.vReturnType.vDataSpecifier = DSPECIFIER_FIRST;
		vAPI.vReturnType.bIsPrimitive = true;
		nRetVal = (*regFunc)(&vAPI);
		if(RET_SUCCESS != nRetVal)
		{
			////DBGPRT(LEVEL_ERROR, (char *)"Function : JpegOMX_UseBuffer registration failed!!!");
			return RET_FAILURE;
		}
	}

	{
		APIDefn vAPI;
		strlcpy(vAPI.cLibraryName, "libFuzzmmstillomxenc.so", MAX_FILE_LEN);
		strlcpy(vAPI.cAPIName, "JpegOMX_SetParameter_QFactor", MAX_API_NAME_LEN);
		vAPI.vFuncPtr     = &JpegOMX_SetParameter_QFactor_WRAP;
		vAPI.nNumArgs     = 4;

		InitArg(&(vAPI.vArgDefn[0]));
		vAPI.vArgDefn[0].nPosition = 0;
		vAPI.vArgDefn[0].nMemSize = sizeof(int);
		vAPI.vArgDefn[0].vDataType = DTYPE_INT;
		vAPI.vArgDefn[0].vDataSpecifier = DSPECIFIER_FIRST;
		vAPI.vArgDefn[0].bIsPrimitive = true;

		InitArg(&(vAPI.vArgDefn[1]));
		vAPI.vArgDefn[1].nPosition = 1;
		vAPI.vArgDefn[1].nMemSize = sizeof( OMX_U32 );
		vAPI.vArgDefn[1].vDataType = DTYPE_LONG_INT;
		vAPI.vArgDefn[1].vDataSpecifier = DSPECIFIER_UNSIGNED;
		vAPI.vArgDefn[1].bIsPrimitive = true;

		InitArg(&(vAPI.vArgDefn[2]));
		vAPI.vArgDefn[2].nPosition = 2;
		vAPI.vArgDefn[2].nMemSize = sizeof( OMX_U32 );
		vAPI.vArgDefn[2].vDataType = DTYPE_LONG_INT;
		vAPI.vArgDefn[2].vDataSpecifier = DSPECIFIER_UNSIGNED;
		vAPI.vArgDefn[2].bIsPrimitive = true;

		InitArg(&(vAPI.vArgDefn[3]));
		vAPI.vArgDefn[3].nPosition = 3;
		vAPI.vArgDefn[3].nMemSize = sizeof(int);
		vAPI.vArgDefn[3].vDataType = DTYPE_INT;
		vAPI.vArgDefn[3].vDataSpecifier = DSPECIFIER_FIRST;
		vAPI.vArgDefn[3].bIsPrimitive = true;

		InitArg(&(vAPI.vReturnType));
		vAPI.vReturnType.nMemSize = sizeof(int);
		vAPI.vReturnType.vDataType = DTYPE_INT;
		vAPI.vReturnType.vDataSpecifier = DSPECIFIER_FIRST;
		vAPI.vReturnType.bIsPrimitive = true;
		nRetVal = (*regFunc)(&vAPI);
		if(RET_SUCCESS != nRetVal)
		{
			////DBGPRT(LEVEL_ERROR, (char *)"Function : JpegOMX_SetParameter_QFactor registration failed!!!");
			return RET_FAILURE;
		}
	}

	{
		APIDefn vAPI;
		strlcpy(vAPI.cLibraryName, "libFuzzmmstillomxenc.so", MAX_FILE_LEN);
		strlcpy(vAPI.cAPIName, "JpegOMX_GetParameter", MAX_API_NAME_LEN);
		vAPI.vFuncPtr     = &JpegOMX_GetParameter_WRAP;
		vAPI.nNumArgs     = 3;

		InitArg(&(vAPI.vArgDefn[0]));
		vAPI.vArgDefn[0].nPosition = 0;
		vAPI.vArgDefn[0].nMemSize = sizeof(int);
		vAPI.vArgDefn[0].vDataType = DTYPE_INT;
		vAPI.vArgDefn[0].vDataSpecifier = DSPECIFIER_FIRST;
		vAPI.vArgDefn[0].bIsPrimitive = true;

		InitArg(&(vAPI.vArgDefn[1]));
		vAPI.vArgDefn[1].nPosition = 1;
		vAPI.vArgDefn[1].nMemSize = sizeof(int);
		vAPI.vArgDefn[1].vDataType = DTYPE_INT;
		vAPI.vArgDefn[1].vDataSpecifier = DSPECIFIER_FIRST;
		vAPI.vArgDefn[1].bIsPrimitive = true;

		InitArg(&(vAPI.vArgDefn[2]));
		vAPI.vArgDefn[2].nPosition = 2;
		vAPI.vArgDefn[2].nMemSize = sizeof(int);
		vAPI.vArgDefn[2].vDataType = DTYPE_INT;
		vAPI.vArgDefn[2].vDataSpecifier = DSPECIFIER_FIRST;
		vAPI.vArgDefn[2].bIsPrimitive = true;

		InitArg(&(vAPI.vReturnType));
		vAPI.vReturnType.nMemSize = sizeof(int);
		vAPI.vReturnType.vDataType = DTYPE_INT;
		vAPI.vReturnType.vDataSpecifier = DSPECIFIER_FIRST;
		vAPI.vReturnType.bIsPrimitive = true;
		nRetVal = (*regFunc)(&vAPI);
		if(RET_SUCCESS != nRetVal)
		{
			////DBGPRT(LEVEL_ERROR, (char *)"Function : JpegOMX_GetParameter registration failed!!!");
			return RET_FAILURE;
		}
	}

	/*  DO NOT REMOVE --<UpdateCodeTag_PopulateRegisterApis>-- */ 
	        
    return RET_SUCCESS;
}

/*==========================================================================*
	Name: int JpegOMX_FreeHandle_WRAP(void *args)

	Input: void *args : Input Arguments

	Return: API return value

	Purpose: Wrapper function that the fuzzer will invoke
*==========================================================================*/
int JpegOMX_FreeHandle_WRAP(FuzzedArgs *vArgList)
{
	// Below code is generated. Do not modify.
	////DBGPRT(LEVEL_INFO, (char *)"JpegOMX_FreeHandle_WRAP called");
	int Value0 = *((int*)(vArgList->vArg[0]));

	*((int*)(vArgList->vReturnArg)) = JpegOMX_FreeHandle(Value0);

	return RET_SUCCESS;
}
#ifndef OMX_CODEC_V1_WRAPPER
/*==========================================================================*
	Name: int JpegOMX_SetParameter_ACbCrOffset_WRAP(void *args)

	Input: void *args : Input Arguments

	Return: API return value

	Purpose: Wrapper function that the fuzzer will invoke
*==========================================================================*/
int JpegOMX_SetParameter_ACbCrOffset_WRAP(FuzzedArgs *vArgList)
{
	// Below code is generated. Do not modify.
	////DBGPRT(LEVEL_INFO, (char *)"JpegOMX_SetParameter_ACbCrOffset_WRAP called");
	int Value0 = *((int*)(vArgList->vArg[0]));
	omx_jpeg_buffer_offset Value1 = *((omx_jpeg_buffer_offset*)(vArgList->vArg[1]));

	*((int*)(vArgList->vReturnArg)) = JpegOMX_SetParameter_ACbCrOffset(Value0, Value1);

	return RET_SUCCESS;
}
#endif
/*==========================================================================*
	Name: int JpegOMX_DeInit_WRAP(void *args)

	Input: void *args : Input Arguments

	Return: API return value

	Purpose: Wrapper function that the fuzzer will invoke
*==========================================================================*/
int JpegOMX_DeInit_WRAP(FuzzedArgs *vArgList)
{
	// Below code is generated. Do not modify.
	////DBGPRT(LEVEL_INFO, (char *)"JpegOMX_DeInit_WRAP called");
	*((int*)(vArgList->vReturnArg)) = JpegOMX_DeInit();

	return RET_SUCCESS;
}
/*==========================================================================*
	Name: int JpegOMX_SetParameter_Thumbnail_WRAP(void *args)

	Input: void *args : Input Arguments

	Return: API return value

	Purpose: Wrapper function that the fuzzer will invoke
*==========================================================================*/
int JpegOMX_SetParameter_Thumbnail_WRAP(FuzzedArgs *vArgList)
{
	// Below code is generated. Do not modify.
	////DBGPRT(LEVEL_INFO, (char *)"JpegOMX_SetParameter_Thumbnail_WRAP called");
	int Value0 = *((int*)(vArgList->vArg[0]));
	int Value1 = *((int*)(vArgList->vArg[1]));
	int Value2 = *((int*)(vArgList->vArg[2]));
	int Value3 = *((int*)(vArgList->vArg[3]));
	int Value4 = *((int*)(vArgList->vArg[4]));
	int Value5 = *((int*)(vArgList->vArg[5]));
	int Value6 = *((int*)(vArgList->vArg[6]));
	int Value7 = *((int*)(vArgList->vArg[7]));

	*((int*)(vArgList->vReturnArg)) = JpegOMX_SetParameter_Thumbnail(Value0, Value1, Value2, Value3, Value4, Value5, Value6, Value7);

	return RET_SUCCESS;
}
/*==========================================================================*
	Name: int JpegOMX_SetParameter_UserPreferences_WRAP(void *args)

	Input: void *args : Input Arguments

	Return: API return value

	Purpose: Wrapper function that the fuzzer will invoke
*==========================================================================*/
int JpegOMX_SetParameter_UserPreferences_WRAP(FuzzedArgs *vArgList)
{
	// Below code is generated. Do not modify.
	////DBGPRT(LEVEL_INFO, (char *)"JpegOMX_SetParameter_UserPreferences_WRAP called");
	int Value0 = *((int*)(vArgList->vArg[0]));
	int Value1 = *((int*)(vArgList->vArg[1]));
	int Value2 = *((int*)(vArgList->vArg[2]));
	int Value3 = *((int*)(vArgList->vArg[3]));

	*((int*)(vArgList->vReturnArg)) = JpegOMX_SetParameter_UserPreferences(Value0, Value1, Value2, Value3);

	return RET_SUCCESS;
}
/*==========================================================================*
	Name: int JpegOMX_SendJpegCommand_WRAP(void *args)

	Input: void *args : Input Arguments

	Return: API return value

	Purpose: Wrapper function that the fuzzer will invoke
*==========================================================================*/
int JpegOMX_SendJpegCommand_WRAP(FuzzedArgs *vArgList)
{
	// Below code is generated. Do not modify.
	////DBGPRT(LEVEL_INFO, (char *)"JpegOMX_SendJpegCommand_WRAP called");
	int Value0 = *((int*)(vArgList->vArg[0]));
	OMX_U32 Value1 = *((OMX_U32*)(vArgList->vArg[1]));

	*((int*)(vArgList->vReturnArg)) = JpegOMX_SendJpegCommand(Value0, Value1);

	return RET_SUCCESS;
}
/*==========================================================================*
	Name: int StartEncode_WRAP(void *args)

	Input: void *args : Input Arguments

	Return: API return value

	Purpose: Wrapper function that the fuzzer will invoke
*==========================================================================*/
int StartEncode_WRAP(FuzzedArgs *vArgList)
{
	// Below code is generated. Do not modify.
//	//DBGPRT(LEVEL_INFO, (char *)"StartEncode_WRAP called");
	*((int*)(vArgList->vReturnArg)) = StartEncode();

	return RET_SUCCESS;
}
/*==========================================================================*
	Name: int JpegOMX_SetParameter_ImageInit_WRAP(void *args)

	Input: void *args : Input Arguments

	Return: API return value

	Purpose: Wrapper function that the fuzzer will invoke
*==========================================================================*/
int JpegOMX_SetParameter_ImageInit_WRAP(FuzzedArgs *vArgList)
{
	// Below code is generated. Do not modify.
	////DBGPRT(LEVEL_INFO, (char *)"JpegOMX_SetParameter_ImageInit_WRAP called");
	int Value0 = *((int*)(vArgList->vArg[0]));
	int Value1 = *((int*)(vArgList->vArg[1]));
	int Value2 = *((int*)(vArgList->vArg[2]));
	int Value3 = *((int*)(vArgList->vArg[3]));
	OMX_U8 Value4 = *((OMX_U8*)(vArgList->vArg[4]));
	OMX_U8 Value5 = *((OMX_U8*)(vArgList->vArg[5]));
	OMX_U8 Value6 = *((OMX_U8*)(vArgList->vArg[6]));
	OMX_U8 Value7 = *((OMX_U8*)(vArgList->vArg[7]));

	*((int*)(vArgList->vReturnArg)) = JpegOMX_SetParameter_ImageInit(Value0, Value1, Value2, Value3, Value4, Value5, Value6, Value7);

	return RET_SUCCESS;
}
/*==========================================================================*
	Name: int JpegOMX_SetAllConfigs_WRAP(void *args)

	Input: void *args : Input Arguments

	Return: API return value

	Purpose: Wrapper function that the fuzzer will invoke
*==========================================================================*/
int JpegOMX_SetAllConfigs_WRAP(FuzzedArgs *vArgList)
{
	// Below code is generated. Do not modify.
//	//DBGPRT(LEVEL_INFO, (char *)"JpegOMX_SetAllConfigs_WRAP called");
	*((int*)(vArgList->vReturnArg)) = JpegOMX_SetAllConfigs();

	return RET_SUCCESS;
}
/*==========================================================================*
	Name: int JpegOMX_SetParameter_Exif_WRAP(void *args)

	Input: void *args : Input Arguments

	Return: API return value

	Purpose: Wrapper function that the fuzzer will invoke
*==========================================================================*/
int JpegOMX_SetParameter_Exif_WRAP(FuzzedArgs *vArgList)
{
	// Below code is generated. Do not modify.
	////DBGPRT(LEVEL_INFO, (char *)"JpegOMX_SetParameter_Exif_WRAP called");
	int Value0 = *((int*)(vArgList->vArg[0]));
	int Value1 = *((int*)(vArgList->vArg[1]));
	int Value2 = *((int*)(vArgList->vArg[2]));
	int Value3 = *((int*)(vArgList->vArg[3]));
	int Value4 = *((int*)(vArgList->vArg[4]));
	int Value5 = *((int*)(vArgList->vArg[5]));
	int Value6 = *((int*)(vArgList->vArg[6]));
	char *Value7 = (char*)(vArgList->vArg[7]);

	*((int*)(vArgList->vReturnArg)) = JpegOMX_SetParameter_Exif(Value0, Value1, Value2, Value3, Value4, Value5, Value6, Value7);

	return RET_SUCCESS;
}
/*==========================================================================*
	Name: int JpegOMX_GetConfig_WRAP(void *args)

	Input: void *args : Input Arguments

	Return: API return value

	Purpose: Wrapper function that the fuzzer will invoke
*==========================================================================*/
int JpegOMX_GetConfig_WRAP(FuzzedArgs *vArgList)
{
	// Below code is generated. Do not modify.
	////DBGPRT(LEVEL_INFO, (char *)"JpegOMX_GetConfig_WRAP called");
	int Value0 = *((int*)(vArgList->vArg[0]));
	int Value1 = *((int*)(vArgList->vArg[1]));
	int Value2 = *((int*)(vArgList->vArg[2]));

	*((int*)(vArgList->vReturnArg)) = JpegOMX_GetConfig(Value0, Value1, Value2);

	return RET_SUCCESS;
}
/*==========================================================================*
	Name: int JpegOMX_AllFreeBuffer_WRAP(void *args)

	Input: void *args : Input Arguments

	Return: API return value

	Purpose: Wrapper function that the fuzzer will invoke
*==========================================================================*/
int JpegOMX_AllFreeBuffer_WRAP(FuzzedArgs *vArgList)
{
	// Below code is generated. Do not modify.
	////DBGPRT(LEVEL_INFO, (char *)"JpegOMX_AllFreeBuffer_WRAP called");
	*((int*)(vArgList->vReturnArg)) = JpegOMX_AllFreeBuffer();

	return RET_SUCCESS;
}
/*==========================================================================*
	Name: int JpegOMX_EmptyThisBuffer_WRAP(void *args)

	Input: void *args : Input Arguments

	Return: API return value

	Purpose: Wrapper function that the fuzzer will invoke
*==========================================================================*/
int JpegOMX_EmptyThisBuffer_WRAP(FuzzedArgs *vArgList)
{
	// Below code is generated. Do not modify.
//	//DBGPRT(LEVEL_INFO, (char *)"JpegOMX_EmptyThisBuffer_WRAP called");
	int Value0 = *((int*)(vArgList->vArg[0]));
	int Value1 = *((int*)(vArgList->vArg[1]));

	*((int*)(vArgList->vReturnArg)) = JpegOMX_EmptyThisBuffer(Value0, Value1);

	return RET_SUCCESS;
}
/*==========================================================================*
	Name: int JpegOMX_ComponentTunnelRequest_WRAP(void *args)

	Input: void *args : Input Arguments

	Return: API return value

	Purpose: Wrapper function that the fuzzer will invoke
*==========================================================================*/
int JpegOMX_ComponentTunnelRequest_WRAP(FuzzedArgs *vArgList)
{
	// Below code is generated. Do not modify.
	////DBGPRT(LEVEL_INFO, (char *)"JpegOMX_ComponentTunnelRequest_WRAP called");
	int Value0 = *((int*)(vArgList->vArg[0]));
	unsigned int Value1 = *((unsigned int*)(vArgList->vArg[1]));
	int Value2 = *((int*)(vArgList->vArg[2]));
	unsigned int Value3 = *((unsigned int*)(vArgList->vArg[3]));
	OMX_TUNNELSETUPTYPE Value4 = *((OMX_TUNNELSETUPTYPE*)(vArgList->vArg[4]));

	*((int*)(vArgList->vReturnArg)) = JpegOMX_ComponentTunnelRequest(Value0, Value1, Value2, Value3, Value4);

	return RET_SUCCESS;
}
/*==========================================================================*
	Name: int JpegOMX_FreeJpegHandle_WRAP(void *args)

	Input: void *args : Input Arguments

	Return: API return value

	Purpose: Wrapper function that the fuzzer will invoke
*==========================================================================*/
int JpegOMX_FreeJpegHandle_WRAP(FuzzedArgs *vArgList)
{
	// Below code is generated. Do not modify.
	////DBGPRT(LEVEL_INFO, (char *)"JpegOMX_FreeJpegHandle_WRAP called");
	*((int*)(vArgList->vReturnArg)) = JpegOMX_FreeJpegHandle();

	return RET_SUCCESS;
}
/*==========================================================================*
	Name: int JpegOMX_SetAllParameters_WRAP(void *args)

	Input: void *args : Input Arguments

	Return: API return value

	Purpose: Wrapper function that the fuzzer will invoke
*==========================================================================*/
int JpegOMX_SetAllParameters_WRAP(FuzzedArgs *vArgList)
{
	// Below code is generated. Do not modify.
	////DBGPRT(LEVEL_INFO, (char *)"JpegOMX_SetAllParameters_WRAP called");
	*((int*)(vArgList->vReturnArg)) = JpegOMX_SetAllParameters();

	return RET_SUCCESS;
}
/*==========================================================================*
	Name: int JpegOMX_SetConfig_InputCrop_WRAP(void *args)

	Input: void *args : Input Arguments

	Return: API return value

	Purpose: Wrapper function that the fuzzer will invoke
*==========================================================================*/
int JpegOMX_SetConfig_InputCrop_WRAP(FuzzedArgs *vArgList)
{
	// Below code is generated. Do not modify.
	////DBGPRT(LEVEL_INFO, (char *)"JpegOMX_SetConfig_InputCrop_WRAP called");
	int Value0 = *((int*)(vArgList->vArg[0]));
	OMX_U32 Value1 = *((OMX_U32*)(vArgList->vArg[1]));
	OMX_U32 Value2 = *((OMX_U32*)(vArgList->vArg[2]));
	OMX_U32 Value3 = *((OMX_U32*)(vArgList->vArg[3]));
	OMX_U32 Value4 = *((OMX_U32*)(vArgList->vArg[4]));
	OMX_U32 Value5 = *((OMX_U32*)(vArgList->vArg[5]));

	*((int*)(vArgList->vReturnArg)) = JpegOMX_SetConfig_InputCrop(Value0, Value1, Value2, Value3, Value4, Value5);

	return RET_SUCCESS;
}
/*==========================================================================*
	Name: int JpegOMX_FreeBuffer_WRAP(void *args)

	Input: void *args : Input Arguments

	Return: API return value

	Purpose: Wrapper function that the fuzzer will invoke
*==========================================================================*/
int JpegOMX_FreeBuffer_WRAP(FuzzedArgs *vArgList)
{
	// Below code is generated. Do not modify.
	////DBGPRT(LEVEL_INFO, (char *)"JpegOMX_FreeBuffer_WRAP called");
	int Value0 = *((int*)(vArgList->vArg[0]));
	OMX_U32 Value1 = *((OMX_U32*)(vArgList->vArg[1]));
	int Value2 = *((int*)(vArgList->vArg[2]));

	*((int*)(vArgList->vReturnArg)) = JpegOMX_FreeBuffer(Value0, Value1, Value2);

	return RET_SUCCESS;
}
/*==========================================================================*
	Name: int JpegOMX_AllocateBuffer_WRAP(void *args)

	Input: void *args : Input Arguments

	Return: API return value

	Purpose: Wrapper function that the fuzzer will invoke
*==========================================================================*/
int JpegOMX_AllocateBuffer_WRAP(FuzzedArgs *vArgList)
{
	// Below code is generated. Do not modify.
	////DBGPRT(LEVEL_INFO, (char *)"JpegOMX_AllocateBuffer_WRAP called");
	int Value0 = *((int*)(vArgList->vArg[0]));
	OMX_U32 Value1 = *((OMX_U32*)(vArgList->vArg[1]));
	int Value2 = *((int*)(vArgList->vArg[2]));
	unsigned int Value3 = *((unsigned int*)(vArgList->vArg[3]));

	*((int*)(vArgList->vReturnArg)) = JpegOMX_AllocateBuffer(Value0, Value1, Value2, Value3);

	return RET_SUCCESS;
}
/*==========================================================================*
	Name: int JpegOMX_GetHandle_WRAP(void *args)

	Input: void *args : Input Arguments

	Return: API return value

	Purpose: Wrapper function that the fuzzer will invoke
*==========================================================================*/
int JpegOMX_GetHandle_WRAP(FuzzedArgs *vArgList)
{
	// Below code is generated. Do not modify.
	////DBGPRT(LEVEL_INFO, (char *)"JpegOMX_GetHandle_WRAP called");
	char *Value0 = (char*)(vArgList->vArg[0]);
	char *Value1 = (char*)(vArgList->vArg[1]);
	OMX_CALLBACKTYPE *Value2 = (OMX_CALLBACKTYPE*)(vArgList->vArg[2]);

	*((int*)(vArgList->vReturnArg)) = JpegOMX_GetHandle(Value0, Value1, Value2);

	return RET_SUCCESS;
}
/*==========================================================================*
	Name: int JpegOMX_SetConfig_WRAP(void *args)

	Input: void *args : Input Arguments

	Return: API return value

	Purpose: Wrapper function that the fuzzer will invoke
*==========================================================================*/
int JpegOMX_SetConfig_WRAP(FuzzedArgs *vArgList)
{
	// Below code is generated. Do not modify.
	////DBGPRT(LEVEL_INFO, (char *)"JpegOMX_SetConfig_WRAP called");
	int Value0 = *((int*)(vArgList->vArg[0]));
	int Value1 = *((int*)(vArgList->vArg[1]));
	int Value2 = *((int*)(vArgList->vArg[2]));

	*((int*)(vArgList->vReturnArg)) = JpegOMX_SetConfig(Value0, Value1, Value2);

	return RET_SUCCESS;
}
/*==========================================================================*
	Name: int JpegOMX_SetParameter_ThumbQuality_WRAP(void *args)

	Input: void *args : Input Arguments

	Return: API return value

	Purpose: Wrapper function that the fuzzer will invoke
*==========================================================================*/
int JpegOMX_SetParameter_ThumbQuality_WRAP(FuzzedArgs *vArgList)
{
	// Below code is generated. Do not modify.
	////DBGPRT(LEVEL_INFO, (char *)"JpegOMX_SetParameter_ThumbQuality_WRAP called");
	int Value0 = *((int*)(vArgList->vArg[0]));
	int Value1 = *((int*)(vArgList->vArg[1]));

	*((int*)(vArgList->vReturnArg)) = JpegOMX_SetParameter_ThumbQuality(Value0, Value1);

	return RET_SUCCESS;
}
/*==========================================================================*
	Name: int JpegOMX_SetConfig_OutputCrop_WRAP(void *args)

	Input: void *args : Input Arguments

	Return: API return value

	Purpose: Wrapper function that the fuzzer will invoke
*==========================================================================*/
int JpegOMX_SetConfig_OutputCrop_WRAP(FuzzedArgs *vArgList)
{
	// Below code is generated. Do not modify.
	////DBGPRT(LEVEL_INFO, (char *)"JpegOMX_SetConfig_OutputCrop_WRAP called");
	int Value0 = *((int*)(vArgList->vArg[0]));
	OMX_U32 Value1 = *((OMX_U32*)(vArgList->vArg[1]));
	OMX_U32 Value2 = *((OMX_U32*)(vArgList->vArg[2]));
	OMX_U32 Value3 = *((OMX_U32*)(vArgList->vArg[3]));
	OMX_U32 Value4 = *((OMX_U32*)(vArgList->vArg[4]));
	OMX_U32 Value5 = *((OMX_U32*)(vArgList->vArg[5]));

	*((int*)(vArgList->vReturnArg)) = JpegOMX_SetConfig_OutputCrop(Value0, Value1, Value2, Value3, Value4, Value5);

	return RET_SUCCESS;
}
/*==========================================================================*
	Name: int JpegOMX_FillThisBuffer_WRAP(void *args)

	Input: void *args : Input Arguments

	Return: API return value

	Purpose: Wrapper function that the fuzzer will invoke
*==========================================================================*/
int JpegOMX_FillThisBuffer_WRAP(FuzzedArgs *vArgList)
{
	// Below code is generated. Do not modify.
	////DBGPRT(LEVEL_INFO, (char *)"JpegOMX_FillThisBuffer_WRAP called");
	int Value0 = *((int*)(vArgList->vArg[0]));
	int Value1 = *((int*)(vArgList->vArg[1]));

	*((int*)(vArgList->vReturnArg)) = JpegOMX_FillThisBuffer(Value0, Value1);

	return RET_SUCCESS;
}
/*==========================================================================*
	Name: int JpegOMX_GetState_WRAP(void *args)

	Input: void *args : Input Arguments

	Return: API return value

	Purpose: Wrapper function that the fuzzer will invoke
*==========================================================================*/
int JpegOMX_GetState_WRAP(FuzzedArgs *vArgList)
{
	// Below code is generated. Do not modify.
	////DBGPRT(LEVEL_INFO, (char *)"JpegOMX_GetState_WRAP called");
	int Value0 = *((int*)(vArgList->vArg[0]));

	*((int*)(vArgList->vReturnArg)) = JpegOMX_GetState(Value0);

	return RET_SUCCESS;
}
/*==========================================================================*
	Name: int JpegOMX_SetParameter_ImagePortFormat_WRAP(void *args)

	Input: void *args : Input Arguments

	Return: API return value

	Purpose: Wrapper function that the fuzzer will invoke
*==========================================================================*/
int JpegOMX_SetParameter_ImagePortFormat_WRAP(FuzzedArgs *vArgList)
{
	// Below code is generated. Do not modify.
	////DBGPRT(LEVEL_INFO, (char *)"JpegOMX_SetParameter_ImagePortFormat_WRAP called");
	int Value0 = *((int*)(vArgList->vArg[0]));
	int Value1 = *((int*)(vArgList->vArg[1]));
	int Value2 = *((int*)(vArgList->vArg[2]));
	int Value3 = *((int*)(vArgList->vArg[3]));
	int Value4 = *((int*)(vArgList->vArg[4]));
	int Value5 = *((int*)(vArgList->vArg[5]));
	OMX_U8 Value6 = *((OMX_U8*)(vArgList->vArg[6]));
	OMX_U8 Value7 = *((OMX_U8*)(vArgList->vArg[7]));
	OMX_U8 Value8 = *((OMX_U8*)(vArgList->vArg[8]));
	OMX_U8 Value9 = *((OMX_U8*)(vArgList->vArg[9]));

	*((int*)(vArgList->vReturnArg)) = JpegOMX_SetParameter_ImagePortFormat(Value0, Value1, Value2, Value3, Value4, Value5, Value6, Value7, Value8, Value9);

	return RET_SUCCESS;
}
/*==========================================================================*
	Name: int JpegOMX_SetParameter_BufferOffset_WRAP(void *args)

	Input: void *args : Input Arguments

	Return: API return value

	Purpose: Wrapper function that the fuzzer will invoke
*==========================================================================*/
int JpegOMX_SetParameter_BufferOffset_WRAP(FuzzedArgs *vArgList)
{
	// Below code is generated. Do not modify.
	////DBGPRT(LEVEL_INFO, (char *)"JpegOMX_SetParameter_BufferOffset_WRAP called");
	int Value0 = *((int*)(vArgList->vArg[0]));
	int Value1 = *((int*)(vArgList->vArg[1]));
	int Value2 = *((int*)(vArgList->vArg[2]));
	int Value3 = *((int*)(vArgList->vArg[3]));
	int Value4 = *((int*)(vArgList->vArg[4]));

	*((int*)(vArgList->vReturnArg)) = JpegOMX_SetParameter_BufferOffset(Value0, Value1, Value2, Value3, Value4);

	return RET_SUCCESS;
}
/*==========================================================================*
	Name: int JpegOMX_GetComponentVersion_WRAP(void *args)

	Input: void *args : Input Arguments

	Return: API return value

	Purpose: Wrapper function that the fuzzer will invoke
*==========================================================================*/
int JpegOMX_GetComponentVersion_WRAP(FuzzedArgs *vArgList)
{
	// Below code is generated. Do not modify.
	////DBGPRT(LEVEL_INFO, (char *)"JpegOMX_GetComponentVersion_WRAP called");
	int Value0 = *((int*)(vArgList->vArg[0]));
	char *Value1 = (char*)(vArgList->vArg[1]);
	int Value2 = *((int*)(vArgList->vArg[2]));
	int Value3 = *((int*)(vArgList->vArg[3]));
	int Value4 = *((int*)(vArgList->vArg[4]));

	*((int*)(vArgList->vReturnArg)) = JpegOMX_GetComponentVersion(Value0, Value1, Value2, Value3, Value4);

	return RET_SUCCESS;
}
/*==========================================================================*
	Name: int JpegOMX_GetExtIdx_WRAP(void *args)

	Input: void *args : Input Arguments

	Return: API return value

	Purpose: Wrapper function that the fuzzer will invoke
*==========================================================================*/
int JpegOMX_GetExtIdx_WRAP(FuzzedArgs *vArgList)
{
	// Below code is generated. Do not modify.
	////DBGPRT(LEVEL_INFO, (char *)"JpegOMX_GetExtIdx_WRAP called");
	int Value0 = *((int*)(vArgList->vArg[0]));
	char *Value1 = (char*)(vArgList->vArg[1]);

	*((int*)(vArgList->vReturnArg)) = JpegOMX_GetExtIdx(Value0, Value1);

	return RET_SUCCESS;
}
/*==========================================================================*
	Name: int JpegOMX_Init_WRAP(void *args)

	Input: void *args : Input Arguments

	Return: API return value

	Purpose: Wrapper function that the fuzzer will invoke
*==========================================================================*/
int JpegOMX_Init_WRAP(FuzzedArgs *vArgList)
{
	// Below code is generated. Do not modify.
	////DBGPRT(LEVEL_INFO, (char *)"JpegOMX_Init_WRAP called");
	*((int*)(vArgList->vReturnArg)) = JpegOMX_Init();

	return RET_SUCCESS;
}
/*==========================================================================*
	Name: int JpegOMX_GetJpegHandle_WRAP(void *args)

	Input: void *args : Input Arguments

	Return: API return value

	Purpose: Wrapper function that the fuzzer will invoke
*==========================================================================*/
int JpegOMX_GetJpegHandle_WRAP(FuzzedArgs *vArgList)
{
	// Below code is generated. Do not modify.
	////DBGPRT(LEVEL_INFO, (char *)"JpegOMX_GetJpegHandle_WRAP called");
	*((int*)(vArgList->vReturnArg)) = JpegOMX_GetJpegHandle();

	return RET_SUCCESS;
}
/*==========================================================================*
	Name: int JpegOMX_AllUseBuffers_WRAP(void *args)

	Input: void *args : Input Arguments

	Return: API return value

	Purpose: Wrapper function that the fuzzer will invoke
*==========================================================================*/
int JpegOMX_AllUseBuffers_WRAP(FuzzedArgs *vArgList)
{
	// Below code is generated. Do not modify.
	////DBGPRT(LEVEL_INFO, (char *)"JpegOMX_AllUseBuffers_WRAP called");
	*((int*)(vArgList->vReturnArg)) = JpegOMX_AllUseBuffers();

	return RET_SUCCESS;
}
/*==========================================================================*
	Name: int JpegOMX_SendCommand_WRAP(void *args)

	Input: void *args : Input Arguments

	Return: API return value

	Purpose: Wrapper function that the fuzzer will invoke
*==========================================================================*/
int JpegOMX_SendCommand_WRAP(FuzzedArgs *vArgList)
{
	// Below code is generated. Do not modify.
	////DBGPRT(LEVEL_INFO, (char *)"JpegOMX_SendCommand_WRAP called");
	int Value0 = *((int*)(vArgList->vArg[0]));
	int Value1 = *((int*)(vArgList->vArg[1]));
	OMX_U32 Value2 = *((OMX_U32*)(vArgList->vArg[2]));
	OMX_MARKTYPE Value3 = *((OMX_MARKTYPE*)(vArgList->vArg[3]));

	*((int*)(vArgList->vReturnArg)) = JpegOMX_SendCommand(Value0, Value1, Value2, Value3);

	return RET_SUCCESS;
}
/*==========================================================================*
	Name: int JpegOMX_SetParameter_WRAP(void *args)

	Input: void *args : Input Arguments

	Return: API return value

	Purpose: Wrapper function that the fuzzer will invoke
*==========================================================================*/
int JpegOMX_SetParameter_WRAP(FuzzedArgs *vArgList)
{
	// Below code is generated. Do not modify.
	////DBGPRT(LEVEL_INFO, (char *)"JpegOMX_SetParameter_WRAP called");
	int Value0 = *((int*)(vArgList->vArg[0]));
	int Value1 = *((int*)(vArgList->vArg[1]));
	int Value2 = *((int*)(vArgList->vArg[2]));

	*((int*)(vArgList->vReturnArg)) = JpegOMX_SetParameter(Value0, Value1, Value2);

	return RET_SUCCESS;
}
/*==========================================================================*
	Name: int JpegOMX_SetParameter_PortDef_WRAP(void *args)

	Input: void *args : Input Arguments

	Return: API return value

	Purpose: Wrapper function that the fuzzer will invoke
*==========================================================================*/
int JpegOMX_SetParameter_PortDef_WRAP(FuzzedArgs *vArgList)
{
	// Below code is generated. Do not modify.
	////DBGPRT(LEVEL_INFO, (char *)"JpegOMX_SetParameter_PortDef_WRAP called");
	int Value0 = *((int*)(vArgList->vArg[0]));
	int Value1 = *((int*)(vArgList->vArg[1]));
	int Value2 = *((int*)(vArgList->vArg[2]));
	OMX_U32 Value3 = *((OMX_U32*)(vArgList->vArg[3]));
	OMX_U32 Value4 = *((OMX_U32*)(vArgList->vArg[4]));
	OMX_U32 Value5 = *((OMX_U32*)(vArgList->vArg[5]));
	OMX_U32 Value6 = *((OMX_U32*)(vArgList->vArg[6]));
	OMX_U32 Value7 = *((OMX_U32*)(vArgList->vArg[7]));
	int Value8 = *((int*)(vArgList->vArg[8]));
	int Value9 = *((int*)(vArgList->vArg[9]));
	int Value10 = *((int*)(vArgList->vArg[10]));
	OMX_STRING Value11 = (OMX_STRING)(vArgList->vArg[11]);
	OMX_U32 Value12 = *((OMX_U32*)(vArgList->vArg[12]));
	OMX_U32 Value13 = *((OMX_U32*)(vArgList->vArg[13]));
	OMX_S32 Value14 = *((OMX_S32*)(vArgList->vArg[14]));
	OMX_U32 Value15 = *((OMX_U32*)(vArgList->vArg[15]));
	int Value16 = *((int*)(vArgList->vArg[16]));
	OMX_U32 Value17 = *((OMX_U32*)(vArgList->vArg[17]));
	OMX_U32 Value18 = *((OMX_U32*)(vArgList->vArg[18]));

	*((int*)(vArgList->vReturnArg)) = JpegOMX_SetParameter_PortDef(Value0, Value1, Value2, Value3, Value4, Value5, Value6, Value7, Value8, Value9, Value10, Value11, Value12, Value13, Value14, Value15, Value16, Value17, Value18);

	return RET_SUCCESS;
}
/*==========================================================================*
	Name: int JpegOMX_SetConfig_Rotate_WRAP(void *args)

	Input: void *args : Input Arguments

	Return: API return value

	Purpose: Wrapper function that the fuzzer will invoke
*==========================================================================*/
int JpegOMX_SetConfig_Rotate_WRAP(FuzzedArgs *vArgList)
{
	// Below code is generated. Do not modify.
	////DBGPRT(LEVEL_INFO, (char *)"JpegOMX_SetConfig_Rotate_WRAP called");
	int Value0 = *((int*)(vArgList->vArg[0]));
	OMX_U32 Value1 = *((OMX_U32*)(vArgList->vArg[1]));
	OMX_S32 Value2 = *((OMX_S32*)(vArgList->vArg[2]));

	*((int*)(vArgList->vReturnArg)) = JpegOMX_SetConfig_Rotate(Value0, Value1, Value2);

	return RET_SUCCESS;
}
/*==========================================================================*
	Name: int JpegOMX_UseBuffer_WRAP(void *args)

	Input: void *args : Input Arguments

	Return: API return value

	Purpose: Wrapper function that the fuzzer will invoke
*==========================================================================*/
int JpegOMX_UseBuffer_WRAP(FuzzedArgs *vArgList)
{
	// Below code is generated. Do not modify.
	////DBGPRT(LEVEL_INFO, (char *)"JpegOMX_UseBuffer_WRAP called");
	int Value0 = *((int*)(vArgList->vArg[0]));
	OMX_U32 Value1 = *((OMX_U32*)(vArgList->vArg[1]));
	int Value2 = *((int*)(vArgList->vArg[2]));
	unsigned int Value3 = *((unsigned int*)(vArgList->vArg[3]));
	int Value4 = *((int*)(vArgList->vArg[4]));

	*((int*)(vArgList->vReturnArg)) = JpegOMX_UseBuffer(Value0, Value1, Value2, Value3, Value4);

	return RET_SUCCESS;
}
/*==========================================================================*
	Name: int JpegOMX_SetParameter_QFactor_WRAP(void *args)

	Input: void *args : Input Arguments

	Return: API return value

	Purpose: Wrapper function that the fuzzer will invoke
*==========================================================================*/
int JpegOMX_SetParameter_QFactor_WRAP(FuzzedArgs *vArgList)
{
	// Below code is generated. Do not modify.
	////DBGPRT(LEVEL_INFO, (char *)"JpegOMX_SetParameter_QFactor_WRAP called");
	int Value0 = *((int*)(vArgList->vArg[0]));
	OMX_U32 Value1 = *((OMX_U32*)(vArgList->vArg[1]));
	OMX_U32 Value2 = *((OMX_U32*)(vArgList->vArg[2]));
	int Value3 = *((int*)(vArgList->vArg[3]));

	*((int*)(vArgList->vReturnArg)) = JpegOMX_SetParameter_QFactor(Value0, Value1, Value2, Value3);

	return RET_SUCCESS;
}
/*==========================================================================*
	Name: int JpegOMX_GetParameter_WRAP(void *args)

	Input: void *args : Input Arguments

	Return: API return value

	Purpose: Wrapper function that the fuzzer will invoke
*==========================================================================*/
int JpegOMX_GetParameter_WRAP(FuzzedArgs *vArgList)
{
	// Below code is generated. Do not modify.
	////DBGPRT(LEVEL_INFO, (char *)"JpegOMX_GetParameter_WRAP called");
	int Value0 = *((int*)(vArgList->vArg[0]));
	int Value1 = *((int*)(vArgList->vArg[1]));
	int Value2 = *((int*)(vArgList->vArg[2]));

	*((int*)(vArgList->vReturnArg)) = JpegOMX_GetParameter(Value0, Value1, Value2);

	return RET_SUCCESS;
}
/*  DO NOT REMOVE --<UpdateCodeTag_DefineApiWrappers>-- */

/*==========================================================================*
	Name: 
		void InitArg(ArgDefn *vArgDefn)

	Input:
	       ArgDefn   

	Return: 
          
	Purpose:
*==========================================================================*/
void InitArg(ArgDefn *vArgDefn)
{
    vArgDefn->nPosition      = 0;
    vArgDefn->bIsPointer     = false;
    vArgDefn->bIsPrimitive   = false;
    vArgDefn->bIsStructure   = false;
    vArgDefn->bIsArray       = false;
    vArgDefn->bIsUnknownType = false;
    vArgDefn->bIsUserDefined = false;
    vArgDefn->nArraySize     = 0;
    vArgDefn->nMemSize       = 0;
    vArgDefn->vDataType      = DTYPE_FIRST;
    vArgDefn->vDataSpecifier = DSPECIFIER_FIRST;
}

/*==========================================================================*
	Name: int signalWaitFunc(const char *cpCallbFuncName, void *vRetValue, int nRetBytes)

	Input:
     const char *cpCallbFuncName : Callback function name
     void       *vRetValue       : Return value 
     int         nRetBytes       : Memory requirement of return value

	Return: 
     RET_SUCCESS - SUCCESS
     RET_FAILURE - ERROR

	Purpose:
	  This function is a callback function that will be invoked by the user
      fuzz library when it registers the APIs
*==========================================================================*/
int signalWaitFunc(const char *cpCallbFuncName, void *vRetValue, int nRetBytes)
{
    //DBGPRT(LEVEL_INFO, "signalWaitFunc called : Callback function : %s", cpCallbFuncName);
    
    int nRetVal = 0;
    
    if(NULL != g_signalCondVar)
    {
        nRetVal = (*g_signalCondVar)(cpCallbFuncName, vRetValue, nRetBytes);
    }
    else
    {
        //DBGPRT(LEVEL_ERROR, "signalWaitFunc : Function pointer not registered with cFuzzer");
        return RET_FAILURE;
    }
    
    //DBGPRT(LEVEL_INFO, "signalWaitFunc : Return value : %d", nRetVal);
    return nRetVal;    
}


