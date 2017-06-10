/**
 * Copyright (c) 2013 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __OMADRMENGINECONST_H__
#define __OMADRMENGINECONST_H__

namespace android {

/**
 * Constants for separate and combined delivary Engine used for exposing engine's capabilities.
 */

#define DRM_EXTENSION_DM           ("DM")

#define DRM_DOTEXTENSION_DM        (".dm")
#define DRM_DOTEXTENSION_FL        (".fl")
#define DRM_DOTEXTENSION_DCF        (".dcf")

#define DRM_MIMETYPE_MESSAGE            ("application/vnd.oma.drm.message")
#define DRM_MIMETYPE_CONTENT       ("application/vnd.oma.drm.content")
#define DRM_MIMETYPE_RIGHTS_XML     ("application/vnd.oma.drm.rights+xml")
#define DRM_MIMETYPE_RIGHTS_WXML    ("application/vnd.oma.drm.rights+wbxml")

#define DRM_MIMETYPE_CD            ("application/x-android-drm-cd")
#define DRM_DESCRIPTION            ("A OMA DRM v1 plugin for FL, CD and SD feature support")
};

#endif /* __OMADRMENGINECONST_H__ */
