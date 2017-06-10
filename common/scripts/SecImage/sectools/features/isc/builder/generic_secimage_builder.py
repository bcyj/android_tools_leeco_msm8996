#===============================================================================
#
# Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
# Qualcomm Technologies Proprietary and Confidential.
#
#===============================================================================

from sectools.common.builder.builderutils import BuilderUtil
from sectools.features.isc.cfgparser import ConfigParser
from sectools.common.utils import c_path
import os

BUILDER_DIR = os.path.dirname(os.path.realpath(__file__))
SECTOOLS_DIR = os.path.join(BUILDER_DIR, "..", "..", "..", "..")

class GenericSecImageBuilder(object):

    def __init__(self, env):
        self.env = env
        self.builderutil = BuilderUtil(env)

    def _generate_config_file(self, src_config, dest_config,
                              image_entry, relocatable):
        config = ConfigParser(src_config)

        image_type_list = config.root.parsegen.get_image_types_list()
        for image_type in image_type_list.get_image_type():
            if image_type.id == "elf_wrapped_mbn":
                #Override relocatable setting
                image_type.ewm_properties.relocatable = relocatable
                if image_entry is not None:
                    #Override image_entry
                    image_type.ewm_properties.image_entry = image_entry

        config.generate(dest_config)

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

        #import pdb; pdb.set_trace()
        self.builderutil.printinfo("sectools_signer_builder: SECTOOLS_DIR = %s" % SECTOOLS_DIR)

        if sign_id == "mba_ewm":
            c_path.create_dir(target_base_dir)
            generated_config = os.path.join(target_base_dir, "generated_config.xml")
            self._generate_config_file(config, generated_config, image_entry, relocatable)
            config_used = generated_config
        else:
            config_used = config

        # Issuing sectools command
        cmds = ["python", os.path.join(SECTOOLS_DIR, "sectools.py"), "secimage", "-i", source,
                "-o", target_base_dir, "-g", sign_id, "-c", config_used]

        cmds.append(sec_image_policy.cmd_options)

        self.builderutil.printinfo("sectools_signer_builder: %s" % " ".join(cmds))
        self.builderutil.execcmds(cmds, target=target)

        return self.builderutil.getReturnValueInBuilder(target)

