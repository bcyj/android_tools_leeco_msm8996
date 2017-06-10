#sources and intermediate files are separated
vpath %.c $(SRCDIR)/msm_uart

MSM_UART_TEST_CFLAGS := $(CFLAGS)
MSM_UART_TEST_CFLAGS += $(QCT_CFLAGS)
  
MSM_UART_TEST_CPPFLAGS := $(CPPFLAGS)
MSM_UART_TEST_CPPFLAGS += $(QCT_CPPFLAGS)

MSM_UART_TEST_LDLIBS := $(LDLIBS)
MSM_UART_TEST_LDLIBS += -lpthread

APP_NAME := msm_uart_test
SRCLIST  := msm_uart_test.c

all: $(APP_NAME)

$(APP_NAME): $(SRCLIST)
	$(CC) $(MSM_UART_TEST_CFLAGS) $(MSM_UART_TEST_CPPFLAGS) $(LDFLAGS) -o $@ $^ $(MSM_UART_TEST_LDLIBS)
