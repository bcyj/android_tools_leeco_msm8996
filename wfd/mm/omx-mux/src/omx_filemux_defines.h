#ifndef _MMI_FILEMUX_DEFINES_H_
#define _MMI_FILEMUX_DEFINES_H_

/*==============================================================================
*        @file omx_filemux_defines.h
*
*  @par DESCRIPTION:
*       This is the macro definitions for the MMI interface for File Mux
*
*
*
*  Copyright (c) 2011,2014 Qualcomm Technologies, Inc. All Rights Reserved.
*  Qualcomm Technologies Proprietary and Confidential.
==============================================================================*/

/*==============================================================================

                      EDIT HISTORY FOR FILE

 $Header: //depot/asic/msmshared/users/sateesh/multimedia2/Video/wfd-source/omx/mm-mux/omxmux/src/omx_filemux_defines.h#1 $


when        who         what, where, why
--------    ------         --------------------------------------------------------

================================================================================
*/

/*******************************************************************************
*                         INCLUDE FILES FOR MODULE
********************************************************************************
*/


/*******************************************************************************
*         MACRO DEFINITION FOR THE CURRENT IMPLEMENTATION OF MMI INTERFACE
********************************************************************************
*/
#define        CONF_TEST

/**-----------------------------------------------------------------------------
          Right now one video port and one audio port supported
--------------------------------------------------------------------------------
*/
#define        NUM_AUDIO_PORT             1
#define        NUM_VIDEO_PORT             1
#define        NUM_TEXT_PORT              0

/**-----------------------------------------------------------------------------
          We dont have any pipeline to fill so these can be 0
--------------------------------------------------------------------------------
*/
#define        NUM_AUDIO_BUFFERS          20
#define        MAX_AUDIO_QUEUE_LEN        NUM_AUDIO_BUFFERS - 1
#define        NUM_VIDEO_BUFFERS          4
#define        NUM_TEXT_BUFFERS           0

#define        AV_SYNC_LIMIT              10000000

/**-----------------------------------------------------------------------------
      Macro definitions for Mallocs and free
--------------------------------------------------------------------------------
*/
#define        OMX_MUX_MAX_SUPPORTED_ROLES    13
#define        OMX_FILEMUX_MALLOC(x)    MM_Malloc((x))
#define        OMX_FILEMUX_FREEIF(x)    if((x))                                \
                                        MM_Free((x));                          \
                                        x = NULL;
#define        OMX_MUX_MEM_SET(x,y,z)    memset(x,y,z)
#define        OMX_MUX_MEM_COPY(x,y,z)   memcpy(x,y,z)
/*------------------------------------------------------------------------------
    Macro returns the port type from port number
--------------------------------------------------------------------------------
*/
#define        GET_PORT_TYPE(x) (x < NUM_AUDIO_PORT  ?                         \
                                OMX_MUX_INDEX_PORT_AUDIO :                     \
                                (x < NUM_AUDIO_PORT + NUM_VIDEO_PORT) ?        \
                                OMX_MUX_INDEX_PORT_VIDEO:                      \
                                OMX_MUX_INDEX_PORT_TEXT)

/*------------------------------------------------------------------------------
    This should be updated when any video codec support added
--------------------------------------------------------------------------------
*/
#define        VALIDATE_VIDEO_FORMAT(x) ((x) == OMX_VIDEO_CodingH263    ||     \
                                         (x) == OMX_VIDEO_CodingMPEG4   ||     \
                                         (x) == OMX_VIDEO_CodingAVC)
/*------------------------------------------------------------------------------
    This should be updated when any audio codec support added
--------------------------------------------------------------------------------
*/
#define        VALIDATE_AUDIO_FORMAT(x) ((x) == OMX_AUDIO_CodingAAC     ||     \
                                         (x) == OMX_AUDIO_CodingAMR     ||     \
                                         (x) == OMX_AUDIO_CodingQCELP13 ||     \
                                         (x) == OMX_AUDIO_CodingEVRC    ||     \
                                         (x) == OMX_AUDIO_CodingPCM    ||     \
                                         (x) == OMX_AUDIO_CodingG711)


#define       VALIDATE_PORT_INDEX_AND_RETURN(x)  if((x) > (NUM_AUDIO_PORT +   \
                                                            NUM_VIDEO_PORT +  \
                                                            NUM_TEXT_PORT)    \
                                                    ) return OMX_ErrorNoMore;


/**----------------------------------------------------------------------------
    Utility to check if a particular port is enabled
--------------------------------------------------------------------------------
*/
#define        IS_PORT_ENABLED(x)       ((arrPortConfig + (x) )->              \
                                                  sPortDef.bEnabled == OMX_TRUE)

/**-----------------------------------------------------------------------------
    Returns the current codecs that is set on a port
--------------------------------------------------------------------------------
*/
#define        VIDEO_COMPRESSION_FORMAT ((arrPortConfig +                      \
                                                  OMX_MUX_INDEX_PORT_VIDEO)->  \
                                       sPortDef.format.video.eCompressionFormat)

#define        AUDIO_COMPRESSION_FORMAT ((arrPortConfig +                      \
                                                  OMX_MUX_INDEX_PORT_AUDIO)->  \
                                                sPortDef.format.audio.eEncoding)

/**-----------------------------------------------------------------------------
    Utility to get the current frame rate and bitrate set,
    returns a default value.
--------------------------------------------------------------------------------
*/
#define        GET_VIDEO_FRAMERATE     (((arrPortConfig +                      \
                                                  OMX_MUX_INDEX_PORT_VIDEO)->  \
                                          sPortDef.format.video.xFramerate)  ? \
                                        ((arrPortConfig +                      \
                                                  OMX_MUX_INDEX_PORT_VIDEO)->  \
                                   sPortDef.format.video.xFramerate >> 16) : 30)


#define        GET_VIDEO_BITRATE       (((arrPortConfig +                      \
                                                  OMX_MUX_INDEX_PORT_VIDEO)->  \
                                          sPortDef.format.video.nBitrate) ?    \
                                        ((arrPortConfig +                      \
                                                  OMX_MUX_INDEX_PORT_VIDEO)->  \
                                            sPortDef.format.video.nBitrate) :  \
                                                  512000)


#define        IS_VIDEO_PORT_USED       (((VIDEO_COMPRESSION_FORMAT            \
                                               != OMX_VIDEO_CodingUnused)      \
                                          && IS_PORT_ENABLED                   \
                                              (OMX_MUX_INDEX_PORT_VIDEO)) ?    \
                                                       OMX_TRUE  :  OMX_FALSE)

#define        IS_AUDIO_PORT_USED      (((AUDIO_COMPRESSION_FORMAT             \
                                               != OMX_AUDIO_CodingUnused)      \
                                          && IS_PORT_ENABLED                   \
                                              (OMX_MUX_INDEX_PORT_AUDIO)) ?    \
                                                       OMX_TRUE  :  OMX_FALSE)


/**-----------------------------------------------------------------------------
   Convert enums from OMX to FileMux
   Defaulting to 7 which is level-70 if we didn't get any level from IL client
--------------------------------------------------------------------------------
*/
#define     GET_H263_LEVEL_INDEX(level)  (level == OMX_VIDEO_H263Level10 ? 0 : \
                                          level == OMX_VIDEO_H263Level20 ? 1 : \
                                          level == OMX_VIDEO_H263Level30 ? 2 : \
                                          level == OMX_VIDEO_H263Level40 ? 3 : \
                                          level == OMX_VIDEO_H263Level45 ? 4 : \
                                          level == OMX_VIDEO_H263Level50 ? 5 : \
                                          level == OMX_VIDEO_H263Level60 ? 6 : \
                                          7)

#define     GET_AVC_LEVEL_INDEX(level)   (level == OMX_VIDEO_AVCLevel1   ? 0 : \
                                          level == OMX_VIDEO_AVCLevel1b  ? 1 : \
                                          level == OMX_VIDEO_AVCLevel11  ? 2 : \
                                          level == OMX_VIDEO_AVCLevel12  ? 3 : \
                                          level == OMX_VIDEO_AVCLevel13  ? 4 : \
                                          level == OMX_VIDEO_AVCLevel2   ? 5 : \
                                          level == OMX_VIDEO_AVCLevel21  ? 6 : \
                                          level == OMX_VIDEO_AVCLevel22  ? 7 : \
                                          level == OMX_VIDEO_AVCLevel3   ? 8 : \
                                          level == OMX_VIDEO_AVCLevel31  ? 9 : \
                                          level == OMX_VIDEO_AVCLevel32  ? 10: \
                                          level == OMX_VIDEO_AVCLevel4   ? 11: \
                                          level == OMX_VIDEO_AVCLevel41  ? 12: \
                                          level == OMX_VIDEO_AVCLevel42  ? 13: \
                                          level == OMX_VIDEO_AVCLevel5   ? 14: \
                                          level == OMX_VIDEO_AVCLevel51  ? 15: \
                                          0)


#define     GET_AVC_PROFILE_INDEX(profile)                                     \
                              (profile == OMX_VIDEO_AVCProfileBaseline   ? 0 : \
                               profile == OMX_VIDEO_AVCProfileMain       ? 1 : \
                               profile == OMX_VIDEO_AVCProfileExtended   ? 2 : \
                               profile == OMX_VIDEO_AVCProfileHigh       ? 3 : \
                               profile == OMX_VIDEO_AVCProfileHigh10     ? 4 : \
                               profile == OMX_VIDEO_AVCProfileHigh422    ? 5 : \
                               profile == OMX_VIDEO_AVCProfileHigh444    ? 6 : \
                               0)

#define     GET_AAC_SAMPLING_RATE_INDEX(nBitrate) (nBitrate == 96000    ? 0  : \
                                          nBitrate     ==     88200      ? 1 : \
                                          nBitrate     ==     64000      ? 2 : \
                                          nBitrate     ==     48000      ? 3 : \
                                          nBitrate     ==     44100      ? 4 : \
                                          nBitrate     ==     32000      ? 5 : \
                                          nBitrate     ==     24000      ? 6 : \
                                          nBitrate     ==     22050      ? 7 : \
                                          nBitrate     ==     16000      ? 8 : \
                                          nBitrate     ==     12000      ? 9 : \
                                          nBitrate     ==     11025      ? 10: \
                                          nBitrate     ==     8000       ? 11: \
                                          0)

#define     VALIDATE_FILE_FORMAT(nBrand) ((nBrand) == QOMX_FORMAT_MP4 ||       \
                                          (nBrand) == QOMX_FORMATMPEG_TS ||    \
                                          (nBrand) == QOMX_FORMAT_3GP ||       \
                                          (nBrand) == QOMX_FORMAT_3G2 ||       \
                                          (nBrand) == QOMX_FORMAT_AMC ||       \
                                          (nBrand) == QOMX_FORMAT_SKM ||       \
                                          (nBrand) == QOMX_FORMAT_K3G ||       \
                                          (nBrand) == QOMX_FORMAT_AMR ||       \
                                          (nBrand) == QOMX_FORMAT_AAC ||       \
                                          (nBrand) == QOMX_FORMAT_EVRC ||      \
                                          (nBrand) == QOMX_FORMAT_QCP ||       \
                                          (nBrand) == QOMX_FORMAT_WAVE )

#define     GET_FILE_FORMAT(nFmt)       (((nFmt) == QOMX_FORMAT_MP4 ||        \
                                          (nFmt) == QOMX_FORMAT_3GP ||        \
                                          (nFmt) == QOMX_FORMAT_3G2 ||        \
                                          (nFmt) == QOMX_FORMAT_AMC ||        \
                                          (nFmt) == QOMX_FORMAT_SKM ||        \
                                          (nFmt) == QOMX_FORMAT_K3G) ?        \
                                          MUX_FMT_MP4 :                       \
                                          (nFmt) == QOMX_FORMAT_AAC ?         \
                                          MUX_FMT_AAC :                       \
                                          (nFmt) == QOMX_FORMAT_EVRC?         \
                                          (AUDIO_COMPRESSION_FORMAT ==        \
                                           (OMX_AUDIO_CODINGTYPE)QOMX_AUDIO_CodingEVRCB ?           \
                                          MUX_FMT_EVRCB :                     \
                                          (AUDIO_COMPRESSION_FORMAT ==        \
                                           (OMX_AUDIO_CODINGTYPE)QOMX_AUDIO_CodingEVRCWB ?          \
                                          MUX_FMT_EVRCWB : MUX_FMT_INVALID)): \
                                          (nFmt) == QOMX_FORMAT_QCP?          \
                                          MUX_FMT_QCP:                        \
                                          (nFmt) == QOMX_FORMAT_AMR?          \
                                          MUX_FMT_AMR:                        \
                                          (nFmt) == QOMX_FORMAT_WAVE?         \
                                          MUX_FMT_WAV :                       \
                                          (nFmt) == QOMX_FORMATMPEG_TS?       \
                                          MUX_FMT_MP2 : MUX_FMT_INVALID)


#define     VOCODER_BITRATE_13K_FULL      14000
#define     VOCODER_BITRATE_13K_HALF      6800
#define     VOCODER_BITRATE_EVRC          9200

#define     VOCODER_PACKET_SIZE_13K_FULL  35
#define     VOCODER_PACKET_SIZE_13K_HALF  17
#define     VOCODER_PACKET_SIZE_EVRC      23
#define     VOCODER_PACKET_SIZE_AMR_1220  32  /* 12.2 kbps mode */
#define     VOCODER_PACKET_SIZE_AMR_1020  27  /* 10.2 kbps mode */
#define     VOCODER_PACKET_SIZE_AMR_0795  21  /* 7.95 kbps mode */
#define     VOCODER_PACKET_SIZE_AMR_0740  20  /* 7.4 kbps mode */
#define     VOCODER_PACKET_SIZE_AMR_0670  18  /* 6.7 kbps mode */
#define     VOCODER_PACKET_SIZE_AMR_0590  16  /* 5.9 kbps mode */
#define     VOCODER_PACKET_SIZE_AMR_0515  14  /* 5.15 kbps mode */
#define     VOCODER_PACKET_SIZE_AMR_0475  13  /* 4.75 kbps mode */

#define     AAC_BITS_PER_SAMPLE_DIVISOR   4096



/**-----------------------------------------------------------------------------
    Macro definitions of the init params for file mux layer
--------------------------------------------------------------------------------
*/
#ifdef FEATURE_7X27_SPECIFIC
#define        OUTPUT_UNIT_SIZE           8192   /**< Size of units in which to
                                                  * write to file
                                                  */
#else  // FEATURE_7X27_SPECIFIC
#define        OUTPUT_UNIT_SIZE           65536   /**< Size of units in which to
                                                  * write to file
                                                  */
#endif // FEATURE_7X27_SPECIFIC

#ifndef MAX
#define         MAX( x, y ) ( ((x) > (y)) ? (x) : (y) )
#endif

#ifndef MIN
#define         MIN( x, y ) ( ((x) < (y)) ? (x) : (y) )
#endif


#define        GET_SAMPLE_DELTA(x,y,z) ((((x) - (y)) * (z) + (1000000 >> 1)) / \
                                          1000000)/*TBD, OMX spec says timestamp
                                                   is in units of microsecs*/


#define        AVC_DEFAULT_PROFILE_COM        0x80
#define        AVC_L1b_PROFILE_COM            0x90

#define        MAX_H263_SUPP_LEVELS           8    /**< Num of supported levels
                                                     *  for h263
                                                     */
#define        MAX_AVC_SUPP_LEVELS            16   /**< Num of supported levels
                                                     *   for AVC
                                                     */

#define        DEFAULT_FRAGMENT_DURATION_MS   15000/**< Fragment duration of 15
                                                    * sec
                                                    */
#define        DEAFULT_INTERLACE_PERIOD_MS    3000 /**<Duration in ms after
                                                    * which audio and video has
                                                    * to be interlaced
                                                    */
#define        MMI_MAX_SAMPLES_PER_STORE      16000
#define        MMI_MAX_STREAM_DURATION        (3600 * 3)

#define        NUM_AUDIO_FORMAT_SUPPORTED     6
#define        NUM_VIDEO_FORMAT_SUPPORTED     6

#define        MMI_VIDEO_STREAM_NUM           1
#define        MMI_AUDIO_STREAM_NUM           0

#define        DEFAULT_AUDIO_BUFFER_SIZE      1024
#define        DEFAULT_VIDEO_BUFFER_SIZE      1024  /**< Worst case buffer size
                                                    *  for video
                                                    */
#define        DEFAULT_TEXT_BUFFER_SIZE        0


/**-----------------------------------------------------------------------------
   These are the number of seconds warning that the engine will attempt to
   give the client in two stages before the recording limit, either running
   out of sample table memory or filesystem space, is reached during
   recording.
--------------------------------------------------------------------------------
*/
#define        LIMIT_NEAR_THRESHOLD           10
#define        LIMIT_IMMINENT_THRESHOLD       3

#define        LIMIT_NEAR_THRESHOLD_LOW       5
#define        LIMIT_IMMINENT_THRESHOLD_LOW   1

#define        LIMIT_NEAR_THRESHOLD_MED       20
#define        LIMIT_IMMINENT_THRESHOLD_MED   6

/**-----------------------------------------------------------------------------
    Highest space limit threshold value.
--------------------------------------------------------------------------------
*/
#define       LIMIT_NEAR_THRESHOLD_HIGH       40
#define       LIMIT_IMMINENT_THRESHOLD_HIGH   12

#define       OPTIMAL_CHUNK_DURATION       (GET_VIDEO_BITRATE >  8000000 ? 1  :\
                                            GET_VIDEO_BITRATE >  4000000 ? 2  :\
                                            GET_VIDEO_BITRATE >  2000000 ? 3  :\
                                            3)   /**< Chunk duration in s */
#define        DESIRED_INTERLACE_RATE      (OPTIMAL_CHUNK_DURATION * 1000)
                                                    /**< Interlace duration  */
#define        OPTIMAL_STREAM_BUF_SIZE_FACTOR 2     /**< Lets allocate stream
                                                     * buffer  twice the chunk
                                                     * size
                                                     */
#define       FRAGMENT_TABLE_SIZE_FACTOR     110
#define       DEFAULT_SAMPLES_TABLE_SIZE     16000
#define       DEFAULT_CHUNKS_TABLE_SIZE      600

#define       GET_VIDEO_CHUNKS_TABLE_SIZE(space_limit_threshold)               \
              (                                                                \
                  MAX (DEFAULT_CHUNKS_TABLE_SIZE,                              \
                      ((space_limit_threshold) * 1000)                         \
                       / DESIRED_INTERLACE_RATE  + 1)                          \
              )



/**----------------------------------------------------------------------------

-------------------------------------------------------------------------------
*/
#define        OPTIMAL_CHUNK_SIZE(bitrate)    ((bitrate + 4) / 8) *           \
                                              OPTIMAL_CHUNK_DURATION
                                                  /**< Size in bytes to process
                                                   * and output to file as a
                                                   * unit
                                                   */
#define        MAX_HEADER_SIZE                1024
#define        MAX_FOOTER_SIZE                1024

#define        STSC_ALGO_RESET_RATE           5   /** restart stsc compression
                                                   * at 5
                                                   */

#define        VOCODER_SAMPLING_RATE          8000 /**< Speech codec samplerate
                                                    */


#define        VOCODER_PACKET_RATE            50   /**< Num of packets per sec
                                                    */

#define        QCP_FILE_HEADER_SIZE           162

#define        AAC_SAMPLES_PER_FRAME          1024

#define        AC3_SAMPLES_PER_FRAME          1536

#define        MIN_AUDIO_MUX_BUFFER_SIZE      24000

#define        STANDARD_UUID_SIZE             16

#define        OMX_MUX_CMD_GET_STD_OMX_PARAM  0xf1
#define        OMX_MUX_CMD_SET_STD_OMX_PARAM  0xf2

#endif /*_MMI_FILEMUX_DEFINES_H_*/



#define OMX_MUX_CMD_BASE       ( 0x40000000 )       /**<
                                                * Base value which need to be
                                                * added in all the command
                                                * codes defined below.
                                                * NOTE: Not to be referred
                                                * directly by the client.
                                                */

/**
 *
 * Purpose:-
 *       This command is used to Set / Get OMX parameter & configurations
 *       into / out of the device.
 *
 *       Many OMX parameter and config settings / queries cannot be handled by
 *       the base class and base class will ask the driver to respond. This
 *       command provides opportunity to the device layer to handle the
 *       SetParameter / GetParameter and SetConfig / GetConfig APIs.
 *
 *       Most technology or media related parameters will be handed over to
 *       device layer.
 *
 * Command type:-
 *       Synchronous
 *
 * Associated command input data type:-
 *       MMI_OmxParamCmdType
 *
 * Command return type:-
 *       MMI_S_COMPLETE on success.
 *       Other error codes as appropriate on failure.
 *
 * Asynchronous respone message code:-
 *       NA
 *
 */


/**
 *
 * Commands for buffer requirements negotiation:
 *
 * CASE A: When external allocator allocates the input buffers:-
 *
 *         Negotiation should be carried out with following commands.
 *        1) OMX_MUX_CMD_GET_CUSTOM_PARAM  - To get driver's buffer requirements.
 *        2) OMX_MUX_CMD_SET_CUSTOM_PARAM  - To set final external allocator's
 *                                       requirement.
 *        3) OMX_MUX_CMD_USE_BUFFER        - To informs the driver which buffers
 *                                       are being used.
 *
 * CASE B: When driver allocates the input buffers:-
 *         Negotiation should be carried out with following commands.
 *        1) OMX_MUX_CMD_GET_CUSTOM_PARAM  - To get driver's buffer requirements.
 *        2) OMX_MUX_CMD_SET_CUSTOM_PARAM  - To set the collective requirements.
 *        3) OMX_MUX_CMD_ALLOC_BUFFER      - To let driver allocate memory.
 *
 * By default driver considers that an external allocator will be used.
 * OMX_MUX_CMD_ALLOC_BUFFER will instruct the driver to be the allocator.
 *
 */

/**
 *
 * Purpose:-
 *       This command is used to Set / Get parameters & configurations defined
 *       in this header file which are not identified by a unique OMX param
 *       index but are domain or technology specific and are required by the
 *       component to set into or query from the device. These parameters are
 *       identified by the customized indexes and structures defined in this
 *       header file.
 *       e.g. OMX port definition format specific parameters can not be
 *       handled by the base class and should be passed to device for action
 *       on these.
 *
 * Command type:-
 *       Synchronous
 *
 * Associated command input data type:-
 *       MMI_CustomParamCmdType
 *
 * Command return type:-
 *       MMI_S_COMPLETE on success.
 *       Other error codes as appropriate on failure.
 *
 * Asynchronous respone message code:-
 *       NA
 *
 */
#define OMX_MUX_CMD_SET_CUSTOM_PARAM             ( OMX_MUX_CMD_BASE + 4 )
#define OMX_MUX_CMD_GET_CUSTOM_PARAM             ( OMX_MUX_CMD_BASE + 5 )

/**
 *
 * Purpose:-
 *       This command is used to get buffer allocation. This command requests
 *       a single buffer allocation from the driver memory.
 *
 * Command type:-
 *       Synchronous
 *
 * Associated command input data type:-
 *       MMI_AllocBufferCmdType
 *
 * Command return type:-
 *       MMI_S_COMPLETE on success.
 *       Other error codes as appropriate on failure.
 *
 * Asynchronous respone message code:-
 *       NA
 *
 */
#define OMX_MUX_CMD_ALLOC_BUFFER                 ( OMX_MUX_CMD_BASE + 8 )

/**
 *
 * Purpose:-
 *       This command is used to free allocated memory. This command is the
 *       counterpart of OMX_MUX_CMD_ALLOC_BUFFER and must be used only on a buffer
 *       that is allocated using this command.
 *
 * Command type:-
 *       Synchronous
 *
 * Associated command input data type:-
 *       MMI_FreeBufferCmdType
 *
 * Command return type:-
 *       MMI_S_COMPLETE on success.
 *       Other error codes as appropriate on failure.
 *
 * Asynchronous respone message code:-
 *       NA
 *
 */
#define OMX_MUX_CMD_FREE_BUFFER                  ( OMX_MUX_CMD_BASE + 10 )

/**
 *
 * Purpose:-
 *       This command is used to pass buffers allocated by the client which
 *       would be used in the data path during the session.
 *
 * Command type:-
 *       Synchronous
 *
 * Associated command input data type:-
 *       MMI_UseBufferCmdType
 *
 * Command return type:-
 *       MMI_S_COMPLETE on success.
 *       Other error codes as appropriate on failure.
 *
 * Asynchronous respone message code:-
 *       NA
 *
 */
#define OMX_MUX_CMD_USE_BUFFER                   ( OMX_MUX_CMD_BASE + 12 )

/**
 *
 * Purpose:-
 *       This command is used as a notification to driver to be prepared to
 *       enable and populating the ports with buffers. Resources may be
 *       reserved but need to be committed only when an LOAD command is
 *       issued. If LOAD command is already done then the additional
 *       allocations should be done at the end of the command.
 *
 * Command type:-
 *       Asynchronous / Synchronous
 *
 * Associated command input data type:-
 *       MMI_PortCmdType
 *
 * Command return type:-
 *       MMI_S_PENDING on asynchronous execution.
 *       MMI_S_COMPLETE on successful synchronous execution.
 *       Other error codes as appropriate on failure.
 *
 * Asynchronous respone message code:-
 *       MMI_RESP_ENABLE_PORT
 *
 */
#define OMX_MUX_CMD_ENABLE_PORT                  ( OMX_MUX_CMD_BASE + 14 )

/**
 *
 * Purpose:-
 *       This command is used as a notification to driver to be prepared to
 *       disable and depopulating the ports. At the end of this command any
 *       additional resources allocated for this port should be deallocated.
 *
 * Command type:-
 *       Asynchronous / Synchronous
 *
 * Associated command input data type:-
 *       MMI_PortCmdType
 *
 * Command return type:-
 *       MMI_S_PENDING on asynchronous execution.
 *       MMI_S_COMPLETE on successful synchronous execution.
 *       Other error codes as appropriate on failure.
 *
 * Asynchronous respone message code:-
 *       MMI_RESP_DISABLE_PORT
 *
 */
#define OMX_MUX_CMD_DISABLE_PORT                 ( OMX_MUX_CMD_BASE + 16 )

/**
 *
 * Purpose:-
 *       This command is used to initialize and start the module with the
 *       currently set configuration. On successful exection of this command
 *       the module device would start processing the buffers. Device would
 *       queue any buffer processing command while start command is being
 *       processed.
 *
 * Command type:-
 *       Asynchronous / Synchronous
 *
 * Associated command input data type:-
 *       None
 *
 * Command return type:-
 *       MMI_S_PENDING on asynchronous execution.
 *       MMI_S_COMPLETE on successful synchronous execution.
 *       Other error codes as appropriate on failure.
 *
 * Asynchronous respone message code:-
 *       MMI_RESP_START
 *
 */
#define OMX_MUX_CMD_START                        ( OMX_MUX_CMD_BASE + 18 )

/**
 *
 * Purpose:-
 *       This command is used to stop the current module. If there are pending
 *       buffers in the pipeline, those would be ignored and not processed
 *       before stopping the module.
 *       This command would not clear the properties set currently. If the
 *       client wants to restart the session it should call OMX_MUX_CMD_START to
 *       reinitialize the module before submitting any buffer for processing.
 *
 * Command type:-
 *       Asynchronous / Synchronous
 *
 * Associated command input data type:-
 *       None
 *
 * Command return type:-
 *       MMI_S_PENDING on asynchronous execution.
 *       MMI_S_COMPLETE on successful synchronous execution.
 *       Other error codes as appropriate on failure.
 *
 * Asynchronous respone message code:-
 *       MMI_RESP_STOP
 *
 */
#define OMX_MUX_CMD_STOP                         ( OMX_MUX_CMD_BASE + 20 )

/**
 *
 * Purpose:-
 *       This command is used to pause the current session. If there are
 *       pending buffers in the pipeline, those would be put on hold and
 *       be processed once the session is resumed. Device would queue any
 *       buffer processing commands while paused.
 *
 * Command type:-
 *       Asynchronous / Synchronous
 *
 * Associated command input data type:-
 *       None
 *
 * Command return type:-
 *       MMI_S_PENDING on asynchronous execution.
 *       MMI_S_COMPLETE on successful synchronous execution.
 *       Other error codes as appropriate on failure.
 *
 * Asynchronous respone message code:-
 *       MMI_RESP_PAUSE
 *
 */
#define OMX_MUX_CMD_PAUSE                        ( OMX_MUX_CMD_BASE + 22 )

/**
 *
 * Purpose:-
 *       This command is used to resume a paused session.
 *
 * Command type:-
 *       Asynchronous / Synchronous
 *
 * Associated command input data type:-
 *       None
 *
 * Command return type:-
 *       MMI_S_PENDING on asynchronous execution.
 *       MMI_S_COMPLETE on successful synchronous execution.
 *       Other error codes as appropriate on failure.
 *
 * Asynchronous respone message code:-
 *       MMI_RESP_RESUME
 *
 */
#define OMX_MUX_CMD_RESUME                       ( OMX_MUX_CMD_BASE + 24 )

/**
 *
 * Purpose:-
 *       This command is used to pass an input buffer which need to processed
 *       and consumed by the device. If external allocator is being used it
 *       should allocate the pool identifier.
 *
 * Command type:-
 *       Asynchronous
 *
 * Associated command input data type:-
 *       MMI_BufferCmdType
 *
 * Command return type:-
 *       MMI_S_PENDING on asynchronous execution.
 *       MMI_S_COMPLETE is not applicable.
 *       Other error codes as appropriate on failure.
 *
 * Asynchronous respone message code:-
 *       MMI_RESP_EMPTY_THIS_BUFFER
 *
 */
#define OMX_MUX_CMD_EMPTY_THIS_BUFFER            ( OMX_MUX_CMD_BASE + 26 )

/**
 *
 * Purpose:-
 *       This command is used to pass an output buffer which need to processed
 *       and filled with data by the device. Once this buffer is passed, the
 *       device takes over the buffer management till the buffer is returned
 *       back by via fill buffer done response.
 *
 * Command type:-
 *       Asynchronous
 *
 * Associated command input data type:-
 *       MMI_BufferCmdType
 *
 * Command return type:-
 *       MMI_S_PENDING on asynchronous execution.
 *       MMI_S_COMPLETE is not applicable.
 *       Other error codes as appropriate on failure.
 *
 * Asynchronous respone message code:-
 *       MMI_RESP_FILL_THIS_BUFFER
 *
 */
#define OMX_MUX_CMD_FILL_THIS_BUFFER             ( OMX_MUX_CMD_BASE + 28 )

/**
 *
 * Purpose:-
 *       This command is used to clear the queued buffers. All outstanding
 *       buffers in the queue should be cleared and returned back via
 *       appropriate callbacks.
 *
 * Command type:-
 *       Asynchronous / Synchronous
 *
 * Associated command input data type:-
 *       MMI_PortCmdType
 *
 * Command return type:-
 *       MMI_S_PENDING on asynchronous execution.
 *       MMI_S_COMPLETE on successful synchronous execution.
 *       Other error codes as appropriate on failure.
 *
 * Asynchronous respone message code:-
 *       MMI_RESP_FLUSH
 *
 */
#define OMX_MUX_CMD_FLUSH                        ( OMX_MUX_CMD_BASE + 30 )

/**
 *
 * Purpose:-
 *       This command is used to send request to load the resources to support
 *       the initial configuration provided. This is the initial reservation
 *       driver would make for the component to operate. If the underlying
 *       device has any notion of reservation for the session, this command
 *       has to be invoked. After successful invocation of this command the
 *       device should not fail on invocation of the start. The device
 *       implementation should be careful to frugal on power even after
 *       successful execution of this command. Base class must call this
 *       after all the buffers are aquired.
 *
 * Command type:-
 *       Asynchronous / Synchronous
 *
 * Associated command input data type:-
 *       None
 *
 * Command return type:-
 *       MMI_S_PENDING on asynchronous execution.
 *       MMI_S_COMPLETE on successful synchronous execution.
 *       Other error codes as appropriate on failure.
 *
 * Asynchronous respone message code:-
 *       MMI_RESP_LOAD_RESOURCES
 *
 */
#define OMX_MUX_CMD_LOAD_RESOURCES               ( OMX_MUX_CMD_BASE + 32 )

/**
 *
 * Purpose:-
 *       This command is used to release all the reserved resources relating
 *       to this device. At the completion of this command there would be no
 *       resources or scratch buffers relating to the device allocated/or
 *       reserved. Base class must call this only after all the buffers are
 *       released.
 *
 * Command type:-
 *       Asynchronous / Synchronous
 *
 * Associated command input data type:-
 *       None
 *
 * Command return type:-
 *       MMI_S_PENDING on asynchronous execution.
 *       MMI_S_COMPLETE on successful synchronous execution.
 *       Other error codes as appropriate on failure.
 *
 * Asynchronous respone message code:-
 *       MMI_RESP_RELEASE_RESOURCES
 *
 */
#define OMX_MUX_CMD_RELEASE_RESOURCES            ( OMX_MUX_CMD_BASE + 34 )

/**
 *
 * Purpose:-
 *       This command is used to request device to respond when the resources
 *       are available for reservation. Usually this command would be called
 *       when resources are preempted or an attempt to allocate the resources
 *       is failed. Device should notify to the client when the resources are
 *       available through wait for resources response. If the resources are
 *       available when the command is called or there is no associated
 *       resource, device still need to return success synchronously or
 *       through asynchronous response.
 *
 * Command type:-
 *       Asynchronous / Synchronous
 *
 * Associated command input data type:-
 *       None
 *
 * Command return type:-
 *       MMI_S_PENDING on asynchronous execution.
 *       MMI_S_COMPLETE on successful synchronous execution.
 *       Other error codes as appropriate on failure.
 *
 * Asynchronous respone message code:-
 *       MMI_RESP_WAIT_FOR_RESOURCES
 *
 */
#define OMX_MUX_CMD_WAIT_FOR_RESOURCES           ( OMX_MUX_CMD_BASE + 36 )

/**
 *
 * Purpose:-
 *       This command is used to request device to release wait on resources
 *       which was put through call to command OMX_MUX_CMD_WAIT_FOR_RESOURCES.
 *       After completion of this command, device would not respond to the
 *       wait command through callback.
 *
 * Command type:-
 *       Synchronous
 *
 * Associated command input data type:-
 *       None
 *
 * Command return type:-
 *       MMI_S_COMPLETE on success.
 *       Other error codes as appropriate on failure.
 *
 * Asynchronous respone message code:-
 *       NA
 *
 */
#define OMX_MUX_CMD_RELEASE_WAIT_ON_RESOURCES    ( OMX_MUX_CMD_BASE + 38 )

/**
 *
 * Purpose:-
 *       This command is used to request the device to setup a proprietary
 *       vendor a port tunnel to another device.  This command will be called
 *       on the device that supplies the output port.  The device should query
 *       the device that supplies the input port to determine if a vendor
 *       tunnel can be setup between the devices, and return the appropriate
 *       response.
 *
 * Command type:-
 *       Synchronous
 *
 * Associated command input data type:-
 *       MMI_VendorTunnelCmdType
 *
 * Command return type:-
 *       MMI_S_COMPLETE if tunnel has been established with the peer device.
 *       MMI_S_EFAIL if tunnel cannot be established with the peer device.
 *
 * Asynchronous respone message code:-
 *       NA
 *
 */
#define OMX_MUX_CMD_VENDOR_TUNNEL_REQUEST        ( OMX_MUX_CMD_BASE + 39 )

/**
 *
 * Purpose:-
 *       This command is used to request the device to return extension index
 *       corresponding to the given extension parameter string.
 *
 *       The value of this index need to be in the following specific range.
 *          Video extensions: 0x7F100000 -- 0x7F1FFFFF
 *          Audio extensions: 0x7F200000 -- 0x7F2FFFFF
 *          Image extensions: 0x7F300000 -- 0x7F3FFFFF
 *
 * Command type:-
 *       Synchronous
 *
 * Associated command input data type:-
 *       MMI_GetExtensionCmdType
 *
 * Command return type:-
 *       MMI_S_COMPLETE on success.
 *       Other error codes as appropriate on failure.
 *
 * Asynchronous respone message code:-
 *       NA
 *
 */
#define OMX_MUX_CMD_GET_EXTENSION_INDEX          ( OMX_MUX_CMD_BASE + 40 )

/*--------------------------------------------------------------------------*/
