#pragma once

#include "VisionUtils.h"

class LoadingPredicate {

public:
	virtual void waitForInputReadiness(BitMapInfo& bmi) = 0;
};