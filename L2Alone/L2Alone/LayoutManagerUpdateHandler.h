#pragma once

#include "L2EventHandlers.h"
#include "LayoutManager.h"

#include <Windows.h>

class LayoutManagerUpdateHandler : public L2WindowRectChangeHandler {

public:
	LayoutManagerUpdateHandler(shared_ptr<LayoutManager> pLayoutManager) {
		this->pLayoutManager = pLayoutManager;
	}

	virtual void onWindowChange(RECT r) {
		this->pLayoutManager->setWindowLayout(r);
	}

private:
	shared_ptr<LayoutManager> pLayoutManager;
};