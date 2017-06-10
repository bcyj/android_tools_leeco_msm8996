#===============================================================================
#
# Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
# Qualcomm Technologies Proprietary and Confidential.
#
#===============================================================================

from generic_secimage_builder import GenericSecImageBuilder
import scons_secimage_builder as scons_secimage_builder
from sectools.common.builder.builderutils import BuilderUtil
from sectools.common.builder.buildpolicy import BuildPolicy
import sectools.features.isc.signer.signerutils as signerutils
import os
import msmjtagid_parse as msm_jtag_parser
from sectools.common.utils import c_path

BUILDER_DIR = os.path.dirname(os.path.realpath(__file__))
SECTOOLS_DIR = os.path.join(BUILDER_DIR, "..", "..", "..", "..")

DEFAULT_MSM_JTAG_DICT = {'default':'0x00000000'}
DEFAULT_DISABLE_FEATURE_FLAGS = ['USES_SECBOOT', 'USES_ENCRYPT_MBN', 'USES_GENSECIMAGE']
DEFAULT_POLICY_FILE = os.path.join(SECTOOLS_DIR, 'config', 'integration', 'sectools_policy.xml')
DEFAULT_CONFIG_FILE = os.path.join(SECTOOLS_DIR, 'config', 'integration', 'secimage.xml')
DEFAULT_MSM_JTAG_MAPPING_FILE = os.path.join('${BUILD_ROOT}', 'build', 'msm_jtag_mapping.xml')
DEFAULT_INSTALL_DEFAULT_FLAGS = ['USES_SEC_POLICY_INSTALL_DEFAULT_SIGN', 'USES_SEC_POLICY_INSTALL_DEFAULT_SIGN_ENCRYPT']
DEFAULT_PILSPLITTER_PATH = "${BUILD_ROOT}/core/bsp/build/scripts/pil_splitter_builder.py"

POSTPROCESS_INSTALL = "install"
POSTPROCESS_PILSPLIT = "pilsplit"


def build(env,
        target_base_dir,
        source,
        sign_id,
        jtagid=None,
        sectools_install_base_dir = None,
        install_file_name = None,
        msmid = None,
        msmid_jtagid_dict = None,
        msm_jtag_mapping_file=DEFAULT_MSM_JTAG_MAPPING_FILE,
        disable_featureflags=DEFAULT_DISABLE_FEATURE_FLAGS,
        config=DEFAULT_CONFIG_FILE,
        policy_file=DEFAULT_POLICY_FILE,
        pilsplitter_target_base_dir = None,
        pilsplitter_path = DEFAULT_PILSPLITTER_PATH,
        pilsplitter_featureflag = [],
        image_entry = None,
        relocatable = False):
    rt = []

    util = BuilderUtil(env)

    source = util.envsubst(source)
    if type(source) is not list:
        source = [source]
    source_str = os.path.realpath(str(source[0]))
    policy_file = os.path.realpath(util.envsubst(policy_file))
    target_dir_realpath = os.path.realpath(util.envsubst(target_base_dir))

    build_policy = BuildPolicy(policy_file, env)

    # don't do anything if none of the tags match.
    if _isPolicyRun(env, disable_featureflags, build_policy, source_str) is True:

        if msmid_jtagid_dict is None:
            msmid_jtagid_dict = get_msm_jtag_dict_from_file(env, msmid)


        rt_signed_list = _execute_sign(env,
                          target_base_dir=target_dir_realpath,
                          source=source_str,
                          sign_id=sign_id,
                          config=os.path.realpath(util.envsubst(config)),
                          msmid_jtagid_dict=msmid_jtagid_dict,
                          build_policy=build_policy,
                          image_entry=image_entry,
                          relocatable=relocatable)
        rt.append(rt_signed_list)

        if sectools_install_base_dir is not None:
            rt_installed_list = _execute_install(env,
                sectools_install_base_dir=sectools_install_base_dir,
                install_file_name=install_file_name,
                sectools_builder_output=rt_signed_list,
                msmid_jtagid_dict=msmid_jtagid_dict,
                build_policy=build_policy)
            rt.append(rt_installed_list)

        if (len(pilsplitter_featureflag) == 0 or
            util.isKeyEnable(pilsplitter_featureflag)) and \
            pilsplitter_target_base_dir is not None:
            rt_pilsplit_list = _execute_pilsplit(env,
                pilsplitter_target_base_dir=pilsplitter_target_base_dir,
                install_file_name=install_file_name,
                sectools_builder_output=rt_signed_list,
                msmid_jtagid_dict=msmid_jtagid_dict,
                build_policy=build_policy,
                pilsplitter_path=pilsplitter_path)
            rt.append(rt_pilsplit_list)


    return rt

def _execute_pilsplit(env,
                     pilsplitter_target_base_dir,
                     install_file_name,
                     sectools_builder_output,
                     msmid_jtagid_dict,
                     build_policy,
                     pilsplitter_path
                     ):

    util = BuilderUtil(env)

    pilsplitter_target_base_dir = os.path.realpath(util.envsubst(pilsplitter_target_base_dir))
    pilsplitter_path = os.path.realpath(util.envsubst(pilsplitter_path))
    install_file_name = util.envsubst(install_file_name)

    rt_list = []
    if c_path.validate_file(pilsplitter_path):
        util.loadToolScript(pilsplitter_path)
    for i, sec_image_policy in enumerate(build_policy.enabled_sec_image_policies):
        rtt = _execute_postprocess_policy(env,
            sectools_install_base_dir=pilsplitter_target_base_dir,
            install_file_name=install_file_name,
            sectools_builder_output=sectools_builder_output[i],
            msmid_jtagid_dict=msmid_jtagid_dict,
            install_policies=build_policy.enabled_install_policies,
            sec_image_policy=sec_image_policy,
            postprocess=POSTPROCESS_PILSPLIT)

        rt_list.append(rtt)

    return rt_list

def _execute_sign(env,
                  target_base_dir,
                  source,
                  sign_id,
                  config,
                  msmid_jtagid_dict,
                  build_policy,
                  image_entry,
                  relocatable):

    rt_signed_list = []
    for sec_image_policy in build_policy.enabled_sec_image_policies:
        rtt=_execute_sec_image_policy(env,
                target_base_dir=target_base_dir,
                source=source,
                sign_id=sign_id,
                config=config,
                msmid_jtagid_dict=msmid_jtagid_dict,
                sec_image_policy=sec_image_policy,
                image_entry=image_entry,
                relocatable=relocatable)
        rt_signed_list.append(rtt)

    return rt_signed_list

def isEnabled(env,
        featureflags=[],
        disable_featureflags=DEFAULT_DISABLE_FEATURE_FLAGS,
        policy_file=DEFAULT_POLICY_FILE):

    build_policy = BuildPolicy(policy_file, env)
    return _isPolicyRun(env, disable_featureflags, build_policy) and build_policy.isFeatureEnabled(featureflags)

def getUnsignedInstallPath(env,
        install_base_dir,
        featureflags=DEFAULT_INSTALL_DEFAULT_FLAGS,
        disable_featureflags=DEFAULT_DISABLE_FEATURE_FLAGS,
        policy_file=DEFAULT_POLICY_FILE):

    build_policy = BuildPolicy(policy_file, env)

    unsigned_install_path = install_base_dir
    if _isPolicyRun(env, disable_featureflags, build_policy) and build_policy.isFeatureEnabled(featureflags):
        unsigned_install_path = os.path.normpath(os.path.join(install_base_dir, "unsigned"))

    return unsigned_install_path

def _isPolicyRun(env, disable_buildtags, build_policy, source=[]):
    isPolicyRun = False

    util = BuilderUtil(env)

    if (disable_buildtags is None or util.isKeyEnable(disable_buildtags) is False) and \
        build_policy.enabled_sec_image_policies != []:
        isPolicyRun = True
    else:
        enabled_policy_ids=[]
        for sec_image_policy in build_policy.enabled_sec_image_policies:
            enabled_policy_ids.append(sec_image_policy.id)
        util.printinfo("sectools is not run for {0} due to build tags constraint.".format(source))
        util.printinfo("enabled_sec_image_policies = {0}".format(enabled_policy_ids))
        util.printinfo("disable_buildtags = %s" % disable_buildtags)

    return isPolicyRun

def _is_excluded(sec_image_policy, sign_id):
    isExcluded = False

    for exclude in sec_image_policy.exclude_list:
        if sign_id in exclude.sign_id:
            isExcluded = True

    return isExcluded


def _execute_sec_image_policy(env, target_base_dir,
        source,
        sign_id,
        config,
        msmid_jtagid_dict,
        sec_image_policy,
        image_entry,
        relocatable):
    #msmid_list can be used to set "chipset" in config file
    msmid_list = msmid_jtagid_dict.keys()
    num_targets = len(msmid_list)
    source_path, source_file = os.path.split(source)

    rt_list = []

    if _is_excluded(sec_image_policy, sign_id) is True:
        #Do not execute if sign_id match the exclude list
        util = BuilderUtil(env)
        util.printinfo("sec_image_policy = {0} is excluded for sign_id = {1}".
                       format(sec_image_policy.id, sign_id))
        return rt_list

    for i in range(0, num_targets):
        policy_target_base_dir = os.path.join(target_base_dir, sec_image_policy.id)
        target_str = os.path.join(policy_target_base_dir, msmid_list[i], sign_id, source_file)

        signerbuilderglue = SecImageBuilderGlue(env)
        rtt = signerbuilderglue.build(target_str, source,
                    sign_id=sign_id,
                    config=config,
                    target_base_dir=policy_target_base_dir,
                    chipset=msmid_list[i],
                    sec_image_policy=sec_image_policy,
                    image_entry=image_entry,
                    relocatable=relocatable)
        rt_list.append(rtt)

    return rt_list

def get_msm_jtag_dict_from_file(env, msmid,
    msm_jtag_mapping_file=DEFAULT_MSM_JTAG_MAPPING_FILE,
    file_type="xml"):

    if msmid is None:
        return DEFAULT_MSM_JTAG_DICT

    util = BuilderUtil(env)

    msm_jtag_mapping_file = os.path.realpath(util.envsubst(msm_jtag_mapping_file))
    util.printinfo("sectools_builder: msm_jtag_mapping_file= %s" % msm_jtag_mapping_file)
    try:
        msmid_jtagid_dict = _get_msm_jtag_dict_from_file_impl(msmid, msm_jtag_mapping_file, file_type)
    except Exception, e:
        util.returnError(str(e))

    return msmid_jtagid_dict

def _get_msm_jtag_dict_from_file_impl(msmid, msm_jtag_mapping_file, file_type="xml"):
    msmid_jtagid_dict = {}
    if not os.path.isfile(msm_jtag_mapping_file):
        #raise RuntimeError, "sectools_builder: msm to jtag mapping file %s does not exist!"  % msm_jtag_mapping_file
        return DEFAULT_MSM_JTAG_DICT

    if file_type.lower() == "txt":
        fp = open(msm_jtag_mapping_file)
        for line in fp:
            (key, val) = line.split()
            msmid_jtagid_dict[key] = val
        fp.close()
    elif file_type.lower() == "xml":
        try:
            msmid_jtagid_dict = msm_jtag_parser.get_jtag_dict_from_msmid(msmid, msm_jtag_mapping_file)
        except Exception, e:
            err_str = "msmid %s cannot be mapped into a valid JTAG ID. Please update %s.\n" % (msmid, msm_jtag_mapping_file)
            err_str = err_str + str(e)
            raise RuntimeError, err_str
    else:
        raise RuntimeError, "sectools_builder_wrapper: file_type = %s is not supported for msm_jtag_mapping_file" % file_type

    return msmid_jtagid_dict

def install(env,
            sectools_install_base_dir,
            sectools_builder_output,
            install_file_name = None,
            msmid = None,
            msmid_jtagid_dict = None,
            msm_jtag_mapping_file=DEFAULT_MSM_JTAG_MAPPING_FILE,
            disable_buildtags=DEFAULT_DISABLE_FEATURE_FLAGS,
            policy_file=DEFAULT_POLICY_FILE):

    rt_list = []
    build_policy = BuildPolicy(policy_file, env)

    if _isPolicyRun(env, disable_buildtags, build_policy, sectools_builder_output) is True:

        if msmid_jtagid_dict is None:
            msmid_jtagid_dict = get_msm_jtag_dict_from_file(env, msmid)

        rt_list = _execute_install(env,
            sectools_install_base_dir=sectools_install_base_dir,
            install_file_name=install_file_name,
            sectools_builder_output=sectools_builder_output[0],
            msmid_jtagid_dict=msmid_jtagid_dict,
            build_policy=build_policy)

    return rt_list

def _execute_install(env,
                     sectools_install_base_dir,
                     install_file_name,
                     sectools_builder_output,
                     msmid_jtagid_dict,
                     build_policy
                     ):

    util = BuilderUtil(env)

    sectools_install_base_dir = os.path.realpath(util.envsubst(sectools_install_base_dir))
    install_file_name = util.envsubst(install_file_name)

    rt_list = []
    for i, sec_image_policy in enumerate(build_policy.enabled_sec_image_policies):
        rtt = _execute_postprocess_policy(env,
            sectools_install_base_dir=sectools_install_base_dir,
            install_file_name=install_file_name,
            sectools_builder_output=sectools_builder_output[i],
            msmid_jtagid_dict=msmid_jtagid_dict,
            install_policies=build_policy.enabled_install_policies,
            sec_image_policy=sec_image_policy)

        rt_list.append(rtt)

    return rt_list

def _execute_postprocess_policy(env,
            sectools_install_base_dir,
            install_file_name,
            sectools_builder_output,
            msmid_jtagid_dict,
            install_policies,
            sec_image_policy,
            postprocess=POSTPROCESS_INSTALL):

    rt_list=[]

    if len(sectools_builder_output) == 0:
        #Empty list - the target is excluded
        return rt_list

    util = BuilderUtil(env)

    for install_policy in install_policies:
        install_directory = install_policy.getDirectory(sec_image_policy.id)
        if install_directory is None:
            continue

        install_directory_processed = signerutils.macro_replace(install_directory,
                            "sectools_install_base_dir",
                            sectools_install_base_dir,
                            isMandatory=False)
        install_directory_processed = os.path.normpath(
                                            util.envsubst(install_directory_processed))

        if install_file_name is not None:
            target_file_path = os.path.join(install_directory_processed,
                                        install_file_name)
        else:
            path, filename = os.path.split(sectools_builder_output[0])
            target_file_path = os.path.join(install_directory_processed, filename)


        if postprocess == POSTPROCESS_INSTALL:
            rt = util.installas(target_file_path, sectools_builder_output[0])
        elif postprocess == POSTPROCESS_PILSPLIT:
            pil_split_path, ext = os.path.splitext(target_file_path)
            rt = util.pilsplit(pil_split_path + ".mdt", sectools_builder_output[0])
        else:
            raise RuntimeError("{0} not supported in _execute_postprocess_policy".format(postprocess))

        rt_list.append(rt)

    return rt_list


class SecImageBuilderGlue(object):

    def __init__(self, env):
        self.env = env
        if env is None:
            self.signerbuilder = GenericSecImageBuilder(env)
        else:
            self.signerbuilder = None

    def build(self,
                target,
                source,
                sign_id,
                config,
                target_base_dir,
                chipset,
                sec_image_policy,
                image_entry,
                relocatable):
        if self.signerbuilder is not None:
            return self.signerbuilder.build(target,
                        source,
                        sign_id,
                        config,
                        target_base_dir,
                        chipset,
                        sec_image_policy,
                        image_entry,
                        relocatable
                        )
        else:
            func_handler = scons_secimage_builder.get_func_handler(self.env)
            return func_handler(target,
                         source,
                         SIGN_ID=sign_id,
                         CONFIG=config,
                         TARGET_BASE_DIR=target_base_dir,
                         CHIPSET=chipset,
                         SEC_IMAGE_POLICY=sec_image_policy,
                         IMAGE_ENTRY=image_entry,
                         RELOCATABLE=relocatable)


