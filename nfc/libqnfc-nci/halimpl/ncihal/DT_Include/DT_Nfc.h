/*
 * Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

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

/*!

* \file  DT_Nfc.h
* \brief Common DT for the upper layer.
*
* Project: NFC-NCI
*
*
*/

#ifndef DT_NFC_H
#define DT_NFC_H

/*@{*/
#define DT_NFC_FILEREVISION "$Revision: #1 $" /**< \ingroup grp_file_attributes */
#define DT_NFC_FILEALIASES    "$Aliases: NFC_NCI"
 /**< \ingroup grp_file_attributes */
/*@}*/
/*************************** Includes *******************************/
/** \defgroup grp_nfc_DT DT Component
 *
 *
 *
 */

/*********************** End of includes ****************************/

/***************************** Macros *******************************/
 /**< Used for messaging by DT as well as Upper Layers */

#define UNSOLICITED (0)
#define SOLICITED (1)

/************************ End of macros *****************************/


/********************* Structures and enums *************************/

typedef enum
{
   ENUM_LINK_TYPE_NONE,
   ENUM_LINK_TYPE_UART,
   ENUM_LINK_TYPE_COM1 = ENUM_LINK_TYPE_UART,
   ENUM_LINK_TYPE_COM2,
   ENUM_LINK_TYPE_COM3,
   ENUM_LINK_TYPE_COM4,
   ENUM_LINK_TYPE_COM5,
   ENUM_LINK_TYPE_COM6,
   ENUM_LINK_TYPE_COM7,
   ENUM_LINK_TYPE_COM8,
   ENUM_LINK_TYPE_I2C,
   ENUM_LINK_TYPE_USB,
   ENUM_LINK_TYPE_TCP,
   ENUM_LINK_TYPE_NB,
   ENUM_LINK_TYPE_FILE,

} eDT_PhyLinkType;

/* Interface to device file and associated physical interface */
typedef struct nfc_sConfig_t
{
   /* Device node of the controller */
    const char      *devFile;
    /* Physical ink to NFCC */
    eDT_PhyLinkType phyType;
    /* The Client ID */
    unsigned int    nRef;

} DT_Nfc_Config_t;


typedef DT_Nfc_Config_t     DT_Nfc_sConfig_t;
typedef DT_Nfc_Config_t     *pDT_Nfc_sConfig_t;



/******************** Function declarations *************************/

extern  uint16_t        DT_Nfc_Write(uint8_t port, uint8_t *pWriteData, uint16_t iBytesToWrite);
extern  NFC_RETURN_CODE DT_Nfc_TestWrite(uint8_t *WriteData, uint16_t length);
extern  uint16_t        DT_Nfc_Read(uint8_t port, uint8_t *pReadData, uint16_t iBytesToRead);
extern  uint16_t        DT_Nfc_TestRead(uint8_t *pReadData, uint16_t iBytesToRead);
extern  NFC_RETURN_CODE DT_Nfc_Open(DT_Nfc_sConfig_t *pDriverConfig, int *pdTransportHandle, void *nci_cb);
extern  NFC_RETURN_CODE DT_Nfc_TestOpen(DT_Nfc_sConfig_t *pDriverConfig, void **pdTransportHandle);
extern  void            DT_Nfc_Close(DT_Nfc_sConfig_t *pDriverConfig);
extern  void            DT_Nfc_TestClose(DT_Nfc_sConfig_t *pDriverConfig);
extern  NFC_RETURN_CODE DT_Nfc_ControllerMode(long ReaderMode);
extern  NFC_RETURN_CODE DT_Nfc_SetTestPoint(long TestPointNumber);
extern  NFC_RETURN_CODE DT_Nfc_SetRxBlockNumber(long BlockNumber);
extern  int             DT_Set_Power(int state);
extern  int             DT_Get_Nfcc_Version(int field);
extern  uint16_t        DT_Unprocessed_Data();
extern  uint8_t         DT_Nfc_RamdumpPerformed;

/* This is from the chip version register 0x00 via host */
#define NFCC_CHIP_VERSION_REG     (0x00)
#define NFCC_CHIP_VERSION_INVALID           (0x0F)
#define NFCC_VERSION_MAJOR_MASK             (0xF0)
#define NFCC_VERSION_MINOR_MASK             (0x0F)
#define NFCC_VERSION_MAJOR_V2               (0x02)
#define NFCC_VERSION_MAJ0R_V3               (0x03)
#define NFCC_VERSION_QCA1990a_MAJOR_V1      (0X01)

#define NFCC_CHIP_REVID_REG       (0x01)
#define NFCC_METAL_REVISION_MASK  (0x0F)
#define NFCC_METAL_MASK0          (0x00)
#define NFCC_METAL_MASK1          (0x01)
#define NFCC_METAL_MASK2          (0x02)
#define NFCC_METAL_MASK3          (0x03)
#define NFCC_METAL_MASK4          (0x04)

#define NFCC_REG_WAKE               (4)

#define NCI_PROP_CMD            (0x2F)
#define NCI_SLEEP_CMD           (0x03)
#define NCI_SLEEP_PL            (0x00)

#endif /* DTNFC_H */

/****************************************** END OF FILE ***************************************************/
