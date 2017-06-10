
LOCAL_CFLAGS :=             \
    -D_ANDROID_             \
    -DDIVXINT_USE_STDINT    \
# add DIVXINT_NATIVE_64BIT?
LOCAL_CFLAGS +=             \
   -DFEATURE_FILESOURCE_AAC \
   -DFEATURE_FILESOURCE_QCP \
   -DFEATURE_FILESOURCE_3GP_PARSER\
   -DFEATURE_FILESOURCE_WAVADPCM\
   -DFEATURE_FILESOURCE_AC3\
   -DFEATURE_FILESOURCE_MP3\
   -DFEATURE_FILESOURCE_AMR\
   -DFEATURE_FILESOURCE_AMRWB\
   -DFEATURE_FILESOURCE_OGG_PARSER\
   -DFEATURE_FILESOURCE_MKV_PARSER\
   -DFEATURE_FILESOURCE_FLV_PARSER\
   -DFEATURE_FILESOURCE_DTS \
   -DFEATURE_FILESOURCE_AVI \
   -DFEATURE_FILESOURCE_MPEG2_PARSER

LOCAL_SRC_FILES:=                                    \
    AMRNBParserLib/src/amrfile.cpp                   \
    AMRNBParserLib/src/amrformatparser.cpp           \
    AMRWBParserLib/src/amrwbfile.cpp                 \
    AMRWBParserLib/src/amrwbparser.cpp               \
    AVIParserLib/src/avifile.cpp                     \
    AVIParserLib/src/aviparser.cpp                   \
    Android_adaptation/src/MMParserExtractor.cpp     \
    Android_adaptation/src/QComExtractorFactory.cpp  \
    Android_adaptation/src/SourcePort.cpp            \
    AC3ParserLib/src/ac3file.cpp                     \
    DTSParserLib/src/dtsfile.cpp                     \
    FileSource/src/filesource.cpp                    \
    FileSource/src/filesourcehelper.cpp              \
    FileBaseLib/src/oscl_file_io.cpp                 \
    FileBaseLib/src/filebase.cpp                     \
    FileBaseLib/src/filesourcestring.cpp             \
    FileBaseLib/src/utf8conv.cpp                     \
    FileBaseLib/src/ztl.cpp                          \
    FileBaseLib/src/qcplayer_oscl_utils.cpp          \
    FileBaseLib/src/os_fs_layer.cpp                  \
    FileBaseLib/src/zrex_string.cpp                  \
    FlacParserLib/src/flacfile.cpp                   \
    FlacParserLib/src/FlacParser.cpp                 \
    FLVParserLib/src/flvfile.cpp                     \
    FLVParserLib/src/flvparser.cpp                   \
    ID3Lib/src/id3.cpp                               \
    ISOBaseFileLib/src/atom.cpp                      \
    ISOBaseFileLib/src/atomutils.cpp                 \
    ISOBaseFileLib/src/boxrecord.cpp                 \
    ISOBaseFileLib/src/cencatoms.cpp                 \
    ISOBaseFileLib/src/dcmduserdataatom.cpp          \
    ISOBaseFileLib/src/fontrecord.cpp                \
    ISOBaseFileLib/src/fonttableatom.cpp             \
    ISOBaseFileLib/src/fullatom.cpp                  \
    ISOBaseFileLib/src/kddiuserdataatom.cpp          \
    ISOBaseFileLib/src/mp4fragmentfile.cpp           \
    ISOBaseFileLib/src/mpeg4file.cpp                 \
    ISOBaseFileLib/src/pdcfatoms.cpp                 \
    ISOBaseFileLib/src/sampleentry.cpp               \
    ISOBaseFileLib/src/stylerecord.cpp               \
    ISOBaseFileLib/src/telop.cpp                     \
    ISOBaseFileLib/src/textsampleentry.cpp           \
    ISOBaseFileLib/src/textsamplemodifiers.cpp       \
    ISOBaseFileLib/src/tsml.cpp                      \
    ISOBaseFileLib/src/tsmlparse.cpp                 \
    ISOBaseFileLib/src/udtaatoms.cpp                 \
    MKAVParserLib/src/mkavfile.cpp                   \
    MKAVParserLib/src/mkavparser.cpp                 \
    MP3ParserLib/src/mp3file.cpp                     \
    MP3ParserLib/src/mp3metadata.cpp                 \
    MP3ParserLib/src/mp3parser.cpp                   \
    MP3ParserLib/src/mp3vbrheader.cpp                \
    OGGParserLib/src/OGGStream.cpp                   \
    OGGParserLib/src/OGGStreamParser.cpp             \
    SIDXParserLib/src/sidxparser.cpp                 \
    VideoFMTReaderLib/src/videofmt.c                 \
    VideoFMTReaderLib/src/videofmt_bs.c              \
    VideoFMTReaderLib/src/videofmt_mp4r_parse.c      \
    VideoFMTReaderLib/src/videofmt_mp4r_read.c       \
    SeekTableLib/src/seektable.cpp                   \
    SeekLib/src/seek.cpp                             \
    WAVParserLib/src/wavfile.cpp                     \
    WAVParserLib/src/wavformatparser.cpp             \
    WAVParserLib/src/CAdpcmDecoderLib.cpp

LOCAL_SRC_FILES +=                                   \
    AACParserLib/src/aacfile.cpp                     \
    AACParserLib/src/aacmetadata.cpp                 \
    AACParserLib/src/aacparser.cpp                   \
    QCPParserLib/src/qcpfile.cpp                     \
    QCPParserLib/src/qcpparser.cpp                   \
    MP2ParserLib/src/DataBits.cpp                    \
    MP2ParserLib/src/MP2Stream.cpp                   \
    MP2ParserLib/src/MP2StreamParser.cpp             \
    MP2ParserLib/src/PESParser.cpp                   \
    MP2ParserLib/src/TSHeaderParser.cpp              \
    MP2ParserLib/src/PSHeaderParser.cpp              \
    FileBaseLib/src/H264HeaderParser.cpp             \


LOCAL_C_INCLUDES:=                                                      \
    $(LOCAL_PATH)/HTTPSource/MAPI08/API                                 \
    $(LOCAL_PATH)/AMRNBParserLib/inc                                    \
    $(LOCAL_PATH)/AMRWBParserLib/inc                                    \
    $(LOCAL_PATH)/AVIParserLib/inc                                      \
    $(LOCAL_PATH)/MP3ParserLib/inc                                      \
    $(LOCAL_PATH)/ID3Lib/inc                                            \
    $(LOCAL_PATH)/SeekLib/inc                                           \
    $(LOCAL_PATH)/AC3ParserLib/inc                                      \
    $(LOCAL_PATH)/DTSParserLib/inc                                      \
    $(LOCAL_PATH)/SeekTableLib/inc                                      \
    $(LOCAL_PATH)/FileSource/inc                                        \
    $(LOCAL_PATH)/FileBaseLib/inc                                       \
    $(LOCAL_PATH)/FlacParserLib/inc                                     \
    $(LOCAL_PATH)/FLVParserLib/inc                                      \
    $(LOCAL_PATH)/MKAVParserLib/inc                                     \
    $(LOCAL_PATH)/OGGParserLib/inc                                      \
    $(LOCAL_PATH)/SIDXParserLib/inc                                     \
    $(LOCAL_PATH)/ISOBaseFileLib/inc                                    \
    $(LOCAL_PATH)/VideoFMTReaderLib/inc                                 \
    $(LOCAL_PATH)/WAVParserLib/inc                                      \
    $(LOCAL_PATH)/../mm-osal/inc                                        \
    $(LOCAL_PATH)/../common/inc                                         \
    $(TARGET_OUT_HEADERS)/mm-core/omxcore

LOCAL_C_INCLUDES+=                                                      \
    $(LOCAL_PATH)/AACParserLib/inc                                      \
    $(LOCAL_PATH)/QCPParserLib/inc                                      \
    $(LOCAL_PATH)/MP2ParserLib/inc                                      \


LOCAL_SHARED_LIBRARIES :=   \
    libutils                \
    libcutils               \
    libdl                   \
    libstagefright          \
    libmmosal

ifneq (1,$(filter 1,$(shell echo "$$(( $(PLATFORM_SDK_VERSION) >= 21 ))")))
LOCAL_CFLAGS += -DFEATURE_ENABLE_ID3_ENCODING_CONVERSION
endif

LINK_ASFPARSERLIB :=false
LINK_ASFPARSERLIB := $(shell if [ -d $(LOCAL_PATH)/Api/ASFParserLib_API ] ; then echo true; fi)

ifeq ($(strip $(LINK_ASFPARSERLIB)),true)

LOCAL_CFLAGS +=             \
   -DFEATURE_FILESOURCE_WINDOWS_MEDIA

LOCAL_STATIC_LIBRARIES +=   \
    libASFParserLib

LOCAL_C_INCLUDES +=                                                      \
    $(LOCAL_PATH)/Api/ASFParserLib_API

endif #LINK_ASFPARSERLIB

LOCAL_PRELINK_MODULE:= false
