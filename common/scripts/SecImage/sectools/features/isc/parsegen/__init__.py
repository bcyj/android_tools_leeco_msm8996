#===============================================================================
#
# Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
# Qualcomm Technologies Proprietary and Confidential.
#
#===============================================================================

# API to register the parsers
def _register_parser(file_type, parser):
    if file_type in PARSEGENS:
        raise RuntimeError('Trying to register parser: ' + str(parser) + ' for file type: ' + str(file_type) + '\n'
                           'Another parser: ' + str(PARSEGENS[file_type]) + ' is already registered for this file type')
    PARSEGENS[file_type] = parser

# API to query a parser
def get_parser(imageinfo):

    # Import within this method
    import os
    from sectools.common.utils.c_misc import load_data_from_file

    # Get source image path
    image_path = imageinfo.src_image.image_path
    image_name = os.path.basename(image_path)
    image_base, image_ext = os.path.splitext(image_name)

    # Check parser exists
    file_type = imageinfo.image_type.file_type
    if file_type not in PARSEGENS:
        raise RuntimeError('Parser unavailable for file type: ' + str(file_type) + '\n'
                           '    ' + 'Available file types are: ' + str(PARSEGENS.keys()))

    # Get the parser
    parser = PARSEGENS[file_type]
    return parser(load_data_from_file(image_path), imageinfo=imageinfo,
                  debug_dir=imageinfo.dest_image.debug_dir_parsegen,
                  debug_prefix=image_base,
                  debug_suffix=image_ext)

# Make sure this module is not already initialized
try:
    ONETIME
except Exception:
    ONETIME = True

if ONETIME is True:
    # Map of file_type to parser
    PARSEGENS = {}

    # List of parsers that are available
    from sectools.features.isc.parsegen import elf
    from sectools.features.isc.parsegen import ewm
    from sectools.features.isc.parsegen import mbn
    from sectools.features.isc.parsegen import bin

    # Dont do this again
    ONETIME = False
