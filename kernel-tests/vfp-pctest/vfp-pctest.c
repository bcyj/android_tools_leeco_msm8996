/*******************************************************************************
* Copyright (c) 2008-2009 Qualcomm Technologies, Inc.
* All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
*******************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include "sleepRestore.h"

int fpu = 0;
int uniqueId = 0;
int cycles = 0;

void load(const int* input)
{
  fpu ? vfpv2_regLoad(input) : neon_regLoad(input);
}

void dump(int*  reg_dump)
{
  fpu ? vfpv2_regStore(reg_dump) : neon_regStore(reg_dump);
}

int vfp2LoadVerifyThread(const int inputValues[64])
{
  int i;
  int totalErrors = 0;
  int errorFlag[64];
  int loadValues[64];
  int dumpValues[64];
  const int regCount = fpu ? 32 : 64;

  for(i = 0; i < regCount; i++)
  {
    errorFlag[i] = 0;
  }

  while(cycles > 0)
  {
    // We need a constantly changing VFP context to detect lazy context switch failures
    for(i = 0; i < regCount; i++)
    {
      loadValues[i] = inputValues[i] ^ cycles;
      dumpValues[i] = 0;
    }
    // load our context into VFP registers
    load(loadValues);
    // sleep to encourage context switch and power collapse
    usleep(500000 + (rand() & 0xfffff));
    // dump vfp registers to struct
    dump(dumpValues);

    // compare results.
    for(i = 0; i < regCount; i++)
    {
      if((dumpValues[i] ^ inputValues[i]) != cycles)
      {
        errorFlag[i]++;
        printf("Id %d: detected error in register index [%d]. dumpValues[i](0x%8X) ^ inputValues[i](0x%8X) != cycles(0x%8X)\n", uniqueId, i, dumpValues[i], inputValues[i], cycles);
      }
    }
    cycles--;
  }
  for(i = 0; i < regCount; i++)
  {
    if(errorFlag[i] != 0)
    {
      totalErrors += errorFlag[i];
      printf("Id %d: register index [%d] had %d errors\n", uniqueId, i, errorFlag[i]);
    }
  }
  printf("Id %d: had total %d errors\n", uniqueId, totalErrors);
  return(totalErrors);
}

/* Use VFP in signal handler */
void sigaction_handler(int signum, siginfo_t *info, void *context)
{
	int i;
	int testValues[64];
	int dumpValues[64];
	const int regCount = fpu ? 32 : 64;

	/* use a different test patttern for each signal */
	for(i= 0; i < 64; i++)
	{
	  testValues[i] = signum + (i  << 8);
	}

	// load our context into VFP registers
	load(testValues);
	// sleep to encourage context switch and power collapse
	usleep(500000);
	// dump vfp registers to struct
	dump(dumpValues);

	// compare results.
	for(i = 0; i < regCount; i++)
	{
	  if(dumpValues[i] != testValues[i])
	  {
	    printf("Id %d: detected error in register index [%d]. dumpValues[i](0x%8X) != testValues[i](0x%8X)\n", uniqueId, i, dumpValues[i], testValues[i]);
	  }
	}
}

void register_signals(void)
{
	struct sigaction act;

	act.sa_sigaction = sigaction_handler;
	sigemptyset(&act.sa_mask);
	act.sa_flags = SA_SIGINFO;
	act.sa_restorer = 0;
	sigaction(SIGUSR1, &act, 0);
	sigaction(SIGUSR2, &act, 0);
	sigaction(SIGFPE, &act, 0);
}

void usage(void)
{
  printf("vfp-pctest [fp] [uniqueId] [cycles]\n");
  printf("  fp        : --vfpv2 | --neon\n");
  printf("  uniqueId  : positive integer used for threadId and random seed for data generation\n");
  printf("  cycles    : positive integer number of test cycles to run (~1 second per cycle)\n");
  printf("\n");
}

int main(int argc, char** argv)
{
  int i;
  int testValues[64];

  if(argc != 4)
  {
    usage();
    return -1;
  }

  if(!strcmp(argv[1], "--vfpv2"))
  {
    fpu = 1;
  }
  else if(!strcmp(argv[1], "--neon"))
  {
    fpu = 0;
  }
  else
  {
    usage();
    return -1;
  }

  if(!(uniqueId = atoi(argv[2])))
  {
    usage();
    return -1;
  }
  if(!(cycles = atoi(argv[3])))
  {
    usage();
    return -1;
  }

  register_signals();

  srand(uniqueId^0x7fA55a83);

  for(i= 0; i < 64; i++)
  {
    testValues[i] = rand();
  }

  srand(0x7fA55a83);
  for(i= 0; i < 64; i++)
  {
    if(testValues[i] == rand())
    {
      printf("rand() not working as expected\n");
      return -1;
    }
  }

  srand(uniqueId^0x7fA55a83);
  for(i= 0; i < 64; i++)
  {
    if(testValues[i] != rand())
    {
      printf("rand() not working as expected\n");
      return -1;
    }
  }

  return vfp2LoadVerifyThread(testValues);
}

