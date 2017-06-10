#===============================================================================
#
# Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
# Qualcomm Technologies Proprietary and Confidential.
#
#===============================================================================

import auto_gen_policy_parser as policy_parser
from builderutils import BuilderUtil

class BuildPolicy(object):
    MSG_ERROR_MULTIPLE_POLICIES_ENABLED = "Only one build policy is allowed but multiple are enabled: {0}"

    def __init__(self, policy_file, env=None):
        self.env = env
        self.policy = policy_parser.parse(policy_file)
        self.builderutil = BuilderUtil(env)
        self.enabled_build_policy = self._get_enabled_build_policy()
        self.enabled_sec_image_policies = self._get_enabled_sec_image_policies(self.enabled_build_policy)
        self.enabled_install_policies = self._get_enabled_install_policy(self.enabled_build_policy)

    def isFeatureEnabled(self, feature_list):
        isEnabled = False

        for sec_image_policy in self.enabled_sec_image_policies:
            isEnabled = isEnabled or sec_image_policy.isFeatureEnabled(feature_list)

        for install_policy in self.enabled_install_policies:
            isEnabled = isEnabled or install_policy.isFeatureEnabled(feature_list)

        return isEnabled

    def _get_enabled_install_policy(self, enabled_build_policy):
        assert self.policy.install_policy

        if enabled_build_policy and enabled_build_policy.install_policy:
            enabled_install_policy = enabled_build_policy.install_policy
        else:
            enabled_install_policy = []

        enabled_policies = self._get_enabled_policies(
                                    self.policy.install_policy.policy,
                                    enabled_install_policy)

        install_policies = []
        for xml_policy in enabled_policies:
            install_policies.append(InstallPolicy(xml_policy))

        return install_policies

    def _get_enabled_sec_image_policies(self, enabled_build_policy):
        assert self.policy.sec_image_policy

        if enabled_build_policy and enabled_build_policy.sec_image_policy:
            enabled_sec_image_policies = enabled_build_policy.sec_image_policy
        else:
            enabled_sec_image_policies = []

        enabled_policies = self._get_enabled_policies(
                                    self.policy.sec_image_policy.policy,
                                    enabled_sec_image_policies)

        sec_image_policies = []
        for xml_policy in enabled_policies:
            sec_image_policies.append(SecImagePolicy(xml_policy))

        return sec_image_policies

    def _get_enabled_build_policy(self):
        if not self.policy.build_policy:
            return None

        enabled_policies = self._get_enabled_policies(
                                            self.policy.build_policy.policy_enable)


        return self._enforce_single_policy(enabled_policies)

    def _get_enabled_policies(self, policy_list, enabled_id_list = []):
        enabled_policies = []
        for i, policy in enumerate(policy_list):
            if policy.feature_flag and self.builderutil.isKeyEnable(policy.feature_flag) or \
                policy.id in enabled_id_list:
                enabled_policies.append(policy)

        return enabled_policies

    def _enforce_single_policy(self, enabled_policies):
        if len(enabled_policies) > 1:
            enabled_flags = []
            for i, policy in enumerate(enabled_policies):
                enabled_flags.append(policy.feature_flag)
            self.builderutil.returnError(self.MSG_ERROR_MULTIPLE_POLICIES_ENABLED.format(enabled_flags))
        elif len(enabled_policies) == 1:
            return_policy = enabled_policies[0]
        else:
            return_policy = None

        return return_policy

class InstallPolicy(object):
    def __init__(self, install_policy_xml):
        self.xml_policy = install_policy_xml

    @property
    def feature_flag(self):
        return self.xml_policy.feature_flag

    def getDirectory(self, sec_image_policy_id):
        for directory in self.xml_policy.directory:
            if directory.sec_image_policy == sec_image_policy_id:
                return directory.value

        return None

    def isFeatureEnabled(self, feature_list):
        isEnabled = False

        for feature in feature_list:
            if (self.feature_flag == feature):
                isEnabled = True

        return isEnabled

class SecImagePolicy(object):
    def __init__(self, sec_image_policy_xml):
        self.xml_policy = sec_image_policy_xml

    @property
    def id(self):
        return self.xml_policy.id

    @property
    def feature_flag(self):
        return self.xml_policy.feature_flag

    @property
    def exclude_list(self):
        return self.xml_policy.exclude

    @property
    def cmd_options(self):
        return self.xml_policy.cmd_options

    def isFeatureEnabled(self, feature_list):
        isEnabled = False

        for feature in feature_list:
            if (self.feature_flag == feature):
                isEnabled = True

        return isEnabled




