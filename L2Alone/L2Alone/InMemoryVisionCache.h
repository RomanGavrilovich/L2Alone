#pragma once

#include "CachedVisionInitializer.h"

class InMemoryVisionCache : public VisionCache {

public:
	bool get(VisionParams& vp);

	void set(VisionParams& vp);

private:
	VisionParams vp;
};

bool InMemoryVisionCache::get(VisionParams& vp) {
	if (vp.hRef.size() == 0) {
		return false;
	}

	vp = this->vp;
	return true;
}

void InMemoryVisionCache::set(VisionParams& vp) {
	this->vp = vp;
}