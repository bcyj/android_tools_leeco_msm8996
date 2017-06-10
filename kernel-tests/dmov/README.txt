Test: dma_test

Usage: ./dma_test.sh [-n | --nominal] [-p <path to dma_test.ko>]
where
 -n: nominal test case
 -p: path to dma_test.ko modules (if not specified, default path will be used)


dma_test.sh is the script wrapper for dma_test test app. It tests if the
/dev/msmdma exists and if not loads the dma_test.ko module and then invoke dma_test.

Users can invoke dma_test directly with ./dma_test

Description:
This test tests the dma allocation, read, write and copy functionality.

Target support:
7201, 7x25, 7x27, 7x30, 8x50, 8x50a, 8x60

Note:
1) This test requires dma_test.ko loaded for /dev/msmdma.
2) Users can provide the module path to dma_test.sh and the default path is
   /system/lib/modules/dma_test.ko. However, one can not assume this default
   path remain unchanged. Therefore, user can specify the modules path to the
   script following -p flag.
