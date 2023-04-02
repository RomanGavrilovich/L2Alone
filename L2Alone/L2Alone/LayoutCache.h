#pragma once

#include <Windows.h>

struct LayoutCacheEntity {
	RECT r;
};

class LayoutCache {

public:
	virtual bool load(LayoutCacheEntity& e) = 0;

	virtual void save(LayoutCacheEntity& e) = 0;
};

class NoOpLayoutCache : public LayoutCache {
public:

	virtual bool load(LayoutCacheEntity& e) {
		return false;
	}

	virtual void save(LayoutCacheEntity& e) {
		// Do nothing
	}
};