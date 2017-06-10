/*
 * Copyright (c) 2012-2013 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#include <stdio.h>
#include "rfsa_common.h"

/****************************************************************************
* Defines                                                                   *
****************************************************************************/
//#define INCLUDE_APR
#define INCLUDE_QMI

#define APR_LAYER 0
#define QMI_LAYER 1

#ifdef INCLUDE_QMI
#include "rfsa_qmi_server.h"
#endif

#ifdef INCLUDE_APR
#include "rfsa_apr_server.h"
#endif

/****************************************************************************
* Definitions                                                               *
****************************************************************************/

/****************************************************************************
* Variables                                                                 *
****************************************************************************/

//static int vtl_layer = APR_LAYER;
static int vtl_layer = QMI_LAYER;

/****************************************************************************
* Implementations                                                           *
****************************************************************************/

/**
* Initialize the server side
*/
int32_t rfsa_vtl_server_init()
{
	int ret = RFSA_EFAILED;

	switch(vtl_layer) {
#ifdef INCLUDE_APR
	case APR_LAYER:
		ret = rfsa_apr_server_init();
		break;
#endif

#ifdef INCLUDE_QMI
	case QMI_LAYER:
		ret = rfsa_qmi_server_init();
		break;
#endif
	default:
		LOGE("rfsa_vtl_server_init not defined for this protocol\n");
		break;
	}

	return ret;
}

/**
* Deinitialize the server side
*/
int32_t rfsa_vtl_server_deinit()
{
	int ret = RFSA_EFAILED;

	switch(vtl_layer) {
#ifdef INCLUDE_APR
	case APR_LAYER:
		ret = rfsa_apr_server_deinit();
		break;
#endif

#ifdef INCLUDE_QMI
	case QMI_LAYER:
		ret = rfsa_qmi_server_deinit();
		break;
#endif
	default:
		LOGE("rfsa_vtl_server_init not defined for this protocol\n");
		break;
	}

	return ret;
}

int32_t rfsa_vtl_server_response(rfsa_server_work_item_t *item)
{
	int ret = RFSA_EFAILED;

	switch(vtl_layer) {
#ifdef INCLUDE_APR
	case APR_LAYER:
		ret = rfsa_apr_server_response(item);
		break;
#endif

#ifdef INCLUDE_QMI
	case QMI_LAYER:
		ret = rfsa_qmi_server_response(item);
		break;
#endif
	default:
		LOGE("rfsa_vtl_server_response not defined for this protocol\n");
		break;
	}

	return ret;
}
