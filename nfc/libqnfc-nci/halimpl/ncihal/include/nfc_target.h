/******************************************************************************
* Copyright (c) 2013, The Linux Foundation. All rights reserved.
* Not a Contribution.
 ******************************************************************************/
/******************************************************************************
 *
 *  Copyright (C) 2001-2012 Broadcom Corporation
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at:
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 ******************************************************************************/
#pragma once
#include "nfc_hal_target.h"

/* the maximum number of NFCEE interface supported */
#ifndef NFC_MAX_EE_INTERFACE
#define NFC_MAX_EE_INTERFACE        4
#endif

/* the maximum number of NFCEE information supported. */
#ifndef NFC_MAX_EE_INFO
#define NFC_MAX_EE_INFO        8
#endif

/* the maximum number of NFCEE TLVs supported */
#ifndef NFC_MAX_EE_TLVS
#define NFC_MAX_EE_TLVS        1
#endif

/* the maximum size of NFCEE TLV list supported */
#ifndef NFC_MAX_EE_TLV_SIZE
#define NFC_MAX_EE_TLV_SIZE        150
#endif

/* Number of times reader/writer should attempt to resend a command on failure */
#ifndef RW_MAX_RETRIES
#define RW_MAX_RETRIES              5
#endif

/* Define to TRUE to include not openned Broadcom Vendor Specific implementation */

/* API macros for DLL (needed to export API functions from DLLs) */
#define NFC_API         EXPORT_API
#define LLCP_API        EXPORT_API

/* Max length of service name */
#ifndef LLCP_MAX_SN_LEN
#define LLCP_MAX_SN_LEN             255     /* max length of service name */
#endif
