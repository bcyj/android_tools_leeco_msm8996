#ifndef _OI_DUMP_H
#define _OI_DUMP_H
/** @file   

  This header file exposes interfaces to various dump utilties for debugging.
  
*/

/**********************************************************************************
  $AccuRev-Revision: 563/1 $
  Copyright 2002 - 2004 Open Interface North America, Inc. All rights reserved.
***********************************************************************************/

//sean
//#include "oi_utils.h"

/** \addtogroup Debugging Debugging APIs */
/**@{*/

#ifdef __cplusplus
extern "C" {
#endif



#ifdef OI_DEBUG

/** Dump utility for debugging L2CAP. */
void OI_L2CAP_Dump(void);

/** Dump utility for debugging RFCOMM. */
void OI_RFCOMM_Dump(void);

/** Dump utility for debugging HCI. */
void OI_HCI_Dump(void);

/** Dump utility for debugging the Device Manager. */
void OI_DEVMGR_Dump(void);

/** Dump utility for debugging the Policy Manager. */
void OI_POLICYMGR_Dump(void);

/** Dump utility for debugging the Security Manager. */
void OI_SECMGR_Dump(void);

/** Dump utility for debugging SDPDB. */
void OI_SDPDB_Print(void);

void OI_DISPATCH_Dump(void);


#else /* OI_DEBUG */

#define OI_L2CAP_Dump()                  OI_Printf("\nNot compiled with OI_DEBUG\n");

#define OI_RFCOMM_Dump()                 OI_Printf("\nNot compiled with OI_DEBUG\n");

#define OI_HCI_Dump()                    OI_Printf("\nNot compiled with OI_DEBUG\n");

#define OI_DEVMGR_Dump()                 OI_Printf("\nNot compiled with OI_DEBUG\n");

#define OI_POLICYMGR_Dump()              OI_Printf("\nNot compiled with OI_DEBUG\n");

#define OI_SECMGR_Dump()                 OI_Printf("\nNot compiled with OI_DEBUG\n");

#define OI_SDPDB_Print()                 OI_Printf("\nNot compiled with OI_DEBUG\n");

#define OI_DISPATCH_Dump()               OI_Printf("\nNot compiled with OI_DEBUG\n");

#endif /* OI_DEBUG */


#ifdef MEMMGR_PROFILE

/** Dump utility for debugging the Memory Manager. */
void OI_MEMMGR_Dump(void);

#else

#define OI_MEMMGR_Dump()           OI_Printf("\nNot compiled with MEMMGR_PROFILE\n")

#endif /* MEMMGR_PROFILE */


#ifdef MEMMGR_DEBUG

/** Dump utility for debugging the used blocks in the Memory Manager. */
void OI_MEMMGR_DumpUsedBlocks(void);

/** Dump utility for debugging the memory pools in the Memory Manager. */
void OI_MEMMGR_DumpPools(void);

#else /* MEMMGR_DEBUG */

#define OI_MEMMGR_DumpUsedBlocks() OI_Printf("\nNot compiled with MEMMGR_DEBUG\n")

#define OI_MEMMGR_DumpPools()      OI_Printf("\nNot compiled with MEMMGR_DEBUG\n")

#endif /* MEMMGR_DEBUG */



#ifdef __cplusplus
}
#endif

/**@}*/

#endif /* _OI_DUMP_H */

