XDR_DIR = ../../../../../../modem-apis/msm7630_surf/api/libs/remote_apis/gpsone_bit_api/xdr

BIN = bit_sample dmn_sample

GEN_SHARED_SRC = \
      gpsone_bit_api_xdr.c \
      gpsone_bit_api_cb_xdr.c \
      gpsone_bit_api_common_xdr.c \
      gpsone_bit_api_cb.h \
      gpsone_bit_api.h \
      gpsone_bit_api_common.h
GEN_SHARED_OBJS = $(patsubst %.c,%.o,$(GEN_SHARED_SRC))

GEN_BIT_SRC = \
      gpsone_bit_api_svc.c \
      gpsone_bit_api_cb_clnt.c
GEN_BIT_OBJS = $(patsubst %.c,%.o,$(GEN_BIT_SRC))

GEN_BIT_SAMPLE_SRC = \
      gpsone_bit_api_server.c \
      gpsone_bit_api_cb_client.c
GEN_BIT_SAMPLE_OBJS = $(patsubst %.c,%.o,$(GEN_BIT_SAMPLE_SRC))

GEN_DMN_SRC = \
      gpsone_bit_api_clnt.c \
      gpsone_bit_api_cb_svc.c
GEN_DMN_OBJS = $(patsubst %.c,%.o,$(GEN_DMN_SRC))

GEN_DMN_SAMPLE_SRC = \
      gpsone_bit_api_client.c \
      gpsone_bit_api_cb_server.c
GEN_DMN_SAMPLE_OBJS = $(patsubst %.c,%.o,$(GEN_DMN_SAMPLE_SRC))

GEN_SRC = \
      $(GEN_SHARED_SRC) \
      $(GEN_BIT_SRC) \
      $(GEN_BIT_SAMPLE_SRC) \
      $(GEN_DMN_SRC) \
      $(GEN_DMN_SAMPLE_SRC)
GEN_OBJS = $(patsubst %.c,%.o,$(GEN_SRC))

RPCCOM = rpcgen -a -b -C -M -N

BIT_SAMPLE_OBJS = $(GEN_BIT_OBJS) $(GEN_SHARED_OBJS) $(GEN_BIT_SAMPLE_OBJS)
DMN_SAMPLE_OBJS = $(GEN_DMN_OBJS) $(GEN_SHARED_OBJS) $(GEN_DMN_SAMPLE_OBJS)

all: $(GEN_SRC) $(GEN_OBJS) $(BIN)

bit_sample: $(BIT_SAMPLE_OBJS)
      $(CC) -o $@ $(BIT_SAMPLE_OBJS)

dmn_sample: $(DMN_SAMPLE_OBJS)
      $(CC) -o $@ $(DMN_SAMPLE_OBJS)

%.o : %.c
      $(CC) -Wall -g -c $(patsubst %.o,%.c,$@)

$(GEN_SRC): $(XDRS)
      cp $(XDR_DIR)/*xdr .
      $(RPCCOM) gpsone_bit_api.xdr
      $(RPCCOM) gpsone_bit_api_common.xdr
      $(RPCCOM) gpsone_bit_api_cb.xdr
      rm -f Makefile.gpsone_bit_api*
      rm *xdr

clean cleanup:
      rm -f $(GEN_SRC) *.o $(BIN)

