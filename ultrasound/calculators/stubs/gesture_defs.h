/*===========================================================================
                           gesture_defs.h

DESCRIPTION: These are the common gesture definitions and is self-contained.

INITIALIZATION AND SEQUENCING REQUIREMENTS: None

Copyright (c) 2012-2013 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
=============================================================================*/
#ifndef INC_QTI_GESTURE_GESTURE_DEFS_H
#define INC_QTI_GESTURE_GESTURE_DEFS_H

#include <cstddef>
#include <cstdint>



// ms and gcc do not have compatible safe string copy functions so create one
static void GsStrlcpy(char* dst, char const* src, size_t len)
{
  if(dst == NULL || len == 0) return;
  if(src != NULL)
  {
    while(*src && --len)
    {
      *dst++ = *src++;
    }
  }
  *dst = '\0';
}



/******************************************************************************
 * Base gesture types
 ******************************************************************************/

// Gesture operating modes
enum GsMode
{
  // externally exposed modes
  kGsModeNone = 0,
  kGsModeNearSwipe = 101,
  kGsModeHandDetect = 102,
  kGsModeEngagement = 103,
  kGsModeEngagementSwipe = 104,
  kGsModeHandTracking = 105,
  // kGsModeClickHold = 106, deprecated, was used for Fruit Ninja demo
  kGsModeUltrasoundSwipe = 107,
  kGsModeUltrasoundProximity = 108,
  kGsModeDrag = 109,
  kGsModeNearAndPointer = 110,
  kGsModeNearFarSwipe = 111,
  kGsModeTrajectory = 113,
  kGsModeProgressiveCircle = 114,
  kGsModeCircle = 115,
  kGsModeHandZoom = 116,

  // internal modes
  kGsModeCapture144 = 1001,     // 144 line capture at 30 fps
  kGsModeCapture720 = 1002,     // 720 line capture at 30 fps
  kGsModeCapture480 = 1003,     // 480 line capture at 30 fps
  kGsModeQht = 1004,            // QHT only
  kGsModeCapture144_60 = 1005,  // 144 line capture at 60 fps
  kGsModeCapture240 = 1006,     // 240 line capture at 30 fps
  kGsModeConcurrent = 1007,     // concurrent US and camera near swipe
};

// Operating submodes, depends on mode
enum GsSubmode
{
  kGsSubmodeNone = 0
};

// Device orientation
enum GsOrientation
{
  kGsOrientation_0 = 0,
  kGsOrientation_90 = 1,
  kGsOrientation_180 = 2,
  kGsOrientation_270 = 3
};

// 3D vector, z = 0 for 2D
struct GsVector
{
  float x_;
  float y_;
  float z_;
  float error_;   // radius of accuracy

  friend bool operator==(GsVector const& lhs, GsVector const& rhs)
  {
    return
      lhs.x_ == rhs.x_ &&
      lhs.y_ == rhs.y_ &&
      lhs.z_ == rhs.z_ &&
      lhs.error_ == rhs.error_;
  }

  friend bool operator!=(GsVector const& lhs, GsVector const& rhs)
  {
    return !(lhs == rhs);
  }

  GsVector() : x_(0), y_(0), z_(0), error_(1)
  {}

  GsVector(float x, float y, float z, float error) :
    x_(x), y_(y), z_(z), error_(error)
  {}
};

// 3D region of interest, z = 0 for 2D, begin == end for a point
struct GsRoi
{
  GsVector begin_;
  GsVector end_;

  friend bool operator==(GsRoi const& lhs, GsRoi const& rhs)
  {
    return
      lhs.begin_ == rhs.begin_ &&
      lhs.end_ == rhs.end_;
  }

  friend bool operator!=(GsRoi const& lhs, GsRoi const& rhs)
  {
    return !(lhs == rhs);
  }

  GsRoi() : begin_(), end_()
  {}

  GsRoi(GsVector const& begin, GsVector const& end) : begin_(begin), end_(end)
  {}
};

// Coordinate Mode
enum GsCoordinateMode
{
  kGsCoordinateModeNormalized,
  kGsCoordinateModeScreen
};

//  Mouse click mode
enum GsClickMode
{
  kGsClickModeNone,
  kGsClickModeHover,
  kGsClickModePose
};

// Gesture outcome
struct GsOutcome
{
  // gesture types
  enum Type
  {
    kTypeNone = 0,
    kTypeDetection = 201,
    kTypeEngagement = 202,
    kTypeTracking = 203,
    kTypeSwipe = 204,
    kTypeMouse = 205,
    kTypeHover = 206,
    kTypeProx = 207,
    kTypeCover = 208,
    kTypeOutline = 209,
    kTypeSkeletal = 210,
    kTypeCircle = 211,
  };

  // gesture subtypes
  enum Subtype
  {
    kSubtypeNone = 0,

    // Detection and Engagement
    kSubtypeLeftPalm = 301,
    kSubtypeRightPalm = 302,
    kSubtypeLeftFist = 303,
    kSubtypeRightFist = 304,

    // Tracking
    kSubtypeNormalized = 401,
    kSubtypeScreen = 402,

    // Swipe
    kSubtypeSwipeLeft = 501,
    kSubtypeSwipeRight = 502,
    kSubtypeSwipeUp = 503,
    kSubtypeSwipeDown = 504,

    // Mouse
    kSubtypeMouseUp = 601,
    kSubtypeMouseDown = 602,
    kSubtypeMouseClear = 603,

    // Proximity
    kSubtypeProxIdle = 701,
    kSubtypeProxDetect = 702,
    kSubtypeProxCovered = 703,

    // Cover
    kSubtypeCover = 801,
    kSubtypeUncover = 802,

    // Circle
    kSubtypeCircleDiscrete = 901,
    kSubtypeCircleStart = 902,
    kSubtypeCircleMove = 903,
    kSubTypeCircleStop = 904,
  };

  int version_;         // the version of this structure, initially 0
  Type type_;           // the type of gesture detected
  Subtype subtype_;     // gesture subtype dependent on type
  uint64_t timestamp_;  // detection camera frame time in microseconds
  int id_;              // identifies this outcome as the same object over time
  GsRoi location_;      // region for pose, start position for motion
  float velocity_;      // gesture velocity
  float confidence_;    // 1.0 = 100% confidence

  // extend the outcome here, incrementing the version

  GsOutcome() :
    version_(0),
    type_(kTypeNone),
    subtype_(kSubtypeNone),
    timestamp_(0),
    id_(0),
    location_(),
    velocity_(0),
    confidence_(0)
  {}

  GsOutcome(
    Type type,
    Subtype subtype,
    uint64_t timestamp,
    int id,
    GsRoi location,
    float velocity,
    float confidence) :
      version_(0),
      type_(type),
      subtype_(subtype),
      timestamp_(timestamp),
      id_(id),
      location_(location),
      velocity_(velocity),
      confidence_(confidence)
  {}
};

// gesture capability
struct GsCapability
{
  int mode_;            // GsMode supported
  float confidence_;    // confidence level for the mode
  float power_;         // power requirement for the mode

  GsCapability() :
    mode_(kGsModeNone),
    confidence_(0),
    power_(0)
  {}

  GsCapability(int mode, float confidence, float power) :
    mode_(mode),
    confidence_(confidence),
    power_(power)
  {}
};

// gesture configuration parameters, sent from the application layer
struct GsConfigurationParams
{
  int version_;                     // version of this structure, initially 0
  int mode_;                        // GsMode, operating mode
  int submode_;                     // GsSubmode, operating submode
  GsOrientation orientation_;       // device orientation
  bool touchEnable_;                // true to enable touch injection
  GsRoi coordinateRange_;           // application coordinate mapping
  GsCoordinateMode coordinateMode_; // coordinate interpretation
  int cursor_;                      // cursor type, 0 for no cursor
  int clickMode_;                   // GsClickMode, click method
  int camera_;                      // camera, set to -1 for no camera

  // extend the parameters here, incrementing the version

  GsConfigurationParams() :
    version_(0),
    mode_(kGsModeNone),
    submode_(kGsSubmodeNone),
    orientation_(kGsOrientation_0),
    touchEnable_(false),
    coordinateRange_(),
    coordinateMode_(kGsCoordinateModeNormalized),
    cursor_(0),
    clickMode_(kGsClickModeNone),
    camera_(-1)
  {}
};

// internal parameters, sent from the gesture manager
struct GsInternalParams
{
  int version_;                 // version of this structure, initially 0
  bool cameraDebug_;            // enable camera gesture debugging
  int imageCaptureCount_;       // capture camera images
  GsRoi cursorMapping_;         // cursor/touch coordinate mapping

  // extend the parameters here, incrementing the version

  GsInternalParams() :
    version_(0),
    cameraDebug_(false),
    imageCaptureCount_(0),
    cursorMapping_()
  {}
};

// log message
struct GsLogMessage
{
  char message[100];
  GsLogMessage(char const* msg = "\0")
  {
    GsStrlcpy(message, msg, sizeof message);
  }
};


/******************************************************************************
 * Gesture Bus Interface
 ******************************************************************************/

// The maximum gesture bus message size
int const GS_BUS_MESSAGE_SIZE_MAX = 1024;

// The maximum number of gesture bus message types
int const GS_BUS_MESSAGE_MAX = 20;

// The bus message dispatch method type
class GsBusListener;
typedef bool (GsBusListener::* GsBusDispatch)(void const* message);

/**
 *  Virtual base class for the bus callback template.
 *
 *  Because it is not possible to pass a function pointer or pointer to member
 *  between processes, this class assigns a dispatch index to each dispatch
 *  method. The dispatch index is passed on the bus and then a dispatch table
 *  at the receiving end converts the index back to a dispatch method.
 */
class GsBusCallbackBase
{
public:

  GsBusCallbackBase() : index(0) {}

  // Add a dispatch method returning its dispatch index
  int AddEntry(GsBusDispatch entry)
  {
    if(index >= GS_BUS_MESSAGE_MAX)
    {
      // dispatch table overflow, should really generate an error somehow
      return -1;
    }

    // add entry to dispatch table
    dispatchTable[index] = entry;
    return index++;
  }

  // get the dispatch method for an index, NULL if invalid
  GsBusDispatch GetDispatch(int dispatchIndex)
  {
    return dispatchIndex < 0 || dispatchIndex >= index ?
      NULL : dispatchTable[dispatchIndex];
  }

private:

  int index;
  GsBusDispatch dispatchTable[GS_BUS_MESSAGE_MAX];
};

// Template for defining a callback method for a particular message type.
template <typename T> class GsBusCallback : private virtual GsBusCallbackBase
{
public:

  GsBusCallback() :
    dispatchIndex(AddEntry(&GsBusCallback<T>::Dispatch)),
    handled(false)
  {
    // if you get a compile error here, your message is too big
    typedef char S[GS_BUS_MESSAGE_SIZE_MAX - int(sizeof(T))];
  }
  virtual ~GsBusCallback() {}

  // Get the dispatch table index of this callback.
  int GetDispatchIndex() const { return dispatchIndex; }

private:

  // Invoked when receiving a particular message. Override in the derived class to
  // receive a gesture bus message callback.
  virtual void OnMessage(T const& /*message*/) { handled = false; }

  // Dispatch a message to this callback, returns true if handled.
  bool Dispatch(void const* message)
  {
    // convert the raw message back to the proper type
    handled = true;
    OnMessage(*static_cast <T const*> (message));
    return handled;
  }

  int dispatchIndex;
  bool handled;
};


// Shutdown message, empty
struct GsBusShutdown {};

// Connect message
struct GsBusConnect
{
  char sourceName[20];    // user friendly name of the gesture source
  explicit GsBusConnect(char const* name)
  {
    GsStrlcpy(sourceName, name, sizeof sourceName);
  }
};

// Disconnect message, empty
struct GsBusDisconnect {};

// Outcome sequence end, empty
struct GsOutcomeEnd {};

// A cursor event, touch or positioning.
struct GsCursorEvent
{
  /**
   *  The cursor type to display, 0 = none. This will be 0 for touch injection
   *  events because we don't want to show that on the screen.
   */
  int cursor_;

  // Microseconds to delay before the event
  uint64_t timeBefore_;

  // Microseconds to delay after the event
  uint64_t timeAfter_;

  // Cursor position. z = 0 for touch, > 0 for not touch.
  GsVector position_;

  GsCursorEvent(int cursor, uint64_t const& timeBefore, uint64_t const& timeAfter, GsVector const& position) :
    cursor_(cursor),
    timeBefore_(timeBefore),
    timeAfter_(timeAfter),
    position_(position)
  {}
};

// Client request to ignore a message type
struct GsMessageIgnore
{
  // the message type to ignore
  int dispatchIndex_;

  GsMessageIgnore(int dispatchIndex) : dispatchIndex_(dispatchIndex)
  {}
};

// camera state message
struct GsCameraState
{
  bool isOn_;
  GsCameraState(bool isOn) : isOn_(isOn) {}
};


/**
 * Abstract Listener (callback) for gesture bus messages
 *
 * This class provides a stable interface for listening to gesture bus messages
 * that is unchanged when new bus messages are added.
 */
class GsBusAbstractListener
{
public:

  // Dispatch a message
  virtual bool Dispatch(int dispatchIndex, void const* message) = 0;

  virtual ~GsBusAbstractListener() {}
};

/**
 * Listener (callback) interface for gesture bus messages.
 *
 * Adding a new gesture bus message is a simple matter of defining a
 * GsBusCallback for it in this class. The rest of the plumbing will then just
 * magically work. NB: gesture bus messages must be trivially copyable since
 * they are sent as raw buffers between processes.
 *
 * The maximum number of message types is defined by GS_BUS_MESSAGE_MAX, which
 * can be increased arbitrarily.
 *
 * Backwards compatibility to old clients is maintained as long as existing
 * messages are not changed and new messages are only added to the end of the
 * list.
 *
 * Each callback is assigned a dispatch table index based on its declaration
 * order (and thus construction order) in this class.
 */
class GsBusListener :
  private virtual GsBusCallbackBase,
  public GsBusAbstractListener,
  public GsBusCallback <GsBusShutdown>,
  public GsBusCallback <GsBusConnect>,
  public GsBusCallback <GsBusDisconnect>,
  public GsBusCallback <GsCapability>,
  public GsBusCallback <GsConfigurationParams>,
  public GsBusCallback <GsInternalParams>,
  public GsBusCallback <GsOutcome>,
  public GsBusCallback <GsOutcomeEnd>,
  public GsBusCallback <GsCursorEvent>,
  public GsBusCallback <GsLogMessage>,
  public GsBusCallback <GsMessageIgnore>,
  public GsBusCallback <GsCameraState>
{
public:

  // Dispatch a gesture bus message, return true if handled
  virtual bool Dispatch(int dispatchIndex, void const* message)
  {
    GsBusDispatch dispatch = GetDispatch(dispatchIndex);
    if(dispatch == NULL)
    {
      // unknown message
      return false;
    }

    // dispatch the message
    return (this->*dispatch)(message);
  }

  virtual ~GsBusListener() {}
};

/**
 * Abstract client for gesture bus messages. The implementation is dependent on
 * the OS environment.
 *
 * Each client must declare its capabilities with one or more GsCapability
 * messages so that the other clients, in particular the application, know what
 * gesture capabilities are present. To ensure that all clients are fully
 * informed of the capabilites of each client, GsCapability messages must be
 * sent when connecting to the bus and upon receiving a GsBusConnect or
 * GsBusDisconnect message from any other client. That way, a client wishing to
 * aggregate all of the gesture capabilities simply clears its list upon seeing
 * a GsBusDisconnect and accumulates the capabilities as received.
 */
class GsBusAbstractClient
{
public:

  /**
   * Connect or reconnect to the gesture bus.
   *
   * @param sourceName The user friendly name of the gesture source.
   * @param listener The callback interface
   * @return bool True if a successful connect.
   *
   * The implementation must post a connect message to the bus.
   */
  virtual bool Connect(char const* sourceName, GsBusAbstractListener& listener) = 0;

  /**
   * Disconnect from the gesture bus. This safe to call if already disconnected.
   *
   * The implementation must post a disconnect message to the bus.
   */
  virtual void Disconnect() = 0;

  /**
   * Post a message to the gesture bus. Although this allows the posting
   * of any message, please do not post system messages such as shutdown and
   * disconnect as that will disrupt the gesture bus operations.
   *
   * Note that callbacks are not made for posted messages; callbacks occur only
   * for messages sent by other clients.
   */
  template <typename T> bool PostMessage(T const& message)
  {
    // keep the dispatch index for later invocations
    static int const dispatchIndex = GsBusListener().GsBusCallback<T>::GetDispatchIndex();
    return SendMessage(dispatchIndex, &message, sizeof message);
  }

  virtual ~GsBusAbstractClient() {}

private:

  // Send a raw message on the gesture bus
  virtual bool SendMessage(
    int dispatchIndex,
    void const* message,
    size_t size) = 0;
};


#endif // INC_QTI_GESTURE_GESTURE_DEFS_H
