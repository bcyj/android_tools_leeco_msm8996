#-----------------------------------------------------------------------------
# Copyright (c) 2011 Qualcomm Technologies, Inc.
# All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
#-----------------------------------------------------------------------------

. $TEST_ENV_SETUP

sps_test_path=/system/lib/modules/msm_sps_test_module.ko

insmod "$sps_test_path"
if [ $? -ne 0 ]; then
	echo "ERROR: failed to load module $sps_test_path"
	exit 1
fi

#invoke test
chmod 755 msm_sps_test
./msm_sps_test


