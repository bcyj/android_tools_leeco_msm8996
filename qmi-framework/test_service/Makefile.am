AM_CFLAGS = -Wundef \
        -Wstrict-prototypes \
        -Wno-trigraphs \
        -g -O0 \
        -fno-inline \
        -fno-short-enums \
        -fpic \
        $(QMI_CFLAGS)

AM_CPPFLAGS = \
        -I../inc \
        -I../qcci/inc \
        -I../common/inc

h_sources =

requiredlibs = -lc \
        -lrt \
        ../qcci/src/libqmi_cci.la \
        ../common/src/libqmi_common_so.la \
	../encdec/libqmi_encdec.la \
        $(QMI_LIBS)

common_sources = test_service_v01.c \
        qmi_test_service_clnt_common.c \
        qmi_test_service_clnt_common_stats.c

bin_PROGRAMS = qmi_test_service_test
qmi_test_service_test_SOURCES = qmi_test_service_clnt.c test_service_v01.c
qmi_test_service_test_LDADD = $(requiredlibs)
qmi_test_service_test_CC = @CC@

bin_PROGRAMS += qmi_test_service_clnt_test_0000
qmi_test_service_clnt_test_0000_SOURCES = qmi_test_service_clnt_test_0000.c $(common_sources)
qmi_test_service_clnt_test_0000_LDADD = $(requiredlibs)
qmi_test_service_clnt_test_0000_CC = @CC@

bin_PROGRAMS += qmi_test_service_clnt_test_0001
qmi_test_service_clnt_test_0001_SOURCES = qmi_test_service_clnt_test_0001.c $(common_sources)
qmi_test_service_clnt_test_0001_LDADD = $(requiredlibs)
qmi_test_service_clnt_test_0001_CC = @CC@

bin_PROGRAMS += qmi_test_service_clnt_test_1000
qmi_test_service_clnt_test_1000_SOURCES = qmi_test_service_clnt_test_1000.c $(common_sources)
qmi_test_service_clnt_test_1000_LDADD = $(requiredlibs)
qmi_test_service_clnt_test_1000_CC = @CC@

bin_PROGRAMS += qmi_test_service_clnt_test_1001
qmi_test_service_clnt_test_1001_SOURCES = qmi_test_service_clnt_test_1001.c $(common_sources)
qmi_test_service_clnt_test_1001_LDADD = $(requiredlibs)
qmi_test_service_clnt_test_1001_CC = @CC@

bin_PROGRAMS += qmi_test_service_clnt_test_2000
qmi_test_service_clnt_test_2000_SOURCES = qmi_test_service_clnt_test_2000.c $(common_sources)
qmi_test_service_clnt_test_2000_LDADD = $(requiredlibs)
qmi_test_service_clnt_test_2000_CC = @CC@

bin_PROGRAMS += qmi_test_mt_client_init_instance
qmi_test_mt_client_init_instance_SOURCES = qmi_test_mt_client_init_instance.c $(common_sources)
qmi_test_mt_client_init_instance_LDADD = $(requiredlibs)
qmi_test_mt_client_init_instance_CC = @CC@
