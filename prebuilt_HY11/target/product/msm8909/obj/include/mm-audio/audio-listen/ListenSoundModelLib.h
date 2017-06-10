/*
 * Copyright (c) 2013-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

/*======================================================================
DESCRIPTION : ListenSoundModelLibrary
====================================================================*/
// VERSION : LSM_v0.8(CSIM_0_12_3). Released 4/5/2013

#ifndef __LISTEN_SOUND_MODEL_LIB_H__
#define __LISTEN_SOUND_MODEL_LIB_H__

#include <stdint.h>
#include <ListenSoundModelLib_types.h>

// #define USER_MATH_H

class ListenSoundModelLib
{
public:
	/*
	* verifyUserRecording
	*
	* Returns the confidence level ( 0 ~ 100 ) that user recording matches keyword
	*
	* Param [in]  pKeywordModel - pointer to user-independent keyword model data
	* Param [in]  pUserRecording - a single recording of user speaking keyword
	* Param [out] pConfidenceLevel - returns confidence level returned by keyword detection
	*
	* Return - status
	*/
	static listen_status_enum       verifyUserRecording(
					listen_model_type           *pKeywordModel,
					listen_user_recording       *pUserRecording,
					int16_t                     *pConfidenceLevel);

   /*
	* getUserKeywordModelSize
	*
	* Get the size required to hold user keyword model using given keyword model
	*
	* Param [in]  pKeywordModel - pointer to keyword model data
	* Param [out] nUserKeywordModelSize - size of user keyword model
	*
	* Return - status
	*/
	static listen_status_enum       getUserKeywordModelSize(
					listen_model_type            *pKeywordModel,
					uint32_t                     *nUserKeywordModelSize);

	/*
	* createUserKeywordModel
	*
	* Create a user keyword model
    * Writes the user keyword model into given memory location
	*
	* Param [in]  pKeywordModel - pointer to Keyword model  or User keyword model data
	* Param [in]  numUserRecording - number of user recordings
	* Param [in]  pUserRecordings - multiple recording of user speaking keyword
	* Param [out]  pUserKeywordModel - pointer to where user keyword model data is to be written
	* Param [out] pUserMatchingScore - pointer to user matching score
	* Return - status
	*/
	static listen_status_enum       createUserKeywordModel(
					listen_model_type           *pKeywordModel,
					int32_t                     numUserRecording,
					listen_user_recording       *pUserRecordings[],
					listen_model_type           *pUserKeywordModel,
					int16_t						*pUserMatchingScore);

   /*
	* recommendUserSensitivity
	*
	* Recommend optimal user sensitivity
	* It can compute optimal user sensitivity better if extra negative user recordings are provided
	* This functions works fine without extra negative user recording
	* (*) negative user recordings : other users speaking
	*
	* Param [in]  pKeywordUserModel - pointer to User keyword model data
	* Param [in]  numUserRecording - number of user recordings
	* Param [in]  pUserRecordings - multiple recording of user speaking keyword
	* Param [in]  numExtraNegativeUserRecording - number of extra negative user recordings
	* Param [in]  pExtraNegativeUserRecording - multiple recording of negative user speaking keyword
	* Param [out]  pRecommendUserSensitive - pointer to where recommended user sensitivity is to be written
	* Return - status
	*/

	static listen_status_enum       recommendUserSensitivity(
					listen_model_type           *pKeywordUserModel,
					int32_t                     numUserRecording,
					listen_user_recording       *pUserRecordings[],
					int32_t                     numExtraNegativeUserRecording,
					listen_user_recording       *pExtraNegativeUserRecording[],
					int16_t                     *pRecommendUserSensitive);

	/*
	* parseDetectionEventData
	*
	* parse event payload into detection event
	*
	* Param [in] pUserKeywordModel - pointer to keyword or user keyword model data
	* Param [in] pEventPayload - pointer to received event payload data
	* Param [out] pDetectEvent - pointer to where detection event data is to be written
	* Return - status
	*/

	static listen_status_enum       parseDetectionEventData(
		            listen_model_type           *pUserKeywordModel,
					listen_event_payload        *pEventPayload,
					listen_detection_event_type      *pDetectionEvent);


	/*
	* querySoundModel
	*
	* Returns the information about a sound model
    * Sound model could be of any type: Keyword, UserKeyword, TargetSound,...
	*
	* Param [in] pSoundModel - pointer to model data
	* Param [out] pListenSoundModelHeader - returns information about the give sound model
	*
	* Return - status
	*/
	static listen_status_enum       querySoundModel(
					listen_model_type        *pSoundModel,
					listen_sound_model_info  *pListenSoundModelHeader);

  /*
	* getKeywordStr
	*
	* Returns the string for the Keyword in a SVA 1.0 sound model
   * Sound model could be of type: Keyword, UserKeyword
	*
	* Param [in] pSoundModel - pointer to model data
	* Param [out] pKeywordStr - keyword copied into this string location
	*
	* Return - status
	*/
	static listen_status_enum getKeywordStr(
	            listen_model_type           *pUserKeywordModel,
	            char *                       pKeywordStr);
};

#endif /* __LISTEN_SOUND_MODEL_LIB_H__ */
