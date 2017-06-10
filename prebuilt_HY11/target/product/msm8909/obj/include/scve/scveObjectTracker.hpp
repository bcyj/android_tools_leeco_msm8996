/**=============================================================================

@file
scveObjectTracker.hpp

@brief
SCVE API Definition for Object-Tracker features.

Copyright (c) 2013-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.

=============================================================================**/

//=============================================================================
///@defgroup scveObjectTracker Object Tracker
///@brief Defines API for SCVE-Object-Tracker feature
///@ingroup scve
//=============================================================================

#ifndef SCVE_OBJECT_TRACKER_HPP
#define SCVE_OBJECT_TRACKER_HPP

#include "scveTypes.hpp"
#include "scveContext.hpp"

namespace SCVE
{

//------------------------------------------------------------------------------
/// @brief
///    Error codes specific to ObjectTracker
///
/// @ingroup scveObjectTracker
//------------------------------------------------------------------------------
enum StatusCodes_ObjectTracker
{
   /// Returned by TrackObjects functions if there are no objects registered to
   /// track.
   SCVE_OBJECTTRACKER_ERROR_NO_OBJECTS               = SCVE_OBJECTTRACKER_ERROR + 1,
   /// Returned by Registration functions if the number of objects being tracked
   /// is already at the maximum allowed.
   SCVE_OBJECTTRACKER_ERROR_TOO_MANY_OBJECTS         = SCVE_OBJECTTRACKER_ERROR + 2,
   /// Returned by Registration functions if the object registraction failed.
   SCVE_OBJECTTRACKER_ERROR_OBJ_REG_FAILED           = SCVE_OBJECTTRACKER_ERROR + 3,
   /// Returned by TrackObjects functions when the size of the result array is
   /// smaller than number of objects registered with the tracker.
   SCVE_OBJECTTRACKER_ERROR_INSUFFCIENT_RESULT_ARRAY = SCVE_OBJECTTRACKER_ERROR + 4,
   /// Returned by TrackObjectsExt function when one or more of the indices
   /// provided are not of valid registered objects. If it was a valid object
   /// before, it might have been unregistered by the time the function is called.
   SCVE_OBJECTTRACKER_ERROR_INVALID_OBJECT_INDEX     = SCVE_OBJECTTRACKER_ERROR + 5,
   /// Returned by Async functions when there are too many concurrent Async
   /// requests. If this error is returned, please wait for the Tracker to send a
   /// callback for the corresponding function before queuing another request.
   SCVE_OBJECTTRACKER_ERROR_TOO_MANY_REQUESTS        = SCVE_OBJECTTRACKER_ERROR + 6,
   /// Returned by GetSaliencyMap function when Saliency Map function is disabled.
   /// This could happen if the object-ids provided not have Saliency Map enabled
   /// on them while registration, or if the color format used is not YUV.
   SCVE_OBJECTTRACKER_ERROR_SALIENCY_MAP_DISABLED    = SCVE_OBJECTTRACKER_ERROR + 7,
};

//------------------------------------------------------------------------------
/// @brief
///    ObjectTracker_Precision structure is used for obtaining tracking results
///    from the Object-Tracker (both through Sync and Async varieties)
///
/// @ingroup scveObjectTracker
//------------------------------------------------------------------------------
typedef enum
{
   /// High precision mode - High processing load, but results are more precise
   SCVE_OBJECTTRACKER_PRECISION_HIGH     = 0,
   /// Low precision mode  - Low processing load, but results are less precise
   SCVE_OBJECTTRACKER_PRECISION_LOW      = 1,
}ObjectTracker_Precision;

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
typedef enum
{
   /// Sets defaults options
   SCVE_OBJECTTRACKER_REGFLAG_DEFAULT    = 0,
   /// Marks the point as a new point
   SCVE_OBJECTTRACKER_REGFLAG_NEW_POINT  = (1<<0),
   /// Tries to return an object immediately with information provided until
   /// that point.
   SCVE_OBJECTTRACKER_REGFLAG_IMM_RETURN = (1<<1),
   /// Enables generation of Saliency Map
   SCVE_OBJECTTRACKER_REGFLAG_SAL_MAP    = (1<<2),
}ObjectTracker_RegistrationFlags;

//------------------------------------------------------------------------------
/// @brief
///    ObjectTrackerResult structure is used for obtaining tracking results from
///    the Object-Tracker (both through Sync and Async varieties)
/// @param sROI
///    ROI - current location and size of the object being tracked.
/// @param nConfidence
///    Confidence (0 to 100).
/// @param nObjectIndex
///    Index of the object whose tracking results are represented.
///
/// @ingroup scveObjectTracker
//------------------------------------------------------------------------------
typedef struct
{
   SCVE::Rect         sROI;
   uint32_t           nConfidence;
   uint32_t           nObjectIndex;
} ObjectTrackerResult;


class ObjectTracker;

//------------------------------------------------------------------------------
/// @brief
///    Definition of the Object-Tracker's Track Callback corresponding to the
///    ObjectTracker::TrackObjects_Async or ObjectTracker::TrackObjectsExt_Async
///    function.
/// @details
///    The Track-Callback function is called for every
///    ObjectTracker::TrackObjects_Async or ObjectTracker::TrackObjectsExt_Async
///    call you make. When the callback is called, the tracking task would be
///    completed and the result will be available in the result-array provided.
/// @param status
///    Contains the status of the Async request.
/// @param pResult
///    Pointer to ObjectTrackerResult array that was passed in through Async
///    function. This array will now contains results of tracking.
/// @param pResultSize
///    Represents number of elements in the 'pResult' array. This only represents
///    size of the array (same as what was passed into the Async function) and
///    not the number of valid results.
/// @param pSessionUserData
///    User-data that was set by you in the ObjectTracker::newInstance function.
/// @param pTaskUserData
///    User-data that was set by you in the TrackObjects(Ext)_Async function
///    which corresponds to this callback.
/// @remarks
///    - See 'Using Async Functions' section in the documentation of
///      ObjectTracker class.
/// @ingroup scveObjectTracker
//------------------------------------------------------------------------------
typedef void (*ObjectTracker_TrackCallback) ( SCVE::Status                status,
                                              SCVE::ObjectTrackerResult*  pResult,
                                              uint32_t                    nResultSize,
                                              void*                       pSessionUserData,
                                              void*                       pTaskUserData );

//------------------------------------------------------------------------------
/// @brief
///    Definition of the Object-Tracker's Registration Callback corresponding to
///    the ObjectTracker::RegisterObject_Async function.
/// @details
///    The Reg-Callback function is called for every
///    ObjectTracker::RegisterObject_Async call you make. When the callback is
///    called, the registration would have finished and all subsequent TrackObjects
///    call will not start tracking this object.
/// @param status
///    Contains the status of the Async request.
/// @param nIndex
///    Index of the newly registered object.
/// @param pSessionUserData
///    User-data that was set by you in the ObjectTracker::newInstance function.
/// @param pTaskUserData
///    User-data that was set by you in the RegisterObject_Async function
///    which corresponds to this callback.
/// @ingroup scveObjectTracker
//------------------------------------------------------------------------------
typedef void (*ObjectTracker_RegCallback)   ( SCVE::Status          status,
                                              uint32_t              nIndex,
                                              void*                 pSessionUserData,
                                              void*                 pTaskUserData );

//------------------------------------------------------------------------------
/// @brief
///    SCVE::ObjectTracker class that implements SCVE's Object Tracker feature.
///
/// @details
///    - It can track ROI Rectangles that can be registered at run-time (using
///      RegisterObject_Sync and RegisterObject_Async) over frames of
///      an input video sequence.
///    - Supports the following color formats - SCVE::SCVE_COLORFORMAT_GREY_8BIT,
///      SCVE_COLORFORMAT_YUV_NV12 and SCVE_COLORFORMAT_YUV_NV21.
///    - Provides two tracking modes for appropriate use cases -
///      SCVE_OBJECTTRACKER_PRECISION_HIGH and SCVE_OBJECTTRACKER_PRECISION_LOW.
///    - Has the ability to re-identify a registered object when it leaves the
///      scope of the the picture and returns to view many frames later.
///    - Is able to cope up with marginal changes in perspective (rotation) of
///      the tracked object.
///
///    <b>Using Async functions</b>
///    - To use Async functions, a corresponding callback function needs to
///      supplied through ObjectTracker::newInstance function. Otherwise, the
///      Async function would return error.
///    - The Async functions return immediately after they have queued the
///      request with the tracker instance.
///    - After the task is completed asynchronously, the corresponding
///      callback function will be called, which will provide the necessary output
///      of the function.
///    - Multiple requests can be queued concurrently through Async functions. They
///      will be processed in the order they were received. However, if there are more
///      than 3 concurrent requests, the function will return the error code.
///    - There are two mechanisms to associate every callback function with it's
///      corresponding request. \b SessionUserData and \b TaskUserData.
///      \a SessionUserData and \a TaskUserData pointers are provided as parameters to
///      every callback function. As the names suggests, \a SessionUserData represents
///      user-data associated with the session from which the callback is coming from
///      and \a TaskUserData represents the original request (Async call) that triggered
///      this callback. You can provide \a SessionUserData to the tracker through the
///      newInstance function, and will remain unchanged for life of that tracker instance.
///      \a TaskUserData is provided to the tracker through the Async functions (every
///      Async function has this parameter).
///      \n
///
///    <b>Choosing ROI for Registration</b>
///    - Since the ROI is a rectangle in the target image, one will need
///      to choose an ROI such that you don't include too much of the background
///      instead of the actual target. This will reduce the chances of false-
///      positives in tracking results.
///
/// @ingroup scveObjectTracker
//------------------------------------------------------------------------------
class SCVE_API ObjectTracker
{
   public:

      //------------------------------------------------------------------------------
      /// @brief
      ///    Deprecated old newInstance function. Though this function is still
      ///    supported for backwards compatibility, it is recommended to use the newer
      ///    newInstance function.
      ///
      /// @deprecated Old version, still supported for backwards compatibility.
      //------------------------------------------------------------------------------
      static ObjectTracker* newInstance    ( SCVE::Context*                    pScveContext,
                                             SCVE::ImageDimension              sDimension,
                                             SCVE::ObjectTracker_TrackCallback cbTracker = NULL,
                                             SCVE::ObjectTracker_RegCallback   cbRegistration = NULL,
                                             SCVE::ObjectTracker_Precision     ePrecision = SCVE_OBJECTTRACKER_PRECISION_HIGH,
                                             void*                             pSessionUserData = NULL);


      //------------------------------------------------------------------------------
      /// @brief
      ///    Creates a new instance of Object Tracker feature.
      /// @details
      ///    This is a static factory-function that creates a new instance of
      ///    ObjectTracker feature. You will not be able to instanciate the class
      ///    by calling the constructor directly. You will be able to create a
      ///    new instance only through this function.
      /// @param pScveContext
      ///    Context under which this feature should be instantiated.
      /// @param sDimension
      ///    ImageDimension structure, which indicates the dimensions of the
      ///    images on which tracking is performed.
      /// @param eColorFormat
      ///    Sets what color format images will be provided. Object Tracker supports SCVE_COLORFORMAT_YUV_NV12,
      ///    SCVE_COLORFORMAT_YUV_NV21 and SCVE_COLORFORMAT_GREY_8BIT
      /// @param cbTracker
      ///    [\a optional] Function pointer to a callback function of type
      ///    ObjectTracker::ObjectTracker_TrackCallback, which will be called for every
      ///    TrackObjects_Async or TrackObjectsExt_Async function call you make.
      ///    This is called once the tracking of that frame finishes. You can pass
      ///    NULL (which is also the default value) to this, which will make
      ///    TrackObjects_Async and TrackObjectsExt_Async functions unusable.
      /// @param cbRegistration
      ///    [\a optional] Function pointer to a callback function of type
      ///    ObjectTracker_RegCallback, which will be called for every
      ///    RegisterObject_Async function call you make. This is called once the
      ///    registration of the object finishes. You can pass NULL (which is also
      ///    the default value) to this, which will make RegisterObject_Async
      ///    function unusable.
      /// @param ePrecision
      ///    [\a optional] Denotes the precision mode in which the tracker should
      ///    operate in. By default, it will always operate in high-precision mode.
      ///    However, the user can chose to switch to low-precision mode, which will
      ///    reduce the computation load on the processor. The consequence of that
      ///    is the results generated by the tracker will be less precise
      ///    (will vary by a few pixels compared to high-precision mode).
      /// @param pSessionUserData
      ///    A private pointer the user can set with this session, and this pointer
      ///    will be provided as parameter to all callback functions originating
      ///    from the current session. This could be used to associate a callback
      ///    to this session. This can only be set once, while creating a new instance
      ///    of the tracker. This value will not/cannot-be changed for life of a
      ///    tracking session.
      /// @retval Non-NULL
      ///    If the initialization was successful. The returned value is a
      ///    pointer to the newly created Object Tracker instance.
      /// @retval NULL
      ///    If the initialization has failed. Check logs for any error messages if
      ///    this occurs.
      ///
      /// @remarks
      ///    - Use the symmetric ObjectTracker::deleteInstance function to de-initialize
      ///    and cleaup the Object Tracker instances created by this function.
      ///    - If the fucntion returns NULL, check the logs and look for any error messages.
      //------------------------------------------------------------------------------
      static ObjectTracker* newInstance    ( SCVE::Context*                    pScveContext,
                                             SCVE::ImageDimension              sDimension,
                                             SCVE::ColorFormat                 eColorFormat,
                                             SCVE::ObjectTracker_TrackCallback cbTracker = NULL,
                                             SCVE::ObjectTracker_RegCallback   cbRegistration = NULL,
                                             SCVE::ObjectTracker_Precision     ePrecision = SCVE_OBJECTTRACKER_PRECISION_HIGH,
                                             void*                             pSessionUserData = NULL);

      //------------------------------------------------------------------------------
      /// @brief
      ///    De-initializes and cleans up all the resources associated with the
      ///    Object Tracker instance.
      /// @details
      ///    This is a factory-destructor method that will cleanup all resources
      ///    associated an Object Tracker instance. The same delete function will be
      ///    able to handle instances created for different operation modes.
      /// @param pObjectTracker
      ///    Pointer to the Object Tracker instance. This is the same pointer that is
      ///    returned by the ObjectTracker::newInstance function.
      ///
      /// @retval SCVE_SUCCESS If the deletion was successful.
      /// @retval SCVE_ERROR_INTERNAL If any error occured.
      //------------------------------------------------------------------------------
      static Status deleteInstance         ( SCVE::ObjectTracker*              pObjectTracker );

      //------------------------------------------------------------------------------
      /// @brief
      ///    A synchronous function to register a Region-of-Interest (ROI) as a
      ///    trackable object with the Tracker.
      ///
      /// @details
      ///    Once the registration is successful and complete, all subsequent
      ///    TrackObjects_Sync or TrackObjects_Async calls will start tracking this
      ///    object.
      /// @param pImage
      ///    Pointer to an Image structure that contains information about the
      ///    image in which the target ROI is located.
      /// @param sROI
      ///    Co-ordinates of the ROI rectangle that represents the object to be
      ///    tracked.
      /// @param bGenerateSaliencyMap
      ///    Boolean to indicate whether to generate saliency map for this object.
      /// @param pIndex
      ///    [\a optional] Pointer to a scalar output parameter. When this points
      ///    to non-NULL, once the function finishes, the output parameter will
      ///    contain the unique 'index' of the registered object. This index could
      ///    then be used with ObjectTracker::UnregisterObject function.
      /// @return
      ///    Returns a status code from either SCVE::StatusCodes or
      ///    SCVE::StatusCodes_ObjectTracker.
      /// @remarks
      ///    - See 'Choosing ROI for Registration' in the documentation of
      ///    ObjectTracker class.
      ///
      /// @see
      ///    RegisterObject_Async
      //------------------------------------------------------------------------------
      virtual Status RegisterObject_Sync   ( SCVE::Image*       pImage,
                                             SCVE::Rect         sROI,
                                             uintptr_t*          pIndex,
                                             uint32_t           eFlags = SCVE_OBJECTTRACKER_REGFLAG_DEFAULT ) = 0;
      //------------------------------------------------------------------------------
      /// @brief
      ///   Register an object to track from a point location. This API
      ///   automatically find the ROI based on motion and color information, and
      ///   register the object.
      ///
      /// @details
      ///    Once the registration is successful and complete, all subsequent
      ///    TrackObjects_Sync or TrackObjects_Async calls will start tracking this
      ///    object.
      /// @param pImage
      ///    Pointer to an Image structure that contains information about the
      ///    image in which the target ROI is located.
      /// @param sPoint
      ///    Co-ordinates of the Point that represents the object to be
      ///    tracked.
      ///
      /// @param motionRoiFlag
      ///      Flag to specify if it is the first attempt to register the object by point.
      ///
      /// @param pIndex
      ///    [\a optional] Pointer to a scalar output parameter. When this points
      ///    to non-NULL, once the function finishes, the output parameter will
      ///    contain the unique 'index' of the registered object. This index could
      ///    then be used with ObjectTracker::UnregisterObject function.
      /// @return
      ///    Returns a status code from either SCVE::StatusCodes or
      ///    SCVE::StatusCodes_ObjectTracker.
      /// @remarks
      ///    - See 'Choosing ROI for Registration' in the documentation of
      ///    ObjectTracker class.
      ///
      /// @see
      ///    RegisterObject_Async
      //------------------------------------------------------------------------------
      virtual Status RegisterObjectByPoint_Sync( SCVE::Image*   pImage,
                                                 SCVE::Point    sPoint,
                                                 uintptr_t*      pIndex,
                                                 uint32_t       eFlags = SCVE_OBJECTTRACKER_REGFLAG_DEFAULT ) = 0;

      //------------------------------------------------------------------------------
      /// @brief
      ///    A synchronous function that tracks all previously registered objects
      ///    in the image provided.
      /// @details
      ///    - This is a synchronous function, and when it returns the results of
      ///    tracking will be available in the ObjectTrackerResult array provided as
      ///    parameter.
      ///    - The results will be provided in the order in which the objects were
      ///    registered. For example, if you registered object X first and object Y
      ///    next, then pResult[0] is the result for object X and pResult[1] is the
      ///    result for object Y.
      ///    - If the tracker fails to track a particular object, the confidence
      ///    value in the results of that object will be zero.
      /// @param pImage
      ///    Image on which the tracking will be performed. The dimensions of this
      ///    image should match with ImageDimension that was provided while creating
      ///    the object tracker instance.
      /// @param pResult
      ///    Pointer to an array of SCVE::ObjectTrackerResult objects. This is an
      ///    output parameter, and when the function returns will have the tracking
      ///    results of all the registered objects.
      /// @param nSize
      ///    Size of the pResult array.
      /// @return
      ///    Returns a status code from either SCVE::StatusCodes or
      ///    SCVE::StatusCodes_ObjectTracker.
      /// @remarks
      ///    - If the function returns SCVE_SUCCESS, it means that function has
      ///    successfully completed. It doesn't mean the tracker was able to track
      ///    all registered objects.
      //------------------------------------------------------------------------------
      virtual Status TrackObjects_Sync     ( SCVE::Image*                pImage,
                                             SCVE::ObjectTrackerResult*  pResult,
                                             uint32_t                    nSize ) = 0;

      //------------------------------------------------------------------------------
      /// @brief
      ///    A extended version of synchronous tracking function that enables
      ///    tracking only a subset of registered objects.
      /// @details
      ///    This function is exactly similar to TrackObjects_Sync with the exception
      ///    that this function lets you provide indices of objects that should be
      ///    tracked. This function facilitates use-cases where you would want to
      ///    temporarily suspend tracking of certain objects.
      /// @param pImage
      ///    Image in which the objects will be tracked.
      /// @param pIndices
      ///    Pointer to an array holding indices of objects that need to be tracked.
      /// @param nNumIndices
      ///    Number of entries in the pIndices array.
      /// @param pResult
      ///    Pointer to an array of ObjectTrackerResult objects which will contain
      ///    the tracking results once the function returns.
      /// @param nSize
      ///    Size of the pResult array.
      //------------------------------------------------------------------------------
      virtual Status TrackObjectsExt_Sync  ( SCVE::Image*                pImage,
                                             uintptr_t*                   pIndices,
                                             uint32_t                    nNumIndices,
                                             SCVE::ObjectTrackerResult*  pResult,
                                             uint32_t                    nSize ) = 0;

      //------------------------------------------------------------------------------
      /// @brief
      ///    An asynchronous function to register a Region-of-Interest (ROI) as a
      ///    trackable object with the Tracker.
      /// @details
      ///    Once the registration is successful and complete, all subsequent
      ///    TrackObjects_Sync or TrackObjects_Async calls will start tracking this
      ///    object.
      /// @param pImage
      ///    Pointer to an Image structure that contains information about the
      ///    image in which the target ROI is located.
      /// @param sROI
      ///    Co-ordinates of the ROI rectangle that represents the object to be
      ///    tracked.
      /// @param pTaskUserData
      ///    User-data (a private pointer that can be set by the user) associated with
      ///    the current registration task. This pointer will be sent as a parameter
      ///    in the ObjectTracker_RegCallback function that was registered with the
      ///    tracker instance.
      /// @return
      ///    Returns a status code from either SCVE::StatusCodes or
      ///    SCVE::StatusCodes_ObjectTracker.
      /// @remarks
      ///    - See 'Using Async Functions' section in the documentation of
      ///      ObjectTracker class.
      ///    - See 'Choosing ROI for Registration' section in the documentation of
      ///      ObjectTracker class.
      /// @see RegisterObject_Sync
      /// @see SCVE::ObjectTracker_RegCallback
      //------------------------------------------------------------------------------
      virtual Status RegisterObject_Async  ( SCVE::Image*       pImage,
                                             SCVE::Rect         sROI,
                                             void*              pTaskUserData = NULL,
                                             uint32_t           eFlags = SCVE_OBJECTTRACKER_REGFLAG_DEFAULT ) = 0;

      //------------------------------------------------------------------------------
      /// @brief
      ///    An asynchronous function to Register an object to track from a point location as a
      ///    trackable object with the Tracker.
      /// @details
      ///    Once the registration is successful and complete, all subsequent
      ///    TrackObjects_Sync or TrackObjects_Async calls will start tracking this
      ///    object.
      /// @param pImage
      ///    Pointer to an Image structure that contains information about the
      ///    image in which the target ROI is located.
      /// @param sPoint
      ///    Co-ordinates of the point that represents the object to be
      ///    tracked.
      /// @param motionRoiFlag
      ///      Flag to specify if it is the first attempt to register the object by point.
      /// @param pTaskUserData
      ///    User-data (a private pointer that can be set by the user) associated with
      ///    the current registration task. This pointer will be sent as a parameter
      ///    in the ObjectTracker_RegCallback function that was registered with the
      ///    tracker instance.
      /// @return
      ///    Returns a status code from either SCVE::StatusCodes or
      ///    SCVE::StatusCodes_ObjectTracker.
      /// @remarks
      ///    - See 'Using Async Functions' section in the documentation of
      ///      ObjectTracker class.
      ///    - See 'Choosing ROI for Registration' section in the documentation of
      ///      ObjectTracker class.
      /// @see RegisterObjectByPoint_Sync
      /// @see SCVE::ObjectTracker_RegCallback
      //------------------------------------------------------------------------------
      virtual Status RegisterObjectByPoint_Async( SCVE::Image*    pImage,
                                                  SCVE::Point     sPoint,
                                                  void*           pTaskUserData = NULL,
                                                  uint32_t        eFlags = SCVE_OBJECTTRACKER_REGFLAG_DEFAULT ) = 0;

      //------------------------------------------------------------------------------
      /// @brief
      ///    An asynchronous function that tracks all previously registered objects
      ///    in the image provided.
      /// @details
      ///    Please refer to TrackObjects_Sync for all the details. This function
      ///    works exactly same as synchronous version, with the exception that
      ///    this function returns almost immediately, and the results are made
      ///    available through the callback function.
      /// @param pImage
      ///    See TrackObjects_Sync for details.
      /// @param pResult
      ///    See TrackObjects_Sync for details.
      /// @param nSize
      ///    See TrackObjects_Sync for details.
      /// @param pTaskUserData
      ///    User-data (a private pointer that can be set by the user) associated with
      ///    the current tracking task. This pointer will be sent as a parameter
      ///    in the ObjectTracker_TrackCallback function that was registered with the
      ///    tracker instance.
      /// @return
      ///    Returns a status code from either SCVE::StatusCodes or
      ///    SCVE::StatusCodes_ObjectTracker.
      /// @remarks
      ///    - See 'Using Async Functions' section in the documentation of
      ///      ObjectTracker class.
      /// @see TrackObjects_Sync
      /// @see SCVE::ObjectTracker_TrackCallback
      ///
      //------------------------------------------------------------------------------
      virtual Status TrackObjects_Async    ( SCVE::Image*                pImage,
                                             SCVE::ObjectTrackerResult*  pResult,
                                             uint32_t                    nSize,
                                             void*                       pTaskUserData = NULL ) = 0;

      //------------------------------------------------------------------------------
      /// @brief
      ///    An asynchronous function that tracks specified registered objects in the
      ///    image provided.
      /// @details
      ///    Please refer to TrackObjectsExt_Sync for all the details. This function
      ///    works exactly same as synchronous version, with the exception that
      ///    this function returns almost immediately, and the results are made
      ///    available through the callback function.
      /// @param pImage
      ///    See TrackObjectsExt_Sync for details.
      /// @param pIndices
      ///    See TrackObjectsExt_Sync for details.
      /// @param nNumIndices
      ///    See TrackObjectsExt_Sync for details.
      /// @param pResult
      ///    See TrackObjectsExt_Sync for details.
      /// @param nSize
      ///    See TrackObjectsExt_Sync for details.
      /// @param pTaskUserData
      ///    User-data (a private pointer that can be set by the user) associated with
      ///    the current tracking task. This pointer will be sent as a parameter
      ///    in the ObjectTracker_TrackCallback function that was registered with the
      ///    tracker instance.
      /// @return
      ///    Returns a status code from either SCVE::StatusCodes or
      ///    SCVE::StatusCodes_ObjectTracker.
      /// @remarks
      ///    - See 'Using Async Functions' section in the documentation of
      ///      ObjectTracker class.
      /// @see TrackObjectsExt_Sync
      /// @see SCVE::ObjectTracker_TrackCallback
      //------------------------------------------------------------------------------
      virtual Status TrackObjectsExt_Async ( SCVE::Image*                pImage,
                                             uintptr_t*                  pIndices,
                                             uint32_t                    nNumIndices,
                                             SCVE::ObjectTrackerResult*  pResult,
                                             uint32_t                    nSize,
                                             void*                       pTaskUserData = NULL ) = 0;

      //------------------------------------------------------------------------------
      /// @brief
      ///    Unregisters a previously registered object.
      /// @details
      ///    Takes the object index provided by RegisterObject functions, and
      ///    unregisters the objects from any further tracking. This will facilitate
      ///    unregistering an object in teh middle of a tracking session.
      /// @param nIndex
      ///    Index of the object to be unregistered. This should be same as what was
      ///    returned by register-object functions.
      /// @return
      ///    Returns a status code from either SCVE::StatusCodes or
      ///    SCVE::StatusCodes_ObjectTracker.
      /// @remarks
      ///    While unregistering every object before 'deleting' an instance is a
      ///    good practice, it is not a required activity. 'Deleting' an instance
      ///    also involves unregistering all objects that are not already unregistered.
      //------------------------------------------------------------------------------
      virtual Status UnregisterObject      ( uintptr_t nIndex ) = 0;


      //------------------------------------------------------------------------------
      /// @brief
      ///    Returns the Saliency Map of specified list of objects.
      /// @details
      ///
      /// @param
      //------------------------------------------------------------------------------
      virtual Status GetSaliencyMap        ( uintptr_t*      pIndices,
                                             uint32_t       nNumObjects,
                                             SCVE::Image*   pSaliencyMap ) = 0;

   protected:
      //------------------------------------------------------------------------------
      /// @brief
      ///    Protected destructor. Please use the static ObjectTracker::deleteInstance
      ///    function for de-initialize and cleaning-up the Object Tracker instances.
      //------------------------------------------------------------------------------
      virtual ~ObjectTracker               ( );

}; //class ObjectTracker

} //namespace SCVE

#endif //SCVE_OBJECT_TRACKER_HPP