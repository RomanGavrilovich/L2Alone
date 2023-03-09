#pragma once

#include <map>

#include "VisionProvider.h"

struct VisionParams {
	map<int, double> hRef;
};

class VisionInitializer {

public:
	virtual VisionParams init(VisionProvider& provider, int timeoutMs) = 0;
};