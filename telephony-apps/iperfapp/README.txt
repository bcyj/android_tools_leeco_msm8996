INSTRUCTIONS TO USE IPERFAPP:
----------------------------

Before launching iperfapp from the UI do the following:

1. Download iperf source from here: http://sourceforge.net/projects/iperf/

2. Compile it for Qualcomm ARM Targets with any ARM compiler.

3. Push the compiled iperf executable under /data/data/com.android.iperfapp

4. Give iperf executable permissions as chmod 777 /data/data/com.android.iperfapp/iperf

ADDITIONAL INFORMATION:
----------------------
1. If a list of predefined parameters for iperf are to be provided to the app, then a config.txt can be pushed to:

/data/data/com.android.iperfapp/config/config.txt

Config.txt can contain the following text:

-c 10.2.5.6 -t 100000 -i 2
-s -u -i 2

and so on...

2. After launching the app from application manager you would be provided with a window with an input box to enter the iperf

parameters(The default is "-s -u -2" - meaning it would run as server in UDP mode and send packets every 2 secs).

3. You will also see 4 options:

    --Clean: This cleans the output logs saved by iPerfApp. By default all output logs are stored under /data/data/com.android.iperfapp/files.

    --FileOn/FileOff: This option toggles writing of output log to file.

    --DispOn/DispOff: This option toggles writing of output to display.

    --Run: This option runs the application. It toggles to "Stop" to stop running iperf.

4. You will also see a message at the top of the screen with the device IP address.

5. If you click on Menu you get 2 options:

    --About: This gives the current version of the app.

    --Saved Commands: This is used to choose the predefined parameters for iperfapp installed through config.txt.

NOTES:
-----

1. Sometime when you press run you may not see any results on the display. You can click stop and run again.

2. When you enable DispOn or LogOn there may be an increase in CPU cycles leading to additional power consumption.

3. For bidirectional data transfer, the data transfer for both Uplink and downlink is displayed in the same command prompt.

4. When you click on iPerf the first time, if it doesnt run then press clean and then run again.

DEBUGGING ANY EMBEDDED DATA CALL ISSUES:
---------------------------------------

1. First check whether your network connection is active and that the device can send and receive data packets.

2. Go to adb shell and type #netcfg. Check if rmnet0 is UP. If not check your Anritsu/Agilent/Aeroflex box!

3. If rmnet0 is up, ping <ip address of the host PC (where your box is connected). If it is not reachable, check the box again!

4. If the IP is reachable, and you dont see data transfer happening, then you can run tcpdump to figure out what is happening to the packets sent and received.

5. For running raw iperf executable, do the following in adb shell:

   --cd /data/data/com.android.iperfapp

   --./iperf <parameters> > /data/com.android.iperfapp/log.txt &

6. The above command will execute raw iperf executable and will save the logs in the above location.
   This is useful for ruling out any iperfApp related issues.
