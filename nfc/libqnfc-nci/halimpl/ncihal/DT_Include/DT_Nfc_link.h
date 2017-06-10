/******************************************************************************

Copyright (c) 2013, The Linux Foundation. All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above
      copyright notice, this list of conditions and the following
      disclaimer in the documentation and/or other materials provided
      with the distribution.
    * Neither the name of The Linux Foundation nor the names of its
      contributors may be used to endorse or promote products derived
      from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
******************************************************************************/

/**
 * \file DT_Nfc_link.h
 * \brief DT generic physical link interface for linux
 *
 *
 */

/**< Basic type definitions */
#include <DT_Nfc_types.h>

#include <DT_Nfc.h>

typedef void                    (*DT_Nfc_link_initialize_CB_t)    (void);
typedef void                    (*DT_Nfc_link_close_CB_t)         (void);
typedef NFC_RETURN_CODE         (*DT_Nfc_link_setup_CB_t)         (pDT_Nfc_sConfig_t pConfig, void **pdTransportHandle);
typedef int                     (*DT_Nfc_link_read_CB_t)          (uint8_t * pStore, int NumToRd);
typedef int                     (*DT_Nfc_link_write_CB_t)         (uint8_t * pStore, int nNbBytesToWrite);
typedef int                     (*DT_Nfc_link_reset_CB_t)         (long state);
typedef int                     (*DT_Nfc_link_version_CB_t)       (long field);
typedef int                     (*DT_Nfc_link_init_ntf_CB_t)      (void);
typedef int                     (*DT_Nfc_link_efuse_CB_t)         (void);
typedef struct
{
DT_Nfc_link_initialize_CB_t    init;
DT_Nfc_link_close_CB_t         close;
DT_Nfc_link_setup_CB_t         setup;
DT_Nfc_link_read_CB_t          rd;
DT_Nfc_link_write_CB_t         wr;
DT_Nfc_link_reset_CB_t         rst;
DT_Nfc_link_version_CB_t       version;
DT_Nfc_link_efuse_CB_t         efuse;
DT_Nfc_link_init_ntf_CB_t      ntf;
} DT_Nfc_Phy_select_t;
