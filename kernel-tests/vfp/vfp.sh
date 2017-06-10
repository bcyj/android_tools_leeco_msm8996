#-----------------------------------------------------------------------------
# Copyright (c) 2011-2014 Qualcomm Technologies, Inc.
# All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
#-----------------------------------------------------------------------------

#!/bin/sh
echo "VFP test starting"

echo "Checking kernel configuration"

C=`ls /proc/config.gz |wc -l`

if [ "$C" == "0" ]
then
	echo "No /proc/config.gz found."
	echo "Test failed."
	exit 1
fi

echo " * Looking for kernel VFP support"
C=`zcat /proc/config.gz |grep -E '(CONFIG_VFP=y)|(CONFIG_ARM64=y)' |wc -l`

if [ "$C" == "0" ]
then
	echo "CONFIG_VFP not enabled in the kernel."
	echo "Test failed."
	exit 1
fi

echo " * Looking for CONFIG_FPE_NWFPE"
C=`zcat /proc/config.gz |grep CONFIG_FPE_NWFPE=y |wc -l`

if [ "$C" -ne "0" ]
then
	echo "CONFIG_FPE_NWFPE is enabled!"
	echo "Test failed."
	exit 1
fi

echo " * Looking for CONFIG_FPE_FASTFPE"
C=`zcat /proc/config.gz |grep CONFIG_FPE_FASTFPE=y |wc -l`

if [ "$C" -ne "0" ]
then
	echo "CONFIG_FPE_FASTFPE is enabled!"
	echo "Test failed."
	exit 1
fi

echo "Running math test..."
./vfptest 500000 10

if [ "$?" -ne "0" ]
	then
	echo "Test failed."
	exit 1
fi

echo "VFP test passed."
