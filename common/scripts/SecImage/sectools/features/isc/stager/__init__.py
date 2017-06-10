#===============================================================================
#
# Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
# Qualcomm Technologies Proprietary and Confidential.
#
#===============================================================================

"""Creates the :class:`..imageinfo.ImageInfo` objects and configures the paths
and ids of these objects based on the params. Takes care of any staging
requirements.
"""


import abc

from sectools.common.utils import c_path


class BaseStager(object):
    __metaclass__ = abc.ABCMeta

    @abc.abstractmethod
    def __init__(self):
        self._output_dir = ''
        self._mini_build_path = ''
        self._image_info_list = []

    @property
    def image_path_list(self):
        return [image_info.src_image.image_path for image_info in self.image_info_list]

    @property
    def image_info_list(self):
        return self._image_info_list

    @property
    def output_dir(self):
        """(str) Output directory to store the output data to."""
        return self._output_dir

    @output_dir.setter
    def output_dir(self, output_dir):
        assert isinstance(output_dir, str)
        output_dir = c_path.normalize(output_dir)
        if not c_path.validate_dir_write(output_dir):
            raise RuntimeError('No write access to output directory: ' + output_dir)

        # Update the output dir of each image in image_info_list
        for image_info in self.image_info_list:
            image_info.dest_image.image_dir_base = output_dir
            image_info.dest_image.image_dir_ext = (image_info.chipset + '/' +
                                                   image_info.sign_id)
        self._output_dir = output_dir

    @property
    def mini_build_path(self):
        return self._mini_build_path

    @mini_build_path.setter
    def mini_build_path(self, mini_build_path):
        assert isinstance(mini_build_path, str)
        mini_build_path = c_path.normalize(mini_build_path)
        if not c_path.validate_dir_write(mini_build_path):
            raise RuntimeError('No write access to minimized build directory: ' + mini_build_path)

        # Update the output dir of each image in image_info_list
        for image_info in self.image_info_list:
            image_info.dest_image.image_dir_base = mini_build_path
        self._mini_build_path = mini_build_path

    def _get_sign_id(self, img_config_parser, image_name, sign_id=None):
        if sign_id is not None:
            # If sign_id is given, check that the sign id exists
            available_sign_ids = img_config_parser.sign_id_list
            if sign_id not in available_sign_ids:
                raise RuntimeError('Unknown sign id provided: ' + sign_id + '\n'
                                   '    Available sign ids: ' + str(available_sign_ids))
        else:
            # Figure out the sign_id if it is not given using the name of the file
            sign_id = img_config_parser.get_sign_id_for_image_name(image_name)
        return sign_id


#------------------------------------------------------------------------------
# Restrict all import
#------------------------------------------------------------------------------
from image_paths_stager import ImagePathsStager
from meta_build_stager import MetaBuildStager
from meta_build_tmp_stager import MetaBuildTmpStager

__all__ = ['ImagePathsStager',
           'MetaBuildStager',
           'MetaBuildTmpStager',]
