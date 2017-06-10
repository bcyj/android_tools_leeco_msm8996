/*=======================================================================
 *                             sidxparser.h
 *DESCRIPTION
 * Segment Index Information parser interface.
 *
 * Copyright (c) 2012 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Qualcomm Technologies Confidential and Proprietary

$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/SIDXParserLib/main/latest/inc/sidxparser.h#6 $
$DateTime: 2012/02/12 21:38:21 $
$Change: 2201602 $
 *======================================================================== */
#ifndef SIDX_PARSER_H
#define SIDX_PARSER_H

#include "sidxparserdatatypes.h"
class sidx_helper;


/*!
  @brief    sidxparser
  @details  sidx atom/chunk syntax conforms to DASH specification.
            Each sidx can point to a media segment/fragment or another sidx.
  @note     Please refer to DASH specification for more details.
*/

class sidxparser
{
public:

  /*!
  @brief    sidxparser constructor
  @details  Instantiates the sidx parser object
  */
  explicit            sidxparser(video::iStreamPort* pport);

  /*!
  @brief    API to parse sidx chunk.
  @note     sidx parser parses one sidx at a time and this API returns SIDX_PARSER_SIDX_BOX_DONE if successful in parsing sidx box.
            User needs to call this API to allow parser to parse subsequent boxes.
            When all the sidx boxes are parsed, API returns SIDX_PARSER_SIDX_PARSING_DONE
            The data for sidx box may not be available when parsing starts,
            thus it's user's responsibility to call this API periodically untill SIDX_PARSER_SIDX_BOX_DONE/SIDX_PARSER_SIDX_PARSING_DONE is reported.
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

  /*!
  @brief    API to check if 'lmsg' brand is present in 'STYP' atom
  @note     when sidx parser parsing the data chunk, it will check
            if the 'styp' atom contain 'lmsg' brand. If it contains 'lmsg'
            which means this chunk is the last chunk.
  */
  bool is_lmsg_present();

  /*!
  @brief    API returns total number of sidx boxes parsed so far..
            When SIDX_PARSER_SIDX_PARSING_DONE is reported, this API will report total number of sidx boxes detected.
  @note     sidx boxes use 0 based indexing, which means sidx#1, sidx#2 can be indexed by 0,1 respectively.
  */
  unsigned long get_sidx_count();

  /*!
  @brief    API returns total number of data chunk parsed so far..
            When SIDX_PARSER_SIDX_PARSING_DONE is reported, this API will report total number of data chunks detected.
  @note     DATA chunks use 0 based indexing, which means DATA-CHUNK#1, DATA-CHUNK#2 can be indexed by 0,1 respectively.
  */
  unsigned long get_data_chunk_count();

  /*!
  @brief    API to retrieve sidx information associated with given sidx ID.
            This API can be used to retrieve all the sidx data except sidx boxes pointed by given sidx id.
            To retrieve sidx indexed information(other sidx pointed by a given sidx), please refer to get_sidx_indexed_info.
  @note     sidx are indexed using 0 based indices.
            Data is copied into out parameter pInfo.
  */
  bool get_sidx_info(unsigned int,sidx_data* pInfo);

  /*!
  @brief    API to retrieve sidx indexed information(chunks pointed by a given sidx) associated with given sidx id.
  @note     sidx are indexed using 0 based indices.

            @param[in]      id          Id to identify the sidx for which indexed information is needed
            @param[in]      type        To indicate type of indexed information requested.
            @param[out]     pcount      Count of indexed items filled in by parser.
            @param[in/out]  pindex_data Pointer to array of referenced_sidx

            User needs to pass NULL for pindex_data to get the size required.
            Allocate the memory and call the API again to retrieve indexed data.
            please refer to referenced_sidx for more information.
            Data is copied into out parameter pindex_data (when not NULL).
  @return   Number of bytes needed when pindex_data is NULL/pointed by pindex_data.
  */
  unsigned int get_sidx_indexed_info(unsigned int id,
                                     index_info_type type,
                                     unsigned int* pcount,
                                     referenced_sidx* pindex_data);
  /*!
  @brief    API to retrieve data chunk information associated with given data chunk ID.
   @note    DATA CHUNKS are indexed using 0 based indices.
            Data is copied into out parameter pInfo
  */
  bool get_data_chunk_info(unsigned int,data_chunk_info* pInfo);

  /*!
  @brief    API to print all the parsed sidx data..
  @note     This API adds nothing functionality wise, the purpose is to
            dump the sidx output.
  */
  void print_all_sidx_data();


  /*!
  @brief    sidxparser destructor
  @details  destroys the sidx parser object
  */
                      ~sidxparser();
private:
  sidx_helper*        m_pHelper;
};

#endif //SIDX_PARSER_H

