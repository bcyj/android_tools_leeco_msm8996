fbtestdir = $(prefix)/fbtest

AM_CFLAGS = -Wall -Wundef -Wstrict-prototypes -Wno-trigraphs

AM_CPPFLAGS = -D__packed__= \
		-DIMAGE_APPS_PROC \
		-DFEATURE_Q_SINGLE_LINK \
		-DFEATURE_Q_NO_SELF_QPTR \
		-DFEATURE_LINUX \
		-DFEATURE_NATIVELINUX \
		-DFEATURE_DSM_DUP_ITEMS \
		-DFEATURE_MEMORY_VIRTUAL \
		-Duint32_t="unsigned int" \
		-Duint16_t="unsigned short" \
		-Duint8_t="unsigned char"\
		-DUSE_ION

fbtest_PROGRAMS  = fbtest
fbtest_SOURCES = fbtest.c fbtestUtils.c
fbtest_LDFLAGS   = -lm -lpthread

if USE_GLIB
   fbtest_CFLAGS = -DUSE_GLIB @GLIB_CFLAGS@
   fbtest_LDFLAGS += @GLIB_LIBS@
endif

if MDP3
AM_CPPFLAGS += -DMDP3_FLAG
fbtest_SOURCES += mdp3.c
endif

if MDP4
AM_CPPFLAGS += -DMDP4_FLAG
fbtest_SOURCES += mdp4.c
endif

if MDSS
# backwards compatible with MDP4
AM_CPPFLAGS += -DMDP4_FLAG -DVENUS_COLOR_FORMAT
fbtest_SOURCES += mdp4.c
endif

if QPIC
AM_CPPFLAGS += -DQPIC_FLAG
endif

dist_fbtest_SCRIPTS = run.sh
dist_fbtest_DATA = README.txt
