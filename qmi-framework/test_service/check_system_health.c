/******************************************************************************
  ---------------------------------------------------------------------------
  Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
*******************************************************************************/
#include <string.h>
#include <getopt.h>
#include "sys_health_mon.h"
static uint32_t iterations = 1;
static int delay;


static void usage(const char *fname)
{
  printf("\n*************************************************************\n");
  printf("Usage: %s PARAMS can be:\n"
	  "  -h,          Help Option to print Usage\n"
	  "  -i <value>,  Number of health check iterations\n"
	  "  -d <value>,  Delay between each iteration in ms\n", fname);
  printf("\n*************************************************************\n");
}

static void parse_command(int argc, char *const argv[])
{
  int command;

  struct option longopts[] = {
	{"iterations", required_argument, NULL, 'i'},
	{"help", no_argument, NULL, 'h'},
	{"delay", required_argument, NULL, 'd'},
	{NULL, 0, NULL, 0},
  };

  while ((command = getopt_long(argc, argv, "h:i:d:", longopts,
				NULL)) != -1) {
    switch (command) {
      case 'h':
        usage(argv[0]);
	exit(0);
      case 'i':
	iterations = atoi(optarg);
	break;
      case 'd':
	delay = atoi(optarg);
	break;
      default:
        usage(argv[0]);
	exit(-1);
    }
  }
}


/*=============================================================================
  FUNCTION main
=============================================================================*/
int main(int argc, char **argv)
{
  uint32_t i;
  int rc;

  parse_command(argc, argv);
  for (i = 0; i < iterations; i++)
  {
    rc = check_system_health();
    if (rc < 0)
    {
      printf("Check system health returned %d @ %d iterations\n", rc, i);
      return rc;
    }
    usleep(delay * 1000);
  }
  return 0;
}
