Test: sd_test

Usage: ./sd_test.sh [-n | --nominal] [-i <block identifier>] [(-v | --verbosity) <verbosity>] [-h | --help]

Argument <block identifier> to "-i" option is the mmcblk identifier used for the SD file system mount
if not using "-i" option and no block identifier provided, the default block identifier for each target
will be used.

Description:
This test tests mount, umount, remount and file operations on sd card devices.

Target: 7x27, 8x50, 7x30, 8660, 8x50a

Notes:
(1) SD cards with vfat format is expected for the test.
(2) On Android adb shell, busybox installation is required for test to run.
(3) The block identifier (mmcblk<n>) that needs to be used for the file system
    mount is automatically set if not provided while invoking the test.
(4) To set the block id automatically, the target type is required. This is done by
    source $TEST_ENV_SETUP which is set to point to test_env_setup.sh. Therefore,
    setting TEST_ENV_SETUP is a prerequisite for this test script.
(5) Since the block identifier depends on not only target types but also product lines or
    whether WLAN is turned ON or OFF etc., there is a possibility that the auto-detected
    default block-id is not correct. Therefore, passing block id as argument is provided.
    To find out the block id, two approaches might be useful:
    - use dmesg and search for mmcblk and SD card info.
    - check which mmcblk device under /dev/block (for Android) disappears and shows up when
      remove card from card slot and then insert it
