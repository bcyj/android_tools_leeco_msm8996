#ifndef _VDM_TYPES_H_
//#error "vdm_types.h  doubly included"

#define _VDM_TYPES_H_

/*
 * The maximum depth of the DM tree. If not defined then
 * there is no limit.
 */
#define VDM_URI_MAX_DEPTH            16

/*
 * The maximum total length of a node URI. If not defined
 * then there is no limit.
 */
#define VDM_URI_MAX_TOT_LEN          255

/*
 * The maximum length of any node name. If not defined then
 * there is no limit.
 */
#define VDM_URI_MAX_SEG_LEN          31

/*
 * The maximum length of the value of the ServerId node
 * in a DM acoount subtree.
 */
#define VDM_SERVERID_MAX_LEN        256

/*
 * The maximum length of a PIN
 */
#define VDM_PIN_MAX_LEN             20

/*
 * The maximum length of the Network Shared Secret
 */
#define VDM_NSS_MAX_LEN             16

/*!
 * Session Enable Modes
 */
typedef enum {
    E_VDM_SessionEnable_allowAll,                   //!< allow all sessions
    E_VDM_SessionEnable_allowClientInitiatedOnly, //!< allow only client initiated sessions
    E_VDM_SessionEnable_allowNone                  //!< do not allow any session
} E_VDM_SessionEnable_t;

/*!
 * User Interface Mode
 */
typedef enum {
    E_VDM_NIA_UIMode_NotSpecified = 0x00, //!< user interaction not specified
    E_VDM_NIA_UIMode_Background = 0x01, //!< perform session in background
    E_VDM_NIA_UIMode_Informative = 0x02, //!< announce beginning of session
    E_VDM_NIA_UIMode_UI = 0x03  //!< prompt before starting session
} E_VDM_NIA_UIMode_t;

/*!
 * Provisioning profiles
 */
typedef enum {
    E_VDM_Prov_Profile_PLAIN,   //!< Plain profile; OMA-DM format message
    E_VDM_Prov_Profile_WAP      //!< WAP profile; OMA-CP format message
} E_VDM_Prov_Profile_t;

/*!
 * Bootstrap security.
 */
typedef enum {
    /*! No security is used. */
    E_VDM_Prov_Security_NONE = -1,

    /*! shared secret - based on a network specific shared secret */
    E_VDM_Prov_Security_NETWPIN = 0,

    /*! shared secret - based on a user PIN. */
    E_VDM_Prov_Security_USERPIN = 1,

    /*! shared secret - network specific shared secret appended with user PIN. */
    E_VDM_Prov_Security_USERNETWPIN = 2,

    /*! out-of-band delivery of MAC authentication information - PIN includes MAC. */
    E_VDM_Prov_Security_USERPINMAC = 3
} E_VDM_Prov_Security_t;

/*!
 ******************************************************************************
 * This callback is called after a Bootstrap session is
 * triggered,to get PIN from user
 *
 * The PIN must be a string of ascii-encoded decimal digits (octets with
 * values 0x30 to 0x39).
 *
 * \param   outUserPin      PIN code string
 * \param   inContext       The context previously set by VDM_Bootstrap_createInstance
 *
 * \return  vDM error code (VDM_ERR_BOOT_PIN if PIN is unobtainable)
 ******************************************************************************
 */
typedef VDM_Error (*VDM_BootGetPinCB)(UTF8Str* outUserPin, void* inContext);

/*!
 ******************************************************************************
 * This callback is called after a Bootstrap session is
 * triggered,to get Network specific Shared Secret (NSS)
 *
 * The NSS is network specific: e.g. for GSM it is the IMSI while for
 * CDMA or TDMA it is the ESN appended with SSD or SSD_S.
 * The value returned by this function should be in a form ready for input
 * to the MAC calculation and must be derived as described in [PROVBOOT]
 * section 6.
 *
 * \param    inBuffer        Preallocated result buffer for the NSS string.
 * \param    inBufferSize    Size of supplied buffer
 * \param    outNssLen       Length of NSS
 * \param    inContext       The context previously set by VDM_Bootstrap_createInstance
 *
 * \return   vDM error code (VDM_ERR_BOOT_NSS if NSS is unobtainable)
 ******************************************************************************
 */
typedef VDM_Error (*VDM_BootGetNssCB)(IU8* inBuffer, IU32 inBufferLen,
        IU32* outNssLenPtr, void* inContext);

/*!
 ******************************************************************************
 * This callback is called after a Bootstrap session is triggered
 * triggered to get Address Type
 *
 * This function is called on receipt of a WAP Profile bootstrap message,
 * in order to determine the value of the DM account node "AddrType".
 * The specification says that the "DM Client chooses the value according
 * to the transports it supports. The DM Server may modify this in a
 * subsequent DM session." ([DM-Bootstrap] 5.3.2).  It is unclear how such
 * a session can take place if AddrType is wrong however!
 *
 * \param    inAddr          Contents of DM account node "Addr", as set by WAP
 *                           bootstrap.
 * \param    outAddrType     Address type. Possible values are:
 *                           "1" for HTTP, "2" for WSP and "3" for OBEX.
 * \param    inContext       The context previously set by
 *                           VDM_Bootstrap_createInstance.
 *
 * \return   vDM error code
 ******************************************************************************
 */
typedef VDM_Error (*VDM_BootGetAddrTypeCB)(UTF8CStr inAddr,
        UTF8CStr *outAddrType, void* inContext);

/*!
 * A structure containing callback functions that may be called during
 * a bootstrap session.
 */
typedef struct {
    VDM_BootGetPinCB getPinCB; /**< get PIN from user */
    VDM_BootGetNssCB getNssCB; /**< get Network Shared Secret */
    VDM_BootGetAddrTypeCB getAddrTypeCB; /**< get 'AddrType' node value */

} VDM_BootCallbacks_t;

/*!
 *******************************************************************************
 * Session context that may be attached to a session triggering request.
 * The session context is then attached to every notification of the state
 * of the session.
 *******************************************************************************
 */
typedef struct {
    /*! The session context will be submitted to all
     *  observers - the initiatorId may be used by
     *  an observer to differentiate between the contexts.
     *  Note: initiatorId may NOT be NULL.
     */
    UTF8CStr initiatorId;

    /*! context data */
    void* data;
} VDM_SessionContext_t;

/*!
 * Session Types
 */
typedef enum {
    VDM_SessionType_Boot,   //!< Bootstrap session
    VDM_SessionType_DM,     //!< DM session
    VDM_SessionType_DL,     //!< DL session
    VDM_SessionType_DS      //!< DS session
} E_VDM_SessionType_t;

/*!
 * Session States
 */
typedef enum {
    VDM_SessionState_Started,   //!< Session has started
    VDM_SessionState_Complete,  //!< Session completed successfully
    VDM_SessionState_Aborted    //!< Session aborted
} E_VDM_SessionState_t;

/*!
 ******************************************************************************
 * Notify on session state change
 *
 * This callback is called when a Bootstrap/DM/DL session changes its state,
 * Session is typically started and moved to either completed or aborted.
 * In DM/DL it may also be suspended/resumed. In case of ABORTED, the lastError
 * parameter will indicate the reason for the failure --- on all other states
 * it will be VDM_ERR_OK. The data parameter is currently not used.
 *
 * \param   inType      The session type, either Bootstrap, DM or DL.
 * \param   inState     The new state of the session.
 * \param   inLastError Relevant for ABORTED only.
 * \param   inDataStr   Currently not used, always NULL.
 * \param   inContext   Context that was passed by the session initiator.
 *
 ******************************************************************************
 */
typedef void (*VDM_SessionStateNotifyCB)(E_VDM_SessionType_t inType,
        E_VDM_SessionState_t inState, VDM_Error inLastError, UTF8CStr inDataStr,
        VDM_SessionContext_t** inContext);

/*!
 ******************************************************************************
 * This callback is called after a Notification Initiated DM session (NIDM) is
 * triggered, to notify the caller of the NIA message content has been
 * parsed successfully. The parsed content is passed as well.
 *
 * \param   inUIMode
 * \param   inDMVersion
 * \param   inVendorSpecificData
 * \param   inVendorSpecificDataLength
 * \param   inSessionContext - session context passed in the NIA DM session
 *          trigger (/ref VDM_triggerNIADMSession).
 *
 * \return
 ******************************************************************************
 */
typedef VDM_Error (*VDM_NIAHandlerCB)(E_VDM_NIA_UIMode_t inUIMode,
        IU16 inDMVersion, IU8* inVendorSpecificData,
        IU16 inVendorSpecificDataLength,
        VDM_SessionContext_t* inSessionContext);

/*!
 *******************************************************************************
 * Download Descriptor
 *******************************************************************************
 */
/**! Download Descriptor attributes **/
typedef enum {
    E_VDM_DDField_size,      //!< Number of bytes to be downloaded from the URI.

    E_VDM_DDField_objectURI, /*!< URI (usually URL) from which the media
     object can be loaded. */
    E_VDM_DDField_type,         //!< MIME type of the media object.

    E_VDM_DDField_name, /*!< A user readable name of the Media Object
     that identifies the object to the user. */
    E_VDM_DDField_DDVersion,  //!< Version of the Download Descriptor technology

    E_VDM_DDField_vendor,  //!< The organisation that provides the media object.

    E_VDM_DDField_description, //!< A short textual description of the media object.

    E_VDM_DDField_installNotifyURI, /*!< URI (or URL) to which a installation
     status report is to be sent */

    E_VDM_DDField_nextURL, /*!< URL to which the client should navigate in case
     the end user selects to invoke a browsing action
     after the download transaction has completed */
    E_VDM_DDField_infoURL,   //!< A URL for further describing the media object.

    E_VDM_DDField_iconURL,      //!< The URI of an icon.

    E_VDM_DDField_installParam, /*!< An installation parameter associated with
     the downloaded media object. */

    E_VDM_DDField_count
} E_VDM_DDField_t;

/*!
 * Structure used to hold pointers into a buffer for the various elements
 * of an XML Download Descriptor (as per OMA DLOTA specs).
 */
typedef struct {
    IU32 size; /*! The number of bytes to be downloaded from the URI */
    UTF8Str strField[E_VDM_DDField_count]; /*!< string values
     (note: redundant "size" field) */
} VDM_Downloader_DD_t;

/*!
 *******************************************************************************
 * This callback is called after a DL session is triggered, to notify
 * the session initiator that a chuck of data has been received. The handler
 * should store the downloaded data chunk in storage, and may choose to display
 * progress to the user.
 *
 * \param   inDownloadDescriptor
 * \param   inOffset
 * \param   inData
 * \param   inDataSize
 * \param   inContext
 *
 * \return
 *******************************************************************************
 */
typedef VDM_Error (*VDM_HandleDLContentCB)(
        VDM_Downloader_DD_t* inDownloadDescriptor, IU32 inOffset, IU8* inData,
        IU32 inDataSize, VDM_SessionContext_t* inContext);

/*!
 *******************************************************************************
 * Prompt user about download.
 *
 * \return TRUE proceed with download; FALSE abort download
 *******************************************************************************
 */
typedef IBOOL (*VDM_DownloadPromptCB)(
        VDM_Downloader_DD_t* inDownloadDescriptor);

/******************************************************************************
 ******************************************************************************
 *
 *                              GENERIC ALERTS
 *
 ******************************************************************************
 ******************************************************************************
 */

#ifdef _RDM_RDM_CONFIG_H_

//The typedefs below are just renaming of RDM structures

typedef RDM_genericAlertItemStructure VDM_GenAlertItem_t;
typedef RDM_genericAlertItemListStructure VDM_GenAlertItem_Node_t;
typedef RDM_genericAlertItemListPtr VDM_GenAlertItem_ListPtr;//ptr to list of item elements

typedef RDM_genericAlertStructure VDM_GenAlert_t;
typedef RDM_genericAlertItemListStructurePtr VDM_GenAlertList_Node_t;

#else

//Explicit definition of generic alert types. - identical to RDM's types

/*!
 * Item element in generic alert message.
 */
typedef struct {
    UTF8Str sourcePath; //!< source path
    UTF8Str data;       //!< data
    UTF8Str type;       //!< type
    UTF8Str format;     //!< format
    UTF8Str mark;       //!< mark
    UTF8Str targetPath; //!< target path

} VDM_GenAlertItem_t;

/*!
 * Node in a linked list of generic alert's item elements.
 */
typedef struct VDM_GenAlertItem_Node_t {
    VDM_GenAlertItem_t item;       //!< item data
    struct VDM_GenAlertItem_Node_t* next;       //!< pointer to next item

} VDM_GenAlertItem_Node_t, *VDM_GenAlertItem_ListPtr;

/*!
 * Generic alert data.
 */
typedef struct VDM_GenAlert_t {
    VDM_GenAlertItem_ListPtr itemList;       //!< list of item elements
    UTF8Str correlator;     //!< correlator (or NULL)

} VDM_GenAlert_t;

/*!
 * Node in a linked list of generic alerts.
 */
typedef struct VDM_GenAlertList_Node_t {
    VDM_GenAlert_t alertList;  //!< alert data
    struct VDM_GenAlertList_Node_t *next;      //!< pointer to next alert

} VDM_GenAlertList_Node_t;

#endif //_RDM_RDM_CONFIG_H_

typedef VDM_GenAlertList_Node_t* VDM_GenAlertListPtr; //!< ptr to list of alerts

/*!
 ******************************************************************************
 * This callback is called before a session of multiple generic alerts is started
 * (VDM_triggerMultipleGenericAlertsSession), to create the list
 * of generic alerts. The list of generic alerts is allocated
 * in this function, and is later released by vDM.
 *
 * \param   inContext   Context that was passed by the session initiator.
 *
 * \return  Pointer to a list of generic alerts, each alert may contain
 *          multiple items.
 *
 ******************************************************************************
 */

typedef VDM_GenAlertList_Node_t* (*VDM_CreateGenericAlertsCB)(
        VDM_SessionContext_t* inContext);

/*!
 *******************************************************************************
 * This callback is called while a CP message is being parsed to notify on each
 *                              CP field-value pair.
 *
 * \param   inField             The full path of the CP param, including
 *                              characteristic type.
 *
 * \param   inValue             The value of the CP param.
 *
 * \param   inIsAlreadyHandled  Whether parameter was already handled by vDM
 *                              engine, as part of CP APPLICATION data handler.
 *
 * \return  \ref VDM_ERR_defs   "An error code" (VDM_ERR_OK on success).
 *******************************************************************************
 */
typedef VDM_Error (*VDM_CPNotifyCB)(UTF8Str inField, UTF8Str inValue,
        IBOOL inIsAlreadyHandled);

#endif
