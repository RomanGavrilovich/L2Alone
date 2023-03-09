#pragma once

#include "VisionUtils.h"

class VisionProvider {
public:
	virtual bool capture(BitMapInfo& bmi) = 0;
};
