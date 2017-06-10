AM_CFLAGS = -Wall -Wundef -Wstrict-prototypes -Wno-trigraphs -Werror

AM_CPPFLAGS = -D__packed__= \
		-DIMAGE_APPS_PROC \
		-DFEATURE_Q_SINGLE_LINK \
		-DFEATURE_Q_NO_SELF_QPTR \
		-DFEATURE_LINUX \
		-DFEATURE_NATIVELINUX \
		-DFEATURE_DSM_DUP_ITEMS

msm_uart_PROGRAMS = msm_uart_test

msm_uart_test_SOURCES = msm_uart_test.c
msm_uart_test_LDFLAGS = -lpthread
msm_uartdir = $(prefix)/msm_uart
dist_msm_uart_SCRIPTS = probe_test.sh
dist_msm_uart_SCRIPTS += msm_uart_test.sh
dist_msm_uart_SCRIPTS += run.sh ../test_env_setup.sh
dist_msm_uart_DATA    = README.txt
