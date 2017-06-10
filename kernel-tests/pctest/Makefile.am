pctestdir           = $(prefix)/pctest

pctest_PROGRAMS     = pctest

pctest_LDFLAGS = -lpthread

pctest_SOURCES = main.c
pctest_SOURCES += idle_test_8x60.c
pctest_SOURCES += suspend_test_8x60.c
pctest_SOURCES += tools.c

dist_pctest_SCRIPTS = pc-compound-test.sh
dist_pctest_SCRIPTS += run.sh ../test_env_setup.sh

dist_pctest_DATA    = README
