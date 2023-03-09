#pragma once

#include "VisionUtils.h"
#include "VisionProvider.h"
#include "VisionInitializer.h"

class VisionCache {

public:
	virtual bool get(VisionParams& vp) = 0;

	virtual void set(VisionParams& vp) = 0;
};

class CachedVisionInitializer : public VisionInitializer {

public:
	CachedVisionInitializer(VisionCache* pCache, VisionInitializer* pTarget);

	VisionParams init(VisionProvider& vp, int timeoutMs) override;

private:
	VisionCache* pCache;
	VisionInitializer* pTarget;

	void waitForLoadingScreen(VisionProvider &vp, int timeoutMs);
};

CachedVisionInitializer::CachedVisionInitializer(VisionCache* pCache, VisionInitializer* pTarget) {
	this->pCache = pCache;
	this->pTarget = pTarget;
}

VisionParams CachedVisionInitializer::init(VisionProvider& provider, int timeoutMs) {

	VisionParams vp;
	if (pCache->get(vp)) {
		waitForLoadingScreen(provider, timeoutMs);
		vp;

	}
	else {
		vp = pTarget->init(provider, timeoutMs);
		pCache->set(vp);
	}

	return vp;
}

void CachedVisionInitializer::waitForLoadingScreen(VisionProvider& provider, int timeoutMs) {

	BitMapInfo bitMapInfo;

	long startTime = GetTickCount64();
	while (GetTickCount64() - startTime < timeoutMs) {
		if (provider.capture(bitMapInfo)) {
			if (isLoadingWindow(bitMapInfo)) {
				logger.log("Loading screen detected");
				return;
			}
		}

		Sleep(100);
	}

	throw exception("Can't detect loading screen");
}