CFLAGS=-Wall -D LINUXPC

#Some Linux distros require added libraries
#LIBS = -lrt -lpthread 
MAIN = ks

OBJECTS=kickstart.o	                \
	hdlc.o			        \
	crc.o			        \
	comm.o			        \
	dload_protocol.o	        \
	sahara_protocol.o               \
	kickstart_log.o			\
    kickstart_utils.o		\

all: kickstart qcks

kickstart: $(OBJECTS)
qcks: qcks.o
clean:
	rm $(OBJECTS) ks qcks.o qcks
