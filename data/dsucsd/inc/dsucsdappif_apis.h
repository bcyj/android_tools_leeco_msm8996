#ifndef DSUCSDAPPIF_APIS_FUSION_H_trimmed
#define DSUCSDAPPIF_APIS_FUSION_H_trimmed
/*=============================================================================
  @file  dsucsdappif_apis.h

  ---------------------------------------------------------------------------
  Copyright (c) 2012 Qualcomm Technologies, Inc.
  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------

  $Id: //depot/asic/CSFB_FUSION/9200/3.81/MODEM_APIS/libs/remote_apis/dsucsdappif_apis_fusion/inc/dsucsdappif_apis_fusion.h#2 $

  Notes:
     ==== Auto-Generated File, do not edit manually ====
     Generated from build type: AAABQOCSH
     #defines containing AABQOCSH replaced with ________
=============================================================================*/
#ifndef DSUCSDAPPIF_APIS_FUSION_H
#define DSUCSDAPPIF_APIS_FUSION_H


/*==========================================================================

                      INCLUDE FILES FOR MODULE

==========================================================================*/
/** Invalid call mode. */
#define DSUCSD_CALL_MODE_INVALID      (0x0)
/** Data call mode. */
#define DSUCSD_CALL_MODE_DATA         (0x1)
/** Voice call mode. */
#define DSUCSD_CALL_MODE_DATA_VOICE (0x2)
/** Voice and data call mode. */
#define DSUCSD_CALL_MODE_VOICE_DATA (0x4)
/** Maximum number of API clients. */
#define DSUCSD_API_MAX_CLIENTS    (2)
/** Maximum value of the client ID. */
#define DSUCSD_MAX_CLIENT_ID      (DSUCSD_API_MAX_CLIENTS-1)
/** Invalid client ID. */
#define DSUCSD_INVALID_CLIENT_ID   (-1)
/** Maximum number of supported RLP sets. */
#define DSUCSD_MAX_RLP_SETS (3)



/** Definitions to manage API client registration. */
typedef int8   ds_ucsd_client_id_type;

/** System modes.
*/
typedef enum
{
  /** @cond
  */
  SYS_SYS_MODE_NONE = -1,  /* FOR INTERNAL USE ONLY! */
  /** @endcond
  */

  SYS_SYS_MODE_NO_SRV=0,
    /**< No service; NV_MODE_INACTIVE. */

  SYS_SYS_MODE_AMPS=1,
    /**< Analog Mobile Phone System (AMPS) mode. */

  SYS_SYS_MODE_CDMA=2,
    /**< CDMA mode. */

  SYS_SYS_MODE_GSM=3,
    /**< GSM mode. */

  SYS_SYS_MODE_HDR=4,
    /**< HDR mode. */

  SYS_SYS_MODE_WCDMA=5,
    /**< WCDMA mode. */

  SYS_SYS_MODE_GPS=6,
    /**< GPS mode. */

  SYS_SYS_MODE_GW=7,
    /**< GSM and WCDMA mode. */

  SYS_SYS_MODE_WLAN=8,
    /**< WLAN mode. */

  SYS_SYS_MODE_LTE=9,
    /**< LTE mode. */

  SYS_SYS_MODE_GWL=10,
    /**< GSM, WCDMA, and LTE mode. */

  SYS_SYS_MODE_TDS=11,
  /**< TDS mode. */

  /** @cond
  */
  SYS_SYS_MODE_MAX   /* FOR INTERNAL USE ONLY! */
  /** @endcond
  */

} sys_sys_mode_e_type;
/*~ SENTINEL sys_sys_mode_e_type.SYS_SYS_MODE_MAX */


/** Enumeration of call events. Clients are able to register
    with the CM to be notified of any subsets of these events.
*/
typedef enum cm_call_event_e {

  /** @cond
  */
  CM_CALL_EVENT_NONE=-1,  /* FOR INTERNAL CM USE ONLY! */
  /** @endcond
 */

  /* 0 */
  CM_CALL_EVENT_ORIG,
    /**< Phone originated a call. */

  CM_CALL_EVENT_ANSWER,
    /**< Incoming call was answered. */

  CM_CALL_EVENT_END_REQ,
    /**< Started call-end process. */

  CM_CALL_EVENT_END,
    /**< Originated/incoming call was ended .*/

  CM_CALL_EVENT_SUPS,
    /**< Phone sent Flash/Flash-with-Info to the BS. */

  /* 5 */
  CM_CALL_EVENT_INCOM,
    /**< Phone received an incoming call. */

  CM_CALL_EVENT_CONNECT,
    /**< Originated/incoming call was connected. */

  CM_CALL_EVENT_SRV_OPT,
    /**< Service option changed while in a call. CDMA only. */

  CM_CALL_EVENT_PRIVACY,
    /**< Privacy mode changed while in a call. CDMA only. */

  CM_CALL_EVENT_PRIVACY_PREF,
    /**< Privacy mode preference changed. */

  /* 10 */
  CM_CALL_EVENT_CALLER_ID,
    /**< Caller ID info was received from the BS. CDMA only. */

  CM_CALL_EVENT_ABRV_ALERT,
    /**< CMDA/AMPS abbreviated alert. CDMA only. */

  CM_CALL_EVENT_ABRV_REORDER,
    /**< AMPS abbreviated reorder. CDMA only. */

  CM_CALL_EVENT_ABRV_INTERCEPT,
    /**< AMPS abbreviated intercept. CDMA only. */

  CM_CALL_EVENT_SIGNAL,
    /**< Signal info was received from the BS. */

  /* 15 */
  CM_CALL_EVENT_DISPLAY,
    /**< Display info was received from the BS. CDMA only. */

  CM_CALL_EVENT_CALLED_PARTY,
    /**< Called party info was received from the BS. CDMA only. */

  CM_CALL_EVENT_CONNECTED_NUM,
    /**< Connected number info was received from the BS. */

  CM_CALL_EVENT_INFO,
    /**< Call information. This event is only sent to the client that
         requests this information through cm_call_cmd_get_call_info. */

  CM_CALL_EVENT_EXT_DISP,
    /**< Extended display was received from the BS. CDMA only. */

  /* 20 */
  CM_CALL_EVENT_NDSS_START,
    /**< Start NDSS redirection. CDMA only. */

  CM_CALL_EVENT_NDSS_CONNECT,
    /**< Call was reconnected due to NDSS. CDMA only. */

  /* The following are for FEATURE_JCDMA. */

  CM_CALL_EVENT_EXT_BRST_INTL,
    /**< Extended burst type; international. */

  CM_CALL_EVENT_NSS_CLIR_REC,
    /**< National Supplementary Services; Calling Line Identity Restriction
        (CLIR). */

  CM_CALL_EVENT_NSS_REL_REC,
    /**< National Supplementary Services; release. */

  /* 25 */
  CM_CALL_EVENT_NSS_AUD_CTRL,
    /**< National Supplementary Services; audio control. */

  CM_CALL_EVENT_L2ACK_CALL_HOLD,
    /**< Call hold. */

  /* The following are for WCDMA/GSM/TDS, except
     CM_CALL_EVENT_SETUP_RES, which is also used by the IP application. */

  CM_CALL_EVENT_SETUP_IND,
    /**< Phone received a setup indication message from the BS. */

  CM_CALL_EVENT_SETUP_RES,
    /**< A setup response was sent (also used for IP call). */

  CM_CALL_EVENT_CALL_CONF,
    /**< The call origination has been accepted. */

  /* The following are for WCDMA/GSM/TDS PS Data. */

  /* 30 */
  CM_CALL_EVENT_PDP_ACTIVATE_IND,
    /**< Phone received an incoming PDP call. */

  CM_CALL_EVENT_PDP_ACTIVATE_RES,
    /**< A response to an incoming PDP was sent. */

  CM_CALL_EVENT_PDP_MODIFY_REQ,
    /**< A PDP modify request was sent. */

  CM_CALL_EVENT_PDP_MODIFY_IND,
    /**< The phone received a PDP modify indication from the BS. */

  CM_CALL_EVENT_PDP_MODIFY_REJ,
    /**< The phone received a PDP modify rejection from the BS. */

  /* 35 */
  CM_CALL_EVENT_PDP_MODIFY_CONF,
    /**< The phone received a PDP modify confirmation from the BS. */

  CM_CALL_EVENT_RAB_REL_IND,
    /**< The phone received an RAB release indication from the BS. */

  CM_CALL_EVENT_RAB_REESTAB_IND,
    /**< The phone received an RAB re-establish indication from the BS. */

  CM_CALL_EVENT_RAB_REESTAB_REQ,
    /**< An RAB re-establish request was sent. */

  CM_CALL_EVENT_RAB_REESTAB_CONF,
    /**< The phone received an RAB re-establish confirmation from the BS. */

  /* 40 */
  CM_CALL_EVENT_RAB_REESTAB_REJ,
    /**< The phone received an RAB re-establish rejection from the BS. */

  CM_CALL_EVENT_RAB_REESTAB_FAIL,
    /**< RAB re-establishment failed. */

  CM_CALL_EVENT_PS_DATA_AVAILABLE,
    /**< A PS data available request was sent. */

  CM_CALL_EVENT_MNG_CALLS_CONF,
    /**< A confirmation for Multi-Party (MPTY) calls. */

  CM_CALL_EVENT_CALL_BARRED,
    /**< Call barred notification. */

  /* 45 */
  CM_CALL_EVENT_CALL_IS_WAITING,
    /**< A call is awaiting notification. */

  CM_CALL_EVENT_CALL_ON_HOLD,
    /**< Call on hold notification. */

  CM_CALL_EVENT_CALL_RETRIEVED,
    /**< Call retrieved notification. */

  CM_CALL_EVENT_ORIG_FWD_STATUS,
    /**< Originated call may be forwarded notification to
         forwarding subscriber. */

  CM_CALL_EVENT_CALL_FORWARDED,
    /**< Call-forwarded notification to forwarding subscriber. */

  /* 50 */
  CM_CALL_EVENT_CALL_BEING_FORWARDED,
    /**< Call being forwarded notification to calling subscriber. */

  CM_CALL_EVENT_INCOM_FWD_CALL,
    /**< Incoming forwarded-call notification to forwarded-to-subscriber. */

  CM_CALL_EVENT_CALL_RESTRICTED,
    /**< Call restricted notification. */

  CM_CALL_EVENT_CUG_INFO_RECEIVED,
    /**< Call forward Closed User Group (CUG) notification. */

  CM_CALL_EVENT_CNAP_INFO_RECEIVED,
    /**< Caller name information notification. */

  /* 55 */
  CM_CALL_EVENT_EMERGENCY_FLASHED,
    /**< A voice call was converted into an emergency call. */

  CM_CALL_EVENT_PROGRESS_INFO_IND,
    /**< Call origination progress indication. */

  CM_CALL_EVENT_CALL_DEFLECTION,
    /**< Call deflection notification. */

  CM_CALL_EVENT_TRANSFERRED_CALL,
    /**< Call transfer notification. */

  CM_CALL_EVENT_EXIT_TC,
    /**< A traffic channel has been torn down. CDMA only. */

  /* 60 */
  CM_CALL_EVENT_REDIRECTING_NUMBER,
    /**< Redirecting a number information record event. CDMA only. */

  CM_CALL_EVENT_PDP_PROMOTE_IND,
    /**< Obsolete with definition of CM_API_SEC_PDP. */

    /* Clients check CM_API_SEC_PDP for SEC PDP support.
       Secondary context being promoted to primary. */

  CM_CALL_EVENT_UMTS_CDMA_HANDOVER_START,
    /**< An event to initiate the UMTS to 1X handoff. */

  CM_CALL_EVENT_UMTS_CDMA_HANDOVER_END,
    /**< An event to indicate the UMTS to 1X handoff is completed.*/

  CM_CALL_EVENT_SECONDARY_MSM,
    /**< Invoke event call back even if the event is from a secondary MSM. The
         CM will not generate any events with this, but it is used by the thin
         client CM layer to call client callbacks for events from a secondary
         MSM. */

  /* 65 */
  CM_CALL_EVENT_ORIG_MOD_TO_SS,
    /**< An Origination command was modified to SS due to Call Control. */

  CM_CALL_EVENT_USER_DATA_IND,
    /**< Indicates user data from the remote side. Clients must check for
         CM_API_USER_USER_DATA before using this event. */

  CM_CALL_EVENT_USER_DATA_CONG_IND,
    /**< Indication from lower layers to stop sending user data. Clients
         must check for CM_API_USER_USER_DATA before using this event. */

  CM_CALL_EVENT_MODIFY_IND,
    /**< The network requested modification of VT to voice, and vice-versa.
         This event is applicable when CM_API_VT is defined. */

  CM_CALL_EVENT_MODIFY_REQ,
    /**< A request to modify VT to Voice, and vice-versa, is sent. This event
         is only applicable when CM_API_VT is defined. */

  /* 70 */
  CM_CALL_EVENT_LINE_CTRL,
    /**< This event is sent when CM_LINE_CTRL_F report is received from
         the lower layers. 1X only. */

  CM_CALL_EVENT_CCBS_ALLOWED,
    /**< Informs that CCBS is allowed on this call. Clients must
         check for CM_API_CCBS before using this event. */

  CM_CALL_EVENT_ACT_CCBS_CNF,
    /**< Sent after CM_5_SEND_CNF is received from the network. Clients
         must check for CM_API_CCBS before using this event. */

  CM_CALL_EVENT_CCBS_RECALL_IND,
    /**< Sent in response to CM_RECALL_IND from the network. Clients
         must check for CM_API_CCBS before using this event. */

  CM_CALL_EVENT_CCBS_RECALL_RSP,
    /**< Sent after receiving a recall response from a client. Clients
         must check for CM_API_CCBS before using this event. */

  /* 75 */
  CM_CALL_EVENT_CALL_ORIG_THR,
    /**< Call origination throttled. */

  CM_CALL_EVENT_VS_AVAIL,
    /**< A Videoshare call is possible for this voice call. Clients must
         check for CM_API_VIDEOSHARE before using this event. */

  CM_CALL_EVENT_VS_NOT_AVAIL,
    /**< A Videoshare call is not possible for this voice call. Clients
         must check for CM_API_VIDEOSHARE before using this event. */

  CM_CALL_EVENT_MODIFY_COMPLETE_CONF,
    /**< This event is sent after an in-call modification confirmation
         has been received by the CM. Clients must check for
         CM_API_VT_FALLBACK_TO_VOICE before using this event. */

  CM_CALL_EVENT_MODIFY_RES,
    /**< This event is sent to indicate a client's response to an MT in-call
         modification indication. Clients must check for
         CM_API_VT_FALLBACK_TO_VOICE before using this event. */

  /* 80 */
  CM_CALL_EVENT_CONNECT_ORDER_ACK,
    /**< This event is sent to indicate a client's response to a user-answered
         call. Clients must check for CM_API_CONNECT_ORDER_ACK before
         using this event. */

  CM_CALL_EVENT_TUNNEL_MSG,
    /**< A tunneled message for the call was received. */

  CM_CALL_EVENT_END_VOIP_CALL,
    /**< Event to end a VOIP call. */

  CM_CALL_EVENT_VOIP_CALL_END_CNF,
    /**< Event to confirm a VOIP call end. */

  /* 84 */
  CM_CALL_EVENT_PS_SIG_REL_REQ,
    /**< Started PS signaling release process. */

  /* 85 */
  CM_CALL_EVENT_PS_SIG_REL_CNF,
    /**< Event to notify PS signaling release confirmation. */

  CM_CALL_EVENT_HANDOFF_CALL_ORIG,
    /**< HOM-originated call.  */

    /* 87 */
  CM_CALL_EVENT_HANDOFF_CALL_CONNECT,
    /**<  HOM-originated incoming call was connected. */

  CM_CALL_EVENT_HANDOFF_CALL_END,
    /**<  HOM-originated incoming call was ended. */

  CM_CALL_EVENT_HANDOFF_MANUAL_CALL_ORIG,
    /**<  Manually originated incoming call. */

    /* 90 */
  CM_CALL_EVENT_MPTY_CALL_IND,
    /**<  Multiparty call indication. */

  CM_CALL_EVENT_OTASP_STATUS,
    /**< OTASP status indication. */

    /* 92 */
  CM_CALL_EVENT_PDP_NW_MODIFY_REQ,
    /**< Phone received a PDP modify request from the BS. */

    /* 93 */
  CM_CALL_EVENT_PDP_NW_MODIFY_RSP,
    /**< PDP network modify response was sent. */

   /* 94 */
  CM_CALL_EVENT_ACT_BEARER_IND,
    /**< Activate bearer request. */

    /* 95 */
  CM_CALL_EVENT_MOD_BEARER_IND,
    /**< Modify bearer indication. */

    /* 96 */
  CM_CALL_EVENT_GET_PDN_CONN_IND,
    /**< Get PDN connectivity request indication. */

    /* 97 */
  CM_CALL_EVENT_PDN_CONN_REJ_IND,
    /**< PDN connectivity reject indication. */

    /* 98 */
  CM_CALL_EVENT_PDN_CONN_FAIL_IND,
    /**< PDN connectivity failed indication. */

    /* 99 */
  CM_CALL_EVENT_PDN_DISCONNECT_REJECT_IND,
    /**< PDN disconnect reject indication. */

    /* 100 */
  CM_CALL_EVENT_RES_ALLOC_REJ_IND,
    /**< Bearer resource allocation reject indication. */

    /* 101 */
  CM_CALL_EVENT_RES_ALLOC_FAIL_IND,
    /**< Bearer resource allocation failed indication. */

    /* 102 */
  CM_CALL_EVENT_BEARER_MOD_REJ_IND,
    /**< Bearer resource modification reject indication. */

    /* 103 */
  CM_CALL_EVENT_HANDOVER_COMPLETE_IND,
    /**< Event to indicate CM client about handover completion. */

    /* 104 */
  CM_CALL_EVENT_PROGRESS_EARLY_MEDIA,
  /**< call origination progress early media announcement */

  CM_CALL_EVENT_SRVCC_COMPLETE_IND,
  /**< LTE->WCDMA SRVCC complete indication
  */

    /* 106 */
  /** @cond
 */
  CM_CALL_EVENT_MAX   /* FOR INTERNAL CM USE ONLY! */
  /** @endcond
 */

} cm_call_event_e_type;
/*~ SENTINEL cm_call_event_e_type.CM_CALL_EVENT_MAX */


/** Enumeration of call end statuses (i.e., the reason for ending a call). For
    compatibility with the QXDM database, do not change the following fixed
    assigned values. If new values are added, update the QXDM database.
*/
typedef enum cm_call_end_e {

  /** @cond
  */
  CM_CALL_END_NONE                         = -1, /* FOR INTERNAL CM USE ONLY! */
  /** @endcond
 */

  /* Common */
  CM_CALL_END_OFFLINE                      = 0,
    /**< The phone is offline. */

  /* CDMA */
  CM_CALL_END_CDMA_LOCK                    = 20,
    /**< The phone is CDMA locked until a power cycle occurs. CDMA only. */

  CM_CALL_END_NO_SRV                       = 21,
    /**< The phone has no service. This is for backward compatibility.
         NO_CDMA_SRV and NO_GW_SRV are mapped to this. */

  CM_CALL_END_FADE                         = 22,
    /**< The call ended abnormally. CDMA only. */

  CM_CALL_END_INTERCEPT                    = 23,
    /**< Received an intercept from the BS -- originating only. CDMA only. */

  CM_CALL_END_REORDER                      = 24,
    /**< Received a reorder from the BS -- originating only. CDMA only. */

  CM_CALL_END_REL_NORMAL                   = 25,
    /**< Received a release from the BS -- no reason given. */

  CM_CALL_END_REL_SO_REJ                   = 26,
    /**< Received a release from the BS -- SO reject. CDMA only. */

  CM_CALL_END_INCOM_CALL                   = 27,
    /**< Received an incoming call from the BS. */

  CM_CALL_END_ALERT_STOP                   = 28,
    /**< Received an alert stop from the BS -- incoming only. CDMA only. */

  CM_CALL_END_CLIENT_END                   = 29,
    /**< The client ended the call. */

  CM_CALL_END_ACTIVATION                   = 30,
    /**< Received an end activation -- OTASP call only. CDMA only. */

  CM_CALL_END_MC_ABORT                     = 31,
    /**< The MC aborted the origination/conversation. CDMA only. */

  CM_CALL_END_MAX_ACCESS_PROBE             = 32,
    /**< Maximum access probes transmitted. CDMA only. */

  CM_CALL_END_PSIST_NG                     = 33,
    /**< Persistence test failure. FEATURE_JCDMA only. CDMA only. */

  CM_CALL_END_UIM_NOT_PRESENT              = 34,
    /**< RUIM is not present. */

  CM_CALL_END_ACC_IN_PROG                  = 35,
    /**< Access attempt already in progress. */

  CM_CALL_END_ACC_FAIL                     = 36,
    /**< Access failure for reasons other than the above. */

  CM_CALL_END_RETRY_ORDER                  = 37,
    /**< Received a retry order -- originating only. IS-2000. CDMA only. */

  CM_CALL_END_CCS_NOT_SUPPORTED_BY_BS      = 38,
    /**< Concurrent service is not supported by the base station. */

  CM_CALL_END_NO_RESPONSE_FROM_BS          = 39,
    /**< No response received from the base station. */

  CM_CALL_END_REJECTED_BY_BS               = 40,
    /**< Call rejected by the base station. CDMA only. */

  CM_CALL_END_INCOMPATIBLE                 = 41,
    /**< Concurrent services requested were not compatible. CDMA only. */

  CM_CALL_END_ACCESS_BLOCK                 = 42,
    /**< Access is blocked by the base station. CDMA only. */

  CM_CALL_END_ALREADY_IN_TC                = 43,
    /**< Corresponds to CM_CALL_ORIG_ERR_ALREADY_IN_TC. */

  CM_CALL_END_EMERGENCY_FLASHED            = 44,
    /**< The call ended because an Emergency call was flashed over this call.
         CDMA only. */

  CM_CALL_END_USER_CALL_ORIG_DURING_GPS    = 45,
    /**< Used if the CM is ending a GPS call in favor of a user call. */

  CM_CALL_END_USER_CALL_ORIG_DURING_SMS    = 46,
    /**< Used if the CM is ending a SMS call in favor of a user call. */

  CM_CALL_END_USER_CALL_ORIG_DURING_DATA   = 47,
    /**< Used if the CM is ending a data call in favor of an emergency call. */

  CM_CALL_END_REDIR_OR_HANDOFF             = 48,
    /**< Call rejected because of redirection or handoff. */

  CM_CALL_END_ACCESS_BLOCK_ALL             = 49,
    /**< Access is blocked by the base station for all mobile devices. KDDI
         specific. CDMA only. */

  CM_CALL_END_OTASP_SPC_ERR                = 50,
    /**< To support an OTASP SPC error indication. */

  CM_CALL_END_IS707B_MAX_ACC               = 51,
    /**< Maximum access probes for IS-707B call. CDMA only .*/

  /* GSM/WCDMA/TDS */
  CM_CALL_END_LL_CAUSE                     = 100,
    /**< Received a reason for ending the call from the lower layer (look in
         cc_cause). WCDMA/GSM/TDS only. */

  CM_CALL_END_CONF_FAILED,
    /**< Call origination request failed. WCDMA/GSM/TDS only. */

  CM_CALL_END_INCOM_REJ,
    /**< The client rejected the incoming call. WCDMA/GSM/TDS only. */

  CM_CALL_END_SETUP_REJ,
    /**< The client rejected the setup_ind. WCDMA/GSM/TDS only. */

  /* 104 */
  CM_CALL_END_NETWORK_END,
    /**< The network ended the call (look in cc_cause). WCDMA/GSM/TDS only. */

  CM_CALL_END_NO_FUNDS,
    /**< GSM/WCDMA/TDS only. */

  CM_CALL_END_NO_GW_SRV,
    /**< Phone has no service. GWM/WCDMA/TDS only. */

  CM_CALL_END_NO_CDMA_SRV,
    /**< Phone has no service. 1X only. */

  /* 108 */
  CM_CALL_END_NO_FULL_SRV,
    /**< Full service is unavailable. */

  CM_CALL_END_MAX_PS_CALLS,
    /**< Indicates resources not available to handle a new MO/MT PS call. */

  /* HDR */
  CM_CALL_END_CD_GEN_OR_BUSY               = 150,
    /**< Abort connection setup due to the reception of a ConnectionDeny message
         with a deny code of general or network busy. */

  CM_CALL_END_CD_BILL_OR_AUTH              = 151,
    /**< Abort connection setup due to the reception of a ConnectionDeny message
         with a deny code of billing failure or authentication failure. */

  CM_CALL_END_CHG_HDR                      = 152,
    /**< Change the HDR system due to redirection or PRL not preferred. */

  CM_CALL_END_EXIT_HDR                     = 153,
    /**< Exit HDR  due to redirection or PRL not preferred. */

  CM_CALL_END_HDR_NO_SESSION               = 154,
    /**< No HDR session. */

  /** @cond
  */
  CM_CALL_END_CM_COLLOC_ACQ_FAIL           = 155,
    /* For internal CM use only -- Failed to acquire co-located HDR for origination. */
  /** @endcond
  */

  CM_CALL_END_HDR_ORIG_DURING_GPS_FIX      = 156,
    /**< Used if CM is ending an HDR call origination in favor of a GPS fix. */

  CM_CALL_END_HDR_CS_TIMEOUT               = 157,
    /**< Connection setup timeout. */

  CM_CALL_END_HDR_RELEASED_BY_CM           = 158,
    /**< The CM released an HDR call so that a 1X call can continue. */

  CM_CALL_END_HOLD_DBM_IN_PROG             = 159,
    /**< The CM is holding the HDR origination to allow a 1X SMS to end. */

  CM_CALL_END_OTASP_COMMIT_IN_PROG         = 160,
    /**< The CM will end the call because an OTASP commit is in progress. */

  CM_CALL_END_NO_HYBR_HDR_SRV              = 161,
    /**< Mobile has no Hybrid HDR service. */

  CM_CALL_END_HDR_NO_LOCK_GRANTED          = 162,
    /**< Call ended because HDR did not get the RF Lock. */

  CM_CALL_END_HOLD_OTHER_IN_PROG           = 163,
    /**< The CM will hold the the current call to allow another call to end. */

  CM_CALL_END_HDR_FADE                     = 164,
    /**< HDR releases a call due to fade. */

  CM_CALL_END_HDR_ACC_FAIL                 = 165,
    /**< HDR releases a call due to access failure attempts. */

  /* The following Call Release reasons are specific to VideoTelephony calls. */
  CM_CALL_END_VIDEO_CONN_LOST              = 301,
    /**< The modem released the call after the modem was connected. */

  CM_CALL_END_VIDEO_SETUP_FAILURE          = 302,
    /**< Call setup failed while trying to set up the modem. */

  CM_CALL_END_VIDEO_PROTOCOL_CLOSED        = 303,
    /**< The video protocol closed after the video protocol setup was done. */

  CM_CALL_END_VIDEO_PROTOCOL_SETUP_FAILURE = 304,
    /**< Video protocol setup failed. */

  CM_CALL_END_INTERNAL_ERROR,
    /**< A CM internal error other than any of the above. */

  /* WLAN */
  CM_CALL_END_NO_WLAN_SRV                  = 200,
    /**< Call was ended because no WLAN service was found. */

  CM_CALL_END_VOIP_FAIL_WLAN               = 201,
    /**< VOIP failed on WLAN. */

  /* IP related */
  CM_CALL_END_IP_FAIL                      = 202,
    /**< Call origination on IP failed. To be used only when CM_API_IP_CALL is
         defined. */

  CM_CALL_END_IP_RETRY                     = 203,
    /**< Call must be retried on IP. To be used only when CM_API_IP_CALL is
         defined. */

  CM_IP_CALL_END_EMERG_ORIG                = 204,
    /**< Call ended due to Emergency origination call. To be used only when
         CM_API_IP_CALL is defined. */

  CM_CALL_END_IP_END                       = 205,
    /**< Used only when CM_API_IP_CALL is defined and the call is to be
         ended.  */

  CM_CALL_END_THERMAL_EMERGENCY            = 206,
    /**< Call ended to put phone in thermal emergency. */

  CM_CALL_END_ORIG_THR                     = 401,
    /**< Origination throttled. */

  CM_CALL_END_1XCSFB_SOFT_FAILURE                     = 402,
     /**< 1XCSFB call is ended because of soft failure. */

  CM_CALL_END_1XCSFB_HARD_FAILURE                     = 403,
     /**< 1XCSFB call is ended because of hard failure. */


  /** @cond
  */
  CM_CALL_END_MAX   /* FOR INTERNAL CM USE ONLY! */
  /** @endcond
 */

} cm_call_end_e_type;
/*~ SENTINEL cm_call_end_e_type.CM_CALL_END_MAX */

/** Classifies the rejection type based on the rejecting layer.
*/
typedef enum
{
   INVALID_REJECT_CAUSE = 0x00, /**< Invalid value. */
   OTA_REJECT_CAUSE,            /**< Over-The-Air rejection. */
   AS_REJECT_CAUSE,             /**< Access Stratum rejection. */
   MM_REJECT_CAUSE,             /**< Mobility Management rejection. */
   CNM_MN_REJECT_CAUSE,         /**< Mobile Network or Connection Manager
                                     rejection. */
   EMM_REJECT_CAUSE,            /**< EPS Connection Management (ECM) rejection. */
   ONEX_CSFB_REJECT_CAUSE       /**< Circuit-Switched Fallback (CSFB)
                                     rejection. */

}reject_type_enum_T;


/** This macro encapsulates the major and minor number and minor number
    of device and returns device_id/port_id. */
#define DEVICE_ID(major, minor) ((major << 8) | (minor & 0x00ff))

/*---------------------------------------------------------------------------*/
/**
@name SIO Major numbers
  The following constants are grouped as SIO major numbers. Sio major numbers
  are assigned to each unique driver-interface.
  @{
*/
/*---------------------------------------------------------------------------*/

#define SIO_MAJOR_LEGACY        0       /**< SIO Major for Legacy device driver */
#define SIO_MAJOR_UART          1       /**< SIO Major for UART driver */
#define SIO_MAJOR_USB           2       /**< SIO Major for USB driver */
#define SIO_MAJOR_HSUSB_ECM     3       /**< SIO Major for HS-USB-ECM driver */
#define SIO_MAJOR_SMD           4       /**< SIO Major for SMD DRIVER */
#define SIO_MAJOR_SMD_BRIDGE    5       /**< SIO Major for SMD-BRIDGE driver */
#define SIO_MAJOR_SMEM          6       /**< SIO Major for SMEM driver */
#define SIO_MAJOR_BLUETOOTH     7       /**< SIO Major for BT driver */
#define SIO_MAJOR_MMGPS         8       /**< SIO Major for MMGPS driver */
#define SIO_MAJOR_APS           9       /**< SIO Major for APS driver */
#define SIO_MAJOR_SMD_BRIDGE_NPROC 10   /**< SIO Major for SMD Bridge driver for N-proc */
#define SIO_MAJOR_SMD_TO_APPS   11      /**< SIO Major for SMD Bridge to Apps*/
#define SIO_MAJOR_SMD_TO_MODEM  12      /**< SIO Major for SMD Bridge to Modem */
#define SIO_MAJOR_SMD_TO_QDSP6  13      /**< SIO Major for SMD Bridge to QDSP6 */
#define SIO_MAJOR_A2            14      /**< SIO Major for A2 driver */
#define SIO_MAJOR_DATA_MUX      15      /**< SIO Major for MUX driver  */
#define SIO_MAJOR_HSUSB_EEM     16      /**< SIO Major for HSUSB EEM driver */
#define SIO_MAJOR_ONDEVICE_DIAG 17      /**< SIO Major for Diag OnDevice driver  */
#define SIO_MAJOR_SDIOC_STREAM_DATA   18      /**< SIO Major for SDIO stream client driver */
#define SIO_MAJOR_SDIOC_PACKET_DATA   19      /**< SIO Major for SDIO packet client driver */
#define SIO_MAJOR_ONDEVICE_DIAG_USB_AUTO   20 /**< SIO Major for Diag Ondevice USB Auto driver */
#define SIO_NULL_DRIVER         21      /**< SIO Major for NULL driver */

/**
   @}
*/


/*---------------------------------------------------------------------------*/
/**
@name SIO Minor numbers
  The following constants are grouped as SIO Minor numbers.
  SIO minor number indemnifies the logical device for device driver.
  Maximum minor numbers allowed per major number is 64.
  @{
*/
/*---------------------------------------------------------------------------*/

/* SIO_LEGACY_MINOR */
#define SIO_MINOR_LEGACY_NULL   0       /**< SIO Minor for NULL Driver */

/* UART DRIVER */
#define SIO_MINOR_UART_1        1       /**< SIO Minor for UART 1 */
#define SIO_MINOR_UART_2        2       /**< SIO Minor for UART 2 */
#define SIO_MINOR_UART_3        3       /**< SIO Minor for UART 3 */
#define SIO_PORT_MAX            64

/* GENERIC (FS/HS) USB DRIVER */
#define SIO_MINOR_USB_MDM       1       /**< SIO Minor for USB MDM device  */
#define SIO_MINOR_USB_SER1      2       /**< SIO Minor for USB SER1 device */
#define SIO_MINOR_USB_SER2      3       /**< SIO Minor for USB SER2 device */
#define SIO_MINOR_USB_SER3      4       /**< SIO Minor for USB SER3 device */
#define SIO_MINOR_USB_MMC       5       /**< SIO Minor for USB MMC device  */
#define SIO_MINOR_USB_RRDM      6       /**< SIO Minor for USB RRDM Device */
#define SIO_MINOR_USB_NET_WWAN  7       /**< SIO Minor for USB WWAN/RMNET1 device */
#define SIO_MINOR_USB_RMNET2    8       /**< SIO Minor for USB RMENT2 device */
#define SIO_MINOR_USB_RMNET3    9       /**< SIO Minor for USB RMNET3 device */
#define SIO_MINOR_USB_BREW_X_DATA 10    /**< SIO Minor for BREW_X_DATA device */
#define SIO_MINOR_USB_BREW_X_NOTIFICATION 11    /**< SIO Minor for USB Brew notification device */
#define SIO_MINOR_USB_EEM       12      /**< SIO Minor for USB EEM device */
#define SIO_MINOR_USB_RMNET4    13      /**< SIO Minor for USB RMNET4 device */
#define SIO_MINOR_USB_RMNET5    14      /**< SIO Minor for USB RMNET5 device */

/* FS-USB DRIVER */
#define SIO_MINOR_FSUSB_MDM       SIO_MINOR_USB_MDM
#define SIO_MINOR_FSUSB_SER1      SIO_MINOR_USB_SER1
#define SIO_MINOR_FSUSB_SER2      SIO_MINOR_USB_SER2
#define SIO_MINOR_FSUSB_SER3      SIO_MINOR_USB_SER3
#define SIO_MINOR_FSUSB_MMC       SIO_MINOR_USB_MMC
#define SIO_MINOR_FSUSB_RRDM      SIO_MINOR_USB_RRDM
#define SIO_MINOR_FSUSB_NET_WWAN  SIO_MINOR_USB_NET_WWAN
#define SIO_MINOR_FSUSB_RMNET2    SIO_MINOR_USB_RMNET2
#define SIO_MINOR_FSUSB_RMNET3    SIO_MINOR_USB_RMNET3

/* HS-USB DRIVER */

#define SIO_MINOR_HSUSB_MDM       SIO_MINOR_USB_MDM
#define SIO_MINOR_HSUSB_SER1      SIO_MINOR_USB_SER1
#define SIO_MINOR_HSUSB_SER2      SIO_MINOR_USB_SER2
#define SIO_MINOR_HSUSB_SER3      SIO_MINOR_USB_SER3
#define SIO_MINOR_HSUSB_MMC       SIO_MINOR_USB_MMC
#define SIO_MINOR_HSUSB_RRDM      SIO_MINOR_USB_RRDM
#define SIO_MINOR_HSUSB_NET_WWAN  SIO_MINOR_USB_NET_WWAN
#define SIO_MINOR_HSUSB_RMNET2    SIO_MINOR_USB_RMNET2
#define SIO_MINOR_HSUSB_RMNET3    SIO_MINOR_USB_RMNET3
#define SIO_MINOR_HSUSB_EEM       SIO_MINOR_USB_EEM
#define SIO_MINOR_HSUSB_RMNET4    SIO_MINOR_USB_RMNET4
#define SIO_MINOR_HSUSB_RMNET5    SIO_MINOR_USB_RMNET5

/* HS-USB-LCM DRIVER */

#define SIO_MINOR_HSUSB_ECM_NET_WWAN    SIO_MINOR_USB_NET_WWAN

/* SMD DRIVER */

#define SIO_MINOR_SMD_FIRST             1   /* used for registration loop */
#define SIO_MINOR_SMD_DS                1   /**< SIO Minor for SMD DS device*/
#define SIO_MINOR_SMD_DIAG              2   /**< SIO Minor for SMD Diag device*/
#define SIO_MINOR_SMD_DIAG_APPS         3   /**< SIO Minor for SMD Diag-to-apps device*/
#define SIO_MINOR_SMD_DIAG_MODEM        4   /**< SIO Minor for SMD Diag-to-modem device*/
#define SIO_MINOR_SMD_DIAG_QDSP         5   /**< SIO Minor for SMD Diag-to-Qdsp6 device*/
#define SIO_MINOR_SMD_RPC_CALL          6   /**< SIO Minor for SMD RPC device*/
#define SIO_MINOR_SMD_RPC_REPLY         7   /**< SIO Minor for SMD RPC reply device*/
#define SIO_MINOR_SMD_BT                8   /**< SIO Minor for SMD Bluetooth device*/
#define SIO_MINOR_SMD_CONTROL           9   /**< SIO Minor for SMD Control device*/
#define SIO_MINOR_SMD_MEMCPY_SPARE1     10  /**< SIO Minor for SMD Memcopy spare 1 device*/
#define SIO_MINOR_SMD_DATA1             11  /**< SIO Minor for SMD data1 device*/
#define SIO_MINOR_SMD_DATA2             12  /**< SIO Minor for SMD data2 device*/
#define SIO_MINOR_SMD_DATA3             13  /**< SIO Minor for SMD data3 device*/
#define SIO_MINOR_SMD_DATA4             14  /**< SIO Minor for SMD data4 device*/
#define SIO_MINOR_SMD_DATA5             15  /**< SIO Minor for SMD data5 device*/
#define SIO_MINOR_SMD_DATA6             16  /**< SIO Minor for SMD data6 device*/
#define SIO_MINOR_SMD_DATA7             17  /**< SIO Minor for SMD data7 device*/
#define SIO_MINOR_SMD_DATA8             18  /**< SIO Minor for SMD data8 device*/
#define SIO_MINOR_SMD_DATA9             19  /**< SIO Minor for SMD data9 device*/
#define SIO_MINOR_SMD_DATA10            20  /**< SIO Minor for SMD data10 device*/
#define SIO_MINOR_SMD_DATA11            21  /**< SIO Minor for SMD data11 device*/
#define SIO_MINOR_SMD_DATA12            22  /**< SIO Minor for SMD data12 device*/
#define SIO_MINOR_SMD_DATA13            23  /**< SIO Minor for SMD data13 device*/
#define SIO_MINOR_SMD_DATA14            24  /**< SIO Minor for SMD data14 device*/
#define SIO_MINOR_SMD_DATA15            25  /**< SIO Minor for SMD data15 device*/
#define SIO_MINOR_SMD_DATA16            26  /**< SIO Minor for SMD data16 device*/
#define SIO_MINOR_SMD_DATA17            27  /**< SIO Minor for SMD data17 device*/
#define SIO_MINOR_SMD_DATA18            28  /**< SIO Minor for SMD data18 device*/
#define SIO_MINOR_SMD_DATA19            29  /**< SIO Minor for SMD data19 device*/
#define SIO_MINOR_SMD_DATA20            30  /**< SIO Minor for SMD data20 device*/
#define SIO_MINOR_SMD_DATA21            31  /**< SIO Minor for SMD data21 device*/
#define SIO_MINOR_SMD_DATA22            32  /**< SIO Minor for SMD data22 device*/
#define SIO_MINOR_SMD_DATA23            33  /**< SIO Minor for SMD data23 device*/
#define SIO_MINOR_SMD_DATA24            34  /**< SIO Minor for SMD data24 device*/
#define SIO_MINOR_SMD_DATA25            35  /**< SIO Minor for SMD data25 device*/
#define SIO_MINOR_SMD_DATA26            36  /**< SIO Minor for SMD data26 device*/
#define SIO_MINOR_SMD_DATA27            37  /**< SIO Minor for SMD data27 device*/
#define SIO_MINOR_SMD_DATA28            38  /**< SIO Minor for SMD data28 device*/
#define SIO_MINOR_SMD_DATA29            39  /**< SIO Minor for SMD data29 device*/
#define SIO_MINOR_SMD_DATA30            40  /**< SIO Minor for SMD data30 device*/
#define SIO_MINOR_SMD_DATA31            41  /**< SIO Minor for SMD data31 device*/
#define SIO_MINOR_SMD_DATA32            42  /**< SIO Minor for SMD data32 device*/
#define SIO_MINOR_SMD_DATA33            43  /**< SIO Minor for SMD data33 device*/
#define SIO_MINOR_SMD_DATA34            44  /**< SIO Minor for SMD data34 device*/
#define SIO_MINOR_SMD_DATA35            45  /**< SIO Minor for SMD data35 device*/
#define SIO_MINOR_SMD_DATA36            46  /**< SIO Minor for SMD data36 device*/
#define SIO_MINOR_SMD_DATA37            47  /**< SIO Minor for SMD data37 device*/
#define SIO_MINOR_SMD_DATA38            48  /**< SIO Minor for SMD data38 device*/
#define SIO_MINOR_SMD_DATA39            49  /**< SIO Minor for SMD data39 device*/
#define SIO_MINOR_SMD_DATA40            50  /**< SIO Minor for SMD data40 device*/
#define SIO_MINOR_SMD_GPS_NMEA          51  /**< SIO Minor for SMD GPS NMEA device*/
#define SIO_MINOR_SMD_DIAG2             52  /**< SIO Minor for SMD Diag device (channel 2)*/
#define SIO_MINOR_SMD_MAX               52  /* used for registration loop */

/* SMD-BRIDGE DRIVER */

#define SIO_MINOR_SMD_BRIDGE_FIRST      1   /* used for registration loop */
#define SIO_MINOR_SMD_BRIDGE_LEGACY     0   /**< SIO Minor for SMD Bridge device legacy*/
#define SIO_MINOR_SMD_BRIDGE_1          1   /**< SIO Minor for SMD Bridge device 1*/
#define SIO_MINOR_SMD_BRIDGE_2          2   /**< SIO Minor for SMD Bridge device 2*/
#define SIO_MINOR_SMD_BRIDGE_3          3   /**< SIO Minor for SMD Bridge device 3*/
#define SIO_MINOR_SMD_BRIDGE_4          4   /**< SIO Minor for SMD Bridge device 4*/
#define SIO_MINOR_SMD_BRIDGE_5          5   /**< SIO Minor for SMD Bridge device 5*/
#define SIO_MINOR_SMD_BRIDGE_MAX        5   /* used for registration loop */

/* SMD-BRIDGE DRIVER NEW */
/** Use SIO_MINOR_SMD_BRIDGE_LEGACY for legacy dual proc target's driver */
/** SIO Minor for SMD Bridge to application processor */
#define SIO_MINOR_SMD_BRIDGE_TO_APPS    SIO_MAJOR_SMD_TO_APPS
/** SIO Minor for SMD Bridge to modem processor */
#define SIO_MINOR_SMD_BRIDGE_TO_MODEM   SIO_MAJOR_SMD_TO_MODEM
/** SIO Minor for SMD Bridge to qdsp6 processor */
#define SIO_MINOR_SMD_BRIDGE_TO_QDSP6   SIO_MAJOR_SMD_TO_QDSP6

/* SMEM DRIVER */

#define SIO_MINOR_SMEM_DS               1   /**< SIO Minor for SMEM DS device*/

/* BT DRIVER */

#define SIO_MINOR_BLUETOOTH_SPP         1   /**< SIO Minor for Bluetooth SPP device*/
#define SIO_MINOR_BLUETOOTH_NA          2   /**< SIO Minor for Bluetooth NA device*/

/* MMGPS DRIVER */

#define SIO_MINOR_MMGPS_LSM_CSD         1   /**< SIO Minor for MMGPS LSM CSD device*/

/* APS DRIVER */

#define SIO_MINOR_APS_DEV               1   /**< SIO Minor for APS_DEV device*/

/* A2 DRIVER */

#define SIO_MINOR_A2_MDM                0   /**< SIO Minor for A2 MDM device*/
#define SIO_MINOR_A2_RMNET_1            1   /**< SIO Minor for A2 RMNET1 device*/
#define SIO_MINOR_A2_RMNET_2            2   /**< SIO Minor for A2 RMNET2 device*/
#define SIO_MINOR_A2_RMNET_3            3   /**< SIO Minor for A2 RMNET3 device*/
/* For SDIO-A2 MUX  */
#define SIO_MINOR_SDIO_MUX_A2_RMNET_0   4   /**< SIO Minor for A2 MUX RMNET0 device*/
#define SIO_MINOR_SDIO_MUX_A2_RMNET_1   5   /**< SIO Minor for A2 MUX RMNET1 device*/
#define SIO_MINOR_SDIO_MUX_A2_RMNET_2   6   /**< SIO Minor for A2 MUX RMNET2 device*/
#define SIO_MINOR_SDIO_MUX_A2_RMNET_3   7   /**< SIO Minor for A2 MUX RMNET3 device*/
#define SIO_MINOR_SDIO_MUX_A2_RMNET_4   8   /**< SIO Minor for A2 MUX RMNET4 device*/
#define SIO_MINOR_SDIO_MUX_A2_RMNET_5   9   /**< SIO Minor for A2 MUX RMNET5 device*/
#define SIO_MINOR_SDIO_MUX_A2_RMNET_6   10  /**< SIO Minor for A2 MUX RMNET6 device*/
#define SIO_MINOR_SDIO_MUX_A2_RMNET_7   11  /**< SIO Minor for A2 MUX RMNET7 device*/
#define SIO_MINOR_SDIO_MUX_A2_RMNET_TETH_0 12  /**< SIO Minor for A2 MUX RMNET Tethered 0 device*/
#define SIO_MINOR_SDIO_MUX_A2_DUN_0     13  /**< SIO Minor for A2 MUX DUN 0 device*/
#define SIO_MINOR_SDIO_MUX_A2_DUN_CTL_0 14  /**< SIO Minor for A2 MUX DUN CTL 0 device*/
#define SIO_MINOR_SDIO_MUX_A2_RMNET_8   17  /**< SIO Minor for A2 MUX RMNET8 device*/
#define SIO_MINOR_SDIO_MUX_A2_RMNET_9   18  /**< SIO Minor for A2 MUX RMNET9 device*/

/* For A2 MUX MINOR Alias */
#define SIO_MINOR_MUX_A2_RMNET_0   SIO_MINOR_SDIO_MUX_A2_RMNET_0
#define SIO_MINOR_MUX_A2_RMNET_1   SIO_MINOR_SDIO_MUX_A2_RMNET_1
#define SIO_MINOR_MUX_A2_RMNET_2   SIO_MINOR_SDIO_MUX_A2_RMNET_2
#define SIO_MINOR_MUX_A2_RMNET_3   SIO_MINOR_SDIO_MUX_A2_RMNET_3
#define SIO_MINOR_MUX_A2_RMNET_4   SIO_MINOR_SDIO_MUX_A2_RMNET_4
#define SIO_MINOR_MUX_A2_RMNET_5   SIO_MINOR_SDIO_MUX_A2_RMNET_5
#define SIO_MINOR_MUX_A2_RMNET_6   SIO_MINOR_SDIO_MUX_A2_RMNET_6
#define SIO_MINOR_MUX_A2_RMNET_7   SIO_MINOR_SDIO_MUX_A2_RMNET_7
#define SIO_MINOR_MUX_A2_RMNET_TETH_0 SIO_MINOR_SDIO_MUX_A2_RMNET_TETH_0
#define SIO_MINOR_MUX_A2_DUN_0     SIO_MINOR_SDIO_MUX_A2_DUN_0
#define SIO_MINOR_MUX_A2_RMNET_8   SIO_MINOR_SDIO_MUX_A2_RMNET_8
#define SIO_MINOR_MUX_A2_RMNET_9   SIO_MINOR_SDIO_MUX_A2_RMNET_9

/* DATA MUX Driver */

#define SIO_MINOR_DATA_MUX_1             1  /**< SIO Minor for Data MUX1  device*/
#define SIO_MINOR_DATA_MUX_2             2  /**< SIO Minor for Data MUX2  device*/
#define SIO_MINOR_DATA_MUX_3             3  /**< SIO Minor for Data MUX3  device*/
#define SIO_MINOR_DATA_MUX_4             4  /**< SIO Minor for Data MUX4  device*/
#define SIO_MINOR_DATA_MUX_5             5  /**< SIO Minor for Data MUX5  device*/
#define SIO_MINOR_DATA_MUX_6             6  /**< SIO Minor for Data MUX6  device*/
#define SIO_MINOR_DATA_MUX_7             7  /**< SIO Minor for Data MUX7  device*/
#define SIO_MINOR_DATA_MUX_8             8  /**< SIO Minor for Data MUX8  device*/
#define SIO_MINOR_DATA_MUX_9             9  /**< SIO Minor for Data MUX9  device*/
#define SIO_MINOR_DATA_MUX_10            10 /**< SIO Minor for Data MUX10 device*/
#define SIO_MINOR_DATA_MUX_11            11 /**< SIO Minor for Data MUX11 device*/
#define SIO_MINOR_DATA_MUX_12            12 /**< SIO Minor for Data MUX12 device*/
#define SIO_MINOR_DATA_MUX_13            13 /**< SIO Minor for Data MUX13 device*/
#define SIO_MINOR_DATA_MUX_14            14 /**< SIO Minor for Data MUX14 device*/
#define SIO_MINOR_DATA_MUX_15            15 /**< SIO Minor for Data MUX15 device*/
#define SIO_MINOR_DATA_MUX_16            16 /**< SIO Minor for Data MUX16 device*/
#define SIO_MINOR_DATA_MUX_MAX           16 /* Used for registration */

/* ONDEVICE Diag driver */

#define SIO_MINOR_ONDEVICE_DIAG          1 /**< SIO Minor for Ondevice diag device*/

/* ONDEVICE Diag USB Auto driver */

#define SIO_MINOR_ONDEVICE_DIAG_USB_AUTO 1 /**< SIO Minor for Ondevice Diag Auto */

/* SDIOC client driver */

#define SIO_MINOR_SDIOC_RPC              1  /**< SIO Minor for SDIOC RPC device*/
#define SIO_MINOR_SDIOC_WWAN             2  /**< SIO Minor for SDIOC WWAN device*/
#define SIO_MINOR_SDIOC_DIAG             3  /**< SIO Minor for SDIOC DIAG device*/
#define SIO_MINOR_SDIOC_CDC_ACM          4  /**< SIO Minor for SDIOC CDC ACM device*/
#define SIO_MINOR_SDIOC_CIQ              5  /**< SIO Minor for SDIOC CIQ device*/

/**
   @}
*/

#define SIO_MAC_ADDR_STR_SIZE 13

/*---------------------------------------------------------------------------*/
/** sio_port_id_type is used to identify various SIO devices.
    Sio port id is 16 bit number, which comprises SIO_MAJOR_NUMBER and
    SIO_MINOR_NUMBER. (SIO_PORT_ID = DEVICE_ID(SIO_MAJOR_NUMBER,
    SIO_MINOR_NUMBER). Upper byte indicates the major number while lower byte
    indicates the minor number. Clients can use SIO port id to communicate with
    given logical port of a particular driver, where logical port is identified
    by minor number and the driver is identified by major number. SIO major
    number is assigned to each unique driver interface. Each Major number can
    support certain number of minor numbers (logical ports), maximum minor
    number is limited to 64. In other words, drivers can support as many as 64
    logical ports with one major number.  */
/*---------------------------------------------------------------------------*/

typedef enum
{
    /** Non-existent/NULL SIO Port.*/
  SIO_PORT_NULL = DEVICE_ID(SIO_MAJOR_LEGACY, SIO_MINOR_LEGACY_NULL ),

  /*--------------------- UART DRIVER --------------------- */

    /** -- SIO Port id for UART 1/main device */
  SIO_PORT_UART_MAIN = DEVICE_ID(SIO_MAJOR_UART , SIO_MINOR_UART_1 ),
    /** -- SIO Port id for UART 2/AUX device */
  SIO_PORT_UART_AUX = DEVICE_ID(SIO_MAJOR_UART , SIO_MINOR_UART_2 ),
    /** -- SIO Port id for UART 3 device */
  SIO_PORT_UART_THIRD = DEVICE_ID(SIO_MAJOR_UART , SIO_MINOR_UART_3 ),

  /*---------------------  USB DRIVER --------------------- */

    /** -- SIO Port id for USB MDM device */
  SIO_PORT_USB_MDM = DEVICE_ID(SIO_MAJOR_USB , SIO_MINOR_FSUSB_MDM ),
    /** -- SIO Port id for USB CDC ACM device */
  SIO_PORT_USB_CDC_ACM = SIO_PORT_USB_MDM,
    /** -- SIO Port id for USB serial 1 device */
  SIO_PORT_USB_SER1 = DEVICE_ID(SIO_MAJOR_USB , SIO_MINOR_USB_SER1 ),
    /** -- SIO Port id for USB Diag device */
  SIO_PORT_USB_DIAG = SIO_PORT_USB_SER1,
    /** -- SIO Port id for USB MMC device */
  SIO_PORT_USB_MMC = DEVICE_ID(SIO_MAJOR_USB , SIO_MINOR_USB_MMC ),
    /** -- SIO Port id for USB serial 2 device */
  SIO_PORT_USB_SER2 = DEVICE_ID(SIO_MAJOR_USB , SIO_MINOR_USB_SER2 ),
    /** -- SIO Port id for USB NMEA device */
  SIO_PORT_USB_NMEA = SIO_PORT_USB_SER2,
    /** -- SIO Port id for USB RRDM device */
  SIO_PORT_USB_RRDM = DEVICE_ID(SIO_MAJOR_USB , SIO_MINOR_USB_RRDM ),
    /** -- SIO Port id for USB NET WWAN/RMNET1 device */
  SIO_PORT_USB_NET_WWAN = DEVICE_ID(SIO_MAJOR_HSUSB_ECM , SIO_MINOR_USB_NET_WWAN ),
    /** -- SIO Port id for USB RMNET 2 device */
  SIO_PORT_USB_RMNET2 = DEVICE_ID(SIO_MAJOR_HSUSB_ECM , SIO_MINOR_USB_RMNET2 ),
    /** -- SIO Port id for USB RMNET 3 device */
  SIO_PORT_USB_RMNET3 = DEVICE_ID(SIO_MAJOR_HSUSB_ECM , SIO_MINOR_USB_RMNET3 ),
     /** -- SIO Port id for USB RMNET 4 device */
  SIO_PORT_USB_RMNET4 = DEVICE_ID(SIO_MAJOR_HSUSB_ECM , SIO_MINOR_USB_RMNET4 ),
     /** -- SIO Port id for USB RMNET 5 device */
  SIO_PORT_USB_RMNET5 = DEVICE_ID(SIO_MAJOR_HSUSB_ECM , SIO_MINOR_USB_RMNET5 ),
    /** -- SIO Port id for USB serial 3 device */
  SIO_PORT_USB_SER3 = DEVICE_ID(SIO_MAJOR_USB , SIO_MINOR_USB_SER3 ),
    /** -- SIO Port id for USB BREW extended data device */
  SIO_PORT_USB_BREW_EXTENDED_DATA = DEVICE_ID(SIO_MAJOR_USB , SIO_MINOR_USB_BREW_X_DATA ),
    /** -- SIO Port id for USB BREW notification device */
  SIO_PORT_USB_BREW_EXTENDED_NOTIFICATION = DEVICE_ID(SIO_MAJOR_USB , SIO_MINOR_USB_BREW_X_NOTIFICATION ),
    /** -- SIO Port id for USB EEM device */
  SIO_PORT_USB_EEM = DEVICE_ID(SIO_MAJOR_HSUSB_EEM , SIO_MINOR_HSUSB_EEM),

  /*---------------------  SMD DRIVER --------------------- */

    /** -- SIO Port id for SMD First device */
  SIO_PORT_SMD_FIRST = DEVICE_ID(SIO_MAJOR_SMD , SIO_MINOR_SMD_DS ),
    /** -- SIO Port id for SMD Data(DS) device */
  SIO_PORT_SMD_DS = SIO_PORT_SMD_FIRST,
    /** -- SIO Port id for SMD Diag device */
  SIO_PORT_SMD_DIAG = DEVICE_ID(SIO_MAJOR_SMD , SIO_MINOR_SMD_DIAG ),
    /** -- SIO Port id for SMD Diag-to-apps device */
  SIO_PORT_SMD_DIAG_APPS = DEVICE_ID(SIO_MAJOR_SMD_TO_APPS , SIO_MINOR_SMD_DIAG ),
    /** -- SIO Port id for SMD Diag-to-modem Ddevice */
  SIO_PORT_SMD_DIAG_MODEM = DEVICE_ID(SIO_MAJOR_SMD_TO_MODEM , SIO_MINOR_SMD_DIAG ),
    /** -- SIO Port id for SMD Diag-to-QDSP6 device */
  SIO_PORT_SMD_DIAG_QDSP = DEVICE_ID(SIO_MAJOR_SMD_TO_QDSP6 , SIO_MINOR_SMD_DIAG ),
    /** -- SIO Port id for SMD RPC call device */
  SIO_PORT_SMD_RPC_CALL = DEVICE_ID(SIO_MAJOR_SMD , SIO_MINOR_SMD_RPC_CALL ),
    /** -- SIO Port id for SMD RPC reply device */
  SIO_PORT_SMD_RPC_REPLY = DEVICE_ID(SIO_MAJOR_SMD , SIO_MINOR_SMD_RPC_REPLY),
    /** -- SIO Port id for SMD Bluetooth device */
  SIO_PORT_SMD_BT = DEVICE_ID(SIO_MAJOR_SMD , SIO_MINOR_SMD_BT ),
    /** -- SIO Port id for SMD control device */
  SIO_PORT_SMD_CONTROL = DEVICE_ID(SIO_MAJOR_SMD , SIO_MINOR_SMD_CONTROL ),
    /** -- SIO Port id for SMD Memcopy spare 1 device */
  SIO_PORT_SMD_MEMCPY_SPARE1 = DEVICE_ID(SIO_MAJOR_SMD , SIO_MINOR_SMD_MEMCPY_SPARE1 ),
    /** -- SIO Port id for SMD Data first device */
  SIO_PORT_SMD_DATA_FIRST = DEVICE_ID(SIO_MAJOR_SMD , SIO_MINOR_SMD_DATA1 ),
    /** -- SIO Port id for SMD data1 device */
  SIO_PORT_SMD_DATA1 = SIO_PORT_SMD_DATA_FIRST,
    /** -- SIO Port id for SMD Winmobile modem port1 device */
  SIO_PORT_SMD_WINMOB_MODEM_PORT1 = SIO_PORT_SMD_DATA1,
    /** -- SIO Port id for SMD data2 device */
  SIO_PORT_SMD_DATA2 = DEVICE_ID(SIO_MAJOR_SMD , SIO_MINOR_SMD_DATA2 ),
    /** -- SIO Port id for SMD Winmobile modem port2 device */
  SIO_PORT_SMD_WINMOB_MODEM_PORT2 = SIO_PORT_SMD_DATA2,
    /** -- SIO Port id for SMD data3 device */
  SIO_PORT_SMD_DATA3 = DEVICE_ID(SIO_MAJOR_SMD , SIO_MINOR_SMD_DATA3 ),
    /** -- SIO Port id for SMD winmobile modem port3 device */
  SIO_PORT_SMD_WINMOB_MODEM_PORT3 = SIO_PORT_SMD_DATA3,
    /** -- SIO Port id for SMD data4 device */
  SIO_PORT_SMD_DATA4 = DEVICE_ID(SIO_MAJOR_SMD , SIO_MINOR_SMD_DATA4 ),
    /** -- SIO Port id for SMD winmobile modem port4 device */
  SIO_PORT_SMD_WINMOB_MODEM_PORT4 = SIO_PORT_SMD_DATA4,
    /** -- SIO Port id for SMD data5 device */
  SIO_PORT_SMD_DATA5 = DEVICE_ID(SIO_MAJOR_SMD , SIO_MINOR_SMD_DATA5 ),
    /** -- SIO Port id for SMD winmobile QMI WWAN device */
  SIO_PORT_SMD_WINMOB_QMI_WWAN = SIO_PORT_SMD_DATA5,
    /** -- SIO Port id for SMD RMNET1 device */
  SIO_PORT_SMD_RMNET1 = SIO_PORT_SMD_DATA5,
    /** -- SIO Port id for SMD data6 device */
  SIO_PORT_SMD_DATA6 = DEVICE_ID(SIO_MAJOR_SMD , SIO_MINOR_SMD_DATA6 ),
    /** -- SIO Port id for SMD RMNET2 device */
  SIO_PORT_SMD_RMNET2 = SIO_PORT_SMD_DATA6,
    /** -- SIO Port id for SMD data7 device */
  SIO_PORT_SMD_DATA7 = DEVICE_ID(SIO_MAJOR_SMD , SIO_MINOR_SMD_DATA7 ),
    /** -- SIO Port id for SMD RMNET3 device */
  SIO_PORT_SMD_RMNET3 = SIO_PORT_SMD_DATA7,
    /** -- SIO Port id for SMD data8 device */
  SIO_PORT_SMD_DATA8 = DEVICE_ID(SIO_MAJOR_SMD , SIO_MINOR_SMD_DATA8 ),
    /** -- SIO Port id for SMD RMNET4 device */
  SIO_PORT_SMD_RMNET4 = SIO_PORT_SMD_DATA8,
    /** -- SIO Port id for SMD data9 device */
  SIO_PORT_SMD_DATA9 = DEVICE_ID(SIO_MAJOR_SMD , SIO_MINOR_SMD_DATA9 ),
    /** -- SIO Port id for SMD RMNET5 device */
  SIO_PORT_SMD_RMNET5 = SIO_PORT_SMD_DATA9,
    /** -- SIO Port id for SMD data10 evice */
  SIO_PORT_SMD_DATA10 = DEVICE_ID(SIO_MAJOR_SMD , SIO_MINOR_SMD_DATA10 ),
    /** -- SIO Port id for SMD winmobile QVP MVS device */
  SIO_PORT_SMD_WINMOB_QVP_MVS = SIO_PORT_SMD_DATA10,
    /** -- SIO Port id for SMD data11 device */
  SIO_PORT_SMD_DATA11 = DEVICE_ID(SIO_MAJOR_SMD , SIO_MINOR_SMD_DATA11 ),
    /** -- SIO Port id for SMD winmobile QVP DATA device */
  SIO_PORT_SMD_WINMOB_QVP_DATA = SIO_PORT_SMD_DATA11,
    /** -- SIO Port id for SMD data12 device */
  SIO_PORT_SMD_DATA12 = DEVICE_ID(SIO_MAJOR_SMD , SIO_MINOR_SMD_DATA12 ),
    /** -- SIO Port id for SMD RMNET6 device */
  SIO_PORT_SMD_RMNET6 = SIO_PORT_SMD_DATA12,
    /** -- SIO Port id for SMD data13 device */
  SIO_PORT_SMD_DATA13 = DEVICE_ID(SIO_MAJOR_SMD , SIO_MINOR_SMD_DATA13 ),
    /** -- SIO Port id for SMD RMNET7 device */
  SIO_PORT_SMD_RMNET7 = SIO_PORT_SMD_DATA13,
    /** -- SIO Port id for SMD data14 device */
  SIO_PORT_SMD_DATA14 = DEVICE_ID(SIO_MAJOR_SMD , SIO_MINOR_SMD_DATA14 ),
    /** -- SIO Port id for SMD RMNET8 device */
  SIO_PORT_SMD_RMNET8 = SIO_PORT_SMD_DATA14,
    /** -- SIO Port id for SMD data15 device */
  SIO_PORT_SMD_DATA15 = DEVICE_ID(SIO_MAJOR_SMD , SIO_MINOR_SMD_DATA15 ),
    /** -- SIO Port id for SMD data16 device */
  SIO_PORT_SMD_DATA16 = DEVICE_ID(SIO_MAJOR_SMD , SIO_MINOR_SMD_DATA16 ),
    /** -- SIO Port id for SMD data17 device */
  SIO_PORT_SMD_DATA17 = DEVICE_ID(SIO_MAJOR_SMD , SIO_MINOR_SMD_DATA17 ),
    /** -- SIO Port id for SMD data18 device */
  SIO_PORT_SMD_DATA18 = DEVICE_ID(SIO_MAJOR_SMD , SIO_MINOR_SMD_DATA18 ),
    /** -- SIO Port id for SMD data19 device */
  SIO_PORT_SMD_DATA19 = DEVICE_ID(SIO_MAJOR_SMD , SIO_MINOR_SMD_DATA19 ),
    /** -- SIO Port id for SMD data20 device */
  SIO_PORT_SMD_DATA20 = DEVICE_ID(SIO_MAJOR_SMD , SIO_MINOR_SMD_DATA20 ),
    /** -- SIO Port id for SMD data21 device */
  SIO_PORT_SMD_DATA21 = DEVICE_ID(SIO_MAJOR_SMD , SIO_MINOR_SMD_DATA21 ),
    /** -- SIO Port id for SMD data22 device */
  SIO_PORT_SMD_DATA22 = DEVICE_ID(SIO_MAJOR_SMD , SIO_MINOR_SMD_DATA22 ),
    /** -- SIO Port id for SMD data23 device */
  SIO_PORT_SMD_DATA23 = DEVICE_ID(SIO_MAJOR_SMD , SIO_MINOR_SMD_DATA23 ),
    /** -- SIO Port id for SMD data24 device */
  SIO_PORT_SMD_DATA24 = DEVICE_ID(SIO_MAJOR_SMD , SIO_MINOR_SMD_DATA24 ),
    /** -- SIO Port id for SMD data25 device */
  SIO_PORT_SMD_DATA25 = DEVICE_ID(SIO_MAJOR_SMD , SIO_MINOR_SMD_DATA25 ),
    /** -- SIO Port id for SMD data26 device */
  SIO_PORT_SMD_DATA26 = DEVICE_ID(SIO_MAJOR_SMD , SIO_MINOR_SMD_DATA26 ),
    /** -- SIO Port id for SMD data27 device */
  SIO_PORT_SMD_DATA27 = DEVICE_ID(SIO_MAJOR_SMD , SIO_MINOR_SMD_DATA27 ),
    /** -- SIO Port id for SMD data28 device */
  SIO_PORT_SMD_DATA28 = DEVICE_ID(SIO_MAJOR_SMD , SIO_MINOR_SMD_DATA28 ),
    /** -- SIO Port id for SMD data29 device */
  SIO_PORT_SMD_DATA29 = DEVICE_ID(SIO_MAJOR_SMD , SIO_MINOR_SMD_DATA29 ),
    /** -- SIO Port id for SMD data30 device */
  SIO_PORT_SMD_DATA30 = DEVICE_ID(SIO_MAJOR_SMD , SIO_MINOR_SMD_DATA30 ),
    /** -- SIO Port id for SMD data31 device */
  SIO_PORT_SMD_DATA31 = DEVICE_ID(SIO_MAJOR_SMD , SIO_MINOR_SMD_DATA31 ),
    /** -- SIO Port id for SMD data32 device */
  SIO_PORT_SMD_DATA32 = DEVICE_ID(SIO_MAJOR_SMD , SIO_MINOR_SMD_DATA32 ),
    /** -- SIO Port id for SMD data33 device */
  SIO_PORT_SMD_DATA33 = DEVICE_ID(SIO_MAJOR_SMD , SIO_MINOR_SMD_DATA33 ),
    /** -- SIO Port id for SMD data34 device */
  SIO_PORT_SMD_DATA34 = DEVICE_ID(SIO_MAJOR_SMD , SIO_MINOR_SMD_DATA34 ),
    /** -- SIO Port id for SMD data35 device */
  SIO_PORT_SMD_DATA35 = DEVICE_ID(SIO_MAJOR_SMD , SIO_MINOR_SMD_DATA35 ),
    /** -- SIO Port id for SMD data36 device */
  SIO_PORT_SMD_DATA36 = DEVICE_ID(SIO_MAJOR_SMD , SIO_MINOR_SMD_DATA36 ),
    /** -- SIO Port id for SMD data37 device */
  SIO_PORT_SMD_DATA37 = DEVICE_ID(SIO_MAJOR_SMD , SIO_MINOR_SMD_DATA37 ),
    /** -- SIO Port id for SMD data38 device */
  SIO_PORT_SMD_DATA38 = DEVICE_ID(SIO_MAJOR_SMD , SIO_MINOR_SMD_DATA38 ),
    /** -- SIO Port id for SMD data39 device */
  SIO_PORT_SMD_DATA39 = DEVICE_ID(SIO_MAJOR_SMD , SIO_MINOR_SMD_DATA39 ),
    /** -- SIO Port id for SMD data40 device */
  SIO_PORT_SMD_DATA40 = DEVICE_ID(SIO_MAJOR_SMD , SIO_MINOR_SMD_DATA40 ),
    /** -- SIO Port id for SMD GPS NMEAdevice */
  SIO_PORT_SMD_GPS_NMEA = DEVICE_ID(SIO_MAJOR_SMD , SIO_MINOR_SMD_GPS_NMEA ),
    /** -- SIO Port id for SMD Diag device */
  SIO_PORT_SMD_DIAG2 = DEVICE_ID(SIO_MAJOR_SMD , SIO_MINOR_SMD_DIAG2 ),
    /** -- SIO Port id for SMD Last device */
  SIO_PORT_SMD_DATA_LAST = SIO_PORT_SMD_DIAG2,


  /*---------------------  SMD-BRIDGE DRIVER --------------------- */

  /** -- SIO Port id for SMD BRIDGE legacy device */
  SIO_PORT_SMD_BRIDGE_LEGACY = DEVICE_ID(SIO_MAJOR_SMD_BRIDGE , SIO_MINOR_SMD_BRIDGE_LEGACY),
    /** -- SIO Port id for SMD BRIDGE first device */
  SIO_PORT_SMD_BRIDGE_FIRST = DEVICE_ID(SIO_MAJOR_SMD_BRIDGE , SIO_MINOR_SMD_BRIDGE_1 ),
    /** -- SIO Port id for SMD BRIDGE 1 device */
  SIO_PORT_SMD_BRIDGE_1 = SIO_PORT_SMD_BRIDGE_FIRST,
    /** -- SIO Port id for SMD BRIDGE 2 device */
  SIO_PORT_SMD_BRIDGE_2 = DEVICE_ID(SIO_MAJOR_SMD_BRIDGE , SIO_MINOR_SMD_BRIDGE_2 ),
    /** -- SIO Port id for SMD BRIDGE 3 device */
  SIO_PORT_SMD_BRIDGE_3 = DEVICE_ID(SIO_MAJOR_SMD_BRIDGE , SIO_MINOR_SMD_BRIDGE_3 ),
    /** -- SIO Port id for SMD BRIDGE 4 device */
  SIO_PORT_SMD_BRIDGE_4 = DEVICE_ID(SIO_MAJOR_SMD_BRIDGE , SIO_MINOR_SMD_BRIDGE_4 ),
    /** -- SIO Port id for SMD BRIDGE 5 device */
  SIO_PORT_SMD_BRIDGE_5 = DEVICE_ID(SIO_MAJOR_SMD_BRIDGE, SIO_MINOR_SMD_BRIDGE_5 ),
    /** -- SIO Port id for SMD BRIDGE last device */
  SIO_PORT_SMD_BRIDGE_LAST = SIO_PORT_SMD_BRIDGE_5,

  /*---------------------  SMD-BRIDGE DRIVER - NEW --------------------- */

    /** -- SIO Port id for SMD Bridge-to-legacy device */
  SIO_PORT_SMD_BRIDGE_TO_LEGACY = DEVICE_ID(SIO_MAJOR_SMD_BRIDGE_NPROC, SIO_MINOR_SMD_BRIDGE_LEGACY),
    /** -- SIO Port id for SMD Bridge-to-apps device */
  SIO_PORT_SMD_BRIDGE_TO_APPS = DEVICE_ID(SIO_MAJOR_SMD_BRIDGE_NPROC, SIO_MINOR_SMD_BRIDGE_TO_APPS),
    /** -- SIO Port id for SMD Bridge-to-modem device */
  SIO_PORT_SMD_BRIDGE_TO_MODEM = DEVICE_ID(SIO_MAJOR_SMD_BRIDGE_NPROC, SIO_MINOR_SMD_BRIDGE_TO_MODEM),
    /** -- SIO Port id for SMD Bridge-to-qdsp6 device */
  SIO_PORT_SMD_BRIDGE_TO_QDSP6 = DEVICE_ID(SIO_MAJOR_SMD_BRIDGE_NPROC, SIO_MINOR_SMD_BRIDGE_TO_QDSP6),
  /*---------------------  SMEM DRIVER --------------------- */

    /** -- SIO Port id for SMEM DS device */
  SIO_PORT_SMEM_DS = DEVICE_ID(SIO_MAJOR_SMEM , SIO_MINOR_SMEM_DS ),

  /*---------------------  BT DRIVER --------------------- */

    /** -- SIO Port id for Bluetooth serial port profile device */
  SIO_PORT_BT_SPP = DEVICE_ID(SIO_MAJOR_BLUETOOTH , SIO_MINOR_BLUETOOTH_SPP ),
    /** -- SIO Port id for Bluetooth network access device */
  SIO_PORT_BT_NA = DEVICE_ID(SIO_MAJOR_BLUETOOTH , SIO_MINOR_BLUETOOTH_NA ),

  /*---------------------  MMGPS DRIVER --------------------- */

    /** -- SIO Port id for LSM LSD device */
  SIO_PORT_LSM_CSD = DEVICE_ID(SIO_MAJOR_MMGPS , SIO_MINOR_MMGPS_LSM_CSD ),

  /*---------------------  APS DRIVER --------------------- */

    /** -- SIO Port id for APS serial VSP device */
  SIO_PORT_APS_SERIAL_VSP = DEVICE_ID(SIO_MAJOR_APS , SIO_MINOR_APS_DEV ),

  /*---------------------  QMIP DRIVER --------------------- */

    /** -- SIO Port id for QMIP device */
  SIO_PORT_QMIP = DEVICE_ID(SIO_MAJOR_LEGACY , SIO_MINOR_LEGACY_NULL ),

  /*---------------------  A2 DRIVER --------------------- */

    /** -- SIO Port id for A2 Modem device */
  SIO_PORT_A2_MDM = DEVICE_ID(SIO_MAJOR_A2 , SIO_MINOR_A2_MDM ),
    /** -- SIO Port id for A2 RMNET1 device */
  SIO_PORT_A2_RMNET_1 = DEVICE_ID(SIO_MAJOR_A2 , SIO_MINOR_A2_RMNET_1),
    /** -- SIO Port id for A2 RMNET2 device */
  SIO_PORT_A2_RMNET_2 = DEVICE_ID(SIO_MAJOR_A2 , SIO_MINOR_A2_RMNET_2),
    /** -- SIO Port id for A2 RMNET3 device */
  SIO_PORT_A2_RMNET_3 = DEVICE_ID(SIO_MAJOR_A2 , SIO_MINOR_A2_RMNET_3),

  /*---------------------  Generic A2 Ports --------------------- */

    /** -- SIO Port id for A2 RMNET0 device */
  SIO_PORT_SDIO_MUX_A2_RMNET_0 = DEVICE_ID(SIO_MAJOR_A2 , SIO_MINOR_SDIO_MUX_A2_RMNET_0),
  SIO_PORT_MUX_A2_RMNET_0 = SIO_PORT_SDIO_MUX_A2_RMNET_0,
    /** -- SIO Port id for A2 RMNET1 device */
  SIO_PORT_SDIO_MUX_A2_RMNET_1 = DEVICE_ID(SIO_MAJOR_A2 , SIO_MINOR_SDIO_MUX_A2_RMNET_1),
  SIO_PORT_MUX_A2_RMNET_1 = SIO_PORT_SDIO_MUX_A2_RMNET_1,
    /** -- SIO Port id for A2 RMNET2 device */
  SIO_PORT_SDIO_MUX_A2_RMNET_2 = DEVICE_ID(SIO_MAJOR_A2 , SIO_MINOR_SDIO_MUX_A2_RMNET_2),
  SIO_PORT_MUX_A2_RMNET_2 = SIO_PORT_SDIO_MUX_A2_RMNET_2,
    /** -- SIO Port id for A2 RMNET3 device */
  SIO_PORT_SDIO_MUX_A2_RMNET_3 = DEVICE_ID(SIO_MAJOR_A2 , SIO_MINOR_SDIO_MUX_A2_RMNET_3),
  SIO_PORT_MUX_A2_RMNET_3 = SIO_PORT_SDIO_MUX_A2_RMNET_3,
    /** -- SIO Port id for A2 RMNET4 device */
  SIO_PORT_SDIO_MUX_A2_RMNET_4 = DEVICE_ID(SIO_MAJOR_A2 , SIO_MINOR_SDIO_MUX_A2_RMNET_4),
  SIO_PORT_MUX_A2_RMNET_4 = SIO_PORT_SDIO_MUX_A2_RMNET_4,
    /** -- SIO Port id for A2 RMNET5 device */
  SIO_PORT_SDIO_MUX_A2_RMNET_5 = DEVICE_ID(SIO_MAJOR_A2 , SIO_MINOR_SDIO_MUX_A2_RMNET_5),
  SIO_PORT_MUX_A2_RMNET_5 = SIO_PORT_SDIO_MUX_A2_RMNET_5,
    /** -- SIO Port id for A2 RMNET6 device */
  SIO_PORT_SDIO_MUX_A2_RMNET_6 = DEVICE_ID(SIO_MAJOR_A2 , SIO_MINOR_SDIO_MUX_A2_RMNET_6),
  SIO_PORT_MUX_A2_RMNET_6 = SIO_PORT_SDIO_MUX_A2_RMNET_6,
    /** -- SIO Port id for A2 RMNET7 device */
  SIO_PORT_SDIO_MUX_A2_RMNET_7 = DEVICE_ID(SIO_MAJOR_A2 , SIO_MINOR_SDIO_MUX_A2_RMNET_7),
  SIO_PORT_MUX_A2_RMNET_7 = SIO_PORT_SDIO_MUX_A2_RMNET_7,
    /** -- SIO Port id for A2 RMNET Tethered 0 */
  SIO_PORT_SDIO_MUX_A2_RMNET_TETH_0 = DEVICE_ID(SIO_MAJOR_A2 , SIO_MINOR_SDIO_MUX_A2_RMNET_TETH_0),
  SIO_PORT_MUX_A2_RMNET_TETH_0 = SIO_PORT_SDIO_MUX_A2_RMNET_TETH_0,
    /** -- SIO Port id for A2 DUN 0 */
  SIO_PORT_SDIO_MUX_A2_DUN_0 = DEVICE_ID(SIO_MAJOR_A2 , SIO_MINOR_SDIO_MUX_A2_DUN_0),
  SIO_PORT_MUX_A2_DUN_0 = SIO_PORT_SDIO_MUX_A2_DUN_0,
    /** -- SIO Port id for A2 DUN CTL 0 - USED BY A2 Driver */
  SIO_PORT_SDIO_MUX_A2_DUN_CTL_0 = DEVICE_ID(SIO_MAJOR_A2 , SIO_MINOR_SDIO_MUX_A2_DUN_CTL_0),
    /** -- SIO Port id for A2 RMNET8 device */
  SIO_PORT_SDIO_MUX_A2_RMNET_8 = DEVICE_ID(SIO_MAJOR_A2 , SIO_MINOR_SDIO_MUX_A2_RMNET_8),
  SIO_PORT_MUX_A2_RMNET_8 = SIO_PORT_SDIO_MUX_A2_RMNET_8,
    /** -- SIO Port id for A2 RMNET9 device */
  SIO_PORT_SDIO_MUX_A2_RMNET_9 = DEVICE_ID(SIO_MAJOR_A2 , SIO_MINOR_SDIO_MUX_A2_RMNET_9),
  SIO_PORT_MUX_A2_RMNET_9 = SIO_PORT_SDIO_MUX_A2_RMNET_9,

  /*---------------------  Data MUX Ports --------------------- */

    /** -- SIO Port id for DATA MUX 1 device */
  SIO_PORT_DATA_MUX_1  = DEVICE_ID(SIO_MAJOR_DATA_MUX, SIO_MINOR_DATA_MUX_1),
    /** -- SIO Port id for DATA MUX 2 device */
  SIO_PORT_DATA_MUX_2  = DEVICE_ID(SIO_MAJOR_DATA_MUX, SIO_MINOR_DATA_MUX_2),
    /** -- SIO Port id for DATA MUX 3 device */
  SIO_PORT_DATA_MUX_3  = DEVICE_ID(SIO_MAJOR_DATA_MUX, SIO_MINOR_DATA_MUX_3),
    /** -- SIO Port id for DATA MUX 4 device */
  SIO_PORT_DATA_MUX_4  = DEVICE_ID(SIO_MAJOR_DATA_MUX, SIO_MINOR_DATA_MUX_4),
    /** -- SIO Port id for DATA MUX 5 device */
  SIO_PORT_DATA_MUX_5  = DEVICE_ID(SIO_MAJOR_DATA_MUX, SIO_MINOR_DATA_MUX_5),
    /** -- SIO Port id for DATA MUX 6 device */
  SIO_PORT_DATA_MUX_6  = DEVICE_ID(SIO_MAJOR_DATA_MUX, SIO_MINOR_DATA_MUX_6),
    /** -- SIO Port id for DATA MUX 7 device */
  SIO_PORT_DATA_MUX_7  = DEVICE_ID(SIO_MAJOR_DATA_MUX, SIO_MINOR_DATA_MUX_7),
    /** -- SIO Port id for DATA MUX 8 device */
  SIO_PORT_DATA_MUX_8  = DEVICE_ID(SIO_MAJOR_DATA_MUX, SIO_MINOR_DATA_MUX_8),
    /** -- SIO Port id for DATA MUX 9 device */
  SIO_PORT_DATA_MUX_9  = DEVICE_ID(SIO_MAJOR_DATA_MUX, SIO_MINOR_DATA_MUX_9),
    /** -- SIO Port id for DATA MUX 10 device */
  SIO_PORT_DATA_MUX_10 = DEVICE_ID(SIO_MAJOR_DATA_MUX, SIO_MINOR_DATA_MUX_10),
    /** -- SIO Port id for DATA MUX 11 device */
  SIO_PORT_DATA_MUX_11 = DEVICE_ID(SIO_MAJOR_DATA_MUX, SIO_MINOR_DATA_MUX_11),
    /** -- SIO Port id for DATA MUX 12 device */
  SIO_PORT_DATA_MUX_12 = DEVICE_ID(SIO_MAJOR_DATA_MUX, SIO_MINOR_DATA_MUX_12),
    /** -- SIO Port id for DATA MUX 13 device */
  SIO_PORT_DATA_MUX_13 = DEVICE_ID(SIO_MAJOR_DATA_MUX, SIO_MINOR_DATA_MUX_13),
    /** -- SIO Port id for DATA MUX 14 device */
  SIO_PORT_DATA_MUX_14 = DEVICE_ID(SIO_MAJOR_DATA_MUX, SIO_MINOR_DATA_MUX_14),
    /** -- SIO Port id for DATA MUX 15 device */
  SIO_PORT_DATA_MUX_15 = DEVICE_ID(SIO_MAJOR_DATA_MUX, SIO_MINOR_DATA_MUX_15),
    /** -- SIO Port id for DATA MUX 16 device */
  SIO_PORT_DATA_MUX_16 = DEVICE_ID(SIO_MAJOR_DATA_MUX, SIO_MINOR_DATA_MUX_16),

  /*---------------------  ONDEVICE DIAG Ports --------------------- */

    /** -- SIO Port id for Ondevice diag device */
  SIO_PORT_ONDEVICE_DIAG = DEVICE_ID(SIO_MAJOR_ONDEVICE_DIAG, SIO_MINOR_ONDEVICE_DIAG),

  /*---------------------  SDIOC Ports --------------------- */

    /** -- SIO Port id for SDIOC RPC device */
  SIO_PORT_SDIOC_RPC  = DEVICE_ID(SIO_MAJOR_SDIOC_STREAM_DATA, SIO_MINOR_SDIOC_RPC),
    /** -- SIO Port id for SDIOC WWAN device */
  SIO_PORT_SDIOC_WWAN = DEVICE_ID(SIO_MAJOR_SDIOC_STREAM_DATA, SIO_MINOR_SDIOC_WWAN),
    /** -- SIO Port id for SDIOC DIAG device */
  SIO_PORT_SDIOC_DIAG = DEVICE_ID(SIO_MAJOR_SDIOC_PACKET_DATA, SIO_MINOR_SDIOC_DIAG),
    /** -- SIO Port id for SDIOC CDC ACM device */
  SIO_PORT_SDIOC_CDC_ACM = DEVICE_ID(SIO_MAJOR_SDIOC_STREAM_DATA, SIO_MINOR_SDIOC_CDC_ACM),
    /** -- SIO Port id for SDIOC CIQ device */
  SIO_PORT_SDIOC_CIQ  = DEVICE_ID(SIO_MAJOR_SDIOC_STREAM_DATA, SIO_MINOR_SDIOC_CIQ),

 /*---------------------  ONDEVICE DIAG Ports --------------------- */

    /** -- SIO Port id for Ondevice diag device */
  SIO_PORT_ONDEVICE_DIAG_USB_AUTO = DEVICE_ID(SIO_MAJOR_ONDEVICE_DIAG_USB_AUTO, SIO_MINOR_ONDEVICE_DIAG_USB_AUTO)

  /*---------------------------------------------------------*/
} sio_port_id_type;

/* Provide a synonymous define for _fusion variants */
#define ds_ucsd_api_register_client_fusion    ds_ucsd_api_register_client
#define ds_ucsd_api_deregister_client_fusion  ds_ucsd_api_deregister_client
#define ds_ucsd_api_originate_call_fusion     ds_ucsd_api_originate_call
#define ds_ucsd_api_confirm_call_fusion       ds_ucsd_api_confirm_call
#define ds_ucsd_api_answer_call_fusion        ds_ucsd_api_answer_call
#define ds_ucsd_api_end_call_fusion           ds_ucsd_api_end_call
#define ds_ucsd_api_modify_call_fusion        ds_ucsd_api_modify_call
#define ds_ucsd_api_lookup_cm_callid_fusion   ds_ucsd_api_lookup_cm_callid
#define ds_ucsd_api_set_rlp_fusion            ds_ucsd_api_set_rlp
#define ds_ucsd_api_get_rlp_fusion            ds_ucsd_api_get_rlp
#define ds_ucsd_api_get_callstats_fusion      ds_ucsd_api_get_callstats
#define ds_ucsd_api_get_calltype_fusion       ds_ucsd_api_get_calltype

/** Invalid instance ID. */
#define DSUCSD_INVALID_INST_ID  ((uint8) 0xFF)
/** No call instance ID. */

/**
  Data rate types. Note that these values are used
  in packet logging and <i>cannot</i> be modified. Also note
  that these rates <i>cannot</i> go beyond 4 bits. */
typedef enum
{
   DS_UCSD_RATE_NONE     = 0x00,  /**< Data rate is none. */
   DS_UCSD_RATE_300      = 0x01,  /**< Data rate is 300. */
   DS_UCSD_RATE_600      = 0x02,  /**< Data rate is 600. */
   DS_UCSD_RATE_1200     = 0x03,  /**< Data rate is 1200. */
   DS_UCSD_RATE_1200_300 = 0x04,  /**< Data rate is 1200_300. */
   DS_UCSD_RATE_2400     = 0x05,  /**< Data rate is 2400. */
   DS_UCSD_RATE_4800     = 0x06,  /**< Data rate is 4800. */
   DS_UCSD_RATE_7200     = 0x07,  /**< Data rate is 7200. */
   DS_UCSD_RATE_9600     = 0x08,  /**< Data rate is 9600. */
   DS_UCSD_RATE_12000    = 0x09,  /**< Data rate is 12000. */
   DS_UCSD_RATE_14400    = 0x0A,  /**< Data rate is 14400. */
   DS_UCSD_RATE_28800    = 0x0B,  /**< Data rate is 28800. */
   DS_UCSD_RATE_38400    = 0x0C,  /**< Data rate is 38400. */
// DS_UCSD_RATE_43200    = 0x0C,
   DS_UCSD_RATE_48000    = 0x0D,  /**< Data rate is 48000. */
   DS_UCSD_RATE_57600    = 0x0E,  /**< Data rate is 57600. */
   DS_UCSD_RATE_64000    = 0x0F   /**< Data rate is 64000. */
} ds_ucsd_data_rate_T;

/** Indicates whether the API call is a command message or response message. */
typedef enum
{
   DS_UCSD_MSG_COMMAND,         /**< Call is command message. */
   DS_UCSD_MSG_RESPONSE         /**< Call is response message. */
} ds_ucsd_msg_type;


/** Dial string modifier definitions. */
typedef enum
{
  DS_UCSD_CLI_DIAL_MODIFIER    = 0x0002,  /**< Dial modifier is CLI.  */
  DS_UCSD_CUG_DIAL_MODIFIER    = 0x0004,  /**< Dial modifier is CUG.  */
  DS_UCSD_CLI_AND_CUG_MODIFIER = 0x0008   /**< Dial modifier is CLI and CUG.  */
} ds_ucsd_dial_modifier_e_type;

/** @brief Calling Line Identification (CLI) dial modifier.
*/
typedef struct
{
  boolean                       cli_enabled;  /**< Whether or not CLI is
                                                   enabled. */
} ds_ucsd_cli_modifier_type;

/** @brief Closed User Group (CUG) dial modifier. */
typedef struct
{
  boolean                       cug_enabled;        /**< Whether or not CUG is
                                                         enabled. */
  boolean                       cug_index_enabled;  /**< Whether or not CUG
                                                         index is enabled. */
  uint8                         cug_index_val;      /**< CUG index value. */
  boolean                       suppress_pref_cug;  /**< Suppress preferred
                                                         CUG. */
  boolean                       suppress_outgoing_access; /**< Suppress
                                                          outgoing access. */
} ds_ucsd_cug_modifier_type;

/** @brief Structure that is sent to the mode-specific call handlers.
*/
typedef struct
{
  boolean modifier_present;              /**< Whether or not a modifier is
                                              present. */
  ds_ucsd_dial_modifier_e_type modifier; /**< Describes the type of
                                              modifier. */

  struct
  {
    ds_ucsd_cli_modifier_type  cli;       /**< CLI. */
    ds_ucsd_cug_modifier_type  cug;       /**< CUG. */
   } modifier_info; /**< Modifier information. */
} ds_ucsd_dial_modifier_info_type;

typedef enum
{
  DS_UMTS_CSD_UUS_ID_NONE        = -1, /**< No UUIE peer address. */
  DS_UMTS_CSD_UUS_ID_RESERVED    = 0,  /**< Reserved.  */
  DS_UMTS_CSD_UUS_ID_EMAIL       = 1,  /**< RFC-822-compliant email address. */
  DS_UMTS_CSD_UUS_ID_URL         = 2,  /**< URL-style address. */
  DS_UMTS_CSD_UUS_ID_H323        = 3,  /**< Basic ISO/IEC 646 address. */
  DS_UMTS_CSD_UUS_ID_FIRST_VALID = DS_UMTS_CSD_UUS_ID_EMAIL,
                                       /**< First valid address. */
  DS_UMTS_CSD_UUS_ID_ENUM_MAX    = 4   /**< Internal use only. */
} ds_ucsd_uus_id_e_type;

/** @brief Used for passing User-to-User Signaling 1 (UUS1)
 data.
*/
typedef struct
{
  boolean                 present;  /**< Whether or not UUS data is present. */
  ds_ucsd_uus_id_e_type   uus_id;   /**< UUS ID. */
  uint8                  *uus_data; /**< UUS data. */
  uint8                   data_len; /**< Data length. */
} ds_ucsd_uus_info_type;

/**
  The values in this enumeration are numeric because they are used in the
  externalized API, and complex enumerations are not supported in the
  external data representation (XDR) format. */
typedef enum ds_ucsd_call
{
   DS_UCSD_UNKNOWN_CALL         = (0x00),   /**< Unknown call type. */
   DS_UCSD_ASYNC_CSD_CALL       = (0x01),   /**< MAKE_CALL_TYPE
                                                (DS_UCSD_CALL_INDEX_ASYNC). */
   DS_UCSD_SYNC_CSD_CALL        = (0x02),   /**< MAKE_CALL_TYPE
                                                (DS_UCSD_CALL_INDEX_SYNC). */
   DS_UCSD_FAX_CALL             = (0x04),   /**< MAKE_CALL_TYPE
                                                (DS_UCSD_CALL_INDEX_FAX). */
   DS_UCSD_VIDEO_TELEPHONY_CALL = (0x08),   /**< MAKE_CALL_TYPE
                                                (DS_UCSD_CALL_INDEX_VT). */
   DS_UCSD_VOICE_CALL           = (0x10)    /**< MAKE_CALL_TYPE
                                                (DS_UCSD_CALL_INDEX_VOICE). */
} ds_ucsd_call_type;

/** @brief Structure for V.42bis data compression parameters.  */
typedef struct ds_ucsd_v42bis
{
  boolean  present;           /**< Validity of the structure. */
  uint8    direction;         /**< Desired direction of operation of the data
                                   compression function; from the Data
                                   Terminal Equipment (DTE) perspective. */
  uint8    negotiation;       /**< Specifies whether or not the Data
                                   Communication Equipment (DCE) should continue
                                   to operate if the desired result is
                                   not obtained. */
  uint16   max_dict;          /**< Specifies the maximum number of dictionary
                                   entries that should be negotiated (may be
                                   used by the DTE to limit the codeword size
                                   transmitted based on its knowledge of the
                                   nature of the data to be transmitted). */
  uint8    max_string;        /**< Specifies the maximum string length to be
                                   negotiated. */
} ds_ucsd_v42bis_type;

/** @brief Structure for RLP parameters.  */
typedef struct ds_ucsd_rlp
{
  boolean                 present;    /**< Validity of the structure. */
  uint8                   version;    /**< RLP version. */
  uint16                  iws;        /**< IWF-to-MS window size. */
  uint16                  mws;        /**< MS-to-IWF window size. */
  uint16                  T1;         /**< Acknowledgement timer T1. */
  uint16                  N2;         /**< Retransmission attempts N2. */
  uint16                  T4;         /**< Resequencing timer. */
} ds_ucsd_rlp_type;


/** @brief Structure for nontransparent call parameters for query results. */
typedef struct ds_ucsd_rlp_sets /* For query results */
{
  uint8                num_sets;                      /**< Number of valid
                                                           RLP sets. */
  ds_ucsd_rlp_type     rlp_sets[DSUCSD_MAX_RLP_SETS]; /**< RLP data
                                                           information. */
  ds_ucsd_v42bis_type  v42_info;                      /**< V.42bis data
                                                           information. */
} ds_ucsd_rlp_sets_type;

/** @brief Structure for nontransparent information. */
typedef struct ds_ucsd_nt_info
{
  ds_ucsd_rlp_type     rlp_info;   /**< RLP data information. */
  ds_ucsd_v42bis_type  v42_info;   /**< V.42bis data information. */
} ds_ucsd_nt_info_type;

/**  @brief Structure for call statistics reporting.  */
typedef struct ds_ucsd_call_stats
{
  boolean  present;                /**< Validity of the structure. */
  boolean  call_active;            /**< Whether or not the call is active. */
  uint32   tx_data_counter;        /**< Transmit data counter. */
  uint32   rx_data_counter;        /**< Receive data counter. */
} ds_ucsd_call_stats_type;


/** @brief Keeps the call control-related termination parameters. */
typedef struct
{
  boolean     cc_cause_present; /**< Flags the presence/validity of the
                                     remaining fields in this structure. */
  uint8       cc_cause;         /**< Call control cause as defined
                                     in GSM 04.08 Table 10.86 constants,
                                     which are defined in cause.h. */
} ds_ucsd_cc_cause_param_type;

/** @brief Keeps the incoming call answer parameters. */
typedef struct
{
  boolean     reject;          /**< Flags whether the incoming call is
                                    accepted or rejected. */
  ds_ucsd_cc_cause_param_type  cause_info; /**< Call control cause
                                                information. */
} ds_ucsd_answer_param_type;


/** @brief Keeps the call rejection parameters. */
typedef struct
{
   boolean                         present;    /**< Tags the presence of the
                                                    reject parameter field;
                                                    reads the remaining fields
                                                    only when this field is
                                                    set to TRUE. */
   reject_type_enum_T              rej_type;   /**< Reject subtype enumeration
                                                    defined in sys_cnst.h. */
   uint8                           rej_reason; /**< GSM 04.08 reject reason
                                                    values defined in
                                                    cause.h. */
} ds_ucsd_call_reject_param_type;

/** @brief Used for passing Over-the-Air (OTA) channel parameters.
*/
typedef struct
{
  uint16 sdus_per_tti;            /**< SDUs per Transmission Time
                                       Interval (TTI). */
  uint16 sdu_size;                /**< SDU size. */
  ds_ucsd_data_rate_T data_rate;  /**< Data rate. */
} ds_ucsd_ota_channel_params_T;


/** @brief Detailed call end parameters. This provides more detail on a call
  termination event. */
typedef struct
{
  cm_call_end_e_type              end_cause; /**< Call termination reason. */
  ds_ucsd_cc_cause_param_type     cc_param;  /**< Call control cause
                                                  parameters. */
  ds_ucsd_call_reject_param_type  rej_param; /**< call rejection parameters. */
} ds_ucsd_call_end_param_type;


/*  Structures & enum for data path specification. */
/** @{ */
/** Data path mode. */
typedef enum ds_ucsd_datapath_mode
{
  DS_UCSD_DPATH_MODE_NULL,          /**< Data path mode is NULL. */
  DS_UCSD_DPATH_MODE_SIO,           /**< Data path mode is SIO. */
  DS_UCSD_DPATH_MODE_WM             /**< Data path mode is WM. */
} ds_ucsd_datapath_mode_type;

/** @brief Serial I/O (SIO) port information.  */
typedef struct sio_info_s {
  sio_port_id_type             sio_port;      /**< SIO or SMD port. */
} ds_ucsd_sio_info_type;

/** @brief Data path and channel-related information. */
typedef struct ds_ucsd_datapath
{
  ds_ucsd_ota_channel_params_T    *chan_info; /**< Access stratum
                                                   parameters. */
  ds_ucsd_datapath_mode_type       mode;      /**< Data path mode (SIO|WM). */
  /** @brief Data path information. */
  union dpathinfo {
    ds_ucsd_sio_info_type          sio_info;  /**< SIO or Shared Memory
                                                   Device (SMD) port. */
    void                          *wm_info;   /**< Tx/Rx watermarks. */
  } info;
} ds_ucsd_datapath_type;
/*~ FIELD ds_ucsd_datapath.chan_info POINTER */
/*~ FIELD dpathinfo.wm_info POINTER */
/*~ FIELD ds_ucsd_datapath.info DISC ds_ucsd_datapath.mode */
/*~   CASE DS_UCSD_DPATH_MODE_SIO dpathinfo.sio_info */
/*~   CASE DS_UCSD_DPATH_MODE_WM  dpathinfo.wm_info  */

/** @} */ /* end_name_group Data Path Specification */


/** @brief Used to indicate to the application that a
  call is connected. */
typedef struct ds_ucsd_call_connect_info
{
  uint8                         inst_id;       /**< CS data call instance. */
  ds_ucsd_call_type             call_type;     /**< CS data call type. */
  boolean                       modify_allowed;/**< Whether or not in-call
                                                    modification is
                                                    permitted. */
  ds_ucsd_datapath_type        *datapath;      /**< Data path information. */
} ds_ucsd_call_connect_info_type;
/*~ FIELD ds_ucsd_call_connect_info.datapath POINTER */

/** @brief Used to indicate to the application that a
  call is ended. */
typedef struct ds_ucsd_call_end_info
{
  uint8                        inst_id;    /**< CS data call instance. */
  ds_ucsd_call_end_param_type *end_param;  /**< Provides more information
                                           about the call termination. The
                                           application should copy
                                           this structure if it wants to
                                           keep the information. The pointer
                                           will be reused immediately
                                           after the callback is returned. */
} ds_ucsd_call_end_info_type;
/*~ FIELD ds_ucsd_call_end_info.end_param POINTER */

/** @brief Used to indicate to the application that there is
  a setup request for an incoming call. */
typedef struct ds_ucsd_call_setup_info
{
  uint8                       inst_id;         /**< CS data call instance. */
  ds_ucsd_call_type           call_type;       /**< CS data call type. */
  boolean                     modify_allowed;  /**< Whether or not in-call
                                                    modification is
                                                    permitted. */
} ds_ucsd_call_setup_info_type;

/** @brief Used to indicate to the application that there is
  an incoming call. */
typedef struct ds_ucsd_call_incoming_info
{
  uint8                       inst_id;           /**< CS data call instance. */
  sys_sys_mode_e_type         network_mode;      /**< Which network the
                                                      system is camped on:
                                                      WCDMA or GSM. */
  uint8                       speed;             /**< +CBST speed. */
  uint8                       name;              /**< Synchronous or
                                                      asynchronous. */
  uint8                       connection_element; /**< Transparent or
                                                       nontransparent. */
  uint8                       waiur;              /**< Wanted air interface
                                                       rate. */
  uint8                      *uus_data;           /**< UUS1 payload. */
  uint8                       uus_data_len;       /**< Length of uus_data. */
  uint8                      *caller_number;      /**< Caller's number. */
  uint8                       caller_number_len;  /**< Length of caller's
                                                      number. */
} ds_ucsd_call_incoming_info_type;
/*~ FIELD ds_ucsd_call_incoming_info.uus_data VARRAY LENGTH ds_ucsd_call_incoming_info.uus_data_len */
/*~ FIELD ds_ucsd_call_incoming_info.caller_number VARRAY LENGTH ds_ucsd_call_incoming_info.caller_number_len */

/** @brief Used to indicate to the application that the
  originated call has received a progress indication. */
typedef struct ds_ucsd_call_progress_info
{
  uint8                         inst_id;       /**< CS data call instance. */
  ds_ucsd_call_type             call_type;     /**< CS data call type. */
  uint8                         progress_info; /**< CM progress information. */
  ds_ucsd_datapath_type        *datapath;      /**< Data path information. */
} ds_ucsd_call_progress_info_type;
/*~ FIELD ds_ucsd_call_progress_info.datapath POINTER */

/** @brief Used to indicate to the application that the
  originated call has received a setup confirmation. */
typedef struct ds_ucsd_call_confirm_info
{
  uint8                       inst_id;         /**< CS data call instance. */
  ds_ucsd_call_type           call_type;       /**< CS data call type. */
  boolean                     modify_allowed;  /**< Whether or not in-call
                                                    modification is
                                                    permitted. */
} ds_ucsd_call_confirm_info_type;


/** @brief Used to indicate to the application that
  there is a modify transaction for an existing call. */
typedef struct ds_ucsd_call_modify_info
{
  uint8                         inst_id;       /**< CS data call instance. */
  ds_ucsd_call_type             call_type;     /**< Type of call. */
  boolean                       status;        /**< Transaction status. */
  boolean                       nw_initiated;  /**< Network or remote party. */
  byte                          rej_cause;     /**< Rejection cause. */
  ds_ucsd_datapath_type        *datapath;      /**< Data path information. */
} ds_ucsd_call_modify_info_type;

/*~ FIELD ds_ucsd_call_modify_info.datapath POINTER */

/** @brief Call event. */
typedef struct ds_ucsd_call_event
{
  cm_call_event_e_type                call_event; /**< CM call event. */

  /** @brief Event payload. */
  union event_info
  {
    ds_ucsd_call_confirm_info_type    confirm;   /**< Call Confirmed event. */
    ds_ucsd_call_progress_info_type   progress;  /**< Call Progress event. */
    ds_ucsd_call_connect_info_type    connect;   /**< Call Connect event. */
    ds_ucsd_call_setup_info_type      setup;     /**< Call SetupIND event. */
    ds_ucsd_call_incoming_info_type   incoming;  /**< Call Incoming event. */
    ds_ucsd_call_end_info_type        end;       /**< Call End event. */
    ds_ucsd_call_modify_info_type     modify;    /**< Call Modify event. */
  } event_info;
} ds_ucsd_call_event_type;
/*~ FIELD ds_ucsd_call_event.event_info DISC ds_ucsd_call_event.call_event */
/*~   CASE  CM_CALL_EVENT_CALL_CONF             event_info.confirm */
/*~   CASE  CM_CALL_EVENT_PROGRESS_INFO_IND     event_info.progress */
/*~   CASE  CM_CALL_EVENT_CONNECT               event_info.connect */
/*~   CASE  CM_CALL_EVENT_SETUP_IND             event_info.setup */
/*~   CASE  CM_CALL_EVENT_INCOM                 event_info.incoming */
/*~   CASE  CM_CALL_EVENT_END                   event_info.end */
/*~   DEFAULT                                   event_info.modify */


/* Callback type  ds_ucsd_call_event_cb_type */

typedef void (*ds_ucsd_call_event_cb_type)
(
  const ds_ucsd_call_event_type *event_ptr,
  void                          *user_info_ptr
);
/*~ PARAM IN user_info_ptr POINTER  */
/*~ PARAM IN event_ptr POINTER  */

/* FUNCTION DS_UCSD_API_REGISTER_CLIENT_FUSION */
/**
  Registers the application client with the UCSD
  subtask. This function should be called by the application task at
  initialization.

  @param call_types [in]    Bitmap of the UCSD call types for which the client
                            subscribes for call control notification
                            events. Currently, fax and voice calls are not
                            supported.
  @param call_event_cb [in] Client callback function to receive call control
                            notification events.
  @param user_info_ptr [in] Client context value, which is provided
                            with call control notification events.
  @return
  The caller is returned a client ID, which must be checked to determine
  the status of registration. A valid client ID is returned on successful
  registration; DSUCSD_INVALID_CLIENT_ID on registration failure.

  @dependencies
  UCSD client support must have been previously initialized.
*/
extern ds_ucsd_client_id_type ds_ucsd_api_register_client
(
  uint8   call_types,
  ds_ucsd_call_event_cb_type call_event_cb,
  void  *user_info_ptr
);
/*~ FUNCTION  ds_ucsd_api_register_client
    RELEASE_FUNC ds_ucsd_api_deregister_client(_RESULT_)
    ONERROR return DSUCSD_INVALID_CLIENT_ID  */
/*~ PARAM IN user_info_ptr POINTER  */

/* FUNCTION DS_UCSD_API_DEREGISTER_CLIENT_FUSION */
/**
  Deregisters the application client from the UCSD
  subtask. This function should be called by the application task at shutdown.

  @param client_id [in] Client ID returned on API registration.

  @return
  TRUE -- Operation is successful. \n
  FALSE -- Operation is not successful.

  @dependencies
  ds_ucsd_api_register_client() must have already been called.
*/
extern boolean ds_ucsd_api_deregister_client
(
  ds_ucsd_client_id_type client_id
);
/*~ FUNCTION ds_ucsd_api_deregister_client
    ONERROR return FALSE  */

/* FUNCTION DS_UCSD_API_ORIGINATE_CALL*/
/**
  Originates a CS data call. This function sends the
  DS_UCSD_APP_ORIG_CALL_CMD command to the UCSD subtask. The function
  should be called by a CS data application.

  @param client_id [in]  Client ID returned on API registration.
  @param call_mode [in]  Specify single or dual mode call. Applicable only
                         for builds supporting multimedia fallback
                         (Service Change and UDI/RDI Fallback (SCUDIF)).
  @param speed [in]      Fixed network data rate, per +CBST AT command.
  @param name [in]       Synchronous or asynchronous name flag per +CBST
                         AT command.
  @param connection_element [in] Connection element per +CBST AT command.
  @param waiur [in]              Wanted air interface rate per +CHSN AT
                                 command.
  @param dial_string [in]        NULL-terminated dial string without dialing
                                 modifiers.
  @param dial_string_len [in]    Length of dial string.
  @param modifiers [in]          Dialing modifiers.
  @param uus_info [in]           User-to-user signaling for videotelephony call
                                 two-stage dialing.

  @return
  A CS data call instance ID is returned on success; DSUCSD_INVALID_INST_ID
  on failure.

  @dependencies
  ds_ucsd_api_register_client() must have already been called.

  @sideeffects
  May allocate an entry in the CS data call table.
*/
extern uint8 ds_ucsd_api_originate_call
(
  ds_ucsd_client_id_type client_id,
  uint8                call_mode,
  uint8                speed,
  uint8                name,
  uint8                connection_element,
  uint8                waiur,
  const uint8         *dial_string,
  uint8                dial_string_len,
  ds_ucsd_dial_modifier_info_type *modifiers,
  ds_ucsd_uus_info_type *uus_info
); /* ds_ucsd_api_originate_call() */
/*~ FUNCTION ds_ucsd_api_originate_call
    ONERROR return DSUCSD_INVALID_INST_ID  */
/*~ PARAM IN dial_string STRING  */
/*~ PARAM IN uus_info POINTER  */
/*~ PARAM IN modifiers POINTER  */

/* FUNCTION DS_UCSD_API_CONFIRM_CALL*/
/**
  Confirms an incoming CS data call. This function sends the
  DS_UCSD_APP_CONFIRM_CALL_CMD command to the UCSD subtask.  The function
  should be called by a CS data application.

  @param client_id [in] Client ID returned on API registration.
  @param inst_id [in]   Allocated CS data call instance ID.
  @param call_type [in] Specify preferred mode of a dual mode call.
                        Applicable only for builds supporting
                        multimedia fallback (SCUDIF).
  @param result_params_ptr [in] Indicates if the setup is rejected and gives
                                a cause value.

  @return
  TRUE -- Operation is successful. \n
  FALSE -- Operation is not successful.

  @dependencies
  ds_ucsd_api_register_client() must have already been called.

  @sideeffects
  If call_type is changed relative to the setup event, a preferred bearer
  order will be changed in the network signaling.
*/
extern boolean ds_ucsd_api_confirm_call
(
  ds_ucsd_client_id_type     client_id,        /* Client identifier        */
  uint8                      inst_id,          /* CSData call instance     */
  ds_ucsd_call_type          call_type,        /* Type of call             */
  ds_ucsd_answer_param_type *result_params_ptr /* Result parameter info    */
); /* ds_ucsd_api_confirm_call() */
/*~ FUNCTION ds_ucsd_api_confirm_call
    ONERROR return FALSE  */
/*~ PARAM IN result_params_ptr POINTER  */

/* FUNCTION DS_UCSD_API_ANSWER_CALL*/
/**
  Answers a CS data call. This function sends the
  DS_UCSD_APP_ANSWER_CALL_CMD command to the UCSD subtask. The function
  should be called by a CS data application.

  @param client_id [in]         Client ID returned on API registration.
  @param inst_id [in]           Allocated CS data call instance ID.
  @param answer_params_ptr [in] Indicates if the call answer is
                                rejected and gives a cause value.

  @return
  TRUE -- Operation is successful. \n
  FALSE -- Operation is not successful.

  @dependencies
  ds_ucsd_api_register_client() must have already been called.
*/
extern boolean ds_ucsd_api_answer_call
(
  ds_ucsd_client_id_type     client_id,
  uint8                      inst_id,
  ds_ucsd_answer_param_type *answer_params_ptr
); /* ds_ucsd_api_answer_call() */
/*~ FUNCTION ds_ucsd_api_answer_call
    ONERROR return FALSE  */
/*~ PARAM IN answer_params_ptr POINTER  */

/* FUNCTION DS_UCSD_API_END_CALL*/
/**
  Ends a CS data call. This function sends the
  DS_UCSD_APP_END_CALL_CMD command to the UCSD subtask.
  The function should be called by a CS data application.

  @param client_id [in]      Client ID returned on API registration.
  @param inst_id [in]        Allocated CS data call instance ID.

  @return
  TRUE -- Operation is successful. \n
  FALSE -- Operation is not successful.

  @dependencies
  ds_ucsd_api_register_client() must have already been called.
*/
extern boolean ds_ucsd_api_end_call
(
  ds_ucsd_client_id_type     client_id,
  uint8                      inst_id
); /* ds_ucsd_api_end_call() */
/*~ FUNCTION ds_ucsd_api_end_call
    ONERROR return FALSE  */

/* FUNCTION DS_UCSD_API_MODIFY_CALL*/
/**
  Modifies a CS data call to a new call type. This function
  sends the DS_UCSD_APP_MODIFY_CALL_CMD command to the UCSD
  subtask. The function should be called by a CS data application.
  It is applicable only for builds supporting multimedia fallback (SCUDIF).

  @param client_id [in]      Client ID returned on API registration.
  @param inst_id [in]        Allocated CS data call instance ID.
  @param msg_type [in]       Specifies command or response.
  @param new_call_type [in]  Indicates the new call type for the originating
                             modify request.
  @param accept [in]         Indicates an accept/reject for the incoming modify
                             request.

  @return
  TRUE -- Operation is successful. \n
  FALSE -- Operation is not successful.

  @dependencies
  ds_ucsd_api_register_client() must have already been called.
*/
extern boolean ds_ucsd_api_modify_call
(
  ds_ucsd_client_id_type     client_id,
  uint8                      inst_id,
  ds_ucsd_msg_type           msg_type,
  ds_ucsd_call_type          new_call_type,
  boolean                    accept
); /* ds_ucsd_api_modify_call() */
/*~ FUNCTION ds_ucsd_api_modify_call
    ONERROR return FALSE  */

/* FUNCTION DS_UCSD_API_LOOKUP_CM_CALLID*/
/**
  Performs a lookup in the CS data call table to find the
  Call Manager (CM) call ID that corresponds to the CS data call
  instance ID.

  @param inst_id [in]  Allocated CS data call instance ID.

  @return
  A mapped CM call ID is returned on success; CM_CALL_ID_INVALID on
  failure.

  @dependencies
  None.
*/
extern uint8 ds_ucsd_api_lookup_cm_callid
(
  const uint8 inst_id
);
/*~ FUNCTION ds_ucsd_api_lookup_cm_callid
    ONERROR return CM_CALL_ID_INVALID  */

/* FUNCTION DS_UCSD_API_SET_RLP*/
/**
  Changes the RLP parameters for
  the modem processor. The changes are system-wide and affect all future
  nontransparent CS data calls.

  @param client_id [in]      Client ID returned on API registration.
  @param rlp_params_ptr [in] Pointer to the RLP and data compression
                             parameters.

  @return
  TRUE -- Operation is successful. \n
  FALSE -- Operation is not successful.

  @dependencies
  ds_ucsd_api_register_client() must have already been called.
*/
extern boolean ds_ucsd_api_set_rlp
(
  ds_ucsd_client_id_type      client_id,
  const ds_ucsd_nt_info_type *rlp_params_ptr
);
/*~ FUNCTION ds_ucsd_api_set_rlp
    ONERROR return FALSE  */
/*~ PARAM IN rlp_params_ptr POINTER  */

/* FUNCTION DS_UCSD_API_GET_RLP*/
/**
  Queries the RLP parameters from the modem processor. The parameters
  apply to nontransparent CS data calls.

  @param client_id [in]       Client ID returned on API registration.
  @param rlp_params_ptr [out] Pointer to the RLP and data compression
                              parameters.

  @return
  TRUE -- Operation is successful. \n
  FALSE -- Operation is not successful.

  @dependencies
  ds_ucsd_api_register_client() must have already been called.
*/
extern boolean ds_ucsd_api_get_rlp
(
  ds_ucsd_client_id_type     client_id,
  ds_ucsd_rlp_sets_type     *rlp_params_ptr
);
/*~ FUNCTION ds_ucsd_api_get_rlp
    ONERROR return FALSE  */
/*~ PARAM OUT rlp_params_ptr POINTER  */

/* FUNCTION DS_UCSD_API_GET_CALLSTATS*/
/**
  Queries the data traffic statistics for the specified
  CS data call. The statistics are tracked only for those calls using the
  SIO data path. If a call is inactive, the last statistics are returned.

  @param client_id [in]       Client ID returned on API registration.
  @param inst_id [in]         Allocated CS data call instance ID.
  @param callstats_ptr [out]  Pointer to the call statistics information.

  @return
  TRUE -- Operation is successful. \n
  FALSE -- Operation is not successful.

  @dependencies
  ds_ucsd_api_register_client() must have already been called.
*/
extern boolean ds_ucsd_api_get_callstats
(
  ds_ucsd_client_id_type     client_id,
  uint8                      inst_id,
  ds_ucsd_call_stats_type   *callstats_ptr
);
/*~ FUNCTION ds_ucsd_api_get_callstats
    ONERROR return FALSE  */
/*~ PARAM OUT callstats_ptr POINTER  */

/* FUNCTION DS_UCSD_API_GET_CALLTYPE*/
/**
  Queries the UCSD stack call type for the specified
  CM call ID. If the call instance mapped to the call ID is
  present, a UCSD call type is returned. If a call instance cannot be
  found, the call type will be DS_UCSD_UNKNOWN_CALL, and the return
  value will be FALSE.

  @param cm_call_id [in]  CM call ID.
  @param call_type [out]  UCSD call type.

  @return
  TRUE -- Operation is successful. \n
  FALSE -- Operation is not successful.

  @dependencies
  None.
*/
extern boolean ds_ucsd_api_get_calltype
(
  const uint8     cm_call_id,
  uint8*          call_type
);
/*~ FUNCTION ds_ucsd_api_get_calltype
    ONERROR return FALSE  */
/*~ PARAM OUT call_type POINTER  */

/* FUNCTION DS_UCSD_API_GET_DATA_PORT */
/**
  Returns the data port to use for the current target

  @param data_port [out]  Data port to use

  @return
  TRUE -- Operation is successful.
  FALSE -- Operation is not successful.

  @dependencies
  None.
*/
extern boolean ds_ucsd_api_get_data_port
(
  const char  **data_port
);

#endif /* DSUCSDAPPIF_APIS_FUSION_H */

#endif /* ! DSUCSDAPPIF_APIS_FUSION_H_trimmed */
