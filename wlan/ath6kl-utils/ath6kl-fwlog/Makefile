ALL=ath6kl-fwlog-record ath6kl-fwlog-reorder

all: $(ALL)

ifndef CC
CC=gcc
endif

ifndef LDO
LDO=$(CC)
endif

ifndef CFLAGS
CFLAGS = -MMD -O2 -Wall -g
endif

ROBJS=ath6kl-fwlog-record.o

ath6kl-fwlog-record: $(ROBJS)
	$(LDO) $(LDFLAGS) -o $@ $(ROBJS) $(LIBS)

OOBJS=ath6kl-fwlog-reorder.o

ath6kl-fwlog-reorder: $(OOBJS)
	$(LDO) $(LDFLAGS) -o $@ $(OOBJS) $(LIBS)

clean:
	rm -f core *~ *.o *.d $(ALL)

-include $(OBJS:%.o=%.d)
