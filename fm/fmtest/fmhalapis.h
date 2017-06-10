/*==========================================================================

                     FTM FM PFAL Header File

Description
  Function declarations  of the PFAL interfaces for FM.

# Copyright (c) 2011 by Qualcomm Technologies, Inc.  All Rights Reserved.
# Qualcomm Technologies Proprietary and Confidential.

===========================================================================*/

/*===========================================================================

                         Edit History


when       who     what, where, why
--------   ---     ----------------------------------------------------------
01/25/11   rakeshk  Created a header file to hold the interface declarations
                    for FM functional APIs
===========================================================================*/
#include "fmcommon.h"

/*===========================================================================
FUNCTION  EnableReceiver

DESCRIPTION
  PFAL specific routine to enable the FM receiver with the Radio Cfg
  parameters passed.

DEPENDENCIES
  NIL

RETURN VALUE
  FM command status

SIDE EFFECTS
  None

===========================================================================*/
fm_cmd_status_type EnableReceiver
(
  fm_config_data*      radiocfgptr
);

/*===========================================================================
FUNCTION  DisableReceiver

DESCRIPTION
  PFAL specific routine to disable the FM receiver and free the FM resources

DEPENDENCIES
  NIL

RETURN VALUE
  FM command status

SIDE EFFECTS
  None

===========================================================================*/
fm_cmd_status_type DisableReceiver
(
);

/*===========================================================================
FUNCTION  ConfigureReceiver

DESCRIPTION
  PFAL specific routine to configure the FM receiver with the Radio Cfg
  parameters passed.

DEPENDENCIES
  NIL

RETURN VALUE
  FM command status

SIDE EFFECTS
  None

===========================================================================*/
fm_cmd_status_type ConfigureReceiver
(
 fm_config_data*      radiocfgptr
);

/*===========================================================================
FUNCTION  SetFrequencyReceiver

DESCRIPTION
  PFAL specific routine to configure the FM receiver's Frequency of reception

DEPENDENCIES
  NIL

RETURN VALUE
  FM command status

SIDE EFFECTS
  None

===========================================================================*/
fm_cmd_status_type SetFrequencyReceiver
(
 uint32 ulfreq
);

/*===========================================================================
FUNCTION  SetMuteModeReceiver

DESCRIPTION
  PFAL specific routine to configure the FM receiver's mute status

DEPENDENCIES
  NIL

RETURN VALUE
  FM command status

SIDE EFFECTS
  None

===========================================================================*/
fm_cmd_status_type SetMuteModeReceiver
(
 mute_type mutemode
);

/*===========================================================================
FUNCTION  SetStereoModeReceiver

DESCRIPTION
  PFAL specific routine to configure the FM receiver's Audio mode on the
  frequency tuned

DEPENDENCIES
  NIL

RETURN VALUE
  FM command status

SIDE EFFECTS
  None

===========================================================================*/
fm_cmd_status_type SetStereoModeReceiver
(
 stereo_type stereomode
);

/*===========================================================================
FUNCTION  GetStationParametersReceiver

DESCRIPTION
  PFAL specific routine to get the station parameters of the Frequency at
  which the Radio receiver is  tuned

DEPENDENCIES
  NIL

RETURN VALUE
  FM command status

SIDE EFFECTS
  None

===========================================================================*/
fm_cmd_status_type GetStationParametersReceiver
(
 fm_station_params_available*    configparams
);
/*===========================================================================
FUNCTION  SetRdsOptionsReceiver

DESCRIPTION
  PFAL specific routine to configure the FM receiver's RDS options

DEPENDENCIES
  NIL

RETURN VALUE
  FM command status

SIDE EFFECTS
  None

===========================================================================*/
fm_cmd_status_type SetRdsOptionsReceiver
(
 fm_rds_options rdsoptions
);

/*===========================================================================
FUNCTION  SetRdsGroupProcReceiver

DESCRIPTION
  PFAL specific routine to configure the FM receiver's RDS group proc options

DEPENDENCIES
  NIL

RETURN VALUE
  FM command status

SIDE EFFECTS
  None

===========================================================================*/
fm_cmd_status_type SetRdsGroupProcReceiver
(
 uint32 rdsgroupoptions
);

/*===========================================================================
FUNCTION  SetPowerModeReceiver

DESCRIPTION
  PFAL specific routine to configure the power mode of FM receiver

DEPENDENCIES
  NIL

RETURN VALUE
  FM command status

SIDE EFFECTS
  None

===========================================================================*/
fm_cmd_status_type SetPowerModeReceiver
(
 uint8 powermode
);

/*===========================================================================
FUNCTION  SetSignalThresholdReceiver

DESCRIPTION
  PFAL specific routine to configure the signal threshold of FM receiver

DEPENDENCIES
  NIL

RETURN VALUE
  FM command status

SIDE EFFECTS
  None

===========================================================================*/
fm_cmd_status_type SetSignalThresholdReceiver
(
 uint8 signalthreshold
);

/*===========================================================================
FUNCTION  SearchStationsReceiver

DESCRIPTION
  PFAL specific routine to search for stations from the current frequency of
  FM receiver and print the information on diag

DEPENDENCIES
  NIL

RETURN VALUE
  FM command status

SIDE EFFECTS
  None

===========================================================================*/
fm_cmd_status_type SearchStationsReceiver
(
fm_search_stations searchstationsoptions
);


/*===========================================================================
FUNCTION  SearchRDSStationsReceiver

DESCRIPTION
  PFAL specific routine to search for stations from the current frequency of
  FM receiver with a specific program type and print the information on diag

DEPENDENCIES
  NIL

RETURN VALUE
  FM command status

SIDE EFFECTS
  None

===========================================================================*/
fm_cmd_status_type SearchRdsStationsReceiver
(
fm_search_rds_stations searchrdsstationsoptions
);


/*===========================================================================
FUNCTION  SearchStationListReceiver

DESCRIPTION
  PFAL specific routine to search for stations with a specific mode of
  informaation like WEAK,STRONG,STRONGEST etc

DEPENDENCIES
  NIL

RETURN VALUE
  FM command status

SIDE EFFECTS
  None

===========================================================================*/
fm_cmd_status_type SearchStationListReceiver
(
fm_search_list_stations searchliststationsoptions
);


/*===========================================================================
FUNCTION  CancelSearchReceiver

DESCRIPTION
  PFAL specific routine to cancel the ongoing search operation

DEPENDENCIES
  NIL

RETURN VALUE
  FM command status

SIDE EFFECTS
  None

===========================================================================*/
fm_cmd_status_type CancelSearchReceiver
(
);

