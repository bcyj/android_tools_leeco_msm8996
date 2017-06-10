# This makefile intended for ptxdist build environment.
# SRCDIR is exported from ptxdist makefile for the gps module (gps.make).
# SRCDIR points to $(PTXDIST_WORKSPACE)/../source/gps

vpath %.c $(SRCDIR)/daemon

# Default CPPFLAGS are exported by the ptxdist. Appened module specific flags.

CPPFLAGS   += $(QCT_CPPFLAGS)
CPPFLAGS   += -I$(SRCDIR)/daemon
CPPFLAGS   += -I$(API_SRCDIR)/libs/remote_apis/gpsone_bit_api/inc
CPPFLAGS   += -DFEATURE_DORMANCY_DISABLE

LDFLAGS  += -L$(SRCDIR)/../../project/platform-fsm9xxx_surf/sysroot-target/opt/qcom/lib
LDFLAGS  += -pthread -lrt

LDLIBS   +=  -lgpsone_bit_api -ldsm -loncrpc -lqueue -ldiag

APP_NAME := gpsonebit
SRCLIST  := gpsone_bit_forward.c \
	gpsone_conn_bridge.c \
	gpsone_conn_bridge_proc.c \
	gpsone_conn_client.c \
	gpsone_daemon_manager.c \
	gpsone_daemon_manager_handler.c \
	gpsone_glue_data_service.c \
	gpsone_glue_msg.c \
	gpsone_glue_pipe.c \
	gpsone_glue_rpc.c \
	gpsone_net.c \
	gpsone_thread_helper.c \
	gpsone_udp_modem.c \
	gpsone_udp_modem_proc.c


all: $(APP_NAME)

$(APP_NAME): $(SRCLIST)
	@ pwd
	$(CC) $(CPPFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)

.PHONY: clean
clean:
	rm -f gpsonebit *.o
