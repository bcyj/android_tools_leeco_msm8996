/*
 * Copyright (c) 2013-2014, Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#define LOG_TAG "BTJNI"

#include <jni.h>
#include <stdint.h>
#include "cutils/log.h"

#include <stdio.h>
#include <dlfcn.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/prctl.h>
#include <sys/capability.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>

#include <private/android_filesystem_config.h>
#include <android/log.h>

#include <hardware/hardware.h>
#include <hardware/bluetooth.h>

/************************************************************************************
**  Constants & Macros
************************************************************************************/

#define BUF_SIZE 256

#define HCI_GRP_LINK_CONTROL_CMDS       (0x01 << 10)            /* 0x0400 */
#define HCI_GRP_LINK_POLICY_CMDS        (0x02 << 10)            /* 0x0800 */
#define HCI_GRP_HOST_CONT_BASEBAND_CMDS (0x03 << 10)            /* 0x0C00 */
#define HCI_GRP_INFORMATIONAL_PARAMS    (0x04 << 10)            /* 0x1000 */
#define HCI_GRP_STATUS_PARAMS           (0x05 << 10)            /* 0x1400 */
#define HCI_GRP_TESTING_CMDS            (0x06 << 10)            /* 0x1800 */

#define HCI_GRP_VENDOR_SPECIFIC         (0x3F << 10)            /* 0xFC00 */

#define HCI_READ_RMT_VERSION_INFO       (0x001D | HCI_GRP_LINK_CONTROL_CMDS)
#define HCI_INQUIRY                     (0x0001 | HCI_GRP_LINK_CONTROL_CMDS)
#define HCI_READ_BD_ADDR                (0x0009 | HCI_GRP_INFORMATIONAL_PARAMS)
//#define HCI_GRP_INFORMATIONAL_PARAMS    (0x04 << 10)            /* 0x1000 */

#define HCI_RESET                       (0x0003 | HCI_GRP_HOST_CONT_BASEBAND_CMDS)
#define PROD_TEST_OPCODE                (0x0004 | HCI_GRP_VENDOR_SPECIFIC)

/*
** Define the packet types in the packet header
*/
#define PKT_TYPE_NULL   0x00
#define PKT_TYPE_POLL   0x01
#define PKT_TYPE_FHS    0x02
#define PKT_TYPE_DM1    0x03

#define PKT_TYPE_DH1    0x04
#define PKT_TYPE_HV1    0x05
#define PKT_TYPE_HV2    0x06
#define PKT_TYPE_HV3    0x07
#define PKT_TYPE_DV     0x08
#define PKT_TYPE_AUX1   0x09

#define PKT_TYPE_DM3    0x0a
#define PKT_TYPE_DH3    0x0b

#define PKT_TYPE_DM5    0x0e
#define PKT_TYPE_DH5    0x0f

/************************************************************************************
**  Static variables
************************************************************************************/

//PROD_TEST_TX_BURST command
static uint8_t burst_array[] = { 0x04,                                //command opcode
                                 0x00, 0x00, 0x00, 0x00, 0x00,        //5 hop channels
                                 0x00,                                //pattern payload
                                 0x00,                                //packet type
                                 0x00,                                //whitening
                                 0x00,                                //power
                                 0x00,                                //receiver gain
                                 0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc,  //BD address
                                 0x01,                                //hopping functionality
                                 0x00, 0x00,                          //payload length
                                 0x00 };                              //logical transport address

//PROD_TEST_TX_CONTINUOUS command
static uint8_t conti_array[] = { 0x05,                                //command opcode
                                 0x00,                                //channel number
                                 0x00,                                //power
                                 0x04,                                //transmit type
                                 0x01,                                //pattern length
                                 0x00, 0x00, 0x00, 0x00 };            //bit pattern

//PROD_TEST_LE_TX_BURST command
static uint8_t le_tx_array[] = { 0x1c,                                //command opcode
                                 0x00,                                //frequency offset
                                 0x00,                                //payload length
                                 0x00 };                              //payload type

static unsigned char main_done = 0;
static bt_status_t status;

//static jint bt_power = 0;

/* Main API */
static bluetooth_device_t* bt_device;

const bt_interface_t* sBtInterface = NULL;

/* Set to 1 when the Bluedroid stack is enabled */
static unsigned char bt_enabled = 0;

/*******************************************************************************
 ** Load stack lib
 *******************************************************************************/

jint HAL_load(void)
{
    jint err = 0;

    hw_module_t* module;
    hw_device_t* device;

    ALOGE("Loading HAL lib + extensions");

    err = hw_get_module(BT_HARDWARE_MODULE_ID, (hw_module_t const**)&module);
    if (err == 0)
    {
        err = module->methods->open(module, BT_HARDWARE_MODULE_ID, &device);
        if (err == 0) {
            bt_device = (bluetooth_device_t *)device;
            sBtInterface = bt_device->get_bluetooth_interface();
        }
    }

    ALOGE("HAL library loaded (%s)", strerror(err));

    return err;
}

jint HAL_unload(void)
{
    jint err = 0;

    ALOGE("Unloading HAL lib");

    sBtInterface = NULL;

    ALOGE("HAL library unloaded (%s)", strerror(err));

    return err;
}

/*******************************************************************************
 ** HAL test functions & callbacks
 *******************************************************************************/
/*
void setup_test_env(void)
{
    int i = 0;

    while (console_cmd_list[i].name != NULL)
    {
        console_cmd_maxlen = MAX(console_cmd_maxlen, (int)strlen(console_cmd_list[i].name));
        i++;
    }
}

void check_return_status(bt_status_t status)
{
    if (status != BT_STATUS_SUCCESS)
    {
        ALOGE("HAL REQUEST FAILED status : %d (%s)", status, dump_bt_status(status));
    }
    else
    {
        ALOGE("HAL REQUEST SUCCESS");
    }
}
*/
static void adapter_state_changed(bt_state_t state)
{
    ALOGE("ADAPTER STATE UPDATED : %s", (state == BT_STATE_OFF)?"OFF":"ON");
    if (state == BT_STATE_ON) {
        bt_enabled = 1;
    } else {
        bt_enabled = 0;
    }
}

static void dut_mode_recv(uint16_t opcode, uint8_t *buf, uint8_t len)
{
    ALOGE("DUT MODE RECV : NOT IMPLEMENTED");
}

#ifdef QCOM_BLUETOOTH
static void hci_event_recv(uint8_t event_code, uint8_t *buf, uint8_t len)
{
    int i = 0;
    ALOGE("hci_event_recv: event_code %d", event_code);
    for( i =0; i< len; i++)
        ALOGE("hci_event_recv: buf[%d] is 0x%x", i, buf[i]);
}
#endif
/*

static void le_test_mode(bt_status_t status, uint16_t packet_count)
{
    ALOGE("LE TEST MODE END status:%s number_of_packets:%d", dump_bt_status(status), packet_count);
}

static void device_found_cb(int num_properties, bt_property_t *properties)
{
    int i;
    for (i = 0; i < num_properties; i++)
    {
        if (properties[i].type == BT_PROPERTY_BDNAME)
        {
            ALOGE("AP name is : %s\n",
                    (char*)properties[i].val);
        }
    }
}
*/
static bt_callbacks_t bt_callbacks = {
    sizeof(bt_callbacks_t),
    adapter_state_changed,
    NULL, /*adapter_properties_cb */
    NULL, /* remote_device_properties_cb */
//    device_found_cb, /* device_found_cb */
    NULL, /* device_found_cb */
    NULL, /* discovery_state_changed_cb */
    NULL, /* pin_request_cb  */
    NULL, /* ssp_request_cb  */
    NULL, /* bond_state_changed_cb */
    NULL, /* acl_state_changed_cb */
    NULL, /* thread_evt_cb */
    dut_mode_recv, /*dut_mode_recv_cb */
    NULL, /* le_test_mode_cb */
#ifdef QCOM_BLUETOOTH
    NULL, /* wake_state_changed_cb */
    hci_event_recv, /*hci_event_recv_cb */
    NULL, /*le_adv_enable_cb*/
    NULL, /* le_extended_scan_result_cb */
    NULL, /* le_lpp_write_rssi_thresh_cb */
    NULL, /* le_lpp_read_rssi_thresh_cb */
    NULL, /* le_lpp_enable_rssi_monitor_cb */
    NULL, /* le_lpp_rssi_threshold_evt_cb */
    NULL /*ble_conn_params_cb*/
#endif
};

jint bdt_init(void)
{
//    int err = 0;
    ALOGE("INIT BT");
    if (sBtInterface) {
//        ALOGE("BT INIT FAILED. err = %d", err);
        return sBtInterface->init(&bt_callbacks);
    }
    return -1;
}

jint bdt_enable(void)
{
    ALOGE("ENABLE BT");
    if (bt_enabled) {
        ALOGE("Bluetooth is already enabled");
        return 0;
    }
    return sBtInterface->enable();
}

jint bdt_disable(void)
{
    ALOGE("DISABLE BT");
    if (!bt_enabled) {
        ALOGE("Bluetooth is already disabled");
        return 0;
    }
    return sBtInterface->disable();
}
/*
void bdt_send_cmd(void)
{
    ALOGE("bdt_send_cmd ");
    if (!bt_enabled) {
        bdt_log("Bluetooth is already disabled");
        return;
    }
    status = sBtInterface->hci_cmd_send(HCI_READ_BD_ADDR ,NULL , 0 );

    check_return_status(status);
    sleep(2);

    uint8_t inq_array[] = { 0x33, 0x8b, 0x9e, 0x04, 0x00 };

    status = sBtInterface->hci_cmd_send(HCI_INQUIRY ,inq_array , 5 );

    check_return_status(status);
    sleep(2);

    uint8_t rmt_array[] = { 0x02, 0x03, 0x00};

    status = sBtInterface->hci_cmd_send(HCI_READ_RMT_VERSION_INFO ,rmt_array, 3 );

    check_return_status(status);
    sleep(2);
}

void bdt_dut_mode_configure(char *p)
{
    int32_t mode = -1;

    ALOGE("BT DUT MODE CONFIGURE");
    if (!bt_enabled) {
        ALOGE("Bluetooth must be enabled for test_mode to work.");
        return;
    }
    mode = get_signed_int(&p, mode);
    if ((mode != 0) && (mode != 1)) {
        ALOGE("Please specify mode: 1 to enter, 0 to exit");
        return;
    }
    status = sBtInterface->dut_mode_configure(mode);

    check_return_status(status);
}
*/
/*******************************************************************************
 ** JNI layer APIs
 *******************************************************************************/

static jint bt_start_test(JNIEnv* env, jobject) {
    ALOGE("%s: start", __func__);
    if ( HAL_load() < 0 ) {
        ALOGE("HAL failed to initialize, exit\n");
        return -1;
    } else {
        return bdt_init();
    }
}

static jint bt_stop_test(JNIEnv* env, jobject) {
    ALOGE("%s: start", __func__);
    return HAL_unload();
}

static jint bt_reset_HCI(JNIEnv* env, jobject) {
    ALOGE("%s: start", __func__);
    if (sBtInterface)
        return sBtInterface->hci_cmd_send(HCI_RESET, NULL, 0 );
    else
        return -1;
}

static jint bt_enter_HCI_mode(JNIEnv* env, jobject) {
    // TODO
    return 0;
}

static jint bt_start_cmd_mode(JNIEnv* env, jobject) {
    // ENABLE BT
    return bdt_enable();
}

static jint bt_stop_cmd_mode(JNIEnv* env, jobject) {
    // DISABLE BT
    return bdt_disable();
}

static jint bt_start_only_burst_test(JNIEnv* env, jobject) {
    int i = 0;
    ALOGE("%s: start", __func__);
    for (i = 0; i < 21; i++)
        ALOGE("%s: burst_array[%d] is 0x%x", __func__, i, burst_array[i]);
    if (sBtInterface)
        return sBtInterface->hci_cmd_send(PROD_TEST_OPCODE, burst_array, 21 );
    else
        return -1;
}

static jint bt_stop_only_burst_test(JNIEnv* env, jobject) {
    // reset hci?
    ALOGE("%s: stop", __func__);
    if (sBtInterface)
        return sBtInterface->hci_cmd_send(HCI_RESET, NULL, 0 );
    else
        return -1;
}

static jint bt_start_continuous_test(JNIEnv* env, jobject) {
    int i = 0;
    ALOGE("%s: start", __func__);
    for (i = 0; i < 9; i++)
        ALOGE("%s: conti_array[%d] is 0x%x", __func__, i, conti_array[i]);
    if (sBtInterface)
        return sBtInterface->hci_cmd_send(PROD_TEST_OPCODE, conti_array, 9 );
    else
        return -1;
}

static jint bt_stop_continuous_test(JNIEnv* env, jobject) {
    // reset hci?
    ALOGE("%s: stop", __func__);
    if (sBtInterface)
        return sBtInterface->hci_cmd_send(HCI_RESET, NULL, 0 );
    else
        return -1;
}

static jint bt_start_le_test(JNIEnv* env, jobject) {
    int i = 0;
    ALOGE("%s: start", __func__);
    for (i = 0; i < 4; i++)
        ALOGE("%s: le_tx_array[%d] is 0x%x", __func__, i, le_tx_array[i]);
    if (sBtInterface)
        return sBtInterface->hci_cmd_send(PROD_TEST_OPCODE, le_tx_array, 4 );
    else
        return -1;
}

static jint bt_stop_le_test(JNIEnv* env, jobject) {
    // reset hci?
    ALOGE("%s: stop", __func__);
    if (sBtInterface)
        return sBtInterface->hci_cmd_send(HCI_RESET, NULL, 0 );
    else
        return -1;
}

static jint bt_set_hopping(JNIEnv* env, jobject, jboolean hopping) {
    ALOGE("%s: hopping = %d", __func__, hopping);
    if (hopping)
        burst_array[17] = 0x01;
    else
        burst_array[17] = 0x00;
    return 0;
}

static jint bt_set_whitening(JNIEnv* env, jobject, jboolean whitening) {
    ALOGE("%s: whitening = %d", __func__, whitening);
    if (whitening)
        burst_array[8] = 0x01;
    else
        burst_array[8] = 0x00;
    return 0;
}

static jint bt_set_channel(JNIEnv* env, jobject, jint channel) {
    // TODO
    return 0;
}

static jint bt_set_channel_array(JNIEnv* env, jobject, jint channel1, jint channel2,
        jint channel3, jint channel4, jint channel5) {
    burst_array[1] = channel1;
    burst_array[2] = channel2;
    burst_array[3] = channel3;
    burst_array[4] = channel4;
    burst_array[5] = channel5;

    return 0;
}

static jint bt_set_power(JNIEnv* env, jobject, jint power) {
    burst_array[9] = power;
    return 0;
}

static jint bt_set_packet_type(JNIEnv* env, jobject, jint type) {
    burst_array[7] = type;
    return 0;
}

static jint bt_set_trans_pattern(JNIEnv* env, jobject, jint pattern) {
    burst_array[6] = pattern;
    return 0;
}

static jint bt_set_length(JNIEnv* env, jobject, jint length) {
    burst_array[18] = (unsigned char)(length >> 4);
    burst_array[19] = (unsigned char)(length);
    return 0;
}

static jint bt_set_test_type(JNIEnv* env, jobject, jint type) {
    conti_array[3] = type;
    return 0;
}

static jint bt_set_test_payload(JNIEnv* env, jobject, jint payload) {
    le_tx_array[3] = payload;
    return 0;
}

static JNINativeMethod btTestMethods[] = {
        { "StartTest", "()I", (void *) bt_start_test },
        { "StopTest", "()I", (void *) bt_stop_test },
        { "ResetHCI", "()I", (void *) bt_reset_HCI },
        { "EnterHCIMode", "()I", (void *) bt_enter_HCI_mode },
        { "StartCMDMode", "()I", (void *) bt_start_cmd_mode },
        { "StopCMDMode", "()I", (void *) bt_stop_cmd_mode },
        { "StartOnlyBurstTest", "()I", (void *) bt_start_only_burst_test },
        { "StopOnlyBurstTest", "()I", (void *) bt_stop_only_burst_test },
        { "SetHopping", "(Z)I", (void *) bt_set_hopping },
        { "SetWhitening", "(Z)I", (void *) bt_set_whitening },
        { "SetChannel", "(I)I", (void *) bt_set_channel },
        { "SetChannelArray", "(IIIII)I", (void *) bt_set_channel_array },
        { "SetPower", "(I)I", (void *) bt_set_power },
        { "SetPacketType", "(I)I", (void *) bt_set_packet_type },
        { "SetTransPattern", "(I)I", (void *) bt_set_trans_pattern },
        { "SetLength", "(I)I", (void *) bt_set_length },
        { "StartContinuousTest", "()I", (void *) bt_start_continuous_test },
        { "StopContinuousTest", "()I", (void *) bt_stop_continuous_test },
        { "SetTestType", "(I)I", (void *) bt_set_test_type },
        { "StartLETest", "()I", (void *) bt_start_le_test },
        { "StopLETest", "()I", (void *) bt_stop_le_test },
        { "SetTestPayload", "(I)I", (void *) bt_set_test_payload },
};

/*
 * Register the methods.
 */
static int registerMethods(JNIEnv* env, const char* className, JNINativeMethod* methods,
        int numMethods) {
    jclass clazz;

    clazz = env->FindClass(className);
    if (clazz == NULL) {
        ALOGE("register methods, unable to find class '%s'\n", className);
        return JNI_FALSE;
    }
    if (env->RegisterNatives(clazz, methods, numMethods) < 0) {
        ALOGE("register methods, failed for '%s'\n", className);
        return JNI_FALSE;
    }

    return JNI_TRUE;
}

/*
 * Load the functions.
 */
jint JNI_OnLoad(JavaVM* vm, void* reserved) {
    JNIEnv* env = NULL;

    if (vm->GetEnv((void**) &env, JNI_VERSION_1_6) != JNI_OK) {
        ALOGE("ERROR Message: GetEnv failed\n");
        return -1;
    }

    if (!registerMethods(env, "com/qti/bttest/BtTestActivity",
            btTestMethods,
            sizeof(btTestMethods) / sizeof(btTestMethods[0]))) {
        ALOGE("ERROR Message: bt jni registration failed\n");
        return -1;
    }

    return JNI_VERSION_1_6;
}

