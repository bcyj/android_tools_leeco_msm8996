#-----------------------------------------------------------------------------
# Copyright (c) 2008-11 Qualcomm Technologies, Inc.
# All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
#-----------------------------------------------------------------------------

# Script to test the SD card driver.

TEST_MNT="$ANDROID_DATA/mnt"
TEST_TMP="$ANDROID_DATA/tmp"

. $TEST_ENV_SETUP #TARGET_TYPE filled in

#function to get block identifier based on target type
get_block_id(){

case "$TARGET_TYPE" in
    "8650" | "7X27" | "7X30")
     BLOCK_IDENTIFIER=0
     ;;
    "8650A" | "8660" | "8960")
     BLOCK_IDENTIFIER=1
     ;;
    *)
     echo "failed to get block id for unsupported target"
     return 1
     ;;
esac

return 0
}


sd_test_help () {
echo "usage: ./sd_test.sh [-n] [-i <block identifier>] [-v <verbosity>] [-h]"
echo "Refer to README to see how to know the block identifier"
}

do_modprobe () {

if ! [ $TEST_TARGET = "ANDROID" ]
then
   if [ ! -d /sys/bus/platform/drivers/msm_sdcc ]
   then
        modprobe msm_sdcc

        if [ $? -eq 0 ]
        then
				module_present=1
				echo "modprobe msm_sdcc successful"
        else
				echo "modprobe msm_sdcc failed, type dmesg to get info "
				exit 1
        fi
   fi
fi

}

do_rmmod () {

if ! [ $TEST_TARGET = "ANDROID" ]
then
        if [ $module_present -eq 1 ]
        then
	        return modprobe -r msm_sdcc
        fi
fi

return 0

}

# function to mount the sd card
mount_test () {

if [ ! -b $blk_dev/mmcblk$BLOCK_IDENTIFIER ]
then
    echo "$blk_dev/mmcblk$BLOCK_IDENTIFIER does not exist."
    return 1
fi

echo "$blk_dev/mmcblk$BLOCK_IDENTIFIER"

dev_file=` ls $blk_dev/mmcblk$BLOCK_IDENTIFIER* | tail -1 `

echo "dev file $dev_file"

if [  -b $dev_file ]
then
	mkdir $TEST_MNT/sd_disk_0

	if [ $? -ne 0 ]
	then
		echo " mkdir $TEST_MNT/sd_disk_0 failed. type dmesg to get info "
		return 1
	fi

	mount -t vfat $dev_file   $TEST_MNT/sd_disk_0

	if [ $? -ne 0 ]
	then
		echo "mount -t vfat  $dev_file failed. type dmesg to get info "

		#Android adb shell doesn't support rm -rf, so use busybox
		busybox rm -rf $TEST_MNT/sd_disk_0

		return  1
	else
		echo "mounted  vfat on card using the device handle $TEST_MNT/sd_disk_0"
	fi

else
	echo "$dev_file is not block device. type dmesg to get info."
	return 1
fi

return 0
}


test_card () {

#Android adb shell doesn't support rm -rf, so use busybox
busybox rm -rf $TEST_MNT/sd_disk_0/*

if [ $? -ne 0 ]
then
	echo " rm -rf $TEST_MNT/sd_disk_0/* failed. type dmesg to get info "
	test_cleanup
	return  1
fi

mkdir $TEST_MNT/sd_disk_0/test_dir

if [ $? -ne 0 ]
then
	echo "mkdir $TEST_MNT/sd_disk_0/test_dir failed.  type dmesg to get info "
	test_cleanup
	return  1
fi

dd if=/dev/urandom  of=$TEST_MNT/sd_disk_0/test_dir/test_file  bs=512 count=2
if [ $? -ne 0 ]
then
	echo " file creation failed . type dmesg to get info "
	test_cleanup
	return 1
fi

find $TEST_MNT/sd_disk_0 -type f -print | sort | xargs md5sum > $TEST_TMP/md5sum.out1

echo "Umount and remount the device ..."
umount $TEST_MNT/sd_disk_0
if [ $? -ne 0 ]
then
	echo "umount $TEST_MNT/sd_disk_0 failed.  type dmesg to get info "
	test_cleanup
	return  1
fi

mount -t vfat $dev_file   $TEST_MNT/sd_disk_0
if [ $? -ne 0 ]
then
	echo "mount -t vfat  $dev_file failed.  type dmesg to get info "
	test_cleanup
	return  1
fi

find $TEST_MNT/sd_disk_0 -type f -print | sort | xargs md5sum > $TEST_TMP/md5sum.out2
echo "Verifying the contents by comparing the files ..."
set +e
cmp $TEST_TMP/md5sum.out1 $TEST_TMP/md5sum.out2
if [ $? -ne 0 ]
then
	echo "ERROR: Directory/File contents do not match after remount"
	exit 1
fi

return 0

}


test_cleanup () {
rm -r $TEST_TMP/md5sum*
umount  $TEST_MNT/sd_disk_0

if [ $? -ne 0 ]
then
	echo " umount $TEST_MNT/sd_disk_0  failed .  type dmesg to get info "
	rm -r $TEST_MNT/sd_disk_0
	return 1
fi

rm -r $TEST_MNT/sd_disk_0
return 0

}


# start of test
# set -xv
BLOCK_IDENTIFIER=""
verbosity=0

while [ $# -gt 0 ]; do
    case $1 in
    -n | --nominal)
	    shift 1
	    ;;
    -i)
	    BLOCK_IDENTIFIER=$2
	    shift 2
	    ;;
    -v | --verbosity)
	    verbosity=$2
	    shift 2
	    ;;
    -h | --help | *)
	    sd_test_help
	    exit 1
	    ;;
    esac
done

if [ $TEST_TARGET = "ANDROID" ]
then
    blk_dev="/dev/block"
else
    blk_dev="/dev"
fi

# set block identifier
# - use identifier argument if provided by user
# - use auto-detection if not provided by user
if [ "$BLOCK_IDENTIFIER" = "" ]; then
    get_block_id
    if [ $? -ne 0 ]; then
        echo "Please enter block identifier as argument while invoking the test."
        echo "exiting..."
        exit 1
    fi
fi

do_modprobe

sleep 8

exit_status=0

if [ $verbosity -gt 0 ];then
    echo "Detected TARGET_TYPE is $TARGET_TYPE"
    echo "BLOCK_IDENTIFIER = $BLOCK_IDENTIFIER"
fi

mount_test
if [ $? -ne 0 ]
then
    echo "Mount test failed"
    exit_status=1
fi

if [ $exit_status -eq 0 ];then
    test_card
    if [ $? -ne 0 ]
    then
	echo "creating dir or file failed on card"
	exit_status=1
    fi
fi

if [ $exit_status -eq 0 ];then
    test_cleanup
    if [ $? -ne 0 ]
    then
	echo "test_cleanup failed on card"
	exit_status=1
    fi
fi

if [ $exit_status -ne 0 ]
then
	echo -e "\n SD test failed \n"
        do_rmmod
	exit 1
else
	do_rmmod

	if [ $? -ne 0 ]
	then
		echo -e "\n rmmod msm_sdcc failed. Type dmesg to get info \n"
		exit 1
	fi

	echo -e "\n SD test passed \n"
	exit 0
fi
