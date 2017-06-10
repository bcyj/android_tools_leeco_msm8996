#-----------------------------------------------------------------------------
# Copyright (c) 2008-09 Qualcomm Technologies, Inc.
# All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
#-----------------------------------------------------------------------------
#!/system/bin/sh

. $TEST_ENV_SETUP

if [ $TEST_TARGET == "ANDROID" ]
then
	TEST_MNT="/cache"
	RMMOD_PATH="/system/bin/rmmod"
	MOUNT_PARTITION="/dev/block/mtdblock1"
else
	TEST_MNT="/mnt/flash"
	RMMOD_PATH="rmmod"
	MOUNT_PARTITION="/dev/mtdblock0"
fi

mtd_driver_test_help () {

	echo "mtd_driver_test.sh < test_to_run > < device_partition_number >"
	echo "Usage: mtd_driver_test.sh < oobtest | pagetest | subpagetest | readtest | stresstest | speedtest > < 0 | 1 >"
	echo "eg: mtd_driver_test.sh subpagetest 1"
	exit 1
}

driver_test () {
echo "==============================================================="
echo "Unmount $TEST_MNT partition"
umount $TEST_MNT
echo "Running the mtd_$1.ko"
insmod mtd_erasepart.ko dev=$2
if [ $? -ne 0 ];
then
	echo "insmod of mtd_erasepart.ko FAILED"
	echo "Mounting back YAFFS2 on the $TEST_MNT partition"
	mount -t yaffs2 $MOUNT_PARTITION $TEST_MNT
	echo "==============================================================="
	exit 1
fi

insmod mtd_$1.ko dev=$2
if [ $? -ne 0 ];
then
	$RMMOD_PATH mtd_erasepart.ko
	echo "insmod of mtd_$1.ko FAILED"
	echo "Mounting back YAFFS2 on the $TEST_MNT partition"
	mount -t yaffs2 $MOUNT_PARTITION $TEST_MNT
	echo "==============================================================="
	exit 1
else
	$RMMOD_PATH mtd_$1.ko
	$RMMOD_PATH mtd_erasepart.ko
	echo "Mounting back YAFFS2 on the $TEST_MNT partition"
	echo "Test PASSED"
	insmod mtd_erasepart.ko dev=$2
	$RMMOD_PATH mtd_erasepart.ko
	mount -t yaffs2 $MOUNT_PARTITION $TEST_MNT
	echo "==============================================================="
	exit 0
fi

}

# start of test
#set -xv

if [ $# -ne 2 ]
then
	mtd_driver_test_help
fi

if [ "$1" == "oobtest" -o "$1" == "pagetest" -o "$1" == "subpagetest" -o \
	"$1" == "readtest" -o "$1" == "stresstest" -o "$1" == "speedtest" ]
then
	driver_test $1 $2
else
	mtd_driver_test_help
fi

