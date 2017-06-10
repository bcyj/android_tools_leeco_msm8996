AM_CFLAGS = -Wall \
		-Wundef \
		-Wstrict-prototypes \
		-Wno-trigraphs

AM_CPPFLAGS = -D__packed__= \
		-DIMAGE_APPS_PROC \
		-DFEATURE_Q_SINGLE_LINK \
		-DFEATURE_Q_NO_SELF_QPTR \
		-DFEATURE_LINUX \
		-DFEATURE_NATIVELINUX \
		-DFEATURE_DSM_DUP_ITEMS \
		-DFEATURE_LE_DIAG \
		-I../tests \
		-I../tests/wmsts \
		-I../../diag/include/ \
		-I../inc \
		-I../src \
		-I../platform \
		-I../../common/inc/ \
		-I../core/lib/inc \
		-I../services

qmi_simple_ril_test_SOURCES = \
	qmi_client_utils.c \
        qmi_simple_ril_core.c    \
        qmi_simple_ril_voice.c    \
	qmi_simple_ril_misc.c \
	qmi_simple_ril_nas.c \
	qmi_simple_ril_sms.c \
	qmi_simple_ril_ss.c \
        qmi_simple_ril_suite.c \
        qmi_test_console.c \
        fota.c \
        fota_cdma.c \
        wmsts/wmsts.c \
        wmsts/wmstscdma.c \
        wmsts/bit.c

qmi_simple_ril_test_LDFLAGS = -lpthread

qmi_simple_ril_test_LDADD = \
	../core/lib/src/libqmiidl.la \
	../services/libqmiservices.la \
	../src/libqmi.la \
        ../qmi_client_qmux/libqmi_client_qmux.la \
        ../qcci_legacy/libqcci_legacy.la

bin_PROGRAMS = qmi_simple_ril_test

qmi_simple_ril_test_CC = @CC@
