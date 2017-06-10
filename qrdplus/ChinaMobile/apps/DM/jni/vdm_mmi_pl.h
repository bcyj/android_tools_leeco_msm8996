#ifdef _VDM_MMI_PL_H_
#error "vdm_mmi_pl.h doubly included"
#endif
#define _VDM_MMI_PL_H_

#ifdef __cplusplus
extern "C" {
#endif

//==============================================================================
//
//
//          Types and Definitions
//
//
//==============================================================================

/*!
 *******************************************************************************
 *
 * General types and enumerations common to all MMI screens.
 *
 *******************************************************************************
 */

/*!
 *******************************************************************************
 *  MMI Result Codes.
 * Indicate that the MMI has not been displayed successfully.
 ******************************************************************************
 */
typedef enum {
    E_VDM_MMI_Result_OK, /**< MMI displayed successfully */
    E_VDM_MMI_Result_TextTooLong, /**< Text too long to fit screen. */
    E_VDM_MMI_Result_TooManyOptions, /**< Too many options to be displayed. */
    E_VDM_MMI_Result_Error /**< General failure */
} E_VDM_MMI_Result_t;

/**  Screen context. */
typedef struct {
    UTF8CStr displayText; /**< Text to be displayed on screen. */

    IU32 minDisplayTime; /**< Minimum display time for a screen in seconds.
     0 is used to indicate 'not set'. */
    IU32 maxDisplayTime; /**< Maximum display time for a screen in seconds.
     0 is used to indicate 'not set'. */

} VDM_MMI_ScreenContext_t;

/*!
 *******************************************************************************
 *
 * Types and enumerations for Info-Message screens.
 *
 *******************************************************************************
 */

/** Information Type enumeration. */
typedef enum {
    E_VDM_MMI_InfoType_Startup, /**< DM client has started. */
    E_VDM_MMI_InfoType_Generic, /**< Generic Information Message screen.
     Message text is supplied by the server. */
    E_VDM_MMI_InfoType_Exiting /**< DM Client his about to exit. */

} E_VDM_MMI_InfoType_t;

/*!
 *******************************************************************************
 *
 * Types and enumerations for Confirmation screens.
 *
 *******************************************************************************
 */

/** Default response. */
typedef enum {
    E_VDM_MMI_Confirm_Undefined = -1, /**< No default response is defined. */
    E_VDM_MMI_Confirm_No, /**< Negative command is the default response.*/
    E_VDM_MMI_Confirm_Yes /**< Positive command is the default response. */

} E_VDM_MMI_ConfirmCmd_t;

/*!
 *******************************************************************************
 *
 * Types and enumerations for Input Query screens.
 *
 *******************************************************************************
 */

/** Type of input text*/
typedef enum {
    E_VDM_MMI_InputType_Undefined = 0, /**< Undefined input type. */
    E_VDM_MMI_InputType_Alphanumeric = 1, /**< All alphanumeric characters are allowed. */
    E_VDM_MMI_InputType_Numeric = 2, /**< [0-9], [+/-] and/or decimal point are allowed. */
    E_VDM_MMI_InputType_Date = 3, /**< Input must be in the form of a date. */
    E_VDM_MMI_InputType_Time = 4, /**< Input must be in the form of a time. */
    E_VDM_MMI_InputType_Phone = 5, /**< Phone number format. Allowed chars: [0-9] '+', 'p', 'w', 's' */
    E_VDM_MMI_InputType_IPAddress = 6 /**< IP address format. Allowed chars: [0-9], '.' */

} E_VDM_MMI_InputType_t;

/** The way input text is echoed to the user while typing. */
typedef enum {
    E_VDM_MMI_EchoType_Undefined = 0, /**< Undefined echo type. */
    E_VDM_MMI_EchoType_Plain = 1, /**< echo is identical to input */
    E_VDM_MMI_EchoType_Masked = 2 /**< For e.g. passwords. */

} E_VDM_MMI_EchoType_t;

/** Context of input screen */
typedef struct {
    E_VDM_MMI_InputType_t inputType; /**< Type of input allowed. */
    E_VDM_MMI_EchoType_t echoType; /**< Plain or masked. */
    IU32 maxLength; /**< Maximum number of characters allowed for user input.
     0 is used to indicate 'not set'.*/
    UTF8Str defaultValue; /**< Default response or NULL. */

} VDM_MMI_InputContext_t;

/*!
 *******************************************************************************
 *
 * Types and enumerations for Choice List screens.
 *
 *******************************************************************************
 */
typedef struct {
    UTF8CStr* items; /**< List items. */
    IU32 itemsCount; /**< Number of items in the list */
    IBITFLAGS defaultSelection; /**< Each bit represents the a list item.
     If bit is set, item should be selected by default.*/
    IBOOL isMultipleSelection; /**< Whether more than one item may be selected*/

} VDM_MMI_ListContext_t;

//----------------------------------
//
//  Callbacks
//
//----------------------------------

/*!
 *******************************************************************************
 * Callback function for notifying vDM Engine that an information message
 * from the DM engine has been closed by the user.
 *******************************************************************************
 */
typedef void (*VDM_MMI_infoMsgCB)(void);

/*!
 *******************************************************************************
 * Callback function for notifying vDM Engine that the user has confirmed
 * or denied an action prompted by the DM server.
 *
 * \param    inContinue          TRUE if user has confirmed the action,
 *                               FALSE if user has denied it.
 *******************************************************************************
 */
typedef void (*VDM_MMI_confirmationQueryCB)(IBOOL isContinue);

/*!
 *******************************************************************************
 * Callback function for notifying vDM Engine the user has entered input
 * requested by the DM server.
 *
 * \param    inUserInput         Text entered by the user.
 *******************************************************************************
 */
typedef void (*VDM_MMI_inputQueryCB)(UTF8CStr inUserInput);

/*!
 *******************************************************************************
 * Callback function for notifying vDM Engine that the user has made a selection
 * on a choice list given by the DM server.
 *
 * \param    inUserSelection     Each bit represents an item in the list. If set,
 *                               then user has selected the item.
 *                               In single-selection list, only one bit must be set.
 *******************************************************************************
 */
typedef void (*VDM_MMI_choiceListQueryCB)(IBITFLAGS inUserSelection);

/*!
 *******************************************************************************
 * Callback function for notifying vDM Engine that the user has canceled the
 * operation.
 *
 * May be called from any MMI screen instead of the screen's result
 * callback.
 *******************************************************************************
 */
typedef void (*VDM_MMI_cancelEventCB)(void);

/*!
 *******************************************************************************
 * Callback function for notifying vDM Engine that a timeout event has occurred
 * (maxDisplayTime seconds have passed) without any user response.
 *
 * May be called from any MMI screen instead of the screen's result
 * callback.
 *******************************************************************************
 */
typedef void (*VDM_MMI_timeoutEventCB)(void);

/*!
 *******************************************************************************
 * The vDM MMI observer, which is notified upon any MMI response.
 *******************************************************************************
 */
typedef struct {
    VDM_MMI_infoMsgCB notifyInfoMsgClosed; /**< Info message box callback. */
    VDM_MMI_confirmationQueryCB notifyConfirmationResult; /**< Confirmation dialog callback. */
    VDM_MMI_inputQueryCB notifyInputResult; /**< Input Query dialog callback. */
    VDM_MMI_choiceListQueryCB notifyChoiceListResult; /**< Choice list dialog callback. */
    VDM_MMI_cancelEventCB notifyCanceled; /**< Call from any dialog to notify action has been cancelled. */
    VDM_MMI_timeoutEventCB notifyTimeout; /**< Call from any dialog to notify action has timed out. */
} VDM_MMI_Observer_t;

//==============================================================================
//
//
//          Function Prototypes
//
//
//==============================================================================

/*!
 *******************************************************************************
 * Perform MMI initialization. This will be called before any other MMI
 * function is called. This MUST NOT cause anything to be displayed.
 *
 * return   TRUE if initialization was successful, FALSE if not.
 *******************************************************************************
 */
extern IBOOL VDM_MMI_PL_init(VDM_MMI_Observer_t* inObserver);

/*!
 *******************************************************************************
 * Perform MMI termination. No MMI functions will be called after this has been
 * called, except for perhaps \ref VDM_MMI_PL_init to reinitialize the MMI
 * system
 *******************************************************************************
 */
extern void VDM_MMI_PL_term(void);

/*!
 *******************************************************************************
 * Display an information message.
 *
 * \param   inScreenContext     Context of the message screen to be displayed.
 * \param   inInfoType          Whether this is a message from the DM server,
 *                              or a message notifying DM Engine has been
 *                              initialized / shutting down.
 *
 * \note    in case of a timeout event, timeout callback should be called.
 *
 * \return  E_VDM_MMI_Result_OK on success or an MMI Error result.
 *******************************************************************************
 */
extern E_VDM_MMI_Result_t VDM_MMI_PL_infoMessage(
        VDM_MMI_ScreenContext_t* inScreenContext,
        E_VDM_MMI_InfoType_t inInfoType);

/*!
 *******************************************************************************
 * Prompt user to confirm / deny a session.
 *
 * \param   inScreenContext     Context of the screen to be displayed.
 * \param   inDefaultCommand    Which command (confirm/deny), if any, should
 *                              be selected by default;
 *
 * \note    One of observer's callbacks should be called to notify vDM engine
 *          upon user response, cancellation, or timeout.
 *
 * \return  E_VDM_MMI_Result_OK on success or an MMI Error result.
 *******************************************************************************
 */
extern E_VDM_MMI_Result_t VDM_MMI_PL_confirmationQuery(
        VDM_MMI_ScreenContext_t* inScreenContext,
        E_VDM_MMI_ConfirmCmd_t inDefaultCommand);

/*!
 *******************************************************************************
 * Prompt user to enter text.
 *
 * \param   inScreenContext     Context of the screen to be displayed.
 * \param   inInputContext      Context of the input field.
 *
 * \note    One of observer's callbacks should be called to notify vDM engine
 *          upon user response, cancellation, or timeout.
 *
 * \return  E_VDM_MMI_Result_OK on success or an MMI Error result.
 *******************************************************************************
 */
extern E_VDM_MMI_Result_t VDM_MMI_PL_inputQuery(
        VDM_MMI_ScreenContext_t* inScreenContext,
        VDM_MMI_InputContext_t* inInputContext);

/*!
 *******************************************************************************
 * Prompt user to select one or more item from a list.
 *
 * \param   inScreenContext     Context of the screen to be displayed.
 * \param   inListContext       Context of the list, including whether a single
 *                              selection or a multiple selection is allowed.
 *
 * \note    One of observer's callbacks should be called to notify vDM engine
 *          upon user response, cancellation, or timeout.
 *
 * \return  E_VDM_MMI_Result_OK on success or an MMI Error result.
 *******************************************************************************
 */
extern E_VDM_MMI_Result_t VDM_MMI_PL_choiceListQuery(
        VDM_MMI_ScreenContext_t* inScreenContext,
        VDM_MMI_ListContext_t* inListContext);

/*!
 *******************************************************************************
 * Display progress information to the user.
 *
 * \param   inCurrentProgress   Current progress status.
 * \param   inMaxProgress       Maximum progress value.
 *******************************************************************************
 */
extern void VDM_MMI_PL_updateProgress(IU32 inCurrentProgress,
        IU32 inMaxProgress);

#ifdef __cplusplus
} /* extern "C" */
#endif
