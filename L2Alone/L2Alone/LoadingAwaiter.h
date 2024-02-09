#pragma once

#include "VisionUtils.h"

class LoadingAwaiter {

public:
	virtual void await(VisionProvider& vp) = 0;
};