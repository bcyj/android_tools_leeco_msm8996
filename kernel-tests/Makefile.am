
ACLOCAL_AMFLAGS = -I m4

topdir           = $(prefix)
dist_top_SCRIPTS = test_env_setup.sh

BUILD_TEST=

if BIGTESTS
if USEFBTEST
BUILD_TEST += fbtest
endif

if USEROTATOR
BUILD_TEST += rotator
endif
endif

if USEADC
BUILD_TEST += adc
endif
if USECACHEFLUSH
BUILD_TEST += cacheflush
endif
if USECLK
BUILD_TEST += clk
endif
if USECLOCKSOURCE
BUILD_TEST += clocksource
endif
if USECPU
BUILD_TEST += cpu
endif
if USECPUFREQ
BUILD_TEST += cpufreq
endif
if USEDMOV
BUILD_TEST += dmov
endif
if USEGPIO_TLMM
BUILD_TEST += gpio_tlmm
endif
if USEI2C_MSM
BUILD_TEST += i2c-msm
endif
if USEIRQ
BUILD_TEST += irq
endif
if USEMPP
BUILD_TEST += mpp
endif
if USEMSM_UART
BUILD_TEST += msm_uart
endif
if USEMSM_UART_HS
BUILD_TEST += msm_uart_hs
endif
if USEMTD
BUILD_TEST += mtd
endif
if USEPCTEST
BUILD_TEST += pctest
endif
if USEQCEDEV
BUILD_TEST += qcedev
endif
if USESDCC
BUILD_TEST += sdcc
endif
if USESMD
BUILD_TEST += smd
endif
if USESOCINFO
BUILD_TEST += socinfo
endif
if USESPI
BUILD_TEST += spi
endif
if USESPS
BUILD_TEST += sps
endif
if USETIMER
BUILD_TEST += timer
endif
if USEVREG
BUILD_TEST += vreg
endif
if USEVFP
BUILD_TEST += vfp
endif
if USEUSF
BUILD_TEST += usf
endif

SUBDIRS = $(BUILD_TEST)

