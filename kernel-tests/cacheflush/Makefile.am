ACLOCAL_AMFLAGS = -I m4

AM_CFLAGS = -Wall -Wundef -Wstrict-prototypes -Wno-trigraphs -DKDEV

AM_CPPFLAGS = -D__packed__= \
		-DKDEV \
		-DIMAGE_APPS_PROC \
		-DFEATURE_Q_SINGLE_LINK \
		-DFEATURE_Q_NO_SELF_QPTR \
		-DFEATURE_LINUX \
		-DFEATURE_NATIVELINUX \
		-DFEATURE_DSM_DUP_ITEMS \
		-DFEATURE_MEMORY_VIRTUAL \
		-Duint32_t="unsigned int" \
		-Duint16_t="unsigned short" \
		-Duint8_t="unsigned char"

cacheflush_SOURCES = cacheflush.c
cacheflush_PROGRAMS = cacheflush
cacheflush_LDFLAGS = -lm -static

cacheflushdir = $(prefix)/cacheflush

cacheflush_SCRIPTS = cacheflush.sh loop.sh _cacheflush.sh run.sh
cacheflush_DATA = README.txt
