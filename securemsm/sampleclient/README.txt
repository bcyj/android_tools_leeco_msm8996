qseecom sample client
======================

This test app present examples illustrating the use of various APIs published in the QSEEComAPI.h header file for the QSEECOM library.

The compliment to this HLOS client is the "sampleapp" that resides and runs on the secure trustzone domain. The qseecom sample client (also known as the qseecom testapp) communicates with the sampleapp via issuing various commands. The same comamnd data structure is used by the teat app and the sampleapp.

Following are the test cases that illustrate the use of APIs in QSEEcom library: The logs for the results of these test (where applicable) cases can be found in data/misc/tzapps/sampleapp.


basic --
   This test case illustrates the use of QSEECom_send_command(). The test case loads the sampleapp using QSEECom_start_app. It then issues a QSEECOM_send_command where it send some data and sample app responds to this this command with a response data that is 10 time the data send in the command. This command is issued numerous times and the result is confirmed to be as expected. The test finishes up by shutting down the sampleapp by calling QSEECom_shutdown_app().

crypto --
   This test case illustrates the use of QSEECom_send_modified_command(). The test case loads the sampleapp using QSEECom_start_app. It then issues a QSEECOM_send_modified_command where it sends a pointer to some buffer within the command structure. sample app responds to the command by adding encrypting the data pointed to by the buffer and then decrypting it. This command is sent numerous times and the results are confirmed to be as expected. The test finishes up by shutting down the sampleapp by calling QSEECom_shutdown_app().

cryptoperf --
   This test case illustrates the use of QSEECom_set_bandwidth() and QSEECom_send_modified_command(). The test case loads the sampleapp using QSEECom_start_app. It then issues a QSEECom_set_bandwidth() followed by a QSEECOM_send_modified_command where it sends a pointer to some buffer within the command structure. sample app responds to the command by adding encrypting the data pointed to by the buffer and then decrypting it. The average time (in ms) taken to complete the encryption followed by decryption is logged into a file (located in data/misc/tzapps/sampleapp). This command is sent numerous times and the results are confirmed to be as expected. The test finishes up by shutting down the sampleapp by calling QSEECom_shutdown_app().





*******************************************************
               Internal Tests
(The following test cases are for internal test
 purposes only and is not intended to be used for
 illustrating any functionality of the QSEECOM library)
********************************************************
regmem --
   Here sample app is started & a data buffer with some data is pointed to the sample app. Sample app adds 10 to the value pointed by sample client & returns the value. This process is performed for several iterations (10000 times) by increasing the pointer address for every iteration. This is followed by Shutdown App.

bufalign --
   This test issues commands with unaligned buffer pointer within the command structure. The test is intended to illustrate that unaligned buffer pointers are not supported. This test will hang and is an expected behavior.

stress --
   'BASIC' test (above) is executed with multiple threads working together.
***********************************************************************************
                  PLEASE SEE THE EXAMPLE RUN BELOW
***********************************************************************************

---------------------------------------------------------
Usage: qseecom_sample_client -[OPTION] -[TEST_TYPE0]..-[TEST_TYPEn]
Runs the user space tests specified by the TEST_TYPE

 parameter(s).


OPTION can be:
  -v, --verbose       Tests with debug messages on

TEST_TYPE can be:
  -b, --basic         Test CLIENT_CMD1_BASIC_DATA command.
  -c, --crypto        Test crypto operations (1KB-32KB packets)
  -p, --cryptoperf    Test crypto performance (1KB-64KB packets)
  -s, --Stress        Test multi app, multi client tests.
  -e, --exit          Exit
  -h, --help          Print this help message and exit


 - 'adb push <BUILD>out/target/product/<TARGET>/obj/EXECUTABLES /qseecom_sample_client_intermediates/qseecom_sample_client /data'
 - Connect to device: From command shell, do 'adb shell'
 - Once in the shell: do 'cd /data'
 - Change permission for qseecom_sample_client:    do  chmod 777 qseecom_sample_client
 - Run qseecom_sample_client:
  do './qseecom_sample_client -<OPTION> -<TEST_TYPE0> -<TEST_TYPE1>'
---------------------------------------------------------
255|root@android:/data # ./qseecom_sample_client -b
./qseecom_sample_client -b
Starting qsecom sample client v1.3
Basic_start_stop_test: thread 1
(This may take a few minutes please wait....)
qsc_run_start_stop_app_basic_test PASSED for thread 1
