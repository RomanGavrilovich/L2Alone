#pragma once

#include <map>

#include "VisionProvider.h"

struct VisionParams {
	map<int, double> hRef;
	int systemMessageLength = 0;
	HSV textColor;
};

class VisionInitializer {

public:
	virtual VisionParams init(VisionProvider& provider, int timeoutMs) = 0;

protected:
	const int CAPTURE_RETRY_DELAY = 100;
};