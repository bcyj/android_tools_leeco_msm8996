/*
 *Copyright (c) 2011-2012 Qualcomm Atheros Inc. All Rights Reserved.
 *Qualcomm Atheros Proprietary and Confidential.
 *
 */

#include <sys/types.h>
#include <sys/file.h>
#include <sys/socket.h>
#include <linux/types.h>
#include <net/if.h>

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <sys/types.h>
#include <stdint.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "athtestcmd.h"
#include "testcmd.h"
#include "libtcmd.h"

const char *progname;
const char commands[] = "commands:\n"
	"--tx <frame/tx99/tx100/sine/off>\n"
	"--txfreq <Tx channel or freq(default 2412)>\n"
	"--txrate <rate index>\n"
	"--txpwr <0-30dBm, 0.5 dBm resolution; sine: 0-60, PCDAC vaule>\n"
	"--tgtpwr <target power>\n"
	"--pcdac <power control DAC>\n"
	"--txantenna <1/2/0 (auto)>\n"
	"--txpktsz <pkt size, [32-1500](default 1500)>\n"
	"--txpattern <tx data pattern, 0: all zeros; 1: all ones;"
	" 2: repeating 10; 3: PN7; 4: PN9; 5: PN15\n"
	"--txchain (1:on chain 0, 2:on chain 1, 3:on both)\n"
	"--tpcm <forcedGain/txpwr/tgtpwr> : Set transmit power control mode,"
	" by default tpcm is set to txpwr\n"
	"--ani (Enable ANI. The ANI is disabled if this option is not"
	" specified)\n"
	"--paprd (Enable PAPRD. The PAPRD is disabled if this option is not"
	" specified)\n"
	"--stbc (Enable STBC. STBC is disabled if this option is not"
	" specified)\n"
	"--ldpc (Enable LDPC. LDPC is disabled if this option is not"
	" specified)\n"
	"--scrambleroff (Disable scrambler. The scrambler is enabled"
	" by default)\n"
	"--aifsn <AIFS slots num,[0-252](Used only under '--tx frame' mode)>\n"
	"--shortguard (use short guard)\n"
	"--mode <ht40plus/ht40minus/ht20>\n"
	"--bw <full/half/quarter>\n"
	"--setlongpreamble <1/0>\n"
	"--numpackets <number of packets to send 0-65535>\n"
	"--tx sine --txfreq <Tx channel or freq(default 2412)>\n"
	"--agg <number of aggregate packets>\n"
	"--bssid\n"
	"--srcmac <source MAC address>\n"
	"--dstmac <destination MAC address>\n"
	"--tx sine --txfreq <Tx channel or freq(default 2412)>\n"
	"--rx <promis/filter/report>\n"
	"--rxfreq <Rx channel or freq(default 2412)>\n"
	"--rxantenna <1/2/0 (auto)>\n"
	"--rxchain <1:chain 0, 2:chain 1, 3:both>\n"
	"--mode <ht40plus/ht40minus>\n"
	"--ack <Enable/Disable ACK, 0:disable; 1:enable, by default ACK"
	" is enabled>\n"
	"--bc <0:receive unicast frame, 1:receive broadcast frame (default)>\n"
	"--lpl <0:LPL off(default), 1:reduced receive, 2:reduced search>\n"
	"--pm <wakeup/sleep/deepsleep>\n"
	"--setmac <mac addr like 00:03:7f:be:ef:11>\n"
	"--setbssid <mac addr like 00:03:7f:be:ef:11>\n"
	"--SetAntSwitchTable <table1 in hex value> <table2 in hex value>"
	" (Set table1=0 and table2=0 will restore the default AntSwitchTable)\n"
	"--efusedump --start <start address> --end <end address>\n"
	"--efusewrite --start <start address> --data <data> (could be one or"
	" multiple data in quotation marks)\n"
	"--otpwrite --data (could be one or multiple data in quotation marks)\n"
	"--otpdump\n"
	"--btmode <put DUT into BT mode>\n";

#define A_ERR(ret, args...)	{		\
				printf(args);	\
				exit(ret);	\
				}

#define A_FREQ_MIN              4920
#define A_FREQ_MAX              5825

#define A_CHAN0_FREQ            5000
#define A_CHAN_MAX              ((A_FREQ_MAX - A_CHAN0_FREQ)/5)

#define BG_FREQ_MIN             2412
#define BG_FREQ_MAX             2484

#define BG_CHAN0_FREQ           2407
#define BG_CHAN_MIN             ((BG_FREQ_MIN - BG_CHAN0_FREQ)/5)
#define BG_CHAN_MAX             14      /* corresponding to 2484 MHz */

#define A_20MHZ_BAND_FREQ_MAX   5000

#define INVALID_FREQ    0

#define ATH6KL_INTERFACE "wlan0"

#ifdef CONFIG_AR6002_REV6
#define A_RATE_NUM      47
#define G_RATE_NUM      47
#else
#define A_RATE_NUM      28
#define G_RATE_NUM      28
#endif

#define VENUS_OTP_SIZE  512
static TC_CMDS sTcCmds;
#define MAX_FRAME_LENGTH         1500
/*the following 4 values are in micro seconds*/
#define TX_FRAME_DURATION        340
#define PREAMBLE_TIME_OFDM       20
#define PREAMBLE_TIME_CCK_LONG   192
#define PREAMBLE_TIME_CCK_SHORT  96
#define SLOT_TIME                9
#define SLOT_TIME_LONG           20
#define MAX_NUM_SLOTS	         255

#define RATE_STR_LEN    30
/* typedef const char RATE_STR[RATE_STR_LEN]; */

const char bgRateStrTbl[G_RATE_NUM][RATE_STR_LEN] = {
	{"1   Mb"},
	{"2   Mb"},
	{"5.5 Mb"},
	{"11  Mb"},
	{"6   Mb"},
	{"9   Mb"},
	{"12  Mb"},
	{"18  Mb"},
	{"24  Mb"},
	{"36  Mb"},
	{"48  Mb"},
	{"54  Mb"},
	{"HT20 MCS0 6.5   Mb"},
	{"HT20 MCS1 13    Mb"},
	{"HT20 MCS2 19.5  Mb"},
	{"HT20 MCS3 26    Mb"},
	{"HT20 MCS4 39    Mb"},
	{"HT20 MCS5 52    Mb"},
	{"HT20 MCS6 58.5  Mb"},
	{"HT20 MCS7 65    Mb"},
#ifdef CONFIG_AR6002_REV6
	{"HT20 MCS8 13    Mb"},
	{"HT20 MCS9 26    Mb"},
	{"HT20 MCS10 39   Mb"},
	{"HT20 MCS11 52   Mb"},
	{"HT20 MCS12 78   Mb"},
	{"HT20 MCS13 104  Mb"},
	{"HT20 MCS14 117  Mb"},
	{"HT20 MCS15 130  Mb"},
#endif
	{"HT40 MCS0 13.5    Mb"},
	{"HT40 MCS1 27.0    Mb"},
	{"HT40 MCS2 40.5    Mb"},
	{"HT40 MCS3 54      Mb"},
	{"HT40 MCS4 81      Mb"},
	{"HT40 MCS5 108     Mb"},
	{"HT40 MCS6 121.5   Mb"},
	{"HT40 MCS7 135     Mb"},
#ifdef CONFIG_AR6002_REV6
	{"HT40 MCS8 27      Mb"},
	{"HT40 MCS9 54      Mb"},
	{"HT40 MCS10 81     Mb"},
	{"HT40 MCS11 108    Mb"},
	{"HT40 MCS12 162    Mb"},
	{"HT40 MCS13 216    Mb"},
	{"HT40 MCS14 243    Mb"},
	{"HT40 MCS15 270    Mb"},
#endif
/* below portion(CCK short preamble rates should always be placed to the end */
	{"2(S)   Mb"},
	{"5.5(S) Mb"},
	{"11(S)  Mb"}
};

const float ActualDataRate[] = {
/* CCK, 0-3 */            1.0,  2.0,  5.5,  11.0,
/* OFDM, 4-11 */          6.0,  9.0,  12.0, 18.0, 24.0, 36.0, 48.0, 54.0,
/* MCS 20 1S, 12-19 */    6.5,  13.0, 19.5, 26.0, 39.0, 52.0, 58.5, 65.0,
#ifdef CONFIG_AR6002_REV6
/* MCS 20 2S, 20-27 */    13.0, 26.0, 39.0, 52.0, 78.0, 104.0, 117.0, 130.0,
#endif
/* MCS 40 1S, 28-35 */    13.5, 27.0, 40.5, 54.0, 81.0, 108.0, 121.5, 135.0,
#ifdef CONFIG_AR6002_REV6
/* MCS 40 2S, 36-43 */    27.0, 54.0, 81.0, 108.0, 162.0, 216.0, 243.0, 270.0,
#endif
/* CCK, 44-45 */          2.0, 5.5, 11, 0
};

/* arbitary macAddr */
static uint8_t BSSID_DEFAULT[ATH_MAC_LEN] = {0x00, 0x03, 0x7f,
						0x03, 0x40, 0x33};

static void rx_cb(void *buf, int len);
static void rxReport(void *buf);
static uint32_t freqValid(uint32_t val);
static uint16_t wmic_ieee2freq(uint32_t chan);
static void prtRateTbl(uint32_t freq);
static uint32_t rateValid(uint32_t val, uint32_t freq);
static uint32_t antValid(uint32_t val);
static uint32_t txPwrValid(TCMD_CONT_TX *txCmd);
static int ath_ether_aton(const char *orig, uint8_t *eth);
static uint32_t pktSzValid(uint32_t val);
static int32_t computeNumSlots(uint32_t dutyCycle, uint32_t dataRateIndex,
				float txFrameDurationPercent);
static void cmdReplyFunc_readThermal(void *buf);
static void cmdReplyFunc(void *buf);

static bool isHex(char c)
{
	bool result;
	result = (((c >= '0') && (c <= '9')) || ((c >= 'A') && (c <= 'F')) ||
			((c >= 'a') && (c <= 'f')));
	return result;
}

static void usage(void)
{
	fprintf(stderr, "usage:\n%s [-i device] commands\n", progname);
	fprintf(stderr, "%s\n", commands);
	prtRateTbl(INVALID_FREQ);
	A_ERR(-1, "Incorrect usage");
}

/* copied from ath6kl */
enum ar6k_testmode_attr {
	__AR6K_TM_ATTR_INVALID  = 0,
	AR6K_TM_ATTR_CMD        = 1,
	AR6K_TM_ATTR_DATA       = 2,
	AR6K_TM_ATTR_STREAM_ID  = 3,

	/* keep last */
	__AR6K_TM_ATTR_AFTER_LAST,
	AR6K_TM_ATTR_MAX        = __AR6K_TM_ATTR_AFTER_LAST - 1
};

enum ar6k_testmode_cmd {
	AR6K_TM_CMD_TCMD                = 0,
};

/* FIXME: find a proper value */
#define AR6K_TM_DATA_MAX_LEN 5000

/* end copy */

int main(int argc, char **argv)
{
	void (*reportCB)(void *) = NULL;
	void (*cmdRespCB)(void *) = cmdReplyFunc_readThermal;
	int c, s = -1, err, devidx;
	unsigned int cmd = 0;
	progname = argv[0];
	char buf[2048];
	TCMD_CONT_TX *txCmd = (TCMD_CONT_TX *)buf;
	TCMD_CONT_RX *rxCmd   = (TCMD_CONT_RX *)buf;
	TCMD_PM *pmCmd = (TCMD_PM *)buf;
	TCMD_SET_REG *setRegCmd = (TCMD_SET_REG *)buf;
	TC_CMDS  *tCmds = (TC_CMDS *)buf;
	bool resp = false;
	uint32_t bufferLen = sizeof(*txCmd);

#ifndef ATHTESTCMD_LIB
	uint16_t efuse_begin = 0, efuse_end = (VENUS_OTP_SIZE - 1);
	uint8_t  efuseBuf[VENUS_OTP_SIZE];
	uint8_t  efuseWriteBuf[VENUS_OTP_SIZE];
	uint16_t data_length = 0;
#endif

	memset(buf, 0, sizeof(buf));

	txCmd->numPackets = 0;
	txCmd->wlanMode = TCMD_WLAN_MODE_NOHT;
	txCmd->tpcm = TPC_TX_PWR;
	txCmd->txChain = 1;	/* default to use chain 0 */
	txCmd->broadcast = 1;	/* default to broadcast */
	txCmd->agg = 1;
	txCmd->miscFlags = 0;
	txCmd->bandwidth = 0;
	memcpy(txCmd->bssid, BSSID_DEFAULT, ATH_MAC_LEN);
	rxCmd->u.para.wlanMode = TCMD_WLAN_MODE_NOHT;

	rxCmd->u.para.ack = true;
	rxCmd->u.para.rxChain = 1;	/* default to use chain 0 */
	rxCmd->u.para.bc = 1;		/* default to receive broadcast frame */
	rxCmd->u.para.freq = 2412;

	if (argc == 1)
		usage();

	while (1) {
		int option_index = 0;
		static struct option long_options[] = {
		{"version", 0, NULL, 'v'},
		{"interface", 1, NULL, 'i'},
		{"tx", 1, NULL, 't'},
		{"txfreq", 1, NULL, 'f'},
		{"txrate", 1, NULL, 'g'},
		{"txpwr", 1, NULL, 'h'},
		{"tgtpwr", 0, NULL, 'H'},
		{"pcdac", 1, NULL, 'I'},
		{"txantenna", 1, NULL, 'j'},
		{"txpktsz", 1, NULL, 'z'},
		{"txpattern", 1, NULL, 'e'},
		{"txchain", 1, NULL, 'c'},
		{"rxchain", 1, NULL, 'C'},
		{"bssid", 1, NULL, 'b'},
		{"srcmac", 1, NULL, 'd'},
		{"dstmac", 1, NULL, 'D'},
		{"rx", 1, NULL, 'r'},
		{"rxfreq", 1, NULL, 'p'},
		{"rxantenna", 1, NULL, 'q'},
		{"ack", 1, NULL, 'k'},
		{"bc", 1, NULL, 'K'},
		{"pm", 1, NULL, 'x'},
		{"setmac", 1, NULL, 's'},
		{"setbssid", 1, NULL, 'X'},
		{"ani", 0, NULL, 'a'},
		{"paprd", 0, NULL, 'A'},
		{"stbc", 0, NULL, 'Y'},
		{"ldpc", 0, NULL, 'Z'},
		{"scrambleroff", 0, NULL, 'o'},
		{"aifsn", 1, NULL, 'u'},
		{"dutycyc", 1, NULL, 'U'},
		{"SetAntSwitchTable", 1, NULL, 'S'},
		{"shortguard", 0, NULL, 'G'},
		{"numpackets", 1, NULL, 'n'},
		{"agg", 1, NULL, 'N'},
		{"mode", 1, NULL, 'M'},
		{"bw", 1, NULL, 'm'},
		{"setlongpreamble", 1, NULL, 'l'},
		{"setreg", 1, NULL, 'R'},
		{"regval", 1, NULL, 'V'},
		{"flag", 1, NULL, 'F'},
		{"writeotp", 0, NULL, 'w'},
		{"otpregdmn", 1, NULL, 'E'},
#ifndef ATHTESTCMD_LIB
		{"efusedump", 0, NULL, TCMD_EFUSE_DUMP},
		{"efusewrite", 0, NULL, TCMD_EFUSE_WRITE},
		{"start", 1, NULL, TCMD_EFUSE_START},
		{"end", 1, NULL, TCMD_EFUSE_END},
		{"data", 1, NULL, TCMD_EFUSE_DATA},
		{"otpwrite", 0, NULL, TCMD_OTP_WRITE},
		{"otpdump", 0, NULL, TCMD_OTP_DUMP},
#endif
		{"btaddr", 1, NULL, 'B'},
		{"therm", 0, NULL, TCMD_READ_THERMAL},
		{"tpcm", 1, NULL, 'P'},
		{"lpl", 1, NULL, TCMD_SET_RX_LPL},
		{"btmode", 0, NULL, TCMD_SET_BT_MODE},
		{0, 0, 0, 0}
		};

		c = getopt_long(argc, argv,
				"vi:t:f:g:h:HI:j:z:e:c:C:b:d:D:r:p:q:"
				"k:K:x:s:X:aAYZouU:S:Gn:N:M:m:l:P:",
				long_options, &option_index);
		if (c == -1)
			break;

		switch (c) {
		case 'i':
			break;
		case 't':
			cmd = TESTMODE_CONT_TX;
			txCmd->testCmdId = TCMD_CONT_TX_ID;
			if (!strcmp(optarg, "sine"))
				txCmd->mode = TCMD_CONT_TX_SINE;
			else if (!strcmp(optarg, "frame"))
				txCmd->mode = TCMD_CONT_TX_FRAME;
			else if (!strcmp(optarg, "tx99"))
				txCmd->mode = TCMD_CONT_TX_TX99;
			else if (!strcmp(optarg, "tx100"))
				txCmd->mode = TCMD_CONT_TX_TX100;
			else if (!strcmp(optarg, "off"))
				txCmd->mode = TCMD_CONT_TX_OFF;
			else
				cmd = 0;
			break;
		case 'f':
			txCmd->freq = freqValid(atoi(optarg));
			break;
		case 'G':
			txCmd->shortGuard = 1;
			break;
		case 'P':
			if (cmd == TESTMODE_CONT_TX) {
				if (!strcmp(optarg, "forcedGain"))
					txCmd->tpcm = TPC_FORCED_GAIN;
			}
			break;
		case 'M':
			if (cmd == TESTMODE_CONT_TX) {
				if (!strcmp(optarg, "ht20"))
					txCmd->wlanMode = TCMD_WLAN_MODE_HT20;
				else if (!strcmp(optarg, "ht40plus"))
					txCmd->wlanMode =
						TCMD_WLAN_MODE_HT40PLUS;
				else if (!strcmp(optarg, "ht40minus"))
					txCmd->wlanMode =
						TCMD_WLAN_MODE_HT40MINUS;
			} else if (cmd == TESTMODE_CONT_RX) {
				if (!strcmp(optarg, "ht20"))
					rxCmd->u.para.wlanMode =
						TCMD_WLAN_MODE_HT20;
				else if (!strcmp(optarg, "ht40plus"))
					rxCmd->u.para.wlanMode =
						TCMD_WLAN_MODE_HT40PLUS;
				else if (!strcmp(optarg, "ht40minus"))
					rxCmd->u.para.wlanMode =
						TCMD_WLAN_MODE_HT40MINUS;
			}
			break;
		case 'm':
			if (cmd == TESTMODE_CONT_TX) {
				if (!strcmp(optarg, "half"))
					txCmd->bandwidth = HALF_SPEED_MODE;
				else if (!strcmp(optarg, "quarter"))
					txCmd->bandwidth = QUARTER_SPEED_MODE;
			} else if (cmd == TESTMODE_CONT_RX) {
				if (!strcmp(optarg, "half"))
					rxCmd->u.para.bandwidth =
						HALF_SPEED_MODE;
				else if (!strcmp(optarg, "quarter"))
					rxCmd->u.para.bandwidth =
						QUARTER_SPEED_MODE;
			}
			break;
		case 'n':
			txCmd->numPackets = atoi(optarg);
			break;
		case 'N':
			txCmd->agg = atoi(optarg);
			break;
		case 'g':
			/* let user input index of rateTable
			* instead of string parse
			*/
			txCmd->dataRate = rateValid(atoi(optarg), txCmd->freq);
			break;
		case 'h':
			{
			int txPowerAsInt;
			/* Get tx power from user.
			* This is given in the form of a number that's supposed
			* to be either an integer, or an integer + 0.5
			*/
			double txPowerIndBm = atof(optarg);

			/*
			* Check to make sure that the number given is
			*  either an integer or an integer + 0.5
			*/
			txPowerAsInt = (int)txPowerIndBm;
			if (((txPowerIndBm - (double)txPowerAsInt) == 0) ||
				(((txPowerIndBm - (double)txPowerAsInt)) == 0.5)
				|| (((txPowerIndBm - (double)txPowerAsInt))
				== -0.5)) {
				if (txCmd->mode != TCMD_CONT_TX_SINE)
					txCmd->txPwr = txPowerIndBm * 2;
				else
					txCmd->txPwr = txPowerIndBm;
			} else {
				printf("Bad argument to --txpwr,"
					" must be in steps of 0.5 dBm\n");
				cmd = 0;
			}

			txCmd->tpcm = TPC_TX_PWR;
			break;
			}
		case 'H':
			txCmd->tpcm = TPC_TGT_PWR;
			break;
		case 'I':
			txCmd->tpcm = TPC_FORCED_GAIN;
			txCmd->txPwr = atof(optarg);
			break;
		case 'j':
			txCmd->antenna = antValid(atoi(optarg));
			break;
		case 'z':
			txCmd->pktSz = pktSzValid(atoi(optarg));
			break;
		case 'e':
			txCmd->txPattern = atoi(optarg);
			break;
		case 'c':
			txCmd->txChain = atoi(optarg);
			break;
		case 'C':
			rxCmd->u.para.rxChain = atoi(optarg);
			break;
		case 'b':
			{
			uint8_t mac[ATH_MAC_LEN];

			if (ath_ether_aton(optarg, mac) != 0)
				A_ERR(-1, "Invalid bssid address format!\n");
			memcpy(txCmd->bssid, mac, ATH_MAC_LEN);
#ifdef TCMD_DEBUG
			printf("JLU: tcmd: set bssid 0x%02x, 0x%02x, 0x%02x,"
				" 0x%02x, 0x%02x, 0x%02x\n",
				mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
#endif
			break;
			}
		case 'd':
			{
			uint8_t mac[ATH_MAC_LEN];

			if (ath_ether_aton(optarg, mac) != 0)
				A_ERR(-1, "Invalid srcmac address format!\n");
			memcpy(txCmd->txStation, mac, ATH_MAC_LEN);
#ifdef TCMD_DEBUG
			printf("JLU: tcmd: set srcmac 0x%02x, 0x%02x, 0x%02x,"
				" 0x%02x, 0x%02x, 0x%02x\n",
				mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
#endif
			break;
			}
		case 'D':
			{
			uint8_t mac[ATH_MAC_LEN];

			if (ath_ether_aton(optarg, mac) != 0)
				A_ERR(-1, "Invalid dstmac address format!\n");
			memcpy(txCmd->rxStation, mac, ATH_MAC_LEN);
#ifdef TCMD_DEBUG
			printf("JLU: tcmd: set dstmac 0x%02x, 0x%02x, 0x%02x,"
				" 0x%02x, 0x%02x, 0x%02x\n",
				mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
#endif
			/* if destination MAC address is present
			then set broadcast to off */
			txCmd->broadcast = 0;
			break;
			}
		case 'r':
			cmd = TESTMODE_CONT_RX;
			rxCmd->testCmdId = TCMD_CONT_RX_ID;

			if (!strcmp(optarg, "promis")) {
				rxCmd->act = TCMD_CONT_RX_PROMIS;
				printf(" Its cont Rx promis mode\n");
			} else if (!strcmp(optarg, "filter")) {
				rxCmd->act = TCMD_CONT_RX_FILTER;
				printf(" Its cont Rx  filter mode\n");
			} else if (!strcmp(optarg, "report")) {
				printf(" Its cont Rx report mode\n");
					rxCmd->act = TCMD_CONT_RX_REPORT;
					resp = true;
			} else
				cmd = 0;
			break;
		case 'p':
			rxCmd->u.para.freq = freqValid(atoi(optarg));
			break;
		case 'q':
			rxCmd->u.para.antenna = antValid(atoi(optarg));
			break;
		case 'k':
			rxCmd->u.para.ack = atoi(optarg);
			break;
		case 'K':
			rxCmd->u.para.bc = atoi(optarg);
			break;
		case TCMD_SET_RX_LPL:
			rxCmd->u.para.lpl = atoi(optarg);
			break;
		case 'x':
			cmd = TESTMODE_PM;
			pmCmd->testCmdId = TCMD_PM_ID;
			if (!strcmp(optarg, "wakeup"))
				pmCmd->mode = TCMD_PM_WAKEUP;
			else if (!strcmp(optarg, "sleep"))
				pmCmd->mode = TCMD_PM_SLEEP;
			else if (!strcmp(optarg, "deepsleep"))
				pmCmd->mode = TCMD_PM_DEEPSLEEP;
			else
				cmd = 0;
			break;
		case 's':
			{
			uint8_t mac[ATH_MAC_LEN];

			cmd = TESTMODE_CONT_RX;
			rxCmd->testCmdId = TCMD_CONT_RX_ID;
			rxCmd->act = TCMD_CONT_RX_SETMAC;
			if (ath_ether_aton(optarg, mac) != 0)
				A_ERR(-1, "Invalid mac address format!\n");
			memcpy(rxCmd->u.mac.addr, mac, ATH_MAC_LEN);
#ifdef TCMD_DEBUG
			printf("JLU: tcmd: setmac 0x%02x, 0x%02x, 0x%02x,"
				" 0x%02x, 0x%02x, 0x%02x\n",
				mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
#endif
			break;
			}
		case 'X':
			{
			uint8_t mac[ATH_MAC_LEN];

			cmd = TESTMODE_CONT_RX;
			rxCmd->testCmdId = TCMD_CONT_RX_ID;
			rxCmd->act = TCMD_CONT_RX_SETMAC;
			if (ath_ether_aton(optarg, mac) != 0)
				A_ERR(-1, "Invalid bssid address format!\n");
			/* memcpy(rxCmd->u.mac.bssid, mac, ATH_MAC_LEN); */
#ifdef TCMD_DEBUG
			printf("JLU: tcmd: setbssid 0x%02x, 0x%02x, 0x%02x,"
				" 0x%02x, 0x%02x, 0x%02x\n",
				mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
#endif
			break;
			}
		case 'u':
			{
			txCmd->aifsn = atoi(optarg) & 0xff;
			printf("AIFS:%d\n", txCmd->aifsn);
			break;
			}
		case 'U':
			{
			int dutyCycle, frameLength;
			float txFrameDurPercent;

			frameLength =
				(uint32_t) (((TX_FRAME_DURATION + 20) * 2 *
					ActualDataRate[txCmd->dataRate]) / 8.0);
			txFrameDurPercent = 1.0;
			if (frameLength > MAX_FRAME_LENGTH) {
				txFrameDurPercent = ((float) MAX_FRAME_LENGTH) /
							(float) frameLength;
				frameLength = MAX_FRAME_LENGTH;
			}

			dutyCycle = atoi(optarg) & 0xff;
			if (dutyCycle == 0)
				dutyCycle = 1;
			if (dutyCycle > 99)
				dutyCycle = 99;

			txCmd->aifsn = computeNumSlots(dutyCycle,
					txCmd->dataRate, txFrameDurPercent);
			txCmd->pktSz = frameLength;
			}
			break;
		case 'a':
			if (cmd == TESTMODE_CONT_TX)
				txCmd->enANI = true;
			else if (cmd == TESTMODE_CONT_RX)
				rxCmd->enANI = true;
			break;
		case 'A':
			txCmd->miscFlags |= PAPRD_ENA_MASK;
			break;
		case 'Y':
			txCmd->miscFlags |= DESC_STBC_ENA_MASK;
			break;
		case 'Z':
			txCmd->miscFlags |= DESC_LDPC_ENA_MASK;
			break;
		case 'o':
			txCmd->scramblerOff = true;
			break;
		case 'S':
			if (argc < 4)
				usage();
			cmd = TESTMODE_CONT_RX;
			rxCmd->testCmdId = TCMD_CONT_RX_ID;
			rxCmd->act = TCMD_CONT_RX_SET_ANT_SWITCH_TABLE;
			rxCmd->u.antswitchtable.antswitch1 = strtoul(argv[2],
							(char **)NULL, 0);
			rxCmd->u.antswitchtable.antswitch2 = strtoul(argv[3],
							(char **)NULL, 0);
			break;
		case 'l':
			/* TODO */
			/* setLpreambleCmd->status = atoi(optarg); */
			printf("This is not supported\n");
			break;
		case 'R':
			if (argc < 5)
				printf("usage:athtestcmd -i eth0 --setreg"
					" 0x1234 --regval 0x01 --flag 0\n");
			cmd = TESTMODE_SETREG;
			setRegCmd->testCmdId = TCMD_SET_REG_ID;
			setRegCmd->regAddr = strtoul(optarg, (char **)NULL, 0);
			/* atoi(optarg); */
			break;
		case 'V':
			setRegCmd->val = strtoul(optarg, (char **)NULL, 0);
			break;
		case 'F':
			setRegCmd->flag = atoi(optarg);
			break;
		case 'w':
			rxCmd->u.mac.otpWriteFlag = 1;
			break;
		case 'E':
			rxCmd->u.mac.regDmn[0] = 0xffff &
				(strtoul(optarg, (char **)NULL, 0));
			rxCmd->u.mac.regDmn[1] = 0xffff &
				(strtoul(optarg, (char **)NULL, 0)>>16);
			break;
		case 'B':
			{
			uint8_t btaddr[ATH_MAC_LEN];
			if (ath_ether_aton(optarg, btaddr) != 0)
				A_ERR(-1, "Invalid mac address format!\n");
			memcpy(rxCmd->u.mac.btaddr, btaddr, ATH_MAC_LEN);
#ifdef TCMD_DEBUG
			printf("JLU: tcmd: setbtaddr 0x%02x, 0x%02x,"
				" 0x%02x, 0x%02x, 0x%02x, 0x%02x\n",
				btaddr[0], btaddr[1], btaddr[2],
				btaddr[3], btaddr[4], btaddr[5]);
#endif
			break;
			}
		case TCMD_READ_THERMAL:
			cmd = TESTMODE_CMDS;
			tCmds->hdr.testCmdId = TC_CMDS_ID;
			tCmds->hdr.u.parm.length = (uint16_t)0;
			tCmds->hdr.u.parm.version = TC_CMDS_VERSION_TS;
			tCmds->hdr.act = TC_CMDS_READTHERMAL;
					/*TC_CMDS_CAL_THERMALVOLT*/
			break;

#ifndef ATHTESTCMD_LIB
		case TCMD_EFUSE_START:
			efuse_begin = atoi(optarg);
			break;

		case TCMD_EFUSE_END:
			efuse_end = atoi(optarg);
			break;

		case TCMD_EFUSE_DATA:
			{
			uint8_t *pucArg = (uint8_t *)optarg;
			uint8_t  c;
			uint8_t  strBuf[256];
			uint8_t  pos = 0;
			uint16_t length = 0;
			uint32_t  data;

			/* Sweep string to end */
			while (1) {
				c = *pucArg++;
				if (isHex(c))
					strBuf[pos++] = c;
				else {
					strBuf[pos] = '\0';
					pos = 0;
					sscanf(((char *)&strBuf), "%x", &data);
					efuseWriteBuf[length++] = (data & 0xFF);

					/* End of arg string */
					if (c == '\0')
						break;
				}
			}

			data_length = length;
			break;
			}

		case TCMD_EFUSE_DUMP:
			cmd = TESTMODE_CMDS;
			tCmds->hdr.testCmdId      = TC_CMDS_ID;
			tCmds->hdr.act            = TC_CMDS_EFUSEDUMP;
			cmdRespCB = cmdReplyFunc;
			break;

		case TCMD_EFUSE_WRITE:
			cmd = TESTMODE_CMDS;
			tCmds->hdr.testCmdId      = TC_CMDS_ID;
			tCmds->hdr.act            = TC_CMDS_EFUSEWRITE;
			cmdRespCB = cmdReplyFunc;
			break;

		case TCMD_OTP_WRITE:
			cmd = TESTMODE_CMDS;
			tCmds->hdr.testCmdId      = TC_CMDS_ID;
			tCmds->hdr.act            = TC_CMDS_OTPSTREAMWRITE;
			cmdRespCB = cmdReplyFunc;
			break;

		case TCMD_OTP_DUMP:
			cmd = TESTMODE_CMDS;
			tCmds->hdr.testCmdId      = TC_CMDS_ID;
			tCmds->hdr.act            = TC_CMDS_OTPDUMP;
			cmdRespCB = cmdReplyFunc;
			break;
		case TCMD_SET_BT_MODE:
			cmd = TESTMODE_CMDS;
			tCmds->hdr.testCmdId      = TC_CMDS_ID;
			tCmds->hdr.act            = TC_CMDS_SET_BT_MODE;
			tCmds->hdr.u.parm.length  = 0;
			tCmds->hdr.u.parm.version = TC_CMDS_VERSION_TS;
			cmdRespCB = cmdReplyFunc;
			break;
#endif

		default:
			usage();
		}
	}

	/* default bufferLen is len of txCmd.. */
	if (cmd == TESTMODE_CONT_RX)
		bufferLen = sizeof(*rxCmd);
	else if (cmd == TESTMODE_PM)
		bufferLen = sizeof(*pmCmd);
	else if (cmd == TESTMODE_SETREG)
		bufferLen = sizeof(*setRegCmd);
	else if (cmd == TESTMODE_CMDS)
		bufferLen = sizeof(*tCmds);
	err = tcmd_tx_init(ATH6KL_INTERFACE, rx_cb);
	if (err)
		return err;

	err = tcmd_tx(buf, bufferLen /* weak */, resp);
	if (err)
		fprintf(stderr, "tcmd_tx had error: %s!\n", strerror(err));
	return 0;
}

static void rxReport(void *buf)
{
	uint16_t rateCnt[TCMD_MAX_RATES];
	uint16_t rateCntShortGuard[TCMD_MAX_RATES];
	uint16_t rcidx = 0;
	uint16_t rcsidx = 0;

	TCMD_CONT_RX *rxReport = (TCMD_CONT_RX *)buf;

	printf("total pkt %d ; crcError pkt %d ; secErr pkt %d ;"
		" average rssi %d\n", rxReport->u.report.totalPkt,
		rxReport->u.report.crcErrPkt, rxReport->u.report.secErrPkt,
		(int32_t)(rxReport->u.report.totalPkt ?
		(rxReport->u.report.rssiInDBm /
		(int32_t)rxReport->u.report.totalPkt) : 0));

	memcpy(rateCnt, rxReport->u.report.rateCnt, sizeof(rateCnt));
	memcpy(rateCntShortGuard, rxReport->u.report.rateCntShortGuard,
		sizeof(rateCntShortGuard));

	if (rateCnt[rcidx])
		printf("1Mbps      %d\n", rateCnt[rcidx]);
	rcidx++;
	if (rateCnt[rcidx])
		printf("2Mbps(L)   %d\n", rateCnt[rcidx]);
	rcidx++;
	if (rateCnt[TCMD_MAX_RATES-3])
		printf("2Mbps(S)   %d\n", rateCnt[TCMD_MAX_RATES-3]);
	if (rateCnt[rcidx])
		printf("5.5Mbps(L) %d\n", rateCnt[rcidx]);
	rcidx++;
	if (rateCnt[TCMD_MAX_RATES-2])
		printf("5.5Mbps(S) %d\n", rateCnt[TCMD_MAX_RATES-2]);
	if (rateCnt[rcidx])
		printf("11Mbps(L)  %d\n", rateCnt[rcidx++]);
	rcidx++;
	if (rateCnt[TCMD_MAX_RATES-1])
		printf("11Mbps(S)  %d\n", rateCnt[TCMD_MAX_RATES-1]);
	if (rateCnt[rcidx])
		printf("6Mbps      %d\n", rateCnt[rcidx]);
	rcidx++;
	if (rateCnt[rcidx])
		printf("9Mbps      %d\n", rateCnt[rcidx]);
	rcidx++;
	if (rateCnt[rcidx])
		printf("12Mbps     %d\n", rateCnt[rcidx]);
	rcidx++;
	if (rateCnt[rcidx])
		printf("18Mbps     %d\n", rateCnt[rcidx]);
	rcidx++;
	if (rateCnt[rcidx])
		printf("24Mbps     %d\n", rateCnt[rcidx]);
	rcidx++;
	if (rateCnt[rcidx])
		printf("36Mbps     %d\n", rateCnt[rcidx]);
	rcidx++;
	if (rateCnt[rcidx])
		printf("48Mbps     %d\n", rateCnt[rcidx]);
	rcidx++;
	if (rateCnt[rcidx])
		printf("54Mbps     %d\n", rateCnt[rcidx]);
	rcidx++;
	if (rateCnt[rcidx] || rateCntShortGuard[rcsidx])
		printf("HT20 MCS0 6.5Mbps   %d (SGI: %d)\n",
			rateCnt[rcidx], rateCntShortGuard[rcsidx]);
	rcidx++; rcsidx++;
	if (rateCnt[rcidx] || rateCntShortGuard[rcsidx])
		printf("HT20 MCS1 13Mbps    %d (SGI: %d)\n",
			rateCnt[rcidx], rateCntShortGuard[rcsidx]);
	rcidx++; rcsidx++;
	if (rateCnt[rcidx] || rateCntShortGuard[rcsidx])
		printf("HT20 MCS2 19.5Mbps  %d (SGI: %d)\n",
			rateCnt[rcidx], rateCntShortGuard[rcsidx]);
	rcidx++; rcsidx++;
	if (rateCnt[rcidx] || rateCntShortGuard[rcsidx])
		printf("HT20 MCS3 26Mbps    %d (SGI: %d)\n",
			rateCnt[rcidx], rateCntShortGuard[rcsidx]);
	rcidx++; rcsidx++;
	if (rateCnt[rcidx] || rateCntShortGuard[rcsidx])
		printf("HT20 MCS4 39Mbps    %d (SGI: %d)\n",
			rateCnt[rcidx], rateCntShortGuard[rcsidx]);
	rcidx++; rcsidx++;
	if (rateCnt[rcidx] || rateCntShortGuard[rcsidx])
		printf("HT20 MCS5 52Mbps    %d (SGI: %d)\n",
			rateCnt[rcidx], rateCntShortGuard[rcsidx]);
	rcidx++; rcsidx++;
	if (rateCnt[rcidx] || rateCntShortGuard[rcsidx])
		printf("HT20 MCS6 58.5Mbps  %d (SGI: %d)\n",
			rateCnt[rcidx], rateCntShortGuard[rcsidx]);
	rcidx++; rcsidx++;
	if (rateCnt[rcidx] || rateCntShortGuard[rcsidx])
		printf("HT20 MCS7 65Mbps    %d (SGI: %d)\n",
			rateCnt[rcidx], rateCntShortGuard[rcsidx]);
	rcidx++; rcsidx++;
#ifdef CONFIG_AR6002_REV6
	if (rateCnt[rcidx] || rateCntShortGuard[rcsidx])
		printf("HT20 MCS8 13Mbps    %d (SGI: %d)\n",
			rateCnt[rcidx], rateCntShortGuard[rcsidx]);
	rcidx++; rcsidx++;
	if (rateCnt[rcidx] || rateCntShortGuard[rcsidx])
		printf("HT20 MCS9 26Mbps    %d (SGI: %d)\n",
			rateCnt[rcidx], rateCntShortGuard[rcsidx]);
	rcidx++; rcsidx++;
	if (rateCnt[rcidx] || rateCntShortGuard[rcsidx])
		printf("HT20 MCS10 39Mbps   %d (SGI: %d)\n",
			rateCnt[rcidx], rateCntShortGuard[rcsidx]);
	rcidx++; rcsidx++;
	if (rateCnt[rcidx] || rateCntShortGuard[rcsidx])
		printf("HT20 MCS11 52Mbps   %d (SGI: %d)\n",
			rateCnt[rcidx], rateCntShortGuard[rcsidx]);
	rcidx++; rcsidx++;
	if (rateCnt[rcidx] || rateCntShortGuard[rcsidx])
		printf("HT20 MCS12 78Mbps   %d (SGI: %d)\n",
			rateCnt[rcidx], rateCntShortGuard[rcsidx]);
	rcidx++; rcsidx++;
	if (rateCnt[rcidx] || rateCntShortGuard[rcsidx])
		printf("HT20 MCS13 104Mbps  %d (SGI: %d)\n",
			rateCnt[rcidx], rateCntShortGuard[rcsidx]);
	rcidx++; rcsidx++;
	if (rateCnt[rcidx] || rateCntShortGuard[rcsidx])
		printf("HT20 MCS14 117Mbps  %d (SGI: %d)\n",
			rateCnt[rcidx], rateCntShortGuard[rcsidx]);
	rcidx++; rcsidx++;
	if (rateCnt[rcidx] || rateCntShortGuard[rcsidx])
		printf("HT20 MCS15 130Mbps  %d (SGI: %d)\n",
			rateCnt[rcidx], rateCntShortGuard[rcsidx]);
	rcidx++; rcsidx++;
#endif
	if (rateCnt[rcidx] || rateCntShortGuard[rcsidx])
		printf("HT40 MCS0 13.5Mbps  %d (SGI: %d)\n",
			rateCnt[rcidx], rateCntShortGuard[rcsidx]);
	rcidx++; rcsidx++;
	if (rateCnt[rcidx] || rateCntShortGuard[rcsidx])
		printf("HT40 MCS1 27.0Mbps  %d (SGI: %d)\n",
			rateCnt[rcidx], rateCntShortGuard[rcsidx]);
	rcidx++; rcsidx++;
	if (rateCnt[rcidx] || rateCntShortGuard[rcsidx])
		printf("HT40 MCS2 40.5Mbps  %d (SGI: %d)\n",
			rateCnt[rcidx], rateCntShortGuard[rcsidx]);
	rcidx++; rcsidx++;
	if (rateCnt[rcidx] || rateCntShortGuard[rcsidx])
		printf("HT40 MCS3 54Mbps    %d (SGI: %d)\n",
			rateCnt[rcidx], rateCntShortGuard[rcsidx]);
	rcidx++; rcsidx++;
	if (rateCnt[rcidx] || rateCntShortGuard[rcsidx])
		printf("HT40 MCS4 81Mbps    %d (SGI: %d)\n",
			rateCnt[rcidx], rateCntShortGuard[rcsidx]);
	rcidx++; rcsidx++;
	if (rateCnt[rcidx] || rateCntShortGuard[rcsidx])
		printf("HT40 MCS5 108Mbps   %d (SGI: %d)\n",
			rateCnt[rcidx], rateCntShortGuard[rcsidx]);
	rcidx++; rcsidx++;
	if (rateCnt[rcidx] || rateCntShortGuard[rcsidx])
		printf("HT40 MCS6 121.5Mbps %d (SGI: %d)\n",
			rateCnt[rcidx], rateCntShortGuard[rcsidx]);
	rcidx++; rcsidx++;
	if (rateCnt[rcidx] || rateCntShortGuard[rcsidx])
		printf("HT40 MCS7 135Mbps   %d (SGI: %d)\n",
			rateCnt[rcidx], rateCntShortGuard[rcsidx]);
	rcidx++; rcsidx++;
#ifdef CONFIG_AR6002_REV6
	if (rateCnt[rcidx] || rateCntShortGuard[rcsidx])
		printf("HT40 MCS8 27Mbps    %d (SGI: %d)\n",
			rateCnt[rcidx], rateCntShortGuard[rcsidx]);
	rcidx++; rcsidx++;
	if (rateCnt[rcidx] || rateCntShortGuard[rcsidx])
		printf("HT40 MCS9 54Mbps    %d (SGI: %d)\n",
			rateCnt[rcidx], rateCntShortGuard[rcsidx]);
	rcidx++; rcsidx++;
	if (rateCnt[rcidx] || rateCntShortGuard[rcsidx])
		printf("HT40 MCS10 81Mbps   %d (SGI: %d)\n",
			rateCnt[rcidx], rateCntShortGuard[rcsidx]);
	rcidx++; rcsidx++;
	if (rateCnt[rcidx] || rateCntShortGuard[rcsidx])
		printf("HT40 MCS11 108Mbps  %d (SGI: %d)\n",
			rateCnt[rcidx], rateCntShortGuard[rcsidx]);
	rcidx++; rcsidx++;
	if (rateCnt[rcidx] || rateCntShortGuard[rcsidx])
		printf("HT40 MCS12 162Mbps  %d (SGI: %d)\n",
			rateCnt[rcidx], rateCntShortGuard[rcsidx]);
	rcidx++; rcsidx++;
	if (rateCnt[rcidx] || rateCntShortGuard[rcsidx])
		printf("HT40 MCS13 216Mbps  %d (SGI: %d)\n",
			rateCnt[rcidx], rateCntShortGuard[rcsidx]);
	rcidx++; rcsidx++;
	if (rateCnt[rcidx] || rateCntShortGuard[rcsidx])
		printf("HT40 MCS14 243Mbps  %d (SGI: %d)\n",
			rateCnt[rcidx], rateCntShortGuard[rcsidx]);
	rcidx++; rcsidx++;
	if (rateCnt[rcidx] || rateCntShortGuard[rcsidx])
		printf("HT40 MCS15 270Mbps  %d (SGI: %d)\n",
			rateCnt[rcidx], rateCntShortGuard[rcsidx]);
	rcidx++; rcsidx++;
#endif
	printf("\n");
}

static void rx_cb(void *buf, int len)
{
	TCMD_ID tcmd;

	printf("Got rx_cb callback\n");

	tcmd = *((uint32_t *) buf + 1);

	switch (tcmd) {
	case TCMD_CONT_RX_REPORT:
		rxReport(buf);
		break;
	default:
		break;
	}

}

static uint32_t freqValid(uint32_t val)
{
	do {
		if (val <= A_CHAN_MAX) {
			uint16_t freq;

			if (val < BG_CHAN_MIN)
				break;

			freq = wmic_ieee2freq(val);
			if (INVALID_FREQ == freq)
				break;
			else
				return freq;
		}

		if ((val == BG_FREQ_MAX) ||
			((val < BG_FREQ_MAX) && (val >= BG_FREQ_MIN) &&
			!((val - BG_FREQ_MIN) % 5)))
			return val;
		else if ((val >= A_FREQ_MIN) && (val < A_20MHZ_BAND_FREQ_MAX) &&
			!((val - A_FREQ_MIN) % 20))
			return val;
		else if ((val >= A_20MHZ_BAND_FREQ_MAX) &&
				(val <= A_FREQ_MAX) &&
				!((val - A_20MHZ_BAND_FREQ_MAX) % 5))
			return val;
	} while (false);

	A_ERR(-1, "Invalid channel or freq #: %d !\n", val);
	return 0;
}

static uint32_t rateValid(uint32_t val, uint32_t freq)
{
	if (((freq >= A_FREQ_MIN) && (freq <= A_FREQ_MAX) &&
		(val >= A_RATE_NUM)) || ((freq >= BG_FREQ_MIN) &&
		(freq <= BG_FREQ_MAX) && (val >= G_RATE_NUM))) {
		printf("Invalid rate value %d for frequency %d!\n", val, freq);
		prtRateTbl(freq);
		A_ERR(-1, "Invalid rate value %d for frequency %d!\n",
			val, freq);
	}

	return val;
}

static void prtRateTbl(uint32_t freq)
{
	int i;

	for (i = 0; i < G_RATE_NUM; i++)
		printf("<rate> %d \t \t %s\n", i, bgRateStrTbl[i]);
	printf("\n");
}

/*
 * converts ieee channel number to frequency
 */
static uint16_t wmic_ieee2freq(uint32_t chan)
{
	uint16_t ret = INVALID_FREQ;
	if (chan == BG_CHAN_MAX)
		return BG_FREQ_MAX;
	if (chan < BG_CHAN_MAX)	{	/* 0-13 */
		ret = BG_CHAN0_FREQ + (chan * 5);
		return ret;
	}
	if (chan <= A_CHAN_MAX) {
		ret = A_CHAN0_FREQ + (chan * 5);
		return ret;
	}

	return ret;
}

static uint32_t antValid(uint32_t val)
{
	if (val > 2)
		A_ERR(-1, "Invalid antenna setting! <0: auto;"
				"  1/2: ant 1/2>\n");

	return val;
}

static uint32_t txPwrValid(TCMD_CONT_TX *txCmd)
{
#if 0
	if (txCmd->mode == TCMD_CONT_TX_SINE) {
		if ((txCmd->txPwr >= 0) && (txCmd->txPwr <= 60))
			return txCmd->txPwr;
	} else if (txCmd->mode != TCMD_CONT_TX_OFF) {
		if (txCmd->tpcm != TPC_FORCED_GAIN) {
			if ((txCmd->txPwr >= -30) && (txCmd->txPwr <= 60))
				return txCmd->txPwr;
		} else {
			if ((txCmd->txPwr >= 0) && (txCmd->txPwr <= 120))
				return txCmd->txPwr;
		}
	} else if (txCmd->mode == TCMD_CONT_TX_OFF) {
		return 0;
	}

	A_ERR(1, "Invalid Tx Power value!\nTx data: [-15 - 14]dBm\n"
		"Tx sine: [  0 - 60]PCDAC value\n");
	return 0;
#endif
	return txCmd->txPwr;
}

static uint32_t pktSzValid(uint32_t val)
{
	if ((val < 32) || (val > 1500))
		A_ERR(-1, "Invalid package size! < 32 - 1500 >\n");
	return val;
}
#ifdef NOTYET

/* Validate a hex character */
static bool _is_hex(char c)
{
	bool ret;
	ret = (((c >= '0') && (c <= '9')) ||
		((c >= 'A') && (c <= 'F')) ||
		((c >= 'a') && (c <= 'f')));
	return ret;
}

/* Convert a single hex nibble */
static int _from_hex(char c)
{
	int ret = 0;

	if ((c >= '0') && (c <= '9'))
		ret = (c - '0');
	else if ((c >= 'a') && (c <= 'f'))
		ret = (c - 'a' + 0x0a);
	else if ((c >= 'A') && (c <= 'F'))
		ret = (c - 'A' + 0x0A);
	return ret;
}

/* Convert a character to lower case */
static char _tolower(char c)
{
	if ((c >= 'A') && (c <= 'Z'))
		c = (c - 'A') + 'a';
	return c;
}

/* Validate alpha */
static bool isalpha(int c)
{
	bool ret;
	ret = (((c >= 'a') && (c <= 'z')) || ((c >= 'A') && (c <= 'Z')));
	return ret;
}

/* Validate digit */
static bool isdigit(int c)
{
	bool ret;
	ret = ((c >= '0') && (c <= '9'));
	return ret;
}

/* Validate alphanum */
static bool isalnum(int c)
{
	bool ret;
	ret = (isalpha(c) || isdigit(c));
	return ret;
}
#endif

/*------------------------------------------------------------------*/
/*
 * Input an Ethernet address and convert to binary.
 */
static int ath_ether_aton(const char *orig, uint8_t *eth)
{
	int mac[6];
	if (sscanf(orig, "%02x:%02x:%02X:%02X:%02X:%02X",
		&mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5]) == 6) {
		int i;
#ifdef DEBUG
		if (*(orig+12+5) != 0) {
			fprintf(stderr, "%s: trailing junk '%s'!\n",
				__func__, orig);
			return A_EINVAL;
		}
#endif
		for (i = 0; i < 6; ++i)
			eth[i] = mac[i] & 0xff;
		return 0;
	}
	return -1;
}

static int32_t computeNumSlots(uint32_t dutyCycle, uint32_t dataRateIndex,
				float txFrameDurationPercent)
{
	uint32_t frameLength = TX_FRAME_DURATION, slotTime;
	int32_t  numSlots;
	frameLength =
		(uint32_t) ((float)TX_FRAME_DURATION * txFrameDurationPercent);
	if ((dataRateIndex >= 0 && dataRateIndex <= 3) ||
		(dataRateIndex >= 44 && dataRateIndex <= 45)) {
		if (dataRateIndex >= 44 && dataRateIndex <= 45)
			frameLength += PREAMBLE_TIME_CCK_SHORT;
		else
			frameLength += PREAMBLE_TIME_CCK_LONG;
		slotTime = SLOT_TIME;
	} else {
		frameLength += PREAMBLE_TIME_OFDM;
		slotTime = SLOT_TIME;
	}
	numSlots = ((int32_t)(((float)frameLength * (100 - dutyCycle)) /
				((float)dutyCycle * slotTime)));
	if (numSlots > MAX_NUM_SLOTS)
		numSlots = MAX_NUM_SLOTS;
	return numSlots;

}

static void cmdReplyFunc(void *buf)
{
	uint8_t  *reply = (uint8_t *)buf;
	TC_CMDS  tCmdReply;
	uint32_t act;
	/* uint32_t i; */

	tCmdReply.hdr.u.parm.length = *(uint16_t *)&(reply[0]);
	tCmdReply.hdr.u.parm.version = (uint8_t)(reply[2]);
	act = tCmdReply.hdr.u.parm.version;

	/* Error Check */
	if (tCmdReply.hdr.u.parm.length > (TC_CMDS_SIZE_MAX + 1)) {
		printf("Error: Reply lenth=%d, limit=%d\n",
			tCmdReply.hdr.u.parm.length, TC_CMDS_SIZE_MAX);
		return;
	} else {
		printf(">> Reply length = %d, type = %d ",
			tCmdReply.hdr.u.parm.length,
			tCmdReply.hdr.u.parm.version);
	}

	switch (act) {
	case TC_CMDS_EFUSEDUMP:
		printf("eFuse data:\n");
		break;
	case TC_CMDS_EFUSEWRITE:
		printf("(write eFuse data)\n");
		break;
	case TC_CMDS_OTPSTREAMWRITE:
		printf("(OTP stream write)\n");
		break;
	case TC_CMDS_OTPDUMP:
		printf("OTP Dump\n");
		break;
	case TC_CMDS_SET_BT_MODE:
		printf("\nDUT is in BT mode\n");
		break;
	default:
		printf("Invalid action!\n");
		break;
	}

	if (tCmdReply.hdr.u.parm.length > 0) {
		/* Copy the data field */
		memcpy((void *)&(tCmdReply.buf), (void *)(buf+4),
			tCmdReply.hdr.u.parm.length);

		memcpy((void *)&(sTcCmds.buf[0]), (void *)&(tCmdReply.buf[0]),
			tCmdReply.hdr.u.parm.length);
		sTcCmds.hdr.u.parm.length = tCmdReply.hdr.u.parm.length;
	}

/*	printf("reply len %d ver %d act %d buf %x %x %x %x %x %x %x %x\n",
	tCmdReply.hdr.u.parm.length, tCmdReply.hdr.u.parm.version,
	tCmdReply.hdr.act, tCmdReply.buf[0], tCmdReply.buf[1],
	tCmdReply.buf[2], tCmdReply.buf[3], tCmdReply.buf[4],
	tCmdReply.buf[5], tCmdReply.buf[6], tCmdReply.buf[7]);
*/

	return;
}

static void cmdReplyFunc_readThermal(void *buf)
{
	uint8_t *reply = (uint8_t *)buf;
	TC_CMDS tCmdReply;
	tCmdReply.hdr.u.parm.length = *(uint16_t *)&(reply[0]);
	tCmdReply.hdr.u.parm.version = (uint8_t)(reply[2]);
	memcpy((void *)&(tCmdReply.buf), (void *)(buf+4),
		tCmdReply.hdr.u.parm.length);

	printf("chip thermal value:%d\n", tCmdReply.buf[0]);
/*
	printf("reply len %d ver %d act %d buf %x %x %x %x %x %x %x %x\n",
		tCmdReply.hdr.u.parm.length, tCmdReply.hdr.u.parm.version,
		tCmdReply.hdr.act, tCmdReply.buf[0], tCmdReply.buf[1],
		tCmdReply.buf[2], tCmdReply.buf[3], tCmdReply.buf[4],
		tCmdReply.buf[5], tCmdReply.buf[6], tCmdReply.buf[7]);
*/
	return;
}
