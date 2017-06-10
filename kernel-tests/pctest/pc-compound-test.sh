#-----------------------------------------------------------------------------
# Copyright (c) 2009 Qualcomm Technologies, Inc.
# All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
#-----------------------------------------------------------------------------

#-----------------------------------------------------------------------------
# pc-compound-test.sh
# Shell script run on target. Runs power test after sequence of other
# DUT activities.
#-----------------------------------------------------------------------------

# Initialize environment
. $TEST_ENV_SETUP


### Global variables ###

# Set application/media locations
AUDIO_MEDIA_LOCATION="/data/media/audio"
VIDEO_MEDIA_LOCATION="/data/media/video"
SD_AUDIO_MEDIA_LOCATION="/sdcard/Audio"
SD_VIDEO_MEDIA_LOCATION="/sdcard/Video"
PCTEST_LOCATION="/data/kernel-tests/pctest"
SCREENTEST_LOCATION="/data/kernel-tests/screentest"
PM_STATS_NODE="/proc/msm_pm_stats"
DIR_KERNEL_TESTS="/data/kernel-tests"
SAVED_AUTOPC_SESSION_LOCATION="/data/kernel-tests/autoPC.sav"
GPS_APP_PROCESS_NAME="com.android.gps.sats"
PCTEST_LOG_FILE="/data/kernel-tests/pctest.log"

# Command parameters
AUDIO_PLAY_DURATION=5
VIDEO_PLAY_DURATION=5
ALARM_DURATION=40
WAIT_DURATION=2
CALL_TEST_DURATION=10
SMS_TEST_DURATION=3
INITIAL_SLEEP=5
SCREEN_TIMEOUT_SECS=30

# Location of the directory holding results logs
RESULTS_LOG_DIRECTORY="/data/kernel-tests/pctestlogs"

# Default delay - reduced
delay=2
autoSuspendDelay=40
sleepOverrideTime=5
autoSuspendSleepOverrideTime=30
timeout=10

# Command strings
AUDIO_PLAY_COMMAND="am start -n com.android.power.audio/.Media -e duration $AUDIO_PLAY_DURATION"
VIDEO_PLAY_COMMAND="am start -n com.android.power.video/.Media -e duration $VIDEO_PLAY_DURATION"
COMPLETE_AUDIO_PLAYBACK_COMMAND="am start -n com.android.power.audio/.Media"
SCREEN_TIMEOUT_TEST_COMMAND="am start -n com.android.power.screen/.Screen -e timeout"
ALARM_SET_COMMAND="am start -n com.android.alarmtest/.AlarmTest -e duration $ALARM_DURATION -e doSuspend no"
ALARM_LAUNCH_COMMAND="am start -n com.android.alarmtest/.AlarmTest -e duration 30 -e doSuspend no -e mode launcher"
GPS_TEST_COMMAND="am start -n com.android.gps.sats/.SatelliteStatus"
CALL_TEST_COMMAND="am instrument -q duration 10000 -e class com.android.voicecalltests.functional.VoicecallTest#testMOAcceptEndCall -w com.android.voicecalltests/.VoicecallInstrumentationTestRunner"
CALL_SUSPEND_TEST_COMMAND="am instrument -q duration 30000 -e class com.android.voicecalltests.functional.VoicecallTest#testMOAcceptEndCall -w com.android.voicecalltests/.VoicecallInstrumentationTestRunner"
SMS_TEST_COMMAND="am start -n com.android.smstest.power/.SMSTest"
CAMERA_TEST_COMMAND="am instrument -e class com.android.mediaframeworktest.stress.MediaRecorderStressTest#testStressCamera -w com.android.mediaframeworktest/.MediaRecorderStressTestRunner"

# Log file results messages
SUSPEND_SUCCESS="Suspend/resume succeeded"
SUSPEND_FAILURE="Suspend/resume test failed"
AUTO_SUSPEND_SUCCESS="Auto suspend/resume succeeded"
AUTO_SUSPEND_FAILURE="Auto suspend/resume test failed"
IDLE_SUCCESS="Idle power collapse succeeded"
IDLE_FAILURE="Idle power collapse failed"

# Options
optAudio=1
optVideo=1
parameter=0
errorCode=0
errorString=""
noTCXO=0

testSequenceFile="testSequence.dat"
testSequence="audio video suspend"
executedSequence=""

suspendPCTestResults=""
autoSuspendPCTestResults=""
idlePCTestResults=""
resultLogsPath=""
pcResultNumber=0

### Functions ###

# Collect idle power collapse statistics
collectIdlePCStats()
{
    # Set the mode correctly
    if [ -e "/sys/module/pm/parameters/idle_sleep_mode" ]; then
	echo "1" > "/sys/module/pm/parameters/idle_sleep_mode"
    fi

    if [ -e "/sys/module/pm2/parameters/idle_sleep_mode" ]; then
	echo "1" > "/sys/module/pm2/parameters/idle_sleep_mode"
    fi

    echo "in idle pc stats"

    if [ -e $SAVED_AUTOPC_SESSION_LOCATION ]; then
	echo "Tryign to run" > /data/kernel-tests/retrieved.sav

        # Read the saved session file to compute idle PC success ratio.
	uptimeInitial=`cat $SAVED_AUTOPC_SESSION_LOCATION`

	echo "Got $uptimeInitial" > /data/kernel-tests/retrieved.sav

	# Collect stats
	uptimeFinal=`cat /proc/uptime | awk '{print $1}'`
	uptimeFinal=`echo $uptimeFinal | awk -F "." '{print $1}'`
	idlePCTotalTime=`cat /proc/msm_pm_stats | grep -A 2 "idle-power-collapse:" | tail -n 1 | awk '{print $2}' | awk -F "." '{print $1}'`
	idlePCCount=`cat /proc/msm_pm_stats | grep -A 2 "idle-power-collapse:" | tail -n 2 | head -n 1 | awk '{print $2}' | awk -F "." '{print $1}'`
	idlePCFailedCount=`cat /proc/msm_pm_stats | grep -A 2 "idle-failed-power-collapse:" | tail -n 2 | head -n 1 | awk '{print $2}' | awk -F "." '{print $1}'`
	uptimeDiff=`expr $uptimeFinal - $uptimeInitial`
        # Extract the integral part.
	uptimeDiff=`echo $uptimeDiff | awk -F "." '{print $1}'`
	idlePCTimeRatio=`expr $idlePCTotalTime \* 100 \/ $uptimeDiff`

	# Write result out to summary logs.
	if [ -d $resultLogsPath ]; then
	    echo "Measured time span: " $uptimeDiff " seconds" > "$resultLogsPath/summary.log"
	    echo "Measured time spend in idle power collapse: " $idlePCTotalTime " seconds" >> "$resultLogsPath/summary.log"
	    echo "Idle power collapse count: " $idlePCCount >> "$resultLogsPath/summary.log"
	    echo "Percentage of time spend in idle power collapse state: " $idlePCTimeRatio "%" >> "$resultLogsPath/summary.log"
	    echo "Idle power collapse failed count: " $idlePCFailedCount >> "$resultLogsPath/summary.log"
	    cp /proc/msm_pm_stats $resultLogsPath
	fi
    else
	if [ -e $PM_STATS_NODE ]; then
            # Reset PM stats read the current uptime value
	    echo "reset" > $PM_STATS_NODE
	    uptimeInitial=`cat /proc/uptime | awk '{print $1}'`
	    uptimeInitial=`echo $uptimeInitial | awk -F "." '{print $1}'`

	    # Write the current value of uptimeInitial to the session file.
	    echo -n $uptimeInitial > $SAVED_AUTOPC_SESSION_LOCATION

	    # Set the alarm to go off in ~30s and relaunch the automation script
	    `$ALARM_LAUNCH_COMMAND > /dev/null`

	    toggleKey

            # Get audio/video playing (playAudioClip will do the sleeping)
            #playAudioClip
	    #sleep 30
	fi
    fi
}

# Display usage info
displayHelp()
{
    echo "Usage: auto2.sh [-f=<sequence file>]"
}

# Display error info and exit
displayErrorInfoAndExit()
{
    if [ -d $resultLogsPath ]; then
	`echo $errorString > $resultLogsPath/summary.log`
    fi

    exit $errorCode
}

# Cleans up the results log directory
cleanUpResultsLogFiles()
{
    if [ -d $RESULTS_LOG_DIRECTORY ]; then
# adb rm comes first in path and does not support -rf
# explicitly use busybox rm
	busybox rm -rf $RESULTS_LOG_DIRECTORY/* > /dev/null
    fi
}

# Create and set the result logs directory
setupResultsLogDirectory()
{
    currentTime=`date | tr [:space:] _`
    resultLogsPath="$RESULTS_LOG_DIRECTORY"

    # Create directory if it doesn't already exist
    if [ ! -d "$resultLogsPath" ]; then
# adb mkdir comes first in path and does not support -p
# explicitly use busybox mkdir
	`busybox mkdir -p $resultLogsPath`


    fi

    if [ -d "$resultLogsPath" ]; then
	echo "Overall test result: Failure" > $resultLogsPath/summary.log
    fi

    if [ ! -d "$resultLogsPath" ]; then
	errorCode=127
	errorString="Unable to create $resultLogsPath."

	displayErrorInfoAndExit
    fi
}

# Set screen to never time out
disableScreenTimeout()
{
    `$SCREEN_TIMEOUT_TEST_COMMAND -1 > /dev/null`

    `input keyevent 82`
    `input keyevent 82`
}

# Set a timeout for the screen
setScreenTimeout()
{
    screenTimeoutValue=`expr $SCREEN_TIMEOUT_SECS \* 1000`

    # Set a 30 sec timeout.
    `$SCREEN_TIMEOUT_TEST_COMMAND $screenTimeoutValue > /dev/null`

    `input keyevent 82`
    `input keyevent 82`
}

# Toggle the '1' key on the FFA
toggleKey()
{
    `input keyevent 82`
    `input keyevent 82`
}

# Log latest results (available in the current PCtest-generated logfile)
logLatestResults()
{
    retCode=0

    if [ -e $PCTEST_LOG_FILE ]; then
	# pctest.log exists. Read it and get the result status.
	# The type of test depends on the input argument.
	if [ $1 = "suspend" ]; then
	    grep "Suspend/resume succeeded" $PCTEST_LOG_FILE
	    retCode=$?

	    # Check return code to determine success
	    if [ $retCode = "0" ]; then
		suspendPCTestResults="$suspendPCTestResults success"
	    elif [ $retCode = "1" ]; then
		suspendPCTestResults="$suspendPCTestResults failure"

		allTestsOk=0
	    fi
	elif [ $1 = "idle" ]; then
	    cat "$PCTEST_LOG_FILE" | grep "Idle\ power\ collapse\ succeeded"
	    retCode=$?
	    echo $retCode >> /data/myfile
	    # Check return code to determine success
	    if [ $retCode = "0" ]; then
		idlePCTestResults="$idlePCTestResults success"
	    else
		idlePCTestResults="$idlePCTestResults failure"
	    fi
	elif [ $1 = "autosuspend" ]; then
	    cat "$PCTEST_LOG_FILE" | grep "Auto\ suspend/resume\ succeeded"
	    retCode=$?

	    # Check return code to determine success
	    if [ $retCode = "0" ]; then
		autoSuspendPCTestResults="$autoSuspendPCTestResults success"
	    elif [ $retCode = "1" ]; then
		autoSuspendPCTestResults="$autoSuspendPCTestResults failure"
	    fi
	fi

	# Move pctest.log to the results log directory. Change its name too.
	if [ -d  $resultLogsPath ]; then
	    pcResultNumber=`expr $pcResultNumber + 1`
	    `cp $PCTEST_LOG_FILE $resultLogsPath/pctest.log.$pcResultNumber`
	fi

	# If the test failed, generate the summary log and exit.
	if [ ! $retCode = "0" ]; then
	    generateTestSummary
	    exit 1
	fi
    fi
}

# Generate summary of tests
generateTestSummary()
{
    suspendSuccessCount=0
    suspendFailureCount=0
    autoSuspendSuccessCount=0
    autoSuspendFailureCount=0
    idleSuccessCount=0
    idleFailureCount=0

    # Suspend summary
    for suspendPCTestResult in $suspendPCTestResults
    do
      if [ $suspendPCTestResult = "success" ]; then
	  suspendSuccessCount=`expr $suspendSuccessCount + 1`
      elif [ $suspendPCTestResult = "failure" ]; then
	  suspendFailureCount=`expr $suspendFailureCount + 1`
      fi
    done

    # Idle PC test summary
    for idlePCTestResult in $idlePCTestResults
    do
      if [ $idlePCTestResult = "success" ]; then
	  idleSuccessCount=`expr $idleSuccessCount + 1`
      elif [ $idlePCTestResult = "failure" ]; then
	  idleFailureCount=`expr $idleFailureCount + 1`
      fi
    done

    # Auto-suspend summary
    for autoSuspendPCTestResult in $autoSuspendPCTestResults
    do
      if [ $autoSuspendPCTestResult = "success" ]; then
	  autoSuspendSuccessCount=`expr $autoSuspendSuccessCount + 1`
      elif [ $autoSuspendPCTestResult = "failure" ]; then
	  autoSuspendFailureCount=`expr $autoSuspendFailureCount + 1`
      fi
    done

    # Write results to file
    suspendSummary="Suspend results: Success: $suspendSuccessCount time(s) ; Failure: $suspendFailureCount time(s)"
    autoSuspendSummary="Auto-suspend results: Success: $autoSuspendSuccessCount time(s) ; Failure: $autoSuspendFailureCount time(s)"
    idleSummary="Idle-PC results: Success: $idleSuccessCount time(s) ; Failure: $idleFailureCount time(s)"

    if [ -d $resultLogsPath ]; then
	if [ $suspendFailureCount = "0" ] && [ $autoSuspendFailureCount = "0" ] && [ $idleFailureCount = "0" ]; then
	    `echo Overall test result: Success > $resultLogsPath/summary.log`
	else
	    `echo Overall test result: Failure > $resultLogsPath/summary.log`
	fi

	`echo Sequence executed: $executedSequence >> $resultLogsPath/summary.log`
	`echo $suspendSummary >> $resultLogsPath/summary.log`
	`echo $autoSuspendSummary >> $resultLogsPath/summary.log`
	`echo $idleSummary >> $resultLogsPath/summary.log`
    fi
}

# Wait for process' termination
waitForProcessTermination()
{
    echo "in wait for ProcessTer..." > /data/kernel-tests/summary.log
    if [ -n "$1" ]; then
	processesRunning=`ps`
	strMatch=`echo "$processesRunning" | grep "$1"`
	echo $strMatch

	while [ -n "$strMatch" ]
	  do

	  sleep 1
	  echo "waiting" >> /data/kernel-tests/summary1.log
	  processesRunning=`ps`
	  ps | grep "$1"

	  if [ ! $? = "0" ]; then
	      # Process not found in ps's output
	      strMatch=""
	      echo "done" >> /data/kernel-tests/summary1.log
	  else
	      strMatch="found"
	  fi

	done
    fi

    echo "Process dead" > /data/kernel-tests/summary1.log
}

# Termination process
killProcess()
{
    if [ -n "$1" ]; then
	processesRunning=`ps`
	strMatch=`echo "$processesRunning" | grep "$1"`

	while [ -n "$strMatch" ]
	  do
	  sleep 1
	  processesRunning=`ps`
	  strMatch=`echo "$processesRunning" | grep "$1"`

	  if [ -n "$strMatch" ]; then
	      # ps | grep $1
	      pidToKill=`ps | grep $1 | awk '{print $1}'`
	      echo  "Killing $pidToKill"
	      kill -9 $pidToKill
	  fi
	done
    fi
}

# Suspend
suspend()
{

    # Perform basic tests
    if [ ! -e $PCTEST_LOCATION ]; then
	errorString="$PCTEST_LOCATION doesn't exist."
	errorCode=127
	displayErrorInfoAndExit
    fi

    if [ ! -x $PCTEST_LOCATION ] || [ ! -r $PCTEST_LOCATION ]; then
	errorString="$PCTEST_LOCATION doesn't have execute or read permissions."
	errorCode=127
	displayErrorInfoAndExit
    fi

    # Disable SD card polling
    for file in /sys/devices/platform/msm_sdcc*/polling; do echo 0 > $file; done;

    # Delete log file
    if [ -e $PCTEST_LOG_FILE ]; then
	`rm $PCTEST_LOG_FILE`
    fi

    toggleKey
    `input keyevent 20`
    `input keyevent 20`

    #Run the suspend command
    tcxoOption=""

    if [ $noTCXO = "1" ]; then
	tcxoOption="-tcxo"
    fi

    echo "TCXO: $tcxoOption"
    $PCTEST_LOCATION suspend $delay $sleepOverrideTime $timeout $tcxoOption > $PCTEST_LOG_FILE
    #waitForProcessTermination $PCTEST_LOCATION
}

# Do an auto-suspend
autoSuspend()
{
    # Perform basic tests
    if [ ! -e $PCTEST_LOCATION ]; then
	errorString="$PCTEST_LOCATION doesn't exist."
	errorCode=127
	displayErrorInfoAndExit
    fi

    if [ ! -x $PCTEST_LOCATION ] || [ ! -r $PCTEST_LOCATION ]; then
	errorString="$PCTEST_LOCATION doesn't have execute or read permissions."
	errorCode=127
	displayErrorInfoAndExit
    fi

    # Disable SD card polling
    for file in /sys/devices/platform/msm_sdcc*/polling; do echo 0 > $file; done;

    # Delete log file
    if [ -e $PCTEST_LOG_FILE ]; then
	`rm $PCTEST_LOG_FILE`
    fi

    # Set screen timeout to 30 secs and let the system idle-suspend.
    autoSuspendSleepOverrideTime=`expr $SCREEN_TIMEOUT_SECS - $delay + 5`
    setScreenTimeout

    chmod 777 /sys/power/state

    $ALARM_SET_COMMAND > /dev/null

    `input keyevent 20`
    `input keyevent 20`

    tcxoOption=""

    if [ $noTCXO = "1" ]; then
	tcxoOption="-tcxo"
    fi

    $PCTEST_LOCATION autosuspend $delay $autoSuspendSleepOverrideTime $timeout $tcxoOption > $PCTEST_LOG_FILE
    #waitForProcessTermination $PCTEST_LOCATION

    # Disable screen timeout again for future tests.
    echo "Disable screen timeout"
    disableScreenTimeout
}

# Idle PC
invokeIdlePowerCollapse()
{
    # Perform basic tests
    if [ ! -e $PCTEST_LOCATION ]; then
	errorString="$PCTEST_LOCATION doesn't exist."
	errorCode=127
	displayErrorInfoAndExit
    fi

    if [ ! -x $PCTEST_LOCATION ] || [ ! -r $PCTEST_LOCATION ]; then
	errorString="$PCTEST_LOCATION doesn't have execute or read permissions."
	errorCode=127
	displayErrorInfoAndExit
    fi

    # Disable SD card polling
    for file in /sys/devices/platform/msm_sdcc*/polling; do echo 0 > $file; done;

    if [ -e "/sys/module/pm/parameters/idle_sleep_mode" ]; then
	echo "1" > "/sys/module/pm/parameters/idle_sleep_mode"
    fi

    if [ -e "/sys/module/pm2/parameters/idle_sleep_mode" ]; then
	echo "1" > "/sys/module/pm2/parameters/idle_sleep_mode"
    fi


    # Simulate down keypresses to make sure the Google search bar doesn't have the focus
    `input keyevent 20`
    `input keyevent 20`
    `input keyevent 20`

    #Run the suspend command
    $PCTEST_LOCATION idle-pc $delay $timeout > $PCTEST_LOG_FILE &

    waitForProcessTermination $PCTEST_LOCATION
}

activateCamera()
{
    $CAMERA_TEST_COMMAND > /dev/null

    if [ ! $? = "0" ]; then
	$errorCode=1
	$errorString="Unable to run camera test."

	displayErrorInfoAndExit
    fi

    sleep 1
}

activateBluetooth()
{
    echo "Y" > /sys/module/bluetooth_power/parameters/power

    if [ ! $? = "0" ]; then
	$errorCode=1
	$errorString="Unable to write to /sys/module/bluetooth_power/parameters/power."

	displayErrorInfoAndExit
    fi

    hci_qcomm_init -s 115200 -d /dev/ttyHS0 œôòóvvv

    if [ ! $? = "0" ]; then
	$errorCode=1
	$errorString="Unable to write to run hci_qcomm_init."

	displayErrorInfoAndExit
    fi

    hciattach /dev/ttyHS0 any 115200

    if [ ! $? = "0" ]; then
	$errorCode=1
	$errorString="Unable to write to run hciattach."

	displayErrorInfoAndExit
    fi

    hciconfig hci0 up

    if [ ! $? = "0" ]; then
	$errorCode=1
	$errorString="Unable to write to run hciconfig."

	displayErrorInfoAndExit
    fi
}

# Suspend immedately after and suspend (do not look for tcxo while determining test status)
makeVoiceCallAndSuspend()
{
    # Run in background so that we can invoke suspend (almost) immediately afterwards.
    `$CALL_SUSPEND_TEST_COMMAND > /dev/null &`

    if [ ! $? = "0" ]; then
	$errorCode=1
	$errorString="Unable to run call-suspend test."

	displayErrorInfoAndExit
    fi

    # Test started. Wait for 5+ (the test APK waits for 5 seconds before actually connecting) seconds,
    # while the phone makes the call, before suspending.
    callTerminationWait=0

    while [ $callTerminationWait -lt 7 ]; do
	input keyevent 19
	callTerminationWait=`expr $callTerminationWait + 1`
	sleep 1
    done

    # Send a key event to bring the screen back
    echo "on" > /sys/power/state

    # Suspend. Before suspending, save the noTCXO flag. This is to tell
    # pcTest to not look for a TCXO when deciding whether a test failed or not.
    saveNoTCXO=$noTCXO
    saveDelay=$delay
    noTCXO=1
    delay=1
    suspend
    logLatestResults suspend
    noTCXO=$saveNoTCXO
    delay=$saveDelay

    # We now need to wait a little longer to make sure the call is no longer active.
    # During this period, keystrokes will have to be sent to the device to keep it from suspending.
    callTerminationWait=0

    echo "About to do keyed-wait" > /data/kernel-tests/summary.log

    while [ $callTerminationWait -lt 20 ]; do
	input keyevent 19
	callTerminationWait=`expr $callTerminationWait + 1`
	sleep 1
    done

    # Since the phone can suspend when the call is active, turn it back on.
    # Send a couple of random keystrokes.
    echo "on" > /sys/power/state
    echo "hello" > /data/kernel-tests/summary.log
    input keyevent 19
    sleep 1
    input keyevent 3
}

makeVoiceCall()
{
    `$CALL_TEST_COMMAND > /dev/null`

    if [ ! $? = "0" ]; then
	$errorCode=1
	$errorString="Unable to run call test."

	displayErrorInfoAndExit
    fi

    # Test was successful.
    sleep $CALL_TEST_DURATION
    sleep 2

    # No need to kill the process. It cleans up by itself.
}

sendSMS()
{
    `$SMS_TEST_COMMAND > /dev/null`

    if [ ! $? = "0" ]; then
	$errorCode=1
	$errorString="Unable to run SMS test."

	displayErrorInfoAndExit
    fi

    # Test was successful.
    sleep $SMS_TEST_DURATION
    sleep 2

    # No need to kill the process. It cleans up by itself.
}

increaseVolume()
{
    input keyevent 24
}

decreaseVolume()
{
    input keyevent 25
}

activateGPS()
{
    `$GPS_TEST_COMMAND > /dev/null`

    if [ ! $? = "0" ]; then
	$errorCode=1
	$errorString="Unable to run GPS test."

	displayErrorInfoAndExit
    fi

    # Test was successful.
    sleep 2

    # Kill the GPS app and move ahead
    killProcess $GPS_APP_PROCESS_NAME
}

# play audio clip
playAudioClip()
{
    # Perform basic tests
    if [ ! -s $AUDIO_MEDIA_LOCATION ] && [ ! -s $SD_AUDIO_MEDIA_LOCATION ]; then
	errorString="$AUDIO_MEDIA_LOCATION and $SD_AUDIO_MEDIA_LOCATION don't exist or are empty."
	errorCode=127
	displayErrorInfoAndExit
    fi

    # Run audio play command
    $AUDIO_PLAY_COMMAND &

    # Sleep a few seconds longer to make sure the clip has ended.
    sleep `expr $AUDIO_PLAY_DURATION + 3`

    echo "Audio clip complete."
}

# Play audio clip
playVideoClip()
{
    # Perform basic tests
    if [ ! -s $VIDEO_MEDIA_LOCATION ] && [ ! -s $SD_VIDEO_MEDIA_LOCATION ]; then
	errorString="$VIDEO_MEDIA_LOCATION and $SD_VIDEO_MEDIA_LOCATION don't exist or are empty."
	errorCode=127
	displayErrorInfoAndExit
    fi

    # Run audio play command
    `$VIDEO_PLAY_COMMAND > /dev/null`

    # Sleep a few seconds longer to make sure the clip has ended.
    sleep `expr $VIDEO_PLAY_DURATION + 3`

    echo "Video clip complete."
}

# Turn screen on/off
setScreenPower()
{
    if [ -n $1 ]; then
	if [ $1 = "on" ]; then
	    `$SCREENTEST_LOCATION on > /dev/null`
	elif [ $1 = "off" ]; then
	    `$SCREENTEST_LOCATION off > /dev/null`
	else
	    errorCode=127
	    errorString="Incorrect argument to $SCREENTEST_LOCATION."
	    displayErrorInfoAndExit
	fi
    fi
}

# Turn SD card polling off
turnSDCardPollingOff()
{
    for file in /sys/devices/platform/msm_sdcc*/polling; do echo 0 > $file; done;
}


### Script code ###

mode="pctest"
alarmLaunched=0

for option in $*
do
  if echo $option | grep -q "\-h"; then
      displayHelp
      exit 0
  elif echo $option | grep -q "\-al"; then
      alarmLaunched=1
  elif echo $option | grep -q "\-f=.*"; then
      testSequenceFile=`echo $option | awk -F = '{print $2}'`
  elif echo $option | grep -q "\-m=.*"; then
      mode=`echo $option | awk -F = '{print $2}'`
  elif echo $option | grep -q "\-tcxo"; then
      noTCXO=1
  fi
done

if [ $alarmLaunched = "0" ]; then
    if [ -e $SAVED_AUTOPC_SESSION_LOCATION ]; then
	rm $SAVED_AUTOPC_SESSION_LOCATION
    fi
fi

if [ -e "$testSequenceFile" ]; then
    testSequence=`cat $testSequenceFile`
fi

# Disable screen timeout
disableScreenTimeout

# Remove old log files
cleanUpResultsLogFiles

# Set up the directory for all logs.
setupResultsLogDirectory

if [ $alarmLaunched = "0" ]; then
    echo "Please pull out the USB cable now."
    sleep $INITIAL_SLEEP
fi

if [ $mode = "idlestats" ]; then
    echo "Yet to implement." >> /data/kernel-tests/retrieved.sav

    collectIdlePCStats
elif [ $mode = "pctest" ]; then
    # Run through the test sequence.
    for testToPerform in $testSequence
    do
        # Record the sequence executed so far. This information
        # will be used at the time of outputting summary logs.
	executedSequence="$executedSequence $testToPerform"

	if [ $testToPerform = "audio" ]; then
	    echo "Playing audio."
	    playAudioClip
	elif [ $testToPerform = "video" ]; then
	  echo "Playing video."
	  playVideoClip
	elif [ $testToPerform = "suspend" ]; then
	    echo "Suspending."
	    `echo usb_bus_active > /sys/power/wake_unlock`
	    suspend
	    logLatestResults suspend
	elif [ $testToPerform = "autosuspend" ]; then
	    echo "Auto-suspending."
	    `echo usb_bus_active > /sys/power/wake_unlock`
	    autoSuspend
	    logLatestResults autosuspend
	elif [ $testToPerform = "idle-pc" ]; then
	    echo "Idle-PC."
	    `echo usb_bus_active > /sys/power/wake_unlock`
	    invokeIdlePowerCollapse
	    logLatestResults idle
	elif [ $testToPerform = "gps" ]; then
	    echo "Power on GPS."
	    activateGPS
	elif [ $testToPerform = "call" ]; then
	    echo "Making call."
	    makeVoiceCall
	elif [ $testToPerform = "call-suspend" ]; then
	    echo "Doing call-suspend."
	    makeVoiceCallAndSuspend
	elif [ $testToPerform = "sms" ]; then
	    echo "Send SMS."
	    sendSMS
	elif [ $testToPerform = "bluetooth-on" ]; then
	    echo "Turn on Bluetooth."
	    activateBluetooth
	elif [ $testToPerform = "camera" ]; then
	    echo "Turn on camera."
	    activateCamera
	elif [ $testToPerform = "screenoff" ]; then
	    setScreenPower off
	elif [ $testToPerform = "screenon" ]; then
	    setScreenPower on
	elif [ $testToPerform = "wait" ]; then
	    sleep $WAIT_DURATION
	else
	    echo "Skipping action $testToPerform."
	fi
    done

    echo "fsdfsd" > /data/kernel-tests/summary.log

    # Generate the test summary.
    generateTestSummary
fi

exit 0
