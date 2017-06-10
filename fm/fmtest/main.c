/*==========================================================================

                     FM Test Application

Description
  Test application for FM V4L2 APIs

# Copyright (c) 2011 by Qualcomm Technologies, Inc.  All Rights Reserved.
# Qualcomm Technologies Proprietary and Confidential.

===========================================================================*/

/*===========================================================================

                         Edit History


when       who     what, where, why
--------   ---     ----------------------------------------------------------
01/25/11   rakeshk  Created a source file to implement a small test application
                    for FM
===========================================================================*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "fmhalapis.h"
char *token[10];

/*===========================================================================
FUNCTION  fm_enable

DESCRIPTION
  Turns on FM with the following default parameters
  1. Band as FM_RX_US_EUROPE
  2. Emphasis as FM_RX_EMP75
  3. Channel spacing as FM_RX_SPACE_50KHZ
  4. RDS system as FM_RX_RDS_SYSTEM
  5. Band limits as 87500-108000

DEPENDENCIES
  NIL

RETURN VALUE
  NONE, It prints the command status on completion

SIDE EFFECTS
  RADIO on event will be printed by the FM interrupt thread

===========================================================================*/

void fm_enable()
{
        printf("Enabling FM ....\n");
	fm_config_data cfg_data =
	{
		.band = FM_RX_US_EUROPE,
		.emphasis = FM_RX_EMP75,
		.spacing = FM_RX_SPACE_50KHZ,
		.rds_system = FM_RX_RDS_SYSTEM,
		.bandlimits = {
			.lower_limit = 87500,
			.upper_limit = 108000
		}
	};

	fm_cmd_status_type ret = EnableReceiver(&cfg_data);
	printf("Cmd Status .... %d\n",ret);

}

/*===========================================================================
FUNCTION  fm_disable

DESCRIPTION
  Turns off FM Receiver

DEPENDENCIES
  NIL

RETURN VALUE
  NONE, It prints the command status on completion

SIDE EFFECTS
  None

===========================================================================*/

void fm_disable()
{
        printf("Disabling FM ....\n");
	fm_cmd_status_type ret = DisableReceiver();
	printf("Cmd Status .... %d\n",ret);

}

/*===========================================================================
FUNCTION  fm_setfreq

DESCRIPTION
  Sets the frequency of FM Receiver

DEPENDENCIES
  NIL

RETURN VALUE
  NONE, It prints the command status on completion

SIDE EFFECTS
  None

===========================================================================*/

void fm_setfreq()
{
	printf("Setfreq %s\n",token[1]);
        printf("Setting frequency .... %d\n",atoi(token[1]));
	fm_cmd_status_type ret = SetFrequencyReceiver(atoi(token[1]));
        printf("Cmd Status .... %d\n",ret);

}

/*===========================================================================
FUNCTION  fm_getcurrentconfig

DESCRIPTION
  Gets the currently tuned stations parameters which include:
  1. Station freq
  2. Whether FM service is available
  3. RSSI level
  4. Audio mode
  5. RDS sync status
  6. Mute status

DEPENDENCIES
  NIL

RETURN VALUE
  NONE

SIDE EFFECTS
  None

===========================================================================*/

void fm_getcurrentconfig()
{
	fm_station_params_available config;
	fm_cmd_status_type ret = GetStationParametersReceiver(&config);
        printf("Current Parameters ....\n");
	printf("Station           : %d\n",config.current_station_freq);
	printf("Service Available : %d\n",config.service_available);
	printf("Rssi              : %d \n",config.rssi);
	printf("Audio mode        : %d\n",config.stype);
	printf("Rds sync          : %d\n",config.rds_sync_status);
	printf("Mute status       : %d\n\n",config.mute_status);
}

/*===========================================================================
FUNCTION  fm_seek

DESCRIPTION
  Seeks for stations in the direction provided as input

DEPENDENCIES
  NIL

RETURN VALUE
  NONE, status of the Seek command

SIDE EFFECTS
  Seeked station will printed by the FM interrupt thread

===========================================================================*/
void fm_seek()
{
	fm_search_stations searchstationsoptions;
        printf("Seeking ....%s\n",token[1]);
	if(!strncmp(token[1],"up",2))
		searchstationsoptions.search_dir = 1;
	else
		searchstationsoptions.search_dir = 0;
	printf("searchstationsoptions.search_dir = %d\n",searchstationsoptions.search_dir);
	searchstationsoptions.search_mode = 0x00;
	searchstationsoptions.dwell_period = 0x07;
	fm_cmd_status_type ret = SearchStationsReceiver(searchstationsoptions);
	printf("Cmd Status .... %d\n",ret);

}

/*===========================================================================
FUNCTION  fm_scan

DESCRIPTION
  Scans for stations in the direction provided as input and dwells at
  the frequency for 7s

DEPENDENCIES
  NIL

RETURN VALUE
  NONE, status of the Scan command

SIDE EFFECTS
  Scannned station will be printed by the FM interrupt thread

===========================================================================*/

void fm_scan()
{
	fm_search_stations searchstationsoptions;
        printf("Scanning ....%s\n",token[1]);
	searchstationsoptions.search_mode = 0x01;
        searchstationsoptions.dwell_period = 0x07;
	if(!strncmp(token[1],"up",2))
                searchstationsoptions.search_dir = 1;
        else
                searchstationsoptions.search_dir = 0;
	fm_cmd_status_type ret = SearchStationsReceiver(searchstationsoptions);
	printf("Cmd Status .... %d\n",ret);

}

/*===========================================================================
FUNCTION  fm_searchlist

DESCRIPTION
  Searchs for a list of good stations within the band limits

DEPENDENCIES
  NIL

RETURN VALUE
  NONE, status of the Search list command

SIDE EFFECTS
  List of starions if any will be printed by the FM interrupt thread

===========================================================================*/

void fm_searchlist()
{
	fm_search_list_stations liststationparams;
	printf("Searching ....\n");
	liststationparams.search_mode = 0x02;
	liststationparams.search_dir = 0x00;
	liststationparams.srch_list_max = 10;
	liststationparams.program_type = 0x00;
	fm_cmd_status_type ret = SearchStationListReceiver(liststationparams);
	printf("Cmd Status .... %d\n",ret);

}

static struct {
        char *cmd;
        void (*fmcmdfunc)();
	char *opt;
        char *desc;
} fmcommand[] = {
        { "enable",         fm_enable, 0,              "Open and initialize FM Radio device" },
	{ "disable",        fm_disable,0,              "Close FM Radio device" },
	{ "setfreq",        fm_setfreq, "<freq >",     "Set Frequency ex. setfreq 93500" },
	{ "getconfig",      fm_getcurrentconfig, 0,    "Get Current Configuration" },
	{ "seek",           fm_seek, "<up/down>",      "Seek <dir> ex. seek up" },
	{ "scan",	    fm_scan, "<up/down>",      "Scan for list of stations ex. scan up" },
	{ "searchlist",     fm_searchlist,0,	       "Search a list of stations" },
	{ "quit",	    NULL,0,		       "Quit application" },
        { NULL, NULL,NULL, 0 }
};


static void usage(void)
{
        int i;

        printf("fmconfig - FM V4L2 device configuration application\n");
	printf("Usage:\n"
                "<command> <opts..>\n");
        for (i=0; fmcommand[i].cmd; i++)
                printf("%-10s\t%-8s\t%s\n", fmcommand[i].cmd,
                fmcommand[i].opt ? fmcommand[i].opt : " ",
                fmcommand[i].desc);
}

int main(int argc, char *argv[])
{
	int option_index = 0;
	int c,i,j = 1;
	char cmd[255];
	char *quit_func = "quit";
	char *delim = " ";
	char *str1;
	char *saveptr1;
	int match = 0;
	usage();
	/* we got a fmconfig enable */
	while(1) {
		printf("\n>");
		fgets(cmd,255,stdin);
		token[0] = strtok_r(cmd, delim, &saveptr1);
		if (token[0] == NULL)
                   break;
		if(!strncmp(token[0],quit_func,3))
			break;
		for (i = 0,match = 0; fmcommand[i].cmd; i++) {
			if (strncmp(fmcommand[i].cmd, token[0], 5))
				continue;
			for(j=1;;j++) {
				token[j] = strtok_r(NULL,delim,&saveptr1);
				if (token[j] == NULL)
					break;
			}
			match = 1;
			fmcommand[i].fmcmdfunc();
		}
		if(!match)
			usage();
	}
	return 0;
}
