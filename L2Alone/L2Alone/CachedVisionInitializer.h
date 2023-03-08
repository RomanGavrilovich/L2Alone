#pragma once

#include "VisionUtils.h"

class VisionCache {

public:
	virtual bool get(VisionParams& vp) = 0;

	virtual void set(VisionParams& vp) = 0;
};

class CachedVisionInitializer : public VisionInitializer {

public:
	CachedVisionInitializer(VisionCache* pCache, VisionInitializer *pTarget);

	VisionParams init(HWND hWindow, int timeoutMs) override;

private:
	VisionCache* pCache;
	VisionInitializer* pTarget;
};

CachedVisionInitializer::CachedVisionInitializer(VisionCache* pCache, VisionInitializer *pTarget) {
	this->pCache = pCache;
	this->pTarget = pTarget;
}

VisionParams CachedVisionInitializer::init(HWND hWindow, int timeoutMs) {
	VisionParams vp;
	if (pCache->get(vp)) {
		return vp;
	}

	vp = pTarget->init(hWindow, timeoutMs);
	pCache->set(vp);

	return vp;
}