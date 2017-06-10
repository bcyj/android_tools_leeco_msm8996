/* ========================================================================= * 
   Purpose:  Shared object library used for fuzzing APIs

           -------------------------------------------------------
       Copyright © 2012 Qualcomm Technologies, Inc. All Rights Reserved.
             Qualcomm Technologies Proprietary and Confidential.
* ========================================================================= */
#include <stdio.h>
#include <string.h>
#include "cFuzzerLog.h"

#include "QCamera2HALFuzzLibHelper.h" 
#include "QCamera2HALFuzzLib.h" 
#include "QCamera2HAL.h" 

const char *lib_name="libFuzzQCamera2HALFuzzLib" ;
cFuzzerLog cFL ;

#define NUM_OF_APIS 26 
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
		strlcpy(vAPI.cLibraryName, "libFuzzQCamera2HALFuzzLib.so", MAX_FILE_LEN);
		strlcpy(vAPI.cAPIName, "msg_type_enabled", MAX_API_NAME_LEN);
		vAPI.vFuncPtr     = &msg_type_enabled_WRAP;
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
			//DBGPRT(LEVEL_ERROR, (char *)"Function : msg_type_enabled registration failed!!!");
			return RET_FAILURE;
		}
	}

	{
		APIDefn vAPI;
		strlcpy(vAPI.cLibraryName, "libFuzzQCamera2HALFuzzLib.so", MAX_FILE_LEN);
		strlcpy(vAPI.cAPIName, "cancel_picture", MAX_API_NAME_LEN);
		vAPI.vFuncPtr     = &cancel_picture_WRAP;
		vAPI.nNumArgs     = 0;

		InitArg(&(vAPI.vReturnType));
		vAPI.vReturnType.nMemSize = sizeof(int);
		vAPI.vReturnType.vDataType = DTYPE_INT;
		vAPI.vReturnType.vDataSpecifier = DSPECIFIER_FIRST;
		vAPI.vReturnType.bIsPrimitive = true;
		nRetVal = (*regFunc)(&vAPI);
		if(RET_SUCCESS != nRetVal)
		{
			//DBGPRT(LEVEL_ERROR, (char *)"Function : cancel_picture registration failed!!!");
			return RET_FAILURE;
		}
	}

	{
		APIDefn vAPI;
		strlcpy(vAPI.cLibraryName, "libFuzzQCamera2HALFuzzLib.so", MAX_FILE_LEN);
		strlcpy(vAPI.cAPIName, "Fuzzer_HAL_init", MAX_API_NAME_LEN);
		vAPI.vFuncPtr     = &Fuzzer_HAL_init_WRAP;
		vAPI.nNumArgs     = 0;

		InitArg(&(vAPI.vReturnType));
		vAPI.vReturnType.nMemSize = sizeof(int);
		vAPI.vReturnType.vDataType = DTYPE_INT;
		vAPI.vReturnType.vDataSpecifier = DSPECIFIER_FIRST;
		vAPI.vReturnType.bIsPrimitive = true;
		nRetVal = (*regFunc)(&vAPI);
		if(RET_SUCCESS != nRetVal)
		{
			//DBGPRT(LEVEL_ERROR, (char *)"Function : Fuzzer_HAL_init registration failed!!!");
			return RET_FAILURE;
		}
	}

	{
		APIDefn vAPI;
		strlcpy(vAPI.cLibraryName, "libFuzzQCamera2HALFuzzLib.so", MAX_FILE_LEN);
		strlcpy(vAPI.cAPIName, "auto_focus", MAX_API_NAME_LEN);
		vAPI.vFuncPtr     = &auto_focus_WRAP;
		vAPI.nNumArgs     = 0;

		InitArg(&(vAPI.vReturnType));
		vAPI.vReturnType.nMemSize = sizeof(int);
		vAPI.vReturnType.vDataType = DTYPE_INT;
		vAPI.vReturnType.vDataSpecifier = DSPECIFIER_FIRST;
		vAPI.vReturnType.bIsPrimitive = true;
		nRetVal = (*regFunc)(&vAPI);
		if(RET_SUCCESS != nRetVal)
		{
			//DBGPRT(LEVEL_ERROR, (char *)"Function : auto_focus registration failed!!!");
			return RET_FAILURE;
		}
	}

	{
		APIDefn vAPI;
		strlcpy(vAPI.cLibraryName, "libFuzzQCamera2HALFuzzLib.so", MAX_FILE_LEN);
		strlcpy(vAPI.cAPIName, "cancel_auto_focus", MAX_API_NAME_LEN);
		vAPI.vFuncPtr     = &cancel_auto_focus_WRAP;
		vAPI.nNumArgs     = 0;

		InitArg(&(vAPI.vReturnType));
		vAPI.vReturnType.nMemSize = sizeof(int);
		vAPI.vReturnType.vDataType = DTYPE_INT;
		vAPI.vReturnType.vDataSpecifier = DSPECIFIER_FIRST;
		vAPI.vReturnType.bIsPrimitive = true;
		nRetVal = (*regFunc)(&vAPI);
		if(RET_SUCCESS != nRetVal)
		{
			//DBGPRT(LEVEL_ERROR, (char *)"Function : cancel_auto_focus registration failed!!!");
			return RET_FAILURE;
		}
	}

	{
		APIDefn vAPI;
		strlcpy(vAPI.cLibraryName, "libFuzzQCamera2HALFuzzLib.so", MAX_FILE_LEN);
		strlcpy(vAPI.cAPIName, "release", MAX_API_NAME_LEN);
		vAPI.vFuncPtr     = &release_WRAP;
		vAPI.nNumArgs     = 0;

		InitArg(&(vAPI.vReturnType));
		vAPI.vReturnType.nMemSize = sizeof(int);
		vAPI.vReturnType.vDataType = DTYPE_INT;
		vAPI.vReturnType.vDataSpecifier = DSPECIFIER_FIRST;
		vAPI.vReturnType.bIsPrimitive = true;
		nRetVal = (*regFunc)(&vAPI);
		if(RET_SUCCESS != nRetVal)
		{
			//DBGPRT(LEVEL_ERROR, (char *)"Function : release registration failed!!!");
			return RET_FAILURE;
		}
	}

	{
		APIDefn vAPI;
		strlcpy(vAPI.cLibraryName, "libFuzzQCamera2HALFuzzLib.so", MAX_FILE_LEN);
		strlcpy(vAPI.cAPIName, "set_parameters", MAX_API_NAME_LEN);
		vAPI.vFuncPtr     = &set_parameters_WRAP;
		vAPI.nNumArgs     = 2;

		InitArg(&(vAPI.vArgDefn[0]));
		vAPI.vArgDefn[0].nPosition = 0;
		vAPI.vArgDefn[0].nMemSize = sizeof(char);
		vAPI.vArgDefn[0].bIsPointer = true;
		vAPI.vArgDefn[0].vDataType = DTYPE_CHAR;
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
			//DBGPRT(LEVEL_ERROR, (char *)"Function : set_parameters registration failed!!!");
			return RET_FAILURE;
		}
	}

	{
		APIDefn vAPI;
		strlcpy(vAPI.cLibraryName, "libFuzzQCamera2HALFuzzLib.so", MAX_FILE_LEN);
		strlcpy(vAPI.cAPIName, "preview_enabled", MAX_API_NAME_LEN);
		vAPI.vFuncPtr     = &preview_enabled_WRAP;
		vAPI.nNumArgs     = 0;

		InitArg(&(vAPI.vReturnType));
		vAPI.vReturnType.nMemSize = sizeof(int);
		vAPI.vReturnType.vDataType = DTYPE_INT;
		vAPI.vReturnType.vDataSpecifier = DSPECIFIER_FIRST;
		vAPI.vReturnType.bIsPrimitive = true;
		nRetVal = (*regFunc)(&vAPI);
		if(RET_SUCCESS != nRetVal)
		{
			//DBGPRT(LEVEL_ERROR, (char *)"Function : preview_enabled registration failed!!!");
			return RET_FAILURE;
		}
	}

	{
		APIDefn vAPI;
		strlcpy(vAPI.cLibraryName, "libFuzzQCamera2HALFuzzLib.so", MAX_FILE_LEN);
		strlcpy(vAPI.cAPIName, "enable_msg_type", MAX_API_NAME_LEN);
		vAPI.vFuncPtr     = &enable_msg_type_WRAP;
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
			//DBGPRT(LEVEL_ERROR, (char *)"Function : enable_msg_type registration failed!!!");
			return RET_FAILURE;
		}
	}

	{
		APIDefn vAPI;
		strlcpy(vAPI.cLibraryName, "libFuzzQCamera2HALFuzzLib.so", MAX_FILE_LEN);
		strlcpy(vAPI.cAPIName, "store_meta_data_in_buffers", MAX_API_NAME_LEN);
		vAPI.vFuncPtr     = &store_meta_data_in_buffers_WRAP;
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
			//DBGPRT(LEVEL_ERROR, (char *)"Function : store_meta_data_in_buffers registration failed!!!");
			return RET_FAILURE;
		}
	}

	{
		APIDefn vAPI;
		strlcpy(vAPI.cLibraryName, "libFuzzQCamera2HALFuzzLib.so", MAX_FILE_LEN);
		strlcpy(vAPI.cAPIName, "set_preview_window", MAX_API_NAME_LEN);
		vAPI.vFuncPtr     = &set_preview_window_WRAP;
		vAPI.nNumArgs     = 0;

		InitArg(&(vAPI.vReturnType));
		vAPI.vReturnType.nMemSize = sizeof(int);
		vAPI.vReturnType.vDataType = DTYPE_INT;
		vAPI.vReturnType.vDataSpecifier = DSPECIFIER_FIRST;
		vAPI.vReturnType.bIsPrimitive = true;
		nRetVal = (*regFunc)(&vAPI);
		if(RET_SUCCESS != nRetVal)
		{
			//DBGPRT(LEVEL_ERROR, (char *)"Function : set_preview_window registration failed!!!");
			return RET_FAILURE;
		}
	}

	{
		APIDefn vAPI;
		strlcpy(vAPI.cLibraryName, "libFuzzQCamera2HALFuzzLib.so", MAX_FILE_LEN);
		strlcpy(vAPI.cAPIName, "put_parameters", MAX_API_NAME_LEN);
		vAPI.vFuncPtr     = &put_parameters_WRAP;
		vAPI.nNumArgs     = 1;

		InitArg(&(vAPI.vArgDefn[0]));
		vAPI.vArgDefn[0].nPosition = 0;
		vAPI.vArgDefn[0].nMemSize = sizeof(char);
		vAPI.vArgDefn[0].bIsPointer = true;
		vAPI.vArgDefn[0].vDataType = DTYPE_CHAR;
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
			//DBGPRT(LEVEL_ERROR, (char *)"Function : put_parameters registration failed!!!");
			return RET_FAILURE;
		}
	}

	{
		APIDefn vAPI;
		strlcpy(vAPI.cLibraryName, "libFuzzQCamera2HALFuzzLib.so", MAX_FILE_LEN);
		strlcpy(vAPI.cAPIName, "disable_msg_type", MAX_API_NAME_LEN);
		vAPI.vFuncPtr     = &disable_msg_type_WRAP;
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
			//DBGPRT(LEVEL_ERROR, (char *)"Function : disable_msg_type registration failed!!!");
			return RET_FAILURE;
		}
	}

	{
		APIDefn vAPI;
		strlcpy(vAPI.cLibraryName, "libFuzzQCamera2HALFuzzLib.so", MAX_FILE_LEN);
		strlcpy(vAPI.cAPIName, "start_preview", MAX_API_NAME_LEN);
		vAPI.vFuncPtr     = &start_preview_WRAP;
		vAPI.nNumArgs     = 0;

		InitArg(&(vAPI.vReturnType));
		vAPI.vReturnType.nMemSize = sizeof(int);
		vAPI.vReturnType.vDataType = DTYPE_INT;
		vAPI.vReturnType.vDataSpecifier = DSPECIFIER_FIRST;
		vAPI.vReturnType.bIsPrimitive = true;
		nRetVal = (*regFunc)(&vAPI);
		if(RET_SUCCESS != nRetVal)
		{
			//DBGPRT(LEVEL_ERROR, (char *)"Function : start_preview registration failed!!!");
			return RET_FAILURE;
		}
	}

	{
		APIDefn vAPI;
		strlcpy(vAPI.cLibraryName, "libFuzzQCamera2HALFuzzLib.so", MAX_FILE_LEN);
		strlcpy(vAPI.cAPIName, "get_parameters", MAX_API_NAME_LEN);
		vAPI.vFuncPtr     = &get_parameters_WRAP;
		vAPI.nNumArgs     = 0;

		InitArg(&(vAPI.vReturnType));
		vAPI.vReturnType.nMemSize = sizeof(char);
		vAPI.vReturnType.bIsPointer = true;
		vAPI.vReturnType.vDataType = DTYPE_CHAR;
		vAPI.vReturnType.vDataSpecifier = DSPECIFIER_FIRST;
		vAPI.vReturnType.bIsPrimitive = true;
		nRetVal = (*regFunc)(&vAPI);
		if(RET_SUCCESS != nRetVal)
		{
			//DBGPRT(LEVEL_ERROR, (char *)"Function : get_parameters registration failed!!!");
			return RET_FAILURE;
		}
	}

	{
		APIDefn vAPI;
		strlcpy(vAPI.cLibraryName, "libFuzzQCamera2HALFuzzLib.so", MAX_FILE_LEN);
		strlcpy(vAPI.cAPIName, "camera_device_open", MAX_API_NAME_LEN);
		vAPI.vFuncPtr     = &camera_device_open_WRAP;
		vAPI.nNumArgs     = 1;

		InitArg(&(vAPI.vArgDefn[0]));
		vAPI.vArgDefn[0].nPosition = 0;
		vAPI.vArgDefn[0].nMemSize = sizeof(char);
		vAPI.vArgDefn[0].bIsPointer = true;
		vAPI.vArgDefn[0].vDataType = DTYPE_CHAR;
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
			//DBGPRT(LEVEL_ERROR, (char *)"Function : camera_device_open registration failed!!!");
			return RET_FAILURE;
		}
	}

	{
		APIDefn vAPI;
		strlcpy(vAPI.cLibraryName, "libFuzzQCamera2HALFuzzLib.so", MAX_FILE_LEN);
		strlcpy(vAPI.cAPIName, "set_callbacks", MAX_API_NAME_LEN);
		vAPI.vFuncPtr     = &set_callbacks_WRAP;
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
			//DBGPRT(LEVEL_ERROR, (char *)"Function : set_callbacks registration failed!!!");
			return RET_FAILURE;
		}
	}

	{
		APIDefn vAPI;
		strlcpy(vAPI.cLibraryName, "libFuzzQCamera2HALFuzzLib.so", MAX_FILE_LEN);
		strlcpy(vAPI.cAPIName, "stop_recording", MAX_API_NAME_LEN);
		vAPI.vFuncPtr     = &stop_recording_WRAP;
		vAPI.nNumArgs     = 0;

		InitArg(&(vAPI.vReturnType));
		vAPI.vReturnType.nMemSize = sizeof(int);
		vAPI.vReturnType.vDataType = DTYPE_INT;
		vAPI.vReturnType.vDataSpecifier = DSPECIFIER_FIRST;
		vAPI.vReturnType.bIsPrimitive = true;
		nRetVal = (*regFunc)(&vAPI);
		if(RET_SUCCESS != nRetVal)
		{
			//DBGPRT(LEVEL_ERROR, (char *)"Function : stop_recording registration failed!!!");
			return RET_FAILURE;
		}
	}

	{
		APIDefn vAPI;
		strlcpy(vAPI.cLibraryName, "libFuzzQCamera2HALFuzzLib.so", MAX_FILE_LEN);
		strlcpy(vAPI.cAPIName, "stop_preview", MAX_API_NAME_LEN);
		vAPI.vFuncPtr     = &stop_preview_WRAP;
		vAPI.nNumArgs     = 0;

		InitArg(&(vAPI.vReturnType));
		vAPI.vReturnType.nMemSize = sizeof(int);
		vAPI.vReturnType.vDataType = DTYPE_INT;
		vAPI.vReturnType.vDataSpecifier = DSPECIFIER_FIRST;
		vAPI.vReturnType.bIsPrimitive = true;
		nRetVal = (*regFunc)(&vAPI);
		if(RET_SUCCESS != nRetVal)
		{
			//DBGPRT(LEVEL_ERROR, (char *)"Function : stop_preview registration failed!!!");
			return RET_FAILURE;
		}
	}

	{
		APIDefn vAPI;
		strlcpy(vAPI.cLibraryName, "libFuzzQCamera2HALFuzzLib.so", MAX_FILE_LEN);
		strlcpy(vAPI.cAPIName, "take_picture", MAX_API_NAME_LEN);
		vAPI.vFuncPtr     = &take_picture_WRAP;
		vAPI.nNumArgs     = 0;

		InitArg(&(vAPI.vReturnType));
		vAPI.vReturnType.nMemSize = sizeof(int);
		vAPI.vReturnType.vDataType = DTYPE_INT;
		vAPI.vReturnType.vDataSpecifier = DSPECIFIER_FIRST;
		vAPI.vReturnType.bIsPrimitive = true;
		nRetVal = (*regFunc)(&vAPI);
		if(RET_SUCCESS != nRetVal)
		{
			//DBGPRT(LEVEL_ERROR, (char *)"Function : take_picture registration failed!!!");
			return RET_FAILURE;
		}
	}

	{
		APIDefn vAPI;
		strlcpy(vAPI.cLibraryName, "libFuzzQCamera2HALFuzzLib.so", MAX_FILE_LEN);
		strlcpy(vAPI.cAPIName, "send_command", MAX_API_NAME_LEN);
		vAPI.vFuncPtr     = &send_command_WRAP;
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
			//DBGPRT(LEVEL_ERROR, (char *)"Function : send_command registration failed!!!");
			return RET_FAILURE;
		}
	}

	{
		APIDefn vAPI;
		strlcpy(vAPI.cLibraryName, "libFuzzQCamera2HALFuzzLib.so", MAX_FILE_LEN);
		strlcpy(vAPI.cAPIName, "close_camera_device", MAX_API_NAME_LEN);
		vAPI.vFuncPtr     = &close_camera_device_WRAP;
		vAPI.nNumArgs     = 0;

		InitArg(&(vAPI.vReturnType));
		vAPI.vReturnType.nMemSize = sizeof(int);
		vAPI.vReturnType.vDataType = DTYPE_INT;
		vAPI.vReturnType.vDataSpecifier = DSPECIFIER_FIRST;
		vAPI.vReturnType.bIsPrimitive = true;
		nRetVal = (*regFunc)(&vAPI);
		if(RET_SUCCESS != nRetVal)
		{
			//DBGPRT(LEVEL_ERROR, (char *)"Function : close_camera_device registration failed!!!");
			return RET_FAILURE;
		}
	}

	{
		APIDefn vAPI;
		strlcpy(vAPI.cLibraryName, "libFuzzQCamera2HALFuzzLib.so", MAX_FILE_LEN);
		strlcpy(vAPI.cAPIName, "Fuzzer_HAL_deinit", MAX_API_NAME_LEN);
		vAPI.vFuncPtr     = &Fuzzer_HAL_deinit_WRAP;
		vAPI.nNumArgs     = 0;

		InitArg(&(vAPI.vReturnType));
		vAPI.vReturnType.nMemSize = sizeof(int);
		vAPI.vReturnType.vDataType = DTYPE_INT;
		vAPI.vReturnType.vDataSpecifier = DSPECIFIER_FIRST;
		vAPI.vReturnType.bIsPrimitive = true;
		nRetVal = (*regFunc)(&vAPI);
		if(RET_SUCCESS != nRetVal)
		{
			//DBGPRT(LEVEL_ERROR, (char *)"Function : Fuzzer_HAL_deinit registration failed!!!");
			return RET_FAILURE;
		}
	}

	{
		APIDefn vAPI;
		strlcpy(vAPI.cLibraryName, "libFuzzQCamera2HALFuzzLib.so", MAX_FILE_LEN);
		strlcpy(vAPI.cAPIName, "start_recording", MAX_API_NAME_LEN);
		vAPI.vFuncPtr     = &start_recording_WRAP;
		vAPI.nNumArgs     = 0;

		InitArg(&(vAPI.vReturnType));
		vAPI.vReturnType.nMemSize = sizeof(int);
		vAPI.vReturnType.vDataType = DTYPE_INT;
		vAPI.vReturnType.vDataSpecifier = DSPECIFIER_FIRST;
		vAPI.vReturnType.bIsPrimitive = true;
		nRetVal = (*regFunc)(&vAPI);
		if(RET_SUCCESS != nRetVal)
		{
			//DBGPRT(LEVEL_ERROR, (char *)"Function : start_recording registration failed!!!");
			return RET_FAILURE;
		}
	}

	{
		APIDefn vAPI;
		strlcpy(vAPI.cLibraryName, "libFuzzQCamera2HALFuzzLib.so", MAX_FILE_LEN);
		strlcpy(vAPI.cAPIName, "release_recording_frame", MAX_API_NAME_LEN);
		vAPI.vFuncPtr     = &release_recording_frame_WRAP;
		vAPI.nNumArgs     = 1;

		InitArg(&(vAPI.vArgDefn[0]));
		vAPI.vArgDefn[0].nPosition = 0;
		vAPI.vArgDefn[0].nMemSize = sizeof(void);
		vAPI.vArgDefn[0].bIsPointer = true;
		vAPI.vArgDefn[0].vDataType = DTYPE_VOID;
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
			//DBGPRT(LEVEL_ERROR, (char *)"Function : release_recording_frame registration failed!!!");
			return RET_FAILURE;
		}
	}

	{
		APIDefn vAPI;
		strlcpy(vAPI.cLibraryName, "libFuzzQCamera2HALFuzzLib.so", MAX_FILE_LEN);
		strlcpy(vAPI.cAPIName, "recording_enabled", MAX_API_NAME_LEN);
		vAPI.vFuncPtr     = &recording_enabled_WRAP;
		vAPI.nNumArgs     = 0;

		InitArg(&(vAPI.vReturnType));
		vAPI.vReturnType.nMemSize = sizeof(int);
		vAPI.vReturnType.vDataType = DTYPE_INT;
		vAPI.vReturnType.vDataSpecifier = DSPECIFIER_FIRST;
		vAPI.vReturnType.bIsPrimitive = true;
		nRetVal = (*regFunc)(&vAPI);
		if(RET_SUCCESS != nRetVal)
		{
			//DBGPRT(LEVEL_ERROR, (char *)"Function : recording_enabled registration failed!!!");
			return RET_FAILURE;
		}
	}

	/*  DO NOT REMOVE --<UpdateCodeTag_PopulateRegisterApis>-- */ 
	        
    return RET_SUCCESS;
}

/*==========================================================================*
	Name: int msg_type_enabled_WRAP(void *args)

	Input: void *args : Input Arguments

	Return: API return value

	Purpose: Wrapper function that the fuzzer will invoke
*==========================================================================*/
int msg_type_enabled_WRAP(FuzzedArgs *vArgList)
{
	// Below code is generated. Do not modify.
	//DBGPRT(LEVEL_INFO, (char *)"msg_type_enabled_WRAP called");
	int Value0 = *((int*)(vArgList->vArg[0]));

	*((int*)(vArgList->vReturnArg)) = msg_type_enabled(Value0);

	return RET_SUCCESS;
}
/*==========================================================================*
	Name: int cancel_picture_WRAP(void *args)

	Input: void *args : Input Arguments

	Return: API return value

	Purpose: Wrapper function that the fuzzer will invoke
*==========================================================================*/
int cancel_picture_WRAP(FuzzedArgs *vArgList)
{
	// Below code is generated. Do not modify.
	//DBGPRT(LEVEL_INFO, (char *)"cancel_picture_WRAP called");
	*((int*)(vArgList->vReturnArg)) = cancel_picture();

	return RET_SUCCESS;
}
/*==========================================================================*
	Name: int Fuzzer_HAL_init_WRAP(void *args)

	Input: void *args : Input Arguments

	Return: API return value

	Purpose: Wrapper function that the fuzzer will invoke
*==========================================================================*/
int Fuzzer_HAL_init_WRAP(FuzzedArgs *vArgList)
{
	// Below code is generated. Do not modify.
	//DBGPRT(LEVEL_INFO, (char *)"Fuzzer_HAL_init_WRAP called");
	*((int*)(vArgList->vReturnArg)) = Fuzzer_HAL_init();

	return RET_SUCCESS;
}
/*==========================================================================*
	Name: int auto_focus_WRAP(void *args)

	Input: void *args : Input Arguments

	Return: API return value

	Purpose: Wrapper function that the fuzzer will invoke
*==========================================================================*/
int auto_focus_WRAP(FuzzedArgs *vArgList)
{
	// Below code is generated. Do not modify.
	//DBGPRT(LEVEL_INFO, (char *)"auto_focus_WRAP called");
	*((int*)(vArgList->vReturnArg)) = auto_focus();

	return RET_SUCCESS;
}
/*==========================================================================*
	Name: int cancel_auto_focus_WRAP(void *args)

	Input: void *args : Input Arguments

	Return: API return value

	Purpose: Wrapper function that the fuzzer will invoke
*==========================================================================*/
int cancel_auto_focus_WRAP(FuzzedArgs *vArgList)
{
	// Below code is generated. Do not modify.
	//DBGPRT(LEVEL_INFO, (char *)"cancel_auto_focus_WRAP called");
	*((int*)(vArgList->vReturnArg)) = cancel_auto_focus();

	return RET_SUCCESS;
}
/*==========================================================================*
	Name: int release_WRAP(void *args)

	Input: void *args : Input Arguments

	Return: API return value

	Purpose: Wrapper function that the fuzzer will invoke
*==========================================================================*/
int release_WRAP(FuzzedArgs *vArgList)
{
	// Below code is generated. Do not modify.
	//DBGPRT(LEVEL_INFO, (char *)"release_WRAP called");
	*((int*)(vArgList->vReturnArg)) = release();

	return RET_SUCCESS;
}
/*==========================================================================*
	Name: int set_parameters_WRAP(void *args)

	Input: void *args : Input Arguments

	Return: API return value

	Purpose: Wrapper function that the fuzzer will invoke
*==========================================================================*/
int set_parameters_WRAP(FuzzedArgs *vArgList)
{
	// Below code is generated. Do not modify.
	//DBGPRT(LEVEL_INFO, (char *)"set_parameters_WRAP called");
	const char *Value0 = (const char*)(vArgList->vArg[0]);
	int Value1 = *((int*)(vArgList->vArg[1]));

	*((int*)(vArgList->vReturnArg)) = set_parameters(Value0, Value1);

	return RET_SUCCESS;
}
/*==========================================================================*
	Name: int preview_enabled_WRAP(void *args)

	Input: void *args : Input Arguments

	Return: API return value

	Purpose: Wrapper function that the fuzzer will invoke
*==========================================================================*/
int preview_enabled_WRAP(FuzzedArgs *vArgList)
{
	// Below code is generated. Do not modify.
	//DBGPRT(LEVEL_INFO, (char *)"preview_enabled_WRAP called");
	*((int*)(vArgList->vReturnArg)) = preview_enabled();

	return RET_SUCCESS;
}
/*==========================================================================*
	Name: int enable_msg_type_WRAP(void *args)

	Input: void *args : Input Arguments

	Return: API return value

	Purpose: Wrapper function that the fuzzer will invoke
*==========================================================================*/
int enable_msg_type_WRAP(FuzzedArgs *vArgList)
{
	// Below code is generated. Do not modify.
	//DBGPRT(LEVEL_INFO, (char *)"enable_msg_type_WRAP called");
	int Value0 = *((int*)(vArgList->vArg[0]));

	*((int*)(vArgList->vReturnArg)) = enable_msg_type(Value0);

	return RET_SUCCESS;
}
/*==========================================================================*
	Name: int store_meta_data_in_buffers_WRAP(void *args)

	Input: void *args : Input Arguments

	Return: API return value

	Purpose: Wrapper function that the fuzzer will invoke
*==========================================================================*/
int store_meta_data_in_buffers_WRAP(FuzzedArgs *vArgList)
{
	// Below code is generated. Do not modify.
	//DBGPRT(LEVEL_INFO, (char *)"store_meta_data_in_buffers_WRAP called");
	int Value0 = *((int*)(vArgList->vArg[0]));

	*((int*)(vArgList->vReturnArg)) = store_meta_data_in_buffers(Value0);

	return RET_SUCCESS;
}
/*==========================================================================*
	Name: int set_preview_window_WRAP(void *args)

	Input: void *args : Input Arguments

	Return: API return value

	Purpose: Wrapper function that the fuzzer will invoke
*==========================================================================*/
int set_preview_window_WRAP(FuzzedArgs *vArgList)
{
	// Below code is generated. Do not modify.
	//DBGPRT(LEVEL_INFO, (char *)"set_preview_window_WRAP called");
	*((int*)(vArgList->vReturnArg)) = set_preview_window();

	return RET_SUCCESS;
}
/*==========================================================================*
	Name: int put_parameters_WRAP(void *args)

	Input: void *args : Input Arguments

	Return: API return value

	Purpose: Wrapper function that the fuzzer will invoke
*==========================================================================*/
int put_parameters_WRAP(FuzzedArgs *vArgList)
{
	// Below code is generated. Do not modify.
	//DBGPRT(LEVEL_INFO, (char *)"put_parameters_WRAP called");
	char *Value0 = (char*)(vArgList->vArg[0]);

	*((int*)(vArgList->vReturnArg)) = put_parameters(Value0);

	return RET_SUCCESS;
}
/*==========================================================================*
	Name: int disable_msg_type_WRAP(void *args)

	Input: void *args : Input Arguments

	Return: API return value

	Purpose: Wrapper function that the fuzzer will invoke
*==========================================================================*/
int disable_msg_type_WRAP(FuzzedArgs *vArgList)
{
	// Below code is generated. Do not modify.
	//DBGPRT(LEVEL_INFO, (char *)"disable_msg_type_WRAP called");
	int Value0 = *((int*)(vArgList->vArg[0]));

	*((int*)(vArgList->vReturnArg)) = disable_msg_type(Value0);

	return RET_SUCCESS;
}
/*==========================================================================*
	Name: int start_preview_WRAP(void *args)

	Input: void *args : Input Arguments

	Return: API return value

	Purpose: Wrapper function that the fuzzer will invoke
*==========================================================================*/
int start_preview_WRAP(FuzzedArgs *vArgList)
{
	// Below code is generated. Do not modify.
	//DBGPRT(LEVEL_INFO, (char *)"start_preview_WRAP called");
	*((int*)(vArgList->vReturnArg)) = start_preview();

	return RET_SUCCESS;
}
/*==========================================================================*
	Name: int get_parameters_WRAP(void *args)

	Input: void *args : Input Arguments

	Return: API return value

	Purpose: Wrapper function that the fuzzer will invoke
*==========================================================================*/
int get_parameters_WRAP(FuzzedArgs *vArgList)
{
	// Below code is generated. Do not modify.
	//DBGPRT(LEVEL_INFO, (char *)"get_parameters_WRAP called");
	vArgList->vReturnArg = (void *)get_parameters();

	return RET_SUCCESS;
}
/*==========================================================================*
	Name: int camera_device_open_WRAP(void *args)

	Input: void *args : Input Arguments

	Return: API return value

	Purpose: Wrapper function that the fuzzer will invoke
*==========================================================================*/
int camera_device_open_WRAP(FuzzedArgs *vArgList)
{
	// Below code is generated. Do not modify.
	//DBGPRT(LEVEL_INFO, (char *)"camera_device_open_WRAP called");
	const char *Value0 = (const char*)(vArgList->vArg[0]);

	*((int*)(vArgList->vReturnArg)) = camera_device_open(Value0);

	return RET_SUCCESS;
}
/*==========================================================================*
	Name: int set_callbacks_WRAP(void *args)

	Input: void *args : Input Arguments

	Return: API return value

	Purpose: Wrapper function that the fuzzer will invoke
*==========================================================================*/
int set_callbacks_WRAP(FuzzedArgs *vArgList)
{
	// Below code is generated. Do not modify.
	//DBGPRT(LEVEL_INFO, (char *)"set_callbacks_WRAP called");
	int Value0 = *((int*)(vArgList->vArg[0]));

	*((int*)(vArgList->vReturnArg)) = set_callbacks(Value0);

	return RET_SUCCESS;
}
/*==========================================================================*
	Name: int stop_recording_WRAP(void *args)

	Input: void *args : Input Arguments

	Return: API return value

	Purpose: Wrapper function that the fuzzer will invoke
*==========================================================================*/
int stop_recording_WRAP(FuzzedArgs *vArgList)
{
	// Below code is generated. Do not modify.
	//DBGPRT(LEVEL_INFO, (char *)"stop_recording_WRAP called");
	*((int*)(vArgList->vReturnArg)) = stop_recording();

	return RET_SUCCESS;
}
/*==========================================================================*
	Name: int stop_preview_WRAP(void *args)

	Input: void *args : Input Arguments

	Return: API return value

	Purpose: Wrapper function that the fuzzer will invoke
*==========================================================================*/
int stop_preview_WRAP(FuzzedArgs *vArgList)
{
	// Below code is generated. Do not modify.
	//DBGPRT(LEVEL_INFO, (char *)"stop_preview_WRAP called");
	*((int*)(vArgList->vReturnArg)) = stop_preview();

	return RET_SUCCESS;
}
/*==========================================================================*
	Name: int take_picture_WRAP(void *args)

	Input: void *args : Input Arguments

	Return: API return value

	Purpose: Wrapper function that the fuzzer will invoke
*==========================================================================*/
int take_picture_WRAP(FuzzedArgs *vArgList)
{
	// Below code is generated. Do not modify.
	//DBGPRT(LEVEL_INFO, (char *)"take_picture_WRAP called");
	*((int*)(vArgList->vReturnArg)) = take_picture();

	return RET_SUCCESS;
}
/*==========================================================================*
	Name: int send_command_WRAP(void *args)

	Input: void *args : Input Arguments

	Return: API return value

	Purpose: Wrapper function that the fuzzer will invoke
*==========================================================================*/
int send_command_WRAP(FuzzedArgs *vArgList)
{
	// Below code is generated. Do not modify.
	//DBGPRT(LEVEL_INFO, (char *)"send_command_WRAP called");
	int Value0 = *((int*)(vArgList->vArg[0]));
	int Value1 = *((int*)(vArgList->vArg[1]));
	int Value2 = *((int*)(vArgList->vArg[2]));

	*((int*)(vArgList->vReturnArg)) = send_command(Value0, Value1, Value2);

	return RET_SUCCESS;
}
/*==========================================================================*
	Name: int close_camera_device_WRAP(void *args)

	Input: void *args : Input Arguments

	Return: API return value

	Purpose: Wrapper function that the fuzzer will invoke
*==========================================================================*/
int close_camera_device_WRAP(FuzzedArgs *vArgList)
{
	// Below code is generated. Do not modify.
	//DBGPRT(LEVEL_INFO, (char *)"close_camera_device_WRAP called");
	*((int*)(vArgList->vReturnArg)) = close_camera_device();

	return RET_SUCCESS;
}
/*==========================================================================*
	Name: int Fuzzer_HAL_deinit_WRAP(void *args)

	Input: void *args : Input Arguments

	Return: API return value

	Purpose: Wrapper function that the fuzzer will invoke
*==========================================================================*/
int Fuzzer_HAL_deinit_WRAP(FuzzedArgs *vArgList)
{
	// Below code is generated. Do not modify.
	//DBGPRT(LEVEL_INFO, (char *)"Fuzzer_HAL_deinit_WRAP called");
	*((int*)(vArgList->vReturnArg)) = Fuzzer_HAL_deinit();

	return RET_SUCCESS;
}
/*==========================================================================*
	Name: int start_recording_WRAP(void *args)

	Input: void *args : Input Arguments

	Return: API return value

	Purpose: Wrapper function that the fuzzer will invoke
*==========================================================================*/
int start_recording_WRAP(FuzzedArgs *vArgList)
{
	// Below code is generated. Do not modify.
	//DBGPRT(LEVEL_INFO, (char *)"start_recording_WRAP called");
	*((int*)(vArgList->vReturnArg)) = start_recording();

	return RET_SUCCESS;
}
/*==========================================================================*
	Name: int release_recording_frame_WRAP(void *args)

	Input: void *args : Input Arguments

	Return: API return value

	Purpose: Wrapper function that the fuzzer will invoke
*==========================================================================*/
int release_recording_frame_WRAP(FuzzedArgs *vArgList)
{
	// Below code is generated. Do not modify.
	//DBGPRT(LEVEL_INFO, (char *)"release_recording_frame_WRAP called");
	const void *Value0 = (const void*)(vArgList->vArg[0]);

	*((int*)(vArgList->vReturnArg)) = release_recording_frame(Value0);

	return RET_SUCCESS;
}
/*==========================================================================*
	Name: int recording_enabled_WRAP(void *args)

	Input: void *args : Input Arguments

	Return: API return value

	Purpose: Wrapper function that the fuzzer will invoke
*==========================================================================*/
int recording_enabled_WRAP(FuzzedArgs *vArgList)
{
	// Below code is generated. Do not modify.
	//DBGPRT(LEVEL_INFO, (char *)"recording_enabled_WRAP called");
	*((int*)(vArgList->vReturnArg)) = recording_enabled();

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


