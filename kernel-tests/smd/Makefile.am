smddir = $(prefix)/smd

AM_CFLAGS = -Wall -Wundef -Wstrict-prototypes -Wno-trigraphs -Werror

AM_CPPFLAGS = -D__packed__= \
		-DIMAGE_APPS_PROC \
		-DFEATURE_Q_SINGLE_LINK \
		-DFEATURE_Q_NO_SELF_QPTR \
		-DFEATURE_LINUX \
		-DFEATURE_NATIVELINUX \
		-DFEATURE_DSM_DUP_ITEMS


smd_PROGRAMS  = smem_log_test
smd_PROGRAMS += smd_tty_loopback_test
smd_tty_loopback_test_LDADD = -lpthread -lrt

smd_PROGRAMS += smd_pkt_loopback_test
smd_pkt_loopback_test_LDADD = -lpthread -lrt

dist_smd_DATA = README
