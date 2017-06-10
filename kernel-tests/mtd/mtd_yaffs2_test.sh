#-----------------------------------------------------------------------------
# Copyright (c) 2008-09 Qualcomm Technologies, Inc.
# All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
#-----------------------------------------------------------------------------
#!/system/bin
# Script to test the YAFFS2/MTD driver.
# File system based test

. $TEST_ENV_SETUP

if [ $TEST_TARGET == "ANDROID" ]
then
	mkdir -p /cache/data
	mkdir -p "$ANDROID_DATA/tmp_md5"
	TEST_TMP_DATA="/cache/data"
	TEST_TMP_MD5="$ANDROID_DATA/tmp_md5"
	TEST_MNT="/cache"
	BUSYBOX_PREFIX="busybox"
	MOUNT_PARTITION="/dev/block/mtdblock1"

else
	mkdir -p /mnt/flash/data
	mkdir -p /tmp/tmp_md5
	TEST_TMP_DATA="/mnt/flash/data"
	TEST_TMP_MD5="/tmp/tmp_md5"
	TEST_MNT="/mnt/flash"
	BUSYBOX_PREFIX=""
	MOUNT_PARTITION="/dev/mtdblock0"
fi

# ========================================================================
# Perform the necessary cleanup and get ready for the test
# Remove old md5sum, & mount yaffs2 partition
# ========================================================================

echo "==============================================================="
umount $TEST_MNT
mount -t yaffs2 $MOUNT_PARTITION $TEST_MNT

# ========================================================================
# Copy new files into $TEST_MNT and compute the md5 sum
# ========================================================================

echo "Creating files in $TEST_TMP_DATA ..."
$BUSYBOX_PREFIX cp -R /etc/* $TEST_TMP_DATA
echo "Storing md5sum of files in $TEST_TMP_MD5 ..."
$BUSYBOX_PREFIX find $TEST_MNT -type f -print | $BUSYBOX_PREFIX sort | $BUSYBOX_PREFIX xargs $BUSYBOX_PREFIX md5sum > $TEST_TMP_MD5/md5sum.out1

# ========================================================================
# Unmount and remount the device and verify the md5 sum
# ========================================================================

echo "Unmount and Remount the device ..."
umount $TEST_MNT
mount -t yaffs2 $MOUNT_PARTITION $TEST_MNT
echo "Verifying contents by comparing md5sum ..."
$BUSYBOX_PREFIX find $TEST_MNT -type f -print | $BUSYBOX_PREFIX sort | $BUSYBOX_PREFIX xargs $BUSYBOX_PREFIX md5sum > $TEST_TMP_MD5/md5sum.out2
$BUSYBOX_PREFIX cmp $TEST_TMP_MD5/md5sum.out1 $TEST_TMP_MD5/md5sum.out2

if [ $? -ne 0 ];
then
	echo "ERROR: Directory/File contents do not match after remount"
	echo "MTD Test Failed"
	$BUSYBOX_PREFIX rm -rf $TEST_TMP_MD5
	$BUSYBOX_PREFIX rm -rf $TEST_TMP_DATA
	umount $TEST_MNT
	mount -t yaffs2 $MOUNT_PARTITION $TEST_MNT
	echo "==============================================================="
	exit 1
else
	echo "MTD Test Passed"
	$BUSYBOX_PREFIX rm -rf $TEST_TMP_MD5
	$BUSYBOX_PREFIX rm -rf $TEST_TMP_DATA
	umount $TEST_MNT
	mount -t yaffs2 $MOUNT_PARTITION $TEST_MNT
	echo "==============================================================="
fi

