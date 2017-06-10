#ifndef SNS_STRING_DB_H
#define SNS_STRING_DB_H

/*============================================================================
@file
sns_string_db.h

@brief
Debug string database for the sensors module.

Copyright (c) 2010-2013 Qualcomm Technologies, Inc. All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential
============================================================================*/

/*=====================================================================
                 INCLUDE FILES
=======================================================================*/
#include "sns_debug_api.h"

/*=====================================================================
                 GLOBAL STRING ARRAY FOR DIAG
=======================================================================*/
/* Important Note on adding strings to the array:
 * When adding a string make sure the string is added as the
 * "n"th element of the array where "n" is the value of the ID */
#define DIAG_STR_DB \
        {  {SNS_DEBUG_DIAG_TEST_STRING0_ID,  "Test String, no params"           }, /*0*/ \
           {SNS_DEBUG_DIAG_TEST_STRING1_ID,  "Test String, 1 param = %d"        }, /*1*/ \
           {SNS_DEBUG_DIAG_TEST_STRING2_ID,  "Test String, 2 param = %d, %d"    }, /*2*/ \
           {SNS_DEBUG_DIAG_TEST_STRING3_ID,  "Test String, 3 param = %d, %d, %d"}  /*3*/ \
        }

/*=====================================================================
                 GLOBAL STRING ARRAY FOR SENSOR MANAGER (DSPS)
=======================================================================*/
/* Important Note on adding strings to the array:
 * When adding a string make sure the string is added as the
 * "n"th element of the array where "n" is the value of the ID */
#define SMGR_STR_DB \
 { {DBG_SMGR_GENERIC_STRING0, "SMGR: Generic String"                         },  /*Id 0*/ \
   {DBG_SMGR_GENERIC_STRING1, "SMGR: One val: %d"                            },  /*Id 1*/ \
   {DBG_SMGR_GENERIC_STRING2, "SMGR: One val: %d, second val: %d"            },  /*Id 2*/ \
   {DBG_SMGR_GENERIC_STRING3, "SMGR: One val: %d, sec val: %d, third val: %d"},  /*Id 3*/ \
   {DBG_SMGR_MESSAGE_FLAG, "SMGR: Message Flag: %d"},                            /*Id 4*/ \
   {DBG_SMGR_DD_INIT_FLAG, "SMGR: DD Init Flag: %d"},                            /*Id 5*/ \
   {DBG_SMGR_DD_SERVICE_FLAG, "SMGR: DD Service Flag: %d"},                      /*Id 6*/ \
   {DBG_SMGR_SCHED_FLAG, "SMGR: DD Sched Flag: %d"},                             /*Id 7*/ \
   {DBG_SMGR_DATA_CYCLE_FLAG, "SMGR: Data Cycle Flag: %d"},                      /*Id 8*/ \
   {DBG_SMGR_PLAN_CYCLE_FLAG, "SMGR: Plan Cycle Flag: %d"},                      /*Id 9*/ \
   {DBG_SMGR_SENSOR_ID, "SMGR: Sensor ID: %d"},                                  /*Id 10*/ \
   {DBG_SMGR_SENSOR_ACTION, "SMGR: Sensor ID %d, Action %d"},                    /*Id 11*/ \
   {DBG_SMGR_SENSOR_STATE, "SMGR: Sensor ID %d, State %d"},                      /*Id 12*/ \
   {DBG_SMGR_SENSOR_READ_STATE, "SMGR: Sensor ID %d, Read State %d"},            /*Id 13*/ \
   {DBG_SMGR_SENSOR_DATA_STATE, "SMGR: Sensor ID %d, Data State %d"},            /*Id 14*/ \
   {DBG_SMGR_SENSOR_INIT_STATE, "SMGR: Sensor ID %d, Init State %d"},            /*Id 15*/ \
   {DBG_SMGR_SENSOR_POWER_STATE, "SMGR: Sensor ID %d, Power State %d"},          /*Id 16*/ \
   {DBG_SMGR_SENSOR_ALL_INIT_STATE, "SMGR: Sensor All Init State: %d"},          /*Id 17*/ \
   {DBG_SMGR_HEADER_ABSTRACT, "SMGR: Header: src_module %d, client_id %d, priority %d"},  /*Id 18*/ \
   {DBG_SMGR_RCVD_REQ, "SMGR: Received request message id %d"},                  /*Id 19*/ \
   {DBG_SMGR_RESPONSE, "SMGR: Response message id %d"},                          /*Id 20*/ \
   {DBG_SMGR_ERR_RESP, "SMGR: Error response message id %d"},                    /*Id 21*/ \
   {DBG_SMGR_REPORT_IND, "SMGR: Report indication: ReportId %d, status %d"},     /*Id 22*/ \
   {DBG_SMGR_CURR_TIMESTAMP, "SMGR: Current Timestamp: %u"},                     /*Id 23*/ \
   {DBG_SMGR_REGISTER_TIMER, "SMGR: Register Timer expire at %u"},               /*Id 24*/ \
   {DBG_SMGR_LAST_TICK, "SMGR: Last Tick: %u"},                                  /*Id 25*/ \
   {DBG_SMGR_CURR_INTERVAL, "SMGR: Current Interval: %u"},                       /*Id 26*/ \
   {DBG_SMGR_NEXT_HB, "SMGR: Next Heartbeat: %u"},                               /*Id 27*/ \
   {DBG_SMGR_DATACYCLE_VAR, "SMGR: DataCycleFlag_g: %d, DataCycleStart_g: %u"},  /*Id 28*/ \
   {DBG_SMGR_TUNING, "SMGR: Device %d, Variant %d, Ticks %u"},                   /*Id 29*/ \
   {DBG_SMGR_ASYNC_DATA_STATUS, "SMGR: Async Data: Sensor %d, Status %d, Timestamp %u"},  /*Id 30*/ \
   {DBG_SMGR_ASYNC_DATA_VALUE, "SMGR: Async Data: X = 0x%x, Y = 0x%x, Z = 0x%x"},         /*Id 31*/ \
   {DBG_SMGR_HIGH_WATER_MARK,  "Size of block : %d, High Watermark : %d"},                 /*Id 32*/ \
   {DBG_SMGR_BLOCK_SUMMARY,  "    Allocated Block (%d/%d)"},                              /*Id 33*/ \
   {DBG_SMGR_BLOCK_DETAIL1,  "Block num : %d, Size : %d allocated on Ts = %d"},              /*Id 34*/ \
   {DBG_SMGR_BLOCK_DETAIL2,  "    Allocated by %d, owned by %d, req size %d"},                /*Id 35*/ \
   {DBG_SMGR_LINE_DIVIDER,  "--------------------------------"}                           /*Id 36*/ \
}

/*=====================================================================
               GLOBAL STRING ARRAY FOR ALS PRX DEVICE DRIVER (DSPS)
=======================================================================*/
/* Important Note on adding strings to the array:
 * When adding a string make sure the string is added as the
 * "n"th element of the array where "n" is the value of the ID */
#define DD_ALPRX_STR_DB \
{ {DBG_DD_ALSPRX_WR_CMD2_REG_ERR, "ALSPRX: Write to command2 register failed"  },  /*Id 0*/ \
  {DBG_DD_ALSPRX_WR_CMD1_REG_ERR, "ALSPRX: Write to command1 register failed"  },  /*Id 1*/ \
  {DBG_DD_ALSPRX_DEV_STATE_PEND, "ALSPRX: PRX/ALS read already pending"        },  /*Id 2*/ \
  {DBG_DD_ALSPRX_PRX_STATE_PEND, "ALSPRX: PRX read already pending"            },  /*Id 3*/ \
  {DBG_DD_ALSPRX_ALS_RES_INVALID, "ALSPRX: Invalid resolution: %d"             },  /*Id 4*/ \
  {DBG_DD_ALSPRX_ALS_RNG_INVALID, "ALSPRX: Invalid range: %d"                  },  /*Id 5*/ \
  {DBG_DD_ALSPRX_MLUX_TOO_LARGE, "ALSPRX: Milli-lux: %d is larger than max supported: %d"},  /*Id 6*/ \
  {DBG_DD_ALSPRX_ALS_RSP_READY, "ALSPRX: ALS response ready raw: %d mlux: %d range: %d"  },  /*Id 7*/ \
  {DBG_DD_ALSPRX_ALS_NXT_RNG_SEL, "ALSPRX: ALS next range selected: %d"        },  /*Id 8*/ \
  {DBG_DD_ALSPRX_ALS_DATA_READY_ERR, "ALSPRX: ALS data ready, but state is not pending"},  /*Id 9*/ \
  {DBG_DD_ALSPRX_ALS_DATA_READY_MAX_ATTEMPTS, "ALSPRX: ALS max attempts to adjust range. Simply report data"},  /*Id 10*/ \
  {DBG_DD_ALSPRX_ALS_DATA_READY_NO_RESAMPLE1, "ALSPRX: ALS data: %d mlux is less than min mlux: %d. No resampling"},  /*Id 11*/ \
  {DBG_DD_ALSPRX_ALS_DATA_READY_RESAMPLE1, "ALSPRX: ALS data: %d mlux is less than min mlux: %d. Sampling again at range: %d"},  /*Id 12*/ \
  {DBG_DD_ALSPRX_ALS_DATA_READY_NO_RESAMPLE2, "ALSPRX: ALS: data: %d mlux clipped at range 4. No need to resample"},  /*Id 13*/ \
  {DBG_DD_ALSPRX_ALS_DATA_READY_RESAMPLE2, "ALSPRX: ALS: data: %d mlux clipping at max mlux: %d. Sampling again at range: %d"},  /*Id 14*/ \
  {DBG_DD_ALSPRX_PRX_BINARY_NEAR, "ALSPRX: PRX Near by"                        },  /*Id 15*/ \
  {DBG_DD_ALSPRX_PRX_BINARY_FAR, "ALSPRX: PRX Far away"                        },  /*Id 16*/ \
  {DBG_DD_ALSPRX_PRX_NOT_PENDING, "ALSPRX: PRX state is not pending"           },  /*Id 17*/ \
  {DBG_DD_ALSPRX_PRX_RSP_READY, "ALSPRX: PRX response ready raw data: %d dist: %d"},  /*Id 18*/ \
  {DBG_DD_ALSPRX_I2C_CMD1_READ_ERR, "ALSPRX: I2C read error when reading CMD1 register"},  /*Id 19*/ \
  {DBG_DD_ALSPRX_I2C_DATA_LSB_READ_ERR, "ALSPRX: I2C read error when reading DATA LSB register"},  /*Id 20*/ \
  {DBG_DD_ALSPRX_I2C_READ_CMD1_NO_INT, "ALSPRX: Interrupt flag was not set in the command1 reg, reg:0x%x"},  /*Id 21*/ \
  {DBG_DD_ALSPRX_I2C_CMD1_READ_WRONG_DATA_TYPE, "ALSPRX: Last issued command(ALS/PRX) does not match received data(PRX/ALS"},  /*Id 22*/ \
  {DBG_DD_ALSPRX_I2C_CMD2_WR_ERR, "ALSPRX: I2C write error when requesting data from sensor"},  /*Id 23*/ \
  {DBG_DD_ALSPRX_ALS_INIT, "ALSPRX: ALS initialization in progress"            },  /*Id 24*/ \
  {DBG_DD_ALSPRX_PRX_INIT, "ALSPRX: PRX initialization in progress"            },  /*Id 25*/ \
  {DBG_DD_ALSPRX_I2C_HI_THRESH_WR_ERR, "ALSPRX: I2C write error when writing to HT_LSB register"},  /*Id 26*/ \
  {DBG_DD_ALSPRX_I2C_LO_THRESH_WR_ERR, "ALSPRX: I2C write error when writing to LT_MSB register"},  /*Id 27*/ \
  {DBG_DD_ALSPRX_CMN_INIT_ERR, "ALSPRX: Common init failed"                    },  /*Id 28*/ \
  {DBG_DD_ALSPRX_ALS_INIT_ERR, "ALSPRX: ALS init failed"                       },  /*Id 29*/ \
  {DBG_DD_ALSPRX_PRX_INIT_ERR, "ALSPRX: PRX init failed"                       },  /*Id 30*/ \
  {DBG_DD_ALSPRX_SET_PWR_ST_ERR, "ALSPRX: Failed to set state to low power"    },  /*Id 31*/ \
  {DBG_DD_ALSPRX_GET_DATA_STATE_ERR, "ALSPRX: Invalid state: %d in get data"   },  /*Id 32*/ \
  {DBG_DD_ALSPRX_GET_DATA_INVALID_SENSOR_ERR, "ALSPRX: Invalid sensor: %d in get data"},  /*Id 33*/ \
  {DBG_DD_ALSPRX_GET_DATA_REQUEST_ERR, "ALSPRX: Common prepare data failed"    },  /*Id 34*/ \
  {DBG_DD_ALSPRX_GET_ATTRIB_RES, "ALSPRX: Get Attrib returned resolution:%d"   },  /*Id 35*/ \
  {DBG_DD_ALSPRX_GET_ATTRIB_PWR, "ALSPRX: Get Attrib returned power: %d"       },  /*Id 36*/ \
  {DBG_DD_ALSPRX_GET_ATTRIB_ACC, "ALSPRX: Get Attrib returned accuracy: %d"    },  /*Id 37*/ \
  {DBG_DD_ALSPRX_GET_ATTRIB_THRESH, "ALSPRX: Get Attrib returned threshold: %d"},  /*Id 38*/ \
  {DBG_DD_ALSPRX_GET_ATTRIB_ERR, "ALSPRX: Get Attrib received invalid attribute: %d"},  /*Id 39*/ \
  {DBG_DD_ALSPRX_GET_ATTRIB_REQ, "ALSPRX: Get Attrib request for sensor: %d attrib: %d"},  /*Id 40*/ \
  {DBG_DD_ALSPRX_GET_ATTRIB_SENSOR_TYPE_ERR, "ALSPRX: Get Attrib received invalid sensor: %d"},  /*Id 41*/ \
  {DBG_DD_ALSPRX_I2C_CMD1_WR_ERR, "ALSPRX: I2C write error when writing to CMD2 register"},   /*Id 42*/ \
  {DBG_DD_ALSPRX_GET_DATA_REQ, "ALSPRX: Get Data for sensor: %d"},  /*Id 43*/ \
  {DBG_DD_ALSPRX_HANDLE_TIMER, "ALSPRX: Timer callback"},  /*Id 44*/ \
  {DBG_DD_ALSPRX_GET_ATTRIB_RES_ADC, "ALSPRX: Get attrib resolution ADC: %d"},  /*Id 45*/ \
  {DBG_DD_ALSPRX_I2C_RD_ERR, "ALSPRX: I2C error when reading register: %d error: %d"},  /*Id 46*/ \
  {DBG_DD_ALSPRX_I2C_WR_ERR, "ALSPRX: I2C error when writing register: %d error: %d"},  /*Id 47*/ \
  {DBG_DD_ALSPRX_ALS_CONVERSION1, "ALSPRX: Lux Conversion. raw data: %x als_conversion: %x data_32: %x"},  /*Id 48*/ \
  {DBG_DD_ALSPRX_ALS_CONVERSION2, "ALSPRX: Lux conversion 64-bit data: %x %x"},  /*Id 49*/ \
  {DBG_DD_ALSPRX_HANDLE_IRQ, "ALSPRX: Interrupt asserted"},  /*Id 50*/ \
  {DBG_DD_ALSPRX_WRONG_IRQ, "ALSPRX: Unexpected IRQ: %d"},  /*Id 51*/ \
  {DBG_DD_ALSPRX_IRQ_NO_INT_FLAG, "ALSPRX: Interrupt flag not set in INT reg, reg:0x%x"},  /*Id 52*/ \
  {DBG_DD_ALSPRX_NV_USE_DRIVER_DEFAULTS, "ALSPRX: Using NV defaults from driver"},  /*Id 53*/ \
  {DBG_DD_ALSPRX_NV_PARAMS, "ALSPRX: NV param: %d"},  /*Id 54*/ \
  {DBG_DD_ALSPRX_NV_DATA_FROM_SMGR, "ALSPRX: NV data from SMGR: %d"},  /*Id 55*/ \
  {DBG_DD_ALSPRX_STRING0, "ALSPRX: debug message"},  /*Id 56*/ \
  {DBG_DD_ALSPRX_STRING1, "ALSPRX: debug message param1: %d"},  /*Id 57*/ \
  {DBG_DD_ALSPRX_STRING2, "ALSPRX: debug message param1: %d param2: %d"},  /*Id 58*/ \
  {DBG_DD_ALSPRX_STRING3, "ALSPRX: debug message param1: %d param2: %d param3: %d"}  /*Id 59*/ \
}

/*=====================================================================
               GLOBAL STRING ARRAY FOR MAG8975 DEVICE DRIVER (DSPS)
=======================================================================*/
/* Important Note on adding strings to the array:
 * When adding a string make sure the string is added as the
 * "n"th element of the array where "n" is the value of the ID */
#define DD_MAG8975_STR_DB \
{ {DBG_DD_MAG8975_INITIALIZING         		, "MAG8975: Initializing" },                                            /*Id 0*/ \
  {DBG_DD_MAG8975_GET_ATTRIB_REQ       		, "MAG8975: Get Attrib request for sensor: %d attrib: %d"},             /*Id 1*/ \
  {DBG_DD_MAG8975_SET_ATTRIB_REQ       		, "MAG8975: Set Attrib request for sensor: %d attrib: %d"},             /*Id 2*/ \
  {DBG_DD_MAG8975_GET_DATA_REQ         		, "MAG8975: Get Data request X=%d, Y=%d, Z=%d"},                        /*Id 3*/ \
  {DBG_DD_MAG8975_GET_ERROR            		, "MAG8975: Get Data error: curr state=%d"},                            /*Id 4*/ \
  {DBG_DD_MAG8975_HANDLE_IRQ_REQ       		, "MAG8975: akm8975_hdle_irq called and it is not implemented"},        /*Id 5*/ \
  {DBG_DD_MAG8975_GET_POWER_INFO_REQ   		, "MAG8975: GET_POWER_INFO active_current=%d, lowpower_current=%d"},    /*Id 6*/ \
  {DBG_DD_MAG8975_GET_RANGE_REQ        		, "MAG8975: GET_RANGE MIN=%d, MAX=%d"},                                 /*Id 7*/ \
  {DBG_DD_MAG8975_GET_RESOLUTION_REQ   		, "MAG8975: GET_RESOLUTION res=%d"},                                    /*Id 8*/ \
  {DBG_DD_MAG8975_GET_DELAYS_REQ       		, "MAG8975: GET_DELAYS time_to_active=%d, time_to_data=%d"},            /*Id 9*/ \
  {DBG_DD_MAG8975_GET_DRIVER_INFO_REQ  		, "MAG8975: GET_DRIVER_INFO %s"},                                       /*Id 10*/ \
  {DBG_DD_MAG8975_GET_DEVICE_INFO_REQ  		, "MAG8975: GET_DEVICE_INFO %s"},                                       /*Id 11*/ \
  {DBG_DD_MAG8975_SET_POWER_STATE_REQ  		, "MAG8975: SET_POWER_STATE state=%d"},                                 /*Id 12*/ \
  {DBG_DD_MAG8975_SET_RESOLUTION_REQ   		, "MAG8975: SET_RESOLUTION AKM8975 only supports 1 resolution level"},  /*Id 13*/ \
  {DBG_DD_MAG8975_SET_RANGE_REQ        		, "MAG8975: SET_RANGE AKM8975 support only 1 set of range"},            /*Id 14*/ \
  {DBG_DD_MAG8975_FAILED_TO_PWR_DOWN   		, "MAG8975: Failed to set power down mode on AKM8975"},            	   	/*Id 15*/ \
  {DBG_DD_MAG8975_PWR_MODE_NOT_SUPPORTED   	, "MAG8975: Power mode requested - %d not supported"},            	   	/*Id 16*/ \
  {DBG_DD_MAG8975_SENSITIVITY_DATA   		, "MAG8975: Sensitivity Data is X=%d, Y=%d, Z=%d"},            	   		/*Id 17*/ \
  {DBG_DD_MAG8975_READ_FAILURE      		, "MAG8975: Failed to read data. Read Register=%d"},           	   		/*Id 18*/ \
  {DBG_DD_MAG8975_TEST                 		, "MAG8975: Test" }                                                    	/*Id 19*/ \
}

/*=====================================================================
               GLOBAL STRING ARRAY FOR ACCEL DEVICE DRIVER (DSPS)
=======================================================================*/
/* Important Note on adding strings to the array:
 * When adding a string make sure the string is added as the
 * "n"th element of the array where "n" is the value of the ID */
#define DD_ACCEL_STR_DB \
{ {DBG_DD_ACCEL_INIT, "Accel Initialized"                                                      }  /*Id 0*/ \
}


/*=====================================================================
               GLOBAL STRING ARRAY FOR GYRO DEVICE DRIVER (DSPS)
=======================================================================*/
/* Important Note on adding strings to the array:
 * When adding a string make sure the string is added as the
 * "n"th element of the array where "n" is the value of the ID */
#define DD_GYRO_STR_DB \
{  {DBG_DD_GYRO_INIT, "Gyro initialized"                                                        } /*Id 0*/ \
}

/*=====================================================================
                 GLOBAL STRING ARRAY FOR SAM (DSPS)
=======================================================================*/
/* IMPORTANT NOTE on adding strings to the array:
 * When adding a string make sure the string is added as the
 * "n"th element of the array where "n" is the value of the string ID */
#define SAM_STR_DB \
{ {DBG_SAM_TIMER_CB_SIGNALERR, "Failed to post signal in SAM timer callback with error code %d"},  /*Id 0*/ \
  {DBG_SAM_REG_TIMER_STARTED, "Periodic timer started for client %d, with period %d"           },  /*Id 1*/ \
  {DBG_SAM_REG_TIMER_FAILED, "SAM timer registration failed with error %d"                     },  /*Id 2*/ \
  {DBG_SAM_DEREG_TIMER_DELETED, "SAM timer for client %d deleted after report %d at time %d"   },  /*Id 3*/ \
  {DBG_SAM_DEREG_TIMER_FAILED, "Failed to deregister SAM timer with error code %d"             },  /*Id 4*/ \
  {DBG_SAM_SAM_TASK_SAMSTARTED, "SAM Started....."                                             },  /*Id 5*/ \
  {DBG_SAM_PROCESS_EVT_UNKWN_EVT, "Ignoring unknown events signaled 0x%x"                      },  /*Id 6*/ \
  {DBG_SAM_PROCESS_MSG_RCVD_MSG, "SAM received message id %d, from module %d for service %d"   },  /*Id 7*/ \
  {DBG_SAM_PROCESS_MSG_STATUS, "SAM processed message type %d with error code %d"              },  /*Id 8*/ \
  {DBG_SAM_HNDL_REPORT_STATUS, "Handled report timeout for client %d, period %d at time %d"    },  /*Id 9*/ \
  {DBG_SAM_PROCESS_REQ_DISABLE_ERR, "ERROR: Algo inst in disable msg %d differs from dbase %d" },  /*Id 10*/ \
  {DBG_SAM_PROCESS_REQ_INVALID_ALGOREQ, "Unsupported algorithm service id %d"                  },  /*Id 11*/ \
  {DBG_SAM_PROCESS_REQ_INVALID_REQ, "Unsupported client request type message id %d"            },  /*Id 12*/ \
  {DBG_SAM_SMGR_RESP_DROPPED, "Dropping manager response for invalid algo instance id %d"      },  /*Id 13*/ \
  {DBG_SAM_SMGR_RESP_SUCCESS, "Rcvd successful response from sensors manager for ReportID %d"  },  /*Id 14*/ \
  {DBG_SAM_SMGR_RESP_ACK_VAL, "Rcvd resp from sensor mgr with Ack value %d, item len %d"       },  /*Id 15*/ \
  {DBG_SAM_SMGR_RESP_RESPINFO, "Item %d, Reason %d"                                            },  /*Id 16*/ \
  {DBG_SAM_SMGR_IND_MSGID, "Dropping unhandled sensors manager message id %d"                  },  /*Id 17*/ \
  {DBG_SAM_SMGR_IND_DROPPED, "Dropping manager indication for invalid algo instance %d"        },  /*Id 18*/ \
  {DBG_SAM_SMGR_IND_STATUS, "Rcvd manager indication for algo instance %d, with status %d"     },  /*Id 19*/ \
  {DBG_SAM_SMGR_IND_INVALID, "Invalid index %d for algorithm service %d"                       },  /*Id 20*/ \
  {DBG_SAM_SMGR_IND_DELIVERY_SUCC, "Sent report to client %d"                                  },  /*Id 21*/ \
  {DBG_SAM_REQ_SNSR_DATA_MSG, "Sent SMGR request for report id %d, sensor %d at rate %d"       },  /*Id 22*/ \
  {DBG_SAM_SEND_RSP_MSG, "Sent response msg id %d to module %d for service %d"                 },  /*Id 23*/ \
  {DBG_SAM_RPT_IND_MSG, "Sent algo report to client %d for service %d with report id %d"       },  /*Id 24*/ \
  {DBG_SAM_RPT_ERRIND_MSG, "Sent error ind to client %d for service %d with report id %d"      },  /*Id 25*/ \
  {DBG_SAM_ADD_CLIENT_MAX_ERR, "Max number of client request reached"                          },  /*Id 26*/ \
  {DBG_SAM_ADD_CLIENT_INFO, "Added client request id %d for module %d, client %d"              },  /*Id 27*/ \
  {DBG_SAM_DELETE_CLIENT_INFO, "Deleted client request id %d for module %d, client %d"         },  /*Id 28*/ \
  {DBG_SAM_GET_ALGO_INDX_ERR, "Algorithm service %d not supported"                             },  /*Id 29*/ \
  {DBG_SAM_REG_ALGO_SUCCESS, "Algorithm service %d registered with SAM"                        },  /*Id 30*/ \
  {DBG_SAM_REG_ALGO_ERR, "Failed to register algorithm service %d with SAM"                    },  /*Id 31*/ \
  {DBG_SAM_REG_ALGO_DFLT_ERR, "Unsupported algorithm service id %d"                            },  /*Id 32*/ \
  {DBG_SAM_ENABLE_ALGO_STATE_NULL, "Algo reset returned with NULL state ptr"                   },  /*Id 33*/ \
  {DBG_SAM_ENABLE_ALGO, "Enabled algo service %d for client req %d with instance %d"           },  /*Id 34*/ \
  {DBG_SAM_DISABLE_ALGO_INSTANCE_ERR, "Incorrect algorithm instance id %d"                     },  /*Id 35*/ \
  {DBG_SAM_DISABLE_ALGO, "Disabled algo service %d for client req %d with instance %d"         },  /*Id 36*/ \
  {DBG_SAM_ALGO_MEM_INFO, "Algorithm service %d, instance mem %d, config mem %d"               },  /*Id 37*/ \
  {DBG_SAM_ALGO_STATE_MEM_INFO, "Algorithm state mem %d, input mem %d, output mem %d"          },  /*Id 38*/ \
  {DBG_SAM_STOP_SNSR_DATA_MSG, "Stop SMGR request for report id %d, sensor %d at rate %d"      },  /*Id 39*/ \
  {DBG_SAM_POWER_VOTE_INFO, "Send power vote %d due to request from client %d at time %d"      },  /*Id 40*/ \
  {DBG_SAM_MOTION_REG_INFO, "Send motion interrupt register/deregister %d msg at time %d"      },  /*Id 41*/ \
  {DBG_SAM_MOTION_REG_FAIL, "motion int de/register failed, error %d, motion %d, int state %d" },  /*Id 42*/ \
  {DBG_SAM_MOTION_IND_DROP, "motion indication %d dropped by SAM, motion %d, int state %d"     },  /*Id 43*/ \
  {DBG_SAM_MOTION_INT_RESP, "Received motion interrupt response in motion %d, int state %d"    },  /*Id 44*/ \
  {DBG_SAM_MOTION_INT_IND, "Received motion interrupt indication in motion %d, int state %d"   },  /*Id 45*/ \
  {DBG_SAM_MOTION_INT_OCCUR, "Received motion interrupt occurred ind, motion %d, int state %d" },  /*Id 46*/ \
  {DBG_SAM_MOTION_STATE_UPDATE, "Motion state updated from %d to %d"                           },  /*Id 47*/ \
  {DBG_SAM_MOTION_STATE_START, "Motion state detection started with data request %d"           },  /*Id 48*/ \
  {DBG_SAM_MOTION_STATE_STOP, "Motion state detection stopped motion %d, int en %d, request %d"},  /*Id 49*/ \
  {DBG_SAM_REG_REQ_COUNT, "Sent %d registry requests for algorithm configuration data"         },  /*Id 50*/ \
  {DBG_SAM_REG_REQ_SUCCESS, "Received algorithm configuration from registry successfully"      },  /*Id 51*/ \
  {DBG_SAM_REG_REQ_FAIL, "Failed to receive %d registry responses for algorithm configuration" },  /*Id 52*/ \
  {DBG_SAM_REG_RESP_ERR, "Received registry response for item %d with result %d error %d"      },  /*Id 53*/ \
  {DBG_SAM_REG_RESP_PROC_ERR, "Failed to process item %d type %d data length %d"               },  /*Id 54*/ \
  {DBG_SAM_PROC_MSG_HDR_ERR, "Failed to process message due to error %d in getting header"     },  /*Id 55*/ \
  {DBG_SAM_MSG_DROP, "SAM dropped unexpected message id %d, from module %d for service %d"     },  /*Id 56*/ \
  {DBG_SAM_MUTEX_ERR, "Error acquiring mutex %d"                                               },  /*Id 57*/ \
  {DBG_SAM_ALLOC_ERR, "Failed to allocate resources"                                           },  /*Id 58*/ \
  {DBG_SAM_SVC_HNDL_ERR, "Unable to acquire service handle %d"                                 },  /*Id 59*/ \
  {DBG_SAM_QMI_REG_ERR, "Failed to register algorithm service %d with QMI %d"                  },  /*Id 60*/ \
  {DBG_SAM_EXT_CLNT_ID_ERR, "Unable to find ext cli id %d (svc %d)"                            },  /*Id 61*/ \
  {DBG_SAM_INIT_SVC_ERR, "Unable to initialize service %d with QCCI"                           },  /*Id 62*/ \
  {DBG_SAM_QCSI_CB, "Received QCSI callback %i for service %i"                                 },  /*Id 63*/ \
  {DBG_SAM_QMI_ERR, "Received error from QMI framework call %d"                                },  /*Id 64*/ \
  {DBG_SAM_INS_ID_ERR, "Error storing instance ID %d; txn: %d"                                 },  /*Id 65*/ \
  {DBG_SAM_MOTION_INT_CLIENT_INFO, "MD interrupt info: svcID %d, dataReqId %d"                 },  /*Id 66*/ \
  {DBG_SAM_NULL_PTR, "SAM: NULL ptr detected (debug value: %d)"                                },  /*Id 67*/ \
  {DBG_SAM_REG_ALGO_PENDING, "Algorithm service %d pending registry request"                   }   /*Id 68*/ \
}

/*=====================================================================
       GLOBAL STRING ARRAY FOR SMR (Common file for Apps and DSPS)
=======================================================================*/
/* IMPORTANT NOTE on adding strings to the array:
 * When adding a string make sure the string is added as the
 * "n"th element of the array where "n" is the value of the string ID */

#define SMR_STR_DB \
{ {DBG_SMR_MSG_HDR_DETAILS1, "MSG Hdr: dst(0x%x), src(0x%x), mtype(%d)"                        },  /*Id 0*/ \
  {DBG_SMR_MSG_HDR_DETAILS2, "MSG Hdr: txn(%d), clid(%d), pri(%d)"                             },  /*Id 1*/ \
  {DBG_SMR_MSG_HDR_DETAILS3, "MSG Hdr: svc(%d), mid(%d), bo_len(%d)"                           },  /*Id 2*/ \
  {DBG_SMR_SEND_STATUS,      "sns_smr_send() is being running"                                 },  /*Id 3*/ \
  {DBG_SMR_SEND_DEST_MODULE, "smr_send_domestic(), dst_module(%d)"                             },  /*Id 4*/ \
  {DBG_SMR_SEND_GET_SRVC_OBJ, "getting service object"                                         },  /*Id 5*/ \
  {DBG_SMR_DST_MODULE,       "smr_dsps/la_send(), dst_module(0x%x)"                            },  /*Id 6*/ \
  {DBG_SMR_ALLOC_ERR,        "sns_smr_msg_alloc: req_body(%d) is greater than MAX_BODY(%d)"    },  /*Id 7*/ \
  {DBG_SMR_BODY_STR,         "%s"                                                              },  /*Id 8*/ \
  {DBG_SMR_LOWMEM,           "SMR Low Memory callback complete"                                }   /*Id 9*/ \
 }

/*=====================================================================
                 GLOBAL STRING ARRAY FOR SCM (DSPS)
=======================================================================*/
/* IMPORTANT NOTE on adding strings to the array:
 * When adding a string make sure the string is added as the
 * "n"th element of the array where "n" is the value of the string ID */
#define SCM_STR_DB \
{ {DBG_SCM_TIMER_CB_SIGNALERR, "Failed to post signal in SCM timer callback with error code %d"},  /*Id 0*/  \
  {DBG_SCM_REG_TIMER_STARTED, "Periodic timer started for client %d, with period %d"           },  /*Id 1*/  \
  {DBG_SCM_REG_TIMER_FAILED, "SCM timer registration failed with error %d"                     },  /*Id 2*/  \
  {DBG_SCM_DEREG_TIMER_DELETED,"SCM timer deleted at time %d"                                  },  /*Id 3*/  \
  {DBG_SCM_DEREG_TIMER_FAILED, "Failed to deregister SCM timer with error code %d"             },  /*Id 4*/  \
  {DBG_SCM_TASK_STARTED, "SCM Started....."                                                    },  /*Id 5*/  \
  {DBG_SCM_PROCESS_EVT_UNKWN_EVT, "Ignoring unknown events signaled 0x%x"                      },  /*Id 6*/  \
  {DBG_SCM_PROCESS_MSG_RCVD, "SCM received message id %d, from module %d for service %d"       },  /*Id 7*/  \
  {DBG_SCM_PROCESS_MSG_STATUS, "SCM processed message type %d with error code %d"              },  /*Id 8*/  \
  {DBG_SCM_REQ_ALGO_SVC, "Sent request mesg for algo svc %d mesg id %d with error %d"          },  /*Id 9*/  \
  {DBG_SCM_SMGR_RESP_TYPE_DROP, "Dropping SMGR response for unhandled msg type %d"             },  /*Id 10*/ \
  {DBG_SCM_SMGR_RESP_DATA_DROP, "Dropping SMGR resp for dataReq %d, AckVal %d, ItemLen %d"     },  /*Id 11*/ \
  {DBG_SCM_SMGR_RESP_DROPPED, "Dropping SMGR response for unhandled msg id %d"                 },  /*Id 12*/ \
  {DBG_SCM_SMGR_RESP_SUCCESS, "Rcvd success resp from smgr for dataReq %d, AckVal %d, len %d"  },  /*Id 13*/ \
  {DBG_SCM_SMGR_RESP_ACK_VAL, "Rcvd smgr resp for data req %d with Ack value %d, item len %d"  },  /*Id 14*/ \
  {DBG_SCM_SMGR_RESP_INFO, "SMGR resp: Item %d, Reason %d"                                     },  /*Id 15*/ \
  {DBG_SCM_SMGR_IND_DROPPED, "Dropping manager indication for data request id %d"              },  /*Id 16*/ \
  {DBG_SCM_SMGR_IND_INFO, "Rcvd manager indication for data request id %d, with status %d"     },  /*Id 17*/ \
  {DBG_SCM_SMGR_IND_RATE_INFO, "Rcvd SMGR ind for data req id %d, with rate %d, req rate %d"   },  /*Id 18*/ \
  {DBG_SCM_SMGR_IND_INVALID, "Invalid index %d for algorithm service %d"                       },  /*Id 19*/ \
  {DBG_SCM_REQ_SNSR_DATA_INFO, "Start sensor data request id %d sensor count %d sensor id %d"  },  /*Id 20*/ \
  {DBG_SCM_REQ_SNSR_STATUS_INFO, "Sent sensor status req type %d, sensor id %d, num data %d"   },  /*Id 21*/ \
  {DBG_SCM_SEND_RSP_INFOMSG, "Sent response msg id %d to module %d for service %d"             },  /*Id 22*/ \
  {DBG_SCM_SAM_RESP_INFO, "Received SAM resp msg id %d for algo inst %d, qmdAlgoInst %d"       },  /*Id 23*/ \
  {DBG_SCM_SAM_RESP_DROPPED, "Dropping SAM response with unhandled msg id %d"                  },  /*Id 24*/ \
  {DBG_SCM_SAM_RESP_TYPE_DROP, "Dropping SAM response with unhandled msg type %d"              },  /*Id 25*/ \
  {DBG_SCM_GET_ALGO_INDX_ERR, "Algorithm service %d not supported"                             },  /*Id 26*/ \
  {DBG_SCM_REG_ALGO_ERR, "Algorithm service %d already registered"                             },  /*Id 27*/ \
  {DBG_SCM_REG_ALGO_DFLT_ERR, "Unsupported algorithm service id %d"                            },  /*Id 28*/ \
  {DBG_SCM_ENABLE_ALGO_STATE_NULL, "Algo reset returned with NULL state ptr"                   },  /*Id 29*/ \
  {DBG_SCM_DISABLE_ALGO_INSTANCE_ERR, "Incorrect algorithm instance id %d"                     },  /*Id 30*/ \
  {DBG_SCM_GYRO_CAL_REQ_INFO, "Gyro calibration request to SMGR: x %d, y %d, z %d"             },  /*Id 31*/ \
  {DBG_SCM_LOG_CONFIG_ERR, "Calibration algo config log failed, algo %d inst %d with error: %d"},  /*Id 32*/ \
  {DBG_SCM_LOG_RESULT_ERR, "Calibration algo result log failed, algo %d inst %d with error: %d"},  /*Id 33*/ \
  {DBG_SCM_ALGO_MEM_INFO, "Algorithm service %d, instance mem %d, config mem %d"               },  /*Id 34*/ \
  {DBG_SCM_ALGO_STATE_MEM_INFO, "Algorithm state mem %d, input mem %d, output mem %d"          },  /*Id 35*/ \
  {DBG_SCM_SAM_IND_INFO, "SAM IND: Active algo inst %d, ind algo inst %d, motion state %d"     },  /*Id 36*/ \
  {DBG_SCM_STOP_SNSR_DATA, "Stop sensor data request id %d sensor count %d sensor id %d"       },  /*Id 37*/ \
  {DBG_SCM_ENABLE_ALGO, "Enabled algo service %d for sensor %d with instance %d"               },  /*Id 38*/ \
  {DBG_SCM_DISABLE_ALGO, "Disabled algo service %d for sensor %d with instance %d"             },  /*Id 39*/ \
  {DBG_SCM_REG_REQ_COUNT, "Sent %d registry requests for algorithm configuration data"         },  /*Id 40*/ \
  {DBG_SCM_REG_REQ_SUCCESS, "Received algorithm configuration from registry successfully"      },  /*Id 41*/ \
  {DBG_SCM_REG_REQ_FAIL, "Failed to receive %d registry responses for algorithm configuration" },  /*Id 42*/ \
  {DBG_SCM_REG_RESP_ERR, "Received registry response for item %d with result %d error %d"      },  /*Id 43*/ \
  {DBG_SCM_REG_RESP_PROC_ERR, "Failed to process item %d type %d data length %d"               },  /*Id 44*/ \
  {DBG_SCM_PROC_MSG_HDR_ERR, "Failed to process message due to error %d in getting header"     },  /*Id 45*/ \
  {DBG_SCM_MSG_DROP, "SCM dropped unexpected message id %d, from module %d for service %d"     }   /*Id 46*/ \
}

#endif /* SNS_STRING_DB_H */
