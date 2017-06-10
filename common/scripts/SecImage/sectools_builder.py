#===============================================================================
#
# sectools_builder.py
#
# GENERAL DESCRIPTION
#    Contains build process utilities for calling sectools
#
# Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
# Qualcomm Technologies Proprietary and Confidential.
#
#===============================================================================
import os
from optparse import OptionParser
import sys

# Name & version of the tool
SECTOOLS_BUILDER_TOOL_NAME = 'Sectools_Builder'
SECTOOLS_BUILDER_TOOL_VERSION = '2.0'

__version__ = SECTOOLS_BUILDER_TOOL_NAME + ' ' + SECTOOLS_BUILDER_TOOL_VERSION

SECTOOLS_DIR = os.path.dirname(os.path.realpath(__file__))

sys.path.append(SECTOOLS_DIR)

import sectools.features.isc.builder.scons_secimage_builder as scons_secimage_builder
import sectools.features.isc.builder.secimage_builder as secimage_builder

#------------------------------------------------------------------------------
# Hooks for Scons
#------------------------------------------------------------------------------
def exists(env):
    return env.Detect('sectools_builder')

def generate(env):
    import SCons
    sectools_mbn_generate(env)


#===============================================================================
# PBN build rules
#===============================================================================
def sectools_mbn_generate(env):
    #-------------------------------------------------------------------------------
    # def builder for PBN
    #
    #import pdb;pdb.set_trace()
    env.AddMethod(secimage_builder.build, "SectoolBuilder")
    env.AddMethod(secimage_builder.install, "SectoolInstall")
    env.AddMethod(secimage_builder.get_msm_jtag_dict_from_file, "SectoolGetMSMToJTAGDict")
    env.AddMethod(secimage_builder.isEnabled, "SectoolIsEnabled")
    env.AddMethod(secimage_builder.getUnsignedInstallPath, "SectoolGetUnsignedInstallPath")

    sectools_signer_act = env.GetBuilderAction(scons_secimage_builder.action)
    sectools_signer_bld = env.Builder(
                          action = sectools_signer_act,
                          emitter = scons_secimage_builder.emitter,
                          suffix = '.mbn')

    env.Append(BUILDERS = {scons_secimage_builder.FUNC_HANDLER_NAME : sectools_signer_bld})


#------------------------------------------------------------------------------
# Parse command line arguments
#------------------------------------------------------------------------------
def parse_args():
    usage = "usage: %prog [options]"
    version = "%prog " + __version__

    parser = OptionParser(usage=usage, version=version)
    parser.add_option("-t", "--target_base_dir", dest="target_base_dir",
                      help="Target base directory to store signed image outputs")
    parser.add_option("-i", "--image_file", dest="source",
                      help="Source file to sign", metavar="<file>")
    parser.add_option("-g", "--sign_id", dest="sign_id", metavar="<id>",
                      help="sign_id to execute")
    parser.add_option("--install_base_dir", dest="sectools_install_base_dir",
                      default=None, metavar="<dir>",
                      help="Base directory for installation. No installation will be performed if not specified.")
    parser.add_option("--install_file_name", metavar="<file>",
                      default=None,
                      help="File name for installation. Original filename will be used if not specified.")
    parser.add_option("--msmid", dest="msmid",
                      default=None,
                      help="Msmid to sign images. Msmid maps to JTAG ID defined in msm_jtag_mapping_file. 'Default' msmid mapped to 0x00000000 will be used if both msmid and msm_jtagid_dict are not specified.")
    parser.add_option("--msm_jtagid_dict", dest="msmid_jtagid_dict",
                      default = None,
                      help="MSM to JTAGID mapping dictionary. Default to {'default':'0x00000000'} if both msmid and msm_jtagid_dict are not specified.")
    parser.add_option("--msm_jtag_mapping_file", dest="msm_jtag_mapping_file",
                      default = secimage_builder.DEFAULT_MSM_JTAG_MAPPING_FILE,
                      help="Path for MSM to JTAG mapping file", metavar="<file>")
    parser.add_option("--disable_featureflags", dest="disable_featureflags", action="append",
                      default = secimage_builder.DEFAULT_DISABLE_FEATURE_FLAGS,
                      help="Feature flags to disable signer. Support multiple feature flags.")
    parser.add_option('-c', '--config', dest="config",
                      default = secimage_builder.DEFAULT_CONFIG_FILE,
                      help="path to the secimage config file.", metavar="<file>")
    parser.add_option('--policy_file', dest="policy_file",
                      default = secimage_builder.DEFAULT_POLICY_FILE,
                      help='path to the sectools policy file.', metavar="<file>")


    (options, args) = parser.parse_args()
    #import pdb;pdb.set_trace()
    if options.target_base_dir is None:
        print "Type sectools_builder.py -h for help."
        parser.error("target_base_dir is not specified.")
    if options.source is None:
        parser.error("source is not specified")
    if options.sign_id is None:
        parser.error("sign_id is not specified")


    return (options, args)

#------------------------------------------------------------------------------
# main
#------------------------------------------------------------------------------
if __name__ == "__main__":
    # get args from cmd line
    (options, args) = parse_args()

    env = None

    secimage_builder.build(
                     env,
                     target_base_dir = options.target_base_dir,
                     source=options.source,
                     sign_id=options.sign_id,
                     sectools_install_base_dir=options.sectools_install_base_dir,
                     install_file_name=options.install_file_name,
                     msmid=options.msmid,
                     msmid_jtagid_dict=options.msmid_jtagid_dict,
                     msm_jtag_mapping_file=options.msm_jtag_mapping_file,
                     disable_featureflags=options.disable_featureflags,
                     config=options.config,
                     policy_file=options.policy_file
                     )
