/*=======================================================================
 *                             sidxparser.cpp
 *DESCRIPTION
 * Segment Index Information parser.
 *
 *Copyright (c) 2011-2013 QUALCOMM Technologies Inc, All Rights Reserved.
 *QUALCOMM Technologies Proprietary and Confidential.

$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/SIDXParserLib/main/latest/src/sidxparser.cpp#36 $
$DateTime: 2013/09/06 05:25:33 $
$Change: 4392124 $
*======================================================================== */

//List all the includes
#include "sidxparser.h"
#include "sidxhelper.h"
#include "oscl_file_io.h"
#include "MMDebugMsg.h"
#include "MMMalloc.h"
#include "MMMemory.h"

//List all the constants and global #defines
//#define SIDX_PARSER_DEBUG
#define SIDX_SIGNATURE            "sidx"
#define MOOF_SIGNATURE            "moof"
#define STYP_SIGNATURE            "styp"
#define LMSG_TYPE                 "lmsg"
#define STYP_SCALE_BYTES          4
#define STYP_SIG_SIZE             4
#define STYP_MAJOR_MINOR_BRAND_SIZE  8
#define SIDX_SIG_SIZE             4
#define MOOF_SIG_SIZE             4
#define SIDX_ALIGN_BOUNDARY       8
#define SIDX_SCALE_BYTES          4
#define SIDX_STREAM_ID_BYTES      4
#define SIDX_VERSION_ONE_SIZE     8
#define SIDX_VERSION_BYTE         1
#define SIDX_VERSION_ZERO_SIZE    4
#define SIDX_FLAG_BYTES           3
#define SIDX_REF_COUNT_SIZE       4
#define SIDX_REF_ENTRY_DATA_SIZE  12
#define SIDX_SAP_DELTA_TIME_SIZE_BITS  28
#define SIDX_BUFFER_SIZE          1024
#define FIRST_REF_INDEX           0
#define SIDX_MAX_DATA_SIZE        0x7FFFFFFF;

//Global inline functions
inline void SIDX_REVERSE_ENDIAN(uint8* ptr,uint8 size)
{
  uint8 endpos = size-1;
  if(ptr && (size >1))
  {
    for(int i =0; (i < size) && (i <= endpos); i++)
    {
      uint8 temp = ptr[i];
      *(ptr+i) = *(ptr+endpos);
      *(ptr+endpos) = temp;
      endpos--;
    }
  }
}
inline bool IS_STATUS_OK(sidx_parser_status status)
{
  bool bret = true;
  if( (status == SIDX_PARSER_INTERNAL_FATAL_ERROR)||
      (status == SIDX_PARSER_READ_FAILED)         ||
      (status == SIDX_PARSER_SIDX_PARSING_DONE)   ||
      (status == SIDX_PARSER_OUT_OF_MEMORY)       ||
      (status == SIDX_PARSER_SIDX_NOT_AVAILABLE) )
  {
    bret = false;
  }
  return bret;
}

/***************************SIDX parser interface*------------------------------*/

/*!
@brief    sidxparser constructor
@details  Instantiates the sidx parser object
*/
sidxparser::sidxparser(video::iStreamPort* pport)
{
  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_LOW, "sidxparser::sidxparser istreamport..");
  m_pHelper = new sidx_helper (pport);
}
/*!
@brief    sidxparser destructor
@details  destroys the sidx parser object
*/
sidxparser::~sidxparser()
{
  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_LOW, "sidxparser::~sidxparser");
  if(m_pHelper)
  {
    delete m_pHelper;
    m_pHelper = NULL;
  }
}
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
sidx_parser_status sidxparser::parse_sidx()
{
  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM, "sidxparser::parse_sidx");
  if(m_pHelper)
  {
    return m_pHelper->parse_sidx();
  }
  return SIDX_PARSER_INTERNAL_FATAL_ERROR;
}

unsigned long long  sidxparser::get_download_offset()
{
  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM, "sidxparser::parse_sidx");
  if(m_pHelper)
  {
    return m_pHelper->get_download_offset();
  }
  return 0;
}

bool sidxparser::is_lmsg_present()
{
  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM, "sidxparser::is_lmsg_present");
  if(m_pHelper)
  {
    return m_pHelper->is_lmsg_present();
  }
  return false;
}
/*!
@brief    API returns total number of sidx boxes parsed so far..
          When SIDX_PARSER_SIDX_PARSING_DONE is reported, this API will report total number of sidx boxes detected.
@note     sidx boxes use 0 based indexing, which means sidx#1, sidx#2 can be indexed by 0,1 respectively.
*/
unsigned long sidxparser::get_sidx_count()
{
  unsigned long ncount = 0;
  if(m_pHelper)
  {
    ncount = m_pHelper->nsidx;
  }
  MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_LOW, "sidxparser::get_sidx_count %ld",ncount);
  return ncount;
}
/*!
@brief    API returns total number of data chunk parsed so far..
          When SIDX_PARSER_SIDX_PARSING_DONE is reported, this API will report total number of data chunks detected.
@note     DATA chunks use 0 based indexing, which means DATA-CHUNK#1, DATA-CHUNK#2 can be indexed by 0,1 respectively.
*/
unsigned long sidxparser::get_data_chunk_count()
{
  unsigned long ncount = 0;
  if(m_pHelper)
  {
    ncount = m_pHelper->nmoof;
  }
  MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_LOW, "sidxparser::get_data_chunk_count %ld",ncount);
  return ncount;
}
/*!
@brief    API to retrieve sidx information associated with given sidx ID.
@note     sidx are indexed using 0 based indices.
          Data is copied into out parameter pInfo
*/
bool sidxparser::get_sidx_info(unsigned int sidxid,sidx_data* pInfo)
{
  bool bret = false;
  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_LOW, "sidxparser::get_sidx_info");
  if(m_pHelper && (sidxid < m_pHelper->nsidx ) && pInfo)
  {
    memcpy(pInfo,m_pHelper->m_p_sidx_info+sidxid,sizeof(sidx_data));
    //we don't want to provide indexed information here.
    pInfo->p_referenced_sidx = NULL;
    bret = true;
  }
  return bret;
}
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
@return     Number of bytes needed when pindex_data is NULL/pointed by pindex_data.
*/
unsigned int sidxparser:: get_sidx_indexed_info( unsigned int id,
                                                 index_info_type type,
                                                 unsigned int* pcount,
                                                 referenced_sidx* pindex_data)
{
  unsigned int nsize = 0;
  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_LOW, "sidxparser::get_sidx_indexed_info");
  if(m_pHelper && (id < m_pHelper->nsidx ) )
  {
    if(pindex_data && pcount)
    {
      *pcount = 0;
      if(type == INDEX_INFO_SIDX)
      {
        memcpy(pindex_data,m_pHelper->m_p_sidx_info[id].p_referenced_sidx,
               (sizeof(referenced_sidx)*m_pHelper->m_p_sidx_info[id].n_sidxpointed));
        *pcount = m_pHelper->m_p_sidx_info[id].n_sidxpointed;
      }//if(type == INDEX_INFO_SIDX)
      else if(type == INDEX_INFO_MOOF)
      {
        int ntoiterate = m_pHelper->m_p_sidx_info[id].n_moofpointed;
        int noutindex = 0;
        bool berror = false;
        for(int i = m_pHelper->m_p_sidx_info[id].n_moofstartid; ntoiterate >= 0;i++)
        {
          //Validate the start-id against number of moofs stored to avoid
          //accessing beyond what we have allocated
          if(i < (int)m_pHelper->nmoof)
          {
            pindex_data[noutindex].offset = m_pHelper->m_p_moof_info[i].n_offset;
            pindex_data[noutindex].nsize  = m_pHelper->m_p_moof_info[i].n_referenced_size;
            //Make sure we will always exit the loop
            ntoiterate--;
          }
          else
          {
            //if we come here, it means there was a bug while storing nmoofstartid/nmoofpointed for sidx pointed by id.
            MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR, "get_sidx_indexed_info INVALID nmoofstartid, nmoofpointed detected");
            MM_MSG_PRIO3(MM_FILE_OPS, MM_PRIO_ERROR,
              "sidx-id %u nmoofstartid %u nmoofpointed %u",id,m_pHelper->m_p_sidx_info[id].n_moofstartid,m_pHelper->m_p_sidx_info[id].n_moofpointed);
            berror = true;
            break;
          }
        }//for(int i = m_pHelper->m_p_sidx_info[id].nmoofstartid; ntoiterate >= 0;i++)
        if(!berror)
        {
          *pcount = m_pHelper->m_p_sidx_info[id].n_moofpointed;
        }
      }//else if(type == INDEX_INFO_MOOF)
    }//if(pindex_data && pcount)
    else
    {
      if(type == INDEX_INFO_SIDX)
      {
        nsize = (sizeof(referenced_sidx)*m_pHelper->m_p_sidx_info[id].n_sidxpointed);
      }//if(type == INDEX_INFO_SIDX)
      else if(type == INDEX_INFO_MOOF)
      {
        nsize = (sizeof(referenced_sidx)*m_pHelper->m_p_sidx_info[id].n_moofpointed);
      }//else if(type == INDEX_INFO_MOOF)
    }
  }//if(m_pHelper && (id < m_pHelper->nsidx ) )
  return nsize;
}

/*!
 @brief   API to retrieve data chunk information associated with given data chunk ID.
 @note    DATA CHUNKS are indexed using 0 based indices.
          Data is copied into out parameter pInfo
*/
bool sidxparser::get_data_chunk_info(unsigned int moofid,data_chunk_info* pInfo)
{
  bool bret = false;
  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_LOW, "sidxparser::get_data_chunk_info");
  if(m_pHelper && (moofid < m_pHelper->nmoof) && pInfo && m_pHelper->m_p_moof_info)
  {
    memcpy(pInfo,m_pHelper->m_p_moof_info+moofid,sizeof(data_chunk_info));
    bret = true;
  }
  return bret;
}
/*!
 @brief    API to print all the parsed sidx data..
 @note     This API adds nothing functionality wise, the purpose is to
           dump the sidx output.
*/
void sidxparser::print_all_sidx_data()
{
  if(m_pHelper)
  {
    m_pHelper->walk_sidx_data();
  }
}
/***************************sidx parser helper*------------------------------*/
sidx_helper::sidx_helper(video::iStreamPort* pport)
{
  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_LOW, "sidx_helper::sidx_helper");
  init_sidx_helper();
  m_pPort = pport;
  OSCL_FILE* pFile = OSCL_FileOpen(pport);
  if(pFile)
  {
    m_pFilePtr = pFile;
    m_pBuffer = (unsigned char*)MM_Malloc(SIDX_BUFFER_SIZE);
    if(m_pBuffer)
    {
      memset(m_pBuffer,0,SIDX_BUFFER_SIZE);
    }
  }
  if(m_pPort)
  {
    int64 ndatalength = 0;
    //set the max size possible as content length may not be available..
    m_ndatasize = SIDX_MAX_DATA_SIZE;
    if(m_pPort->GetContentLength(&ndatalength) == video::iStreamPort::DS_SUCCESS)
    {
      m_ndatasize = (unsigned long long)ndatalength;
    }
  }
}
//! Initialize the sidx helper
void sidx_helper::init_sidx_helper()
{
  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_LOW, "sidx_helper::init_sidx_helper");
  m_noffset = 0;
  m_navailoffset = 0;
  m_pFilePtr = NULL;
  m_pBuffer = NULL;
  m_pPort = NULL;
  m_estatus = SIDX_PARSER_INIT;
  m_ndatasize = 0;
  m_pOffsetQ = NULL;
  nsidx = 0;
  nmoof = 0;
  m_p_sidx_info = NULL;
  m_p_moof_info = NULL;
  ntotalsidxtobeparsed = 0;
  nsidxwriteindex = 0;
  nrefwriteindex = 0;
  m_bLmsgPresent = false;
  m_bmemoryAllocated = false;
  ntotalsidxtobeparsedCount = 0;

}

/*!
  @brief    API to find out the download offset value
  @note     This function looks for first SIDX atom with reference type "0".
            If such SIDX atom is not found, it returns the next known atom size
            value as required download offset value.
  */
unsigned long long sidx_helper::get_download_offset()
{
  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_LOW, "sidx_helper::get_download_offset");
  OSCL_FILE* pFile = (OSCL_FILE*)m_pFilePtr;
  uint32 ullDataRead = 0;
  uint32 ulAtomSize = 0;
  uint64 ullOffset = m_noffset;

  if( (pFile) && (m_pPort) )
  {
    while( (true) && (IS_STATUS_OK(m_estatus)) )
    {
      if(check_available_data(ullOffset, SIDX_ALIGN_BOUNDARY))
      {
        ullDataRead = OSCL_FileSeekRead(m_pBuffer,
                                        1,
                                        SIDX_ALIGN_BOUNDARY,
                                        pFile,
                                        ullOffset,
                                        SEEK_SET);
        if(ullDataRead)
        {
          /* Calculate atom size */
          memcpy(&ulAtomSize, m_pBuffer, SIDX_SIG_SIZE);
          SIDX_REVERSE_ENDIAN((uint8*)&ulAtomSize, SIDX_SIG_SIZE);

          if(!memcmp(m_pBuffer + SIDX_SIG_SIZE,SIDX_SIGNATURE,SIDX_SIG_SIZE) &&
             check_available_data(ullOffset, ulAtomSize))
          {
#ifdef SIDX_PARSER_DEBUG
            MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,
              "sidx_helper::parse_sidx located sidx @ %lld",ullOffset);
#endif
              uint64 ullStartoffset = ullOffset + SIDX_ALIGN_BOUNDARY;

#ifdef SIDX_PARSER_DEBUG
                MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,
                             "sidx_helper::get_download_offset nsidxsize  %lu",
                             ulAtomSize);
#endif
                uint8 nversion = 0;
                ullDataRead = OSCL_FileSeekRead(m_pBuffer,
                                                1,
                                                SIDX_VERSION_BYTE,
                                                pFile,
                                                ullStartoffset,
                                                SEEK_SET);
            memcpy(&nversion,m_pBuffer,SIDX_VERSION_BYTE);
            ullStartoffset += SIDX_VERSION_BYTE;

            /* Skip Flag bytes */
            ullStartoffset += SIDX_FLAG_BYTES;

            /* Skip Stream ID bytes */
            ullStartoffset += SIDX_STREAM_ID_BYTES;

            /* Skip Scale bytes */
            ullStartoffset += SIDX_SCALE_BYTES;

            uint64 ullFirstOffset = 0;
            if(nversion)
            {
              /* Skip earliest presentation bytes */
              ullStartoffset += SIDX_VERSION_ONE_SIZE;

              ullDataRead = OSCL_FileSeekRead(m_pBuffer, 1,
                                              SIDX_VERSION_ONE_SIZE, pFile,
                                              ullStartoffset, SEEK_SET);

              memcpy(&ullFirstOffset,m_pBuffer,SIDX_VERSION_ONE_SIZE);
              SIDX_REVERSE_ENDIAN((uint8*)&ullFirstOffset,
                                  SIDX_VERSION_ONE_SIZE);
              ullStartoffset += SIDX_VERSION_ONE_SIZE;
#ifdef SIDX_PARSER_DEBUG
              MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,
                       "V#1 sidx::get_download_offset ullFirstOffset %llu",
                           ullFirstOffset);
#endif
            }
            else
            {
              /* Skip earliest presentation bytes */
              ullStartoffset += SIDX_VERSION_ZERO_SIZE;

              ullDataRead = OSCL_FileSeekRead(m_pBuffer, 1,
                                              SIDX_VERSION_ZERO_SIZE,
                                              pFile, ullStartoffset,
                                              SEEK_SET);

              memcpy(&ullFirstOffset,m_pBuffer,SIDX_VERSION_ZERO_SIZE);
              SIDX_REVERSE_ENDIAN((uint8*)&ullFirstOffset,
                                  SIDX_VERSION_ZERO_SIZE);
              ullStartoffset += SIDX_VERSION_ZERO_SIZE;
#ifdef SIDX_PARSER_DEBUG
              MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_HIGH,
                       "V#0 sidx::get_download_offset ullFirstOffset %llu",
                           ullFirstOffset);
#endif
            }

            /* Skip Reference count bytes */
            ullStartoffset += SIDX_REF_COUNT_SIZE;

            bool   bReferenceType = false;
            uint32 ulReferencedSize = 0;

            ullDataRead = OSCL_FileSeekRead(m_pBuffer,1,
                                            SIDX_REF_ENTRY_DATA_SIZE,
                                            pFile, ullStartoffset,
                                            SEEK_SET);
            memcpy(&ulReferencedSize,m_pBuffer,sizeof(uint32));
            SIDX_REVERSE_ENDIAN((uint8*)&ulReferencedSize,sizeof(uint32));

            bReferenceType = (ulReferencedSize & 0x80000000)?true:false;

            /* Check the reference type. If reference type is "0", then
               SIDX atom pointing to MOOF atom. Calculate MOOF atom
               start offset and break loop. else, skip current atom and go
               to next atom*/
            if(bReferenceType)
            {
              ullOffset += ulAtomSize;
            }
            else
            {
              ullOffset += (ulAtomSize + ullFirstOffset +
                            2 * SIDX_ALIGN_BOUNDARY);
              break;
            }
          }//if(memcmp(m_pBuffer,SIDX_SIGNATURE,SIDX_SIG_SIZE)== 0)
          else
          {
            ullOffset += ulAtomSize;
          }
        }//if(ullDataRead)
        else
        {
          MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_FATAL,
                       "get_download_offset read %u failed @offset %llu",
                       SIDX_ALIGN_BOUNDARY, ullOffset);
          m_estatus = SIDX_PARSER_READ_FAILED;
          break;
        }
      }//if(check_available_data(m_noffset,nreqbytes))
      else
      {
        ullOffset += (2 * SIDX_ALIGN_BOUNDARY);
        break;
      }
    }//while( (true) && (IS_STATUS_OK(m_estatus)) )
  }//if( (pFile) && (m_pPort) )
  return ullOffset;
}

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
sidx_parser_status sidx_helper::parse_sidx()
{
  sidx_parser_status status = m_estatus;
  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_LOW, "sidx_helper::parse_sidx");
  OSCL_FILE* pFile = (OSCL_FILE*)m_pFilePtr;
  uint32 nread = 0;
  uint32 nsidxsize = 0;
  uint32 nstypsize = 0, nConsumeDataSize = 0;

  if( (pFile) && (m_pPort) )
  {
    while( (true) && (IS_STATUS_OK(m_estatus)) )
    {
      unsigned long nreqbytes = SIDX_ALIGN_BOUNDARY;
      if( (m_noffset + SIDX_ALIGN_BOUNDARY) > m_ndatasize)
      {
        nreqbytes = (unsigned long)(m_ndatasize - m_noffset);
#ifdef SIDX_PARSER_DEBUG
        MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,
          "sidx_helper::parse_sidx adjusted nreqbytes %ld",nreqbytes);
#endif
      }
      if(check_available_data(m_noffset,nreqbytes) && nreqbytes)
      {
        nread = OSCL_FileSeekRead(m_pBuffer,
                                  1,
                                  SIDX_ALIGN_BOUNDARY,
                                  pFile,
                                  m_noffset,
                                  SEEK_SET);
        if(nread)
        {
          if(!memcmp(m_pBuffer + STYP_SIG_SIZE,STYP_SIGNATURE,STYP_SIG_SIZE))
          {
            memcpy(&nstypsize,m_pBuffer,STYP_SIG_SIZE);

            SIDX_REVERSE_ENDIAN((uint8*)&nstypsize,STYP_SIG_SIZE);

            if(check_available_data(m_noffset, nstypsize))
            {
              /* Read complete styp atom data*/
              nread = OSCL_FileSeekRead(m_pBuffer,
                                        1,
                                        nstypsize - SIDX_ALIGN_BOUNDARY,
                                        pFile,
                                        m_noffset + SIDX_ALIGN_BOUNDARY,
                                        SEEK_SET);

              nConsumeDataSize = 0 ;
              while(nConsumeDataSize < nstypsize - SIDX_ALIGN_BOUNDARY)
              {
                if(!memcmp(m_pBuffer +nConsumeDataSize,LMSG_TYPE,STYP_SIG_SIZE))
                {
                  m_bLmsgPresent = true;
                }
                nConsumeDataSize += STYP_SIG_SIZE;
              }
              m_noffset += nstypsize;
            }
          }
          else if(!memcmp(m_pBuffer + SIDX_SIG_SIZE,SIDX_SIGNATURE,SIDX_SIG_SIZE))
          {
            uint64 ullSidxStartOffset = m_noffset;
#ifdef SIDX_PARSER_DEBUG
            MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,
              "sidx_helper::parse_sidx located sidx @ %lld",m_noffset);
#endif
            memcpy(&nsidxsize,m_pBuffer,SIDX_SIG_SIZE);
            SIDX_REVERSE_ENDIAN((uint8*)&nsidxsize,SIDX_SIG_SIZE);

            /* Allocate the memory for SIDX Structure without waiting
               for entire SIDX data to download.We have to allocate memory
               only once for that particular sidx */
            if((m_bmemoryAllocated || setup_memory(SIDX_SIGNATURE)) &&
               (m_p_sidx_info))
            {
              m_p_sidx_info[nsidxwriteindex].n_size = nsidxsize;
              m_p_sidx_info[nsidxwriteindex].n_offset =
                                     (unsigned long)m_noffset;
            }

            /* To avoid data under-run scenarios in the middle, look for MOOF
               atom start offset. MOOF atom can be considered as boundary to
               SIDX. Instead of going byte after byte, Parser knows further
               data will be organized as atoms only. Thats why, following logic
               reads SIDX_ALIGN_BOUNDARY(8) bytes at the start of each atom to
               calculate atom size and type.*/
            m_noffset += nsidxsize;
            do
            {
              uint32 atomsize = 0;
              nread = OSCL_FileSeekRead(m_pBuffer,
                                        1,
                                        SIDX_ALIGN_BOUNDARY,
                                        pFile,
                                        m_noffset,
                                        SEEK_SET);
              if(0 == nread)
              {
                /* If Data read is not successful, then check end of data flag.
                   If EOF flag is set to true, then assume complete SIDX is
                   already downloaded. */
                int64 sllOffset = 0;
                bool  bEOS      = false;
                m_pPort->GetAvailableOffset(&sllOffset, &bEOS);
                if(bEOS == true)
                {
                  MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM,
                    "sidx_helper::EOF is reached at %lld",sllOffset);
                  break;
                }
                m_estatus = SIDX_PARSER_SIDX_PARSING_PENDING;
                /* Update m_noffset value to start of SIDX */
                m_noffset = ullSidxStartOffset;
                return m_estatus;
              }
              memcpy(&atomsize,m_pBuffer,SIDX_SIG_SIZE);
              SIDX_REVERSE_ENDIAN((uint8*)&atomsize,SIDX_SIG_SIZE);
              m_noffset += atomsize;
            } while(memcmp(m_pBuffer + MOOF_SIG_SIZE,
                           MOOF_SIGNATURE,
                           MOOF_SIG_SIZE));

            /* Revert m_noffest value to parent SIDX start value and Reset
               parent SIDX atom size value. */
            m_noffset = ullSidxStartOffset + SIDX_SIG_SIZE;

            if((check_available_data(m_noffset, nsidxsize)) &&
               (m_p_sidx_info))
            {
              unsigned long long nstartoffset = m_noffset - SIDX_SIG_SIZE;
              m_bmemoryAllocated = false;
              //we are starting parsing of new sidx,add it in Q.
              if( ntotalsidxtobeparsedCount || push_in_q(nstartoffset))
              {
                m_noffset += SIDX_SIG_SIZE;
#ifdef SIDX_PARSER_DEBUG
                MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,
                           "sidx_helper::parse_sidx nsidxsize  %ld",nsidxsize);
#endif
                uint8 nversion = 0;
                nread = OSCL_FileSeekRead(m_pBuffer,1,SIDX_VERSION_BYTE,pFile,(uint32)m_noffset,SEEK_SET);
                memcpy(&nversion,m_pBuffer,SIDX_VERSION_BYTE);
                m_p_sidx_info[nsidxwriteindex].n_version = nversion;
                m_noffset += SIDX_VERSION_BYTE;

                uint32 nflags = 0;
                nread = OSCL_FileSeekRead(m_pBuffer,1,SIDX_FLAG_BYTES,pFile,(uint32)m_noffset,SEEK_SET);
                memcpy(&nflags,m_pBuffer,SIDX_FLAG_BYTES);
                SIDX_REVERSE_ENDIAN((uint8*)&nflags,(sizeof(uint32)) );
                m_p_sidx_info[nsidxwriteindex].n_flags = nflags;
                m_noffset+=SIDX_FLAG_BYTES;

                nread = OSCL_FileSeekRead(m_pBuffer,1,SIDX_STREAM_ID_BYTES,pFile,(uint32)m_noffset,SEEK_SET);
                uint32 nstreamid = 0;
                memcpy(&nstreamid,m_pBuffer,SIDX_STREAM_ID_BYTES);
                SIDX_REVERSE_ENDIAN((uint8*)&nstreamid,SIDX_STREAM_ID_BYTES);
                m_p_sidx_info[nsidxwriteindex].n_streamid = nstreamid;
#ifdef SIDX_PARSER_DEBUG
                MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,
                            "sidx_helper::parse_sidx nstreamid %ld",nstreamid);
#endif
                m_noffset+= SIDX_STREAM_ID_BYTES;

                nread = OSCL_FileSeekRead(m_pBuffer,1,SIDX_SCALE_BYTES,pFile,(uint32)m_noffset,SEEK_SET);
                uint32 nscale = 0;
                memcpy(&nscale,m_pBuffer,SIDX_SCALE_BYTES);
                SIDX_REVERSE_ENDIAN((uint8*)&nscale,SIDX_SCALE_BYTES);
                m_p_sidx_info[nsidxwriteindex].n_scale = nscale;
#ifdef SIDX_PARSER_DEBUG
                MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,
                             "sidx_helper::parse_sidx nversion %ld",nversion);
#endif
                m_noffset+= SIDX_SCALE_BYTES;

                uint64 nearliestprestime = 0;
                uint64 nsubsegment_endtime = 0;
                uint64 nfirstoffset = 0;
                if(nversion)
                {
                  nread = OSCL_FileSeekRead(m_pBuffer,1,SIDX_VERSION_ONE_SIZE,pFile,(uint32)m_noffset,SEEK_SET);
                  memcpy(&nearliestprestime,m_pBuffer,SIDX_VERSION_ONE_SIZE);
                  SIDX_REVERSE_ENDIAN((uint8*)&nearliestprestime,SIDX_VERSION_ONE_SIZE);
                  nsubsegment_endtime = nearliestprestime;
                  if(nscale)
                  {
                    nearliestprestime = (((uint64)nearliestprestime * MILLISEC_TIMESCALE_UNIT)/nscale);
                  }
                  m_noffset+= SIDX_VERSION_ONE_SIZE;
                  nread = OSCL_FileSeekRead(m_pBuffer,1,SIDX_VERSION_ONE_SIZE,pFile,(uint32)m_noffset,SEEK_SET);
                  memcpy(&nfirstoffset,m_pBuffer,SIDX_VERSION_ONE_SIZE);
                  SIDX_REVERSE_ENDIAN((uint8*)&nfirstoffset,SIDX_VERSION_ONE_SIZE);
                  m_noffset+= SIDX_VERSION_ONE_SIZE;
#ifdef SIDX_PARSER_DEBUG
                  MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_HIGH,
        "V#1 sidx_helper::parse_sidx nearliestprestime %lld nfirstoffset %lld",
                               nearliestprestime, nfirstoffset);
#endif
                }
                else
                {
                  nread = OSCL_FileSeekRead(m_pBuffer,1,SIDX_VERSION_ZERO_SIZE,pFile,(uint32)m_noffset,SEEK_SET);
                  memcpy(&nearliestprestime,m_pBuffer,SIDX_VERSION_ZERO_SIZE);
                  SIDX_REVERSE_ENDIAN((uint8*)&nearliestprestime,SIDX_VERSION_ZERO_SIZE);
                  nsubsegment_endtime = nearliestprestime;
                  nearliestprestime =  (uint32)(((uint64)nearliestprestime * MILLISEC_TIMESCALE_UNIT)/nscale);
                  m_noffset+= SIDX_VERSION_ZERO_SIZE;
                  nread = OSCL_FileSeekRead(m_pBuffer,1,SIDX_VERSION_ZERO_SIZE,pFile,(uint32)m_noffset,SEEK_SET);
                  memcpy(&nfirstoffset,m_pBuffer,SIDX_VERSION_ZERO_SIZE);
                  SIDX_REVERSE_ENDIAN((uint8*)&nfirstoffset,SIDX_VERSION_ZERO_SIZE);
                  m_noffset+= SIDX_VERSION_ZERO_SIZE;
#ifdef SIDX_PARSER_DEBUG
                  MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_HIGH,
                    "V#0 parse_sidx nearliestprestime %llu nfirstoffset %llu",
                    nearliestprestime,nfirstoffset);
#endif
                }
                m_p_sidx_info[nsidxwriteindex].n_earliestprestime = nearliestprestime;
                m_p_sidx_info[nsidxwriteindex].n_firstoffset = nfirstoffset;

                uint32 nrefcount = 0;
                nread = OSCL_FileSeekRead(m_pBuffer,1,SIDX_REF_COUNT_SIZE,pFile,(uint32)m_noffset,SEEK_SET);
                memcpy(&nrefcount,m_pBuffer,SIDX_REF_COUNT_SIZE);
                SIDX_REVERSE_ENDIAN((uint8*)&nrefcount,SIDX_REF_COUNT_SIZE);
                m_p_sidx_info[nsidxwriteindex].n_refcount = nrefcount;
                MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM,
                  "sidx_helper::parse_sidx nrefcount %ld",nrefcount);
                m_noffset+= SIDX_REF_COUNT_SIZE;

                bool   breference_type = false;
                uint32 nnreferenced_size = 0;

                bool   m_bstarts_with_sap = false;
                uint32 nSAP_type = 0;
                uint32 nSAP_delta_time = 0;
                uint32 i =1;
                unsigned long long nrefoffset = 0;
                unsigned long long nstarttime = 0;

                if(m_p_sidx_info)
                {
                  for(i = 0; i < nrefcount;i++)
                  {
                    if(SIDX_PARSER_INTERNAL_FATAL_ERROR == m_estatus)
                    {
                      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,
                                  "sidx_helper::parse_sidx(moof found before SIDX)");
                      break;
                    }
                    nread = OSCL_FileSeekRead(m_pBuffer,1,SIDX_REF_ENTRY_DATA_SIZE,pFile,(uint32)m_noffset,SEEK_SET);
                    memcpy(&nnreferenced_size,m_pBuffer,sizeof(uint32));
                    SIDX_REVERSE_ENDIAN((uint8*)&nnreferenced_size,sizeof(uint32));
                    breference_type = (nnreferenced_size & 0x80000000)?true:false;
                    if(breference_type)
                    {
                      ntotalsidxtobeparsed++;
                    }
                    //nnreferenced_size &= 0xFFFFFFFE;
                    nnreferenced_size &= 0x7FFFFFFF;

                    uint32 tmp_duration;
                    memcpy(&tmp_duration,m_pBuffer+sizeof(uint32),sizeof(uint32));
                    SIDX_REVERSE_ENDIAN((uint8*)&tmp_duration,sizeof(uint32));
                    uint32 nsubsegment_duration = (uint32)((nsubsegment_endtime + tmp_duration) * MILLISEC_TIMESCALE_UNIT / nscale -
                      (nsubsegment_endtime * MILLISEC_TIMESCALE_UNIT / nscale));
                    nsubsegment_endtime += tmp_duration;

                    memcpy(&nSAP_delta_time,m_pBuffer+(2*sizeof(uint32)),sizeof(uint32));
                    SIDX_REVERSE_ENDIAN((uint8*)&nSAP_delta_time,sizeof(uint32));

                    m_bstarts_with_sap = (nSAP_delta_time & 0x80000000)?true:false;

                    nSAP_type = (nSAP_delta_time & 0x70000000) >> SIDX_SAP_DELTA_TIME_SIZE_BITS;

                    nSAP_delta_time &= 0x0FFFFFFF;
                    nSAP_delta_time = (uint32)(((uint64)nSAP_delta_time * MILLISEC_TIMESCALE_UNIT)/nscale);

                    /* Update the moof details only when SIDX points to moof
                    which means reference type should be 0 */
                    if(!breference_type)
                    {
                      if(!setup_memory("moof") || NULL == m_p_moof_info)
                      {
                        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,
                        "sidx_helper::parse_sidx(moof setup_memory failed..)");
                        break;
                      }
                      m_p_moof_info[nrefwriteindex].b_starts_with_sap = m_bstarts_with_sap;
                      m_p_moof_info[nrefwriteindex].b_reference_type = breference_type;
                      m_p_moof_info[nrefwriteindex].n_sap_type=nSAP_type;
                      m_p_moof_info[nrefwriteindex].n_sapdelta_time = nSAP_delta_time;
                      m_p_moof_info[nrefwriteindex].n_referenced_size = nnreferenced_size;
                      m_p_moof_info[nrefwriteindex].n_subsegment_duration = nsubsegment_duration;
                      m_p_moof_info[nrefwriteindex].n_sidxid = nsidxwriteindex;
                    }

#ifdef SIDX_PARSER_DEBUG
                    MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_HIGH, "sidx_helper::parse_sidx breference_type %d nnreferenced_size %ld",breference_type,nnreferenced_size);
                    MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_HIGH, "sidx_helper::parse_sidx nsubsegment_duration %ld, nSAP_type %d",nsubsegment_duration,nSAP_type);
                    MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_HIGH, "sidx_helper::parse_sidx m_bstarts_with_sap %d nSAP_delta_time %ld",m_bstarts_with_sap,nSAP_delta_time);
#endif
                    nrefoffset = 0;
                    if(i == FIRST_REF_INDEX)
                    {
                      nrefoffset = m_p_sidx_info[nsidxwriteindex].n_offset +
                        m_p_sidx_info[nsidxwriteindex].n_size   +
                        m_p_sidx_info[nsidxwriteindex].n_firstoffset;
                      nstarttime = m_p_sidx_info[nsidxwriteindex].n_earliestprestime;
                    }
                    else
                    {
                      if((breference_type) && (m_p_sidx_info[nsidxwriteindex].p_referenced_sidx))
                      {
                        nrefoffset = m_p_sidx_info[nsidxwriteindex].p_referenced_sidx[m_p_sidx_info[nsidxwriteindex].n_sidxpointed -1].nsize +
                        m_p_sidx_info[nsidxwriteindex].p_referenced_sidx[m_p_sidx_info[nsidxwriteindex].n_sidxpointed -1].offset;
                      }
                      else if(m_p_moof_info && nrefwriteindex)
                      {
                        nrefoffset = m_p_moof_info[nrefwriteindex-1].n_offset +
                        m_p_moof_info[nrefwriteindex-1].n_referenced_size;
                        nstarttime+= m_p_moof_info[nrefwriteindex-1].n_subsegment_duration;
                      }
                    }
                    if(!breference_type)
                    {
                      m_p_moof_info[nrefwriteindex].n_offset = nrefoffset;
                      m_p_moof_info[nrefwriteindex].n_start_time = nstarttime;
                      nrefwriteindex++;
                      if(m_p_sidx_info[nsidxwriteindex].n_moofstartid == -1)
                      {
                        m_p_sidx_info[nsidxwriteindex].n_moofstartid =
                          m_p_sidx_info[nsidxwriteindex].n_moofpointed;

                        /* Update MOOF start-id properly by using previous
                           SIDX info structure details. */
                        if (nsidxwriteindex)
                        {
                          m_p_sidx_info[nsidxwriteindex].n_moofstartid =
                            m_p_sidx_info[nsidxwriteindex - 1].n_moofpointed;
                          if (m_p_sidx_info[nsidxwriteindex - 1].n_moofstartid > 0)
                          {
                            m_p_sidx_info[nsidxwriteindex].n_moofstartid +=
                              m_p_sidx_info[nsidxwriteindex - 1].n_moofstartid;
                          }
                        }
                      }
                      m_p_sidx_info[nsidxwriteindex].n_moofpointed++;
                    }
                    else
                    {
                      //Infinite loop, breaks once SIDX atom is found!
                      do
                      {
                        /* All SIDX atoms need not be in continuous order.
                           Some other atoms such as SSIX can also be present
                           in between. But we need only SIDX atoms at present.
                           So to push only SIDX atoms into queue, read atom
                           type before pushing into Q.
                        */
                        /* All the data will always be available before
                           processing. so read will never be failed. */
                        nread = OSCL_FileSeekRead(m_pBuffer,
                                                  1,
                                                  SIDX_REF_ENTRY_DATA_SIZE,
                                                  pFile,
                                                  nrefoffset,
                                                  SEEK_SET);
                        if(0 == nread)
                        {
                          status = m_estatus = SIDX_PARSER_READ_FAILED;
                          MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_FATAL,
                                   "read is failied failed at offset %llu.",
                                   nrefoffset);
                        }

                        memcpy(&nnreferenced_size,m_pBuffer,sizeof(uint32));
                        SIDX_REVERSE_ENDIAN((uint8*)&nnreferenced_size,
                                            sizeof(uint32));
                        if (!memcmp(m_pBuffer+4, MOOF_SIGNATURE,MOOF_SIG_SIZE))
                        {
                          status = m_estatus = SIDX_PARSER_INTERNAL_FATAL_ERROR;
                          nrefoffset += nnreferenced_size;
                          ntotalsidxtobeparsed--;
                          break;
                        }
                        else if(memcmp(m_pBuffer+4,SIDX_SIGNATURE,SIDX_SIG_SIZE))
                        {
                          nrefoffset += nnreferenced_size;
                          continue;
                        }
                        else
                        {
                          break;
                        }
                      } while(1);
                      //we have new sidx which is not yet parsed,
                      //so add the offset to the q.
                      if (!memcmp(m_pBuffer + 4, SIDX_SIGNATURE, SIDX_SIG_SIZE))
                      {
                        push_in_q(nrefoffset);
                        (void)setup_sidx_indexed_memory(nsidxwriteindex,
                                                        nrefoffset,
                                                        nnreferenced_size);
                      }
                    }
                    m_noffset += SIDX_REF_ENTRY_DATA_SIZE;
                  }//for(i = 0; i<nrefcount;i++)

                  //we are done parsing this sidx, so remove the offset from q and mark it as done.
                  if (SIDX_PARSER_INTERNAL_FATAL_ERROR != m_estatus)
                  {
                    status = m_estatus = SIDX_PARSER_SIDX_BOX_DONE;
                  }

                  if(!remove_from_q(nstartoffset))
                  {
                    //if we fail to remove from q, we should report fatal error and stop parsing
                    //as failure to remove from q would never trigger parsing_done.
                    m_estatus = SIDX_PARSER_INTERNAL_FATAL_ERROR;
                    MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_FATAL,
                                 "remove_from_q %lld failed...",nstartoffset);
                  }

                  //check if there are more sidx to be parsed...
                  if((SIDX_PARSER_INTERNAL_FATAL_ERROR != m_estatus) &&
                     (is_sidx_parsing_done()) )
                  {
                    m_estatus = SIDX_PARSER_SIDX_PARSING_DONE;
                  }
                  else if(get_q_count() && (!pop_from_q(&m_noffset)) )
                  {
                    //if we fail to pop from q, we should report fatal error and stop parsing
                    //as failure to remove from q would never trigger parsing_done.
                    m_estatus = SIDX_PARSER_INTERNAL_FATAL_ERROR;
                    MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_FATAL,
                                 "pop_from_q %lld failed...", nstartoffset);
                  }
                }//if(m_p_allsidx_info)
              }//if(setup_sidx_memory(SIDX_SIGNATURE) && push_in_q(nstartoffset) )
            }//if(check_available_data(m_noffset,nsidxsize))
            else
            {
              MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_MEDIUM,
                "parse_sidx under-run: Need %ld bytes from offset %lld",
                nsidxsize,m_noffset);
              m_estatus = SIDX_PARSER_SIDX_PARSING_PENDING;
              break;
            }
          }//if(memcmp(m_pBuffer,SIDX_SIGNATURE,SIDX_SIG_SIZE)== 0)
          /* If at least one SIDX atom is not found before start of MOOF atom,
             we can assume that SIDX atom is not available in the file/buffer
             further.*/
          else if((!m_p_sidx_info) &&
                  (memcmp(m_pBuffer + MOOF_SIG_SIZE, MOOF_SIGNATURE,
                          MOOF_SIG_SIZE) == 0))
          {
            MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
                      "sidx_helper::No SIDX atom is found before first MOOF");
            m_estatus = SIDX_PARSER_SIDX_NOT_AVAILABLE;
            break;
          }
          /* If at least one SIDX atom is not found before start of TS segment,
             we can assume that SIDX atom is not available in the file/buffer
             further.*/
          else if(m_pBuffer[0] == 0x47)
          {
            MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
              "sidx_helper::No SIDX atom is found before first TS Segment");
            m_estatus = SIDX_PARSER_SIDX_NOT_AVAILABLE;
            break;
          }
          else
          {
            uint32 ulAtomSize = 0;
            memcpy(&ulAtomSize,m_pBuffer,SIDX_SIG_SIZE);
            SIDX_REVERSE_ENDIAN((uint8*)&ulAtomSize,SIDX_SIG_SIZE);
            m_noffset += ulAtomSize ;
          }
        }//if(nread)
        else
        {
          MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_FATAL,
                       "parse_sidx read %u failed @offset %llu",
                       SIDX_ALIGN_BOUNDARY,m_noffset);
          m_estatus = SIDX_PARSER_READ_FAILED;
          break;
        }
      }//if(check_available_data(m_noffset,nreqbytes))
      else
      {
        int64 sllOffset = 0;
        bool  bEOS      = false;
        m_pPort->GetAvailableOffset(&sllOffset, &bEOS);

        if((nreqbytes) && (false == bEOS))
        {
          MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM,
                      "parse_sidx read under-run....");
          m_estatus = SIDX_PARSER_SIDX_PARSING_PENDING;
          break;
        }
        else
        {
          status = m_estatus;
        }
      }
    }//while( (true) && (IS_STATUS_OK(m_estatus)) )
  }//if( (pFile) && (m_pPort) )
  status = m_estatus;
  return status;
}

bool  sidx_helper::is_lmsg_present()
{
  return m_bLmsgPresent;
}

/*!
  @brief    API to print the information about all sidx boxes and
            individual chunks pointed by each sidx box.
  @details
*/
 void  sidx_helper::walk_sidx_data()
 {
   MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_LOW, "\n------------------walk_sidx_data------------------\n");
   {
     MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_LOW, "\n#sidx parsed %u #sidx located in stream %u\n",nsidx,ntotalsidxtobeparsed);
     if(m_p_sidx_info)
     {
       for(unsigned int i = 0; i < nsidx; i++)
       {
         MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_LOW,
                      "\nPrinting #SIDX %u #REF CNT %u Information...\n",(i+1),m_p_sidx_info[i].n_refcount);
         MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_LOW,"SIDX offset %lu SIDX size %lu",m_p_sidx_info[i].n_offset,m_p_sidx_info[i].n_size);
         MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_LOW,"SIDX EarliestPresTime %llu SIDX FirstOffset %llu",m_p_sidx_info[i].n_earliestprestime, m_p_sidx_info[i].n_firstoffset);
         MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_LOW,"SIDX Version %u",m_p_sidx_info[i].n_version);
         MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_LOW,"SIDX Scale %u SIDX StreamID %u\n",m_p_sidx_info[i].n_scale,m_p_sidx_info[i].n_streamid);

         for(unsigned int j = 0; j < m_p_sidx_info[i].n_moofpointed; j++)
         {
           MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_LOW, "#REF %u",(j+1));

           MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_LOW,"Absolute FileOffset %llu",
             m_p_moof_info[j].n_offset);

           MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_LOW,"bSap %d ",
             m_p_moof_info[j].b_starts_with_sap);

           MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_LOW,"bReferenceType %d",
             m_p_moof_info[j].b_reference_type);

           MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_LOW,"nSAPType %d",
             m_p_moof_info[j].n_sap_type);

           MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_LOW,"SapDeltaTime %d",
             m_p_moof_info[j].n_sapdelta_time);

           MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_LOW,"Start Time for this Fragment %llu",
             m_p_moof_info[j].n_start_time);


           MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_LOW,"SubSegDuration %u",
             m_p_moof_info[j].n_subsegment_duration);

           MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_LOW, "------end-of-#REF %u------\n",(j+1));
         }
         MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_LOW,
                      "------end-of-#SIDX %u #REF CNT %u------\n",(i+1),m_p_sidx_info[i].n_refcount);
       }
     }
   }
   MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_LOW, "\n------------------end_walk_sidx_data------------------\n");
 }

/*!
  @brief    sidxparser destructor
  @details  destroys the sidx parser object
  */
sidx_helper::~sidx_helper()
{
  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_LOW, "sidx_helper::~sidx_helper");
  if(m_p_sidx_info)
  {
    for(unsigned int i = 0; i< nsidx;i++)
    {
      if(m_p_sidx_info[i].p_referenced_sidx)
      {
        MM_Free(m_p_sidx_info[i].p_referenced_sidx);
      }
    }
    MM_Free(m_p_sidx_info);
    m_p_sidx_info = NULL;
  }
  if(m_p_moof_info)
  {
    MM_Free(m_p_moof_info);
    m_p_moof_info = NULL;
  }
  if(m_pOffsetQ)
  {
    while(m_pOffsetQ->pHead)
    {
      sidx_offset_q_node* pnode = m_pOffsetQ->pHead;
      m_pOffsetQ->pHead = pnode->pNext;
      MM_Free(pnode);
    }
    MM_Free(m_pOffsetQ);
    m_pOffsetQ = NULL;
  }
  if(m_pBuffer)
  {
    MM_Free(m_pBuffer);
    m_pBuffer = NULL;
  }
  if(m_pFilePtr)
  {
    (void)OSCL_FileClose((OSCL_FILE*)m_pFilePtr);
    m_pFilePtr = NULL;
  }
}

//! To check data availability before parsing/reading
bool sidx_helper::check_available_data(unsigned long long nstartoffset,unsigned long nbytes)
{
  bool bret = false;
  if(m_pPort && m_navailoffset <= nstartoffset + nbytes)
  {
    int64 noffset = 0;
    bool bend = false;
    if(m_pPort->GetAvailableOffset(&noffset,&bend) == video::iStreamPort::DS_SUCCESS)
    {
      m_navailoffset = noffset;
      if( (nstartoffset+nbytes) < (uint64)noffset)
      {
        bret = true;
      }
      else if(bend)
      {
        bret = true;
      }
      if(nbytes == 0)
      {
        m_estatus = SIDX_PARSER_SIDX_PARSING_DONE;
        bret = false;
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH, "marking SIDX_PARSER_SIDX_PARSING_DONE...");
      }
    }
  }
  else if(m_pPort && m_navailoffset > nstartoffset + nbytes)
  {
    bret = true;
  }
  return bret;
}
//!setup memory used for storing sidx data pointed by a sidx
bool  sidx_helper::setup_sidx_indexed_memory(unsigned int index,unsigned long long offset,unsigned int nsize)
{
  bool bret = false;
  if(m_p_sidx_info )
  {
    if(!m_p_sidx_info[index].p_referenced_sidx)
    {
      m_p_sidx_info[index].p_referenced_sidx = (referenced_sidx*)MM_Malloc(sizeof(referenced_sidx));
      if(m_p_sidx_info[index].p_referenced_sidx)
      {
        memset(m_p_sidx_info[index].p_referenced_sidx,0,sizeof(referenced_sidx));
        m_p_sidx_info[index].p_referenced_sidx[m_p_sidx_info[index].n_sidxpointed].nsize = nsize;
        m_p_sidx_info[index].p_referenced_sidx[m_p_sidx_info[index].n_sidxpointed].offset = offset;
        m_p_sidx_info[index].n_sidxpointed++;
        bret = true;
      }
    }//if(!m_p_sidx_info[index].preferenced_sidx)
    else
    {
      referenced_sidx* ptemp = (referenced_sidx*)
        MM_Realloc(m_p_sidx_info[index].p_referenced_sidx,(sizeof(referenced_sidx)*(m_p_sidx_info[index].n_sidxpointed+1)));
      if(ptemp)
      {
        m_p_sidx_info[index].p_referenced_sidx = ptemp;
        m_p_sidx_info[index].p_referenced_sidx[m_p_sidx_info[index].n_sidxpointed].nsize = nsize;
        m_p_sidx_info[index].p_referenced_sidx[m_p_sidx_info[index].n_sidxpointed].offset = offset;
        m_p_sidx_info[index].n_sidxpointed++;
        bret = true;
      }
    }
  }
  return bret;
}

 //! To manage the memory allocation, reallocation
 bool sidx_helper::setup_memory(const char* item)
 {
   bool bret = false;
   if(item && (memcmp(item,SIDX_SIGNATURE,SIDX_SIG_SIZE) == 0) )
   {
     if(!m_p_sidx_info)
     {
       MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_LOW, "setup_memory for first sidx...");
       m_p_sidx_info = (sidx_data*)MM_Malloc(sizeof(sidx_data));
       if(m_p_sidx_info)
       {
         memset(m_p_sidx_info,0,sizeof(sidx_data));
         m_p_sidx_info->n_moofstartid = -1;
         nsidx++;
         ntotalsidxtobeparsed++;
         nsidxwriteindex = nsidx -1;
         bret = true;
         m_bmemoryAllocated = true;
       }
     }//if(!m_p_sidx_info)
     else
     {
       //make room for storing another sidx
       MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_LOW, "setup_memory making room for sidx %d",(nsidx+1));
       sidx_data* ptemp = (sidx_data*)MM_Realloc(m_p_sidx_info,sizeof(sidx_data)* (nsidx+1));
       if(ptemp)
       {
         m_p_sidx_info = ptemp;
         memset(m_p_sidx_info+nsidx,0,sizeof(sidx_data));
         m_p_sidx_info[nsidx].n_moofstartid = -1;
         nsidx++;
         nsidxwriteindex = nsidx -1;
         bret = true;
         m_bmemoryAllocated = true;
         MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_LOW, "setup_memory successfully made room for sidx..");
       }
     }
   }//if(item && (memcmp(item,SIDX_SIGNATURE,SIDX_SIG_SIZE) == 0) )

   if(item && (memcmp(item,MOOF_SIGNATURE,MOOF_SIG_SIZE) == 0) )
   {
     if(!m_p_moof_info)
     {
       MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_LOW, "setup_memory for first moof...");
       m_p_moof_info = (data_chunk_info*)MM_Malloc(sizeof(data_chunk_info));
       if(m_p_moof_info)
       {
         memset(m_p_moof_info,0,sizeof(data_chunk_info));
         nmoof++;
         nrefwriteindex= nmoof -1;
         bret = true;
       }
     }//if(!m_p_moof_info)
     else
     {
       //make room for storing another moof
       MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_LOW, "setup_memory making room for moof %d",(nmoof+1));
       data_chunk_info* ptemp = (data_chunk_info*)MM_Realloc(m_p_moof_info,sizeof(data_chunk_info)* (nmoof+1));
       if(ptemp)
       {
         m_p_moof_info = ptemp;
         memset(m_p_moof_info+nmoof,0,sizeof(data_chunk_info));
         nmoof++;
         nrefwriteindex= nmoof -1;
         bret = true;
         MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_LOW, "setup_memory successfully made room for moof..");
       }
     }
   }//if(item && (memcmp(item,SIDX_SIGNATURE,SIDX_SIG_SIZE) == 0) )

   if(!bret)
   {
     MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL, "setup_memory failed..aborting subsequent sidx parsing...");
     m_estatus = SIDX_PARSER_OUT_OF_MEMORY;
   }
   return bret;
 }

 //! Checks if all sidx boxes have been parsed
 bool sidx_helper::is_sidx_parsing_done()
 {
   bool bret = false;

   if(m_p_sidx_info)
   {
     MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_LOW, "is_sidx_parsing_done..");
     if((nsidx) && (ntotalsidxtobeparsed == nsidx) )
     {
       bret = true;
       MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM, "is_sidx_parsing_done returning true...");
     }
     if(!get_q_count() && m_p_sidx_info)
     {
       bret = true;
       MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_LOW, "Q count is 0-is_sidx_parsing_done returning true...");
     }
   }
   return bret;
 }
 //! Manage offset Q insertion
 bool sidx_helper::push_in_q(unsigned long long noffset)
 {
   bool bret = false;
   MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_LOW, "push_in_q %lld",noffset);
   if(!m_pOffsetQ)
   {
     m_pOffsetQ = (sidx_offset_q*)MM_Malloc(sizeof(sidx_offset_q));
     if(m_pOffsetQ)
     {
       memset(m_pOffsetQ,0,sizeof(sidx_offset_q));
       sidx_offset_q_node* pnode = (sidx_offset_q_node*)MM_Malloc(sizeof(sidx_offset_q_node));
       if(pnode)
       {
         pnode->nOffset = noffset;
         pnode->pNext = NULL;
         m_pOffsetQ->pHead = pnode;
         m_pOffsetQ->pLast = pnode;
         m_pOffsetQ->nCount++;
         bret = true;
         ntotalsidxtobeparsedCount++;
         MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_LOW, "push_in_q first item successful...");
       }
     }
   }//if(!m_pOffsetQ)
   else
   {
     sidx_offset_q_node* pnode = (sidx_offset_q_node*)MM_Malloc(sizeof(sidx_offset_q_node));
     if(pnode)
     {
       pnode->nOffset = noffset;
       pnode->pNext = NULL;
       if(m_pOffsetQ->pHead)
       {
         m_pOffsetQ->pLast->pNext = pnode;
         m_pOffsetQ->pLast = pnode;
         MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_LOW, "push_in_q <subsequent item> successful...");
       }
       else
       {
         m_pOffsetQ->pHead = pnode;
         m_pOffsetQ->pLast = pnode;
         MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_LOW, "push_in_q pHead was NULL, adding as the first item...");
       }
       ntotalsidxtobeparsedCount++;
       m_pOffsetQ->nCount++;
       bret = true;
     }//if(pnode)
   }
   return bret;
 }
 //! Manage offset Q removal
 bool sidx_helper::remove_from_q(unsigned long long noffset)
 {
   bool bret = false;
   MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_LOW, "remove_from_q %lld",noffset);
   if(m_pOffsetQ && m_pOffsetQ->nCount)
   {
     if(m_pOffsetQ->pHead)
     {
       if(m_pOffsetQ->pHead->nOffset == noffset)
       {
         MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_LOW, "remove_from_q successful...");
         sidx_offset_q_node* pnode = m_pOffsetQ->pHead;
         m_pOffsetQ->pHead = m_pOffsetQ->pHead->pNext;
         m_pOffsetQ->nCount--;
         ntotalsidxtobeparsedCount--;
         MM_Free(pnode);
         bret = true;
       }
       else
       {
         MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_LOW, "remove_from_q:NOT FIFO?????NEED TO CHECK...");
         //our model supports first in/first out.So, we should never come here...
         //might need to change it if need arises..
       }
     }
   }
   return bret;
 }
 //! Manage offset Q popping
 bool sidx_helper::pop_from_q(unsigned long long* poffset)
 {
   bool bret = false;
   MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_LOW, "pop_from_q");
   if(m_pOffsetQ && m_pOffsetQ->nCount && poffset)
   {
     if(m_pOffsetQ->pHead)
     {
      *poffset = m_pOffsetQ->pHead->nOffset;
      MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_LOW, "pop_from_q successful offset %lld",(*poffset));
      bret = true;
     }
   }
   return bret;
 }

 //! Returns count of queued items
 unsigned long sidx_helper::get_q_count()
 {
   unsigned long ncount = 0;
   if(m_pOffsetQ)
   {
     ncount = (unsigned long)m_pOffsetQ->nCount;
   }
   return ncount;
 }
