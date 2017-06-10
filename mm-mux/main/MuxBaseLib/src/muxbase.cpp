/* =======================================================================
                              muxbase.cpp
DESCRIPTION
  Definition of the muxbase class.

  Copyright (c) 2011 Qualcomm Technologies, Inc., All Rights Reserved
  Qualcomm Technologies Proprietary and Confidential.
========================================================================== */

/* =======================================================================
                             Edit History

$Header: //depot/asic/msmshared/users/sateesh/multimedia2/Video/wfd-source/mm-mux/main/MuxBaseLib/src/muxbase.cpp#2 $

========================================================================== */

/* ==========================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */

#include "muxbase.h"

#ifdef FEATURE_FILEMUX_3GP
#include "isobasefilemux.h"
#endif //FEATURE_FILEMUX_3GP

#ifdef FEATURE_FILEMUX_QCP
#include "QCPBaseFile.h"
#endif //FEATURE_FILEMUX_QCP

#ifdef FEATURE_FILEMUX_AAC
#include "AACBaseFile.h"
#endif //FEATURE_FILEMUX_AAC

#ifdef FEATURE_FILEMUX_AMR
#include "AMRBaseFile.h"
#endif //FEATURE_FILEMUX_AMR

#ifdef FEATURE_FILEMUX_WAV
#include "WAVBaseFile.h"
#endif //FEATURE_FILEMUX_WAV

#ifdef FEATURE_FILEMUX_EVRCB
#include "EVRCBBaseFile.h"
#endif //FEATURE_FILEMUX_EVRCB

#ifdef FEATURE_FILEMUX_EVRCWB
#include "EVRCWBBaseFile.h"
#endif //FEATURE_FILEMUX_EVRCWB

#ifdef FEATURE_FILEMUX_MP2
#include "MP2BaseFile.h"
#endif //FEATURE_FILEMUX_MP2

#ifdef FEATURE_FILEMUX_SECURE_MP2
#include "MP2BaseFileSecure.h"
#endif //FEATURE_FILEMUX_SECURE_MP2

/* ==========================================================================

                DEFINITIONS AND DECLARATIONS FOR MODULE

This section contains definitions for constants, macros, types, variables
and other items needed by this module.

========================================================================== */

/* -----------------------------------------------------------------------
** Constant / Define Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Type Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Local Object Definitions
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Forward Declarations
** ----------------------------------------------------------------------- */

/* =======================================================================
**                            Function Definitions
** ======================================================================= */

/*! ======================================================================
    @brief   Static method to create a base file instance and return MUXBase interface

    @detail  Static method to create a base file instance and return MUXBase interface

    @param[in]  Params params to configure the MUX
	@param[in]  file_format file_format
	@param[in]  file_brand file_brand
	@param[in]  output_handle output_handle to write the formatted data


    @return     MUXBase returns object pointer.
=========================================================================*/
MUXBase *MUXBase::CreateMuxInstance( MUX_create_params_type *Params
                                    ,MUX_fmt_type file_format
                                    ,MUX_brand_type file_brand
                                    ,MUX_handle_type *output_handle
                                    ,boolean /*reorder_atom*/
#ifdef FILEMUX_WRITE_ASYNC
                                    , mux_write_file_cb_func_ptr_type file_write_fn
                                    , void * client_data
#endif /* FILEMUX_WRITE_ASYNC */
                                   )
{
  MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "MUXBase::CreateMuxInstance");

#ifdef FEATURE_FILEMUX_3GP
  if(Params && (file_format == MUX_FMT_MP4))
  {
    ISOBaseFile *isobasefile = NULL;
#ifdef FILEMUX_WRITE_ASYNC
    isobasefile = MM_New_Args(ISOBaseFile, ( Params, file_format, file_brand,
                                                output_handle,reorder_atom,
                                                file_write_fn, client_data ));
#else
    isobasefile = MM_New_Args(ISOBaseFile, ( Params, file_format, file_brand,
                                                output_handle,reorder_atom ) );
#endif  /* FILEMUX_WRITE_ASYNC  */
    if(isobasefile)
    {
      if ( isobasefile->FileSuccess() )
      {
        return isobasefile;
      }
      else
      {
        MM_Delete( isobasefile );
      }
    }
  }
#endif //FEATURE_FILEMUX_3GP

#ifdef FEATURE_FILEMUX_QCP
  if(Params && (file_format == MUX_FMT_QCP))
  {
    QCPBaseFile *qcpbasefile = NULL;
    qcpbasefile = MM_New_Args(QCPBaseFile, (Params, file_format, file_brand, output_handle));
    if(qcpbasefile)
    {
      if ( qcpbasefile->FileSuccess() )
      {
        return qcpbasefile;
      }
      else
      {
        MM_Delete( qcpbasefile );
      }
    }
  }
#endif //FEATURE_FILEMUX_QCP

#ifdef FEATURE_FILEMUX_AAC
  if(Params && (file_format == MUX_FMT_AAC))
  {
    AACBaseFile *aacbasefile = NULL;
    aacbasefile = MM_New_Args(AACBaseFile, (Params, file_format, file_brand, output_handle));
    if(aacbasefile)
    {
      if ( aacbasefile->FileSuccess() )
      {
        return aacbasefile;
      }
      else
      {
        MM_Delete( aacbasefile );
      }
    }
  }
#endif //FEATURE_FILEMUX_AAC

#ifdef FEATURE_FILEMUX_AMR
    if(Params && (file_format == MUX_FMT_AMR))
  {
    AMRBaseFile *amrbasefile = NULL;
    amrbasefile = MM_New_Args(AMRBaseFile, (Params, file_format, file_brand, output_handle));
    if(amrbasefile)
    {
      if ( amrbasefile->FileSuccess() )
      {
        return amrbasefile;
      }
      else
      {
        MM_Delete( amrbasefile );
      }
    }
  }
#endif //FEATURE_FILEMUX_AMR

#ifdef FEATURE_FILEMUX_WAV
    if(Params && (file_format == MUX_FMT_WAV))
  {
    WAVBaseFile *wavbasefile = NULL;

    if(file_brand == MUX_BRAND_PCMALaw)
    {
      file_brand = (MUX_brand_type)AUDIO_BRAND_G711_ALAW;
    }
    else if(file_brand == MUX_BRAND_PCMMULaw)
    {
      file_brand = (MUX_brand_type)AUDIO_BRAND_G711_MULAW;
    }
    //Defaulting to AUDIO_BRAND_PCM
    else
    {
      file_brand = (MUX_brand_type)AUDIO_BRAND_PCM;
    }
    wavbasefile = MM_New_Args(WAVBaseFile, (Params, file_format, file_brand, output_handle));
    if(wavbasefile)
    {
      if ( wavbasefile->FileSuccess() )
      {
        return wavbasefile;
      }
      else
      {
        MM_Delete( wavbasefile );
      }
    }
  }
#endif //FEATURE_FILEMUX_WAV

#ifdef FEATURE_FILEMUX_EVRCB
      if(Params && (file_format == MUX_FMT_EVRCB))
  {
    EVRCBBaseFile *evrcbbasefile = NULL;
    evrcbbasefile = MM_New_Args(EVRCBBaseFile, (Params, file_format, file_brand, output_handle));
    if(evrcbbasefile)
    {
      if ( evrcbbasefile->FileSuccess() )
      {
        return evrcbbasefile;
      }
      else
      {
        MM_Delete( evrcbbasefile );
      }
    }
  }
#endif //FEATURE_FILEMUX_EVRCB

#ifdef FEATURE_FILEMUX_EVRCWB
        if(Params && (file_format == MUX_FMT_EVRCWB))
  {
    EVRCWBBaseFile *evrcwbbasefile = NULL;
    evrcwbbasefile = MM_New_Args(EVRCWBBaseFile, (Params, file_format, file_brand, output_handle));
    if(evrcwbbasefile)
    {
      if ( evrcwbbasefile->FileSuccess() )
      {
        return evrcwbbasefile;
      }
      else
      {
        MM_Delete( evrcwbbasefile );
      }
    }
  }
#endif //FEATURE_FILEMUX_EVRCWB
#ifdef FEATURE_FILEMUX_MP2
  if(Params && (file_format == MUX_FMT_MP2))
  {
    MP2BaseFile *mp2basefile = NULL;
    mp2basefile = MM_New_Args(MP2BaseFile, ( Params, file_format, file_brand, output_handle ));
    if(mp2basefile)
    {
      if ( mp2basefile->FileSuccess() )
      {
        return mp2basefile;
      }
      else
      {
        MM_Delete( mp2basefile );
      }
    }
  }
#endif /* FEATURE_FILEMUX_MP2 */
#ifdef FEATURE_FILEMUX_SECURE_MP2
  if(Params && (file_format == MUX_FMT_SECURE_MP2))
  {
    MP2BaseFileSecure *mp2basefileSecure = NULL;
    mp2basefileSecure = MM_New_Args(MP2BaseFileSecure, ( Params, file_format, file_brand, output_handle));
    if(mp2basefileSecure)
    {
      if ( mp2basefileSecure->FileSuccess() )
      {
        return mp2basefileSecure;
      }
      else
      {
        MM_Delete( mp2basefileSecure );
      }
    }
  }
#endif /* FEATURE_FILEMUX_SECURE_MP2 */
  return NULL;
}

/*! ======================================================================
    @brief   Static method to release an already created Mux instance

    @detail  Static method to release an already created Mux instance

    @param[in]  Void pointer to mux instance.

    @return     MUXBase returns object pointer.
=========================================================================*/
void MUXBase::ReleaseMuxInstance(void *pMux)
{
    if(pMux)
        MM_Delete((MUXBase*)pMux);

}
