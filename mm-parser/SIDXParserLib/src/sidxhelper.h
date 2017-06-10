/*=======================================================================
 *                             sidxhelper.h
 *DESCRIPTION
 * Segment Index Information parser helper.
 *
 * Copyright (c) 2012 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Qualcomm Technologies Confidential and Proprietary

$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/SIDXParserLib/main/latest/src/sidxhelper.h#8 $
$DateTime: 2012/03/27 23:56:10 $
$Change: 2302654 $
 *======================================================================== */
#ifndef SIDX_PARSER_HELPER_H
#define SIDX_PARSER_HELPER_H

typedef struct sidx_offset_queue_node_t
{
  unsigned long long          nOffset;//Offset of sidx which is yet to be parsed.
  sidx_offset_queue_node_t*   pNext;  //Points to the next item(sidx offset) in Q
}sidx_offset_q_node;

typedef struct sidx_offset_queue_t
{
  unsigned long long    nCount;//Total count of items in Q
  sidx_offset_q_node*   pHead; //Points to the first item in Q.
  sidx_offset_q_node*   pLast; //Points to the last item in Q.
}sidx_offset_q;

#include "sidxparserdatatypes.h"

/*!
  @brief    sidx_helper
*/
class sidx_helper
{
private:
  /*!
  @brief    sidx_helper constructor
  @details  Instantiates the sidx_helper object
  */
  explicit            sidx_helper(video::iStreamPort*);

  /*!
  @brief    sidx_helper destructor
  @details  destroys the sidx_helper object
  */
                      ~sidx_helper();

  friend class sidxparser;
  /*!
  @brief    API to start parsing sidx chunks
  @note     All the sidx boxes data may not be available,so it is possible that
            parser may not be able to parse all the sidx boxes in one shot.
            If all sidx boxes data is available, this API would return SIDX_PARSER_SIDX_PARSING_DONE.
            If it encounteres data under-run, the return type would be SIDX_PARSER_SIDX_PARSING_PENDING.
            sidx parser does not have it's own thread, so it is callers responsibility,
            to call parse_sidx if under run is reported..
            Please refer to sidx_parser_status enum.
  */
  sidx_parser_status  parse_sidx();

  /*!
  @brief    API to find out the download offset value
  @note     This function looks for first SIDX atom with reference type "0".
            If such SIDX atom is not found, it returns the next known atom size
            value as required download offset value.
  */
  unsigned long long  get_download_offset();

   /* API to check if 'lmsg' brand is present in 'STYP' atom */
  bool is_lmsg_present();

  //!Debug routine to print all the information
  void walk_sidx_data();

  //! Initialize the sidx helper
  void                init_sidx_helper();
  //! To check data availability before parsing/reading
  bool                check_available_data(unsigned long long,unsigned long);
  //! To manage the memory allocation, reallocation
  bool                setup_memory(const char*);
  //!setup memory used for storing sidx data pointed by a sidx
  bool                setup_sidx_indexed_memory(unsigned int,unsigned long long,unsigned int);
  //! Checks if all sidx boxes have been parsed
  bool                is_sidx_parsing_done();
  //! Manage offset Q insertion
  bool                push_in_q(unsigned long long);
   //! Manage offset Q popping
  bool                pop_from_q(unsigned long long*);
  //! Manage offset Q removal
  bool                remove_from_q(unsigned long long);
  //! Returns count of queued items
  unsigned long       get_q_count();

  //! Current read offset
  unsigned long long  m_noffset;
  unsigned long long  m_navailoffset;

  //! Total data size
  unsigned long long  m_ndatasize;

  //! File pointer to read the data
  void*               m_pFilePtr;

  //! Internal buffer used for parsing/reading
  unsigned char*      m_pBuffer;

  //! Points to all sidx info parsed so far...
  sidx_data*          m_p_sidx_info;

  //!points to all moof info parsed so far..
  data_chunk_info*           m_p_moof_info;

  //! Current status of the parser
  sidx_parser_status  m_estatus;

  //! Handle to IStreamPort used for reading the data
  video::iStreamPort* m_pPort;

  //!Pointer to queue which holds offset of sidx which are not yet parsed.
  sidx_offset_q*      m_pOffsetQ;

  //!total number of sidx allocated
  unsigned int        nsidx;

   //!Incremented whenever the sidx is encountered
  unsigned int        ntotalsidxtobeparsed;

  //! Total number of SIDX at present to be parsed
  unsigned int        ntotalsidxtobeparsedCount;

  //!Index of sidx being parsed right now
  unsigned int        nsidxwriteindex;

  //!total number of moofs pointed by all the sidx parsed so far...
  unsigned int        nmoof;

  //!Index of reference item being parsed...
  unsigned int        nrefwriteindex;

  bool                m_bLmsgPresent;

  //!flag to indicate if memory allocated to this sidx
  bool                m_bmemoryAllocated;
};

#endif //SIDX_PARSER_HELPER_H

