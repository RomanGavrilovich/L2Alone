#pragma once

#include "LayoutCache.h"
#include <memory>

using namespace std;

class InMemoryLayoutCache : public LayoutCache {
public:

	InMemoryLayoutCache(shared_ptr<LayoutCache> target) {
		this->target = target;
	}

	bool load(LayoutCacheEntity& e) override {
		if (initialized) {
			e = cached;
			return true;
		}

		bool r = target->load(e);
		if (r) {
			initialized = true;
			cached = e;
		}

		return r;
	}

	void save(LayoutCacheEntity& e) override {
		cached = e;
		initialized = true;

		target->save(e);
	}

private:
	bool initialized = false;
	LayoutCacheEntity cached;
	shared_ptr<LayoutCache> target;
};