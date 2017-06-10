#!/bin/sh

# ---------------------------------------------------------------------------
# Copyright (c) 2008 Qualcomm Technologies, Inc.
# All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
# ---------------------------------------------------------------------------

if [ -e /dev/ttyHSUSB0 ]
then
	echo "ttyHSUSB0 already exists, continuing to run the port bridge application"
else
	mknod /dev/ttyHSUSB0 c 127 0
fi

export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/opt/qcom/lib

echo "Starting the port_bridge application..."
/opt/qcom/bin/./port_bridge /dev/smd0 /dev/ttyHSUSB0 &
