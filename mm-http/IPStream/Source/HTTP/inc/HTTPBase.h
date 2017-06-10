#ifndef __HTTPBASE_H__
#define __HTTPBASE_H__
/************************************************************************* */
/**
 * HTTPBase.h
 * @brief Header file for iHTTPBase interface definition.
 *
 * COPYRIGHT 2011-2013 Qualcomm Technologies, Inc.
 * All rights reserved. Qualcomm Technologies proprietary and confidential.
 *
 ************************************************************************* */
/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/IPStream/Source/HTTP/dev/DASH/inc/HTTPBase.h#16 $
$DateTime: 2013/09/20 11:38:26 $
$Change: 4469780 $

========================================================================== */
/* =======================================================================
**               Include files for HTTPBase.h
** ======================================================================= */
#include "httpInternalDefs.h"

/*************************************************************************
FIGURE OUT WHAT THE RIGHT PLACE IS FOR THIS FILE - DEFINITELY NOT INSIDE
MAPI08 AS THIS IS COMMON BETWEEN AUDIO SMM AND MAPI SMM
*************************************************************************/

namespace video {
/* -----------------------------------------------------------------------
** Constant / Macro Declarations
** ----------------------------------------------------------------------- */
/**
HTTP source property type. The unsigned integer is expected to be converted
to std::string before setting the property. And client is expected to use
the iPropertyBag interface for setting HTTP properties on the source.
*/
typedef unsigned long int HTTPSourceProperty;

/**
HTTP source event ID type.
*/
typedef unsigned long int HTTPSourceEvent;

/* -----------------------------------------------------------------------
** Global Data Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Forward Declarations
** ----------------------------------------------------------------------- */

/* =======================================================================
**                        Class & Function Declarations
** ======================================================================= */
/**
iHTTPBase
The iHTTPBase interface defines HTTP specific events, properties and
enumerations common to both MAPI and Audio Source Media Modules.
*/
class iHTTPBase
{
public:
  /**
  Data storage option for downloaded content.
  */
  enum DataStorageType
  {
    DEFAULT_STORAGE,    //Dont'care, leave it upto the HTTP downloader
    HEAP,               //Heap storage (capped by HTTP_PROPERTY_DATA_HEAP_STORAGE_LIMIT)
    FILE_SYSTEM,        //Phone's file system or external storage device
    HEAP_FILE_SYSTEM,   //Try heap first and if unsuccessful fallback to file system
    BUFFER_PORT,        //Third-party buffer port impl
    SEGMENT_STORE,      //Segment data store. A segment may be stored on heap or file system.
  };

  enum SegmentStorageType
  {
    SEGMENT_DEFAULT,      // Leave it upto segment manager.
    SEGMENT_HEAP,         // Store segment on heap only.
    SEGMENT_FILE_SYSTEM,  // Store segment on file system only.
  };

  /**
   *Dtor
   */
  virtual ~iHTTPBase() {}

  /**
  Downloaded data storage option for playback purposes. Must set this
  before Open.
  */
  static const HTTPSourceProperty HTTP_PROPERTY_DATA_STORAGE                            = 0x010732D2;

  /**
  Max limit for downloadable content length, that applies to heap storage (in MB).
  Must set this before Open.
  */
  static const HTTPSourceProperty HTTP_PROPERTY_DATA_HEAP_STORAGE_LIMIT                 = 0x010732DE;

  /**
  Number of HTTP Reqs. to send in parallel.
  */
  static const HTTPSourceProperty     HTTP_PROPERTY_NUM_REQUESTS                     = 0x010737F9;

  static const HTTPSourceProperty HTTP_PROPERTY_DISABLE_DATA_UNIT_CANCELLATION = 0x0107392A;

  static const HTTPSourceProperty HTTP_PROPERTY_MAX_SUPPORTED_REPRESENTATION_BANDWIDTH = 0x01A71C2B;
  static const HTTPSourceProperty HTTP_PROPERTY_MAX_SUPPORTED_ASC_VALUE = 0x0C19522B;

  /**
   * Enable this to start playback behind the current live point
   * so that time to accumulate preroll for live case is not
   * constrained by segment availability.
   */
  static const HTTPSourceProperty HTTP_PROPERTY_USE_TSB_FOR_STARTUP_LATENCY_IMPROVEMENT = 0x0A47190E;

  static const HTTPSourceProperty HTTP_PROPERTY_QUALCOMM_TRANSPORT_ACCELERATOR= 0x0C37800A;

  static const HTTPSourceProperty HTTP_PROPERTY_ENABLE_AUDIO_SWITCHING = 0x06177791;
};

}/* namespace video */
#endif /* __HTTPBASE_H__ */
