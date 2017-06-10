#===============================================================================
#
# gensecimage_builder.py
#
# GENERAL DESCRIPTION
#    Contains build process utilities for calling gensecimage
#
# Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
# Qualcomm Technologies Proprietary and Confidential.
#
#-------------------------------------------------------------------------------
#
#  $Header:
#  $DateTime: 2013/12/07 08:21:17 $
#  $Author: yliong $
#                      EDIT HISTORY FOR FILE
#
#  This section contains comments describing changes made to the module.
#  Notice that changes are listed in reverse chronological order.
#
# when       who     what, where, why
# --------   ---     ---------------------------------------------------------
#
#===============================================================================
import os
import glob
import shutil
from optparse import OptionParser
import sys

CURRENT_VERSION = '1.0.1'

GENSECIMAGE_DIR = os.path.dirname(os.path.realpath(__file__))

sys.path.append(GENSECIMAGE_DIR)

#------------------------------------------------------------------------------
# Hooks for Scons
#------------------------------------------------------------------------------
def exists(env):
   return env.Detect('gensecimage_mbn_builder')

def generate(env):
   import SCons
   gensecimage_mbn_generate(env)

   gensecimage_cfg_generate(env)

#===============================================================================
# PBN build rules
#===============================================================================
def gensecimage_mbn_generate(env):
   #-------------------------------------------------------------------------------
   # def builder for PBN
   #
   #import pdb;pdb.set_trace()
   env.AddMethod(gensecimage_builder_wrapper, "GensecimageMbnBuilder")
   env.AddMethod(gensecimage_builder_install, "GensecimageInstall")
   env.AddMethod(get_msm_jtag_dict_from_file, "GetMSMToJTAGDict")

   gensecimage_mbn_act = env.GetBuilderAction(gensecimage_mbn_builder)
   gensecimage_mbn_bld = env.Builder(
						 action = gensecimage_mbn_act,
					     emitter = gensecimage_mbn_emitter,
                         suffix = '.mbn')

   env.Append(BUILDERS = {'GensecimageMbnInternalBuilder' : gensecimage_mbn_bld})

   os.environ['GENSECIMAGE_LOCK_ENABLE'] = 'false'

def gensecimage_cfg_generate(env):
   #-------------------------------------------------------------------------------
   # def builder for PBN
   #
   #import pdb;pdb.set_trace()
   env.AddMethod(gensecimage_cfg_builder_wrapper, "GensecimageCfgBuilder")

   gensecimage_cfg_act = env.GetBuilderAction(gensecimage_cfg_builder)
   gensecimage_cfg_bld = env.Builder(
						 action = gensecimage_cfg_act,
					     emitter = gensecimage_cfg_emitter,
                         suffix = '.cfg')

   env.Append(BUILDERS = {'GensecimageCfgInternalBuilder' : gensecimage_cfg_bld})

#===============================================================================
# Helper functions to work with or without scons
#===============================================================================

def str2bool(v):
  return v.lower() in ("yes", "true", "t", "1")

def isKeyEnable(env, key_list):
  ret = False

  if key_list is None:
      return False

  if env is not None:
      return env.IsKeyEnable(key_list)

  if type(key_list) is not list:
      key_list = [key_list]

  for key in key_list:
      key_enable = str2bool(os.environ.get(key, "false"))
      ret = ret | key_enable
      print "%s:%s" % (key, key_enable)

  return ret

def printdebuginfo(env, mesg):
  if env is not None:
      env.PrintDebugInfo('gensecimage', mesg)
  else:
      print mesg

def printinfo(env, mesg):
  if env is not None:
      env.PrintInfo(mesg)
  else:
      print mesg

def getenv(env, key, default):
  ret = None
  if env is not None:
      ret = env.get(key, default)
  else:
      ret = os.environ.get(key, default)
  return ret

def depends(env, target, dependency):
  if env is not None:
      env.Depends(target, dependency)

def envsubst(env, string):
  if env is not None:
      return env.subst(string)
  else:
      return string

def execcmds(env, cmds, target):
  if env is not None:
      data, err, rv = env.ExecCmds(cmds, target=target)
  else:
      rv = os.system(cmds)

  return rv

def get_msm_jtag_dict_from_file(env, msmid,
    msm_jtag_mapping_file='${BUILD_ROOT}/build/msm_jtag_mapping.xml',
    file_type="xml"):
  msm_jtag_mapping_file = os.path.realpath(envsubst(env, msm_jtag_mapping_file))
  printinfo(env, "gensecimage_builder_wrapper: msm_jtag_mapping_file= %s" % msm_jtag_mapping_file)
  try:
      msmid_jtagid_dict = get_msm_jtag_dict_from_file_impl(msmid, msm_jtag_mapping_file, file_type)
  except Exception, e:
      returnError(env,str(e))

  return msmid_jtagid_dict

def get_msm_jtag_dict_from_file_impl(msmid, msm_jtag_mapping_file, file_type="xml"):
  msmid_jtagid_dict = {}
  if not os.path.isfile(msm_jtag_mapping_file):
      raise RuntimeError, "gensecimage_builder_wrapper: msm to jtag mapping file %s does not exist!"  % msm_jtag_mapping_file

  if file_type.lower() == "txt":
      file = open(msm_jtag_mapping_file)
      for line in file:
         (key, val) = line.split()
         msmid_jtagid_dict[key] = val
      file.close()
  elif file_type.lower() == "xml":
      try:
          import msmjtagid_parse
          msmid_jtagid_dict = msmjtagid_parse.get_jtag_dict_from_msmid(msmid, msm_jtag_mapping_file)
      except Exception, e:
          err_str = "msmid %s cannot be mapped into a valid JTAG ID. Please update %s.\n" % (msmid, msm_jtag_mapping_file)
          err_str = err_str + str(e)
          raise RuntimeError, err_str
  else:
      raise RuntimeError, "gensecimage_builder_wrapper: file_type = %s is not supported for msm_jtag_mapping_file" % file_type

  return msmid_jtagid_dict

#msmid_jtagid_dict is added in 1.0.1
def gensecimage_builder_wrapper(env,
                    target_base_dir,
                    source,
                    enable_buildtags='USES_GENSECIMAGE',
                    disable_buildtags=['USES_SECBOOT', 'USES_ENCRYPT_MBN'],
                    basic_cfg_template='gensecimage.cfg.template',
                    basic_cfg='gensecimage.cfg',
                    signing_cfg_template='signingattr_qpsa.cfg.template',
                    signing_cfg='signingattr_qpsa.cfg',
                    postsign_cfg_from_builder=None,
                    gensecimage_section=None, generate_signcfg=True, generate_cfg=True,
                    gensecimage_msmid=None, gensecimage_jtagid=None,
                    gensecimage_enable_encrypt=False,
                    msm_jtag_mapping_file='${BUILD_ROOT}/build/msm_jtag_mapping.txt',
                    msmid_jtagid_dict=None):
   rt = None

   source = envsubst(env, source)
   if type(source) is not list:
       source = [source]
   source_str = os.path.realpath(str(source[0]))

   # don't do anything if none of the tags match.
   if (enable_buildtags is None or isKeyEnable(env, enable_buildtags) is True) and \
      (disable_buildtags is None or isKeyEnable(env, disable_buildtags) is False):

       selected_id = None
       isMSMToJtagMapNeeded = False
       MSM_TO_JTAG_DICT = {}
       msm_jtag_mapping_file = os.path.realpath(envsubst(env, msm_jtag_mapping_file))
       if generate_signcfg is True:
           if msmid_jtagid_dict is None and gensecimage_msmid is None and gensecimage_jtagid is None:
              returnError(env, "msmid_jtagid_dict, gensecimage_msmid and gensecimage_jtagid cannot be all NULL.")
           elif msmid_jtagid_dict is not None:
              selected_id = msmid_jtagid_dict.keys()
           elif gensecimage_jtagid is not None:
              selected_id = gensecimage_jtagid
           else:
              selected_id = gensecimage_msmid
              isMSMToJtagMapNeeded = True
              MSM_TO_JTAG_DICT = get_msm_jtag_dict_from_file(env=env,
                                    msmid=None,
                                    msm_jtag_mapping_file=msm_jtag_mapping_file,
                                    file_type="txt")

       if type(selected_id) is not list:
          selected_id = [selected_id]

       num_targets = len(selected_id)

       rt_list = []
       target_dir_realpath = os.path.realpath(envsubst(env, target_base_dir))
       source_path, source_file = os.path.split(source_str)
       postsign_cfg_full = None

       for i in range(0, num_targets):
           target_path = os.path.join(target_dir_realpath, selected_id[i])
           target_str = os.path.join(target_path, source_file)
           basic_cfg_full = os.path.join(target_path, basic_cfg)
           signing_cfg_full = os.path.join(target_path, signing_cfg)
           if postsign_cfg_from_builder is not None and i<len(postsign_cfg_from_builder):
               postsign_cfg_full = os.path.realpath(str(postsign_cfg_from_builder[i][0]))

           printdebuginfo(env, "gensecimage_builder_wrapper:target_str=%s" % target_str)

           #config_list is used for scons environment only
           config_list = []
           if generate_cfg is True:
               if (gensecimage_enable_encrypt is True):
                   enable_encrypt = "yes"
               else:
                   enable_encrypt = "no"

               rtt = gensecimage_cfg_builder_dispatcher(env, target=basic_cfg_full,
                    source=os.path.realpath(envsubst(env, basic_cfg_template)),
                    GENSECIMAGE_SECTION=gensecimage_section,
                    GENSECIMAGE_CFG_TAGS_TO_REPLACE=[gensecimage_section.upper() + "_SRCFILE", gensecimage_section.upper() + "_OUTPUTDIR", "ENABLE_ENCRYPT"],
                    GENSECIMAGE_CFG_REPLACE_VALUES=[source_str, target_path, enable_encrypt])
               config_list.append(rtt)
           else:
               config_list.append(basic_cfg_full)

           if generate_signcfg is True:
               gensecimage_dir = getenv(env, 'GENSECIMAGE_DIR', GENSECIMAGE_DIR)
               printinfo(env, "gensecimage_builder_wrapper: GENSECIMAGE_DIR = %s" % gensecimage_dir)

               if msmid_jtagid_dict is not None:
                   jtag_id = msmid_jtagid_dict[selected_id[i]]
               else:
                   jtag_id = getJTAGIDFromBuiltDict(
                                MSM_TO_JTAG_DICT,
                                selected_id[i],
                                isMSMToJtagMapNeeded,
                                msm_jtag_mapping_file)

               rtt = gensecimage_cfg_builder_dispatcher(env, target=signing_cfg_full,
                    source=os.path.realpath(envsubst(env, signing_cfg_template)),
                    GENSECIMAGE_SECTION=gensecimage_section,
                    GENSECIMAGE_CFG_TAGS_TO_REPLACE=["JTAGID"],
                    GENSECIMAGE_CFG_REPLACE_VALUES=[jtag_id])
               config_list.append(rtt)
           else:
               config_list.append(signing_cfg_full)

           if postsign_cfg_full is not None:
               config_list.append(postsign_cfg_full)

           rtt = gensecimage_mbn_builder_dispatcher(env, target_str, source, GENSECIMAGE_SECTION=gensecimage_section,
                BASIC_CFG=basic_cfg_full,
                SIGNING_CFG=signing_cfg_full,
                POSTSIGN_CFG=postsign_cfg_full,
                CONFIG_LIST=config_list)
           rt_list.append(rtt)
           config_list = []

       return rt_list

   else:
       printinfo(env, "gensecimage is not run for %s due to build tags constraint." % source_str)
       printinfo(env, "enable_buildtags = %s" % enable_buildtags)
       printinfo(env, "disable_buildtags = %s" % disable_buildtags)

   return rt

def gensecimage_builder_install(env, install_base_dir, install_sub_dir, install_file_name, gensecimage_builder_output=None,
                                alternate_source = None):
   rt_list = []

   if gensecimage_builder_output is not None:
       if type(install_sub_dir) is not list:
           install_sub_dir = [install_sub_dir]

       for i in range(0, len(install_sub_dir)):
           target_file_base, target_file_ext = os.path.splitext(install_file_name)
           target_path = os.path.join(install_base_dir, install_sub_dir[i])
           target_file_path = os.path.join(target_path, install_file_name)
           if alternate_source is not None:
               install_signed_mbn = env.InstallAs(target_file_path, alternate_source)
           else:
               install_signed_mbn = env.InstallAs(target_file_path, gensecimage_builder_output[i][0])
           #import pdb;pdb.set_trace()
           install_signed_cert = env.InstallAs(os.path.join(target_path, 'cert', target_file_base + '.zip'), gensecimage_builder_output[i][1])
           rt_list.append([install_signed_mbn, install_signed_cert])

   return rt_list

#-------------------------------------------------------------------------------
# Emitter builder to add clean actions
#
def gensecimage_mbn_emitter(target, source, env):
   # here one can also add dependencies for say aexe.exe (if it needs to be build)
   # env.Depends(target, propgen_tool)
   target_str = os.path.realpath(str(target[0]))

   target_path, target_file = os.path.split(target_str)
   signed_file = target_str
   signed_file_base, ext = os.path.splitext(signed_file)

   files_in_dir = glob.glob(target_path)
   for file in files_in_dir:
       if (file != signed_file):
           #SideEffect doesn't seem to clean the file here
           #env.SideEffect(file, target)
           env.Clean(target, file)

   #import pdb;pdb.set_trace()
   # Add this side effect to prevent concurrency access
   log_dir = env.subst('${BUILD_ROOT}/build/ms/')
   env.SideEffect(os.path.join(log_dir, 'gensecimage.log'), target)
   env.PrintDebugInfo('gensecimage', "gensecimage sideeffect:%s" % os.path.join(log_dir, 'gensecimage.log'))

   target.append(signed_file_base + '.zip')

   return (target, source)

def gensecimage_mbn_builder_dispatcher(env, target, source,
                GENSECIMAGE_SECTION,
                BASIC_CFG,
                SIGNING_CFG,
                POSTSIGN_CFG,
                CONFIG_LIST):

   #CONFIG_LIST is only used in scons to set the source, in order to get the dependency correct
   if env is not None:
       return env.GensecimageMbnInternalBuilder(target, [source, CONFIG_LIST], GENSECIMAGE_SECTION=GENSECIMAGE_SECTION,
                BASIC_CFG=BASIC_CFG,
                SIGNING_CFG=SIGNING_CFG,
                POSTSIGN_CFG=POSTSIGN_CFG)
   else:
       return gensecimage_mbn_internal_builder(env, target, str(source[0]), GENSECIMAGE_SECTION=GENSECIMAGE_SECTION,
                BASIC_CFG=BASIC_CFG,
                SIGNING_CFG=SIGNING_CFG,
                POSTSIGN_CFG=POSTSIGN_CFG)

#-------------------------------------------------------------------------------
# def builder for qcoreapp_booting.pbn
#
def gensecimage_mbn_builder(target, source, env):
   return gensecimage_mbn_internal_builder(env, str(target[0]), str(source[0]),
                GENSECIMAGE_SECTION=env.get('GENSECIMAGE_SECTION', None),
                BASIC_CFG=env.get('BASIC_CFG', None),
                SIGNING_CFG=env.get('SIGNING_CFG', None),
                POSTSIGN_CFG=env.get('POSTSIGN_CFG', None))

def gensecimage_mbn_internal_builder(env, target, source,
                GENSECIMAGE_SECTION,
                BASIC_CFG,
                SIGNING_CFG,
                POSTSIGN_CFG):
   # init and fix variebles
   source_str = os.path.realpath(source)
   target_str = os.path.realpath(target)
   target_path, target_file = os.path.split(target_str)

   gensecimage_dir = getenv(env, 'GENSECIMAGE_DIR', GENSECIMAGE_DIR)
   printinfo(env, "gensecimage_mbn_builder: GENSECIMAGE_DIR = %s" % gensecimage_dir)

   # Get parameters from environment
   gensecimage_section = GENSECIMAGE_SECTION
   if gensecimage_section is None:
       raise RuntimeError, "gensecimage_mbn_builder: GENSECIMAGE_SECTION cannot be None"

   gensecimage_cfg = BASIC_CFG
   gensecimage_signcfg = SIGNING_CFG
   gensecimage_postsigncfg = POSTSIGN_CFG

   printinfo(env, "gensecimage_mbn_builder: gensecimage config = %s" % gensecimage_cfg)
   printinfo(env, "gensecimage_mbn_builder: signing config = %s" % gensecimage_signcfg)
   printinfo(env, "gensecimage_mbn_builder: postsign config = %s" % gensecimage_postsigncfg)

   # Issuing gensecimage command
   cmds = envsubst(env, "python " + os.path.join(gensecimage_dir, "gensecimage.py") + " --stage=qpsa:sign " + \
                    "--section=" + gensecimage_section.lower() + " " + \
                    "--config=" + gensecimage_cfg)

   if gensecimage_signcfg is not None:
       cmds = cmds + " --signconfig=" + gensecimage_signcfg

   if gensecimage_postsigncfg is not None:
       cmds = cmds + " --postsignconfig=" + gensecimage_postsigncfg

   printinfo(env, "gensecimage_mbn_builder: %s" % cmds)
   rv = execcmds(env, cmds, target=target_str)

   # Backup gensecimage log file
   if os.path.exists('gensecimage.log'):
       shutil.move('gensecimage.log',  target_path)

   # Remove qpsa log file as it is copied already
   if os.path.exists('qpsa.log'):
       os.remove('qpsa.log')

   return None


def gensecimage_cfg_builder_wrapper(env,
                    target_base_dir,
                    target_filename,
                    source,
                    tags_to_replace,
                    replace_values,
                    gensecimage_section,
                    enable_buildtags='USES_GENSECIMAGE',
                    disable_buildtags=['USES_SECBOOT', 'USES_ENCRYPT_MBN'],
                    target_sub_dir=None,
                    ):
   rt = None

   # don't do anything if none of the tags match.
   if (enable_buildtags is None or isKeyEnable(env, enable_buildtags) is True) and \
      (disable_buildtags is None or isKeyEnable(env, disable_buildtags) is False):
       #import pdb;pdb.set_trace()
       if type(tags_to_replace) is not list:
          tags_to_replace = [tags_to_replace]
       if type(replace_values) is not list:
          replace_values = [replace_values]
       if type(target_sub_dir) is not list:
          target_sub_dir = [target_sub_dir]

       num_targets = len(target_sub_dir)

       rt_list = []
       target_dir_realpath = os.path.realpath(target_base_dir)
       for i in range(0, num_targets):
           target_path = os.path.join(target_dir_realpath, target_sub_dir[i])
           target_str = os.path.join(target_path, target_filename)

           printdebuginfo(env, 'gensecimage', "gensecimage_cfg_builder_wrapper:target_str=%s" % target_str)

           #import pdb;pdb.set_trace()
           rt1 = env.GensecimageCfgInternalBuilder(target=target_str,
                source=os.path.realpath(source),
                GENSECIMAGE_SECTION=gensecimage_section,
                GENSECIMAGE_CFG_TAGS_TO_REPLACE=tags_to_replace,
                GENSECIMAGE_CFG_REPLACE_VALUES=replace_values)
           rt_list.append(rt1)

       return rt_list

   return rt


#-------------------------------------------------------------------------------
# Emitter builder to add clean actions
#
def gensecimage_cfg_emitter(target, source, env):
   # here one can also add dependencies for say aexe.exe (if it needs to be build)


   return (target, source)


def gensecimage_cfg_builder_dispatcher(env, target, source,
                    GENSECIMAGE_SECTION,
                    GENSECIMAGE_CFG_TAGS_TO_REPLACE,
                    GENSECIMAGE_CFG_REPLACE_VALUES):

   if env is not None:
       # scons hook
       return env.GensecimageCfgInternalBuilder(target=target,
        source=source,
        GENSECIMAGE_SECTION=GENSECIMAGE_SECTION,
        GENSECIMAGE_CFG_TAGS_TO_REPLACE=GENSECIMAGE_CFG_TAGS_TO_REPLACE,
        GENSECIMAGE_CFG_REPLACE_VALUES=GENSECIMAGE_CFG_REPLACE_VALUES)
   else:
       return gensecimage_cfg_internal_builder(env, target=target,
        source=source,
        GENSECIMAGE_SECTION=GENSECIMAGE_SECTION,
        GENSECIMAGE_CFG_TAGS_TO_REPLACE=GENSECIMAGE_CFG_TAGS_TO_REPLACE,
        GENSECIMAGE_CFG_REPLACE_VALUES=GENSECIMAGE_CFG_REPLACE_VALUES)


#-------------------------------------------------------------------------------
# def builder to generate gensecimage.cfg from template
#
def gensecimage_cfg_builder(target, source, env):

   gensecimage_cfg_internal_builder(env, target=str(target[0]),
        source=str(source[0]),
        GENSECIMAGE_SECTION=env.get('GENSECIMAGE_SECTION', None),
        GENSECIMAGE_CFG_TAGS_TO_REPLACE=env.get('GENSECIMAGE_CFG_TAGS_TO_REPLACE', None),
        GENSECIMAGE_CFG_REPLACE_VALUES=env.get('GENSECIMAGE_CFG_REPLACE_VALUES', None))


def gensecimage_cfg_internal_builder(env, target, source,
                    GENSECIMAGE_SECTION,
                    GENSECIMAGE_CFG_TAGS_TO_REPLACE,
                    GENSECIMAGE_CFG_REPLACE_VALUES):

   # init and fix variebles
   source_str = os.path.realpath(source)
   target_str = os.path.realpath(target)

   # Get parameters from environment
   gensecimage_section = GENSECIMAGE_SECTION
   if gensecimage_section is None:
       raise RuntimeError, "gensecimage_cfg_builder: GENSECIMAGE_SECTION cannot be None"

   tag_list = GENSECIMAGE_CFG_TAGS_TO_REPLACE
   if tag_list is None:
       raise RuntimeError, "gensecimage_cfg_builder: GENSECIMAGE_CFG_TAGS_TO_REPLACE cannot be None"
   if type(tag_list) is not list:
       tag_list = [tag_list]

   value_list = GENSECIMAGE_CFG_REPLACE_VALUES
   if value_list is None:
       raise RuntimeError, "gensecimage_cfg_builder: GENSECIMAGE_CFG_REPLACE_VALUES cannot be None"
   if type(value_list) is not list:
       value_list = [value_list]

   # Generate gensecimage config
   if not os.path.isfile(source_str):
       returnError(env, "gensecimage_cfg_builder: source=%s does not exist!"  % source_str)

   generate_config(source_str, target_str, \
                gensecimage_section,
                tag_list,
                value_list)

   return None

# Use returnError() when the error is expected.
# Use raise RuntimeError when the error is unexpected and call stack is needed.
def returnError(env, errorMsg):
   if env is not None:
       import SCons
       env.PrintError("")
       env.PrintError("-------------------------------------------------------------------------------")
       env.PrintError(errorMsg)
       env.PrintError("-------------------------------------------------------------------------------")
       env.PrintError("")
       SCons.Script.Exit(1)
   else:
       print ""
       print "-------------------------------------------------------------------------------"
       print "ERROR: " + errorMsg
       print "-------------------------------------------------------------------------------"
       print ""
       exit(1)

def getJTAGIDFromBuiltDict(MSM_TO_JTAG_DICT, selected_id, isMsmToJtagMapNeeded, msm_jtag_mapping_file):
   # Initialize jtag_id
   if isMsmToJtagMapNeeded is False:
       jtag_id = selected_id
   elif selected_id in MSM_TO_JTAG_DICT:
       jtag_id = MSM_TO_JTAG_DICT[selected_id]
   else:
       errormsg = "gensecimage_builder getJTAGIDFromBuiltDict: msmid %s cannot be mapped into a valid JTAG ID. Please update %s." % (selected_id, msm_jtag_mapping_file)
       returnError(env, errormsg)

   return jtag_id

def generate_config(src_config_file, dest_config_file, gensecimage_section, tag_list, value_list):
   if len(tag_list) != len(value_list):
       returnError(env, "gensecimage generate_config: length of tag_list (%d) is different from value_list(%d)" \
                % (len(tag_list), len(value_list)))

   path, file = os.path.split(dest_config_file)
   if not os.path.exists(path):
       os.makedirs(path)
   new_file = open(dest_config_file,'w')
   old_file = open(src_config_file, 'r')
   for line in old_file:
       for i in range(0, len(tag_list)):
           line = line.replace("%" + tag_list[i] + "%", value_list[i])
       new_file.write(line)
   new_file.close()
   old_file.close()

#------------------------------------------------------------------------------
# Parse command line arguments
#------------------------------------------------------------------------------
def parse_args():
   usage = "usage: %prog [options]"
   version = "%prog " + CURRENT_VERSION

   parser = OptionParser(usage=usage, version=version)
   parser.add_option("-t", "--target_base_dir", dest="target_base_dir",
                  help="Target base directory to store signed image outputs")
   parser.add_option("-s", "--source", dest="source",
                  help="Source file to sign", metavar="FILE")
   parser.add_option("--section", dest="gensecimage_section",
                  help="Section name in gensecimage config to execute")
   parser.add_option("--msmid", dest="gensecimage_msmid", action="append",
                  help="Msmid to sign images. Msmid maps to JTAG ID defined in msm_jtag_mapping_file. Support multiple msmids")
   parser.add_option("--jtagid", dest="gensecimage_jtagid", action="append",
                  help="Jtag ID to sign images. Support multiple jtag IDs")
   parser.add_option("--basic_cfg_template", dest="basic_cfg_template",
                  help="Path for gensecimage basic configuration template", metavar="FILE")
   parser.add_option("--signing_cfg_template", dest="signing_cfg_template",
                  help="Path for signing configuration template", metavar="FILE")
   parser.add_option("--msm_jtag_mapping_file", dest="msm_jtag_mapping_file",
                  help="Path for MSM to JTAG mapping file", metavar="FILE")
   parser.add_option("--enable_buildtag", action="append",
                  help="Build tag to enable gensecimage. Support multiple buildtag.")
   parser.add_option("--disable_buildtag", action="append",
                  help="Build tag to disable gensecimage. Support multiple buildtag.")

   (options, args) = parser.parse_args()
   #import pdb;pdb.set_trace()
   if options.target_base_dir is None:
       print "Type gensecimage_builder.py -h for help."
       parser.error("target_base_dir is not specified.")
   if options.gensecimage_msmid is None and options.gensecimage_jtagid is None:
       parser.error("msmid or jtag id is not specified")
   if options.source is None:
       parser.error("source is not specified")
   if options.gensecimage_section is None:
       parser.error("section is not specified")
   if options.msm_jtag_mapping_file is None:
       parser.error("msm_jtag_mapping_file is not specified")
   if options.basic_cfg_template is None:
       parser.error("basic_cfg_template is not specified")
   if options.signing_cfg_template is None:
       parser.error("signing_cfg_template is not specified")

   return (options, args)

#------------------------------------------------------------------------------
# main
#------------------------------------------------------------------------------
if __name__ == "__main__":
   # get args from cmd line
   (options, args) = parse_args()

   env = None

   gensecimage_builder_wrapper(
                    env,
                    target_base_dir = options.target_base_dir,
                    source=options.source,
                    gensecimage_section = options.gensecimage_section,
                    gensecimage_msmid = options.gensecimage_msmid,
                    gensecimage_jtagid = options.gensecimage_jtagid,
                    basic_cfg_template=options.basic_cfg_template,
                    signing_cfg_template=options.signing_cfg_template,
                    msm_jtag_mapping_file=options.msm_jtag_mapping_file,
                    enable_buildtags=options.enable_buildtag,
                    disable_buildtags=options.disable_buildtag,
                    )
