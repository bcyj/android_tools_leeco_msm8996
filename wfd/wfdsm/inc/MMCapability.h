/*==============================================================================
*        @file MMCapability.h
*
*  @par DESCRIPTION:
*        Thread class.
*
*
*  Copyright (c) 2011 - 2012 Qualcomm Technologies, Inc., All Rights Reserved.
*  Qualcomm Technologies Proprietary and Confidential.
==============================================================================*/

/*==============================================================================

                      EDIT HISTORY FOR FILE

  $Header:


when        who         what, where, why
--------    ------      --------------------------------------------------------

================================================================================
*/

#ifndef _MMCAPABILITY_H
#define _MMCAPABILITY_H

#include "wdsm_mm_interface.h"
#include "UIBCDefs.h"
#include "rtsp_wfd.h"
#include <string>


/* Forward Declarations */
class rtspWfd;


/**----------------------------------------------------------------------------
   MMCapability class
-------------------------------------------------------------------------------
*/

class MMCapability
{
private:
    char strABNF[4096];

    long hex2Long(char*, int);
    string getUibcCapStr();
    string getHDCPCapStr();

public:
    MMCapability();
    ~MMCapability();

    WFD_MM_capability_t* pCapability;
    WFD_uibc_capability_t* pUibcCapability;
    bool m_bEdidValidity;
	//This function is  to derive negotiated capabilities from modes bitmap since they are not populated anywhere
    void getResolutionFromBitmap(WFD_h264_codec_config_t* , uint32 );
    char* getKeyValue(char*);
    bool setKeyValue(char*, char*);
    void dump();
    void configure(rtspWfd&);
    bool isHDCPVersionSupported(int nCapVersion);
};




#endif /*_MMCAPABILITY_H*/
