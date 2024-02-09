#pragma once

#include "LoadingAwaiter.h"
#include "VisionProvider.h"

class NoOpLoadingAwaiter : public LoadingAwaiter {

public:
	void await(HWND hWindow, VisionProvider& bmi) override;
};

void NoOpLoadingAwaiter::await(HWND hWindow, VisionProvider& vp) {

}