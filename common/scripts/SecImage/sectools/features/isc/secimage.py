#===============================================================================
#
# Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
# Qualcomm Technologies Proprietary and Confidential.
#
#===============================================================================

"""Provides a command line interface to the services provided by isc.

.. data:: SECIMAGE_TOOL_NAME

    Name of the tool

.. data:: SECIMAGE_TOOL_VERSION

    Version of the tool

.. data:: DEF_SECIMAGE_OUTPUT_DIR_NAME

    Name of the default output directory

.. data:: DEF_SECIMAGE_OUTPUT_DIR_PATH

    Absolute path of the default output directory
"""

import os
import optparse
import sys
import traceback

sys.path.append(os.path.join(os.path.dirname(os.path.realpath(__file__)), '../../..'))

from sectools.common.utils import c_path
from sectools.common.utils.c_base import dynamicToolStatus, CoreOptionParser
from sectools.common.utils.c_logging import logger
from sectools.common.utils.c_misc import TablePrinter
from sectools.features.isc import SecImageCore


# Tool name for command arg
CMD_ARG_TOOL_NAME = 'secimage'

# Name & version of the tool
SECIMAGE_TOOL_NAME = 'SecImage'
SECIMAGE_TOOL_VERSION = '2.9'

# Path definitions
DEF_SECIMAGE_OUTPUT_DIR_NAME = CMD_ARG_TOOL_NAME + '_output'
DEF_SECIMAGE_OUTPUT_DIR_PATH = c_path.join(dynamicToolStatus.toolDir, DEF_SECIMAGE_OUTPUT_DIR_NAME)

__version__ = SECIMAGE_TOOL_NAME + ' ' + SECIMAGE_TOOL_VERSION


class SecImageParser(CoreOptionParser):
    """Parser for command line arguments supported by SecImage."""

    _LIST_TAG = 'LIST'

    @property
    def c_description(self):
        """(str) Returns the description of the program."""
        return 'This program signs, encrypts & validates images'

    @property
    def c_version(self):
        """(str) Returns the version of the program."""
        return __version__

    @property
    def c_epilog(self):
        """(str) Returns the epilog for the program."""
        prog = self.c_prog
        _list = self._LIST_TAG
        return ('\n'
                'Extended Usage: ' + '\n'
                '1. To get a list of available chipsets: ' + '\n'
                '      ' + prog + ' -h --chipset ' + _list + '\n'
                '2. To get a list of available sign ids for a certain chipset: ' + '\n'
                '      ' + prog + ' -h --chipset <id> --sign_id ' + _list + '\n'
                '3. To get a list of available sign ids for a certain config file: ' + '\n'
                '      ' + prog + ' -h --config_path <path> --sign_id ' + _list + '\n')

    def c_add_options(self):
        """Adds the command line args supported by secimage."""
        # Signing individual image
        img_group = self.add_option_group('Signing individual image')
        img_group.add_option('-i', '--image_file', metavar='<file>',
                             help='path to the image file.')
        img_group.add_option('-g', '--sign_id', metavar='<id>',
                             help='sign id corresponding to the image_file provided.')
        img_group.add_option('-p', '--chipset', metavar='<id>',
                             help='id of the chipset corresponding to the image_file.')
        img_group.add_option('-c', '--config_path', metavar='<file>',
                             help='path to the secimage config file.')

        # Signing metabuild
        meta_group = self.add_option_group('Signing images from metabuild')
        meta_group.add_option('-m', '--meta_build', metavar='<dir>',
                             help='path to the meta-build to be used for obtaining the images to sign.')

        # Specifying the output location
        output_group = self.add_option_group('Specifying output location')
        output_group.add_option('-o', '--output_dir', metavar='<dir>',
                                help='directory to store output files. DEFAULT: "./' + DEF_SECIMAGE_OUTPUT_DIR_NAME + '"',
                                default=DEF_SECIMAGE_OUTPUT_DIR_PATH)
        output_group.add_option('-n', '--mini_build', metavar='<dir>',
                                help='path to the minimized build to store the signed images to. '
                                'This option works with the meta_build option.')

        # Specifying the operation
        operations_group = self.add_option_group('Operations')
        operations_group.add_option('-t', '--integrity_check', action='store_true',
                                    default=False, help='add hash table segment.')
        operations_group.add_option('-s', '--sign', action='store_true',
                                    default=False, help='sign the image.')
        operations_group.add_option('-e', '--encrypt', action='store_true',
                                    default=False, help='encrypt the image.')
        operations_group.add_option('--decrypt', action='store_true',
                                    default=False, help=optparse.SUPPRESS_HELP)
        operations_group.add_option('-a', '--validate', action='store_true',
                                    default=False, help='validate the image.')
        operations_group.add_option('-l', '--verify_inputs', action='store_true',
                                    default=False,
                                    help='verify the command line options.')

    @property
    def c_epilog_from_params(self):
        """(str) Based on the arguments provided, return the extended epilog
        message.
        """
        args = self.parsed_args
        help_obj = SecImageCore()
        epilog = ''

        if args.chipset == self._LIST_TAG:
            # Compile a list of available chipsets
            epilog += '\n' + 'Chipsets available: ' + '\n'
            for idx, chipset in enumerate(help_obj.available_chipsets):
                epilog += str(idx + 1) + '. ' + chipset + '\n'

        if (args.sign_id == self._LIST_TAG and
                args.chipset != self._LIST_TAG and
                args.config_path != self._LIST_TAG):
            if args.chipset:
                # Compile a list of available sign ids for a chipset
                help_obj.chipset = args.chipset
                epilog += '\n' + 'Sign-ids available for chipset "' + args.chipset + '": ' + '\n'
                for idx, sign_id in enumerate(help_obj.available_sign_ids):
                    epilog += str(idx + 1) + '. ' + sign_id + '\n'
            elif args.config_path:
                # Compile a list of available sign ids for a config file
                help_obj.config_path = args.config_path
                epilog += '\n' + 'Sign-ids available for config_path "' + args.config_path + '": ' + '\n'
                for idx, sign_id in enumerate(help_obj.available_sign_ids):
                    epilog += str(idx + 1) + '. ' + sign_id + '\n'

        return epilog

    def c_validate(self):
        """Validates the command line args provided by the user.

        :raises: RuntimeError if any error occurs.
        """
        args = self.parsed_args
        err = []

        # Check if the meta build supports sign id
        meta_supports_sign_id = False
        if args.meta_build:
            meta_supports_sign_id = SecImageCore.meta_supports_sign_id(args.meta_build)

        # Check the input files
        if ((args.image_file and args.meta_build) or
                (not args.image_file and not args.meta_build)):
            err.append('Provide either image_file or a meta_build for processing.')

        # Check the configuration option and output dir
        if args.image_file or (args.meta_build and not meta_supports_sign_id):
            if ((args.chipset and args.config_path) or
                    (not args.chipset and not args.config_path)):
                err.append('Provide either chipset or a config_path to process images.')
            if not args.output_dir:
                err.append('Provide the output_dir for storing the output.')
        elif args.meta_build and not meta_supports_sign_id:
            if not args.output_dir and not args.mini_build:
                err.append('Provide either output_dir or mini_build for storing the output.')

        # Check the operations
        if not (args.integrity_check or args.sign or args.encrypt or
                args.decrypt or args.validate or args.verify_inputs):
            err.append('Specify one or more operations to perform.')

        # Check and sanitize any paths for read access
        for path in ['image_file', 'config_path']:
            path_val = getattr(args, path, None)
            if path_val:
                path_val = c_path.normalize(path_val)
                if not c_path.validate_file(path_val):
                    err.append('Cannot access ' + path + ' at: ' + path_val)
                setattr(args, path, path_val)

        # Check and sanitize any paths for read dir access
        for path in ['meta_build']:
            path_val = getattr(args, path, None)
            if path_val:
                path_val = c_path.normalize(path_val)
                if not c_path.validate_dir(path_val):
                    err.append('Cannot access ' + path + ' at: ' + path_val)
                setattr(args, path, path_val)

        # Check and sanitize paths for write access
        for path in ['output_dir', 'mini_build']:
            path_val = getattr(args, path, None)
            if path_val:
                path_val = c_path.normalize(path_val)
                try:
                    c_path.create_dir(path_val)
                except Exception as e:
                    err.append('Cannot write at: ' + path_val + '\n'
                               '    ' + 'Error: ' + str(e))
                setattr(args, path, path_val)

        # Raise error if any
        if err:
            if len(err) > 1:
                err = [('  ' + str(idx + 1) + '. ' + error) for idx, error in enumerate(err)]
                err = 'Please check the command line args:\n\n' + '\n'.join(err)
            else:
                err = err[0]
            raise RuntimeError(err)


def log_to_file(folder):
    """Configures the logger to log to filesystem

    :param str folder: Directory to generate the logs in.
    """
    folder = c_path.normalize(folder)
    try:
        c_path.create_dir(folder)
    except Exception as e:
        raise RuntimeError('Unable to create directory for logging: ' + folder + '\n'
                           '    ' + 'Error: ' + str(e))
    logger.enable_file_logging(SECIMAGE_TOOL_NAME, num_logs=1, log_dir=folder)

def print_summary(args, image_info_list):
    """Prints the summary of the actions performed by SecImage"""
    actions = []

    # Check which actions were performed
    if args.integrity_check:
        actions.append('integrity_check')
    if args.sign:
        actions.append('sign')
    if args.encrypt:
        actions.append('encrypt')
    if args.validate:
        actions.append('validate')
    if not actions:
        return

    # Figure out the output directory
    output_print = (('Output is saved at: ' + args.output_dir + '\n') if args.output_dir else
                    ('Minimized build was updated at: ' + args.mini_build + '\n') if args.mini_build else
                    '\n')

    # Log the actions and output directory
    logger.info('SUMMARY:' + '\n' +
                'Following actions were performed: "' + ', '.join(actions) + '"' + '\n' +
                output_print)

    # Table information
    summary_table = TablePrinter([1])
    #summary_table = TablePrinter([0])
    COLUMN_IDX = 0
    COLUMN_SIGN_ID = 1
    COLUMN_PARSE = 2
    COLUMN_INTEGRITY_CHECK = 3
    COLUMN_SIGN = 4
    COLUMN_ENCRYPT = 5
    COLUMN_VAL_PARSE = 6
    COLUMN_VAL_INTEGRITY_CHECK = 7
    COLUMN_VAL_SIGN = 8
    COLUMN_VAL_ENCRYPT = 9

    # First row
    summary_table.insert_data(0, COLUMN_IDX, 'Idx')
    summary_table.insert_data(0, COLUMN_SIGN_ID, 'SignId')
    summary_table.insert_data(0, COLUMN_PARSE, 'Parse')
    summary_table.insert_data(0, COLUMN_INTEGRITY_CHECK, 'Integrity')
    summary_table.insert_data(0, COLUMN_SIGN, 'Sign')
    summary_table.insert_data(0, COLUMN_ENCRYPT, 'Encrypt')
    summary_table.insert_data(0, COLUMN_VAL_PARSE, 'Validate', column_end=COLUMN_VAL_ENCRYPT)

    # Second row
    summary_table.insert_data(1, COLUMN_VAL_PARSE, 'Parse')
    summary_table.insert_data(1, COLUMN_VAL_INTEGRITY_CHECK, 'Integrity')
    summary_table.insert_data(1, COLUMN_VAL_SIGN, 'Sign')
    summary_table.insert_data(1, COLUMN_VAL_ENCRYPT, 'Encrypt')

    # Data rows
    for idx, image in enumerate(image_info_list):
        idx += 2
        summary_table.insert_data(idx, COLUMN_IDX, str(idx - 1) + '.')
        summary_table.insert_data(idx, COLUMN_SIGN_ID, image.sign_id)
        summary_table.insert_data(idx, COLUMN_PARSE, image.status.parsegen.state)
        summary_table.insert_data(idx, COLUMN_INTEGRITY_CHECK, image.status.integrity_check.state)
        summary_table.insert_data(idx, COLUMN_SIGN, image.status.sign.state)
        summary_table.insert_data(idx, COLUMN_ENCRYPT, image.status.encrypt.state)
        summary_table.insert_data(idx, COLUMN_VAL_PARSE, image.status.validate_parsegen.state)
        summary_table.insert_data(idx, COLUMN_VAL_INTEGRITY_CHECK, image.status.validate_integrity_check.state)
        summary_table.insert_data(idx, COLUMN_VAL_SIGN, image.status.validate_sign.state)
        summary_table.insert_data(idx, COLUMN_VAL_ENCRYPT, image.status.validate_encrypt.state)

    # TODO: put the error and image paths

    logger.info(summary_table.get_data())

def main(args):
    """Parses the command line arguments, performs any basic operations based on
    the parsed arguments and starts processing using the isc module.
    """
    # Log to file
    log_to_file(args.output_dir)

    # Print the tool's launch command
    logger.debug('\n\n    SecImage launched as: "' + ' '.join(sys.argv) + '"\n')

    # Initialize SecImageCore
    isc = SecImageCore(debug=args.debug)

    # Configure image signer
    if args.image_file or (args.meta_build and not
                           SecImageCore.meta_supports_sign_id(args.meta_build)):
        if args.chipset:
            isc.chipset = args.chipset
        elif args.config_path:
            isc.config_path = args.config_path

    # Set the input
    if args.image_file:
        isc.set_image_path(args.image_file, args.sign_id)
    elif args.meta_build:
        isc.set_meta_build_path(args.meta_build, [] if args.sign_id is None else [args.sign_id])

    # Set the output
    if args.mini_build:
        isc.mini_build_path = args.mini_build
    elif args.output_dir:
        isc.output_dir = args.output_dir

    # Process the images
    isc.process(verify_setup=args.verify_inputs,
                integrity_check=args.integrity_check,
                sign=args.sign,
                encrypt=args.encrypt,
                decrypt=args.decrypt,
                val_image=args.validate,
                val_integrity_check=args.validate,
                val_sign=args.validate,
                val_encrypt=args.validate)

    # Print the summary
    print_summary(args, isc.image_info_list)

def parse_args(argv):
    return SecImageParser(argv).parsed_args

if __name__ == '__main__':
    try:
        # Call the main of the tool
        main(parse_args(sys.argv))

    except Exception:
        logger.debug(traceback.format_exc())
        logger.error(sys.exc_info()[1])
        sys.exit(1)

    except KeyboardInterrupt:
        print
        logger.error('Keyboard Interrupt Received. Exiting!')
        sys.exit(1)

    sys.exit(0)

