#!/bin/bash
#==========================================================================
# Copyright (c) 2013 Qualcomm Atheros, Inc.
# All Rights Reserved.
# Qualcomm Atheros Confidential and Proprietary.
#==========================================================================
#===========================================================================
#
# Usage:
#     mksdkjar.sh [output-path]
#
# Note, this script requires the existence of the build binary -
# out/target/common/obj/JAVA_LIBRARIES/framework_intermediates/classes.jar
#
#==========================================================================

DIR=${0%/*}
CMD=${0##*/}
OUT="/tmp/sdkaddon"

if [ -n "$1" ]
then
    OUT=$1
fi

if [ ! -e $DIR/../../../../../../../out/target/common/obj/JAVA_LIBRARIES/framew\
ork_intermediates/classes.jar ]
then
    echo "this script requires Android frameworks to have been built"
    exit
fi

mkdir -p $OUT/classes
#cd $DIR
javac -cp $DIR/../../../../../../../out/target/common/obj/JAVA_LIBRARIES/framework_intermediates/classes.jar -sourcepath $DIR/../java -d $OUT/classes/ $DIR/../java/com/qualcomm/snapdragon/sdk/location/*.java
jar cvf $OUT/sdkaddon.jar -C $OUT/classes/ .

mkdir -p $OUT/docs
javadoc -d $OUT/docs -sourcepath $DIR/../java/ -exclude my.tests.geofence -exclude com.qualcomm.services.location -classpath $DIR/../../../../../../../out/target/common/obj/JAVA_LIBRARIES/framework_intermediates/classes.jar com.qualcomm.snapdragon.sdk.location

rm -rf $OUT/classes

echo "Java docs for sdk available at $OUT"
