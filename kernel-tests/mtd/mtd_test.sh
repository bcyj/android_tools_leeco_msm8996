#-----------------------------------------------------------------------------
# Copyright (c) 2008-09 Qualcomm Technologies, Inc.
# All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
#-----------------------------------------------------------------------------

. $TEST_ENV_SETUP

if [ $TEST_TARGET == "ANDROID" ]
then
	dev=1;
else
	dev=0;
fi

if [ "$1" == "-n" -o "$1" == "--nominal" -o $# -eq 0 ]
then
	echo "Running mtd test, Category: nominal"
	sh mtd_yaffs2_test.sh
	if [ $? -ne 0 ]; then
		exit 1
	fi
	#Adrversarial test
        sh mtd_driver_test.sh "oobtest" $dev
	if [ $? -ne 0 ]; then
		exit 1
	fi
	sh mtd_driver_test.sh "pagetest" $dev
	if [ $? -ne 0 ]; then
		exit 1
	fi
	sh mtd_driver_test.sh "subpagetest" $dev
	if [ $? -ne 0 ]; then
		exit 1
	fi
	#Preformance test
	sh mtd_driver_test.sh "speedtest" $dev
	if [ $? -ne 0 ]; then
		exit 1
	fi
elif [ "$1" == "-a" -o "$1" == "--adversarial" ]; then
	echo "Running mtd test, Category: adversarial"
	sh mtd_driver_test.sh "oobtest" $dev
	if [ $? -ne 0 ]; then
		exit 1
	fi
	sh mtd_driver_test.sh "pagetest" $dev
	if [ $? -ne 0 ]; then
		exit 1
	fi
	sh mtd_driver_test.sh "subpagetest" $dev
	if [ $? -ne 0 ]; then
		exit 1
	fi
elif [ "$1" == "-r" -o "$1" == "--release" ]
then
	echo "Running mtd test, Category: release"
	#Nominal test
	sh mtd_yaffs2_test.sh
	if [ $? -ne 0 ]; then
		exit 1
	fi
	#Adrversarial test
        sh mtd_driver_test.sh "oobtest" $dev
	if [ $? -ne 0 ]; then
		exit 1
	fi
	sh mtd_driver_test.sh "pagetest" $dev
	if [ $? -ne 0 ]; then
		exit 1
	fi
	sh mtd_driver_test.sh "subpagetest" $dev
	if [ $? -ne 0 ]; then
		exit 1
	fi
	#Stress test
	sh mtd_driver_test.sh "stresstest" $dev
	if [ $? -ne 0 ]; then
		exit 1
	fi
	#Preformance test
	sh mtd_driver_test.sh "speedtest" $dev
	if [ $? -ne 0 ]; then
		exit 1
	fi
elif [ "$1" == "-s" -o "$1" == "--stress" ]
then
	echo "Running mtd test, Category: stress"
	sh mtd_driver_test.sh "stresstest" $dev
	if [ $? -ne 0 ]; then
		exit 1
	fi
elif [ "$1" == "-p" -o "$1" == "--repeatability" ]
then
	echo "Running mtd test, Category: repeatability"
        for i in 1 2 3 4 5
        do
		#Nominal test
		sh mtd_yaffs2_test.sh
		if [ $? -ne 0 ]; then
			exit 1
		fi
		#Advarserial test
		sh mtd_driver_test.sh "oobtest" $dev
		if [ $? -ne 0 ]; then
			exit 1
		fi
		sh mtd_driver_test.sh "pagetest" $dev
		if [ $? -ne 0 ]; then
			exit 1
		fi
	        sh mtd_driver_test.sh "subpagetest" $dev
		if [ $? -ne 0 ]; then
			exit 1
		fi
      done
elif [ "$1" == "-h" -o "$1" == "--help" ]
then
	echo "usage: ./mtd_test.sh -n, --nominal | -a, --adversarial | -s, --stress | -r, --release | -p, --repeatability | -h, --help"
	exit 1
else
	echo "usage: ./mtd_test.sh -n, --nominal | -a, --adversarial | -s, --stress | -r, --release | -p, --repeatability | -h, --help"
	exit 1
fi
