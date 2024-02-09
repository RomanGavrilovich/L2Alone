#pragma once

#include "VisionUtils.h"

class LoadingAwaiter {

public:
	virtual void await(HWND hWindow, VisionProvider& vp) = 0;
};