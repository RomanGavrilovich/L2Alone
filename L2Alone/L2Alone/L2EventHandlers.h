#pragma once

#include <Windows.h>
#include "Utils.h"

class L2WindowRectChangeHandler {

public:
	virtual void onWindowChange(RECT r) = 0;
};	

class L2WindowCreateHandler {
public:
	virtual void onWindowCreate(HWND hWindow) = 0;
};

class SubmitL2WindowCreateHandler : public L2WindowCreateHandler {
public:
	void onWindowCreate(HWND hWindow) {
		postControlMessage(hWindow, VK_RETURN);
	}
};