Test: i2c-msm-test

Usage: i2c-msm-test [OPTIONS]...

OPTIONS can be (defaults in parenthesis):
  -D, --device          name of the device to test (/dev/i2c-0)
  -s, --slave_addr      slave address of the I2C device under test (0x52)
  -b, --block_size      size of data blocks (in bytes) to read/write (74 bytes)
  -n, --num_blocks      number of blocks to read/write per pass (20 blocks)
  -o, --offset          address offset from beginning of device (59 bytes)
  -i, --iterations      number of times (passes) to repeat the test (1 pass)
  -l, --scan            scans the i2c bus for available slaves (lists devices)
  -v, --verbose         run with debug messages on (off)
  -p, --probe           used to detect if the device is present without testing

Description:
This test is used to exercise the MSM I2C device driver by performing reads and
writes to a known EEPROM device (AT24C128B 128K 16,384 x 8).  This device is
present on most Qualcomm SURFs.

For running basic unit tests, the i2c-msm-test.sh script should be used.
The script uses /sys/devices/soc/soc0/build_id to get the target information.
By default, the EEPROM will be on the primary interface (/dev/i2c-0).
On 8x50 targets, the EEPROM is on auxilliary interface (/dev/i2c-1).
On 8660 targets, the EEPROM is on /dev/i2c-3 and for 8960 it's on /dev/i2c-10.
For targets beyond 8960, SW will try to assign /dev/i2c-0 to the I2C bus on
which EEPROM is located. That way the script doesn't have to keep changing.

Since the test is written for an EEPROM device, the reads and writes are
performed as blocks.  The available OPTIONS can be used to specify the size of
the blocks and the number of blocks to be tested.  The block size and number of
blocks don't need to match page size for the EEPROM device.  The test will
correctly handle blocks that cross page boundaries.

For slave addressed, some device manufacturers will specify the I2C address as
a write address (i.e. 0xA4) and a read address (0xA5).  The slave address
specified for this test is the write address logically shifted right by one bit.
(i.e. 0xA4 >> 1 = 0x52).

Target support: 7x27, 8x50, 8x50A, 7x30, 8x10, 8x26, 8x55, 8x60, 8974, 8x26
