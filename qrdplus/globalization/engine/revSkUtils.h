/****************************************************************************

**

** Copyright (C) 2013 Reverie Language Technologies.

** http://www.reverie.co.in

**

** This file is part of Rendering Utilities.

****************************************************************************/


#ifndef __REVSKUTILS_H__
#define __REVSKUTILS_H__


#include "SkPaint.h"
#include "SkDrawProcs.h"
#include "SkScalar.h"

#ifdef __cplusplus
extern "C" {
#endif


typedef int64_t Sk48Dot16;
typedef void (*JoinBoundsProc)(const SkGlyph&, SkRect*, Sk48Dot16);

int getPaintWidth(const SkPaint* paint, const char *text, size_t byteLength,
	SkScalar widths[], SkRect bounds[], SkScalar& scale, SkGlyphCache* cache,
	SkMeasureCacheProc& glyphCacheProc, const int& xyIndex, int count);

int getPaintMText(const SkPaint *paint, SkGlyphCache* cache, const char* text,
	size_t& byteLength, int* count, SkRect* bounds, JoinBoundsProc joinBoundsProc,
	SkMeasureCacheProc glyphCacheProc, int& xyIndex);

void drawAdd(const char text[], const SkPaint& paint, size_t& byteLength, SkDraw1Glyph& d1g,
	SkDraw1Glyph::Proc& proc, SkGlyphCache *cache, SkFixed& fx, SkFixed& fy,
		SkAutoKern& autokern);

void drawAdd1(const char text[], const SkPaint& paint, size_t& byteLength, SkDraw1Glyph& d1g,
	SkDraw1Glyph::Proc& proc, SkGlyphCache *cache, SkFixed& fx, SkFixed& fy);

#ifdef __cplusplus
}
#endif
#endif /*__REVSKUTILS_H__*/

