#pragma once

#include "L2EventHandlers.h"
#include "Logger.h"

#include <Windows.h>

class LayoutManager {

public:
	
	void setWindowLayout(RECT r) {
		logger.log("Window position changed");

		this->layout = r;
		initialized = true;
	}

	bool getWindowLayout(RECT& r) {

		if (initialized) {
			r = this->layout;
		}
		
		return initialized;
	}

private:
	RECT layout;
	bool initialized = false;
};