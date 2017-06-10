#ifndef SNS_SAM_H
#define SNS_SAM_H

/*============================================================================
  @file sns_sam.h

  @brief
  Data structures and constants used by the SAM Framework.  These APIs should
  only be used internally to the framework, and not by the algorithms or
  external modules.

  Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ===========================================================================*/

/*===========================================================================
  Include Files
  ===========================================================================*/
#include <stdint.h>
#include "sns_common.h"
#include "qmi_idl_lib.h"
#include "sns_debug_str.h"
#include "fixed_point.h"
#include "sns_queue.h"
#include "sns_usmr.h"
#include "sns_em.h"
#include "sns_common_v01.h"
#include "sns_sam_algo_api.h"
#include "sns_sam_pm.h"

/*============================================================================
  Preprocessor Definitions and Constants
  ===========================================================================*/

/**
 * All algorithm services must support the following messages with specified
 * message ids.  Algorithms are only permitted to have custom versions of the
 * enable req, get report resp, report ind, and batch ind.  All other messages
 * must use the "official" structure.
 */
#define SNS_SAM_ALGO_CANCEL_REQ 0x00
#define SNS_SAM_ALGO_CANCEL_RESP 0x00
#define SNS_SAM_ALGO_VERSION_REQ 0x01
#define SNS_SAM_ALGO_VERSION_RESP 0x01
#define SNS_SAM_ALGO_ENABLE_REQ 0x02
#define SNS_SAM_ALGO_ENABLE_RESP 0x02
#define SNS_SAM_ALGO_DISABLE_REQ 0x03
#define SNS_SAM_ALGO_DISABLE_RESP 0x03
#define SNS_SAM_ALGO_GET_REPORT_REQ 0x04
#define SNS_SAM_ALGO_GET_REPORT_RESP 0x04
#define SNS_SAM_ALGO_REPORT_IND 0x05
#define SNS_SAM_ALGO_ERROR_IND 0x06
#define SNS_SAM_ALGO_UPDATE_REQ 0x20
#define SNS_SAM_ALGO_UPDATE_RESP 0x20
#define SNS_SAM_ALGO_BATCH_REQ 0x21
#define SNS_SAM_ALGO_BATCH_RESP 0x21
#define SNS_SAM_ALGO_BATCH_IND 0x22
#define SNS_SAM_ALGO_UPDATE_BATCH_PERIOD_REQ 0x23
#define SNS_SAM_ALGO_UPDATE_BATCH_PERIOD_RESP 0x23
#define SNS_SAM_ALGO_GET_ATTRIB_REQ 0x24
#define SNS_SAM_ALGO_GET_ATTRIB_RESP 0x24

/* Each signal should use a unique bit */
#define SNS_SAM_SVC_RESUME_SIG 0x01
#define SNS_SAM_REPORT_TIMER_SIG 0x02
#define SNS_SAM_REG_INIT_TIMER_SIG 0x04
#define SNS_SAM_CLIENT_INIT_SIG 0x08
#define SNS_SAM_CLIENT_ERROR_SIG 0x10
#define SNS_SAM_DC_TIMER_SIG 0x20
#define SNS_SAM_BATCH_TIMER_SIG 0x40
#define SNS_SAM_REQ_MSG_SIG 0x80
#define SNS_SAM_RESP_MSG_SIG 0x100
#define SNS_SAM_IND_MSG_SIG 0x200
#define SNS_SAM_PM_SIG 0x400
#define SNS_SAM_ALGO_TIMER_SIG 0x800
#define SNS_SAM_SIGNAL_MASK (SNS_SAM_REQ_MSG_SIG | SNS_SAM_RESP_MSG_SIG |   \
        SNS_SAM_IND_MSG_SIG | SNS_SAM_REPORT_TIMER_SIG |                    \
        SNS_SAM_REG_INIT_TIMER_SIG | SNS_SAM_CLIENT_INIT_SIG |              \
        SNS_SAM_CLIENT_ERROR_SIG | SNS_SAM_DC_TIMER_SIG |                   \
        SNS_SAM_BATCH_TIMER_SIG | SNS_SAM_SVC_RESUME_SIG | SNS_SAM_PM_SIG | \
        SNS_SAM_ALGO_TIMER_SIG )

#if defined(SNS_DSPS_BUILD) || (defined(SNS_PCSIM) && !defined(ENABLE_APPS_PLAYBACK))
# define SNS_SAM_MODULE              SNS_MODULE_DSPS_SAM
# define SNS_SAM_MODULE_PRIORITY     SNS_MODULE_PRI_DSPS_SAM
# define SNS_SAM_DBG_MOD             SNS_DBG_MOD_DSPS_SAM
# define SNS_SAM_MODULE_STK_SIZE     SNS_MODULE_STK_SIZE_DSPS_SAM
# define SNS_SAM_PROCESSOR_TYPE      SNS_PROC_SSC_V01
# define SNS_SAM_SMR_SVC_PRI         SNS_SMR_SVC_PRI_MED
#else
# define SNS_SAM_MODULE              SNS_MODULE_APPS_SAM
# define SNS_SAM_MODULE_PRIORITY     SNS_MODULE_PRI_APPS_SAM
# define SNS_SAM_MR_MODULE_PRIORITY  SNS_MODULE_PRI_APPS_SAM_MR
# define SNS_SAM_DBG_MOD             SNS_DBG_MOD_APPS_SAM
# define SNS_SAM_MODULE_STK_SIZE     SNS_MODULE_STK_SIZE_APPS_SAM
# define SNS_SAM_PROCESSOR_TYPE      SNS_PROC_APPS_V01
# define SNS_SAM_SMR_SVC_PRI         SNS_SMR_SVC_PRI_LOW
#endif

/*============================================================================
  External Objects
  ===========================================================================*/

extern sns_proc_type_e_v01 localProcessor;

/* SAM module ID to be used for debug string messages */
extern sns_debug_module_id_e samModule;

/* Contains all active algorithm instances */
extern sns_q_s algoInstQ;
/* Contains all local and remote algorithms and other sensors (i.e. SMGR).
 * All local SAM algorithms will be located at the beginning of this queue. */
extern sns_q_s sensorQ;
/* Report Timers that are active*/
extern sns_q_s samReportTimersQ;

/* SAM event signal */
extern OS_FLAG_GRP *sns_sam_sig_event;

/*============================================================================
  Type Declarations
  ===========================================================================*/

/*
* SAM classifies sensors as streaming sensors and as event sensor. A streaming
* sensor can be combined with an event sensor to create an event gated sensor.
*
* An event gated sensor is created by bitwise XOR'ing the SUID of a streaming
* sensor and an event sensor.
*/
typedef enum {
  /* All physical sensors, eg: Accel, Gyro*/
  SNS_SAM_SENSOR_STREAM,
  /* Sensor events, eg: MD interrupt */
  SNS_SAM_SENSOR_EVENT,
  /* Event gated sensor stream. Eg: MD interrupt gated accel */
  SNS_SAM_SENSOR_GATED_STREAM
} sns_sam_gated_sensor_stream_type;

/**
 * Refers to a request for a specific sensor type.
 */
struct sns_sam_sensor_req
{
  /* Sensor type associated with this request */
  struct sns_sam_sensor *sensor;
  /* An enable request message for this sensor type */
  sns_sam_enable_req const *enableReq;
  /* SMR Client handle used to send/receive messages for this stream */
  smr_client_hndl clientHndl;
  /* SAM instance ID; SMGR report ID will always be 0 */
  uint8_t instanceID;
  /* Batch period; only applies to dependent SAM sensors */
  uint32_t batchPeriod;
};
typedef struct sns_sam_sensor_req sns_sam_sensor_req;

/**
 * General information for each sensor, whether it be a SAM algorithm or SMGR
 * physical sensor.  Information is populated and used by the SAM framework.
 * An instance of this object will be created for all known SAM algorithms,
 * regardless of service location.
 *
 * A global list of known sensors will be available for look-up.
 */
struct sns_sam_sensor
{
  /* Data field necessary to add this object to a SAM list */
  sns_q_link_s qLink;

  /* Sensor type; global unique IDs for all SAM algorithms and SMGR sensors.
   * This field is the definitive location for this ID; all other users,
   * including algorithms, must use its address. */
  sns_sam_sensor_uid sensorUID;
  /* Which processor does this sensor come from (or unknown) */
  sns_proc_type_e_v01 sensorLocation;
  /* Sensor specific attributes provided by the algorithm or SMGR */
  sns_sam_algo_attr attributes[ SAM_ALGO_ATTR_CNT ];
  /* QMI service object corresponding to the service */
  qmi_idl_service_object_type serviceObj;
  /* Sensor request only used for SMR handle during Framework initialization */
  sns_sam_sensor_req sensorReq;
  /* Whether this sensor is currently available via SMR */
  bool isAvailable;
  /* Whether this is a local algorithm serviced by SAM;
   *i.e. is this of type sns_sam_sensor_algo  */
  bool isLocal;
  /* Gating type, if any, of this sensor  */
  sns_sam_gated_sensor_stream_type gatingType;
};
typedef struct sns_sam_sensor sns_sam_sensor;

/**
 * Refers to a SAM algorithm that exists in this instance of the SAM
 * Framework.  Conceptually, this is a subclass of sns_sam_sensor.
 */
struct sns_sam_sensor_algo
{
  sns_sam_sensor sensor;

  /* Algorithm API, implemented per algorithm */
  struct sns_sam_algo_api *algoAPI;
  /* Algorithm-provided API which translates custom request/etc. messages */
  struct sns_sam_algo_msg_api *algoMsgAPI;
  /* Outstanding registry groups requested from the algorithm; or -1 */
  sns_sam_reg_group registryGroups[ SNS_SAM_MAX_REG_GRP ];
  /* Sensors that *must* exist for this algorithm to operate */
  struct sns_sam_sensor *dependencies[ SNS_SAM_MAX_DEPENDENT_SENSOR ];
  /* Size of the various static buffers used by the algorithm API */
  sns_sam_algo_const_buf_size bufferSizes;
  /* Persistant data across algorithm instances (i.e. registry data);
   * no concurrency protection */
  sns_sam_algo_persist persistData;
  /* SMR Service handle for this local SAM algorithm */
  smr_service_hndl serviceProvider;
  /* Indication message size */
  uint32_t qmiIndSize;
  /* Batch Indication message size */
  uint32_t qmiBatchIndSize;
};
typedef struct sns_sam_sensor_algo sns_sam_sensor_algo;

/**
* Preallocated input and output data memory for algorithm instances.  This data
* will only be allocated if the algorithm instance is intended to be run in
* uImage.  The input/output data itself is not directly accessible, but only
* indirectly through the input/output arrays below.
*
* @note preallocInput shall point to the beginning of the pre-allocated buffer,
* and will be the only pointer freed.
*/
struct sns_algo_inst_io
{
  /* Array of SAM input objects.  The associated memory shall not be freed
   * until the algorithm instance is deleted.  Each item shall be considered
   * "in use" if it is presently on a queue.  Length of array will always be
   * SNS_SAM_ALGO_MAX_IO. */
  sns_sam_algo_input *preallocInput;
  /* Same idea as preallocInput, except contains output data */
  sns_sam_algo_output *preallocOutput;
};

typedef struct sns_algo_inst_io sns_algo_inst_io;
/**
 * A valid instance of a SAM algorithm.  May be shared amongst client requests.
 */
struct sns_sam_algo_inst
{
  /* Data field necessary to add this object to a SAM list */
  sns_q_link_s qLink;

  /* Corresponding algorithm */
  struct sns_sam_sensor_algo *algorithm;
  /* If all pre-allocated IO and state memory is located in TCM */
  bool uImageReady;
  /* BIGIMAGE - Preallocated IO, state, or code is located in DDR
   * NOMEM_UIMAGE - At least one item in the input/output buffer is in DDR
   * UIMAGE - All memory is in TCM; algorithm may safely run in uImage */
  sns_sam_algoinst_uimage_mode imageMode;
  /* Queue of associated sam_client_req's */
  sns_q_s clientRequests;

  /* Duty Cycle state of this algorithm instance; true = active */
  bool dcState;
  /* Timer indicating that the dcState should change */
  sns_em_timer_obj_t dcTimer;

  /* Algorithm-specific timer set via sns_sam_algo_timer_reg() */
  sns_em_timer_obj_t algoTimer;
  /* Callback function associated with algoTimer */
  sns_sam_algo_timer_cb algoTimerCB;

  /* Size of the various buffers used by the algorithm API */
  sns_sam_algo_buf_size bufferSizes;
  /* Opaque handle used by algorithms */
  sns_sam_algo_state algoStateData;
  /* List of sns_sam_algo_input objects */
  sns_sam_input_q algoInputQ;
  /* Pointer to last input added to algoInputQ. Reset to end of the queue when a new Indication is recieved*/
  sns_q_link_s * lastInputPtr;
  /* List of sns_sam_algo_output objects. */
  sns_q_s algoOutputQ;
  /* Preallocated input and output buffers for algorithms */
  sns_algo_inst_io algoPreallocBuf;
  /* Callback functions for this algorithm instance */
  sns_sam_algo_callback cbFuncs;
  /* Sensor streams requested by the algorithm */
  struct sns_sam_sensor_req *sensorRequests[ SNS_SAM_MAX_DEPENDENT_SENSOR ];
};
typedef struct sns_sam_algo_inst sns_sam_algo_inst;

/**
 * A request for a SAM service from some client.  The client may be external,
 * or internal to the SSC or from another SAM service.
 */
struct sam_client_req
{
  /* Data field necessary to add this object to a SAM list */
  sns_q_link_s qLink;

  /* Algorithm instance created or used to service this client */
  struct sns_sam_algo_inst *algoInstance;
  /* Instance ID returned to the sensors client to refer to this stream. */
  uint32_t extInstanceID;
  /* SMR handle to be used to send reports to the associated client */
  smr_qmi_client_handle serviceHndl;
  /* If the client is busy; whether to send indications at the moment */
  bool clientBusy;
  /* Corresponding algorithm */
  struct sns_sam_sensor_algo const *algorithm;
  /* A client event has occurred, either:
     New report needs to be generated; sent to the client if not batched;
     New batched indication needs to be generated and sent to the client */
  sns_em_timer_obj_t clientTimer;
  /* Maximum batching period requested; multiple of the report period */
  q16_t batchPeriod;
  /* Active batching period requested; multiple of the report period */
  q16_t batchPeriodActive;
  /* The timestamp at which the next batched indication should be sent */
  uint64_t nextBatchTS;
  /* Whether SAM should flush its buffer when it becomes full */
  bool wuffEnabled;
  /* Attributes describing the client's request */
  sns_sam_client_attr clientAttr;
  /* Queue of output data that will be sent to the client during the next
   * report or batching period. Pointers into sns_sam_algo_inst::algoOutput. */
  sns_q_s outputDataQ;
};
typedef struct sam_client_req sam_client_req;

/* For incoming request messages to the SAM Framework */
struct sns_sam_req_msg
{
  /* The actual message and its meta data */
  struct sns_sam_msg msg;
  /* The SMR request handle associated with the request message */
  smr_req_handle reqHndl;
  /* SMR handle to the client connection that initiated this request */
  smr_qmi_client_handle serviceHndl;
  /* Memory allocated by sns_sam_service to hold the response message */
  struct sns_sam_msg respMsg;
};
typedef struct sns_sam_req_msg sns_sam_req_msg;

/* For incoming response and indication messages to the SAM Framework */
struct sns_sam_msg_int
{
  /* The actual message and its meta data */
  struct sns_sam_msg msg;
  /* The sensor request associated with this message */
  sns_sam_sensor_req const *sensorReq;
};
typedef struct sns_sam_msg_int sns_sam_resp;
typedef struct sns_sam_msg_int sns_sam_ind;

/*
 * Used to store lists of active report timers to guarentee that timers will not be used after
 * they are cancelled.
 */
struct sns_sam_algo_report_timers
{
  sns_q_link_s qLink;
  sam_client_req *clientReq;
};
typedef struct sns_sam_algo_report_timers sns_sam_algo_report_timers;

/*============================================================================
  Internal Function Declarations
  ===========================================================================*/

/**
 * Looks-up a local SAM algorithm from a sensor UID.
 *
 * @param[i] sensorUID Sensor to look-up
 *
 * @return Algorithm pointer, or NULL if not found.
 */
sns_sam_sensor_algo* sns_sam_lookup_algo( sns_sam_sensor_uid const *sensorUID );

/**
 * Lookup a sensor based on the SensorUID from the "sensorQ".
 * The sensor may be a local or remote SAM algorithm, SMGR sensor, etc..
 *
 * @param[i] sensorUID Sensor to look-up
 *
 * @return Pointer to sensor object if found; NULL otherwise
 */
sns_sam_sensor* sns_sam_lookup_sensor_from_suid( sns_sam_sensor_uid const *sensorUID );

/**
 * Allocate memory for a new sensor object.
 *
 * @param[o] sensor Sensor object pointer
 *
 * @return SAM_ENONE
 *         SAM_ENOMEM Not enough memory to allocate sns_sam_sensor
 */
sns_sam_err sns_sam_init_sensor( sns_sam_sensor **sensor );

/**
 * Lookup the maximum enable request message size for an algorithm's
 * dependencies.
 *
 * @param[i] sensorUID Sensor that we wish to query
 *
 * @return Size of the QMI c-struct in bytes
 */
uint32_t sns_sam_enable_req_size( sns_sam_sensor_uid const *sensorUID );

#endif /* SNS_SAM_H */
