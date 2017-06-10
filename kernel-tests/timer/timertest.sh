#-----------------------------------------------------------------------------
# Copyright (c) 2012 Qualcomm Technologies, Inc.
# All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
#-----------------------------------------------------------------------------


if [ -n "$1" ]
then
time=$1
else
time=60
fi
echo "Sleeping Now . If this takes more or less than $time seconds the timer is broken"
sleep $time
echo "Done"

