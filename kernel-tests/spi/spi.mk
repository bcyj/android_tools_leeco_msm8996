#sources and intermediate files are separated
vpath %.c $(SRCDIR)/spi

SPI_TEST_CFLAGS := $(CFLAGS)
SPI_TEST_CFLAGS += $(QCT_CFLAGS)
  
SPI_TEST_CPPFLAGS := $(CPPFLAGS)
SPI_TEST_CPPFLAGS += $(QCT_CPPFLAGS)
  
APP_NAME := spitest
SRCLIST  := spitest.c

all: $(APP_NAME)

$(APP_NAME): $(SRCLIST)
	$(CC) $(SPI_TEST_CFLAGS) $(SPI_TEST_CPPFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)

