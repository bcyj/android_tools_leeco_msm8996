/*
 * ----------------------------------------------------------------------------
 *  Copyright (c) 2012-2013 Qualcomm Technologies, Inc.  All Rights Reserved.
 *  Qualcomm Technologies Proprietary and Confidential.
 *  ---------------------------------------------------------------------------
 *
 */

The event sniffer tool (evt-sniff) is a Linux command line based executable which
runs on the target. It accepts configuration data from the user and executes a
specified set of tests, captures the touch events and measures the performance
of the touch controller. Below are the features of the tool --

	1. It is a multi-threaded and light-weight tool. It emulates the
	userspace behavior by maintaining dedicated threads to read and
	process touch data, thus eliminating the possibility of missing
	touch events.

	2. Developed to measure and characterize the performance of the touch
	controller by capturing touch events at the kernel-userspace boundary.

	3. Prompts the user to perform certain specific operations on the
	screen and captures the generated touch events. Analyzes these events
	for sample rate, uniformity and jitter. Displays the results, analysis
	and sanity of the test on the console.

	4. Independent of the touch protocol supported by the driver. It
	dynamically detects support for single-touch or multi-touch
	(protocol A and protocol B).

	5. Determines the touch pixel to display pixel mapping (scaling factor)
	and reports results based on this.

	6. Has an option to specify the display measurements (in mm) via the
	config file. These measurements enable the data to be analyzed and
	characterized effectively.

	7. Logs every test and facilitates comparing results by allowing the
	user to select the input file for comparison.


Tool Usage and Location

Tool's Location: /data/evt-test/evt-sniff
Tool's Documnetation: vendor/qcom/proprietary/ts_firmware/ts_tools/Readme.txt

If the tool is explicitly received as a binary executable follow the below steps
to execute the tool on the device.

	1. Create a evt-test folder on the device - adb shell mkdir /data/evt-test
	2. Copy to the tool to the target - adb push evt-sniff /data/evt-test/
	2. Change the permissions of the tool - adb shell chmod 777 /data/evt-test/evt-sniff
	3. Execute  ./evt-sniff <options>

If you are unable to create a folder /data/evt-test/* or have problems executing
from the /data/evt-test/evt-sniff, move the tool to a location (in /data) which
has rwx permissions.

Note: This tool must be run as root.

The user options (arguments) for the tool are described below.

	1. --info, -i	This option displays information of the touch controller
			and the display parameters of the target. The details
			reported by this option include -
			(a) Touch screen -- Device node of the controller,
					driver name, Single/Multi touch
					support, Protocol A / B support,
					X/Y min/max range.
			(b) Display -- Display X and Y resolution
			(c) Touch to Display mapping -- Scaling factor of touch to display.

			Sample Output:
			./evt-sniff -i

			<Device No.> : <Touch Device Path> : <Touch Device Name>
			<0>          :</dev/input/event6>  :<gesture_mouse>
			<1>          :</dev/input/event5>  :<gesture_touch>
			<2>          :</dev/input/event2>  :<cyttsp-i2c>
			Enter device number to debug the touch device:2
			The selected touch device is :
			/dev/input/event1 : cyttsp-i2c

			CFG file : evt-sniff.cfg

			** TOUCH DEVICES **
	                Device Node = /dev/input/event1, Driver Name = cyttsp-i2c
	                Multi-Touch supported = Yes
		        Touch Protocol = A
			X minimum = 0, X maximum = 616
			Y minimum = 16, Y maximum = 1023
		        ** DISPLAY INFO **
	                X-Resolution in pixels  = 600
		        Y-Resolution in pixels  = 1024
			** TOUCH to DISPLAY MAPPING **
	                In X-Direction
			        Touch to Display (Pixel ratio)  = 1.03 : 1
		                Screen Length (mm)              = 52
				Display Pixels per mm           = 11.54
				Touch Pixels per mm             = 11.87
	                In Y-Direction
				Touch to Display (Pixel ratio)  = 0.98 : 1
				Screen Length (mm)              = 89
				Display Pixels per mm           = 11.51
				Touch Pixels per mm             = 11.33

	2. --config, -c  <cfg file>	This option facilitates the user to specify
					the configuration file. The default configuration
					file used by the tool is "evt-sniff.cfg" which is
					located in the same location as the tool. The user
					can modify the config parameters in this file.
					There is also an option to provide a separate
					config. file by pushing that file on the target.
					The parameters specifed in the CFG file are --

					1. PPIX -- Pixels per inch in X-Direction of the display.
					2. PPIY -- Pixels per inch in Y-Direction of the display.
					2. XSIZE and YSIZE -- Display size (physical) in mm
					   These can be specified if PPIX and PPIY is not available.
					3. XRES and YRES -- Display resolution in pixels
					   These are required if the tool is unable to read
					   these values from the device.
					4. TIME -- Time in seconds for which the test is executed.

					NOTE: The format for specifying the cfg. info is
					Parameter<space>=<space>Value

						PPIX = 160
						PPIY = 160
						# XSIZE = 77  // commented
						# YSIZE = 100
						# TIME = 30

					The tool either needs XSIZE and YSIZE or PPI,
					in order to give accurate results.

	3. --time, -t  <seconds>	This option specifies the test run time. The tests
					supported by this tool are associated with an
					execution (run) time. The tool collects the touch
					data for each test for this specific time. If this
					option is not specified, the time defaults to 15
					seconds. This time specified here is in seconds
					with minimum = 1 and maximum = 60.  Time specified
					via this option overwrites the time specified through
					the config file.

	4. --uniformity, -u		This option is to select the uniformity test.
					Specifying this option enables the tool to perform
					the Uniformity test. The tool prompts the user to
					perform a specific movement on the screen and captures
					the touch events during this time. These events are then
					analyzed for uniformity and the results are displayed on
					the console. More details regarding this test are specified
					in the Test specifics section.

					Sample Output:
					1) ./evt-sniff -u -t 5

					/dev/input/event1 : cyttsp-i2c

					CFG file : evt-sniff.cfg
					Log File created :	./logs.19700102-01-15-05.log
					CSV Log File created :  ./logs/19700102-01-15-05.csv
					Press a key to continue with the UNIFORMITY test

						***** TEST : UNIFORMITY / SAMPLE RATE ******
			                Move your finger SLOWLY in a STRAIGHT line for the next 5 seconds

			                Stopped collecting data, you can now stop moving your finger
			                Analysing Events.....

			                ****Uniformity Test Results*****

					Total Events Analysed = 159 in Y-Direction
				        Touch Event Delta        Number of Events	Distance in mm
			                        9                       31		0.795
			                        10                      23		0.883


	5. --sample rate, -s		This option is to select the sample rate test. Specifying
					this option enables the tool to perform the Sample rate test.
					The tool prompts the user to perform a specific movement on
					the screen and captures the touch events during this time.
					These events are then used to calculate the sample rate.
					The tool reports the max, min, average and the standard deviation
					of the sample rate. More details regarding this test are
					specified in the Test specifics section.

					Sample Output:
					1) ./evt-sniff -s -t 10

					<Device No.> : <Touch Device Path> : <Touch Device Name>
					<0>          :</dev/input/event1>  :<atmel_mxt_ts>
					<1>          :</dev/input/event3>  :<tch-emulator-A>
					<2>          :</dev/input/event5>  :<tch-emulator-B>
					Enter device number to debug the touch device:0
					The selected touch device is :
					/dev/input/event1 : atmel_mxt_ts

					CFG file : evt-sniff.cfg
						        Log File created :	./logs.19700102-01-15-05.log
						        CSV Log File created :  ./logs/19700102-01-15-05.csv

					        ***** TEST : UNIFORMITY / SAMPLE RATE ******
					Move your finger across the screen for the next 10 seconds
			                Stopped collecting data, you can now stop moving your finger
			                Analysing Events.....
			                ****Sample Rate Test Results*****
				                Total events analyzed = 1662
				                Minimum Sample Rate (events per second)         = 13
				                Maximum Sample Rate (events per second)         = 70
				                Average Sample Rate (events per second)         = 60
				                Standard Deviation (deviation wrt average)      = 13

			                Expected : Average Sample Rate between 60 and 90
			                Result  : Sample rate is 60 and in the expected range

				        ----Sample Rate Spread---
			                Sample Rate             Number of events(with specified rate)
				                13                      1
			                        17                      2
			                        22                      1
			                        34                      71
			                        64                      47
			                        68                      283

					2) ./evt-sniff -s -t 10 -u

						Error : Invalid combination of options
						Please follow the event sniffer tool manual.

	6. --jitter, -j			This option is to select the jitter test. Specifying
					this option enables the tool to perform the jitter test.
					The tool prompts the user to touch a specific location
					on the screen multiple ( > 10) times. It then analyses
					the captured data for jitter and displays the results on
					the console.
					Note: For this test to give accurate results, some kind of
					pointed device needs to be fixed to the display and that
					pointed device needs to touch the display at exact same
					place every single time during the test.

	7. --diff , -d <logfile>	This option specifies the log file to compare with the
					current experiment. The user can specify a log file to
					compare against the current test results. Only results
					from Uniformity and Sample Rate tests can be used for
					comparison. The difference in the results is displayed
					on the console. If the version of the tool and the specified
					log-file does not match the results will not be compared.
					This option should be used in conjunction with -u or -s option.

					Sample output:
						./evt-sniff -s -t 5 -d 19700102-00-47-04.log

					Press a key to continue to COMPARE RESULTS with the LOG-FILE =
									/data/evt-test/19700102-00-47-04.log
				        ***** RESULTS COMPARISON ******
		                                ------SAMPLE RATE TEST COMPARISON-----
										    CURRENT RUN	    LOG FILE
			                Maximum (events per second)                     70		17
					Minimum (events per second)                     12		1
				        Average (events per second)                     64		6
			                Standard Deviation (events per second)          10		4


	8. --version, -v		This option indicates the version of the tool. This option also
					lists down the new features added in this this version. Every
					generated logfile displays the version of the tool which has
					generated the file. The diff. option (7) will work only if the
					current version and the log version match.

					Sample Output:
						./evt-sniff -v

					        Evt-Sniff Tool Current Version Number = 1.2
				                New Features
					                -v1.0
								1. Version Infomation
								2. CSV logging
								3. Sample Rate Spread
							-v1.01
								1. -n option (device node)
								2. Relative path logging (./logs/*)
							-v1.1
								1. Consecutive(2) event delta analysis
								2. On-the-fly event analysis (-r option)
							-v1.1.1
								1. Re-enabled the Jitter test
								2. Fixed Segmentation fault while running sample rate
							-v1.2
								1. Multi touch logging support with protocol A, B (-l option)
								2. On-the-fly event analysis with protocol A, B
								3. Point accuracy metric measurements with multitouch protocol
								4. Spatial X line jitter test
								5. Spatial Y line jitter test
								6. Temporal jitter test
								7. Enumerate all touch devices and allow the user to choose one

	9. --node, -n <device-node>	This option adds ability to specify a specific device-node
					to the tool. The tool validates this node (for touch capability)
					and then reads events only from this node (it does not scan
					for any other touch-controllers on the device). This is useful
					if there are multiple touch-devieces (or touch-emulators) present
					in the system.

					Sample Output:
					1) ./evt-sniff -i -n /dev/input/event1

						** TOUCH DEVICES **

				        Device Node = /dev/input/event1, Driver Name = cyttsp-i2c
					Multi-Touch supported = Yes
				        Touch Protocol = A
				        X minimum = 0, X maximum = 616
				        Y minimum = 16, Y maximum = 1023

					No test selected : Exiting

					2) ./evt-sniff -i -n /dev/input/event2

						The specifed device node (/dev/input/event2)
						does not exist OR is not a touch device node

	10. --multitouch_log, -l	This option is to read multi finger touch information and log that information.
					As per multitouch protocol if it follows protocol A, then the logged format is
					Multitouch protocol A:
						Time stamp, X(px), Y(px)

					If it follows multitouch protocol B, then the logged file will be in this format
					Multi touch protocol B:
						Time stamp, X(px), Y(px), TrackingID, SlotID, Sample rate

					Multitouch logging time and multitouch output log file name is the command line
					argument for this option. Multitouch logging time is mandatory where output log
					file name is user specific and it is an optional command line argument for multitouch
					logging option.

					Note: Multitouch logging option is independent. multitouch_log option should
					not be mixed with any other option.

					Sample Output:
					1) ./evt-sniff -l 10

					/dev/input/event1 : cyttsp-i2c
					CFG file : evt-sniff.cfg

					Log File created :	./logs/19700103-23-07-42.log
					CSV Log File created :	./logs/19700103-23-07-42.csv

					******************TEST: MULTITOUCH LOGGING**************
						 MULTI TOUCH LOGGING TIME = 10 seconds

						press any key to start .....

					1) ./evt-sniff -l 320 multitouch_log_spatial_xline_jitter

					<Device No.> : <Touch Device Path> : <Touch Device Name>
					<0>          :</dev/input/event1>  :<atmel_mxt_ts>
					<1>          :</dev/input/event6>  :<gesture_mouse>
					<2>          :</dev/input/event5>  :<gesture_touch>
					Enter device number to debug the touch device:0
					The selected touch device is :
					/dev/input/event1 : atmel_mxt_ts

					CFG file : evt-sniff.cfg

					Log File created :	./logs/multitouch_log_spatial_xline_jitter.log
					CSV Log File created :	./logs/multitouch_log_spatial_xline_jitter.csv

					******************TEST: MULTITOUCH LOGGING**************
						 MULTI TOUCH LOGGING TIME = 320 seconds

						press any key to start .....

					2) ./evt-sniff -l 30 -r

						Error : Invalid combination of options
						Please follow the event sniffer tool manual.

	11. --runtime, -r		This option displays the touch events and analyzes them for uniformity and
					sample rate on the fly. Run time option display the time stamp, position x,
					position y, sample rate, time difference between two events (Time-Diff),
					positional event difference with respect to x and y coordinate. Sample rate
					is calculated on the basis of events per second. It puts a '*' mark if the touch
					events value not in expected range like,

					(a) Sample rate expected range( > MIN_SAMPLE_RATE & < MAX_SAMPLE_RATE).
					(b) Positional event difference with respect to x and y coordinate is greater than 1mm.

					Sample Output:
					1) ./evt-sniff -r

					/dev/input/event1 : cyttsp-i2c

					CFG : evt-sniff.cfg
					Test Execution time not specified. Default Test Time = 15

					Log File created :	./logs/19700103-23-11-02.log
					CSV Log File created :	./logs/19700103-11-02.csv

					******************TEST: RUNTIME EVENT ANALYSIS**************
					PROCEDURE: Move your finger across the screen

					Test time = 15 seconds

					NOTE-
						1. '*' against a sample rate value indicates that the rate is not in the expected range ( > 60 & < 150 )
						2. '*' against a x-diff/y-diff value indicates the event difference is greater than 1mm

					Press any key to start ....

					TIME(sec.usec)	X(px)	Y(px)	SRATE(events/sec)	Time-Diff(usec)	X-DIFF(px)	Y-DIFF(px)

								Diff > 100, ignoring this event difference (no event dropped)

					20130.150783	248	416		0*		000000		0		0

					20130.265265	248	416		8*		114482		0		0

					20131.993572	188	331		0*		1728307		60*		85*

					20132.432821	186	340		2*		439249		2		9*

					20132.449058	186	341		61		016237		0		1


	12. --point_accuracy, -a	This option creates a touch metric to measure point accuracy.
					Point accuracy option takes a multitouch log (.csv) file as an input and
					produces a point accuracy log as an output. Output log contains the point
					accuracy touch metric. Multi touch logging use case to create a perfect
					touch metric : "Short distance drags in an equidistance manner along X-axis
					starting from X-MIN to X-MAX, Repeat the above use case from top to bottom
					of the touch device".

					Note: Point accuracy option is independent. point_accuracy option should
					not be mixed with any other option.

					Sample Output:
					1) ./evt-sniff -a 19700134-12-02-56.csv

						CFG file : evt-sniff.cfg

						Test Execution time not specified. Default Test Time = 15
						Log File created :      ./logs/19700102-00-02-20.log
						CSV Log File created :  ./logs/19700102-00-02-20.csv

						Press a key to continue ...
						***** POINT ACCURACY TEST ******

					2) ./evt-sniff -a 19700134-12-02-56.csv -r

						Error : Invalid combination of options
						Please follow the event sniffer tool manual.

					3) ./evt-sniff -a 19700134-12-02-56

						CFG file : evt-sniff.cfg

						Test Execution time not specified. Default Test Time = 15
						Log File created :      ./logs/19700102-00-11-31.log
						CSV Log File created :  ./logs/19700102-00-11-31.csv

						Press a key to continue ...
						Unable to locate the log file.
						Please specify the correct log file.

	13. --x_line_jitter, -x		This option validates spatial X axis jitter test. Input for
					this option is a multitouch log (.csv) file and pixel pack(k). It reads
					the samples from input  multitouch log file and computes max X-delta
					among the samples in each pixel pack. It outputs the jitter computation
					results to a log file.
					Multi touch logging use case for X-line jitter: " Move your finger slowly
					in a straight line in X-direction starting form X-MIN to X-MAX repeat the
					above use case from top to bottom of the touch device".
					Spatial jitter has continuous samples, so we split the samples with
					K-pixel buckets. The coverage(K) of the bucket is user configurable.

					Note: Spatial X line jitter option should not be mixed with any other test option.
					x_line_jitter option is only mixed with pixel pack option (--pixel_pack, -k).

					Sample Output:
					1) ./evt-sniff -x 19700102-11-37-35.csv

						CFG file : evt-sniff.cfg

						Test Execution time not specified. Default Test Time = 15
						Log File created :      ./logs/19700102-00-37-56.log
						CSV Log File created :  ./logs/19700102-00-37-56.csv

						Press a key to continue ...
						***** X_LINE SPATIAL JITTER TEST ******

					2) ./evt-sniff -x 19700102-09-43-30.csv -k 40

						CFG file : evt-sniff.cfg

						Test Execution time not specified. Default Test Time = 15
						Pixel per output sample value: 40
						Log File created :      ./logs/19700102-00-38-27.log
						CSV Log File created :  ./logs/19700102-00-38-27.csv

						Press a key to continue ...
						***** X_LINE SPATIAL JITTER TEST ******

					3) ./evt-sniff -x 19700102-09-43-30.csv -k 10 -r

						Error : Invalid combination of options
						Please follow the event sniffer tool manual.

	14. --y_line_jitter, -y		This option validates spatial Y axis jitter test. Input for
					this option is a multitouch log (.csv) file and pixel pack(k). It reads
					the samples from input  multitouch log file and computes max Y-delta
					among the samples in each pixel pack. It outputs the jitter computation
					results to a log file.
					Multi touch logging use case for Y-line jitter: " Move your finger slowly
					in a straight line in Y-direction starting form Y-MIN to Y-MAX, repeat the
					above use case from X-MIN to X-MAX of the touch device".
					Spatial jitter has continuous samples, so we split the samples with
					K-pixel buckets. The coverage(K) of the bucket is user configurable.

					Note: Spatial Y line jitter option should not be mixed with any other test option.
					y_line_jitter option is only mixed with pixel pack option (--pixel_pack, -k).

					Sample Output:
					1) ./evt-sniff -y 19700102-09-43-30.csv

						CFG file : evt-sniff.cfg

						Test Execution time not specified. Default Test Time = 15
						Log File created :      ./logs/19700102-00-37-56.log
						CSV Log File created :  ./logs/19700102-00-37-56.csv

						Press a key to continue ...
						***** Y_LINE SPATIAL JITTER TEST ******

					2) ./evt-sniff -y 19700102-09-43-30.csv -k 75

						CFG file : evt-sniff.cfg

						Test Execution time not specified. Default Test Time = 15
						Pixel per output sample value: 75
						Log File created :      ./logs/19700102-00-38-27.log
						CSV Log File created :  ./logs/19700102-00-38-27.csv

						Press a key to continue ...
						***** Y_LINE SPATIAL JITTER TEST ******

					3) ./evt-sniff -y 19700102-09-43-30.csv -k 10 -r

						Error : Invalid combination of options
						Please follow the event sniffer tool manual.

	15. --pixel_pack, -k	Pixel_pack represents pack N pixels per output sample.
				This option is only used along with spatial X and Y line jitter test
				option. Spatial jitter has continuous samples, so we split the samples
				with K-pixel buckets.

				Sample Output:
				1)  ./evt-sniff -x 19700102-09-43-30.csv -k 100

					CFG file : evt-sniff.cfg

					Test Execution time not specified. Default Test Time = 15
					Pixel per output sample value: 100
					Log File created :      ./logs/19700102-00-38-27.log
					CSV Log File created :  ./logs/19700102-00-38-27.csv

					Press a key to continue ...
					***** X_LINE SPATIAL JITTER TEST ******

				2) ./evt-sniff -y 19700102-09-43-30.csv -k 70

					CFG file : evt-sniff.cfg

					Test Execution time not specified. Default Test Time = 15
					Pixel per output sample value: 70
					Log File created :      ./logs/19700102-00-38-27.log
					CSV Log File created :  ./logs/19700102-00-38-27.csv

					Press a key to continue ...
					***** Y_LINE SPATIAL JITTER TEST ******

				3) ./evt-sniff -y 19700102-09-43-30.csv -k -60

					CFG file : evt-sniff.cfg

					Test Execution time not specified. Default Test Time = 15
					Invalid pixels per output sample value: -60
					Default pixel per output sample value: 50
					Log File created :      ./logs/19700102-00-38-35.log
					CSV Log File created :  ./logs/19700102-00-38-35.csv

					Press a key to continue ...
					***** Y_LINE SPATIAL JITTER TEST ******

	16. --temporal_jitter, -p	This option validates temporal jitter test. Input for this option
				is a multitouch log (.csv) file. It reads the samples from input  multitouch log file
				and computes max X and max Y delta at each touch location. It outputs the jitter computation
				results to a log file.
				Multi touch logging use case for temporal jitter: " Touch specific location N times starting
				from X-MIN to X-MAX, repeats the above use case from top to bottom of the touch device".

				Let L(x,y) be the touch loaction where we touch N times, the temporal jitter computes below
				jitter specific value for touch location L(x,y).
				1) Delta(X) = maximum_deviation(Y) - minimum_deviation(Y).
				2) Delta(Y) = maximum_deviation(X) - minimum_deviation(X).
				3) Number of single touch events at specific location (L(x,y)).
				4) Maximum multitouch events at specific location (L(x,y)).
				5) Density = Number of events tested for jitter at specific location L(x,y).

				Note: Temporal jitter option is independent. Temporal jitter option should not be mixed with
				any other option.

				Sample Output:
				1) ./evt-sniff -p 19700134-12-02-11.csv

					CFG file : evt-sniff.cfg

					Test Execution time not specified. Default Test Time = 15
					Log File created :      ./logs/19700111-00-02-33.log
					CSV Log File created :  ./logs/19700111-00-02-33.csv

					Press a key to continue ...
					***** TEMPORAL JITTER TEST ******

				2) ./evt-sniff -p 19700134-12-02-56.csv -s

					Error : Invalid combination of options
					Please follow the event sniffer tool manual.

				3) ./evt-sniff -p 19700134-12-02-56

					CFG file : evt-sniff.cfg

					Test Execution time not specified. Default Test Time = 15
					Log File created :      ./logs/19700222-00-11-03.log
					CSV Log File created :  ./logs/19700222-00-11-03.csv

					Press a key to continue ...
					Unable to locate the log file.
					Please specify the correct log file.

	17. --linearity, -e     This option validates the linearity error of the touch device.
				Input for this option is a multi touch log (.csv) file. It reads
				the samples from input multi touch log file and computes
				the linearity error of the touch device.

				Linearity error is reported in millimeters.

				Multi touch logging usecase for Linearity test: "Draw a diagonal line
				from the top left corner position of the display device and repeat the
				above usecase from the top right corner of the display device".
				Let 'D1' is the diagonal line drwan from top left corner, 'D2' is the
				diagonal line drawn from top right corner. 'L' is the line least square
				best fit line derived from the reported touch coordinate.
				'Pi' is the distance from 'L' to reported touch coordinate
				(Xi, Yi), Max(Pi) is the linearity error of the diagonal.
				Max(linearity(D1) , Linearity(D2)) is the Linearity error of the device.

				Sample Output:
				1) ./evt-sniff -e linearity.csv

					CFG file : evt-sniff.cfg

					Test Execution time not specified. Default Test Time = 15
					Log File created :      ./logs/19700103-20-14-05.log
					CSV Log File created :  ./logs/19700103-20-14-05.csv

					Press a key to continue ...
					***** TEST: ANALYZE LINEARITY ******

					Digonal num: 1
					Linearity Error: 2.911205mm
					Digonal num: 2
					Linearity Error: 1.770386mm
					Device Linearity Error: 2.911205mm

				1) ./evt-sniff -e linearity

					CFG file : evt-sniff.cfg

					Test Execution time not specified. Default Test Time = 15
					Log File created :      ./logs/19700103-20-15-16.log
					CSV Log File created :  ./logs/19700103-20-15-16.csv

					Press a key to continue ...
					Unable to locate the log file.
					Please specify the correct log file.

18. --finger_separation, -f    This option validates the minimum finger separation
				between two touches. Input for this option is a multi touch
				log (.csv) file. It reads the samples from input  multi touch
				log file and computes the minimum finger separation between two touches.
				The minimum finger separation is represented in millimeters.

				Multi touch logging usecase for Minimum Finger Separation test :
				The test procedure involves the following steps, for determining
				Minimum Y-Axis Finger Seperation,
				1) Position a two-finger probe (with 20mm separation) set over the device.
				2) Draw two vertical (parallel) lines parallel to its vertical axis.
				3) Collect the logs & repeat Step 2, with probe separation reduced by 2mm.
				4) Repeat steps 2 & 3 until the probe separation gets to 4mm.

				For determining Minimum X-axis Finger separation, repeat the above test
				procedure with the probes running along the X-axis of the device.For
				determining Minimum Diagonal Finger separation, repeat the above test
				procedure with the probes running along the diagonal axis of the device.

				For a particular the minimum probe separation distance over which
				the linearity is still within acceptable limits, is the minimum
				finger separation for the device along that particular axis.

				Sample Output:
				1) ./evt-sniff -f min_finger_separation.csv

					CFG file : evt-sniff.cfg

					Test Execution time not specified. Default Test Time = 15
					Log File created :      ./logs/19700103-20-19-13.log
					CSV Log File created :  ./logs/19700103-20-19-13.csv

					Press a key to continue ...
					***** MINIMUM FINGER SEPARATION TEST******

					Linearity of parallel lines number : 1
						Probe separation : 20mm
						line 1 linearity is : 2.31154mm
						line 2 linearity is : 3.77481mm
					Linearity of parallel lines number : 2
						Probe separation : 18mm
						line 1 linearity is : 2.20014mm
						line 2 linearity is : 1.61281mm
					Linearity of parallel lines number : 3
						Probe separation : 16mm
						line 1 linearity is : 1.03054mm
						line 2 linearity is : 2.14681mm
					Linearity of parallel lines number : 4
						Probe separation : 14mm
						line 1 linearity is : 2.13054mm
						line 2 linearity is : 2.14682mm
					Linearity of parallel lines number : 5
						Probe separation : 12mm
						line 1 linearity is : 2.50054mm
						line 2 linearity is : 2.97681mm
					Linearity of parallel lines number : 6
						Probe separation : 10mm
						line 1 linearity is : 3.03054mm
						line 2 linearity is : 2.74681mm
					Linearity of parallel lines number : 7
						Probe separation : 8mm
						line 1 linearity is : 3.10011mm
						line 2 linearity is : 2.14132mm
					Linearity of parallel lines number : 8
						Probe separation : 6mm
						line 1 linearity is : 2.50054mm
						line 2 linearity is : 2.97681mm
					Linearity of parallel lines number : 9
						Probe separation : 4mm
						line 1 linearity is : 2.31134mm
						line 2 linearity is : 2.70681mm

					Touch Device Name : cyttsp-i2c
					Minimum finger separation between two touches is : 4mm

19. --resolution, -o   This option validates the resolution of the touch device.
			Input for this option is a multi touch log (.csv) file.
			It reads the samples from input  multi touch log file
			and computes the resolution of the touch device.
			The resolution is represented in millimeters.

			The objective of this test is to quantify the minimum spatial distance
			that generates a touch report from the TSC. The resolution of
			the touch device is calculated the minimum spatial distance between
			two touch co-ordinates reported by the touch screen controller.
			Multi touch logging usecase for Resolution test: "Draw a 3-inch swipe
			on the device along the X-axis and Y-axis respectively".
			Let S1 is the spatial distance between two touch co-ordinates (Xi, Yi)
			and (Xi+1, Yi+1). Calculate the spacial distance(Si) for each consecutive
			touch co-ordinates reported by the touch screen controller.
			The resolution of the touch device is MIN(S1, S2, S3, ....,Sn).

			Sample Output:
			1) ./evt-sniff -o resolution_multitouch_log.csv

				CFG file : evt-sniff.cfg

				Test Execution time not specified. Default Test Time = 15

				Log File created : ./logs/19700117-02-04-53.log
				CSV File created : ./logs/19700117-02-04-53.csv

				Press a key to continue ...
				***** RESOLUTION TEST ******

				Touch Device Resolution along X-Axis is: 3.00mm
				Touch Device Resolution along Y-Axis is: 9.00mm

20. --suspend, -S   This option validates the performance of the touch device
		during frequent state switching of the touch controller driver
		from suspend state to resume state or viceversa.

		Sample Output:
		1) ./evt-sniff -S
		<Device No.> : <Touch Device Path> : <Touch Device Name>
		<0>          :</dev/input/event8>  :<gesture_mouse>
		<1>          :</dev/input/event7>  :<gesture_touch>
		<2>          :</dev/input/event1>  :<atmel_mxt_ts>

		Enter device number to debug the touch device:2
		/dev/input/event1 : atmel_mxt_ts

		CFG file : evt-sniff.cfg

		Test Execution time not specified. Default Test Time = 15
		Log File created :      ./logs/19700102-00-01-02.log
		CSV Log File created :  ./logs/19700102-00-01-02.csv

		******* TEST: AUTOMATIC SUSPEND/RESUME *********
                TEST TIME = NA (use CTRL-\ to break)
			Time interval between suspend and active not specified
			Default time interval set 5
			Number of times touch device supend not specified
			Default time counter set 15

                Press any key to start .....
                Touch device suspend for 5 seconds
                Touch device active for 5 seconds
                Touch device suspend for 5 seconds
                Touch device active for 5 seconds
                Touch device suspend for 5 seconds
                Touch device active for 5 seconds..

21  --suspend_interval, -I   Time interval between two suspend operation of touch device.
			This option is only used along with suspend test option.

		Sample Output:
		1) ./evt-sniff -S -I 5

		CFG file : evt-sniff.cfg

		Test Execution time not specified. Default Test Time = 15
		Log File created :      ./logs/19700102-00-05-37.log
		CSV Log File created :  ./logs/19700102-00-05-37.csv

		******* TEST: AUTOMATIC SUSPEND/RESUME *********
		TEST TIME = NA (use CTRL-\ to break)
			Time interval between suspend and active: 5
			Number of times touch device supend not specified
			Default time counter set 15

		Press any key to start .....
                Touch device suspend for 5 seconds
                Touch device active for 5 seconds
                Touch device suspend for 5 seconds..

22  --suspend_times, -T   Number of times suspend touch device in suspend test option.
			This option is only used along with suspend test option.

		Sample Output:
		1) ./evt-sniff -S -I 5 -T 12
		<Device No.> : <Touch Device Path> : <Touch Device Name>
		<0>          :</dev/input/event8>  :<gesture_mouse>
		<1>          :</dev/input/event7>  :<gesture_touch>
		<2>          :</dev/input/event1>  :<atmel_mxt_ts>

		Enter device number to debug the touch device:2
		/dev/input/event1 : atmel_mxt_ts

		CFG file : evt-sniff.cfg

		Test Execution time not specified. Default Test Time = 15
		Log File created :      ./logs/19700102-00-06-06.log
		CSV Log File created :  ./logs/19700102-00-06-06.csv

		******* TEST: AUTOMATIC SUSPEND/RESUME *********
		TEST TIME = NA (use CTRL-\ to break)
			Time interval between suspend and active: 5
			Number of times touch device suspend: 12

                Press any key to start .....
                Touch device suspend for 5 seconds
                Touch device active for 5 seconds
                Touch device suspend for 5 seconds..

Typical execution command

				./evt-sniff -i -c evt-siff.cfg -t 15 -u -s -d 2011-12-12-20-15-40.log

				This command will do the following (in-order)
				1. -c	Parse the config file to and store the parameters
				2. -i	Display the touch, display and touch to
					display mapping (based on data from the cfg)
				3. -t	Set the test timing to 10 seconds
				4. -u	Execute the Uniformity test
				5. -s	Execute the Sample rate test
				6. Display the results of Uniformity and Sample rate tests
				7. -d   Parse the 2011-12-12-20-15-40.log file and compare the
					results with the current test
				8. Display the comparison results on the screen.

	 NOTE: When you run the uniformity test(-u option) or sample rate test(-s option),
	       if you get the following output:

		               In-sufficient data, please run this test again
			       Press any key to continue....

		Then, most likely, the number of samples detected were very less, which could be
		because the finger was moved very slowly. Try moving the finger a little faster
		in the next run.



Test Specifics

	Uniformity test -	The uniformity test determines how uniformly the touch events
				are generated by the controller. As explained in the earlier
				section, a stutter in the display can be noticed if the
				consecutive touch events reported are spaced more 1mm apart.
				This test checks if this case is seen with the underlying
				touch controller.

				The tool performs this test when a "-u" option is specified in
				the runtime argument. For this test the tool prompts the user
				to move one finger slowly on the screen in a specific direction
				(X or Y). The user is expected to move his finger for the time
				set with the .t option  (or a default time of 15 secs). The
				tool captures all the events during this time and analyzes them
				for uniformity.

				Precautions to be taken while running this test.

					1. Do not move the finger too fast. The finger motion
					should be around 2-3cms / sec (20 to 30 mm/sec). Typically,
					the way the screen is scrolled while reading an email/article.
					If the finger has moved too fast the tool will put out
					the following error .

			                            !!!!!WARNING!!!!!

			            Touch events are too far apart. Results might be inaccurate.
			            Please retry the test moving the finger slower.

					2. Do not move the finger in a zigzag manner. The test
					   requires the finger movement in specific direction
					   (X or Y). A zigzag/irregular motion does not generate
					   correct events.

					3. Do not have more than one point of contact on the screen.
					   The event data gets corrupted if multiple fingers are
					   placed on the screen. Please re-run the test, if such a
					   case occurs.

					4. If possible specify  PPI (or XSIZE and YSIZE if PPI is
					   not available) via the config file. This enables the
					   results displayed to include the distance information
					   (distance moved per mm) which makes it easy to validate.

					5. Run the test (finger movement) for the complete time as
					   specified with the -t option (or 15 secs). The tool will
					   generate the a error there are not enough samples to analyze.

			If the test completes  successfully, the results will be displayed as follows.

			****Uniformity Test Results*****

			Total Events Analyzed = 819 in X-Direction
			Touch Event Delta	Number of Events	Distance in mm
				1			68			0.146
			        2	                39			0.293
			        3			712			0.439

			The first row (Total Events Analyzed) indicates the number of touch events
			analyzed for this test. It also indicates the direction (X / Y) of movement of
			the user.

			The first column (Touch Event Delta) indicates the difference in pixels of
			consecutive touch events.

			The second column (Number of Events) points to the number of events with
			the corresponding touch delta. For example, in row 1 - 68 events were seen
			with a touch delta of 1.

			The third column (Distance in mm) is the distance in mm for each of the delta.
			For example - Row 3 indicates that a touch delta of "3" has a distance of
			0.43mm associated with it.  So, if 2 events are seen with a touch delta of "3"
			the distance between them is 0.439mm.
			[Abnormal case is when the Distance/mm value is >= 1mm]

			Abnormalities

			1. If there are events with >= 1mm span (Distance in mm column), there
			might be a stutter in the display.  In case the (Distance in mm) column 3 is
			absent in the result (case when PPIX/PPIY/XSIZE/YSIZE is not specified in the config
			file) pixels per mm can be calculated from the below.
				a. If the phone specification mentions the ppi (pixels per inch), specify
				   the PPIX and PPIY in the config file.  (PPIX = <value>, PPIY = <value>)
				b. If nothing is specified you can assume a default ppi of 160 i.e. 6
				   pixels per mm for both X and Y direction.

			2. If the event spread is not uniform, i.e. there are noticeable gaps in
			the event delta (column 1).  The event delta should have events present with
			consecutive deltas.

			In either of the above cases the touch controller needs further
			tuning/calibration.



	Sample Rate Test	The sample rate test calculates the report rate (events per
				second) of the touch controller. As explained in the earlier
				section, if the sample rate cannot be too high or too low.
				A high sample rate can cause over buffering and a low sample
				rate can cause lags in UX. Thus, it is very important to have
				an optimal sample rate. On Android devices, a good sample rate is
				between 60 and 90 events per second. The tool performs this test
				when a -s option is specified in the runtime argument. For this
				test, the tool prompts the user to move his finger across the
				screen. The user is expected to move his finger for the time set
				with the -t option (or a default time of 15 secs). The tool captures
				all the events during this time and analyzes them for sample rate.

				Precautions to be taken while running this test .

				1. Do not move the finger too fast or too slow. This test is to be carried
				out at a speed of 5 . 6 cm/sec (which is a normal movement of a user).

				2. Do not have multiple fingers on the screen while running this test. This
				will directly affect the sample rate (multiplies the sample rate with the number
				of points of contact). Re-run the test if such a case occurs.

				3. Run the test (finger movement) for the complete time as specified with
				the -t option (or 15 secs). The tool will generate an error if there are not
				enough samples to analyze.

				If the test completes successfully, the results will be displayed as follows.

				****Sample Rate Test Results*****

		                Total events analyzed = 3984

		                Minimum Sample Rate (events per second)       = 5
		                Maximum Sample Rate (events per second)      = 200
		                Average Sample Rate (events per second)         = 88
		                Standard Deviation (deviation wrt average)    = 8

		                Expected : Average Sample Rate between 60 and 90
		                Result  : Sample rate is 88 and in the expected range


			        ----Sample Rate Spread---

				Sample Rate             Number of events(with specified rate)
			           8					 1
			           14					 1
			           62					1200

				The sample rate test result has 2 parts. The first part indicates the overall
				statistics. This includes the total number of events analyzed, the maximum,
				minimum, average and the standard deviation of the sample rate. The final 2 rows
				analyze the sample rate and suggest if there is a need to change the sample
				rate. The second part of the result displays the sample rate spread. This
				indicates the overall spread of the sample rate across the analyzed events.

				Abnormalities

				1. If the Average sample rate is may higher than 90, there is a possibility
				that the userspace will not be able to handle this rate and might lead to
				buffering of the touch events. Check the maxEventsperSecond configuration set in
				that device,  the recommended value is  ~ 1.5 times the average sample rate.

				2. The standard deviation reported should not have a huge value. The
				standard deviation is an indication of the deviation of the sample rate from the
				average. A huge number indicates that the touch controller has an abnormal
				sample rate and this needs to be further calibrate/tuned.

				3. The sample rate spread should have a high concentration of the events
				near the average sample rate. If there is substantial event count for lower and
				higher sample rate (compared to the average) it indicates that the event rate is
				not consistent and there might be a configuration problem with the controller.
				Generally, if the finger moves very slowly (or stops) the sample rate goes very
				low (check if this is the case with your controller), this will reflect in the
				sample rate spread and should not be flagged as an abnormality.


	Jitter Test		The jitter test calculates the spatial jitter of the touch
				controller. The touch controller is expected to report the same coordinates (or
				a very small acceptable difference) when the same point is touched multiple
				times. The acceptable range for here is 1mm, i.e. the data reported by the
				controller should be within 1mm of the touched area. The tool performs this
				this test when a -j option is specified in the runtime argument. For this test,
				the tool prompts the user to touch one point multiple times (at least 10)
				within 10 seconds [press and release].  It then analyzes if the data reported
				in within the acceptable range.

				Precautions to be taken while running this test .

				1. Do not have multiple points of contacts on the screen while running this test.
				2. Make sure that the same location is pressed and released at least 10 times
				within 10 seconds.

				If the test completes successfully, the results will be displayed as follows .

				Acceptable Range (number of pixels) in X-Direction = 6
				Acceptable Range (number of pixels) in Y-Direction = 6
				Location (pixel number) used for jitter test (first touch) X = 456, Y = 1043

				Touch Number    Delta in X-direction    Delta in Y-Direction	Acceptable Range
				1			0			2			Yes
				2			1			3			Yes
				3			0			3			Yes
				4			0			8			No
				5			2			7			No

				Acceptable range is generally determined from the PPIX/PPIY/XSIZE,YSIZE and is equal
				to number of touch pixels in 2.5 mm. If the PPIX/PPIY/XSIZE,YSIZE is not specified the
				acceptable rate defaults to 15 (160 ppi).


				Abnormalities

				1. Please run this test multiple times and compare the results. If there
				are very frequent failures the touch controller needs further fine tuning.

	Logging

				Every execution of the tool is logged with the log details (filename) displayed
				on the console when the tool starts execution. Below is an example -

				Log File created :     ./logs/19700102-00-05-25.log
				CSV Log File created :  ./logs/19700102-00-05-25.csv

				The tool generates two log files, one of them is an internal logging file which
				can be used with the diff. option (--diff, -d) of the tool to compare results
				from two trail runs. The files can only be compared if they are generated by the
				same version of the tool. These logging files are of the type ".log". This
				internal logging file can also be exported to another device and the results can
				be compared. The other log file is of the type ".csv" which can be imported
				from the target and analyzed in EXCEL.

				The file names are derived from the current date and time ($DATE-$TIME.log) and
				are placed in "logs" directory in the current directory where the tool is
				located on the target. While exporting the file to another device, the logfile
				needs to placed in the same relative location as the tool (./logs/*). Both the
				log files include the version information of the tool.


	Queries

			For any queries regarding the tool please contact linux.iodevices.dev
