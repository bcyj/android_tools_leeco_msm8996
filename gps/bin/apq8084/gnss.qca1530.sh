#!/system/bin/sh

#
# Copyright (c) 2013-2014 QUALCOMM Atheros, Inc.
# All Rights Reserved.
# Qualcomm Atheros Confidential and Proprietary
#

QCA1530_DIR=/data/misc/location/qca1530
QCA1530_DIR_EFS=$QCA1530_DIR/efs
QCA1530_CFG=$QCA1530_DIR/gnss-fsh.bin
QCA1530_CFG_SRC=/system/vendor/etc/gnss/gnss-fsh.bin
QCA1530_EXE=/system/vendor/bin/gnss.qca1530.svcd
QCA1530_PROP=sys.qca1530
QCA1530_CHIPSTATE=/sys/kernel/qca1530/chip_state
QCA1530_RESET0=/sys/kernel/qca1530/reset

if [ -$LOG_TAG = - ]
then
  LOG_TAG=qca1530
fi

function log_error
{
  log -p e -t $LOG_TAG $*
}

function log_debug
{
  log -p d -t $LOG_TAG $*
}

function log_verbose
{
  log -p v -t $LOG_TAG $*
}

function die
{
  log_error $*
  exit 1
}

function qca1530_setup_reset
{
  if [ -f $QCA1530_RESET0 ]
  then
    #driver reset
    chown gps:system $QCA1530_RESET0
    chmod 0644 $QCA1530_RESET0
  elif [ -f $QCA1530_RESET1 ]
  then
    #gpio133 on APQ8084 CDP
    chown gps:system $QCA1530_RESET1
    chmod 0644 $QCA1530_RESET1
  elif [ -f $QCA1530_RESET2 ]
  then
    #gpio128 on APQ8084 LIQUID
    chown gps:system $QCA1530_RESET2
    chmod 0644 $QCA1530_RESET2
  fi
}

function qca1530_setup
{
  qca1530_setup_reset

  if [ -d $QCA1530_DIR ]
  then
    log_debug "QCA1530: NV directory exists"
  else
    log_debug "QCA1530: Creating NV directory"
    mkdir -p $QCA1530_DIR || die "QCA1530: Failed to create NV directory"
    chown gps:gps $QCA1530_DIR
    chmod 750 $QCA1530_DIR
  fi

  if [ -d $QCA1530_DIR_EFS ]
  then
    log_debug "QCA1530: EFS directory exists"
  else
    log_debug "QCA1530: Creating EFS directory"
    mkdir -p $QCA1530_DIR_EFS || die "QCA1530: Failed to create EFS directory"
    chown gps:gps $QCA1530_DIR_EFS
    chmod 750 $QCA1530_DIR_EFS
  fi

  #make available the NV file for service to start
  if [ -f $QCA1530_CFG ]
  then
    log_debug "QCA1530: FSH File exists"
  else
    log_debug "QCA1530: FSH File doesn't exist. Copying file."
    cat $QCA1530_CFG_SRC > $QCA1530_CFG \
      || die "QCA1530: Failed to install FSH file"
    chown gps:gps $QCA1530_CFG
    chmod 600 $QCA1530_CFG
  fi

  if [ -f $QCA1530_CHIPSTATE ]
  then
    log_debug "Setting access rights to chip state control"
    chown root:gps $QCA1530_CHIPSTATE
    chmod 660 $QCA1530_CHIPSTATE
  fi
}

function qca1530_init
{
  while true
  do
    PROP=`getprop $QCA1530_PROP`
    case "$PROP" in
    yes)
      log_debug "QCA1530: starting the service"
      start gnss-svcd
      break
      ;;
    no)
      log_debug "QCA1530: chip is not available"
      break
      ;;
    *)
      log_verbose "QCA1530: waiting for detection result"
      sleep 1
      ;;
    esac
  done
}

function qca1530_detect
{
  log_debug "QCA1530: performing QCA1530 detection"
  echo 15 > $QCA1530_CHIPSTATE
  $QCA1530_EXE -fsh $QCA1530_CFG_SRC -detect-only $*

  PROP=`getprop $QCA1530_PROP`
  case $PROP in
  yes)
    log_debug "QCA1530: SoC detected"
    ;;
  no)
    log_debug "QCA1530: SoC is not detected or no matching firmware is available"
    ;;
  *)
    log_error "QCA1530: unrecognized SoC detection result"
    ;;
  esac
  echo 0 > $QCA1530_CHIPSTATE
}

function qca1530_start
{
  cd $QCA1530_DIR
  echo 15 > $QCA1530_CHIPSTATE
  exec $QCA1530_EXE -fsh $QCA1530_CFG,$QCA1530_CFG_SRC $*
}

function qca1530_reset
{
  if [ -w $QCA1530_RESET0 ]
  then
    log_debug "QCA1530 reset control is detected on driver"
    path=$QCA1530_RESET0
  else
    log_error "QCA1530 reset control file is not detected"
    exit 1
  fi

  case $1 in
  0)
    log_debug "Disabling QCA1530"
    echo 0 > ${path}
    ;;
  1)
    log_debug "Enabling QCA1530"
    echo 1 > ${path}
    ;;
  *)
    log_error "Bad argument"
    exit 1
    ;;
  esac
}

function qca1530_power
{
  case $1 in
  0)
    log_debug "Swiching QCA1530 power off"
    echo 0 > $QCA1530_CHIPSTATE
    ;;
  1)
    log_debug "Switching QCA1530 power on"
    echo 15 > $QCA1530_CHIPSTATE
    ;;
  *)
    log_error "Bad argument"
    exit 1
    ;;
  esac
}

if [ "$#" == "0" ]
then
  echo "Usage: $0 start|reset|detect|init|power args"
  exit 0
fi

case $1 in
start)
  shift
  qca1530_setup
  qca1530_start $*
  ;;
reset)
  shift
  qca1530_reset $*
  ;;
init)
  shift
  qca1530_init $*
  ;;
detect)
  shift
  qca1530_setup_reset
  qca1530_detect $*
  ;;
power)
  shift
  qca1530_setup_reset
  qca1530_power $*
  ;;
*)
  log_error "Invalid arguments: $*"
  ;;
esac

