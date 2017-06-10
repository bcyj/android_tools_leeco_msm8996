Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.

These tools are used to generate Secure Software Download (SSD) binaries.

ReadMe.txt           - This file.
gen_keys.py          - Used to generate AES and RSA keys.
gen_keystore.py      - Used to generate a TZ keystore.
gen_ssd_bin.py       - Used to create a SSD binary.
key_config.xml       - Used to describe key files for keystore generation and
                       SSD binary generation.
ssd_bin.cfg          - Configuration file used by gen_ssd_bin.py to set
                       options for SSD binary generation.
key_config_parser.py - Helper module to parse key_config.xml.
zeros.dat            - Helper file used as an easy way to generate arbitrary
                       lengthed buffers with zeros.  Will be removed
                       eventually.


------------------------------------------------------------------------------
 Generating Required SSD files
------------------------------------------------------------------------------
1) Generate keys if no keys are already available.
  \> python gen_keys.py to see usage for this tool

2) Update key_config.xml to describe keys that should be used for binary
   encryption and keystore generation.  See the existing key_config.xml file
   for the format.

3) Generate TZ keystore
  \> python gen_keystore.py
  (This will output a file, "keystore.dat," in the "keys" subfolder)

4a) Update ssd_bin.cfg to set options for SSD binary creation

4b) Generate SSD binary or multiple SSD binaries using gen_ssd_bin.py
  \> python gen_ssd_bin.py to see usage for this tool

------------------------------------------------------------------------------
 Using SSD
------------------------------------------------------------------------------
1) Provision device with the TZ keystore generated above, keystore.dat
2) Any SSD image may now be passed to the secure or non-secure SSD APIs

------------------------------------------------------------------------------
 Provisioning Details
------------------------------------------------------------------------------
LA targets:
	\>fastboot flash ssd keystore.dat

2) Windows targets:
	TBD
==============================================================================


