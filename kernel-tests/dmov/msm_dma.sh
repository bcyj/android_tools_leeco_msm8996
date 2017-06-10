#-----------------------------------------------------------------------------
# Copyright (c) 2011 Qualcomm Technologies, Inc.
# All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
#-----------------------------------------------------------------------------

. $TEST_ENV_SETUP

device="/dev/msmdma"
dm_path=/system/lib/modules/dma_test.ko
dm_path_oe=/lib/modules/dma_test.ko
need_rmmod=0

while [ $# -gt 0 ]; do
        case $1 in
        -n | --nominal)
        # already nominal test case by default
        shift 1
        ;;
	-p)
	dm_path=$2
	shift 2
	;;
        -h | --help | *)
        echo "Usage: $0 [-n | --nominal] [-p <path to dma_test.ko>]";
        exit 1
	;;
        esac
done

#check if device node exist, if not insmod dma_test.ko
if [ ! -e "$device" ]; then
    if [ -e "$dm_path_oe" ]; then
        dm_path=$dm_path_oe;
    fi
    if [ -e "$dm_path" ]; then
	insmod "$dm_path"
	if [ $? -ne 0 ]; then
	    echo "ERROR: failed to load module $dm_path"
	    exit 1
	fi
	need_rmmod=1
    else
	echo "ERROR: failed to find $dm_path"
	exit 1
    fi
fi

#invoke test
./msm_dma
ret=$?

if [ "$need_rmmod" -eq 1 ]; then
    rmmod dma_test
fi

if [ $ret -ne 0 ];then
    echo "Test Failed"
else
    echo "Test Passed"
fi

exit $ret
