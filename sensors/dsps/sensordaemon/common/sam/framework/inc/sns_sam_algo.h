#ifndef SNS_SAM_ALGO_H
#define SNS_SAM_ALGO_H

/*============================================================================
  @file sns_sam_algo.h

  Sensors Algorithm Manager header

  This header file contains types for the Sensors Algorithm Manager algorithms

  Copyright (c) 2011-2013 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
============================================================================*/

#include "sns_em.h"
#include "fixed_point.h"

#define SNS_SAM_MAX_ALGO_DEPS 3

#ifdef SNS_PCSIM
#include <stdio.h>
#define SNS_PRINTF_STRING_ERROR_0(level, msg) printf(msg)
#define SNS_PRINTF_STRING_ERROR_1(level, msg, p1) printf(msg, p1)
#define SNS_PRINTF_STRING_ERROR_2(level, msg, p1, p2) printf(msg, p1, p2)
#define SNS_PRINTF_STRING_ERROR_3(level, msg, p1, p2, p3) printf(msg, p1, p2, p3)

#define SNS_PRINTF_STRING_FATAL_1(level, msg, p1) printf(msg, p1)
#define SNS_PRINTF_STRING_FATAL_2(level, msg, p1, p2) printf(msg, p1, p2)
#define SNS_PRINTF_STRING_HIGH_1(level, msg, p1) printf(msg, p1)

#define SNS_PRINTF_STRING_LOW_0(level, msg) printf(msg)
#define SNS_PRINTF_STRING_LOW_1(level, msg, p1) printf(msg, p1)
#define SNS_PRINTF_STRING_LOW_2(level, msg, p1, p2) printf(msg, p1, p2)
#define SNS_PRINTF_STRING_LOW_3(level, msg, p1, p2, p3) printf(msg, p1, p2, p3)
#endif

//generic structure for algorithm memory management
//(config, state, input)
typedef struct {
   uint32_t memSize;
   void *memPtr;
} sns_sam_algo_mem_s;

//registry item type
typedef enum
{
  SNS_SAM_REG_ITEM_TYPE_NONE,
  SNS_SAM_REG_ITEM_TYPE_SINGLE,
  SNS_SAM_REG_ITEM_TYPE_GROUP
} sns_sam_reg_item_type_e;

//Algorithm API
typedef struct {
   //Algorithm state memory requirement query API
   /*=========================================================================
     FUNCTION:  sns_sam_algo_mem_req
     =======================================================================*/
   /*!
       @brief Query interface to get memory requirement of algorithm state
       based on specified configuration

       @param[i] configPtr: Pointer to algorithm configuration

       @return Size of memory required for algorithm state
   */
   /*=======================================================================*/
   int32_t (*sns_sam_algo_mem_req)(void *configPtr);

   /*=========================================================================
     FUNCTION:  sns_sam_algo_reset
     =======================================================================*/
   /*!
       @brief Reset/Initialize the state of the algorithm instance

       @param[i] configPtr: Pointer to algorithm configuration
       @param[i] statePtr: Pointer to algorithm state

       @return Pointer to algorithm state if successful
       NULL if error
   */
   /*=======================================================================*/
   void* (*sns_sam_algo_reset)(
      void *configPtr,
      void *statePtr);

   /*=========================================================================
     FUNCTION:  sns_sam_algo_update
     =======================================================================*/
   /*!
       @brief Execute the algorithm to generate output using specified input

       @param[i] statePtr: Pointer to algorithm state
       @param[i] inputPtr: Pointer to input data
       @param[o] outputPtr: Pointer to output data

       @return Error code
   */
   /*=======================================================================*/
   void (*sns_sam_algo_update)(
      void *statePtr,
      void *inputPtr,
      void *outputPtr);

   /*=========================================================================
     FUNCTION:  sns_sam_algo_register_client
     =======================================================================*/
   /*!
       @brief Register/Deregister client with algorithm

       @param[i] client_id: Client Id
       @param[i] action: true to register client, false to deregister client
       @param[i] statePtr: Pointer to state data
       @param[i] outputPtr: Pointer to output data
       @param[i] timestamp: time of client registration

       @return true if successful
   */
   /*=======================================================================*/
   bool (*sns_sam_algo_register_client)(
      uint8_t client_id,
      bool action,
      void *statePtr,
      void *outputPtr,
      uint32_t timestamp);

   /*=========================================================================
     FUNCTION:  sns_sam_algo_reset_client_stats
     =======================================================================*/
   /*!
       @brief Reset all statistics for the specified client

       @param[i] client_id: Client Id
       @param[i] statePtr: Pointer to state data
       @param[i] outputPtr: Pointer to output data

       @return true if successful
   */
   /*=======================================================================*/
   bool (*sns_sam_algo_reset_client_stats)(
      uint8_t client_id,
      void *statePtr,
      void *outputPtr);

   /*=========================================================================
     FUNCTION:  sns_sam_algo_handle_duty_cycle_state_change
     =======================================================================*/
   /*!
       @brief Notify algorithm about duty cycle state change

       @param[i] state: Duty cycle state: 0 - off, 1 - on
       @param[i] statePtr: Pointer to state data
       @param[i] outputPtr: Pointer to output data
       @param[i] timestamp: Time of state change

       @return none
   */
   /*=======================================================================*/
   void (*sns_sam_algo_handle_duty_cycle_state_change)(
      bool state,
      void *statePtr,
      void *outputPtr,
      uint32_t timestamp);

} sns_sam_algo_api_s;

//algorithm specific information
typedef struct {
   sns_sam_algo_mem_s defConfigData;
   uint32_t defInputDataSize;
   uint32_t defOutputDataSize;
   sns_sam_algo_api_s algoApi;
   sns_sam_reg_item_type_e regItemType;
   uint32_t regItemId;
   uint8_t serviceId;
   uint8_t algoDepCount;
   uint8_t algoDepDbase[SNS_SAM_MAX_ALGO_DEPS]; /*algo dependency service ids*/
   q16_t defSensorReportRate;
   bool dataSyncRequired; /*algorithm requires sync data for dependencies*/
   uint32_t algorithm_revision; /* Revision number of the algorithm */
   uint32_t supported_reporting_modes; /* Bitmask of all reporting modes supported by algorithm. See sns_sam_report_e for reporting options */
   int32_t min_report_rate; /* Minimum report rate supported by algorithm (in Hz, Q16) */
   int32_t max_report_rate; /* Maximum report rate supported by algorithm (in Hz, Q16) */
   int32_t min_sample_rate; /* Minimum sample rate supported by algorithm (in Hz, Q16) */
   int32_t max_sample_rate; /* Maximum sample rate supported by algorithm (in Hz, Q16) */
   uint32_t max_batch_size; /* The maximum batch size (in reports) supported by this service. Returns 0, if batching is not supported */
   int32_t power; /* Power estimate of algorithm (in mA, Q16) */
} sns_sam_algo_s;

// 16 byte Sensor UUID
typedef struct {
  uint8_t byte[16];
} sns_sam_sensor_uuid_s;

#endif /* SNS_SAM_H */
