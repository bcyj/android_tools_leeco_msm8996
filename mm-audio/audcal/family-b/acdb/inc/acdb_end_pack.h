/**
@file acdb_end_pack.h

@brief This file defines pack attributes for different compilers to be used to
pack aDSP API data structures.
*/

/*===========================================================================
                    Copyright (c) 2010-2014 QUALCOMM Technologies Incorporated.
                    All Rights Reserved.
                    Qualcomm Technologies Proprietary and Confidential.
===========================================================================*/

/*========================================================================
Edit History

when       who     what, where, why
--------   ---     -------------------------------------------------------
09/22/10    sd      (Tech Pubs) Edited Doxygen markup and comments.
06/07/10   rkc      Created file.

========================================================================== */

/** @addtogroup acph_api_pack
  @{ */

#if defined( __qdsp6__ )
/* No packing atrributes for Q6 compiler; all structs manually packed */
#elif defined( __GNUC__ )
  __attribute__((packed))
#elif defined( __arm__ )
#elif defined( _MSC_VER )
  #pragma pack( pop )
#else
  #error "Unsupported compiler."
#endif /* __GNUC__ */

/** @} */  /* end_ addtogroup acph_api_pack */
