#! /bin/sh

# Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
# Qualcomm Technologies Proprietary and Confidential.

set -e

case "$1" in
  start)
        echo -n "Starting tftp_server: "
        start-stop-daemon -S -b -a /sbin/tftp_server

        echo "done"
        ;;
  stop)
        echo -n "Stopping tftp_server: "
        start-stop-daemon -K -n tftp_server
        echo "done"
        ;;
  restart)
        $0 stop
        $0 start
        ;;
  *)
        echo "Usage tftp_server { start | stop | restart}" >&2
        exit 1
        ;;
esac

exit 0
