#-----------------------------------------------------------------------------
# Copyright (c) 2008-2011, 2013 Qualcomm Technologies, Inc.
# All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
#-----------------------------------------------------------------------------

#  Unit test for the SPI driver.
#
# Invoke by spidevtest.sh
# This will run nominal testcases.

. $TEST_ENV_SETUP

# Search the dev filesystem for the location of the device associated with the
# spidev
dev_location="/dev"
test="/data/kernel-tests/spidevtest"

spi_module="spi_qsd"
spidev_module="spidev"
modules_location="/system/lib/modules"
spimod_location="/sys/bus/platform/devices"
if [ -f /sys/devices/soc0/hw_platform ]; then
	device_location="/sys/devices/soc0/hw_platform"
else
	device_location="/sys/devices/system/soc/soc0/hw_platform"
fi
busnum="0"
chipselect="0"
bufsiz="35000"

device_type=`cat $device_location`
if [ "$TARGET_TYPE" = "" ];then
  echo "Target not supported."
  exit 1
elif [ $device_type = "Unknown" ]; then
  echo "Device type info at $device_location unrecognized.Please validate setup."
  exit 1
elif [ $TARGET_TYPE  = "8660" ] && [ $device_type = "Fluid" ]; then
  busnum="1"
  chipselect="1"
elif [ $TARGET_TYPE  = "8960" ]; then
  chipselect="2"
fi

export spimod_exist=`ls $spimod_location | grep $spi_module`
if [ "$spimod_exist" = "" ]; then
  export spi_exist`lsmod | grep $spi_module | cut -d" " -f1`
  if [ "$spi_exist" = "" ]; then
     insmod $modules_location/$spi_module".ko"
     i=`echo $?`
     if [[ $i -ne 0 ]]
     then
        echo "Failed to insert $spi_module: $i"
	exit 1
     fi
  fi
fi

export spidev_exist=`ls $dev_location | grep $spidev_module`
if [ "$spidev_exist" = "" ]; then
  insmod $modules_location/$spidev_module".ko" "busnum=$busnum chipselect=$chipselect bufsiz=$bufsiz"
  i=`echo $?`
  if [[ $i -ne 0 ]]; then
    echo "Failed to insert "$modules_location/$spidev_module ": $i"
    if [ "$spimod_exist" = "" ]; then
        rmmod "$spi_module"
    fi
    exit 1
  fi
  export dev=`ls $dev_location | grep spidev`
  if test ! -c "$dev_location/$dev"
  then
    echo "device spidev not found in "$dev_location
    rmmod "$spidev_module"
    if [ "$spimod_exist" = "" ]; then
       rmmod "$spi_module"
    fi
    exit 1
  fi
fi

#Run Test
$test $dev
ret=`echo $?`

if [ "$spidev_exist" = "" ]; then
  rmmod "$spidev_module"
fi
if [ "$spimod_exist" = "" ]; then
  rmmod "$spi_module"
fi

exit $ret

