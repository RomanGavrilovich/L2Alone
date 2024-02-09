#pragma once

#include "LoadingAwaiter.h"
#include "VisionProvider.h"

class NoOpLoadingAwaiter : public LoadingAwaiter {

public:
	void await(VisionProvider& bmi) override;
};

void NoOpLoadingAwaiter::await(VisionProvider& vp) {

}