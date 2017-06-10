/** @file ConnectionManagerBase.h
*/

/*=======================================================================
Copyright (c)2014-2015 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.
************************************************************************/

#ifndef QIMSCM_CONNECTIONMANAGERBASE_H
#define QIMSCM_CONNECTIONMANAGERBASE_H


#define IMSCM_UNUSED_PARAM(v) if((v))\
              {\
              }

namespace IMSConnectionManager
{
    const uint32_t QIMS_CM_AUTHNC_LEN = 9;
    const uint32_t QIMS_CM_IMEI_STR_LEN = 32;
    const uint32_t QIMS_CM_IPADDRESS_LEN = 46;
    const uint32_t QIMS_CM_PASSWORD_LEN = 60;
    const uint32_t QIMS_CM_URI_LEN = 128;
    const uint32_t QIMS_CM_AUTHCHALLENGE_LEN = 400;
    const uint32_t QIMS_CM_ROUTE_LEN = 1000;
	const uint32_t QIMSCM_SECURITY_VERIFY_STR_LEN = 1500;

	static const char* NULL_FT_STRING = "NULL_FT";

	typedef enum
	{
		CM_SUCCESS,
		/* Request is processed successfully. */
		CM_FAILURE,
		/* Request is processed unsuccessfully. */
		CM_MEMORY_ERROR,
		/* Error in memory allocation. */
		CM_INVALID_LISTENER,
		/* Provided listener is not valid. */
		CM_INVALID_PARAM,
		/* Invalid parameter(s). */
		CM_SERVICE_NOTALLOWED,
		/* Service is not allowed. */
		CM_SERVICE_UNAVAILABLE,
		/* Service is not available. */
		CM_INVALID_FEATURE_TAG,
		/* Invalid feature tag. */
		CM_DNSQUERY_PENDING,
		/* Dns query pending. */
		CM_DNSQUERY_FAILURE,
		/* Dns query failed. */
                CM_SERVICE_DIED,
                /* Android Native Service died */
		CM_INVALID_MAX,
		/* Max */
	}CM_STATUSCODE;

    typedef  enum
    {
      CM_STATUS_DEINIT,
      /**< status is NULL.*/
      CM_STATUS_INIT_IN_PROGRESS,
      /**< Service is being brought up.*/
      CM_STATUS_SUCCESS,
      /**< Service Init Success.*/
      CM_STATUS_FAILURE,
      /**< Service Init Failure.*/
      CM_STATUS_SERVICE_DIED,
      /* Android Native Service died */
    } CM_STATUS;


/** @addtogroup qimscm_datatypes
@{ */

    /** Structure containing the other configuration values needed by clients.
    */
    struct DeviceConfiguration
    {
        bool UEBehindNAT;
        /**< Indicates whether the UE is behind NAT. */
		bool IPSecEnabled;
        /**< Indicates whether IPSec is enabled. */
        bool CompactFormEnabled;
        /**< Indicates whether Compact Form is enabled. */
        bool KeepAliveStatusEnabled;
        /**< Indicates whether Keep Alive is enabled. */
        bool GruuEnabled;
        /**< Indicates whether GRUU is enabled. */
        char SipOutBoundProxyName[QIMS_CM_URI_LEN];
        /**< Outbound SIP proxy name/IP. */
        uint32_t SipOutBoundProxyPort;
        /**< Outbound SIP proxy port. */
        uint32_t PCSCFClientPort;
        /**< P-CSCF client port. */
        uint32_t PCSCFServerPort;
        /**< P-CSCF server port. */
        char AuthChallenge[QIMS_CM_AUTHCHALLENGE_LEN];
        /**< Authentication header. */
        char NonceCount[QIMS_CM_AUTHNC_LEN];
        /**< Nonce count. */
        char ServiceRoute[QIMS_CM_AUTHNC_LEN];
        /**< Service route value. */
		char  SecurityVerify[QIMSCM_SECURITY_VERIFY_STR_LEN];
        /**< Security verify value. */
		uint32_t PCSCFOldSAClientPort;
        /**< IPSec old SA P-CSCF client port. */
    };

    /** Structure containing the user configuration values needed by clients.
    */
    struct UserConfiguration
    {
        uint32_t UEClientPort;
        /**< UE client port. */
        uint32_t UEServerPort;
        /**< UE server port. */
        char AssociatedUri[QIMS_CM_ROUTE_LEN];
        /**< Asociated URI value. */
        char UEPublicIPAddress[QIMS_CM_IPADDRESS_LEN];
        /**< Received UE public IP address. */
        uint32_t UEPublicPort;
        /**< UE public IP port. */
        char SipPublicUserId[QIMS_CM_URI_LEN];
        /**< User public ID. */
        char SipPrivateUserId[QIMS_CM_URI_LEN];
        /**< User private ID. */
        char SipHomeDomain[QIMS_CM_URI_LEN];
        /**< Home domain address. */
        char UEPubGruu[QIMS_CM_URI_LEN];
        /**< UE public GRUU. */
        char LocalHostIPAddress[QIMS_CM_IPADDRESS_LEN];
		/**< UE public IP address. */
		uint32_t IPType;
        /**< UE IP type. */
        char IMEI[QIMS_CM_IMEI_STR_LEN];
        /**< UE IMEI value. */
		uint32_t UEOldSAClientPort;
        /**< IPSec old SA UE client port. */
    };

    /** Union for CM configuration.
    */
    union CMConfiguration
    {
        struct UserConfiguration userConfiguration;
        /**< User configuration. */
        struct DeviceConfiguration deviceConfiguration;
        /**< Device configuration. @newpagetable */
    };

/** @} */ /* end_addtogroup qimscm_datatypes */


    typedef void* cmNativeHandle;
    typedef void* cmConnectionNativeHandle;
    typedef void* cmConfigurationNativeHandle;

};
#endif
