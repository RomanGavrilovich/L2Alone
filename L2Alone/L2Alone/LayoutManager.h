#pragma once

#include "L2EventHandlers.h"
#include "Logger.h"
#include "LayoutCache.h"

#include <Windows.h>

class LayoutManager {

public:

	LayoutManager(shared_ptr<LayoutCache> pLayoutCache) {
		this->pLayoutCache = pLayoutCache;
	}
	
	void setWindowLayout(RECT r) {
		auto cacheEntity = LayoutCacheEntity{ r };
		this->pLayoutCache->save(cacheEntity);
	}

	bool getWindowLayout(RECT& r) {
		LayoutCacheEntity cacheEntity;
		bool loaded = this->pLayoutCache->load(cacheEntity);

		if (loaded) {
			r = cacheEntity.r;
		}

		return loaded;
	}

private:
	shared_ptr<LayoutCache> pLayoutCache;
};