#pragma once

#include <Windows.h>

class L2WindowRectChangeHandler {

public:
	virtual void onWindowChange(RECT r) = 0;
};	