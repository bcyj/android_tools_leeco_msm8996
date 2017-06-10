#===============================================================================
#
# Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
# Qualcomm Technologies Proprietary and Confidential.
#
#===============================================================================

'''
Created on Feb 21, 2014

@author: hraghav
'''


import os
import sys

from . import BaseStager
from sectools.features.isc.cfgparser import ConfigDir, ConfigParser
from sectools.common.utils import c_path
from sectools.common.utils.c_logging import logger
from sectools.features.isc.imageinfo import ImageInfo, DestImagePath, ImagePath


class MetaBuildStager(BaseStager):

    META_LIB_PATH_REL = 'common/tools/meta'

    def __init__(self, meta_build_path, config_dir_obj, sign_id_list=[]):
        assert isinstance(meta_build_path, str)
        assert isinstance(config_dir_obj, ConfigDir)

        # Initialize the BaseStager
        BaseStager.__init__(self)

        self.config_dir_obj = config_dir_obj

        # Create internal attributes
        self._meta_build_path = meta_build_path

        # Validate that the meta_build path exists
        meta_build_path = c_path.normalize(meta_build_path)
        if not c_path.validate_dir(meta_build_path):
            raise RuntimeError('No read access to the meta build path: ' + meta_build_path)

        # Get the meta lib module from the metabuild
        meta_info = self.get_meta_info(meta_build_path)

        # Create the image info list based on the meta data
        for sign_id, chipset, image_src_path, image_dest_path in self.get_image_info_from_meta(meta_info):
            # Filer on the sign id
            if sign_id_list:
                if sign_id not in sign_id_list:
                    continue

            try:
                img_config_parser = self.get_image_config_parser(chipset)

                # Validate the sign_id
                sign_id = self._get_sign_id(img_config_parser,
                                            os.path.basename(image_src_path.image_path),
                                            sign_id)

                # Get the config block for the sign id
                img_config_block = img_config_parser.get_config_for_sign_id(sign_id)

                # Create the one image info object
                image_info = ImageInfo('', sign_id, img_config_block,
                                       img_config_parser)

                # Set the src path
                image_info.src_image = image_src_path
                image_info.image_under_operation = image_info.src_image.image_path

                # Set the dest path
                image_info.dest_image = image_dest_path

                # Check if the dest image name should be overriden
                if img_config_block.output_file_name is not None:
                    image_info.dest_image.image_name = img_config_block.output_file_name

                # Put the image info object into the list
                self._image_info_list.append(image_info)

            except Exception as e:
                logger.error(str(e))

        if not self._image_info_list:
            raise RuntimeError('No images found from the meta build.')

    def get_image_config_parser(self, chipset):
        return ConfigParser(self.config_dir_obj.get_chipset_config_path(chipset))

    @classmethod
    def get_meta_info(cls, meta_build_path):
        sys.path.append(c_path.join(meta_build_path, cls.META_LIB_PATH_REL))
        import meta_lib
        return meta_lib.meta_info(meta_build_path)

    @classmethod
    def get_image_info_from_meta(cls, meta_info):
        # Get a list of all files tagged with sign_id
        meta_images_list = meta_info.get_files_detailed('sign_id')

        for image in meta_images_list:
            try:
                logger.debug('Found image from meta_build for signing: ' + image.sign_id)

                source_path = None
                dest_path = None
                for each_path in image.file_path:
                    if source_path is None:
                        if getattr(each_path, 'sign_source', False):
                            source_path = each_path.value
                    if dest_path is None:
                        if getattr(each_path, 'sign_dest', False):
                            dest_path = each_path.value
                    if source_path and dest_path:
                        break

                if source_path is None or dest_path is None:
                    raise RuntimeError('SourcePath, DestPath should not be missing.')

                sign_id = image.sign_id
                chipset = image.chipset
                image_src_path = ImagePath()
                image_dest_path = DestImagePath()

                image_src_path.image_dir_base = image.image_dir_base
                image_src_path.image_dir_ext = source_path
                image_src_path.image_name = image.file_name[0].value
                image_dest_path.image_dir_base = image.image_dir_base
                image_dest_path.image_dir_ext = dest_path
                image_dest_path.image_name = image.file_name[0].value

            except Exception as e:
                logger.error(str(e))
                continue

            yield (sign_id, chipset, image_src_path, image_dest_path)

    @classmethod
    def meta_supports_sign_id(cls, meta_build_path):
        meta_info = cls.get_meta_info(meta_build_path)
        return (len(meta_info.get_file_vars('sign_id')) > 0)

