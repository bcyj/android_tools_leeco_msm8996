Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.

SecImage can be used for image signing and encryption to generate secure images.

<sectools>/
| sectools.py (main tool launcher command interface)
|
| –- bin/WIN (Windows binary to perform cryptographic operations)
| –- bin/LIN (Linux binary to perform cryptographic operations)
|
| -- config/                (chipset-specific config files directory)
| -- config/<chipset>/      (preconfigured templates directory)
| -- config/xsd             (xsd for config xml)
|
| -- sectools/features/isc/secimage.py (main Secimage python script)
| -- sectools/features/isc/            (main Secimage core code)
|
| -- resources/openssl          (OpenSSL configuration resources)
| -- resources/testpki          (QPSA Test PKIs)
| -- resources/unified_encryption_test_keys (encryption test keys)
|
| -- sectools/common/core       (infrastructure)
| -- sectools/common/crypto     (cryptographic services)
| -- sectools/common/parsegen   (image utilities)
| -- sectools/common/utils      (core utilities)

The config/<chipset>/<chipset>_secimage.xml config file is chipset specific, pre-populated with the most common values for all its parameters.
For the most part the user need not change this file, however, for any configuration requirement please look into this file first.

---------------------------------------------------------------------------------------------------
Example Usage of the tool
---------------------------------------------------------------------------------------------------
Quick Help:

python sectools.py secimage -h
Options:
  --version             show program's version number and exit
  -h, --help            show this help message
  -v, --verbose         enable more logging.

  Signing individual image:
    -i <file>, --image_file=<file>
                        path to the image file.
    -g <id>, --sign_id=<id>
                        sign id corresponding to the image_file provided.
    -p <id>, --chipset=<id>
                        id of the chipset corresponding to the image_file.
    -c <file>, --config_path=<file>
                        path to the secimage config file.

  Signing images from metabuild:
    -m <dir>, --meta_build=<dir>
                        path to the meta-build to be used for obtaining the
                        images to sign.

  Specifying output location:
    -o <dir>, --output_dir=<dir>
                        directory to store output files. DEFAULT:
                        "./secimage_output"
    -n <dir>, --mini_build=<dir>
                        path to the minimized build to store the signed images
                        to. This option works with the meta_build option.

  Operations:
    -t, --integrity_check  add hash table segment.
    -s, --sign             sign the image.
    -e, --encrypt          encrypt the image.
	-a, --validate         validate the image.
    -l, --verify_inputs    verify the command line options.

---------------------------------------------------------------------------------------------------
Signing entire meta build and validate the output:
---------------------------------------------------------------------------------------------------
python sectools.py secimage -m C:\A8084AAAAANLGD13001305.2 -c config\8084\8084_secimage.xml -o C:\output -sa

Where:
-m C:\A8084AAAAANLGD13001305.2 = Perform operations on all images in a build
-c config\8084_secimage.xml    = Path to chipset specific config xml
-o C:\output                   = Path to output folder. The tool save the signed images
								 as well as the run log in this folder.
-s                             = Perform Sign Operation
-a                             = Validate the output images

---------------------------------------------------------------------------------------------------
Sign single image and validate the output:
---------------------------------------------------------------------------------------------------
python sectools.py secimage -i C:\sbl1.mbn -c config\9x35\9x35_secimage.xml -o C:\output -g sbl1_mbn -sa

Where:
-i C:\sbl1.mbn 					= Perform operations on single image
-c config\9x35_secimage.xml  	= Path to chipset specific config xml
-o C:\output            		= Path to output folder. The tool save the signed images
						          as well as the run log in this folder.
-g sbl1_mbn                     = specify the sign_id as input image has different signing formats
-s                      		= Perform Sign Operation
-a                              = Validate the output image

---------------------------------------------------------------------------------------------------
Sign and encrypt single image from meta build and validate the output:
---------------------------------------------------------------------------------------------------
python sectools.py secimage -p 8994 -m c:\build\M8994AAAAANLGD000017.1 -g sdi -o C:\output -sea

Where:
-p 8994 			                = Select chipset to use config\<chipset>\<chipset>_secimage.xml
-m c:\build\M8994AAAAANLGD000017.1 	= Path to meta build
-g sdi                              = sign_id for the image user want to operate
-o C:\output                		= Path to output folder. The tool logs its output to this folder.
-s                          		= Perform Sign Operation
-e                          		= Perform Encryption Operation
-a                          		= Validate output image

---------------------------------------------------------------------------------------------------
Encrypt single image:
---------------------------------------------------------------------------------------------------
python sectools.py secimage -p 8916 -i c:\tz.mbn -o C:\output -e

Where:
-p 8916 			                = Select chipset to use config\<chipset>\<chipset>_secimage.xml
-i c:\tz.mbn 	                    = Path to image file
-o C:\output                		= Path to output folder. The tool logs its output to this folder.
-e                          		= Perform Encryption Operation

---------------------------------------------------------------------------------------------------
List all supported chipsets:
---------------------------------------------------------------------------------------------------
python sectools.py secimage -p LIST -h

---------------------------------------------------------------------------------------------------
List all sign_ids for a given chipset:
---------------------------------------------------------------------------------------------------
python sectools.py secimage -p <chipset> -g LIST -h


-----------------------------------------------------------------------------------------------------
Disclaimer
-----------------------------------------------------------------------------------------------------
The licensee takes responsibility for the use of the information and tools in the SecImage.
The licensee takes responsibility for what code they authorize. Should the licensee sign malware,
poorly behaving or poorly developed code, QC takes no responsibility for such code.

All the non-commercial root certificates packaged in resources are intended only for pre-commercial
device development and is NOT suitable for inclusion in commercial devices. Qualcomm makes no effort
to safeguard the security of the private key of the non-commercial root certificates or the private
key of the non-commercial CA certificates or the non-commercial CA certificates issued under the
non-commercial root certificates each of which are intended for distribution to multiple unknown
parties allowing such parties to generate signatures that will allow applications to execute on
devices which embed the non-commercial root certificate.

