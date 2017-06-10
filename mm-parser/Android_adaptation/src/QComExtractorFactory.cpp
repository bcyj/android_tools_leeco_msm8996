/*
 * Copyright (c) 2010 - 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

//#define LOG_NDEBUG 0
#define LOG_TAG "QComExtractorFactory"
#include <utils/Log.h>
#include <utils/String8.h>
#include <cutils/properties.h>

#include "MMParserExtractor.h"
#include <utils/RefBase.h>
#include <media/stagefright/MediaDefs.h>
#include <QCMediaDefs.h>
#include <media/stagefright/DataSource.h>

namespace android {

extern "C" MediaExtractor* createExtractor(const sp<DataSource> &source, const char* mime) {
    if (!strcasecmp(mime, MEDIA_MIMETYPE_CONTAINER_AVI) ||
        !strcasecmp(mime, MEDIA_MIMETYPE_AUDIO_AC3) ||
        !strcasecmp(mime, MEDIA_MIMETYPE_CONTAINER_ASF) ||
        !strcasecmp(mime, MEDIA_MIMETYPE_AUDIO_AAC) ||
        !strcasecmp(mime, MEDIA_MIMETYPE_CONTAINER_QCP) ||
        !strcasecmp(mime, MEDIA_MIMETYPE_CONTAINER_3G2) ||
        !strcasecmp(mime, MEDIA_MIMETYPE_AUDIO_DTS) ||
        !strcasecmp(mime, MEDIA_MIMETYPE_AUDIO_DTS_LBR) ||
        !strcasecmp(mime, MEDIA_MIMETYPE_AUDIO_EAC3) ||
        !strcasecmp(mime, MEDIA_MIMETYPE_CONTAINER_MPEG4) ||
        !strcasecmp(mime, MEDIA_MIMETYPE_CONTAINER_QCMPEG4) ||
        !strcasecmp(mime, MEDIA_MIMETYPE_CONTAINER_QCMPEG2TS) ||
        !strcasecmp(mime, MEDIA_MIMETYPE_CONTAINER_QCMPEG2PS) ||
        !strcasecmp(mime, MEDIA_MIMETYPE_CONTAINER_QCOGG) ||
        !strcasecmp(mime, MEDIA_MIMETYPE_CONTAINER_QCMATROSKA) ||
        !strcasecmp(mime, MEDIA_MIMETYPE_CONTAINER_QCMPEG) ||
        !strcasecmp(mime, MEDIA_MIMETYPE_CONTAINER_QCAMR_NB) ||
        !strcasecmp(mime, MEDIA_MIMETYPE_CONTAINER_QCAMR_WB) ||
        !strcasecmp(mime, MEDIA_MIMETYPE_CONTAINER_QCFLV) ||
        !strcasecmp(mime, MEDIA_MIMETYPE_CONTAINER_QTIFLAC ) ||
        !strcasecmp(mime, MEDIA_MIMETYPE_CONTAINER_QCWAV)) {
        LOGV(" instantiate MMParserExtractor \n");
        return new MMParserExtractor(source, mime);
    }
    return NULL;
}

typedef enum
{
    Sniff_OGG,
    Sniff_AVI,
    Sniff_AC3,
    Sniff_ASF,
    Sniff_AMRNB,
    Sniff_AMRWB,
    Sniff_MKAV,
    Sniff_3GP,
    Sniff_QCP,
    Sniff_FLAC,
    Sniff_FLV,
    Sniff_MPEG2TS,
    Sniff_3G2,
    Sniff_MPEG2PS,
    Sniff_AAC,
    Sniff_MP3,
    Sniff_DTS,
    Sniff_WAV,
    MAX_SINFF_COUNT
}Sniff_codes;

//! DTS format can have sync marker within 64KB worth of data
#define MAX_FORMAT_HEADER_SIZE (64*1024)

#define FORMAT_HEADER_SIZE 1024
#define AAC_FORMAT_HEADER_SIZE 8202
#define MIN_HEADER_DATA_NEEDED 20

bool ExtendedSniff(const sp<DataSource> &source, String8 *mimeType, float *confidence, sp<AMessage> */*meta*/){

  LOGV("Sniff Start\n");
  bool retVal = false;

  FileSourceFileFormat formatToCheck;
  uint32_t buffSize   = FORMAT_HEADER_SIZE;
  uint8_t* headerBuff = (uint8_t*)malloc(MAX_FORMAT_HEADER_SIZE);
  uint8_t* pTempBuf   = NULL;
  FileSourceStatus status = FILE_SOURCE_INVALID;
  if(NULL == headerBuff) {
      LOGE("sniff fail: Malloc failed");
      return false;
  }

  //! Read 1024bytes worth of data in first attempt
  ssize_t retReadSize = source->readAt(0, headerBuff, FORMAT_HEADER_SIZE);
  //! Minimum 1024 bytes worth of data needs to be read
  if(retReadSize < (ssize_t)buffSize) {
      LOGE("Sniff FAIL :: coundn't pull enough data for sniffing");
      //! Free the buffer before exit
      if(headerBuff != NULL) {
          free(headerBuff);
          headerBuff = NULL;
      }
      return false;
  }
  uint32 nDataRead = (uint32)retReadSize;
  LOGV("amount of data read at start %lu", nDataRead);
  char property_value[PROPERTY_VALUE_MAX] = {0};
  int parser_flags = 0;
  property_get("mm.enable.qcom_parser", property_value, "0");
  parser_flags = atoi(property_value);
  LOGV("Parser_Flags == %x",parser_flags);
  for(int index=0; index<MAX_SINFF_COUNT; ++index)
  {
      bool foundMatching = false;
      switch(index)
      {
        case Sniff_AVI:
          if( PARSER_AVI & parser_flags ) {
            LOGV("SniffAVI ===>\n");
            formatToCheck = FILE_SOURCE_AVI;
            foundMatching = true;
          }
          break;

        case Sniff_AC3:
          if( PARSER_AC3 & parser_flags ) {
            LOGV("SniffAC3 ===>\n");
            formatToCheck = FILE_SOURCE_AC3;
            foundMatching = true;
          }
          break;

        case Sniff_ASF:
          if( PARSER_ASF & parser_flags ) {
            LOGV("SniffASF ===>\n");
            formatToCheck = FILE_SOURCE_ASF;
            foundMatching = true;
          }
          break;

        case Sniff_3GP:
          if( PARSER_3GP & parser_flags ) {
            LOGV("Sniff3GP ===>\n");
            formatToCheck = FILE_SOURCE_MPEG4;
            foundMatching = true;
          }
          break;

        case Sniff_OGG:
          if( PARSER_OGG & parser_flags ) {
            LOGV("SniffOGG ===>\n");
            formatToCheck = FILE_SOURCE_OGG;
            foundMatching = true;
          }
          break;

        case Sniff_MKAV:
          if( PARSER_MKV & parser_flags ) {
            LOGV("SniffMKAV ===>\n");
            formatToCheck = FILE_SOURCE_MKV;
            foundMatching = true;
          }
          break;

        case Sniff_FLV:
          if( PARSER_FLV & parser_flags ) {
            LOGV("SniffFLV ===>\n");
            formatToCheck = FILE_SOURCE_FLV;
            foundMatching = true;
          }
          break;

        case Sniff_QCP:
          if( PARSER_QCP & parser_flags ) {
            LOGV("SniffQCP ===>\n");
            formatToCheck = FILE_SOURCE_QCP;
            foundMatching = true;
          }
          break;

        case Sniff_AMRNB:
          if( PARSER_AMR_NB & parser_flags ) {
            LOGV("SniffAMRNB ===>\n");
            formatToCheck = FILE_SOURCE_AMR_NB;
            foundMatching = true;
          }
          break;

        case Sniff_AMRWB:
          if( PARSER_AMR_WB & parser_flags ) {
            LOGV("SniffAMRWB ===>\n");
            formatToCheck = FILE_SOURCE_AMR_WB;
            foundMatching = true;
          }
          break;

        case Sniff_AAC:
          if( PARSER_AAC & parser_flags) {
            LOGV("SniffAAC ===>\n");
            formatToCheck = FILE_SOURCE_AAC;
            foundMatching = true;
          }
          break;

        case Sniff_DTS:
          if( PARSER_DTS & parser_flags ) {
            LOGV("SniffDTS ===>\n");
            formatToCheck = FILE_SOURCE_DTS;
            foundMatching = true;
          }
          break;

        case Sniff_WAV:
          if( PARSER_WAV & parser_flags ) {
            LOGV("SniffWAV ===>\n");
            formatToCheck = FILE_SOURCE_WAV;
            foundMatching = true;
          }
          break;

        case Sniff_MP3:
          if( PARSER_MP3 & parser_flags ) {
            LOGV("SniffMP3 ===>\n");
            formatToCheck = FILE_SOURCE_MP3;
            foundMatching = true;
          }
          break;

         case Sniff_MPEG2TS:
          // By Default, QCOM MP2TS Parser is used in QRD branch
          if( PARSER_MP2TS & parser_flags ) {
            LOGV("SniffMP2TS ===>\n");
            formatToCheck = FILE_SOURCE_MP2TS;
            foundMatching = true;
          }
          break;

        case Sniff_3G2:
          if( PARSER_3G2 & parser_flags ) {
            LOGV("Sniff3G2 ===>\n");
            formatToCheck = FILE_SOURCE_3G2;
            foundMatching = true;
          }
          break;

        case Sniff_MPEG2PS:
          if( PARSER_MP2PS & parser_flags ) {
            LOGV("SniffMPEG2PS ===>\n");
            formatToCheck = FILE_SOURCE_MP2PS;
            foundMatching = true;
          }
          break;

        case Sniff_FLAC:
          if( PARSER_FLAC & parser_flags ) {
            LOGV("SniffFLAC ===>\n");
            formatToCheck = FILE_SOURCE_FLAC;
            foundMatching = true;
          }
          break;

        default:
          LOGW("Not matched with any sniff above, lets try other sniffs ");
          foundMatching = false;
          break;
     } //! switch(index)

     if(!foundMatching)
         continue;

#if defined(FEATURE_FILESOURCE_AAC) || defined(FEATURE_FILESOURCE_MP3)
     if(formatToCheck == FILE_SOURCE_AAC ||
        formatToCheck == FILE_SOURCE_MP3) {

         //! AAC and MP3 may have ID3 tags at the start of file
         //! Also we need to validate minimum three frames
         //! Read data only if it is less than 8k bytes
         if( nDataRead <  AAC_FORMAT_HEADER_SIZE) {
             retReadSize = source->readAt(0, headerBuff, AAC_FORMAT_HEADER_SIZE);
             nDataRead   = retReadSize;
             //! Minimum 1024 bytes worth of data needs to be read
             if(retReadSize < (ssize_t)buffSize) {
                 LOGE("Sniff FAIL :: coundn't pull enough data for sniffing");
                 break;
             }
         }

         status =  FileSource::CheckFileFormat(formatToCheck,headerBuff,
                                               &nDataRead);

         LOGV("Return status after 1st call %d, size %lu", status, nDataRead);
         if(FILE_SOURCE_DATA_NOTAVAILABLE == status)
         {
             //! Reallocate memory only if it is not sufficient
             if( nDataRead > MAX_FORMAT_HEADER_SIZE) {
                 pTempBuf = (uint8_t *)realloc(headerBuff,nDataRead);
                 if(NULL == pTempBuf) {
                     LOGE("AAC/MP3 sniff fail: Re-alloc failed");
                     break;
                 }
                 headerBuff = pTempBuf;
             }

             retReadSize = source->readAt(0,headerBuff,nDataRead);
             if(retReadSize < (ssize_t)nDataRead)
             {
                 LOGE("Sniff FAIL :: coundn't pull enough data for sniffing");
                 break;
             }
             nDataRead = retReadSize;
             //! Re-check for MP3/AAC one more time
             status =  FileSource::CheckFileFormat(formatToCheck,headerBuff,&nDataRead);
        } //! if(FILE_SOURCE_DATA_NOTAVAILABLE == status)
     }else
#endif //FEATURE_FILESOURCE_AAC
     if(formatToCheck == FILE_SOURCE_DTS)
     {
         //! DTS requires 65K worth of data to validate DTS sync marker
         retReadSize = source->readAt(0, headerBuff, MAX_FORMAT_HEADER_SIZE);
         nDataRead   = retReadSize;
         //! Min 1024 worth of data needs to be read
         if(retReadSize < (ssize_t)buffSize) {
             LOGE("Sniff FAIL :: coundn't pull enough data for sniffing");
             break;
         }
         status =  FileSource::CheckFileFormat(formatToCheck,headerBuff,&nDataRead);
     }
     else
     {
         status =  FileSource::CheckFileFormat(formatToCheck,headerBuff,&nDataRead);
         LOGV("Updated nDataRead value after format %d check is %lu",
              formatToCheck, nDataRead);
     }

     if(status == FILE_SOURCE_SUCCESS)
     {
         retVal = true;
         *confidence = 0.8;

         switch(formatToCheck)
         {
             case FILE_SOURCE_AVI:
                 *mimeType = MEDIA_MIMETYPE_CONTAINER_AVI;
                 LOGV(" SniffAVI success<=== \n");
                 break;

            case FILE_SOURCE_AC3:
                 *mimeType = MEDIA_MIMETYPE_AUDIO_AC3;
                 LOGV(" SniffAC3 success<=== \n");
                 break;

             case FILE_SOURCE_ASF:
                 *mimeType = MEDIA_MIMETYPE_CONTAINER_ASF;
                 LOGV(" SniffASF success<=== \n");
                 break;

             case FILE_SOURCE_QCP:
                 *mimeType = MEDIA_MIMETYPE_CONTAINER_QCP;
                 LOGV(" SniffQCP success<=== \n");
                 break;

             case FILE_SOURCE_AAC:
                 *mimeType = MEDIA_MIMETYPE_AUDIO_AAC;
                 LOGV(" SniffAAC success<=== \n");
                 break;

             case FILE_SOURCE_DTS:
                 *mimeType = MEDIA_MIMETYPE_AUDIO_DTS;
                 LOGV(" SniffDTS success<=== \n");
                 break;

             case FILE_SOURCE_AMR_NB:
                 *mimeType = MEDIA_MIMETYPE_CONTAINER_QCAMR_NB;
                 LOGV(" SniffAMRNB success<=== \n");
                 break;

             case FILE_SOURCE_AMR_WB:
                 *mimeType = MEDIA_MIMETYPE_CONTAINER_QCAMR_WB;
                 LOGV(" SniffAMRWB success<=== \n");
                 break;

             case FILE_SOURCE_WAV:
                 *mimeType = MEDIA_MIMETYPE_CONTAINER_QCWAV;
                 LOGE(" SniffWAV success<=== \n");
                 break;

             case FILE_SOURCE_OGG:
                 *mimeType = MEDIA_MIMETYPE_CONTAINER_QCOGG;
                 LOGV(" SniffOGG success<=== \n");
                 break;

             case FILE_SOURCE_MP3:
                 *mimeType = MEDIA_MIMETYPE_CONTAINER_QCMPEG;
                 LOGV(" SniffMP3 success<=== \n");
                 break;

             case FILE_SOURCE_MP2TS:
                 *mimeType = MEDIA_MIMETYPE_CONTAINER_QCMPEG2TS;
                 LOGV(" SniffMPEG2TS success<=== \n");
                 break;

             case FILE_SOURCE_MKV:
                 *mimeType = MEDIA_MIMETYPE_CONTAINER_QCMATROSKA;
                  LOGV(" SniffMKV success<=== \n");
                  break;

             case FILE_SOURCE_FLV:
                 *mimeType = MEDIA_MIMETYPE_CONTAINER_QCFLV;
                 LOGV(" SniffFLV success<=== \n");
                 break;

            case FILE_SOURCE_MP2PS:
                 *mimeType = MEDIA_MIMETYPE_CONTAINER_QCMPEG2PS;
                 LOGV(" SniffMPEG2PS success<=== \n");
                 break;

            case FILE_SOURCE_MPEG4:
                 *mimeType = MEDIA_MIMETYPE_CONTAINER_QCMPEG4;
                 LOGV(" Sniff3gp success<=== \n");
                 break;

             case FILE_SOURCE_3G2:
                 *mimeType = MEDIA_MIMETYPE_CONTAINER_3G2;
                 LOGV(" Sniff3G2 success<=== \n");
                 break;

             case FILE_SOURCE_FLAC:
                 *mimeType = MEDIA_MIMETYPE_CONTAINER_QTIFLAC;
                 LOGV(" SniffFLAC success<=== \n");
                 break;

             default:
                 LOGE("Something wrong, breaking ");
                 break;
         }
     } else {
         LOGV("Didn't match the sniff <=== status = %d, formattocheck %d ",status, formatToCheck);
         buffSize = FORMAT_HEADER_SIZE;   // making sure this is not over written
     }

     if(retVal)
        break;

  } //! for(int index=0; index<MAX_SINFF_COUNT; ++index)
  //! Free the buffer before exit
  if(headerBuff != NULL) {
     free(headerBuff);
     headerBuff = NULL;
  }

  return retVal;

}

#if 0
bool Sniff3G2(const sp<DataSource> &source, String8 *mimeType, float *confidence, sp<AMessage> */*meta*/){

  LOGV("Sniff3G2 Start\n");
  bool retVal = false;

  FileSourceFileFormat formatToCheck = FILE_SOURCE_3G2;
  uint32 buffSize= FORMAT_HEADER_SIZE;
  uint8_t headerBuff[buffSize];
  FileSourceStatus status = FILE_SOURCE_INVALID;

  ssize_t retSize = source->readAt(0, headerBuff, buffSize);
  if(retSize < (ssize_t)buffSize) {
      LOGE("Sniff FAIL :: coundn't pull enough data for sniffing");
      return false;
  }

  status =  FileSource::CheckFileFormat(formatToCheck,headerBuff,&buffSize);

  if(status == FILE_SOURCE_SUCCESS) {
      *mimeType = MEDIA_MIMETYPE_CONTAINER_3G2;
      LOGV("Sniff_3G2 success<=== \n");
      *confidence = 0.8; //bumping the confidence
      retVal = true;
  } else {
      LOGW("Didn't match the sniff3G2 <=== status = %d ",status);
  }

  return retVal;
}
#endif

const DataSource::SnifferFunc sniffers[] = {
    ExtendedSniff
};


extern "C" void snifferArray(const DataSource::SnifferFunc* snifferArray[], int *count) {
    (*snifferArray)=sniffers;
    *count = sizeof(sniffers)/sizeof(DataSource::SnifferFunc);
}

} //namespace android
