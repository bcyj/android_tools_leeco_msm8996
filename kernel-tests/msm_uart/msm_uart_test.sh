#-----------------------------------------------------------------------------
# Copyright (c) 2011 Qualcomm Technologies, Inc.
# All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
#-----------------------------------------------------------------------------

. $TEST_ENV_SETUP #TARGET_TYPE filled in

#get the port number based on target type
has_num=0
num=0

get_port_num(){
    case $TARGET_TYPE in
    8660 | 8960)
	    num=4
	    ;;
    7X30)
            num=2
	    ;;
    8650 | 8650A | 7X27)
            num=3
	    ;;
    *)
            echo "Not be able to get target type use UART 1"
	    num=1
	    ;;
    esac

}

cmd_line="$@"

while [ $# -gt 0 ]; do
        case $1 in
        --port)
		has_num=1
		shift 2
		;;
	-n)
		shift 1
		;;
	-e)
		shift 1
		;;
	-l)
		shift 1
		;;
	-h | --help | *)
		echo "Usage: $0 [-n] [-e] [-l] [--port n]"
		exit 1
		;;
        esac
done

if [ $has_num -eq 1 ]; then
    ./msm_uart_test $cmd_line
else
    get_port_num
    ./msm_uart_test $cmd_line --port $num
fi
