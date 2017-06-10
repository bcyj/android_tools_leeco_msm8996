/*
 * Copyright (c) 2012-2013 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

//#define LOG_NDEBUG 0
#define LOG_TAG "WFDSource"

#include "common_log.h"
#include <utils/Log.h>
#include <dlfcn.h>  // for dlopen/dlclose
#include <stdio.h>


#include "WFDSource.h"
#include "AnotherPacketSource.h"

#include <media/stagefright/foundation/ABuffer.h>
#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/foundation/AMessage.h>
#include <media/stagefright/MediaErrors.h>
#include <media/stagefright/MetaData.h>
#include <media/stagefright/MediaBuffer.h>
#include <media/stagefright/MediaSource.h>
#include <media/stagefright/MediaExtractor.h>
#include <media/stagefright/DataSource.h>

#include <media/stagefright/MediaDefs.h>
#include "parserdatadef.h"


namespace android {
 struct ExtendedDataSource;

int32_t getPortFromUrl(const char *urlPath) {
    int startIndex = strlen("rtp://wfd/");
    int endIndx = strlen(urlPath);

    LOGV("startIndex (%d)  endIndx(%d)",startIndex, endIndx);
    int port = 0;

    while(startIndex < endIndx) {
       if(urlPath[startIndex] >= '0' && urlPath[startIndex] <= '9') {
           port = port*10 + ((int)urlPath[startIndex] - (int)'0');
       }else if(urlPath[startIndex] == '/') {
           break;
       }
       ++startIndex;
    }

    return port;
}


DashPlayer::WFDSource::WFDSource(const char *urlPath                         ,
                               const KeyedVector<String8, String8> *headers ,
                               OMX_U32 &nReturn                             ,
                               bool bValidUid                               ,
                               uid_t nUid)
    : mEOS(false),
      mAudioEOS(false),
      mVideoEOS(false),
      m_pRTPStreamPort(NULL),
      m_pFileSource(NULL),
      mResponseReceived(false),
      mResponseAcknoledge(false),
      mOpenFileSourceStatus(FILE_SOURCE_OPEN_FAIL),
      mAudioTrackId(-1),
      mVideoTrackId(-1),
      mAudioTimescale(0),
      mVideoTimescale(0),
      mTracksCount(0),
      mVideoMaxBufferSize(0),
      mAudioMaxBufferSize(0),
      mPauseIndication (false),
      mFinalResult(OK) {
    // Create RTP Data source from the URL

    int32_t portNumber = getPortFromUrl(urlPath);

    status_t initStatus = OK;

    m_pRTPStreamPort = new RTPStreamPort(portNumber);

    if(m_pRTPStreamPort == NULL) {
        LOGE("RTP Datasource creation failed");
        return ;
    }

    CHECK(m_pRTPStreamPort != NULL);

    m_pRTPStreamPort->Start();

    //2 :: Create File Source
    m_pFileSource = new FileSource(fileSourceEventCallback, this);

    if(m_pFileSource == NULL) {
        LOGE("Extractor creation failed");
        delete m_pRTPStreamPort;
        m_pRTPStreamPort = NULL;
        return ;
    }

    if(m_pFileSource->OpenFile(m_pRTPStreamPort, FILE_SOURCE_MP2TS, true) != FILE_SOURCE_SUCCESS) {
        delete m_pRTPStreamPort;
        delete m_pFileSource;
        m_pRTPStreamPort = NULL;
        m_pFileSource = NULL;
        LOGV("@@@@:: File Source Open failed ");
        return;
    }else {
        LOGV("@@@@:: After FileSource Open ");
    }

    mVideoMetaData = NULL;
    mAudioMetaData = NULL;
    nReturn = OK;
    fp1 = NULL;
    fp2 = NULL;

}

void DashPlayer::WFDSource::writeToFile(bool audio, void* buffer,size_t size) {
    FILE* fp = NULL;

    if(audio) {
        if(fp2 == NULL) {
            fp2 = fopen("/data/audioBuff", "w+");
        }
        fp = fp2;
    } else {
        if(fp1 == NULL) {
            fp1 = fopen("/data/videoBuff", "w+");
        }
        fp = fp1;
    }

    if(fp != NULL) {
        fwrite(buffer,size,sizeof(uint8_t),fp );
    }
}

DashPlayer::WFDSource::~WFDSource() {

    LOGV("~WFDSource destructor ");
    if(m_pRTPStreamPort != NULL)
        delete m_pRTPStreamPort;

    if(m_pFileSource != NULL)
        delete m_pFileSource;

    /* delete all the objects which might have been created */
    // flush all the audio ABuffers from Audio Packet source
    // flush all the video ABuffers
    //smart pointer deletes the object we just return NULL
    //smart pointer deletes the object we just return NULL
    /* delete audio/video media source object, but before that we stop media sources */

}

void DashPlayer::WFDSource::fileSourceEventCallback(FileSourceCallBackStatus status, void *pClientData)
{
  LOGV("@@@@:fileSourceEventCallback with status %d", status);

  DashPlayer::WFDSource*  pWFDPlayer  = (DashPlayer::WFDSource*)pClientData;
  pWFDPlayer->setStatus(status);
}

void DashPlayer::WFDSource::setStatus(FileSourceCallBackStatus status)
{
    mOpenFileSourceStatus = status;
    mResponseReceived = true;
}
void DashPlayer::WFDSource::start() {
    LOGV("Start Called ");
    checkThenStart();
}

void DashPlayer::WFDSource::pause() {
    LOGV("pause Called ");
    CHECK(m_pRTPStreamPort != NULL);
    m_pRTPStreamPort->Pause();
    mPauseIndication = true;
}

void DashPlayer::WFDSource::resume() {
    LOGV("resume Called ");
    CHECK(m_pRTPStreamPort != NULL);
    m_pRTPStreamPort->Resume();
    mPauseIndication = false;
}

const char *MediaType2MIME(FileSourceMnMediaType minorType){
    LOGV("static function MediaType2MIME");
    switch (minorType) {

        case FILE_SOURCE_MN_TYPE_AC3:
            LOGV(" MediaType2MIME FILE_SOURCE_MN_TYPE_AC3 \n");
            return MEDIA_MIMETYPE_AUDIO_AC3;
        case FILE_SOURCE_MN_TYPE_AAC:
        case FILE_SOURCE_MN_TYPE_AAC_ADTS:
        case FILE_SOURCE_MN_TYPE_AAC_ADIF:
            LOGV(" MediaType2MIME FILE_SOURCE_MN_TYPE_AAC minor type %d\n", minorType);
            return MEDIA_MIMETYPE_AUDIO_AAC;
        case FILE_SOURCE_MN_TYPE_AAC_LOAS:
            LOGV(" MediaType2MIME FILE_SOURCE_MN_TYPE_AAC_LOAS minor type is not supported ");
            return NULL;
        case FILE_SOURCE_MN_TYPE_MPEG4:
            LOGV(" MediaType2MIME FILE_SOURCE_MN_TYPE_MPEG4 \n");
            return MEDIA_MIMETYPE_VIDEO_MPEG4;
        case FILE_SOURCE_MN_TYPE_H263:
            LOGV(" MediaType2MIME FILE_SOURCE_MN_TYPE_H263\n");
            return MEDIA_MIMETYPE_VIDEO_H263;
        case FILE_SOURCE_MN_TYPE_H264:
            LOGV(" MediaType2MIME FILE_SOURCE_MN_TYPE_H264\n");
            return MEDIA_MIMETYPE_VIDEO_AVC;
        case FILE_SOURCE_MN_TYPE_PCM:
            LOGV(" MediaType2MIME FILE_SOURCE_MN_TYPE_PCM \n");
            return MEDIA_MIMETYPE_AUDIO_RAW;
        default:
            LOGE("MediaType2MIME  minor type not supported, returns NULL string, caller should handle minor type %d", minorType);
            return NULL;
    }
}


status_t DashPlayer::WFDSource::createTracks() {

    LOGV("@@@@:: WFDSource: updateTracks");

    FileSourceTrackIdInfoType trackList[FILE_SOURCE_MAX_NUM_TRACKS];
    int32 numTracks = m_pFileSource->GetWholeTracksIDList(trackList);
    LOGV("@@@@:: --- createTracks --- numTracks (%d)",numTracks);

    for(int trackIndex = 0;trackIndex < numTracks; trackIndex++)
     {
        FileSourceMjMediaType majorType;
        FileSourceMnMediaType minorType;
        const char* pMimeType =NULL;
        FileSourceStatus status = m_pFileSource->GetMimeType(trackList[trackIndex].id, majorType, minorType);
        if (status == FILE_SOURCE_SUCCESS) {
            pMimeType = MediaType2MIME(minorType);
            if (NULL!= pMimeType) {
                LOGV("@@@@::readMetaData Valid Mime type =%s \n ",pMimeType);
            } else {
                if ((trackIndex == (numTracks - 1)) && !mTracksCount) {
                    LOGE("Error - No valid tracks - Un supported Minor Type = %d  \n",minorType);
                    return BAD_TYPE;
                } else {
                    LOGV("Un supported Minor Type = %d - Iterate through tracklist \n",minorType);
                    continue;
                }
                LOGE("@@@@::readMetaData Error Un supported Minor Type =%d  \n",minorType);
                mTracksCount = 0; // we dont want to support a subset of tracks, bail out and reset track count
                return BAD_TYPE;
            }
        }
        else {
            LOGE("@@@@::readMetaData failed in GetMimeType %d", status);
            return BAD_TYPE;
        }

       // get mediatrackinfo
       MediaTrackInfo mediaTrackInfo;
       status = m_pFileSource->GetMediaTrackInfo(trackList[trackIndex].id, &mediaTrackInfo);

       if(status != FILE_SOURCE_SUCCESS) {
          return BAD_TYPE;
       }

       if (mVideoMetaData == NULL && FILE_SOURCE_MJ_TYPE_VIDEO == trackList[trackIndex].majorType)
       {
         LOGV("WFDSource:createTracks:Updating Video track Id %d", trackList[trackIndex].id);
         mVideoTrackId = trackList[trackIndex].id;
         mVideoMetaData = new MetaData;
         mVideoMetaData->setCString(kKeyMIMEType, pMimeType);
         mVideoMetaData->setInt32(kKeyWidth, mediaTrackInfo.videoTrackInfo.frameWidth);
         mVideoMetaData->setInt32(kKeyHeight, mediaTrackInfo.videoTrackInfo.frameHeight);
         mVideoMetaData->setInt32(kKeyRotation, 0);     //To do - think about what to do here.
         mVideoTimescale = mediaTrackInfo.videoTrackInfo.timeScale;

         mVideoMaxBufferSize = 1.2 * mediaTrackInfo.videoTrackInfo.frameWidth * mediaTrackInfo.videoTrackInfo.frameHeight;
         LOGV("@@@@:: frameWidth(%d)  frameHeight(%d) mVideoMaxBufferSize(%d) ",mediaTrackInfo.videoTrackInfo.frameWidth,mediaTrackInfo.videoTrackInfo.frameHeight, mVideoMaxBufferSize);

         uint32 nFormatBlockSize = 0;
         //Get the size of the format block.
         status = m_pFileSource->GetFormatBlock(mVideoTrackId, NULL, &nFormatBlockSize);
         LOGV("getFormatBlock size = %lu \n", nFormatBlockSize);

         if((status==FILE_SOURCE_SUCCESS) && nFormatBlockSize!=0) {
             uint8_t *buffer = new uint8_t[nFormatBlockSize];
             if(buffer!=NULL) {
                 status = m_pFileSource->GetFormatBlock(mVideoTrackId, buffer, &nFormatBlockSize);

                 if(status==FILE_SOURCE_SUCCESS) {
                     mVideoMetaData->setData(kKeyRawCodecSpecificData, MetaData::TYPE_NONE, buffer, nFormatBlockSize);
                 } else {
                     LOGE(" getFormatBlock failed \n");
                 }
             }
             delete[] buffer;
         }

         if (m_pFileSource->IsDRMProtection()) {
             mVideoMetaData->setInt32(kKeyIsDRM, true);
         }
         ++mTracksCount;
       }
       else if (mAudioMetaData == NULL && FILE_SOURCE_MJ_TYPE_AUDIO == trackList[trackIndex].majorType)
       {
         LOGV("WFDSource:createTracks:Updating Audeo track Id %d", trackList[trackIndex].id);
         mAudioTrackId = trackList[trackIndex].id;
         mAudioMetaData = new MetaData;
         mAudioMetaData->setCString(kKeyMIMEType, pMimeType);
         mAudioMetaData->setInt32(kKeyChannelCount, mediaTrackInfo.audioTrackInfo.numChannels);
         mAudioMetaData->setInt32(kKeySampleRate, mediaTrackInfo.audioTrackInfo.samplingRate);
         mAudioMetaData->setInt32(kKeyBitRate, mediaTrackInfo.audioTrackInfo.bitRate);

         mAudioTimescale = mediaTrackInfo.audioTrackInfo.timeScale;

         mAudioMaxBufferSize = m_pFileSource->GetTrackMaxFrameBufferSize(trackList[trackIndex].id);
         LOGV("@@@@::createTracks: MIMEType(%s) ChannelCount(%d) SampleRate(%d) BitRate(%d)  MaxBufferSize(%d)",pMimeType,
               mediaTrackInfo.audioTrackInfo.numChannels,mediaTrackInfo.audioTrackInfo.samplingRate,
               mediaTrackInfo.audioTrackInfo.bitRate,mAudioMaxBufferSize);

         if( pMimeType == MEDIA_MIMETYPE_AUDIO_AAC ) {
             LOGE("Inside AAC block ");
             uint32 nFormatBlockSize = 0;
             status = m_pFileSource->GetFormatBlock(mAudioTrackId, NULL, &nFormatBlockSize);
             LOGE(" getFormatBlock size = %d \n", nFormatBlockSize);
             uint8_t *buffer = new uint8_t[nFormatBlockSize];
             if(buffer!=NULL) {

                 LOGV("@@@@^^^^:: getFormatBlock size = %lu \n", nFormatBlockSize);
                 for(int i = 0; i<nFormatBlockSize; i++ ){
                    LOGV("@@@@^^^^:: buffer[%d] = %c ",i,buffer[i]);
                 }

                 status = m_pFileSource->GetFormatBlock(mAudioTrackId, buffer, &nFormatBlockSize);
                 if(status==FILE_SOURCE_SUCCESS) {
                    LOGV("@@@@:: setting AAC format ");
                    mAudioMetaData->setData(kKeyAacCodecSpecificData, MetaData::TYPE_NONE, buffer, nFormatBlockSize);
                 } else {
                    LOGE(" getFormatBlock failed decoder config can fail, continue \n");
                 }
                 delete[] buffer;
                 buffer = NULL;
             }

             FileSourceConfigItem pitem;
             status_t configStatus;
             AacCodecData pCodecData;
             bool codecDataStatus = m_pFileSource->GetAACCodecData(mAudioTrackId,&pCodecData);
             if(codecDataStatus && pCodecData.eAACProfile == 4)
             {
                  LOGV("@@@@^^^^:: setting LTP profile to meta data");
                  mAudioMetaData->setInt32(kkeyAacFormatLtp, true);
             }

             if( minorType == FILE_SOURCE_MN_TYPE_AAC_ADIF ) {
                  LOGV("@@@@^^^^:: setting FILE_SOURCE_MEDIA_STRIP_AUDIO_HEADER, if ADIF");
                  mAudioMetaData->setInt32(kkeyAacFormatAdif, true);
                  configStatus = m_pFileSource->SetConfiguration(mAudioTrackId, &pitem, FILE_SOURCE_MEDIA_STRIP_AUDIO_HEADER);
             }
             else {
                  LOGV("@@@@^^^^:: setting kkeyAacFormatAdif = false");
                  mAudioMetaData->setInt32(kkeyAacFormatAdif, false);

                  if ( codecDataStatus ) {
                      // Set strip audio header config only if audio object type is
                      // FILE_SOURCE_AAC_PROFILE_LC or FILE_SOURCE_AAC_PROFILE_LTP
                      mAudioMaxBufferSize = 1536*2;
                      if( pCodecData.eAACProfile == 2 || pCodecData.eAACProfile == 4 )
                          LOGV("@@@@^^^^:: setting FILE_SOURCE_MEDIA_STRIP_AUDIO_HEADER, if eAACProfile is 2 or 4");
                          //configStatus = m_pFileSource->SetConfiguration(mAudioTrackId, &pitem, FILE_SOURCE_MEDIA_STRIP_AUDIO_HEADER );
                  } else {
                      LOGV("@@@@^^^^:: Error in determining Audio Object Type");
                  }
                  LOGV("@@@@^^^^:: setting FILE_SOURCE_MEDIA_OUTPUT_SINGLE_AUDIO_FRAME");
                  configStatus = m_pFileSource->SetConfiguration(mAudioTrackId, &pitem, FILE_SOURCE_MEDIA_OUTPUT_SINGLE_AUDIO_FRAME);
             }
             if( configStatus != FILE_SOURCE_SUCCESS ) {
                  LOGV("@@@@^^^^:: Error in setting AAC config %d", configStatus );
             }
          }else if( minorType == FILE_SOURCE_MN_TYPE_AC3 ) {
              FileSourceConfigItem pitem;
              status_t configStatus;
              configStatus = m_pFileSource->SetConfiguration(mAudioTrackId, &pitem, FILE_SOURCE_MEDIA_OUTPUT_SINGLE_AUDIO_FRAME);
              if( configStatus != FILE_SOURCE_SUCCESS ) {
                  LOGE("Setting AC3 config %d", configStatus );
              }
          }//end of if AC3
          ++mTracksCount;
       }
     }

    return OK;
}

sp<MetaData> DashPlayer::WFDSource::getFormat(int iMedia) {

    LOGV("@@@@:: WFDSource getFormat audio:%d", iMedia);
    sp<MetaData> meta = NULL;


    /* DashPlayer sends only true (for audio) and false (for video) to get the format */
    if (iMedia)
    {
        //Return Audio MetaData
        meta = mAudioMetaData;
    }
    else
    {
        // Return Video MetaData
        meta = mVideoMetaData;
    }

    return meta;
}

void DashPlayer::WFDSource::stop() {
      LOGV("@@@@:: WFDSource: Stop called");
      // TO-DO
      // @@@@:: ::Check if this can be sent to FileSource
      //m_pRTPStreamPort->Stop();
}

status_t DashPlayer::WFDSource::checkThenStart() {
    LOGV("@@@@:: checkThenStart :: mResponseReceived(%d) mResponseAcknoledge(%d)",mResponseReceived,mResponseAcknoledge);
    status_t retVal =  -EWOULDBLOCK;

    if(mResponseReceived) {
        if(mOpenFileSourceStatus == FILE_SOURCE_OPEN_COMPLETE) {
            mResponseAcknoledge = true;
            retVal = createTracks();
        } else if (mOpenFileSourceStatus == FILE_SOURCE_OPEN_FAIL) {
            //@@@@::  Have to close the DashPLayer here
            mResponseAcknoledge = true;
            retVal = UNKNOWN_ERROR;
        }
    }

    return retVal;
}

status_t DashPlayer::WFDSource::feedMoreTSData() {
    LOGV("@@@@:: WFDSource: feedMoreTSDats  -- mResponseReceived(%d)  mResponseAcknoledge(%d)",mResponseReceived,mResponseAcknoledge);

    status_t nReturn = OK;

    if(!mResponseReceived) {
        return OK;
    }

    if(mPauseIndication) {
        return -EWOULDBLOCK;
    }

    if(!mResponseAcknoledge) {
        nReturn = checkThenStart();

        if(nReturn != OK) {
            if(nReturn == -EWOULDBLOCK) {
                return OK;
            }else {
                LOGE("@@@@:: FeedMoreTSData:: checkThenStart returned error ... shutingdown the DashPLayer Session");
                return nReturn;
            }
        }
    }

    if(mAudioMetaData== NULL || mVideoMetaData== NULL) {
        nReturn = createTracks();

        if ((nReturn != OK ) && (nReturn != -EWOULDBLOCK))  {
            return nReturn;
        }
    }

    if(mAudioMetaData== NULL || mVideoMetaData== NULL) {
        return -EWOULDBLOCK;
    }

    //read audio
    if(mAudioMetaData != NULL) {
        nReturn = fillAudioPacketSource();
    }

    if((nReturn != OK) && (nReturn != -EWOULDBLOCK)){
        return nReturn;
    }

    // continue reading video frame
    if(mVideoMetaData != NULL) {
        nReturn = fillVideoPacketSource();
    }

    if((nReturn != OK) && (nReturn != -EWOULDBLOCK)) {
        return nReturn;
    }

    return OK;
}


status_t DashPlayer::WFDSource::dequeueAccessUnit(
        int iMedia, sp<ABuffer> *accessUnit) {
 LOGV("@@@@:: WFDSource: dequeueAccessUnit");

    if(mPauseIndication) {
        return -EWOULDBLOCK;
    }

    sp<AnotherPacketSource> packetSourceHndl = NULL;
    accessUnit->clear();

    //DashPlayer sends only true (for audio) and false (for video)
    if (iMedia){
        packetSourceHndl  =  mAudioPacketSource;
    }else {
        packetSourceHndl  =  mVideoPacketSource;
    }

    if (packetSourceHndl == NULL) {
        LOGE("WFDSource dequeueAccessUnit -> Audio or Video Another Packet Source not found");
        return -EWOULDBLOCK;
    }

    status_t finalResult;
    // if access unit is there @ packet source
    if (!packetSourceHndl->hasBufferAvailable(&finalResult)) {
        return finalResult == OK ? -EWOULDBLOCK : finalResult;
    }

    // return the Access unt
    finalResult =  packetSourceHndl->dequeueAccessUnit(accessUnit);

    if(!iMedia) {
        size_t size = (*accessUnit)->size();
        LOGV("@@@@:: packet Size is %d ", size);
    }

    return finalResult;

}


status_t DashPlayer::WFDSource::getDuration(int64_t *pDurationUs) {

     LOGV("@@@@:: WFDSource: getDuration");

    //Live stream , no duration
    pDurationUs = 0;
    return OK;
}

status_t DashPlayer::WFDSource::seekTo(int64_t nSeekTimeUs) {
    LOGV("@@@@:: WFDSource: seekTo");
    //Seek not allowed
    return INVALID_OPERATION;
}

bool DashPlayer::WFDSource::isSeekable() {
     LOGV("@@@@:: WFDSource: isSeekable");
    //RTP datasource is not seekable
    return false;
}

status_t DashPlayer::WFDSource::getNewSeekTime(int64_t* newSeekTime)
{
    LOGV("@@@@:: WFDSource: getNewSeekTime");

    return INVALID_OPERATION;
}

void DashPlayer::WFDSource::notifyRenderingPosition(int64_t nRenderingTS)
{
    //not needed
    LOGV("@@@@:: WFDSource: notifyRenderingPosition");
}

// WFDSource calss private function definition starts here
status_t DashPlayer::WFDSource::flush(bool audio)
{
   LOGV("@@@@:: WFDSource: flush");

   if (audio) {
      LOGV("Flushing Audio Packet source queue");
      sp<ABuffer> accessUnit;   // need to see how to handle codec change (INFO_DISCONTINEUTY)
      while(dequeueAccessUnit(audio,&accessUnit) == OK) {
         accessUnit = NULL;
      }
   }
   else {
      LOGE("Flushing Video Packet source queue");
      sp<ABuffer> accessUnit;    // need to see how to handle codec change (INFO_DISCONTINEUTY)
      while(dequeueAccessUnit(audio,&accessUnit) == OK) {
        accessUnit = NULL;
      }
   }
   return OK;

}

status_t DashPlayer::WFDSource::getAudioStream()
{

  status_t nReturn = UNKNOWN_ERROR;
  sp<ABuffer> accessUnit = new ABuffer(mAudioMaxBufferSize);

  FileSourceSampleInfo sampleInfo;
  size_t size = accessUnit->size();
  FileSourceMediaStatus mediaStatus = m_pFileSource->GetNextMediaSample(mAudioTrackId, accessUnit->data(), (uint32*)&size, sampleInfo);

  switch(mediaStatus)
  {
     case FILE_SOURCE_DATA_OK: // if access unit got
        //To-Do ::: Construct ABuffer here
        accessUnit->setRange(0, size);
        accessUnit->meta()->setInt64("timeUs", ((int64_t)sampleInfo.startTime * 1000000) / mAudioTimescale);// stage fright specific
        LOGV("@@@@::getAudioStream, timeUs(%lld) size(%d) capacity(%d)",((int64_t)sampleInfo.startTime * 1000000) / mAudioTimescale, size,mAudioMaxBufferSize);

        mAudioPacketSource->queueAccessUnit(accessUnit);
        nReturn = OK;
        break;

     case FILE_SOURCE_DATA_END: // if EOS is reached
        // EOS case
        LOGV("@@@@:: getAudioStream--> stream ended ");
        mAudioEOS = true;
        if(mVideoMetaData != NULL && !mEOS) {
          mEOS = mVideoEOS;
        }else {
          mEOS = mAudioEOS;
        }

        if (mEOS){
          mFinalResult = ERROR_END_OF_STREAM;
        }

        accessUnit = NULL;
        LOGE("Audio EOS reached: %d",mEOS);
        mAudioPacketSource->signalEOS(ERROR_END_OF_STREAM);
        nReturn = ERROR_END_OF_STREAM;
        break;

     case FILE_SOURCE_DATA_UNDERRUN:  // codec change is detected
        accessUnit = NULL;
        LOGE("FILE_SOURCE_DATA_UNDERRUN  received (%d) ",FILE_SOURCE_DATA_UNDERRUN);
        nReturn = FILE_SOURCE_DATA_UNDERRUN;
        break;

     case FILE_SOURCE_DATA_INSUFFICIENT:
        LOGE("@@@@ :FILE_SOURCE_DATA_INSUFFICIENT  received for Audio (%d) ",FILE_SOURCE_DATA_UNDERRUN);
        nReturn = FILE_SOURCE_DATA_UNDERRUN;
        break;

    case FILE_SOURCE_DATA_ERROR: // ABORT has been issued
    default:
        accessUnit = NULL;
        // signal error.. so that PB can be stopped
        LOGE("Audio Read Error (%d) , queueing ERROR_END_OF_STREAM for both the streams",nReturn);
        mAudioPacketSource->signalEOS(ERROR_END_OF_STREAM);
        mAudioEOS = true;

        if (mVideoPacketSource != NULL) {
           //mVideoEOS = true;
           //mVideoPacketSource->signalEOS(ERROR_END_OF_STREAM);
        }
        nReturn = UNKNOWN_ERROR;
        break;
  }

  return nReturn;
}


status_t DashPlayer::WFDSource::getVideoStream()
 {
   //LOGV("@@@@:: WFDSource: getVideoStream ");
   status_t nReturn = OK;
   sp<ABuffer> accessUnit = new ABuffer(mVideoMaxBufferSize);
   FileSourceSampleInfo sampleInfo;
   size_t size = accessUnit->capacity();
   FileSourceMediaStatus mediaStatus = m_pFileSource->GetNextMediaSample(mVideoTrackId, accessUnit->data(), (uint32*)&size, sampleInfo);
   static bool startTime = false;
   bool fillBuffer = true;
   int64_t timeUs = 0;
   LOGV("@@@@:: getVideoStream --> DATA_OK :: timeUs (%lld) size(%d)",((int64_t)sampleInfo.startTime * 1000000) / mVideoTimescale,size);

   switch(mediaStatus)
   {
      case FILE_SOURCE_DATA_OK: // if access unit got
        //To-Do ::: Construct ABuffer here
        timeUs = ((int64_t)sampleInfo.startTime * 1000000) / mVideoTimescale;
        if(timeUs == 0){
            if(!startTime) {
                startTime = true;
            }else {
                fillBuffer = false;
            }
        }

        if(fillBuffer) {
            accessUnit->meta()->setInt64("timeUs", ((int64_t)sampleInfo.startTime * 1000000) / mVideoTimescale);// stage fright specific
            accessUnit->setRange(0,size);
            accessUnit->meta()->setInt32("bytesLost",sampleInfo.nBytesLost);
            mVideoPacketSource->queueAccessUnit(accessUnit);

            LOGV("@@@@:: getVideoStream --> Video data pushed in another Packet source");
            //writeToFile(false, (void*)accessUnit->data(), size);
        }
        nReturn = OK;
        break;

      case FILE_SOURCE_DATA_END: // if EOS is reached
         // EOS case
         LOGV("@@@@:: getVideoStream--> stream ended ");
         mVideoEOS = true;
         if(mAudioMetaData != NULL && !mEOS) {
           mEOS = mAudioEOS;
         }else {
           mEOS = mVideoEOS;
         }

         if (mEOS){
           mFinalResult = ERROR_END_OF_STREAM;
         }

         accessUnit = NULL;
         LOGE("Video EOS reached: %d",mEOS);
         mVideoPacketSource->signalEOS(ERROR_END_OF_STREAM);
         nReturn = ERROR_END_OF_STREAM;
         break;

      case FILE_SOURCE_DATA_UNDERRUN:  // codec change is detected
         accessUnit = NULL;
         LOGE("FILE_SOURCE_DATA_UNDERRUN  received (%d) ", FILE_SOURCE_DATA_UNDERRUN);
         nReturn = FILE_SOURCE_DATA_UNDERRUN;
         break;

      case FILE_SOURCE_DATA_INSUFFICIENT:
         LOGE("@@@@: FILE_SOURCE_DATA_INSUFFICIENT  received for Video (%d) ", FILE_SOURCE_DATA_UNDERRUN);
         nReturn =  FILE_SOURCE_DATA_UNDERRUN;
         break;

      case FILE_SOURCE_DATA_ERROR: // ABORT has been issued
      default:

         accessUnit = NULL;
         // signal error.. so that PB can be stopped
         LOGE("Video Read Error (%d) , queueing ERROR_END_OF_STREAM for both the streams",nReturn);
         mVideoPacketSource->signalEOS(ERROR_END_OF_STREAM);
         mVideoEOS = true;

         if (mAudioPacketSource != NULL) {
            //mAudioEOS = true;
            //mAudioPacketSource->signalEOS(ERROR_END_OF_STREAM);
         }
         nReturn = UNKNOWN_ERROR;
         break;
   }

   return nReturn;
 }


status_t DashPlayer::WFDSource::fillAudioPacketSource()
{
 //LOGV("@@@@:: WFDSource: fillAudioPacketSource");

    status_t retVal = OK;

    sp<MetaData> meta = NULL;

    // if audio is available from the content then read the data from audio media source
    meta = mAudioMetaData;

    if (meta != NULL) {
      if (mAudioPacketSource == NULL) {
         // !warning : Make sure to update the meta information with in Another packet source when CODEC Change happens
         mAudioPacketSource = new AnotherPacketSource(meta);
      }

      int count = 0;
      if(mAudioPacketSource->getQueueSize()> 5)
           return OK;

      do {
          retVal = getAudioStream();
      } while(retVal == OK && count++ < 5);

      if ( (retVal == INFO_DISCONTINUITY) ||
           (retVal == ERROR_IO) || (retVal == FILE_SOURCE_DATA_UNDERRUN)) {
          LOGE("nQueueStatus : %d", retVal);
          // we can continue the playback for that stream
          retVal = OK;
      }

    }
    meta = NULL;
    return retVal;
}

status_t DashPlayer::WFDSource::fillVideoPacketSource()
{
    //LOGV("@@@@:: WFDSource: fillVideoPacketSource");

    status_t retVal = OK;

    sp<MetaData> meta = NULL;

    // if video is available from the content then read the data from video media source
    meta = mVideoMetaData;
    if (meta != NULL) {
      if (mVideoPacketSource == NULL) {
        mVideoPacketSource = new AnotherPacketSource(meta);
      }


      // queues the frame into Video Packet source
      int count = 0;

      if(mVideoPacketSource->getQueueSize()> 2)
          return OK;

      do {
          retVal = getVideoStream();
      }while(retVal == OK && count ++ <2);

      if ( (retVal == INFO_DISCONTINUITY) ||
           (retVal == ERROR_IO) || (retVal == FILE_SOURCE_DATA_UNDERRUN)) {
          LOGV("@@@@:: ret Val is  : %d", retVal);
          // we can continue the playback of the stream
          retVal = OK;
      }

    }
    meta = NULL;
    return retVal;
}

}
